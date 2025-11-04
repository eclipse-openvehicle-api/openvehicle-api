#include "listener.h"
#include <support/toml.h>
#include <interfaces/com.h>
#include <interfaces/app.h>
#include <support/pssup.h>

CChannelBroker::CChannelBroker(CListener& rListener) : m_rListener(rListener)
{}

sdv::u8string CChannelBroker::RequestChannel(/*in*/ const sdv::u8string& /*ssConfig*/)
{
    // Get the communication control
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return {};
    }

    // Get the repository
    sdv::TInterfaceAccessPtr ptrRespository = sdv::core::GetObject("RepositoryService");
    if (!ptrRespository)
    {
        SDV_LOG_ERROR("Failed to get repository service!");
        return {};
    }

    // Get the channel control.
    sdv::ipc::ICreateEndpoint* pEndpoint = nullptr;
    if (m_rListener.IsLocalListener())
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    else
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("RemoteChannelControl");
    if (!pEndpoint)
    {
        SDV_LOG_ERROR("No local channel control!");
        return {};
    }

    // Create the endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(sdv::u8string());
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for channel request!");
        return sdv::u8string();
    }
    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

    // Assign the endpoint to the communication service.
    sdv::com::TConnectionID tConnection = pConnectionControl->AssignServerEndpoint(ptrEndpoint, ptrRespository, 100, false);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.
    if (!tConnection)
    {
        SDV_LOG_ERROR("Could not assign the server endpoint!");
        return {};
    }

    if (sdv::app::ConsoleIsVerbose())
        std::cout << "Client connection established..." << std::endl;

    // Return the connection string
    return sEndpoint.ssConnectString;
}

CListener::CListener() : m_broker(*this)
{}

void CListener::Initialize(const sdv::u8string& ssObjectConfig)
{
    const sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (!pContext)
    {
        SDV_LOG_ERROR("Failed to get application context!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    sdv::ipc::ICreateEndpoint* pEndpoint = nullptr;
    std::string ssConfig;
    try
    {
        // Determine whether the service is running as server or as client.
        sdv::toml::CTOMLParser config(ssObjectConfig);
        std::string ssType = config.GetDirect("Listener.Type").GetValue();
        if (ssType == "Local")
        {
            uint32_t uiInstanceID = config.GetDirect("Listener.Instance").GetValue();
            m_bLocalListener = true;
            pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
            if (!pEndpoint)
            {
                SDV_LOG_ERROR("No local channel control!");
                m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
                return;
            }

            // Request the instance ID from the app control
            ssConfig = std::string(R"code([IpcChannel]
Name = "LISTENER_)code") + std::to_string(uiInstanceID ? uiInstanceID : pContext->GetInstanceID()) + R"code("
Size = 2048
)code";
        }
        else if (ssType == "Remote")
        {
            m_bLocalListener = false;
            std::string ssInterface = config.GetDirect("Listener.Interface").GetValue();
            uint32_t uiPort = config.GetDirect("Listener.Interface").GetValue();
            if (ssInterface.empty() || !uiPort)
            {
                SDV_LOG_ERROR("Missing interface or port number to initialize a remote listener!");
                m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
                return;
            }
            pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("RemoteChannelControl");
            if (!pEndpoint)
            {
                SDV_LOG_ERROR("No remote channel control!");
                m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
                return;
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
            return;
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        SDV_LOG_ERROR("Invalid service configuration for listener service: ", rexcept.what(), "!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Create the endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(ssConfig);
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for listener service!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

    // Assign the endpoint to the communication service.
    m_tConnection = pConnectionControl->AssignServerEndpoint(ptrEndpoint, &m_broker, 100, true);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.
    if (!m_tConnection)
    {
        SDV_LOG_ERROR("Could not assign the server endpoint!");
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CListener::GetStatus() const
{
    return m_eObjectStatus;
}

void CListener::SetOperationMode(sdv::EOperationMode eMode)
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

void CListener::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Shutdown the listener...
    if (m_tConnection != sdv::com::TConnectionID{})
    {
        sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
        if (!pConnectionControl)
            SDV_LOG_ERROR("Failed to get communication control; cannot shutdown gracefully!");
        else
            pConnectionControl->RemoveConnection(m_tConnection);
        m_tConnection = {};
    }

    m_ptrConnection.Clear();

    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

bool CListener::IsLocalListener() const
{
    return m_bLocalListener;
}


