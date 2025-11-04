#ifndef MARSHALL_OBJECT_H
#define MARSHALL_OBJECT_H

#include <interfaces/ipc.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>
#include <interfaces/core_ps.h>

// Forward declarations
class CCommunicationControl;
class CChannelConnector;

inline sdv::process::TProcessID GetProcessID()
{
    static sdv::process::TProcessID tProcessID = 0;
    if (!tProcessID)
    {
        const sdv::process::IProcessInfo* pProcessInfo = sdv::core::GetObject<sdv::process::IProcessInfo>("ProcessControlService");
        if (!pProcessInfo) return 0;
        tProcessID = pProcessInfo->GetProcessID();
    }
    return tProcessID;
}

/**
 * @brief Storage class for a proxy or a stub object.
 */
class CMarshallObject : public sdv::ps::IMarshall
{
public:
    /**
     * @brief Constructor
     * @param[in] rcontrol Reference to the communication control class.
     */
    CMarshallObject(CCommunicationControl& rcontrol);

    /**
     * @brief Destructor
     */
    ~CMarshallObject();

    /**
     * @brief Is this a valid marshal object?
     * @return Returns whether the marshal object is valid.
     */
    bool IsValid() const;

    /**
     * @brief Reset the marshal object.
     */
    void Reset();

    /**
     * @brief Initialize the marshall object as proxy.
     * @param[in] uiProxyIndex The index of this proxy; becoming part of the Proxy ID.
     * @param[in] id The ID of the interface this object marshalls the calls for.
     * @param[in] tStubID The stub ID this proxy is communicating to.
     * @param[in] rConnector Reference to channel connector.
     * @return Returns a pointer to proxy interface or empty when the initialization failed.
     */
    sdv::interface_t InitializeAsProxy(uint32_t uiProxyIndex, sdv::interface_id id, sdv::ps::TMarshallID tStubID,
        CChannelConnector& rConnector);

    /**
     * @brief Initialize the marshall object as stub.
     * @param[in] uiStubIndex The index of this stub; becoming part of the Stub ID.
     * @param[in] ifc Interface to the object to be marshalled to.
     * @return Returns 'true' when initialization was successful; 'false' when not.
     */
    bool InitializeAsStub(uint32_t uiStubIndex, sdv::interface_t ifc);

    /**
     * @brief Return the proxy/stub ID.
     * @details The marshall ID consist of an index to easily access the marshalling details and a control value to increase higher
     * security. The control value is a randomly generated value used in the communication to check whether the marshall object ID
     * is valid. Both index and control value must be known by the caller for the call to succeed. If one is wrong, the call won't
     * be made.
     * @return The ID of this marshall object.
     */
    sdv::ps::TMarshallID GetMarshallID() const;

    /**
     * @brief Return the proxy to the interface.
     * @return Proxy interface.
    */
    sdv::interface_t GetProxy();

    /**
     * @brief Marshall a function call. Ovverload of sdv::ps::IMarshall::Call.
     * @remarks This function call is synchronous and does not return until the call has been finalized or a timeout
     * exception has occurred.
     * @remarks The sequence contains all data to make the call. It is important that the data in the sequence is
     * complete and in the correct order.
     * @param[inout] seqInputData Reference to sequence of input data pointers. The first data pointer contains the
     * marshalling header. The second contains the parameters (if available) and the others contain raw data pointers
     * (if available). The call is allowed to change the sequence to be able to add additional information during the
     * communication without having to copy the existing data.
     * @return Sequence of output data pointers. The first data pointer contains the marshalling header. The second
     * contains the return value and parameters (if available) and the others contain raw data pointers (if available).
     */
    virtual sdv::sequence<sdv::pointer<uint8_t>> Call(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqInputData) override;

private:
    /// Marshall object type
    enum class EType {unknown, proxy, stub};

    CCommunicationControl&      m_rcontrol;                 ///< Reference to the communication control class.
    EType                       m_eType = EType::unknown;   ///< Type of object.
    sdv::ps::TMarshallID        m_tMarshallID = {};         ///< The ID of this marshall object.
    sdv::ps::IMarshall*         m_pMarshall = nullptr;      ///< The marshall object pointer (only used for a stub).
    sdv::TObjectPtr             m_ptrMarshallObject;        ///< The marshall object.
    sdv::ps::TMarshallID        m_tStubID = {};             ///< Stub ID (only used for a proxy).
    sdv::interface_t            m_ifcProxy = {};            ///< Proxy interface (only used for a proxy).
    CChannelConnector*          m_pConnector = nullptr;     ///< Pointer to the connector (only used for a proxy).
};

#endif // !defined MARSHALL_OBJECT_H