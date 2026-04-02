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

// Include transport and tunnel manager
#include "../../../sdv_services/uds_win_tunnel/channel_mgnt.h"
#include "../../../sdv_services/uds_win_tunnel/connection.h"

// Include UDS Windows helpers for path generation
#include "../../../sdv_services/uds_win_sockets/channel_mgnt.h"   // ONLY FOR path helpers (MakeShortUdsPath)
#include "../../../sdv_services/uds_win_sockets/connection.h"

#include <windows.h>
#include <afunix.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring> 
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>

// Utility namespace from original tests
namespace test_utils
{
inline std::string Expand(const std::string& in)
{
    if (in.find('%') == std::string::npos)
        return in;
    char buf[4096] = {};
    DWORD n = ExpandEnvironmentStringsA(in.c_str(), buf, sizeof(buf));
    return (n > 0 && n < sizeof(buf)) ? std::string(buf) : in;
}

inline void EnsureParentDir(const std::string& fullPath)
{
    auto pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos)
        return;
    std::string dir = fullPath.substr(0, pos);
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
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}

inline std::string UniqueUds(const char* prefix)
{
    return MakeShortUdsPath((std::string(prefix) + "_" + RandomHex() + ".sock").c_str());
}

inline void SpinUntilServerArmed(sdv::ipc::IConnect* server, uint32_t maxWaitMs = 300)
{
    using namespace std::chrono;
    auto deadline = steady_clock::now() + milliseconds(maxWaitMs);
    while (server->GetConnectState() == sdv::ipc::EConnectState::uninitialized &&
           steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

inline void SleepTiny(uint32_t ms = 20)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace test_utils

using namespace test_utils;

// Unified test receiver (status + payload), identical to UDS tests
class CTunnelTestReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IConnectEventCallback,
    public sdv::ipc::IDataReceiveCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    void SetConnectState(sdv::ipc::EConnectState s) override
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_state = s;
        }
        m_cv.notify_all();
    }

    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& seq) override
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_data = seq;
            m_hasData = true;
        }
        m_cv.notify_all();
    }

    bool WaitForStatus(sdv::ipc::EConnectState expected, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                             [&]{ return m_state == expected; });
    }

    bool WaitForData(uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                             [&]{ return m_hasData; });
    }

    sdv::sequence<sdv::pointer<uint8_t>> Data()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_data;
    }

private:
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    sdv::ipc::EConnectState m_state{sdv::ipc::EConnectState::uninitialized};
    sdv::sequence<sdv::pointer<uint8_t>> m_data;
    bool m_hasData{false};
};

// Helper to create server + client via manager
struct TunnelPair
{
    sdv::TObjectPtr serverObj;
    sdv::ipc::IConnect* server = nullptr;

    sdv::TObjectPtr clientObj;
    sdv::ipc::IConnect* client = nullptr;
};

static TunnelPair CreateTunnelPair(CSocketsTunnelChannelMgnt& mgr, const std::string& cs)
{
    TunnelPair out;

    out.serverObj = mgr.Access(cs);
    out.server = out.serverObj ? out.serverObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    out.clientObj = mgr.Access(cs);
    out.client = out.clientObj ? out.clientObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    return out;
}

// TEST SUITE START
// Instantiate manager
TEST(WinTunnelIPC, InstantiateManager)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    CSocketsTunnelChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    app.Shutdown();
}

// Basic connect/disconnect
TEST(WinTunnelIPC, BasicConnectDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    app.SetRunningMode();
    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string path = UniqueUds("tunnel_basic");
    const std::string cs = "proto=tunnel;path=" + path + ";";

    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelTestReceiver sr, cr;
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

// Simple data (hello)
TEST(WinTunnelIPC, DataPath_SimpleHello_Tunnel)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = "proto=tunnel;path=" + UniqueUds("hello_t") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);
    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    // Build payload = "hello"
    sdv::pointer<uint8_t> p;
    p.resize(5);
    memcpy(p.get(), "hello", 5);
    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);
    EXPECT_TRUE(sender->SendData(seq));

    EXPECT_TRUE(sr.WaitForData(3000));
    auto recv = sr.Data();

    // header eliminated
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(memcmp(recv[0].get(), "hello", 5), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Multi-chunk
TEST(WinTunnelIPC, DataPath_MultiChunk_Tunnel)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = "proto=tunnel;path=" + UniqueUds("mc_t") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelTestReceiver sr, cr;
    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);
    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    sdv::pointer<uint8_t> p1, p2;
    p1.resize(3);
    memcpy(p1.get(), "sdv", 3);
    p2.resize(9);
    memcpy(p2.get(), "framework", 9);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p1);
    seq.push_back(p2);

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

// Large fragmentation
TEST(WinTunnelIPC, DataPath_LargePayload_Tunnel)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = "proto=tunnel;path=" + UniqueUds("big_t") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelTestReceiver sr, cr;
    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);
    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    const size_t N = 256 * 1024;
    sdv::pointer<uint8_t> payload;
    payload.resize(N);

    for (size_t i = 0; i < N; ++i)
        payload.get()[i] = uint8_t(i & 0xFF);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(payload);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);
    EXPECT_TRUE(sender->SendData(seq));

    ASSERT_TRUE(sr.WaitForData(5000));
    auto recv = sr.Data();

    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), N);
    EXPECT_EQ(memcmp(recv[0].get(), payload.get(), N), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Header stripping test (tunnel feature)
TEST(WinTunnelIPC, DataPath_HeaderStripped_Tunnel)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    app.SetRunningMode();

    CSocketsTunnelChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs =
        "proto=tunnel;path=" + UniqueUds("headerstrip") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreateTunnelPair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);
    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    const char* msg = "HEADER_TEST";
    const size_t len = strlen(msg);

    sdv::pointer<uint8_t> p;
    p.resize(len);
    memcpy(p.get(), msg, len);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    EXPECT_TRUE(sender->SendData(seq));
    ASSERT_TRUE(sr.WaitForData(3000));

    auto recv = sr.Data();

    // check header stripped
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), len);
    EXPECT_EQ(memcmp(recv[0].get(), msg, len), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();
    mgr.Shutdown(); 
    app.Shutdown();
}

