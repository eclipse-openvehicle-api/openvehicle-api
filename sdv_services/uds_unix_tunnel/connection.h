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
#ifndef UNIX_SOCKET_TUNNEL_CONNECTION_H
#define UNIX_SOCKET_TUNNEL_CONNECTION_H

#include <interfaces/ipc.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>

#include "../sdv_services/uds_unix_sockets/connection.h"   // existing UDS transport

/**
 * @class CUnixTunnelConnection
 * @brief Logical tunnel connection on top of a shared Unix Domain Socket (UDS) transport.
 *
 * This class wraps an existing CUnixSocketConnection (physical tunnel port) and provides
 * optional tunneling capabilities such as a prepended tunnel header (channelId, flags).
 * It is designed to enable demultiplexing of incoming payloads per logical channel in future versions.
 *
 * Currently, this acts as a simple pass-through wrapper. Tunnel header parsing and multi-channel
 * routing are planned as future improvements (see TODOs in implementation).
 */
class CUnixTunnelConnection :
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
     * @brief Small header prepended to each tunneled SDV message.
     *
     * Used for logical channel identification and control flags. This enables future support for
     * multiplexing and advanced features (QoS, direction, etc.).
     */
    struct STunnelHeader
    {
        uint16_t uiChannelId;  ///< Logical channel ID (e.g., IPC_x / REMOTE_IPC_x)
        uint16_t uiFlags;      ///< Reserved for future use (QoS, direction, etc.)
    };

    /**
     * @brief Constructs a tunnel connection wrapper.
     * @param transport Shared pointer to the underlying UDS transport (physical tunnel port).
     * @param channelId Default logical channel ID for this object view (may be ignored if full demux is implemented later).
     */
    explicit CUnixTunnelConnection(
        std::shared_ptr<CUnixSocketConnection> transport,
        uint16_t channelId);

    /**
     * @brief Destructor. Cleans up resources if needed.
     */
    virtual ~CUnixTunnelConnection() = default;

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    // ---------- IDataSend ----------
    /**
     * @brief Sends a sequence of buffers via the tunnel.
     *
     * Prepends a STunnelHeader to the outgoing message buffers and forwards them to the underlying UDS transport.
     * In the current implementation, the channel header is always present but not yet used for multiplexing.
     *
     * @param seqData Sequence of message buffers (may be modified by callee).
     * @return true on successful send, false otherwise.
     */
    bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    // ---------- IConnect ----------
    /**
     * @brief Starts asynchronous connect on the underlying transport and registers this object as receiver.
     * @param pReceiver Pointer to callback interface for data and state notifications.
     * @return true if connect started, false otherwise.
     */
    bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Waits until the underlying transport becomes 'connected'.
     *
     * Forwards to CUnixSocketConnection::WaitForConnection.
     * @param uiWaitMs Timeout in milliseconds to wait.
     * @return true if connection established, false on timeout or error.
     */
    bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /**
     * @brief Cancels any pending connect or wait operation.
     * Delegated to the underlying transport, if needed.
     */
    void CancelWait() override;

    /**
     * @brief Disconnects the tunnel and underlying transport.
     */
    void Disconnect() override;

    /**
     * @brief Registers a state event callback (forwards to transport).
     * @param pEventCallback Pointer to event callback interface.
     * @return Registration cookie (nonzero) or 0 on failure.
     */
    uint64_t RegisterStateEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /**
     * @brief Unregisters a previously registered state event callback.
     * @param uiCookie Registration cookie returned by RegisterStateEventCallback.
     */
    void UnregisterStateEventCallback(/*in*/ uint64_t uiCookie) override;

    /**
     * @brief Gets the current state from the underlying transport.
     * @return The current connection state.
     */
    sdv::ipc::EConnectState GetConnectState() const override;

    // ---------- IObjectDestroy ----------
    /**
     * @brief Releases and cleans up all resources associated with this object.
     */
    void DestroyObject() override;

    // ---------- IDataReceiveCallback ----------
    /**
     * @brief Receives data from the underlying UDS transport.
     *
     * Expects the first chunk of seqData to be a STunnelHeader, strips and processes it,
     * and delivers the remaining payload to the upper-layer receiver registered via AsyncConnect.
     *
     * @param seqData Sequence of received message buffers (header chunk is removed by this call).
     */
    void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    // ---------- IConnectEventCallback ----------
    /**
     * @brief Forwards state changes from the underlying UDS transport to the upper layer.
     * @param state New connection state.
     */
    void SetConnectState(sdv::ipc::EConnectState state) override;

    /**
     * @brief Assigns a logical channel ID for this connection.
     * Optional helper; you may extend with more routing metadata for advanced tunnel use-cases.
     * @param channelId The logical channel ID to set.
     */
    void SetChannelId(uint16_t channelId);

    /**
     * @brief Get the logical channel ID for this connection
     * @return The channel ID
     */
    uint16_t GetChannelId() const noexcept { return m_ChannelId; }

private:
    std::shared_ptr<CUnixSocketConnection> m_Transport;             ///< shared physical tunnel port
    uint16_t m_ChannelId {0};                                       ///< default logical channel id
    sdv::ipc::IDataReceiveCallback* m_pUpperReceiver {nullptr};     ///< Callback to upper layer (data receive)
    sdv::ipc::IConnectEventCallback* m_pUpperEvent {nullptr};       ///< Callback to upper layer (state event)
    mutable std::mutex m_CallbackMtx;                               ///< Mutex to guard callback access
};

#endif // UNIX_SOCKET_TUNNEL_CONNECTION_H
#endif // defined(__unix__)