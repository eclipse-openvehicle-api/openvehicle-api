/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../global/ipc_named_mutex.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <string>

namespace
{
std::string MakeUniqueMutexName(const char* suffix)
{
    return std::string("HELLO_") + suffix + "_" +
        std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}
}

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
    ipc::named_mutex mtx(MakeUniqueMutexName("Construction"));
    EXPECT_NE(mtx.native_handle(), nullptr);
}

TEST(NamedMutexTest, CritSectSyncManualLock)
{
    const std::string mutexName = MakeUniqueMutexName("CritSectSyncManualLock");

    // Counter function check for correct counter value.
    // The checking is manipulated by the bEnable flag. When disabled, no sync will be done and the check will fail. When enabled,
    // sync will be done and the check will succeed.
    int32_t iCnt = 0;
    std::atomic_bool bSuccess = true;
    std::atomic_bool bEnable = false;
    auto fn = [&]()
    {
        ipc::named_mutex mtx(mutexName);

        if (bEnable)
            mtx.lock();
        bSuccess = bSuccess && (iCnt == 0);
        iCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
        iCnt--;
        bSuccess = bSuccess && (iCnt == 0);
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
    const std::string mutexName = MakeUniqueMutexName("CritSectSyncAutoLock");

    // Counter function check for correct counter value.
    int32_t iCnt = 0;
    std::atomic_bool bSuccess = true;
    auto fn = [&]()
    {
        ipc::named_mutex mtx(mutexName);

        std::unique_lock<ipc::named_mutex> lock(mtx);
        bSuccess = bSuccess && (iCnt == 0);
        iCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
        iCnt--;
        bSuccess = bSuccess && (iCnt == 0);
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
    const std::string mutexName = MakeUniqueMutexName("TryLock");

    std::atomic_bool bRunning = false;
    auto fn = [&]()
    {
        ipc::named_mutex mtx(mutexName);

        mtx.lock();
        while (bRunning) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mtx.unlock();
    };

    bRunning = true;
    std::thread thread(fn);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Try locking; doesn't work since thread still locks.
    ipc::named_mutex mtx2(mutexName);
    EXPECT_FALSE(mtx2.try_lock());

    bRunning = false;
    thread.join();

    // Try locking; this works. Thread has ended.
    EXPECT_TRUE(mtx2.try_lock());

    mtx2.unlock();
}

TEST(NamedMutexTest, Naming)
{
    ipc::named_mutex mtx1(MakeUniqueMutexName("Naming"));
    EXPECT_FALSE(mtx1.name().empty());

    ipc::named_mutex mtx2;
    EXPECT_FALSE(mtx2.name().empty());
}
