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

const char* GetTunnelModuleName()
{
#ifdef _WIN32
    return "uds_win_tunnel.sdv";
#else
    return "uds_unix_tunnel.sdv";
#endif
}

const char* GetTunnelChannelControlClassName()
{
#ifdef _WIN32
    return "WinTunnelChannelControl";
#else
    return "UnixTunnelChannelControl";
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

std::string MakeTunnelName(const char* base)
{
    return std::string("t_") + base + "_" + UniqueSuffix();
}

std::string MakeTunnelConfigByName(const std::string& channelName,
                                   const std::string& tunnelName)
{
    return std::string("[Provider]\n")
        + "Name = \"unix_domain_sockets_tunnel\"\n\n"
        + "[IpcChannel]\n"
        + "Name = \"" + channelName + "\"\n"
        + "Tunnel = \"" + tunnelName + "\"\n";
}

void LoadRequiredModulesAndObjects(bool createTunnelChannelControl = true)
{
    auto* pModuleControl =
        sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);

    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load(GetTunnelModuleName()), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);

    auto* pRepositoryControl =
        sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);

    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);

    if (createTunnelChannelControl)
    {
        EXPECT_NE(
            pRepositoryControl->CreateObject(GetTunnelChannelControlClassName(), {}, {}),
            0);
    }
}

} // anonymous namespace

TEST(IPC_Connect_Test_UDS_Tunnel, InstantiateListener)
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
    const std::string tunnelName  = MakeTunnelName("listener");

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = MakeTunnelConfigByName(channelName, tunnelName);

    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);

    listener.Initialize(sListenerInfo);

    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_EQ(listener.GetProviderName(), "unix_domain_sockets_tunnel");

    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_UDS_Tunnel, InstantiateClientNoListener)
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
    const std::string tunnelName  = MakeTunnelName("no_listener");

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = MakeTunnelConfigByName(channelName, tunnelName);

    CClientConnect client;
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialization_pending);

    client.Initialize(sClientInfo);

    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_FALSE(client.Connect());
    EXPECT_FALSE(client.IsConnected());

    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_UDS_Tunnel, InstantiateClientAndListener)
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
    const std::string tunnelName  = MakeTunnelName("client_listener");

    sdv::SObjectInfo sListenerInfo{};
    sListenerInfo.ssConfig = MakeTunnelConfigByName(channelName, tunnelName);

    CListener listener;
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialization_pending);

    listener.Initialize(sListenerInfo);

    EXPECT_EQ(listener.GetProviderName(), "unix_domain_sockets_tunnel");
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::initialized);

    // Allow listener endpoint publication to complete before client connect.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = MakeTunnelConfigByName(channelName, tunnelName);

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

    client.Shutdown();
    EXPECT_EQ(client.GetObjectState(), sdv::EObjectState::destruction_pending);

    listener.Shutdown();
    EXPECT_EQ(listener.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test_UDS_Tunnel, InvalidProvider_ShouldNotInitializeClient)
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

    sdv::SObjectInfo sClientInfo{};
    sClientInfo.ssConfig = R"toml(
[Provider]
Name = "invalid_tunnel_provider"

[IpcChannel]
Name = "CHANNEL_1234"
Tunnel = "t_invalid_provider"
)toml";

    CClientConnect client;
    client.Initialize(sClientInfo);

    EXPECT_NE(client.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_FALSE(client.Connect());
    EXPECT_FALSE(client.IsConnected());

    client.Shutdown();
    appcontrol.Shutdown();
}