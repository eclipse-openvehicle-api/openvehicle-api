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

extern int StartUpWinSock();

namespace
{
/**
 * @brief Parse a tunnel connect/config string and extract the path.
 *
 * Expected format:
 *   "proto=tunnel;path=<something>;"
 *
 * Behavior:
 *  - If "proto=tunnel" missing -> false
 *  - If "path=" missing -> true and outPath.clear()
 *
 * @param[in] cs The connect/config string to parse.
 * @param[out] outPath The extracted path, or empty if not found.
 * @return true if parsing succeeded, false otherwise.
 */
static bool ParseTunnelPath(const std::string& cs, std::string& outPath)
{
    constexpr const char* protoKey = "proto=tunnel";
    constexpr const char* pathKey  = "path=";

    if (cs.find(protoKey) == std::string::npos)
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

/**
 * @brief Expands Windows environment variables in a string (e.g., %TEMP%).
 * @param[in] in Input string possibly containing environment variables.
 * @return String with environment variables expanded, or original if expansion fails.
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
 * @brief Clamps a UDS path to the maximum allowed by SOCKADDR_UN.
 * @param[in] p The input path.
 * @return The clamped path.
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

// Only for logging – basename
/**
 * @brief Normalizes a raw UDS path for Windows, extracting the basename and ensuring a default if empty.
 * @param[in] raw The raw path string.
 * @return The normalized basename, clamped to max length.
 */
static std::string NormalizeUdsPathForWindows(const std::string& raw)
{
    std::string p = ExpandEnvVars(raw);
    const size_t pos = p.find_last_of("/\\");
    std::string base = (pos == std::string::npos) ? p : p.substr(pos + 1);
    if (base.empty())
    {
        base = "sdv_tunnel.sock";
    }
    SDV_LOG_INFO("[AF_UNIX][Tunnel] Normalize raw='", raw, "' -> base='", base, "'");
    return ClampUdsPath(base);
}

/**
 * @brief Creates a short, safe UDS path in the Windows temp directory.
 * @param[in] raw The raw path string.
 * @return The full path in %TEMP%\sdv\, clamped to max length.
 */
static std::string MakeShortWinUdsPath(const std::string& raw)
{
    std::string p = ExpandEnvVars(raw);
    const size_t pos = p.find_last_of("/\\");
    std::string base = (pos == std::string::npos) ? p : p.substr(pos + 1);
    if (base.empty())
    {
        base = "sdv_tunnel.sock";
    }

    std::string dir = ExpandEnvVars("%TEMP%\\sdv\\");
    CreateDirectoryA(dir.c_str(), nullptr);
    const std::string full = dir + base;

    return ClampUdsPath(full);
}

/**
 * @brief Creates an AF_UNIX listen socket at the specified path.
 * @param[in] rawPath The raw path for the socket.
 * @return The created socket handle, or INVALID_SOCKET on failure.
 */
static SOCKET CreateUnixListenSocket(const std::string& rawPath)
{
    SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] socket() FAIL (listen), WSA=", WSAGetLastError());
        return INVALID_SOCKET;
    }

    std::string udsPath = MakeShortWinUdsPath(rawPath);
    SOCKADDR_UN addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

    const int addrlen = static_cast<int>(
        offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1);

    ::remove(udsPath.c_str());
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), addrlen) == SOCKET_ERROR)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] bind FAIL, WSA=",
                      WSAGetLastError(), ", path='", udsPath, "'");
        closesocket(s);
        return INVALID_SOCKET;
    }
    if (listen(s, SOMAXCONN) == SOCKET_ERROR)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] listen FAIL, WSA=",
                      WSAGetLastError(), ", path='", udsPath, "'");
        closesocket(s);
        return INVALID_SOCKET;
    }

    SDV_LOG_INFO("[AF_UNIX][Tunnel] bind+listen OK, path='", udsPath, "'");
    return s;
}

/**
 * @brief Connects to an AF_UNIX socket at the specified path, retrying until timeout.
 * @param[in] rawPath The raw path to connect to.
 * @param[in] totalTimeoutMs Total timeout in milliseconds.
 * @param[in] retryDelayMs Delay between retries in milliseconds.
 * @return The connected socket handle, or INVALID_SOCKET on failure.
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
        offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1);

    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(totalTimeoutMs);

    int lastError = 0;
    while (true)
    {
        SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            lastError = WSAGetLastError();
            SDV_LOG_ERROR("[AF_UNIX][Tunnel] socket() FAIL (client), WSA=", lastError);
            return INVALID_SOCKET;
        }

        if (connect(s, reinterpret_cast<const sockaddr*>(&addr), addrlen) == 0)
        {
            SDV_LOG_INFO("[AF_UNIX][Tunnel] connect OK, path='", udsPath, "'");
            return s;
        }

        lastError = WSAGetLastError();
        closesocket(s);

        if (std::chrono::steady_clock::now() >= deadline)
        {
            SDV_LOG_ERROR("[AF_UNIX][Tunnel] connect TIMEOUT, last WSA=",
                          lastError, ", path='", udsPath, "'");
            return INVALID_SOCKET;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
    }
}

} // anonymous namespace

bool CSocketsTunnelChannelMgnt::OnInitialize()
{
    return true;
}

void CSocketsTunnelChannelMgnt::OnShutdown()
{}

// -------- Server bookkeeping (optional) --------
void CSocketsTunnelChannelMgnt::OnServerClosed(const std::string& udsPath, CWinTunnelConnection* ptr)
{
    std::lock_guard<std::mutex> lock(m_udsMtx);
    auto it = m_udsServers.find(udsPath);
    if (it != m_udsServers.end() && it->second.get() == ptr)
    {
        m_udsServers.erase(it);
    }
    m_udsServerClaimed.erase(udsPath);
}

// -------- ICreateEndpoint --------
sdv::ipc::SChannelEndpoint CSocketsTunnelChannelMgnt::CreateEndpoint(const sdv::u8string& cfgStr)
{
    sdv::ipc::SChannelEndpoint ep{};

    if (StartUpWinSock() != 0)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] WinSock startup failed in CreateEndpoint");
        return ep;
    }

    // Optional TOML config: [IpcChannel] Path = "..."
    std::string udsRaw;
    if (!cfgStr.empty())
    {   
        //for toml file
        bool isTOML = cfgStr.find('=') == std::string::npos;
        if(isTOML)
        {
            sdv::toml::CTOMLParser cfg(cfgStr.c_str());
            auto pathNode = cfg.GetDirect("IpcChannel.Path");
            if (pathNode.GetType() == sdv::toml::ENodeType::node_string)
            {
                udsRaw = static_cast<std::string>(pathNode.GetValue());
            }
        }

        //for connect string
        if (udsRaw.empty())
        {
            const std::string s(cfgStr);
            const std::string key = "path=";
            auto pos = s.find(key);
            if (pos != std::string::npos)
            {
                auto end = s.find(';', pos + key.size());
                if (end == std::string::npos)
                    udsRaw = s.substr(pos + key.size());
                else
                    udsRaw = s.substr(pos + key.size(), end - pos - key.size());
            }
        }
    }
    if (udsRaw.empty())
    {
        udsRaw = "%LOCALAPPDATA%/sdv/tunnel.sock";
    }

    std::string udsPathBase = NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX][Tunnel] endpoint udsPath=", udsPathBase);

    SOCKET listenSocket = CreateUnixListenSocket(udsPathBase);
    if (listenSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Failed to create listen socket for endpoint: ", udsPathBase);
        return ep;
    }

    auto serverTransport = std::make_shared<CWinsockConnection>( static_cast<unsigned long long>(listenSocket), true);
    auto serverTunnel    = std::make_shared<CWinTunnelConnection>(serverTransport, /*channelId*/ static_cast<uint16_t>(0u));

    {
        std::lock_guard<std::mutex> lock(m_udsMtx);
        m_udsServers[udsPathBase] = serverTunnel;
        m_udsServerClaimed.erase(udsPathBase);
    }

    ep.pConnection     = static_cast<sdv::IInterfaceAccess*>(serverTunnel.get());
    ep.ssConnectString = "proto=tunnel;role=server;path=" + udsPathBase + ";";

    return ep;
}

sdv::IInterfaceAccess* CSocketsTunnelChannelMgnt::Access(const sdv::u8string& cs)
{
    if (StartUpWinSock() != 0)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] WinSock startup failed in Access()" );
        return nullptr;
    }

    std::string connectStr = static_cast<std::string>(cs);
    std::string udsRaw;
    if (!ParseTunnelPath(connectStr, udsRaw))
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Invalid tunnel connect string: ", connectStr);
        return nullptr;
    }

    if (udsRaw.empty())
    {
        udsRaw = "%LOCALAPPDATA%/sdv/tunnel.sock";
    }

    std::string udsPathBase = NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX][Tunnel] Access udsPath=", udsPathBase);

    const bool isServer =
        (connectStr.find("role=server") != std::string::npos);

    {
        std::lock_guard<std::mutex> lock(m_udsMtx);
        auto it = m_udsServers.find(udsPathBase);
        if (isServer && it != m_udsServers.end() && it->second != nullptr)
        {
            if (!m_udsServerClaimed.count(udsPathBase))
            {
                m_udsServerClaimed.insert(udsPathBase);
                SDV_LOG_INFO("[AF_UNIX][Tunnel] Access -> RETURN SERVER for ", udsPathBase);
                return it->second.get(); // Ownership: managed by m_udsServers (do not delete)
            }
        }
    }

    // CLIENT: create AF_UNIX client socket and wrap it in a tunnel
    SOCKET s = ConnectUnixSocket(udsPathBase, /*totalTimeoutMs*/ 5000, /*retryDelayMs*/ 50);
    if (s == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Failed to connect client socket for ", udsPathBase);
        return nullptr;
    }

    SDV_LOG_INFO("[AF_UNIX][Tunnel] Access -> CREATE CLIENT for ", udsPathBase);
    auto clientTransport = std::make_shared<CWinsockConnection>(s, /*acceptRequired*/ false);
    // Ownership: The returned pointer must be managed and deleted by the SDV framework via IObjectDestroy
    auto* tunnelClient   = new CWinTunnelConnection(clientTransport, /*channelId*/ 0u);

    return static_cast<sdv::IInterfaceAccess*>(tunnelClient);
}
#endif