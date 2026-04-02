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

// Reuse helpers from uds_win_sockets (path normalization, MakeShortUdsPath)
#include "../../../sdv_services/uds_win_sockets/channel_mgnt.h"
#include "../../../sdv_services/uds_win_sockets/connection.h"

#include <atomic>
#include <chrono> 
#include <condition_variable>
#include <mutex>
#include <thread>
#include <random>
#include <sstream>
#include <cstring>
#include <windows.h>
#include <afunix.h>

// Helper namespace 
namespace tunnel_utils
{
inline std::string Expand(const std::string& in)
{
    if (in.find('%') == std::string::npos)
        return in;
    char buf[4096] = {};
    const DWORD n = ExpandEnvironmentStringsA(in.c_str(), buf, sizeof(buf));
    return (n > 0 && n < sizeof(buf)) ? std::string(buf) : in;
}

inline void EnsureParentDir(const std::string& full)
{
    auto p = full.find_last_of("\\/");
    if (p == std::string::npos)
        return;
    std::string dir = full.substr(0, p);
    CreateDirectoryA(dir.c_str(), nullptr);
}

inline std::string MakeShortUdsPath(const char* name)
{
    std::string base = R"(%LOCALAPPDATA%\sdv\)";
    base = Expand(base);
    EnsureParentDir(base);
    return base + name;
}

inline std::string RandomHex()
{
    std::mt19937_64 r{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> d;
    std::ostringstream oss;
    oss << std::hex << d(r);
    return oss.str();
}

inline std::string Unique(const char* prefix)
{
    return MakeShortUdsPath((std::string(prefix) + "_" + RandomHex() + ".sock").c_str());
}

inline void SpinUntilServerArmed(sdv::ipc::IConnect* server)
{
    using namespace std::chrono;
    const auto deadline = steady_clock::now() + milliseconds(500);
    while (server->GetConnectState() == sdv::ipc::EConnectState::uninitialized &&
           steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(milliseconds(2));
    }
}

} // namespace tunnel_utils

using namespace tunnel_utils;

// Unified receiver (state + data)
class CTunnelMgrTestReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IDataReceiveCallback,
    public sdv::ipc::IConnectEventCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    void SetConnectState(sdv::ipc::EConnectState s) override
    {
        { std::lock_guard<std::mutex> lk(m_mtx); m_state = s; }
        m_cv.notify_all();
    }

    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& seq) override
    {
        { std::lock_guard<std::mutex> lk(m_mtx);
          m_last = seq; m_has = true; }
        m_cv.notify_all();
    }

    bool WaitForState(sdv::ipc::EConnectState s, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                             [&]{ return m_state == s; });
    }

    bool WaitForData(uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                             [&]{ return m_has; });
    }

    sdv::sequence<sdv::pointer<uint8_t>> Data()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_last;
    }

private:
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    sdv::ipc::EConnectState m_state{sdv::ipc::EConnectState::uninitialized};
    sdv::sequence<sdv::pointer<uint8_t>> m_last;
    bool m_has{false};
};

// Helper to create server + client
struct TunnelPair
{
    sdv::TObjectPtr serverObj;
    sdv::ipc::IConnect* server = nullptr;

    sdv::TObjectPtr clientObj;
    sdv::ipc::IConnect* client = nullptr;
};

static TunnelPair CreateTunnelPair(CSocketsTunnelChannelMgnt& mgr,
                                   const std::string& cs)
{
    TunnelPair out;
    out.serverObj = mgr.Access(cs);
    out.server    = out.serverObj ?
        out.serverObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    out.clientObj = mgr.Access(cs);
    out.client    = out.clientObj ?
        out.clientObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    return out;
}

// TESTS
// Manager instantiate + lifecycle
TEST(WinTunnelChannelMgnt, InstantiateAndLifecycle)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    CSocketsTunnelChannelMgnt mgr;

    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    mgr.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::configuring);

    mgr.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    mgr.Shutdown();
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    app.Shutdown();
}

// Basic connect/disconnect using manager (server + client)
TEST(WinTunnelChannelMgnt, BasicConnectDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    app.SetRunningMode();
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string uds = Unique("tunnel_mgr_basic");
    const std::string cs = "proto=tunnel;path=" + uds + ";";

    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelMgrTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    EXPECT_TRUE(pair.server->WaitForConnection(5000));
    EXPECT_TRUE(pair.client->WaitForConnection(5000));

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Simple hello (header stripped)
TEST(WinTunnelChannelMgnt, DataPath_SimpleHello_ViaManager)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = "proto=tunnel;path=" + Unique("hello_mgr") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelMgrTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    sdv::pointer<uint8_t> msg;
    msg.resize(5);
    memcpy(msg.get(), "hello", 5);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(msg);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    EXPECT_TRUE(sender->SendData(seq));
    ASSERT_TRUE(sr.WaitForData(3000));

    auto recv = sr.Data();
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(memcmp(recv[0].get(), "hello", 5), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Multi-chunk
TEST(WinTunnelChannelMgnt, DataPath_MultiChunk_ViaManager)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = "proto=tunnel;path=" + Unique("mc_mgr") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelMgrTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);
    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    sdv::pointer<uint8_t> a, b;
    a.resize(3);
    memcpy(a.get(), "sdv", 3);
    b.resize(9);
    memcpy(b.get(), "framework", 9);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(a);
    seq.push_back(b);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    EXPECT_TRUE(sender->SendData(seq));
    ASSERT_TRUE(sr.WaitForData(3000));

    auto recv = sr.Data();
    ASSERT_EQ(recv.size(), 2u);
    EXPECT_EQ(memcmp(recv[0].get(), "sdv", 3), 0);
    EXPECT_EQ(memcmp(recv[1].get(), "framework", 9), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();
    mgr.Shutdown();
    app.Shutdown();
}

// Header stripping invariant
TEST(WinTunnelChannelMgnt, HeaderStrippedInvariant)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs ="proto=tunnel;path=" + Unique("hdr_mgr") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelMgrTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    const char* msg = "HDR_TEST";
    const size_t len = strlen(msg);

    sdv::pointer<uint8_t> buf;
    buf.resize(len);
    memcpy(buf.get(), msg, len);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(buf);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    EXPECT_TRUE(sender->SendData(seq));
    ASSERT_TRUE(sr.WaitForData(3000));

    auto recv = sr.Data();
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), len);
    EXPECT_EQ(memcmp(recv[0].get(), msg, len), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();
    mgr.Shutdown();
    app.Shutdown();
}

// Long path normalization
TEST(WinTunnelChannelMgnt, CreateEndpoint_LongPath_Normalized)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    std::string veryLong(200, 'A');
    const std::string raw ="C:\\Users\\" + veryLong + "\\AppData\\Local\\sdv\\tunnel_long_" + RandomHex() + ".sock";
    const std::string cs = "proto=tunnel;path=" + raw + ";";

    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    // path must be normalized (basename only)
    EXPECT_NE(ep.ssConnectString.find("path=tunnel_long_"), std::string::npos);

    sdv::TObjectPtr obj = mgr.Access(ep.ssConnectString);
    auto* server = obj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    CTunnelMgrTestReceiver sr;
    server->AsyncConnect(&sr);
    SpinUntilServerArmed(server);

    // boot client
    sdv::TObjectPtr cobj = mgr.Access(ep.ssConnectString);
    auto* client = cobj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    CTunnelMgrTestReceiver cr;
    client->AsyncConnect(&cr);

    EXPECT_TRUE(server->WaitForConnection(5000));
    EXPECT_TRUE(client->WaitForConnection(5000));

    client->Disconnect();
    server->Disconnect();
    mgr.Shutdown();
    app.Shutdown(); 
}