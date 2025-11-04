#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <utility>
#include <atomic>
#include <chrono>

#include <support/signal_support.h>
#include <interfaces/dispatch.h>
#include <support/app_control.h>
#include <support/timer.h>
#include <support/sdv_test_macro.h>
#include "../../../global/process_watchdog.h"
#include "../global/ascformat/ascreader.cpp"
#include "../global/ascformat/ascwriter.cpp"
#include "../global/exec_dir_helper.h"

#define CDataLink CAscReaderDataLink
#define SObjectClassInstance_CDataLink SObjectClassInstance_CAscReaderDataLink
#define g_CDataLink g_CAscReaderDataLink
#undef CDataLink
#undef SObjectClassInstance_CDataLink
#undef g_CDataLink
#define CDataLink CDbcStructDataLink
#define SObjectClassInstance_CDataLink SObjectClassInstance_CDbcStructDataLink
#define g_CDataLink g_CSbcStructDataLink
#include "generated/dbc_struct_test/can_dl/datalink.cpp"
#undef CDataLink
#undef SObjectClassInstance_CDataLink
#undef g_CDataLink

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(DbcUtilCanDLTest, Prerequisitives)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "asc_writer_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "asc_writer_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_asc.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    sdv::TInterfaceAccessPtr ptrDispatchService = sdv::core::GetObject("DataDispatchService");
    EXPECT_TRUE(ptrDispatchService);

    sdv::TInterfaceAccessPtr ptrCanComObj = sdv::core::GetObject("CAN_Communication_Object");
    EXPECT_TRUE(ptrCanComObj);

    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitBigEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalBE1 = dispatch.AddPublisher("TestBE.SignalBE1");
    EXPECT_TRUE(signalBE1);
    sdv::core::CSignal signalBE2 = dispatch.AddPublisher("TestBE.SignalBE2");
    EXPECT_TRUE(signalBE2);
    sdv::core::CSignal signalBE3 = dispatch.AddPublisher("TestBE.SignalBE3");
    EXPECT_TRUE(signalBE3);
    sdv::core::CSignal signalBE4 = dispatch.AddPublisher("TestBE.SignalBE4");
    EXPECT_TRUE(signalBE4);
    sdv::core::CSignal signalBE5 = dispatch.AddPublisher("TestBE.SignalBE5");
    EXPECT_TRUE(signalBE5);
    sdv::core::CSignal signalBE6 = dispatch.AddPublisher("TestBE.SignalBE6");
    EXPECT_TRUE(signalBE6);
    appcontrol.SetRunningMode();

    EXPECT_NO_THROW(signalBE1.Write(0xa));
    EXPECT_NO_THROW(signalBE2.Write(0x5d));
    EXPECT_NO_THROW(signalBE3.Write(0x31));
    EXPECT_NO_THROW(signalBE4.Write(0x2d));
    EXPECT_NO_THROW(signalBE5.Write(0x96));
    EXPECT_NO_THROW(signalBE6.Write(0x33cc55aa));

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalBE1.Reset();
    signalBE2.Reset();
    signalBE3.Reset();
    signalBE4.Reset();
    signalBE5.Reset();
    signalBE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0x6, {0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x96, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x96, 0x33, 0xCC, 0x55, 0xAA}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, SpontaneousTransmit64BitBigEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalInt64BE = dispatch.AddPublisher("TestInt64BE.SignalInt64BE");
    EXPECT_TRUE(signalInt64BE);
    sdv::core::CSignal signalUint64BE = dispatch.AddPublisher("TestUint64BE.SignalUint64BE");
    EXPECT_TRUE(signalUint64BE);
    sdv::core::CSignal signalScaleInt64BE = dispatch.AddPublisher("TestScale64BE.SignalScaleInt64BE");
    EXPECT_TRUE(signalScaleInt64BE);
    appcontrol.SetRunningMode();

    EXPECT_NO_THROW(signalInt64BE.Write(-6144092013047381999ll));   // 0xaabbccddeeff0011
    EXPECT_NO_THROW(signalUint64BE.Write(0xaabbccddeeff0011));
    EXPECT_NO_THROW(signalScaleInt64BE.Write(987.654321012));       // 0xE5F4C8F374

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalInt64BE.Reset();
    signalUint64BE.Reset();
    signalScaleInt64BE.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xCB, {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}},
        { 0x67, {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}},
        { 0x1F7, {0x00, 0x00, 0x00, 0xE5, 0xF4, 0xC8, 0xF3, 0x74}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveBigEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0x6, {0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x96, 0x00, 0x00, 0x00, 0x00}},
        { 0x6, {0xA5, 0xDC, 0x6D, 0x96, 0x33, 0xCC, 0x55, 0xAA}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    uint32_t rguiVal[6] = {};
    size_t nCnt = 0;
    sdv::core::CSignal signalBE1 = dispatch.Subscribe("TestBE.SignalBE1", [&](sdv::any_t tVal) {rguiVal[0] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE1);
    sdv::core::CSignal signalBE2 = dispatch.Subscribe("TestBE.SignalBE2", [&](sdv::any_t tVal) {rguiVal[1] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE2);
    sdv::core::CSignal signalBE3 = dispatch.Subscribe("TestBE.SignalBE3", [&](sdv::any_t tVal) {rguiVal[2] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE3);
    sdv::core::CSignal signalBE4 = dispatch.Subscribe("TestBE.SignalBE4", [&](sdv::any_t tVal) {rguiVal[3] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE4);
    sdv::core::CSignal signalBE5 = dispatch.Subscribe("TestBE.SignalBE5", [&](sdv::any_t tVal) {rguiVal[4] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE5);
    sdv::core::CSignal signalBE6 = dispatch.Subscribe("TestBE.SignalBE6", [&](sdv::any_t tVal) {rguiVal[5] = tVal; nCnt++; });
    EXPECT_TRUE(signalBE6);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    EXPECT_EQ(nCnt, 36u);
    EXPECT_EQ(rguiVal[0], 0xau);
    EXPECT_EQ(rguiVal[1], 0x5du);
    EXPECT_EQ(rguiVal[2], 0x31u);
    EXPECT_EQ(rguiVal[3], 0x2du);
    EXPECT_EQ(rguiVal[4], 0x96u);
    EXPECT_EQ(rguiVal[5], 0x33cc55aau);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalBE1.Reset();
    signalBE2.Reset();
    signalBE3.Reset();
    signalBE4.Reset();
    signalBE5.Reset();
    signalBE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, Receive64BitBigEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xCB, {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}},
        { 0x67, {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}},
        { 0x1F7, {0x00, 0x00, 0x00, 0xE5, 0xF4, 0xC8, 0xF3, 0x74}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    int64_t iInt64Val = 0;
    uint64_t uiUint64Val = 0u;
    double dScaleInt64Val = 0;
    sdv::core::CSignal signalInt64BE = dispatch.Subscribe("TestInt64BE.SignalInt64BE", [&](sdv::any_t tVal) {iInt64Val = tVal; });
    EXPECT_TRUE(signalInt64BE);
    sdv::core::CSignal signalUint64BE = dispatch.Subscribe("TestUint64BE.SignalUint64BE", [&](sdv::any_t tVal) {uiUint64Val = tVal; });
    EXPECT_TRUE(signalUint64BE);
    sdv::core::CSignal signalScaleInt64BE = dispatch.Subscribe("TestScale64BE.SignalScaleInt64BE", [&](sdv::any_t tVal) {dScaleInt64Val = tVal; });
    EXPECT_TRUE(signalScaleInt64BE);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    EXPECT_EQ(iInt64Val, -6144092013047381999ll);
    EXPECT_EQ(uiUint64Val, 0xaabbccddeeff0011);
    EXPECT_EQ(round(dScaleInt64Val * 1e8), round(987.654321012 * 1e8));

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalInt64BE.Reset();
    signalUint64BE.Reset();
    signalScaleInt64BE.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitLittleEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalLE1 = dispatch.AddPublisher("TestLE.SignalLE1");
    EXPECT_TRUE(signalLE1);
    sdv::core::CSignal signalLE2 = dispatch.AddPublisher("TestLE.SignalLE2");
    EXPECT_TRUE(signalLE2);
    sdv::core::CSignal signalLE3 = dispatch.AddPublisher("TestLE.SignalLE3");
    EXPECT_TRUE(signalLE3);
    sdv::core::CSignal signalLE4 = dispatch.AddPublisher("TestLE.SignalLE4");
    EXPECT_TRUE(signalLE4);
    sdv::core::CSignal signalLE5 = dispatch.AddPublisher("TestLE.SignalLE5");
    EXPECT_TRUE(signalLE5);
    sdv::core::CSignal signalLE6 = dispatch.AddPublisher("TestLE.SignalLE6");
    EXPECT_TRUE(signalLE6);
    appcontrol.SetRunningMode();

    EXPECT_NO_THROW(signalLE1.Write(0xa));
    EXPECT_NO_THROW(signalLE2.Write(0x5d));
    EXPECT_NO_THROW(signalLE3.Write(0x31));
    EXPECT_NO_THROW(signalLE4.Write(0x2d));
    EXPECT_NO_THROW(signalLE5.Write(0x96));
    EXPECT_NO_THROW(signalLE6.Write(0x33cc55aa));

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalLE1.Reset();
    signalLE2.Reset();
    signalLE3.Reset();
    signalLE4.Reset();
    signalLE5.Reset();
    signalLE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0x7, {0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x96, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x96, 0xAA, 0x55, 0xCC, 0x33}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, SpontaneousTransmit64BitLittleEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalInt64LE = dispatch.AddPublisher("TestInt64LE.SignalInt64LE");
    EXPECT_TRUE(signalInt64LE);
    sdv::core::CSignal signalUint64LE = dispatch.AddPublisher("TestUint64LE.SignalUint64LE");
    EXPECT_TRUE(signalUint64LE);
    sdv::core::CSignal signalScaleInt64LE = dispatch.AddPublisher("TestScale64LE.SignalScaleInt64LE");
    EXPECT_TRUE(signalScaleInt64LE);
    appcontrol.SetRunningMode();

    EXPECT_NO_THROW(signalInt64LE.Write(-6144092013047381999ll));   // 0xaabbccddeeff0011
    EXPECT_NO_THROW(signalUint64LE.Write(0xaabbccddeeff0011));
    EXPECT_NO_THROW(signalScaleInt64LE.Write(987.654321012));       // 0xE5F4C8F374

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalInt64LE.Reset();
    signalUint64LE.Reset();
    signalScaleInt64LE.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xCA, {0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}},
        { 0x66, {0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}},
        { 0x1F6, {0x74, 0xF3, 0xC8, 0xF4, 0xE5, 0x00, 0x00, 0x00}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveLittleEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0x7, {0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x00, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x96, 0x00, 0x00, 0x00, 0x00}},
        { 0x7, {0xDA, 0x15, 0xB7, 0x96, 0xAA, 0x55, 0xCC, 0x33}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    uint32_t rguiVal[6] = {};
    size_t nCnt = 0;
    sdv::core::CSignal signalLE1 = dispatch.Subscribe("TestLE.SignalLE1", [&](sdv::any_t tVal) {rguiVal[0] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE1);
    sdv::core::CSignal signalLE2 = dispatch.Subscribe("TestLE.SignalLE2", [&](sdv::any_t tVal) {rguiVal[1] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE2);
    sdv::core::CSignal signalLE3 = dispatch.Subscribe("TestLE.SignalLE3", [&](sdv::any_t tVal) {rguiVal[2] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE3);
    sdv::core::CSignal signalLE4 = dispatch.Subscribe("TestLE.SignalLE4", [&](sdv::any_t tVal) {rguiVal[3] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE4);
    sdv::core::CSignal signalLE5 = dispatch.Subscribe("TestLE.SignalLE5", [&](sdv::any_t tVal) {rguiVal[4] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE5);
    sdv::core::CSignal signalLE6 = dispatch.Subscribe("TestLE.SignalLE6", [&](sdv::any_t tVal) {rguiVal[5] = tVal; nCnt++; });
    EXPECT_TRUE(signalLE6);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    EXPECT_EQ(nCnt, 36u);
    EXPECT_EQ(rguiVal[0], 0xau);
    EXPECT_EQ(rguiVal[1], 0x5du);
    EXPECT_EQ(rguiVal[2], 0x31u);
    EXPECT_EQ(rguiVal[3], 0x2du);
    EXPECT_EQ(rguiVal[4], 0x96u);
    EXPECT_EQ(rguiVal[5], 0x33cc55aau);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalLE1.Reset();
    signalLE2.Reset();
    signalLE3.Reset();
    signalLE4.Reset();
    signalLE5.Reset();
    signalLE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, Receive64BitLittleEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xCA, {0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}},
        { 0x66, {0x11, 0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}},
        { 0x1F6, {0x74, 0xF3, 0xC8, 0xF4, 0xE5, 0x00, 0x00, 0x00}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    int64_t iInt64Val = 0;
    uint64_t uiUint64Val = 0u;
    double dScaleInt64Val = 0;
    sdv::core::CSignal signalInt64LE = dispatch.Subscribe("TestInt64LE.SignalInt64LE", [&](sdv::any_t tVal) {iInt64Val = tVal; });
    EXPECT_TRUE(signalInt64LE);
    sdv::core::CSignal signalUint64LE = dispatch.Subscribe("TestUint64LE.SignalUint64LE", [&](sdv::any_t tVal) {uiUint64Val = tVal; });
    EXPECT_TRUE(signalUint64LE);
    sdv::core::CSignal signalScaleInt64LE = dispatch.Subscribe("TestScale64LE.SignalScaleInt64LE", [&](sdv::any_t tVal) {dScaleInt64Val = tVal; });
    EXPECT_TRUE(signalScaleInt64LE);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    EXPECT_EQ(iInt64Val, -6144092013047381999ll);
    EXPECT_EQ(uiUint64Val, 0xaabbccddeeff0011);
    EXPECT_EQ(round(dScaleInt64Val * 1e8), round(987.654321012 * 1e8));

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalInt64LE.Reset();
    signalUint64LE.Reset();
    signalScaleInt64LE.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransactionalTransmitBigEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalBE1 = dispatch.AddPublisher("TestBE.SignalBE1");
    EXPECT_TRUE(signalBE1);
    sdv::core::CSignal signalBE2 = dispatch.AddPublisher("TestBE.SignalBE2");
    EXPECT_TRUE(signalBE2);
    sdv::core::CSignal signalBE3 = dispatch.AddPublisher("TestBE.SignalBE3");
    EXPECT_TRUE(signalBE3);
    sdv::core::CSignal signalBE4 = dispatch.AddPublisher("TestBE.SignalBE4");
    EXPECT_TRUE(signalBE4);
    sdv::core::CSignal signalBE5 = dispatch.AddPublisher("TestBE.SignalBE5");
    EXPECT_TRUE(signalBE5);
    sdv::core::CSignal signalBE6 = dispatch.AddPublisher("TestBE.SignalBE6");
    EXPECT_TRUE(signalBE6);
    appcontrol.SetRunningMode();

    sdv::core::CTransaction transaction = dispatch.CreateTransaction();
    EXPECT_TRUE(transaction);
    EXPECT_NO_THROW(signalBE1.Write(0xa, transaction));
    EXPECT_NO_THROW(signalBE2.Write(0x5d, transaction));
    EXPECT_NO_THROW(signalBE3.Write(0x31, transaction));
    EXPECT_NO_THROW(signalBE4.Write(0x2d, transaction));
    EXPECT_NO_THROW(signalBE5.Write(0x96, transaction));
    EXPECT_NO_THROW(signalBE6.Write(0x33cc55aa, transaction));
    transaction.Finish();

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalBE1.Reset();
    signalBE2.Reset();
    signalBE3.Reset();
    signalBE4.Reset();
    signalBE5.Reset();
    signalBE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    // Get first message
    std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
    EXPECT_TRUE(prMsg.second);
    ++reader;
    EXPECT_EQ(prMsg.first.uiChannel, 1ul);
    EXPECT_EQ(prMsg.first.uiId, 6ul);
    EXPECT_FALSE(prMsg.first.bExtended);
    EXPECT_FALSE(prMsg.first.bCanFd);
    EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
    EXPECT_EQ(prMsg.first.uiLength, 8ul);
    EXPECT_EQ(prMsg.first.rguiData[0], 0xa5);
    EXPECT_EQ(prMsg.first.rguiData[1], 0xdC);
    EXPECT_EQ(prMsg.first.rguiData[2], 0x6d);
    EXPECT_EQ(prMsg.first.rguiData[3], 0x96);
    EXPECT_EQ(prMsg.first.rguiData[4], 0x33);
    EXPECT_EQ(prMsg.first.rguiData[5], 0xcc);
    EXPECT_EQ(prMsg.first.rguiData[6], 0x55);
    EXPECT_EQ(prMsg.first.rguiData[7], 0xaa);
}

TEST(DbcUtilCanDLTest, SpontaneousTransactionalTransmitLittleEndian)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalLE1 = dispatch.AddPublisher("TestLE.SignalLE1");
    EXPECT_TRUE(signalLE1);
    sdv::core::CSignal signalLE2 = dispatch.AddPublisher("TestLE.SignalLE2");
    EXPECT_TRUE(signalLE2);
    sdv::core::CSignal signalLE3 = dispatch.AddPublisher("TestLE.SignalLE3");
    EXPECT_TRUE(signalLE3);
    sdv::core::CSignal signalLE4 = dispatch.AddPublisher("TestLE.SignalLE4");
    EXPECT_TRUE(signalLE4);
    sdv::core::CSignal signalLE5 = dispatch.AddPublisher("TestLE.SignalLE5");
    EXPECT_TRUE(signalLE5);
    sdv::core::CSignal signalLE6 = dispatch.AddPublisher("TestLE.SignalLE6");
    EXPECT_TRUE(signalLE6);
    appcontrol.SetRunningMode();

    sdv::core::CTransaction transaction = dispatch.CreateTransaction();
    EXPECT_TRUE(transaction);
    EXPECT_NO_THROW(signalLE1.Write(0xa, transaction));
    EXPECT_NO_THROW(signalLE2.Write(0x5d, transaction));
    EXPECT_NO_THROW(signalLE3.Write(0x31, transaction));
    EXPECT_NO_THROW(signalLE4.Write(0x2d, transaction));
    EXPECT_NO_THROW(signalLE5.Write(0x96, transaction));
    EXPECT_NO_THROW(signalLE6.Write(0x33cc55aa, transaction));
    transaction.Finish();

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalLE1.Reset();
    signalLE2.Reset();
    signalLE3.Reset();
    signalLE4.Reset();
    signalLE5.Reset();
    signalLE6.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    // Get first message
    std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
    EXPECT_TRUE(prMsg.second);
    ++reader;
    EXPECT_EQ(prMsg.first.uiChannel, 1ul);
    EXPECT_EQ(prMsg.first.uiId, 7ul);
    EXPECT_FALSE(prMsg.first.bExtended);
    EXPECT_FALSE(prMsg.first.bCanFd);
    EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
    EXPECT_EQ(prMsg.first.uiLength, 8ul);
    EXPECT_EQ(prMsg.first.rguiData[0], 0xda);
    EXPECT_EQ(prMsg.first.rguiData[1], 0x15);
    EXPECT_EQ(prMsg.first.rguiData[2], 0xb7);
    EXPECT_EQ(prMsg.first.rguiData[3], 0x96);
    EXPECT_EQ(prMsg.first.rguiData[4], 0xaa);
    EXPECT_EQ(prMsg.first.rguiData[5], 0x55);
    EXPECT_EQ(prMsg.first.rguiData[6], 0xcc);
    EXPECT_EQ(prMsg.first.rguiData[7], 0x33);
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitBigEndianAllDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to integer based signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalIntBE1 = dispatch.AddPublisher("TestIntBE.SignalIntBE1");
    EXPECT_TRUE(signalIntBE1);
    sdv::core::CSignal signalIntBE2 = dispatch.AddPublisher("TestIntBE.SignalIntBE2");
    EXPECT_TRUE(signalIntBE2);
    sdv::core::CSignal signalIntBE3 = dispatch.AddPublisher("TestIntBE.SignalIntBE3");
    EXPECT_TRUE(signalIntBE3);
    sdv::core::CSignal signalIntBE4 = dispatch.AddPublisher("TestIntBE.SignalIntBE4");
    EXPECT_TRUE(signalIntBE4);
    sdv::core::CSignal signalIntBE5 = dispatch.AddPublisher("TestIntBE.SignalIntBE5");
    EXPECT_TRUE(signalIntBE5);
    sdv::core::CSignal signalIntBE6 = dispatch.AddPublisher("TestIntBE.SignalIntBE6");
    EXPECT_TRUE(signalIntBE6);

    // Subscribe to unsigned integer based signals
    sdv::core::CSignal signalUintBE1 = dispatch.AddPublisher("TestUintBE.SignalUintBE1");
    EXPECT_TRUE(signalUintBE1);
    sdv::core::CSignal signalUintBE2 = dispatch.AddPublisher("TestUintBE.SignalUintBE2");
    EXPECT_TRUE(signalUintBE2);
    sdv::core::CSignal signalUintBE3 = dispatch.AddPublisher("TestUintBE.SignalUintBE3");
    EXPECT_TRUE(signalUintBE3);
    sdv::core::CSignal signalUintBE4 = dispatch.AddPublisher("TestUintBE.SignalUintBE4");
    EXPECT_TRUE(signalUintBE4);
    sdv::core::CSignal signalUintBE5 = dispatch.AddPublisher("TestUintBE.SignalUintBE5");
    EXPECT_TRUE(signalUintBE5);
    sdv::core::CSignal signalUintBE6 = dispatch.AddPublisher("TestUintBE.SignalUintBE6");
    EXPECT_TRUE(signalUintBE6);

    // Subscribe to floating point based signals
    sdv::core::CSignal signalFloatBE1 = dispatch.AddPublisher("TestFloatBE.SignalFloatBE1");
    EXPECT_TRUE(signalFloatBE1);
    sdv::core::CSignal signalFloatBE2 = dispatch.AddPublisher("TestFloatBE.SignalFloatBE2");
    EXPECT_TRUE(signalFloatBE2);

    // Subscribe to double precision floating point based signals
    sdv::core::CSignal signalDoubleBE1 = dispatch.AddPublisher("TestDoubleBE.SignalDoubleBE1");
    EXPECT_TRUE(signalDoubleBE1);
    appcontrol.SetRunningMode();

    for (size_t n = 10; n < 20; n++)
    {
        sdv::core::CTransaction transaction = dispatch.CreateTransaction();
        signalIntBE1.Write((static_cast<int32_t>(n) & 0b1) * (n & 1 ? -1 : 1), transaction);     // 2 bits incl. sign
        signalIntBE2.Write((static_cast<int32_t>(n) & 0b1111) * (n & 1 ? -1 : 1), transaction);  // 5 bits incl. sign
        signalIntBE3.Write((static_cast<int32_t>(n) & 0b11111) * (n & 1 ? -1 : 1), transaction); // 6 bits incl. sign
        signalIntBE4.Write((static_cast<int32_t>(n) & 0b11) * (n & 1 ? -1 : 1), transaction);    // 3 bits incl. sign
        signalIntBE5.Write((static_cast<int32_t>(n)) * (n & 1 ? -1 : 1), transaction);           // 18 bits incl. sign
        signalIntBE6.Write((static_cast<int32_t>(n)) * (n & 1 ? -1 : 1), transaction);           // 30 bits incl. sign
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalUintBE1.Write(static_cast<uint32_t>(n) & 0b1, transaction);        // 1 bit
        signalUintBE2.Write(static_cast<uint32_t>(n) & 0b11, transaction);       // 2 bits
        signalUintBE3.Write(static_cast<uint32_t>(n) & 0b111111, transaction);   // 6 bits
        signalUintBE4.Write(static_cast<uint32_t>(n) & 0b1111111, transaction);  // 7 bits
        signalUintBE5.Write(static_cast<uint32_t>(n), transaction);              // 18 bits
        signalUintBE6.Write(static_cast<uint32_t>(n), transaction);              // 30 bits
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalFloatBE1.Write(static_cast<float>(n) * 1.111f, transaction);
        signalFloatBE2.Write(static_cast<float>(n) * 2.222f, transaction);
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalDoubleBE1.Write(static_cast<double>(n) * 3.333, transaction);
        transaction.Finish();
    }

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalIntBE1.Reset();
    signalIntBE2.Reset();
    signalIntBE3.Reset();
    signalIntBE4.Reset();
    signalIntBE5.Reset();
    signalIntBE6.Reset();

    signalUintBE1.Reset();
    signalUintBE2.Reset();
    signalUintBE3.Reset();
    signalUintBE4.Reset();
    signalUintBE5.Reset();
    signalUintBE6.Reset();

    signalFloatBE1.Reset();
    signalFloatBE2.Reset();

    signalDoubleBE1.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xC9 , {0x14, 0x52, 0x00, 0x02, 0x80, 0x00, 0x00, 0x0A}},
        { 0x65 , {0x45, 0x0A, 0x00, 0x02, 0x80, 0x00, 0x00, 0x0A}},
        { 0x12D, {0x41, 0x31, 0xC2, 0x8F, 0x41, 0xB1, 0xC2, 0x8F}},
        { 0x191, {0x40, 0x40, 0xAA, 0x3D, 0x70, 0xA3, 0xD7, 0x0A}},
        { 0xC9 , {0xEB, 0xAD, 0xFF, 0xFD, 0x7F, 0xFF, 0xFF, 0xF5}},
        { 0x65 , {0xE5, 0x8B, 0x00, 0x02, 0xC0, 0x00, 0x00, 0x0B}},
        { 0x12D, {0x41, 0x43, 0x89, 0x37, 0x41, 0xC3, 0x89, 0x37}},
        { 0x191, {0x40, 0x42, 0x54, 0xDD, 0x2F, 0x1A, 0x9F, 0xBF}},
        { 0xC9 , {0x18, 0x60, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C}},
        { 0x65 , {0x06, 0x0C, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C}},
        { 0x12D, {0x41, 0x55, 0x4F, 0xDE, 0x41, 0xD5, 0x4F, 0xDE}},
        { 0x191, {0x40, 0x43, 0xFF, 0x7C, 0xED, 0x91, 0x68, 0x73}},
        { 0xC9 , {0xE7, 0x9F, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xF3}},
        { 0x65 , {0xA6, 0x8D, 0x00, 0x03, 0x40, 0x00, 0x00, 0x0D}},
        { 0x12D, {0x41, 0x67, 0x16, 0x86, 0x41, 0xE7, 0x16, 0x86}},
        { 0x191, {0x40, 0x45, 0xAA, 0x1C, 0xAC, 0x08, 0x31, 0x27}},
        { 0xC9 , {0x1C, 0x72, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0E}},
        { 0x65 , {0x47, 0x0E, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0E}},
        { 0x12D, {0x41, 0x78, 0xDD, 0x2E, 0x41, 0xF8, 0xDD, 0x2E}},
        { 0x191, {0x40, 0x47, 0x54, 0xBC, 0x6A, 0x7E, 0xF9, 0xDC}},
        { 0xC9 , {0xE3, 0x8D, 0xFF, 0xFC, 0x7F, 0xFF, 0xFF, 0xF1}},
        { 0x65 , {0xE7, 0x8F, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x0F}},
        { 0x12D, {0x41, 0x85, 0x51, 0xEB, 0x42, 0x05, 0x51, 0xEB}},
        { 0x191, {0x40, 0x48, 0xFF, 0x5C, 0x28, 0xF5, 0xC2, 0x90}},
        { 0xC9 , {0x00, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10}},
        { 0x65 , {0x08, 0x10, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10}},
        { 0x12D, {0x41, 0x8E, 0x35, 0x3F, 0x42, 0x0E, 0x35, 0x3F}},
        { 0x191, {0x40, 0x4A, 0xA9, 0xFB, 0xE7, 0x6C, 0x8B, 0x44}},
        { 0xC9 , {0xFF, 0x7F, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xEF}},
        { 0x65 , {0xA8, 0x91, 0x00, 0x04, 0x40, 0x00, 0x00, 0x11}},
        { 0x12D, {0x41, 0x97, 0x18, 0x93, 0x42, 0x17, 0x18, 0x93}},
        { 0x191, {0x40, 0x4C, 0x54, 0x9B, 0xA5, 0xE3, 0x53, 0xF8}},
        { 0xC9 , {0x04, 0x92, 0x00, 0x04, 0x80, 0x00, 0x00, 0x12}},
        { 0x65 , {0x49, 0x12, 0x00, 0x04, 0x80, 0x00, 0x00, 0x12}},
        { 0x12D, {0x41, 0x9F, 0xFB, 0xE7, 0x42, 0x1F, 0xFB, 0xE7}},
        { 0x191, {0x40, 0x4D, 0xFF, 0x3B, 0x64, 0x5A, 0x1C, 0xAC}},
        { 0xC9 , {0xFB, 0x6D, 0xFF, 0xFB, 0x7F, 0xFF, 0xFF, 0xED}},
        { 0x65 , {0xE9, 0x93, 0x00, 0x04, 0xC0, 0x00, 0x00, 0x13}},
        { 0x12D, {0x41, 0xA8, 0xDF, 0x3B, 0x42, 0x28, 0xDF, 0x3B}},
        { 0x191, {0x40, 0x4F, 0xA9, 0xDB, 0x22, 0xD0, 0xE5, 0x61}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveBigEndianAllDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xC9 , {0x14, 0x52, 0x00, 0x02, 0x80, 0x00, 0x00, 0x0A}},
        { 0x65 , {0x45, 0x0A, 0x00, 0x02, 0x80, 0x00, 0x00, 0x0A}},
        { 0x12D, {0x41, 0x31, 0xC2, 0x8F, 0x41, 0xB1, 0xC2, 0x8F}},
        { 0x191, {0x40, 0x40, 0xAA, 0x3D, 0x70, 0xA3, 0xD7, 0x0A}},
        { 0xC9 , {0xEB, 0xAD, 0xFF, 0xFD, 0x7F, 0xFF, 0xFF, 0xF5}},
        { 0x65 , {0xE5, 0x8B, 0x00, 0x02, 0xC0, 0x00, 0x00, 0x0B}},
        { 0x12D, {0x41, 0x43, 0x89, 0x37, 0x41, 0xC3, 0x89, 0x37}},
        { 0x191, {0x40, 0x42, 0x54, 0xDD, 0x2F, 0x1A, 0x9F, 0xBF}},
        { 0xC9 , {0x18, 0x60, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C}},
        { 0x65 , {0x06, 0x0C, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C}},
        { 0x12D, {0x41, 0x55, 0x4F, 0xDE, 0x41, 0xD5, 0x4F, 0xDE}},
        { 0x191, {0x40, 0x43, 0xFF, 0x7C, 0xED, 0x91, 0x68, 0x73}},
        { 0xC9 , {0xE7, 0x9F, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xF3}},
        { 0x65 , {0xA6, 0x8D, 0x00, 0x03, 0x40, 0x00, 0x00, 0x0D}},
        { 0x12D, {0x41, 0x67, 0x16, 0x86, 0x41, 0xE7, 0x16, 0x86}},
        { 0x191, {0x40, 0x45, 0xAA, 0x1C, 0xAC, 0x08, 0x31, 0x27}},
        { 0xC9 , {0x1C, 0x72, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0E}},
        { 0x65 , {0x47, 0x0E, 0x00, 0x03, 0x80, 0x00, 0x00, 0x0E}},
        { 0x12D, {0x41, 0x78, 0xDD, 0x2E, 0x41, 0xF8, 0xDD, 0x2E}},
        { 0x191, {0x40, 0x47, 0x54, 0xBC, 0x6A, 0x7E, 0xF9, 0xDC}},
        { 0xC9 , {0xE3, 0x8D, 0xFF, 0xFC, 0x7F, 0xFF, 0xFF, 0xF1}},
        { 0x65 , {0xE7, 0x8F, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x0F}},
        { 0x12D, {0x41, 0x85, 0x51, 0xEB, 0x42, 0x05, 0x51, 0xEB}},
        { 0x191, {0x40, 0x48, 0xFF, 0x5C, 0x28, 0xF5, 0xC2, 0x90}},
        { 0xC9 , {0x00, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10}},
        { 0x65 , {0x08, 0x10, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10}},
        { 0x12D, {0x41, 0x8E, 0x35, 0x3F, 0x42, 0x0E, 0x35, 0x3F}},
        { 0x191, {0x40, 0x4A, 0xA9, 0xFB, 0xE7, 0x6C, 0x8B, 0x44}},
        { 0xC9 , {0xFF, 0x7F, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xEF}},
        { 0x65 , {0xA8, 0x91, 0x00, 0x04, 0x40, 0x00, 0x00, 0x11}},
        { 0x12D, {0x41, 0x97, 0x18, 0x93, 0x42, 0x17, 0x18, 0x93}},
        { 0x191, {0x40, 0x4C, 0x54, 0x9B, 0xA5, 0xE3, 0x53, 0xF8}},
        { 0xC9 , {0x04, 0x92, 0x00, 0x04, 0x80, 0x00, 0x00, 0x12}},
        { 0x65 , {0x49, 0x12, 0x00, 0x04, 0x80, 0x00, 0x00, 0x12}},
        { 0x12D, {0x41, 0x9F, 0xFB, 0xE7, 0x42, 0x1F, 0xFB, 0xE7}},
        { 0x191, {0x40, 0x4D, 0xFF, 0x3B, 0x64, 0x5A, 0x1C, 0xAC}},
        { 0xC9 , {0xFB, 0x6D, 0xFF, 0xFB, 0x7F, 0xFF, 0xFF, 0xED}},
        { 0x65 , {0xE9, 0x93, 0x00, 0x04, 0xC0, 0x00, 0x00, 0x13}},
        { 0x12D, {0x41, 0xA8, 0xDF, 0x3B, 0x42, 0x28, 0xDF, 0x3B}},
        { 0x191, {0x40, 0x4F, 0xA9, 0xDB, 0x22, 0xD0, 0xE5, 0x61}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    int32_t rgiIntCnt[6] = {10, 10, 10, 10, 10, 10};
    sdv::core::CSignal signalIntBE1 = dispatch.Subscribe("TestIntBE.SignalIntBE1", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[0]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b1) * (i & 1 ? -1 : 1);   // 2 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE1);
    sdv::core::CSignal signalIntBE2 = dispatch.Subscribe("TestIntBE.SignalIntBE2", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[1]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b1111) * (i & 1 ? -1 : 1);    // 5 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE2);
    sdv::core::CSignal signalIntBE3 = dispatch.Subscribe("TestIntBE.SignalIntBE3", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[2]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b11111) * (i & 1 ? -1 : 1);   // 6 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE3);
    sdv::core::CSignal signalIntBE4 = dispatch.Subscribe("TestIntBE.SignalIntBE4", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[3]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b11) * (i & 1 ? -1 : 1);  // 3 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE4);
    sdv::core::CSignal signalIntBE5 = dispatch.Subscribe("TestIntBE.SignalIntBE5", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[4]++;
            int32_t iCalculated = (static_cast<int32_t>(i)) * (i & 1 ? -1 : 1); // 18 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE5);
    sdv::core::CSignal signalIntBE6 = dispatch.Subscribe("TestIntBE.SignalIntBE6", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[5]++;
            int32_t iCalculated = (static_cast<int32_t>(i)) * (i & 1 ? -1 : 1); // 30 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntBE6);

    // Subscribe to unsigned integer based signals
    uint32_t rguiUintCnt[6] = {10u, 10u, 10u, 10u, 10u, 10u};
    sdv::core::CSignal signalUintBE1 = dispatch.Subscribe("TestUintBE.SignalUintBE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[0]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b1;   // 1 bit
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE1);
    sdv::core::CSignal signalUintBE2 = dispatch.Subscribe("TestUintBE.SignalUintBE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[1]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b11;   // 2 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE2);
    sdv::core::CSignal signalUintBE3 = dispatch.Subscribe("TestUintBE.SignalUintBE3", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[2]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b111111;   // 6 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE3);
    sdv::core::CSignal signalUintBE4 = dispatch.Subscribe("TestUintBE.SignalUintBE4", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[3]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b1111111;   // 7 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE4);
    sdv::core::CSignal signalUintBE5 = dispatch.Subscribe("TestUintBE.SignalUintBE5", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[4]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui);   // 18 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE5);
    sdv::core::CSignal signalUintBE6 = dispatch.Subscribe("TestUintBE.SignalUintBE6", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[5]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui);   // 30 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE6);

    // Subscribe to floating point based signals
    uint32_t rguiFloatCnt[2] = {10u, 10u};
    sdv::core::CSignal signalFloatBE1 = dispatch.Subscribe("TestFloatBE.SignalFloatBE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiFloatCnt[0]++;
            float fCalculated = static_cast<float>(ui) * 1.111f;
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalFloatBE1);
    sdv::core::CSignal signalFloatBE2 = dispatch.Subscribe("TestFloatBE.SignalFloatBE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiFloatCnt[1]++;
            float fCalculated = static_cast<float>(ui) * 2.222f;
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalFloatBE2);

    // Subscribe to double precision floating point based signals
    uint32_t uiDoubleCnt = 10u;
    sdv::core::CSignal signalDoubleBE1 = dispatch.Subscribe("TestDoubleBE.SignalDoubleBE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = uiDoubleCnt++;
            double dCalculated = static_cast<double>(ui) * 3.333;
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalDoubleBE1);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Check for correct counters
    for (int32_t i : rgiIntCnt)
        EXPECT_EQ(i, 20);
    for (uint32_t ui : rguiUintCnt)
        EXPECT_EQ(ui, 20u);
    for (uint32_t ui : rguiFloatCnt)
        EXPECT_EQ(ui, 20u);
    EXPECT_EQ(uiDoubleCnt, 20u);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalIntBE1.Reset();
    signalIntBE2.Reset();
    signalIntBE3.Reset();
    signalIntBE4.Reset();
    signalIntBE5.Reset();
    signalIntBE6.Reset();

    signalUintBE1.Reset();
    signalUintBE2.Reset();
    signalUintBE3.Reset();
    signalUintBE4.Reset();
    signalUintBE5.Reset();
    signalUintBE6.Reset();

    signalFloatBE1.Reset();
    signalFloatBE2.Reset();

    signalDoubleBE1.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitLittleEndianAllDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to integer based signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalIntLE1 = dispatch.AddPublisher("TestIntLE.SignalIntLE1");
    EXPECT_TRUE(signalIntLE1);
    sdv::core::CSignal signalIntLE2 = dispatch.AddPublisher("TestIntLE.SignalIntLE2");
    EXPECT_TRUE(signalIntLE2);
    sdv::core::CSignal signalIntLE3 = dispatch.AddPublisher("TestIntLE.SignalIntLE3");
    EXPECT_TRUE(signalIntLE3);
    sdv::core::CSignal signalIntLE4 = dispatch.AddPublisher("TestIntLE.SignalIntLE4");
    EXPECT_TRUE(signalIntLE4);
    sdv::core::CSignal signalIntLE5 = dispatch.AddPublisher("TestIntLE.SignalIntLE5");
    EXPECT_TRUE(signalIntLE5);
    sdv::core::CSignal signalIntLE6 = dispatch.AddPublisher("TestIntLE.SignalIntLE6");
    EXPECT_TRUE(signalIntLE6);

    // Subscribe to unsigned integer based signals
    sdv::core::CSignal signalUintLE1 = dispatch.AddPublisher("TestUintLE.SignalUintLE1");
    EXPECT_TRUE(signalUintLE1);
    sdv::core::CSignal signalUintLE2 = dispatch.AddPublisher("TestUintLE.SignalUintLE2");
    EXPECT_TRUE(signalUintLE2);
    sdv::core::CSignal signalUintLE3 = dispatch.AddPublisher("TestUintLE.SignalUintLE3");
    EXPECT_TRUE(signalUintLE3);
    sdv::core::CSignal signalUintLE4 = dispatch.AddPublisher("TestUintLE.SignalUintLE4");
    EXPECT_TRUE(signalUintLE4);
    sdv::core::CSignal signalUintLE5 = dispatch.AddPublisher("TestUintLE.SignalUintLE5");
    EXPECT_TRUE(signalUintLE5);
    sdv::core::CSignal signalUintLE6 = dispatch.AddPublisher("TestUintLE.SignalUintLE6");
    EXPECT_TRUE(signalUintLE6);

    // Subscribe to floating point based signals
    sdv::core::CSignal signalFloatLE1 = dispatch.AddPublisher("TestFloatLE.SignalFloatLE1");
    EXPECT_TRUE(signalFloatLE1);
    sdv::core::CSignal signalFloatLE2 = dispatch.AddPublisher("TestFloatLE.SignalFloatLE2");
    EXPECT_TRUE(signalFloatLE2);

    // Subscribe to double precision floating point based signals
    sdv::core::CSignal signalDoubleLE1 = dispatch.AddPublisher("TestDoubleLE.SignalDoubleLE1");
    EXPECT_TRUE(signalDoubleLE1);
    appcontrol.SetRunningMode();

    for (size_t n = 10; n < 20; n++)
    {
        sdv::core::CTransaction transaction = dispatch.CreateTransaction();
        signalIntLE1.Write((static_cast<int32_t>(n) & 0b1) * (n & 1 ? -1 : 1), transaction);     // 2 bits incl. sign
        signalIntLE2.Write((static_cast<int32_t>(n) & 0b1111) * (n & 1 ? -1 : 1), transaction);  // 5 bits incl. sign
        signalIntLE3.Write((static_cast<int32_t>(n) & 0b11111) * (n & 1 ? -1 : 1), transaction); // 6 bits incl. sign
        signalIntLE4.Write((static_cast<int32_t>(n) & 0b11) * (n & 1 ? -1 : 1), transaction);    // 3 bits incl. sign
        signalIntLE5.Write((static_cast<int32_t>(n)) * (n & 1 ? -1 : 1), transaction);           // 18 bits incl. sign
        signalIntLE6.Write((static_cast<int32_t>(n)) * (n & 1 ? -1 : 1), transaction);           // 30 bits incl. sign
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalUintLE1.Write(static_cast<uint32_t>(n) & 0b1, transaction);        // 1 bit
        signalUintLE2.Write(static_cast<uint32_t>(n) & 0b11, transaction);       // 2 bits
        signalUintLE3.Write(static_cast<uint32_t>(n) & 0b111111, transaction);   // 6 bits
        signalUintLE4.Write(static_cast<uint32_t>(n) & 0b1111111, transaction);  // 7 bits
        signalUintLE5.Write(static_cast<uint32_t>(n), transaction);              // 18 bits
        signalUintLE6.Write(static_cast<uint32_t>(n), transaction);              // 30 bits
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalFloatLE1.Write(static_cast<float>(n) * 1.111f, transaction);
        signalFloatLE2.Write(static_cast<float>(n) * 2.222f, transaction);
        transaction.Finish();

        transaction = dispatch.CreateTransaction();
        signalDoubleLE1.Write(static_cast<double>(n) * 3.333, transaction);
        transaction.Finish();
    }

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalIntLE1.Reset();
    signalIntLE2.Reset();
    signalIntLE3.Reset();
    signalIntLE4.Reset();
    signalIntLE5.Reset();
    signalIntLE6.Reset();

    signalUintLE1.Reset();
    signalUintLE2.Reset();
    signalUintLE3.Reset();
    signalUintLE4.Reset();
    signalUintLE5.Reset();
    signalUintLE6.Reset();

    signalFloatLE1.Reset();
    signalFloatLE2.Reset();

    signalDoubleLE1.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xC8 , {0x28, 0x45, 0x0A, 0x00, 0x28, 0x00, 0x00, 0x00}},
        { 0x64 , {0x54, 0x14, 0x0A, 0x00, 0x28, 0x00, 0x00, 0x00}},
        { 0x12C, {0x8F, 0xC2, 0x31, 0x41, 0x8F, 0xC2, 0xB1, 0x41}},
        { 0x190, {0x0A, 0xD7, 0xA3, 0x70, 0x3D, 0xAA, 0x40, 0x40}},
        { 0xC8 , {0xD7, 0xBA, 0xF5, 0xFF, 0xD7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x5F, 0x16, 0x0B, 0x00, 0x2C, 0x00, 0x00, 0x00}},
        { 0x12C, {0x37, 0x89, 0x43, 0x41, 0x37, 0x89, 0xC3, 0x41}},
        { 0x190, {0xBF, 0x9F, 0x1A, 0x2F, 0xDD, 0x54, 0x42, 0x40}},
        { 0xC8 , {0x30, 0x06, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x00}},
        { 0x64 , {0x60, 0x18, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x00}},
        { 0x12C, {0xDE, 0x4F, 0x55, 0x41, 0xDE, 0x4F, 0xD5, 0x41}},
        { 0x190, {0x73, 0x68, 0x91, 0xED, 0x7C, 0xFF, 0x43, 0x40}},
        { 0xC8 , {0xCF, 0xF9, 0xF3, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x6B, 0x1A, 0x0D, 0x00, 0x34, 0x00, 0x00, 0x00}},
        { 0x12C, {0x86, 0x16, 0x67, 0x41, 0x86, 0x16, 0xE7, 0x41}},
        { 0x190, {0x27, 0x31, 0x08, 0xAC, 0x1C, 0xAA, 0x45, 0x40}},
        { 0xC8 , {0x38, 0x47, 0x0E, 0x00, 0x38, 0x00, 0x00, 0x00}},
        { 0x64 , {0x74, 0x1C, 0x0E, 0x00, 0x38, 0x00, 0x00, 0x00}},
        { 0x12C, {0x2E, 0xDD, 0x78, 0x41, 0x2E, 0xDD, 0xF8, 0x41}},
        { 0x190, {0xDC, 0xF9, 0x7E, 0x6A, 0xBC, 0x54, 0x47, 0x40}},
        { 0xC8 , {0xC7, 0xB8, 0xF1, 0xFF, 0xC7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x7F, 0x1E, 0x0F, 0x00, 0x3C, 0x00, 0x00, 0x00}},
        { 0x12C, {0xEB, 0x51, 0x85, 0x41, 0xEB, 0x51, 0x05, 0x42}},
        { 0x190, {0x90, 0xC2, 0xF5, 0x28, 0x5C, 0xFF, 0x48, 0x40}},
        { 0xC8 , {0x00, 0x08, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00}},
        { 0x64 , {0x80, 0x20, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00}},
        { 0x12C, {0x3F, 0x35, 0x8E, 0x41, 0x3F, 0x35, 0x0E, 0x42}},
        { 0x190, {0x44, 0x8B, 0x6C, 0xE7, 0xFB, 0xA9, 0x4A, 0x40}},
        { 0xC8 , {0xFF, 0xF7, 0xEF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x8B, 0x22, 0x11, 0x00, 0x44, 0x00, 0x00, 0x00}},
        { 0x12C, {0x93, 0x18, 0x97, 0x41, 0x93, 0x18, 0x17, 0x42}},
        { 0x190, {0xF8, 0x53, 0xE3, 0xA5, 0x9B, 0x54, 0x4C, 0x40}},
        { 0xC8 , {0x08, 0x49, 0x12, 0x00, 0x48, 0x00, 0x00, 0x00}},
        { 0x64 , {0x94, 0x24, 0x12, 0x00, 0x48, 0x00, 0x00, 0x00}},
        { 0x12C, {0xE7, 0xFB, 0x9F, 0x41, 0xE7, 0xFB, 0x1F, 0x42}},
        { 0x190, {0xAC, 0x1C, 0x5A, 0x64, 0x3B, 0xFF, 0x4D, 0x40}},
        { 0xC8 , {0xF7, 0xB6, 0xED, 0xFF, 0xB7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x9F, 0x26, 0x13, 0x00, 0x4C, 0x00, 0x00, 0x00}},
        { 0x12C, {0x3B, 0xDF, 0xA8, 0x41, 0x3B, 0xDF, 0x28, 0x42}},
        { 0x190, {0x61, 0xE5, 0xD0, 0x22, 0xDB, 0xA9, 0x4F, 0x40}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 8ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
        EXPECT_EQ(prMsg.first.rguiData[7], sData.rguiData[7]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveLittleEndianAllDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[8];
    } rgsData[] = {
        { 0xC8 , {0x28, 0x45, 0x0A, 0x00, 0x28, 0x00, 0x00, 0x00}},
        { 0x64 , {0x54, 0x14, 0x0A, 0x00, 0x28, 0x00, 0x00, 0x00}},
        { 0x12C, {0x8F, 0xC2, 0x31, 0x41, 0x8F, 0xC2, 0xB1, 0x41}},
        { 0x190, {0x0A, 0xD7, 0xA3, 0x70, 0x3D, 0xAA, 0x40, 0x40}},
        { 0xC8 , {0xD7, 0xBA, 0xF5, 0xFF, 0xD7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x5F, 0x16, 0x0B, 0x00, 0x2C, 0x00, 0x00, 0x00}},
        { 0x12C, {0x37, 0x89, 0x43, 0x41, 0x37, 0x89, 0xC3, 0x41}},
        { 0x190, {0xBF, 0x9F, 0x1A, 0x2F, 0xDD, 0x54, 0x42, 0x40}},
        { 0xC8 , {0x30, 0x06, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x00}},
        { 0x64 , {0x60, 0x18, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x00}},
        { 0x12C, {0xDE, 0x4F, 0x55, 0x41, 0xDE, 0x4F, 0xD5, 0x41}},
        { 0x190, {0x73, 0x68, 0x91, 0xED, 0x7C, 0xFF, 0x43, 0x40}},
        { 0xC8 , {0xCF, 0xF9, 0xF3, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x6B, 0x1A, 0x0D, 0x00, 0x34, 0x00, 0x00, 0x00}},
        { 0x12C, {0x86, 0x16, 0x67, 0x41, 0x86, 0x16, 0xE7, 0x41}},
        { 0x190, {0x27, 0x31, 0x08, 0xAC, 0x1C, 0xAA, 0x45, 0x40}},
        { 0xC8 , {0x38, 0x47, 0x0E, 0x00, 0x38, 0x00, 0x00, 0x00}},
        { 0x64 , {0x74, 0x1C, 0x0E, 0x00, 0x38, 0x00, 0x00, 0x00}},
        { 0x12C, {0x2E, 0xDD, 0x78, 0x41, 0x2E, 0xDD, 0xF8, 0x41}},
        { 0x190, {0xDC, 0xF9, 0x7E, 0x6A, 0xBC, 0x54, 0x47, 0x40}},
        { 0xC8 , {0xC7, 0xB8, 0xF1, 0xFF, 0xC7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x7F, 0x1E, 0x0F, 0x00, 0x3C, 0x00, 0x00, 0x00}},
        { 0x12C, {0xEB, 0x51, 0x85, 0x41, 0xEB, 0x51, 0x05, 0x42}},
        { 0x190, {0x90, 0xC2, 0xF5, 0x28, 0x5C, 0xFF, 0x48, 0x40}},
        { 0xC8 , {0x00, 0x08, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00}},
        { 0x64 , {0x80, 0x20, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00}},
        { 0x12C, {0x3F, 0x35, 0x8E, 0x41, 0x3F, 0x35, 0x0E, 0x42}},
        { 0x190, {0x44, 0x8B, 0x6C, 0xE7, 0xFB, 0xA9, 0x4A, 0x40}},
        { 0xC8 , {0xFF, 0xF7, 0xEF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x8B, 0x22, 0x11, 0x00, 0x44, 0x00, 0x00, 0x00}},
        { 0x12C, {0x93, 0x18, 0x97, 0x41, 0x93, 0x18, 0x17, 0x42}},
        { 0x190, {0xF8, 0x53, 0xE3, 0xA5, 0x9B, 0x54, 0x4C, 0x40}},
        { 0xC8 , {0x08, 0x49, 0x12, 0x00, 0x48, 0x00, 0x00, 0x00}},
        { 0x64 , {0x94, 0x24, 0x12, 0x00, 0x48, 0x00, 0x00, 0x00}},
        { 0x12C, {0xE7, 0xFB, 0x9F, 0x41, 0xE7, 0xFB, 0x1F, 0x42}},
        { 0x190, {0xAC, 0x1C, 0x5A, 0x64, 0x3B, 0xFF, 0x4D, 0x40}},
        { 0xC8 , {0xF7, 0xB6, 0xED, 0xFF, 0xB7, 0xFF, 0xFF, 0xFF}},
        { 0x64 , {0x9F, 0x26, 0x13, 0x00, 0x4C, 0x00, 0x00, 0x00}},
        { 0x12C, {0x3B, 0xDF, 0xA8, 0x41, 0x3B, 0xDF, 0x28, 0x42}},
        { 0x190, {0x61, 0xE5, 0xD0, 0x22, 0xDB, 0xA9, 0x4F, 0x40}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 8ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        sMsg.rguiData[7] = sData.rguiData[7];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    int32_t rgiIntCnt[6] = {10, 10, 10, 10, 10, 10};
    sdv::core::CSignal signalIntLE1 = dispatch.Subscribe("TestIntLE.SignalIntLE1", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[0]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b1) * (i & 1 ? -1 : 1);   // 2 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE1);
    sdv::core::CSignal signalIntLE2 = dispatch.Subscribe("TestIntLE.SignalIntLE2", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[1]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b1111) * (i & 1 ? -1 : 1);    // 5 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE2);
    sdv::core::CSignal signalIntLE3 = dispatch.Subscribe("TestIntLE.SignalIntLE3", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[2]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b11111) * (i & 1 ? -1 : 1);   // 6 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE3);
    sdv::core::CSignal signalIntLE4 = dispatch.Subscribe("TestIntLE.SignalIntLE4", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[3]++;
            int32_t iCalculated = (static_cast<int32_t>(i) & 0b11) * (i & 1 ? -1 : 1);  // 3 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE4);
    sdv::core::CSignal signalIntLE5 = dispatch.Subscribe("TestIntLE.SignalIntLE5", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[4]++;
            int32_t iCalculated = (static_cast<int32_t>(i)) * (i & 1 ? -1 : 1); // 18 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE5);
    sdv::core::CSignal signalIntLE6 = dispatch.Subscribe("TestIntLE.SignalIntLE6", [&](sdv::any_t tVal)
        {
            int32_t i = rgiIntCnt[5]++;
            int32_t iCalculated = (static_cast<int32_t>(i)) * (i & 1 ? -1 : 1); // 30 bits incl. sign
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalIntLE6);

    // Subscribe to unsigned integer based signals
    uint32_t rguiUintCnt[6] = {10u, 10u, 10u, 10u, 10u, 10u};
    sdv::core::CSignal signalUintLE1 = dispatch.Subscribe("TestUintLE.SignalUintLE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[0]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b1;   // 1 bit
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE1);
    sdv::core::CSignal signalUintLE2 = dispatch.Subscribe("TestUintLE.SignalUintLE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[1]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b11;   // 2 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE2);
    sdv::core::CSignal signalUintLE3 = dispatch.Subscribe("TestUintLE.SignalUintLE3", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[2]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b111111;   // 6 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE3);
    sdv::core::CSignal signalUintLE4 = dispatch.Subscribe("TestUintLE.SignalUintLE4", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[3]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui) & 0b1111111;   // 7 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE4);
    sdv::core::CSignal signalUintLE5 = dispatch.Subscribe("TestUintLE.SignalUintLE5", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[4]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui);   // 18 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE5);
    sdv::core::CSignal signalUintLE6 = dispatch.Subscribe("TestUintLE.SignalUintLE6", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiUintCnt[5]++;
            uint32_t uiCalculated = static_cast<uint32_t>(ui);   // 30 bits
            EXPECT_EQ(uiCalculated, static_cast<uint32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE6);

    // Subscribe to floating point based signals
    uint32_t rguiFloatCnt[2] = {10u, 10u};
    sdv::core::CSignal signalFloatLE1 = dispatch.Subscribe("TestFloatLE.SignalFloatLE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiFloatCnt[0]++;
            float fCalculated = static_cast<float>(ui) * 1.111f;
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalFloatLE1);
    sdv::core::CSignal signalFloatLE2 = dispatch.Subscribe("TestFloatLE.SignalFloatLE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiFloatCnt[1]++;
            float fCalculated = static_cast<float>(ui) * 2.222f;
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalFloatLE2);

    // Subscribe to double precision floating point based signals
    uint32_t uiDoubleCnt = 10u;
    sdv::core::CSignal signalDoubleLE1 = dispatch.Subscribe("TestDoubleLE.SignalDoubleLE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = uiDoubleCnt++;
            double dCalculated = static_cast<double>(ui) * 3.333;
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalDoubleLE1);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Check for correct counters
    for (int32_t i : rgiIntCnt)
        EXPECT_EQ(i, 20);
    for (uint32_t ui : rguiUintCnt)
        EXPECT_EQ(ui, 20u);
    for (uint32_t ui : rguiFloatCnt)
        EXPECT_EQ(ui, 20u);
    EXPECT_EQ(uiDoubleCnt, 20u);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalIntLE1.Reset();
    signalIntLE2.Reset();
    signalIntLE3.Reset();
    signalIntLE4.Reset();
    signalIntLE5.Reset();
    signalIntLE6.Reset();

    signalUintLE1.Reset();
    signalUintLE2.Reset();
    signalUintLE3.Reset();
    signalUintLE4.Reset();
    signalUintLE5.Reset();
    signalUintLE6.Reset();

    signalFloatLE1.Reset();
    signalFloatLE2.Reset();

    signalDoubleLE1.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitBigEndianScaledDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to integer based signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalUintBE1 = dispatch.AddPublisher("TestScaleBE.SignalScaleUintBE1");
    EXPECT_TRUE(signalUintBE1);
    sdv::core::CSignal signalUintBE2 = dispatch.AddPublisher("TestScaleBE.SignalScaleUintBE2");
    EXPECT_TRUE(signalUintBE2);
    sdv::core::CSignal signalUintBE3 = dispatch.AddPublisher("TestScaleBE.SignalScaleUintBE3");
    EXPECT_TRUE(signalUintBE3);
    sdv::core::CSignal signalUintBE4 = dispatch.AddPublisher("TestScaleBE.SignalScaleUintBE4");
    EXPECT_TRUE(signalUintBE4);
    sdv::core::CSignal signalIntBE5 = dispatch.AddPublisher("TestScaleBE.SignalScaleIntBE5");
    EXPECT_TRUE(signalIntBE5);
    sdv::core::CSignal signalIntBE6 = dispatch.AddPublisher("TestScaleBE.SignalScaleIntBE6");
    EXPECT_TRUE(signalIntBE6);
    sdv::core::CSignal signalFloatBE7 = dispatch.AddPublisher("TestScaleBE.SignalScaleFloatBE7");
    EXPECT_TRUE(signalFloatBE7);
    appcontrol.SetRunningMode();

    for (size_t n = 4; n < 12; n++)
    {
        sdv::core::CTransaction transaction = dispatch.CreateTransaction();
        signalUintBE1.Write(static_cast<double>(n) / 1000.0, transaction);      // 4 bits - factor 0.001
        signalUintBE2.Write(static_cast<int32_t>(n) - 5, transaction);          // 4 bits - offset -3.5
        signalUintBE3.Write(static_cast<int32_t>(n) * -1, transaction);         // 4 bits - factor -1
        signalUintBE4.Write(static_cast<int32_t>(n) - 1, transaction);          // 4 bits - offset -2
        signalIntBE5.Write(static_cast<float>(n) / 1000.0f, transaction);       // 4 bits incl. sign - factor 0.001, offset 0.008
        signalIntBE6.Write(static_cast<int32_t>(n) - 10, transaction);          // 4 bits incl. sign - offset -3.5
        signalFloatBE7.Write(static_cast<float>(n) / 10000.0f, transaction);    // 32 bits - factor 0.001, offset 100
        transaction.Finish();
    }

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalUintBE1.Reset();
    signalUintBE2.Reset();
    signalUintBE3.Reset();
    signalUintBE4.Reset();
    signalIntBE5.Reset();
    signalIntBE6.Reset();
    signalFloatBE7.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[7];
    } rgsData[] = {
        { 0x1F5, {0x43, 0x45, 0xCD, 0xC7, 0xC3, 0x4F, 0xCD}},
        { 0x1F5, {0x54, 0x56, 0xDE, 0xC7, 0xC3, 0x4F, 0xBF}},
        { 0x1F5, {0x65, 0x67, 0xEF, 0xC7, 0xC3, 0x4F, 0xB2}},
        { 0x1F5, {0x76, 0x78, 0xF1, 0xC7, 0xC3, 0x4F, 0xA6}},
        { 0x1F5, {0x87, 0x89, 0x02, 0xC7, 0xC3, 0x4F, 0x99}},
        { 0x1F5, {0x98, 0x9A, 0x13, 0xC7, 0xC3, 0x4F, 0x8C}},
        { 0x1F5, {0xA9, 0xAB, 0x24, 0xC7, 0xC3, 0x4F, 0x7F}},
        { 0x1F5, {0xBA, 0xBC, 0x35, 0xC7, 0xC3, 0x4F, 0x73}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 7ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveBigEndianScaledDataType)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[7];
    } rgsData[] = {
        { 0x1F5, {0x43, 0x45, 0xCD, 0xC7, 0xC3, 0x4F, 0xCD}},
        { 0x1F5, {0x54, 0x56, 0xDE, 0xC7, 0xC3, 0x4F, 0xBF}},
        { 0x1F5, {0x65, 0x67, 0xEF, 0xC7, 0xC3, 0x4F, 0xB2}},
        { 0x1F5, {0x76, 0x78, 0xF1, 0xC7, 0xC3, 0x4F, 0xA6}},
        { 0x1F5, {0x87, 0x89, 0x02, 0xC7, 0xC3, 0x4F, 0x99}},
        { 0x1F5, {0x98, 0x9A, 0x13, 0xC7, 0xC3, 0x4F, 0x8C}},
        { 0x1F5, {0xA9, 0xAB, 0x24, 0xC7, 0xC3, 0x4F, 0x7F}},
        { 0x1F5, {0xBA, 0xBC, 0x35, 0xC7, 0xC3, 0x4F, 0x73}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 7ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    uint32_t rguiCnt[7] = {4u, 4u, 4u, 4u, 4u, 4u, 4u};
    sdv::core::CSignal signalUintBE1 = dispatch.Subscribe("TestScaleBE.SignalScaleUintBE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[0]++;
            double dCalculated = static_cast<double>(ui) / 1000.0;   // 4 bits - factor 0.001
            EXPECT_EQ(std::round(dCalculated * 1000.0), std::round(static_cast<double>(tVal) * 1000.0));
        });
    EXPECT_TRUE(signalUintBE1);
    sdv::core::CSignal signalUintBE2 = dispatch.Subscribe("TestScaleBE.SignalScaleUintBE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[1]++;
            // Since the offset is a floating point value, but the data type is a unsigned integer, the value is being rounded.
            double dCalculated = static_cast<int32_t>(std::round(static_cast<int32_t>(ui) - 5.0 + 3.5)) - 3.5;   // 4 bits - offset -3.5
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalUintBE2);
    sdv::core::CSignal signalUintBE3 = dispatch.Subscribe("TestScaleBE.SignalScaleUintBE3", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[2]++;
            int32_t iCalculated = static_cast<int32_t>(ui) * -1;   // 4 bits - factor -1
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE3);
    sdv::core::CSignal signalUintBE4 = dispatch.Subscribe("TestScaleBE.SignalScaleUintBE4", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[3]++;
            int32_t iCalculated = static_cast<int32_t>(ui) - 1;   // 4 bits - offset -2
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalUintBE4);
    sdv::core::CSignal signalIntBE5 = dispatch.Subscribe("TestScaleBE.SignalScaleIntBE5", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[4]++;
            float fCalculated = static_cast<float>(ui) / 1000.0f;   // 4 bits incl. sign - factor 0.001, offset 0.008
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalIntBE5);
    sdv::core::CSignal signalIntBE6 = dispatch.Subscribe("TestScaleBE.SignalScaleIntBE6", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[5]++;
            // Since the offset is a floating point value, but the data type is a integer, the value is being rounded.
            double dCalculated = static_cast<int32_t>(std::round(static_cast<int32_t>(ui) - 10.0 + 3.5)) - 3.5;   // 4 bits incl. sign - offset -3.5
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalIntBE6);
    sdv::core::CSignal signalFloatBE7 = dispatch.Subscribe("TestScaleBE.SignalScaleFloatBE7", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[6]++;
            float fCalculated = static_cast<float>(ui) / 10000.0f;   // 32 bits - factor 0.001, offset 100
            EXPECT_EQ(std::round(fCalculated * 1000.f), std::round(static_cast<float>(tVal) * 1000.f));
        });
    EXPECT_TRUE(signalFloatBE7);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Check for correct counters
    for (uint32_t ui : rguiCnt)
        EXPECT_EQ(ui, 12u);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalUintBE1.Reset();
    signalUintBE2.Reset();
    signalUintBE3.Reset();
    signalUintBE4.Reset();
    signalIntBE5.Reset();
    signalIntBE6.Reset();
    signalFloatBE7.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, SpontaneousTransmitLittleEndianScaledDataTypes)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to integer based signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalUintLE1 = dispatch.AddPublisher("TestScaleLE.SignalScaleUintLE1");
    EXPECT_TRUE(signalUintLE1);
    sdv::core::CSignal signalUintLE2 = dispatch.AddPublisher("TestScaleLE.SignalScaleUintLE2");
    EXPECT_TRUE(signalUintLE2);
    sdv::core::CSignal signalUintLE3 = dispatch.AddPublisher("TestScaleLE.SignalScaleUintLE3");
    EXPECT_TRUE(signalUintLE3);
    sdv::core::CSignal signalUintLE4 = dispatch.AddPublisher("TestScaleLE.SignalScaleUintLE4");
    EXPECT_TRUE(signalUintLE4);
    sdv::core::CSignal signalIntLE5 = dispatch.AddPublisher("TestScaleLE.SignalScaleIntLE5");
    EXPECT_TRUE(signalIntLE5);
    sdv::core::CSignal signalIntLE6 = dispatch.AddPublisher("TestScaleLE.SignalScaleIntLE6");
    EXPECT_TRUE(signalIntLE6);
    sdv::core::CSignal signalFloatLE7 = dispatch.AddPublisher("TestScaleLE.SignalScaleFloatLE7");
    EXPECT_TRUE(signalFloatLE7);
    appcontrol.SetRunningMode();

    for (size_t n = 4; n < 12; n++)
    {
        sdv::core::CTransaction transaction = dispatch.CreateTransaction();
        signalUintLE1.Write(static_cast<double>(n) / 1000.0, transaction);      // 4 bits - factor 0.001
        signalUintLE2.Write(static_cast<int32_t>(n) - 5, transaction);          // 4 bits - offset -3.5
        signalUintLE3.Write(static_cast<int32_t>(n) * -1, transaction);         // 4 bits - factor -1
        signalUintLE4.Write(static_cast<int32_t>(n) - 1, transaction);          // 4 bits - offset -2
        signalIntLE5.Write(static_cast<float>(n) / 1000.0f, transaction);       // 4 bits incl. sign - factor 0.001, offset 0.008
        signalIntLE6.Write(static_cast<int32_t>(n) - 10, transaction);          // 4 bits incl. sign - offset -3.5
        signalFloatLE7.Write(static_cast<float>(n) / 10000.0f, transaction);    // 32 bits - factor 0.001, offset 100
        transaction.Finish();
    }

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalUintLE1.Reset();
    signalUintLE2.Reset();
    signalUintLE3.Reset();
    signalUintLE4.Reset();
    signalIntLE5.Reset();
    signalIntLE6.Reset();
    signalFloatLE7.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[7];
    } rgsData[] = {
        { 0x1F4, {0x34, 0x54, 0xDC, 0xCD, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x45, 0x65, 0xED, 0xBF, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x56, 0x76, 0xFE, 0xB2, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x67, 0x87, 0x1F, 0xA6, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x78, 0x98, 0x20, 0x99, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x89, 0xA9, 0x31, 0x8C, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x9A, 0xBA, 0x42, 0x7F, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0xAB, 0xCB, 0x53, 0x73, 0x4F, 0xC3, 0xC7}}
    };

    // Compare with test data.
    for (const STestData& sData : rgsData)
    {
        EXPECT_FALSE(reader.IsEOF());
        std::pair<asc::SCanMessage, bool> prMsg;
        do {
            prMsg = reader.Get();
            EXPECT_TRUE(prMsg.second);
            ++reader;
        } while (!reader.IsEOF() && prMsg.first.uiId != sData.uiID);   // Skip other messages
        EXPECT_EQ(prMsg.first.uiChannel, 1ul);
        EXPECT_EQ(prMsg.first.uiId, sData.uiID);
        EXPECT_FALSE(prMsg.first.bExtended);
        EXPECT_FALSE(prMsg.first.bCanFd);
        EXPECT_EQ(prMsg.first.uiLength, 7ul);
        EXPECT_EQ(prMsg.first.eDirection, asc::SCanMessage::EDirection::tx);
        EXPECT_EQ(prMsg.first.rguiData[0], sData.rguiData[0]);
        EXPECT_EQ(prMsg.first.rguiData[1], sData.rguiData[1]);
        EXPECT_EQ(prMsg.first.rguiData[2], sData.rguiData[2]);
        EXPECT_EQ(prMsg.first.rguiData[3], sData.rguiData[3]);
        EXPECT_EQ(prMsg.first.rguiData[4], sData.rguiData[4]);
        EXPECT_EQ(prMsg.first.rguiData[5], sData.rguiData[5]);
        EXPECT_EQ(prMsg.first.rguiData[6], sData.rguiData[6]);
    }
}

TEST(DbcUtilCanDLTest, ReceiveLittleEndianScaledDataType)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    struct STestData
    {
        uint32_t    uiID;
        uint8_t     rguiData[7];
    } rgsData[] = {
        { 0x1F4, {0x34, 0x54, 0xDC, 0xCD, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x45, 0x65, 0xED, 0xBF, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x56, 0x76, 0xFE, 0xB2, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x67, 0x87, 0x1F, 0xA6, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x78, 0x98, 0x20, 0x99, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x89, 0xA9, 0x31, 0x8C, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0x9A, 0xBA, 0x42, 0x7F, 0x4F, 0xC3, 0xC7}},
        { 0x1F4, {0xAB, 0xCB, 0x53, 0x73, 0x4F, 0xC3, 0xC7}}
    };

    // Create ASC file with a delay of 100 (to allow the system to initialize)
    asc::CAscWriter writer;
    double dTimestamp = 0.100;
    for (const STestData& sData : rgsData)
    {
        asc::SCanMessage sMsg{};
        sMsg.dTimestamp = dTimestamp;
        dTimestamp += 0.001;
        sMsg.uiChannel = 1ul;
        sMsg.uiId = sData.uiID;
        sMsg.bExtended = false;
        sMsg.bCanFd = false;
        sMsg.uiLength = 7ul;
        sMsg.eDirection = asc::SCanMessage::EDirection::rx;
        sMsg.rguiData[0] = sData.rguiData[0];
        sMsg.rguiData[1] = sData.rguiData[1];
        sMsg.rguiData[2] = sData.rguiData[2];
        sMsg.rguiData[3] = sData.rguiData[3];
        sMsg.rguiData[4] = sData.rguiData[4];
        sMsg.rguiData[5] = sData.rguiData[5];
        sMsg.rguiData[6] = sData.rguiData[6];
        writer.AddSample(sMsg);
    }

    // Add a dummy message with a big timegap to prevent the repetition function to jump in.
    asc::SCanMessage sMsgDummy{};
    sMsgDummy.dTimestamp = 100.0;
    sMsgDummy.uiChannel = 1ul;
    sMsgDummy.uiId = 999;
    writer.AddSample(sMsgDummy);

    // Write the ASC file
    EXPECT_TRUE(writer.Write(GetExecDirectory() / "receiver_test.asc"));
    writer.Clear();

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_rx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    uint32_t rguiCnt[7] = {4u, 4u, 4u, 4u, 4u, 4u, 4u};
    sdv::core::CSignal signalUintLE1 = dispatch.Subscribe("TestScaleLE.SignalScaleUintLE1", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[0]++;
            double dCalculated = static_cast<double>(ui) / 1000.0;   // 4 bits - factor 0.001
            EXPECT_EQ(std::round(dCalculated * 1000.0), std::round(static_cast<double>(tVal) * 1000.0));
        });
    EXPECT_TRUE(signalUintLE1);
    sdv::core::CSignal signalUintLE2 = dispatch.Subscribe("TestScaleLE.SignalScaleUintLE2", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[1]++;
            // Since the offset is a floating point value, but the data type is a unsigned integer, the value is being rounded.
            double dCalculated = static_cast<int32_t>(std::round(static_cast<int32_t>(ui) - 5.0 + 3.5)) - 3.5;   // 4 bits - offset -3.5
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalUintLE2);
    sdv::core::CSignal signalUintLE3 = dispatch.Subscribe("TestScaleLE.SignalScaleUintLE3", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[2]++;
            int32_t iCalculated = static_cast<int32_t>(ui) * -1;   // 4 bits - factor -1
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE3);
    sdv::core::CSignal signalUintLE4 = dispatch.Subscribe("TestScaleLE.SignalScaleUintLE4", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[3]++;
            int32_t iCalculated = static_cast<int32_t>(ui) - 1;   // 4 bits - offset -2
            EXPECT_EQ(iCalculated, static_cast<int32_t>(tVal));
        });
    EXPECT_TRUE(signalUintLE4);
    sdv::core::CSignal signalIntLE5 = dispatch.Subscribe("TestScaleLE.SignalScaleIntLE5", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[4]++;
            float fCalculated = static_cast<float>(ui) / 1000.0f;   // 4 bits incl. sign - factor 0.001, offset 0.008
            EXPECT_EQ(fCalculated, static_cast<float>(tVal));
        });
    EXPECT_TRUE(signalIntLE5);
    sdv::core::CSignal signalIntLE6 = dispatch.Subscribe("TestScaleLE.SignalScaleIntLE6", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[5]++;
            // Since the offset is a floating point value, but the data type is a integer, the value is being rounded.
            double dCalculated = static_cast<int32_t>(std::round(static_cast<int32_t>(ui) - 10.0 + 3.5)) - 3.5;   // 4 bits incl. sign - offset -3.5
            EXPECT_EQ(dCalculated, static_cast<double>(tVal));
        });
    EXPECT_TRUE(signalIntLE6);
    sdv::core::CSignal signalFloatLE7 = dispatch.Subscribe("TestScaleLE.SignalScaleFloatLE7", [&](sdv::any_t tVal)
        {
            uint32_t ui = rguiCnt[6]++;
            float fCalculated = static_cast<float>(ui) / 10000.0f;   // 32 bits - factor 0.001, offset 100
            EXPECT_EQ(std::round(fCalculated * 1000.f), std::round(static_cast<float>(tVal) * 1000.f));
        });
    EXPECT_TRUE(signalFloatLE7);
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Check for correct counters
    for (uint32_t ui : rguiCnt)
        EXPECT_EQ(ui, 12u);

    // Shutdown
    appcontrol.SetConfigMode();
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalUintLE1.Reset();
    signalUintLE2.Reset();
    signalUintLE3.Reset();
    signalUintLE4.Reset();
    signalIntLE5.Reset();
    signalIntLE6.Reset();
    signalFloatLE7.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();
}

TEST(DbcUtilCanDLTest, CyclicTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestCyclic.Counter");
    EXPECT_TRUE(signalCounter);

    // The cycle time is 10 ms. Write the counter 5 times with 50ms difference
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    sdv::core::CTaskTimer timer(50, [&]()
        {
            if (n == 5)
                cv.notify_all();
            else
                signalCounter.Write(n++);
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    timer.Reset();
    lock.unlock();
    appcontrol.SetConfigMode();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    double dLastTime = 0;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;
        dLastTime = prMsg.first.dTimestamp;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0x8) continue;
        nSignalCnt++;

        int32_t iCntTemp = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_TRUE(iCntTemp == iCnt || iCntTemp == iCnt + 1);
        iCnt = iCntTemp;
        EXPECT_LT(iCnt, 5);
        if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (iCnt && !vecTime[iCnt - 1]) vecTime[iCnt - 1] = prMsg.first.dTimestamp;
        }
    }
    if (!vecTime[4]) vecTime[4] = dLastTime;
    if (iCnt < 4)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
    }

    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (vecStat[n] > 6)       // In the unlucky case, 6 triggers might have occurred (during startup, there might be many more...).
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(vecStat[n], 6, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(vecStat[n], 6, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            double dPeriod = std::round((vecTime[n] - vecTime[n - 1]) * 1000.0) / 1000.0;
            if (dPeriod > 0.051)   // Max 51ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (n == 4)
            {
                if (dPeriod < 0.019)   // Min 19ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (dPeriod < 0.039)   // Min 39ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
        }
        if (vecStat[n] < 4u)  // At least 4 triggers.
        {
            if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                SDV_TIMING_EXPECT_GE(vecStat[n], 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
            else
                SDV_TIMING_EXPECT_GE(vecStat[n], 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
        }
    }
}

TEST(DbcUtilCanDLTest, CyclicIfActiveTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestCyclicIfActive.Counter");
    EXPECT_TRUE(signalCounter);

    // The counter value 2 is the default value and therefore determines the active state.
    // The cycle time is 10 ms. Write the counter 5 times with 50ms difference
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    sdv::core::CTaskTimer timer(50, [&]()
        {
            if (n == 5)
                cv.notify_all();
            else
                signalCounter.Write(n++);
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    appcontrol.SetConfigMode();
    timer.Reset();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    double dLastTime = 0;
    bool bInit = false;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;
        dLastTime = prMsg.first.dTimestamp;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0xa) continue;
        nSignalCnt++;

        iCnt = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_LT(iCnt, 5);
        if (!bInit)
        {
            // Expected iCnt to be the default value...
            if (iCnt != 0 && iCnt != 2)
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_EQ(iCnt, 0, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_EQ(iCnt, 0, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }

            bInit = true;
        }
        else if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (iCnt && !vecTime[iCnt - 1]) vecTime[iCnt - 1] = prMsg.first.dTimestamp;
        }
    }
    if (!vecTime[4]) vecTime[4] = dLastTime;
    if (iCnt < 4)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
    }

    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (n == 2)
            {
                if (vecStat[n] != 1)       // One trigger should have occurred due to default value
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 1, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 1, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (vecStat[n] > 6)       // In the unlucky case, 6 triggers might have occurred (during startup, there might be many more...).
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_LE(vecStat[n], 6, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_LE(vecStat[n], 6, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            double dPeriod = std::round((vecTime[n] - vecTime[n - 1]) * 1000.0) / 1000.0;
            if (dPeriod > 0.051)   // Max 51ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (n == 4)
            {
                if (dPeriod < 0.019)   // Min 19ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (dPeriod < 0.039)   // Min 39ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
        }
    }
}

TEST(DbcUtilCanDLTest, CyclicAndSpontaneousTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestCyclicAndSpontaneous.Counter");
    EXPECT_TRUE(signalCounter);

    // The cycle time is 50 ms. Write a value every 50ms. This should result in a spontaneous and a cyclic trigger.
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    sdv::core::CTaskTimer timer(50, [&]()
        {
            if (n == 5)
                cv.notify_all();
            else
                signalCounter.Write(n++);
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    appcontrol.SetConfigMode();
    timer.Reset();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    double dLastTime = 0;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;
        dLastTime = prMsg.first.dTimestamp;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0x9) continue;
        nSignalCnt++;

        int32_t iCntTemp = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_TRUE(iCntTemp == iCnt || iCntTemp == iCnt + 1);
        iCnt = iCntTemp;
        EXPECT_LT(iCnt, 5);
        if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (iCnt && !vecTime[iCnt - 1]) vecTime[iCnt - 1] = prMsg.first.dTimestamp;
        }
    }
    if (!vecTime[4]) vecTime[4] = dLastTime;
    if (iCnt < 4)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
    }

    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (vecStat[n] != 2)       // One trigger and one cycle.
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            double dPeriod = std::round((vecTime[n] - vecTime[n - 1]) * 1000.0) / 1000.0;
            if (dPeriod > 0.051)   // Max 51ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (n == 4)
            {
                if (dPeriod < 0.019)   // Min 19ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (dPeriod < 0.039)   // Min 39ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
        }
    }
}

TEST(DbcUtilCanDLTest, SpontaneousDelayTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestSpontaneousDelay.Counter");
    EXPECT_TRUE(signalCounter);

    // The delay is 20ms. Write the counter 5 times every 10ms.
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    size_t nCycle = 0;
    sdv::core::CTaskTimer timer(10, [&]()
        {
            if (n < 5) signalCounter.Write(n);
            if (nCycle++ == 5)
            {
                nCycle = 0;
                if (n == 5) cv.notify_all();
                n++;
            }
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    appcontrol.SetConfigMode();
    timer.Reset();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecFirstTime(5);
    std::vector<double> vecLastTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0xc) continue;
        nSignalCnt++;

        int32_t iCntTemp = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_TRUE(iCntTemp == iCnt || iCntTemp == iCnt + 1);
        iCnt = iCntTemp;
        EXPECT_LT(iCnt, 5);
        if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (!vecFirstTime[iCnt]) vecFirstTime[iCnt] = prMsg.first.dTimestamp;
            vecLastTime[iCnt] = prMsg.first.dTimestamp;
        }
    }
    if (iCnt < 4)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
    }

    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (vecStat[n] > 4)       // Max 4 times
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(vecStat[n], 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(vecStat[n], 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (vecStat[n] < 3)       // Min 3 times
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_GE(vecStat[n], 3, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_GE(vecStat[n], 3, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            double dPeriod = std::round((vecLastTime[n] - vecFirstTime[n]) * 1000.0) / 1000.0;
            if (dPeriod > 0.061)   // Max 61ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.061, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.061, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (dPeriod < 0.039)   // Min 39ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
        }
    }
}

TEST(DbcUtilCanDLTest, CyclicAndSpontaneousDelayTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestCyclicAndSpontaneousDelay.Counter");
    EXPECT_TRUE(signalCounter);

    // The cycle time is 50ms. The delay is 25ms. Write the counter 2 times every 25ms.
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    sdv::core::CTaskTimer timer(25, [&]()
        {
            if (n == 10)
                cv.notify_all();
            else
            {
                signalCounter.Write(n / 2);
                n++;
            }
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    appcontrol.SetConfigMode();
    timer.Reset();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    double dLastTime = 0;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;
        dLastTime = prMsg.first.dTimestamp;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0xd) continue;
        nSignalCnt++;

        int32_t iCntTemp = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_TRUE(iCntTemp == iCnt || iCntTemp == iCnt + 1);
        iCnt = iCntTemp;
        EXPECT_LT(iCnt, 5);
        if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (iCnt && !vecTime[iCnt - 1]) vecTime[iCnt - 1] = prMsg.first.dTimestamp;
        }
    }
    if (!vecTime[4]) vecTime[4] = dLastTime;
    if (iCnt < 4)
        {
            if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
            else
                SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
        }

    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (vecStat[n] != 2)       // Two trigger and/or cycle.
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            double dPeriod = std::round((vecTime[n] - vecTime[n - 1]) * 1000.0) / 1000.0;
            if (dPeriod > 0.051)   // Max 51ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                     SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                     SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (n == 4)
            {
                if (dPeriod < 0.019)   // Min 19ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (dPeriod < 0.049)   // Min 49ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.049, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.049, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
        }
    }
}

TEST(DbcUtilCanDLTest, CyclicIfActiveAndSpontaneousTransmit)
{
    try
    {
        std::filesystem::remove(GetExecDirectory() / "receiver_test.asc");
        std::filesystem::remove(GetExecDirectory() / "transmitter_test.asc");
    } catch (const std::filesystem::filesystem_error&)
    {}
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "receiver_test.asc"));
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "transmitter_test.asc"));

    std::filesystem::current_path(GetExecDirectory());

    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dbc_util_config_tx.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Start the data link
    CDbcStructDataLink dl;
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialization_pending);
    dl.Initialize("");
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::initialized);
    dl.SetOperationMode(sdv::EOperationMode::running);
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::running);

    // Subscribe to several signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalCounter = dispatch.AddPublisher("TestCyclicIfActiveAndSpontaneous.Counter");
    EXPECT_TRUE(signalCounter);

    // The cycle time is 50 ms. Write a value every 50ms. This should result in a spontaneous and a cyclic trigger.
    size_t n = 0;
    std::mutex mtx;
    std::condition_variable cv;
    sdv::core::CTaskTimer timer(50, [&]()
        {
            if (n == 5)
                cv.notify_all();
            else
                signalCounter.Write(n++);
        });
    appcontrol.SetRunningMode();
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    appcontrol.SetConfigMode();
    timer.Reset();

    // Shutdown
    dl.Shutdown();
    EXPECT_EQ(dl.GetStatus(), sdv::EObjectStatus::destruction_pending);

    signalCounter.Reset();

    // Shutdown. This will close the CAN recording.
    appcontrol.Shutdown();

    // Read the CAN recording.
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "transmitter_test.asc"));

    std::vector<size_t> vecStat(5);
    std::vector<double> vecTime(5);
    int32_t iCnt = -1;
    size_t nSignalCnt = 0;
    double dLastTime = 0;
    bool bInit = false;
    while (!reader.IsEOF())
    {
        std::pair<asc::SCanMessage, bool> prMsg = reader.Get();
        EXPECT_TRUE(prMsg.second);
        ++reader;
        dLastTime = prMsg.first.dTimestamp;

        // Only look at the cyclic message
        if (prMsg.first.uiId != 0xb) continue;
        nSignalCnt++;

        iCnt = static_cast<int32_t>(prMsg.first.rguiData[0] >> 5);
        EXPECT_LT(iCnt, 5);
        if (!bInit)
        {
            // Expected iCnt to be the default value...
            if (iCnt != 0 && iCnt != 2)
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_EQ(iCnt, 0, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_EQ(iCnt, 0, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }

            bInit = true;
        }
        else if (iCnt < 5)
        {
            vecStat[iCnt]++;
            if (iCnt && !vecTime[iCnt - 1]) vecTime[iCnt - 1] = prMsg.first.dTimestamp;
        }
    }
    if (!vecTime[4]) vecTime[4] = dLastTime;
    if (iCnt < 4)
    {
        if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_REDUCED);
        else
            SDV_TIMING_EXPECT_GE(iCnt, 4, sdv::TEST::WarningLevel::WARNING_ENABLED);
    }
    for (n = 0; n < 5; n++)
    {
        if (n != 0)
        {
            if (n == 2)
            {
                if (vecStat[n] != 1)       // One trigger no cycle.
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 1, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 1, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (vecStat[n] != 2)       // One trigger and one cycle.
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_EQ(vecStat[n], 2, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            double dPeriod = std::round((vecTime[n] - vecTime[n - 1]) * 1000.0) / 1000.0;
            if (dPeriod > 0.051)   // Max 51ms
            {
                if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_REDUCED);
                else
                    SDV_TIMING_EXPECT_LE(dPeriod, 0.051, sdv::TEST::WarningLevel::WARNING_ENABLED);
            }
            if (n == 4)
            {
                if (dPeriod < 0.019)   // Min 19ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                        SDV_TIMING_EXPECT_GE(dPeriod, 0.019, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
            else
            {
                if (dPeriod < 0.039)   // Min 39ms
                {
                    if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                         SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_REDUCED);
                    else
                         SDV_TIMING_EXPECT_GE(dPeriod, 0.039, sdv::TEST::WarningLevel::WARNING_ENABLED);
                }
            }
        }
    }
}
