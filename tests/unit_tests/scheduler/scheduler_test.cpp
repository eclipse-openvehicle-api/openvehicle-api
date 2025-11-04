#include <fstream>
#include <gtest/gtest.h>
#include <chrono>
#include "../../../global/process_watchdog.h"
#include "../../../global/scheduler/scheduler.cpp"

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

TEST(TaskSchedulerTest, Construction)
{
    CTaskScheduler scheduler(2);
    EXPECT_EQ(scheduler.GetThreadCount(), 2);
    EXPECT_EQ(scheduler.GetBusyThreadCount(), 0);
    EXPECT_EQ(scheduler.GetIdleThreadCount(), 2);
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 2);

    bool bFinished = false;
    scheduler.Schedule([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            bFinished = true;
        });
    EXPECT_FALSE(bFinished);
    scheduler.WaitForExecution();
    EXPECT_TRUE(bFinished);
}

TEST(TaskSchedulerTest, PreallocatedConcurrencyExecution)
{
    CTaskScheduler scheduler(4);

    // Schedule tasks onto 4 threads
    std::atomic_size_t nExecuted = 0;
    bool bWait = true;
    for (size_t n = 0; n < 4; n++)
        scheduler.Schedule([&]()
            {
                while (bWait)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                nExecuted++;
            });
    bWait = false;

    // Wait until all threads are finalized
    scheduler.WaitForExecution();

    // Should be in parallel and threfore shorter to sequential.
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 4u);
    EXPECT_EQ(nExecuted, 4u);
}

TEST(TaskSchedulerTest, AllocateConcurrency)
{
    CTaskScheduler scheduler(2);

    // Schedule 4 tasks onto two threads pre-allocated and two just-in-time-allocated threads
    std::atomic_size_t nExecuted = 0;
    bool bWait = true;
    for (size_t n = 0; n < 4; n++)
        scheduler.Schedule([&]()
            {
                while (bWait)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                nExecuted++;
            });
    bWait = false;

    // Wait until all threads are finalized
    scheduler.WaitForExecution();

    // Four tasks should be in parallel at the time.
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 4u);
    EXPECT_EQ(nExecuted, 4u);
}

TEST(TaskSchedulerTest, ConcurrencyAndTaskQueueing)
{
    CTaskScheduler scheduler(2, 2);

    // Schedule 4 tasks onto two threads
    std::atomic_size_t nExecuted = 0;
    bool bWait = true;
    for (size_t n = 0; n < 4; n++)
        scheduler.Schedule([&]()
            {
                while (bWait)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                nExecuted++;
            });
    bWait = false;

    // Wait until all threads are finalized
    scheduler.WaitForExecution();

    // Two tasks should be in parallel at the time.
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 2u);
    EXPECT_EQ(nExecuted, 4u);
}

TEST(TaskSchedulerTest, ConcurrencyAndDisallowTaskQueueing)
{
    CTaskScheduler scheduler(2, 2);

    // Schedule 4 tasks onto two threads.. Disallow the second task to be queued and the fourth task to be queued.
    std::atomic_size_t nExecuted = 0;
    bool bWait = true;
    for (size_t n = 0; n < 4; n++)
        scheduler.Schedule([&]()
            {
                while (bWait)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                nExecuted++;
            }, (n % 2 ? CTaskScheduler::EScheduleFlags::no_queue: CTaskScheduler::EScheduleFlags::normal));
    bWait = false;

    // Wait until all threads are finalized
    scheduler.WaitForExecution();

    // Two tasks should be in parallel at the time. Three tasks were executed
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 2u);
    EXPECT_EQ(nExecuted, 3u);
}

TEST(TaskSchedulerTest, ConcurrencyAndPrioritizedTaskQueueing)
{
    CTaskScheduler scheduler(2, 2);

    // Schedule 5 tasks onto two threads.. Queue the fifth task with high priority.
    std::atomic_size_t nExecuted = 0;
    bool bWait = true;
    std::vector<size_t> vecOrder;
    std::mutex mtxOrder;
    for (size_t n = 0; n < 5; n++)
        scheduler.Schedule([&, n]()
            {
                // Synchronize start
                while (bWait)
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));

                // Add the task...
                std::unique_lock<std::mutex> lock(mtxOrder);
                vecOrder.push_back(n);
                lock.unlock();

                // Wait once more
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                nExecuted++;
            }, (n == 4 ? CTaskScheduler::EScheduleFlags::priority : CTaskScheduler::EScheduleFlags::normal));
    bWait = false;

    // Wait until all threads are finalized
    scheduler.WaitForExecution();

    std::cout << "vecOrder={";
    for (size_t nx : vecOrder)
        std::cout << nx << ", ";
    std::cout << "}" << std::endl;

    // Two tasks should be in parallel at the time. Five tasks were executed. The first two can come in any order. Then the fifth
    // task comes and the last two can also be in any order.
    EXPECT_EQ(scheduler.GetMaxThreadCount(), 2u);
    EXPECT_EQ(nExecuted, 5u);
    ASSERT_EQ(vecOrder.size(), 5u);
    EXPECT_TRUE(vecOrder[0] == 0 || vecOrder[1] == 0);
    EXPECT_TRUE(vecOrder[0] == 1 || vecOrder[1] == 1);
    EXPECT_EQ(vecOrder[2], 4u);
    EXPECT_TRUE(vecOrder[3] == 2 || vecOrder[4] == 2);
    EXPECT_TRUE(vecOrder[3] == 3 || vecOrder[4] == 3);
}
