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
#ifdef _WIN32
#ifndef WIN_TUNNEL_CHANNEL_MGNT_H
#define WIN_TUNNEL_CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>
#include "../sdv_services/uds_win_sockets/channel_mgnt.h"              
#include "../sdv_services/uds_win_sockets/connection.h"
#include "connection.h"   

#include <mutex>
#include <map>
#include <memory>
#include <set>
#include <string>

// Winsock headers are required for SOCKET / AF_UNIX / WSAStartup
// NOTE: The actual initialization is done via StartUpWinSock()
#include <ws2tcpip.h>

class CWinTunnelConnection;

/**
 * @class CSocketsTunnelChannelMgnt
 * @brief IPC channel management class for Windows AF_UNIX tunnel communication.
 *
 * Similar to CSocketsChannelMgnt (proto=uds), but:
 *  - uses CWinTunnelConnection (tunnel wrapper) on top of CWinsockConnection
 *  - uses proto=tunnel in connect strings
 *
 * Provides creation and access to tunnel endpoints, manages server-side tunnel lifetimes,
 * and integrates with the SDV object/component framework.
 */
class CSocketsTunnelChannelMgnt :
    public sdv::CSdvObject,
    public sdv::ipc::ICreateEndpoint,
    public sdv::ipc::IChannelAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IChannelAccess)
        SDV_INTERFACE_ENTRY(sdv::ipc::ICreateEndpoint)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("WinTunnelChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("LocalChannelControl")
    DECLARE_DEFAULT_OBJECT_NAME("LocalChannelControl")
    DECLARE_OBJECT_SINGLETON()

    virtual ~CSocketsTunnelChannelMgnt() = default;

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

    /**
     * @brief Creates a tunnel endpoint (server side) and returns endpoint info.
     * @param[in] cfgStr Optional config string (TOML or connect string).
     * @return The channel endpoint structure.
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(const sdv::u8string& cfgStr) override;

    /**
     * @brief Creates or accesses a connection object from the channel connect string.
     * @param[in] cs The channel connect string.
     * @return Pointer to connection access interface.
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& cs) override;

    /**
     * @brief Called by server tunnel when closing (bookkeeping).
     * @param[in] udsPath The UDS path for the server.
     * @param[in] ptr Pointer to the tunnel connection being closed.
     */
    void OnServerClosed(const std::string& udsPath, CWinTunnelConnection* ptr);

private:
    std::mutex m_udsMtx;
    std::map<std::string, std::shared_ptr<CWinTunnelConnection>> m_udsServers;
    std::set<std::string> m_udsServerClaimed;
};

DEFINE_SDV_OBJECT(CSocketsTunnelChannelMgnt)

#endif // ! defined WIN_TUNNEL_CHANNEL_MGNT_H
#endif