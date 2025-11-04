#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../global/ipc_named_mutex.h"
#include <chrono>
#include <thread>

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(NamedMutexTest, Construction)
{
    ipc::named_mutex mtx("HELLO");
    EXPECT_NE(mtx.native_handle(), nullptr);
}

TEST(NamedMutexTest, CritSectSyncManualLock)
{
    // Counter function check for correct counter value.
    // The checking is manipulated by the bEnable flag. When disabled, no sync will be done and the check will fail. When enabled,
    // sync will be done and the check will succeed.
    int32_t iCnt = 0;
    bool bSuccess = true;
    bool bEnable = false;
    auto fn = [&]()
    {
        ipc::named_mutex mtx("HELLO");

        if (bEnable)
            mtx.lock();
        bSuccess &= (iCnt == 0);
        iCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
        iCnt--;
        bSuccess &= (iCnt == 0);
        if (bEnable)
            mtx.unlock();
    };

    // Test no sync
    std::thread rgThreads[100];
    for (std::thread& rthread : rgThreads)
        rthread = std::thread(fn);
    for (std::thread& rthread : rgThreads)
        rthread.join();
    EXPECT_FALSE(bSuccess);

    // Test sync
    iCnt = 0;
    bSuccess = true;
    bEnable = true;
    for (std::thread& rthread : rgThreads)
        rthread = std::thread(fn);
    for (std::thread& rthread : rgThreads)
        rthread.join();

    EXPECT_TRUE(bSuccess);
}

TEST(NamedMutexTest, CritSectSyncAutoLock)
{
    // Counter function check for correct counter value.
    int32_t iCnt = 0;
    bool bSuccess = true;
    auto fn = [&]()
    {
        ipc::named_mutex mtx("HELLO");

        std::unique_lock<ipc::named_mutex> lock(mtx);
        bSuccess &= (iCnt == 0);
        iCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
        iCnt--;
        bSuccess &= (iCnt == 0);
    };

    // Test sync
    std::thread rgThreads[100];
    for (std::thread& rthread : rgThreads)
        rthread = std::thread(fn);
    for (std::thread& rthread : rgThreads)
        rthread.join();

    EXPECT_TRUE(bSuccess);
}

TEST(NamedMutexTest, TryLock)
{
    bool bRunning = false;
    auto fn = [&]()
    {
        ipc::named_mutex mtx("HELLO");

        mtx.lock();
        while (bRunning) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mtx.unlock();
    };

    bRunning = true;
    std::thread thread(fn);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Try locking; doesn't work since thread still locks.
    ipc::named_mutex mtx2("HELLO");
    EXPECT_FALSE(mtx2.try_lock());

    bRunning = false;
    thread.join();

    // Try locking; this works. Thread has ended.
    EXPECT_TRUE(mtx2.try_lock());

    mtx2.unlock();
}

TEST(NamedMutexTest, Naming)
{
    ipc::named_mutex mtx1("HELLO");
    EXPECT_FALSE(mtx1.name().empty());

    ipc::named_mutex mtx2(nullptr);
    EXPECT_FALSE(mtx2.name().empty());
}
