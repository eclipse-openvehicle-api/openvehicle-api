#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <atomic>
#include <chrono>

#include <support/signal_support.h>
#include <interfaces/dispatch.h>
#include <support/app_control.h>

TEST(DataDispatchServiceTest, SingleSpontaneousTrigger)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; });
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, 7);

    // Send data through the publisher
    signal8.Write(101);
    signal9.Write(111);
    signal10.Write(121);
    signal11.Write(131);
    signal12.Write(141);
    signal13.Write(151);
    signal14.Write(161);
    EXPECT_EQ(nTriggerCnt, 14);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, MultipleSpontaneousTrigger)
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

    // Create a trigger
    size_t nTrigger1Cnt = 0;
    sdv::core::CTrigger trigger1 = dispatch.CreateTxTrigger([&] { nTrigger1Cnt++; });
    EXPECT_TRUE(trigger1);
    trigger1.AddSignal(signal1);
    trigger1.AddSignal(signal2);
    trigger1.AddSignal(signal3);
    size_t nTrigger2Cnt = 0;
    sdv::core::CTrigger trigger2 = dispatch.CreateTxTrigger([&] { nTrigger2Cnt++; });
    EXPECT_TRUE(trigger2);
    trigger2.AddSignal(signal4);
    trigger2.AddSignal(signal5);
    trigger2.AddSignal(signal6);
    trigger2.AddSignal(signal7);
    EXPECT_EQ(nTrigger2Cnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTrigger1Cnt, 3);
    EXPECT_EQ(nTrigger2Cnt, 4);

    trigger1.Reset();
    trigger2.Reset();

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

TEST(DataDispatchServiceTest, TriggerRemoveSignals)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; });
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, 7);

    // Remove signals from trigger
    trigger.RemoveSignal(signal4);
    trigger.RemoveSignal(signal4);  // Check double removal
    trigger.RemoveSignal(signal5);

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, 12);

    // Reset signal
    signal6.Reset();
    signal7.Reset();

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, 15);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, SpontaneousTransactionalTrigger)
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

    // Start read transaction
    sdv::core::CTransaction transaction = dispatch.CreateTransaction();

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; });
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    signal8.Write(100, transaction);
    signal9.Write(110, transaction);
    signal10.Write(120, transaction);
    signal11.Write(130, transaction);
    signal12.Write(140, transaction);
    signal13.Write(150, transaction);
    signal14.Write(160, transaction);
    EXPECT_EQ(nTriggerCnt, 0);

    // Send data through the publisher
    signal8.Write(200, transaction);
    signal9.Write(210, transaction);
    signal10.Write(220, transaction);
    signal11.Write(230, transaction);
    signal12.Write(240, transaction);
    signal13.Write(250, transaction);
    signal14.Write(260, transaction);
    EXPECT_EQ(nTriggerCnt, 0);

    // Finalize the transaction
    transaction.Finish();
    EXPECT_EQ(nTriggerCnt, 1);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, PeriodicTrigger)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; }, false, 0, 100);
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    size_t nTriggerCntTemp = nTriggerCnt;
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp);

    // Sleep for 750 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    // Check trigger count.
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp + 7);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, PeriodicTriggerOnlyActive)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; }, false, 0, 100, true);
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    size_t nTriggerCntTemp = nTriggerCnt;
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp);

    // Sleep for 220 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(220));

    // Back to default value
    nTriggerCntTemp = nTriggerCnt;
    signal8.Write(10);
    signal9.Write(20);
    signal10.Write(30);
    signal11.Write(40);
    signal12.Write(50);
    signal13.Write(60);
    signal14.Write(70);
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp);

    // Sleep for 700 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp + 1);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, SpontaneousAndPeriodicTrigger)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; }, true, 0, 100);
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    size_t nTriggerCntTemp = nTriggerCnt;
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp + 7);

    // Sleep for 750 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    // Check trigger count.
    EXPECT_EQ(nTriggerCnt, nTriggerCntTemp + 14);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

TEST(DataDispatchServiceTest, SingleSpontaneousTriggerWithDelay)
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

    // Create a trigger
    size_t nTriggerCnt = 0;
    sdv::core::CTrigger trigger = dispatch.CreateTxTrigger([&] { nTriggerCnt++; }, true, 100);
    EXPECT_TRUE(trigger);
    trigger.AddSignal(signal1);
    trigger.AddSignal(signal2);
    trigger.AddSignal(signal3);
    trigger.AddSignal(signal4);
    trigger.AddSignal(signal5);
    trigger.AddSignal(signal6);
    trigger.AddSignal(signal7);
    EXPECT_EQ(nTriggerCnt, 0);
    appcontrol.SetRunningMode();

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);
    EXPECT_EQ(nTriggerCnt, 1);

    // Send data through the publisher
    signal8.Write(101);
    signal9.Write(111);
    signal10.Write(121);
    signal11.Write(131);
    signal12.Write(141);
    signal13.Write(151);
    signal14.Write(161);
    EXPECT_EQ(nTriggerCnt, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(nTriggerCnt, 2);

    appcontrol.SetConfigMode();
    trigger.Reset();

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

