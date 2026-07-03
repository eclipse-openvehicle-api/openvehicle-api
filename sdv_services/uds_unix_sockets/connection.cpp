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

#if defined(__unix__)

#include "connection.h"

#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <numeric>
#include <sstream>
#include <thread>

#include <iostream>

// Anonymous namespace: local helpers (not exported)
namespace
{
    /**
    * @brief Return the directory component of a UNIX path.
    *
    * Behavior matches POSIX dirname-like semantics:
    *   - If there is no '/', returns "."
    *   - If the only slash is at index 0, returns "/"
    *   - Otherwise returns the substring up to the final '/'
    *
    * Examples:
    *    "/tmp/a/b.sock" → "/tmp/a"
    *    "b.sock"        → "."
    *    "/a"            → "/"
    *
    * @param[in] p  Full path string.
    *
    * @return Directory component of the path.
    */
    static std::string DirNameOf(const std::string& p)
    {
        const auto pos = p.find_last_of('/');
        if (pos == std::string::npos) return ".";
        if (pos == 0)                  return "/";
        return p.substr(0, pos);
    }

    /**
    * @brief Ensure that a directory exists on the filesystem
    *
    * Equivalent to `mkdir -p` for a single-level directory:
    *   - If the directory already exists, returns true
    *   - If it does not exist, attempts to create it using mode 0770
    *   - Returns true on success, false on failure
    *
    * NOTE:
    *   - Only creates one directory level (not recursive)
    *   - errno is checked for EEXIST to handle race conditions
    *
    * @param[in] dir  Directory path to check or create
    *
    * @return true if the directory exists or was successfully created;
    *         false otherwise
    */
    static bool EnsureDir(const std::string& dir)
    {
        struct stat st{};
        if (::stat(dir.c_str(), &st) == 0) return S_ISDIR(st.st_mode);
        if (::mkdir(dir.c_str(), 0770) == 0) return true;
        return errno == EEXIST;
    }

    /**
    * @brief Return the current UNIX process ID as a 64-bit value
    *
    * Used in SDV AF_UNIX connect headers to identify processes across
    * handshake messages (connect_request / connect_answer)
    *
    * @return Current process ID as uint64_t
    */
    uint64_t GetCurrentProcessID_Local()
    {
        return static_cast<uint64_t>(::getpid());
    }


} // namespace

// Construction / Destruction
CUnixSocketConnection::CUnixSocketConnection(int preconfiguredFd,
                                             bool acceptConnectionRequired,
                                             const std::string& udsPath)
    : m_Fd(preconfiguredFd)
    , m_ListenFd(-1)
    , m_AcceptConnectionRequired(acceptConnectionRequired)
    , m_UdsPath(udsPath)
    , m_StopReceiveThread(false)
    , m_StopConnectThread(false)
    , m_eConnectState(sdv::ipc::EConnectState::uninitialized)
    , m_pReceiver(nullptr)
    , m_pEvent(nullptr)
{
    // clean constructor
}

CUnixSocketConnection::~CUnixSocketConnection()
{
    try
    {
        StopThreadsAndCloseSockets(/*unlinkPath*/ false);
    }
    catch (...)
    {
        // never throw from destructor
    }
}

void CUnixSocketConnection::SetWatchDogRemoveCallback(std::function<void(const void*)> callback)
{
    std::lock_guard<std::mutex> lock(m_WatchdogMtx);
    m_WatchdogRemoveCallback = std::move(callback);
}

// Public API
std::string CUnixSocketConnection::GetConnectionString()
{
    std::ostringstream oss;
    oss << "proto=uds;"
        << "role="  << (m_AcceptConnectionRequired ? "server" : "client") << ";"
        << "path="  << m_UdsPath << ";"
        << "timeout_ms=" << 5000;
    return oss.str();
}

bool CUnixSocketConnection::SendSizedPacket(const void* pData, uint32_t uiDataLength)
{
    if (m_Fd < 0)
        return false;

#ifdef MSG_NOSIGNAL
    constexpr int kSendFlags = MSG_NOSIGNAL;
#else
    constexpr int kSendFlags = 0;
#endif

    auto sendAll = [&](const void* data, uint32_t length) -> int
    {
        const char* ptr = reinterpret_cast<const char*>(data);
        uint32_t remaining = length;

        while (remaining > 0)
        {
            const ssize_t sent = ::send(m_Fd, ptr, remaining, kSendFlags);

            if (sent < 0)
            {
                if (errno == EINTR)
                    continue;

                return errno;
            }

            if (sent == 0)
            {
                return EPIPE;
            }

            ptr += sent;
            remaining -= static_cast<uint32_t>(sent);
        }

        return 0;
    };

    const uint32_t len = uiDataLength;

    int sendErr = 0;

    {
        std::lock_guard<std::mutex> lock(m_SendMtx);

        // Transport header: packet size
        sendErr = sendAll(&len, static_cast<uint32_t>(sizeof(len)));

        // SDV payload
        if (sendErr == 0 && uiDataLength > 0)
        {
            sendErr = sendAll(pData, uiDataLength);
        }
    }

    if (sendErr != 0)
    {
        if (sendErr == EPIPE || sendErr == ECONNRESET || sendErr == ENOTCONN)
        {
            SDV_LOG_WARNING("[UDS][TX] Peer closed connection during send, errno=",
                            sendErr, " (", std::strerror(sendErr), ")");

            SetConnectState(sdv::ipc::EConnectState::disconnected);
        }
        else
        {
            SDV_LOG_WARNING("[UDS][TX] send() failed, errno=",
                            sendErr, " (", std::strerror(sendErr), ")");

            SetConnectState(sdv::ipc::EConnectState::communication_error);
        }

        return false;
    }

    return true;
}

bool CUnixSocketConnection::SendData(sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
#if ENABLE_REPORTING >= 1
    std::stringstream sizes;
    for (const auto& rptr : seqData) { if (!sizes.str().empty()) sizes << ", "; sizes << rptr.size(); }
    TRACE("[UDS] Send ", seqData.size(), " chunk(s) [", sizes.str(), "] bytes");
#endif

    // Only send when connected and FD valid
    if (m_eConnectState != sdv::ipc::EConnectState::connected || m_Fd < 0)
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        SDV_LOG_WARNING("[UDS][TX] Send requested while not connected or FD invalid (state=", static_cast<int>(m_eConnectState.load()), ")");
        return false;
    }

    // Build the length table (little-endian) and compute total payload size
    sdv::sequence<const sdv::pointer<uint8_t>*> seqTemp;
    sdv::pointer<uint8_t> table;

    const uint32_t nChunks    = static_cast<uint32_t>(seqData.size());
    const uint32_t tableBytes = (nChunks + 1u) * sizeof(uint32_t);
    table.resize(tableBytes);

    // Write the chunk count (little endian)
    std::array<uint8_t, sizeof(uint32_t)> raw{};
    std::memcpy(raw.data(), &nChunks, sizeof(uint32_t));
    size_t idx = 0;
    for (uint8_t b : raw) table[idx++] = b;

    uint64_t required = sizeof(uint32_t); // count field

    // Write each chunk size
    for (const sdv::pointer<uint8_t>& buf : seqData)
    {
        const uint32_t len = static_cast<uint32_t>(buf.size());
        required += sizeof(uint32_t);
        required += static_cast<uint64_t>(len);

        std::memcpy(raw.data(), &len, sizeof(uint32_t));
        for (uint8_t b : raw) table[idx++] = b;

        seqTemp.push_back(&buf);
    }

    // Prepend table as the first "chunk"
    seqTemp.insert(seqTemp.begin(), &table);

    // Per-frame capacity (leave header room)
    const uint32_t maxPayloadData =
        (kMaxUdsPacketSize > sizeof(SMsgHdr)) ? (kMaxUdsPacketSize - static_cast<uint32_t>(sizeof(SMsgHdr))) : 0;

    const uint32_t maxPayloadFrag =
        (kMaxUdsPacketSize > sizeof(SFragmentedMsgHdr)) ? (kMaxUdsPacketSize - static_cast<uint32_t>(sizeof(SFragmentedMsgHdr))) : 0;

    // Single-frame data?
    const bool fitsSingle = (required <= static_cast<uint64_t>(maxPayloadData));

    auto   itChunk  = seqTemp.cbegin();
    size_t pos      = 0;
    uint32_t offset = 0;

    if (fitsSingle)
    {
        const uint32_t payloadBytes = static_cast<uint32_t>(required);
        const uint32_t totalBytes   = payloadBytes + static_cast<uint32_t>(sizeof(SMsgHdr));

        std::vector<uint8_t> frame(totalBytes);
        uint32_t msgOff = 0;

        // Header
        {
            SMsgHdr hdr{};
            hdr.uiVersion = SDVFrameworkInterfaceVersion;
            hdr.eType = EMsgType::data;
            std::memcpy(frame.data(), &hdr, sizeof(SMsgHdr));
            msgOff         = static_cast<uint32_t>(sizeof(SMsgHdr));
        }

        // Table + chunks, contiguously
        while (itChunk != seqTemp.cend() && msgOff < totalBytes)
        {
            const auto& ref = *itChunk;
            const uint32_t len = static_cast<uint32_t>(ref->size());
            const uint8_t* src = reinterpret_cast<const uint8_t*>(ref->get());

            const uint32_t canCopy = std::min<uint32_t>(len - static_cast<uint32_t>(pos),
                                                        totalBytes - msgOff);
            if (canCopy)
                std::memcpy(frame.data() + msgOff, src + pos, canCopy);

            pos    += canCopy;
            msgOff += canCopy;

            if (pos >= len) { ++itChunk; pos = 0; }
        }

        if (!SendSizedPacket(frame.data(), totalBytes))
        {
            SDV_LOG_ERROR("[UDS][TX] SendSizedPacket failed for single-frame data (", totalBytes, " bytes)");
            return false;
        }
        return true;
    }

    // Fragmented sending
    if (maxPayloadFrag == 0)
    {
        SDV_LOG_ERROR("[UDS][TX] Fragmentation impossible: header too large (maxPayloadFrag=0)");
        return false;
    }

    while (itChunk != seqTemp.cend() && offset < required)
    {
        const uint32_t remaining   = static_cast<uint32_t>(required - offset);
        const uint32_t dataBytes   = std::min(remaining, maxPayloadFrag);
        const uint32_t allocBytes  = dataBytes + static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
        if (dataBytes == 0) { SDV_LOG_ERROR("[UDS][TX] Internal error: dataBytes==0 during fragmentation"); return false; }

        std::vector<uint8_t> frame(allocBytes);
        uint32_t msgOff = 0;

        // Fragment header
        {
            SFragmentedMsgHdr hdr{};
            hdr.uiVersion = SDVFrameworkInterfaceVersion;
            hdr.eType = EMsgType::data_fragment;
            hdr.uiTotalLength = static_cast<uint32_t>(required);
            hdr.uiOffset = offset;
            std::memcpy(frame.data(), &hdr, sizeof(SFragmentedMsgHdr));
            msgOff            = static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
        }

        // Copy table + payload slice for this fragment
        uint32_t copied = 0;
        while (itChunk != seqTemp.cend() && copied < dataBytes)
        {
            const auto& ref = *itChunk;
            const uint32_t len = static_cast<uint32_t>(ref->size());
            const uint8_t* src = reinterpret_cast<const uint8_t*>(ref->get());

            const uint32_t canCopy = std::min<uint32_t>(len - static_cast<uint32_t>(pos),
                                                        dataBytes - copied);
            if (canCopy)
                std::memcpy(frame.data() + msgOff, src + pos, canCopy);

            pos    += canCopy;
            msgOff += canCopy;
            copied += canCopy;

            if (pos >= len) { ++itChunk; pos = 0; }
        }

        if (!SendSizedPacket(frame.data(), allocBytes))
        {
            SDV_LOG_ERROR("[UDS][TX] SendSizedPacket failed for fragment (offset=", offset, ", size=", allocBytes, ")");
            return false;
        }
        offset += copied;
    }

    if (offset < required)
    {
        SDV_LOG_ERROR("[UDS][TX] Incomplete fragmented send: required=", required, ", sent=", offset);
        return false;
    }
    return true;
}

uint64_t CUnixSocketConnection::RegisterStateEventCallback(sdv::IInterfaceAccess* pEventCallback)
{
    if (!pEventCallback)
        return 0;

    // use SDV interface pointer for safe casting and lifetime management
    sdv::TInterfaceAccessPtr ptr(pEventCallback);

    auto* pCallback = ptr.GetInterface<sdv::ipc::IConnectEventCallback>();
    if (!pCallback)
    {
        SDV_LOG_WARNING("[UDS] RegisterStateEventCallback: invalid callback interface");
        return 0;
    }

    // check valid cookie generation (avoid 0)
    uint64_t uiCookie = 0;
    while (uiCookie == 0)
    {
        uiCookie = static_cast<uint64_t>(rand());
    }

    // protectiong shared list with write lock
    {
        std::unique_lock<std::shared_mutex> lock(m_mtxEventCallbacks);

        m_lstEventCallbacks.emplace_front(SEventCallback{
            uiCookie,
            pCallback
        });
    }

    return uiCookie;
}

void CUnixSocketConnection::UnregisterStateEventCallback(uint64_t uiCookie)
{
    if (!uiCookie) return;

    // Read-lock first (fast path): find entry
    {
        std::shared_lock<std::shared_mutex> rlock(m_mtxEventCallbacks);
        auto it = std::find_if(m_lstEventCallbacks.begin(), m_lstEventCallbacks.end(),
                               [&](const auto& e){ return e.uiCookie == uiCookie; });
        if (it == m_lstEventCallbacks.end())
            return;
        // mark as removed under read lock? Prefer upgrading to write-lock to mutate.
    }

    // Write-lock to mutate safely
    {
        std::unique_lock<std::shared_mutex> wlock(m_mtxEventCallbacks);
        auto it = std::find_if(m_lstEventCallbacks.begin(), m_lstEventCallbacks.end(),
                               [&](const auto& e){ return e.uiCookie == uiCookie; });
        if (it != m_lstEventCallbacks.end())
        {
            it->pCallback = nullptr;              // logically removed
            // Optional: compact immediately to keep list clean
            m_lstEventCallbacks.remove_if([](const SEventCallback& e){ return e.pCallback == nullptr; });
        }
    }
}

bool CUnixSocketConnection::AsyncConnect(sdv::IInterfaceAccess* pReceiver)
{
    const auto currentState = m_eConnectState.load(std::memory_order_acquire);
    if (currentState == sdv::ipc::EConnectState::connected)
    {
        return true;
    }

    if (currentState == sdv::ipc::EConnectState::initializing && m_ConnectThread.joinable())
    {
        SDV_LOG_WARNING("[UDS] AsyncConnect ignored: connect worker already running");
        return false;
    }

    // Capture callbacks under lock
    {
        std::lock_guard<std::mutex> lk(m_StateMtx);
        m_pReceiver = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IDataReceiveCallback>();
        m_pEvent    = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IConnectEventCallback>();
        m_eConnectState   = sdv::ipc::EConnectState::initializing;
        m_bConnectedOnce.store(false, std::memory_order_release);

        // Reset stop flags
        m_StopReceiveThread.store(false);
        m_StopConnectThread.store(false);
    }
    m_StateCv.notify_all();

    // If an old worker exists, join it to avoid duplicates.
    // Joining an in-progress worker here can deadlock when a previous connect attempt is still pending.
    if (m_ConnectThread.joinable()) m_ConnectThread.join();

    // Start the unique connect worker (server/client)
    m_ConnectThread = sdv::core::secure_thread(&CUnixSocketConnection::ConnectWorker, this);
    return true;
}

int CUnixSocketConnection::AcceptConnection() // deprecated
{
    const std::string dir = DirNameOf(m_UdsPath);
    if (!EnsureDir(dir))
    {
        SDV_LOG_ERROR("[UDS][Server] ensure_dir('", dir, "') failed: ", std::strerror(errno));
        return -1;
    }

    const int listenFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        SDV_LOG_ERROR("[UDS][Server] socket() failed: ", std::strerror(errno));
        return -1;
    }

    ::unlink(m_UdsPath.c_str());
    sockaddr_un addr{}; addr.sun_family = AF_UNIX;
    ::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", m_UdsPath.c_str());

    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        SDV_LOG_ERROR("[UDS][Server] bind('", m_UdsPath, "') failed: ", std::strerror(errno));
        ::close(listenFd);
        return -1;
    }
    ::chmod(m_UdsPath.c_str(), 0770);

    if (::listen(listenFd, 1) < 0)
    {
        SDV_LOG_ERROR("[UDS][Server] listen() failed: ", std::strerror(errno));
        ::close(listenFd);
        return -1;
    }

    const int clientFd = ::accept(listenFd, nullptr, nullptr);
    if (clientFd < 0)
    {
        SDV_LOG_ERROR("[UDS][Server] accept() failed: ", std::strerror(errno));
        ::close(listenFd);
        return -1;
    }
    ::close(listenFd);
    return clientFd;
}

void CUnixSocketConnection::CloseClientSocketOnly()
{
    const int fd = m_Fd;
    m_Fd = -1;

    if (fd >= 0)
    {
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    }
}


void CUnixSocketConnection::WaitForClientEnd()
{
    std::unique_lock<std::mutex> lock(m_MtxConnect);

    m_cvConnect.wait(lock, [this]
    {
        const auto state = m_eConnectState.load(std::memory_order_acquire);

        return m_StopConnectThread.load(std::memory_order_acquire) ||
               state == sdv::ipc::EConnectState::disconnected ||
               state == sdv::ipc::EConnectState::disconnected_forced ||
               state == sdv::ipc::EConnectState::communication_error ||
               state == sdv::ipc::EConnectState::connection_error;
    });

}

bool CUnixSocketConnection::WaitForConnection(uint32_t uiWaitMs)
{
#if ENABLE_REPORTING >= 1
    if (m_eConnectState == sdv::ipc::EConnectState::connected)
        TRACE("Not waiting for a connection - already connected");
    else
        TRACE("Waiting for a connection of ", uiWaitMs, "ms");
#endif

    std::unique_lock<std::mutex> lock(m_MtxConnect);

    auto isConnected = [&]()
    {
        return m_eConnectState.load(std::memory_order_acquire) ==
               sdv::ipc::EConnectState::connected;
    };


    if (isConnected())
        return true;

    // Wait for the connection to take place.
    // Attention: sporadic
    if (uiWaitMs)
        m_cvConnect.wait_for(lock, std::chrono::milliseconds(uiWaitMs), isConnected);

#if ENABLE_REPORTING >= 1
    if (m_eConnectState == sdv::ipc::EConnectState::connected)
        TRACE("Waiting finished - connection established");
    else
        TRACE("Waiting finished - timeout occurred");
#endif

    return m_eConnectState == sdv::ipc::EConnectState::connected;
}

void CUnixSocketConnection::CancelWait()
{
#if ENABLE_REPORTING >= 1
    TRACE("Cancel the (potential) waiting for a connection");
#endif

    m_cvConnect.notify_all();
}

void CUnixSocketConnection::Disconnect()
{

    // Try to notify peer before closing the socket.
    if (m_Fd >= 0 &&
        m_eConnectState.load(std::memory_order_acquire) == sdv::ipc::EConnectState::connected)
    {
        SendControlMessage(EMsgType::connect_term);
    }

    // Notify listeners before entering teardown mode.
    SetConnectState(sdv::ipc::EConnectState::disconnected);
    m_cvConnect.notify_all();

    m_StopReceiveThread.store(true, std::memory_order_release);
    m_StopConnectThread.store(true, std::memory_order_release);
    m_cvConnect.notify_all(); // wake WaitForClientEnd before joins

    StopThreadsAndCloseSockets(/*unlinkPath*/ false);
}

sdv::ipc::EConnectState CUnixSocketConnection::GetConnectState() const
{
    return m_eConnectState;
}

void CUnixSocketConnection::DestroyObject()
{
    bool expected = false;
    if (!m_DestroyObjectCalled.compare_exchange_strong(expected, true))
    {
        return;
    }

    m_StopReceiveThread.store(true);
    m_StopConnectThread.store(true);
    SetConnectState(sdv::ipc::EConnectState::terminating);

    // Perform full teardown (threads/sockets/state/waiters).
    Disconnect();

    std::function<void(const void*)> removeCallback;
    {
        std::lock_guard<std::mutex> lock(m_WatchdogMtx);
        removeCallback = std::move(m_WatchdogRemoveCallback);
    }
    if (removeCallback)
    {
        removeCallback(this);
    }
}

void CUnixSocketConnection::SetConnectState(sdv::ipc::EConnectState eConnectState)
{
    if (eConnectState == sdv::ipc::EConnectState::connected)
        m_bConnectedOnce.store(true, std::memory_order_release);

    // Update internal state atomically and wake up waiters.
    {
        std::lock_guard<std::mutex> lk(m_MtxConnect);
        m_eConnectState.store(eConnectState, std::memory_order_release);
    }

    // During teardown, owner objects may already be destructing. Do not callback then.
    const bool bTeardown = m_StopReceiveThread.load(std::memory_order_acquire) ||
                           m_StopConnectThread.load(std::memory_order_acquire);

    // Notify the legacy single-listener (kept for backward compatibility).
    if (!bTeardown && m_pEvent)
    {
        try
        {
            m_pEvent->SetConnectState(eConnectState);
        }
        catch (...) { /* swallow: user callback must not crash transport */ }
    }

    // Notify all registered listeners from the registry (read-mostly path).
    bool needCompact = false;
    if (!bTeardown)
    {
        std::shared_lock<std::shared_mutex> rlock(m_mtxEventCallbacks);
        for (const auto& entry : m_lstEventCallbacks)
        {
            if (!entry.pCallback) { needCompact = true; continue; }
            try
            {
                entry.pCallback->SetConnectState(eConnectState);
            }
            catch (...) { /* swallow per-listener */ }
        }
    }

    // Compact registry if we saw null entries (write path).
    if (!bTeardown && needCompact)
    {
        std::unique_lock<std::shared_mutex> wlock(m_mtxEventCallbacks);
        m_lstEventCallbacks.remove_if([](const SEventCallback& e){ return e.pCallback == nullptr; });
    }
}

bool CUnixSocketConnection::IsServer() const
{
    return m_AcceptConnectionRequired;
}

// Transport helpers
bool CUnixSocketConnection::ReadNumberOfBytes(char* buffer, uint32_t bufferLength)
{
    uint32_t remaining = bufferLength;
    char*    ptr       = buffer;

    while (remaining > 0 && !m_StopReceiveThread.load())
    {
        const int fd = m_Fd;       // snapshot to reduce races
        if (fd < 0) return false;

        const ssize_t ret = ::recv(fd, ptr, remaining, 0);
        if (ret == 0)        return false; // EOF
        if (ret < 0)
        {
            if (errno == EINTR)                 continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            SDV_LOG_WARNING("[UDS][RX] recv() error: ", std::strerror(errno));
            return false;
        }

        ptr       += ret;
        remaining -= static_cast<uint32_t>(ret);
    }
    return (remaining == 0);
}

bool CUnixSocketConnection::ReadTransportHeader(uint32_t& packetSize)
{
    if (!ReadNumberOfBytes(reinterpret_cast<char*>(&packetSize), sizeof(packetSize)))
        return false;

    if (packetSize == 0 || packetSize > kMaxUdsPacketSize)
    {
        SDV_LOG_WARNING("[UDS][RX] Invalid UDS packet size: ", packetSize);
        return false;
    }

    return true;
}


// Connect worker (server/client)
void CUnixSocketConnection::ConnectWorker()
{
    try
    {
        if (m_AcceptConnectionRequired)
        {
            // --- SERVER ---
            const std::string dir = DirNameOf(m_UdsPath);
            if (!EnsureDir(dir))
            {
                SDV_LOG_ERROR("[UDS][Server] ensure_dir('", dir, "') failed: ", std::strerror(errno));
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

            m_ListenFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_ListenFd < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] socket() failed: ", std::strerror(errno));
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

            ::unlink(m_UdsPath.c_str());

            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;

            if (m_UdsPath.size() >= sizeof(addr.sun_path))
            {
                SDV_LOG_ERROR("[UDS] Path too long: ", m_UdsPath, " (max ", sizeof(addr.sun_path) - 1, ")");
                ::close(m_ListenFd);
                m_ListenFd = -1;
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

            strncpy(addr.sun_path, m_UdsPath.c_str(), sizeof(addr.sun_path) - 1);
            addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

            socklen_t len = offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path) + 1;

            if (::bind(m_ListenFd, reinterpret_cast<sockaddr*>(&addr), len) < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] bind('", m_UdsPath, "') failed: ", std::strerror(errno));
                ::close(m_ListenFd);
                m_ListenFd = -1;
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

            ::chmod(m_UdsPath.c_str(), 0770);

            if (::listen(m_ListenFd, 1) < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] listen() failed: ", std::strerror(errno));
                ::close(m_ListenFd);
                m_ListenFd = -1;
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

        #if ENABLE_REPORTING >= 1
            TRACE("[UDS][Server] Listening on path ", m_UdsPath);
        #endif

            SetConnectState(sdv::ipc::EConnectState::initializing);

            // Keep listener alive and accept clients repeatedly.
            while (!m_StopConnectThread.load())
            {
                int clientFd = -1;
                bool acceptFatal = false;

                // Cancellable accept loop
                while (!m_StopConnectThread.load())
                {
                    struct pollfd pfd{ m_ListenFd, POLLIN, 0 };
                    const int pr = ::poll(&pfd, 1, /*timeout_ms*/ 10);

                    if (pr < 0)
                    {
                        if (errno == EINTR) continue;
                        if (m_StopConnectThread.load()) break;
                        SDV_LOG_WARNING("[UDS][Server] poll() error: ", std::strerror(errno));
                        acceptFatal = true;
                        break;
                    }

                    if (pr == 0) continue;

                    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
                    {
        #if ENABLE_REPORTING >= 1
                        TRACE("[UDS][Server] Listening socket closed/invalidated");
        #endif
                        if (!m_StopConnectThread.load())
                            acceptFatal = true;
                        break;
                    }

                    if (pfd.revents & POLLIN)
                    {
                        clientFd = ::accept(m_ListenFd, nullptr, nullptr);

                        if (clientFd < 0)
                        {
                            if (errno == EINTR) continue;
                            if (m_StopConnectThread.load()) break;
                            SDV_LOG_ERROR("[UDS][Server] accept() failed: ", std::strerror(errno));
                            acceptFatal = true;
                            break;
                        }
                        break; // one client accepted
                    }
                }

                if (m_StopConnectThread.load())
                    break;

                if (acceptFatal || clientFd < 0)
                {
                    SetConnectState(sdv::ipc::EConnectState::connection_error);
                    break;
                }

                // Serve one client session
                m_Fd = clientFd;
                SetConnectState(sdv::ipc::EConnectState::initialized);
                StartReceiveThread_Unsafe();

                // Wait until that client disconnects or teardown is requested
                WaitForClientEnd();
                CloseClientSocketOnly();

                if (!m_StopConnectThread.load())
                {
                    // Re-arm state for next accept
                    SetConnectState(sdv::ipc::EConnectState::initializing);
                }
            }

            // Close listen FD on final shutdown/error
            const int lfd = m_ListenFd;
            m_ListenFd = -1;
            if (lfd >= 0) ::close(lfd);

            if (m_StopConnectThread.load())
            {
        #if ENABLE_REPORTING >= 1
                TRACE("[UDS][Server] ConnectWorker stopped intentionally");
        #endif
            }

            return;
        }
        else
        {
        // --- CLIENT ---
            m_Fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_Fd < 0)
            {
                SDV_LOG_ERROR("[UDS][Client] socket() failed: ", std::strerror(errno));
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }

            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;  
            
            if (m_UdsPath.size() >= sizeof(addr.sun_path)) 
            {
                SDV_LOG_ERROR("[UDS] Path too long: ", m_UdsPath, " (max ", sizeof(addr.sun_path) - 1, ")");
                ::close(m_Fd);
                m_Fd = -1;
                SetConnectState(sdv::ipc::EConnectState::connection_error);
                return;
            }
            
            strncpy(addr.sun_path, m_UdsPath.c_str(), sizeof(addr.sun_path) - 1);
            addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

            socklen_t len = offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path) + 1;

#if ENABLE_REPORTING >= 1
            TRACE("[UDS][Client] Connecting to ", m_UdsPath);
#endif

            int attempts = 10;
            while (attempts-- > 0 && !m_StopConnectThread.load())
            {
                if (::connect(m_Fd, reinterpret_cast<struct sockaddr*>(&addr), len) == 0)
                {
                    SetConnectState(sdv::ipc::EConnectState::initialized);
                    StartReceiveThread_Unsafe();
                    return;
                }
                else
                {
                    SDV_LOG_WARNING("Connect failed: ", strerror(errno));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            // Disconnect() requested while retries were running: do not overwrite state with connection_error.
            if (m_StopConnectThread.load())
            {
                ::close(m_Fd);
                m_Fd = -1;
                return;
            }

            SDV_LOG_WARNING("[UDS][Client] connect() timeout to '", m_UdsPath,
                            "', last errno=", errno, " (", std::strerror(errno), ")");

            ::close(m_Fd);
            m_Fd = -1;
            SetConnectState(sdv::ipc::EConnectState::connection_error);
            return;
        }
    }
    catch (const std::exception& ex)
    {
        if (!m_StopConnectThread.load(std::memory_order_acquire))
        {
            SDV_LOG_ERROR("[UDS][ConnectWorker] exception: ", ex.what());
            SetConnectState(sdv::ipc::EConnectState::connection_error);
        }
    }
    catch (...)
    {
        if (!m_StopConnectThread.load(std::memory_order_acquire))
        {
            SDV_LOG_ERROR("[UDS][ConnectWorker] unknown exception");
            SetConnectState(sdv::ipc::EConnectState::connection_error);
        }
    }
}

void CUnixSocketConnection::ReceiveMessages()
{
    try
    {
        // Inform potential waiters that RX started
        m_StateCv.notify_all();

        SDataContext dataCtx;
        auto tpStart = std::chrono::high_resolution_clock::time_point{};

#if ENABLE_REPORTING >= 1
        TRACE("[UDS][RX] Start receive loop");
#endif

        while (!m_StopReceiveThread.load())
        {
            if (m_eConnectState == sdv::ipc::EConnectState::terminating) break;

            // Snapshot FD
            const int fd = m_Fd;
            if (fd < 0)
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                SDV_LOG_WARNING("[UDS][RX] FD invalidated -> disconnected");
                break;
            }

            // SHM-like pacing via poll()
            struct pollfd pfd{ fd, POLLIN, 0 };
            const int pr = ::poll(&pfd, 1, /*timeout_ms*/ 100);

            if (pr == 0)
            {
                if (!m_AcceptConnectionRequired && 
                    (m_eConnectState == sdv::ipc::EConnectState::initialized ||
                     m_eConnectState == sdv::ipc::EConnectState::disconnected))
                {
                    auto now = std::chrono::high_resolution_clock::now();
                    if (std::chrono::duration<double>(now - tpStart).count() > 0.5)
                    {
                        tpStart = now;

                        if (!SendControlMessage(EMsgType::sync_request))
                        {
                            SDV_LOG_WARNING("[UDS][RX] Failed to send periodic sync_request");
                        }
                    }
                }
                continue;
            }
            if (pr < 0)
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                SDV_LOG_WARNING("[UDS][RX] poll() failed -> disconnected, errno=", errno,
                    " (", std::strerror(errno), ")");
                break;
            }

            const short revents = pfd.revents;
            const bool bHasPollIn = (revents & POLLIN) != 0;
            const bool bHasPollErr = (revents & (POLLERR | POLLNVAL)) != 0;
            const bool bHasPollHup = (revents & POLLHUP) != 0;
            const bool bHasPollNval = (revents & POLLNVAL) != 0;

            // POLLNVAL commonly happens when another thread closed/replaced the FD during local shutdown.
            // In this case treat it as an expected teardown signal.
            if (bHasPollNval && (m_StopReceiveThread.load() || m_Fd < 0))
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                break;
            }

            // If POLLIN is present together with HUP/ERR, consume pending data first.
            // This avoids dropping the last frame when peer closes after sending.
            if (bHasPollErr && !bHasPollIn)
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                break;
            }

            if (bHasPollHup && !bHasPollIn)
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                break;
            }

            if (!bHasPollIn) continue;


            // Transport header
            uint32_t packetSize = 0;
            if (!ReadTransportHeader(packetSize))
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                if (!bHasPollHup)
                    SDV_LOG_WARNING("[UDS][RX] Invalid/missing transport header -> disconnected");
                break;
            }

            // Payload
            std::vector<uint8_t> payload(packetSize);
            if (!ReadNumberOfBytes(reinterpret_cast<char*>(payload.data()), packetSize))
            {
                SetConnectState(sdv::ipc::EConnectState::disconnected);
                if (!bHasPollHup)
                    SDV_LOG_WARNING("[UDS][RX] Incomplete payload read -> disconnected");
                break;
            }

            CMessage msg(std::move(payload));
            if (!msg.IsValid())
            {
                SetConnectState(sdv::ipc::EConnectState::communication_error);
                SDV_LOG_WARNING("[UDS][RX] Invalid SDV message (envelope)");
                continue;
            }

            if (m_eConnectState == sdv::ipc::EConnectState::terminating) break;

#if ENABLE_REPORTING >= 1
            switch (msg.GetMsgHdr().eType)
            {
                case EMsgType::data:
                case EMsgType::data_fragment: break;
                default:
                    TRACE("[UDS][RX] Receive raw ", static_cast<uint32_t>(msg.GetMsgHdr().eType),
                          " (", msg.GetSize(), " bytes)");
            }
#endif

            // SDV state machine
            switch (msg.GetMsgHdr().eType)
            {
                case EMsgType::sync_request:     ReceiveSyncRequest(msg);            break;
                case EMsgType::connect_request:  ReceiveConnectRequest(msg);         break;
                case EMsgType::sync_answer:      ReceiveSyncAnswer(msg);             break;
                case EMsgType::connect_answer:   ReceiveConnectAnswer(msg);          break;
                case EMsgType::connect_term:
                    ReceiveConnectTerm(msg);
                    if (m_AcceptConnectionRequired) tpStart = std::chrono::high_resolution_clock::now();
                    break;
                case EMsgType::data:             ReceiveDataMessage(msg, dataCtx);   break;
                case EMsgType::data_fragment:    ReceiveDataFragmentMessage(msg, dataCtx); break;
                default: /* ignore */ break;
            }
        }

#if ENABLE_REPORTING >= 1
        TRACE("[UDS][RX] Stop receive loop");
#endif
    }
    catch (const std::exception& ex)
    {
        SDV_LOG_ERROR("[UDS][RX] exception: ", ex.what());
        SetConnectState(sdv::ipc::EConnectState::disconnected);
    }
    catch (...)
    {
        SDV_LOG_ERROR("[UDS][RX] unknown exception");
        SetConnectState(sdv::ipc::EConnectState::disconnected);
    }
}

void CUnixSocketConnection::ReceiveSyncRequest(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();

    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        return;
    }

    if (!IsServer())
        return;

    SetConnectState(sdv::ipc::EConnectState::connecting);

    if (!SendControlMessage(EMsgType::sync_answer))
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        return;
    }

}

// Receive thread and SDV state machine
void CUnixSocketConnection::ReceiveSyncAnswer(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        SDV_LOG_WARNING("[UDS][RX] sync_answer with invalid version");
        return;
    }
 
    if (IsServer())
        return;

    SetConnectState(sdv::ipc::EConnectState::negotiating);

    if (!SendConnectMessage(EMsgType::connect_request))
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        return;
    }

}

void CUnixSocketConnection::ReceiveConnectRequest(const CMessage& message)
{
    //if (m_eConnectState == sdv::ipc::EConnectState::connecting)
    //{
    	const auto hdr = message.GetConnectHdr();
	if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
	{
	    SetConnectState(sdv::ipc::EConnectState::communication_error);
	    SDV_LOG_WARNING("[UDS][RX] connect_request with invalid version");
	    return;
	}

	// connect_request must be handled only by server
	if (!IsServer())
	{
	    SDV_LOG_WARNING("[UDS][RX] Client received connect_request - ignored");
	    return;
	}
	
	if (!SendConnectMessage(EMsgType::connect_answer))
	{
	    SDV_LOG_ERROR("[UDS][RX] Failed to send connect_answer");
	    SetConnectState(sdv::ipc::EConnectState::communication_error);
	    return;
	}

	SetConnectState(sdv::ipc::EConnectState::connected);
    m_cvConnect.notify_all();

#if ENABLE_REPORTING >= 1
        TRACE("Trigger connected");
#endif

	//}
}

void CUnixSocketConnection::ReceiveConnectAnswer(const CMessage& message)
{
    //if (m_eConnectState == sdv::ipc::EConnectState::negotiating)
    //{	
	    const auto hdr = message.GetConnectHdr();
	    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
	    {
	        SetConnectState(sdv::ipc::EConnectState::communication_error);
	        SDV_LOG_WARNING("[UDS][RX] connect_answer with invalid version");
	        return;
	    }
	    // connect_answer must be handled only by client
	    if (IsServer())
	    {
	    	SDV_LOG_WARNING("[UDS][RX] Server received connect_answer - ignored");
		    return;
	    }

    	    SetConnectState(sdv::ipc::EConnectState::connected);
            m_cvConnect.notify_all();   
#if ENABLE_REPORTING >= 1
		TRACE("[UDS][RX] Client connected after connect_answer");
#endif

	//}
	
}

bool CUnixSocketConnection::SendControlMessage(EMsgType type)
{
    // Only simple header-based control messages are allowed here.
    if (type != EMsgType::sync_request &&
        type != EMsgType::sync_answer &&
        type != EMsgType::connect_term)
    {
        SDV_LOG_ERROR("[UDS][TX] Invalid control message type");
        return false;
    }

    SMsgHdr msg{};
    msg.uiVersion = SDVFrameworkInterfaceVersion;
    msg.eType     = type;

#if ENABLE_REPORTING >= 1
    TRACE("[UDS][TX] Send control message type=", static_cast<uint32_t>(type));
#endif

    return SendSizedPacket(&msg, sizeof(msg));
}

bool CUnixSocketConnection::SendConnectMessage(EMsgType type)
{
    if (type != EMsgType::connect_request &&
        type != EMsgType::connect_answer)
    {
        SDV_LOG_ERROR("[UDS][TX] Invalid connect message type");
        return false;
    }

    SConnectMsg msg{};
    msg.uiVersion  = SDVFrameworkInterfaceVersion;
    msg.eType      = type;
    msg.tProcessID = GetCurrentProcessID_Local();

#if ENABLE_REPORTING >= 1
    TRACE("[UDS][TX] Send ",
          type == EMsgType::connect_request ? "connect_request" : "connect_answer",
          " pid=", msg.tProcessID);
#endif

    return SendSizedPacket(&msg, sizeof(msg));
}

void CUnixSocketConnection::ReceiveConnectTerm(const CMessage& /*message*/)
{
    // Peer requested termination
    SetConnectState(sdv::ipc::EConnectState::disconnected);
    // m_rWatchDog.RemoveMonitor(this);

    m_StopReceiveThread.store(true, std::memory_order_release);
    m_cvConnect.notify_all();

}

void CUnixSocketConnection::StartReceiveThread_Unsafe()
{
    if (m_ReceiveThread.joinable())
    {
        if (m_ReceiveThread.get_id() == std::this_thread::get_id())
            m_ReceiveThread.detach();
        else
            m_ReceiveThread.join();
    }

    m_StopReceiveThread.store(false);
    m_ReceiveThread = sdv::core::secure_thread(&CUnixSocketConnection::ReceiveMessages, this);
}

void CUnixSocketConnection::StopThreadsAndCloseSockets(bool unlinkPath)
{

    {
        // Prevent callbacks into potentially destroyed owners during teardown.
        std::lock_guard<std::mutex> lk(m_StateMtx);
        m_pReceiver = nullptr;
        m_pEvent = nullptr;
    }
    {
        std::unique_lock<std::shared_mutex> wlock(m_mtxEventCallbacks);
        m_lstEventCallbacks.clear();
    }

    // Signal stop
    m_StopReceiveThread.store(true);
    m_StopConnectThread.store(true);

    m_cvConnect.notify_all(); // defensive wake-up for any waiter

    // Close listen FD to break accept()
    const int lfd = m_ListenFd; m_ListenFd = -1;
    if (lfd >= 0) ::close(lfd);

    // Close active FD to break recv()/send()
    const int fd = m_Fd; m_Fd = -1;
    if (fd >= 0) { ::shutdown(fd, SHUT_RDWR); ::close(fd); }

    // Join/detach threads (avoid self-join)
    const auto self = std::this_thread::get_id();

    if (m_ReceiveThread.joinable())
    {
        if (m_ReceiveThread.get_id() == self) m_ReceiveThread.detach();
        else                                   m_ReceiveThread.join();
    }

    if (m_ConnectThread.joinable())
    {
        if (m_ConnectThread.get_id() == self) m_ConnectThread.detach();
        else                                   m_ConnectThread.join();
    }

    if (unlinkPath && !m_UdsPath.empty())
        ::unlink(m_UdsPath.c_str());
}

// Data receive helpers
void CUnixSocketConnection::ReceiveDataMessage(const CMessage& rMessage, SDataContext& rsDataCtxt)
{
#if ENABLE_REPORTING >= 1
    TRACE("[UDS][RX] Start receive data message");
#endif
    uint32_t uiOffset = ReadDataTable(rMessage, rsDataCtxt);
    if (!uiOffset) 
    { 
        SetConnectState(sdv::ipc::EConnectState::communication_error); 
        SDV_LOG_WARNING("[UDS][RX] Invalid data table"); 
        return; 
    }

#if ENABLE_REPORTING >= 1
    std::stringstream ss;
    for (const auto& p : rsDataCtxt.seqDataChunks) { if (!ss.str().empty()) ss << ", "; ss << p.size(); }
    TRACE("[UDS][RX] Data message has ", rsDataCtxt.seqDataChunks.size(), " chunk(s) [", ss.str(), "] bytes");
#endif

    if (!ReadDataChunk(rMessage, uiOffset, rsDataCtxt))
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        SDV_LOG_WARNING("[UDS][RX] Failed to read data chunk");
        return;
    }

#if ENABLE_REPORTING >= 1
    TRACE("[UDS][RX] Finished reception of data message");
#endif
}

void CUnixSocketConnection::ReceiveDataFragmentMessage(const CMessage& rMessage, SDataContext& rsDataCtxt)
{
    uint32_t uiOffset = sizeof(SFragmentedMsgHdr);

    if (rMessage.GetFragmentedHdr().uiOffset == 0)
    {
#if ENABLE_REPORTING >= 1
        TRACE("[UDS][RX] Start receive fragmented data message");
#endif
        uiOffset = ReadDataTable(rMessage, rsDataCtxt);
        if (!uiOffset) 
        { 
            SetConnectState(sdv::ipc::EConnectState::communication_error); 
            SDV_LOG_WARNING("[UDS][RX] Invalid fragmented data table"); 
            return; 
        }

#if ENABLE_REPORTING >= 1
        std::stringstream ss;
        for (const auto& p : rsDataCtxt.seqDataChunks) { if (!ss.str().empty()) ss << ", "; ss << p.size(); }
        TRACE("[UDS][RX] Fragmented message has ", rsDataCtxt.seqDataChunks.size(), " chunk(s) [", ss.str(), "] bytes");
#endif
    }

    if (!ReadDataChunk(rMessage, uiOffset, rsDataCtxt))
    {
        SetConnectState(sdv::ipc::EConnectState::communication_error);
        SDV_LOG_WARNING("[UDS][RX] Failed to read fragmented chunk");
        return;
    }
}

uint32_t CUnixSocketConnection::ReadDataTable(const CMessage& rMessage, SDataContext& rsDataCtxt)
{
    uint32_t uiOffset = 0;

    switch (rMessage.GetMsgHdr().eType)
    {
        case EMsgType::data:
            uiOffset = static_cast<uint32_t>(sizeof(SMsgHdr));
            rsDataCtxt.uiTotalSize = rMessage.GetSize() - static_cast<uint32_t>(sizeof(SMsgHdr));
            break;
        case EMsgType::data_fragment:
            uiOffset = static_cast<uint32_t>(sizeof(SFragmentedMsgHdr));
            if (rMessage.GetFragmentedHdr().uiOffset) return 0; // only in first fragment
            rsDataCtxt.uiTotalSize = rMessage.GetFragmentedHdr().uiTotalLength;
            break;
        default:
            return 0;
    }

    rsDataCtxt.uiCurrentOffset = 0;

    // Buffer count
    if (rMessage.GetSize() < (uiOffset + static_cast<uint32_t>(sizeof(uint32_t)))) return 0;
    uint32_t uiAmount = 0;
    std::memcpy(&uiAmount, rMessage.GetData() + uiOffset, sizeof(uiAmount));
    uiOffset += sizeof(uint32_t);
    rsDataCtxt.uiCurrentOffset += sizeof(uint32_t);

    // Chunk sizes
    if (rMessage.GetSize() < (uiOffset + uiAmount * static_cast<uint32_t>(sizeof(uint32_t)))) return 0;

    std::vector<size_t> sizes;
    sizes.reserve(uiAmount);

    for (uint32_t i = 0; i < uiAmount; ++i)
    {
        uint32_t sz = 0;
        std::memcpy(&sz, rMessage.GetData() + uiOffset, sizeof(sz));
        sizes.push_back(static_cast<size_t>(sz));
        uiOffset += sizeof(uint32_t);
        rsDataCtxt.uiCurrentOffset += sizeof(uint32_t);
    }

    const uint32_t computed = rsDataCtxt.uiCurrentOffset +
                              static_cast<uint32_t>(std::accumulate(sizes.begin(), sizes.end(), static_cast<size_t>(0)));
    if (computed != rsDataCtxt.uiTotalSize) return 0;

    // Allocate chunks
    for (size_t n : sizes)
    {
        rsDataCtxt.seqDataChunks.push_back(sdv::pointer<uint8_t>());
        rsDataCtxt.seqDataChunks.back().resize(n);
    }

    rsDataCtxt.nChunkIndex   = 0;
    rsDataCtxt.uiChunkOffset = 0;

    return uiOffset;
}

bool CUnixSocketConnection::ReadDataChunk(const CMessage& rMessage, uint32_t uiOffset, SDataContext& rsDataCtxt)
{
    if (uiOffset < sizeof(SMsgHdr)) return false; // header not skipped
    if (rMessage.GetMsgHdr().eType == EMsgType::data_fragment && uiOffset < sizeof(SFragmentedMsgHdr)) return false;

    while (uiOffset < rMessage.GetSize() && rsDataCtxt.nChunkIndex < rsDataCtxt.seqDataChunks.size())
    {
        const uint32_t msgAvail = rMessage.GetSize() - uiOffset;
        sdv::pointer<uint8_t>& chunk = rsDataCtxt.seqDataChunks[rsDataCtxt.nChunkIndex];

        if (rsDataCtxt.uiChunkOffset > static_cast<uint32_t>(chunk.size())) return false;

        const uint32_t need   = static_cast<uint32_t>(chunk.size()) - rsDataCtxt.uiChunkOffset;
        const uint32_t toCopy = std::min(msgAvail, need);

        std::copy(rMessage.GetData() + uiOffset,
                  rMessage.GetData() + uiOffset + toCopy,
                  chunk.get() + rsDataCtxt.uiChunkOffset);

        uiOffset += toCopy;
        rsDataCtxt.uiChunkOffset += toCopy;

        // Done with this chunk?
        if (rsDataCtxt.uiChunkOffset >= static_cast<uint32_t>(chunk.size()))
        {
            rsDataCtxt.uiChunkOffset = 0;
            ++rsDataCtxt.nChunkIndex;

            if (rsDataCtxt.nChunkIndex == rsDataCtxt.seqDataChunks.size())
            {
#if ENABLE_REPORTING >= 1
                std::stringstream ss;
                for (const auto& p : rsDataCtxt.seqDataChunks) { if (!ss.str().empty()) ss << ", "; ss << p.size(); }
                TRACE("Received complete data package with ", rsDataCtxt.seqDataChunks.size(),
                      " chunk(s) of {", ss.str(), "} bytes");
#endif

#if ENABLE_DECOUPLING > 0
                // Optional queueing path (omitted here-feature behind macro)
#else
                if (m_pReceiver) m_pReceiver->ReceiveData(rsDataCtxt.seqDataChunks);
                rsDataCtxt = SDataContext(); // reset context
#endif
                break;
            }
        }
    }

    return true;
}

#endif // defined(__unix__)