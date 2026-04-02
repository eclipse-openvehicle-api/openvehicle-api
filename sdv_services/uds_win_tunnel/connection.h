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
#ifndef UDS_WIN_TUNNEL_CONNECTION_H
#define UDS_WIN_TUNNEL_CONNECTION_H

#include <interfaces/ipc.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>

#include <WinSock2.h>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

#include "../sdv_services/uds_win_sockets/connection.h" // existing AF_UNIX transport: CWinsockConnection


/**
 * @brief Logical tunnel connection on top of a shared Windows AF_UNIX transport.
 *
 * This class does NOT create sockets by itself. It wraps an existing
 * CWinsockConnection (Winsock AF_UNIX) and adds:
 *  - tunnel header (channelId, flags)
 *  - (later) demultiplexing of incoming payloads per logical channel.
 */
class CWinTunnelConnection :
    public sdv::IInterfaceAccess,
    public sdv::IObjectDestroy,
    public sdv::ipc::IDataSend,
    public sdv::ipc::IConnect,
    public sdv::ipc::IDataReceiveCallback,
    public sdv::ipc::IConnectEventCallback
{
public:

    /**
     * @struct STunnelHeader
     * @brief Header prepended to each tunneled SDV message for logical channel identification and flags.
     */
    struct STunnelHeader
    {
        uint16_t uiChannelId;  ///< Logical channel ID (IPC_x / REMOTE_IPC_x)
        uint16_t uiFlags;      ///< Reserved for future use (QoS, direction, etc.)
    };

    /**
     * @brief Construct a tunnel wrapper over an existing AF_UNIX transport.
     * @param[in] transport Shared pointer to the underlying AF_UNIX transport.
     * @param[in] channelId Logical channel ID for this tunnel instance.
     */
    explicit CWinTunnelConnection(
        std::shared_ptr<CWinsockConnection> transport,
        uint16_t channelId);

    /**
     * @brief Destructor.
     */
    virtual ~CWinTunnelConnection() = default;

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    // ---------- IDataSend ----------
    /**
     * @brief Send a sequence of buffers via the tunnel.
     * @param[in,out] seqData Sequence of message buffers (may be modified by callee).
     * @return true on successful send, false otherwise.
     */
    bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    // ---------- IConnect ----------
    /**
     * @brief Start asynchronous connect and register this object as receiver.
     * @param[in] pReceiver Pointer to callback interface for data and state notifications.
     * @return true if connect started, false otherwise.
     */
    bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Wait until the underlying transport becomes 'connected'.
     * @param[in] uiWaitMs Timeout in milliseconds to wait.
     * @return true if connection established, false on timeout or error.
     */
    bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /**
     * @brief Cancel any pending connect or wait operation.
     */
    void CancelWait() override;

    /**
     * @brief Disconnect the tunnel and underlying transport.
     */
    void Disconnect() override;

    /**
     * @brief Register a state event callback (forwards to transport).
     * @param[in] pEventCallback Pointer to event callback interface.
     * @return Registration cookie (nonzero) or 0 on failure.
     */
    uint64_t RegisterStateEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /**
     * @brief Unregister a previously registered state event callback.
     * @param[in] uiCookie Registration cookie returned by RegisterStateEventCallback.
     */
    void UnregisterStateEventCallback(/*in*/ uint64_t uiCookie) override;
    /**
     * @brief Get the current state from the underlying transport.
     * @return The current connection state.
     */
    sdv::ipc::EConnectState GetConnectState() const override;

    // ---------- IObjectDestroy ----------
    /**
     * @brief Release and clean up all resources associated with this object.
     */
    void DestroyObject() override;

    // ---------- IDataReceiveCallback ----------
    /**
     * @brief Receive data from the underlying AF_UNIX transport.
     * @param[in,out] seqData Sequence of received message buffers (header chunk is removed by this call).
     */
    void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    // ---------- IConnectEventCallback ----------
    /**
     * @brief Forward state changes from the underlying transport to the upper layer.
     * @param[in] state New connection state.
     */
    void SetConnectState(sdv::ipc::EConnectState state) override;

    // Helpers
    void SetChannelId(uint16_t channelId) { m_ChannelId = channelId; }
    uint16_t GetChannelId() const noexcept { return m_ChannelId; }

private:
    std::shared_ptr<CWinsockConnection> m_Transport; ///< shared physical tunnel port
    uint16_t m_ChannelId{0};                  ///< default logical channel id

    // Upper layer callbacks (original VAPI receiver)
    sdv::ipc::IDataReceiveCallback*   m_pUpperReceiver{nullptr};
    sdv::ipc::IConnectEventCallback*  m_pUpperEvent{nullptr};
    mutable std::mutex                m_CallbackMtx;
};

#endif // UDS_WIN_TUNNEL_CONNECTION_H
#endif