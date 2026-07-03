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

#include <support/app_control.h>
#include <support/pssup.h>
#include <support/sdv_core.h>

#include "../../../sdv_services/ipc_connect/client.h"
#include "../../../sdv_services/ipc_connect/listener.h"
#include "../../include/gtest_custom.h"

TEST(IPC_Connect_Test, InstantiateListenerWithUnknownProvider)
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
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemory", {}, {}), 0);

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);
    listener.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_failure);

    // Shutdown
    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_Shared_Mem, InstantiateClientWithoutConfiguration)
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
    
    // Start client without configuration
    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);
    client.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_failure);

    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}
