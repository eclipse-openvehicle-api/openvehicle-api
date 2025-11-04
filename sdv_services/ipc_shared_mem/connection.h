/**
 * @file connection.h
 * @author Erik Verhoeven FRD DISDS1 (mailto:erik.verhoeven@zf.com)
 * @brief Implementation of connection class.
 * @version 2.0
 * @date 2024-06-24
 *
 * @copyright Copyright ZF Friedrichshafen AG (c) 2023-2025
 *
 */

#ifndef CHANNEL_H
#define CHANNEL_H

 /// Enables the reporting of messages when set to...
 ///     1. info only (no protocol, no data)
 ///     2. info and protocol (no data)
 ///     3. info, protocol and data protocol
 ///     4. info, protocol, data protocol and data content
#define ENABLE_REPORTING 0

/// When put to 1, decoupling of receive data is activated (default is not activated).
#define ENABLE_DECOUPLING 0

#if ENABLE_REPORTING > 0
/// Enable tracing
#define ENABLE_TRACE 1
#endif

#include <thread>
#include <algorithm>
#include "in_process_mem_buffer.h"
#include "shared_mem_buffer_posix.h"
#include "shared_mem_buffer_windows.h"
#include <interfaces/ipc.h>
#include <interfaces/process.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <support/component_impl.h>
#include <queue>
#include <list>
#include "../../global/trace.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

// Forward declaration
class CWatchDog;

/**
 * Class for local IPC connection
 * Created and managed by IPCAccess::AccessLocalIPCConnection(best use unique_ptr to store, so memory address stays
 * valid)
 */
class CConnection : public std::enable_shared_from_this<CConnection>, public sdv::IInterfaceAccess, public sdv::IObjectDestroy,
    public sdv::ipc::IDataSend, public sdv::ipc::IConnect
{
public:
    /**
     * @brief default constructor used by create endpoint - allocates new buffers m_Sender and m_Receiver
     * @param[in] rWatchDog Reference to the watch dog object monitoring the connected processes.
     * @param[in] uiSize Optional size of the buffer. If zero, a default buffer size of 10k is configured.
     * @param[in] rssName Optional name to be used for the connection. If empty, a random name is generated.
     * @param[in] bServer When set, the connection is the server connection; otherwise it is the client connection (determines the
     * initial communication).
     */
    CConnection(CWatchDog& rWatchDog, uint32_t uiSize, const std::string& rssName, bool bServer);

    /**
     * @brief Access existing connection
     * @param[in] rWatchDog Reference to the watch dog object monitoring the connected processes.
     * @param[in] rssConnectionString Reference to string with connection information.
     */
    CConnection(CWatchDog& rWatchDog, const std::string& rssConnectionString);

    /**
    * @brief Virtual destructor needed for "delete this;".
    */
    virtual ~CConnection();

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief get the connection string for the sender and the receiver
     * @return Returns the connection string for the sender and the receiver together
     */
    std::string GetConnectionString();

    /**
    * @brief Sends data consisting of multiple data chunks via the IPC connection.
    * Overload of sdv::ipc::IDataSend::SendData.
    * @param[inout] seqData Sequence of data buffers to be sent. The sequence might be changed to optimize the communication
    * without having to copy the data.
    * @return Return 'true' if all data could be sent; 'false' otherwise.
    */
    virtual bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    /**
     * @brief Establish a connection and start sending/receiving messages. Overload of
     * sdv::ipc::IConnect::AsyncConnect.
     * @param[in] pReceiver The message has to be forwarded.
     * @return Returns 'true' when a connection could be established. Use IConnectStatus or IConnectEventCallback to check the
     * connection state.
     */
    virtual bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Wait for a connection to take place. Overload of sdv::ipc::IConnect::WaitForConnection.
     * @param[in] uiWaitMs Wait for a connection to take place. A value of 0 doesn't wait at all, a value of 0xffffffff
     * waits for infinite time.
     * @return Returns 'true' when a connection took place.
     */
    virtual bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /**
     * @brief Cancel a wait for connection. Overload of sdv::ipc::IConnect::CancelWait.
     */
    virtual void CancelWait() override;

    // Suppress cppcheck warning. The destructor calls Disconnect without dynamic binding. This is correct so.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Disconnect from a connection. This will set the connect status to disconnected. Overload of
     * sdv::ipc::IConnect::Disconnect.
     */
    virtual void Disconnect() override;

    /**
     * @brief Register event callback interface. Overload of sdv::ipc::IConnect::RegisterStatusEventCallback.
     * @details Register a connection status event callback interface. The exposed interface must be of type
     * IConnectEventCallback. The registration will exist until a call to the unregister function with the returned cookie
     * or until the connection is terminated.
     * @param[in] pEventCallback Pointer to the object exposing the IConnectEventCallback interface.
     * @return The cookie assigned to the registration.
     */
    virtual uint64_t RegisterStatusEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /**
     * @brief Unregister the status event callback with the returned cookie from the registration. Overload of
     * sdv::ipc::IConnect::UnregisterStatusEventCallback.
     * @param[in] uiCookie The cookie returned by a previous call to the registration function.
     */
    virtual void UnregisterStatusEventCallback(/*in*/ uint64_t uiCookie) override;

    /**
     * @brief Get status of the connection. Overload of sdv::ipc::IConnect::GetStatus.
     * @return Returns the ipc::EConnectStatus struct
     */
    virtual sdv::ipc::EConnectStatus GetStatus() const override;

    /**
     * @brief Destroy the object. Overload of IObjectDestroy::DestroyObject.
     * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
     */
    virtual void DestroyObject() override;

    /**
     * @brief Set the connection status and if needed call the event callback.
     * @param[in] eStatus The new status.
     */
    void SetStatus(sdv::ipc::EConnectStatus eStatus);

    /**
     * @brief Returns whether this is a server connection or a client connection.
     * @return The server connection flag. If 'true' the connection is a server connection; otherwise a client connection.
     */
    bool IsServer() const;

#ifdef TIME_TRACKING
    /**
     * @brief Get the last fragment sent time. Used to detect gaps.
     * @return The last sent time.
     */
    std::chrono::high_resolution_clock::time_point GetLastSentTime() const { return m_tpLastSent; }

    /**
     * @brief Get the last fragment received time. Used to detect gaps.
     * @return The last received time.
     */
    std::chrono::high_resolution_clock::time_point GetLastReceiveTime() const { return m_tpLastReceived; }

    /**
     * @brief Get the last fragment received loop time. Used to detect gaps.
     * @return The last received loop time.
     */
    std::chrono::duration<double> GetLargestReceiveLoopDuration() const { return m_durationLargestDeltaReceived; }
#endif

private:
#if ENABLE_REPORTING > 0
    template <typename... TArgs>
    void Trace(TArgs... tArgs) const
    {
        return ::Trace("this=", static_cast<const void*>(this), " ", tArgs...);
    }
#endif

    /**
     * @brief Message type enum
     */
    enum class EMsgType : uint32_t
    {
        sync_request = 0,               ///< Sync request message (version check; no data).
        sync_answer = 1,                ///< Sync answer message (version check; no data).
        connect_request = 10,           ///< Connection initiation request (SConnectMsg is used)
        connect_answer = 11,            ///< Connection answer request (SConnectMsg is used)
        connect_term = 90,              ///< Connection terminated
        data = 0x10000000,              ///< Data message
        data_fragment = 0x10000001,     ///< Data fragment (if data is longer than 1/4th of the buffer).
    };

    /**
     * @brief Message header
     */
    struct SMsgHdr
    {
        uint32_t    uiVersion;      ///< Header version
        EMsgType    eType;          ///< Type of packet
    };

    /**
     * @brief Connection initiation message
     */
    struct SConnectMsg : SMsgHdr
    {
        sdv::process::TProcessID    tProcessID;    ///< Process ID needed for lifetime monitoring
    };

    /**
     * @brief Fragmented data message header.
    */
    struct SFragmentedMsgHdr : SMsgHdr
    {
        uint32_t            uiTotalLength;          ///< The total length the data has.
        uint32_t            uiOffset;               ///< Current offset of the data.
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

    CWatchDog&                              m_rWatchDog;                    ///< Reference to the watch dog object monitoring
                                                                            ///< the connected processes.
    sdv::CLifetimeCookie                    m_cookie = sdv::CreateLifetimeCookie(); ///< Lifetime cookie to manage module lifetime.
    CSharedMemBufferTx                      m_sender;                       ///< Shared buffer for sending.
    CSharedMemBufferRx                      m_receiver;                     ///< Shared buffer for receiving.
    std::thread                             m_threadReceive;                ///< Thread which receives data from the socket.
    std::atomic<sdv::ipc::EConnectStatus>   m_eStatus = sdv::ipc::EConnectStatus::uninitialized; ///< the status of the connection
    sdv::ipc::IDataReceiveCallback*         m_pReceiver = nullptr;          ///< Receiver to pass the messages to if available
    std::shared_mutex                       m_mtxEventCallbacks;            ///< Protect access to callback list. Only locking when
                                                                            ///< inserting.
    std::list<SEventCallback>               m_lstEventCallbacks;            ///< List containing event callbacks. New callbacks will
                                                                            ///< be inserted in front (called first). Removed
                                                                            ///< callbacks are NULL; the entry stays to allow
                                                                            ///< removal during a SetStatus call.
    mutable std::mutex                      m_mtxSend;                      ///< Synchronize all packages to be sent.
    std::mutex                              m_mtxConnect;                   ///< Connection mutex.
    std::condition_variable                 m_cvConnect;                    ///< Connection variable for connecting.
    std::condition_variable                 m_cvStartConnect;               ///< Start connection variable for connecting.
    bool                                    m_bStarted = false;             ///< When set, the reception thread has started.
    bool                                    m_bServer = false;              ///< When set, the connection is a server connection.
#if ENABLE_DECOUPLING > 0
    std::mutex                              m_mtxReceive;                   ///< Protect receive queue.
    std::queue<sdv::sequence<sdv::pointer<uint8_t>>> m_queueReceive;        ///< Receive queue to decouple receiving and processing.
    std::thread                             m_threadDecoupleReceive;        ///< Decoupled receive thread.
    std::condition_variable                 m_cvReceiveAvailable;           ///< Condition variable synchronizing the processing.
    std::condition_variable                 m_cvReceiveProcessed;           ///< Condition variable synchronizing the processing.
#endif
#ifdef TIME_TRACKING
    std::chrono::high_resolution_clock::time_point  m_tpLastSent{};         ///< Last time a fragment was sent.
    std::chrono::high_resolution_clock::time_point  m_tpLastReceived{};     ///< Last time a fragment was received.
    std::chrono::duration<double>           m_durationLargestDeltaReceived; ///< Largest duration
#endif

    /**
     * @brief Raw send function.
     * @param[in] pData to be send
     * @param[in] uiDataLength size of the data to be sent
     * @return Returns number of bytes which has been sent
     */
    uint32_t Send(const void* pData, uint32_t uiDataLength);

    /**
     * @brief Templated send implementation
     * @tparam T Type of data (structure) to send
     * @param[in] rt Reference to the data (structure).
     * @return Returns 'true' on successful sending; otherwise returns 'false'.
     */
    template <typename T>
    bool Send(const T& rt)
    {
        return Send(&rt, sizeof(rt)) == sizeof(rt);
    }

    /**
     * @brief Function to receive data, runs in a thread
     */
    void ReceiveMessages();

    /**
     * @brief Message context structure used when receiving data.
     */
    class CMessage : public CAccessorRxPacket
    {
    public:
        /**
         * @brief Constructor moving the packet content into the message.
         * @param[in] rPacket Reference to the packet to assign.
         */
        CMessage(CAccessorRxPacket&& rPacket);

        /**
         * @brief Destructor accepting the packet if not previously rejected by calling Reset.
         */
        ~CMessage();

        /**
         * @brief Returns whether the message is valid (has at least the size of the required header).
         * @return
         */
        bool IsValid() const;

        /**
         * @brief Get the message header if the data is at least the size of the header.
         * @return The message header or an empty header.
         */
        SMsgHdr GetMsgHdr() const;

        /**
         * @brief Get the connect header if the data is at least the size of the header and has type connect header.
         * @return The connect header or an empty header.
         */
        SConnectMsg GetConnectHdr() const;

        /**
         * @brief Get the fragmented message header. if the data is at least the size of the header and has type fragmented header.
         * @return The fragmented message header or an empty header.
        */
        SFragmentedMsgHdr GetFragmentedHdr() const;

        //// The various headers have the SMsgHdr in common.
        //union
        //{
        //    SMsgHdr                 sMsgHdr;        ///< Current message header
        //    SConnectMsg             sConnectHdr;    ///< Connect header
        //    SFragmentedMsgHdr       sFragmentHdr;   ///< Fragment header
        //    uint8_t                 rgData[std::max(sizeof(sConnectHdr), sizeof(sFragmentHdr))];
        //};
        //uint32_t                uiSize = 0;     ///< Complete size of the message (incl. size of the header)
        //uint32_t                uiOffset = 0;   ///< Current read offset within the message. Complete message when offset == size.

        /**
         * @brief Trace the protocol data (dependent on ENABLE_REPORTING setting).
         * @param[in] rConnection Reference to the connection class containing the connection information.
         */
        void PrintHeader(const CConnection& rConnection) const;
    };

    /**
     * @brief Data context structure
     */
    struct SDataContext
    {
        uint32_t        uiTotalSize = 0;                    ///< The total data size among all messages (without message header).
        uint32_t        uiCurrentOffset = 0;                ///< The current offset within the complete fragmented data to be filled during the read process.
        size_t          nChunkIndex = 0;                    ///< The current chunk index that is to be filled during the read process.
        uint32_t        uiChunkOffset = 0;                  ///< The offset within the current chunk of data to be filled during the read process.
        sdv::sequence<sdv::pointer<uint8_t>> seqDataChunks; ///< The data chunks allocated during table reading and available after uiCurrentOffset is identical to uiTotalSize.
    };

    /**
     * @brief Read the data size table (amount of data buffers followed by the size of each buffer).
     * @param[in] rMessage Reference to the message containing the table.
     * @param[in] rsDataCtxt Data context structure to be initialized - buffers will be allocated.
     * @return Returns the current offset of the data within the buffer following the table or 0 if the table could not be read.
     */
    uint32_t ReadDataTable(CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Read the data chunk to the buffers created by the ReadDataTable function. Subsequent calls can be made to this
     * function to fill the buffers. The last call (when the chunk index passes the last index in the table) the data will be
     * dispatched.
     * @param[in] rMessage Reference to the message containing the table.
     * @param[in] uiOffset The offset within the message data to start reading the data chunk.
     * @param[in] rsDataCtxt Data context structure to be filled.
     * @return Returns 'true' if the table could be read successfully; false if not.
     */
    bool ReadDataChunk(CMessage& rMessage, uint32_t uiOffset, SDataContext& rsDataCtxt);

#if ENABLE_DECOUPLING > 0
    /**
     * @brief Decoupled receive data. Prevents blocking the receive buffer while processing.
     */
    void DecoupleReceive();
#endif

    /**
     * @brief Received a synchronization request.
     * @param[in] rMessage Reference to the message containing the request.
     */
    void ReceiveSyncRequest(const CMessage& rMessage);

    /**
     * @brief Received a connection request.
     * @param[in] rMessage Reference to the message containing the request.
     */
    void ReceiveConnectRequest(const CMessage& rMessage);

    /**
     * @brief Received a synchronization answer.
     * @param[in] rMessage Reference to the message containing the answer.
     */
    void ReceiveSyncAnswer(const CMessage& rMessage);

    /**
     * @brief Received a connection answer.
     * @param[in] rMessage Reference to the message containing the answer.
     */
    void ReceiveConnectAnswer(const CMessage& rMessage);

    /**
     * @brief Received a connection termination information.
     * @param[in] rMessage Reference to the message containing the information.
     */
    void ReceiveConnectTerm(CMessage& rMessage);

    /**
     * @brief Received data message.
     * @param[in] rMessage Reference to the message containing the information.
     * @param[in] rsDataCtxt Reference to the data message context.
     */
    void ReceiveDataMessage(CMessage& rMessage, SDataContext& rsDataCtxt);

    /**
     * @brief Received data fragment message.
     * @param[in] rMessage Reference to the message containing the information.
     * @param[in] rsDataCtxt Reference to the data message context.
     */
    void ReceiveDataFragementMessage(CMessage& rMessage, SDataContext& rsDataCtxt);
};

#endif // !define CHANNEL_H
