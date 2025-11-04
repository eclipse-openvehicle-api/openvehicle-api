#ifndef COM_CHANNEL_H
#define COM_CHANNEL_H

#include <support/pssup.h>
#include <interfaces/ipc.h>
#include "../../global/scheduler/scheduler.h"

// Forward declaration
class CCommunicationControl;
class CMarshallObject;

/**
 * @brief Communication channel connector (endpoint).
 */
class CChannelConnector : public sdv::IInterfaceAccess, public sdv::ipc::IConnectEventCallback,
    public sdv::ipc::IDataReceiveCallback
{
public:
    /**
     * @brief Constructor for establishing the server connection.
     * @param[in] rcontrol Reference to the communication control class.
     * @param[in] uiIndex The current index of this connection (used to create the connection ID).
     * @param[in] pChannelEndpoint Interface pointer to the channel.
     */
    CChannelConnector(CCommunicationControl& rcontrol, uint32_t uiIndex, sdv::IInterfaceAccess* pChannelEndpoint);

    /**
     * @brief Destructor
     */
    ~CChannelConnector();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Connect as server (attaching the channel to the target object using a stub).
     * @param[in] pObject Pointer to the object to attach.
     * @param[in] bAllowReconnect Allow a disconnect and re-connect (disconnect doesn't trigger a channel removal).
     * @return Returns 'true' when the attachment succeeds; 'false' when not.
     */
    bool ServerConnect(sdv::IInterfaceAccess* pObject, bool bAllowReconnect);

    /**
     * @brief Connect as client (connecting to an existing server and creating a proxy).
     * @param[in] uiTimeoutMs The timeout time trying to connect.
     * @return Returns a pointer to the proxy object representing the object at the other end of the channel. Or NULL when a
     * timeout occurred.
     */
    sdv::IInterfaceAccess* ClientConnect(uint32_t uiTimeoutMs);

    /**
     * @brief Is the communication channel currently connected?
     * @return Returns whether the connector has an active connection.
     */
    bool IsConnected() const;

    /**
     * @brief Set the current status. Overload of sdv::ipc::IConnectEventCallback::SetStatus.
     * @param[in] eConnectStatus The connection status.
     */
    virtual void SetStatus(/*in*/ sdv::ipc::EConnectStatus eConnectStatus) override;

    /**
     * @brief Callback to be called by the IPC connection when receiving a data packet. Overload of
     * sdv::ipc::IDataReceiveCallback::ReceiveData.
     * @param[inout] seqData Sequence of data buffers to received. The sequence might be changed to optimize the communication
     * without having to copy the data.
     */
    virtual void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    /**
     * @brief Decoupled receive callback to be called by the scheduler when receiving a data packet.
     * @param[inout] seqData Sequence of data buffers to received. The sequence might be changed to optimize the communication
     * without having to copy the data.
     */
    void DecoupledReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData);

    /**
    * @brief Sends data consisting of multiple data chunks via the IPC connection.
    * @param[in] tProxyID Marshall ID of the proxy (source).
    * @param[in] tStubID Marshall ID of the stub (target).
    * @param[in] rseqInputData Sequence of data buffers to be sent. May be altered during processing to add/change the sequence content
    * without having to copy the data.
    * @return Returns the results of the call or throws a marshall exception.
    */
    sdv::sequence<sdv::pointer<uint8_t>> MakeCall(sdv::ps::TMarshallID tProxyID, sdv::ps::TMarshallID tStubID,
        sdv::sequence<sdv::pointer<uint8_t>>& rseqInputData);

    /**
    * @brief Get a proxy for the interface connection to the stub.
    * @param[in] id The ID of the interface this object marshalls the calls for.
    * @param[in] tStubID The stub ID this proxy is communicating to.
    * @return Returns a shared pointer to the proxy object.
    */
    std::shared_ptr<CMarshallObject> GetOrCreateProxy(sdv::interface_id id, sdv::ps::TMarshallID tStubID);

    /**
     * @brief Get the connection ID of this connector.
     * @return The connection ID.
     */
    sdv::com::TConnectionID GetConnectionID() const;

private:
    /**
     * @brief Endpoint type the channel connector
     */
    enum class EEndpointType {server, client};

    /**
     * @brief Call entry structure that is defined for a call to wait for the result.
     */
    struct SCallEntry
    {
        sdv::sequence<sdv::pointer<uint8_t>>    seqResult;          ///< The result data.
        std::mutex                              mtxWaitForResult;   ///< Mutex to protect result processing.
        std::condition_variable                 cvWaitForResult;    ///< Condition variable to trigger result processing.
        bool                                    bCancel = false;    ///< Cancel processing when set.
    };

    CCommunicationControl&              m_rcontrol;                     ///< Reference to the communication control class.
    sdv::TObjectPtr                     m_ptrChannelEndpoint;           ///< Managed pointer to the channel endpoint.
    uint64_t                            m_uiConnectionStatusCookie = 0; ///< Connection status cookie (received after registration).
    std::shared_ptr<CMarshallObject>    m_ptrInitialMarshallObject;     ///< Initial marshall object used after a connect event.
    sdv::ipc::EConnectStatus            m_eConnectStatus = sdv::ipc::EConnectStatus::uninitialized;     ///< Current connection status.
    bool                                m_bAllowReconnect = false;      ///< When set, allow reconnection of the server.
    EEndpointType                       m_eEndpointType = EEndpointType::client;    ///< Endpoint type of this connector.
    std::recursive_mutex                m_mtxMarshallObjects;           ///< Synchronize access to the marshall object vector.
    std::map<sdv::ps::TMarshallID, std::shared_ptr<CMarshallObject>> m_mapProxyObjects;    ///< Map of stub IDs to proxy objects
    sdv::com::TConnectionID             m_tConnectionID{};              ///< Connection ID for this connector.
    sdv::ipc::IDataSend*                m_pDataSend = nullptr;          ///< Pointer to the send interface.
    std::mutex                          m_mtxCalls;                     ///< Call map protection.
    std::map<uint64_t, SCallEntry&>     m_mapCalls;                     ///< call map.
    CTaskScheduler                      m_scheduler;                    ///< Scheduler to process incoming calls.
};

#endif // !defined COM_CHANNEL_H