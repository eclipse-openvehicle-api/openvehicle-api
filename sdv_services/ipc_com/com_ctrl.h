#ifndef COM_CTRL_H
#define COM_CTRL_H

#include <support/pssup.h>
#include <support/component_impl.h>
#include <interfaces/com.h>

// Forward declaration
class CChannelConnector;
class CMarshallObject;

/**
 * @brief Test object simulating an component isolation service implementation for channel implementation testing
 */
class CCommunicationControl : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::com::IConnectionControl,
    public sdv::ps::IMarshallAccess
{
public:
    /**
     * @brief Constructor.
     */
    CCommunicationControl();

    /**
     * @brief Destructor added to cleanly stop service in case process is stopped (without shutdown via repository service)
     */
    virtual ~CCommunicationControl() override;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ps::IMarshallAccess)
        SDV_INTERFACE_ENTRY(sdv::com::IConnectionControl)
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    // Component declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_SINGLETON()
    DECLARE_OBJECT_CLASS_NAME("CommunicationControl")

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    virtual void Shutdown() override;

    /**
     * @brief Create an IPC channel endpoint and use it for SDV communication. Overload of
     * sdv::com::IConnectionControl::CreateServerConnection.
     * @remarks The channel will be destroyed automatically when a timeout occurs (no initial connection took place within the
     * specified time).
     * @remarks The function doesn't wait until a connection has been made, but returns straight after the channel creation.
     * @param[in] eChannelType Type of channel to create. Must be local or remote.
     * @param[in] pObject Initial object to start the communication with.
     * @param[in] uiTimeoutMs Timeout for waiting for a connection.
     * @param[out] ssConnectionString String describing the connection details to connect to this channel.
     * @return Channel connection ID if successful or 0 if not.
     */
    virtual sdv::com::TConnectionID CreateServerConnection(/*in*/ sdv::com::EChannelType eChannelType,
        /*in*/ sdv::IInterfaceAccess* pObject, /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::u8string& ssConnectionString) override;

    /**
     * @brief Connect to an SDV channel (an IPC channel managed by the channel control and waiting for an initial connection.
     * Overload of sdv::com::IConnectionControl::CreateClientConnection.
     * @remarks The connection will be destroyed automatically when a timeout occurs (no initial connection took place within
     * the specified time).
     * @param[in] ssConnectionString The string describing the connection details.
     * @param[in] uiTimeoutMs Timeout for waiting for a connection.
     * @param[out] pProxy Pointer to the object representing the remote object (a proxy to the remote object). Or nullptr
     * when a timeout occurred.
     * @return Channel connection ID if successful or 0 if not.
     */
    virtual sdv::com::TConnectionID CreateClientConnection(/*in*/ const sdv::u8string& ssConnectionString,
        /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::IInterfaceAccess*& pProxy) override;

    /**
     * @brief Assign and take over an already initialized IPC channel server endpoint for use with SDV communication. Overload of
     * sdv::com::IConnectionControl::AssignServerEndpoint.
     * @remarks The channel will be destroyed automatically when a timeout occurs (no initial connection took place within the
     * specified time) unless the flag bAllowReconnect has been set.
     * @remarks The channel uses the interface sdv::IObjectLifetime to control the lifetime of the channel. If IObjectLifetime is
     * not available, IObjectDestroy will be used.
     * @remarks The function doesn't wait until a connection has been made, but returns straight after the assignment.
     * @param[in] pChannelEndpoint Pointer to the channel endpoint to be assigned.
     * @param[in] pObject Initial object to start the communication with.
     * @param[in] uiTimeoutMs Timeout for waiting for an initial connection. Not used when the bAllowReconnect flag has been set.
     * @param[in] bAllowReconnect When set, the channel is allowed to be disconnected and reconnected again.
     * @return Channel connection ID if successful or 0 if not.
     */
    virtual sdv::com::TConnectionID AssignServerEndpoint(/*in*/ sdv::IInterfaceAccess* pChannelEndpoint,
        /*in*/ sdv::IInterfaceAccess* pObject, /*in*/ uint32_t uiTimeoutMs, /*in*/ bool bAllowReconnect) override;

    /**
     * @brief Assign and take over an already initialized IPC channel client endpoint for use with SDV communication. Overload of
     * sdv::com::IConnectionControl::AssignClientEndpoint.
     * @remarks The connection will be destroyed automatically when a timeout occurs (no initial connection took place
     * within the specified time).
     * @remarks The channel uses the interface sdv::IObjectLifetime to control the lifetime of the channel. If
     * IObjectLifetime is not available, IObjectDestroy will be used.
     * @param[in] pChannelEndpoint Pointer to the channel endpoint to be assigned.
     * @param[in] uiTimeoutMs Timeout for waiting for an initial connection. Not used when the bAllowReconnect flag has
     * been set.
     * @param[out] pProxy Pointer to the object representing the remote object (a proxy to the remote object). Or nullptr
     * when a timeout occurred.
     * @return Channel connection ID if successful or 0 if not.
     */
    virtual sdv::com::TConnectionID AssignClientEndpoint(/*in*/ sdv::IInterfaceAccess* pChannelEndpoint,
        /*in*/ uint32_t uiTimeoutMs, /*out*/ sdv::IInterfaceAccess*& pProxy) override;

    /**
     * @brief Remove a connection with the provided connection ID. Overload of sdv::com::IConnectionControl::RemoveConnection.
     * @param[in] tConnectionID The connection ID of the connection to remove.
     */
    virtual void RemoveConnection(/*in*/ const sdv::com::TConnectionID& tConnectionID) override;

    /**
     * @brief Get a proxy for the interface connection to the stub. Overload of sdv::ps::IMarshallAcess::GetProxy.
     * @param[in] tStubID Reference to the ID of the stub to connect to.
     * @param[in] id The interface ID to get the proxy for.
     * @return Returns the interface to the proxy object.
     */
    virtual sdv::interface_t GetProxy(/*in*/ const sdv::ps::TMarshallID& tStubID, /*in*/ sdv::interface_id id) override;

    /**
     * @brief Get a stub for the interface with the supplied ID. Overload of sdv::ps::IMarshallAcess::GetStub.
     * @param[in] ifc The interface to get the stub for..
     * @return Returns the Stub ID that is assigned to the interface. Or an empty ID when no stub could be found.
     */
    virtual sdv::ps::TMarshallID GetStub(/*in*/ sdv::interface_t ifc) override;

    /**
    * @brief Create a proxy for the interface connection to the stub.
    * @remarks Unlike stubs, which are unique for the process they run in, proxy objects are unique within the channel they are
    * used - using the identification of the stub ID of the process the call. The proxy object is not stored here; just created.
    * @param[in] id The ID of the interface this object marshalls the calls for.
    * @param[in] tStubID The stub ID this proxy is communicating to.
    * @param[in] rConnector Reference to channel connector.
    * @return Returns a shared pointer to the proxy object.
    */
    std::shared_ptr<CMarshallObject> CreateProxy(sdv::interface_id id, sdv::ps::TMarshallID tStubID,
        CChannelConnector& rConnector);

    /**
     * @brief Get a stub for the interface with the supplied ID. Overload of sdv::ps::IMarshallAcess::GetStub.
     * @param[in] ifc The interface to get the stub for..
     * @return Returns the Stub ID that is assigned to the interface. Or an empty ID when no stub could be found.
     */
    std::shared_ptr<CMarshallObject> GetOrCreateStub(sdv::interface_t ifc);

    /**
     * @brief To identify the send and receive packets belonging to one call, the call is identified with a unique index, which
     * is created here.
     * @return The unique call index.
     */
    uint64_t CreateUniqueCallIndex();

    /**
     * @brief Set the channel connector context for the current thread. This is used to marshall interfaces over the same connector.
     * @param[in] pConnectorContext Pointer to the connector currently being used.
     */
    void SetConnectorContext(CChannelConnector* pConnectorContext);

    /**
     * @brief Call the stub function.
     * @remarks This function call is synchronous and does not return until the call has been finalized or a timeout
     * exception has occurred.
     * @remarks The sequence contains all data to make the call. It is important that the data in the sequence is
     * complete and in the correct order.
     * @param[in] tStubID ID of the stub to call.
     * @param[inout] seqInputData Reference to sequence of input data pointers. The first data pointer contains the
     * marshalling header. The second contains the parameters (if available) and the others contain raw data pointers
     * (if available). The call is allowed to change the sequence to be able to add additional information during the
     * communication without having to copy the existing data.
     * @return Sequence of output data pointers. The first data pointer contains the marshalling header. The second
     * contains the return value and parameters (if available) and the others contain raw data pointers (if available).
     */
    sdv::sequence<sdv::pointer<uint8_t>> CallStub(sdv::ps::TMarshallID tStubID, sdv::sequence<sdv::pointer<uint8_t>>& seqInputData);

private:
    sdv::EObjectStatus                              m_eObjectStatus = sdv::EObjectStatus::initialization_pending;   ///< Object status.
    std::mutex                                      m_mtxChannels;                  ///< Protect the channel map.
    std::vector<std::shared_ptr<CChannelConnector>> m_vecChannels;                  ///< Channel vector.
    std::vector<std::thread>                        m_vecInitialConnectMon;         ///< Initial connection monitor.
    std::recursive_mutex                            m_mtxObjects;                   ///< Protect object vectors.
    std::vector<std::weak_ptr<CMarshallObject>>     m_vecMarshallObjects;           ///< Vector with marshall objects; lifetime is handled by channel.
    std::map<sdv::interface_t, std::shared_ptr<CMarshallObject>> m_mapStubObjects;  ///< Map of interfaces to stub objects
    std::atomic_uint64_t                            m_uiCurrentCallCnt = 0;         ///< The current call count.
    thread_local static CChannelConnector*          m_pConnectorContext;            ///< The current connector; variable local to each thread.
};
DEFINE_SDV_OBJECT(CCommunicationControl)

#endif // !defined COM_CTRL_H