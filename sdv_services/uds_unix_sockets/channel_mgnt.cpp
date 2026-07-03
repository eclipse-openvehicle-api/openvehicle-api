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
#include "connection.h"

#include <support/toml.h>

#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

// Anonymous namespace for internal helpers
namespace
{
    /**
    * @brief Parse a semicolon-separated list of key=value pairs
    *
    * Expected input format:
    *    "key1=value1;key2=value2;key3=value3;"
    *
    * Whitespace is not trimmed and empty entries are ignored
    * Keys without '=' are skipped
    *
    * Example:
    *    Input:  "proto=uds;path=/tmp/test.sock;"
    *    Output: { {"proto","uds"}, {"path","/tmp/test.sock"} }
    *
    * @param[in] s   Raw string containing "key=value;" pairs
    *
    * @return A map of parsed key/value pairs (order not preserved)
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
    * @brief Clamp an AF_UNIX pathname to Linux `sockaddr_un::sun_path` size
    *
    * Linux allows paths up to ~108 bytes inside `sun_path`
    * If the input string exceeds the allowed size, it is truncated so that:
    *    resulting_length <= sizeof(sockaddr_un::sun_path) - 1
    *
    * @param[in] p  Original path string
    *
    * @return A safe, clamped path that fits inside `sun_path`
    */
    static std::string ClampSunPath(const std::string& p)
    {
        constexpr size_t MaxLen = sizeof(sockaddr_un::sun_path);
        return (p.size() < MaxLen) ? p : p.substr(0, MaxLen - 1);
    }
}

// Directory selection (/run/user/<uid>/sdv or /tmp/sdv)
std::string CUnixDomainSocketsChannelMgnt::MakeUserRuntimeDir()
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

bool CUnixDomainSocketsChannelMgnt::OnInitialize()
{
    return true;
}

void CUnixDomainSocketsChannelMgnt::OnShutdown()
{}

void CUnixDomainSocketsChannelMgnt::OnDestroy()
{
    m_watchdog.Clear();
}

// Endpoint creation (server)
sdv::ipc::SChannelEndpoint CUnixDomainSocketsChannelMgnt::CreateEndpoint(const sdv::u8string& ssEndpointConfig)
{
    const std::string baseDir = MakeUserRuntimeDir();
    std::string name = "UDS_" + std::to_string(::getpid());
    std::string path = baseDir + "/" + name + ".sock";

    if (!ssEndpointConfig.empty())
    {
        sdv::toml::CTOMLParser cfg(ssEndpointConfig.c_str());

        auto nameNode = cfg.GetDirect("IpcChannel.Name");
        if (nameNode.GetType() == sdv::toml::ENodeType::node_string)
            name = static_cast<std::string>(nameNode.GetValue());

        auto pathNode = cfg.GetDirect("IpcChannel.Path");
        if (pathNode.GetType() == sdv::toml::ENodeType::node_string)
            path = static_cast<std::string>(pathNode.GetValue());
        else
            path = baseDir + "/" + name + ".sock";
    }

    path = ClampSunPath(path);

    // Keep lifetime consistent with shared_mem: watchdog owns active connections.
    std::shared_ptr<CUnixSocketConnection> server = std::make_shared<CUnixSocketConnection>(-1, true, path);
    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!server)
        return {};
    server->SetWatchDogRemoveCallback([this](const void* connection)
    {
        m_watchdog.RemoveConnection(connection);
    });
    m_watchdog.AddConnection(server);

    sdv::ipc::SChannelEndpoint ep{};
    ep.pConnection = static_cast<IInterfaceAccess*>(server.get());

    // Publish a Provider-wrapped connect string for compatibility with
    // CCommunicationControl::CreateClientConnection() flows.
    const std::string udsConnectString = server->GetConnectionString();
    ep.ssConnectString = std::string("[Provider]\n") +
        "Name = \"unix_domain_sockets\"\n" +
        "ConnectString = \"" + udsConnectString + "\"\n";
    return ep;
}

// Access existing endpoint (server or client)
sdv::IInterfaceAccess* CUnixDomainSocketsChannelMgnt::Access(const sdv::u8string& ssConnectString)
{
    const std::string input = static_cast<std::string>(ssConnectString);

    bool isServer = false;
    bool parsed = false;
    std::string path;

    // Parse structured TOML forms first, but only for non-raw inputs.
    // Raw connect strings (proto=uds;...) are intentionally not TOML and
    // would produce noisy parser diagnostics like "Missing value".
    if (input.rfind("proto=uds", 0) != 0)
    {
        sdv::toml::CTOMLParser parser(input);
        if (!parser.IsValid())
            return nullptr;

        const std::string providerName = parser.GetDirect("Provider.Name").GetValue();
        if (!providerName.empty())
        {
            if (providerName != "unix_domain_sockets" &&
                providerName != "UnixSocketsChannelControl" &&
                providerName != "UnixDomainSocketsChannelControl")
            {
                return nullptr;
            }

            const std::string nested = parser.GetDirect("Provider.ConnectString").GetValue();
            if (!nested.empty())
            {
                const auto kv = ParseKV(nested);
                if (kv.count("path"))
                    path = kv.at("path");
            }

            // Provider descriptions are consumed as client endpoints.
            parsed = true;
            isServer = false;
        }
        else
        {
            // Shared-memory style callers pass [IpcChannel] config directly to Access().
            parsed = true;
            isServer = false;
        }

        if (path.empty())
        {
            auto pathNode = parser.GetDirect("IpcChannel.Path");
            if (pathNode.GetType() == sdv::toml::ENodeType::node_string)
            {
                path = static_cast<std::string>(pathNode.GetValue());
            }
            else
            {
                auto nameNode = parser.GetDirect("IpcChannel.Name");
                if (nameNode.GetType() == sdv::toml::ENodeType::node_string)
                {
                    const std::string name = static_cast<std::string>(nameNode.GetValue());
                    if (!name.empty())
                        path = MakeUserRuntimeDir() + "/" + name + ".sock";
                }
            }
        }
    }

    // Raw KV fallback (legacy/tests): proto=uds;role=...;path=...;
    if (!parsed)
    {
        // Only treat as raw format when it actually starts with proto=uds.
        if (input.rfind("proto=uds", 0) != 0)
            return nullptr;

        const auto kv = ParseKV(input);
        parsed = true;
        isServer = (kv.count("role") && kv.at("role") == "server");
        if (kv.count("path"))
            path = kv.at("path");
    }

    if (!parsed)
        return nullptr;

    if (path.empty())
        path = MakeUserRuntimeDir() + "/UDS_auto.sock";

    path = ClampSunPath(path);

    std::shared_ptr<CUnixSocketConnection> connection =
        std::make_shared<CUnixSocketConnection>(-1, isServer, path);
    // Ignore cppcheck warning; if construction failed, an exception is expected first.
    // cppcheck-suppress knownConditionTrueFalse
    if (!connection)
        return nullptr;

    connection->SetWatchDogRemoveCallback([this](const void* instance)
    {
        m_watchdog.RemoveConnection(instance);
    });

    m_watchdog.AddConnection(connection);
    return static_cast<IInterfaceAccess*>(connection.get());
}

#endif // defined(__unix__)