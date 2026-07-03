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

#include "../../include/gtest_custom.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/pssup.h>
#include <support/interface_ptr.h>
#include "../../../sdv_services/ipc_connect/listener.h"
#include "../../../sdv_services/ipc_connect/client.h"

TEST(IPC_Connect_Test_Shared_Mem, InstantiateListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"
Instance = 1234

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemory", {}, {}), 0);

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = R"toml(
[Provider]
Name = "DefaultSharedMemory"

[IpcChannel]
Name = "MyPersonalChannelName"
)toml";

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);
    listener.Initialize(sListenerInfo);
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);

    EXPECT_EQ(listener.GetProviderName(), "DefaultSharedMemory");

    // Shutdown
    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_Shared_Mem, InstantiateClientNoListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = R"toml(
[Provider]
Name = "DefaultSharedMemory"

[IpcChannel]
Name = "MyPersonalChannelName"
)toml";

    // Start client
    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);
    client.Initialize(sClientInfo);
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_FALSE(client.Connect());

    // Shutdown
    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_Shared_Mem, InstantiateClientAndListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = R"toml([Provider]
Name = "DefaultSharedMemory"
[IpcChannel]
Name = "CHANNEL_1234"
)toml";

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);
    listener.Initialize(sListenerInfo);
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = R"toml([Provider]
Name = "DefaultSharedMemory"
[IpcChannel]
Name = "CHANNEL_1234"
)toml";

    // Start client
    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);
    client.Initialize(sClientInfo);
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_TRUE(client.Connect());
    EXPECT_TRUE(client.IsConnected());

    // The client is a pointer to the repository. Request the module control service
    sdv::core::IObjectAccess* pObjectAccess =
        sdv::TInterfaceAccessPtr(client.GetRemoteRepository()).GetInterface<sdv::core::IObjectAccess>();
    EXPECT_NE(pObjectAccess, nullptr);
    sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
        pModuleInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService")).GetInterface<sdv::core::IModuleInfo>();
    EXPECT_NE(pModuleInfo, nullptr);
    if (pModuleInfo)
    {
        EXPECT_FALSE(pModuleInfo->GetModuleList().empty());
    }
    EXPECT_TRUE(client.Disconnect());

    // Shutdown
    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);
    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}

