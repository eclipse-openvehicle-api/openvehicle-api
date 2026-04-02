#if defined(__unix__)

#include "gtest/gtest.h"

#include <support/app_control.h>
#include <interfaces/ipc.h>
#include <support/interface_ptr.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

#include "../sdv_services/uds_unix_tunnel/channel_mgnt.h"
#include "../sdv_services/uds_unix_tunnel/connection.h"
#include "../sdv_services/uds_unix_sockets/connection.h"

// Reuse the same CTunnelTestReceiver from unix_tunnel_connect_tests
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

    void ReceiveData(sdv::sequence<sdv::pointer<uint8_t>>& seqData) override
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_lastData = seqData;
            m_received = true;
        }
        m_cv.notify_all();
    }

    void SetConnectState(sdv::ipc::EConnectState s) override
    {
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

    bool WaitForData(uint32_t ms = 2000)
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        return m_cv.wait_for(lk, std::chrono::milliseconds(ms), [&]{ return m_received; });
    }

    sdv::sequence<sdv::pointer<uint8_t>> GetLastData() const
    {
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

//Manager instantiate + lifecycle
TEST(UnixTunnelChannelMgnt, InstantiateAndLifecycle)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    CUnixTunnelChannelMgnt mgr;

    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::initialized);

    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::configuring));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::configuring);

    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    EXPECT_NO_THROW(mgr.Shutdown());
    EXPECT_EQ(mgr.GetObjectState(), sdv::EObjectState::destruction_pending);

    app.Shutdown();
}

// CreateEndpoint -> Access(server/client) -> AsyncConnect -> Wait -> Disconnect
TEST(UnixTunnelChannelMgnt, BasicConnectDisconnect)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixTunnelChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    // Create a tunnel endpoint (server)
    auto ep = mgr.CreateEndpoint("");
    ASSERT_NE(ep.pConnection, nullptr);
    ASSERT_FALSE(ep.ssConnectString.empty());

    const std::string serverCS = ep.ssConnectString;
    // Convert to client by role=client
    std::string clientCS = serverCS;
    {
        const std::string from = "role=server";
        const std::string to   = "role=client";
        auto pos = clientCS.find(from);
        if (pos != std::string::npos)
            clientCS.replace(pos, from.size(), to);
    }

    // SERVER
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);

    CTunnelMgrTestReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // CLIENT (thread)
    std::atomic<int> clientResult{0};
    std::thread clientThread([&]{
        sdv::TObjectPtr clientObj = mgr.Access(clientCS);
        if (!clientObj) { clientResult = 1; return; }
        auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
        if (!clientConn) { clientResult = 2; return; }
        CTunnelMgrTestReceiver cRcvr;
        if (!clientConn->AsyncConnect(&cRcvr)) { clientResult = 3; return; }
        if (!clientConn->WaitForConnection(5000)) { clientResult = 4; return; }
        if (clientConn->GetConnectState() != sdv::ipc::EConnectState::connected) { clientResult = 5; return; }
        clientConn->Disconnect();
        clientResult = 0;
    });

    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_EQ(serverConn->GetConnectState(), sdv::ipc::EConnectState::connected);

    clientThread.join();
    EXPECT_EQ(clientResult.load(), 0);

    serverConn->Disconnect();

    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

// Data path: "hello" via channel manager (using proto=tunnel)
TEST(UnixTunnelChannelMgnt, DataPath_SimpleHello_ViaManager)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixTunnelChannelMgnt mgr;
    EXPECT_NO_THROW(mgr.Initialize(""));
    EXPECT_NO_THROW(mgr.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_EQ(mgr.GetObjectState(), sdv::EObjectState::running);

    auto ep = mgr.CreateEndpoint("");
    ASSERT_FALSE(ep.ssConnectString.empty());
    const std::string serverCS = ep.ssConnectString;

    std::string clientCS = serverCS;
    {
        const std::string from = "role=server";
        const std::string to   = "role=client";
        auto pos = clientCS.find(from);
        if (pos != std::string::npos)
            clientCS.replace(pos, from.size(), to);
    }

    // Server
    sdv::TObjectPtr serverObj = mgr.Access(serverCS);
    ASSERT_TRUE(serverObj);
    auto* serverConn = serverObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(serverConn, nullptr);
    CTunnelMgrTestReceiver sRcvr;
    ASSERT_TRUE(serverConn->AsyncConnect(&sRcvr));

    // Client
    sdv::TObjectPtr clientObj = mgr.Access(clientCS);
    ASSERT_TRUE(clientObj);
    auto* clientConn = clientObj.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(clientConn, nullptr);
    CTunnelMgrTestReceiver cRcvr;
    ASSERT_TRUE(clientConn->AsyncConnect(&cRcvr));

    EXPECT_TRUE(serverConn->WaitForConnection(5000));
    EXPECT_TRUE(clientConn->WaitForConnection(5000));

    // Payload "hello"
    sdv::pointer<uint8_t> p;
    p.resize(5);
    std::memcpy(p.get(), "hello", 5);
    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientConn);
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));

    EXPECT_TRUE(sRcvr.WaitForData(3000));
    sdv::sequence<sdv::pointer<uint8_t>> recv = sRcvr.GetLastData();

    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "hello", 5), 0);

    clientConn->Disconnect();
    serverConn->Disconnect();

    EXPECT_NO_THROW(mgr.Shutdown());
    app.Shutdown();
}

#endif // defined(__unix__)
