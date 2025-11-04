#ifndef CLIENT_H
#define CLIENT_H

#include <support/pssup.h>
#include <support/component_impl.h>
#include <interfaces/com.h>

// Forward declaration.
class CClient;

/**
 * @brief Class managing the connection and providing access to the server repository through a proxy.
*/
class CRepositoryProxy : public sdv::IInterfaceAccess, public sdv::IObjectDestroy
{
public:
    /**
     * @brief Constructor
     * @param[in] rClient Reference to the client class.
     * @param[in] tConnection The connection ID to the server.
     * @param[in] pRepositoryProxy Proxy to the server repository.
     */
    CRepositoryProxy(CClient& rClient, sdv::com::TConnectionID tConnection, sdv::IInterfaceAccess* pRepositoryProxy);

    /**
     * @brief Do not allow a copy constructor.
     * @param[in] rRepository Reference to the remote repository.
     */
    CRepositoryProxy(const CRepositoryProxy& rRepository) = delete;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrRepositoryProxy)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Copy assignment is not allowed.
     * @param[in] rRepository Reference to the remote repository.
     */
    CRepositoryProxy& operator=(const CRepositoryProxy& rRepository) = delete;

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
     */
    virtual void DestroyObject() override;

private:
    CClient&                        m_rClient;                  ///< Reference to the client object.
    sdv::com::TConnectionID         m_tConnection = {};         ///< Connection ID.
    sdv::TInterfaceAccessPtr        m_ptrRepositoryProxy;       ///< Smart pointer to the remote repository.
};

/**
 * @brief Client object
 */
class CClient : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::com::IClientConnect
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::com::IClientConnect)
    END_SDV_INTERFACE_MAP()

    // Object declaration
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("ConnectionService")
    DECLARE_OBJECT_SINGLETON()

    /**
    * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
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
     * @brief Connect to a remote system using the connection string to contact the system. Overload of
     * sdv::com::IClientConnect::Connect.
     * @remarks After a successful connection, the ConnectClient utility is not needed any more.
     * @param[in] ssConnectString Optional connection string to use for connection. If not provided, the connection will
     * automatically get the connection ID from the app-control service (default). The connection string for a local
     * connection can be of the form:
     * @code
     * [Client]
     * Type = "Local"
     * Instance = 1234  # Optional: only use when connecting to a system with a different instance ID.
     * @endcode
     * And the following can be used for a remote connection:
     * @code
     * [Client]
     * Type = "Remote"
     * Interface = "127.0.0.1"
     * Port = 2000
     * @endcode
     * @return Returns an interface to the repository of the remote system or a NULL pointer if not found.
     */
    virtual sdv::IInterfaceAccess* Connect(const sdv::u8string& ssConnectString) override;

    /**
     * @brief Disconnect and remove the remote repository object.
     * @param[in] tConnectionID The ID of the connection.
     */
    void Disconnect(sdv::com::TConnectionID tConnectionID);

private:
    sdv::EObjectStatus      m_eObjectStatus = sdv::EObjectStatus::initialization_pending; ///< Object status.
    std::mutex              m_mtxRepositoryProxies;                             ///< Protect access to the remnote repository map.
    std::map<sdv::com::TConnectionID, CRepositoryProxy> m_mapRepositoryProxies; ///< map of remote repositories.
};

DEFINE_SDV_OBJECT(CClient)


#endif // !defined CLIENT_H