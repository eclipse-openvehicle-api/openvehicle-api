#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"
#include "pattern_gen.h"
#include "../../../sdv_services/ipc_shared_mem/shared_mem_buffer_posix.h"
#include "../../../sdv_services/ipc_shared_mem/shared_mem_buffer_windows.h"
#include "../../include/logger_test_helper.h"
#include <../global/base64.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <interfaces/ipc.h>
#include <interfaces/process.h>

#if defined(__GNUC__) && !defined(_WIN32)
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#endif

/**
* @brief Load support modules to publish the needed services.
*/
void LoadSupportServices();

TEST(SharedMemoryBufferTest, CreateBuffer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, TriggerTestRx)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    bool bShutdown = false;
    size_t nCorrectCnt = 0;
    std::condition_variable cvStart;
    std::mutex mtxStart;
    auto fnWaitForTrigger = [&]()
    {
        std::unique_lock<std::mutex> lock(mtxStart);
        lock.unlock();
        cvStart.notify_all();
        while (!bShutdown)
        {
            bool bResult = receiver.WaitForFreeSpace(200);
            if (bShutdown) break;
            if (bResult)
                nCorrectCnt++;
        }
    };

    std::unique_lock<std::mutex> lockStart(mtxStart);
    std::thread thread(fnWaitForTrigger);
    cvStart.wait(lockStart);

    for (size_t n = 0; n < 20; n++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sender.TriggerDataReceive();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Let the buffer finish its sending.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bShutdown = true;
    thread.join();

    EXPECT_GE(nCorrectCnt, 20);
}

TEST(SharedMemoryBufferTest, TriggerTestTx)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    bool bShutdown = false;
    size_t nCorrectCnt = 0;
    std::condition_variable cvStart;
    std::mutex mtxStart;
    auto fnWaitForTrigger = [&]()
    {
        std::unique_lock<std::mutex> lock(mtxStart);
        lock.unlock();
        cvStart.notify_all();
        while (!bShutdown)
        {
            bool bResult = receiver.WaitForData(200);
            if (bShutdown) break;
            if (bResult)
                nCorrectCnt++;
        }
    };

    std::unique_lock<std::mutex> lockStart(mtxStart);
    std::thread thread(fnWaitForTrigger);
    cvStart.wait(lockStart);

    for (size_t n = 0; n < 20; n++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sender.TriggerDataSend();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Let the buffer finish its sending.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bShutdown = true;
    thread.join();

    EXPECT_GE(nCorrectCnt, 20);
}

TEST(SharedMemoryBufferTest, TriggerTestRxTx)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    bool bShutdown = false;
    size_t nCorrectCnt = 0;
    std::condition_variable cvSenderStart, cvReceiverStart;
    std::mutex mtxReceiverStart;
    std::mutex mtxSenderStart;
    auto fnWaitForTriggerReceiver = [&]()
    {
        std::unique_lock<std::mutex> lockReceiver(mtxReceiverStart);
        lockReceiver.unlock();
        cvReceiverStart.notify_all();
        while (!bShutdown)
        {
            if (receiver.WaitForData(200))
                receiver.TriggerDataReceive();
        }
    };
    auto fnWaitForTriggerSender = [&]()
    {
        std::unique_lock<std::mutex> lockSender(mtxSenderStart);
        lockSender.unlock();
        cvSenderStart.notify_all();
        std::unique_lock<std::mutex> lockReceiver(mtxReceiverStart);
        cvReceiverStart.wait(lockReceiver);
        lockReceiver.unlock();
        while (!bShutdown)
        {
            bool bResult = sender.WaitForFreeSpace(200);
            if (bShutdown) break;
            if (bResult)
                nCorrectCnt++;
        }
    };

    std::unique_lock<std::mutex> lockStartSender(mtxSenderStart);
    std::thread threadSender(fnWaitForTriggerSender);
    cvSenderStart.wait(lockStartSender);
    lockStartSender.unlock();
    std::unique_lock<std::mutex> lockStartReceiver(mtxReceiverStart);
    std::thread threadReceiver(fnWaitForTriggerReceiver);
    cvReceiverStart.wait(lockStartReceiver);
    lockStartReceiver.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));  // Needed for the threads to enter their loop.
    for (size_t n = 0; n < 200; n++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sender.TriggerDataSend();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Let the buffer finish its sending.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bShutdown = true;
    threadSender.join();
    threadReceiver.join();

    EXPECT_GE(nCorrectCnt, 200);
}

TEST(SharedMemoryBufferTest, SimpleSynchronousWriteRead)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    EXPECT_TRUE(sender.TryWrite("HELLO", 6));
    auto optPacket = receiver.TryRead();
    ASSERT_TRUE(optPacket);
    EXPECT_EQ(strcmp(optPacket->GetData<char>(), "HELLO"), 0);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, ReadWithoutSending)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    auto optPacket = receiver.TryRead();
    EXPECT_FALSE(optPacket);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, RequestReadPacketSize)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    EXPECT_TRUE(sender.TryWrite("HELLO", 5));

    auto optPacket = receiver.TryRead();
    EXPECT_TRUE(optPacket);
    EXPECT_EQ(optPacket->GetSize(), 5u);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, FragmentRead)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    EXPECT_TRUE(sender.TryWrite("HELLO", 6));
    EXPECT_TRUE(sender.TryWrite("HELLO2", 7));

    auto optPacket = receiver.TryRead();
    EXPECT_TRUE(optPacket);
    ASSERT_EQ(optPacket->GetSize(), 6u);
    EXPECT_EQ(strcmp(optPacket->GetData<char>(), "HELLO"), 0);

    optPacket = receiver.TryRead();
    EXPECT_TRUE(optPacket);
    ASSERT_EQ(optPacket->GetSize(), 7u);
    EXPECT_EQ(strcmp(optPacket->GetData<char>(), "HELLO2"), 0);

    optPacket = receiver.TryRead();
    EXPECT_FALSE(optPacket);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, BufferBoundary)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender(256);
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    // The buffer header has 16 bytes
    // Each allocation is 8 bytes header, 6 bytes data and 2 bytes alignment
    for (int32_t iIndex = 0; iIndex < 14; iIndex++)
        EXPECT_TRUE(sender.TryWrite("HELLO", 6));
    EXPECT_FALSE(sender.TryWrite("HELLO", 6));

    // Read packets again
    std::optional<CAccessorRxPacket> optPacket;
    for (int32_t iIndex = 0; iIndex < 14; iIndex++)
    {
        optPacket = receiver.TryRead();
        EXPECT_TRUE(optPacket);
        ASSERT_EQ(optPacket->GetSize(), 6u);
        EXPECT_EQ(strcmp(optPacket->GetData<char>(), "HELLO"), 0);
        optPacket->Accept();
    }
    optPacket = receiver.TryRead();
    EXPECT_FALSE(optPacket);

    // Try storing a large packet
    char szLargeText[256] = {};
    EXPECT_FALSE(sender.TryWrite(szLargeText, 241));

    // Loop 100 times storing a 100 byte packet
    for (int32_t iIndex = 0; iIndex < 100; iIndex++)
    {
        EXPECT_TRUE(sender.TryWrite(szLargeText, 100));
        optPacket = receiver.TryRead();
        EXPECT_TRUE(optPacket);
        EXPECT_EQ(optPacket->GetSize(), 100u);
        optPacket->Accept();
    }

    // Loop 1000 times storing a 10 byte packet
    for (int32_t iIndex = 0; iIndex < 1000; iIndex++)
    {
        EXPECT_TRUE(sender.TryWrite(szLargeText, 10));
        optPacket = receiver.TryRead();
        EXPECT_TRUE(optPacket);
        EXPECT_EQ(optPacket->GetSize(), 10u);
        optPacket->Accept();
    }

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, ReserveCommitAccessReleaseNonChronologicalOrder)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender(256);
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    // Reserve buffers for strings
    // The buffer header has 16 bytes
    // Each allocation is 8 bytes header, 5 bytes data and 3 bytes alignment
    CAccessorTxPacket rgTxPackets[32] = {};
    for (int32_t iIndex = 0; iIndex < 15; iIndex++)
    {
        auto optTxPacket = sender.Reserve(5);
        if (iIndex == 14)	// 14th allocation should fail (buffer full)
        {
            EXPECT_FALSE(optTxPacket);
            continue;
        }

        ASSERT_TRUE(optTxPacket);
        rgTxPackets[iIndex] = std::move(*optTxPacket);
        EXPECT_NE(rgTxPackets[iIndex].GetDataPtr(), nullptr);
        if (rgTxPackets[iIndex])
            strcpy(rgTxPackets[iIndex].GetDataPtr<char>(), std::to_string(iIndex).c_str());
    }

    // Reading should fail, nothing is committed
    auto optRxPacket = receiver.TryRead();
    EXPECT_FALSE(optRxPacket);

    // Commit the buffers in reverse order
    for (int32_t iIndex = 13; iIndex >= 0; iIndex--)
    {
        rgTxPackets[iIndex].Commit();

        // Reading succeeds only after the index equals 0 (being the first entry).
        optRxPacket = receiver.TryRead();
        if (iIndex == 0)
        {
            // The text should contain the number 0
            EXPECT_TRUE(optRxPacket);
            EXPECT_NE(optRxPacket->GetData(), nullptr);
            EXPECT_EQ(std::to_string(0), optRxPacket->GetData<char>());
            optRxPacket->Accept();
        }
        else
            EXPECT_FALSE(optRxPacket);
    }

    // Since one packet was read, only one can be written again
    for (int32_t iIndex = 14; iIndex < 16; iIndex++)
    {
        auto optTxPacket = sender.Reserve(5);
        if (iIndex >= 15) // 15th allocation should fail (buffer full)
        {
            EXPECT_FALSE(optTxPacket);
            continue;
        }

        ASSERT_TRUE(optTxPacket);
        rgTxPackets[iIndex] = std::move(*optTxPacket);
        EXPECT_NE(rgTxPackets[iIndex].GetDataPtr(), nullptr);
        if (rgTxPackets[iIndex])
            strcpy(rgTxPackets[iIndex].GetDataPtr<char>(), std::to_string(iIndex).c_str());
        rgTxPackets[iIndex].Commit();
    }

    // Access the packets
    CAccessorRxPacket rgRxPackets[32];
    for (int32_t iIndex = 1; iIndex < 16; iIndex++)
    {
        // 13 allocations are stored; the allocation with index 15 should fail
        optRxPacket = receiver.TryRead();
        if (iIndex == 15)
            EXPECT_FALSE(optRxPacket);
        else
        {
            rgRxPackets[iIndex] = std::move(CAccessorRxPacket(std::move(*optRxPacket)));
            EXPECT_TRUE(rgRxPackets[iIndex]);
            EXPECT_NE(rgRxPackets[iIndex].GetData(), nullptr);
            EXPECT_NE(rgRxPackets[iIndex].GetSize(), 0u);
        }
        if (rgRxPackets[iIndex])
        {
            EXPECT_EQ(std::to_string(iIndex), rgRxPackets[iIndex].GetData<char>());
        }
    }

    // It should not be possible to reserve another packet
    EXPECT_FALSE(sender.Reserve(5));

    // Release the read packets in reverse order. Only after the packet with the first index has been released it will be possible
    // to write packets again.
    for (int32_t iIndex = 14; iIndex >= 1; iIndex--)
    {
        rgRxPackets[iIndex].Accept();

        if (iIndex == 1)
            EXPECT_TRUE(sender.Reserve(5));
        else
            EXPECT_FALSE(sender.Reserve(5));
    }

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, SendReceivePattern)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    CPatternReceiver pattern_inspector(receiver);
    CPatternSender pattern_generator(sender);

    // Wait for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S));

    // Shutdown
    pattern_generator.Shutdown();
    pattern_inspector.Shutdown();

    std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
              << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
    std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
              << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
              << " errors, " << std::endl;
    EXPECT_NE(pattern_generator.GetCycleCnt(), 0u);
    EXPECT_NE(pattern_generator.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_generator.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_inspector.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_inspector.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetByteCnt(), 0ull);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, DelayedSendReceivePattern)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    CPatternReceiver pattern_inspector(receiver);
    CPatternSender	 pattern_generator(sender, 10);

    // Wait for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S));

    // Shutdown
    pattern_generator.Shutdown();
    pattern_inspector.Shutdown();

    std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
              << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
    std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
              << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
              << " errors, " << std::endl;
    EXPECT_NE(pattern_generator.GetCycleCnt(), 0u);
    EXPECT_NE(pattern_generator.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_generator.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_inspector.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_inspector.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetByteCnt(), 0ull);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, SendDelayedReceivePattern)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx sender;
    EXPECT_TRUE(sender.IsValid());

    CSharedMemBufferRx receiver(sender.GetConnectionString());
    EXPECT_TRUE(receiver.IsValid());

    CPatternReceiver pattern_inspector(receiver, 10);
    CPatternSender	 pattern_generator(sender);

    // Wait for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S));

    // Shutdown
    pattern_generator.Shutdown();
    pattern_inspector.Shutdown();

    std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
              << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
    std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
              << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
              << " errors, " << std::endl;
    EXPECT_NE(pattern_generator.GetCycleCnt(), 0u);
    EXPECT_NE(pattern_generator.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_generator.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_inspector.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_inspector.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetByteCnt(), 0ull);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, SendRepeatReceivePattern)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(""));

    CSharedMemBufferTx bufferTX;
    EXPECT_TRUE(bufferTX.IsValid());
    CSharedMemBufferRx bufferRX;
    EXPECT_TRUE(bufferRX.IsValid());

    // The connection string containing the RX and TX strings for the repeater
    std::string ssConnectionString = bufferTX.GetConnectionString() + "\n" + bufferRX.GetConnectionString();

    CSharedMemBufferRx bufferRepeaterRX(ssConnectionString);
    EXPECT_TRUE(bufferRepeaterRX.IsValid());
    CSharedMemBufferTx bufferRepeaterTX(ssConnectionString);
    EXPECT_TRUE(bufferRepeaterTX.IsValid());

    CPatternReceiver pattern_inspector(bufferRX);
    CPatternRepeater pattern_repeater(bufferRepeaterRX, bufferRepeaterTX);
    CPatternSender	 pattern_generator(bufferTX);

    // Wait for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S));

    // Shutdown
    pattern_generator.Shutdown();
    pattern_repeater.Shutdown();
    pattern_inspector.Shutdown();

    std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
              << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
    std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
              << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
              << " errors, " << std::endl;
    std::cout << "Pattern repeater: " << pattern_repeater.GetCycleCnt() << " cyles, " << pattern_repeater.GetPacketCnt()
              << " packets, " << pattern_repeater.GetByteCnt() << " bytes, " << pattern_repeater.GetErrorCnt()
              << " errors, " << std::endl;
    EXPECT_NE(pattern_generator.GetCycleCnt(), 0u);
    EXPECT_NE(pattern_generator.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_generator.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_inspector.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_inspector.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_repeater.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_repeater.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_repeater.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_repeater.GetByteCnt(), 0ull);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, AppProcessSendRepeatReceivePattern)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    CSharedMemBufferTx bufferTX;
    EXPECT_TRUE(bufferTX.IsValid());
    CSharedMemBufferRx bufferRX;
    EXPECT_TRUE(bufferRX.IsValid());

    // Start process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs = {Base64EncodePlainText(bufferTX.GetConnectionString() + "\n" + bufferRX.GetConnectionString())};
    sdv::process::TProcessID tProcessID = pProcessControl->Execute("UnitTest_SharedMemTests_App_Repeater", seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    CPatternSender	 pattern_generator(bufferTX);
    CPatternReceiver pattern_inspector(bufferRX);

    // Wait for 4 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S * 2));

    // Shutdown
    pattern_generator.Shutdown();
    pattern_inspector.Shutdown();

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID, 0xffffffff);

    std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
              << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
    std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
              << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
              << " errors, " << std::endl;
    EXPECT_NE(pattern_generator.GetCycleCnt(), 0u);
    EXPECT_NE(pattern_generator.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_generator.GetByteCnt(), 0ull);
    EXPECT_NE(pattern_inspector.GetCycleCnt(), 0u);
    EXPECT_EQ(pattern_inspector.GetErrorCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetPacketCnt(), 0u);
    EXPECT_NE(pattern_inspector.GetByteCnt(), 0ull);

    appcontrol.Shutdown();
}

TEST(SharedMemoryBufferTest, SendRepeatReceivePatternBetweenTwoAppProcesses)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode = "Essential")code"));
    LoadSupportServices();

    // test starts 2 app processes, one should be the sender of the pattern, the other is the repeater
    // the log file must not contain any error
    DeleteLoggerFile("appTransmitter.log");
    DeleteLoggerFile("appReceiver.log");

    CSharedMemBufferTx bufferTransmitterTx;
    EXPECT_TRUE(bufferTransmitterTx.IsValid());
    CSharedMemBufferRx bufferTransmitterRx(bufferTransmitterTx.GetConnectionString());
    EXPECT_TRUE(bufferTransmitterRx.IsValid());
    CSharedMemBufferTx bufferRepeaterTx;
    EXPECT_TRUE(bufferRepeaterTx.IsValid());
    CSharedMemBufferRx bufferRepeaterRx(bufferRepeaterTx.GetConnectionString());
    EXPECT_TRUE(bufferRepeaterRx.IsValid());

    // Start processes
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    ASSERT_NE(pProcessControl, nullptr);
    ASSERT_NE(pProcessLifetime, nullptr);
    sdv::sequence<sdv::u8string> seqArgs1 = {Base64EncodePlainText(bufferTransmitterRx.GetConnectionString() + "\n" + bufferRepeaterTx.GetConnectionString()), "SURROGATE_TRANSMITTER"};
    sdv::process::TProcessID tProcessID1 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Repeater", seqArgs1, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);
    sdv::sequence<sdv::u8string> seqArgs2 = {Base64EncodePlainText(bufferTransmitterTx.GetConnectionString() + "\n" + bufferRepeaterRx.GetConnectionString())};
    sdv::process::TProcessID tProcessID2 = pProcessControl->Execute("UnitTest_SharedMemTests_App_Repeater", seqArgs2, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    // Wait for 4 seconds
    std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S * 3 ));

    // Wait for process termination
    pProcessLifetime->WaitForTerminate(tProcessID1, 0xffffffff);
    pProcessLifetime->WaitForTerminate(tProcessID2, 0xffffffff);

    appcontrol.Shutdown();
}
