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
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

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
class CClient : public sdv::CSdvObject, public sdv::com::IClientConnect
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::com::IClientConnect)
    END_SDV_INTERFACE_MAP()

    // Object declaration
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("ConnectionService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

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
    std::mutex              m_mtxRepositoryProxies;                             ///< Protect access to the remnote repository map.
    std::map<sdv::com::TConnectionID, CRepositoryProxy> m_mapRepositoryProxies; ///< map of remote repositories.
};

DEFINE_SDV_OBJECT(CClient)


#endif // !defined CLIENT_H