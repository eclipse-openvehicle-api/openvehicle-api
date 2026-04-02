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

#include "gtest/gtest.h"
#include <support/app_control.h>
#include <interfaces/ipc.h>
#include "../../../sdv_services/uds_win_tunnel/channel_mgnt.h"
#include "../../../sdv_services/uds_win_tunnel/connection.h"

// Include UDS Windows helpers for path generation
#include "../../../sdv_services/uds_win_sockets/channel_mgnt.h"   // ONLY FOR path helpers (MakeShortUdsPath)
#include "../../../sdv_services/uds_win_sockets/connection.h"

// Helper for unique UDS path
template<typename T=void>
static std::string UniqueUds(const char* prefix) {
    char buf[64];
    sprintf(buf, "%s_%08x.sock", prefix, rand());
    return std::string("%LOCALAPPDATA%/sdv/") + buf;
}

// Negative: invalid connect string
TEST(WinTunnelNegative, InvalidConnectString)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    // Missing proto, missing path
    auto obj = mgr.Access("role=server;");
    EXPECT_EQ(obj, nullptr);
    app.Shutdown();
}

// Negative: connect to non-existent server
TEST(WinTunnelNegative, ConnectToNonExistentServer)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    std::string cs = "proto=tunnel;path=" + UniqueUds("no_server") + ";";
    auto obj = mgr.Access(cs);
    if (!obj) {
        SUCCEED() << "Client object is nullptr as expected when server does not exist";
    } else {
        auto* client = obj->GetInterface<sdv::ipc::IConnect>();
        ASSERT_NE(client, nullptr);
        EXPECT_FALSE(client->WaitForConnection(200));
    }
    app.Shutdown();
}

// Edge: double disconnect
TEST(WinTunnelEdge, DoubleDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    std::string cs = "proto=tunnel;path=" + UniqueUds("double_disc") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto obj = mgr.Access(ep.ssConnectString);
    auto* conn = obj->GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(conn, nullptr);
    conn->Disconnect();
    // Should not crash or throw
    EXPECT_NO_THROW(conn->Disconnect());
    app.Shutdown();
}

// Edge: repeated connect/disconnect cycles (recreate endpoint each time)
TEST(WinTunnelEdge, RepeatedConnectDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    std::string cs = "proto=tunnel;path=" + UniqueUds("repeat") + ";";
    for (int i = 0; i < 3; ++i) {
        auto ep = mgr.CreateEndpoint(cs); // recreate endpoint every time
        sdv::TObjectPtr obj = mgr.Access(ep.ssConnectString);
        auto* conn = obj.GetInterface<sdv::ipc::IConnect>();
        ASSERT_NE(conn, nullptr);
        conn->AsyncConnect(nullptr);
        conn->WaitForConnection(200);
        conn->Disconnect();
        // obj goes out of scope and cleans up
    }
    app.Shutdown();
}

// Edge: simultaneous multiple clients
TEST(WinTunnelEdge, MultipleClients)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    std::string cs = "proto=tunnel;path=" + UniqueUds("multi_client") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto obj1 = mgr.Access(ep.ssConnectString);
    auto obj2 = mgr.Access(ep.ssConnectString);
    auto* c1 = obj1->GetInterface<sdv::ipc::IConnect>();
    auto* c2 = obj2->GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(c1, nullptr);
    ASSERT_NE(c2, nullptr);
    c1->AsyncConnect(nullptr);
    c2->AsyncConnect(nullptr);
    c1->WaitForConnection(500); 
    c2->WaitForConnection(500);
    c1->Disconnect();
    c2->Disconnect();
    app.Shutdown();  
}

// Edge: callback throws exception (should not crash)
class ThrowingReceiver : public sdv::IInterfaceAccess, public sdv::ipc::IDataReceiveCallback, public sdv::ipc::IConnectEventCallback {
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()
    void SetConnectState(sdv::ipc::EConnectState) override { throw std::runtime_error("SetConnectState fail"); }
    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>&) override { throw std::runtime_error("ReceiveData fail"); }
};

TEST(WinTunnelEdge, CallbackThrows) 
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);
    std::string cs = "proto=tunnel;path=" + UniqueUds("cb_throw") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto obj = mgr.Access(ep.ssConnectString);
    auto* conn = obj->GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(conn, nullptr);
    ThrowingReceiver rcv;
    // Should not crash even if callback throws
    EXPECT_NO_THROW(conn->AsyncConnect(&rcv));
    conn->Disconnect();
    app.Shutdown();
}
