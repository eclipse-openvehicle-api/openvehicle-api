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
#ifndef UNIX_SOCKET_CHANNEL_MGNT_H
#define UNIX_SOCKET_CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>

class CUnixSocketConnection;

/**
 * @brief IPC channel management class for Unix Domain Sockets communication.
 */
class CUnixDomainSocketsChannelMgnt : 
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
    DECLARE_OBJECT_CLASS_NAME("UnixDomainSocketsChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("LocalChannelControl")
    DECLARE_DEFAULT_OBJECT_NAME("LocalChannelControl")
    DECLARE_OBJECT_SINGLETON()

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
     * @brief Create IPC connection object and return the endpoint information. Overload of
     * sdv::ipc::ICreateEndpoint::CreateEndpoint.
     * @details The endpoints are generated using either a size and a name based on the provided channel configuration or if no
     * configuration is supplied a default size of 10k and a randomly generated name. The following configuration
     * can be supplied:
     * @code
     * [IpcChannel]
     * Name = "CHANNEL_1234"
     * Size = 10240
     * @endcode
     * @param[in] ssChannelConfig Optional channel type specific endpoint configuration.
     * @return IPC connection object
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(/*in*/ const sdv::u8string& ssChannelConfig) override;

    /**
     * @brief Create a connection object from the channel connection parameters string
     * @param[in] ssConnectString Reference to the string containing the channel connection parameters.
     * @return Pointer to IInterfaceAccess interface of the connection object or NULL when the object cannot be created.
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& ssConnectString) override;

private:

    // Helper: choose runtime dir (/run/user/<uid>/sdv) or fallback (/tmp/sdv)
    static std::string MakeUserRuntimeDir();

    std::vector<std::shared_ptr<CUnixSocketConnection>> m_ServerConnections;

};
DEFINE_SDV_OBJECT(CUnixDomainSocketsChannelMgnt)

#endif // UNIX_SOCKET_CHANNEL_MGNT_H
#endif // defined(__unix__)