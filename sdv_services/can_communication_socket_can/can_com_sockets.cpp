#include "can_com_sockets.h"

void CCANSockets::Initialize(const sdv::u8string& rssObjectConfig)
{
    std::deque<std::string> vecConfigInterfaces;
    try
    {
        sdv::toml::CTOMLParser config(rssObjectConfig.c_str());
        sdv::toml::CNodeCollection nodeSource = config.GetDirect("canSockets");
        if (nodeSource.GetType() == sdv::toml::ENodeType::node_array)
        {
            for (size_t nIndex = 0; nIndex < nodeSource.GetCount(); nIndex++)
            {
                sdv::toml::CNode nodeInterface = nodeSource[nIndex];
                if (nodeInterface.GetType() == sdv::toml::ENodeType::node_string)
                {
                    std::string ssIfcName = nodeInterface.GetValue();
                    if (!ssIfcName.empty())
                    {
                        vecConfigInterfaces.push_back(ssIfcName);
                    }
                }
            }
        }
        if (vecConfigInterfaces.size() == 0)
        {
            auto node = config.GetDirect("canSockets");
            if (node.GetType() == sdv::toml::ENodeType::node_string)
            {
                vecConfigInterfaces.push_back(node.GetValue());
            }
        }  
    }

    catch (const sdv::toml::XTOMLParseException& e)
    {
        SDV_LOG_ERROR("Configuration could not be read: ", e.what());
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
    
    if (vecConfigInterfaces.size() == 0)
    {
        SDV_LOG_WARNING("Configuration failure, no 'canSockets' param found");
    } 
   
    if (!SetupCANSockets(vecConfigInterfaces))
    {
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    m_threadReceive = std::thread(&CCANSockets::ReceiveThreadFunc, this);

    m_eStatus = sdv::EObjectStatus::initialized;
}

bool CCANSockets::SetupCANSockets(const std::deque<std::string>& vecConfigInterfaces)
{
    // Retrieve the list of available interfaces    
    std::set<std::string> availableInterfaces;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) != -1)
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            availableInterfaces.insert(ifa->ifa_name);
        }   
    }
    freeifaddrs(ifaddr);

    CreateAndBindSockets(vecConfigInterfaces, availableInterfaces);

    bool isSocketCreated = false;
    std::unique_lock<std::mutex> lock(m_mtxSockets);
    for (auto socket : m_vecSockets)
    {
        if ((socket.localSocket > 0) && (socket.networkInterface > 0))
        {
            isSocketCreated = true;
            SDV_LOG_INFO("Enabling CAN interface: ", socket.name);
        }
    }

    if (isSocketCreated)
    {
        return true;
    }

    SDV_LOG_ERROR(" Error: Socket list is empty.");
    return false;
}

void CCANSockets::CreateAndBindSockets(const std::deque<std::string>& vecConfigInterfaces, 
    const std::set<std::string>& availableInterfaces)
{
    for (const auto& configInterface : vecConfigInterfaces) 
    {
        SSocketDefinition socketDef;
        socketDef.name = "";
        socketDef.networkInterface = 0;
        socketDef.localSocket = -1;

        // Create a socket
        int localSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (localSocket == -1)
        {
            SDV_LOG_ERROR("Error creating Socket");
            return;
        }     

        if(availableInterfaces.find(configInterface) == availableInterfaces.end())
        {
            SDV_LOG_WARNING("Interface not found: ", configInterface);
            std::unique_lock<std::mutex> lock(m_mtxSockets);            
            m_vecSockets.push_back(socketDef);
            continue;
        }

        auto ifIndex = if_nametoindex(configInterface.c_str());
        socketDef.networkInterface = ifIndex;

        // Bind the socket to the specified CAN interface
        sockaddr_can sAddr = {};
        sAddr.can_family = AF_CAN;
        sAddr.can_ifindex = ifIndex;
        if (bind(localSocket, reinterpret_cast<sockaddr*>(&sAddr), sizeof(sAddr)) == -1)
        {
            SDV_LOG_ERROR("Error binding socket to interface ", configInterface);
            std::unique_lock<std::mutex> lock(m_mtxSockets);            
            m_vecSockets.push_back(socketDef);            
            close(localSocket);
            continue;
        }

        // Set socket to be non-blocking
        if (fcntl(localSocket, F_SETFL, O_NONBLOCK) == -1)
        {
            perror("fcntl - F_SETFL");
            SDV_LOG_ERROR("Error setting socket to be non-blocking:", configInterface);
            close(localSocket);
            std::unique_lock<std::mutex> lock(m_mtxSockets);               
            m_vecSockets.push_back(socketDef);
            continue;
        }

        // Store the successfully configured socket
        socketDef.name = configInterface;
        socketDef.localSocket = localSocket;
        std::unique_lock<std::mutex> lock(m_mtxSockets);
        m_vecSockets.push_back(socketDef);
    }
}

sdv::EObjectStatus CCANSockets::GetStatus() const
{
    return m_eStatus;
}

void CCANSockets::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eStatus == sdv::EObjectStatus::running || m_eStatus == sdv::EObjectStatus::initialized)
            m_eStatus = sdv::EObjectStatus::configuring;
        break;
    case sdv::EOperationMode::running:
        if (m_eStatus == sdv::EObjectStatus::configuring || m_eStatus == sdv::EObjectStatus::initialized)
            m_eStatus = sdv::EObjectStatus::running;
        break;
    default:
        break;
    }
}

void CCANSockets::Shutdown()
{
    m_eStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Wait until the receiving thread is finished.
    if (m_threadReceive.joinable())
        m_threadReceive.join();

    std::unique_lock<std::mutex> lock(m_mtxSockets);
    for (const auto& socket : m_vecSockets) 
    {
        if (socket.localSocket > 0)
        {
            close(socket.localSocket);  
        }
    }

    m_eStatus = sdv::EObjectStatus::destruction_pending;
}

void CCANSockets::RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    if (m_eStatus != sdv::EObjectStatus::configuring) return;
    if (!pReceiver) return;

    SDV_LOG_INFO("Registering VAPI CAN communication receiver...");

    std::unique_lock<std::mutex> lock(m_mtxReceivers);
    m_setReceivers.insert(pReceiver);
}

void CCANSockets::UnregisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    // NOTE: Normally the remove function should be called in the configuration mode. Since it doesn't give
    // feedback and the associated caller might delete any receiving function, allow the removal to take place even
    // when running.

    if (!pReceiver) return;

    SDV_LOG_INFO("Unregistering VAPI CAN communication receiver...");

    std::unique_lock<std::mutex> lock(m_mtxReceivers);
    m_setReceivers.erase(pReceiver);
}

sdv::sequence<sdv::u8string> CCANSockets::GetInterfaces() const
{
    sdv::sequence<sdv::u8string> seqIfcNames;
    if (m_eStatus != sdv::EObjectStatus::running) 
    {
        return seqIfcNames;
    }

    std::unique_lock<std::mutex> lock(m_mtxSockets);    
    for (const auto& rprInterface : m_vecSockets)
    {
        if (!rprInterface.name.empty())
        {
            seqIfcNames.push_back(rprInterface.name);
        }
    }

    return seqIfcNames;
}

void CCANSockets::Send(const sdv::can::SMessage& sMsg, uint32_t uiConfigIndex)
{
    if (m_eStatus != sdv::EObjectStatus::running) return;
    if (sMsg.bCanFd) return;  // CAN-FD not supported
    if (sMsg.seqData.size() > 8) return;  // Invalid message length.

    sockaddr_can sAddr{};
    can_frame sFrame{};

    // Convert the message structure from VAPI SMessage to SocketCAN can_frame
    memset(&sFrame,0, sizeof(sFrame));
    sFrame.can_dlc = sMsg.seqData.size();    
    if (sFrame.can_dlc > 8) 
        return;

    sFrame.can_id = sMsg.uiID;
    if (sMsg.bExtended)
    {
        sFrame.can_id |= CAN_EFF_FLAG;
    }

    for (uint32_t index = 0; index < sFrame.can_dlc; ++index)
    {
        sFrame.data[index] = sMsg.seqData[index];
    }

    std::unique_lock<std::mutex> lock(m_mtxSockets);

    auto it = m_vecSockets.begin();
    std::advance(it, uiConfigIndex); 
    if (it != m_vecSockets.end()) 
    {
        if ((it->localSocket > 0) && (it->networkInterface > 0))
        {
            sAddr.can_ifindex = it->networkInterface;
            sAddr.can_family  = AF_CAN;
            sendto(it->localSocket, &sFrame, sizeof(can_frame), 0, reinterpret_cast<sockaddr*>(&sAddr), sizeof(sAddr));
        }
    }
}

void CCANSockets::ReceiveThreadFunc()
{
    while (true)
    {
        enum {retry, cont, exit} eNextStep = exit;
        switch (m_eStatus)
        {
        case sdv::EObjectStatus::configuring:
        case sdv::EObjectStatus::initialization_pending:
        case sdv::EObjectStatus::initialized:
        case sdv::EObjectStatus::initializing:
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            eNextStep = retry;
            break;
        case sdv::EObjectStatus::running:
            eNextStep = cont;
            break;
        default:
            break;
        }
        if (eNextStep == exit) break;
        if (eNextStep == retry) continue;

        sockaddr_can sAddr{};
        can_frame sFrame{};
        socklen_t nAddrLen = sizeof(sAddr);

        for (const auto& socket : m_vecSockets)
        {
            if ((socket.localSocket > 0) && (socket.networkInterface > 0))
            {
                //auto socketIndex = socket.localSocket;
                ssize_t nBytes = recvfrom(socket.localSocket, &sFrame, sizeof(can_frame), 0, reinterpret_cast<sockaddr*>(&sAddr), &nAddrLen);
                if (nBytes < 0)
                {
                    // No data received... wait for 1 ms and try again
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                sdv::can::SMessage sMsg{};
                sMsg.uiID = sFrame.can_id;
                for (size_t nDataIndex = 0; nDataIndex < static_cast<size_t>(sFrame.can_dlc); nDataIndex++)
                {
                    sMsg.seqData.push_back(sFrame.data[nDataIndex]);
                }

                // Broadcast the message to the receivers
                std::unique_lock<std::mutex> lockReceivers(m_mtxReceivers);
                for (sdv::can::IReceive* pReceiver : m_setReceivers)
                {
                    pReceiver->Receive(sMsg, socket.networkInterface);
                }
            }
        }
    }
}
