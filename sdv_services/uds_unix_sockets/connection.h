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
#ifndef CONNECTION_H
#define CONNECTION_H

#include <interfaces/ipc.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <list>

/**
 * @brief Local IPC connection over Unix Domain Sockets (UDS).
 *
 * Frames on the wire are sent as size-prefixed buffers:
 *   [ uint32_t packetSize ][ SDV message bytes ... ]
 *
 * SDV protocol decoding (SMsgHdr / EMsgType / fragmentation)
 */
class CUnixSocketConnection
    : public std::enable_shared_from_this<CUnixSocketConnection>
    , public sdv::IInterfaceAccess
    , public sdv::IObjectDestroy
    , public sdv::ipc::IDataSend
    , public sdv::ipc::IConnect
{
public:
    /**
     * @brief Construct a UDS connection.
     * @param preconfiguredFd Already-open FD (>=0) or -1 if none.
     * @param acceptConnectionRequired true for server (must accept()); false for client.
     * @param udsPath Filesystem path of the UDS socket.
     */
    CUnixSocketConnection(int preconfiguredFd, bool acceptConnectionRequired, const std::string& udsPath);

    /** @brief Virtual destructor. */
    virtual ~CUnixSocketConnection();

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    /** @brief Returns the connection string (proto/role/path/timeout). */
    std::string GetConnectionString();

    // ---------- IDataSend ----------
    /** @brief Send a sequence of data buffers (may be fragmented). */
    bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    // ---------- IConnect ----------
    /** @brief Start asynchronous connect/accept worker. */
    bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Wait for the connection to become 'connected'.
     * @param uiWaitMs Timeout in milliseconds (0 = immediate, 0xFFFFFFFFu = infinite).
     * @return true if connected within the timeout; false otherwise.
     */
    bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /** @brief Optionally cancel WaitForConnection (no-op in current implementation). */
    void CancelWait() override;

    /** @brief Disconnect and teardown threads/FDs; sets state to 'disconnected'. */
    void Disconnect() override;

    // ---------- IConnect: event callbacks ----------
    /** @brief Register a state event callback (no-op storage in UDS). */
    uint64_t RegisterStateEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /** @brief Unregister a previously registered callback (no-op storage in UDS). */
    void UnregisterStateEventCallback(/*in*/ uint64_t uiCookie) override;

    /** @brief Get current connection state. */
    sdv::ipc::EConnectState GetConnectState() const override;

    /** @brief Destroy object (IObjectDestroy). */
    void DestroyObject() override;

    /** @brief Set state and notify listeners (callback-safe). */
    void SetConnectState(sdv::ipc::EConnectState eConnectState);

    /** @brief @return true if this side is server (needs accept()), false otherwise. */
    bool IsServer() const;

    // ---------- Protocol types ----------
    /** @brief SDV message type. */
    enum class EMsgType : uint32_t
    {
        sync_request     = 0,         ///< Sync request (version check; no payload).
        sync_answer      = 1,         ///< Sync answer  (version check; no payload).
        connect_request  = 10,        ///< Connection initiation (SConnectMsg).
        connect_answer   = 11,        ///< Connection answer (SConnectMsg).
        connect_term     = 90,        ///< Connection terminated.
        data             = 0x10000000,///< Data message.
        data_fragment    = 0x10000001 ///< Data fragment if payload exceeds frame size.
    };

    /** @brief SDV base message header. */
    struct SMsgHdr
    {
        uint32_t uiVersion; ///< Protocol version.
        EMsgType eType;     ///< Message type.
    };

    /** @brief Connection initiation message (extends SMsgHdr). */
    struct SConnectMsg : SMsgHdr
    {
        sdv::process::TProcessID tProcessID; ///< Process ID for lifetime monitoring.
    };

    /** @brief Fragmented data message header. */
    struct SFragmentedMsgHdr : SMsgHdr
    {
        uint32_t uiTotalLength; ///< Total payload length across all fragments.
        uint32_t uiOffset;      ///< Current byte offset into the total payload.
    };

    // ---------- Minimal SDV message wrapper ----------
    class CMessage
    {
    public:
        explicit CMessage(std::vector<uint8_t>&& data) : m_Data(std::move(data)) {}

        /** @return Pointer to message bytes (may be null if empty). */
        const uint8_t* GetData() const { return m_Data.empty() ? nullptr : m_Data.data(); }

        /** @return Message size in bytes. */
        uint32_t GetSize() const { return static_cast<uint32_t>(m_Data.size()); }

        /** @return Interpreted SDV base header (or a fallback). */
        SMsgHdr GetMsgHdr() const
        {
            if (GetSize() < sizeof(SMsgHdr)) return SMsgHdr{0, EMsgType::connect_term};
            return *reinterpret_cast<const SMsgHdr*>(GetData());
        }

        /** @return SDV connect header (or default if undersized). */
        SConnectMsg GetConnectHdr() const
        {
            if (GetSize() < sizeof(SConnectMsg)) return SConnectMsg{};
            return *reinterpret_cast<const SConnectMsg*>(GetData());
        }

        /** @return SDV fragmented header (or default if undersized). */
        SFragmentedMsgHdr GetFragmentedHdr() const
        {
            if (GetSize() < sizeof(SFragmentedMsgHdr)) return SFragmentedMsgHdr{};
            return *reinterpret_cast<const SFragmentedMsgHdr*>(GetData());
        }

        /** @return true if the SDV envelope is well-formed. */
        bool IsValid() const
        {
            if (!GetData() || GetSize() < sizeof(SMsgHdr)) return false;
            SMsgHdr hdr = GetMsgHdr();
            switch (hdr.eType)
            {
                case EMsgType::sync_request:
                case EMsgType::sync_answer:
                case EMsgType::connect_term:
                case EMsgType::data:
                    return true;
                case EMsgType::connect_request:
                case EMsgType::connect_answer:
                    return GetSize() >= sizeof(SConnectMsg);
                case EMsgType::data_fragment:
                    return GetSize() >= sizeof(SFragmentedMsgHdr);
                default:
                    return false;
            }
        }

    private:
        std::vector<uint8_t> m_Data;
    };

    /** @brief Receive-time data reassembly context. */
    struct SDataContext
    {
        uint32_t uiTotalSize      = 0; ///< Total payload size (without SDV header).
        uint32_t uiCurrentOffset  = 0; ///< Current filled byte count.
        size_t   nChunkIndex      = 0; ///< Current chunk index being filled.
        uint32_t uiChunkOffset    = 0; ///< Offset within the current chunk.
        sdv::sequence<sdv::pointer<uint8_t>> seqDataChunks; ///< Output buffers.
    };

    /**
     * @brief Event callback structure.
     */
    struct SEventCallback
    {
        uint64_t                            uiCookie = 0;           ///< Registration cookie
        sdv::ipc::IConnectEventCallback*    pCallback = nullptr;    ///< Pointer to the callback. Could be NULL when the callback
                                                                    ///< was deleted.
    };

    /** @brief UDS transport maximum frame size (safety cap). */
    static constexpr uint32_t kMaxUdsPacketSize = 64u * 1024u * 1024u; // 64 MiB

    // ---------- Transport helpers ----------
    /**
     * @brief Accept incoming client (server side).
     * @return Returns a connected socket FD or -1 on error.
     * @deprecated The poll-based accept loop in ConnectWorker() is used instead.
     */
    int AcceptConnection();

    /**
     * @brief Read exactly 'bufferLength' bytes from m_Fd into 'buffer' (blocking).
     * @return true on success; false on EOF/error.
     */
    bool ReadNumberOfBytes(char* buffer, uint32_t bufferLength);

    /**
     * @brief Read UDS transport header (size prefix) into packetSize.
     * @param[out] packetSize SDV message size to follow.
     * @return true if the header was read and valid; false otherwise.
     */
    bool ReadTransportHeader(uint32_t& packetSize);

    /**
     * @brief Send an SDV message as a UDS frame: [packetSize][payload].
     * @param pData Pointer to serialized SDV payload.
     * @param uiDataLength Payload size.
     * @return true if fully sent; false otherwise.
     */
    bool SendSizedPacket(const void* pData, uint32_t uiDataLength);

    /**
     * @brief Receive loop: read UDS frames and dispatch to the SDV state machine.
     */
    void ReceiveMessages();

    /**
     * @brief Handle an incoming sync_request message.
     *
     * Sent by the peer during the initial handshake.  
     * The server replies with sync_answer.
     *
     * @param message SDV envelope containing the sync_request header.
     */
    void ReceiveSyncRequest(const CMessage& message);

    /**
     * @brief Handle an incoming sync_answer message.
     *
     * Received by the client after sending sync_request.  
     * Triggers the next handshake step: the client sends connect_request.
     *
     * @param message SDV envelope containing the sync_answer header.
     */
    void ReceiveSyncAnswer(const CMessage& message);

    /**
     * @brief Handle an incoming connect_request message.
     *
     * Sent by the peer to request establishment of a logical SDV connection.  
     * The server replies with connect_answer.
     *
     * @param message SDV envelope containing the connect_request header.
     */
    void ReceiveConnectRequest(const CMessage& message);

    /**
     * @brief Handle an incoming connect_answer message.
     *
     * Sent by the peer after receiving our connect_request.  
     * Marks completion of the SDV handshake on the client side.
     *
     * @param message SDV envelope containing the connect_answer header.
     */
    void ReceiveConnectAnswer(const CMessage& message);

    /**
     * @brief Handle an incoming connect_term message.
     *
     * Indicates that the peer requests immediate termination of the connection.  
     * Sets state to disconnected and stops the RX loop.
     *
     * @param message SDV envelope containing the connect_term header.
     */
    void ReceiveConnectTerm(const CMessage& message);

    /**
     * @brief Handle a non-fragmented SDV data message.
     *
     * Format:  
     *   [SMsgHdr][LengthTable][DataChunks...]  
     * The function decodes the table, allocates chunk buffers,  
     * copies the full payload, and dispatches it via IDataReceiveCallback.
     *
     * @param rMessage SDV envelope containing the full data frame.
     * @param rsDataCtxt Reassembly context used to extract chunks.
     */
    void ReceiveDataMessage(const CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Handle a fragmented SDV data message.
     *
     * Format:  
     *   [SFragmentedMsgHdr][(LengthTable only in first fragment)][PayloadSlice]  
     *
     * This function appends each slice into the appropriate chunk buffer.  
     * When the final fragment is received and all chunks are complete,  
     * the assembled data is dispatched via IDataReceiveCallback.
     *
     * @param rMessage SDV envelope containing the current fragment.
     * @param rsDataCtxt Reassembly context shared across fragments.
     */
    void ReceiveDataFragmentMessage(const CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Read the data size table (buffer count + each buffer size).
     * @param rMessage SDV message containing the table.
     * @param rsDataCtxt Output reassembly context; buffers are allocated.
     * @return Offset within message data where the payload begins; 0 on error.
     */
    uint32_t ReadDataTable(const CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Copy payload bytes from 'rMessage' into the buffers allocated by ReadDataTable.
     *        Multiple calls may be required for fragmented messages.
     * @param rMessage SDV message holding the current data/fragment.
     * @param uiOffset Offset within 'rMessage' where payload starts in this frame.
     * @param rsDataCtxt Reassembly context to be filled; callback fires when complete.
     * @return true if progress was made / completion achieved, false if invalid.
     */
    bool ReadDataChunk(const CMessage& rMessage, uint32_t uiOffset, SDataContext& rsDataCtxt);

    // ---------- Internal threading ----------
    /** @brief Connect worker (server accept loop or client connect retry). */
    void ConnectWorker();
    /** @brief Start RX thread (precondition: state=connected, FD valid). */
    void StartReceiveThread_Unsafe();
    /**
     * @brief Stop workers and close sockets, then optionally unlink path.
     * @param unlinkPath If true and server, unlink the socket path on exit.
     */
    void StopThreadsAndCloseSockets(bool unlinkPath);

private:
    //Transport state
    int         m_Fd                     { -1 };  ///< Active connection FD.
    int         m_ListenFd               { -1 };  ///< Server-side listening FD.
    bool        m_AcceptConnectionRequired { false };
    std::string m_UdsPath;

    //Threads & control
    std::atomic<bool> m_StopReceiveThread { false };
    std::atomic<bool> m_StopConnectThread { false };
    std::thread       m_ReceiveThread;
    std::thread       m_ConnectThread;

    //State & synchronization
    std::condition_variable                m_StateCv;
    std::atomic<sdv::ipc::EConnectState>   m_eConnectState { sdv::ipc::EConnectState::uninitialized };
    sdv::ipc::IDataReceiveCallback*        m_pReceiver { nullptr };
    sdv::ipc::IConnectEventCallback*       m_pEvent    { nullptr };
    std::mutex                             m_MtxConnect;
    std::condition_variable                m_CvConnect;
    std::mutex                             m_StateMtx;  ///< Protects receiver/event assignment.
    std::list<SEventCallback>              m_lstEventCallbacks;  ///< List containing event callbacks
    std::shared_mutex                      m_mtxEventCallbacks;  ///< Protect access to callback list

    //TX synchronization
    std::mutex m_SendMtx;
};

#endif // CONNECTION_H
#endif // defined(__unix__)