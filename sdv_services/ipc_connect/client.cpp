#include "client.h"
#include <support/toml.h>
#include <interfaces/com.h>
#include <interfaces/ipc.h>
#include <support/pssup.h>
#include <interfaces/app.h>

CRepositoryProxy::CRepositoryProxy(CClient& rClient, sdv::com::TConnectionID tConnection,
    sdv::IInterfaceAccess* pRepositoryProxy) :
    m_rClient(rClient), m_tConnection(tConnection), m_ptrRepositoryProxy(pRepositoryProxy)
{}

void CRepositoryProxy::DestroyObject()
{
    // Call the client to disconnect the connection and destroy the object.
    m_rClient.Disconnect(m_tConnection);
}

void CClient::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CClient::GetStatus() const
{
    return m_eObjectStatus;
}

void CClient::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::configuring;
        break;
    case sdv::EOperationMode::running:
        if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::running;
        break;
    default:
        break;
    }
}

void CClient::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
        SDV_LOG_ERROR("Failed to get communication control!");

    // Disconnect from all repositories
    std::unique_lock<std::mutex> lock(m_mtxRepositoryProxies);
    auto mapRepositoryProxiesCopy = std::move(m_mapRepositoryProxies);
    lock.unlock();
    if (pConnectionControl)
    {
        for (const auto& rvtRepository : mapRepositoryProxiesCopy)
            pConnectionControl->RemoveConnection(rvtRepository.first);
    }

    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::IInterfaceAccess* CClient::Connect(const sdv::u8string& ssConnectString)
{
    const sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (!pContext)
    {
        SDV_LOG_ERROR("Failed to get application context!");
        return nullptr;
    }
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return nullptr;
    }

    sdv::ipc::IChannelAccess* pChannelAccess = nullptr;
    std::string ssConfig;
    try
    {
        // Determine whether the service is running as server or as client.
        sdv::toml::CTOMLParser config(ssConnectString);
        std::string ssType = config.GetDirect("Client.Type").GetValue();
        if (ssType.empty()) ssType = "Local";
        if (ssType == "Local")
        {
            uint32_t uiInstanceID = config.GetDirect("Client.Instance").GetValue();
            pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("LocalChannelControl");
            if (!pChannelAccess)
            {
                SDV_LOG_ERROR("No local channel control or channel control not configured as client!");
                return nullptr;
            }

            ssConfig = std::string(R"code([IpcChannel]
Name = "LISTENER_)code") + std::to_string(uiInstanceID ? uiInstanceID : pContext->GetInstanceID()) + R"code("
)code";

        }
        else if (ssType == "Remote")
        {
            std::string ssInterface = config.GetDirect("Client.Interface").GetValue();
            uint32_t uiPort = config.GetDirect("Client.Interface").GetValue();
            if (ssInterface.empty() || !uiPort)
            {
                SDV_LOG_ERROR("Missing interface or port number to initialize a remote client!");
                return nullptr;
            }
            pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("RemoteChannelControl");
            if (!pChannelAccess)
            {
                SDV_LOG_ERROR("No remote channel control or channel control not configured as client!");
                return nullptr;
            }

            ssConfig = R"code([IpcChannel]
Interface = ")code" + ssInterface + R"code(
Port = ")code" + std::to_string(uiPort) + R"code(
)code";
        }
        else
        {
            SDV_LOG_ERROR("Invalid or missing listener configuration for listener service!");
            m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
            return nullptr;
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        SDV_LOG_ERROR("Invalid service configuration for listener service: ", rexcept.what(), "!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return nullptr;
    }

    // First access the listener channel. This allows us to access the channel creation interface.

    // TODO: Use named mutex to prevent multiple connections at the same time.
    // Connect to the channel.
    sdv::TObjectPtr ptrListenerEndpoint = pChannelAccess->Access(ssConfig);

    // Assign the endpoint to the communication service.
    sdv::IInterfaceAccess* pListenerProxy = nullptr;
    sdv::com::TConnectionID tListenerConnection = pConnectionControl->AssignClientEndpoint(ptrListenerEndpoint, 5000,
        pListenerProxy);
    ptrListenerEndpoint.Clear();    // Lifetime has been taken over by communication control.
    if (!tListenerConnection || !pListenerProxy)
    {
        SDV_LOG_ERROR("Could not assign the client endpoint!");
        if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);
        return nullptr;
    }
    sdv::TInterfaceAccessPtr ptrListenerProxy(pListenerProxy);

    // Request for a private channel
    sdv::com::IRequestChannel* pRequestChannel = ptrListenerProxy.GetInterface<sdv::com::IRequestChannel>();
    if (!pRequestChannel)
    {
        SDV_LOG_ERROR("Could not get the channel creation interface!");
        if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);
        return nullptr;
    }
    sdv::u8string ssConnectionString = pRequestChannel->RequestChannel("");

    // Disconnect from the listener
    if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);

    if (ssConnectionString.empty())
    {
        SDV_LOG_ERROR("Could not get the private channel connection information!");
        return nullptr;
    }

    // TODO: Use named mutex to prevent multiple connections at the same time.
    // Connect to the privatechannel.
    sdv::TObjectPtr ptrPrivateEndpoint = pChannelAccess->Access(ssConnectionString);

    // Get and return the proxy
    sdv::IInterfaceAccess* pPrivateProxy = nullptr;
    sdv::com::TConnectionID tPrivateConnection = pConnectionControl->AssignClientEndpoint(ptrPrivateEndpoint, 5000, pPrivateProxy);
    ptrPrivateEndpoint.Clear();    // Lifetime has been taken over by communication control.
    if (!tPrivateConnection || !pPrivateProxy)
    {
        SDV_LOG_ERROR("Could not assign the client endpoint to the private channel!");
        if (tPrivateConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tPrivateConnection);
        return nullptr;
    }

    // Create a remote repository object
    std::unique_lock<std::mutex> lock(m_mtxRepositoryProxies);
    m_mapRepositoryProxies.try_emplace(tPrivateConnection, *this, tPrivateConnection, pPrivateProxy);

    return pPrivateProxy;
}

void CClient::Disconnect(sdv::com::TConnectionID tConnectionID)
{
    // Find the connection, disconnect and remove the connection from the repository list.
    std::unique_lock<std::mutex> lock(m_mtxRepositoryProxies);
    auto itRepository = m_mapRepositoryProxies.find(tConnectionID);
    if (itRepository == m_mapRepositoryProxies.end()) return;

    // Disconnect
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
        SDV_LOG_ERROR("Failed to get communication control!");
    else
        pConnectionControl->RemoveConnection(itRepository->first);

    // Remove entry
    m_mapRepositoryProxies.erase(itRepository);
}
