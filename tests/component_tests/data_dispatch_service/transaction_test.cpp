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

TEST(DataDispatchServiceTest, TransactionalTxSignalTransmission)
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

    // Start read transaction
    sdv::core::CTransaction transaction = dispatch.CreateTransaction();

    // Request data for transmission using the transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction).get<int>(), 70);

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);

    // Request data for transmission using the transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction).get<int>(), 70);

    // Send data through the publisher
    signal8.Write(200);
    signal9.Write(210);
    signal10.Write(220);
    signal11.Write(230);
    signal12.Write(240);
    signal13.Write(250);
    signal14.Write(260);

    // Finalize the transaction
    transaction.Finish();

    // Request data for transmission - should be new values
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

    // Check once more...
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

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

TEST(DataDispatchServiceTest, TransactionalRxSignalReceptionCallback)
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

    // Start write transaction
    sdv::core::CTransaction transaction = dispatch.CreateTransaction();

    // Update the values
    signal1.Write(10.11f, transaction);
    signal2.Write(50, transaction);
    signal3.Write(101.202, transaction);
    signal4.Write('X', transaction);
    signal5.Write(1234567890ll, transaction);
    signal6.Write('x', transaction);
    signal7.Write(327, transaction);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values
    signal1.Write(20.11f, transaction);
    signal2.Write(60, transaction);
    signal3.Write(201.202, transaction);
    signal4.Write('Y', transaction);
    signal5.Write(2234567890ll, transaction);
    signal6.Write('y', transaction);
    signal7.Write(427, transaction);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Finalize the transaction
    transaction.Finish();

    // Check values - should be updated.
    EXPECT_EQ(f8, 20.11f);
    EXPECT_EQ(i9, 60);
    EXPECT_EQ(d10, 201.202);
    EXPECT_EQ(c11, 'Y');
    EXPECT_EQ(ui12, 2234567890ull);
    EXPECT_EQ(c13, U'y');
    EXPECT_EQ(i14, 427);

    // Update the values
    signal1.Write(30.11f);
    signal2.Write(70);
    signal3.Write(301.202);
    signal4.Write('Z');
    signal5.Write(3234567890ll);
    signal6.Write('z');
    signal7.Write(527);

    // Check values - should be updated.
    EXPECT_EQ(f8, 30.11f);
    EXPECT_EQ(i9, 70);
    EXPECT_EQ(d10, 301.202);
    EXPECT_EQ(c11, 'Z');
    EXPECT_EQ(ui12, 3234567890ull);
    EXPECT_EQ(c13, U'z');
    EXPECT_EQ(i14, 527);

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

TEST(DataDispatchServiceTest, TransactionalRxSignalReceptionData)
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

    // Start write transaction
    sdv::core::CTransaction transaction = dispatch.CreateTransaction();

    // Update the values
    signal1.Write(10.11f, transaction);
    signal2.Write(50, transaction);
    signal3.Write(101.202, transaction);
    signal4.Write('X', transaction);
    signal5.Write(1234567890ll, transaction);
    signal6.Write('x', transaction);
    signal7.Write(327, transaction);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values
    signal1.Write(20.11f, transaction);
    signal2.Write(60, transaction);
    signal3.Write(201.202, transaction);
    signal4.Write('Y', transaction);
    signal5.Write(2234567890ll, transaction);
    signal6.Write('y', transaction);
    signal7.Write(427, transaction);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Finalize the transaction
    transaction.Finish();

    // Check values - should be updated.
    EXPECT_EQ(f8, 20.11f);
    EXPECT_EQ(i9, 60);
    EXPECT_EQ(d10, 201.202);
    EXPECT_EQ(c11, 'Y');
    EXPECT_EQ(ui12, 2234567890ull);
    EXPECT_EQ(c13, U'y');
    EXPECT_EQ(i14, 427);

    // Update the values
    signal1.Write(30.11f);
    signal2.Write(70);
    signal3.Write(301.202);
    signal4.Write('Z');
    signal5.Write(3234567890ll);
    signal6.Write('z');
    signal7.Write(527);

    // Check values - should be updated.
    EXPECT_EQ(f8, 30.11f);
    EXPECT_EQ(i9, 70);
    EXPECT_EQ(d10, 301.202);
    EXPECT_EQ(c11, 'Z');
    EXPECT_EQ(ui12, 3234567890ull);
    EXPECT_EQ(c13, U'z');
    EXPECT_EQ(i14, 527);

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

TEST(DataDispatchServiceTest, TransactionalSignalPublishing)
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

    // Start write transaction
    sdv::core::CTransaction transaction = dispatch.CreateTransaction();

    // Send data through the publisher using the transaction
    signal8.Write(100, transaction);
    signal9.Write(110, transaction);
    signal10.Write(120, transaction);
    signal11.Write(130, transaction);
    signal12.Write(140, transaction);
    signal13.Write(150, transaction);
    signal14.Write(160, transaction);

    // Request data for transmission using the transaction - should be default values
    EXPECT_EQ(signal1.Read().get<int>(), 10);
    EXPECT_EQ(signal2.Read().get<int>(), 20);
    EXPECT_EQ(signal3.Read().get<int>(), 30);
    EXPECT_EQ(signal4.Read().get<int>(), 40);
    EXPECT_EQ(signal5.Read().get<int>(), 50);
    EXPECT_EQ(signal6.Read().get<int>(), 60);
    EXPECT_EQ(signal7.Read().get<int>(), 70);

    // Finalize the transaction
    transaction.Finish();

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

TEST(DataDispatchServiceTest, MultiTransactionalTxSignalTransmission)
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

    // Start read transactions
    sdv::core::CTransaction transaction1 = dispatch.CreateTransaction();
    sdv::core::CTransaction transaction2 = dispatch.CreateTransaction();

    // Request data for transmission using the first transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction1).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction1).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction1).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction1).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction1).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction1).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction1).get<int>(), 70);

    // Request data for transmission using the second transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction2).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction2).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction2).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction2).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction2).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction2).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction2).get<int>(), 70);

    // Send data through the publisher
    signal8.Write(100);
    signal9.Write(110);
    signal10.Write(120);
    signal11.Write(130);
    signal12.Write(140);
    signal13.Write(150);
    signal14.Write(160);

    // Request data for transmission using the first transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction1).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction1).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction1).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction1).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction1).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction1).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction1).get<int>(), 70);

    // Request data for transmission using the second transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction1).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction1).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction1).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction1).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction1).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction1).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction1).get<int>(), 70);

    // Send data through the publisher
    signal8.Write(200);
    signal9.Write(210);
    signal10.Write(220);
    signal11.Write(230);
    signal12.Write(240);
    signal13.Write(250);
    signal14.Write(260);

    // Finalize the first transaction
    transaction1.Finish();

    // Request data for transmission - should be new values
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

    // Request data for transmission using the second transaction - should be default values
    EXPECT_EQ(signal1.Read(transaction2).get<int>(), 10);
    EXPECT_EQ(signal2.Read(transaction2).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transaction2).get<int>(), 30);
    EXPECT_EQ(signal4.Read(transaction2).get<int>(), 40);
    EXPECT_EQ(signal5.Read(transaction2).get<int>(), 50);
    EXPECT_EQ(signal6.Read(transaction2).get<int>(), 60);
    EXPECT_EQ(signal7.Read(transaction2).get<int>(), 70);

    // Finalize the second transaction
    transaction2.Finish();

    // Request data for transmission - should be new value, except for signal 2, 4 and 6. They should have default value, since
    // they were requested before and reset after reading.
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

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

TEST(DataDispatchServiceTest, MultiTransactionalRxSignalReceptionCallback)
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

    // Start write transactions
    sdv::core::CTransaction transaction1 = dispatch.CreateTransaction();
    sdv::core::CTransaction transaction2 = dispatch.CreateTransaction();

    // Update the values using the first transaction
    signal1.Write(10.11f, transaction1);
    signal2.Write(50, transaction1);
    signal3.Write(101.202, transaction1);
    signal4.Write('X', transaction1);
    signal5.Write(1234567890ll, transaction1);
    signal6.Write('x', transaction1);
    signal7.Write(327, transaction1);

    // Update the values using the second transaction
    signal1.Write(20.22f, transaction1);
    signal2.Write(60, transaction1);
    signal3.Write(202.303, transaction1);
    signal4.Write('Y', transaction1);
    signal5.Write(2234567890ll, transaction1);
    signal6.Write('y', transaction1);
    signal7.Write(427, transaction1);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values using the first transaction
    signal1.Write(40.44f, transaction1);
    signal2.Write(70, transaction1);
    signal3.Write(303.404, transaction1);
    signal4.Write('Z', transaction1);
    signal5.Write(3234567890ll, transaction1);
    signal6.Write('z', transaction1);
    signal7.Write(527, transaction1);

    // Update the values using the second transaction
    signal1.Write(50.55f, transaction2);
    signal2.Write(80, transaction2);
    signal3.Write(404.505, transaction2);
    signal4.Write('A', transaction2);
    signal5.Write(4234567890ll, transaction2);
    signal6.Write('a', transaction2);
    signal7.Write(627, transaction2);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Finalize the first transaction
    transaction1.Finish();

    // Check values - should be updated with values of first transaction
    EXPECT_EQ(f8, 40.44f);
    EXPECT_EQ(i9, 70);
    EXPECT_EQ(d10, 303.404);
    EXPECT_EQ(c11, 'Z');
    EXPECT_EQ(ui12, 3234567890ull);
    EXPECT_EQ(c13, U'z');
    EXPECT_EQ(i14, 527);

    // Update the values
    signal1.Write(60.66f);
    signal2.Write(90);
    signal3.Write(606.707);
    signal4.Write('B');
    signal5.Write(5234567890ll);
    signal6.Write('b');
    signal7.Write(727);

    // Check values - should be updated.
    EXPECT_EQ(f8, 60.66f);
    EXPECT_EQ(i9, 90);
    EXPECT_EQ(d10, 606.707);
    EXPECT_EQ(c11, 'B');
    EXPECT_EQ(ui12, 5234567890ull);
    EXPECT_EQ(c13, U'b');
    EXPECT_EQ(i14, 727);

    // Finalize the second transaction
    transaction2.Finish();

    // Check values - should be updated with the values of second transaction.
    EXPECT_EQ(f8, 50.55f);
    EXPECT_EQ(i9, 80);
    EXPECT_EQ(d10, 404.505);
    EXPECT_EQ(c11, 'A');
    EXPECT_EQ(ui12, 4234567890ull);
    EXPECT_EQ(c13, U'a');
    EXPECT_EQ(i14, 627);

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

TEST(DataDispatchServiceTest, MultiTransactionalRxSignalReceptionData)
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

    // Start write transactions
    sdv::core::CTransaction transaction1 = dispatch.CreateTransaction();
    sdv::core::CTransaction transaction2 = dispatch.CreateTransaction();

    // Update the values using the first transaction
    signal1.Write(10.11f, transaction1);
    signal2.Write(50, transaction1);
    signal3.Write(101.202, transaction1);
    signal4.Write('X', transaction1);
    signal5.Write(1234567890ll, transaction1);
    signal6.Write('x', transaction1);
    signal7.Write(327, transaction1);

    // Update the values using the second transaction
    signal1.Write(20.22f, transaction1);
    signal2.Write(60, transaction1);
    signal3.Write(202.303, transaction1);
    signal4.Write('Y', transaction1);
    signal5.Write(2234567890ll, transaction1);
    signal6.Write('y', transaction1);
    signal7.Write(427, transaction1);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Update the values using the first transaction
    signal1.Write(40.44f, transaction1);
    signal2.Write(70, transaction1);
    signal3.Write(303.404, transaction1);
    signal4.Write('Z', transaction1);
    signal5.Write(3234567890ll, transaction1);
    signal6.Write('z', transaction1);
    signal7.Write(527, transaction1);

    // Update the values using the second transaction
    signal1.Write(50.55f, transaction2);
    signal2.Write(80, transaction2);
    signal3.Write(404.505, transaction2);
    signal4.Write('A', transaction2);
    signal5.Write(4234567890ll, transaction2);
    signal6.Write('a', transaction2);
    signal7.Write(627, transaction2);

    // Check values - should still be default.
    EXPECT_EQ(f8, 0.0f);
    EXPECT_EQ(i9, 0);
    EXPECT_EQ(d10, 0.0);
    EXPECT_EQ(c11, '\0');
    EXPECT_EQ(ui12, 0ull);
    EXPECT_EQ(c13, U'\0');
    EXPECT_EQ(i14, 0);

    // Finalize the first transaction
    transaction1.Finish();

    // Check values - should be updated with values of first transaction
    EXPECT_EQ(f8, 40.44f);
    EXPECT_EQ(i9, 70);
    EXPECT_EQ(d10, 303.404);
    EXPECT_EQ(c11, 'Z');
    EXPECT_EQ(ui12, 3234567890ull);
    EXPECT_EQ(c13, U'z');
    EXPECT_EQ(i14, 527);

    // Update the values
    signal1.Write(60.66f);
    signal2.Write(90);
    signal3.Write(606.707);
    signal4.Write('B');
    signal5.Write(5234567890ll);
    signal6.Write('b');
    signal7.Write(727);

    // Check values - should be updated.
    EXPECT_EQ(f8, 60.66f);
    EXPECT_EQ(i9, 90);
    EXPECT_EQ(d10, 606.707);
    EXPECT_EQ(c11, 'B');
    EXPECT_EQ(ui12, 5234567890ull);
    EXPECT_EQ(c13, U'b');
    EXPECT_EQ(i14, 727);

    // Finalize the second transaction
    transaction2.Finish();

    // Check values - should be updated with the values of second transaction.
    EXPECT_EQ(f8, 50.55f);
    EXPECT_EQ(i9, 80);
    EXPECT_EQ(d10, 404.505);
    EXPECT_EQ(c11, 'A');
    EXPECT_EQ(ui12, 4234567890ull);
    EXPECT_EQ(c13, U'a');
    EXPECT_EQ(i14, 627);

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

TEST(DataDispatchServiceTest, MultiTransactionalSignalPublishing)
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

    // Start write transactions
    sdv::core::CTransaction transaction1 = dispatch.CreateTransaction();
    sdv::core::CTransaction transaction2 = dispatch.CreateTransaction();

    // Send data through the publisher using the first transaction
    signal8.Write(100, transaction1);
    signal9.Write(110, transaction1);
    signal10.Write(120, transaction1);
    signal11.Write(130, transaction1);
    signal12.Write(140, transaction1);
    signal13.Write(150, transaction1);
    signal14.Write(160, transaction1);

    // Send data through the publisher using the first transaction
    signal8.Write(200, transaction2);
    signal9.Write(210, transaction2);
    signal11.Write(230, transaction2);
    signal12.Write(240, transaction2);
    signal13.Write(250, transaction2);
    signal14.Write(260, transaction2);
    signal10.Write(220, transaction2);

    // Request data for transmission using the transaction - should be default values
    EXPECT_EQ(signal1.Read().get<int>(), 10);
    EXPECT_EQ(signal2.Read().get<int>(), 20);
    EXPECT_EQ(signal3.Read().get<int>(), 30);
    EXPECT_EQ(signal4.Read().get<int>(), 40);
    EXPECT_EQ(signal5.Read().get<int>(), 50);
    EXPECT_EQ(signal6.Read().get<int>(), 60);
    EXPECT_EQ(signal7.Read().get<int>(), 70);

    // Finalize the first transaction
    transaction1.Finish();

    // Request data for transmission - should be new values from first transaction
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

    // Finalize the second transaction
    transaction2.Finish();

    // Request data for transmission - should be new values from second transaction
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

    // Check once more...
    EXPECT_EQ(signal1.Read().get<int>(), 200);
    EXPECT_EQ(signal2.Read().get<int>(), 210);
    EXPECT_EQ(signal3.Read().get<int>(), 220);
    EXPECT_EQ(signal4.Read().get<int>(), 230);
    EXPECT_EQ(signal5.Read().get<int>(), 240);
    EXPECT_EQ(signal6.Read().get<int>(), 250);
    EXPECT_EQ(signal7.Read().get<int>(), 260);

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

TEST(DataDispatchServiceTest, TransactionalRxTxSignalCommunication)
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

    // Create the transactions
    sdv::core::CTransaction transactionRx = dispatch.CreateTransaction();
    sdv::core::CTransaction transactionTx = dispatch.CreateTransaction();
    sdv::core::CTransaction transactionPublish = dispatch.CreateTransaction();

    // Add subscribers for the TX signals
    uint32_t uiPublishCnt = 0;
    sdv::core::CSignal signal13 = dispatch.Subscribe("mno", [&](sdv::any_t any)
        {
            uiPublishCnt++;
            signal9.Write(any, transactionPublish);
        });
    EXPECT_TRUE(signal13);
    sdv::core::CSignal signal14 = dispatch.Subscribe("pqr", [&](sdv::any_t any)
        {
            uiPublishCnt++;
            signal10.Write(any, transactionPublish);
        });
    EXPECT_TRUE(signal14);
    sdv::core::CSignal signal15 = dispatch.Subscribe("stu", [&](sdv::any_t any)
        {
            uiPublishCnt++;
            signal11.Write(any, transactionPublish);
        });
    EXPECT_TRUE(signal15);
    sdv::core::CSignal signal16 = dispatch.Subscribe("vwx", [&](sdv::any_t any)
        {
            uiPublishCnt++;
            signal12.Write(any, transactionPublish);
        });
    EXPECT_TRUE(signal16);
    appcontrol.SetRunningMode();

    // Read the TX signals; should be the default
    EXPECT_EQ(signal1.Read(transactionTx).get<float>(), 10.0f);
    EXPECT_EQ(signal2.Read(transactionTx).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transactionTx).get<double>(), 30.0);
    EXPECT_EQ(signal4.Read(transactionTx).get<char>(), 'A');

    // The publish count should be 0
    EXPECT_EQ(uiPublishCnt, 0ul);

    // Write the RX signals
    signal5.Write(10.11f, transactionRx);
    signal6.Write(50, transactionRx);
    signal7.Write(101.202, transactionRx);
    signal8.Write('X', transactionRx);

    // The publish count should be 0
    EXPECT_EQ(uiPublishCnt, 0ul);

    // Read the TX signals; should be the default
    EXPECT_EQ(signal1.Read(transactionTx).get<float>(), 10.0f);
    EXPECT_EQ(signal2.Read(transactionTx).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transactionTx).get<double>(), 30.0);
    EXPECT_EQ(signal4.Read(transactionTx).get<char>(), 'A');

    // Finalize the RX transaction; all signals are written.
    transactionRx.Finish();

    // The publish count should be 4
    EXPECT_EQ(uiPublishCnt, 4ul);

    // Read the TX signals; should be the default
    EXPECT_EQ(signal1.Read(transactionTx).get<float>(), 10.0f);
    EXPECT_EQ(signal2.Read(transactionTx).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transactionTx).get<double>(), 30.0);
    EXPECT_EQ(signal4.Read(transactionTx).get<char>(), 'A');

    // Finalize the publish transaction; all signals are published
    transactionPublish.Finish();

    // Read the TX signals; should be the default
    EXPECT_EQ(signal1.Read(transactionTx).get<float>(), 10.0f);
    EXPECT_EQ(signal2.Read(transactionTx).get<int>(), 20);
    EXPECT_EQ(signal3.Read(transactionTx).get<double>(), 30.0);
    EXPECT_EQ(signal4.Read(transactionTx).get<char>(), 'A');

    // Finalize the TX transaction; all written signals are available.
    transactionTx.Finish();

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

TEST(DataDispatchServiceTest, TransactionalRxTxSignalConcurrency)
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

    std::atomic_uint64_t uiValueCnt = 1000;

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
                sdv::core::CTransaction transaction = dispatch.CreateTransaction();
                uint64_t uiValue = std::rand();
                signalPubA.Write(uiValue, transaction);
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubB.Write(uiValue + 10, transaction);
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubC.Write(uiValue + 20, transaction);
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                signalPubD.Write(uiValue + 30, transaction);
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                transaction.Finish();
            }
        }
        catch (...)
        {
            // Should not crash
            EXPECT_TRUE(false);
        }
    };

    // Consumer thread function
    const size_t nConsumeThreads = 16;
    std::atomic<uint32_t> uiA[nConsumeThreads], uiB[nConsumeThreads], uiC[nConsumeThreads], uiD[nConsumeThreads];
    auto fnConsumer = [&](uint32_t uiIndex) -> void
    {
        try
        {
            std::string ssSignalName = "SigRx" + std::to_string(uiIndex % 4 + 1);
            sdv::core::CSignal signalConsA = dispatch.Subscribe(ssSignalName + "a", uiA[uiIndex % nConsumeThreads]);
            EXPECT_TRUE(signalConsA);
            sdv::core::CSignal signalConsB = dispatch.Subscribe(ssSignalName + "b", uiB[uiIndex % nConsumeThreads]);
            EXPECT_TRUE(signalConsB);
            sdv::core::CSignal signalConsC = dispatch.Subscribe(ssSignalName + "c", uiC[uiIndex % nConsumeThreads]);
            EXPECT_TRUE(signalConsC);
            sdv::core::CSignal signalConsD = dispatch.Subscribe(ssSignalName + "d", uiD[uiIndex % nConsumeThreads]);
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
    std::thread rgConsumeThreads[nConsumeThreads];
    for (std::thread& rThread :rgConsumeThreads)
        rThread = std::thread(fnConsumer, uiCnt++);

    // Trigger the "GO"
    while (uiInitCnt < 32) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    appcontrol.SetRunningMode();
    lockStart.unlock();

    // Send and receive data for 5 seconds
    const auto start = std::chrono::high_resolution_clock::now();
    uint64_t uiTxA[4], uiTxB[4], uiTxC[4], uiTxD[4];
    try
    {
        while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count() < 2000)
        {
            sdv::core::CTransaction transaction = dispatch.CreateTransaction();
            uiTxA[0] = signalTx1a.Read(transaction);
            uiTxB[0] = signalTx1b.Read(transaction);
            uiTxC[0] = signalTx1c.Read(transaction);
            uiTxD[0] = signalTx1d.Read(transaction);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiTxA[1] = signalTx2a.Read(transaction);
            uiTxB[1] = signalTx2b.Read(transaction);
            uiTxC[1] = signalTx2c.Read(transaction);
            uiTxD[1] = signalTx2d.Read(transaction);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiTxA[2] = signalTx3a.Read(transaction);
            uiTxB[2] = signalTx3b.Read(transaction);
            uiTxC[2] = signalTx3c.Read(transaction);
            uiTxD[2] = signalTx3d.Read(transaction);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiTxA[3] = signalTx4a.Read(transaction);
            uiTxB[3] = signalTx4b.Read(transaction);
            uiTxC[3] = signalTx4c.Read(transaction);
            uiTxD[3] = signalTx4d.Read(transaction);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uint64_t uiValue = std::rand();
            signalRx1a.Write(uiValue);
            signalRx1b.Write(uiValue + 10);
            signalRx1c.Write(uiValue + 20);
            signalRx1d.Write(uiValue + 30);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiValue = std::rand();
            signalRx2a.Write(uiValue);
            signalRx2b.Write(uiValue + 10);
            signalRx2c.Write(uiValue + 20);
            signalRx2d.Write(uiValue + 30);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiValue = std::rand();
            signalRx3a.Write(uiValue);
            signalRx3b.Write(uiValue + 10);
            signalRx3c.Write(uiValue + 20);
            signalRx3d.Write(uiValue + 30);
            transaction.Finish();
            transaction = dispatch.CreateTransaction();
            uiValue = std::rand();
            signalRx4a.Write(uiValue);
            signalRx4b.Write(uiValue + 10);
            signalRx4c.Write(uiValue + 20);
            signalRx4d.Write(uiValue + 30);
            transaction.Finish();
        }
    } catch (...)
    {
        // Should not crash
        EXPECT_TRUE(false);
    }

    // Wait for everything to finalize
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Wait for all threads to finalize
    appcontrol.SetConfigMode();
    bShutdown = true;
    for (std::thread& rThread : rgPublishThreads)
        if (rThread.joinable()) rThread.join();
    for (std::thread& rThread : rgConsumeThreads)
        if (rThread.joinable()) rThread.join();

    // Check consume ranges
    for (size_t nIndex = 0; nIndex < nConsumeThreads; nIndex++)
    {
        EXPECT_EQ(uiB[nIndex] - uiA[nIndex], 10u);
        EXPECT_EQ(uiC[nIndex] - uiA[nIndex], 20u);
        EXPECT_EQ(uiD[nIndex] - uiA[nIndex], 30u);
    }

    // Check Tx ranges
    for (size_t nIndex = 0; nIndex < 4; nIndex++)
    {
        EXPECT_EQ(uiTxB[nIndex] - uiTxA[nIndex], 10u);
        EXPECT_EQ(uiTxC[nIndex] - uiTxA[nIndex], 20u);
        EXPECT_EQ(uiTxD[nIndex] - uiTxA[nIndex], 30u);
    }

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

