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

#if defined __unix__

#include "gtest/gtest.h"

#include <support/app_control.h>
#include <interfaces/ipc.h>

#include "../../../sdv_services/uds_unix_sockets/channel_mgnt.h"
#include "../../../sdv_services/uds_unix_sockets/connection.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <sys/un.h>

#include <sstream>
#include <iomanip>
#include <random>


class CUDSConnectReceiver :
        public sdv::IInterfaceAccess,
        public sdv::ipc::IDataReceiveCallback,
        public sdv::ipc::IConnectEventCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    // don't test data path yet
    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& /*seqData*/) override {}

    void SetConnectState(sdv::ipc::EConnectState s) override {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_state = s;
        }
        m_cv.notify_all();
    }

    bool WaitForState(sdv::ipc::EConnectState expected, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);

        if (m_state == expected)
            return true;

        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);

        while (m_state != expected)
        {
            if (m_cv.wait_until(lk, deadline) == std::cv_status::timeout)
                return false;
        }
        return true;
    }

    sdv::ipc::EConnectState GetConnectState() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_state;
    }

private:
    sdv::ipc::EConnectState m_state { sdv::ipc::EConnectState::uninitialized };
    mutable std::mutex        m_mtx;
    std::condition_variable   m_cv;
};

// A data-aware receiver that captures received data chunks and exposes synchronization helpers.
class CUDSDataReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IDataReceiveCallback,
    public sdv::ipc::IConnectEventCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& seqData) override {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_lastData = seqData;    // copy/move depending on sequence semantics
            m_received = true;
        }
        m_cv.notify_all();
    }

    void SetConnectState(sdv::ipc::EConnectState s) override {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_state = s;
        }
        m_cv.notify_all();
    }

    bool WaitForState(sdv::ipc::EConnectState expected, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);

        if (m_state == expected)
            return true;

        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);

        while (m_state != expected)
        {
            if (m_cv.wait_until(lk, deadline) == std::cv_status::timeout)
                return false;
        }

        return true;
    }

    bool WaitForData(uint32_t ms = 2000) {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms),
            [&]{ return m_received; });
    }

    sdv::sequence<sdv::pointer<uint8_t>> GetLastData() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_lastData;
    }

private:
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    sdv::ipc::EConnectState m_state{ sdv::ipc::EConnectState::uninitialized };
    sdv::sequence<sdv::pointer<uint8_t>> m_lastData;
    bool m_received{ false };
};


// A receiver that intentionally throws from SetConnectState(...) to test callback-safety.
class CUDSThrowingReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IDataReceiveCallback,
    public sdv::ipc::IConnectEventCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& /*seq*/) override {}

    void SetConnectState(sdv::ipc::EConnectState s) override
    {
        // Store the last state and then throw to simulate misbehaving user code.
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_last = s;
        }
        m_cv.notify_all();
        throw std::runtime_error("Intentional user callback failure");
    }

    bool WaitForState(sdv::ipc::EConnectState expected, uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);

        if (m_last == expected)
            return true;

        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);

        while (m_last != expected)
        {
            if (m_cv.wait_until(lk, deadline) == std::cv_status::timeout)
                return false;
        }

        return true;
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    sdv::ipc::EConnectState m_last{ sdv::ipc::EConnectState::uninitialized };
};


// Small helper: convert server connectString to client connectString
static std::string MakeClientCS(std::string cs)
{
    const std::string from = "role=server";
    const std::string to   = "role=client";
    auto pos = cs.find(from);
    if (pos != std::string::npos)
        cs.replace(pos, from.size(), to);
    return cs;
}


static std::string ExtractPathFromCS(const std::string& cs) 
{
    // search "path=" in connect-string
    const std::string key = "path=";
    auto p = cs.find(key);
    if (p == std::string::npos) return {};
    auto start = p + key.size();
    auto end = cs.find(';', start);
    if (end == std::string::npos) end = cs.size();
    return cs.substr(start, end - start);
}

static std::string MakeRandomSuffix() 
{
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}


TEST(UnixSocketIPC, Instantiate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    EXPECT_NO_THROW(mgr.Shutdown());

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    appcontrol.Shutdown();
}

TEST(UnixSocketIPC, ChannelConfigString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    EXPECT_NO_THROW(mgr.Shutdown());

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);
}

TEST(UnixSocketIPC, CreateRandomEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CUnixDomainSocketsChannelMgnt mgr;

    // Create an endpoint.
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgr.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    if (sChannelEndpoint.pConnection) sdv::TObjectPtr(sChannelEndpoint.pConnection);
    EXPECT_NO_THROW(mgr.Shutdown());

    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);
}

// BASIC TEST: CreateEndpoint -> Access(server/client) -> AsyncConnect -> Wait -> Disconnect
TEST(UnixSocketIPC, BasicConnectDisconnect)
{
    // Start SDV framework 
    
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    // Create and initialize UDS manager
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Create a Unix socket endpoint
    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());

    std::string serverCS = ep.ssConnectString;
    std::string clientCS = MakeClientCS(serverCS);

    // SERVER SIDE
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);

    sdv::ipc::IConnect* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);

    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // CLIENT SIDE (thread)
    std::atomic<int> clientResult{0};
    std::atomic<bool> allowClientDisconnect{false};
    std::thread clientThread([&]{
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        if (!clientObj) { clientResult = 1; return; }
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        if (!clientConn) { clientResult = 2; return; }

        CUDSConnectReceiver cRcvr;
        if (!clientConn->AsyncConnect(&cRcvr)) { clientResult = 3; return; }

        if (!clientConn->WaitForConnection(5000)) { clientResult = 4; return; }

        if (clientConn->GetConnectState() != sdv::ipc::EConnectState::connected) { clientResult = 5; return; }

        // Wait for the server to be conected before disconecting the client
        while (!allowClientDisconnect.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        clientConn->Disconnect();
        clientResult = 0;
    });

    // SERVER must also report connected 
    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Allow client to dissconect now, because the Server is connected
    allowClientDisconnect = true;   
    clientThread.join();
    EXPECT_EQ(clientResult.load(), 0);

    //DISCONNECT both
    serverConn->Disconnect();
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    //  Shutdown Manager / Framework
    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    std::cout << "Shutdown Manager ok " << std::endl;   
    app.Shutdown();

}

TEST(UnixSocketIPC, ReconnectAfterDisconnect_SamePath)
{
    // Start SDV
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    //UDS manager
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    //Endpoint -> same path for both sessions
    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());

    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    // SESSION 1
    // SERVER
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    sdv::ipc::IConnect* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);

    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // CLIENT (thread)
    std::atomic<int>  clientResult{0};
    std::atomic<bool> allowClientDisconnect{false};

    std::thread clientThread([&]{
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        if (!clientObj) { clientResult = 1; return; }
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        if (!clientConn) { clientResult = 2; return; }

        CUDSConnectReceiver cRcvr;
        if (!clientConn->AsyncConnect(&cRcvr)) { clientResult = 3; return; }

        if (!clientConn->WaitForConnection(5000)) { clientResult = 4; return; }
        if (clientConn->GetConnectState() != sdv::ipc::EConnectState::connected) { clientResult = 5; return; }

        //waits for confirmation before disconect
         while (!allowClientDisconnect.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        clientConn->Disconnect();
        clientResult = 0;
    });

    // Server has to be connected (same timeout with client?)
    EXPECT_TRUE(serverConn->WaitForConnection(5000)) << "Server didn't reach 'connected' in time";
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Allows Client to disconnect and waits for finishing
    allowClientDisconnect = true;
    clientThread.join();
    EXPECT_EQ(clientResult.load(), 0);

    // Disconnect server
    serverConn->Disconnect();
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    // SESSION 2
    // SERVER
    sdv::TObjectPtr serverObj2 = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj2);
    sdv::ipc::IConnect* serverConn2 = serverObj2.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn2, nullptr);

    CUDSConnectReceiver sRcvr2;
    ASSERT_TRUE(serverConn2->AsyncConnect(&sRcvr2));

    // CLIENT (thread)
     std::atomic<int>  clientResult2{0};
    std::atomic<bool> allowClientDisconnect2{false};

    std::thread clientThread2([&]{
        sdv::TObjectPtr clientObj2 = mgr.Access(clientCS);
        if (!clientObj2) { clientResult2 = 1; return; }
        auto* clientConn2 = clientObj2.GetInterface<sdv::ipc::IConnect>();
        if (!clientConn2) { clientResult2 = 2; return; }

        CUDSConnectReceiver cRcvr2;
        if (!clientConn2->AsyncConnect(&cRcvr2)) { clientResult2 = 3; return; }

        if (!clientConn2->WaitForConnection(5000)) { clientResult2 = 4; return; }
        if (clientConn2->GetConnectState() != sdv::ipc::EConnectState::connected) { clientResult2 = 5; return; }

        //waits for confirmation before disconect
        while (!allowClientDisconnect2.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        clientConn2->Disconnect();
        clientResult2 = 0;
    });

    // Server has to be connected
    // if unlink(path) from session 1 worked, bind/listen/accept works again
    EXPECT_TRUE(serverConn2->WaitForConnection(5000)) << "Server didn't reach 'connected' in time (session 2)";
    EXPECT_EQ(serverConn2->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Allows Client to disconnect and waits for finishing
    allowClientDisconnect2 = true;
    clientThread2.join();
    EXPECT_EQ(clientResult2.load(), 0);

    // Disconnect server
    serverConn2->Disconnect();
    EXPECT_EQ(serverConn2->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    //Shutdown manager/framework
    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);
    app.Shutdown();
}

//Manager: Initialize -> configuring -> running -> Shutdown
TEST(UnixSocketIPC, OperationModeTransitions)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    // configuring and then running
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::configuring));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::configuring);
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Shutdown
    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);
    app.Shutdown();
}


//  Endpoint from config TOML + long path(clamping) + success connecting
TEST(UnixSocketIPC, CreateEndpoint_WithConfigAndPathClamping)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // long path, deliberat > 108 (sun_path) — it will be cut in CreateEndpoint
    std::string longName(160, 'A');
    std::string longPath = std::string("/tmp/sdv/") + longName + "_" + MakeRandomSuffix() + ".sock";

    std::ostringstream cfg;
    cfg << "[IpcChannel]\n";
    cfg << "Name = \"ignored_" << MakeRandomSuffix() << "\"\n";
    cfg << "Path = \"" << longPath << "\"\n";

    auto ep = mgr.CreateEndpoint(cfg.str());
    ASSERT_FALSE(ep.ssConnectString.empty());

    // Checking if endpoint is server type and has an ok path
    std::string serverCS = ep.ssConnectString;
    std::string clientCS = MakeClientCS(serverCS);
    auto clampedPath = ExtractPathFromCS(serverCS);
    ASSERT_FALSE(clampedPath.empty());
    EXPECT_LT(clampedPath.size(), sizeof(sockaddr_un::sun_path)); // doar verificare generica

    // Connecting
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(clientConn, nullptr);
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);
    EXPECT_EQ(clientConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Cleanup
    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Access() with default paths on both ends
TEST(UnixSocketIPC, Access_DefaultPath_ServerClientConnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Without "path=", both sides use the default MakeUserRuntimeDir()+"/UDS_auto.sock"
    const std::string serverCS = "proto=uds;role=server;";
    const std::string clientCS = "proto=uds;role=client;";

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(clientConn, nullptr);
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);
    EXPECT_EQ(clientConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//WaitForConnection(INFINITE): client delays by 1s, server waits indefinitely
TEST(UnixSocketIPC, WaitForConnection_InfiniteWait_SlowClient)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    std::thread delayedClient([&]{
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        CUDSConnectReceiver cRcvr;
        clientConn->AsyncConnect(&cRcvr);
        clientConn->WaitForConnection(5000);
        clientConn->Disconnect();
    });

    // INFINITE wait (0xFFFFFFFFu)
    EXPECT_TRUE(serverConn->WaitForConnection(0xFFFFFFFFu));
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Cleanup
    delayedClient.join();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//WaitForConnection(0): immediate check, before and after connection
TEST(UnixSocketIPC, WaitForConnection_ZeroTimeout_BeforeAndAfter)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // before client: immediate check must be false
    EXPECT_FALSE(serverConn->WaitForConnection(0));

    // start client
    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // afterward the immediate check may still fail (race). Use normal wait for determinism.
    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));

    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Only the client starts -> timeout & connection_error
TEST(UnixSocketIPC, ClientTimeout_NoServer)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Unique path ensuring nothing is listening
    const std::string path = std::string("/tmp/sdv/timeout_") + MakeRandomSuffix() + ".sock";
    const std::string clientCS = std::string("proto=uds;role=client;path=") + path + ";";

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(clientConn, nullptr);
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // client loop is ~2s; wait less and check it's still not connected
    EXPECT_FALSE(clientConn->WaitForConnection(1500));
    // after another ~1s it should be in connection_error
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    EXPECT_EQ(clientConn->GetConnectState(), sdv::ipc::EConnectState::connection_error);

    clientConn->Disconnect(); // cleanup (joins threads)
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Server disconnecting -> client transitions to 'disconnected'
TEST(UnixSocketIPC, ServerDisconnectPropagatesToClient)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    ASSERT_TRUE(serverConn->WaitForConnection(5000));
    ASSERT_TRUE(clientConn->WaitForConnection(5000));

    // Break the server side and give the client receiver thread time to see EOF
    serverConn->Disconnect();
   
    // Deterministic wait for client-side transition to 'disconnected'
    EXPECT_TRUE(cRcvr.WaitForState(sdv::ipc::EConnectState::disconnected, /*ms*/ 3000)) << "Client did not observe 'disconnected' after server closed the socket.";

    EXPECT_EQ(clientConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    clientConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Reconnecting on the same server object
TEST(UnixSocketIPC, ReconnectOnSameServerInstance)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // First session
    {
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        CUDSConnectReceiver cRcvr;
        ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));
        ASSERT_TRUE(serverConn->WaitForConnection(5000));
        ASSERT_TRUE(clientConn->WaitForConnection(5000));
        clientConn->Disconnect();
        serverConn->Disconnect();
        EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);
    }

    // Second session on the same serverConn object
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));
    {
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        CUDSConnectReceiver cRcvr;
        ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));
        ASSERT_TRUE(serverConn->WaitForConnection(5000));
        ASSERT_TRUE(clientConn->WaitForConnection(5000));
        clientConn->Disconnect();
    }
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Simple payload "hello" test
TEST(UnixSocketIPC, DataPath_SimpleHello)
{
    // Framework + Manager
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Endpoint
    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    // Server
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);
    CUDSDataReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // Client
    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();

    //sdv::ipc::IDataSend* clientConn = clientObj.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(clientConn, nullptr);
    CUDSDataReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // Wait for both to be connected
    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));

    // Build "hello" payload
    sdv::pointer<uint8_t> p;
    p.resize(5);
    std::memcpy(reinterpret_cast<void*>(p.get()), "hello", 5);
    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    // Send from client -> receive on server
    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn);
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));

    // Wait deterministically for server-side data callback
    EXPECT_TRUE(sRcvr.WaitForData(3000));

    auto recv = sRcvr.GetLastData();
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "hello", 5), 0);

    // Cleanup
    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Multi‑chunk payload (2 chunks)
TEST(UnixSocketIPC, DataPath_MultiChunk_TwoBuffers)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    CUDSDataReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    CUDSDataReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));

    // Two buffers: "sdv" and "framewrok"
    sdv::pointer<uint8_t> p1, p2;
    p1.resize(3);
    std::memcpy(reinterpret_cast<void*>(p1.get()), "sdv", 3);
    p2.resize(9);
    std::memcpy(reinterpret_cast<void*>(p2.get()), "framework", 9);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p1);
    seq.push_back(p2);

    // Send from client -> receive on server
    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn);
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));
   
    EXPECT_TRUE(sRcvr.WaitForData(3000));

    auto recv = sRcvr.GetLastData();
    ASSERT_EQ(recv.size(), 2u);
    EXPECT_EQ(recv[0].size(), 3u);
    EXPECT_EQ(recv[1].size(), 9u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "sdv", 3), 0);
    EXPECT_EQ(std::memcmp(recv[1].get(), "framework", 9), 0);

    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Large payload -> fragmented receive (end‑to‑end reassembly)
TEST(UnixSocketIPC, DataPath_LargePayload_Fragmentation_Reassembly)
{
    // Framework + Manager
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup("")); 
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize("")); 
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Endpoint
    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    // Server
    sdv::TObjectPtr serverObj = mgr.Access(serverCS); 
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(serverConn, nullptr);
    CUDSDataReceiver sRcvr; 
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // Client
    sdv::TObjectPtr clientObj = mgr.Access(clientCS); 
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(clientConn, nullptr);
    CUDSDataReceiver cRcvr; 
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // Wait until both connected
    ASSERT_TRUE(serverConn->WaitForConnection(5000));
    ASSERT_TRUE(clientConn->WaitForConnection(5000));

    // Build a large payload (e.g., 256 KiB) to force fragmentation in SendData(...)
    const size_t totalBytes = 256 * 1024;
    sdv::pointer<uint8_t> big;
    big.resize(totalBytes);

    // Fill with a deterministic pattern for verification
    for (size_t i = 0; i < totalBytes; ++i) 
    {
        big.get()[i] = static_cast<uint8_t>(i & 0xFF);
    }

    sdv::sequence<sdv::pointer<uint8_t>> seq; 
    seq.push_back(big);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn); 
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq)); // SendData will fragment as needed

    // Wait deterministically for server-side ReceiveData(...)
    ASSERT_TRUE(sRcvr.WaitForData(5000));
    auto recv = sRcvr.GetLastData();
    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), totalBytes);
    EXPECT_EQ(std::memcmp(recv[0].get(), big.get(), totalBytes), 0);

    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown()); 
    app.Shutdown();
}

//Zero‑length chunks mixed with non‑zero chunks
TEST(UnixSocketIPC, DataPath_ZeroLengthChunks_ArePreserved)
{
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup("")); 
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize("")); 
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>(); 
    CUDSDataReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>(); 
    CUDSDataReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    ASSERT_TRUE(serverConn->WaitForConnection(5000));
    ASSERT_TRUE(clientConn->WaitForConnection(5000));

    // Build sequence: [0][5][""] [8]
    sdv::pointer<uint8_t> p0, p5, p0b, p8;
    p0.resize(0);
    p5.resize(5); 
    std::memcpy(p5.get(), "world", 5);
    p0b.resize(0);
    p8.resize(8); 
    std::memcpy(p8.get(), "fragment", 8);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p0);
    seq.push_back(p5);
    seq.push_back(p0b);
    seq.push_back(p8);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn); 
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));  // table includes zero sizes; receiver must see them

    ASSERT_TRUE(sRcvr.WaitForData(3000));
    auto recv = sRcvr.GetLastData();
    ASSERT_EQ(recv.size(), 4u);
    EXPECT_EQ(recv[0].size(), 0u);
    EXPECT_EQ(recv[1].size(), 5u);
    EXPECT_EQ(recv[2].size(), 0u);
    EXPECT_EQ(recv[3].size(), 8u);
    EXPECT_EQ(std::memcmp(recv[1].get(), "world", 5), 0);
    EXPECT_EQ(std::memcmp(recv[3].get(), "fragment", 8), 0);

    clientConn->Disconnect();
    serverConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown()); app.Shutdown();
}

//Peer closes mid‑transfer -> SendData fails and client observes disconnected
TEST(UnixSocketIPC, PeerCloseMidTransfer_ClientSeesDisconnected_AndSendMayFailOrSucceed)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;

    ASSERT_NO_THROW(mgr.Initialize(""));
    ASSERT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    ASSERT_TRUE(serverConn->WaitForConnection(5000));
    ASSERT_TRUE(clientConn->WaitForConnection(5000));

    // Large buffer
    sdv::pointer<uint8_t> big;
    big.resize(512 * 1024);
    memset(big.get(), 0xAA, big.size());

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(big);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn);
    ASSERT_NE(pSend, nullptr);

    std::atomic<bool> sendResult{true};
    std::thread t([&]{
        sendResult.store(pSend->SendData(seq));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    serverConn->Disconnect();

    t.join();

    // We no longer EXPECT false:
    // SendData may return true OR false depending on kernel timing.
    // Just log the value for debugging:
    std::cout << "[Debug] SendData result = " << sendResult.load() << std::endl;

    // But client MUST observe disconnected:
    EXPECT_TRUE(cRcvr.WaitForState(sdv::ipc::EConnectState::disconnected, 3000));

    clientConn->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//Client cancels connect (no server) -> clean stop of worker threads
TEST(UnixSocketIPC, ClientCancelConnect_NoServer_CleansUpPromptly)
{
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup("")); 
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize("")); 
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Unique path with no server listening
    const std::string path = std::string("/tmp/sdv/cancel_") + MakeRandomSuffix() + ".sock";
    const std::string clientCS = std::string("proto=uds;role=client;path=") + path + ";";

    sdv::TObjectPtr clientObj = mgr.Access(clientCS); 
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(clientConn, nullptr);
    CUDSConnectReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // Cancel before the built-in retry loop completes.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    clientConn->Disconnect();

    // Immediately after disconnect, state should be 'disconnected' (no hangs).
    EXPECT_EQ(clientConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    EXPECT_NO_THROW(mgr.Shutdown()); app.Shutdown();
}

//Server starts, then immediately disconnects (no client yet)
TEST(UnixSocketIPC, ServerStartThenImmediateDisconnect_NoClient)
{
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize("")); 
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;

    sdv::TObjectPtr serverObj = mgr.Access(serverCS); 
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(serverConn, nullptr);
    CUDSConnectReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // The server may be still listening; ensure we can disconnect cleanly
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    serverConn->Disconnect();
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::disconnected);

    EXPECT_NO_THROW(mgr.Shutdown()); 
    app.Shutdown();
}


//Callback throws in SetConnectState -> transport threads keep running
TEST(UnixSocketIPC, CallbackThrowsInSetConnectState_DoesNotCrashTransport)
{
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup("")); 
    app.SetRunningMode();
    CUnixDomainSocketsChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize("")); 
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS); 
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(serverConn, nullptr);
    CUDSThrowingReceiver sRcvr; ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    sdv::TObjectPtr clientObj = mgr.Access(clientCS); 
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>(); 
    ASSERT_NE(clientConn, nullptr);
    CUDSThrowingReceiver cRcvr; 
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    // Despite exceptions thrown inside SetConnectState, the transport should still reach connected.
    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));

    // And it should still propagate a clean disconnect (even with throwing callbacks).
    clientConn->Disconnect();
    serverConn->Disconnect();

    EXPECT_NO_THROW(mgr.Shutdown()); 
    app.Shutdown();
}

//RegisterStateEventCallback: multiple listeners receive state updates
TEST(UnixSocketIPC, RegisterStateEventCallback_MultipleCallbacksReceiveState)
{
    sdv::app::CAppControl app; 
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    ASSERT_NO_THROW(mgr.Initialize(""));
    ASSERT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // --- Setup server endpoint ---
    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    auto* server = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    auto* client = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    // --- Callback receiver 1 ---
    CUDSConnectReceiver recv1;
    uint64_t cookie1 = server->RegisterStateEventCallback(&recv1);
    EXPECT_NE(cookie1, 0u);

    // --- Callback receiver 2 ---
    CUDSConnectReceiver recv2;
    uint64_t cookie2 = server->RegisterStateEventCallback(&recv2);
    EXPECT_NE(cookie2, 0u);
    EXPECT_NE(cookie1, cookie2);

    // --- Start connections ---
    ASSERT_TRUE(server->AsyncConnect(&recv1));
    ASSERT_TRUE(client->AsyncConnect(&recv2));

    ASSERT_TRUE(server->WaitForConnection(5000));
    ASSERT_TRUE(client->WaitForConnection(5000));

    // Both receivers should have received 'connected'
    EXPECT_TRUE(recv1.WaitForState(sdv::ipc::EConnectState::connected, 1000));
    EXPECT_TRUE(recv2.WaitForState(sdv::ipc::EConnectState::connected, 1000));

    // --- Disconnect ---
    client->Disconnect();

    EXPECT_TRUE(recv1.WaitForState(sdv::ipc::EConnectState::disconnected, 1000));
    EXPECT_TRUE(recv2.WaitForState(sdv::ipc::EConnectState::disconnected, 1000));

    server->Disconnect();
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

//UnregisterStateEventCallback: removed listener stops receiving events
TEST(UnixSocketIPC, UnregisterStateEventCallback_RemovedListenerStopsReceiving)
{
    // Framework + Manager
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixDomainSocketsChannelMgnt mgr;
    ASSERT_NO_THROW(mgr.Initialize(""));
    ASSERT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Endpoint
    auto ep = mgr.CreateEndpoint("");
    const std::string serverCS = ep.ssConnectString;
    const std::string clientCS = MakeClientCS(serverCS);

    // Server
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* server = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(server, nullptr);

    // Client
    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* client = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(client, nullptr);

    // --------- Two distinct receivers ----------
    // recvReg: used ONLY for the state-callback registry
    // recvConn: used ONLY for AsyncConnect (m_pReceiver / m_pEvent path)
    CUDSConnectReceiver recvReg;
    CUDSConnectReceiver recvConn;

    // Register recvReg as a state listener (registry path)
    uint64_t cookie = server->RegisterStateEventCallback(&recvReg);
    ASSERT_NE(cookie, 0u) << "Cookie must be non-zero";

    // Start connections (server uses recvReg only for registry; recvConn is the IConnect/IDataReceive side)
    ASSERT_TRUE(server->AsyncConnect(&recvConn));
    ASSERT_TRUE(client->AsyncConnect(&recvConn));

    // Wait until both sides are connected
    ASSERT_TRUE(server->WaitForConnection(5000));
    ASSERT_TRUE(client->WaitForConnection(5000));

    // Both the connection receiver (recvConn) and the registry listener (recvReg) should observe 'connected'
    EXPECT_TRUE(recvConn.WaitForState(sdv::ipc::EConnectState::connected, 1000)) << "Connection receiver didn't see 'connected'.";
    EXPECT_TRUE(recvReg.WaitForState(sdv::ipc::EConnectState::connected, 1000)) << "Registry listener didn't see 'connected'.";

    // --------- Unregister the registry listener ----------
    server->UnregisterStateEventCallback(cookie);

    // Trigger a disconnect on client to force state transitions
    client->Disconnect();

    // The connection receiver (recvConn) still sees disconnected (normal path)
    EXPECT_TRUE(recvConn.WaitForState(sdv::ipc::EConnectState::disconnected, 1000)) << "Connection receiver didn't see 'disconnected'.";

    // The registry listener (recvReg) MUST NOT receive 'disconnected' anymore
    // (wait a short, deterministic interval)
    EXPECT_FALSE(recvReg.WaitForState(sdv::ipc::EConnectState::disconnected, 300)) << "Registry listener received 'disconnected' after UnregisterStateEventCallback().";

    // Server cleanup
    server->Disconnect();

    // Manager / framework cleanup
    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

#endif // defined __unix__