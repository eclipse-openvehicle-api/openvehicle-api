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

} // anonymous namespace

std::string CUnixTunnelChannelMgnt::MakeUserRuntimeDir()
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

bool CUnixTunnelChannelMgnt::OnInitialize()
{
    return true;
}

void CUnixTunnelChannelMgnt::OnShutdown()
{
    // Actual cleanup is handled by destructors of CUnixTunnelConnection
    // and CUnixSocketConnection (shared_ptr).
}

sdv::ipc::SChannelEndpoint CUnixTunnelChannelMgnt::CreateEndpoint(
    const sdv::u8string& ssChannelConfig)
{
    sdv::ipc::SChannelEndpoint endpoint{};

    const std::string baseDir = MakeUserRuntimeDir();
    std::string name = "TUNNEL_" + std::to_string(::getpid());
    std::string path = baseDir + "/" + name + ".sock";

    // Parse optional TOML config for custom name/path
    if (!ssChannelConfig.empty())
    {
        sdv::toml::CTOMLParser cfg(ssChannelConfig.c_str());
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

    // Create underlying UDS server transport
    auto udsServer = std::make_shared<CUnixSocketConnection>(
        -1,
        /*acceptConnectionRequired*/ true,
        path);

    // Create tunnel wrapper on top of UDS
    auto tunnelServer = std::make_shared<CUnixTunnelConnection>(
        udsServer,
        /*channelId*/ 0u);

    m_ServerTunnels.push_back(tunnelServer);

    endpoint.pConnection = static_cast<sdv::IInterfaceAccess*>(tunnelServer.get());
    endpoint.ssConnectString = "proto=tunnel;role=server;path=" + path + ";";

    return endpoint;
}

sdv::IInterfaceAccess* CUnixTunnelChannelMgnt::Access(
    const sdv::u8string& ssConnectString)
{
    const auto kv = ParseKV(static_cast<std::string>(ssConnectString));

    // Only handle proto=tunnel
    if (!kv.count("proto") || kv.at("proto") != "tunnel")
    {
        return nullptr;
    }

    const bool isServer =
        (kv.count("role") && kv.at("role") == "server");

    const std::string path =
        kv.count("path")
        ? kv.at("path")
        : (MakeUserRuntimeDir() + "/TUNNEL_auto.sock");

    if (isServer)
    {
        // For simplicity, create a new server tunnel instance for each Access().
        // The SDV framework is expected to call Access(serverCS) only once in normal cases.
        auto udsServer = std::make_shared<CUnixSocketConnection>(
            -1,
            /*acceptConnectionRequired*/ true,
            path);

        auto tunnelServer = std::make_shared<CUnixTunnelConnection>(
            udsServer,
            /*channelId*/ 0u);

        m_ServerTunnels.push_back(tunnelServer);
        return static_cast<sdv::IInterfaceAccess*>(tunnelServer.get());
    }

    // Client: allocate raw pointer (expected to be managed by SDV framework via IObjectDestroy)
    auto udsClient = std::make_shared<CUnixSocketConnection>(
        -1,
        /*acceptConnectionRequired*/ false,
        path);

    auto* tunnelClient =
        new CUnixTunnelConnection(udsClient, /*channelId*/ 0u);

    return static_cast<sdv::IInterfaceAccess*>(tunnelClient);
}

#endif // defined(__unix__)