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

#include "listener.h"
#include <support/toml.h>
#include <interfaces/com.h>
#include <interfaces/app.h>
#include <support/pssup.h>
#include <support/local_service_access.h>

CChannelBroker::CChannelBroker(CListener& rListener) : m_rListener(rListener)
{}

sdv::u8string CChannelBroker::RequestChannel(/*in*/ const sdv::u8string& ssConfig)
{
    // Get the communication control
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        SDV_LOG_ERROR("Failed to get communication control!");
        return {};
    }

    // Get the repository
    sdv::TInterfaceAccessPtr ptrRespository = sdv::core::GetObject("RepositoryService");
    if (!ptrRespository)
    {
        SDV_LOG_ERROR("Failed to get repository service!");
        return {};
    }

    // Get the channel control.
    sdv::ipc::ICreateEndpoint* pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>(m_rListener.GetProviderName());
    if (!pEndpoint)
    {
        SDV_LOG_ERROR("No local channel control!");
        return {};
    }

    // Forward protocol-specific configuration to the provider when creating the private channel.
    SDV_LOG_INFO("[IPC_CONNECT][Listener] RequestChannel input config:\n", ssConfig);

    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(ssConfig);
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for channel request!");
        return {};
    }

    SDV_LOG_INFO("[IPC_CONNECT][Listener] RequestChannel produced connect string: ", sEndpoint.ssConnectString);

    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

    // Restrict access permissions
    sdv::core::CAccessPermission permission = sdv::core::RestrictAccessPermission(sdv::core::EAccessPermission::local_access);

    // Assign the endpoint to the communication service.
    sdv::com::TConnectionID tConnection = pConnectionControl->AssignServerEndpoint(ptrEndpoint, ptrRespository, 100, false);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.
    if (!tConnection)
    {
        SDV_LOG_ERROR("Could not assign the server endpoint!");
        return {};
    }

    if (sdv::app::ConsoleIsVerbose())
        std::cout << "Client connection established..." << std::endl;

    // Return the connection string
    return sEndpoint.ssConnectString;
}

CListener::CListener() : m_broker(*this)
{}

bool CListener::OnInitialize()
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
        SDV_LOG_ERROR("Missing provider name for creating a listener object!");
        return false;
    }
    sdv::ipc::ICreateEndpoint* pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>(m_ssProvider);
    if (!pEndpoint)
    {
        SDV_LOG_ERROR("Cannot instantiate provider '", m_ssProvider, "' for the creation of a listener object!");
        return false;
    }

    // Get the IpcChannel information from the the configuration
    auto ssConfig = BuildObjectConfig();

    // Create the endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(ssConfig);
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for listener service!");
        return false;
    }
    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

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
    pObjectDependency->AddObjectDependency(Self().ssName, m_ssProvider);

    // Assign the endpoint to the communication service.
    m_tConnection = pConnectionControl->AssignServerEndpoint(ptrEndpoint, &m_broker, 100, true);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.
    if (!m_tConnection)
    {
        SDV_LOG_ERROR("Could not assign the server endpoint!");
        return false;
    }
    return true;
}

void CListener::OnShutdown()
{
    // Shutdown the listener...
    if (m_tConnection != sdv::com::TConnectionID{})
    {
        sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
        if (!pConnectionControl)
            SDV_LOG_ERROR("Failed to get communication control; cannot shutdown gracefully!");
        else
            pConnectionControl->RemoveConnection(m_tConnection);
        m_tConnection = {};
    }

    m_ptrConnection.Clear();
}


const sdv::u8string& CListener::GetProviderName()
{
    return m_ssProvider;
}
