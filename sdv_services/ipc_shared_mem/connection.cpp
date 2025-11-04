#include "connection.h"
#include "watchdog.h"
#include <numeric>

/**
 * @brief Return the process ID of the current process.
 * @return The process ID.
 */
inline sdv::process::TProcessID GetProcessID()
{
    const sdv::process::IProcessInfo* pProcessInfo = sdv::core::GetObject<sdv::process::IProcessInfo>("ProcessControlService");
    return pProcessInfo ? pProcessInfo->GetProcessID() : 0;
}

inline std::string ConnectState(sdv::ipc::EConnectStatus eState)
{
    switch (eState)
    {
    case sdv::ipc::EConnectStatus::uninitialized:               return "uninitialized";
    case sdv::ipc::EConnectStatus::initializing:                return "initializing";
    case sdv::ipc::EConnectStatus::initialized:                 return "initialized";
    case sdv::ipc::EConnectStatus::connecting:                  return "connecting";
    case sdv::ipc::EConnectStatus::negotiating:                 return "negotiating";
    case sdv::ipc::EConnectStatus::connection_error:            return "connection_error";
    case sdv::ipc::EConnectStatus::connected:                   return "connected";
    case sdv::ipc::EConnectStatus::communication_error:         return "communication_error";
    case sdv::ipc::EConnectStatus::disconnected:                return "disconnected";
    case sdv::ipc::EConnectStatus::disconnected_forced:         return "disconnected_forced";
    case sdv::ipc::EConnectStatus::terminating:                 return "terminating";
    default:                                                    return "unknown";
    }
}

CConnection::CConnection(CWatchDog& rWatchDog, uint32_t uiSize, const std::string& rssName, bool bServer) :
    m_rWatchDog(rWatchDog), m_sender(uiSize, rssName, bServer), m_receiver(uiSize, rssName, bServer), m_bServer(bServer)
{
#if ENABLE_REPORTING >= 1
    TRACE("Accessing ", bServer ? "server" : "client", " connection with shared mem buffer of ", uiSize, " bytes and names \"",
        m_sender.GetName(), "\" and \"", m_receiver.GetName(), "\"");
#endif
}

CConnection::CConnection(CWatchDog& rWatchDog, const std::string& rssConnectionString) :
    m_rWatchDog(rWatchDog), m_sender(rssConnectionString), m_receiver(rssConnectionString)
{
    // Interpret the connection string
    sdv::toml::CTOMLParser config(rssConnectionString);

    // Get endpoint information
    std::string ssEndpointType = config.GetDirect("Endpoint.Type").GetValue();
    m_bServer = ssEndpointType == "server";

#if ENABLE_REPORTING >= 1
    TRACE("Opening ", m_bServer ? "server" : "client", " connection with shared mem buffer of ", m_sender.GetSize(),
        " bytes and names \"", m_sender.GetName(), "\" and \"", m_receiver.GetName(), "\"");
#endif

    if (!m_sender.IsValid())
    {
        SDV_LOG_ERROR("No valid shared memory for sending: ", m_sender.GetError());
    }
    if (!m_receiver.IsValid())
    {
        SDV_LOG_WARNING("No valid shared memory for receiving: ", m_receiver.GetError());
    }
}

CConnection::~CConnection()
{
#if ENABLE_REPORTING >= 1
    TRACE(m_bServer ? "Destroying server" : "Closing client", " connections \"", m_sender.GetName(), "\" and \"",
        m_receiver.GetName(), "\"");
#endif

    // If still connected, disconnect.
    if (m_eStatus == sdv::ipc::EConnectStatus::connected)
        Disconnect();

    // Set to terminating to allow the threads to shut down.
    m_eStatus = sdv::ipc::EConnectStatus::terminating;

    // Stop the receive thread to prevent accepting any more messages from the server.
    if (m_threadReceive.joinable())
        m_threadReceive.join();

#if ENABLE_DECOUPLING > 0
    // Stop the processing thread
    if (m_threadDecoupleReceive.joinable())
        m_threadDecoupleReceive.join();
#endif

    //// If not a server connection, detach the shared memory to allow reuse.
    //// NOTE: Windows uses a symetric opening and closing (every open needs a close again and the last close will delete the
    //// resource). POSIX uses only one call to release the resources for every opening. Therefore, detaching the allocation
    //// in case of a client will prevent release of resources.
    //if (!m_bServer)
    //{
    //    m_sender.Detach();
    //    m_receiver.Detach();
    //}
}

std::string CConnection::GetConnectionString()
{
    // The connection string is of the form:
    // [Provider]
    // Name = "DefaultSharedMemoryChannelControl"
    //
    // [[ConnectParam]]
    // Type = "shared_mem"
    // Location = "SDV_SHARED_RESPONSE_1234"
    // SyncTx = "SDV_TX_SYNC_RESPONSE_1234"
    // SyncRx = "SDV_RX_SYNC_RESPONSE_1234"
    // Direction = "response"
    //
    // [[ConnectParam]]
    // Type = "shared_mem"
    // Location = "SDV_SHARED_REQUEST_1234"
    // SyncTx = "SDV_TX_SYNC_REQUEST_1234"
    // SyncRx = "SDV_RX_SYNC_REQUEST_1234"
    // Direction = "request"
    std::string ssConnectionString = R"code([Provider]
Name = "LocalChannelControl"
)code" + std::string("\n") + m_sender.GetConnectionString() + "\n" + m_receiver.GetConnectionString();
    return ssConnectionString;
}

uint32_t CConnection::Send(const void* pData, uint32_t uiDataLength)
{
#if ENABLE_REPORTING >= 1
    TRACE("Send raw data 0x", pData, " of ", uiDataLength, " bytes");
#endif
#if ENABLE_REPORTING >= 2
    switch (((SMsgHdr*)pData)->eType)
    {
    case EMsgType::sync_request: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND SYNC_REQUEST (", ConnectState(m_eStatus), ")"); break;
    case EMsgType::sync_answer: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND SYNC_ANSWER (", ConnectState(m_eStatus), ")"); break;
    case EMsgType::connect_request: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND CONNECT_REQUEST (", ConnectState(m_eStatus), ")"); break;
    case EMsgType::connect_answer: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND CONNECT_ANSWER (", ConnectState(m_eStatus), ")"); break;
    case EMsgType::connect_term: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND CONNECT_TERM (", ConnectState(m_eStatus), ")"); break;
#if ENABLE_REPORTING >= 3
    case EMsgType::data: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND DATA ", uiDataLength - sizeof(SMsgHdr), " bytes (", ConnectState(m_eStatus), ")"); break;
    case EMsgType::data_fragment: TRACE(m_bServer ? "SERVER" : "CLIENT", " RECEIVE DATA FRAGMENT ", uiDataLength - sizeof(SFragmentedMsgHdr), " bytes (", ConnectState(m_eStatus), ")"); break;
#else
    case EMsgType::data: break;
    case EMsgType::data_fragment: break;
#endif
    default: TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND UNKNOWN (", ConnectState(m_eStatus), ")"); break;
    }
#endif

    if (!m_sender.TryWrite(pData, uiDataLength))
    {
        auto error = m_sender.GetError();
        SDV_LOG_ERROR("FAILED to send message with size ", uiDataLength, " (", error, ")");
        return 0;
    }
#if ENABLE_REPORTING >= 1
    TRACE("Finished Sending raw data of ", uiDataLength, " bytes");
#endif
    return uiDataLength;
}

bool CConnection::SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
#if ENABLE_REPORTING >= 1
    std::stringstream sstreamReport;
    for (const auto& rptr : seqData)
    {
        if (!sstreamReport.str().empty())
            sstreamReport << ", ";
        sstreamReport << rptr.size();
    }
    TRACE("Send a sequence of data with ", seqData.size(), " pointers with the length {", sstreamReport.str(), "} bytes");
#endif
    // Only allow sending messages when the status is connected
    if (m_eStatus != sdv::ipc::EConnectStatus::connected)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return false;
    }

    // The data chunks need to stay in tact. Add a table of lengths at the beginning of the message.
    // The table consists of a 32-bit integer containing the amount of chunks following by multiple 32-bit integer with the length
    // of each data chunks.

    // Easiest way of sending would be a list of data buffers. Adding the table to the beginning of the sequence is not possible
    // due to "const" qualifier. Also, creating a new sequence with buffers would copy the data. Instead, make a sequence with
    // buffer references.
    sdv::sequence<const sdv::pointer<uint8_t>*> seqDataTemp;
    sdv::pointer<uint8_t> ptrTable;
    seqDataTemp.push_back(&ptrTable);
    // False cppcheck warning; uiVal is seen as unused. This is not the case. Suppress warning
    // cppcheck-suppress unusedStructMember
    union UVal32 { uint32_t uiVal; uint8_t rguiData[sizeof(uint32_t)]; };
    ptrTable.resize((seqData.size() + 1) * sizeof(uint32_t));   // Amount of chunks plus one containing the amount
    size_t nTableIndex = 0;
    UVal32 uAmount = { static_cast<uint32_t>(seqData.size()) };
    for (uint8_t uiByte : uAmount.rguiData)
        ptrTable[nTableIndex++] = uiByte;

    // Calculate the required message size and add each chunk to the temp sequence.
    uint32_t uiRequiredSize = sizeof(uint32_t);     // The amount of chunks.
    for (const sdv::pointer<uint8_t>& rBuffer : seqData)
    {
        uiRequiredSize += static_cast<uint32_t>(rBuffer.size());
        uiRequiredSize += sizeof(uint32_t);     // The size of each chunk.
        seqDataTemp.push_back(&rBuffer);        // Add to temp buffer
        UVal32 uChunkSize{ static_cast<uint32_t>(rBuffer.size()) };
        for (uint8_t uiByte : uChunkSize.rguiData)
            ptrTable[nTableIndex++] = uiByte;
    };

    // Block race conditions of messages
    std::unique_lock<std::mutex> lock(m_mtxSend);

    uint32_t uiOffset = 0;
    uint32_t uiFragmentSize = m_sender.GetSize() / 4;
#if ENABLE_REPORTING >= 1
    TRACE("Sending data in ", (uiRequiredSize / uiFragmentSize + (uiRequiredSize % uiFragmentSize ? 1 : 0)), " chunks");
    size_t nChunkCount = 0;
#endif
    auto itChunk = seqDataTemp.cbegin();
    size_t nPos = 0;
    while (itChunk != seqDataTemp.cend() && uiOffset < uiRequiredSize)
    {
#ifdef TIME_TRACKING
        m_tpLastSent = std::chrono::high_resolution_clock::now();
#endif

        uint32_t uiDataSize = 0;
        uint32_t uiAllocSize = 0;
        bool bFragmented = false;
        if (uiFragmentSize < uiRequiredSize)    // Fragmented sending required
        {
            uiDataSize = std::min(uiFragmentSize, uiRequiredSize - uiOffset);
            uiAllocSize = uiDataSize + sizeof(SFragmentedMsgHdr);
            bFragmented = true;
        }
        else // Complete message fits
        {
            uiDataSize = uiRequiredSize;
            uiAllocSize = uiDataSize + sizeof(SMsgHdr);
        }

        // Allocate the message buffer.
        auto optPacket = m_sender.Reserve(uiAllocSize);
        if (!optPacket)
        {
            if (m_eStatus == sdv::ipc::EConnectStatus::connected)
                SDV_LOG_ERROR("Could not reserve a buffer to send a message of ", uiDataSize, " bytes.");
            return false;
        }

        // Fill in the message header
        uint32_t uiMsgOffset = 0;
        if (bFragmented)
        {
            SFragmentedMsgHdr* pHdr = reinterpret_cast<SFragmentedMsgHdr*>(optPacket->GetDataPtr());
            pHdr->uiVersion = SDVFrameworkInterfaceVersion;
            pHdr->eType = EMsgType::data_fragment;
            pHdr->uiTotalLength = uiRequiredSize;
            pHdr->uiOffset = uiOffset;
            uiMsgOffset = sizeof(SFragmentedMsgHdr);

#if ENABLE_REPORTING >= 3
            TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND DATA FRAGMENT ", uiOffset, "-", uiOffset + uiDataSize - 1, " of ", uiRequiredSize, " bytes (", ConnectState(m_eStatus), ")");
#endif
        }
        else
        {
            SMsgHdr* pHdr = reinterpret_cast<SMsgHdr*>(optPacket->GetDataPtr());
            pHdr->uiVersion = SDVFrameworkInterfaceVersion;
            pHdr->eType = EMsgType::data;
            uiMsgOffset = sizeof(SMsgHdr);

#if ENABLE_REPORTING >= 3
            TRACE(m_bServer ? "SERVER" : "CLIENT", " SEND DATA ", uiRequiredSize, " bytes (", ConnectState(m_eStatus), ")");
#endif
        }

        // Copy the content
        while (itChunk != seqDataTemp.cend() && uiMsgOffset < uiAllocSize)
        {
            // Calculate the length from the supplied fragment and the leftover space of te data message.
            size_t nLen = std::min((*itChunk)->size() - nPos, static_cast<size_t>(uiAllocSize - uiMsgOffset));
            if (nLen)
            {
                std::memcpy(reinterpret_cast<char*>(optPacket->GetDataPtr() + uiMsgOffset), (*itChunk)->get() + nPos, nLen);
#if ENABLE_REPORTING >= 4
                std::stringstream sstream;
                sstream << "DATA: 0x";
                for (size_t n = 0; n < nLen; n++)
                    sstream << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(reinterpret_cast<char*>(optPacket->GetDataPtr() + uiMsgOffset)[n]);
                TRACE(sstream.str());
#endif
            }
            nPos += nLen;
            uiMsgOffset += static_cast<uint32_t>(nLen);
            if (nPos >= (*itChunk)->size())
            {
                ++itChunk;
                nPos = 0;
            }
        }

        // Update the offset of the data message fragments.
        uiOffset += uiDataSize;

        // Commit the data
        optPacket->Commit();

#if ENABLE_REPORTING >= 1
        nChunkCount++;
#endif
    }

#if ENABLE_REPORTING >= 1
    TRACE("Finished sending data in ", nChunkCount, " chunks");
#endif

    // Extra safety... should not occur
    if (uiOffset < uiRequiredSize)
    {
        SDV_LOG_ERROR("Could not send complete data message of ", uiRequiredSize, " bytes (currently sent ",
            uiOffset, " bytes).");
        return false;
    }

    return true;
}

bool CConnection::AsyncConnect(sdv::IInterfaceAccess* pReceiver)
{
#if ENABLE_REPORTING >= 1
    TRACE("Asynchronous connect to receiver 0x", (void*) pReceiver);
#endif

    std::unique_lock<std::mutex> lock(m_mtxConnect);

    // Allowed to connect?
    if (m_eStatus != sdv::ipc::EConnectStatus::uninitialized)
    {
        for (auto& rprEventCallback : m_lstEventCallbacks)
            if (rprEventCallback.pCallback) rprEventCallback.pCallback->SetStatus(sdv::ipc::EConnectStatus::connection_error);
        return false;
    }

    SetStatus(sdv::ipc::EConnectStatus::initializing);

    // Initialized?
    if (!m_sender.IsValid() && !m_receiver.IsValid())
    {
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
        SDV_LOG_ERROR("Could not establish connection: sender(", (m_sender.IsValid() ? "valid" : "invalid"), ") receiver(",
            (m_receiver.IsValid() ? "valid" : "invalid"), ")");
        SetStatus(sdv::ipc::EConnectStatus::uninitialized);
        return false;
    }

    // Assign the receiver
    m_pReceiver = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IDataReceiveCallback>();
    SetStatus(sdv::ipc::EConnectStatus::initialized);

    // Start the receiving thread (wait until started).
    m_threadReceive = std::thread(&CConnection::ReceiveMessages, this);
#if ENABLE_DECOUPLING > 0
    m_threadDecoupleReceive = std::thread(&CConnection::DecoupleReceive, this);
#endif
    if (!m_bStarted)
        m_cvStartConnect.wait_for(lock, std::chrono::milliseconds(1000));

#if ENABLE_REPORTING >= 1
    if (m_bStarted)
        TRACE("Asynchronous connection initiated");
    else
        TRACE("Asynchronous connection not initiated - reception thread didn't start.");
#endif

    return m_bStarted;
}

bool CConnection::WaitForConnection(/*in*/ uint32_t uiWaitMs)
{
#if ENABLE_REPORTING >= 1
    if (m_eStatus == sdv::ipc::EConnectStatus::connected)
        TRACE("Not waiting for a connection - already connected");
    else
        TRACE("Waiting for a connection of ", uiWaitMs, "ms");
#endif

    if (m_eStatus == sdv::ipc::EConnectStatus::connected) return true;

    std::unique_lock<std::mutex> lock(m_mtxConnect);

    // Wait for the connection to take place.
    // Attention: sporadic
    if (uiWaitMs)
        m_cvConnect.wait_for(lock, std::chrono::milliseconds(uiWaitMs));

#if ENABLE_REPORTING >= 1
    if (m_eStatus == sdv::ipc::EConnectStatus::connected)
        TRACE("Waiting finished - connection established");
    else
        TRACE("Waiting finished - timeout occurred");
#endif

    return m_eStatus == sdv::ipc::EConnectStatus::connected;
}

void CConnection::CancelWait()
{
#if ENABLE_REPORTING >= 1
    TRACE("Cancel the (potential) waiting for a connection");
#endif

    m_cvConnect.notify_all();
}

void CConnection::Disconnect()
{
#if ENABLE_REPORTING >= 1
    TRACE("Disconnect");
#endif

    // Cancel any waits, just in case
    CancelWait();

    // Set the disconnect status
    sdv::ipc::EConnectStatus eStatus = m_eStatus;
    SetStatus(sdv::ipc::EConnectStatus::disconnected);

    // Release the interface
    m_pReceiver = nullptr;

    // If connected, send termination message.
    switch (eStatus)
    {
    case sdv::ipc::EConnectStatus::connecting:
    case sdv::ipc::EConnectStatus::negotiating:
    case sdv::ipc::EConnectStatus::connected:
        Send(SMsgHdr{ SDVFrameworkInterfaceVersion, EMsgType::connect_term });
        break;
    default:
        break;
    }

#if ENABLE_REPORTING >= 1
    TRACE("Disconnection has been finished");
#endif
}

uint64_t CConnection::RegisterStatusEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback)
{
    if (!pEventCallback) return 0;
    sdv::ipc::IConnectEventCallback* pCallback = pEventCallback->GetInterface<sdv::ipc::IConnectEventCallback>();
    if (!pCallback) return 0;
    uint64_t uiCookie = rand();
    std::unique_lock<std::shared_mutex> lock(m_mtxEventCallbacks);
    m_lstEventCallbacks.emplace(m_lstEventCallbacks.begin(), std::move(SEventCallback{ uiCookie, pCallback }));
    return uiCookie;
}

void CConnection::UnregisterStatusEventCallback(/*in*/ uint64_t uiCookie)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxEventCallbacks);
    auto itEventCallback = std::find_if(m_lstEventCallbacks.begin(), m_lstEventCallbacks.end(),
        [&](const auto& rprCallback) { return rprCallback.uiCookie == uiCookie; });
    if (itEventCallback == m_lstEventCallbacks.end()) return;
    itEventCallback->pCallback = nullptr;
}

sdv::ipc::EConnectStatus CConnection::GetStatus() const
{
    return m_eStatus;
}

void CConnection::DestroyObject()
{
#if ENABLE_REPORTING >= 1
    TRACE("Object destruction initiated");
#endif

    // Disconnect
    Disconnect();

    // Set termination status.
    SetStatus(sdv::ipc::EConnectStatus::terminating);

    // Clear all events callbacks (if not done so already)
    std::shared_lock<std::shared_mutex> lock(m_mtxEventCallbacks);
    for (auto& rprEventCallback : m_lstEventCallbacks)
        rprEventCallback.pCallback = nullptr;
    lock.unlock();

    // Just in case... so no calls are made into the destructed class any more.
    m_rWatchDog.RemoveMonitor(this);

    // Remove the connection
    m_rWatchDog.RemoveConnection(this, m_threadReceive.joinable() && std::this_thread::get_id() == m_threadReceive.get_id());

#if ENABLE_REPORTING >= 1
    TRACE("Object destruction confirmed");
#endif
}

void CConnection::SetStatus(sdv::ipc::EConnectStatus eStatus)
{
#if ENABLE_REPORTING >= 1
    TRACE(m_bServer ? "SERVER" : "CLIENT", " Changing connect status from '", ConnectState(m_eStatus), "' to '", ConnectState(eStatus), "'");
#endif

    // Do not change the status when terminated.
    if (m_eStatus == sdv::ipc::EConnectStatus::terminating)
        return;

    // Only set the member variable if the status is not communication_error
    if (eStatus != sdv::ipc::EConnectStatus::communication_error)
        m_eStatus = eStatus;
    std::shared_lock<std::shared_mutex> lock(m_mtxEventCallbacks);
    for (auto& rprEventCallback : m_lstEventCallbacks)
        if (rprEventCallback.pCallback) rprEventCallback.pCallback->SetStatus(eStatus);

    // If disconnected by force update the disconnect status.
    if (m_eStatus == sdv::ipc::EConnectStatus::disconnected_forced)
        m_eStatus = sdv::ipc::EConnectStatus::disconnected;

#if ENABLE_REPORTING >= 1
    TRACE("Status updated...");
#endif
}

bool CConnection::IsServer() const
{
    return m_bServer;
}

void CConnection::ReceiveMessages()
{
    m_bStarted = true;

    std::unique_lock<std::mutex> lock(m_mtxConnect);

#if ENABLE_REPORTING >= 1
    TRACE("Start data reception thread...");
#endif

#ifdef _WIN32
    // Increase thread priority
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    if (!m_sender.IsValid() || !m_receiver.IsValid())
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_ERROR("No valid shared memory for receiving.");
        m_cvStartConnect.notify_all();
        return;
    }

    m_cvStartConnect.notify_all();
    lock.unlock();

    // Read processing
    auto tpStart = std::chrono::high_resolution_clock::time_point();
#ifdef TIME_TRACKING
    std::chrono::high_resolution_clock::time_point tpLastReceiveLoop = std::chrono::high_resolution_clock::now();
#endif
    SDataContext sDataCtxt;
    while (m_eStatus != sdv::ipc::EConnectStatus::terminating)
    {
#ifdef TIME_TRACKING
        std::chrono::high_resolution_clock::time_point tpTrackNow = std::chrono::high_resolution_clock::now();
        //if (m_durationLargestDeltaReceived < std::chrono::duration<double>(tpTrackNow - tpLastReceiveLoop))
        //    m_durationLargestDeltaReceived = std::chrono::duration<double>(tpTrackNow - tpLastReceiveLoop);
        tpLastReceiveLoop = tpTrackNow;
#endif
        // Request a read
        auto optPacket = m_receiver.TryRead();
        if (!optPacket)
        {
            // Start communication, but only if connection is client based. Server based should not start the communication. If
            // there is no client, the server would otherwise fill its send-buffer. Repeat sending every 500ms.
            if (!m_bServer && (/*m_eStatus == sdv::ipc::EConnectStatus::disconnected ||*/ m_eStatus == sdv::ipc::EConnectStatus::initialized))
            {
                // Send request
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration<double>(tpNow - tpStart).count() > 0.5)
                {
                    tpStart = tpNow;
                    Send(SMsgHdr{ SDVFrameworkInterfaceVersion, EMsgType::sync_request });
                }
            }
            else
                m_receiver.WaitForData(10);
            continue;
        }

        CMessage message(std::move(*optPacket));

#ifdef TIME_TRACKING
        m_tpLastReceived = tpTrackNow;
#endif

        // The message should be valid
        if (!message.IsValid())
        {
            auto ssError = m_receiver.GetError();
            SetStatus(sdv::ipc::EConnectStatus::communication_error);
            SDV_LOG_ERROR("The message is invalid (invalid size or invalid type).");
            continue;
        }

        // Print header information
        message.PrintHeader(*this);

        // Extra check to prevent race condition.
        if (m_eStatus == sdv::ipc::EConnectStatus::terminating) break;

#if ENABLE_REPORTING >= 1
        switch (message.GetMsgHdr().eType)
        {
        case EMsgType::data_fragment:
        case EMsgType::data:
            break;
        default:
            TRACE("Receive raw data 0x", (void*) &message.GetMsgHdr(), " of ", message.GetSize(), " bytes");
        }
#endif

        // Message state machine
        switch (message.GetMsgHdr().eType)
        {
        case EMsgType::sync_request:
            ReceiveSyncRequest(message);
            break;
        case EMsgType::connect_request:
            ReceiveConnectRequest(message);
            break;
        case EMsgType::sync_answer:
            ReceiveSyncAnswer(message);
            break;
        case EMsgType::connect_answer:
            ReceiveConnectAnswer(message);
            break;
        case EMsgType::connect_term:
            ReceiveConnectTerm(message);
            if (m_bServer)
                tpStart = std::chrono::high_resolution_clock::now();
            break;
        case EMsgType::data:
            ReceiveDataMessage(message, sDataCtxt);
            break;
        case EMsgType::data_fragment:
            ReceiveDataFragementMessage(message, sDataCtxt);
            break;
        default:
            break;  // Do nothing
        }
    }

#if ENABLE_REPORTING >= 1
    TRACE("Stop data reception thread...");
#endif

    m_bStarted = false;
}

uint32_t CConnection::ReadDataTable(CMessage& rMessage, SDataContext& rsDataCtxt)
{
    // The table is composed as follows:
    //    uint32 - the amount of chunks
    //    for each chunk...
    //          uint32 - the size of the chunk

    // Get the total size from the message header
    uint32_t uiOffset = 0;
    switch (rMessage.GetMsgHdr().eType)
    {
    case EMsgType::data:
        // Size is the message size without the header.
        uiOffset += sizeof(SMsgHdr);
        rsDataCtxt.uiTotalSize = rMessage.GetSize() - static_cast<uint32_t>(sizeof(SMsgHdr));
        break;
    case EMsgType::data_fragment:
        // Size is stored inside the header.
        uiOffset += sizeof(SFragmentedMsgHdr);
        if (rMessage.GetFragmentedHdr().uiOffset) return 0; // Only allowed for first fragment.
        rsDataCtxt.uiTotalSize = rMessage.GetFragmentedHdr().uiTotalLength;
        break;
    default:
        return 0;   // Only for data messages.
    }
    rsDataCtxt.uiCurrentOffset = 0;

    // Read the amount of buffers.
    if (rMessage.GetSize() < (uiOffset + static_cast<uint32_t>(sizeof(uint32_t))))
        return false;   // Not enough space for buffer count
    uint32_t uiAmount = *reinterpret_cast<const uint32_t*>(rMessage.GetData() + uiOffset);
    uiOffset += sizeof(uint32_t);
    rsDataCtxt.uiCurrentOffset += sizeof(uint32_t);

    // Read the chunk sizes.
    if (rMessage.GetSize() < ( uiOffset + uiAmount * static_cast<uint32_t>(sizeof(uint32_t))))
        return 0;   // Not enough space for the buffer table
    std::vector<size_t> vecSizes;
    for (uint32_t uiIndex = 0; uiIndex < uiAmount; uiIndex++)
    {
        uint32_t uiChunkSize = *reinterpret_cast<const uint32_t*>(rMessage.GetData() + uiOffset);
        vecSizes.push_back(static_cast<size_t>(uiChunkSize));
        uiOffset += sizeof(uint32_t);
        rsDataCtxt.uiCurrentOffset += sizeof(uint32_t);
    }

    // Count the sizes and verify with the total size.
    uint32_t uiTotalSize = rsDataCtxt.uiCurrentOffset +
        static_cast<uint32_t>(std::accumulate(vecSizes.begin(), vecSizes.end(), static_cast<size_t>(0)));
    if (uiTotalSize != rsDataCtxt.uiTotalSize) return 0;       // Must be the same

#ifdef TIME_TRACKING
    std::chrono::high_resolution_clock::time_point tpIntNow = std::chrono::high_resolution_clock::now();
#endif

    // Allocate the memory for all chunks
    for (size_t nSize : vecSizes)
    {
        rsDataCtxt.seqDataChunks.push_back(sdv::pointer<uint8_t>());
        rsDataCtxt.seqDataChunks.back().resize(nSize);
    }

#ifdef TIME_TRACKING
    std::chrono::high_resolution_clock::time_point tpIntNow2 = std::chrono::high_resolution_clock::now();
    if (m_durationLargestDeltaReceived < std::chrono::duration<double>(tpIntNow2 - tpIntNow))
        m_durationLargestDeltaReceived = std::chrono::duration<double>(tpIntNow2 - tpIntNow);
#endif

    // Start filling at first byte of the first chunk.
    rsDataCtxt.nChunkIndex = 0;
    rsDataCtxt.uiChunkOffset = 0;

    // Return the current position within the current message.
    return uiOffset;
}

bool CConnection::ReadDataChunk(CMessage& rMessage, uint32_t uiOffset, SDataContext& rsDataCtxt)
{
    // Check whether the header has been skipped already
    if (uiOffset < sizeof(SMsgHdr)) return false;     // Header not read.
    switch (rMessage.GetMsgHdr().eType)
    {
    case EMsgType::data:
        break;
    case EMsgType::data_fragment:
        if (uiOffset < sizeof(SFragmentedMsgHdr)) return false;  // Header not read completely.
        break;
    default:
        return false;   // Only for data messages.
    }

    // As long as there is data in the message, read the data to the chunk...
    while (uiOffset < rMessage.GetSize() && rsDataCtxt.nChunkIndex < rsDataCtxt.seqDataChunks.size())
    {
        // Calculate the mount of data to read...
        uint32_t uiMsgDataAvailable = rMessage.GetSize() - uiOffset;
        sdv::pointer<uint8_t>& rptrChunk = rsDataCtxt.seqDataChunks[rsDataCtxt.nChunkIndex];
        if (rsDataCtxt.uiChunkOffset > static_cast<uint32_t>(rptrChunk.size()))
            return false; // Invalid; should not occur
        uint32_t uiChunkDataNeeded = static_cast<uint32_t>(rptrChunk.size()) - rsDataCtxt.uiChunkOffset;

        // Copy the (partial) chunk data
        uint32_t uiChunkDataToCopy = std::min(uiMsgDataAvailable, uiChunkDataNeeded);
        std::copy(rMessage.GetData() + uiOffset, rMessage.GetData() + uiOffset + uiChunkDataToCopy, rptrChunk.get() + rsDataCtxt.uiChunkOffset);
        uiOffset += uiChunkDataToCopy;
        rsDataCtxt.uiChunkOffset += uiChunkDataToCopy;

        // Update data pointers
        if (rsDataCtxt.uiChunkOffset >= static_cast<uint32_t>(rptrChunk.size()))
        {
            rsDataCtxt.uiChunkOffset = 0;
            rsDataCtxt.nChunkIndex++;

            if (rsDataCtxt.nChunkIndex == rsDataCtxt.seqDataChunks.size())
            {
#if ENABLE_REPORTING >= 4
                for (size_t nChunkIndex = 0; nChunkIndex < rsDataCtxt.seqDataChunks.size(); nChunkIndex++)
                {
                    std::stringstream sstream;
                    sstream << "DATA_CHUNK #" << rsDataCtxt.nChunkIndex << ": 0x";
                    for (size_t n = 0; n < rsDataCtxt.seqDataChunks[nChunkIndex].size(); n++)
                        sstream << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(rsDataCtxt.seqDataChunks[nChunkIndex].get()[n]);
                    TRACE(sstream.str());
                }
#endif

#if ENABLE_REPORTING >= 1
                std::stringstream sstream;
                for (const sdv::pointer<uint8_t>& rptrData : rsDataCtxt.seqDataChunks)
                {
                    if (!sstream.str().empty()) sstream << ", ";
                    sstream << rptrData.size();
                }
                TRACE("Received complete data package with ", rsDataCtxt.seqDataChunks.size(), " chunks of {", sstream.str(), "} bytes");
#endif

#if ENABLE_DECOUPLING > 0
                // Store at the most 16 data frames in the queue... then wait...
                std::unique_lock<std::mutex> lockReceive(m_mtxReceive);
                while (m_queueReceive.size() >= 16)
                {
                    if (m_eStatus == sdv::ipc::EConnectStatus::terminating) return false;
                    m_cvReceiveProcessed.wait_for(lockReceive, std::chrono::milliseconds(100));
                }

                // Queue the data...
                m_queueReceive.push(std::move(rsDataCtxt.seqDataChunks));

                m_cvReceiveAvailable.notify_all();
#else
                if (m_pReceiver) m_pReceiver->ReceiveData(rsDataCtxt.seqDataChunks);
                rsDataCtxt = SDataContext();    // Reset
#endif
                break;  // Done...
            }
        }
    }
    return true;
}

#if ENABLE_DECOUPLING > 0
void CConnection::DecoupleReceive()
{
    while (m_eStatus != sdv::ipc::EConnectStatus::terminating)
    {
        // Wait for data
        std::unique_lock<std::mutex> lock(m_mtxReceive);
        if (m_queueReceive.empty())
        {
            m_cvReceiveAvailable.wait_for(lock, std::chrono::milliseconds(5));
            continue;
        }

        // Get the top most data
        sdv::sequence<sdv::pointer<uint8_t>> seqData = std::move(m_queueReceive.front());
        m_queueReceive.pop();
        lock.unlock();

#if ENABLE_REPORTING >= 3
        size_t nSize = 0;
        for (const sdv::pointer<uint8_t>& ptrData : seqData)
            nSize += ptrData.size();
        TRACE(m_bServer ? "SERVER" : "CLIENT", " DECOUPLED REVEICE DATA ", nSize, " bytes (", ConnectState(m_eStatus), ")");
#endif

        // Process the data
        if (m_pReceiver) m_pReceiver->ReceiveData(seqData);

        // Data processed.
        m_cvReceiveProcessed.notify_all();
    }
}
#endif

void CConnection::ReceiveSyncRequest(const CMessage& rMessage)
{
    if (rMessage.GetSize() != sizeof(SMsgHdr))
    {
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
        SDV_LOG_ERROR("Sync request received but with incorrect structure size ", rMessage.GetSize(), " in the request, but ",
            sizeof(SMsgHdr), " needed!");
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
        return;
    }

    // Check for compatibility
    if (rMessage.GetMsgHdr().uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
        SDV_LOG_ERROR("Sync request received for an incompatible communication; interface version ", rMessage.GetMsgHdr().uiVersion,
            " requested, but ", SDVFrameworkInterfaceVersion, " needed!");
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
        return;
    }
    else
    {
        // Start connecting
        if (m_eStatus == sdv::ipc::EConnectStatus::disconnected || m_eStatus == sdv::ipc::EConnectStatus::initialized)
        {
            SetStatus(sdv::ipc::EConnectStatus::connecting);

            // Send an answer
            Send(SMsgHdr{ SDVFrameworkInterfaceVersion, EMsgType::sync_answer });
        }
    }
}

void CConnection::ReceiveConnectRequest(const CMessage& rMessage)
{
    // Start negotiating
    if (m_eStatus == sdv::ipc::EConnectStatus::connecting)
    {
        // The connect message contains the process ID to monitor.
        m_rWatchDog.AddMonitor(rMessage.GetConnectHdr().tProcessID, this);

        // Replay to the request
        SetStatus(sdv::ipc::EConnectStatus::negotiating);
        Send(SConnectMsg{ {SDVFrameworkInterfaceVersion, EMsgType::connect_answer},
            static_cast<sdv::process::TProcessID>(GetProcessID()) });

        // Connected
        SetStatus(sdv::ipc::EConnectStatus::connected);
#if ENABLE_REPORTING >= 1
        TRACE("Trigger connected");
#endif
        m_cvConnect.notify_all();
    }
}

void CConnection::ReceiveSyncAnswer(const CMessage& rMessage)
{
    if (m_eStatus != sdv::ipc::EConnectStatus::disconnected && m_eStatus != sdv::ipc::EConnectStatus::initialized)
        return;

    // Check for compatibility
    if (rMessage.GetMsgHdr().uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_ERROR("Sync answer received for an incompatible communication; interface version ",
            rMessage.GetMsgHdr().uiVersion, " requested, but ", SDVFrameworkInterfaceVersion, " needed!");
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
        return;
    }

    // Start negotiating
    SetStatus(sdv::ipc::EConnectStatus::negotiating);
    Send(SConnectMsg{ {SDVFrameworkInterfaceVersion, EMsgType::connect_request}, GetProcessID() });
}

void CConnection::ReceiveConnectAnswer(const CMessage& rMessage)
{
    // Connection established
    if (m_eStatus == sdv::ipc::EConnectStatus::negotiating)
    {
        // The connect message contains the process ID to monitor.
        m_rWatchDog.AddMonitor(rMessage.GetConnectHdr().tProcessID, this);

        // Connected
        SetStatus(sdv::ipc::EConnectStatus::connected);
#if ENABLE_REPORTING >= 1
        TRACE("Trigger connected...");
#endif
        m_cvConnect.notify_all();
    }
}

void CConnection::ReceiveConnectTerm(CMessage& /*rMessage*/)
{
    SetStatus(sdv::ipc::EConnectStatus::disconnected);
    m_rWatchDog.RemoveMonitor(this);

    // Cancel any outstanding write... and reset the read position of the sender (otherwise any outstanding data will have
    // to be read before a new connection can take place).
    m_sender.CancelSend();
    m_sender.ResetRx();

    // Send sync request (do not wait until next round in case a very short connection <100ms took place).
    if (m_bServer)
        Send(SMsgHdr{ SDVFrameworkInterfaceVersion, EMsgType::sync_request });
}

void CConnection::ReceiveDataMessage(CMessage& rMessage, SDataContext& rsDataCtxt)
{
#if ENABLE_REPORTING >= 1
    TRACE("Start receive data message of ", rsDataCtxt.uiTotalSize, " bytes");
#endif

    // Read the data directory table...
    uint32_t uiOffset = ReadDataTable(rMessage, rsDataCtxt);
    if (!uiOffset)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

#if ENABLE_REPORTING >= 1
    std::stringstream sstream;
    for (const sdv::pointer<uint8_t>& rptrData : rsDataCtxt.seqDataChunks)
    {
        if (!sstream.str().empty()) sstream << ", ";
        sstream << rptrData.size();
    }
    TRACE("Data message has ", rsDataCtxt.seqDataChunks.size(), " chunks of {", sstream.str(), "} bytes");
#endif

    // Read data
    if (!ReadDataChunk(rMessage, uiOffset, rsDataCtxt))
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }
#if ENABLE_REPORTING >= 1
    TRACE("Finished reception of data message of ", rsDataCtxt.uiTotalSize, " bytes");
#endif
}

void CConnection::ReceiveDataFragementMessage(CMessage& rMessage, SDataContext& rsDataCtxt)
{
    // Data offset starts after the header.
    uint32_t uiOffset = sizeof(SFragmentedMsgHdr);

    // Is this the first chunk? Then read the data directory table.
    if (!rMessage.GetFragmentedHdr().uiOffset)
    {
#if ENABLE_REPORTING >= 1
        TRACE("Start receive fragmented data message of ", rsDataCtxt.uiTotalSize, " bytes");
#endif
        // Read the data directory table...
        // Note: it is assumed that the table fits in the first message completely.
        uiOffset = ReadDataTable(rMessage, rsDataCtxt);
        if (!uiOffset)
        {
            SetStatus(sdv::ipc::EConnectStatus::communication_error);
            return;
        }

#if ENABLE_REPORTING >= 1
        std::stringstream sstream;
        for (const sdv::pointer<uint8_t>& rptrData : rsDataCtxt.seqDataChunks)
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << rptrData.size();
        }
        TRACE("Fragmented message has ", rsDataCtxt.seqDataChunks.size(), " of {", sstream.str(), "} bytes");
#endif
    }

    // Read data chunk
    if (!ReadDataChunk(rMessage, uiOffset, rsDataCtxt))
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }
}

CConnection::CMessage::CMessage(CAccessorRxPacket&& rPacket) : CAccessorRxPacket(std::move(rPacket))
{}

CConnection::CMessage::~CMessage()
{
    if (GetData()) Accept();
}

bool CConnection::CMessage::IsValid() const
{
    if (!GetData() || GetSize() < static_cast<uint32_t>(sizeof(SMsgHdr))) return false;
    SMsgHdr sMsgHdr = *reinterpret_cast<const SMsgHdr*>(GetData());
    switch (sMsgHdr.eType)
    {
    case EMsgType::sync_request:
    case EMsgType::sync_answer:
    case EMsgType::connect_term:
    case EMsgType::data:
        return true;
    case EMsgType::connect_request:
    case EMsgType::connect_answer:
        return GetSize() >= static_cast<uint32_t>(sizeof(SConnectMsg));
    case EMsgType::data_fragment:
        return GetSize() >= static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
    default:
        return false;
    }
}

CConnection::SMsgHdr CConnection::CMessage::GetMsgHdr() const
{
    if (GetSize() < static_cast<uint32_t>(sizeof(SMsgHdr))) return {};
    return *reinterpret_cast<const SMsgHdr*>(GetData());
}

CConnection::SConnectMsg CConnection::CMessage::GetConnectHdr() const
{
    if (GetSize() < static_cast<uint32_t>(sizeof(SConnectMsg))) return {};
    SConnectMsg sConnectHdr = *reinterpret_cast<const SConnectMsg*>(GetData());
    switch (sConnectHdr.eType)
    {
    case EMsgType::connect_request:
    case EMsgType::connect_answer:
        return sConnectHdr;
    default:
        return {};
    }
}

CConnection::SFragmentedMsgHdr CConnection::CMessage::GetFragmentedHdr() const
{
    if (GetSize() < static_cast<uint32_t>(sizeof(SFragmentedMsgHdr))) return {};
    SFragmentedMsgHdr sFragmentedHdr = *reinterpret_cast<const SFragmentedMsgHdr*>(GetData());
    switch (sFragmentedHdr.eType)
    {
    case EMsgType::data_fragment:
        return sFragmentedHdr;
    default:
        return {};
    }
}

void CConnection::CMessage::PrintHeader([[maybe_unused]] const CConnection& rConnection) const
{
#if ENABLE_REPORTING >= 2
    switch (GetMsgHdr().eType)
    {
    case EMsgType::sync_request: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE SYNC_REQUEST (", ConnectState(rConnection.GetStatus()), ")"); break;
    case EMsgType::sync_answer: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE SYNC_ANSWER (", ConnectState(rConnection.GetStatus()), ")"); break;
    case EMsgType::connect_request: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE CONNECT_REQUEST (", ConnectState(rConnection.GetStatus()), ")"); break;
    case EMsgType::connect_answer: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE CONNECT_ANSWER (", ConnectState(rConnection.GetStatus()), ")"); break;
    case EMsgType::connect_term: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE CONNECT_TERM (", ConnectState(rConnection.GetStatus()), ")"); break;
#if ENABLE_REPORTING >= 3
    case EMsgType::data: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE DATA ", GetSize() - sizeof(SMsgHdr), " bytes (", ConnectState(rConnection.GetStatus()), ")"); break;
    case EMsgType::data_fragment: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE DATA FRAGMENT ", GetSize() - sizeof(SFragmentedMsgHdr), " bytes (", ConnectState(rConnection.GetStatus()), ")"); break;
#else
    case EMsgType::data: break;
    case EMsgType::data_fragment: break;
#endif
    default: rConnection.TRACE(rConnection.IsServer() ? "SERVER" : "CLIENT", " RECEIVE UNKNOWN version=", static_cast<uint32_t>(GetMsgHdr().uiVersion), " type=", static_cast<uint32_t>(GetMsgHdr().eType), "(", ConnectState(rConnection.GetStatus()), ")"); break;
    }
#endif
}