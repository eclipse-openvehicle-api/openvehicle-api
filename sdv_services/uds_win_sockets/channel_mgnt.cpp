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

#include "../../global/base64.h"
#include <support/toml.h>
#include <interfaces/process.h>

#include <future>

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

    // Must contain "proto=uds" to be considered UDS
    if (cs.find(protoKey) == std::string::npos)
    {
        return false;
    }

    const auto p = cs.find(pathKey);
    if (p == std::string::npos)
    {
        // No path given, but proto=uds is present -> use default later
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

    char  buf[4096] = {};
    DWORD n         = ExpandEnvironmentStringsA(in.c_str(), buf, static_cast<DWORD>(sizeof(buf)));

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
    SOCKADDR_UN    tmp{};
    constexpr auto kMax = sizeof(tmp.sun_path) - 1;

    if (p.size() <= kMax)
    {
        return p;
    }

    return p.substr(0, kMax);
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

    const size_t pos  = p.find_last_of("/\\");
    std::string  base = (pos == std::string::npos) ? p : p.substr(pos + 1);

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

/**
 * @brief Connect to a Windows AF_UNIX server socket with retry logic
 *
 * Repeatedly attempts to connect to the server's UDS path until either:
 *   - connection succeeds, or
 *   - total timeout is exceeded
 *
 * On each attempt:
 *   - a new socket() is created
 *   - connect() is attempted
 *   - on failure the socket is closed and retried
 *
 * This mirrors Linux AF_UNIX behavior where the client waits for the
 * server's socket file to appear/become ready
 *
 * @param[in] rawPath         Raw UDS path from configuration
 * @param[in] totalTimeoutMs  Maximum total wait time in milliseconds
 * @param[in] retryDelayMs    Delay between retries in milliseconds
 *
 * @return Connected SOCKET on success, INVALID_SOCKET on timeout or error
 */
static SOCKET ConnectUnixSocket(
    const std::string& rawPath,
    uint32_t totalTimeoutMs,
    uint32_t retryDelayMs)
{
    const std::string udsPath = MakeShortWinUdsPath(rawPath);

    SOCKADDR_UN addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

    const int addrlen = static_cast<int>(
        offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1
    );

    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(totalTimeoutMs);

    while (true)
    {
        SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("[AF_UNIX] socket() FAIL (client), WSA=", WSAGetLastError());
            return INVALID_SOCKET;
        }

        if (connect(s, reinterpret_cast<const sockaddr*>(&addr), addrlen) == 0)
        {
            SDV_LOG_INFO("[AF_UNIX] connect OK (pathname), path='", udsPath, "'");
            return s;
        }

        int lastError = WSAGetLastError();
        closesocket(s);

        if (std::chrono::steady_clock::now() >= deadline)
        {
            SDV_LOG_ERROR(
                "[AF_UNIX] connect TIMEOUT (pathname), last WSA=",
                lastError, ", path='", udsPath, "'"
            );
            return INVALID_SOCKET;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
    }
}

} // anonymous namespace

bool CSocketsChannelMgnt::OnInitialize()
{
    return true;
}

void CSocketsChannelMgnt::OnServerClosed(const std::string& udsPath, CWinsockConnection* ptr)
{
    std::lock_guard<std::mutex> lock(m_udsMtx);

    auto it = m_udsServers.find(udsPath);
    if (it != m_udsServers.end() && it->second.get() == ptr)
    {
        // Remove the server entry only if it matches the pointer we know
        m_udsServers.erase(it);
    }

    // Mark this UDS path as no longer claimed
    m_udsServerClaimed.erase(udsPath);
}

void CSocketsChannelMgnt::OnShutdown()
{}

sdv::ipc::SChannelEndpoint CSocketsChannelMgnt::CreateEndpoint(const sdv::u8string& cfgStr)
{
    // Ensure WinSock is initialized on Windows
    if (StartUpWinSock() != 0)
    {
        // If WinSock cannot be initialized, we cannot create an endpoint
        return {};
    }

    // Parse UDS path from config. If proto!=uds, we still default to UDS
    std::string udsRaw;
    bool        udsRequested = ParseUdsPath(cfgStr, udsRaw);

    if (!udsRequested || udsRaw.empty())
    {
        // Default path if not provided or not UDS-specific
        udsRaw = "%LOCALAPPDATA%/sdv/vapi.sock";
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

    {
        std::lock_guard<std::mutex> lock(m_udsMtx);
        m_udsServers[udsPath] = server;
        m_udsServerClaimed.erase(udsPath);
    }

    sdv::ipc::SChannelEndpoint ep{};
    ep.pConnection   = static_cast<IInterfaceAccess*>(server.get());
    ep.ssConnectString = "proto=uds;path=" + udsPath + ";";

    return ep;
}

sdv::IInterfaceAccess* CSocketsChannelMgnt::Access(const sdv::u8string& cs)
{
    // Ensure WinSock is initialized
    if (StartUpWinSock() != 0)
    {
        return nullptr;
    }

    std::string udsRaw;
    if (!ParseUdsPath(cs, udsRaw))
    {
        // Not a UDS connect string
        return nullptr;
    }

    if (udsRaw.empty())
    {
        udsRaw = "%LOCALAPPDATA%/sdv/vapi.sock";
    }

    std::string udsPath = NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX] Access udsPath=", udsPath);

    {
        std::lock_guard<std::mutex> lock(m_udsMtx);

        auto it = m_udsServers.find(udsPath);
        if (it != m_udsServers.end() && it->second != nullptr)
        {
            // Return the server-side object only once for this UDS path
            if (!m_udsServerClaimed.count(udsPath))
            {
                m_udsServerClaimed.insert(udsPath);
                SDV_LOG_INFO("[AF_UNIX] Access -> RETURN SERVER for ", udsPath);
                return it->second.get(); // server object (acceptRequired=true)
            }
            // Otherwise, later calls will create a client socket below
        }
    }

    // CLIENT: create a socket connected to the same udsPath
    SOCKET s = ConnectUnixSocket(udsPath,
                                 /*totalTimeoutMs*/ 5000,
                                 /*retryDelayMs*/   50);

    if (s == INVALID_SOCKET)
    {
        return nullptr;
    }

    SDV_LOG_INFO("[AF_UNIX] Access -> CREATE CLIENT for ", udsPath);

    // Client-side connection object (acceptRequired=false)
    // Ownership is transferred to the caller (VAPI runtime)
    return new CWinsockConnection(s, /*acceptRequired*/ false);
}

#endif