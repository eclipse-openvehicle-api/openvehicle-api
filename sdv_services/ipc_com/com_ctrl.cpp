#include "com_ctrl.h"
#include <interfaces/ipc.h>
#include <support/toml.h>
#include "com_channel.h"
#include "marshall_object.h"

thread_local CChannelConnector* CCommunicationControl::m_pConnectorContext = nullptr;

CCommunicationControl::CCommunicationControl()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

CCommunicationControl::~CCommunicationControl()
{}

void CCommunicationControl::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CCommunicationControl::GetStatus() const
{
    return m_eObjectStatus;
}

void CCommunicationControl::SetOperationMode(sdv::EOperationMode eMode)
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

void CCommunicationControl::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Wait for threads to terminate... in case one is still running
    // (use a copy to prevent circular access)
    std::unique_lock<std::mutex> lock(m_mtxChannels);
    auto vecInitialConnectMon = std::move(m_vecInitialConnectMon);
    lock.unlock();
    for (std::thread& rthread : vecInitialConnectMon)
    {
        if (rthread.joinable())
            rthread.join();
    }
    vecInitialConnectMon.clear();

    // Clear the channels and remove the stub objects (use a copy to prevent circular access)
    lock.lock();
    auto vecChannels = std::move(m_vecChannels);
    auto mapStubObjects = std::move(m_mapStubObjects);
    lock.unlock();
    vecChannels.clear();
    mapStubObjects.clear();

    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::com::TConnectionID CCommunicationControl::CreateServerConnection(/*in*/ sdv::com::EChannelType eChannelType,
    /*in*/ sdv::IInterfaceAccess* pObject, /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::u8string& ssConnectionString)
{
    std::string ssChannelServer;
    switch (eChannelType)
    {
    case sdv::com::EChannelType::local_channel:
        ssChannelServer = "LocalChannelControl";
        break;
    case sdv::com::EChannelType::remote_channel:
        ssChannelServer = "RemoteChannelControl";
        break;
    default:
        return {};
    }

    // Create the channel endpoint
    sdv::ipc::ICreateEndpoint* pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>(ssChannelServer);
    if (!pEndpoint)
    {
        SDV_LOG_ERROR("No channel control!");
        return {};
    }

    // Create a standard endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint("");
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint!");
        return {};
    }

    // Manage the channel endpoint object
    sdv::TObjectPtr ptrChannelEndpoint(sEndpoint.pConnection);

    // Manage the endpoint connection object.
    sdv::com::TConnectionID tConnectionID = AssignServerEndpoint(ptrChannelEndpoint, pObject, uiTimeoutMs, false);
    if (!tConnectionID.uiControl) return {};

    ssConnectionString = sEndpoint.ssConnectString;
    return tConnectionID;
}

sdv::com::TConnectionID CCommunicationControl::CreateClientConnection(/*in*/ const sdv::u8string& ssConnectionString,
    /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::IInterfaceAccess*& pProxy)
{
    pProxy = nullptr;

    // The channel connection string contains the nme of the provider
    sdv::toml::CTOMLParser parser(ssConnectionString);
    if (!parser.IsValid()) return {};
    std::string ssProvider = parser.GetDirect("Provider.Name").GetValue();

    // Get the channel access interface
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>(ssProvider);
    if (!pChannelAccess)
    {
        SDV_LOG_ERROR("No channel control!");
        return {};
    }

    // Create the endpoint and establish the channel
    sdv::TObjectPtr ptrChannelEndpoint(pChannelAccess->Access(ssConnectionString));

    return AssignClientEndpoint(ptrChannelEndpoint, uiTimeoutMs, pProxy);
}

sdv::com::TConnectionID CCommunicationControl::AssignServerEndpoint(/*in*/ sdv::IInterfaceAccess* pChannelEndpoint,
    /*in*/ sdv::IInterfaceAccess* pObject, /*in*/ uint32_t uiTimeoutMs, /*in*/ bool bAllowReconnect)
{
    if (!pChannelEndpoint || !pObject || !(uiTimeoutMs || bAllowReconnect)) return {};

    // Create a communication channel object
    std::unique_lock<std::mutex> lock(m_mtxChannels);
    auto ptrCommunication = std::make_shared<CChannelConnector>(*this, static_cast<uint32_t>(m_vecChannels.size()), pChannelEndpoint);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrCommunication)
    {
        SDV_LOG_ERROR("Failed to allocate SDV communication channel!");
        return {};
    }

    // Start the connection
    if (!ptrCommunication->ServerConnect(pObject, bAllowReconnect)) return {};

    // Add the channel to the vector
    m_vecChannels.push_back(ptrCommunication);

    //// If reconnect is not allowed and if not already connected, start the monitor...
    //if (!bAllowReconnect && !ptrCommunication->IsConnected())
    //    m_vecInitialConnectMon.emplace_back([&]()
    //        {
    //            auto ptrLocalChannel = ptrCommunication;
    //            auto tpStart = std::chrono::high_resolution_clock::now();
    //            while (!ptrLocalChannel->IsConnected())
    //            {
    //                std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //                if (std::chrono::duration_cast<std::chrono::milliseconds>(
    //                    std::chrono::high_resolution_clock::now() - tpStart).count() >
    //                    uiTimeoutMs) break;
    //            }

    //            if (!ptrLocalChannel->IsConnected())
    //                RemoveChannelConnector(ptrLocalChannel.get());
    //        });

    // Done!
    return ptrCommunication->GetConnectionID();
}

sdv::com::TConnectionID CCommunicationControl::AssignClientEndpoint(/*in*/ sdv::IInterfaceAccess* pChannelEndpoint,
    /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::IInterfaceAccess*& pProxy)
{
    pProxy = nullptr;

    // Create a communication channel object
    std::unique_lock<std::mutex> lock(m_mtxChannels);
    auto ptrCommunication =
        std::make_shared<CChannelConnector>(*this, static_cast<uint32_t>(m_vecChannels.size()), pChannelEndpoint);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrCommunication)
    {
        SDV_LOG_ERROR("Failed to allocate SDV communication channel!");
        return {};
    }

    // Start the connection
    pProxy = ptrCommunication->ClientConnect(uiTimeoutMs);
    if (!pProxy)
    {
        SDV_LOG_ERROR("Could not connect client!");
        return {};
    }

    // Add the channel to the vector
    m_vecChannels.push_back(ptrCommunication);

    return ptrCommunication->GetConnectionID();
}

void CCommunicationControl::RemoveConnection(/*in*/ const sdv::com::TConnectionID& tConnectionID)
{
    // Clear the vector entry.
    std::unique_lock<std::mutex> lock(m_mtxChannels);
    std::shared_ptr<CChannelConnector> ptrChannelsCopy;
    if (tConnectionID.uiIdent < m_vecChannels.size())
    {
        ptrChannelsCopy = m_vecChannels[tConnectionID.uiIdent]; // Keep the channel alive while clearing the vector entry.
        m_vecChannels[tConnectionID.uiIdent].reset();   // Only clear the pointer; do not remove the entry (this would mix up IDs).
    }
    lock.unlock();

    // Clear the channel.
    ptrChannelsCopy.reset();
}

sdv::interface_t CCommunicationControl::GetProxy(/*in*/ const sdv::ps::TMarshallID& tStubID, /*in*/ sdv::interface_id id)
{
    if (!m_pConnectorContext) return {};

    // Get the proxy
    std::shared_ptr<CMarshallObject> ptrProxy = m_pConnectorContext->GetOrCreateProxy(id, tStubID);
    if (!ptrProxy) return {};

    return ptrProxy->GetProxy();
}

sdv::ps::TMarshallID CCommunicationControl::GetStub(/*in*/ sdv::interface_t ifc)
{
    // Get the stub
    auto ptrStub = GetOrCreateStub(ifc);
    if (!ptrStub) return {};

    return ptrStub->GetMarshallID();
}

std::shared_ptr<CMarshallObject> CCommunicationControl::CreateProxy(sdv::interface_id id, sdv::ps::TMarshallID tStubID,
    CChannelConnector& rConnector)
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxObjects);

    // Create the marshall object.
    size_t nIndex = m_vecMarshallObjects.size();
    auto ptrMarshallObject = std::make_shared<CMarshallObject>(*this);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrMarshallObject)
        return {};
    m_vecMarshallObjects.push_back(ptrMarshallObject);
    if (!ptrMarshallObject->InitializeAsProxy(static_cast<uint32_t>(nIndex), id, tStubID, rConnector))
        ptrMarshallObject.reset();
    if (!ptrMarshallObject) return {};
    return ptrMarshallObject;
}

std::shared_ptr<CMarshallObject> CCommunicationControl::GetOrCreateStub(sdv::interface_t ifc)
{
    if (!ifc) return {};
    std::unique_lock<std::recursive_mutex> lock(m_mtxObjects);

    // TODO: Check the proxy list if the interface is a proxy object. If so, get the corresponding stub ID instead of creating
    // another stub object.

    // If not existing, add an empty object in the map.
    auto itMarshallObject = m_mapStubObjects.find(ifc);
    if (itMarshallObject == m_mapStubObjects.end())
    {
        auto prMarshallObject = m_mapStubObjects.try_emplace(ifc, std::make_shared<CMarshallObject>(*this));
        if (!prMarshallObject.second) return {};
        itMarshallObject = prMarshallObject.first;
        size_t nIndex = m_vecMarshallObjects.size();
        m_vecMarshallObjects.push_back(itMarshallObject->second);
        if (!itMarshallObject->second->InitializeAsStub(static_cast<uint32_t>(nIndex), ifc))
            itMarshallObject->second.reset();
    }

    return itMarshallObject->second;
}

uint64_t CCommunicationControl::CreateUniqueCallIndex()
{
    // Return the next call count.
    return m_uiCurrentCallCnt++;
}

void CCommunicationControl::SetConnectorContext(CChannelConnector* pConnectorContext)
{
    // Store the current channel for this thread (needed during the proxy creation).
    m_pConnectorContext = pConnectorContext;
}

sdv::sequence<sdv::pointer<uint8_t>> CCommunicationControl::CallStub(sdv::ps::TMarshallID tStubID,
    sdv::sequence<sdv::pointer<uint8_t>>& seqInputData)
{
    // Find stub and call the function
    std::unique_lock<std::recursive_mutex> lock(m_mtxObjects);
    if (tStubID.uiIdent >= m_vecMarshallObjects.size()) throw sdv::ps::XMarshallIntegrity();
    auto ptrMarshallObject = m_vecMarshallObjects[tStubID.uiIdent].lock();
    lock.unlock();

    // Check for a valid object
    if (!ptrMarshallObject) throw sdv::ps::XMarshallIntegrity();
    if (ptrMarshallObject->GetMarshallID() != tStubID) throw sdv::ps::XMarshallIntegrity();

    // Make the call
    return ptrMarshallObject->Call(seqInputData);
}
