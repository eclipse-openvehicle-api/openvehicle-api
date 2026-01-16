#include "gtest/gtest.h"
#include "../../../sdv_services/ipc_shared_mem/channel_mgnt.h"
#include "../../../sdv_services/ipc_shared_mem/connection.h"
#include "../../../sdv_services/ipc_shared_mem/watchdog.h"
#include "../../include/logger_test_helper.h"
#include <../global/base64.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <interfaces/ipc.h>
#include <algorithm>
#include <condition_variable>
#include <queue>
#include <atomic>

/**
 * @brief Receiver helper class.
*/
class CConnectReceiver : public sdv::IInterfaceAccess, public sdv::ipc::IDataReceiveCallback, public sdv::ipc::IConnectEventCallback
{
public:
    /**
     * @brief Constructor
     * @param[in] bEnableEvent When set, enable the connection event callback interface.
     */
    CConnectReceiver(bool bEnableEvent = false) : m_bEnableEvent(bEnableEvent)
    {}

    /**
     * @brief Destructor
     */
    ~CConnectReceiver()
    {
        m_bShutdown = true;
        std::unique_lock<std::mutex> lock(m_mtxData);
        lock.unlock();
        if (m_threadDecoupledSend.joinable())
            m_threadDecoupledSend.join();
    }

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_CHECK_CONDITION(m_bEnableEvent)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Assign a sender interface.
     * @param[in] pSend Pointer to the sending interface.
     */
    void AssignSender(sdv::ipc::IDataSend* pSend)
    {
        m_pSend = pSend;
    }

    /**
     * @brief Callback to be called by the IPC connection when receiving a data packet. Overload
     * sdv::ipc::IDataReceiveCallback::ReceiveData.
     * @param[inout] seqData Sequence of data buffers to received. The sequence might be changed to optimize the communication
     * without having to copy the data.
     */
    virtual void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override
    {
        std::unique_lock<std::mutex> lock(m_mtxData);

        // Copy the data
        m_seqDataCopy = seqData;

        // Start the processing thread if needed
        if (!m_threadDecoupledSend.joinable()) m_threadDecoupledSend =
                std::thread(&CConnectReceiver::DecoupledSendThread, this);

        // Store data into the queue for sending.
        m_queueDecoupledSend.push(std::move(seqData));
        m_cvDecoupledSend.notify_all();
    }

    /**
     * @brief Set the current status. Overload of sdv::ipc::IConnectEventCallback::SetStatus.
     * @param[in] eConnectStatus The connection status.
     */
    virtual void SetStatus(/*in*/ sdv::ipc::EConnectStatus eConnectStatus) override
    {
        switch (eConnectStatus)
        {
        case sdv::ipc::EConnectStatus::connection_error:
            m_bConnectError = true;
            break;
        case sdv::ipc::EConnectStatus::communication_error:
            m_bCommError = true;
            break;
        case sdv::ipc::EConnectStatus::disconnected_forced:
            m_bForcedDisconnect = true;
            break;
        default:
            break;
        }
        m_eStatus = eConnectStatus;
    }

    /**
     * @brief Return the received connection status.
     * @return The received connection status.
     */
    sdv::ipc::EConnectStatus GetReceivedStatus() const { return m_eStatus; }

    /**
     * @brief Has a connection error occurred?
     * @return Connection error flag.
     */
    bool ConnectionErrorOccurred() const { return m_bConnectError; }

    /**
     * @brief Has a communication error occurred?
     * @return Communication error flag.
     */
    bool CommunicationbErrorOccurred() const { return m_bCommError; }

    /**
     * @brief Was a disconnect triggered forcefully?
     * @return Forced disconnect flag.
     */
    bool ForcedDisconnectOccurred() const { return m_bForcedDisconnect; }

    /**
     * @brief Reset the received status event flags.
     */
    void ResetStatusEvents()
    {
        m_bConnectError = false;
        m_bCommError = false;
        m_bForcedDisconnect = false;
    }

    /**
     * @brief Get the data
     * @return Copy of the data.
     */
    sdv::sequence<sdv::pointer<uint8_t>> GetData() const
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        return m_seqDataCopy;
    }

private:
    /**
     * @brief Decoupled send thread function.
     */
    void DecoupledSendThread()
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        while (!m_bShutdown)
        {
            if (m_queueDecoupledSend.empty())
            {
                m_cvDecoupledSend.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }

            auto seqData = std::move(m_queueDecoupledSend.front());
            m_queueDecoupledSend.pop();

            if (m_pSend)
                m_pSend->SendData(seqData);
        }
    }

    bool                                    m_bEnableEvent = false;                 ///< When set, enable the event interface.
    sdv::ipc::IDataSend*                    m_pSend = nullptr;                      ///< Send interface to implement repeating function.
    mutable std::mutex                      m_mtxData;                              ///< Protect data access.
    sdv::sequence<sdv::pointer<uint8_t>>    m_seqDataCopy;                          ///< Copy of the data.
    sdv::ipc::EConnectStatus m_eStatus = sdv::ipc::EConnectStatus::uninitialized;   ///< Current received status.
    bool                                    m_bConnectError = false;                ///< Connection error ocurred.
    bool                                    m_bCommError = false;                   ///< Communication error occurred.
    bool                                    m_bForcedDisconnect = false;            ///< Force disconnect.
    std::thread                             m_threadDecoupledSend;                  ///< Decoupled send thread.
    std::queue<sdv::sequence<sdv::pointer<uint8_t>>> m_queueDecoupledSend;          ///< Data queue for sending.
    std::condition_variable                 m_cvDecoupledSend;                      ///< Trigger decoupled sending.
    std::atomic_bool                        m_bShutdown = false;                    ///< Shutdown send thread.
};

/**
 * @brief Load support modules to publish the needed services.
 */
void LoadSupportServices();

TEST(SharedMemChannelService, Instantiate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgnt;
    EXPECT_NO_THROW(mgnt.Initialize(""));

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::initialized);

    EXPECT_NO_THROW(mgnt.Shutdown());

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::destruction_pending);

    appcontrol.Shutdown();
}

TEST(SharedMemChannelService, ChannelConfigString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgnt;
    EXPECT_NO_THROW(mgnt.Initialize(""));

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::initialized);

    EXPECT_NO_THROW(mgnt.Shutdown());

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, CreateRandomEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgnt;

    // Create an endpoint.
    EXPECT_NO_THROW(mgnt.Initialize(""));
    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgnt.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    if (sChannelEndpoint.pConnection) sdv::TObjectPtr(sChannelEndpoint.pConnection);
    EXPECT_NO_THROW(mgnt.Shutdown());

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, CreateExplicitEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgnt;

    // Create an endpoint.
    EXPECT_NO_THROW(mgnt.Initialize(""));
    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgnt.CreateEndpoint(R"code([IpcChannel]
Name = "CHANNEL_1234"
Size = 10240
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    if (sChannelEndpoint.pConnection) sdv::TObjectPtr(sChannelEndpoint.pConnection);
    EXPECT_NO_THROW(mgnt.Shutdown());

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, GetRandomEndpointAccess)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient.Initialize(""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrConnection);
    EXPECT_NO_THROW(ptrConnection.Clear());

    if (sChannelEndpoint.pConnection) sdv::TObjectPtr(sChannelEndpoint.pConnection);
    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, GetExplicitEndpointAccess)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient.Initialize(""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code([IpcChannel]
Name = "CHANNEL_1234"
Size = 10240
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrConnection);
    EXPECT_NO_THROW(ptrConnection.Clear());

    if (sChannelEndpoint.pConnection) sdv::TObjectPtr(sChannelEndpoint.pConnection);
    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, WaitForConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgnt;

    // Create an endpoint.
    EXPECT_NO_THROW(mgnt.Initialize(""));
    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgnt.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrConnection);

    CConnectReceiver receiverServer;

    // Establish the server connection.
    sdv::ipc::IConnect* pConnection = ptrConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pConnection, nullptr);
    EXPECT_TRUE(pConnection->AsyncConnect(&receiverServer));

    // Wait for connection with timeout
    EXPECT_FALSE(pConnection->WaitForConnection(100));
    EXPECT_TRUE(pConnection->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pConnection->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    // Wait for connection for infinite period with cancel.
    std::thread threadCancelWait([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            pConnection->CancelWait();
        });
    EXPECT_FALSE(pConnection->WaitForConnection(0xffffffff));    // Note: wait indefinitely. Cancel get called in 500ms.
    threadCancelWait.join();
    EXPECT_TRUE(pConnection->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pConnection->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    EXPECT_NO_THROW(ptrConnection.Clear());

    EXPECT_NO_THROW(mgnt.Shutdown());

    EXPECT_EQ(mgnt.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, AsyncConnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient.Initialize(""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClientConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClientConnection);

    CConnectReceiver receiverServer, receiverClient;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    ptr.resize(4);
    ptr[0] = 1u;
    ptr[1] = 2u;
    ptr[2] = 3u;
    ptr[3] = 4u;
    seqPattern.push_back(ptr);
    ptr[0] = 5u;
    ptr[1] = 6u;
    ptr[2] = 7u;
    ptr[3] = 8u;
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);
    sdv::ipc::IDataSend* pClientSend = ptrClientConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pClientSend, nullptr);
    receiverClient.AssignSender(pClientSend);

    // Try send; should fail, since not connected
    EXPECT_FALSE(pServerSend->SendData(seqPattern));

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnect->WaitForConnection(25));    // Note: 25ms will not get a connection.
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    // Try send; should fail, since not connected
    EXPECT_FALSE(pServerSend->SendData(seqPattern));

    // Establish the client connection
    sdv::ipc::IConnect* pClientConnection = ptrClientConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto seqServerPattern = receiverServer.GetData();
    ASSERT_EQ(seqServerPattern.size(), 2);
    ASSERT_EQ(seqServerPattern[0].size(), 4);
    ASSERT_EQ(seqServerPattern[1].size(), 4);
    auto seqClientPattern = receiverClient.GetData();
    ASSERT_EQ(seqClientPattern.size(), 2);
    ASSERT_EQ(seqClientPattern[0].size(), 4);
    ASSERT_EQ(seqClientPattern[1].size(), 4);

    for (size_t nSeqIndex = 0; nSeqIndex < seqPattern.size(); nSeqIndex++)
    {
        for (size_t nPtrIndex = 0; nPtrIndex < seqPattern[nSeqIndex].size(); nPtrIndex++)
        {
            EXPECT_EQ((uint32_t)seqPattern[nSeqIndex][nPtrIndex], (uint32_t)seqServerPattern[nSeqIndex][nPtrIndex]);
            EXPECT_EQ((uint32_t)seqPattern[nSeqIndex][nPtrIndex], (uint32_t)seqClientPattern[nSeqIndex][nPtrIndex]);
        }
    }

    EXPECT_NO_THROW(ptrClientConnection.Clear());
    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, EstablishConnectionEvents)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient.Initialize(""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClientConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClientConnection);

    CConnectReceiver receiverServer(true), receiverClient(true);

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    ptr.resize(4);
    ptr[0] = 1u;
    ptr[1] = 2u;
    ptr[2] = 3u;
    ptr[3] = 4u;
    seqPattern.push_back(ptr);
    ptr[0] = 5u;
    ptr[1] = 6u;
    ptr[2] = 7u;
    ptr[3] = 8u;
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);
    sdv::ipc::IDataSend* pClientSend = ptrClientConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pClientSend, nullptr);
    receiverClient.AssignSender(pClientSend);

    // Try send; should fail, since not connected
    EXPECT_FALSE(receiverServer.CommunicationbErrorOccurred());
    EXPECT_FALSE(pServerSend->SendData(seqPattern));
    EXPECT_FALSE(receiverServer.CommunicationbErrorOccurred()); // No events registered yet... only after a call to AsyncConnect.
    receiverServer.ResetStatusEvents();

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_NE(pServerConnect->RegisterStatusEventCallback(&receiverServer), 0);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnect->WaitForConnection(25));    // Note: 25ms will not get a connection.
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_TRUE(receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::initialized ||
        receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());

    // Try send; should fail, since not connected
    EXPECT_FALSE(receiverServer.CommunicationbErrorOccurred());
    EXPECT_FALSE(pServerSend->SendData(seqPattern));
    EXPECT_TRUE(receiverServer.CommunicationbErrorOccurred());
    receiverServer.ResetStatusEvents();

    // Establish the client connection
    sdv::ipc::IConnect* pClientConnection = ptrClientConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_NE(pClientConnection->RegisterStatusEventCallback(&receiverClient), 0);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());
    EXPECT_FALSE(receiverClient.ConnectionErrorOccurred());
    EXPECT_EQ(receiverServer.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(receiverClient.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    auto seqServerPattern = receiverServer.GetData();
    ASSERT_EQ(seqServerPattern.size(), 2);
    ASSERT_EQ(seqServerPattern[0].size(), 4);
    ASSERT_EQ(seqServerPattern[1].size(), 4);
    auto seqClientPattern = receiverClient.GetData();
    ASSERT_EQ(seqClientPattern.size(), 2);
    ASSERT_EQ(seqClientPattern[0].size(), 4);
    ASSERT_EQ(seqClientPattern[1].size(), 4);

    for (size_t nSeqIndex = 0; nSeqIndex < seqPattern.size(); nSeqIndex++)
    {
        for (size_t nPtrIndex = 0; nPtrIndex < seqPattern[nSeqIndex].size(); nPtrIndex++)
        {
            EXPECT_EQ((uint32_t)seqPattern[nSeqIndex][nPtrIndex], (uint32_t)seqServerPattern[nSeqIndex][nPtrIndex]);
            EXPECT_EQ((uint32_t)seqPattern[nSeqIndex][nPtrIndex], (uint32_t)seqClientPattern[nSeqIndex][nPtrIndex]);
        }
    }

    EXPECT_FALSE(receiverServer.CommunicationbErrorOccurred());
    EXPECT_FALSE(receiverClient.CommunicationbErrorOccurred());

    EXPECT_NO_THROW(ptrClientConnection.Clear());
    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, EstablishReconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient1, mgntClient2;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient1.Initialize(""));
    EXPECT_EQ(mgntClient1.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient2.Initialize(""));
    EXPECT_EQ(mgntClient2.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClient1Connection = mgntClient1.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClient1Connection);

    CConnectReceiver receiverServer, receiverClient1, receiverClient2;

    std::cout << "Connect server..." << std::endl;

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnect->WaitForConnection(25));    // Note: 25ms will not get a connection.
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    std::cout << "Connect client 1..." << std::endl;
    // Establish the client1 connection
    sdv::ipc::IConnect* pClientConnection = ptrClient1Connection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient1));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));   // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    std::cout << "Disconnect client 1..." << std::endl;

    // Disconnect the client 1
    EXPECT_NO_THROW(ptrClient1Connection.Clear());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));     // Note: Handle connection terminate.
    EXPECT_NO_THROW(mgntClient1.Shutdown());
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::disconnected ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    std::cout << "Connect client 2... (eStatus = " << (uint32_t) pServerConnect->GetStatus() << ")" << std::endl;

    // Establish the client2 connection
    sdv::TObjectPtr ptrClient2Connection = mgntClient2.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClient2Connection);
    pClientConnection = ptrClient2Connection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient2));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));   // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    std::cout << "Disconnect client 2..." << std::endl;

    // Disconnect the client 2
    EXPECT_NO_THROW(ptrClient2Connection.Clear());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));     // Note: Handle connection terminate
    EXPECT_NO_THROW(mgntClient2.Shutdown());
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::disconnected ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    std::cout << "Shutdown server..." << std::endl;

    EXPECT_NO_THROW(ptrServerConnection.Clear());
    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, EstablishReconnectEvents)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemChannelMgnt mgntServer, mgntClient1, mgntClient2;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient1.Initialize(""));
    EXPECT_EQ(mgntClient1.GetStatus(), sdv::EObjectStatus::initialized);
    EXPECT_NO_THROW(mgntClient2.Initialize(""));
    EXPECT_EQ(mgntClient2.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClient1Connection = mgntClient1.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClient1Connection);

    CConnectReceiver receiverServer(true), receiverClient1(true), receiverClient2(true);

    std::cout << "Connect server..." << std::endl;

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_NE(pServerConnect->RegisterStatusEventCallback(&receiverServer), 0);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnect->WaitForConnection(25));    // Note: 25ms will not get a connection.
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_TRUE(receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::initialized ||
        receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());

    std::cout << "Connect client 1..." << std::endl;

    // Establish the client1 connection
    sdv::ipc::IConnect* pClientConnection = ptrClient1Connection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_NE(pClientConnection->RegisterStatusEventCallback(&receiverClient1), 0);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient1));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));   // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());
    EXPECT_FALSE(receiverClient1.ConnectionErrorOccurred());
    EXPECT_EQ(receiverServer.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(receiverClient1.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);

    std::cout << "Disconnect client 1..." << std::endl;

    // Disconnect the client 1
    EXPECT_NO_THROW(ptrClient1Connection.Clear());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));     // Note: Handle connection terminate.
    EXPECT_NO_THROW(mgntClient1.Shutdown());
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::disconnected ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_TRUE(receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::disconnected ||
        receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());

    std::cout << "Connect client 2..." << std::endl;

    // Establish the client2 connection
    sdv::TObjectPtr ptrClient2Connection = mgntClient2.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClient2Connection);
    pClientConnection = ptrClient2Connection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_NE(pClientConnection->RegisterStatusEventCallback(&receiverClient2), 0);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient2));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));   // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnect->WaitForConnection(1000));    // Note: 1000ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());
    EXPECT_FALSE(receiverClient2.ConnectionErrorOccurred());
    EXPECT_EQ(receiverServer.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(receiverClient2.GetReceivedStatus(), sdv::ipc::EConnectStatus::connected);

    std::cout << "Disconnect client 2..." << std::endl;

    // Disconnect the client 2
    EXPECT_NO_THROW(ptrClient2Connection.Clear());
    std::this_thread::sleep_for(std::chrono::milliseconds(25));     // Note: Handle connection terminate
    EXPECT_NO_THROW(mgntClient2.Shutdown());
    EXPECT_TRUE(pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::disconnected ||
        pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_TRUE(receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::disconnected ||
        receiverServer.GetReceivedStatus() == sdv::ipc::EConnectStatus::connecting);
    EXPECT_FALSE(receiverServer.ConnectionErrorOccurred());

    std::cout << "Shutdown server..." << std::endl;

    EXPECT_NO_THROW(ptrServerConnection.Clear());
    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, AppEstablishConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    CSharedMemChannelMgnt mgntServer;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrServerConnection);

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = { "NONE", Base64EncodePlainText(sChannelEndpoint.ssConnectString)};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CConnectReceiver receiverServer;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    ptr.resize(4);
    ptr[0] = 1u;
    ptr[1] = 2u;
    ptr[2] = 3u;
    ptr[3] = 4u;
    seqPattern.push_back(ptr);
    ptr[0] = 5u;
    ptr[1] = 6u;
    ptr[2] = 7u;
    ptr[3] = 8u;
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_TRUE(pServerConnect->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    auto seqServerPattern = receiverServer.GetData();
    ASSERT_EQ(seqServerPattern.size(), 2);
    ASSERT_EQ(seqServerPattern[0].size(), 4);
    ASSERT_EQ(seqServerPattern[1].size(), 4);

    for (size_t nSeqIndex = 0; nSeqIndex < seqPattern.size(); nSeqIndex++)
    {
        for (size_t nPtrIndex = 0; nPtrIndex < seqPattern[nSeqIndex].size(); nPtrIndex++)
        {
            EXPECT_EQ((uint32_t)seqPattern[nSeqIndex][nPtrIndex], (uint32_t)seqServerPattern[nSeqIndex][nPtrIndex]);
        }
    }

    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID, 0xffffffff);
}

TEST(SharedMemChannelService, AppGracefullyShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    CSharedMemChannelMgnt mgntServer;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrServerConnection);

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = {"NONE", Base64EncodePlainText(sChannelEndpoint.ssConnectString)};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CConnectReceiver receiverServer;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    ptr.resize(4);
    ptr[0] = 1u;
    ptr[1] = 2u;
    ptr[2] = 3u;
    ptr[3] = 4u;
    seqPattern.push_back(ptr);
    ptr[0] = 5u;
    ptr[1] = 6u;
    ptr[2] = 7u;
    ptr[3] = 8u;
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_TRUE(pServerConnect->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Try send; should succeed, since connected
    auto tpStart = std::chrono::high_resolution_clock::now();
    while (pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connected)
    {
        EXPECT_TRUE(pServerSend->SendData(seqPattern));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        auto tpNow = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration<double>(tpNow - tpStart).count() > 4.0)
        {
            std::cout << "Expected the app to have ended!" << std::endl;
            break; // Max 4000 ms
        }
    }
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::disconnected);

    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, AppForcedShutdown_Watchdog)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    CSharedMemChannelMgnt mgntServer;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrServerConnection);

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = {"NONE", Base64EncodePlainText(sChannelEndpoint.ssConnectString), "FORCE_TERMINATE"};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CConnectReceiver receiverServer(true);

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    ptr.resize(4);
    ptr[0] = 1u;
    ptr[1] = 2u;
    ptr[2] = 3u;
    ptr[3] = 4u;
    seqPattern.push_back(ptr);
    ptr[0] = 5u;
    ptr[1] = 6u;
    ptr[2] = 7u;
    ptr[3] = 8u;
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnect = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnect, nullptr);
    EXPECT_NE(pServerConnect->RegisterStatusEventCallback(&receiverServer), 0);
    EXPECT_TRUE(pServerConnect->AsyncConnect(&receiverServer));
    EXPECT_TRUE(pServerConnect->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Try send; should succeed, since connected
    auto tpStart = std::chrono::high_resolution_clock::now();
    while (pServerConnect->GetStatus() == sdv::ipc::EConnectStatus::connected)
    {
        EXPECT_TRUE(pServerSend->SendData(seqPattern));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        auto tpNow = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration<double>(tpNow - tpStart).count() > 4.0)
        {
            std::cout << "Expected the app to have ended!" << std::endl;
            break; // Max 4000 ms
        }
    }
    EXPECT_TRUE(receiverServer.ForcedDisconnectOccurred());
    EXPECT_EQ(pServerConnect->GetStatus(), sdv::ipc::EConnectStatus::disconnected);

    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID, 0xffffffff);
}

TEST(SharedMemChannelService, IndirectAppGracefullyShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);

    // Create the first control endpoint.
    CSharedMemChannelMgnt mgntControl1;
    EXPECT_NO_THROW(mgntControl1.Initialize(""));
    EXPECT_EQ(mgntControl1.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint1 = mgntControl1.CreateEndpoint("");
    EXPECT_NE(sEndpoint1.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint1.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection1(sEndpoint1.pConnection);

    // Start first process
    sdv::sequence<sdv::u8string> seqArgs1 = {Base64EncodePlainText(sEndpoint1.ssConnectString)};
    sdv::process::TProcessID tProcessID1 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs1, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);

    // Establish the first control connection
    CConnectReceiver receiverControl1;
    sdv::ipc::IConnect* pControlConnect1 = ptrControlConnection1.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect1, nullptr);
    EXPECT_TRUE(pControlConnect1->AsyncConnect(&receiverControl1));
    EXPECT_TRUE(pControlConnect1->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pControlConnect1->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for data
    sdv::sequence<sdv::pointer<uint8_t>> seqDataConnectString;
    for (size_t n = 0; n < 100; n++)
    {
        seqDataConnectString = receiverControl1.GetData();
        if (!seqDataConnectString.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(seqDataConnectString.empty());
    std::string ssDataConnectString;
    for (size_t nCharIndex = 0; nCharIndex < seqDataConnectString[0].size(); nCharIndex++)
        ssDataConnectString.push_back(seqDataConnectString[0][nCharIndex]);
    EXPECT_FALSE(ssDataConnectString.empty());

    // Create the second control endpoint.
    CSharedMemChannelMgnt mgntControl2;
    EXPECT_NO_THROW(mgntControl2.Initialize(""));
    EXPECT_EQ(mgntControl2.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint2 = mgntControl2.CreateEndpoint("");
    EXPECT_NE(sEndpoint2.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint2.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection2(sEndpoint2.pConnection);

    // Start second process
    sdv::sequence<sdv::u8string> seqArgs2 = {Base64EncodePlainText(sEndpoint2.ssConnectString), Base64EncodePlainText(ssDataConnectString)};
    sdv::process::TProcessID tProcessID2 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs2, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    // Establish the second control connection
    CConnectReceiver receiverControl2;
    sdv::ipc::IConnect* pControlConnect2 = ptrControlConnection2.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect2, nullptr);
    EXPECT_TRUE(pControlConnect2->AsyncConnect(&receiverControl2));
    EXPECT_TRUE(pControlConnect2->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pControlConnect2->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID1, 0xffffffff);
    pProcessLifetime->WaitForTerminate(tProcessID2, 0xffffffff);

    ptrControlConnection2.Clear();
    ptrControlConnection1.Clear();
    mgntControl2.Shutdown();
    mgntControl1.Shutdown();
    appcontrol.Shutdown();
}

TEST(SharedMemChannelService, IndirectAppServerForceShutdown_Watchdog)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);

    CSharedMemChannelMgnt mgntControl;

    // Create the first control endpoint.
    CSharedMemChannelMgnt mgntControl1;
    EXPECT_NO_THROW(mgntControl1.Initialize(""));
    EXPECT_EQ(mgntControl1.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint1 = mgntControl1.CreateEndpoint("");
    EXPECT_NE(sEndpoint1.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint1.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection1(sEndpoint1.pConnection);

    // Start first process
    sdv::sequence<sdv::u8string> seqArgs1 = {Base64EncodePlainText(sEndpoint1.ssConnectString), "FORCE_TERMINATE"};
    sdv::process::TProcessID tProcessID1 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs1, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);

    // Establish the first control connection
    CConnectReceiver receiverControl1;
    sdv::ipc::IConnect* pControlConnect1 = ptrControlConnection1.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect1, nullptr);
    EXPECT_TRUE(pControlConnect1->AsyncConnect(&receiverControl1));
    EXPECT_TRUE(pControlConnect1->WaitForConnection(2000));  // Note: Connection should be possible within 5000ms.
    EXPECT_EQ(pControlConnect1->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for connection string (max. 500*10ms).
    sdv::sequence<sdv::pointer<uint8_t>> seqDataConnectString;
    for (size_t n = 0; n < 500; n++)
    {
        seqDataConnectString = receiverControl1.GetData();
        if (!seqDataConnectString.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(seqDataConnectString.empty());
    std::string ssDataConnectString;
    for (size_t nCharIndex = 0; nCharIndex < seqDataConnectString[0].size(); nCharIndex++)
        ssDataConnectString.push_back(seqDataConnectString[0][nCharIndex]);
    EXPECT_FALSE(ssDataConnectString.empty());

    // Create the second control endpoint.
    CSharedMemChannelMgnt mgntControl2;
    EXPECT_NO_THROW(mgntControl2.Initialize(""));
    EXPECT_EQ(mgntControl2.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint2 = mgntControl2.CreateEndpoint("");
    EXPECT_NE(sEndpoint2.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint2.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection2(sEndpoint2.pConnection);

    // Start second process
    sdv::sequence<sdv::u8string> seqArgs2 = {Base64EncodePlainText(sEndpoint2.ssConnectString), Base64EncodePlainText(ssDataConnectString)};
    sdv::process::TProcessID tProcessID2 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs2, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    // Establish the second control connection
    CConnectReceiver receiverControl2;
    sdv::ipc::IConnect* pControlConnect2 = ptrControlConnection2.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect2, nullptr);
    EXPECT_TRUE(pControlConnect2->AsyncConnect(&receiverControl2));
    EXPECT_TRUE(pControlConnect2->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pControlConnect2->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID1, 0xffffffff);
    pProcessLifetime->WaitForTerminate(tProcessID2, 0xffffffff);

    ptrControlConnection2.Clear();
    ptrControlConnection1.Clear();
    mgntControl2.Shutdown();
    mgntControl1.Shutdown();
    appcontrol.Shutdown();
}

TEST(SharedMemChannelService, IndirectAppClientForceShutdown_Watchdog)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);

    // Create the first control endpoint.
    CSharedMemChannelMgnt mgntControl1;
    EXPECT_NO_THROW(mgntControl1.Initialize(""));
    EXPECT_EQ(mgntControl1.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint1 = mgntControl1.CreateEndpoint("");
    EXPECT_NE(sEndpoint1.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint1.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection1(sEndpoint1.pConnection);

    // Start first process
    sdv::sequence<sdv::u8string> seqArgs1 = {Base64EncodePlainText(sEndpoint1.ssConnectString)};
    sdv::process::TProcessID tProcessID1 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs1, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);

    // Establish the first control connection
    CConnectReceiver receiverControl1;
    sdv::ipc::IConnect* pControlConnect1 = ptrControlConnection1.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect1, nullptr);
    EXPECT_TRUE(pControlConnect1->AsyncConnect(&receiverControl1));
    EXPECT_TRUE(pControlConnect1->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pControlConnect1->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for connection string (max. 500*10ms).
    sdv::sequence<sdv::pointer<uint8_t>> seqDataConnectString;
    for (size_t n = 0; n < 500; n++)
    {
        seqDataConnectString = receiverControl1.GetData();
        if (!seqDataConnectString.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(seqDataConnectString.empty());
    std::string ssDataConnectString;
    for (size_t nCharIndex = 0; nCharIndex < seqDataConnectString[0].size(); nCharIndex++)
        ssDataConnectString.push_back(seqDataConnectString[0][nCharIndex]);
    EXPECT_FALSE(ssDataConnectString.empty());

    // Create the second control endpoint.
    CSharedMemChannelMgnt mgntControl2;
    EXPECT_NO_THROW(mgntControl2.Initialize(""));
    EXPECT_EQ(mgntControl2.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::ipc::SChannelEndpoint sEndpoint2 = mgntControl2.CreateEndpoint("");
    EXPECT_NE(sEndpoint2.pConnection, nullptr);
    EXPECT_FALSE(sEndpoint2.ssConnectString.empty());
    sdv::TObjectPtr ptrControlConnection2(sEndpoint2.pConnection);

    // Start second process
    sdv::sequence<sdv::u8string> seqArgs2 = {Base64EncodePlainText(sEndpoint2.ssConnectString), Base64EncodePlainText(ssDataConnectString), "FORCE_TERMINATE"};
    sdv::process::TProcessID tProcessID2 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs2, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    // Establish the second control connection
    CConnectReceiver receiverControl2;
    sdv::ipc::IConnect* pControlConnect2 = ptrControlConnection2.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pControlConnect2, nullptr);
    EXPECT_TRUE(pControlConnect2->AsyncConnect(&receiverControl2));
    EXPECT_TRUE(pControlConnect2->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pControlConnect2->GetStatus(), sdv::ipc::EConnectStatus::connected);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID1, 0xffffffff);
    pProcessLifetime->WaitForTerminate(tProcessID2, 0xffffffff);

    ptrControlConnection2.Clear();
    ptrControlConnection1.Clear();
    mgntControl2.Shutdown();
    mgntControl1.Shutdown();
    appcontrol.Shutdown();
}

