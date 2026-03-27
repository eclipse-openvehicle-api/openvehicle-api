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

CChannelBroker::CChannelBroker(CListener& rListener) : m_rListener(rListener)
{}

sdv::u8string CChannelBroker::RequestChannel(/*in*/ const sdv::u8string& /*ssConfig*/)
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
    sdv::ipc::ICreateEndpoint* pEndpoint = nullptr;
    if (m_rListener.IsLocalListener())
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
    else
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("RemoteChannelControl");
    if (!pEndpoint)
    {
        SDV_LOG_ERROR("No local channel control!");
        return {};
    }

    // Create the endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(sdv::u8string());
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for channel request!");
        return sdv::u8string();
    }
    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

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

    sdv::ipc::ICreateEndpoint* pEndpoint = nullptr;
    std::string ssConfig;
    if (m_ssType == "Local")
    {
        m_bLocalListener = true;
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("LocalChannelControl");
        if (!pEndpoint)
        {
            SDV_LOG_ERROR("No local channel control!");
            return false;
        }

        // Request the instance ID from the app control
        ssConfig = std::string(R"code([IpcChannel]
Name = "LISTENER_)code") + std::to_string(m_uiInstanceID ? m_uiInstanceID : pContext->GetInstanceID()) + R"code("
Size = 2048
)code";
    }
    else if (m_ssType == "Remote")
    {
        m_bLocalListener = false;
        if (m_ssInterface.empty() || !m_uiPort)
        {
            SDV_LOG_ERROR("Missing interface or port number to initialize a remote listener!");
            return false;
        }
        pEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("RemoteChannelControl");
        if (!pEndpoint)
        {
            SDV_LOG_ERROR("No remote channel control!");
            return false;
        }

        ssConfig = R"code([IpcChannel]
Interface = ")code" + m_ssInterface + R"code(
Port = ")code" + std::to_string(m_uiPort) + R"code(
)code";
    }
    else
    {
        SDV_LOG_ERROR("Invalid or missing listener configuration for listener service!");
        return false;
    }

    // Create the endpoint
    sdv::ipc::SChannelEndpoint sEndpoint = pEndpoint->CreateEndpoint(ssConfig);
    if (!sEndpoint.pConnection)
    {
        SDV_LOG_ERROR("Could not create the endpoint for listener service!");
        return false;
    }
    sdv::TObjectPtr ptrEndpoint(sEndpoint.pConnection); // Does automatic destruction if failure happens.

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

bool CListener::IsLocalListener() const
{
    return m_bLocalListener;
}


