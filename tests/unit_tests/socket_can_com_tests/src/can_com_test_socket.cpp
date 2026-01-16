#include "../../../../global/process_watchdog.h"
#include "../include/can_com_test_helper.h"
#include <atomic>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

class CANSocketTest : public ::testing::Test
{
    public:
    virtual void SetUp() override
    {
        if (vcanIsInstalled)
        {
            CComTestHelper testhelper;
            // interfaces required for the test
            RequiredVCAN(testhelper, true, "vcan0", "sudo ip link add dev vcan0 type vcan", "sudo ip link set up vcan0");
            RequiredVCAN(testhelper, true, "vcan1", "sudo ip link add dev vcan1 type vcan", "sudo ip link set up vcan1");
            RequiredVCAN(testhelper, true, "vcan2", "sudo ip link add dev vcan2 type vcan", "sudo ip link set up vcan2");
            RequiredVCAN(testhelper, true, "vcan3", "sudo ip link add dev vcan3 type vcan", "sudo ip link set up vcan3");
            RequiredVCAN(testhelper, true, "vcan4", "sudo ip link add dev vcan4 type vcan", "sudo ip link set up vcan4");
            // "vcan8" & "vcan9" were interfaces that must not exist.
            RequiredVCAN(testhelper, false, "vcan8", "sudo ip link set down vcan8", "sudo ip link delete vcan8");
            RequiredVCAN(testhelper, false, "vcan9", "sudo ip link set down vcan9", "sudo ip link delete vcan9");
        }
        else
        {
            GTEST_SKIP() << "Skipping test because vcan is not installed.";
        }
    }

    virtual void TearDown() override
    {
    }

    static void SetUpTestCase()
    {
        CComTestHelper testhelper;
        vcanIsInstalled = testhelper.IsVcanInstalled();
    }

    static void TearDownTestSuite() {}

    bool RequiredVCAN(CComTestHelper testhelper, bool expected, std::string name,
        std::string addDevcommand, std::string setupCommand)
    {
        if (!testhelper.ValidateVCanSetup(name.c_str(), expected))
        {
            testhelper.exec(addDevcommand.c_str());
            testhelper.exec(setupCommand.c_str());
            if (!testhelper.ValidateVCanSetup(name.c_str(), expected))
            {
                return false;
            }
        }
        return true;
    }

    void RemoveCanDumpFilesWithPrefix()
    {
        std::string directory = ".";  // Current directory
        std::string prefix = "candump_output_vcan";
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_regular_file() && entry.path().filename().string().find(prefix) == 0)
            {
                try
                {
                    std::filesystem::remove(entry.path());
                    std::cout << "Deleted: " << entry.path() << std::endl;
                } catch (const std::filesystem::filesystem_error&)
                {
                    std::cout << "Failed to delete: " << entry.path() << std::endl;
                }
            }
        }
    }

    static bool vcanIsInstalled;
};
// Define the static member variable
bool CANSocketTest::vcanIsInstalled = false;

class CTestCANSocket : public CCANSockets
{
public:
    virtual void Initialize(const sdv::u8string& ssObjectConfig) override
    {
        CCANSockets::Initialize(ssObjectConfig);
    }

    virtual void Shutdown() override
    {
        CCANSockets::Shutdown();
    }

    virtual void Send(const sdv::can::SMessage& sMsg, uint32_t uiIfcIndex) override
    {
        CCANSockets::Send(sMsg, uiIfcIndex);
        m_MessagesSent++;
    }

    virtual void RegisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        CCANSockets::RegisterReceiver(pReceiver);
    }

    virtual void UnregisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        CCANSockets::UnregisterReceiver(pReceiver);
    }

    virtual sdv::sequence<sdv::u8string> GetInterfaces() const override
    {
        return CCANSockets::GetInterfaces();
    }

    sdv::EObjectStatus GetTestStatus() const
    {
        return CCANSockets::GetStatus();
    }

    uint64_t GetMessagesSent() const
    {
        return m_MessagesSent;
    }

    uint64_t m_MessagesSent = 0;
};

class MockCANReceiver : public sdv::can::IReceive
{
public:
    void Receive(const sdv::can::SMessage& msg, uint32_t ifIndex) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_receivedMessages.push_back(std::make_pair(GetInterfaceName(ifIndex), msg));
    }

    void Error(const sdv::can::SErrorFrame&, uint32_t) override {}

    std::deque<std::pair<std::string, sdv::can::SMessage>>GetReceivedMessages() const
    {
        return m_receivedMessages;
    }

private:
    std::string GetInterfaceName (int ifIndex)
    {
        std::string name = "UnknownInterface";
        // Retrieve the list of available interfaces
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) != -1)
        {
            std::vector<std::string> available;
            for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
            {
                int indexFromName = if_nametoindex(ifa->ifa_name);
                if (indexFromName == ifIndex)
                {
                    name = ifa->ifa_name;
                    break;
                }
            }
        }
        return name;
    }

    std::deque<std::pair<std::string, sdv::can::SMessage>> m_receivedMessages; ///< Interface name, message
    std::mutex m_mutex;
};

bool InitializeAppControl(sdv::app::CAppControl* appcontrol, const std::string& configFileName)
{
	auto bResult = appcontrol->AddModuleSearchDir("../../bin");
	bResult &= appcontrol->Startup("");
	appcontrol->SetConfigMode();
	bResult &= appcontrol->AddConfigSearchDir("../../tests/bin/config");

	if (!configFileName.empty())
	{
		bResult &= appcontrol->LoadConfig(configFileName.c_str()) == sdv::core::EConfigProcessResult::successful;
	}

	return bResult;
}

void InitializeCanComObject(CTestCANSocket& canComObj, const std::string config, MockCANReceiver& mockRcv)
{
    ASSERT_NO_THROW(canComObj.Initialize(config.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);
    ASSERT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::configuring));
    ASSERT_NO_THROW(canComObj.RegisterReceiver(&mockRcv));
    EXPECT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::running));
}

void ShutDownCanComObject(CTestCANSocket& canComObj, MockCANReceiver& mockRcv)
{
    EXPECT_NO_THROW(canComObj.UnregisterReceiver(&mockRcv));
    ASSERT_NO_THROW(canComObj.Shutdown());
}

void SendThread(std::atomic_bool& stop, CTestCANSocket& canComObj, sdv::can::SMessage& testData)
{
    while (!stop)
    {
        EXPECT_NO_THROW(canComObj.Send(testData, 1)); // Send to second configuration
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

TEST_F(CANSocketTest, ValidConfigString)
{
    sdv::can::SMessage testMsg {1, 0, 0, {0x11, 0x22, 0x33, 0x44}};

    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(canSockets = "vcan0")"; // vcan0 interface must exist
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    ASSERT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    ASSERT_NO_THROW(canComObj.Send(testMsg, 0));

    ASSERT_NO_THROW(canComObj.Shutdown());
    ASSERT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, InvalidConfigString)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(canSockets = "vcan08")";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialization_failure);

    ASSERT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, ValidConfigArray)
{
    sdv::can::SMessage testMsg {1, 0, 0, {0x11, 0x22, 0x33, 0x44}};

    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(canSockets = ["vcan0", "vcan1"])";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    ASSERT_NO_THROW(canComObj.Send(testMsg, 0));

    EXPECT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, InvalidConfigArray)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(canSockets = ["vcan08", "vcan09"])";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialization_failure);

    ASSERT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, ValidConfigArrayButUnknownElement)
{
    sdv::can::SMessage testMsg {1, 0, 0, {0x11, 0x22, 0x33, 0x44}};

    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(canSockets = ["vcan0", "vcan08"])";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    ASSERT_NO_THROW(canComObj.Send(testMsg, 0));

    ASSERT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, InvalidConfigIdentifier)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig = R"(invalidCanSockets = ["vcan0", "vcan1"])"; // Invalid config identifier
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialization_failure);

    ASSERT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

TEST_F(CANSocketTest, ReceiveTestDifferentDataSizes)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssConfig1 = R"(canSockets = ["vcan1", "vcan2"])";
    sdv::u8string ssConfig2 = R"(canSockets = ["vcan2", "vcan1"])";

    // Object1 sends to Object2; Object2 sends to Object1
    CTestCANSocket canComObj1;
    CTestCANSocket canComObj2;
    MockCANReceiver mockRcv1;
    MockCANReceiver mockRcv2;
    InitializeCanComObject(canComObj1, ssConfig1, mockRcv1);
    InitializeCanComObject(canComObj2, ssConfig2, mockRcv2);

    uint32_t msgId1 = 102;
    uint32_t msgId2 = 201;
    uint32_t dataSize = 1;
    uint32_t expectToBeSend = 8; // other data sizes are to large
    uint32_t msgToBeSend = 16;
    CComTestHelper testHelper;
    while (dataSize < msgToBeSend)
    {
        auto testData1 = testHelper.CreateTestData(msgId1, dataSize);
        auto testData2 = testHelper.CreateTestData(msgId2, dataSize);
        EXPECT_NO_THROW(canComObj1.Send(testData1, 1)); // Send to vcan2
        EXPECT_NO_THROW(canComObj2.Send(testData2, 1)); // Send to vcan1
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        dataSize++;
    }

    auto receivedMessages1 = mockRcv1.GetReceivedMessages();
    auto receivedMessages2 = mockRcv2.GetReceivedMessages();

    if ((expectToBeSend != receivedMessages1.size()) || (expectToBeSend != receivedMessages2.size()))
    {
        std::stringstream sstream;
        sstream << " Failed, expected " << std::to_string(expectToBeSend ) << " messages, got from vcan1 to vcan2: "
                << std::to_string(receivedMessages1.size()) << " messages, from vcan2 to vcan1: "
                << std::to_string(receivedMessages2.size()) << " messages.";
        FAIL() << sstream.str();
    }
    else
    {
        dataSize = 1;
        for(auto receivedMessage : receivedMessages1)
        {
            auto testData = testHelper.CreateTestData(msgId2, dataSize);
            EXPECT_EQ(msgId2, receivedMessage.second.uiID);
            EXPECT_EQ(dataSize++, receivedMessage.second.seqData.size());
            EXPECT_EQ(testData.seqData, receivedMessage.second.seqData);
        }
        dataSize = 1;
        for(auto receivedMessage : receivedMessages2)
        {
            auto testData = testHelper.CreateTestData(msgId1, dataSize);
            EXPECT_EQ(msgId1, receivedMessage.second.uiID);
            EXPECT_EQ(dataSize++, receivedMessage.second.seqData.size());
            EXPECT_EQ(testData.seqData, receivedMessage.second.seqData);
        }
    }

    ShutDownCanComObject(canComObj1, mockRcv1);
    ShutDownCanComObject(canComObj2, mockRcv2);
}

TEST_F(CANSocketTest, ReceiveTestDifferentDataSizesAndInvalidConfiguration)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssConfig1 = R"(canSockets = ["vcan1", "vcan8", "vcan9", "vcan2"])";
    sdv::u8string ssConfig2 = R"(canSockets = ["vcan2", "vcan9", "vcan1"])";

    // Object1 sends to Object2; Object2 sends to Object1
    CTestCANSocket canComObj1;
    CTestCANSocket canComObj2;
    MockCANReceiver mockRcv1;
    MockCANReceiver mockRcv2;
    InitializeCanComObject(canComObj1, ssConfig1, mockRcv1);
    InitializeCanComObject(canComObj2, ssConfig2, mockRcv2);

    CComTestHelper testHelper;
    uint32_t msgId1 = 102;
    uint32_t msgId2 = 201;
    uint32_t dataSize = 1;
    uint32_t expectToBeSend = 8; // other data sizes are to large
    uint32_t msgToBeSend = 16;
    while (dataSize < msgToBeSend)
    {
        auto testData1 = testHelper.CreateTestData(msgId1, dataSize);
        auto testData2 = testHelper.CreateTestData(msgId2, dataSize);
        EXPECT_NO_THROW(canComObj1.Send(testData1, 3)); // Send to vcan2, index 1 & 2 are invalid in config
        EXPECT_NO_THROW(canComObj2.Send(testData2, 2)); // Send to vcan1, index 1 in config
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        dataSize++;
    }

    auto receivedMessages1 = mockRcv1.GetReceivedMessages();
    auto receivedMessages2 = mockRcv2.GetReceivedMessages();

    if ((expectToBeSend != receivedMessages1.size()) || (expectToBeSend != receivedMessages2.size()))
    {
        std::stringstream sstream;
        sstream << " Failed to send " << std::to_string(expectToBeSend ) << " messages, got from vcan1 to vcan2: "
                << std::to_string(receivedMessages1.size()) << " messages, from vcan2 to vcan1: "
                << std::to_string(receivedMessages2.size()) << " messages.";
        FAIL() << sstream.str();
    }
    else
    {
        dataSize = 1;
        for(auto receivedMessage : receivedMessages1)
        {
            auto testData = testHelper.CreateTestData(msgId2, dataSize);
            EXPECT_EQ(msgId2, receivedMessage.second.uiID);
            EXPECT_EQ(dataSize++, receivedMessage.second.seqData.size());
            EXPECT_EQ(testData.seqData, receivedMessage.second.seqData);
        }
        dataSize = 1;
        for(auto receivedMessage : receivedMessages2)
        {
            auto testData = testHelper.CreateTestData(msgId1, dataSize);
            EXPECT_EQ(msgId1, receivedMessage.second.uiID);
            EXPECT_EQ(dataSize++, receivedMessage.second.seqData.size());
            EXPECT_EQ(testData.seqData, receivedMessage.second.seqData);
        }
    }

    ShutDownCanComObject(canComObj1, mockRcv1);
    ShutDownCanComObject(canComObj2, mockRcv2);
}

TEST_F(CANSocketTest, StressTestWith3Objects)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssConfig1 = R"(canSockets = ["vcan1", "vcan2"])";
    sdv::u8string ssConfig2 = R"(canSockets = ["vcan2", "vcan3"])";
    sdv::u8string ssConfig3 = R"(canSockets = ["vcan3", "vcan1"])";

    // Object1 sends to Object2; Object2 sends to Object3; Object3 sends to Object1
    CTestCANSocket canComObj1;
    CTestCANSocket canComObj2;
    CTestCANSocket canComObj3;
    MockCANReceiver mockRcv1;
    MockCANReceiver mockRcv2;
    MockCANReceiver mockRcv3;
    InitializeCanComObject(canComObj1, ssConfig1, mockRcv1);
    InitializeCanComObject(canComObj2, ssConfig2, mockRcv2);
    InitializeCanComObject(canComObj3, ssConfig3, mockRcv3);

    CComTestHelper testHelper;
    auto testData1 = testHelper.CreateTestData(10, 6); // Testdata to send to vcan2, size = 6
    auto testData2 = testHelper.CreateTestData(20, 7); // Testdata to send to vcan3, size = 7
    auto testData3 = testHelper.CreateTestData(30, 8); // Testdata to send to vcan1, size = 8

	std::atomic_bool stopSendThread = false;
    std::cout << "Start thread sending messages..." << std::endl;
	std::thread thSendThread1(SendThread, std::ref(stopSendThread), std::ref(canComObj1), std::ref(testData1));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::thread thSendThread2(SendThread, std::ref(stopSendThread), std::ref(canComObj2), std::ref(testData2));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::thread thSendThread3(SendThread, std::ref(stopSendThread), std::ref(canComObj3), std::ref(testData3));

    std::this_thread::sleep_for(std::chrono::seconds(10));

	stopSendThread = true;
	if (thSendThread1.joinable())
		thSendThread1.join();
    if (thSendThread2.joinable())
		thSendThread2.join();
    if (thSendThread3.joinable())
		thSendThread3.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto receivedMessages1 = mockRcv1.GetReceivedMessages();
    auto receivedMessages2 = mockRcv2.GetReceivedMessages();
    auto receivedMessages3 = mockRcv3.GetReceivedMessages();
    std::cout << "Number of messages from vcan1 to vcan2: " << std::to_string(canComObj1.GetMessagesSent())
              << ", from vcan2 to vcan3: " << std::to_string(canComObj2.GetMessagesSent())
              << ", from vcan3 to vcan1: " << std::to_string(canComObj3.GetMessagesSent()) << std::endl;
    EXPECT_EQ(testHelper.ValidateReceivedMessages(receivedMessages1, testData3, "vcan1", canComObj3.GetMessagesSent()), true);
    EXPECT_EQ(testHelper.ValidateReceivedMessages(receivedMessages2, testData1, "vcan2", canComObj1.GetMessagesSent()), true);
    EXPECT_EQ(testHelper.ValidateReceivedMessages(receivedMessages3, testData2, "vcan3", canComObj2.GetMessagesSent()), true);

    ShutDownCanComObject(canComObj1, mockRcv1);
    ShutDownCanComObject(canComObj2, mockRcv2);
    ShutDownCanComObject(canComObj3, mockRcv3);
}

TEST_F(CANSocketTest, SendWithVAPICompTest)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "test_can_socket.toml");

    EXPECT_EQ(bResult, true);

    // Load SocketCAN
    sdv::TInterfaceAccessPtr ptrCanComObj = sdv::core::GetObject("CAN_Communication_Object");
    EXPECT_NE(&ptrCanComObj, nullptr);

    sdv::can::ISend* pCanSend = ptrCanComObj.GetInterface<sdv::can::ISend>();
    EXPECT_NE(&pCanSend, nullptr);

    ASSERT_NO_THROW(appControl.SetRunningMode());

    // Clear the candump output file if it exists
    std::string candumpFile = "candump_vapi_send.txt";
    try
    {
        if (std::filesystem::exists(candumpFile))
            std::filesystem::remove(candumpFile);
    } catch (const std::filesystem::filesystem_error&)
    {}

    // Start candump in the background
    std::string candumpCommand = "candump vcan3 > " + candumpFile + " &";
    system(candumpCommand.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Ensure candump is ready

    CComTestHelper testHelper;
    uint32_t msgId = 999;
    uint32_t dataSize = 1;
    uint32_t msgToBeSend = 8;
    while (dataSize < msgToBeSend)
    {
        auto testData1 = testHelper.CreateTestData(msgId, dataSize);
        ASSERT_NO_THROW(pCanSend->Send(testData1, 0));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        msgId++;
        dataSize++;
    }

    // Wait for messages to be written to the file
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Compare sent messages with the candump output
    std::ifstream inputFile(candumpFile);
    ASSERT_TRUE(inputFile.is_open()) << "Failed to open candump output file.";

    msgId = 999;
    dataSize = 1;
    std::string line;
    while (std::getline(inputFile, line) && dataSize < msgToBeSend)
    {
        auto expectedData = testHelper.CreateTestData(msgId, dataSize);
        std::stringstream expectedLine;

        // Format the CAN interface name ("vcan3") and the message ID (uiID) in hexadecimal,
        // ensuring it is uppercase, 3 characters wide, and padded with zeros if necessary.
        expectedLine << "vcan3  " << std::hex << std::uppercase << std::setw(3) << std::setfill('0') << expectedData.uiID
                     << "   [" << expectedData.seqData.size() << "] ";

        // Loop through each byte in the data sequence (seqData) and format it as a
        // 2-character wide, zero-padded hexadecimal value, appending it to the stringstream.
        for (auto byte : expectedData.seqData)
        {
            expectedLine << std::setw(2) << std::setfill('0') << std::hex << (int)byte << " ";
        }

        std::string normalizedExpected = testHelper.NormalizeWhitespace(expectedLine.str());
        std::string normalizedActual = testHelper.NormalizeWhitespace(line);

        if (normalizedActual != normalizedExpected)
        {
            FAIL() << "Mismatch in candump output for message ID " << msgId
                   << "\nExpected: " << normalizedExpected
                   << "\nActual: " << normalizedActual;
        }

        msgId++;
        dataSize++;
    }

    inputFile.close();
    ASSERT_NO_THROW(appControl.Shutdown());
    // Stop candump
    system("pkill candump");
}

TEST_F(CANSocketTest, ReceiveWithVAPICompTest)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "test_can_socket.toml");

    EXPECT_EQ(bResult, true);

    // Load SocketCAN
    sdv::TInterfaceAccessPtr ptrCanComObj = sdv::core::GetObject("CAN_Communication_Object");
    EXPECT_NE(&ptrCanComObj, nullptr);

    sdv::can::IRegisterReceiver* pCanRegRcvr = ptrCanComObj.GetInterface<sdv::can::IRegisterReceiver>();
    EXPECT_NE(&pCanRegRcvr, nullptr);

    MockCANReceiver mockRcv;
    ASSERT_NO_THROW(pCanRegRcvr->RegisterReceiver(&mockRcv));

    ASSERT_NO_THROW(appControl.SetRunningMode());

    // Send messages from vcan3 using candump
    CComTestHelper testHelper;
    uint32_t msgId = 777;
    uint32_t dataSize = 1;
    uint32_t msgToBeSend = 5;

    while (dataSize <= msgToBeSend)
    {
        auto testData = testHelper.CreateTestData(msgId, dataSize);
        std::stringstream cansendCommand;
        cansendCommand << "cansend vcan3 " << std::hex << std::uppercase << std::setw(3) << std::setfill('0') << testData.uiID << "#";
        for (auto byte : testData.seqData)
        {
            cansendCommand << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
        }
        system(cansendCommand.str().c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        msgId++;
        dataSize++;
    }

    if (dataSize == msgToBeSend)
    {
        system("pkill cansend");
    }

    // Wait for messages to be received
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto receivedMessages = mockRcv.GetReceivedMessages();

    dataSize = 1;
    msgId = 777;
    for (const auto& receivedMessage : receivedMessages)
    {
        auto expectedData = testHelper.CreateTestData(msgId, dataSize);
        EXPECT_EQ(receivedMessage.second.uiID, expectedData.uiID) << "Mismatch in message ID for data size " << dataSize;
        EXPECT_EQ(receivedMessage.second.seqData, expectedData.seqData) << "Mismatch in message data for ID " << msgId;
        msgId++;
        dataSize++;
    }

    ASSERT_NO_THROW(pCanRegRcvr->UnregisterReceiver(&mockRcv));
    ASSERT_NO_THROW(appControl.Shutdown());
}

// Manual / Visual 'Send' & 'Received' test
// Configuration ["vcan0", "vcan3", "vcan8", "vcan1", "vcan8", "vcan4", "vcan2"])"
//    Index 0 = vcan0
//    Index 1 = vcan3
//    Index 2 = vcan8  // should not exist, expect to be invalid
//    Index 3 = vcan1
//    Index 4 = vcan9  // should not exist, expect to be invalid
//    Index 5 = vcan4
//    Index 6 = vcan2
// Required: vcan0, vcan1, vcan2, vcan3, vcan4
//      sudo ip link add dev vcan0 type vcan
//      sudo ip link set up vcan0
// Show all can interfaces:   ip addr | grep "vcan"
//
// Verifiy Send:
//      On 5 terminals execute command with the correct interface to visualize the sended messages
//      candump vcan0
// Expected: name of interface, MessageId represends interface
//      vcan1  001   [x]  02 04 06 08 0A 0C 0E 10
//
// Verifiy Receive:
//      On another terminal execute
//      cansend vcan4 100#DEADBEEF
// Expected: name of interface, MessageId is +1 than the interface
//      Got Message from: vcan4, MsgID: 256 '222' '173' '190' '239'
TEST_F(CANSocketTest, ManualSendAndReceiveTestOfMulitpleSockets)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    uint32_t vcan0Index = 0; // must fit to the configuration string
    uint32_t vcan1Index = 3;
    uint32_t vcan2Index = 6;
    uint32_t vcan3Index = 1;
    uint32_t vcan4Index = 5;
    sdv::u8string ssConfig = R"(canSockets = ["vcan0", "vcan3", "vcan8", "vcan1", "vcan9", "vcan4", "vcan2"])";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    // Register a receiver.
    MockCANReceiver mockRcv;
    ASSERT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::configuring));
    ASSERT_NO_THROW(canComObj.RegisterReceiver(&mockRcv));
    EXPECT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::running));

    CComTestHelper testHelper;
    sdv::sequence<sdv::u8string> expectedInterfaces{"vcan0", "vcan3", "vcan1", "vcan4", "vcan2"};
    EXPECT_TRUE(testHelper.ValidateInterfaces(expectedInterfaces, canComObj.GetInterfaces()));

    uint32_t msgId = 0;
    uint32_t dataSize = 8;
    auto testData0 = testHelper.CreateTestData(msgId + 0, dataSize); // Testdata for vcan0
    auto testData1 = testHelper.CreateTestData(msgId + 1, dataSize); // Testdata for vcan1
    auto testData2 = testHelper.CreateTestData(msgId + 2, dataSize); // Testdata for vcan2
    auto testData3 = testHelper.CreateTestData(msgId + 3, dataSize); // Testdata for vcan3
    auto testData4 = testHelper.CreateTestData(msgId + 4, dataSize); // Testdata for vcan4
    EXPECT_NO_THROW(canComObj.Send(testData0, vcan0Index));
    EXPECT_NO_THROW(canComObj.Send(testData1, vcan1Index));
    EXPECT_NO_THROW(canComObj.Send(testData2, vcan2Index));
    EXPECT_NO_THROW(canComObj.Send(testData3, vcan3Index));
    EXPECT_NO_THROW(canComObj.Send(testData4, vcan4Index));

    // Wait for messages to be received
    uint32_t waitForMessagesLoop = 1;
    while (waitForMessagesLoop > 0)
    {
        waitForMessagesLoop--;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    auto receivedMessages = mockRcv.GetReceivedMessages();
    for(auto receivedMessage : receivedMessages)
    {
        std::cout << "Got Message from: " << receivedMessage.first << ", MsgID: " << std::to_string(receivedMessage.second.uiID) ;
        for (uint8_t i = 0; i < receivedMessage.second.seqData.size(); ++i)
        {
            std::cout << " '" << std::to_string(receivedMessage.second.seqData[i]) << "'";
        }
        std::cout << std::endl;
    }

    EXPECT_NO_THROW(canComObj.UnregisterReceiver(&mockRcv));
    ASSERT_NO_THROW(canComObj.Shutdown());  // Shutdown the object after the test
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

// Test similar to ManualSendAndReceiveTestOfMulitpleSockets
// but with different messages size
// Result:
//       vcan0  000   [x]  00 02 04 06
//       vcan1  001   [x]  01 03 05 07 09
//       vcan2  002   [x]  02 04 06 08 0A 0C
//       vcan3  003   [x]  03 05 07 09 0B 0D 0F
//       vcan4  no message
TEST_F(CANSocketTest, ManualSendAndReceiveTestWithDifferentDataSizes)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    uint32_t vcan0Index = 0; // must fit to the configuration string
    uint32_t vcan1Index = 3;
    uint32_t vcan2Index = 6;
    uint32_t vcan3Index = 1;
    uint32_t vcan4Index = 5;
    sdv::u8string ssConfig = R"(canSockets = ["vcan0", "vcan3", "vcan8", "vcan1", "vcan9", "vcan4", "vcan2"])";
    CTestCANSocket canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    // Register a receiver.
    MockCANReceiver mockRcv;
    ASSERT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::configuring));
    ASSERT_NO_THROW(canComObj.RegisterReceiver(&mockRcv));
    EXPECT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::running));

    CComTestHelper testHelper;
    sdv::sequence<sdv::u8string> expectedInterfaces{"vcan0", "vcan3", "vcan1", "vcan4", "vcan2"};
    EXPECT_TRUE(testHelper.ValidateInterfaces(expectedInterfaces, canComObj.GetInterfaces()));

    uint32_t msgId = 0;
    uint32_t dataSize = 8;
    auto testData0 = testHelper.CreateTestData(msgId + 0, dataSize -4); // Testdata for vcan0
    auto testData1 = testHelper.CreateTestData(msgId + 1, dataSize -3); // Testdata for vcan1
    auto testData2 = testHelper.CreateTestData(msgId + 2, dataSize -2); // Testdata for vcan2
    auto testData3 = testHelper.CreateTestData(msgId + 3, dataSize -1); // Testdata for vcan3
    auto testData4 = testHelper.CreateTestData(msgId + 4, dataSize +1); // Testdata for vcan4, is to large
    EXPECT_NO_THROW(canComObj.Send(testData0, vcan0Index));
    EXPECT_NO_THROW(canComObj.Send(testData1, vcan1Index));
    EXPECT_NO_THROW(canComObj.Send(testData2, vcan2Index));
    EXPECT_NO_THROW(canComObj.Send(testData3, vcan3Index));
    EXPECT_NO_THROW(canComObj.Send(testData4, vcan4Index));

    // Wait for messages to be received
    uint32_t waitForMessagesLoop = 1;
    while (waitForMessagesLoop > 0)
    {
        waitForMessagesLoop--;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    auto receivedMessages = mockRcv.GetReceivedMessages();
    for(auto receivedMessage : receivedMessages)
    {
        std::cout << "Got Message from: " << receivedMessage.first << ", MsgID: " << std::to_string(receivedMessage.second.uiID) ;
        for (uint8_t i = 0; i < receivedMessage.second.seqData.size(); ++i)
        {
            std::cout << " '" << std::to_string(receivedMessage.second.seqData[i]) << "'";
        }
        std::cout << std::endl;
    }

    EXPECT_NO_THROW(canComObj.UnregisterReceiver(&mockRcv));
    ASSERT_NO_THROW(canComObj.Shutdown());  // Shutdown the object after the test
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::destruction_pending);
}

extern "C" int main(int argc, char* argv[])
{
    CProcessWatchdog watchdog;

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
