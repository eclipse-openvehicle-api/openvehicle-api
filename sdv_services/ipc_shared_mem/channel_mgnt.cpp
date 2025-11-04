#include "channel_mgnt.h"
#include "connection.h"
#include <support/toml.h>

void CSharedMemChannelMgnt::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    if (m_eObjectStatus != sdv::EObjectStatus::initialization_pending)
    {
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CSharedMemChannelMgnt::GetStatus() const
{
    return m_eObjectStatus;
}

void CSharedMemChannelMgnt::SetOperationMode(sdv::EOperationMode eMode)
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

void CSharedMemChannelMgnt::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;
    m_watchdog.Clear();
    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::ipc::SChannelEndpoint CSharedMemChannelMgnt::CreateEndpoint(/*in*/ const sdv::u8string& ssEndpointConfig)
{
    std::string ssName;
    uint32_t uiSize = 10*1024;
    if (!ssEndpointConfig.empty())
    {
        sdv::toml::CTOMLParser config(ssEndpointConfig);
        ssName = static_cast<std::string>(config.GetDirect("IpcChannel.Name").GetValue());
        if (!ssName.empty())
        {
            uiSize = config.GetDirect("IpcChannel.Size").GetValue();
            if (!uiSize) uiSize = 128 * 1024;
        }
    }

    // Create a connection
    std::shared_ptr<CConnection> ptrConnection = std::make_shared<CConnection>(m_watchdog, uiSize, ssName, true);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrConnection)
        return {};
    m_watchdog.AddConnection(ptrConnection);

    // Return the connection details.
    sdv::ipc::SChannelEndpoint connectionEndpoint{};
    connectionEndpoint.pConnection = static_cast<IInterfaceAccess*>(ptrConnection.get());
    connectionEndpoint.ssConnectString = ptrConnection->GetConnectionString();
    return connectionEndpoint;
}

sdv::IInterfaceAccess* CSharedMemChannelMgnt::Access(const sdv::u8string& ssConnectString)
{
    sdv::toml::CTOMLParser parser(ssConnectString);
    if (!parser.IsValid()) return nullptr;

    // Is this a configuration provided by the endpoint (uses a "Provider" key), then this is a connection string. Use this
    // to connect to the shared memory.
    std::shared_ptr<CConnection> ptrConnection;
    if (parser.GetDirect("Provider").IsValid())
        ptrConnection = std::make_shared<CConnection>(m_watchdog, ssConnectString.c_str());
    else
    {
        std::string ssName = static_cast<std::string>(parser.GetDirect("IpcChannel.Name").GetValue());
        ptrConnection = std::make_shared<CConnection>(m_watchdog, 0,ssName, false);
    }
    if (!ptrConnection) return {};
    m_watchdog.AddConnection(ptrConnection);

    // Return the connection
    IInterfaceAccess* pInterface = ptrConnection.get();
    return pInterface;
}
