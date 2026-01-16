#include "com_channel.h"
#include "com_ctrl.h"
#include "marshall_object.h"
#include <support/pssup.h>
#include <support/serdes.h>
#include <support/local_service_access.h>
#include <interfaces/serdes/core_ps_serdes.h>
#include "../../global/scheduler/scheduler.cpp"

CChannelConnector::CChannelConnector(CCommunicationControl& rcontrol, uint32_t uiIndex, sdv::IInterfaceAccess* pChannelEndpoint) :
    m_rcontrol(rcontrol), m_ptrChannelEndpoint(pChannelEndpoint),
    m_pDataSend(m_ptrChannelEndpoint.GetInterface<sdv::ipc::IDataSend>())
{
    m_tConnectionID.uiIdent = uiIndex;
    while (!m_tConnectionID.uiControl)
        m_tConnectionID.uiControl = static_cast<uint32_t>(rand());
}

CChannelConnector::~CChannelConnector()
{
    // Finalize the scheduled calls.
    m_scheduler.WaitForExecution();

    // Remove all calls from the queue
    std::unique_lock<std::mutex> lock(m_mtxCalls);
    while (m_mapCalls.size())
    {
        SCallEntry& rsEntry = m_mapCalls.begin()->second;
        m_mapCalls.erase(m_mapCalls.begin());
        lock.unlock();

        // Cancel the processing
        rsEntry.eState = SCallEntry::EState::canceled;
        rsEntry.cvWaitForResult.notify_all();

        // Handle next call.
        lock.lock();
    }
    lock.unlock();

    // Disconnect
    sdv::ipc::IConnect* pConnection = m_ptrChannelEndpoint.GetInterface<sdv::ipc::IConnect>();
    if (pConnection)
    {
        if (m_uiConnectionStatusCookie) pConnection->UnregisterStatusEventCallback(m_uiConnectionStatusCookie);
        pConnection->Disconnect();
    }

    // There are several dependencies on this channel connector, which should be availble during the processing of the asynchronous
    // disconnect function. Wait for a quarter second to allow the processing to complete.
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

bool CChannelConnector::ServerConnect(sdv::IInterfaceAccess* pObject, bool bAllowReconnect)
{
    if (!m_ptrChannelEndpoint || !m_pDataSend) return false; // No channel
    if (!pObject) return false; // No object
    if (m_ptrInitialMarshallObject) return false;   // Has already a marshall object.
    m_bAllowReconnect = bAllowReconnect;
    m_eEndpointType = EEndpointType::server;

    m_ptrInitialMarshallObject = m_rcontrol.GetOrCreateStub(pObject);
    if (!m_ptrInitialMarshallObject) return false;

    // Establish connection...
    sdv::ipc::IConnect* pConnection = m_ptrChannelEndpoint.GetInterface<sdv::ipc::IConnect>();
    if (!pConnection) return false;
    m_uiConnectionStatusCookie = pConnection->RegisterStatusEventCallback(this);
    return m_uiConnectionStatusCookie != 0 && pConnection->AsyncConnect(this);
}

sdv::IInterfaceAccess* CChannelConnector::ClientConnect(uint32_t uiTimeoutMs)
{
    if (!m_ptrChannelEndpoint || !m_pDataSend) return nullptr; // No channel
    if (!uiTimeoutMs) return nullptr; // No timeout
    if (m_ptrInitialMarshallObject) return nullptr; // Has already a marshall object.
    m_eEndpointType = EEndpointType::client;

    // Get or create the proxy
    m_ptrInitialMarshallObject = GetOrCreateProxy(sdv::GetInterfaceId<sdv::IInterfaceAccess>(), sdv::ps::TMarshallID{});
    if (!m_ptrInitialMarshallObject)
    {
        SDV_LOG_ERROR("Could not get or create proxy object!");
        return nullptr;
    }

    // Establish connection...
    sdv::ipc::IConnect* pConnection = m_ptrChannelEndpoint.GetInterface<sdv::ipc::IConnect>();
    if (pConnection) m_uiConnectionStatusCookie = pConnection->RegisterStatusEventCallback(this);
    if (!pConnection || m_uiConnectionStatusCookie == 0 || !pConnection->AsyncConnect(this) ||
        !pConnection->WaitForConnection(uiTimeoutMs))
    {
        SDV_LOG_ERROR("Could not establish a connection!");
        m_ptrInitialMarshallObject.reset();
        return nullptr;
    }

    return m_ptrInitialMarshallObject->GetProxy().get<sdv::IInterfaceAccess>();
}

bool CChannelConnector::IsConnected() const
{
    return m_eConnectStatus == sdv::ipc::EConnectStatus::connected;
}

void CChannelConnector::SetStatus(/*in*/ sdv::ipc::EConnectStatus eConnectStatus)
{
    auto eConnectStatusTemp = m_eConnectStatus;
    m_eConnectStatus = eConnectStatus;
    switch (m_eConnectStatus)
    {
    case sdv::ipc::EConnectStatus::disconnected:
    case sdv::ipc::EConnectStatus::disconnected_forced:
        // Invalidate the proxy objects.
        for (auto& rvtProxyObject : m_mapProxyObjects)
            rvtProxyObject.second.reset();

        if (m_eEndpointType == EEndpointType::server)
        {
            // Report information (but only once)
            if (sdv::app::ConsoleIsVerbose() && eConnectStatusTemp != sdv::ipc::EConnectStatus::disconnected)
                std::cout << "Client disconnected (ID#" << m_tConnectionID.uiIdent << ")" << std::endl;

            // Remove the connection if reconnection is not enabled (normal case).
            if (!m_bAllowReconnect)
                m_rcontrol.RemoveConnection(m_tConnectionID);
        }
        break;
    case sdv::ipc::EConnectStatus::connected:
        if (m_eEndpointType == EEndpointType::server)
        {
            // Report information
            if (sdv::app::ConsoleIsVerbose())
                std::cout << "Client connected (ID#" << m_tConnectionID.uiIdent << ")" << std::endl;
        }
        break;
    default:
        break;
    }
}

void CChannelConnector::ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    // Schedule the data reception
    std::mutex mtxSyncData;
    std::condition_variable cvSyncData;
    bool bSyncData = false;
    bool bResult = m_scheduler.Schedule([&]()
        {
            // Copy the data to keep validity
            std::unique_lock<std::mutex> lock(mtxSyncData);
            sdv::sequence<sdv::pointer<uint8_t>> seqDataCopy = std::move(seqData);
            lock.unlock();
            bSyncData = true;
            cvSyncData.notify_all();

            // Call the receive function
            DecoupledReceiveData(seqDataCopy);
        });
    std::unique_lock<std::mutex> lockSyncData(mtxSyncData);
    while (bResult && !bSyncData) cvSyncData.wait_for(lockSyncData, std::chrono::milliseconds(10));
    lockSyncData.unlock();

    // TODO: Handle a schedule failure - send back a resource depletion.
}

void CChannelConnector::DecoupledReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    // The first data pointer in the sequence contains the address header
    if (seqData.size() < 1) return;
    sdv::pointer<uint8_t> rptrAddress = std::move(seqData.front());
    seqData.erase(seqData.begin());

    // The data should be at least the size of the header.
    if (rptrAddress.size() < sizeof(sdv::ps::SMarshallAddress)) return;     // Invalid size

    // Deserialize the address portion of the header
    // The first byte in the data pointer determines the endianness
    sdv::EEndian eSourceEndianess = static_cast<sdv::EEndian>(rptrAddress[0]);

    // Deserialize the address structure
    sdv::ps::SMarshallAddress sAddress{};
    if (eSourceEndianess == sdv::EEndian::big_endian)
    {
        sdv::deserializer<sdv::EEndian::big_endian> desInput;
        desInput.attach(rptrAddress, 0);  // Do not check checksum...
        serdes::CSerdes<sdv::ps::SMarshallAddress>::Deserialize(desInput, sAddress);
    } else
    {
        sdv::deserializer<sdv::EEndian::little_endian> desInput;
        desInput.attach(rptrAddress, 0);  // Do not check checksum...
        serdes::CSerdes<sdv::ps::SMarshallAddress>::Deserialize(desInput, sAddress);
    }

    // If the data should be interprated as input data, the data should go into a stub object.
    // If the data should be interprated as output data, the data is returning from the call and should go into the proxy object.
    if (sAddress.eInterpret == sdv::ps::EMarshallDataInterpret::input_data)
    {
        // A proxy ID should be present in any case - if not, this is a serious error, since it is not possible to inform the
        // caller.
        if (!sAddress.tProxyID) return;

        // In case the stub ID is not provided, the stub ID is the ID of the initial marshall object
        sdv::ps::TMarshallID tStubID = sAddress.tStubID;
        if (!tStubID) tStubID = m_ptrInitialMarshallObject->GetMarshallID();

        // Store the channel context (used to marshall interfaces over the same connector)
        m_rcontrol.SetConnectorContext(this);

        // Call the stub function
        sdv::sequence<sdv::pointer<uint8_t>> seqResult;
        try
        {
            seqResult = m_rcontrol.CallStub(tStubID, seqData);
        }
        catch (...)
        {
            // Should not occur.
            std::cout << "Exception occurred..." << std::endl;
        }

        // Store the address struct into the result sequence
        sAddress.eInterpret = sdv::ps::EMarshallDataInterpret::output_data;
        if (eSourceEndianess == sdv::EEndian::big_endian)
        {
            sdv::serializer<sdv::EEndian::big_endian> serOutput;
            serdes::CSerdes<sdv::ps::SMarshallAddress>::Serialize(serOutput, sAddress);
            seqResult.insert(seqResult.begin(), serOutput.buffer());
        } else
        {
            sdv::serializer<sdv::EEndian::little_endian> serOutput;
            serdes::CSerdes<sdv::ps::SMarshallAddress>::Serialize(serOutput, sAddress);
            seqResult.insert(seqResult.begin(), serOutput.buffer());
        }

        // Send the result back
        m_pDataSend->SendData(seqResult);
    }
    else
    {
        // Look for the call entry
        std::unique_lock<std::mutex> lockCallMap(m_mtxCalls);
        auto itCall = m_mapCalls.find(sAddress.uiCallIndex);
        if (itCall == m_mapCalls.end())
            return;
        SCallEntry& rsCallEntry = itCall->second;
        m_mapCalls.erase(itCall);
        lockCallMap.unlock();
        if (rsCallEntry.eState != SCallEntry::EState::processing)
            return;

        // Update the result
        std::unique_lock<std::mutex> lockCall(rsCallEntry.mtxWaitForResult);
        rsCallEntry.seqResult = seqData;
        lockCall.unlock();
        rsCallEntry.eState = SCallEntry::EState::processed;
        rsCallEntry.cvWaitForResult.notify_all();
    }
}

sdv::sequence<sdv::pointer<uint8_t>> CChannelConnector::MakeCall(sdv::ps::TMarshallID tProxyID, sdv::ps::TMarshallID tStubID,
    sdv::sequence<sdv::pointer<uint8_t>>& rseqInputData)
{
    if (!m_pDataSend) throw sdv::ps::XMarshallNotInitialized();

    // Create an address structure
    sdv::ps::SMarshallAddress sAddress{};
    sAddress.eEndian = sdv::GetPlatformEndianess();
    sAddress.uiVersion = SDVFrameworkInterfaceVersion;
    sAddress.tProxyID = tProxyID;
    sAddress.tStubID = tStubID;
    sAddress.uiCallIndex = m_rcontrol.CreateUniqueCallIndex();
    sAddress.eInterpret = sdv::ps::EMarshallDataInterpret::input_data;

    // Create an additional stream for the address struct.
    sdv::serializer serAddress;
    serdes::CSerdes<sdv::ps::SMarshallAddress>::Serialize(serAddress, sAddress);
    rseqInputData.insert(rseqInputData.begin(), serAddress.buffer());

    // Add a call entry to be able to receive the result.
    std::unique_lock<std::mutex> lock(m_mtxCalls);
    SCallEntry sResult;
    m_mapCalls.try_emplace(sAddress.uiCallIndex, sResult);
    sResult.eState = SCallEntry::EState::processing;
    lock.unlock();

    // Store the channel context (used to marshall interfaces over the same connector)
    m_rcontrol.SetConnectorContext(this);

    // Send the data
    std::unique_lock<std::mutex> lockResult(sResult.mtxWaitForResult);
    try
    {
        if (!m_pDataSend->SendData(rseqInputData)) throw sdv::ps::XMarshallExcept();
    }
    catch (const sdv::ps::XMarshallExcept& /*rexcept*/)
    {
        // Exception occurred. Remove the call und rethrow.
        lock.lock();
        m_mapCalls.erase(sAddress.uiCallIndex);
        lock.unlock();
        throw;
    }

    // Wait for the result
    // NOTE: Sinde the conditional variable doesn't keep its state, it might happen, that the variable is set before the wait
    // function has been entered (race condition). This would cause the function to wait forever.
    while (sResult.eState == SCallEntry::EState::processing)
        sResult.cvWaitForResult.wait_for(lockResult, std::chrono::milliseconds(1));

    if (sResult.eState == SCallEntry::EState::canceled)
            throw sdv::ps::XMarshallTimeout();

    return sResult.seqResult;
}

std::shared_ptr<CMarshallObject> CChannelConnector::GetOrCreateProxy(sdv::interface_id id, sdv::ps::TMarshallID tStubID)
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxMarshallObjects);

    // Check for an existing proxy.
    auto itMarshallObject = m_mapProxyObjects.find(tStubID);
    if (itMarshallObject != m_mapProxyObjects.end())
        return itMarshallObject->second;

    // Proxy doesn't exist; create a new proxy.
    auto ptrMarshallObject = m_rcontrol.CreateProxy(id, tStubID, *this);
    if (!ptrMarshallObject)return {};
    m_mapProxyObjects[tStubID] = ptrMarshallObject;
    return ptrMarshallObject;
}

sdv::com::TConnectionID CChannelConnector::GetConnectionID() const
{
    return m_tConnectionID;
}
