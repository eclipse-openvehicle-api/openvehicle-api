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
#ifndef UNIX_TUNNEL_CHANNEL_MGNT_H
#define UNIX_TUNNEL_CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>
#include "watchdog.h"
#include <algorithm>

class CUnixTunnelConnection;

/**
 * @class CUnixTunnelChannelMgnt
 * @brief IPC channel management class for Unix Domain Socket tunnel communication.
 *
 * This manager exposes the "tunnel" IPC type, similar to UDS, with channel multiplexing planned.
 * It provides creation and access to tunnel endpoints, manages server-side tunnel lifetimes,
 * and integrates with the SDV object/component framework.
 */
class CUnixTunnelChannelMgnt :
    public sdv::CSdvObject,
    public sdv::ipc::ICreateEndpoint,
    public sdv::ipc::IChannelAccess
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IChannelAccess)
        SDV_INTERFACE_ENTRY(sdv::ipc::ICreateEndpoint)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("UnixTunnelChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("unix_domain_sockets_tunnel")
    DECLARE_DEFAULT_OBJECT_NAME("unix_domain_sockets_tunnel")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Destructor for CUnixTunnelChannelMgnt.
     */
    virtual ~CUnixTunnelChannelMgnt() = default;

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
     * @brief Last function called before destruction. Overload of sdv::CSdvObject::OnDestroy.
     */
    virtual void OnDestroy() override;

    /**
     * @brief Creates a tunnel endpoint (server side) and returns endpoint info.
     *
     * Optionally uses a TOML config string for custom endpoint parameters.
     *
     * @param ssChannelConfig Optional config string (TOML).
     * @return The channel endpoint structure.
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(const sdv::u8string& ssChannelConfig) override;

    /**
     * @brief Creates or accesses a connection object from the channel connect string.
     *
     * Parses the connect string and returns a pointer to the appropriate connection access interface.
     *
     * @param ssConnectString The channel connect string.
     * @return Pointer to connection access interface.
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& ssConnectString) override;

private:
    /**
     * @brief Helper: chooses runtime dir (/run/user/<uid>/sdv) or fallback (/tmp/sdv).
     *
     * Used for determining the directory path for runtime sockets.
     *
     * @return Directory path for runtime sockets.
     */
    static std::string MakeUserRuntimeDir();

    CUnixTunnelConnectionWatchDog m_watchdog;
};

DEFINE_SDV_OBJECT(CUnixTunnelChannelMgnt)

#endif // UNIX_TUNNEL_CHANNEL_MGNT_H
#endif // defined(__unix__)