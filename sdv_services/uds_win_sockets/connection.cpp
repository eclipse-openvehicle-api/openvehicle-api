/********************************************************************************
* Copyright (c) 2025-2026 ZF Friedrichshafen AG
*
* This program and the accompanying materials are made available under the
* terms of the Apache License Version 2.0 which is available at
* https://www.apache.org/licenses/LICENSE-2.0
*
* SPDX-License-Identifier: Apache-2.0
*
* Contributors:
*   Denisa Ros - initial API and implementation
********************************************************************************/

#include "connection.h"

#include <numeric>
#include <WS2tcpip.h>
#include <cstring>
#include <chrono>

CConnection::CConnection(SOCKET preconfiguredSocket, bool acceptConnectionRequired)
    : m_ConnectionStatus(sdv::ipc::EConnectStatus::uninitialized)
    , m_AcceptConnectionRequired(acceptConnectionRequired)
    , m_CancelWait(false)
{
    // Initialize legacy buffers with zero (kept for potential compatibility)
    std::fill(std::begin(m_SendBuffer), std::end(m_SendBuffer), '\0');
    std::fill(std::begin(m_ReceiveBuffer), std::end(m_ReceiveBuffer), '\0');

    if (m_AcceptConnectionRequired)
    {
        // Server side: we own a listening socket, active socket is not yet accepted
        m_ListenSocket     = preconfiguredSocket;
        m_ConnectionSocket = INVALID_SOCKET;
    }
    else
    {
        // Client side: we already have a connected socket
        m_ListenSocket     = INVALID_SOCKET;
        m_ConnectionSocket = preconfiguredSocket;
    }
}

CConnection::~CConnection()
{
    try
    {
        StopThreadsAndCloseSockets();
        m_ConnectionStatus = sdv::ipc::EConnectStatus::disconnected;
    }
    catch (...)
    {
        // Destructors must not throw
    }
}

void CConnection::SetStatus(sdv::ipc::EConnectStatus status)
{
    {
        std::lock_guard<std::mutex> lk(m_MtxConnect);
        m_ConnectionStatus.store(status, std::memory_order_release);
    }

    // Wake up any waiter
    m_CvConnect.notify_all();

    // Notify event callback if registered
    if (m_pEvent)
    {
        try
        {
            m_pEvent->SetStatus(status);
        }
        catch (...)
        {
            // Ignore callbacks throwing exceptions
        }
    }
}

int32_t CConnection::Send(const char* data, int32_t dataLength)
{
    int32_t total = 0;

    while (total < dataLength)
    {
        const int32_t n = ::send(m_ConnectionSocket, data + total, dataLength - total, 0);
        if (n == SOCKET_ERROR)
        {
            SDV_LOG_ERROR("send failed with error: ", std::to_string(WSAGetLastError()));
            m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
            m_ConnectionSocket = INVALID_SOCKET;
            return (total > 0) ? total : SOCKET_ERROR;
        }
        total += n;
    }

    return total;
}

int CConnection::SendExact(const char* data, int len)
{
    int total = 0;

    while (total < len)
    {
        const int n = ::send(m_ConnectionSocket, data + total, len - total, 0);
        if (n <= 0)
        {
            return -1;
        }
        total += n;
    }

    return total;
}

bool CConnection::SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    // Must be connected and have a valid socket
    if (m_ConnectionStatus != sdv::ipc::EConnectStatus::connected ||
        m_ConnectionSocket == INVALID_SOCKET)
    {
        m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
        return false;
    }

    std::lock_guard<std::recursive_mutex> lk(m_SendMutex);

    // Build SDV length table
    sdv::sequence<const sdv::pointer<uint8_t>*> seqTemp;
    sdv::pointer<uint8_t>                       table;

    const uint32_t nChunks    = static_cast<uint32_t>(seqData.size());
    const uint32_t tableBytes = (nChunks + 1u) * sizeof(uint32_t);

    table.resize(tableBytes);

    // First uint32_t = chunk count
    std::memcpy(table.get(), &nChunks, sizeof(uint32_t));

    uint32_t offset   = sizeof(uint32_t);
    uint64_t required = sizeof(uint32_t);

    // Next are n chunk sizes
    for (auto& buf : seqData)
    {
        const uint32_t len = static_cast<uint32_t>(buf.size());
        std::memcpy(table.get() + offset, &len, sizeof(uint32_t));
        offset   += sizeof(uint32_t);
        required += sizeof(uint32_t);
        required += len;
        seqTemp.push_back(&buf);
    }

    // Prepend table as the first "chunk"
    seqTemp.insert(seqTemp.begin(), &table);

    const uint32_t maxPayloadData =
        (kMaxUdsPacketSize > sizeof(SMsgHdr)) ?
        (kMaxUdsPacketSize - static_cast<uint32_t>(sizeof(SMsgHdr))) :
        0;

    const uint32_t maxPayloadFrag =
        (kMaxUdsPacketSize > sizeof(SFragmentedMsgHdr)) ?
        (kMaxUdsPacketSize - static_cast<uint32_t>(sizeof(SFragmentedMsgHdr))) :
        0;

    if (maxPayloadFrag == 0)
    {
        return false;
    }

    // Single-frame vs. fragmented
    const bool fitsSingle = (required <= static_cast<uint64_t>(maxPayloadData));

    auto  itChunk = seqTemp.cbegin();
    size_t pos    = 0;

    if (fitsSingle)
    {
        const uint32_t payloadBytes = static_cast<uint32_t>(required);
        const uint32_t totalBytes   = payloadBytes + static_cast<uint32_t>(sizeof(SMsgHdr));

        std::vector<uint8_t> frame(totalBytes);
        uint32_t             msgOff = 0;

        // SDV header
        {
            auto* hdr   = reinterpret_cast<SMsgHdr*>(frame.data());
            hdr->uiVersion = SDVFrameworkInterfaceVersion;
            hdr->eType     = EMsgType::data;
            msgOff         = static_cast<uint32_t>(sizeof(SMsgHdr));
        }

        // Copy table + chunks
        while (itChunk != seqTemp.cend() && msgOff < totalBytes)
        {
            const auto&   ref     = *itChunk;
            const uint32_t len    = static_cast<uint32_t>(ref->size());
            const uint8_t* src    = ref->get();
            const uint32_t canCopy =
                std::min<uint32_t>(len - static_cast<uint32_t>(pos), totalBytes - msgOff);

            if (canCopy)
            {
                std::memcpy(frame.data() + msgOff, src + pos, canCopy);
            }

            pos    += canCopy;
            msgOff += canCopy;

            if (pos >= len)
            {
                pos = 0;
                ++itChunk;
            }
        }

        uint32_t packetSize = totalBytes;
        if (SendExact(reinterpret_cast<char*>(&packetSize), sizeof(packetSize)) < 0)
            return false;
        if (SendExact(reinterpret_cast<char*>(frame.data()), totalBytes) < 0)
            return false;

        return true;
    }

    // Fragmented sending
    uint32_t sentOffset = 0;

    while (itChunk != seqTemp.cend() && sentOffset < required)
    {
        const uint32_t remaining  = static_cast<uint32_t>(required - sentOffset);
        const uint32_t dataBytes  = std::min<uint32_t>(remaining, maxPayloadFrag);
        const uint32_t allocBytes = dataBytes + static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));

        std::vector<uint8_t> frame(allocBytes);
        uint32_t             msgOff = 0;

        // Fragment header
        {
            auto* hdr        = reinterpret_cast<SFragmentedMsgHdr*>(frame.data());
            hdr->uiVersion   = SDVFrameworkInterfaceVersion;
            hdr->eType       = EMsgType::data_fragment;
            hdr->uiTotalLength = static_cast<uint32_t>(required);
            hdr->uiOffset    = sentOffset;
            msgOff           = static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
        }

        // Copy slice across the sequence
        uint32_t copied = 0;

        while (itChunk != seqTemp.cend() && copied < dataBytes)
        {
            const auto&   ref = *itChunk;
            const uint8_t* src = ref->get();
            const uint32_t len = static_cast<uint32_t>(ref->size());

            const uint32_t canCopy =
                std::min<uint32_t>(len - static_cast<uint32_t>(pos), dataBytes - copied);

            if (canCopy)
            {
                std::memcpy(frame.data() + msgOff, src + pos, canCopy);
            }

            pos    += canCopy;
            msgOff += canCopy;
            copied += canCopy;

            if (pos >= len)
            {
                pos = 0;
                ++itChunk;
            }
        }

        uint32_t packetSize = allocBytes;
        if (SendExact(reinterpret_cast<char*>(&packetSize), sizeof(packetSize)) < 0)
            return false;
        if (SendExact(reinterpret_cast<char*>(frame.data()), allocBytes) < 0)
            return false;

        sentOffset += copied;
    }

    return (sentOffset == required);
}

SOCKET CConnection::AcceptConnection()
{
    if (m_ListenSocket == INVALID_SOCKET)
    {
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
        return INVALID_SOCKET;
    }

    while (!m_StopConnectThread.load())
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_ListenSocket, &rfds);

        TIMEVAL tv{};
        tv.tv_sec  = 0;
        tv.tv_usec = 50 * 1000; // 50 ms

        const int sr = ::select(0, &rfds, nullptr, nullptr, &tv);
        if (sr == SOCKET_ERROR)
        {
            SDV_LOG_ERROR("[AF_UNIX] select(listen) FAIL, WSA=", WSAGetLastError());
            SetStatus(sdv::ipc::EConnectStatus::connection_error);
            return INVALID_SOCKET;
        }

        if (sr == 0)
        {
            continue; // timeout, re-check stop flag
        }

        SOCKET c = ::accept(m_ListenSocket, nullptr, nullptr);
        if (c == INVALID_SOCKET)
        {
            const int err = WSAGetLastError();
            if (err == WSAEINTR || err == WSAEWOULDBLOCK)
            {
                continue;
            }

            SDV_LOG_ERROR("[AF_UNIX] accept FAIL, WSA=", err);
            SetStatus(sdv::ipc::EConnectStatus::connection_error);
            return INVALID_SOCKET;
        }

        SDV_LOG_INFO("[AF_UNIX] accept OK, socket=", static_cast<uint64_t>(c));
        return c;
    }

    SDV_LOG_ERROR("[AF_UNIX] accept canceled (stop flag)");
    SetStatus(sdv::ipc::EConnectStatus::connection_error);
    return INVALID_SOCKET;
}

bool CConnection::AsyncConnect(sdv::IInterfaceAccess* pReceiver)
{
    // Store callbacks
    m_pReceiver = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IDataReceiveCallback>();
    m_pEvent    = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IConnectEventCallback>();

    // Reset stop flags
    m_StopReceiveThread.store(false);
    m_StopConnectThread.store(false);
    m_CancelWait.store(false);

    // Join old threads if any
    if (m_ReceiveThread.joinable())
        m_ReceiveThread.join();
    if (m_ConnectThread.joinable())
        m_ConnectThread.join();

    // Start the connect worker
    m_ConnectThread = std::thread(&CConnection::ConnectWorker, this);
    return true;
}

bool CConnection::WaitForConnection(uint32_t uiWaitMs)
{
    if (m_ConnectionStatus.load(std::memory_order_acquire) ==
        sdv::ipc::EConnectStatus::connected)
    {
        return true;
    }

    std::unique_lock<std::mutex> lk(m_MtxConnect);

    if (uiWaitMs == 0xFFFFFFFFu) // INFINITE
    {
        m_CvConnect.wait(
            lk,
            [this]
            {
                return m_ConnectionStatus.load(std::memory_order_acquire) ==
                       sdv::ipc::EConnectStatus::connected;
            });
        return true;
    }

    if (uiWaitMs == 0u) // zero wait
    {
        return (m_ConnectionStatus.load(std::memory_order_acquire) ==
                sdv::ipc::EConnectStatus::connected);
    }

    // finite wait
    return m_CvConnect.wait_for(
        lk,
        std::chrono::milliseconds(uiWaitMs),
        [this]
        {
            return m_ConnectionStatus.load(std::memory_order_acquire) ==
                   sdv::ipc::EConnectStatus::connected;
        });
}

void CConnection::CancelWait()
{
    m_CancelWait.store(true);
}

void CConnection::Disconnect()
{
    m_CancelWait.store(true);
    StopThreadsAndCloseSockets();
    SetStatus(sdv::ipc::EConnectStatus::disconnected);
}

uint64_t CConnection::RegisterStatusEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback)
{
    // Extract IConnectEventCallback interface
    m_pEvent = sdv::TInterfaceAccessPtr(pEventCallback).GetInterface<sdv::ipc::IConnectEventCallback>();

    // Only one callback is supported; cookie 1 = valid
    return (m_pEvent != nullptr) ? 1ULL : 0ULL;
}

void CConnection::UnregisterStatusEventCallback(/*in*/ uint64_t uiCookie)
{
    // Only one callback supported -> cookie value is 1
    if (uiCookie == 1ULL)
    {
        m_pEvent = nullptr;
    }
}

sdv::ipc::EConnectStatus CConnection::GetStatus() const
{
    return m_ConnectionStatus;
}

void CConnection::DestroyObject()
{
    m_StopReceiveThread  = true;
    m_StopConnectThread  = true;

    StopThreadsAndCloseSockets();
    m_ConnectionStatus = sdv::ipc::EConnectStatus::disconnected;
}

void CConnection::ConnectWorker()
{
    try
    {
        if (m_AcceptConnectionRequired)
        {
            // SERVER SIDE
            if (m_ListenSocket == INVALID_SOCKET)
            {
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            SetStatus(sdv::ipc::EConnectStatus::initializing);
            SDV_LOG_INFO("[AF_UNIX] Srv ConnectWorker start: listen=%llu",
                         static_cast<uint64_t>(m_ListenSocket));

            SOCKET c = AcceptConnection();
            SDV_LOG_INFO("[AF_UNIX] Srv AcceptConnection returned: sock=%llu status=%d",
                         static_cast<uint64_t>(c),
                         static_cast<int>(m_ConnectionStatus.load()));

            if (c == INVALID_SOCKET)
            {
                if (m_pEvent)
                {
                    try
                    {
                        m_pEvent->SetStatus(m_ConnectionStatus);
                    }
                    catch (...)
                    {
                    }
                }
                return;
            }

            m_ConnectionSocket = c;
            SetStatus(sdv::ipc::EConnectStatus::connected);
            StartReceiveThread_Unsafe();
            return;
        }
        else
        {
            // CLIENT SIDE
            if (m_ConnectionSocket == INVALID_SOCKET)
            {
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }
        }

        // Client side: socket is already connected
        SetStatus(sdv::ipc::EConnectStatus::connected);
        StartReceiveThread_Unsafe();
    }
    catch (...)
    {
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
    }
}

void CConnection::StartReceiveThread_Unsafe()
{
    if (m_ReceiveThread.joinable())
    {
        m_ReceiveThread.join();
    }

    m_StopReceiveThread.store(false);
    m_ReceiveThread = std::thread(&CConnection::ReceiveMessages, this);
}

void CConnection::StopThreadsAndCloseSockets()
{
    // Signal stop
    m_StopReceiveThread.store(true);
    m_StopConnectThread.store(true);

    // Close listen socket to break select()/accept
    SOCKET l = m_ListenSocket;
    m_ListenSocket = INVALID_SOCKET;
    if (l != INVALID_SOCKET)
    {
        ::closesocket(l);
    }

    // Close active connection socket
    SOCKET s = m_ConnectionSocket;
    m_ConnectionSocket = INVALID_SOCKET;
    if (s != INVALID_SOCKET)
    {
        ::shutdown(s, SD_BOTH);
        ::closesocket(s);
    }

    const auto self = std::this_thread::get_id();

    if (m_ReceiveThread.joinable())
    {
        if (m_ReceiveThread.get_id() == self)
        {
            m_ReceiveThread.detach();
        }
        else
        {
            m_ReceiveThread.join();
        }
    }

    if (m_ConnectThread.joinable())
    {
        if (m_ConnectThread.get_id() == self)
        {
            m_ConnectThread.detach();
        }
        else
        {
            m_ConnectThread.join();
        }
    }

    SDV_LOG_INFO("[AF_UNIX] StopThreadsAndCloseSockets: closing listen=%llu conn=%llu",
                 static_cast<uint64_t>(l),
                 static_cast<uint64_t>(s));
}

bool CConnection::ReadNumberOfBytes(char* buffer, uint32_t length)
{
    uint32_t received = 0;

    while (!m_StopReceiveThread.load() && received < length)
    {
        const int n = ::recv(m_ConnectionSocket, buffer + received, length - received, 0);
        if (n == 0)
        {
            // Graceful close
            return false;
        }
        if (n < 0)
        {
            const int err = ::WSAGetLastError();
            if (err == WSAEINTR)
                continue; // retry
            if (err == WSAEWOULDBLOCK)
            {
                ::Sleep(1);
                continue;
            }

            SDV_LOG_WARNING("[UDS][RX] recv() error: ", std::strerror(err));
            return false;
        }

        received += static_cast<uint32_t>(n);
    }

    return (received == length);
}

bool CConnection::ValidateHeader(const SMsgHeader& msgHeader)
{
    // Kept only for compatibility with any legacy users (not used in SDV path)
    if ((msgHeader.msgStart == m_MsgStart) && (msgHeader.msgEnd == m_MsgEnd))
    {
        return (msgHeader.msgSize != 0);
    }
    return false;
}


uint32_t CConnection::ReadDataTable(const CMessage& message, SDataContext& dataCtx)
{
    uint32_t offset = 0;

    switch (message.GetMsgHdr().eType)
    {
    case EMsgType::data:
        offset            = static_cast<uint32_t>(sizeof(SMsgHdr));
        dataCtx.uiTotalSize = message.GetSize() - static_cast<uint32_t>(sizeof(SMsgHdr));
        break;

    case EMsgType::data_fragment:
        offset = static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
        if (message.GetFragmentedHdr().uiOffset != 0)
            return 0; // table only in first fragment
        dataCtx.uiTotalSize = message.GetFragmentedHdr().uiTotalLength;
        break;

    default:
        return 0;
    }

    dataCtx.uiCurrentOffset = 0;

    if (message.GetSize() < (offset + static_cast<uint32_t>(sizeof(uint32_t))))
        return 0;

    const uint32_t count = *reinterpret_cast<const uint32_t*>(message.GetData() + offset);

    offset += static_cast<uint32_t>(sizeof(uint32_t));
    dataCtx.uiCurrentOffset += static_cast<uint32_t>(sizeof(uint32_t));

    if (message.GetSize() <
        (offset + count * static_cast<uint32_t>(sizeof(uint32_t))))
    {
        return 0;
    }

    std::vector<size_t> sizes;
    sizes.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t sz = *reinterpret_cast<const uint32_t*>(message.GetData() + offset);
        sizes.push_back(static_cast<size_t>(sz));
        offset += static_cast<uint32_t>(sizeof(uint32_t));
        dataCtx.uiCurrentOffset += static_cast<uint32_t>(sizeof(uint32_t));
    }

    const uint32_t computed =
        dataCtx.uiCurrentOffset +
        static_cast<uint32_t>(
            std::accumulate(
                sizes.begin(),
                sizes.end(),
                static_cast<size_t>(0)));

    if (computed != dataCtx.uiTotalSize)
        return 0;

    dataCtx.seqDataChunks.clear();
    for (size_t n : sizes)
    {
        dataCtx.seqDataChunks.push_back(sdv::pointer<uint8_t>());
        dataCtx.seqDataChunks.back().resize(n);
    }

    dataCtx.nChunkIndex   = 0;
    dataCtx.uiChunkOffset = 0;

    return offset;
}

bool CConnection::ReadDataChunk(const CMessage& message, uint32_t offset, SDataContext& dataCtx)
{
    if (offset < sizeof(SMsgHdr))
        return false;

    if (message.GetMsgHdr().eType == EMsgType::data_fragment &&
        offset < sizeof(SFragmentedMsgHdr))
        return false;

    while (offset < message.GetSize() &&
           dataCtx.nChunkIndex < dataCtx.seqDataChunks.size())
    {
        const uint32_t avail = message.GetSize() - offset;
        sdv::pointer<uint8_t>& chunk = dataCtx.seqDataChunks[dataCtx.nChunkIndex];

        if (dataCtx.uiChunkOffset > static_cast<uint32_t>(chunk.size()))
            return false;

        const uint32_t need =
            static_cast<uint32_t>(chunk.size()) - dataCtx.uiChunkOffset;
        const uint32_t toCopy = std::min(avail, need);

        std::copy(
            message.GetData() + offset,
            message.GetData() + offset + toCopy,
            chunk.get() + dataCtx.uiChunkOffset);

        offset                    += toCopy;
        dataCtx.uiChunkOffset     += toCopy;

        if (dataCtx.uiChunkOffset >= static_cast<uint32_t>(chunk.size()))
        {
            dataCtx.uiChunkOffset = 0;
            ++dataCtx.nChunkIndex;

            if (dataCtx.nChunkIndex == dataCtx.seqDataChunks.size())
            {
#if ENABLE_DECOUPLING > 0
                // optional queueing path...
#else
                if (m_pReceiver)
                    m_pReceiver->ReceiveData(dataCtx.seqDataChunks);
                dataCtx = SDataContext(); // reset context
#endif
                break;
            }
        }
    }

    return true;
}

void CConnection::ReceiveSyncRequest(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

    SMsgHdr reply{};
    reply.uiVersion = SDVFrameworkInterfaceVersion;
    reply.eType     = EMsgType::sync_answer;

    uint32_t packetSize = static_cast<uint32_t>(sizeof(reply));

    if (SendExact(reinterpret_cast<char*>(&packetSize), sizeof(packetSize)) < 0)
        return;
    if (SendExact(reinterpret_cast<char*>(&reply), sizeof(reply)) < 0)
        return;
}

void CConnection::ReceiveSyncAnswer(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

    SConnectMsg req{};
    req.uiVersion = SDVFrameworkInterfaceVersion;
    req.eType     = EMsgType::connect_request;
    req.tProcessID = static_cast<uint64_t>(::GetCurrentProcessId());

    const uint32_t packetSize = static_cast<uint32_t>(sizeof(req));

    if (SendExact(reinterpret_cast<char const*>(&packetSize), sizeof(packetSize)) < 0)
        return;
    if (SendExact(reinterpret_cast<char const*>(&req), sizeof(req)) < 0)
        return;
}

void CConnection::ReceiveConnectRequest(const CMessage& message)
{
    const auto hdr = message.GetConnectHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

    SConnectMsg reply{};
    reply.uiVersion = SDVFrameworkInterfaceVersion;
    reply.eType     = EMsgType::connect_answer;
    reply.tProcessID = static_cast<uint64_t>(::GetCurrentProcessId());

    const uint32_t packetSize = static_cast<uint32_t>(sizeof(reply));

    if (SendExact(reinterpret_cast<char const*>(&packetSize), sizeof(packetSize)) < 0)
        return;
    if (SendExact(reinterpret_cast<char const*>(&reply), sizeof(reply)) < 0)
        return;
}

void CConnection::ReceiveConnectAnswer(const CMessage& message)
{
    const auto hdr = message.GetConnectHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

    // Handshake complete (client side)
    SetStatus(sdv::ipc::EConnectStatus::connected);
}

void CConnection::ReceiveConnectTerm(const CMessage& /*message*/)
{
    SetStatus(sdv::ipc::EConnectStatus::disconnected);
    m_StopReceiveThread.store(true);
}

void CConnection::ReceiveDataMessage(const CMessage& message, SDataContext& dataCtx)
{
    const uint32_t payloadOffset = ReadDataTable(message, dataCtx);
    if (payloadOffset == 0)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }

    if (!ReadDataChunk(message, payloadOffset, dataCtx))
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }
}

void CConnection::ReceiveDataFragmentMessage(const CMessage& message, SDataContext& dataCtx)
{
    uint32_t offset = static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));

    if (message.GetFragmentedHdr().uiOffset == 0)
    {
        offset = ReadDataTable(message, dataCtx);
        if (offset == 0)
        {
            SetStatus(sdv::ipc::EConnectStatus::communication_error);
            return;
        }
    }

    if (!ReadDataChunk(message, offset, dataCtx))
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        return;
    }
}

void CConnection::ReceiveMessages()
{
    try
    {
        SDataContext dataCtx;

        while (!m_StopReceiveThread.load() &&
               m_ConnectionSocket != INVALID_SOCKET)
        {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(m_ConnectionSocket, &rfds);

            TIMEVAL tv{};
            tv.tv_sec  = 0;
            tv.tv_usec = 50 * 1000; // 50 ms

            int sr = ::select(0, &rfds, nullptr, nullptr, &tv);
            if (sr == SOCKET_ERROR)
            {
                ::Sleep(1);
                continue;
            }

            if (sr == 0)
            {
                continue; // timeout
            }

            // 1. Transport header: packet size (LE, 4 bytes)
            uint32_t packetSize = 0;
            if (!ReadNumberOfBytes(reinterpret_cast<char*>(&packetSize),
                                   sizeof(packetSize)))
            {
                SetStatus(sdv::ipc::EConnectStatus::disconnected);
                SDV_LOG_WARNING("[UDS][RX] Incomplete payload read -> disconnected");
                break;
            }

            if (packetSize == 0 || packetSize > kMaxUdsPacketSize)
            {
                SetStatus(sdv::ipc::EConnectStatus::communication_error);
                break;
            }

            // 2. Payload
            std::vector<uint8_t> payload(packetSize);
            if (!ReadNumberOfBytes(reinterpret_cast<char*>(payload.data()),
                                   packetSize))
            {
                SetStatus(sdv::ipc::EConnectStatus::communication_error);
                SDV_LOG_WARNING("[UDS][RX] Invalid SDV message (envelope)");
                break;
            }

            // 3. Parse & dispatch SDV message
            CMessage msg(std::move(payload));
            if (!msg.IsValid())
            {
                SetStatus(sdv::ipc::EConnectStatus::communication_error);
                continue;
            }

            switch (msg.GetMsgHdr().eType)
            {
            case EMsgType::sync_request:    ReceiveSyncRequest(msg);          break;
            case EMsgType::sync_answer:     ReceiveSyncAnswer(msg);           break;
            case EMsgType::connect_request: ReceiveConnectRequest(msg);       break;
            case EMsgType::connect_answer:  ReceiveConnectAnswer(msg);        break;
            case EMsgType::connect_term:    ReceiveConnectTerm(msg);          break;
            case EMsgType::data:            ReceiveDataMessage(msg, dataCtx); break;
            case EMsgType::data_fragment:   ReceiveDataFragmentMessage(msg, dataCtx); break;
            default:
                // Ignore unknown types
                break;
            }

            if (m_ConnectionStatus == sdv::ipc::EConnectStatus::terminating)
                break;
        }
    }
    catch (...)
    {
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
    }
}