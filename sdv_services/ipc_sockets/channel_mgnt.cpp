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
#include <array>

// Resolve conflict
#pragma pop_macro("GetObject")
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

/**
* Define for the connection  string
*/
#define SHARED_SOCKET "SHARED_SOCKET"

void CSocketsChannelMgnt::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    if (m_eObjectStatus != sdv::EObjectStatus::initialization_pending)
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
    else
        m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CSocketsChannelMgnt::GetStatus() const
{
    return m_eObjectStatus;
}

void CSocketsChannelMgnt::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::configuring;
        break;
    case sdv::EOperationMode::running:
        if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::running;
        break;
    default:
        break;
    }
}

void CSocketsChannelMgnt::Shutdown()
{
    //m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;
    //...
    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::ipc::SChannelEndpoint CSocketsChannelMgnt::CreateEndpoint(const sdv::u8string& /*ssChannelConfig*/)
{
    StartUpWinSock();
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family	  = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags	  = AI_PASSIVE;

    SOCKET listenSocket = CreateSocket(hints);

    if (listenSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("CreateSocket failed (could not create endpoint)");
        sdv::ipc::SChannelEndpoint connectionEndpoint{};
        return connectionEndpoint;
    }

    CConnection* pRemoteIPCConnection = new CConnection(listenSocket, true);

    uint16_t	port			  = GetPort(listenSocket);
    std::string ipcCompleteConfig = "localhost";
    ipcCompleteConfig.append(";");
    ipcCompleteConfig.append(std::to_string(port));
    SDV_LOG_INFO("IPC command param: '", ipcCompleteConfig, "'");

    sdv::ipc::SChannelEndpoint connectionEndpoint{};
    connectionEndpoint.pConnection	   = static_cast<IInterfaceAccess*>(pRemoteIPCConnection);
    connectionEndpoint.ssConnectString = ipcCompleteConfig;

    return connectionEndpoint;
}


sdv::IInterfaceAccess* CSocketsChannelMgnt::Access(const sdv::u8string& ssConnectString)
{
    bool sharedSocketRequired = ssConnectString.find(SHARED_SOCKET) != std::string::npos ? true : false;

    if (sharedSocketRequired)
    {
        std::string base64Data(ssConnectString);
        const std::string ext(SHARED_SOCKET);
        base64Data = base64Data.substr(0, base64Data.size() - ext.size());

        WSAPROTOCOL_INFO socketInfo = DecodeBase64<WSAPROTOCOL_INFO>(base64Data);

        SOCKET sharedSocket = WSASocket(0, 0, 0, &socketInfo, 0, 0);
        std::string success = "Socket sharing success";
        if (sharedSocket == INVALID_SOCKET)
        {
            success = "Socket sharing did not work!";
        }

        return static_cast<IInterfaceAccess*>(new CConnection(sharedSocket, false));
    }

    StartUpWinSock();
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family	  = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags	  = AI_PASSIVE;

    std::string host{"localhost"};
    std::string param{ssConnectString};
    auto		it = param.find(";");
    if (it != std::string::npos)
    {
        host  = param.substr(0, it);
        param = param.substr(it + 1, param.size() - it - 1);
    }

    SOCKET socket = CreateAndConnectToExistingSocket(hints, host.c_str(), param.c_str());
    if (socket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("Could  not create my socket and connect to the existing socket.");
    }

    return static_cast<IInterfaceAccess*>(new CConnection(socket, false));
}

uint16_t CSocketsChannelMgnt::GetPort(SOCKET socket) const
{
    sockaddr_in sockAddr;
    sockAddr.sin_port = 0;
    int nameLength	  = sizeof(sockAddr);
    getsockname(socket, reinterpret_cast<sockaddr*>(&sockAddr), &nameLength);
    return ntohs(sockAddr.sin_port);
}

SOCKET CSocketsChannelMgnt::CreateAndConnectToExistingSocket(const addrinfo& hints,
    const char*	  hostName,
    const char*	  portName)
{
    SOCKET invalidSocket{INVALID_SOCKET};

    // Resolve the server address and port
    CAddrInfo result;
    int		  error = getaddrinfo(hostName, portName, &hints, &result.AddressInfo);
    if (error != 0)
    {

        SDV_LOG_ERROR("getaddrinfo failed with error: ",
            std::to_string(error),
            " host: ",
            hostName,
            " port: ",
            portName);
        return invalidSocket;
    }

    SOCKET connectSocket{INVALID_SOCKET};
    // Attempt to connect to an address until one succeeds
    for (addrinfo* ptr = result.AddressInfo; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("socket failed with error: ", std::to_string(error));
            return invalidSocket;
        }

        // Connect to server.
        error = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (error == SOCKET_ERROR)
        {
            SDV_LOG_ERROR("connect failed with error: ", std::to_string(error));
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    return connectSocket;
}

SOCKET CSocketsChannelMgnt::CreateSocket(const addrinfo& hints)
{
    static constexpr const char* defaultPort_0{"0"}; // In case a defined port is required
    SOCKET						 invalidSocket{INVALID_SOCKET};
    CAddrInfo					 result;
    int							 error = getaddrinfo(NULL, defaultPort_0, &hints, &result.AddressInfo);
    if (error != 0)
    {
        SDV_LOG_ERROR("getaddrinfo failed with error: ", std::to_string(error));
        return invalidSocket;
    }

    SOCKET connectSocket{INVALID_SOCKET};
    connectSocket =
        socket(result.AddressInfo->ai_family, result.AddressInfo->ai_socktype, result.AddressInfo->ai_protocol);
    if (connectSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("error at socket(): ", std::to_string(error));
        return invalidSocket;
    }

    error = bind(connectSocket, result.AddressInfo->ai_addr, (int)result.AddressInfo->ai_addrlen);
    if (error == SOCKET_ERROR)
    {
        closesocket(connectSocket);
        SDV_LOG_ERROR("bind failed with error: ", std::to_string(error));
        return invalidSocket;
    }

    if (listen(connectSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(connectSocket);
        SDV_LOG_ERROR("listen failed with error: ", std::to_string(WSAGetLastError()));
        return invalidSocket;
    }

    // Change the socket mode on the listening socket from blocking to
    // non-block so the application will not block waiting for requests
    u_long NonBlock = 1;
    if (ioctlsocket(connectSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        closesocket(connectSocket);
        SDV_LOG_ERROR("ioctlsocket failed with error: ", std::to_string(WSAGetLastError()));
        return invalidSocket;
    }

    return connectSocket;
}


SOCKET CSocketsChannelMgnt::CreateAndConnectToSocket(const addrinfo& hints, const char* defaultHost, const char* defaultPort)
{
    SOCKET ConnectSocket{ INVALID_SOCKET };

    // Resolve the server address and port
    CAddrInfo result;
    if (getaddrinfo(defaultHost, defaultPort, &hints, &result.AddressInfo) != 0)
    {
        SDV_LOG_ERROR("getaddrinfo() failed: ", std::to_string(WSAGetLastError()));
        return ConnectSocket;
    }

    // Attempt to connect to an address until one succeeds
    for (addrinfo* ptr = result.AddressInfo; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            SDV_LOG_ERROR("creating SOCKET for connecting failed: ", std::to_string(WSAGetLastError()));
            break;
        }

        // Connect to server.
        if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
        {
            SDV_LOG_ERROR("connect to servcer failed: ", std::to_string(WSAGetLastError()));
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("Failed to create valid sockaet in CreateAndConnectToSocket()");
    }

    return ConnectSocket;
}

SOCKET CSocketsChannelMgnt::Listen(const addrinfo& hints, uint32_t port)
{
    SOCKET listenSocket = INVALID_SOCKET;
    CAddrInfo result;
    int error = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result.AddressInfo);
    if (error != 0)
    {
        SDV_LOG_ERROR("getaddrinfo() failed: ", std::to_string(WSAGetLastError()));
        return listenSocket;
    }

    listenSocket = socket(result.AddressInfo->ai_family, result.AddressInfo->ai_socktype, result.AddressInfo->ai_protocol);
    if (listenSocket == INVALID_SOCKET)
    {
        SDV_LOG_ERROR("2creating SOCKET for connecting failed:  failed: ", std::to_string(WSAGetLastError()));
        return listenSocket;
    }

    error = bind(listenSocket, result.AddressInfo->ai_addr, (int)result.AddressInfo->ai_addrlen);
    if (error == SOCKET_ERROR)
    {
        SDV_LOG_ERROR("bind failed with error: ", std::to_string(WSAGetLastError()));
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return listenSocket;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        SDV_LOG_ERROR("listen to SOCKET failed: ", std::to_string(WSAGetLastError()));
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return listenSocket;
    }

    return listenSocket;
}


SocketConnection CSocketsChannelMgnt::CreateConnectedSocketPair()
{
    SocketConnection connection;

    uint32_t port = 0;
    SOCKET listenSocket = Listen(getHints, port);

    uint16_t portOfListenSocket = GetPort(listenSocket);
    auto future = std::async([listenSocket]() { return accept(listenSocket, NULL, NULL); });

    connection.From = CreateAndConnectToSocket(getHints, "localhost", std::to_string(portOfListenSocket).c_str());

    // Future::Get has to be called after the CreateAndConnect-Function
    connection.To = future.get();
    return connection;
}
