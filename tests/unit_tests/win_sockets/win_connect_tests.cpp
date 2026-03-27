/********************************************************************************
* Copyright (c) 2025-2026 ZF Friedrichshafen AG
*
* This program and the accompanying materials are made available under the 
* terms of the Apache License Version 2.0 which is available at
* https://www.apache.org/licenses/LICENSE-2.0
*
* SPDX-License-Identifier: Apache-2.0 
********************************************************************************/

#if defined(_WIN32)

#include "gtest/gtest.h"

#include <support/app_control.h>
#include <interfaces/ipc.h>

#include "../../../sdv_services/uds_win_sockets/channel_mgnt.cpp"
#include "../../../sdv_services/uds_win_sockets/connection.cpp"

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

//  Helper namespace
namespace test_utils {

/**
 * @brief Ensure the directory for the given full path exists
 */
inline void EnsureParentDir(const std::string& fullPath)
{
    auto pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos)
        return;

    std::string dir = fullPath.substr(0, pos);
    CreateDirectoryA(dir.c_str(), nullptr);
}

/**
 * @brief Expand environment variables such as %LOCALAPPDATA%
 */
inline std::string Expand(const std::string& in)
{
    if (in.find('%') == std::string::npos)
        return in;

    char buf[4096] = {};
    DWORD n = ExpandEnvironmentStringsA(in.c_str(), buf, sizeof(buf));
    return (n > 0 && n < sizeof(buf)) ? std::string(buf) : in;
}

/**
 * @brief Build a short Win32 path for UDS sockets
 */
inline std::string MakeShortUdsPath(const char* name)
{
    std::string base = R"(%LOCALAPPDATA%\sdv\)";
    base = Expand(base);
    EnsureParentDir(base);
    return base + name;
}

/**
 * @brief Generate a random hex suffix for unique paths
 */
inline std::string RandomHex()
{
    std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}

/**
 * @brief Unique absolute path (keeps tests independent)
 */
inline std::string UniqueUds(const char* prefix)
{
    return MakeShortUdsPath((std::string(prefix) + "_" + RandomHex() + ".sock").c_str());
}

/**
 * @brief Wait until a server connection worker has reached "armed" state
 */
inline void SpinUntilServerArmed(sdv::ipc::IConnect* server, uint32_t maxWaitMs = 300)
{
    using namespace std::chrono;
    auto deadline = steady_clock::now() + milliseconds(maxWaitMs);

    while (server->GetStatus() == sdv::ipc::EConnectStatus::uninitialized &&
           steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/**
 * @brief Small sleep helper for teardown time
 */
inline void SleepTiny(uint32_t ms = 20)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace test_utils

//  Unified test receiver (status + data)
class CTestReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IConnectEventCallback,
    public sdv::ipc::IDataReceiveCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    void SetStatus(sdv::ipc::EConnectStatus s) override
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_status = s;
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

    bool WaitForStatus(sdv::ipc::EConnectStatus expected, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                             [&]{ return m_status == expected; });
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

    sdv::ipc::EConnectStatus m_status{ sdv::ipc::EConnectStatus::uninitialized };
    sdv::sequence<sdv::pointer<uint8_t>> m_data;
    bool m_hasData{false};
};


//  Helper for creating server + client on a given UDS connect-string
struct ServerClient
{
    sdv::TObjectPtr serverObj;
    sdv::ipc::IConnect* server = nullptr;
    sdv::TObjectPtr clientObj;
    sdv::ipc::IConnect* client = nullptr;
};

static ServerClient CreatePair(CSocketsChannelMgnt& mgr, const std::string& cs)
{
    ServerClient out;
    // Server
    out.serverObj = mgr.Access(cs);
    out.server = out.serverObj ? out.serverObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    // Client
    out.clientObj = mgr.Access(cs);
    out.client = out.clientObj ? out.clientObj.GetInterface<sdv::ipc::IConnect>() : nullptr;

    return out;
}

//  TESTS START HERE
using namespace test_utils;

// Instantiate manager
TEST(WindowsAFUnixIPC, Instantiate)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    CSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);
    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    app.Shutdown();
}

// Basic connect/disconnect (server + client)
TEST(WindowsAFUnixIPC, BasicConnectDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string path = UniqueUds("basic");
    const std::string cs = std::string("proto=uds;path=") + path + ";";

    // Create endpoint
    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    // Create server + client objects
    auto pair = CreatePair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    // Server side connect
    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    // Client side connect
    pair.client->AsyncConnect(&cr);

    EXPECT_TRUE(pair.server->WaitForConnection(5000));
    EXPECT_TRUE(pair.client->WaitForConnection(5000));

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Simple data path: client -> server sends "hello"
TEST(WindowsAFUnixIPC, DataPath_SimpleHello)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + UniqueUds("hello") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreatePair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    // Build hello payload
    sdv::pointer<uint8_t> p;
    p.resize(5);
    std::memcpy(p.get(), "hello", 5);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    EXPECT_TRUE(sender->SendData(seq));
    EXPECT_TRUE(sr.WaitForData(3000));

    auto recv = sr.Data();
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "hello", 5), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Server disconnect propagates to client
TEST(WindowsAFUnixIPC, ServerDisconnectPropagates)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + UniqueUds("disc") + ";";
    auto ep = mgr.CreateEndpoint(cs);

    auto pair = CreatePair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    pair.server->Disconnect();

    EXPECT_TRUE(cr.WaitForStatus(sdv::ipc::EConnectStatus::disconnected, 3000));

    pair.client->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// DataPath – multi-chunk payload
TEST(WindowsAFUnixIPC, DataPath_MultiChunk)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + UniqueUds("mc") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreatePair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    // Build multichunk
    sdv::pointer<uint8_t> p1, p2;
    p1.resize(3);  std::memcpy(p1.get(), "sdv", 3);
    p2.resize(9);  std::memcpy(p2.get(), "framework", 9);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p1);
    seq.push_back(p2);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);
    EXPECT_TRUE(sender->SendData(seq));

    ASSERT_TRUE(sr.WaitForData(3000));
    auto recv = sr.Data();

    ASSERT_EQ(recv.size(), 2u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "sdv", 3), 0);
    EXPECT_EQ(std::memcmp(recv[1].get(), "framework", 9), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Large payload fragmentation + reassembly
TEST(WindowsAFUnixIPC, DataPath_LargePayloadFragmentation)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + UniqueUds("big") + ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreatePair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

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
    EXPECT_EQ(std::memcmp(recv[0].get(), payload.get(), N), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Zero-length chunks preserved
TEST(WindowsAFUnixIPC, DataPath_ZeroLengthChunks)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + UniqueUds("zlen") + ";";
    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreatePair(mgr, ep.ssConnectString);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    sdv::pointer<uint8_t> p0a, p5, p0b, p8;
    p0a.resize(0);
    p5.resize(5); std::memcpy(p5.get(), "world",5);
    p0b.resize(0);
    p8.resize(8); std::memcpy(p8.get(), "fragment",8);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p0a);
    seq.push_back(p5);
    seq.push_back(p0b);
    seq.push_back(p8);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);
    EXPECT_TRUE(sender->SendData(seq));

    ASSERT_TRUE(sr.WaitForData(3000));
    auto recv = sr.Data();

    ASSERT_EQ(recv.size(), 4u);
    EXPECT_EQ(recv[0].size(), 0u);
    EXPECT_EQ(recv[1].size(), 5u);
    EXPECT_EQ(recv[2].size(), 0u);
    EXPECT_EQ(recv[3].size(), 8u);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Operation mode transitions
TEST(WindowsAFUnixIPC, OperationModeTransitions)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);
    mgr.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::configuring);
    mgr.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    mgr.Shutdown();
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    app.Shutdown();
}

// Reconnect using the same UDS path (two consecutive sessions)
TEST(WindowsAFUnixIPC, ReconnectAfterDisconnect_SamePath)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string udsPath = MakeShortUdsPath(("vapi_win_reconn_" + RandomHex() + ".sock").c_str());

    const std::string cs = std::string("proto=uds;path=") + udsPath + ";";

    // ----- Session 1 -----
    {
        auto ep = mgr.CreateEndpoint(cs);
        auto pair = CreatePair(mgr, ep.ssConnectString);
        ASSERT_NE(pair.server, nullptr);
        ASSERT_NE(pair.client, nullptr);

        CTestReceiver sr, cr;
        pair.server->AsyncConnect(&sr);
        SpinUntilServerArmed(pair.server);

        std::atomic<int> clientRes{0};

        std::thread ct([&]{
            if (!pair.clientObj) { clientRes = 1; return; }
            auto* c = pair.client;
            if (!c) { clientRes = 2; return; }
            c->AsyncConnect(&cr);
            if (!c->WaitForConnection(5000)) { clientRes = 3; return; }
            c->Disconnect();
            clientRes = 0;
        });

        EXPECT_TRUE(pair.server->WaitForConnection(5000));
        ct.join();
        EXPECT_EQ(clientRes.load(), 0);

        pair.server->Disconnect();
    }

    SleepTiny(20);

    // ----- Session 2 -----
    {
        auto ep = mgr.CreateEndpoint(cs);
        auto pair = CreatePair(mgr, ep.ssConnectString);
        ASSERT_NE(pair.server, nullptr);
        ASSERT_NE(pair.client, nullptr);

        CTestReceiver sr, cr;
        pair.server->AsyncConnect(&sr);
        SpinUntilServerArmed(pair.server);

        std::atomic<int> clientRes{0};

        std::thread ct([&]{
            if (!pair.clientObj) { clientRes = 1; return; }
            auto* c = pair.client;
            if (!c) { clientRes = 2; return; }
            c->AsyncConnect(&cr);
            if (!c->WaitForConnection(5000)) { clientRes = 3; return; }
            c->Disconnect();
            clientRes = 0;
        });

        EXPECT_TRUE(pair.server->WaitForConnection(5000));
        ct.join();
        EXPECT_EQ(clientRes.load(), 0);

        pair.server->Disconnect();
    }

    mgr.Shutdown();
    app.Shutdown();
}

// WaitForConnection(INFINITE) with a delayed client
TEST(WindowsAFUnixIPC, WaitForConnection_InfiniteWait_SlowClient)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs = std::string("proto=uds;path=") + MakeShortUdsPath(("vapi_win_slow_" + RandomHex() + ".sock").c_str()) + ";";
    auto ep = mgr.CreateEndpoint(cs);

    sdv::TObjectPtr sObj = mgr.Access(ep.ssConnectString);
    auto* server = sObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    CTestReceiver sr;
    server->AsyncConnect(&sr);
    SpinUntilServerArmed(server);

    std::thread delayedClient([&]{
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sdv::TObjectPtr cObj = mgr.Access(ep.ssConnectString);
        auto* client = cObj.GetInterface<sdv::ipc::IConnect>();
        CTestReceiver cr;
        client->AsyncConnect(&cr);
        client->WaitForConnection(5000);
        client->Disconnect();
    });

    EXPECT_TRUE(server->WaitForConnection(6000));
    delayedClient.join();
    server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// WaitForConnection(0) before and after establishing the connection
TEST(WindowsAFUnixIPC, WaitForConnection_ZeroTimeout_BeforeAndAfter)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs =
        std::string("proto=uds;path=") +
        MakeShortUdsPath(("vapi_win_zero_" + RandomHex() + ".sock").c_str()) +
        ";";

    auto ep = mgr.CreateEndpoint(cs);
    sdv::TObjectPtr sObj = mgr.Access(ep.ssConnectString);
    auto* server = sObj.GetInterface<sdv::ipc::IConnect>();

    ASSERT_NE(server, nullptr);

    CTestReceiver sr;
    server->AsyncConnect(&sr);
    SpinUntilServerArmed(server);

    EXPECT_FALSE(server->WaitForConnection(0));

    sdv::TObjectPtr cObj = mgr.Access(ep.ssConnectString);
    auto* client = cObj.GetInterface<sdv::ipc::IConnect>();

    CTestReceiver cr;
    client->AsyncConnect(&cr);

    EXPECT_TRUE(server->WaitForConnection(5000));
    EXPECT_TRUE(client->WaitForConnection(5000));

    client->Disconnect();
    server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

// Client timeout when NO server exists on that path
TEST(WindowsAFUnixIPC, ClientTimeout_NoServer)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string raw = MakeShortUdsPath(("vapi_win_nosrv_" + RandomHex() + ".sock").c_str());
    const std::string cs = std::string("proto=uds;path=") + raw + ";";

    sdv::TObjectPtr cObj = mgr.Access(cs);

    if (!cObj)
    {
        SUCCEED() << "Access(proto=uds;path=..., no-server) may return nullptr immediately on Windows.";
        mgr.Shutdown();
        app.Shutdown();
        return;
    }

    auto* client = cObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    CTestReceiver cr;
    client->AsyncConnect(&cr);

    EXPECT_FALSE(client->WaitForConnection(1500));
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    EXPECT_EQ(client->GetStatus(), sdv::ipc::EConnectStatus::connection_error);

    client->Disconnect();
    mgr.Shutdown();
    app.Shutdown();
}

// Server closes the connection during a large transfer → client sees disconnected
TEST(WindowsAFUnixIPC, PeerCloseMidTransfer_ClientDetectsDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs =
        std::string("proto=uds;path=") +
        MakeShortUdsPath(("vapi_win_midclose_" + RandomHex() + ".sock").c_str()) +
        ";";

    auto ep = mgr.CreateEndpoint(cs);
    auto pair = CreatePair(mgr, ep.ssConnectString);
    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);
    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    sdv::pointer<uint8_t> buf;
    buf.resize(512 * 1024);
    std::memset(buf.get(), 0xAA, buf.size());

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(buf);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);

    std::atomic<bool> sendOk{true};
    std::thread t([&]{
        sendOk.store(sender->SendData(seq));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pair.server->Disconnect();

    t.join();

    EXPECT_TRUE(cr.WaitForStatus(sdv::ipc::EConnectStatus::disconnected, 3000));

    pair.client->Disconnect();
    mgr.Shutdown();
    app.Shutdown();
}

// Client cancels connect attempt when no server exists → must clean up promptly
TEST(WindowsAFUnixIPC, ClientCancelConnect_NoServer_Cleanup)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string raw = MakeShortUdsPath(("vapi_win_cancel_" + RandomHex() + ".sock").c_str());
    const std::string cs = std::string("proto=uds;path=") + raw + ";";
    sdv::TObjectPtr cObj = mgr.Access(cs);

    if (!cObj)
    {
        SUCCEED() << "Immediate nullptr for no-server case is valid Windows behavior.";
        mgr.Shutdown();
        app.Shutdown();
        return;
    }

    auto* client = cObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    CTestReceiver cr;
    client->AsyncConnect(&cr);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    client->Disconnect();

    EXPECT_EQ(client->GetStatus(), sdv::ipc::EConnectStatus::disconnected);

    mgr.Shutdown();
    app.Shutdown();
}

// Server starts and immediately disconnects (no client)
TEST(WindowsAFUnixIPC, ServerStartThenImmediateDisconnect_NoClient)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs =
        std::string("proto=uds;path=") +
        MakeShortUdsPath(("vapi_win_srvonly_" + RandomHex() + ".sock").c_str()) +
        ";";

    auto ep = mgr.CreateEndpoint(cs);
    sdv::TObjectPtr sObj = mgr.Access(ep.ssConnectString);
    auto* server = sObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    CTestReceiver sr;
    server->AsyncConnect(&sr);
    SpinUntilServerArmed(server);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    server->Disconnect();
    EXPECT_EQ(server->GetStatus(), sdv::ipc::EConnectStatus::disconnected);

    mgr.Shutdown();
    app.Shutdown();
}

// UnregisterStatusEventCallback: ensure single-listener semantics
TEST(WindowsAFUnixIPC, UnregisterStatusEventCallback_SingleListenerSemantics)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string cs =
        std::string("proto=uds;path=") +
        MakeShortUdsPath(("vapi_win_cb_" + RandomHex() + ".sock").c_str()) +
        ";";

    auto ep = mgr.CreateEndpoint(cs);

    sdv::TObjectPtr sObj = mgr.Access(ep.ssConnectString);
    auto* server = sObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    sdv::TObjectPtr cObj = mgr.Access(ep.ssConnectString);
    auto* client = cObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    CTestReceiver regListener;
    const uint64_t cookie = server->RegisterStatusEventCallback(&regListener);
    ASSERT_NE(cookie, 0u);

    CTestReceiver mainRecv;

    server->AsyncConnect(&mainRecv);
    client->AsyncConnect(&mainRecv);
    SpinUntilServerArmed(server);

    EXPECT_TRUE(server->WaitForConnection(5000));
    EXPECT_TRUE(client->WaitForConnection(5000));

    EXPECT_TRUE(mainRecv.WaitForStatus(sdv::ipc::EConnectStatus::connected, 1000));
    EXPECT_FALSE(regListener.WaitForStatus(sdv::ipc::EConnectStatus::connected, 300)) << "The registry listener should NOT receive events while main receiver is active.";

    server->UnregisterStatusEventCallback(cookie);

    client->Disconnect();
    EXPECT_TRUE(mainRecv.WaitForStatus(sdv::ipc::EConnectStatus::disconnected, 1500));

    EXPECT_FALSE(regListener.WaitForStatus(sdv::ipc::EConnectStatus::disconnected, 300));

    server->Disconnect();
    mgr.Shutdown();
    app.Shutdown();
}

// CreateEndpoint with very long path → must be normalized to basename
TEST(WindowsAFUnixIPC, CreateEndpoint_LongInputPath_Normalized)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CSocketsChannelMgnt mgr;
    mgr.Initialize("");
    mgr.SetOperationMode(sdv::EOperationMode::running);

    std::string longName(160, 'A');

    const std::string rawPath = "C:\\Users\\" + longName + "\\AppData\\Local\\sdv\\vapi_win_long_" + RandomHex() + ".sock";
    const std::string cs = std::string("proto=uds;path=") + rawPath + ";";

    auto ep = mgr.CreateEndpoint(cs);

    ASSERT_FALSE(ep.ssConnectString.empty());

    const std::string publishedCS = ep.ssConnectString;
    EXPECT_NE(publishedCS.find("path=vapi_win_long_"), std::string::npos);

    sdv::TObjectPtr sObj = mgr.Access(publishedCS);
    auto* server = sObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    CTestReceiver sr;
    server->AsyncConnect(&sr);
    SpinUntilServerArmed(server);

    sdv::TObjectPtr cObj = mgr.Access(publishedCS);
    auto* client = cObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    CTestReceiver cr;
    client->AsyncConnect(&cr);

    EXPECT_TRUE(server->WaitForConnection(5000));
    EXPECT_TRUE(client->WaitForConnection(5000));

    client->Disconnect();
    server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}

#endif // _WIN32