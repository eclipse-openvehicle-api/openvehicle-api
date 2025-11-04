#ifndef LISTENER_H
#define LISTENER_H

#include <support/component_impl.h>
#include <interfaces/com.h>
#include <interfaces/ipc.h>

// Forward declarations
class CListener;

/**
 * @brief Channel broker to request new channels. This object is exposed to the client.
 */
class CChannelBroker : public sdv::IInterfaceAccess, public sdv::com::IRequestChannel
{
public:
    /**
     * @brief Constructor
     * @param[in] rListener Reference to the listener to forward the calls to.
     */
    CChannelBroker(CListener& rListener);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::com::IRequestChannel)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Request a channel. Overload of sdv::com::IRequestChannel::RequestChannel
     * @details This function creates a new endpoint and returns access to the repository through the new channel.
     * @param[in] ssConfig Configuration; currently not used.
     * @return The channel string needed to initialize the channel.
     */
    virtual sdv::u8string RequestChannel(/*in*/ const sdv::u8string& ssConfig) override;

private:
    CListener& m_rListener;         ///< Reference to the listener to forward the calls to.
};

/**
 * @brief Listener object
 */
class CListener : public sdv::CSdvObject, public sdv::IObjectControl
{
public:
    /**
     * @brief Default constructor
     */
    CListener();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    // Object declaration
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("ConnectionListenerService")

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @details The object configuration contains the information needed to start the listener. The following configuration is
     * available for the local listener:
     * @code
     * [Listener]
     * Type = "Local"
     * Instance = 1000    # Normally not used; system instance ID is used automatically.
     * @endcode
     * And the following is available for a remote listener:
     * @code
     * [Listener]
     * Type = "Remote"
     * Interface = "127.0.0.1"
     * Port = 2000
     * @endcode
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief When set, the listener is configured to be a local listener. Otherwise the listerner is configured as remote listener.
     * @return Boolean set when local lostener.
     */
    bool IsLocalListener() const;

private:
    sdv::EObjectStatus          m_eObjectStatus = sdv::EObjectStatus::initialization_pending; ///< To update the object status when it changes.
    sdv::TObjectPtr             m_ptrConnection;            ///< The connection object.
    CChannelBroker              m_broker;                   ///< Channel broker, used to request new channels
    bool                        m_bLocalListener = true;    ///< When set, the listener is a local listener; otherwise a remote listener.
    sdv::com::TConnectionID     m_tConnection = {};         ///< Channel connection ID.
};

DEFINE_SDV_OBJECT(CListener)

#endif // ! defined LISTENER_H