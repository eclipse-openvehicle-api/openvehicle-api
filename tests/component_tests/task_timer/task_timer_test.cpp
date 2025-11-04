#include "gtest/gtest.h"
#include <support/timer.h>
#include <chrono>
#include <thread>
#include <support/app_control.h>
#include "../../../global/process_watchdog.h"

static uint32_t TimeTolerance = 50; //tolerate a small delay for task execution

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

//test object which counts how many times the cb has been called
class CTestTask : public sdv::core::ITaskExecute, public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::core::ITaskExecute)
    END_SDV_INTERFACE_MAP()

    std::atomic_bool m_bEnabled = true;
    void Enable(bool bEnable) { m_bEnabled = bEnable; }

    virtual void Execute() override
    {
        if (!m_bEnabled) return;
        std::unique_lock<std::mutex> lock(mtxTimes);
        vecTimes.push_back(std::chrono::high_resolution_clock::now());

        counter++;
    }

    double CalcAvrgTime()
    {
        if (vecTimes.empty()) return 0.0;
        double dTotal = 0.0;
        for (size_t n = 1; n < vecTimes.size(); n++)
            dTotal += std::chrono::duration<double>(vecTimes[n] - vecTimes[n - 1]).count();
        return dTotal / static_cast<double>(vecTimes.size() - 1);
    }

    std::atomic<uint64_t> counter = 0ull;
    std::mutex mtxTimes;
    std::vector<std::chrono::high_resolution_clock::time_point> vecTimes;
};

TEST(TaskTimerTest, Initialization)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);
    appcontrol.SetRunningMode();

    sdv::TInterfaceAccessPtr ptrDispatchService = sdv::core::GetObject("TaskTimerService");
    sdv::core::ITaskTimer* pTimerSvc = ptrDispatchService.GetInterface<sdv::core::ITaskTimer>();
    EXPECT_NE(pTimerSvc, nullptr);

    appcontrol.Shutdown();
}

TEST(TaskTimerTest, BasicCounterTestInterface)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task;

    sdv::core::CTaskTimer timer(100, &task);
    EXPECT_TRUE(timer);
    appcontrol.SetRunningMode();

    std::chrono::milliseconds sleepDuration (500 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    timer.Reset();

    EXPECT_EQ(5ull, task.counter);

    sleepDuration = std::chrono::milliseconds(20 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    EXPECT_EQ(5ull, task.counter);

    appcontrol.Shutdown();
}

TEST(TaskTimerTest, BasicCounterTestFunction)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task;

    sdv::core::CTaskTimer timer(100, [&]() {task.Execute(); });
    EXPECT_TRUE(timer);
    appcontrol.SetRunningMode();

    std::chrono::milliseconds sleepDuration (500 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    timer.Reset();

    EXPECT_EQ(5ull, task.counter);

    sleepDuration = std::chrono::milliseconds(20 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    EXPECT_EQ(5ull, task.counter);

    // 1ms buffer
    if (0.101 <= task.CalcAvrgTime())
        std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
        "Expected: (0.101) > (task.CalcAvrgTime()), actual: " <<
        (0.101) << "vs" << task.CalcAvrgTime() << std::endl;
    //EXPECT_GT(0.101, task.CalcAvrgTime());
    if (0.099 >= task.CalcAvrgTime())
        std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
        "Expected: (0.099) < (task.CalcAvrgTime()), actual: " <<
        (0.099) << "vs" << task.CalcAvrgTime() << std::endl;
    //EXPECT_LT(0.099, task.CalcAvrgTime());

    appcontrol.Shutdown();
}

TEST(TaskTimerTest, LongPeriodTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task;

    sdv::core::CTaskTimer timer(1500, &task);
    EXPECT_TRUE(timer);
    appcontrol.SetRunningMode();

    std::chrono::milliseconds sleepDuration (3000 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    timer.Reset();

    EXPECT_EQ(2ull, task.counter);

    // 1ms buffer
    if (1.501 <= task.CalcAvrgTime())
        std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
        "Expected: (1.501) > (task.CalcAvrgTime()), actual: " <<
        (1.501) << "vs" << task.CalcAvrgTime() << std::endl;
    //EXPECT_GT(1.501, task.CalcAvrgTime());
    if (1.499 >= task.CalcAvrgTime())
        std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
        "Expected: (1.499) < (task.CalcAvrgTime()), actual: " <<
        (1.499) << "vs" << task.CalcAvrgTime() << std::endl;
    //EXPECT_LT(1.499, task.CalcAvrgTime());

    appcontrol.Shutdown();
}

TEST(TaskTimerTest, SpawnTaskTwice)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task;

    sdv::core::CTaskTimer timer1(100, &task);
    sdv::core::CTaskTimer timer2(100, &task);
    EXPECT_TRUE(timer1);
    EXPECT_TRUE(timer2);
    appcontrol.SetRunningMode();

    std::chrono::milliseconds sleepDuration (500 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    //shutdown both tasks here
    timer1.Reset();
    timer2.Reset();

    //check for counter - expectation: twice the count
    EXPECT_EQ(10ull, task.counter);

    appcontrol.Shutdown();
}

TEST(TaskTimerTest, MultiTaskTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    uint32_t rguiPeriods[] = { 100, 133, 150, 250, 333, 50, 73, 120, 10, 415, 115, 60, 5/*, 1*/ };
	const size_t nExtent = std::extent_v<decltype(rguiPeriods)>;
    CTestTask rgtask[nExtent];
    sdv::core::CTaskTimer rgtimer[nExtent];
    for (size_t n = 0; n < nExtent; n++)
    {
        rgtask[n].Enable(false);
        rgtimer[n] = sdv::core::CTaskTimer(rguiPeriods[n], rgtask + n);
        EXPECT_TRUE(rgtimer[n]);
    }
    appcontrol.SetRunningMode();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    for (size_t n = 0; n < std::extent_v<decltype(rguiPeriods)>; n++)
    {
        rgtask[n].Enable(true);
    }

    std::chrono::milliseconds sleepDuration(1500 + TimeTolerance);
    std::this_thread::sleep_for(sleepDuration);

    // NOTE: If running in a virtual environment, the constraints cannot be kept.
    for (size_t n = 0; n < nExtent; n++)
    {
        rgtimer[n].Reset();
        EXPECT_FALSE(rgtimer[n]);
        if (rgtask[n].counter < 1500 / rguiPeriods[n])
            std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
            "Expected: (rgtask[n].counter) >= (1500 / (rguiPeriods[n]), actual: " << rgtask[n].counter << "vs" << (1500 / rguiPeriods[n]) << std::endl;
        //EXPECT_GE(rgtask[n].counter, 1500 / rguiPeriods[n]);

        // 1ms buffer
        if ((static_cast<double>(rguiPeriods[n]) / 1000.0 + 0.001) <= rgtask[n].CalcAvrgTime())
            std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
            "Expected: (static_cast<double>(rguiPeriods[n]) / 1000.0 + 0.001) > (rgtask[n].CalcAvrgTime()), actual: " <<
            (static_cast<double>(rguiPeriods[n]) / 1000.0 + 0.001) << "vs" << rgtask[n].CalcAvrgTime() << std::endl;
        //EXPECT_GT(static_cast<double>(rguiPeriods[n]) / 1000.0 + 0.001, rgtask[n].CalcAvrgTime());
        if ((static_cast<double>(rguiPeriods[n]) / 1000.0 - 0.001) >= rgtask[n].CalcAvrgTime())
            std::cout << __FILE__ << ":" << __LINE__ << ":" << "Warning" << std::endl <<
            "Expected: (static_cast<double>(rguiPeriods[n]) / 1000.0 - 0.001) < (rgtask[n].CalcAvrgTime()), actual: " <<
            (static_cast<double>(rguiPeriods[n]) / 1000.0 + 0.001) << "vs" << rgtask[n].CalcAvrgTime() << std::endl;
        //EXPECT_LT(static_cast<double>(rguiPeriods[n]) / 1000.0 - 0.001, rgtask[n].CalcAvrgTime());
    }

    appcontrol.Shutdown();
}
