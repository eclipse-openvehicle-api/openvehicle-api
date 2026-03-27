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
    std::ostringstream oss;
    oss << "/run/user/" << ::getuid();

    struct stat st{};
    if (::stat(oss.str().c_str(), &st) == 0)
    {
        std::string path = oss.str() + "/sdv";
        ::mkdir(path.c_str(), 0770);
        return path;
    }

    ::mkdir("/tmp/sdv", 0770);
    return "/tmp/sdv";
}

bool CUnixDomainSocketsChannelMgnt::OnInitialize()
{
    return true;
}

void CUnixDomainSocketsChannelMgnt::OnShutdown()
{
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

    // Use a shared_ptr and store it to keep the server connection alive
    auto server = std::make_shared<CUnixSocketConnection>(-1, true, path);
    m_ServerConnections.push_back(server);

    sdv::ipc::SChannelEndpoint ep{};
    ep.pConnection = static_cast<IInterfaceAccess*>(server.get());
    ep.ssConnectString = server->GetConnectionString();
    return ep;
}

// Access existing endpoint (server or client)
sdv::IInterfaceAccess* CUnixDomainSocketsChannelMgnt::Access(const sdv::u8string& ssConnectString)
{
    const auto kv = ParseKV(static_cast<std::string>(ssConnectString));
    const bool isServer = (kv.count("role") && kv.at("role") == "server");
    const std::string path = kv.count("path") ? kv.at("path") : (MakeUserRuntimeDir() + "/UDS_auto.sock");

    if (isServer)
    {
        auto server = std::make_shared<CUnixSocketConnection>(-1, true, path);
        m_ServerConnections.push_back(server);
        return static_cast<IInterfaceAccess*>(server.get());
    }

    // Client: allocated raw pointer (expected to be managed by SDV framework)
    auto* client = new CUnixSocketConnection(-1, false, path);
    return static_cast<IInterfaceAccess*>(client);
}

#endif // defined(__unix__)