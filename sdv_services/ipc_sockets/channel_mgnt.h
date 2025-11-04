#ifndef CHANNEL_MGNT_H
#define CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

/**
* @brief The CAddrInfo structure is used by the getaddrinfo function to hold host address information.
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
    * @param[in] other Reference to the structure to move.
    */
    CAddrInfo(CAddrInfo&& other)           = delete;

    /**
    * @brief Move operator.
    * @param[in] other Reference to the structure to move.
    * @return Returns reference to CAddrInfo structure
    */
    CAddrInfo& operator=(CAddrInfo&& other) = delete;

    ~CAddrInfo()
    {
        freeaddrinfo(AddressInfo);
    }

    addrinfo* AddressInfo{nullptr}; ///< The CAddrInfo structure holding host address information.
};

/**
* @brief Initial startup of winSock
* @return Returns 0 in case of no error, otherwise the error code
*/
inline int StartUpWinSock()
{
    static bool isInitialized = false;
    if (isInitialized)
    {
        return 0;
    }

    WSADATA wsaData;
    int		error = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
}

/**
* @brief Holds to sockets.
* Used by the core process to create a connection between to shild processes
*/
struct SocketConnection
{
    SOCKET From{ INVALID_SOCKET }; ///< socket from child process
    SOCKET To{ INVALID_SOCKET };   ///< socket to child process
};


/**
 * @brief IPC channel management class for the shared memory communication.
 */
class CSocketsChannelMgnt : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::ipc::ICreateEndpoint,
    public sdv::ipc::IChannelAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::ipc::IChannelAccess)
        SDV_INTERFACE_ENTRY(sdv::ipc::ICreateEndpoint)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("DefaultSocketsChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("RemoteChannelControl")
    DECLARE_DEFAULT_OBJECT_NAME("RemoteChannelControl")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Create IPC connection object and return the endpoint information. Overload of
     * sdv::ipc::ICreateEndpoint::CreateEndpoint.
     * @details The endpoints are generated using either a size and a name based on the interface and port number provided through
     * the channel configuration or if no configuration is supplied a randomly generated size and name. The following configuration
     * can be supplied:
     * @code
     * [IpcChannel]
     * Interface = "127.0.0.1"
     * Port = 2000
     * @endcode
     * @param[in] ssChannelConfig Optional channel type specific endpoint configuration.
     * @return IPC connection object
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(/*in*/ const sdv::u8string& ssChannelConfig) override;

    /**
     * @brief Create a connection object from the channel connection parameters string
     * @param[in] ssConnectString Reference to the string containing the channel connection parameters.
     * @return Pointer to IInterfaceAccess interface of the connection object or NULL when the object cannot be created.
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& ssConnectString) override;

private:
    /**
    * @brief Creates a listen socket without setting the port and configured the socket to return directly without
    * blocking. No blocking access when data is received. Therefore any receive function must use polling
    * @return Returns the listen socket
    */
    SOCKET CreateSocket(const addrinfo& hints);

    /**
    * @brief get port number of a given socket
    * @param[in] socket the port number is requested
    * @return Returns the port number of the socket
    */
    uint16_t GetPort(SOCKET socket) const;

    SocketConnection CreateConnectedSocketPair();
    SOCKET Listen(const addrinfo& hints, uint32_t port);
    SOCKET CreateAndConnectToSocket(const addrinfo& hints, const char* defaultHost, const char* defaultPort);

    /**
    * @brief Creates an own socket and connects to an existing socket
    * @param[in] hints The CAddrInfo structure to create the  socket.
    * @param[in] hostName host name.
    * @param[in] portName port name.
    * @return Returns an socket
    */
    SOCKET CreateAndConnectToExistingSocket(const addrinfo& hints, const char* hostName, const char* portName);

    inline static const addrinfo getHints
    {
        []() constexpr {
            addrinfo hints{0, 0, 0, 0, 0, nullptr, nullptr, nullptr};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;
            hints.ai_next = nullptr;

            return hints;
        }()
    };

    sdv::EObjectStatus  m_eObjectStatus = sdv::EObjectStatus::initialization_pending;    ///< Object status.
};
DEFINE_SDV_OBJECT(CSocketsChannelMgnt)

#endif // ! defined CHANNEL_MGNT_H