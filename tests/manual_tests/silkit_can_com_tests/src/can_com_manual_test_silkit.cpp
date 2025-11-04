#include "../../../../global/process_watchdog.h"
#include "../include/can_com_test_helper.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

class CANSilkitTest : public ::testing::Test
{
  public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

class CTestCANSilkit : public CCANSilKit
{
  public:
    virtual void Initialize(const sdv::u8string& ssObjectConfig) override
    {
        CCANSilKit::Initialize(ssObjectConfig);
    }

    virtual void Shutdown() override
    {
        CCANSilKit::Shutdown();
    }

    virtual void Send(const sdv::can::SMessage& sMsg, uint32_t uiIfcIndex) override
    {
        CCANSilKit::Send(sMsg, uiIfcIndex);
        m_MessagesSent++;
    }

    virtual void RegisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        CCANSilKit::RegisterReceiver(pReceiver);
    }

    virtual void UnregisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        CCANSilKit::UnregisterReceiver(pReceiver);
    }

    virtual sdv::sequence<sdv::u8string> GetInterfaces() const override
    {
        return CCANSilKit::GetInterfaces();
    }

    sdv::EObjectStatus GetTestStatus() const
    {
        return CCANSilKit::GetStatus();
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
        m_receivedMessages.push_back(std::make_pair(ifIndex, msg));
    }

    void Error(const sdv::can::SErrorFrame&, uint32_t) override {}

    std::deque<std::pair<uint32_t, sdv::can::SMessage>>GetReceivedMessages() const
    {
        return m_receivedMessages;
    }

  private:
    std::deque<std::pair<uint32_t, sdv::can::SMessage>> m_receivedMessages; ///<  Storing index and received messages
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

void InitializeCanComObject(CTestCANSilkit& canComObj, const std::string config, MockCANReceiver& mockRcv)
{
    ASSERT_NO_THROW(canComObj.Initialize(config.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);
    ASSERT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::configuring));
    ASSERT_NO_THROW(canComObj.RegisterReceiver(&mockRcv));
    EXPECT_NO_THROW(canComObj.SetOperationMode(sdv::EOperationMode::running));
}

void ShutDownCanComObject(CTestCANSilkit& canComObj, MockCANReceiver& mockRcv)
{
    EXPECT_NO_THROW(canComObj.UnregisterReceiver(&mockRcv));
    ASSERT_NO_THROW(canComObj.Shutdown());
}

void SendThread(bool& stop, CTestCANSilkit& canComObj, sdv::can::SMessage& testData)
{
    while (!stop)
    {
        EXPECT_NO_THROW(canComObj.Send(testData, 1)); // Send to second configuration
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

TEST_F(CANSilkitTest, ValidConfigString)
{
    sdv::can::SMessage testMsg {1, 0, 0, {0x11, 0x22, 0x33, 0x44}};

    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig =
    R"(
        DebugInfo = true
        SyncMode = true
        SilKitParticipantName = "can_writer"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                            "Logging": {
                                "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                        },
            }"""
    )";
    CTestCANSilkit canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    ASSERT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialized);

    ASSERT_NO_THROW(canComObj.Send(testMsg, 0));

    ASSERT_NO_THROW(canComObj.Shutdown());
    ASSERT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::shutdown_in_progress);
}

TEST_F(CANSilkitTest, InvalidConfigIdentifier)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssObjectConfig =
    R"(
        DebugInfo = true
        SyncMode = true
        InvalidSilKitParticipantName = "can_writer"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                            "Logging": {
                                "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                        },
            }"""
    )";

    CTestCANSilkit canComObj;
    ASSERT_NO_THROW(canComObj.Initialize(ssObjectConfig.c_str()));
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::initialization_failure);

    ASSERT_NO_THROW(canComObj.Shutdown());
    EXPECT_EQ(canComObj.GetStatus(), sdv::EObjectStatus::shutdown_in_progress);
}

TEST_F(CANSilkitTest, SendReceiveTest)
{
    sdv::app::CAppControl appControl;
    appControl.Startup("");

    sdv::u8string ssConfig1 =
    R"(
        DebugInfo = true
        SyncMode = true
        SilKitParticipantName = "can_writer"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                            "Logging": {
                                "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                        },
            }"""
    )";
    sdv::u8string ssConfig2 =
    R"(
        DebugInfo = true
        SyncMode = true
        SilKitParticipantName = "can_reader"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                            "Logging": {
                                "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                        },
            }"""
    )";

    CTestCANSilkit canComObj1;
    CTestCANSilkit canComObj2;
    MockCANReceiver mockRcv;

    canComObj1.Initialize(ssConfig1.c_str());
    ASSERT_EQ(canComObj1.GetStatus(), sdv::EObjectStatus::initialized);
    canComObj2.Initialize(ssConfig2.c_str());
    ASSERT_EQ(canComObj2.GetStatus(), sdv::EObjectStatus::initialized);

    ASSERT_NO_THROW(canComObj1.SetOperationMode(sdv::EOperationMode::running));
    ASSERT_NO_THROW(canComObj2.SetOperationMode(sdv::EOperationMode::configuring));
    ASSERT_NO_THROW(canComObj2.RegisterReceiver(&mockRcv));
    ASSERT_NO_THROW(canComObj2.SetOperationMode(sdv::EOperationMode::running));

    uint8_t msgId = 102;
    uint8_t dataSize = 1;
    uint32_t expectToBeSend = 8;
    uint32_t msgToBeSend = 8;
    CComTestHelper testHelper;
    while (dataSize <= msgToBeSend)
    {
        auto testData = testHelper.CreateTestData(msgId, dataSize);
        EXPECT_NO_THROW(canComObj1.Send(testData, 0)); // Send to CAN1
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        dataSize++;
    }

    auto receivedMessages = mockRcv.GetReceivedMessages();

    if (expectToBeSend != receivedMessages.size())
    {
        std::stringstream sstream;
        sstream << " Failed, expected " << std::to_string(expectToBeSend ) << " messages, got from CAN1 to CAN2: "
                << std::to_string(receivedMessages.size());
        FAIL() << sstream.str();
    }
    else
    {
        dataSize = 1;
        for(auto rcvMsg : receivedMessages)
        {
            auto testData = testHelper.CreateTestData(msgId, dataSize);
            EXPECT_EQ(msgId, rcvMsg.second.uiID);
            EXPECT_EQ(dataSize++, rcvMsg.second.seqData.size());
            EXPECT_EQ(testData.seqData, rcvMsg.second.seqData);
            std::cout << "Expected " << std::to_string(expectToBeSend ) << " messages, got from CAN1 to CAN2: "
                      << std::to_string(receivedMessages.size()) << std::endl;
        }
    }

    ASSERT_NO_THROW(canComObj2.UnregisterReceiver(&mockRcv));
    canComObj1.Shutdown();
    canComObj2.Shutdown();
}
