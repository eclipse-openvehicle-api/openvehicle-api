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

#if defined(__unix__)

#include "gtest/gtest.h"

#include <interfaces/ipc.h>
#include <support/interface_ptr.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>

// Adjust include paths to your tree layout:
#include "../sdv_services/uds_unix_sockets/connection.h"      // CUnixSocketConnection
#include "../sdv_services/uds_unix_tunnel/connection.h"       // CUnixTunnelConnection

// ===================== Test helpers =====================
class CTunnelTestReceiver :
    public sdv::IInterfaceAccess,
    public sdv::ipc::IDataReceiveCallback,
    public sdv::ipc::IConnectEventCallback
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    // IDataReceiveCallback
    void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_lastData  = seqData;
            m_received  = true;
        }
        m_cv.notify_all();
    }

    // IConnectEventCallback
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

    sdv::ipc::EConnectState GetConnectState() const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_state;
    }

private:
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;

    sdv::ipc::EConnectState m_state { sdv::ipc::EConnectState::uninitialized };
    sdv::sequence<sdv::pointer<uint8_t>> m_lastData;
    bool m_received { false };
};

// Small helper similar to MakeRandomSuffix() in uds_connect_tests.cpp
static std::string MakeRandomSuffix()
{
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(rng);
    return oss.str();
}

// ===================== Tests =====================

// BASIC: server + client CUnixSocketConnection wrapped by CUnixTunnelConnection.
//        Just test connect + disconnect via tunnel.
TEST(UnixTunnelIPC, BasicConnectDisconnectViaTunnel)
{
    const std::string udsPath = std::string("/tmp/sdv_tunnel_") + MakeRandomSuffix() + ".sock";

    // --- Physical transports (UDS) ---
    auto serverTransport = std::make_shared<CUnixSocketConnection>(
        -1,   /*preconfiguredFd*/
        true, /*acceptConnectionRequired (server)*/
        udsPath);

    auto clientTransport = std::make_shared<CUnixSocketConnection>(
        -1,
        false, /*client*/
        udsPath);

    // --- Tunnel wrappers ---
    auto serverTunnel = std::make_shared<CUnixTunnelConnection>(
        serverTransport,
        /*channelId*/ 1);

    auto clientTunnel = std::make_shared<CUnixTunnelConnection>(
        clientTransport,
        /*channelId*/ 1);

    CTunnelTestReceiver serverRcvr;
    CTunnelTestReceiver clientRcvr;

    // Register state callbacks (optional, but useful)
    uint64_t srvCookie = serverTunnel->RegisterStateEventCallback(&serverRcvr);
    uint64_t cliCookie = clientTunnel->RegisterStateEventCallback(&clientRcvr);
    EXPECT_NE(srvCookie, 0u);
    EXPECT_NE(cliCookie, 0u);

    // Async connect on both ends
    ASSERT_TRUE(serverTunnel->AsyncConnect(&serverRcvr));
    ASSERT_TRUE(clientTunnel->AsyncConnect(&clientRcvr));

    // Wait for connections
    EXPECT_TRUE(serverTunnel->WaitForConnection(5000));
    EXPECT_TRUE(clientTunnel->WaitForConnection(5000));

    EXPECT_EQ(serverTunnel->GetConnectState(), sdv::ipc::EConnectState::connected);
    EXPECT_EQ(clientTunnel->GetConnectState(), sdv::ipc::EConnectState::connected);

    // Disconnect client first
    clientTunnel->Disconnect();
    EXPECT_TRUE(clientRcvr.WaitForState(sdv::ipc::EConnectState::disconnected, 2000));

    // Disconnect server
    serverTunnel->Disconnect();
    EXPECT_TRUE(serverRcvr.WaitForState(sdv::ipc::EConnectState::disconnected, 2000));

    serverTunnel->UnregisterStateEventCallback(srvCookie);
    clientTunnel->UnregisterStateEventCallback(cliCookie);
}

// DATA PATH: send "hello" via tunnel and verify it is received on server side.
// For now the tunnel is "pass-through" (no STunnelHeader yet).
TEST(UnixTunnelIPC, DataPath_SimpleHello_ViaTunnel)
{
    const std::string udsPath = std::string("/tmp/sdv_tunnel_data_") + MakeRandomSuffix() + ".sock";

    // Physical transports
    auto serverTransport = std::make_shared<CUnixSocketConnection>(
        -1, true, udsPath);
    auto clientTransport = std::make_shared<CUnixSocketConnection>(
        -1, false, udsPath);

    // Tunnel wrappers (same logical channel for both ends)
    auto serverTunnel = std::make_shared<CUnixTunnelConnection>(serverTransport, 1);
    auto clientTunnel = std::make_shared<CUnixTunnelConnection>(clientTransport, 1);

    CTunnelTestReceiver serverRcvr;
    CTunnelTestReceiver clientRcvr;

    ASSERT_TRUE(serverTunnel->AsyncConnect(&serverRcvr));
    ASSERT_TRUE(clientTunnel->AsyncConnect(&clientRcvr));

    ASSERT_TRUE(serverTunnel->WaitForConnection(5000));
    ASSERT_TRUE(clientTunnel->WaitForConnection(5000));

    // Build "hello" payload
    sdv::sequence<sdv::pointer<uint8_t>> seq;
    sdv::pointer<uint8_t> p;
    const char* msg = "hello";
    const size_t len = std::strlen(msg);
    p.resize(len);
    std::memcpy(p.get(), msg, len);
    seq.push_back(p);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientTunnel.get());
    ASSERT_NE(pSend, nullptr);

    EXPECT_TRUE(pSend->SendData(seq));

    // Wait for server-side data callback
    ASSERT_TRUE(serverRcvr.WaitForData(3000));
    sdv::sequence<sdv::pointer<uint8_t>> recv = serverRcvr.GetLastData();

    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), len);
    EXPECT_EQ(std::memcmp(recv[0].get(), msg, len), 0);

    clientTunnel->Disconnect();
    serverTunnel->Disconnect();
}

TEST(UnixTunnelIPC, DataPath_MultiChunk_ViaTunnel)
{
    const std::string udsPath = std::string("/tmp/sdv_tunnel_multi_") + MakeRandomSuffix() + ".sock";

    auto serverTransport = std::make_shared<CUnixSocketConnection>(-1, true, udsPath);
    auto clientTransport = std::make_shared<CUnixSocketConnection>(-1, false, udsPath);

    auto serverTunnel = std::make_shared<CUnixTunnelConnection>(serverTransport, 1);
    auto clientTunnel = std::make_shared<CUnixTunnelConnection>(clientTransport, 1);

    CTunnelTestReceiver serverRcvr;
    CTunnelTestReceiver clientRcvr;

    ASSERT_TRUE(serverTunnel->AsyncConnect(&serverRcvr));
    ASSERT_TRUE(clientTunnel->AsyncConnect(&clientRcvr));
    ASSERT_TRUE(serverTunnel->WaitForConnection(5000));
    ASSERT_TRUE(clientTunnel->WaitForConnection(5000));

    // Two chunks: "sdv" and "framework"
    sdv::pointer<uint8_t> p1, p2;
    p1.resize(3);
    std::memcpy(p1.get(), "sdv", 3);
    p2.resize(9);
    std::memcpy(p2.get(), "framework", 9);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p1);
    seq.push_back(p2);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientTunnel.get());
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));

    ASSERT_TRUE(serverRcvr.WaitForData(3000));
    sdv::sequence<sdv::pointer<uint8_t>> recv = serverRcvr.GetLastData();

    ASSERT_EQ(recv.size(), 2u);
    EXPECT_EQ(recv[0].size(), 3u);
    EXPECT_EQ(recv[1].size(), 9u);
    EXPECT_EQ(std::memcmp(recv[0].get(), "sdv", 3), 0);
    EXPECT_EQ(std::memcmp(recv[1].get(), "framework", 9), 0);

    clientTunnel->Disconnect();
    serverTunnel->Disconnect();
}

TEST(UnixTunnelIPC, DataPath_LargePayload_Fragmentation_ViaTunnel)
{
    const std::string udsPath = std::string("/tmp/sdv_tunnel_large_") + MakeRandomSuffix() + ".sock";

    auto serverTransport = std::make_shared<CUnixSocketConnection>(-1, true, udsPath);
    auto clientTransport = std::make_shared<CUnixSocketConnection>(-1, false, udsPath);

    auto serverTunnel = std::make_shared<CUnixTunnelConnection>(serverTransport, 1);
    auto clientTunnel = std::make_shared<CUnixTunnelConnection>(clientTransport, 1);

    CTunnelTestReceiver serverRcvr;
    CTunnelTestReceiver clientRcvr;

    ASSERT_TRUE(serverTunnel->AsyncConnect(&serverRcvr));
    ASSERT_TRUE(clientTunnel->AsyncConnect(&clientRcvr));
    ASSERT_TRUE(serverTunnel->WaitForConnection(5000));
    ASSERT_TRUE(clientTunnel->WaitForConnection(5000));

    // Build a large payload (e.g. 256 KiB)
    const size_t totalBytes = 256 * 1024;
    sdv::pointer<uint8_t> big;
    big.resize(totalBytes);
    for (size_t i = 0; i < totalBytes; ++i)
        big.get()[i] = static_cast<uint8_t>(i & 0xFF);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(big);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientTunnel.get());
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));

    ASSERT_TRUE(serverRcvr.WaitForData(5000));
    sdv::sequence<sdv::pointer<uint8_t>> recv = serverRcvr.GetLastData();

    ASSERT_EQ(recv.size(), 1u);
    ASSERT_EQ(recv[0].size(), totalBytes);
    EXPECT_EQ(std::memcmp(recv[0].get(), big.get(), totalBytes), 0);

    clientTunnel->Disconnect();
    serverTunnel->Disconnect();
}

// DATA PATH + HEADER: ensure that the upper receiver sees only the original
// payload (no STunnelHeader bytes in front)
TEST(UnixTunnelIPC, DataPath_HeaderStripped_ForUpperReceiver)
{
    const std::string udsPath =
        std::string("/tmp/sdv_tunnel_header_") + MakeRandomSuffix() + ".sock";

    // Physical transports (UDS)
    auto serverTransport = std::make_shared<CUnixSocketConnection>(
        -1, /*server*/ true, udsPath);
    auto clientTransport = std::make_shared<CUnixSocketConnection>(
        -1, /*client*/ false, udsPath);

    // Tunnel wrappers
    auto serverTunnel = std::make_shared<CUnixTunnelConnection>(serverTransport, /*channelId*/ 42);
    auto clientTunnel = std::make_shared<CUnixTunnelConnection>(clientTransport, /*channelId*/ 42);

    CTunnelTestReceiver serverRcvr;
    CTunnelTestReceiver clientRcvr;

    ASSERT_TRUE(serverTunnel->AsyncConnect(&serverRcvr));
    ASSERT_TRUE(clientTunnel->AsyncConnect(&clientRcvr));
    ASSERT_TRUE(serverTunnel->WaitForConnection(5000));
    ASSERT_TRUE(clientTunnel->WaitForConnection(5000));

    // Build payload "HEADER_TEST"
    const char* msg = "HEADER_TEST";
    const size_t len = std::strlen(msg);

    sdv::pointer<uint8_t> p;
    p.resize(len);
    std::memcpy(p.get(), msg, len);

    sdv::sequence<sdv::pointer<uint8_t>> seq;
    seq.push_back(p);

    auto* pSend = dynamic_cast<sdv::ipc::IDataSend*>(clientTunnel.get());
    ASSERT_NE(pSend, nullptr);
    EXPECT_TRUE(pSend->SendData(seq));

    // Wait for server-side data callback
    ASSERT_TRUE(serverRcvr.WaitForData(3000));
    auto recv = serverRcvr.GetLastData();

    // --- Assertions that header was stripped for upper receiver ---
    // Upper receiver must see exactly one chunk with the original size, without extra bytes for STunnelHeader
    ASSERT_EQ(recv.size(), 1u) << "Upper receiver must see exactly one payload chunk";
    ASSERT_EQ(recv[0].size(), len) << "Payload size must equal original size (no header)";

    EXPECT_EQ(std::memcmp(recv[0].get(), msg, len), 0)
        << "Payload content must match exactly the original message (no header in front)";

    clientTunnel->Disconnect();
    serverTunnel->Disconnect();
}

#endif // defined(__unix__)