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
 *   Denisa Ros - initial API and implementation
 ********************************************************************************/

#include "../../include/gtest_custom.h"

#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/pssup.h>
#include <support/interface_ptr.h>

#include <chrono>
#include <string>
#include <thread>

#include "../../../sdv_services/ipc_connect/listener.h"
#include "../../../sdv_services/ipc_connect/client.h"

namespace
{

const char* GetUdsModuleName()
{
#ifdef _WIN32
    return "uds_win_sockets.sdv";
#else
    return "uds_unix_sockets.sdv";
#endif
}

const char* GetUdsChannelControlClassName()
{
#ifdef _WIN32
    return "WinSocketsChannelControl";
#else
    return "UnixSocketsChannelControl";
#endif
}

std::string UniqueSuffix()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::to_string(static_cast<long long>(now));
}

std::string MakeChannelName(const char* base)
{
    return std::string(base) + "_" + UniqueSuffix();
}

std::string MakeUdsConfigByName(const std::string& channelName)
{
    return std::string("[Provider]\n")
        + "Name = \"unix_domain_sockets\"\n\n"
        + "[IpcChannel]\n"
        + "Name = \"" + channelName + "\"\n";
}

void LoadRequiredModulesAndObjects(bool createUdsChannelControl = true)
{
    auto* pModuleControl =
        sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);

    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load(GetUdsModuleName()), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);

    auto* pRepositoryControl =
        sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);

    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);

    if (createUdsChannelControl)
    {
        EXPECT_NE(
            pRepositoryControl->CreateObject(GetUdsChannelControlClassName(), {}, {}),
            0);
    }
}

} // anonymous namespace

TEST(IPC_Connect_Test_UDS, InstantiateListener)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"
Instance = 1234

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    LoadRequiredModulesAndObjects(true);

    const std::string channelName = MakeChannelName("MyPersonalChannelName");

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = MakeUdsConfigByName(channelName);

    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);

    listener.Initialize(sListenerInfo);

    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_EQ(listener.GetProviderName(), "unix_domain_sockets");

    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_UDS, InstantiateClientNoListener)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    LoadRequiredModulesAndObjects(true);

    const std::string channelName = MakeChannelName("MyPersonalChannelName");

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = MakeUdsConfigByName(channelName);

    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);
    client.Initialize(sClientInfo);

    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialized);
    std::cout << "Connecting client..." << std::endl;
    EXPECT_FALSE(client.Connect());
    std::cout << "Client failed to connect." << std::endl;
    EXPECT_FALSE(client.IsConnected());

    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_UDS, InstantiateClientAndListener)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    LoadRequiredModulesAndObjects(true);

    const std::string channelName = MakeChannelName("CHANNEL_1234");

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = MakeUdsConfigByName(channelName);

    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);

    listener.Initialize(sListenerInfo);

    EXPECT_EQ(listener.GetProviderName(), "unix_domain_sockets");
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);

    // Small delay to avoid startup races between listener init and client connect.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = MakeUdsConfigByName(channelName);

    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);

    client.Initialize(sClientInfo);

    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_TRUE(client.Connect());
    EXPECT_TRUE(client.IsConnected());

    // The client should expose the remote repository after successful connect.
    sdv::core::IObjectAccess* pObjectAccess =
        sdv::TInterfaceAccessPtr(client.GetRemoteRepository())
            .GetInterface<sdv::core::IObjectAccess>();
    EXPECT_NE(pObjectAccess, nullptr);

    sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
    {
        pModuleInfo =
            sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService"))
                .GetInterface<sdv::core::IModuleInfo>();
    }

    EXPECT_NE(pModuleInfo, nullptr);
    if (pModuleInfo)
    {
        EXPECT_FALSE(pModuleInfo->GetModuleList().empty());
    }

    EXPECT_TRUE(client.Disconnect());
    std::cout << "Client Shutdown" << std::endl;
    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);
    std::cout << "Listener Shutdown" << std::endl;
    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}