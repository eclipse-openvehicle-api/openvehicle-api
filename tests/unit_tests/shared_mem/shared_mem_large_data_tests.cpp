#include "gtest/gtest.h"
#define TIME_TRACKING
#include "../../../sdv_services/ipc_shared_mem/channel_mgnt.h"
#include "../../../sdv_services/ipc_shared_mem/connection.h"
#include "../../../sdv_services/ipc_shared_mem/watchdog.h"
#include "../../include/logger_test_helper.h"
#include <../global/base64.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/sdv_test_macro.h>
#include <interfaces/ipc.h>
#include <algorithm>
#include <queue>

/**
* @brief Load support modules to publish the needed services.
*/
void LoadSupportServices();

/**
* @brief Receiver helper class.
*/
class CLargeDataReceiver : public sdv::IInterfaceAccess, public sdv::ipc::IDataReceiveCallback, public sdv::ipc::IConnectEventCallback
{
public:
    /**
    * @brief Constructor
    * @param[in] bEnableEvent When set, enable the connection event callback interface.
    */
    CLargeDataReceiver(bool bEnableEvent = false) : m_bEnableEvent(bEnableEvent),
        m_threadDecoupledSend(&CLargeDataReceiver::DecoupledSendThread, this)
    {}

    /**
     * @brief Destructor
     */
    ~CLargeDataReceiver()
    {
        m_bShutdown = true;
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
        m_nCount++;

        std::unique_lock<std::mutex> lock(m_mtxData);

        // Copy the data
        for (const sdv::pointer<uint8_t>& rptrData : seqData)
            m_queueDataCopy.push(rptrData);

        // Store data into the queue for sending.
        m_queueDecoupledSend.push(std::move(seqData));
        m_cvDecoupledSend.notify_all();

        // Send the same data back again (if needed).
        if (m_pSend)
            m_pSend->SendData(seqData);

        m_cvReceived.notify_all();
    }

    /**
     * @brief Wait until the caller hasn't sent anything anymore for 1 second.
     */
    void WaitForNoActivity(sdv::IInterfaceAccess* pSender, size_t nCount = 1, uint32_t uiTimeoutMs = 20000)
    {
        CConnection* pConnection = dynamic_cast<CConnection*>(pSender);
        double dTimeout = static_cast<double>(uiTimeoutMs) / 1000.0;
        if (dTimeout < 1.0) dTimeout = 1.0;

        // Wait until there is no activity any more.
        std::chrono::high_resolution_clock::time_point tpTickSent = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point tpTickReceive = std::chrono::high_resolution_clock::now();
        while (m_eStatus != sdv::ipc::EConnectStatus::disconnected)
        {
            std::unique_lock<std::mutex> lock(m_mtxData);
            m_cvReceived.wait_for(lock, std::chrono::milliseconds(50));

            // Does the connection have a better time?
            if (pConnection) tpTickSent = pConnection->GetLastSentTime();
            if (pConnection) tpTickReceive = pConnection->GetLastReceiveTime();
            std::chrono::high_resolution_clock::time_point tpNow = std::chrono::high_resolution_clock::now();

            // Amount reached
            if (GetReceiveCount() >= nCount) break;

            // A duration of more than a second should not occur.
            if (std::chrono::duration<double>(tpNow - tpTickSent).count() > dTimeout &&
                std::chrono::duration<double>(tpNow - tpTickReceive).count() > dTimeout)
                break;
        }

        TRACE("No new data for ", uiTimeoutMs, " milliseconds...");

        if (pConnection)
            TRACE("Largest receive loop time is ", pConnection->GetLargestReceiveLoopDuration().count(), " seconds");
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
     * @brief Return the receive count.
     * @return The receive count.
    */
    size_t GetReceiveCount() const
    {
        return m_nCount;
    }

    /**
     * @brief Return the data pointer count.
     * @return The data count.
    */
    size_t GetDataCount() const
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        return m_queueDataCopy.size();
    }

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
    sdv::pointer<uint8_t> GetData()
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        if (m_queueDataCopy.empty()) return sdv::pointer<uint8_t>();
        sdv::pointer<uint8_t> ptrData = std::move(m_queueDataCopy.front());
        m_queueDataCopy.pop();
        return ptrData;
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

    bool                                    m_bEnableEvent = false;         ///< When set, enable the event interface.
    sdv::ipc::IDataSend*                    m_pSend = nullptr;              ///< Send interface to implement repeating function.
    mutable std::mutex                      m_mtxData;                      ///< Protect data access.
    std::queue<sdv::pointer<uint8_t>>       m_queueDataCopy;                ///< Copy of the data.
    sdv::ipc::EConnectStatus                m_eStatus = sdv::ipc::EConnectStatus::uninitialized; ///< Current received status.
    bool                                    m_bConnectError = false;        ///< Connection error ocurred.
    bool                                    m_bCommError = false;           ///< Communication error occurred.
    bool                                    m_bForcedDisconnect = false;    ///< Force disconnect.
    size_t                                  m_nCount = 0;                   ///< Receive counter.
    std::condition_variable                 m_cvReceived;                   ///< Receive event.
    std::thread                             m_threadDecoupledSend;          ///< Decoupled send thread.
    std::queue<sdv::sequence<sdv::pointer<uint8_t>>> m_queueDecoupledSend;  ///< Data queue for sending.
    std::condition_variable                 m_cvDecoupledSend;              ///< Trigger decoupled sending.
    bool                                    m_bShutdown = false;            ///< Shutdown send thread.
};

TEST(SharedMemChannelService, CommunicateOneLargeBlock)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code(
[Console]
Report = "Silent"
        )code"));
    appcontrol.SetConfigMode();

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    mgntServer.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::running);
    EXPECT_NO_THROW(mgntClient.Initialize("service = \"client\""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    mgntClient.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::running);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClientConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClientConnection);

    CLargeDataReceiver receiverServer, receiverClient;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    const size_t nCount = 10 * 1024 * 1024;
    ptr.resize(nCount * sizeof(uint32_t));
    uint32_t* pData = reinterpret_cast<uint32_t*>(ptr.get());
    for (size_t n = 0; n < nCount; n++)
        pData[n] = static_cast<uint32_t>(n);
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);
    sdv::ipc::IDataSend* pClientSend = ptrClientConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pClientSend, nullptr);
    receiverClient.AssignSender(pClientSend);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnection = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnection, nullptr);
    EXPECT_TRUE(pServerConnection->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnection->WaitForConnection(50));    // Note: 50ms will not get a connection.
    EXPECT_TRUE(pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    // Establish the client connection
    sdv::ipc::IConnect* pClientConnection = ptrClientConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnection->WaitForConnection(50));    // Note: 50ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    appcontrol.SetRunningMode();

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    receiverServer.WaitForNoActivity(ptrServerConnection);
    auto ptrServerPattern = receiverServer.GetData();
    ASSERT_EQ(ptrServerPattern.size(), nCount * sizeof(uint32_t));
    auto ptrClientPattern = receiverClient.GetData();
    ASSERT_EQ(ptrClientPattern.size(), nCount * sizeof(uint32_t));
    pData = reinterpret_cast<uint32_t*>(ptrServerPattern.get());
    for (size_t n = 0; n < nCount; n++)
    {
        EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
        if (pData[n] != static_cast<uint32_t>(n)) break;
    }
    pData = reinterpret_cast<uint32_t*>(ptrClientPattern.get());
    for (size_t n = 0; n < nCount; n++)
    {
        EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
        if (pData[n] != static_cast<uint32_t>(n)) break;
    }

    appcontrol.SetConfigMode();
    EXPECT_NO_THROW(ptrClientConnection.Clear());
    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, CommunicateMultiLargeBlock)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));
    appcontrol.SetConfigMode();

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    mgntServer.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::running);
    EXPECT_NO_THROW(mgntClient.Initialize("service = \"client\""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    mgntClient.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::running);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClientConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClientConnection);

    CLargeDataReceiver receiverServer, receiverClient;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    const size_t nCount = 10 * 1024 * 1024;
    ptr.resize(nCount * sizeof(uint32_t));
    uint32_t* pData = reinterpret_cast<uint32_t*>(ptr.get());
    for (size_t n = 0; n < nCount; n++)
        pData[n] = static_cast<uint32_t>(n);
    seqPattern.push_back(ptr);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);
    sdv::ipc::IDataSend* pClientSend = ptrClientConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pClientSend, nullptr);
    receiverClient.AssignSender(pClientSend);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnection = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnection, nullptr);
    EXPECT_TRUE(pServerConnection->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnection->WaitForConnection(50));    // Note: 50ms will not get a connection.
    EXPECT_TRUE(pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    // Establish the client connection
    sdv::ipc::IConnect* pClientConnection = ptrClientConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnection->WaitForConnection(50));    // Note: 50ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    appcontrol.SetRunningMode();

    // Try send; should succeed, since connected
    for (size_t nCnt = 0; nCnt < 30; nCnt++)
        EXPECT_TRUE(pServerSend->SendData(seqPattern));
    receiverServer.WaitForNoActivity(ptrServerConnection, 30);

    // Check server
    size_t nCnt = 0;
    bool bCorrect = true;
    EXPECT_EQ(receiverServer.GetReceiveCount(), 30u);
    while (bCorrect)
    {
        auto ptrServerPattern = receiverServer.GetData();
        if (!ptrServerPattern) break;
        nCnt++;
        ASSERT_EQ(ptrServerPattern.size(), nCount * sizeof(uint32_t));
        pData = reinterpret_cast<uint32_t*>(ptrServerPattern.get());
        for (size_t n = 0; n < nCount; n++)
        {
            EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
            if (pData[n] != static_cast<uint32_t>(n))
            {
                bCorrect = false;
                break;
            }
        }
    }
    EXPECT_EQ(nCnt, 30);

    nCnt = 0;
    bCorrect = true;
    EXPECT_EQ(receiverClient.GetReceiveCount(), 30u);
    while (bCorrect)
    {
        auto ptrClientPattern = receiverClient.GetData();
        if (!ptrClientPattern) break;
        nCnt++;
        ASSERT_EQ(ptrClientPattern.size(), nCount * sizeof(uint32_t));
        pData = reinterpret_cast<uint32_t*>(ptrClientPattern.get());
        for (size_t n = 0; n < nCount; n++)
        {
            EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
            if (pData[n] != static_cast<uint32_t>(n))
            {
                bCorrect = false;
                break;
            }
        }
    }
    EXPECT_EQ(nCnt, 30);

    appcontrol.SetConfigMode();

    EXPECT_NO_THROW(ptrClientConnection.Clear());
    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, CommunicateFragmentedLargeBlock)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));
    appcontrol.SetConfigMode();

    CSharedMemChannelMgnt mgntServer, mgntClient;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    mgntServer.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::running);
    EXPECT_NO_THROW(mgntClient.Initialize("service = \"client\""));
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::initialized);
    mgntClient.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::running);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    sdv::TObjectPtr ptrClientConnection = mgntClient.Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrServerConnection);
    EXPECT_TRUE(ptrClientConnection);

    CLargeDataReceiver receiverServer, receiverClient;

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    const size_t nCol = 10 * 1024;
    const size_t nRow = 512;
    size_t nCount = 0;
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern(nRow);
    uint32_t* pData = nullptr;
    for (size_t nRowCnt = 0; nRowCnt < nRow; nRowCnt++)
    {
        seqPattern[nRowCnt].resize(nCol * sizeof(uint32_t));
        pData = reinterpret_cast<uint32_t*>(seqPattern[nRowCnt].get());
        for (size_t nColCnt = 0; nColCnt < nCol; nColCnt++)
            pData[nColCnt] = static_cast<uint32_t>(nCount++);
    }
    EXPECT_EQ(nCount, nRow * nCol);
    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);
    sdv::ipc::IDataSend* pClientSend = ptrClientConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pClientSend, nullptr);
    receiverClient.AssignSender(pClientSend);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnection = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnection, nullptr);
    EXPECT_TRUE(pServerConnection->AsyncConnect(&receiverServer));
    EXPECT_FALSE(pServerConnection->WaitForConnection(50));    // Note: 50ms will not get a connection.
    EXPECT_TRUE(pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::initialized ||
        pServerConnection->GetStatus() == sdv::ipc::EConnectStatus::connecting);

    // Establish the client connection
    sdv::ipc::IConnect* pClientConnection = ptrClientConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnection, nullptr);
    EXPECT_TRUE(pClientConnection->AsyncConnect(&receiverClient));
    EXPECT_TRUE(pClientConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_TRUE(pServerConnection->WaitForConnection(50));    // Note: 50ms to also receive the connection at the server.
    EXPECT_EQ(pClientConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    EXPECT_EQ(pServerConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    appcontrol.SetRunningMode();

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    receiverServer.WaitForNoActivity(ptrServerConnection);
    EXPECT_EQ(receiverServer.GetReceiveCount(), 1);
    EXPECT_EQ(receiverClient.GetReceiveCount(), 1u);
    EXPECT_EQ(receiverServer.GetDataCount(), 512u);
    EXPECT_EQ(receiverClient.GetDataCount(), 512u);
    size_t nCountServer = 0, nCountClient = 0;
    while (true)
    {
        auto ptrServerPattern = receiverServer.GetData();
        if (!ptrServerPattern) break;
        EXPECT_EQ(ptrServerPattern.size(), nCol * sizeof(uint32_t));
        auto ptrClientPattern = receiverClient.GetData();
        EXPECT_EQ(ptrClientPattern.size(), nCol * sizeof(uint32_t));
        if (!ptrClientPattern) break;

        pData = reinterpret_cast<uint32_t*>(ptrServerPattern.get());
        for (size_t n = 0; n < ptrServerPattern.size() / sizeof(uint32_t); n++)
        {
            EXPECT_EQ(pData[n], static_cast<uint32_t>(nCountServer));
            if (pData[n] != static_cast<uint32_t>(nCountServer)) break;
            nCountServer++;
        }

        pData = reinterpret_cast<uint32_t*>(ptrClientPattern.get());
        for (size_t n = 0; n < ptrClientPattern.size() / sizeof(uint32_t); n++)
        {
            EXPECT_EQ(pData[n], static_cast<uint32_t>(nCountClient));
            if (pData[n] != static_cast<uint32_t>(nCountClient)) break;
            nCountClient++;
        }
    }

    appcontrol.SetConfigMode();

    EXPECT_NO_THROW(ptrClientConnection.Clear());
    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());
    EXPECT_NO_THROW(mgntClient.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);
    EXPECT_EQ(mgntClient.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST(SharedMemChannelService, AppCommunicateOneLargeBlock)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code(
[Application]
Mode="Essential")code"));
    LoadSupportServices();
    appcontrol.SetConfigMode();

    CSharedMemChannelMgnt mgntServer;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    mgntServer.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::running);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrServerConnection);

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    const size_t nCount = 10 * 1024 * 1024;
    ptr.resize(nCount * sizeof(uint32_t));
    uint32_t* pData = reinterpret_cast<uint32_t*>(ptr.get());
    for (size_t n = 0; n < nCount; n++)
        pData[n] = static_cast<uint32_t>(n);
    seqPattern.push_back(ptr);

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = {"NONE", Base64EncodePlainText(sChannelEndpoint.ssConnectString)};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CLargeDataReceiver receiverServer;

    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnection = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnection, nullptr);
    EXPECT_TRUE(pServerConnection->AsyncConnect(&receiverServer));
    EXPECT_TRUE(pServerConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pServerConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    TRACE("Connection estabished...");
    appcontrol.SetRunningMode();

    // Try send; should succeed, since connected
    EXPECT_TRUE(pServerSend->SendData(seqPattern));
    receiverServer.WaitForNoActivity(ptrServerConnection);
    auto ptrServerPattern = receiverServer.GetData();
    ASSERT_EQ(ptrServerPattern.size(), nCount * sizeof(uint32_t));
    pData = reinterpret_cast<uint32_t*>(ptrServerPattern.get());
    for (size_t n = 0; n < nCount; n++)
    {
        EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
        if (pData[n] != static_cast<uint32_t>(n)) break;
    }

    appcontrol.SetConfigMode();

    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID, 0xffffffff);
}

TEST(SharedMemChannelService, AppCommunicateMultiLargeBlock)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code(
[Application]
Mode="Essential")code"));
    LoadSupportServices();
    appcontrol.SetConfigMode();

    CSharedMemChannelMgnt mgntServer;

    // Create an endpoint.
    EXPECT_NO_THROW(mgntServer.Initialize(""));
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::initialized);
    mgntServer.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::running);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = mgntServer.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());

    sdv::TObjectPtr ptrServerConnection(sChannelEndpoint.pConnection);
    EXPECT_TRUE(ptrServerConnection);

    // Get sending interfaces and assign the server send to the client to get a repeater function.
    sdv::sequence<sdv::pointer<uint8_t>> seqPattern;
    sdv::pointer<uint8_t> ptr;
    const size_t nCount = 10 * 1024 * 1024;
    ptr.resize(nCount * sizeof(uint32_t));
    uint32_t* pData = reinterpret_cast<uint32_t*>(ptr.get());
    for (size_t n = 0; n < nCount; n++)
        pData[n] = static_cast<uint32_t>(n);
    seqPattern.push_back(ptr);

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = {"NONE", Base64EncodePlainText(sChannelEndpoint.ssConnectString), "LONG_LIFE"};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Connect", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CLargeDataReceiver receiverServer;

    sdv::ipc::IDataSend* pServerSend = ptrServerConnection.GetInterface<sdv::ipc::IDataSend>();
    ASSERT_NE(pServerSend, nullptr);

    // Establish the server connection
    sdv::ipc::IConnect* pServerConnection = ptrServerConnection.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pServerConnection, nullptr);
    EXPECT_TRUE(pServerConnection->AsyncConnect(&receiverServer));
    EXPECT_TRUE(pServerConnection->WaitForConnection(2000));  // Note: Connection should be possible within 2000ms.
    EXPECT_EQ(pServerConnection->GetStatus(), sdv::ipc::EConnectStatus::connected);
    TRACE("Connection estabished...");
    appcontrol.SetRunningMode();

    // Try send; should succeed, since connected
    for (size_t nCnt = 0; nCnt < 30; nCnt++)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_EQ(pServerSend->SendData(seqPattern), true, sdv::TEST::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_EQ(pServerSend->SendData(seqPattern), true, sdv::TEST::WARNING_ENABLED);
    }
    receiverServer.WaitForNoActivity(ptrServerConnection, 30);

    // Check server
    size_t nCnt = 0;
    bool bCorrect = true;
    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
        SDV_TIMING_EXPECT_EQ(receiverServer.GetReceiveCount(), 30u, sdv::TEST::WARNING_REDUCED);
    else
        SDV_TIMING_EXPECT_EQ(receiverServer.GetReceiveCount(), 30u, sdv::TEST::WARNING_ENABLED);
    while (bCorrect)
    {
        auto ptrServerPattern = receiverServer.GetData();
        if (!ptrServerPattern) break;
        nCnt++;
        ASSERT_EQ(ptrServerPattern.size(), nCount * sizeof(uint32_t));
        pData = reinterpret_cast<uint32_t*>(ptrServerPattern.get());
        for (size_t n = 0; n < nCount; n++)
        {
            EXPECT_EQ(pData[n], static_cast<uint32_t>(n));
            if (pData[n] != static_cast<uint32_t>(n))
            {
                bCorrect = false;
                break;
            }
        }
    }
    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
        SDV_TIMING_EXPECT_EQ(nCnt, 30, sdv::TEST::WARNING_REDUCED);
    else
        SDV_TIMING_EXPECT_EQ(nCnt, 30, sdv::TEST::WARNING_ENABLED);

    appcontrol.SetConfigMode();

    EXPECT_NO_THROW(ptrServerConnection.Clear());

    EXPECT_NO_THROW(mgntServer.Shutdown());

    EXPECT_EQ(mgntServer.GetStatus(), sdv::EObjectStatus::destruction_pending);

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID, 0xffffffff);
}
