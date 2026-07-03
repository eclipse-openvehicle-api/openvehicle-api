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
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <random>

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
// Helper namespace 
namespace tunnel_utils
{

inline std::string Expand(const std::string& in)
{
    // Simple expand for $HOME / $TMPDIR etc.
    std::string out = in;

    auto replace_env = [&](const std::string& key, const char* env)
    {
        const char* val = std::getenv(env);
        if (!val) return;

        auto pos = out.find(key);
        if (pos != std::string::npos)
        {
            out.replace(pos, key.size(), val);
        }
    };

    replace_env("$HOME", "HOME");
    replace_env("$TMPDIR", "TMPDIR");

    return out;
}

inline void EnsureParentDir(const std::string& full)
{
    auto p = full.find_last_of('/');
    if (p == std::string::npos)
        return;

    std::filesystem::create_directories(full.substr(0, p));
}

inline std::string MakeShortUdsPath(const char* name)
{
    std::string base = "/tmp/sdv/";

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

inline std::string UniqueTunnel()
{
    return "t_" + RandomHex();
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

struct EndpointClientPair
{
    sdv::ipc::IConnect* server = nullptr;  // from ep.pConnection
    sdv::TObjectPtr clientObj;
    sdv::ipc::IConnect* client = nullptr;
};

static EndpointClientPair CreateEndpointClientPair(
    CUnixTunnelChannelMgnt& mgr,
    const sdv::ipc::SChannelEndpoint& ep)
{
    EndpointClientPair out;

    out.server = ep.pConnection
        ? ep.pConnection->GetInterface<sdv::ipc::IConnect>()
        : nullptr;

    out.clientObj = mgr.Access(ep.ssConnectString);
    out.client = out.clientObj
        ? out.clientObj.GetInterface<sdv::ipc::IConnect>()
        : nullptr;

    return out;
}

//Manager instantiate + lifecycle
TEST(UnixTunnelChannelMgnt, InstantiateAndLifecycle)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    CUnixTunnelChannelMgnt mgr;

    EXPECT_NO_THROW(mgr.Initialize(sdv::SObjectInfo()));
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
    mgr.Initialize(sdv::SObjectInfo());
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string tunnel = "t_" + RandomHex();
    const std::string cs =
        "proto=tunnel;path=" + Unique("tunnel_mgr_basic") +
        ";tunnel=" + tunnel + ";";

    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    auto pair = CreateEndpointClientPair(mgr, ep);

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

// Data path: "hello" via channel manager (using proto=tunnel)
// Simple hello (header stripped)
TEST(UnixTunnelChannelMgnt, DataPath_SimpleHello_ViaManager)
{
    sdv::app::CAppControl app;
    ASSERT_TRUE(app.Startup(""));
    app.SetRunningMode();

    CUnixTunnelChannelMgnt mgr;
    mgr.Initialize(sdv::SObjectInfo());
    mgr.SetOperationMode(sdv::EOperationMode::running);

    const std::string tunnel = "t_" + RandomHex();
    const std::string cs =
        "proto=tunnel;path=" + Unique("hello_mgr") +
        ";tunnel=" + tunnel + ";";

    auto ep = mgr.CreateEndpoint(cs);
    ASSERT_FALSE(ep.ssConnectString.empty());

    auto pair = CreateEndpointClientPair(mgr, ep);

    ASSERT_NE(pair.server, nullptr);
    ASSERT_NE(pair.client, nullptr);

    CTunnelMgrTestReceiver sr, cr;

    pair.server->AsyncConnect(&sr);
    SpinUntilServerArmed(pair.server);

    pair.client->AsyncConnect(&cr);

    ASSERT_TRUE(pair.server->WaitForConnection(5000));
    ASSERT_TRUE(pair.client->WaitForConnection(5000));

    // Payload "hello"
    sdv::pointer<uint8_t> p;
    p.resize(5);
    std::memcpy(p.get(), "hello", 5);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* sender = dynamic_cast<sdv::ipc::IDataSend*>(pair.client);
    ASSERT_NE(sender, nullptr);
    EXPECT_TRUE(sender->SendData(seq));

    ASSERT_TRUE(sr.WaitForData(3000));
    auto recv = sr.GetLastData();

    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), 5u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "hello", 5), 0);

    pair.client->Disconnect();
    pair.server->Disconnect();

    mgr.Shutdown();
    app.Shutdown();
}


#endif // defined(__unix__)
