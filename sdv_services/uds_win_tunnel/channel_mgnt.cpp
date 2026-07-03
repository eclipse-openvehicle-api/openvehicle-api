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

#include <chrono>
#include <future>
#include <mutex>
#include <thread>
#include "../../global/base64.h"
#include <support/toml.h>
#include <interfaces/process.h>

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

//#include "../sdv_services/uds_win_sockets/channel_mgnt.cpp"              

namespace
{

    static std::atomic<uint32_t> g_nextChannelId{1};

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

    static std::string BuildNamedUdsRawPath(const std::string& channelName)
    {
        return "%LOCALAPPDATA%/sdv/" + SanitizeUdsName(channelName) + ".sock";
    }

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

    static std::string ExtractTunnelNameFromToml(sdv::toml::CTOMLParser& parser)
    {
        std::string tunnel;

        auto ipcTunnelNode = parser.GetDirect("IpcChannel.Tunnel");
        if (ipcTunnelNode.GetType() == sdv::toml::ENodeType::node_string)
        {
            tunnel = static_cast<std::string>(ipcTunnelNode.GetValue());
        }

        if (tunnel.empty())
        {
            auto providerTunnelNode = parser.GetDirect("Provider.Tunnel");
            if (providerTunnelNode.GetType() == sdv::toml::ENodeType::node_string)
            {
                tunnel = static_cast<std::string>(providerTunnelNode.GetValue());
            }
        }

        return tunnel;
    }
   
    static bool ExtractTunnelConnectString(const std::string& in,
                                        std::string& outTunnelConnectString)
    {
        std::string path;

        // Case 1: strict raw tunnel connect string
        if (ParseTunnelPath(in, path))
        {
            outTunnelConnectString = in;
            return true;
        }

        // Case 2: structured TOML
        sdv::toml::CTOMLParser parser(in);
        if (!parser.IsValid())
        {
            return false;
        }

        const std::string providerName = parser.GetDirect("Provider.Name").GetValue();
        if (!providerName.empty() &&
            providerName != "unix_domain_sockets_tunnel" &&
            providerName != "WinTunnelChannelControl" &&
            providerName != "UnixTunnelChannelControl")
        {
            return false;
        }

        // Provider.ConnectString = "proto=tunnel;path=...;tunnel=...;"
        const std::string nested = parser.GetDirect("Provider.ConnectString").GetValue();
        if (!nested.empty())
        {
            if (ParseTunnelPath(nested, path))
            {
                outTunnelConnectString = nested;
                return true;
            }
            return false;
        }

        // Build a tunnel connect string from [IpcChannel]
        const std::string tunnel = ExtractTunnelNameFromToml(parser);

        const std::string cfgPath = parser.GetDirect("IpcChannel.Path").GetValue();
        if (!cfgPath.empty())
        {
            outTunnelConnectString = "proto=tunnel;path=" + cfgPath + ";";
            if (!tunnel.empty())
            {
                outTunnelConnectString += "tunnel=" + tunnel + ";";
            }
            return true;
        }

        const std::string cfgName = parser.GetDirect("IpcChannel.Name").GetValue();
        if (!cfgName.empty())
        {
            outTunnelConnectString = "proto=tunnel;path=" + BuildNamedUdsRawPath(cfgName) + ";";
            if (!tunnel.empty())
            {
                outTunnelConnectString += "tunnel=" + tunnel + ";";
            }
            return true;
        }

        return false;
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

     static std::string BuildFinalUdsPath(const std::string& rawPath)
    {
        // Expand environment variables
        std::string full = ExpandEnvVars(rawPath);

        // Ensure directory exists (parent folder)
        auto pos = full.find_last_of("\\/");
        if (pos != std::string::npos)
        {
            std::string dir = full.substr(0, pos);

            // Ensure immediate parent exists (base/tunnel directory is created earlier)
            CreateDirectoryA(dir.c_str(), nullptr);
        }

        // Clamp to AF_UNIX limit
        return ClampUdsPath(full);
    }   

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
     * @brief Creates an AF_UNIX listen socket at the specified path.
     * @param[in] rawPath The raw path for the socket.
     * @return The created socket handle, or INVALID_SOCKET on failure.
     */
    static SOCKET CreateUnixListenSocket(const std::string& rawPath)
    {
        
        if (!EnsureWSAInitialized())
            return INVALID_SOCKET;

        SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("[AF_UNIX] socket FAIL (listen), WSA=", WSAGetLastError());
            return INVALID_SOCKET;
        }

        //bulletproof path handling
        std::string udsPath = BuildFinalUdsPath(rawPath);

        SOCKADDR_UN addr{};
        addr.sun_family = AF_UNIX;
        strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

        const int addrlen = static_cast<int>(
            offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1);

        ::remove(udsPath.c_str());

        if (bind(s, reinterpret_cast<sockaddr*>(&addr), addrlen) == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            SDV_LOG_ERROR("[AF_UNIX] bind FAIL, WSA=", err, ", path=", udsPath);
            closesocket(s);
            return INVALID_SOCKET;
        }

        if (listen(s, SOMAXCONN) == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            SDV_LOG_ERROR("[AF_UNIX] listen FAIL, WSA=", err, ", path=", udsPath);
            closesocket(s);
            return INVALID_SOCKET;
        }

        SDV_LOG_INFO("[AF_UNIX] bind+listen OK: ", udsPath);
        return s;
    }
    
    static bool ExtractTunnelName(const std::string& in, std::string& outTunnel)
    {
        auto pos = in.find("tunnel=");
        if (pos == std::string::npos)
        {
            outTunnel.clear();
            return false;
        }

        pos += 7;
        auto end = in.find(';', pos);
        outTunnel = (end == std::string::npos) ? in.substr(pos) : in.substr(pos, end - pos);
        return !outTunnel.empty();
    }

    /**
     * @brief Connects to an AF_UNIX socket at the specified path, retrying until timeout.
     * @param[in] rawPath The raw path to connect to.CreateUnixListenSocket
     * @param[in] totalTimeoutMs Total timeout in milliseconds.
     * @param[in] retryDelayMs Delay between retries in milliseconds.
     * @return The connected socket handle, or INVALID_SOCKET on failure.
     */
    static SOCKET ConnectUnixSocket(
        const std::string& rawPath,
        uint32_t totalTimeoutMs,
        uint32_t retryDelayMs)
    {
        if (!EnsureWSAInitialized())
            return INVALID_SOCKET;

        //SAME path logic ca server
        const std::string udsPath = BuildFinalUdsPath(rawPath);
        SDV_LOG_INFO("[AF_UNIX][Tunnel] Attempting to connect to ", udsPath, "with rawPath=", rawPath);
        SOCKADDR_UN addr{};
        addr.sun_family = AF_UNIX;
        strcpy_s(addr.sun_path, sizeof(addr.sun_path), udsPath.c_str());

        const int addrlen = static_cast<int>(
            offsetof(SOCKADDR_UN, sun_path) + std::strlen(addr.sun_path) + 1);

        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(totalTimeoutMs);

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

            int err = WSAGetLastError();
            closesocket(s);

            if (std::chrono::steady_clock::now() >= deadline)
            {
                SDV_LOG_ERROR("[AF_UNIX] connect TIMEOUT, WSA=", err, ", path=", udsPath);
                return INVALID_SOCKET;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        }
    }
} // anonymous namespace

bool CSocketsTunnelChannelMgnt::OnInitialize()
{
    return EnsureWSAInitialized();
}

void CSocketsTunnelChannelMgnt::OnShutdown()
{}

void CSocketsTunnelChannelMgnt::OnDestroy()
{
    m_watchdog.Clear();
}


// Note:
// tunnel name is required by the current Windows tunnel design for namespace isolation.
// [IpcChannel.Path]/[IpcChannel.Name] alone are not sufficient unless Tunnel is also provided.
sdv::ipc::SChannelEndpoint CSocketsTunnelChannelMgnt::CreateEndpoint(const sdv::u8string& cfgStr)
{
    sdv::ipc::SChannelEndpoint ep{};
    std::string udsRaw;
    std::string tunnelConnectString;

    if (ExtractTunnelConnectString(cfgStr, tunnelConnectString))
    {
        ParseTunnelPath(tunnelConnectString, udsRaw);
    }
    else if (!cfgStr.empty())
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
        else
        {
            udsRaw = GetDefaultUdsRawPath();
        }
    }
    if (udsRaw.empty())
    {
        udsRaw = cfgStr.empty() ? GetUniqueEndpointUdsRawPath() : GetDefaultUdsRawPath();
    }
    
    if (tunnelConnectString.empty())
    {
        tunnelConnectString = cfgStr;
    }
    
    std::string tunnel;
    if (!ExtractTunnelName(tunnelConnectString, tunnel))
    {
        // fallback for AppConnect (config minimal)
        sdv::toml::CTOMLParser parser(cfgStr);
        const std::string cfgName = parser.GetDirect("IpcChannel.Name").GetValue();

        if (!cfgName.empty())
        {
            tunnel = cfgName; 
            SDV_LOG_INFO("[AF_UNIX][Tunnel] Using channel name as tunnel name: ", tunnel);
        }
        else
        {
            // fallback 
            tunnel = "default";
            SDV_LOG_WARNING("[AF_UNIX][Tunnel] Missing tunnel and channel name, using default");
        }
    }

    std::string base = ExpandEnvVars("%TEMP%\\sdv\\");
    CreateDirectoryA(base.c_str(), nullptr);

    std::string dir = base + tunnel + "\\";
    CreateDirectoryA(dir.c_str(), nullptr);

    std::string udsPathBase = dir + NormalizeUdsPathForWindows(udsRaw);

    SDV_LOG_INFO("[AF_UNIX][Tunnel] endpoint udsPath=", udsPathBase);

    SOCKET listenSocket = CreateUnixListenSocket(udsPathBase);
    if (listenSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Failed to create listen socket for endpoint: ", udsPathBase);
        return ep;
    }

    auto serverTransport = std::make_shared<CWinsockConnection>(static_cast<unsigned long long>(listenSocket), true);
    uint32_t chId = g_nextChannelId++;
    auto serverTunnel = std::make_shared<CWinTunnelConnection>(serverTransport, /*channelId*/ static_cast<uint16_t>(chId));
    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!serverTunnel)
    {
        return ep;
    }
    // Retain shared_ptr to keep object alive for the duration of the connection.
    // This ensures shared_from_this() works correctly in DestroyObject().
    {
        std::lock_guard<std::mutex> lock(m_ConnectionsMutex);
        m_ptrConnections[serverTunnel.get()] = serverTunnel;
    }

    serverTunnel->SetWatchDogRemoveCallback([this](const void* connection)
    {
        m_watchdog.RemoveConnection(connection);
        // Remove the retained shared_ptr when connection is destroyed
        std::lock_guard<std::mutex> lock(m_ConnectionsMutex);
        m_ptrConnections.erase(connection);
    });

    m_watchdog.AddConnection(serverTunnel);

    ep.pConnection     = static_cast<sdv::IInterfaceAccess*>(serverTunnel.get());

    const std::string clientConnectString = "proto=tunnel;path=" + udsPathBase + ";tunnel=" + tunnel + ";";

    // Publish raw connect string (cleaner and avoids TOML escaping issues on Windows)
    ep.ssConnectString = clientConnectString;

    return ep;
}

sdv::IInterfaceAccess* CSocketsTunnelChannelMgnt::Access(const sdv::u8string& cs)
{
    std::string tunnelConnectString;
    const std::string input = static_cast<std::string>(cs);

    // Operational metadata, not part of the tunnel connect string itself
    const bool isServer = (input.find("role=server") != std::string::npos);

    // Parse only the real connect/config part
    std::string parseInput = input;
    const auto rolePos = parseInput.find(";role=");
    if (rolePos != std::string::npos)
    {
        const auto roleEnd = parseInput.find(';', rolePos + 1);
        if (roleEnd != std::string::npos)
        {
            parseInput.erase(rolePos, roleEnd - rolePos + 1);
        }
        else
        {
            parseInput.erase(rolePos);
        }
    }
 
    if (!ExtractTunnelConnectString(parseInput, tunnelConnectString))
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Invalid tunnel connect/config string");
        return nullptr;
    }

    std::string udsRaw;
    ParseTunnelPath(tunnelConnectString, udsRaw);
    SDV_LOG_INFO("[AF_UNIX][Tunnel] Access requested with connect string: ",
                 tunnelConnectString, ", extracted path: ", udsRaw);

    if (udsRaw.empty())
    {
        udsRaw = GetDefaultUdsRawPath();
        SDV_LOG_INFO("[AF_UNIX][Tunnel] No path specified, using default: ", udsRaw);
    }

    std::string tunnel;

    if (!ExtractTunnelName(tunnelConnectString, tunnel))
    {
        // fallback: derive from filename
        std::string path;
        ParseTunnelPath(tunnelConnectString, path);

        // extract filename
        auto pos = path.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? path.substr(pos + 1) : path;

        // remove ".sock"
        auto dot = filename.rfind(".sock");
        if (dot != std::string::npos)
        {
            tunnel = filename.substr(0, dot);
        }
        else
        {
            tunnel = filename;
        }

        if (!tunnel.empty())
        {
            SDV_LOG_INFO("[AF_UNIX][Tunnel] Derived tunnel from filename: ", tunnel);
        }
    }

    if (tunnel.empty())
    {
        SDV_LOG_ERROR("[AF_UNIX][Tunnel] Missing required tunnel name");
        return nullptr;
    }

    std::string base = ExpandEnvVars("%TEMP%\\sdv\\");
    CreateDirectoryA(base.c_str(), nullptr);

    std::string dir = base + tunnel + "\\";
    CreateDirectoryA(dir.c_str(), nullptr);

    std::string udsPathBase = dir + NormalizeUdsPathForWindows(udsRaw);
    SDV_LOG_INFO("[AF_UNIX][Tunnel] Access udsPath=", udsPathBase);

    std::shared_ptr<CWinTunnelConnection> connection;
    if (isServer)
    {
        SOCKET listenSocket = CreateUnixListenSocket(udsPathBase);
        if (listenSocket == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("[AF_UNIX][Tunnel] Failed to create server socket for ", udsPathBase);
            return nullptr;
        }

        auto serverTransport = std::make_shared<CWinsockConnection>(
            static_cast<unsigned long long>(listenSocket), true);
        uint32_t chId = g_nextChannelId++;
        connection = std::make_shared<CWinTunnelConnection>(
            serverTransport, static_cast<uint16_t>(chId));
    }
    else
    {
        SOCKET s = ConnectUnixSocket(udsPathBase, 5000, 50);
        if (s == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("[AF_UNIX][Tunnel] Failed to connect client socket for ", udsPathBase);
            return nullptr;
        }

        SDV_LOG_INFO("[AF_UNIX][Tunnel] Access -> CREATE CLIENT for ", udsPathBase);
        auto clientTransport = std::make_shared<CWinsockConnection>(s, false);
        uint32_t chId = g_nextChannelId++;
        connection = std::make_shared<CWinTunnelConnection>(
            clientTransport, static_cast<uint16_t>(chId));
    }

    if (!connection)
    {
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(m_ConnectionsMutex);
        m_ptrConnections[connection.get()] = connection;
    }

    connection->SetWatchDogRemoveCallback([this](const void* instance)
    {
        m_watchdog.RemoveConnection(instance);
        std::lock_guard<std::mutex> lock(m_ConnectionsMutex);
        m_ptrConnections.erase(instance);
    });

    m_watchdog.AddConnection(connection);
    return static_cast<sdv::IInterfaceAccess*>(connection.get());
}
#endif