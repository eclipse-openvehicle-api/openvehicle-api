#include "../../include/gtest_custom.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <interfaces/process.h>

#include "../../../sdv_services/process_control/process_control.cpp"
#include "process_control_ifc.h"
#include "../../../global/trace.h"

TEST(ProcessControlTest, Instantiate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessControl control;
    EXPECT_EQ(control.GetProcessID(), static_cast<sdv::process::TProcessID>(getpid()));

    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteProcessNormalShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    std::cout << GetTimestamp() << "Initiate execution..." << std::endl;
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);
    std::cout << GetTimestamp() << "Execution initiated. Waiting for finalization..." << std::endl;
    EXPECT_TRUE(control.WaitForTerminate(tProcessID, 5000));
    std::cout << GetTimestamp() << "Process execution finalized..." << std::endl;

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteProcessEmergencyExit)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    std::cout << GetTimestamp() << "Initiate execution..." << std::endl;
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);
    std::cout << GetTimestamp() << "Execution initiated. Waiting for finalization..." << std::endl;
    EXPECT_TRUE(control.WaitForTerminate(tProcessID, 5000));
    std::cout << GetTimestamp() << "Process execution finalized..." << std::endl;

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteProcessNormalShutdownWithMonitor)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    EXPECT_TRUE(monitor.Wait5000ms());
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteProcessEmergencyExitWithMonitor)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    EXPECT_TRUE(monitor.Wait5000ms());
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), -20);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteMultiProcessNormalShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    std::cout << GetTimestamp() << "Process#1 execution initiated PID#" << tProcessID1 << std::endl;
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::wait_for_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    std::cout << GetTimestamp() << "Process#2 execution initiated PID#" << tProcessID2 << std::endl;
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), 0);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    EXPECT_EQ(monitor2.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteMultiEmergencyAccess)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::wait_for_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), -20);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    // NOTE: The process #2 doesn't have the rights to get the exit code of process #1. Therefore it returns 0.
    //EXPECT_EQ(monitor2.GetRetValue(), -10);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteProcessAndTerminate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    bool bTerminateResult = false;
    std::thread thread([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            bTerminateResult = control.Terminate(tProcessID);
        });

    EXPECT_TRUE(monitor.Wait5000ms());
    thread.join();
    EXPECT_TRUE(bTerminateResult);
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), -100);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ParentRightsExecuteMultiTerminate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::terminate_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), -100);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    EXPECT_EQ(monitor2.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteProcessNormalShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    std::cout << GetTimestamp() << "Initiate execution..." << std::endl;
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID, 0u);
    std::cout << GetTimestamp() << "Execution initiated. Waiting for finalization..." << std::endl;
    EXPECT_TRUE(control.WaitForTerminate(tProcessID, 5000));
    std::cout << GetTimestamp() << "Process execution finalized..." << std::endl;

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteProcessEmergencyExit)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    std::cout << GetTimestamp() << "Initiate execution..." << std::endl;
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID, 0u);
    std::cout << GetTimestamp() << "Execution initiated. Waiting for finalization..." << std::endl;
    EXPECT_TRUE(control.WaitForTerminate(tProcessID, 5000));
    std::cout << GetTimestamp() << "Process execution finalized..." << std::endl;

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteProcessNormalShutdownWithMonitor)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    EXPECT_TRUE(monitor.Wait5000ms());
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteProcessEmergencyExitWithMonitor)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    EXPECT_TRUE(monitor.Wait5000ms());
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), -20);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteMultiProcessNormalShutdown)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::wait_for_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), 0);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    EXPECT_EQ(monitor2.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteMultiEmergencyAccess)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::wait_for_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), -20);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    // NOTE: The process #2 doesn't have the rights to get the exit code of process #1. Therefore it returns 0.
    //EXPECT_EQ(monitor2.GetRetValue(), -10);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteProcessAndTerminate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie = control.RegisterMonitor(tProcessID, &monitor);
    EXPECT_NE(uiCookie, 0u);

    bool bTerminateResult = false;
    std::thread thread([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            bTerminateResult = control.Terminate(tProcessID);
        });

    EXPECT_TRUE(monitor.Wait5000ms());
    thread.join();
    EXPECT_TRUE(bTerminateResult);
    EXPECT_EQ(monitor.GetProcessID(), tProcessID);
    EXPECT_EQ(monitor.GetRetValue(), -100);
    control.UnregisterMonitor(uiCookie);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ReducedRightsExecuteMultiTerminate)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::emergency_exit_1000ms)));
    sdv::process::TProcessID tProcessID1 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID1, 0u);

    seqArgs.clear();
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::terminate_process)));
    seqArgs.push_back(std::to_string(tProcessID1));
    sdv::process::TProcessID tProcessID2 = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::reduced_rights);
    EXPECT_NE(tProcessID2, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID1, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID2, &monitor2);
    EXPECT_NE(uiCookie2, 0u);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_TRUE(monitor2.Wait5000ms());
    EXPECT_EQ(monitor1.GetProcessID(), tProcessID1);
    EXPECT_EQ(monitor1.GetRetValue(), -100);
    EXPECT_EQ(monitor2.GetProcessID(), tProcessID2);
    EXPECT_EQ(monitor2.GetRetValue(), 0);
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie2);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}

TEST(ProcessControlTest, ExecuteProcessNormalShutdownWithMultipleMonitors)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"));

    CProcessMonitorHelper monitor1;
    CProcessMonitorHelper monitor2;
    CProcessMonitorHelper monitor3;

    CProcessControl control;
    control.Initialize("");             // Needed since local instantiation
#ifdef _WIN32
    const std::string ssModule = "UnitTest_ProcessControlApp.exe";
#else
    const std::string ssModule = "UnitTest_ProcessControlApp";
#endif
    sdv::sequence<sdv::u8string> seqArgs;
    seqArgs.push_back(std::to_string(static_cast<uint32_t>(EOperatingmode::normal_shutdown_1000ms)));
    sdv::process::TProcessID tProcessID = control.Execute(ssModule, seqArgs, sdv::process::EProcessRights::parent_rights);
    EXPECT_NE(tProcessID, 0u);

    uint32_t uiCookie1 = control.RegisterMonitor(tProcessID, &monitor1);
    EXPECT_NE(uiCookie1, 0u);
    uint32_t uiCookie2 = control.RegisterMonitor(tProcessID, &monitor2);
    EXPECT_NE(uiCookie2, 0u);
    uint32_t uiCookie3 = control.RegisterMonitor(tProcessID, &monitor3);
    EXPECT_NE(uiCookie3, 0u);

    control.UnregisterMonitor(uiCookie2);

    EXPECT_TRUE(monitor1.Wait5000ms());
    EXPECT_FALSE(monitor2.Wait5000ms());
    EXPECT_TRUE(monitor3.Wait5000ms());
    control.UnregisterMonitor(uiCookie1);
    control.UnregisterMonitor(uiCookie3);

    control.Shutdown();                 // Needed to prevent clash with core
    appcontrol.Shutdown();
}
