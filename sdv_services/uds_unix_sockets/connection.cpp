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
    , m_eStatus(sdv::ipc::EConnectStatus::uninitialized)
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
    if (m_Fd < 0) return false;

    const uint32_t len = uiDataLength;

    std::lock_guard<std::mutex> lock(m_SendMtx);

    // Transport header: packet size
    if (::send(m_Fd, &len, sizeof(len), 0) != static_cast<ssize_t>(sizeof(len)))
        return false;

    // SDV payload
    const char* ptr     = reinterpret_cast<const char*>(pData);
    uint32_t    remain  = uiDataLength;

    while (remain > 0)
    {
        const ssize_t sent = ::send(m_Fd, ptr, remain, 0);
        if (sent <= 0) return false;
        ptr    += sent;
        remain -= static_cast<uint32_t>(sent);
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
    if (m_eStatus != sdv::ipc::EConnectStatus::connected || m_Fd < 0)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_WARNING("[UDS][TX] Send requested while not connected or FD invalid (status=", static_cast<int>(m_eStatus.load()), ")");
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
            auto* hdr   = reinterpret_cast<SMsgHdr*>(frame.data());
            hdr->uiVersion = SDVFrameworkInterfaceVersion;
            hdr->eType     = EMsgType::data;
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
            auto* hdr    = reinterpret_cast<SFragmentedMsgHdr*>(frame.data());
            hdr->uiVersion    = SDVFrameworkInterfaceVersion;
            hdr->eType        = EMsgType::data_fragment;
            hdr->uiTotalLength= static_cast<uint32_t>(required);
            hdr->uiOffset     = offset;
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

uint64_t CUnixSocketConnection::RegisterStatusEventCallback(sdv::IInterfaceAccess* pEventCallback)
{
    if (!pEventCallback) return 0;

    auto* pCb = sdv::TInterfaceAccessPtr(pEventCallback).GetInterface<sdv::ipc::IConnectEventCallback>();
    if (!pCb) return 0;

    // Generate a non-zero cookie
    uint64_t cookie = 0;
    do 
    { 
        cookie = static_cast<uint64_t>(::rand()); 
    } while (cookie == 0);

    // Write-lock: add entry
    {
        std::unique_lock<std::shared_mutex> lock(m_mtxEventCallbacks);
        m_lstEventCallbacks.emplace_front(SEventCallback{ cookie, pCb });
    }

    return cookie;
}

void CUnixSocketConnection::UnregisterStatusEventCallback(uint64_t uiCookie)
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
    // Capture callbacks under lock
    {
        std::lock_guard<std::mutex> lk(m_StateMtx);
        m_pReceiver = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IDataReceiveCallback>();
        m_pEvent    = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IConnectEventCallback>();
        m_eStatus   = sdv::ipc::EConnectStatus::initializing;

        // Reset stop flags
        m_StopReceiveThread.store(false);
        m_StopConnectThread.store(false);
    }
    m_StateCv.notify_all();

    // If an old worker exists, join it to avoid duplicates
    if (m_ConnectThread.joinable()) m_ConnectThread.join();

    // Start the unique connect worker (server/client)
    m_ConnectThread = std::thread(&CUnixSocketConnection::ConnectWorker, this);
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

bool CUnixSocketConnection::WaitForConnection(uint32_t uiWaitMs)
{
    if (m_eStatus.load(std::memory_order_acquire) == sdv::ipc::EConnectStatus::connected) return true;

    std::unique_lock<std::mutex> lk(m_MtxConnect);

    if (uiWaitMs == 0xFFFFFFFFu)
    {
        m_CvConnect.wait(lk, [this]{
            return m_eStatus.load(std::memory_order_acquire) == sdv::ipc::EConnectStatus::connected;
        });
        return true;
    }
    if (uiWaitMs == 0u)
        return (m_eStatus.load(std::memory_order_acquire) == sdv::ipc::EConnectStatus::connected);

    return m_CvConnect.wait_for(lk, std::chrono::milliseconds(uiWaitMs),
                                [this]{ return m_eStatus.load(std::memory_order_acquire) == sdv::ipc::EConnectStatus::connected; });
}

void CUnixSocketConnection::CancelWait()
{
    // optional: m_CvConnect.notify_all();
}

void CUnixSocketConnection::Disconnect()
{
    StopThreadsAndCloseSockets(/*unlinkPath*/ false);
    SetStatus(sdv::ipc::EConnectStatus::disconnected);
}

sdv::ipc::EConnectStatus CUnixSocketConnection::GetStatus() const
{
    return m_eStatus;
}

void CUnixSocketConnection::DestroyObject()
{
    m_StopReceiveThread.store(true);
    m_eStatus = sdv::ipc::EConnectStatus::disconnected;
}

void CUnixSocketConnection::SetStatus(sdv::ipc::EConnectStatus eStatus)
{
    // Update internal state atomically and wake up waiters.
    {
        std::lock_guard<std::mutex> lk(m_MtxConnect);
        m_eStatus.store(eStatus, std::memory_order_release);
    }
    m_CvConnect.notify_all();

    // Notify the legacy single-listener (kept for backward compatibility).
    try
    {
        m_pEvent->SetStatus(eStatus);
    }
    catch (...) { /* swallow: user callback must not crash transport */ }

    // Notify all registered listeners from the registry (read-mostly path).
    bool needCompact = false;
    {
        std::shared_lock<std::shared_mutex> rlock(m_mtxEventCallbacks);
        for (const auto& entry : m_lstEventCallbacks)
        {
            if (!entry.pCallback) { needCompact = true; continue; }
            try
            {
                entry.pCallback->SetStatus(eStatus);
            }
            catch (...) { /* swallow per-listener */ }
        }
    }

    // Compact registry if we saw null entries (write path).
    if (needCompact)
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
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            m_ListenFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_ListenFd < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] socket() failed: ", std::strerror(errno));
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            ::unlink(m_UdsPath.c_str());
            sockaddr_un addr{}; addr.sun_family = AF_UNIX;
            ::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", m_UdsPath.c_str());

            if (::bind(m_ListenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] bind('", m_UdsPath, "') failed: ", std::strerror(errno));
                ::close(m_ListenFd); m_ListenFd = -1;
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            ::chmod(m_UdsPath.c_str(), 0770);

            if (::listen(m_ListenFd, 1) < 0)
            {
                SDV_LOG_ERROR("[UDS][Server] listen() failed: ", std::strerror(errno));
                ::close(m_ListenFd); m_ListenFd = -1;
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

#if ENABLE_REPORTING >= 1
            TRACE("[UDS][Server] Listening on path ", m_UdsPath);
#endif

            // Cancellable accept loop (poll)
            int clientFd = -1;
            while (!m_StopConnectThread.load())
            {
                struct pollfd pfd{ m_ListenFd, POLLIN, 0 };
                const int pr = ::poll(&pfd, 1, /*timeout_ms*/ 100);
                if (pr < 0) { if (errno == EINTR) continue; SDV_LOG_WARNING("[UDS][Server] poll() error: ", std::strerror(errno)); break; }
                if (pr == 0) continue;

                if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
                {
#if ENABLE_REPORTING >= 1
                    TRACE("[UDS][Server] Listening socket closed/invalidated");
#endif
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
                        break;
                    }
                    break; // accepted
                }
            }

            // Close listen fd (safe if already closed elsewhere)
            const int lfd = m_ListenFd; m_ListenFd = -1;
            if (lfd >= 0) ::close(lfd);

            if (m_StopConnectThread.load())
            {
#if ENABLE_REPORTING >= 1
                TRACE("[UDS][Server] ConnectWorker stopped intentionally");
#endif
                return; // intentional stop
            }
            if (clientFd < 0)
            {
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            m_Fd = clientFd;
            SetStatus(sdv::ipc::EConnectStatus::connected);
            StartReceiveThread_Unsafe();
            return;
        }
        else
        {
            // --- CLIENT ---
            m_Fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (m_Fd < 0)
            {
                SDV_LOG_ERROR("[UDS][Client] socket() failed: ", std::strerror(errno));
                SetStatus(sdv::ipc::EConnectStatus::connection_error);
                return;
            }

            sockaddr_un addr{}; addr.sun_family = AF_UNIX;
            ::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", m_UdsPath.c_str());

#if ENABLE_REPORTING >= 1
            TRACE("[UDS][Client] Connecting to ", m_UdsPath);
#endif

            int attempts = 10;
            while (attempts-- > 0 && !m_StopConnectThread.load())
            {
                if (::connect(m_Fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0)
                {
                    SetStatus(sdv::ipc::EConnectStatus::connected);
#if ENABLE_REPORTING >= 1
                    TRACE("[UDS][Client] Connected");
#endif
                    StartReceiveThread_Unsafe();
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (!m_StopConnectThread.load())
            {
                SDV_LOG_WARNING("[UDS][Client] connect() timeout to '", m_UdsPath,
                                "', last errno=", errno, " (", std::strerror(errno), ")");
            }

            ::close(m_Fd); m_Fd = -1;
            SetStatus(sdv::ipc::EConnectStatus::connection_error);
            return;
        }
    }
    catch (const std::exception& ex)
    {
        SDV_LOG_ERROR("[UDS][ConnectWorker] exception: ", ex.what());
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
    }
    catch (...)
    {
        SDV_LOG_ERROR("[UDS][ConnectWorker] unknown exception");
        SetStatus(sdv::ipc::EConnectStatus::connection_error);
    }
}

// Receive thread and SDV state machine
void CUnixSocketConnection::ReceiveSyncAnswer(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_WARNING("[UDS][RX] sync_answer with invalid version");
        return;
    }

    // Client -> send connect_request
    SConnectMsg msg{};
    msg.uiVersion  = SDVFrameworkInterfaceVersion;
    msg.eType      = EMsgType::connect_request;
    msg.tProcessID = GetCurrentProcessID_Local();

    if (!SendSizedPacket(&msg, sizeof(msg)))
        SDV_LOG_ERROR("[UDS][RX] Failed to send connect_request in response to sync_answer");
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
            if (m_eStatus == sdv::ipc::EConnectStatus::terminating) break;

            // Snapshot FD
            const int fd = m_Fd;
            if (fd < 0)
            {
                SetStatus(sdv::ipc::EConnectStatus::disconnected);
                SDV_LOG_WARNING("[UDS][RX] FD invalidated -> disconnected");
                break;
            }

            // SHM-like pacing via poll()
            struct pollfd pfd{ fd, POLLIN, 0 };
            const int pr = ::poll(&pfd, 1, /*timeout_ms*/ 100);

            if (pr == 0)
            {
                if (!m_AcceptConnectionRequired && (m_eStatus == sdv::ipc::EConnectStatus::initialized))
                {
                    auto now = std::chrono::high_resolution_clock::now();
                    if (std::chrono::duration<double>(now - tpStart).count() > 0.5)
                    {
                        tpStart = now;
                        SMsgHdr sync{ SDVFrameworkInterfaceVersion, EMsgType::sync_request };
                        if (!SendSizedPacket(&sync, static_cast<uint32_t>(sizeof(sync))))
                            SDV_LOG_WARNING("[UDS][RX] Failed to send periodic sync_request (client pacing)");
                    }
                }
                continue;
            }

            if (pr < 0 || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
            {
                SetStatus(sdv::ipc::EConnectStatus::disconnected);
                SDV_LOG_WARNING("[UDS][RX] poll() hangup/error -> disconnected");
                break;
            }
            if ((pfd.revents & POLLIN) == 0) continue;

            // Transport header
            uint32_t packetSize = 0;
            if (!ReadTransportHeader(packetSize))
            {
                SetStatus(sdv::ipc::EConnectStatus::disconnected);
                SDV_LOG_WARNING("[UDS][RX] Invalid/missing transport header -> disconnected");
                break;
            }

            // Payload
            std::vector<uint8_t> payload(packetSize);
            if (!ReadNumberOfBytes(reinterpret_cast<char*>(payload.data()), packetSize))
            {
                SetStatus(sdv::ipc::EConnectStatus::disconnected);
                SDV_LOG_WARNING("[UDS][RX] Incomplete payload read -> disconnected");
                break;
            }

            CMessage msg(std::move(payload));
            if (!msg.IsValid())
            {
                SetStatus(sdv::ipc::EConnectStatus::communication_error);
                SDV_LOG_WARNING("[UDS][RX] Invalid SDV message (envelope)");
                continue;
            }

            if (m_eStatus == sdv::ipc::EConnectStatus::terminating) break;

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
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
    }
    catch (...)
    {
        SDV_LOG_ERROR("[UDS][RX] unknown exception");
        SetStatus(sdv::ipc::EConnectStatus::disconnected);
    }
}

void CUnixSocketConnection::ReceiveSyncRequest(const CMessage& message)
{
    const auto hdr = message.GetMsgHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_WARNING("[UDS][RX] sync_request with invalid version");
        return;
    }

    // Reply with sync_answer
    SMsgHdr reply{};
    reply.uiVersion = SDVFrameworkInterfaceVersion;
    reply.eType     = EMsgType::sync_answer;

    if (!SendSizedPacket(&reply, sizeof(reply)))
        SDV_LOG_ERROR("[UDS][RX] Failed to send sync_answer to sync_request");
}

void CUnixSocketConnection::ReceiveConnectRequest(const CMessage& message)
{
    const auto hdr = message.GetConnectHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_WARNING("[UDS][RX] connect_request with invalid version");
        return;
    }

    // Reply with connect_answer
    SConnectMsg reply{};
    reply.uiVersion  = SDVFrameworkInterfaceVersion;
    reply.eType      = EMsgType::connect_answer;
    reply.tProcessID = GetCurrentProcessID_Local();

    if (!SendSizedPacket(&reply, sizeof(reply)))
        SDV_LOG_ERROR("[UDS][RX] Failed to send connect_answer");
}

void CUnixSocketConnection::ReceiveConnectAnswer(const CMessage& message)
{
    const auto hdr = message.GetConnectHdr();
    if (hdr.uiVersion != SDVFrameworkInterfaceVersion)
    {
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
        SDV_LOG_WARNING("[UDS][RX] connect_answer with invalid version");
        return;
    }

    // Fully established
    SetStatus(sdv::ipc::EConnectStatus::connected);
}

void CUnixSocketConnection::ReceiveConnectTerm(const CMessage& /*message*/)
{
    // Peer requested termination
    SetStatus(sdv::ipc::EConnectStatus::disconnected);
    m_StopReceiveThread.store(true);
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
    m_ReceiveThread = std::thread(&CUnixSocketConnection::ReceiveMessages, this);
}

void CUnixSocketConnection::StopThreadsAndCloseSockets(bool unlinkPath)
{
    // Signal stop
    m_StopReceiveThread.store(true);
    m_StopConnectThread.store(true);

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
        SetStatus(sdv::ipc::EConnectStatus::communication_error); 
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
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
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
            SetStatus(sdv::ipc::EConnectStatus::communication_error); 
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
        SetStatus(sdv::ipc::EConnectStatus::communication_error);
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
    const uint32_t uiAmount = *reinterpret_cast<const uint32_t*>(rMessage.GetData() + uiOffset);
    uiOffset += sizeof(uint32_t);
    rsDataCtxt.uiCurrentOffset += sizeof(uint32_t);

    // Chunk sizes
    if (rMessage.GetSize() < (uiOffset + uiAmount * static_cast<uint32_t>(sizeof(uint32_t)))) return 0;

    std::vector<size_t> sizes;
    sizes.reserve(uiAmount);

    for (uint32_t i = 0; i < uiAmount; ++i)
    {
        const uint32_t sz = *reinterpret_cast<const uint32_t*>(rMessage.GetData() + uiOffset);
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
