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

#ifndef WIN_SOCKETS_CHANNEL_MGNT_H
#define WIN_SOCKETS_CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>
#include "connection.h"

#include <map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <string>

#ifdef _WIN32
// Winsock headers are required for SOCKET / AF_UNIX / WSAStartup
// NOTE: The actual initialization is done via StartUpWinSock()
#   include <ws2tcpip.h>
#endif

/**
 * @brief RAII wrapper for addrinfo returned by getaddrinfo().
 *
 * This structure owns the addrinfo list and frees it automatically in the
 * destructor. Copy and move operations are disabled to avoid double-free.
 */
struct CAddrInfo
{
    /**
    * @brief Constructor
    */
    CAddrInfo()							   = default;
    /**
    * @brief Constructor
    */
    CAddrInfo(const CAddrInfo&)			   = delete;
    /**
    * @brief Copy constructor
    */
    CAddrInfo& operator=(const CAddrInfo&) = delete;
    /**
    * @brief Move constructor
    * @param[in] other Reference to the structure to move
    */
    CAddrInfo(CAddrInfo&& other)           = delete;

    /**
    * @brief Move operator.
    * @param[in] other Reference to the structure to move
    * @return Returns reference to CAddrInfo structure
    */
    CAddrInfo& operator=(CAddrInfo&& other) = delete;

    ~CAddrInfo()
    {
        if (AddressInfo != nullptr)
        {
            freeaddrinfo(AddressInfo);
            AddressInfo = nullptr;
        }
    }

    /// @brief Pointer to the addrinfo list returned by getaddrinfo()
    addrinfo* AddressInfo { nullptr };
};

/**
 * @brief Initialize WinSock on Windows (idempotent)
 *
 * This helper ensures WSAStartup() is called only once in the process
 *
 * @return 0 on success, otherwise a WinSock error code
 */
inline int StartUpWinSock()
{
#ifdef _WIN32
    static bool isInitialized = false;
    if (isInitialized)
    {
        return 0;
    }

    WSADATA wsaData {};
    const int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (error != 0)
    {
        SDV_LOG_ERROR("WSAStartup failed with error: ", std::to_string(error));
    }
    else
    {
        SDV_LOG_INFO("WSAStartup initialized");
        isInitialized = true;
    }
    return error;
#else
    // Non-Windows: nothing to do. Return success for symmetry
    return 0;
#endif
}

/**
 * @brief Simple pair of sockets used to connect two child processes
 *
 * This is reserved for potential future use (e.g. TCP/IP or in-proc socket pairs)
 */
struct SocketConnection
{
    SOCKET From { INVALID_SOCKET }; ///< Socket used by the "from" side (child process)
    SOCKET To   { INVALID_SOCKET }; ///< Socket used by the "to" side (child process)
};

/**
 * @brief IPC channel management class for socket-based communication on Windows
 *
 * This implementation uses AF_UNIX (Unix domain sockets) provided by WinSock
 * on Windows to establish a local IPC channel between VAPI processes
 *
 * The class:
 *  - implements IObjectControl (lifecycle, operation mode)
 *  - implements ipc::ICreateEndpoint (endpoint creation)
 *  - implements ipc::IChannelAccess (client access via connect string)
 */
class CSocketsChannelMgnt
    : public sdv::CSdvObject
    , public sdv::ipc::ICreateEndpoint
    , public sdv::ipc::IChannelAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IChannelAccess)
        SDV_INTERFACE_ENTRY(sdv::ipc::ICreateEndpoint)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("WinSocketsChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("LocalChannelControl")
    DECLARE_DEFAULT_OBJECT_NAME("LocalChannelControl")
    DECLARE_OBJECT_SINGLETON()

    virtual ~CSocketsChannelMgnt() = default;

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
     * @brief Create an IPC endpoint and return its connection information
     *
     * Overload of sdv::ipc::ICreateEndpoint::CreateEndpoint
     *
     * The endpoint is implemented as a local AF_UNIX server socket
     * (listen socket) on Windows. The connect string has the format:
     *
     *   proto=uds;path=<udsPath>;
     *
     * If no configuration is provided or no path is specified, a default
     * path is used (under %LOCALAPPDATA%/sdv)
     *
     * Example configuration:
     * @code
     * [IpcChannel]
     * Interface = "127.0.0.1"
     * Port      = 2000
     * @endcode
     *
     * @param[in] ssChannelConfig Optional channel-specific configuration string.
     * @return SChannelEndpoint structure containing the connection object and
     *         the connect string
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(/*in*/ const sdv::u8string& ssChannelConfig) override;

    /**
     * @brief Create or access a connection object from a connection string
     *
     * Overload of sdv::ipc::IChannelAccess::Access
     *
     * The connect string is expected to contain:
     *   proto=uds;path=<udsPath>;
     *
     * For the first Access() call with a given path, the server-side
     * connection object created by CreateEndpoint() can be returned.
     * Subsequent calls will create client-side connections
     *
     * @param[in] ssConnectString String containing the channel connection parameters
     * @return Pointer to IInterfaceAccess interface of the connection object or
     *         nullptr when the object cannot be created
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& ssConnectString) override;

    /**
     * @brief Called by a CWinsockConnection instance when the server side is closed
     *
     * Used to clean up internal registries for a given UDS path
     *
     * @param ptr      Pointer to the CWinsockConnection instance that was closed
     */
    void OnServerClosed(const std::string& udsPath, CWinsockConnection* ptr);

private:
    /// @brief Registry of AF_UNIX server connections keyed by normalized UDS path
    std::map<std::string, std::shared_ptr<CWinsockConnection>> m_udsServers;

    /**
     * @brief Set of UDS paths that already returned their server-side
     *        connection once via Access()
     *
     * This prevents returning the same server object multiple times
     */
    std::unordered_set<std::string> m_udsServerClaimed;

    /// @brief Mutex protecting m_udsServers and m_udsServerClaimed
    std::mutex m_udsMtx;
};

// SDV object factory macro
DEFINE_SDV_OBJECT(CSocketsChannelMgnt)

#endif // ! defined WIN_SOCKETS_CHANNEL_MGNT_H
#endif