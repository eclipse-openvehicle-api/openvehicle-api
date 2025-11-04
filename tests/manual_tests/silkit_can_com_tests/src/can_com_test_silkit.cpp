#include <iostream>
#include <set>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <condition_variable>
#include <gtest/gtest.h>
#include <interfaces/can.h>
#include <support/app_control.h>
#include <support/toml.h>
#include <support/component_impl.h>
#include "../../../global/process_watchdog.h"
#include "../sdv_services/core/toml_parser/parser_toml.h"

/**
 * @brief Dummy CAN Component to be able to load data link for testing purposes.
 * Sends and receives CAN messages with multiple signals.
 */
class CDummyCANSilKit
    : public sdv::CSdvObject
    , public sdv::IObjectControl
    , public sdv::can::IRegisterReceiver
    , public sdv::can::ISend
{
public:

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::can::IRegisterReceiver)
        SDV_INTERFACE_ENTRY(sdv::can::ISend)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("Dummy_CAN_Sockets")
    DECLARE_DEFAULT_OBJECT_NAME("CAN_Communication_Object")
    DECLARE_OBJECT_SINGLETON()

    virtual void Initialize(const sdv::u8string& ) override
    {
        m_StopThread = false;
        //m_thSend2DatalinkThread = std::thread(&CDummyCANSockets::Send2DatalinkThread, this);
        m_status = sdv::EObjectStatus::initialized;
    }

    virtual sdv::EObjectStatus GetStatus() const override
    {
        return m_status;
    }

    void SetOperationMode(sdv::EOperationMode eMode) override
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_status == sdv::EObjectStatus::running || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_status == sdv::EObjectStatus::configuring || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    virtual void Shutdown() override
    {
        m_StopThread = true;
        if (m_thSend2DatalinkThread.joinable())
        {
            m_thSend2DatalinkThread.join();
        }
    }

    virtual void RegisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        if (!pReceiver)
        {
            return;
        }

        std::cout << "Registering Dummy CAN communication receiver...\n";
        std::unique_lock<std::mutex> lock(m_mtxReceivers);
        m_setReceivers.insert(pReceiver);
    }

    virtual void UnregisterReceiver(sdv::can::IReceive* pReceiver) override
    {
        if (!pReceiver)
        {
            return;
        }

        std::cout << "Unregistering Dummy CAN communication receiver...\n";
        std::unique_lock<std::mutex> lock(m_mtxReceivers);
        m_setReceivers.erase(pReceiver);
    }

    virtual void Send(const sdv::can::SMessage& sMsg, uint32_t uiIfcIndex) override
    {
        {
            std::unique_lock<std::mutex> lockReceivers(m_mtxReceivers);
            for (sdv::can::IReceive* pReceiver : m_setReceivers)
            {
                pReceiver->Receive(sMsg, uiIfcIndex);
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_messageSent = true;
        }
        m_cv.notify_one();
    }

    // Synchronization members
    std::condition_variable m_cv;
    std::mutex m_mtx;
    bool m_messageSent = false;

private:
    bool m_StopThread = false;
    std::thread                     m_thSend2DatalinkThread;
    mutable std::mutex              m_mtxReceivers;
    std::set<sdv::can::IReceive*>   m_setReceivers;
    std::atomic<sdv::EObjectStatus> m_status = { sdv::EObjectStatus::initialization_pending };
};

class MockCANReceiver : public sdv::can::IReceive
{
public:
    std::vector<sdv::can::SMessage> vecReceivedMessages;

    void Receive(const sdv::can::SMessage& sMessage, uint32_t) override
    {
        vecReceivedMessages.push_back(sMessage);
    }

    void Error(const sdv::can::SErrorFrame&, uint32_t) override {}
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


std::string ReplaceWhitespaceWithSingleSpaceAndEscapeQuotes(const std::string& input)
{
    std::stringstream result;
    bool inWhitespace = false;

    for (uint8_t singleChar : input)
    {
        if (std::isspace(static_cast<uint8_t>(singleChar)))
        {
            if (!inWhitespace)
            {
                result << ' ';  // Add a single space for whitespace
                inWhitespace = true;
            }
        }
        else if (singleChar == '"')
        {
            // Escape the quote character
            result << "\\\"";
        }
        else
        {
            result << singleChar;  // Add the character as is
            inWhitespace = false;
        }
    }

    return result.str();
}

bool StringToBool(const std::string& value) {
    return value == "true" || value == "1";  // Return true for "true" or "1", false otherwise
}

std::string ExtractValue(const std::string& content, const std::string& key)
{
    std::size_t startPos = content.find(key + " = ");
    if (startPos == std::string::npos) {
        return ""; // Key not found
    }

    startPos += key.length() + 3; // Move past the key and " = "

    // If the value is multi-line (enclosed by triple quotes), find the start and end
    std::size_t endPos;
    if (content.substr(startPos, 3) == "\"\"\"") {  // Check for starting triple quotes
        startPos += 3;  // Skip over the triple quotes
        endPos = content.find("\"\"\"", startPos);  // Look for the closing triple quotes
        if (endPos == std::string::npos) {
            return "";  // End of value not found, invalid format
        }
    } else {
        // Otherwise, find the first newline or end of the string
        endPos = content.find_first_of("\n", startPos);
        if (endPos == std::string::npos) {
            endPos = content.length();
        }
    }

    // Extract the value between startPos and endPos
    std::string value = content.substr(startPos, endPos - startPos);

    // Remove surrounding whitespace and newline characters if present
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());
    value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());

    return value;
}

class CANCommunicationTest : public ::testing::Test
{
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(CANCommunicationTest, BasicSendAndReceiveMessageTest)
{
    // Dummy test message
    sdv::can::SMessage testMsg;
    testMsg.uiID = 0x1234;
    testMsg.bCanFd = false;
    testMsg.bExtended = false;
    testMsg.seqData = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x77, 0x88};

    // Configuration content as a string
    std::string testConfigFileContent = R"(
        [Configuration]
        Version = 100

        [[Component]]
        Path = "task_timer.sdv"
        Class = "TaskTimerService"

        [[Component]]
        Path = "data_dispatch_service.sdv"
        Class = "DataDispatchService"

        [[Component]]
        Path = "can_com_silkit.sdv"
        Class = "CAN_Com_SilKit"
        DebugInfo = true
        SyncMode = true
        CanSilKitChannel = "CAN1"
        CanSilKitNetwork = "PrivateCAN"
        RegistryURI = "silkit://localhost:8500"
        SilKitConfig = """{
                "Logging": {
                    "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
                            },
                        }"""

        [[Component]]
        Path = "can_datalink.sdv"
        Class = "CAN_data_link"
        """
    )";

    sdv::app::CAppControl appControl;
    auto bResult = InitializeAppControl(&appControl, "test_manual_can_com_silkit.toml");
    EXPECT_EQ(bResult, true);

    std::string SilKitChannel = ExtractValue(testConfigFileContent, "CanSilKitChannel");
    EXPECT_EQ(SilKitChannel, "CAN1");

    // Check RegistryURI
    std::string SilKitRegistryUri = ExtractValue(testConfigFileContent, "RegistryURI");
    EXPECT_EQ(SilKitRegistryUri, "silkit://localhost:8500");

    // Check CanSilKitNetwork
    std::string SilKitNetwork = ExtractValue(testConfigFileContent, "CanSilKitNetwork");
    EXPECT_EQ(SilKitNetwork, "PrivateCAN");

    // Check SyncMode
    std::string SyncModeString = ExtractValue(testConfigFileContent, "SyncMode");

    // Convert the extracted value to a bool
    bool SilKitSyncMode = StringToBool(SyncModeString);
    EXPECT_EQ(SilKitSyncMode, true);

    // Check SilKitConfig
    std::string SilKitJSONConfigContent = ExtractValue(testConfigFileContent, "SilKitConfig");
    std::string ExpectedConfigContent = R"({
                Logging: {
                    Sinks: [ { Type: Stdout, Level: Info } ]
                            },
                        })";
    EXPECT_EQ(ReplaceWhitespaceWithSingleSpaceAndEscapeQuotes(SilKitJSONConfigContent), ReplaceWhitespaceWithSingleSpaceAndEscapeQuotes(ExpectedConfigContent));


    // Create an object of the CDummyCANSilKit class
    CDummyCANSilKit canComObj;
    canComObj.Initialize("");

    sdv::can::ISend* pCanSend = &canComObj;
    EXPECT_NE(pCanSend, nullptr);

    sdv::can::IRegisterReceiver* pCanRegRcvr = &canComObj;
    EXPECT_NE(pCanRegRcvr, nullptr);

    MockCANReceiver mockRcv;
    EXPECT_NO_THROW(pCanRegRcvr->RegisterReceiver(&mockRcv));

    appControl.SetRunningMode();

    for (int i = 0; i < 5; ++i)
    {
        // Send a message
        EXPECT_NO_THROW(pCanSend->Send(testMsg, 0));

        // Wait for the message to be received
        std::unique_lock<std::mutex> lock(canComObj.m_mtx);
        canComObj.m_cv.wait(lock, [&] { return canComObj.m_messageSent; });

        // Verify received messages
        ASSERT_FALSE(mockRcv.vecReceivedMessages.empty());
        auto rcvMsg = mockRcv.vecReceivedMessages.front();
        EXPECT_EQ(rcvMsg.uiID, testMsg.uiID);
        EXPECT_EQ(rcvMsg.seqData, testMsg.seqData);

        // Reset the flag for the next iteration
        canComObj.m_messageSent = false;
        mockRcv.vecReceivedMessages.clear();
    }

    EXPECT_NO_THROW(pCanRegRcvr->UnregisterReceiver(&mockRcv));

    // Ensure proper shutdown
    canComObj.Shutdown();
}

TEST_F(CANCommunicationTest, ExtractAndCompareSignals)
{
    // Dummy test message
    sdv::can::SMessage testMsg;
    testMsg.uiID = 0x1234;
    testMsg.bCanFd = false;
    testMsg.bExtended = false;
    testMsg.seqData = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x77, 0x88};

    sdv::app::CAppControl appControl;
    auto bResult = InitializeAppControl(&appControl, "test_manual_can_com_silkit.toml");
    EXPECT_EQ(bResult, true);

    // Create a normal object of the CDummyCANSilKit class
    CDummyCANSilKit canComObj;
    canComObj.Initialize("");

    sdv::can::ISend* pCanSend = &canComObj;
    EXPECT_NE(pCanSend, nullptr);

    sdv::can::IRegisterReceiver* pCanRegRcvr = &canComObj;
    EXPECT_NE(pCanRegRcvr, nullptr);

    MockCANReceiver mockRcv;
    EXPECT_NO_THROW(pCanRegRcvr->RegisterReceiver(&mockRcv));

    appControl.SetRunningMode();

    for (int i = 0; i < 5; ++i)
    {
        // Send a message
        EXPECT_NO_THROW(pCanSend->Send(testMsg, 0));

        // Wait for the message to be received
        std::unique_lock<std::mutex> lock(canComObj.m_mtx);
        canComObj.m_cv.wait(lock, [&] { return canComObj.m_messageSent; });

        // Verify received messages
        ASSERT_FALSE(mockRcv.vecReceivedMessages.empty());
        auto rcvMsg = mockRcv.vecReceivedMessages.front();
        EXPECT_EQ(rcvMsg.uiID, testMsg.uiID);
        EXPECT_EQ(rcvMsg.seqData, testMsg.seqData);

        // Extract and compare signals
        uint32_t sentSpeed = (testMsg.seqData[6] << 8) | testMsg.seqData[7];
        uint32_t receivedSpeed = (rcvMsg.seqData[6] << 8) | rcvMsg.seqData[7];
        EXPECT_EQ(sentSpeed, receivedSpeed);

        int32_t sentSteeringWheel = (testMsg.seqData[4] << 8) | testMsg.seqData[5];
        int32_t receivedSteeringWheel = (rcvMsg.seqData[4] << 8) | rcvMsg.seqData[5];
        EXPECT_EQ(sentSteeringWheel, receivedSteeringWheel);

        // Reset the flag for the next iteration
        canComObj.m_messageSent = false;
        mockRcv.vecReceivedMessages.clear();
    }

    EXPECT_NO_THROW(pCanRegRcvr->UnregisterReceiver(&mockRcv));

    // Ensure proper shutdown
    canComObj.Shutdown();
}

extern "C" int main(int argc, char* argv[])
{
    CProcessWatchdog watchdog;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}