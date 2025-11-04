#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <atomic>
#include <chrono>
#include <shared_mutex>

#include <support/signal_support.h>
#include <interfaces/dispatch.h>
#include <support/app_control.h>

TEST(DataDispatchServiceTest, Initialization)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    sdv::TInterfaceAccessPtr ptrDispatchService = sdv::core::GetObject("DataDispatchService");
    sdv::core::ISignalTransmission* pRegister = ptrDispatchService.GetInterface<sdv::core::ISignalTransmission>();
    EXPECT_NE(pRegister, nullptr);
    sdv::core::ISignalAccess* pAccess = ptrDispatchService.GetInterface<sdv::core::ISignalAccess>();
    EXPECT_NE(pAccess, nullptr);

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, RegisterRxSignals)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterRxSignal("def");
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);
    sdv::core::CSignal signal8 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal8);

    // Check for the signals
    auto seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 7u);
    EXPECT_EQ(seqSignals[0].ssName, "abc");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[1].ssName, "def");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[2].ssName, "ghi");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[3].ssName, "jkl");
    EXPECT_EQ(seqSignals[3].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[4].ssName, "mno");
    EXPECT_EQ(seqSignals[4].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[5].ssName, "pqr");
    EXPECT_EQ(seqSignals[5].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[6].ssName, "stu");
    EXPECT_EQ(seqSignals[6].eType, sdv::core::ESignalDirection::sigdir_rx);

    // Unregister the signals
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();

    // Check for the remaining signals
    seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 4u);
    EXPECT_EQ(seqSignals[0].ssName, "abc");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[1].ssName, "mno");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[2].ssName, "pqr");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[3].ssName, "stu");
    EXPECT_EQ(seqSignals[3].eType, sdv::core::ESignalDirection::sigdir_rx);

    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, RegisterTxSignals)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterTxSignal("abc", 10);
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterTxSignal("def", 20);
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterTxSignal("ghi", 30);
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterTxSignal("jkl", 40);
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterTxSignal("mno", 50);
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterTxSignal("pqr", 60);
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterTxSignal("stu", 70);
    EXPECT_TRUE(signal7);
    sdv::core::CSignal signal8 = dispatch.RegisterTxSignal("abc", 80);
    EXPECT_TRUE(signal8);

    // Check for the signals
    auto seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 7u);
    EXPECT_EQ(seqSignals[0].ssName, "abc");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[1].ssName, "def");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[2].ssName, "ghi");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[3].ssName, "jkl");
    EXPECT_EQ(seqSignals[3].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[4].ssName, "mno");
    EXPECT_EQ(seqSignals[4].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[5].ssName, "pqr");
    EXPECT_EQ(seqSignals[5].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[6].ssName, "stu");
    EXPECT_EQ(seqSignals[6].eType, sdv::core::ESignalDirection::sigdir_tx);

    // Unregister the signals
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();

    // Check for the remaining signals
    seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 4u);
    EXPECT_EQ(seqSignals[0].ssName, "abc");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[1].ssName, "mno");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[2].ssName, "pqr");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[3].ssName, "stu");
    EXPECT_EQ(seqSignals[3].eType, sdv::core::ESignalDirection::sigdir_tx);

    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, RegisterMixedSignals)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterTxSignal("abc", 10);
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterTxSignal("def", 20);
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterTxSignal("mno", 50);
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterTxSignal("pqr", 60);
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);

    // Check for the signals
    auto seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 7u);
    EXPECT_EQ(seqSignals[0].ssName, "ghi");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[1].ssName, "jkl");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[2].ssName, "stu");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[3].ssName, "abc");
    EXPECT_EQ(seqSignals[3].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[4].ssName, "def");
    EXPECT_EQ(seqSignals[4].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[5].ssName, "mno");
    EXPECT_EQ(seqSignals[5].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[6].ssName, "pqr");
    EXPECT_EQ(seqSignals[6].eType, sdv::core::ESignalDirection::sigdir_tx);

    // Unregister the signals
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();

    // Check for the remaining signals
    seqSignals = dispatch.GetRegisteredSignals();
    ASSERT_EQ(seqSignals.size(), 3u);
    EXPECT_EQ(seqSignals[0].ssName, "stu");
    EXPECT_EQ(seqSignals[0].eType, sdv::core::ESignalDirection::sigdir_rx);
    EXPECT_EQ(seqSignals[1].ssName, "mno");
    EXPECT_EQ(seqSignals[1].eType, sdv::core::ESignalDirection::sigdir_tx);
    EXPECT_EQ(seqSignals[2].ssName, "pqr");
    EXPECT_EQ(seqSignals[2].eType, sdv::core::ESignalDirection::sigdir_tx);

    signal5.Reset();
    signal6.Reset();
    signal7.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, AccessPublisherSignals)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterTxSignal("abc", 10);
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterTxSignal("def", 20);
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterTxSignal("ghi", 30);
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterTxSignal("jkl", 40);
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterTxSignal("mno", 50);
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterTxSignal("pqr", 60);
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterTxSignal("stu", 70);
    EXPECT_TRUE(signal7);

    // Check for the signals
    sdv::core::CSignal signal8 = dispatch.AddPublisher("abc");
    EXPECT_TRUE(signal8);
    sdv::core::CSignal signal9 = dispatch.AddPublisher("def");
    EXPECT_TRUE(signal9);
    sdv::core::CSignal signal10 = dispatch.AddPublisher("ghi");
    EXPECT_TRUE(signal10);
    sdv::core::CSignal signal11 = dispatch.AddPublisher("jkl");
    EXPECT_TRUE(signal11);
    sdv::core::CSignal signal12 = dispatch.AddPublisher("mno");
    EXPECT_TRUE(signal12);
    sdv::core::CSignal signal13 = dispatch.AddPublisher("pqr");
    EXPECT_TRUE(signal13);
    sdv::core::CSignal signal14 = dispatch.AddPublisher("stu");
    EXPECT_TRUE(signal14);
    sdv::core::CSignal signal15 = dispatch.AddPublisher("vwx");
    EXPECT_FALSE(signal15);

    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();
    signal15.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, AccessSubscriberSignalCallbacks)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterRxSignal("def");
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);

    // Check for the signals
    sdv::core::CSignal signal8 = dispatch.Subscribe("abc", [](sdv::any_t) {});
    EXPECT_TRUE(signal8);
    sdv::core::CSignal signal9 = dispatch.Subscribe("def", [](sdv::any_t) {});
    EXPECT_TRUE(signal9);
    sdv::core::CSignal signal10 = dispatch.Subscribe("ghi", [](sdv::any_t) {});
    EXPECT_TRUE(signal10);
    sdv::core::CSignal signal11 = dispatch.Subscribe("jkl", [](sdv::any_t) {});
    EXPECT_TRUE(signal11);
    sdv::core::CSignal signal12 = dispatch.Subscribe("mno", [](sdv::any_t) {});
    EXPECT_TRUE(signal12);
    sdv::core::CSignal signal13 = dispatch.Subscribe("pqr", [](sdv::any_t) {});
    EXPECT_TRUE(signal13);
    sdv::core::CSignal signal14 = dispatch.Subscribe("stu", [](sdv::any_t) {});
    EXPECT_TRUE(signal14);
    sdv::core::CSignal signal15 = dispatch.Subscribe("vwx", [](sdv::any_t) {});
    EXPECT_FALSE(signal15);

    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();
    signal15.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, AccessSubscriberSignalData)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterRxSignal("def");
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);

    // Check for the signals
    std::atomic<float> f8;
    sdv::core::CSignal signal8 = dispatch.Subscribe("abc", f8);
    EXPECT_TRUE(signal8);
    std::atomic<int32_t> i9;
    sdv::core::CSignal signal9 = dispatch.Subscribe("def", i9);
    EXPECT_TRUE(signal9);
    std::atomic<double> d10;
    sdv::core::CSignal signal10 = dispatch.Subscribe("ghi", d10);
    EXPECT_TRUE(signal10);
    std::atomic<char> c11;
    sdv::core::CSignal signal11 = dispatch.Subscribe("jkl", c11);
    EXPECT_TRUE(signal11);
    std::atomic<uint64_t> ui12;
    sdv::core::CSignal signal12 = dispatch.Subscribe("mno", ui12);
    EXPECT_TRUE(signal12);
    std::atomic<char32_t> c13;
    sdv::core::CSignal signal13 = dispatch.Subscribe("pqr", c13);
    EXPECT_TRUE(signal13);
    std::atomic<int16_t> i14;
    sdv::core::CSignal signal14 = dispatch.Subscribe("stu", i14);
    EXPECT_TRUE(signal14);
    std::atomic<uint8_t> ui15;
    sdv::core::CSignal signal15 = dispatch.Subscribe("vwx", ui15);
    EXPECT_FALSE(signal15);

    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();
    signal15.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, DirectTxSignalTransmission)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterTxSignal("abc", 10);
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterTxSignal("def", 20);
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterTxSignal("ghi", 30);
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterTxSignal("jkl", 40);
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterTxSignal("mno", 50);
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterTxSignal("pqr", 60);
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterTxSignal("stu", 70);
    EXPECT_TRUE(signal7);

    // Add publisher for the signals
    sdv::core::CSignal signal8 = dispatch.AddPublisher("abc");
    EXPECT_TRUE(signal8);
    sdv::core::CSignal signal9 = dispatch.AddPublisher("def");
    EXPECT_TRUE(signal9);
    sdv::core::CSignal signal10 = dispatch.AddPublisher("ghi");
    EXPECT_TRUE(signal10);
    sdv::core::CSignal signal11 = dispatch.AddPublisher("jkl");
    EXPECT_TRUE(signal11);
    sdv::core::CSignal signal12 = dispatch.AddPublisher("mno");
    EXPECT_TRUE(signal12);
    sdv::core::CSignal signal13 = dispatch.AddPublisher("pqr");
    EXPECT_TRUE(signal13);
    sdv::core::CSignal signal14 = dispatch.AddPublisher("stu");
    EXPECT_TRUE(signal14);
    appcontrol.SetRunningMode();

    // Request data for transmission - should be default values
    EXPECT_EQ(signal1.Read().get<int>(), 10);
    EXPECT_EQ(signal2.Read().get<int>(), 20);
    EXPECT_EQ(signal3.Read().get<int>(), 30);
    EXPECT_EQ(signal4.Read().get<int>(), 40);
    EXPECT_EQ(signal5.Read().get<int>(), 50);
    EXPECT_EQ(signal6.Read().get<int>(), 60);
    EXPECT_EQ(signal7.Read().get<int>(), 70);

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);

    // Request data for transmission - should be new values
    EXPECT_EQ(signal1.Read().get<int>(), 100);
    EXPECT_EQ(signal2.Read().get<int>(), 110);
    EXPECT_EQ(signal3.Read().get<int>(), 120);
    EXPECT_EQ(signal4.Read().get<int>(), 130);
    EXPECT_EQ(signal5.Read().get<int>(), 140);
    EXPECT_EQ(signal6.Read().get<int>(), 150);
    EXPECT_EQ(signal7.Read().get<int>(), 160);

    // Check once more...
    EXPECT_EQ(signal1.Read().get<int>(), 100);
    EXPECT_EQ(signal2.Read().get<int>(), 110);
    EXPECT_EQ(signal3.Read().get<int>(), 120);
    EXPECT_EQ(signal4.Read().get<int>(), 130);
    EXPECT_EQ(signal5.Read().get<int>(), 140);
    EXPECT_EQ(signal6.Read().get<int>(), 150);
    EXPECT_EQ(signal7.Read().get<int>(), 160);

    appcontrol.SetConfigMode();
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, DirectRxSignalReceptionCallback)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterRxSignal("def");
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);

    // Subscribe to the signals
    float f8 = 0.0f;
    sdv::core::CSignal signal8 = dispatch.Subscribe("abc", [&](sdv::any_t any) { f8 = any.get<float>(); });
    EXPECT_TRUE(signal8);
    int32_t i9 = 0;
    sdv::core::CSignal signal9 = dispatch.Subscribe("def", [&](sdv::any_t any) { i9 = any.get<int32_t>(); });
    EXPECT_TRUE(signal9);
    double d10 = 0.0;
    sdv::core::CSignal signal10 = dispatch.Subscribe("ghi", [&](sdv::any_t any) { d10 = any.get<double>(); });
    EXPECT_TRUE(signal10);
    char c11 = '\0';
    sdv::core::CSignal signal11 = dispatch.Subscribe("jkl", [&](sdv::any_t any) { c11 = any.get<char>(); });
    EXPECT_TRUE(signal11);
    uint64_t ui12 = 0ull;
    sdv::core::CSignal signal12 = dispatch.Subscribe("mno", [&](sdv::any_t any) { ui12 = any.get<uint64_t>(); });
    EXPECT_TRUE(signal12);
    char32_t c13 = U'\0';
    sdv::core::CSignal signal13 = dispatch.Subscribe("pqr", [&](sdv::any_t any) { c13 = any.get<char32_t>(); });
    EXPECT_TRUE(signal13);
    int16_t i14 = 0;
    sdv::core::CSignal signal14 = dispatch.Subscribe("stu", [&](sdv::any_t any) { i14 = any.get<int16_t>(); });
    EXPECT_TRUE(signal14);
    appcontrol.SetRunningMode();

    // Check values - should be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values
    signal1.Write(10.11f);
    signal2.Write(50);
    signal3.Write(101.202);
    signal4.Write('X');
    signal5.Write(1234567890ll);
    signal6.Write('x');
    signal7.Write(327);

    // Check values - should be updated.
    EXPECT_EQ(f8, 10.11f);
    EXPECT_EQ(i9, 50);
    EXPECT_EQ(d10, 101.202);
    EXPECT_EQ(c11, 'X');
    EXPECT_EQ(ui12,1234567890ull);
    EXPECT_EQ(c13, U'x');
    EXPECT_EQ(i14, 327);

    appcontrol.SetConfigMode();
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, DirectRxSignalReceptionData)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the signals
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterRxSignal("abc");
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterRxSignal("def");
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterRxSignal("ghi");
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterRxSignal("jkl");
    EXPECT_TRUE(signal4);
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);

    // Subscribe to the signals
    std::atomic<float> f8 = 0.0f;
    sdv::core::CSignal signal8 = dispatch.Subscribe("abc", f8);
    EXPECT_TRUE(signal8);
    std::atomic<int32_t> i9 = 0;
    sdv::core::CSignal signal9 = dispatch.Subscribe("def", i9);
    EXPECT_TRUE(signal9);
    std::atomic<double> d10 = 0.0;
    sdv::core::CSignal signal10 = dispatch.Subscribe("ghi", d10);
    EXPECT_TRUE(signal10);
    std::atomic<char> c11 = '\0';
    sdv::core::CSignal signal11 = dispatch.Subscribe("jkl", c11);
    EXPECT_TRUE(signal11);
    std::atomic<uint64_t> ui12 = 0ull;
    sdv::core::CSignal signal12 = dispatch.Subscribe("mno", ui12);
    EXPECT_TRUE(signal12);
    std::atomic<char32_t> c13 = U'\0';
    sdv::core::CSignal signal13 = dispatch.Subscribe("pqr", c13);
    EXPECT_TRUE(signal13);
    std::atomic<int16_t> i14 = 0;
    sdv::core::CSignal signal14 = dispatch.Subscribe("stu", i14);
    EXPECT_TRUE(signal14);
    appcontrol.SetRunningMode();

    // Check values - should be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values
    signal1.Write(10.11f);
    signal2.Write(50);
    signal3.Write(101.202);
    signal4.Write('X');
    signal5.Write(1234567890ll);
    signal6.Write('x');
    signal7.Write(327);

    // Check values - should be updated.
    EXPECT_EQ(f8, 10.11f);
    EXPECT_EQ(i9, 50);
    EXPECT_EQ(d10, 101.202);
    EXPECT_EQ(c11, 'X');
    EXPECT_EQ(ui12,1234567890ull);
    EXPECT_EQ(c13, U'x');
    EXPECT_EQ(i14, 327);

    appcontrol.SetConfigMode();
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, DirectRxTxSignalCommunication)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the TX signals (receiving data from dispatch to be transmitted)
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signal1 = dispatch.RegisterTxSignal("abc", 10.0f);
    EXPECT_TRUE(signal1);
    sdv::core::CSignal signal2 = dispatch.RegisterTxSignal("def", 20);
    EXPECT_TRUE(signal2);
    sdv::core::CSignal signal3 = dispatch.RegisterTxSignal("ghi", 30.0);
    EXPECT_TRUE(signal3);
    sdv::core::CSignal signal4 = dispatch.RegisterTxSignal("jkl", 'A');
    EXPECT_TRUE(signal4);

    // Register the RX signals (sending received data to dispatch)
    sdv::core::CSignal signal5 = dispatch.RegisterRxSignal("mno");
    EXPECT_TRUE(signal5);
    sdv::core::CSignal signal6 = dispatch.RegisterRxSignal("pqr");
    EXPECT_TRUE(signal6);
    sdv::core::CSignal signal7 = dispatch.RegisterRxSignal("stu");
    EXPECT_TRUE(signal7);
    sdv::core::CSignal signal8 = dispatch.RegisterRxSignal("vwx");
    EXPECT_TRUE(signal8);

    // Add publishers for the TX signals
    sdv::core::CSignal signal9 = dispatch.AddPublisher("abc");
    EXPECT_TRUE(signal9);
    sdv::core::CSignal signal10 = dispatch.AddPublisher("def");
    EXPECT_TRUE(signal10);
    sdv::core::CSignal signal11 = dispatch.AddPublisher("ghi");
    EXPECT_TRUE(signal11);
    sdv::core::CSignal signal12 = dispatch.AddPublisher("jkl");
    EXPECT_TRUE(signal12);

    // Add subscribers for the TX signals
    sdv::core::CSignal signal13 = dispatch.Subscribe("mno", [&](sdv::any_t any) { signal9.Write(any); });
    EXPECT_TRUE(signal13);
    sdv::core::CSignal signal14 = dispatch.Subscribe("pqr", [&](sdv::any_t any) { signal10.Write(any); });
    EXPECT_TRUE(signal14);
    sdv::core::CSignal signal15 = dispatch.Subscribe("stu", [&](sdv::any_t any) { signal11.Write(any); });
    EXPECT_TRUE(signal15);
    sdv::core::CSignal signal16 = dispatch.Subscribe("vwx", [&](sdv::any_t any) { signal12.Write(any); });
    EXPECT_TRUE(signal16);
    appcontrol.SetRunningMode();

    // Read the TX signals; should be the default
    EXPECT_EQ(signal1.Read().get<float>(), 10.0f);
    EXPECT_EQ(signal2.Read().get<int>(), 20);
    EXPECT_EQ(signal3.Read().get<double>(), 30.0);
    EXPECT_EQ(signal4.Read().get<char>(), 'A');

    // Write the RX signals
    signal5.Write(10.11f);
    signal6.Write(50);
    signal7.Write(101.202);
    signal8.Write('X');

    // Read the TX signals; should be the written value
    EXPECT_EQ(signal1.Read().get<float>(), 10.11f);
    EXPECT_EQ(signal2.Read().get<int>(), 50);
    EXPECT_EQ(signal3.Read().get<double>(), 101.202);
    EXPECT_EQ(signal4.Read().get<char>(), 'X');

    // Read the TX signals once more; signal 2 and 4 should be default, 1 and 3 the written value
    EXPECT_EQ(signal1.Read().get<float>(), 10.11f);
    EXPECT_EQ(signal2.Read().get<int>(), 50);
    EXPECT_EQ(signal3.Read().get<double>(), 101.202);
    EXPECT_EQ(signal4.Read().get<char>(), 'X');

    appcontrol.SetConfigMode();
    signal1.Reset();
    signal2.Reset();
    signal3.Reset();
    signal4.Reset();
    signal5.Reset();
    signal6.Reset();
    signal7.Reset();
    signal8.Reset();
    signal9.Reset();
    signal10.Reset();
    signal11.Reset();
    signal12.Reset();
    signal13.Reset();
    signal14.Reset();
    signal15.Reset();
    signal16.Reset();

    appcontrol.Shutdown();
}

TEST(DataDispatchServiceTest, DirectRxTxSignalConcurrency)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_dds_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    // Register the TX signals (receiving data from dispatch to be transmitted)
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx1a = dispatch.RegisterTxSignal("SigTx1a", 10);
    EXPECT_TRUE(signalTx1a);
    sdv::core::CSignal signalTx1b = dispatch.RegisterTxSignal("SigTx1b", 20);
    EXPECT_TRUE(signalTx1b);
    sdv::core::CSignal signalTx1c = dispatch.RegisterTxSignal("SigTx1c", 30);
    EXPECT_TRUE(signalTx1c);
    sdv::core::CSignal signalTx1d = dispatch.RegisterTxSignal("SigTx1d", 40);
    EXPECT_TRUE(signalTx1d);
    sdv::core::CSignal signalTx2a = dispatch.RegisterTxSignal("SigTx2a", 50);
    EXPECT_TRUE(signalTx2a);
    sdv::core::CSignal signalTx2b = dispatch.RegisterTxSignal("SigTx2b", 60);
    EXPECT_TRUE(signalTx2b);
    sdv::core::CSignal signalTx2c = dispatch.RegisterTxSignal("SigTx2c", 70);
    EXPECT_TRUE(signalTx2c);
    sdv::core::CSignal signalTx2d = dispatch.RegisterTxSignal("SigTx2d", 80);
    EXPECT_TRUE(signalTx2d);
    sdv::core::CSignal signalTx3a = dispatch.RegisterTxSignal("SigTx3a", 90);
    EXPECT_TRUE(signalTx3a);
    sdv::core::CSignal signalTx3b = dispatch.RegisterTxSignal("SigTx3b", 100);
    EXPECT_TRUE(signalTx3b);
    sdv::core::CSignal signalTx3c = dispatch.RegisterTxSignal("SigTx3c", 110);
    EXPECT_TRUE(signalTx3c);
    sdv::core::CSignal signalTx3d = dispatch.RegisterTxSignal("SigTx3d", 120);
    EXPECT_TRUE(signalTx3d);
    sdv::core::CSignal signalTx4a = dispatch.RegisterTxSignal("SigTx4a", 130);
    EXPECT_TRUE(signalTx4a);
    sdv::core::CSignal signalTx4b = dispatch.RegisterTxSignal("SigTx4b", 140);
    EXPECT_TRUE(signalTx4b);
    sdv::core::CSignal signalTx4c = dispatch.RegisterTxSignal("SigTx4c", 150);
    EXPECT_TRUE(signalTx4c);
    sdv::core::CSignal signalTx4d = dispatch.RegisterTxSignal("SigTx4d", 160);
    EXPECT_TRUE(signalTx4d);

    // Register the RX signals (sending received data to dispatch)
    sdv::core::CSignal signalRx1a = dispatch.RegisterRxSignal("SigRx1a");
    EXPECT_TRUE(signalRx1a);
    sdv::core::CSignal signalRx1b = dispatch.RegisterRxSignal("SigRx1b");
    EXPECT_TRUE(signalRx1b);
    sdv::core::CSignal signalRx1c = dispatch.RegisterRxSignal("SigRx1c");
    EXPECT_TRUE(signalRx1c);
    sdv::core::CSignal signalRx1d = dispatch.RegisterRxSignal("SigRx1d");
    EXPECT_TRUE(signalRx1d);
    sdv::core::CSignal signalRx2a = dispatch.RegisterRxSignal("SigRx2a");
    EXPECT_TRUE(signalRx2a);
    sdv::core::CSignal signalRx2b = dispatch.RegisterRxSignal("SigRx2b");
    EXPECT_TRUE(signalRx2b);
    sdv::core::CSignal signalRx2c = dispatch.RegisterRxSignal("SigRx2c");
    EXPECT_TRUE(signalRx2c);
    sdv::core::CSignal signalRx2d = dispatch.RegisterRxSignal("SigRx2d");
    EXPECT_TRUE(signalRx2d);
    sdv::core::CSignal signalRx3a = dispatch.RegisterRxSignal("SigRx3a");
    EXPECT_TRUE(signalRx3a);
    sdv::core::CSignal signalRx3b = dispatch.RegisterRxSignal("SigRx3b");
    EXPECT_TRUE(signalRx3b);
    sdv::core::CSignal signalRx3c = dispatch.RegisterRxSignal("SigRx3c");
    EXPECT_TRUE(signalRx3c);
    sdv::core::CSignal signalRx3d = dispatch.RegisterRxSignal("SigRx3d");
    EXPECT_TRUE(signalRx3d);
    sdv::core::CSignal signalRx4a = dispatch.RegisterRxSignal("SigRx4a");
    EXPECT_TRUE(signalRx4a);
    sdv::core::CSignal signalRx4b = dispatch.RegisterRxSignal("SigRx4b");
    EXPECT_TRUE(signalRx4b);
    sdv::core::CSignal signalRx4c = dispatch.RegisterRxSignal("SigRx4c");
    EXPECT_TRUE(signalRx4c);
    sdv::core::CSignal signalRx4d = dispatch.RegisterRxSignal("SigRx4d");
    EXPECT_TRUE(signalRx4d);

    bool bShutdown = false;
    std::srand(static_cast<unsigned>(std::time(0)));

    // Thread sync
    std::shared_mutex mtxStart;
    std::atomic_uint32_t uiInitCnt = 0;

    // Publisher thread function
    auto fnPublisher = [&](uint32_t uiIndex) -> void
    {
        try
        {
            std::string ssSignalName = "SigTx" + std::to_string(uiIndex % 4 + 1);
            sdv::core::CSignal signalPubA = dispatch.AddPublisher(ssSignalName + "a");
            EXPECT_TRUE(signalPubA);
            sdv::core::CSignal signalPubB = dispatch.AddPublisher(ssSignalName + "b");
            EXPECT_TRUE(signalPubB);
            sdv::core::CSignal signalPubC = dispatch.AddPublisher(ssSignalName + "c");
            EXPECT_TRUE(signalPubC);
            sdv::core::CSignal signalPubD = dispatch.AddPublisher(ssSignalName + "d");
            EXPECT_TRUE(signalPubD);

            uiInitCnt++;
            std::shared_lock<std::shared_mutex> lock(mtxStart);

            while (!bShutdown)
            {
                signalPubA.Write(std::rand());
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubB.Write(std::rand());
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubC.Write(std::rand());
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubD.Write(std::rand());
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
            }
        }
        catch (...)
        {
            // Should not crash
            EXPECT_TRUE(false);
        }
    };

    // Consumer thread function
    auto fnConsumer = [&](uint32_t uiIndex) -> void
    {
        try
        {
            std::atomic<uint32_t> uiA, uiB, uiC, uiD;
            std::string ssSignalName = "SigRx" + std::to_string(uiIndex % 4 + 1);
            sdv::core::CSignal signalConsA = dispatch.Subscribe(ssSignalName + "a", uiA);
            EXPECT_TRUE(signalConsA);
            sdv::core::CSignal signalConsB = dispatch.Subscribe(ssSignalName + "b", uiB);
            EXPECT_TRUE(signalConsB);
            sdv::core::CSignal signalConsC = dispatch.Subscribe(ssSignalName + "c", uiC);
            EXPECT_TRUE(signalConsC);
            sdv::core::CSignal signalConsD = dispatch.Subscribe(ssSignalName + "d", uiD);
            EXPECT_TRUE(signalConsD);

            uiInitCnt++;
            std::shared_lock<std::shared_mutex> lock(mtxStart);

            while (!bShutdown)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            signalConsA.Reset();
            signalConsB.Reset();
            signalConsC.Reset();
            signalConsD.Reset();
        }
        catch (...)
        {
            // Should not crash
            EXPECT_TRUE(false);
        }
    };

    // Start the threads
    std::unique_lock<std::shared_mutex> lockStart(mtxStart);
    uint32_t uiCnt = 0;
    std::thread rgPublishThreads[16];
    for (std::thread& rThread : rgPublishThreads)
        rThread = std::thread(fnPublisher, uiCnt++);
    std::thread rgConsumeThreads[16];
    for (std::thread& rThread : rgConsumeThreads)
        rThread = std::thread(fnConsumer, uiCnt++);

    // Trigger the "GO"
    while (uiInitCnt < 32) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    appcontrol.SetRunningMode();
    lockStart.unlock();

    // Send and receive data for 5 seconds
    const auto start = std::chrono::high_resolution_clock::now();
    try
    {
        while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count() < 2000)
        {
            signalTx1a.Read();
            signalTx1b.Read();
            signalTx1c.Read();
            signalTx1d.Read();
            signalTx2a.Read();
            signalTx2b.Read();
            signalTx2c.Read();
            signalTx2d.Read();
            signalTx3a.Read();
            signalTx3b.Read();
            signalTx3c.Read();
            signalTx3d.Read();
            signalTx4a.Read();
            signalTx4b.Read();
            signalTx4c.Read();
            signalTx4d.Read();
            signalRx1a.Write(std::rand());
            signalRx1b.Write(std::rand());
            signalRx1c.Write(std::rand());
            signalRx1d.Write(std::rand());
            signalRx2a.Write(std::rand());
            signalRx2b.Write(std::rand());
            signalRx2c.Write(std::rand());
            signalRx2d.Write(std::rand());
            signalRx3a.Write(std::rand());
            signalRx3b.Write(std::rand());
            signalRx3c.Write(std::rand());
            signalRx3d.Write(std::rand());
            signalRx4a.Write(std::rand());
            signalRx4b.Write(std::rand());
            signalRx4c.Write(std::rand());
            signalRx4d.Write(std::rand());
        }
    } catch (...)
    {
        // Should not crash
        EXPECT_TRUE(false);
    }

    // Wait for all threads to finalize
    appcontrol.SetConfigMode();
    bShutdown = true;
    for (std::thread& rThread : rgPublishThreads)
        if (rThread.joinable()) rThread.join();
    for (std::thread& rThread : rgConsumeThreads)
        if (rThread.joinable()) rThread.join();

    signalTx1a.Reset();
    signalTx1b.Reset();
    signalTx1c.Reset();
    signalTx1d.Reset();
    signalTx2a.Reset();
    signalTx2b.Reset();
    signalTx2c.Reset();
    signalTx2d.Reset();
    signalTx3a.Reset();
    signalTx3b.Reset();
    signalTx3c.Reset();
    signalTx3d.Reset();
    signalTx4a.Reset();
    signalTx4b.Reset();
    signalTx4c.Reset();
    signalTx4d.Reset();
    signalRx1a.Reset();
    signalRx1b.Reset();
    signalRx1c.Reset();
    signalRx1d.Reset();
    signalRx2a.Reset();
    signalRx2b.Reset();
    signalRx2c.Reset();
    signalRx2d.Reset();
    signalRx3a.Reset();
    signalRx3b.Reset();
    signalRx3c.Reset();
    signalRx3d.Reset();
    signalRx4a.Reset();
    signalRx4b.Reset();
    signalRx4c.Reset();
    signalRx4d.Reset();

    appcontrol.Shutdown();
}
