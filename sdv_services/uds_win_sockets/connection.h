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
#ifdef _WIN32
#ifndef WIN_SOCKETS_CONNECTION_H
#define WIN_SOCKETS_CONNECTION_H

#include <interfaces/ipc.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <support/component_impl.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <algorithm>
#include <condition_variable>
#include <vector>
#include <cstdint>
#include <cstddef>

#ifdef _WIN32
#   include <WinSock2.h>
#   ifdef _MSC_VER
#       pragma comment(lib, "Ws2_32.lib")
#   endif
#endif


/// @brief Legacy framing markers for the old message header (not used by SDV envelope)
constexpr uint32_t m_MsgStart = 0x01020304; ///< Value to mark the start of the legacy message header
constexpr uint32_t m_MsgEnd   = 0x05060708; ///< Value to mark the end of the legacy message header

/**
 * @brief Legacy message header used before the SDV envelope was introduced
 *
 * Today, the active transport framing is:
 *  - 4 bytes: packet size (LE)
 *  - N bytes: SDV protocol envelope (SMsgHdr + payload)
 *
 * This header is kept for compatibility / potential reuse, but is not used in
 * the current SDV-based protocol
 */
struct SMsgHeader
{
    uint32_t msgStart         = 0; ///< Marker for the start of the header
    uint32_t msgId            = 0; ///< Message ID, must match for all message fragments
    uint32_t msgSize          = 0; ///< Size of the message without the header
    uint32_t packetNumber     = 0; ///< Index of this packet (starting at 1)
    uint32_t totalPacketCount = 0; ///< Total number of packets required for the message
    uint32_t msgEnd           = 0; ///< Marker for the end of the header
};

/**
 * @brief Remote IPC connection used by the AF_UNIX-over-WinSock transport.
 *
 * Instances of this class are typically created and owned by the
 * CSocketsChannelMgnt implementation (see channel_mgnt.*).
 *
 * Exposes:
 *  - sdv::ipc::IDataSend   : sending SDV-framed messages
 *  - sdv::ipc::IConnect    : async connect / wait / state / events
 *  - sdv::IObjectDestroy   : explicit destruction hook for SDV runtime
 */
class CWinsockConnection
    : public sdv::IInterfaceAccess
    , public sdv::ipc::IDataSend
    , public sdv::ipc::IConnect
    , public sdv::IObjectDestroy
{
public:
    /**
     * @brief default constructor used by create endpoint - allocates new buffers m_Sender and m_Receiver
     */
    CWinsockConnection();

    /**
     * @brief access existing connection
     * @param[in] preconfiguredSocket Prepared socket for the connection
     * @param[in] acceptConnectionRequired If true connection has to be accepted before receive thread can be started
     */
    CWinsockConnection(unsigned long long preconfiguredSocket, bool acceptConnectionRequired);

    /**
    * @brief Virtual destructor needed for "delete this;"
    */
    virtual ~CWinsockConnection();

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Sends data consisting of multiple data chunks via the IPC connection
     *
     * Overload of sdv::ipc::IDataSend::SendData
     *
     * The sequence may be rearranged internally to avoid copying payload
     * where possible. The transport uses an SDV envelope compatible with
     * the Linux implementation (length table + chunks, with fragmentation)
     *
     * @param[in,out] seqData Sequence of data buffers to be sent
     * @return true if all data could be sent; false otherwise
     */
    bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    /**
     * @brief Establish a connection and start sending/receiving messages
     *
     * Overload of sdv::ipc::IConnect::AsyncConnect
     *
     * @param[in] pReceiver Object that exposes IDataReceiveCallback and
     *                      optionally IConnectEventCallback
     * @return true when the connect worker was started successfully
     *         Use GetConnectState / WaitForConnection / callbacks for state
     */
    bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Wait for a connection to be established
     *
     * Overload of sdv::ipc::IConnect::WaitForConnection
     *
     * @param[in] uiWaitMs
     *      - 0         : do not wait, just check current state
     *      - 0xFFFFFFFF: wait indefinitely
     *      - otherwise : wait up to uiWaitMs milliseconds
     * @return true if the connection is in connected state when returning
     */
    bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /**
     * @brief Cancel a pending WaitForConnection
     *
     * Overload of sdv::ipc::IConnect::CancelWait
     */
    void CancelWait() override;

    /**
     * @brief Disconnect from the peer and stop I/O threads
     *
     * This sets the state to disconnected and releases the event interface
     */
    void Disconnect() override;

    /**
     * @brief Register event callback interface for connection state updates
     *
     * Overload of sdv::ipc::IConnect::RegisterStateEventCallback
     *
     * @param[in] pEventCallback Pointer to an object exposing
     *            sdv::ipc::IConnectEventCallback.
     * @return A registration cookie (1 = valid, 0 = failure)
     */
    uint64_t RegisterStateEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /**
     * @brief Unregister the state event callback using its cookie
     *
     * Overload of sdv::ipc::IConnect::UnregisterStateEventCallback
     *
     * @param[in] uiCookie Cookie returned by RegisterStateEventCallback
     */
    void UnregisterStateEventCallback(/*in*/ uint64_t uiCookie) override;

    /**
     * @brief Get current state of the connection
     *
     * @return Current sdv::ipc::EConnectState
     */
    sdv::ipc::EConnectState GetConnectState() const override;

    /**
     * @brief Destroy the object
     *
     * Overload of sdv::IObjectDestroy::DestroyObject
     * After calling this, all exposed interfaces become invalid
     */
    void DestroyObject() override;

private:

	std::mutex              m_MtxConnect;
	std::condition_variable m_CvConnect;
	std::thread					 			m_ReceiveThread;				///< Thread which receives data from the socket
	std::thread                         	m_ConnectThread;
	std::atomic<bool>			 			m_StopReceiveThread{false};		///< bool variable to stop thread
	std::atomic<bool>                   	m_StopConnectThread{false};
	std::atomic<sdv::ipc::EConnectState>	m_ConnectionState;				///< the state of the connection
	
	sdv::ipc::IDataReceiveCallback*         m_pReceiver = nullptr;			///< Receiver to pass the messages if available
	sdv::ipc::IConnectEventCallback*		m_pEvent = nullptr;				///< Event receiver
	bool						 			m_AcceptConnectionRequired;		///< if true connection has to be accepted before receive thread can be started
	mutable std::recursive_mutex 			m_SendMutex;					///< Synchronize all packages to be send

	SOCKET                              	m_ListenSocket{INVALID_SOCKET}; 			///< Server-side listening socket
	SOCKET                              	m_ConnectionSocket{INVALID_SOCKET}; 		///< Active connected socket (client <-> server)

	static constexpr uint32_t	 			m_SendMessageSize{ 1024 };									///< size for the message to be send
	static constexpr uint32_t    			m_SendBufferSize = sizeof(SMsgHeader) + m_SendMessageSize;	///< Initial size of the send buffer
	char					     			m_SendBuffer[m_SendBufferSize];								///< send buffer length
	char                         			m_ReceiveBuffer[sizeof(SMsgHeader)];						///< receive buffer, just for reading the message header
	uint32_t	                 			m_ReceiveBufferLength = sizeof(SMsgHeader);					///< receive buffer length

	std::atomic<bool> m_CancelWait{false};   

    /// @brief Server accept loop / client connect confirmation
    void ConnectWorker();

    /// @brief Start the RX thread (pre: state=connected, socket valid)
    void StartReceiveThread_Unsafe();

    /// @brief Stop worker threads and close all sockets cleanly
    void StopThreadsAndCloseSockets();

    /**
     * @brief Accept a connection from a client (server mode)
     *
     * Uses select() with a short timeout to remain responsive to stop flags
     *
     * @return A valid client socket, or INVALID_SOCKET on error
     */
    SOCKET AcceptConnection();

    /**
     * @brief Send exactly \p dataLength bytes over the connection socket
     *
     * @param[in] data       Pointer to bytes to be sent
     * @param[in] dataLength Number of bytes to send
     * @return Number of bytes actually sent, or SOCKET_ERROR on failure
     */
    int32_t Send(const char* data, int32_t dataLength);

    /**
     * @brief Low-level helper: send exactly len bytes, retrying until done
     *
     * @param[in] data Pointer to buffer
     * @param[in] len  Number of bytes to send
     * @return Total bytes sent, or -1 on failure
     */
    int SendExact(const char* data, int len);

    /// @brief Main receive loop (runs in m_ReceiveThread)
    void ReceiveMessages();

    /**
     * @brief Legacy header validation (not used in current SDV path)
     *
     * @param[in] msgHeader Header to validate
     * @return true if header markers are valid and msgSize != 0
     */
    bool ValidateHeader(const SMsgHeader& msgHeader);

    /**
     * @brief Read exactly @p length bytes into @p buffer from the socket
     *
     * @param[out] buffer Target buffer
     * @param[in]  length Number of bytes expected
     * @return true if all bytes were read, false on error/EOF
     */
    bool ReadNumberOfBytes(char* buffer, uint32_t length);

	// Protocol types
    /** @brief SDV message type */
    enum class EMsgType : uint32_t
    {
        sync_request    = 0,        ///< Sync request (version check; no payload)
        sync_answer     = 1,        ///< Sync answer (version check; no payload)
        connect_request = 10,       ///< Connection initiation (SConnectMsg)
        connect_answer  = 11,       ///< Connection answer (SConnectMsg)
        connect_term    = 90,       ///< Connection terminated
        data            = 0x10000000, ///< Data message
        data_fragment   = 0x10000001  ///< Data fragment if payload exceeds frame size
    };

    /** @brief SDV base message header */
    struct SMsgHdr
    {
        uint32_t uiVersion; ///< Protocol version
        EMsgType eType;     ///< Message type
    };

    /** @brief Connection initiation message (extends SMsgHdr) */
    struct SConnectMsg : SMsgHdr
    {
        sdv::process::TProcessID tProcessID; ///< Process ID for lifetime monitoring
    };

    /** @brief Fragmented data message header */
    struct SFragmentedMsgHdr : SMsgHdr
    {
        uint32_t uiTotalLength; ///< Total payload length across all fragments
        uint32_t uiOffset;      ///< Current byte offset into the total payload
    };

	// Minimal SDV message wrapper
    class CMessage
    {
    public:
        explicit CMessage(std::vector<uint8_t>&& data) : m_Data(std::move(data)) {}

        /** @return Pointer to message bytes (may be null if empty) */
        const uint8_t* GetData() const
        {
            return m_Data.empty() ? nullptr : m_Data.data();
        }

        /** @return Message size in bytes */
        uint32_t GetSize() const
        {
            return static_cast<uint32_t>(m_Data.size());
        }

        /** @return Interpreted SDV base header (or a fallback) */
        SMsgHdr GetMsgHdr() const
        {
            if (GetSize() < sizeof(SMsgHdr))
            {
                return SMsgHdr{0, EMsgType::connect_term};
            }
            return *reinterpret_cast<const SMsgHdr*>(GetData());
        }

        /** @return SDV connect header (or default if undersized) */
        SConnectMsg GetConnectHdr() const
        {
            if (GetSize() < sizeof(SConnectMsg))
            {
                return SConnectMsg{};
            }
            return *reinterpret_cast<const SConnectMsg*>(GetData());
        }

        /** @return SDV fragmented header (or default if undersized) */
        SFragmentedMsgHdr GetFragmentedHdr() const
        {
            if (GetSize() < sizeof(SFragmentedMsgHdr))
            {
                return SFragmentedMsgHdr{};
            }
            return *reinterpret_cast<const SFragmentedMsgHdr*>(GetData());
        }

        /** @return true if the SDV envelope is well-formed */
        bool IsValid() const
        {
            if (!GetData() || GetSize() < sizeof(SMsgHdr))
            {
                return false;
            }

            const SMsgHdr hdr = GetMsgHdr();
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

	    /** @brief Receive-time data reassembly context */
    struct SDataContext
    {
        uint32_t uiTotalSize      = 0; ///< Total payload size (without SDV header)
        uint32_t uiCurrentOffset  = 0; ///< Current filled byte count
        size_t   nChunkIndex      = 0; ///< Current chunk index being filled
        uint32_t uiChunkOffset    = 0; ///< Offset within the current chunk
        sdv::sequence<sdv::pointer<uint8_t>> seqDataChunks; ///< Output buffers
    };

    /**
     * @brief Event callback structure.
     */
    struct SEventCallback
    {
        uint64_t                            uiCookie = 0;           ///< Registration cookie
        sdv::ipc::IConnectEventCallback*    pCallback = nullptr;    ///< Pointer to the callback; Could be NULL when the callback
                                                                    ///< was deleted
    };

    /** @brief UDS transport maximum frame size (safety cap) */
    static constexpr uint32_t kMaxUdsPacketSize = 64u * 1024u * 1024u; // 64 MiB

    /**
     * @brief Handle an incoming sync_request message
     *
     * Sent by the peer during the initial handshake
     * The server replies with sync_answer
     *
     * @param message SDV envelope containing the sync_request header
     */
    void ReceiveSyncRequest(const CMessage& message);

    /**
     * @brief Handle an incoming sync_answer message
     *
     * Received by the client after sending sync_request
     * Triggers the next handshake step: the client sends connect_request
     *
     * @param message SDV envelope containing the sync_answer header
     */
    void ReceiveSyncAnswer(const CMessage& message);

    /**
     * @brief Handle an incoming connect_request message
     *
     * Sent by the peer to request establishment of a logical SDV connection
     * The server replies with connect_answer
     *
     * @param message SDV envelope containing the connect_request header
     */
    void ReceiveConnectRequest(const CMessage& message);

    /**
     * @brief Handle an incoming connect_answer message
     *
     * Sent by the peer after receiving our connect_request
     * Marks completion of the SDV handshake on the client side
     *
     * @param message SDV envelope containing the connect_answer header
     */
    void ReceiveConnectAnswer(const CMessage& message);

    /**
     * @brief Handle an incoming connect_term message
     *
     * Indicates that the peer requests immediate termination of the connection
     * Sets state to disconnected and stops the RX loop
     *
     * @param message SDV envelope containing the connect_term header
     */
    void ReceiveConnectTerm(const CMessage& message);

    /**
     * @brief Handle a non-fragmented SDV data message
     *
     * Format:  
     *   [SMsgHdr][LengthTable][DataChunks...]  
     * The function decodes the table, allocates chunk buffers,  
     * copies the full payload, and dispatches it via IDataReceiveCallback.
     *
     * @param rMessage SDV envelope containing the full data frame
     * @param rsDataCtxt Reassembly context used to extract chunks
     */
    void ReceiveDataMessage(const CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Handle a fragmented SDV data message
     *
     * Format:  
     *   [SFragmentedMsgHdr][(LengthTable only in first fragment)][PayloadSlice]  
     *
     * This function appends each slice into the appropriate chunk buffer
     * When the final fragment is received and all chunks are complete,  
     * the assembled data is dispatched via IDataReceiveCallback.
     *
     * @param rMessage SDV envelope containing the current fragment
     * @param rsDataCtxt Reassembly context shared across fragments
     */
    void ReceiveDataFragmentMessage(const CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Read the data size table (buffer count + each buffer size)
     * @param rMessage SDV message containing the table
     * @param rsDataCtxt Output reassembly context; buffers are allocated
     * @return Offset within message data where the payload begins; 0 on error
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

    /// @brief Centralized state update (notifies waiters and callbacks)
    void SetConnectState(sdv::ipc::EConnectState state);
};

#endif // WIN_SOCKETS_CONNECTION_H
#endif