/************************************************************
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
 ************************************************************/

#ifdef _WIN32
#include "connection.h"


CWinTunnelConnection::CWinTunnelConnection(
    std::shared_ptr<CWinsockConnection> transport,
    uint16_t channelId)
    : m_Transport(std::move(transport))
    , m_ChannelId(channelId)
{
    // No additional initialization required; acts as a thin wrapper.
}

bool CWinTunnelConnection::SendData(
    /*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] SendData failed: transport is null");
        return false;
    }

    // Build tunnel header buffer
    sdv::pointer<uint8_t> hdrBuf;
    hdrBuf.resize(sizeof(STunnelHeader));

    STunnelHeader hdr{};
    hdr.uiChannelId = m_ChannelId;  // Logical channel for this connection
    hdr.uiFlags     = 0;            // Reserved for future use

    std::memcpy(hdrBuf.get(), &hdr, sizeof(STunnelHeader));

    // Compose new sequence: [header] + original payload chunks
    sdv::sequence<sdv::pointer<uint8_t>> seqWithHdr;
    seqWithHdr.push_back(hdrBuf);
    for (auto& chunk : seqData)
    {
        seqWithHdr.push_back(chunk);
    }

    bool result = m_Transport->SendData(seqWithHdr);
    if (!result) {
        SDV_LOG_ERROR("[WinTunnel] SendData failed in underlying transport");
    }
    return result;
}

bool CWinTunnelConnection::AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver)
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] AsyncConnect failed: transport is null");
        return false;
    }

    // Store upper-layer callbacks (safe for null)
    {
        std::lock_guard<std::mutex> lock(m_CallbackMtx);
        sdv::TInterfaceAccessPtr acc(pReceiver);
        m_pUpperReceiver = acc.GetInterface<sdv::ipc::IDataReceiveCallback>();
        m_pUpperEvent    = acc.GetInterface<sdv::ipc::IConnectEventCallback>();
    }

    // Register this tunnel as the data/event receiver in the AF_UNIX transport.
    bool result = m_Transport->AsyncConnect(this);
    if (!result) {
        SDV_LOG_ERROR("[WinTunnel] AsyncConnect failed in underlying transport");
    }
    return result;
}

bool CWinTunnelConnection::WaitForConnection(/*in*/ uint32_t uiWaitMs)
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] WaitForConnection failed: transport is null");
        return false;
    }
    return m_Transport->WaitForConnection(uiWaitMs);
}

void CWinTunnelConnection::CancelWait()
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] CancelWait failed: transport is null");
        return;
    }
    m_Transport->CancelWait();
}

void CWinTunnelConnection::Disconnect()
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] Disconnect failed: transport is null");
        return;
    }

    m_Transport->Disconnect();

    // Clear upper-layer callbacks (thread-safe)
    {
        std::lock_guard<std::mutex> lock(m_CallbackMtx);
        m_pUpperReceiver = nullptr;
        m_pUpperEvent    = nullptr;
    }
}

uint64_t CWinTunnelConnection::RegisterStateEventCallback(
    /*in*/ sdv::IInterfaceAccess* pEventCallback)
{
    if (!m_Transport)
    {
        SDV_LOG_ERROR("[WinTunnel] RegisterStateEventCallback failed: transport is null");
        return 0ULL;
    }

    // Forward directly to underlying CWinsockConnection
    return m_Transport->RegisterStateEventCallback(pEventCallback);
}

void CWinTunnelConnection::UnregisterStateEventCallback(
    /*in*/ uint64_t uiCookie)
{
    if (!m_Transport || uiCookie == 0ULL)
    {
        SDV_LOG_ERROR("[WinTunnel] UnregisterStateEventCallback failed: transport is null or cookie is 0");
        return;
    }

    m_Transport->UnregisterStateEventCallback(uiCookie);
}

sdv::ipc::EConnectState CWinTunnelConnection::GetConnectState() const
{
    if (!m_Transport)
    {
        return sdv::ipc::EConnectState::uninitialized;
    }
    return m_Transport->GetConnectState();
}

void CWinTunnelConnection::DestroyObject()
{
    Disconnect();
    std::lock_guard<std::mutex> lock(m_CallbackMtx);
    m_Transport.reset();
}

void CWinTunnelConnection::ReceiveData(
    /*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
    // Expect at least one chunk (the tunnel header)
    if (seqData.empty())
    {
        SDV_LOG_ERROR("[WinTunnel] ReceiveData: empty sequence");
        return; // nothing to do
    }

    const auto& hdrChunk = seqData[0];
    if (hdrChunk.size() < sizeof(STunnelHeader))
    {
        SDV_LOG_ERROR("[WinTunnel] ReceiveData: invalid tunnel header size");
        // Invalid tunnel frame; drop it for now (could set communication_error)
        return;
    }

    STunnelHeader hdr{};
    std::memcpy(&hdr, hdrChunk.get(), sizeof(STunnelHeader));


    // TODO: use channelId for multiplexing later

    // Build payload-only sequence: drop header chunk, keep others
    sdv::sequence<sdv::pointer<uint8_t>> payload;
    for (size_t i = 1; i < seqData.size(); ++i)
    {
        payload.push_back(seqData[i]);
    }

    if (m_pUpperReceiver)
    {
        try {
            m_pUpperReceiver->ReceiveData(payload); // header stripped
        } catch (...) {
            SDV_LOG_ERROR("[WinTunnel] Exception in upper receiver's ReceiveData");
        }
    }
}

void CWinTunnelConnection::SetConnectState(sdv::ipc::EConnectState state)
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
            SDV_LOG_ERROR("[WinTunnel] Exception in upper event callback's SetConnectState");
            // Never let user callback crash the transport.
        }
    }
}
#endif