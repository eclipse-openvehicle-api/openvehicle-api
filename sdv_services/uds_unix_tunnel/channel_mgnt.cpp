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

#include "channel_mgnt.h"
#include "connection.h"   // CUnixTunnelConnection
#include "../sdv_services/uds_unix_sockets/connection.h" // CUnixSocketConnection

#include <support/toml.h>

#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace
{
static std::atomic<uint32_t> g_nextChannelId{1};

/**
 * @brief Parses a semicolon-separated list of key=value pairs into a map.
 *
 * Example input: "proto=tunnel;role=server;path=/tmp/tunnel.sock;"
 * Example output: { {"proto","tunnel"}, {"role","server"}, {"path","/tmp/tunnel.sock"} }
 *
 * @param s The input string to parse.
 * @return Map of key-value pairs.
 */
static std::map<std::string, std::string> ParseKV(const std::string& s)
{
    std::map<std::string, std::string> kv;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ';'))
    {
        auto pos = item.find('=');
        if (pos != std::string::npos)
            kv[item.substr(0, pos)] = item.substr(pos + 1);
    }
    return kv;
}


/**
 * @brief Clamps an AF_UNIX pathname to the maximum allowed size for sockaddr_un::sun_path.
 *
 * @param p The path to clamp.
 * @return The clamped path string.
 */
static std::string ClampSunPath(const std::string& p)
{
    constexpr size_t MaxLen = sizeof(sockaddr_un::sun_path);
    return (p.size() < MaxLen) ? p : p.substr(0, MaxLen - 1);
}

static std::string GetUserRuntimeDir()
{
    const std::string path = "/run/ipc/sdv";

    struct stat st{};
    if (::stat(path.c_str(), &st) == 0)
    {
        return path;
    }

    // fallback if /run/ipc/sdv is not available
    const std::string fallback = "/tmp/sdv";
    ::mkdir(fallback.c_str(), 0770);

    return fallback;
}


static std::string BuildTunnelPath(const std::string& baseDir,
                                   const std::string& tunnel,
                                   uint32_t instanceId,
                                   uint32_t pid)
{
    std::string dir = baseDir + "/" + tunnel;
 
    if (::mkdir(dir.c_str(), 0770) != 0 && errno != EEXIST)
    {
        SDV_LOG_WARNING("Failed to create tunnel directory");
    }

    return dir + "/vapi_" +
           std::to_string(instanceId) + "_" +
           std::to_string(pid) + ".sock";
}

static bool ExtractTunnelPathFromConfig(const std::string& input, std::string& outPath)
{
    const auto kv = ParseKV(input);
    if (kv.count("proto"))
    {
        if (kv.at("proto") != "tunnel")
            return false;

        auto it = kv.find("path");
        outPath = (it != kv.end()) ? it->second : std::string{};
        return true;
    }

    sdv::toml::CTOMLParser parser(input);
    if (!parser.IsValid())
        return false;

    const std::string providerName = parser.GetDirect("Provider.Name").GetValue();
    if (!providerName.empty() &&
        providerName != "unix_domain_sockets_tunnel" &&
        providerName != "UnixTunnelChannelControl" &&
        providerName != "WinTunnelChannelControl")
    {
        return false;
    }

    const std::string nested = parser.GetDirect("Provider.ConnectString").GetValue();
    if (!nested.empty())
    {
        return ExtractTunnelPathFromConfig(nested, outPath);
    }

    auto pathNode = parser.GetDirect("IpcChannel.Path");
    if (pathNode.GetType() == sdv::toml::ENodeType::node_string)
    {
        outPath = static_cast<std::string>(pathNode.GetValue());
        return true;
    }

    auto nameNode = parser.GetDirect("IpcChannel.Name");
    if (nameNode.GetType() == sdv::toml::ENodeType::node_string)
    {
        const std::string name = static_cast<std::string>(nameNode.GetValue());
        if (!name.empty())
        {
            outPath = GetUserRuntimeDir() + "/" + name + ".sock";
            return true;
        }
    }

    outPath.clear();
    return true;
}

static std::string ExtractTunnelName(const std::string& input)
{
    const auto kv = ParseKV(input);
    if (kv.count("tunnel"))
    {
        return kv.at("tunnel");
    }

    // Fallback default
    return "default";
}


} // anonymous namespace

std::string CUnixTunnelChannelMgnt::MakeUserRuntimeDir()
{
    return GetUserRuntimeDir();
}

bool CUnixTunnelChannelMgnt::OnInitialize()
{
    return true;
}

void CUnixTunnelChannelMgnt::OnShutdown()
{}

void CUnixTunnelChannelMgnt::OnDestroy()
{
    m_watchdog.Clear();
}

sdv::ipc::SChannelEndpoint CUnixTunnelChannelMgnt::CreateEndpoint(
    const sdv::u8string& ssChannelConfig)
{
    sdv::ipc::SChannelEndpoint endpoint{};

    const std::string baseDir = MakeUserRuntimeDir();
    uint32_t instanceId = 1000;
    const sdv::app::IAppContext* pCtx = sdv::core::GetCore<sdv::app::IAppContext>();
    if (pCtx && pCtx->GetInstanceID() != 0)
    {
        instanceId = pCtx->GetInstanceID();
    }

    // Extract tunnel name from config
    std::string input = static_cast<std::string>(ssChannelConfig);
    std::string tunnel = ExtractTunnelName(input);

    // Build safe path
    std::string path = BuildTunnelPath(baseDir, tunnel, instanceId, static_cast<uint32_t>(::getpid()));

    if (!ssChannelConfig.empty())
    {
        std::string configuredPath;

        if (ExtractTunnelPathFromConfig(input, configuredPath) && !configuredPath.empty())
        {
            std::string baseName = configuredPath;
            auto pos = baseName.find_last_of('/');
            if (pos != std::string::npos)
            {
                baseName = baseName.substr(pos + 1);
            }
            path = baseDir + "/" + tunnel + "/" + baseName;
            SDV_LOG_WARNING("Using custom path without tunnel isolation");
        }
        else
        {
            // fallback - still use tunnel-based path
            tunnel = ExtractTunnelName(input);
            instanceId = 1000;

            if (pCtx && pCtx->GetInstanceID() != 0)
            {
                instanceId = pCtx->GetInstanceID();
            }

            path = BuildTunnelPath(baseDir, tunnel, instanceId, static_cast<uint32_t>(::getpid()));
        }
    }

    path = ClampSunPath(path);

    // Create underlying UDS server transport
    auto udsServer = std::make_shared<CUnixSocketConnection>(-1, /*acceptConnectionRequired*/ true, path);

    uint32_t chId = g_nextChannelId++;
    // Create tunnel wrapper on top of UDS
    auto tunnelServer = std::make_shared<CUnixTunnelConnection>(udsServer,  /*channelId*/ chId);

    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!tunnelServer)
        return {};

    tunnelServer->SetWatchDogRemoveCallback([this](const void* connection)
    {
        m_watchdog.RemoveConnection(connection);
    });
    m_watchdog.AddConnection(tunnelServer);

    endpoint.pConnection = static_cast<sdv::IInterfaceAccess*>(tunnelServer.get());
    const std::string tunnelConnectString = "proto=tunnel;path=" + path + ";tunnel=" + tunnel + ";";
    endpoint.ssConnectString = std::string("[Provider]\n") +
        "Name = \"unix_domain_sockets_tunnel\"\n" +
        "ConnectString = \"" + tunnelConnectString + "\"\n";

    return endpoint;
}

sdv::IInterfaceAccess* CUnixTunnelChannelMgnt::Access(const sdv::u8string& ssConnectString)
{
    const std::string input = static_cast<std::string>(ssConnectString);
    std::string tunnel = ExtractTunnelName(input);
    bool parsed = false;
    bool isServer = false;
    std::string path;

    // Parse structured TOML forms first; raw connect strings are handled below.
    if (input.rfind("proto=tunnel", 0) != 0)
    {

        sdv::toml::CTOMLParser parser(input);
        if (!parser.IsValid())
        {
            SDV_LOG_WARNING("[TUNNEL][Access] TOML parse failed. Trying text fallback.");

            // Fallback for malformed TOML: try to recover an embedded proto=tunnel connect string.
            const auto protoPos = input.find("proto=tunnel");
            if (protoPos != std::string::npos)
            {
                std::string fallbackCs = input.substr(protoPos);

                const auto quotePos = fallbackCs.find('"');
                if (quotePos != std::string::npos)
                    fallbackCs = fallbackCs.substr(0, quotePos);

                const auto newlinePos = fallbackCs.find('\n');
                if (newlinePos != std::string::npos)
                    fallbackCs = fallbackCs.substr(0, newlinePos);

                const auto kv = ParseKV(fallbackCs);
                if (kv.count("proto") && kv.at("proto") == "tunnel")
                {
                    parsed = true;
                    isServer = (kv.count("role") && kv.at("role") == "server");
                    if (kv.count("path"))
                        path = kv.at("path");
                }
            }

            if (!parsed)
            {
                SDV_LOG_WARNING("[TUNNEL][Access] Fallback parse failed. Returning nullptr");
                return nullptr;
            }
        }

        if (!parsed)
        {
            const std::string providerName = parser.GetDirect("Provider.Name").GetValue();
            if (!providerName.empty())
            {
                if (providerName != "unix_domain_sockets_tunnel" &&
                    providerName != "UnixTunnelChannelControl" &&
                    providerName != "WinTunnelChannelControl")
                {
                    SDV_LOG_WARNING("[TUNNEL][Access] Unsupported provider. Returning nullptr");
                    return nullptr;
                }

                const std::string nested = parser.GetDirect("Provider.ConnectString").GetValue();
                if (!nested.empty())
                {
                    const auto nestedKv = ParseKV(nested);                  
                    if (nestedKv.count("tunnel"))
                    {
                        tunnel = nestedKv.at("tunnel");
                    }

                    if (nestedKv.count("proto") && nestedKv.at("proto") != "tunnel")
                    {
                        SDV_LOG_WARNING("[TUNNEL][Access] Nested proto is not tunnel. Returning nullptr");
                        return nullptr;
                    }

                    if (nestedKv.count("path"))
                    {                      
                        std::string baseName = nestedKv.at("path");
                        auto pos = baseName.find_last_of('/');
                        if (pos != std::string::npos)
                        {
                            baseName = baseName.substr(pos + 1);
                        }

                        path = MakeUserRuntimeDir() + "/" + tunnel + "/" + baseName;
                    }
                }

                parsed = true;
                isServer = false;
            }
            else
            {
                // Callers can pass [IpcChannel] directly to Access().
                parsed = true;
                isServer = false;
            }

            if (path.empty())
            {
                auto pathNode = parser.GetDirect("IpcChannel.Path");
                if (pathNode.GetType() == sdv::toml::ENodeType::node_string)
                {
                    std::string baseName = static_cast<std::string>(pathNode.GetValue());

                    auto pos = baseName.find_last_of('/');
                    if (pos != std::string::npos)
                    {
                        baseName = baseName.substr(pos + 1);
                    }

                    path = MakeUserRuntimeDir() + "/" + tunnel + "/" + baseName;
                }
                else
                {
                    auto nameNode = parser.GetDirect("IpcChannel.Name");
                    if (nameNode.GetType() == sdv::toml::ENodeType::node_string)
                    {
                        const std::string name = static_cast<std::string>(nameNode.GetValue());
                        if (!name.empty())
                        {
                            path = MakeUserRuntimeDir() + "/" + tunnel + "/" + name + ".sock";
                        }
                    }
                }
            }
        }
    }

    // Raw KV fallback: proto=tunnel;role=...;path=...;
    if (!parsed)
    {
        if (input.rfind("proto=tunnel", 0) != 0)
        {
            SDV_LOG_WARNING("[TUNNEL][Access] Raw KV parse rejected: input does not start with proto=tunnel. Returning nullptr");
            return nullptr;
        }

        const auto kv = ParseKV(input);
        if (!kv.count("proto") || kv.at("proto") != "tunnel")
        {
            SDV_LOG_WARNING("[TUNNEL][Access] Raw KV parse rejected: missing/invalid proto. Returning nullptr ");
            return nullptr;
        }

        parsed = true;
        isServer = (kv.count("role") && kv.at("role") == "server");
        if (kv.count("path"))
        {
            path = kv.at("path");
        }
    }

    if (!parsed)
    {
        return nullptr;
    }

    if (path.empty())
    {
        tunnel = ExtractTunnelName(input);

        uint32_t instanceId = 1000;
        const sdv::app::IAppContext* pCtx = sdv::core::GetCore<sdv::app::IAppContext>();
        if (pCtx && pCtx->GetInstanceID() != 0)
        {
            instanceId = pCtx->GetInstanceID();
        }

        path = BuildTunnelPath(MakeUserRuntimeDir(), tunnel, instanceId, static_cast<uint32_t>(::getpid()));
    }

    path = ClampSunPath(path);

    std::shared_ptr<CUnixSocketConnection> transport = std::make_shared<CUnixSocketConnection>(-1, /*acceptConnectionRequired*/ isServer, path);
    uint32_t chId = g_nextChannelId++;
    std::shared_ptr<CUnixTunnelConnection> connection = std::make_shared<CUnixTunnelConnection>(transport, /*channelId*/ chId);

    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!connection)
        return nullptr;

    connection->SetWatchDogRemoveCallback([this](const void* instance)
    {
        m_watchdog.RemoveConnection(instance);
    });

    m_watchdog.AddConnection(connection);
    return static_cast<sdv::IInterfaceAccess*>(connection.get());
}

#endif // defined(__unix__)