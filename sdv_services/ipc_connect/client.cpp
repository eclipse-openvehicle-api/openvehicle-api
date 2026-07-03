
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

#include "client.h"
#include <support/toml.h>
#include <interfaces/com.h>
#include <interfaces/ipc.h>
#include <support/pssup.h>
#include <interfaces/app.h>

CRepositoryProxy::CRepositoryProxy(CClientConnect& rClient, sdv::com::TConnectionID tConnection,
    sdv::IInterfaceAccess* pRepositoryProxy) :
    m_rClient(rClient), m_tConnection(tConnection), m_ptrRepositoryProxy(pRepositoryProxy)
{}

void CRepositoryProxy::DestroyObject()
{
    // Call the client to disconnect the connection and destroy the object.
    m_rClient.Disconnect();
}

sdv::com::TConnectionID CRepositoryProxy::GetConnectionID() const
{
    return m_tConnection;
}

bool CClientConnect::OnInitialize()
{
    const sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (!pContext)
    {
        SDV_LOG_ERROR("Failed to get application context!");
        return false;
    }
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return false;
    }

    // Check for a provider. Without provider there is no object to use for listening.
    if (m_ssProvider.empty())
    {
        SDV_LOG_ERROR("Missing provider name for creating a client object!");
        return false;
    }
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>(m_ssProvider);
    if (!pChannelAccess)
    {
        SDV_LOG_ERROR("Cannot instantiate provider '", m_ssProvider, "' for the creation of a client object!");
        return false;
    }

    // The connection will be established in the Connect function.
    return true;
}

void CClientConnect::OnShutdown()
{
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
        SDV_LOG_ERROR("Failed to get communication control!");

    // Disconnect
    Disconnect();
}

bool CClientConnect::Connect()
{
    sdv::u8string ssProvider;
    sdv::u8string ssConfig;

    {
        std::unique_lock<std::mutex> lock(m_mtx);
        ssProvider = m_ssProvider;
        ssConfig   = BuildObjectConfig();
    }

    const sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (!pContext)
    {
        SDV_LOG_ERROR("Failed to get application context!");
        return false;
    }
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return false;
    }

    // Check for a provider. Without provider there is no object to use for listening.
    if (ssProvider.empty())
    {
        SDV_LOG_ERROR("Missing provider name for creating a client object!");
        return false;
    }

    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>(ssProvider);
    if (!pChannelAccess)
    {
        SDV_LOG_ERROR("Cannot instantiate provider '", m_ssProvider, "' for the creation of a client object!");
        return false;
    }

    // Get the repository
    sdv::TInterfaceAccessPtr ptrRespository = sdv::core::GetObject("RepositoryService");
    if (!ptrRespository)
    {
        SDV_LOG_ERROR("Failed to get repository service!");
        return false;
    }

    // Add a dependency of the provider to this service (to prevent the provider to be terminated while the service is still
    // running).
    sdv::core::IObjectDependency* pObjectDependency = ptrRespository.GetInterface<sdv::core::IObjectDependency>();
    if (!pObjectDependency)
    {
        SDV_LOG_ERROR("Failed to get the object dependency interface!");
        return false;
    }

    pObjectDependency->AddObjectDependency(Self().ssName, ssProvider);


    // First access the listener channel. This allows us to access the channel creation interface.

    // TODO: Use named mutex to prevent multiple connections at the same time.
    // Connect to the channel.
    sdv::TObjectPtr ptrListenerEndpoint = pChannelAccess->Access(ssConfig);
    

    // Assign the endpoint to the communication service.
    sdv::IInterfaceAccess* pListenerProxy = nullptr;
    sdv::com::TConnectionID tListenerConnection =
        pConnectionControl->AssignClientEndpoint(ptrListenerEndpoint, 5000, pListenerProxy);
    ptrListenerEndpoint.Clear();    // Lifetime has been taken over by communication control.
    if (!tListenerConnection || !pListenerProxy)
    {
        SDV_LOG_ERROR("Could not assign the client endpoint!");
        if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);
        return false;
    }
    sdv::TInterfaceAccessPtr ptrListenerProxy(pListenerProxy);

    // Request for a private channel
    sdv::com::IRequestChannel* pRequestChannel = ptrListenerProxy.GetInterface<sdv::com::IRequestChannel>();
    if (!pRequestChannel)
    {
        SDV_LOG_ERROR("Could not get the channel creation interface!");
        if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);
        return false;
    }
    
    sdv::u8string ssRequestConfig;

    // Tunnel providers need the full object config so the private channel
    // can preserve provider-specific fields such as the tunnel name.
    // For shared memory we keep the legacy behavior and let the provider
    // create its own private channel details.
    if (ssProvider == "unix_domain_sockets_tunnel")
    {
        ssRequestConfig = ssConfig;
    }
    else
    {
        ssRequestConfig.clear();
    }

    sdv::u8string ssConnectionString = pRequestChannel->RequestChannel(ssRequestConfig);


    // Disconnect from the listener
    if (tListenerConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tListenerConnection);

    if (ssConnectionString.empty())
    {
        SDV_LOG_ERROR("Could not get the private channel connection information!");
        return false;
    }

    // TODO: Use named mutex to prevent multiple connections at the same time.
    // Connect to the privatechannel.
    sdv::TObjectPtr ptrPrivateEndpoint = pChannelAccess->Access(ssConnectionString);

    // Get and return the proxy
    sdv::IInterfaceAccess* pPrivateProxy = nullptr;
    sdv::com::TConnectionID tPrivateConnection = pConnectionControl->AssignClientEndpoint(ptrPrivateEndpoint, 5000, pPrivateProxy);
    ptrPrivateEndpoint.Clear();    // Lifetime has been taken over by communication control.
    if (!tPrivateConnection || !pPrivateProxy)
    {
        SDV_LOG_ERROR("Could not assign the client endpoint to the private channel!");
        if (tPrivateConnection != sdv::com::TConnectionID{}) pConnectionControl->RemoveConnection(tPrivateConnection);
        return false;
    }

    // Create a remote repository object
    m_ptrRemoteRepo = std::make_shared<CRepositoryProxy>(*this, tPrivateConnection, pPrivateProxy);

    return true;
}

bool CClientConnect::Disconnect()
{
    std::unique_lock<std::mutex> lock(m_mtx);

    if (!m_ptrRemoteRepo) return false;

    // Whatever happens, the connection will be removed
    std::shared_ptr<CRepositoryProxy> ptrRemoteRepoLocal = std::move(m_ptrRemoteRepo);

    // Disconnect
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return false;
    }
    pConnectionControl->RemoveConnection(ptrRemoteRepoLocal->GetConnectionID());

    // Get the repository
    sdv::TInterfaceAccessPtr ptrRespository = sdv::core::GetObject("RepositoryService");
    if (!ptrRespository)
    {
        SDV_LOG_ERROR("Failed to get repository service!");
        return false;
    }

    // Add a dependency of the provider to this service (to prevent the provider to be terminated while the service is still
    // running).
    sdv::core::IObjectDependency* pObjectDependency = ptrRespository.GetInterface<sdv::core::IObjectDependency>();
    if (!pObjectDependency)
    {
        SDV_LOG_ERROR("Failed to get the object dependency interface!");
        return false;
    }
    pObjectDependency->RemoveObjectDependency("ClientConnectService", m_ssProvider);

    return true;
}

bool CClientConnect::IsConnected() const
{
    std::unique_lock<std::mutex> lock(m_mtx);

    return m_ptrRemoteRepo ? true : false;
}

sdv::IInterfaceAccess* CClientConnect::GetRemoteRepository()
{
    std::unique_lock<std::mutex> lock(m_mtx);

    return m_ptrRemoteRepo ? m_ptrRemoteRepo.get() : nullptr;
}
