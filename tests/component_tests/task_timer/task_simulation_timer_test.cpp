#include "gtest/gtest.h"
#include <support/timer.h>
#include <chrono>
#include <thread>
#include <support/app_control.h>
#include "../../../global/process_watchdog.h"

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
        counter++;
    }

    uint32_t counter = 0;
};

TEST(TaskSimulationTimerTest, Initialization)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);
    appcontrol.SetRunningMode();

    sdv::TInterfaceAccessPtr ptrDispatchService = sdv::core::GetObject("SimulationTaskTimerService");
    sdv::core::ITaskTimer* pTimerSvc = ptrDispatchService.GetInterface<sdv::core::ITaskTimer>();
    EXPECT_NE(pTimerSvc, nullptr);

    appcontrol.Shutdown();
}

TEST(TaskSimulationTimerTest, BasicCounterTestInterface)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task;

    sdv::core::CTaskTimer timer(3, &task);
    EXPECT_TRUE(timer);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected = 5;
    uint32_t expectedAdditional = 11;
    uint32_t loop = expected;

    while (loop > 0) // 5 x 3 steps = 15 steps ==> 5 executions
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }
    EXPECT_EQ(expected, task.counter);

    loop = expected;
    while (loop > 0) // 5 * 7 steps = 35 steps ==> 11 executions, 2 rest
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }
    EXPECT_EQ(expected + expectedAdditional, task.counter);

    pTimerSimulationStep->SimulationStep(1000); // one more step, one more execution
    EXPECT_EQ(expected + expectedAdditional + 1, task.counter);

    timer.Reset();

    appcontrol.Shutdown();
}

TEST(TaskSimulationTimerTest, LargerTaskPeriodTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task150;
    CTestTask task1150;
    CTestTask task3100;

    sdv::core::CTaskTimer timer150(150, &task150);
    sdv::core::CTaskTimer timer1150(1150, &task1150);
    sdv::core::CTaskTimer timer3100(3100, &task3100);
    EXPECT_TRUE(timer150);
    EXPECT_TRUE(timer1150);
    EXPECT_TRUE(timer3100);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected150 = 33;
    uint32_t expected1150 = 4;
    uint32_t expected3100 = 1;
    uint32_t loop = 5000;
    while (loop > 0) // 5 x 3 steps = 15 steps ==> 5 executions
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
    }

    EXPECT_EQ(expected150, task150.counter);
    EXPECT_EQ(expected1150, task1150.counter);
    EXPECT_EQ(expected3100, task3100.counter);

    loop = 7000;
    while (loop > 0)
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
    }
    EXPECT_EQ(expected150 + 47, task150.counter);
    EXPECT_EQ(expected1150 + 6, task1150.counter);
    EXPECT_EQ(expected3100 + 2, task3100.counter);

    timer150.Reset();
    timer1150.Reset();
    timer3100.Reset();

    appcontrol.Shutdown();
}


TEST(TaskSimulationTimerTest, MultipleTimerIdenticalTaskPeriodTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task1;
    CTestTask task2;
    CTestTask task3;

    sdv::core::CTaskTimer timer1(3, &task1);
    sdv::core::CTaskTimer timer2(3, &task2);
    sdv::core::CTaskTimer timer3(3, &task3);
    EXPECT_TRUE(timer1);
    EXPECT_TRUE(timer2);
    EXPECT_TRUE(timer3);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected = 5;
    uint32_t expectedAdditional = 11;
    uint32_t loop = expected;

    while (loop > 0) // 5 x 3 steps = 15 steps ==> 5 executions
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }

    EXPECT_EQ(expected, task1.counter);
    EXPECT_EQ(expected, task2.counter);
    EXPECT_EQ(expected, task3.counter);

    loop = expected;
    while (loop > 0) // 5 * 7 steps = 35 steps ==> 11 executions, 2 rest
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }
    EXPECT_EQ(expected + expectedAdditional, task1.counter);
    EXPECT_EQ(expected + expectedAdditional, task2.counter);
    EXPECT_EQ(expected + expectedAdditional, task3.counter);

    pTimerSimulationStep->SimulationStep(1000); // one more step, one more execution
    EXPECT_EQ(expected + expectedAdditional + 1, task1.counter);
    EXPECT_EQ(expected + expectedAdditional + 1, task2.counter);
    EXPECT_EQ(expected + expectedAdditional + 1, task3.counter);

    timer1.Reset();
    timer2.Reset();
    timer3.Reset();

    appcontrol.Shutdown();
}

TEST(TaskSimulationTimerTest, MultipleTimerDifferentTaskPeriodTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task2;
    CTestTask task3;
    CTestTask task4;

    sdv::core::CTaskTimer timer2(2, &task2);
    sdv::core::CTaskTimer timer3(3, &task3);
    sdv::core::CTaskTimer timer4(4, &task4);
    EXPECT_TRUE(timer2);
    EXPECT_TRUE(timer3);
    EXPECT_TRUE(timer4);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected2 = 7;
    uint32_t expected3 = 5;
    uint32_t expected4 = 3;
    uint32_t expectedAdditional2 = 18;
    uint32_t expectedAdditional3 = 11;
    uint32_t expectedAdditional4 = 9;
    uint32_t loop = 5;

    while (loop > 0) // 5 x 3 steps = 15 steps ==> 5 executions
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }

    EXPECT_EQ(expected2, task2.counter);
    EXPECT_EQ(expected3, task3.counter);
    EXPECT_EQ(expected4, task4.counter);

    loop = 5;
    while (loop > 0)
    {
        loop--;
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
        pTimerSimulationStep->SimulationStep(1000);
    }
    EXPECT_EQ(expected2 + expectedAdditional2, task2.counter);
    EXPECT_EQ(expected3 + expectedAdditional3, task3.counter);
    EXPECT_EQ(expected4 + expectedAdditional4, task4.counter);

    pTimerSimulationStep->SimulationStep(1000);
    EXPECT_EQ(expected2 + expectedAdditional2 + 0, task2.counter);
    EXPECT_EQ(expected3 + expectedAdditional3 + 1, task3.counter);
    EXPECT_EQ(expected4 + expectedAdditional4 + 0, task4.counter);

    timer2.Reset();
    timer3.Reset();
    timer4.Reset();

    appcontrol.Shutdown();
}


TEST(TaskSimulationTimerTest, MultipleTimerLargeSimulationStepTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task1;
    CTestTask task150;
    CTestTask task450;

    sdv::core::CTaskTimer timer1(1, &task1);
    sdv::core::CTaskTimer timer150(150, &task150);
    sdv::core::CTaskTimer timer450(450, &task450);
    EXPECT_TRUE(timer1);
    EXPECT_TRUE(timer150);
    EXPECT_TRUE(timer450);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected1 = 480;
    uint32_t expected150 = 3;
    uint32_t expected450 = 1;

    pTimerSimulationStep->SimulationStep(80000);
    pTimerSimulationStep->SimulationStep(80000);
    pTimerSimulationStep->SimulationStep(80000);
    pTimerSimulationStep->SimulationStep(80000);
    pTimerSimulationStep->SimulationStep(80000);
    pTimerSimulationStep->SimulationStep(80000);

    EXPECT_EQ(expected1, task1.counter);
    EXPECT_EQ(expected150, task150.counter);
    EXPECT_EQ(expected450, task450.counter);

    timer1.Reset();
    timer150.Reset();
    timer450.Reset();

    appcontrol.Shutdown();
}

TEST(TaskSimulationTimerTest, MultipleTimerLargeAndDifferentSimulationStepTest)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.Startup("");
    EXPECT_TRUE(bResult);
    appcontrol.SetConfigMode();
    sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_simulation_tt_config.toml");
    EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);

    CTestTask task1;
    CTestTask task150;
    CTestTask task450;

    sdv::core::CTaskTimer timer1(1, &task1);
    sdv::core::CTaskTimer timer150(150, &task150);
    sdv::core::CTaskTimer timer450(450, &task450);
    EXPECT_TRUE(timer1);
    EXPECT_TRUE(timer150);
    EXPECT_TRUE(timer450);
    appcontrol.SetRunningMode();

    // Get the simulation task timer service.
    sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
    EXPECT_TRUE(pTimerSimulationStep);

    uint32_t expected1 = 480;
    uint32_t expected150 = 3;
    uint32_t expected450 = 1;

    pTimerSimulationStep->SimulationStep(67000);
    pTimerSimulationStep->SimulationStep(99000);
    pTimerSimulationStep->SimulationStep(72000);
    pTimerSimulationStep->SimulationStep(64000);
    pTimerSimulationStep->SimulationStep(85000);
    pTimerSimulationStep->SimulationStep(93000);

    EXPECT_EQ(expected1, task1.counter);
    EXPECT_EQ(expected150, task150.counter);
    EXPECT_EQ(expected450, task450.counter);

    timer1.Reset();
    timer150.Reset();
    timer450.Reset();

    appcontrol.Shutdown();
}