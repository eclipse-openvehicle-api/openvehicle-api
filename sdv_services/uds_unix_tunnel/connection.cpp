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
#include "connection.h"


/**
 * @brief Constructs a tunnel connection wrapper on top of an existing UDS transport.
 * @param transport Shared pointer to the underlying UDS transport.
 * @param channelId Logical channel ID for this tunnel instance.
 */
CUnixTunnelConnection::CUnixTunnelConnection(
    std::shared_ptr<CUnixSocketConnection> transport,
    uint16_t channelId)
    : m_Transport(std::move(transport))
    , m_ChannelId(channelId)
{
    // No additional initialization required; acts as a thin wrapper.
}


/**
 * @brief Prepends a tunnel header and forwards the data to the underlying transport.
 * @param seqData Sequence of message buffers to send (may be modified).
 * @return true if data was sent successfully, false otherwise.
 */
bool CUnixTunnelConnection::SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    if (!m_Transport)
    {
        return false;
    }

    // Build tunnel header buffer
    sdv::pointer<uint8_t> hdrBuf;
    hdrBuf.resize(sizeof(STunnelHeader));

    STunnelHeader hdr{};
    hdr.uiChannelId = m_ChannelId;  // Logical channel for this connection
    hdr.uiFlags     = 0;            // Reserved for future use

    // Copy header structure into the first buffer (little-endian host layout)
    std::memcpy(hdrBuf.get(), &hdr, sizeof(STunnelHeader));

    // Compose new sequence: [header] + original payload chunks
    sdv::sequence<sdv::pointer<uint8_t>> seqWithHdr;
    seqWithHdr.push_back(hdrBuf);
    for (auto& chunk : seqData)
    {
        seqWithHdr.push_back(chunk);
    }

    return m_Transport->SendData(seqWithHdr);
}


/**
 * @brief Starts asynchronous connect and registers upper-layer callbacks.
 * @param pReceiver Pointer to callback interface for data and state notifications.
 * @return true if connect started, false otherwise.
 */
bool CUnixTunnelConnection::AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver)
{
    if (!m_Transport)
    {
        return false;
    }

    // Store upper-layer callbacks (safe for null)
    {
        std::lock_guard<std::mutex> lock(m_CallbackMtx);
        sdv::TInterfaceAccessPtr acc(pReceiver);
        m_pUpperReceiver = acc.GetInterface<sdv::ipc::IDataReceiveCallback>();
        m_pUpperEvent    = acc.GetInterface<sdv::ipc::IConnectEventCallback>();
    }

    // Register this tunnel as the data/event receiver in the UDS transport.
    // Do NOT pass pReceiver to UDS, only to our upper fields!
    return m_Transport->AsyncConnect(this);
}

bool CUnixTunnelConnection::WaitForConnection(/*in*/ uint32_t uiWaitMs)
{
    if (!m_Transport)
    {
        return false;
    }
    return m_Transport->WaitForConnection(uiWaitMs);
}

void CUnixTunnelConnection::CancelWait()
{
    if (!m_Transport)
    {
        return;
    }
    m_Transport->CancelWait();
}

void CUnixTunnelConnection::Disconnect()
{
    if (!m_Transport)
    {
        return;
    }
    m_Transport->Disconnect();

    // Clear upper-layer callbacks (thread-safe)
    std::lock_guard<std::mutex> lock(m_CallbackMtx);
    m_pUpperReceiver = nullptr;
    m_pUpperEvent    = nullptr;
}

uint64_t CUnixTunnelConnection::RegisterStateEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback)
{
    if (!m_Transport)
    {
        return 0ULL;
    }

    // Directly forward to the underlying transport. This allows external
    // components to receive connect-state changes without the tunnel
    // having to implement IConnectEventCallback itself
    return m_Transport->RegisterStateEventCallback(pEventCallback);
}

void CUnixTunnelConnection::UnregisterStateEventCallback(/*in*/ uint64_t uiCookie)
{
    if (!m_Transport || uiCookie == 0ULL)
    {
        return;
    }
    m_Transport->UnregisterStateEventCallback(uiCookie);
}

sdv::ipc::EConnectState CUnixTunnelConnection::GetConnectState() const
{
    if (!m_Transport)
    {
        // Reasonable default if transport is missing
        return sdv::ipc::EConnectState::uninitialized;
    }
    return m_Transport->GetConnectState();
}

void CUnixTunnelConnection::DestroyObject()
{
    // Disconnect underlying transport and clear callbacks.
    Disconnect();

    std::lock_guard<std::mutex> lock(m_CallbackMtx);
    m_Transport.reset();
}

void CUnixTunnelConnection::ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
#ifdef DEBUG_TUNNEL_RECEIVE
    // Optional debug: count every call and print buffer size.
    static std::atomic<uint64_t> s_counter{0};
    auto id = ++s_counter;
    std::cerr << "[Tunnel] ReceiveData call #" << id
              << ", seqData.size=" << seqData.size() << std::endl;
#endif

    if (seqData.empty())
    {
        return;
    }

    // Extract and validate tunnel header, then remove it
    const auto& hdrChunk = seqData[0];
    if (hdrChunk.size() < sizeof(STunnelHeader))
    {
        return;
    }

    STunnelHeader hdr{};
    std::memcpy(&hdr, hdrChunk.get(), sizeof(STunnelHeader));

    seqData.erase(seqData.begin());  // remove header chunk

    // Forward rest of data to upper-layer receiver (set by AsyncConnect)
    sdv::ipc::IDataReceiveCallback* upper = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_CallbackMtx);
        upper = m_pUpperReceiver;
    }

    if (upper)
    {
        upper->ReceiveData(seqData);
    }
}

void CUnixTunnelConnection::SetChannelId(uint16_t channelId)
{
    m_ChannelId = channelId;
}

void CUnixTunnelConnection::SetConnectState(sdv::ipc::EConnectState state)
{
    sdv::ipc::IConnectEventCallback* upper = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_CallbackMtx);
        upper = m_pUpperEvent;
    }

    if (upper)
    {
        try
        {
            upper->SetConnectState(state);
        }
        catch (...)
        {
            // Never let user callback crash the transport.
        }
    }
}
#endif // defined(__unix__)