#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <iostream>
#include "../../../global/process_watchdog.h"

#ifdef WIN32
#include <Windows.h>
#endif

// It turns out that not all thread libraries handle processor load very well. This causes many tests that are executed in the
// test cases of the platform to malfunction due to timeouts. To prevent this situation, test the concurrency of the thread
// library by executing four threads in parallel and measuring the execution time. If the time exceeds 100ms, the thread execution
// was blocked for more than 100ms, which is a very bad sign for a system that needs to simulate real-time.
// The tests use an additional of 32 threads creating noise (heavy CPU load) to simulate a heavily loaded system.
// The following thread libraries are known to work well:
//      MS Windows native thread library (MSVCRT)
//      POSIX Threads
//
// The following thread libraries are known to not work so well:
//      MCF Gthread Library
//
// The following hints should be adhered when dealing with threads:
//  - Do not instantiate threads in a constructor of a global variable. - Thread management is blocked during the initialization
//    of a an application or dynamic library (at least in Windows this is the case). A thread management call (for example the
//    creation of a new thread) will cause a deadlock.
//  - Do not instantiate the thread in the constructor initialization list. In some cases the constructor initialization and
//    thread instantiation will overlap. With POSIX threads this results in sporadic dead locks. Example of bad code:
//          class CThread
//          {
//          public:
//              CThread() : m_thread(&CThread::ThreadFunc, this)
//              {}
//          ...
//              void ThreadFunc()
//              { ... }
//          private:
//              std::thread m_thread;
//          };
//  - Thread instantiation can take a long time. Use a boolean or a condition variable to wait until the thread has started to
//    prevent race conditions when waiting for a thread that is not even running yet.
//  - Condition variables are stateless. If a condition variable is used to detect whether the condition has occurred and the
//    trigger happens before the wait function was called, the wait function doesn't get informed and waits for the occurrence to
//    stil happen. This could cause a deadlock when assuming the condition will be triggered, but the wait function is called too
//    late (e.g. due to heavy system load). While loops running and checking for the condition and waiting for entering or exiting
//    threads might be an example.
//    This situation can be solved by using a variable which is set just before the notify is called and keeps it setting until the
//    trigger is processed - then it is reset. The wait with prediction could be used for this. Or a while loop checking at
//    intervals.

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

/**
 * @brief Make noise by having a certain amount of thread execute the random generation of numbers at full speed. This would
 * produce processor load.
 * @tparam nAmount The amount of threads to create noise.
 */
template <size_t nAmount = 32>
struct SMakeNoice
{
    /**
     * @brief Constructor
     */
    SMakeNoice()
    {
        for (std::thread& rthread : rgThreads)
            rthread = std::thread([&]()
                {
                    nStarted++;
                    int nCnt = 0;
                    while (!bShutdown)
                        nCnt += std::rand();
                });
        while (nStarted < nAmount) {}   // Wait until all have started.
    }

    /**
     * @brief Destructor
     */
    ~SMakeNoice()
    {
        bShutdown = true;
        for (std::thread& rthread : rgThreads)
            rthread.join();
    }

    std::atomic_bool    bShutdown = false;      ///< Run threads until shutdown is set.
    std::atomic_size_t  nStarted = 0;           ///< Amount of threads that were started. This is to wait for all threads to start.
    std::thread         rgThreads[nAmount];     ///< The noise generating threads.
};

TEST(ConcurrencyTest, ConditionVarWaitFor_SeparateMutexForEachThread)
{
    std::condition_variable cv1, cv2, cv3, cv4;
    std::mutex mtx1, mtx2, mtx3, mtx4;
    std::atomic_size_t nCnt = 0;
    std::atomic_size_t nThreadCnt = 0;
    std::atomic_size_t nViolationCnt = 0;
    SMakeNoice noice;
    std::thread thread1([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt++ < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx4);
                cv1.notify_all();
                cv4.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread2([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx1);
                cv2.notify_all();
                cv1.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread3([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx2);
                cv3.notify_all();
                cv2.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread4([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx3);
                cv4.notify_all();
                cv3.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    cv1.notify_all();
    cv2.notify_all();
    cv3.notify_all();
    cv4.notify_all();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    if (nViolationCnt >= 50) // 5% is allowed to fail...
        std::cout << "TIMING ConcurrencyTest - ConditionVarWaitFor_SeparateMutexForEachThread nViolationCnt < 50; currently " << nViolationCnt << std::endl;
    //EXPECT_LT(nViolationCnt, 50); // 5% is allowed to fail...
}

TEST(ConcurrencyTest, ConditionVarWaitForPrediction_SeparateMutexForEachThread)
{
    std::condition_variable cv1, cv2, cv3, cv4;
    std::mutex mtx1, mtx2, mtx3, mtx4;
    std::atomic_size_t nCnt = 0;
    std::atomic_size_t nThreadCnt = 0;
    std::atomic_size_t nViolationCnt = 0;
    bool b1 = false, b2 = false, b3 = false, b4 = false;
    SMakeNoice noice;
    std::thread thread1([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt++ < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx4);
                b1 = true;
                cv1.notify_all();
                cv4.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b4; b4 = false; return b;});
            }
        });
    std::thread thread2([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx1);
                b2 = true;
                cv2.notify_all();
                cv1.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b1; b1 = false; return b;});
            }
        });
    std::thread thread3([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx2);
                b3 = true;
                cv3.notify_all();
                cv2.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b2; b2 = false; return b;});
            }
        });
    std::thread thread4([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx3);
                b4 = true;
                cv4.notify_all();
                cv3.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b3; b3 = false; return b;});
            }
        });
    cv1.notify_all();
    cv2.notify_all();
    cv3.notify_all();
    cv4.notify_all();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    if (nViolationCnt >= 50) // 5% is allowed to fail...
        std::cout << "TIMING ConcurrencyTest - ConditionVarWaitForPrediction_SeparateMutexForEachThread nViolationCnt < 50; currently " << nViolationCnt << std::endl;
    //EXPECT_LT(nViolationCnt, 50); // 5% is allowed to fail...
}

TEST(ConcurrencyTest, ConditionVarWaitFor_JointMutexForEachThread)
{
    std::condition_variable cv1, cv2, cv3, cv4;
    std::mutex mtx;
    std::atomic_size_t nCnt = 0;
    std::atomic_size_t nThreadCnt = 0;
    std::atomic_size_t nViolationCnt = 0;
    SMakeNoice noice;
    std::thread thread1([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt++ < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx);
                cv1.notify_all();
                cv4.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread2([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx);
                cv2.notify_all();
                cv1.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread3([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx);
                cv3.notify_all();
                cv2.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    std::thread thread4([&]()
        {
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx);
                cv4.notify_all();
                cv3.wait_for(lock, std::chrono::milliseconds(10));
            }
        });
    cv1.notify_all();
    cv2.notify_all();
    cv3.notify_all();
    cv4.notify_all();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    if (nViolationCnt >= 50) // 5% is allowed to fail...
        std::cout << "TIMING ConcurrencyTest - ConditionVarWaitFor_JointMutexForEachThread nViolationCnt < 50; currently " << nViolationCnt << std::endl;
    //EXPECT_LT(nViolationCnt, 50); // 5% is allowed to fail...
}

#if defined WIN32
TEST(ConcurrencyTest, ConditionVarWaitForPrediction_SeparateMutexForEachThread_IncreasedThreadPriority)
{
    std::condition_variable cv1, cv2, cv3, cv4;
    std::mutex mtx1, mtx2, mtx3, mtx4;
    std::atomic_size_t nCnt = 0;
    std::atomic_size_t nThreadCnt = 0;
    std::atomic_size_t nViolationCnt = 0;
    std::atomic_bool b1 = false, b2 = false, b3 = false, b4 = false;
    SMakeNoice noice;
    std::thread thread1([&]()
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt++ < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx4);
                b1 = true;
                cv1.notify_all();
                cv4.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b4; b4 = false; return b;});
            }
        });
    std::thread thread2([&]()
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx1);
                b2 = true;
                cv2.notify_all();
                cv1.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b1; b1 = false; return b;});
            }
        });
    std::thread thread3([&]()
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx2);
                b3 = true;
                cv3.notify_all();
                cv2.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b2; b2 = false; return b;});
            }
        });
    std::thread thread4([&]()
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
            nThreadCnt++;
            while (nThreadCnt != 4) {}  // Wait until all threads have started

            auto tpStart = std::chrono::high_resolution_clock::now();
            while (nCnt < 1000)
            {
                auto tpNow = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - tpStart).count() > 100)
                    nViolationCnt++;
                tpStart = tpNow;
                std::unique_lock<std::mutex> lock(mtx3);
                b4 = true;
                cv4.notify_all();
                cv3.wait_for(lock, std::chrono::milliseconds(10), [&]() {bool b = b3; b3 = false; return b;});
            }
        });
    cv1.notify_all();
    cv2.notify_all();
    cv3.notify_all();
    cv4.notify_all();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    if (nViolationCnt > 0) // 0% is allowed to fail...
        std::cout << "TIMING ConcurrencyTest - ConditionVarWaitForPrediction_SeparateMutexForEachThread_IncreasedThreadPriority nViolationCnt < 50; currently " << nViolationCnt << std::endl;
    //EXPECT_EQ(nViolationCnt, 0); // 0% is allowed to fail...
}
#endif // defined WIN32