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
class CClientConnect;

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
    CRepositoryProxy(CClientConnect& rClient, sdv::com::TConnectionID tConnection, sdv::IInterfaceAccess* pRepositoryProxy);

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

    /**
     * @brief Get the connection ID for this connection.
     * @return The connection ID.
     */
    sdv::com::TConnectionID GetConnectionID() const;

private:
    CClientConnect&                 m_rClient;                  ///< Reference to the client object.
    sdv::com::TConnectionID         m_tConnection = {};         ///< Connection ID.
    sdv::TInterfaceAccessPtr        m_ptrRepositoryProxy;       ///< Smart pointer to the remote repository.
};

/**
 * @brief Client object
 */
class CClientConnect : public sdv::CSdvObject, public sdv::com::IClientConnect
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::com::IClientConnect)
    END_SDV_INTERFACE_MAP()

    // Object declaration
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("ClientConnectService")
    DECLARE_OBJECT_DEPENDENCIES("CommunicationControl")

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_GROUP("Provider")
        SDV_PARAM_ENTRY(m_ssProvider, "Name", "", "", "Provider name to create a connection for.")
    END_SDV_PARAM_MAP()

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
     * @brief Connect to a remote system. Overload of IClientConnect::Connect.
     * @return Returns whether connect was successful.
     */
    virtual bool Connect() override;

    /**
     * @brief Disconnect from a connected system. Overload of IClientConnect::Disconnect.
     * @return Returns whether disconnect was successful.
     */
    bool Disconnect() override;

    /**
     * @brief State of the current connection. Overload of IClientConnect::IsConnected.
     * @return Returns whether an active connection exists.
     * @return The connect state.
     */
    bool IsConnected() const override;

    /**
     * @brief Get the remote repository that is available after connection. Overload of IClientConnect::GetRemoteRepository.
     * @remarks For main, isolated and external applications, the remote repository will be automatically linked to the
     * local repository. Hence a requests for the repository is not needed. For all other applications, access must be
     * explicitly acquired through this interface.
     * @return Interface to the remote repository if a successful connection is established. The interface is valid until
     * disconnect is called or the client connection service is terminated.
     */
    sdv::IInterfaceAccess* GetRemoteRepository() override;

private:
    mutable std::mutex                  m_mtx;                      ///< Protect against multiple parallel connection activities.
    sdv::u8string                       m_ssProvider;               ///< Name of the provider to use for the listening.
    std::shared_ptr<CRepositoryProxy>   m_ptrRemoteRepo;            ///< Interface to the remote repository.
};

DEFINE_SDV_OBJECT(CClientConnect)

#endif // !defined CLIENT_H