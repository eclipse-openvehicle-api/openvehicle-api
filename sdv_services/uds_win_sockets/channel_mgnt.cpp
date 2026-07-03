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
#include "channel_mgnt.h"
#include "connection.h"

#include <chrono>
#include <future>
#include <mutex>
#include <thread>
#include <cstring>

#include "../../global/base64.h"
#include <interfaces/app.h>
#include <interfaces/process.h>
#include <support/local_service_access.h>
#include <support/toml.h>

#pragma push_macro("interface")
#undef interface

#pragma push_macro("GetObject")
#undef GetObject

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <afunix.h>

#include <array>

#pragma pop_macro("GetObject")
#pragma pop_macro("interface")

namespace
{

static bool EnsureWSAInitialized()
{
    static std::once_flag s_once;
    static bool s_ok = false;

    std::call_once(s_once, []()
    {
        WSADATA wsa{};
        const int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
        s_ok = (rc == 0);
        if (!s_ok)
        {
            SDV_LOG_ERROR("[AF_UNIX] WSAStartup failed, rc=", rc);
        }
    });

    return s_ok;
}

/**
 * @brief Expand Windows environment variables in the form %VAR%.
 *
 * Example:
 *    "%TEMP%\\sdv\\vapi.sock"  → "C:\\Users\\...\\AppData\\Local\\Temp\\sdv\\vapi.sock"
 *
 * If environment expansion fails, the original string is returned unchanged
 *
 * @param[in] in  Input string that may contain %VAR% tokens
 *
 * @return Expanded string, or the original input on failure
 */

static std::string ExpandEnvVars(const std::string& in)
{
    if (in.find('%') == std::string::npos)
    {
        return in;
    }

    char buf[4096] = {};
    DWORD n = ExpandEnvironmentStringsA(in.c_str(), buf, static_cast<DWORD>(sizeof(buf)));
    if (n > 0 && n < sizeof(buf))
    {
        return std::string(buf);
    }

    return in;
}

/**
 * @brief Clamp a Unix Domain Socket pathname to the maximum allowed size
 *
 * Windows AF_UNIX pathname sockets require that `sun_path` fits in
 * `sizeof(sockaddr_un.sun_path) - 1` bytes (including terminating NUL)
 *
 * If the input exceeds this limit, it is truncated
 *
 * @param[in] p  The original pathname
 *
 * @return A safe pathname guaranteed to fit into sun_path
 */

static std::string ClampUdsPath(const std::string& p)
{
    SOCKADDR_UN tmp{};
    constexpr auto kMax = sizeof(tmp.sun_path) - 1;
    if (p.size() <= kMax)
    {
        return p;
    }
    return p.substr(0, kMax);
}

/*static std::string BuildFinalUdsPath(const std::string& rawPath)
{
    std::string full = ExpandEnvVars(rawPath);

    auto pos = full.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        const std::string dir = full.substr(0, pos);
        CreateDirectoryA(dir.c_str(), nullptr);
    }

    return ClampUdsPath(full);
}*/



/**
 * @brief Parse a UDS connect/config string and extract the path
 *
 * Expected format (substring-based, not strict):
 *   "proto=uds;path=<something>;"
 *
 * Behavior:
 *  - If "proto=uds" is missing   -> returns false (not a UDS config)
 *  - If "path=" is missing       -> returns true and outPath is cleared
 *  - If "path=" is present       -> extracts the substring until ';' or end
 *
 * @param cs       Input configuration / connect string
 * @param outPath  Output: extracted path (possibly empty)
 * @return true if this looks like a UDS string, false otherwise
 */
static bool ParseUdsPath(const std::string& cs, std::string& outPath)
{
    constexpr const char* protoKey = "proto=uds";
    constexpr const char* pathKey  = "path=";

    // Strict raw connect string only
    if (cs.rfind(protoKey, 0) != 0)
    {
        return false;
    }

    const auto p = cs.find(pathKey);
    if (p == std::string::npos)
    {
        outPath.clear();
        return true;
    }

    const auto start = p + std::strlen(pathKey);
    const auto end   = cs.find(';', start);
    if (end == std::string::npos)
    {
        outPath = cs.substr(start);
    }
    else
    {
        outPath = cs.substr(start, end - start);
    }

    return true;
}

static std::string SanitizeUdsName(std::string name)
{
    for (char& ch : name)
    {
        const bool isAlphaNum = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
        if (!isAlphaNum && ch != '_' && ch != '-')
        {
            ch = '_';
        }
    }

    if (name.empty())
    {
        name = "sdv";
    }

    return name;
}

/**
 * @brief Build a named UDS raw path using the channel name
 *
 * @param[in] channelName  The name of the channel
 *
 * @return A raw UDS path suitable for further processing
 */
static std::string BuildNamedUdsRawPath(const std::string& channelName)
{
    return "%LOCALAPPDATA%/sdv/" + SanitizeUdsName(channelName) + ".sock";
}

/**
 * @brief Extract a UDS connect string from either raw UDS format or Provider TOML.
 *
 * Supported inputs:
 *  - "proto=uds;path=<...>;"
 *  - [Provider]\nName="unix_domain_sockets"\nConnectString="proto=uds;path=<...>;"
 */
static bool ExtractUdsConnectString(const std::string& in,
                                    std::string& outUdsConnectString)
{
    std::string path;

    // Case 1: strict raw UDS connect string
    if (ParseUdsPath(in, path))
    {
        outUdsConnectString = in;
        return true;
    }

    // Do not run the TOML parser on obviously non-TOML garbage
    //if (!LooksLikeToml(in))
    //{
    //    return false;
    //}

    // Case 2: structured TOML
    sdv::toml::CTOMLParser parser(in);
    if (!parser.IsValid())
    {
        return false;
    }

    const std::string providerName = parser.GetDirect("Provider.Name").GetValue();
    if (!providerName.empty() &&
        providerName != "unix_domain_sockets" &&
        providerName != "WinSocketsChannelControl")
    {
        return false;
    }

    const std::string nested = parser.GetDirect("Provider.ConnectString").GetValue();
    if (!nested.empty())
    {
        if (ParseUdsPath(nested, path))
        {
            outUdsConnectString = nested;
            return true;
        }
        return false;
    }

    const std::string cfgPath = parser.GetDirect("IpcChannel.Path").GetValue();
    if (!cfgPath.empty())
    {
        outUdsConnectString = "proto=uds;path=" + cfgPath + ";";
        return true;
    }

    const std::string cfgName = parser.GetDirect("IpcChannel.Name").GetValue();
    if (!cfgName.empty())
    {
        outUdsConnectString = "proto=uds;path=" + BuildNamedUdsRawPath(cfgName) + ";";
        return true;
    }

    return false;
}


/**
 * @brief Normalize a UDS path for display/logging purposes
 *
 * Extracts the basename from an input path and clamps it to the
 * AF_UNIX pathname size limit. This is *not* the final path used for
 * the socket bind/connection — it is intended only for user-visible logs.
 *
 * Example:
 *    Input:  "C:/Users/.../very/long/path/vapi.sock"
 *    Output: "vapi.sock"
 *
 * @param[in] raw  Raw input path (may contain directories or %VAR%)
 *
 * @return Normalized/clamped basename suitable for logging
 */
static std::string NormalizeUdsPathForWindows(const std::string& raw)
{
    std::string p = ExpandEnvVars(raw);
    const size_t pos = p.find_last_of("/\\");
    std::string base = (pos == std::string::npos) ? p : p.substr(pos + 1);
    if (base.empty())
    {
        base = "sdv.sock";
    }

    SDV_LOG_INFO("[AF_UNIX] Normalize raw='", raw, "' -> base='", base, "'");
    return ClampUdsPath(base);
}


/**
 * @brief Build a short absolute Win32 path suitable for AF_UNIX `sun_path`
 *
 * AF_UNIX pathname sockets on Windows require short, absolute paths
 * under the OS temporary directory.
 *
 * Algorithm:
 *   1. Expand environment variables
 *   2. Extract the basename
 *   3. Place it under "%TEMP%\\sdv\\"
 *   4. Ensure the directory exists
 *   5. Clamp to AF_UNIX size limits
 *
 * Example:
 *    raw:  "%TEMP%\\sdv\\vapi.sock"
 *    ->     "<expanded temp>\\sdv\\vapi.sock"
 *
 * @param[in] raw  Raw input path (may contain %VAR%)
 *
 * @return Fully expanded, clamped, absolute path suitable for bind()/connect()
 */
static std::string MakeShortWinUdsPath(const std::string& raw)
{
    // Expand raw first (may already contain environment variables)
    std::string p = ExpandEnvVars(raw);

    const size_t pos  = p.find_last_of("/\\");
    std::string  base = (pos == std::string::npos) ? p : p.substr(pos + 1);

    if (base.empty())
    {
        base = "sdv.sock";
    }

    // Use %TEMP%\sdv\ as a base directory
    std::string dir = ExpandEnvVars("%TEMP%\\sdv\\");
    CreateDirectoryA(dir.c_str(), nullptr); // OK if already exists

    const std::string full = dir + base;

    // Ensure it fits into sun_path
    return ClampUdsPath(full);
}

/**
 * @brief Build the default UDS path using the current SDV instance ID.
 *
 * Using a single global socket filename makes independently running test
 * processes trample each other when the build executes multiple post-build
 * tests in parallel. Namespace the implicit endpoint per instance instead.
 */
static std::string GetDefaultUdsRawPath()
{
    uint32_t instanceId = 1000u;
    const sdv::app::IAppContext* pAppContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (pAppContext && pAppContext->GetInstanceID() != 0u)
    {
        instanceId = pAppContext->GetInstanceID();
    }

    return "%LOCALAPPDATA%/sdv/vapi_" + std::to_string(instanceId) + ".sock";
}

static std::string GetUniqueEndpointUdsRawPath()
{
    static std::atomic<uint32_t> nextEndpointId { 0u };

    uint32_t instanceId = 1000u;
    const sdv::app::IAppContext* pAppContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (pAppContext && pAppContext->GetInstanceID() != 0u)
    {
        instanceId = pAppContext->GetInstanceID();
    }

    const uint32_t endpointId = nextEndpointId.fetch_add(1u, std::memory_order_relaxed);
    return "%LOCALAPPDATA%/sdv/vapi_" + std::to_string(instanceId) + "_" +
        std::to_string(static_cast<uint32_t>(GetCurrentProcessId())) + "_" + std::to_string(endpointId) + ".sock";
}

/**
 * @brief Create a listening AF_UNIX socket on Windows
 *
 * This creates a WinSock AF_UNIX stream socket, constructs a pathname
 * using MakeShortWinUdsPath(), removes any stale socket file, binds,
 * and marks it for listening.
 *
 * It logs all success/error cases for diagnostic purposes
 *
 * @param[in] rawPath  Raw path string from configuration/connect-string
 *
 * @return A valid SOCKET on success, or INVALID_SOCKET on failure
 */
static SOCKET CreateUnixListenSocket(const std::string& rawPath)
{
    if (!EnsureWSAInitialized())
        return INVALID_SOCKET;

    SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("[AF_UNIX] socket() FAIL (listen), WSA=", WSAGetLastError());
        return INVALID_SOCKET;
    }

    // Short absolute path, common for server and clients
    std::string udsPath = MakeShortWinUdsPath(rawPath);

    SOCKADDR_UN addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

    // Effective length: offsetof + strlen + 1 for "pathname" AF_UNIX
    const int addrlen = static_cast<int>(
        offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1
    );

    // Remove any leftover file for that path
    ::remove(udsPath.c_str());

    if (bind(s, reinterpret_cast<sockaddr*>(&addr), addrlen) == SOCKET_ERROR)
    {
        SDV_LOG_ERROR(
            "[AF_UNIX] bind FAIL (pathname), WSA=",
            WSAGetLastError(), ", path='", udsPath, "'"
        );
        closesocket(s);
        return INVALID_SOCKET;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR)
    {
        SDV_LOG_ERROR(
            "[AF_UNIX] listen FAIL, WSA=",
            WSAGetLastError(), ", path='", udsPath, "'"
        );
        closesocket(s);
        return INVALID_SOCKET;
    }

    SDV_LOG_INFO("[AF_UNIX] bind OK (pathname), listen OK, path='", udsPath, "'");
    return s;
}

/*static SOCKET ConnectUnixSocket(const std::string& rawPath,
                                uint32_t totalTimeoutMs,
                                uint32_t retryDelayMs)
{
    if (!EnsureWSAInitialized())
    {
        return INVALID_SOCKET;
    }

    const std::string udsPath = BuildFinalUdsPath(rawPath);

    SOCKADDR_UN addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

    const int addrlen = static_cast<int>(
        offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1);

    auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(totalTimeoutMs);

    while (true)
    {
        SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("[AF_UNIX] socket FAIL (client), WSA=", WSAGetLastError());
            return INVALID_SOCKET;
        }

        if (connect(s, reinterpret_cast<const sockaddr*>(&addr), addrlen) == 0)
        {
            SDV_LOG_INFO("[AF_UNIX] connect OK: ", udsPath);
            return s;
        }

        const int err = WSAGetLastError();
        closesocket(s);

        if (std::chrono::steady_clock::now() >= deadline)
        {
            SDV_LOG_ERROR("[AF_UNIX] connect TIMEOUT, WSA=", err, ", path=", udsPath);
            return INVALID_SOCKET;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
    }
}*/

} // anonymous namespace

bool CSocketsChannelMgnt::OnInitialize()
{
    return EnsureWSAInitialized();
}

void CSocketsChannelMgnt::OnShutdown()
{
}

void CSocketsChannelMgnt::OnDestroy()
{
    m_watchdog.Clear();
}

sdv::ipc::SChannelEndpoint CSocketsChannelMgnt::CreateEndpoint(const sdv::u8string& cfgStr)
{
    // Parse UDS path from config. If proto!=uds, we still default to UDS
    std::string udsRaw;
    bool        udsRequested = ParseUdsPath(cfgStr, udsRaw);

    if (!udsRequested || udsRaw.empty())
    {
        sdv::toml::CTOMLParser parser(cfgStr);
        const std::string cfgPath = parser.GetDirect("IpcChannel.Path").GetValue();
        const std::string cfgName = parser.GetDirect("IpcChannel.Name").GetValue();

        if (!cfgPath.empty())
        {
            udsRaw = cfgPath;
        }
        else if (!cfgName.empty())
        {
            udsRaw = BuildNamedUdsRawPath(cfgName);
        }
        else if (cfgStr.empty())
        {
            udsRaw = GetUniqueEndpointUdsRawPath();
        }
        else
        {
            udsRaw = GetDefaultUdsRawPath();
        }
    }

    std::string udsPath = NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX] endpoint udsPath=", udsPath);

    SOCKET listenSocket = CreateUnixListenSocket(udsPath);
    if (listenSocket == INVALID_SOCKET)
    {
        // Endpoint creation failed
        return {};
    }

    // Server-side CWinsockConnection, it will accept() a client on first use
    auto server = std::make_shared<CWinsockConnection>(listenSocket, /*acceptRequired*/ true);
    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!server)
    {
        return {};
    }
    server->SetWatchDogRemoveCallback([this](const void* connection)
    {
        m_watchdog.RemoveConnection(connection);
    });
    m_watchdog.AddConnection(server);

    sdv::ipc::SChannelEndpoint ep{};
    ep.pConnection = static_cast<IInterfaceAccess*>(server.get());

    // Keep compatibility with CCommunicationControl::CreateClientConnection,
    // which expects a Provider TOML section with Provider.Name.
    const std::string udsConnectString = "proto=uds;path=" + udsPath + ";";
    ep.ssConnectString = udsConnectString;

    return ep;
}

sdv::IInterfaceAccess* CSocketsChannelMgnt::Access(const sdv::u8string& cs)
{
    std::string udsConnectString;
    if (!ExtractUdsConnectString(cs, udsConnectString))
    {
        // Not a UDS connect string / provider description
        return nullptr;
    }

    std::string udsRaw;
    ParseUdsPath(udsConnectString, udsRaw);

    if (udsRaw.empty())
    {
        udsRaw = GetDefaultUdsRawPath();
    }

    std::string udsPath = NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX] Access udsPath=", udsPath);

    const bool isServer = (udsConnectString.find("role=server") != std::string::npos);
    std::shared_ptr<CWinsockConnection> connection;
    if (isServer)
    {
        SOCKET listenSocket = CreateUnixListenSocket(udsPath);
        if (listenSocket == INVALID_SOCKET)
        {
            return nullptr;
        }
        connection = std::make_shared<CWinsockConnection>(listenSocket, /*acceptRequired*/ true);
    }
    else
    {
        // Client-side endpoint object. The actual socket connect is deferred to AsyncConnect,
        // matching the semantic contract used by ipc_com and shared memory.
        connection = std::make_shared<CWinsockConnection>(udsPath);
    }

    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!connection)
    {
        return nullptr;
    }

    connection->SetWatchDogRemoveCallback([this](const void* instance)
    {
        m_watchdog.RemoveConnection(instance);
    });

    m_watchdog.AddConnection(connection);
    return static_cast<IInterfaceAccess*>(connection.get());
}

#endif