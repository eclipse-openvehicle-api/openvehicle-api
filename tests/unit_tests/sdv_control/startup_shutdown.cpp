#include "includes.h"
#include "../../../sdv_executables/sdv_control/startup_shutdown.h"

/*
* The following tests are needed: (- not implemented; + implemented; o disabled)
* + Startup default instance
* + Startup specific instance
* + Shutdown default instance
* + Shutdown specific instance
* + Shutdown not-running default instance - fails
* + Shutdown not-running specific instance - fails
* o Recurring startup and shutdown default instance
* o Recurring startup and shutdown specific instance
* + Startup already running default instance - fails
* + Startup multiple specific services simultaneously
*/

TEST(SDV_Control_Test, StartupServerDefaultInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sContext.seqCmdLine = { "STARTUP" };
    int iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, 0);
}

TEST(SDV_Control_Test, StartupServerSpecificInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.bSilent = true;
    sContext.uiInstanceID = 1234;
    sContext.bServerSilent = true;
    sContext.seqCmdLine = { "STARTUP" };
    int iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, 0);
}

TEST(SDV_Control_Test, StartupShutdownServerDefaultInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sContext.seqCmdLine ={ "STARTUP" };
    int iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, 0);

    // Shutdown the server
    sContext.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext);
    EXPECT_EQ(iRet, 0);
}

TEST(SDV_Control_Test, StartupShutdownServerSpecificInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.uiInstanceID = 1234;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sContext.seqCmdLine = { "STARTUP"};
    int iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, 0);

    // Shutdown the server
    sContext.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext);
    EXPECT_EQ(iRet, 0);
}

TEST(SDV_Control_Test, TryShutdownNonexistantServerDefaultInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sContext.seqCmdLine = { "SHUTDOWN" };
    int iRet = ShutdownSDVServer(sContext);
    EXPECT_EQ(iRet, -124);
}

TEST(SDV_Control_Test, TryShutdownNonexistantServerSpecificInstance)
{
    CAppControlHelper appcontrol;

    // Startup the server
    SContext sContext;
    sContext.uiInstanceID = 1234;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sContext.seqCmdLine = { "SHUTDOWN" };
    int iRet = ShutdownSDVServer(sContext);
    EXPECT_EQ(iRet, -124);
}

// TODO: SDV_Control test has been disabled due to failing test issues during the build process. Bug report has been filed under:
// https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/608132
TEST(SDV_Control_Test, DISABLED_RecurringStartupShutdownServerDefaultInstance)
{
    CAppControlHelper appcontrol;

    SContext sContext;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sdv::process::TProcessID rgtServerID[3] = {};
    for (uint32_t ui = 0; ui < 3; ui++)
    {
        // Startup the server
        sContext.seqCmdLine = { "STARTUP" };
        int iRet = StartupSDVServer(sContext);
        if (iRet != 0)
            std::cout << "Failure on iteration: " << ui << std::endl;
        EXPECT_EQ(iRet, 0);
        rgtServerID[ui] = GetServerProcessID();

        // Shutdown the server
        sContext.seqCmdLine = { "SHUTDOWN" };
        iRet = ShutdownSDVServer(sContext);
        EXPECT_EQ(iRet, 0);

        // Wait three seconds before starting the next iteration (allow the server to shut down).
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

    // Just in case...
    for (uint32_t ui = 0; ui < 3; ui++)
        appcontrol.TerminateServerProcess(rgtServerID[ui]);
}

// TODO: SDV_Control test has been disabled due to failing test issues during the build process. Bug report has been filed under:
// https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/608132
TEST(SDV_Control_Test, DISABLED_RecurringStartupShutdownServerSpecificInstance)
{
    CAppControlHelper appcontrol;

    SContext sContext;
    sContext.uiInstanceID = 1234;
    sContext.bSilent = true;
    sContext.bServerSilent = true;
    sdv::process::TProcessID rgtServerID[3] = {};
    for (uint32_t ui = 0; ui < 3; ui++)
    {
        // Startup the server
        sContext.seqCmdLine = { "STARTUP" };
        int iRet = StartupSDVServer(sContext);
        EXPECT_EQ(iRet, 0);
        rgtServerID[ui] = GetServerProcessID();

        // Shutdown the server
        sContext.seqCmdLine = { "SHUTDOWN" };
        iRet = ShutdownSDVServer(sContext);
        EXPECT_EQ(iRet, 0);

        // Wait three seconds before starting the next iteration (allow the server to shut down).
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

    // Just in case...
    for (uint32_t ui = 0; ui < 3; ui++)
        appcontrol.TerminateServerProcess(rgtServerID[ui]);
}

TEST(SDV_Control_Test, TryStartupShutdownDuplicateServerDefaultInstance)
{
    CAppControlHelper appcontrol;

    SContext sContext;
    sContext.bSilent = true;
    sContext.bServerSilent = true;

    // Startup the server #0
    sContext.seqCmdLine ={ "STARTUP" };
    int iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, 0);
    sdv::process::TProcessID tServerID0 = GetServerProcessID();

    // Startup the server #1
    iRet = StartupSDVServer(sContext);
    EXPECT_EQ(iRet, -822);
    sdv::process::TProcessID tServerID1 = GetServerProcessID();

    // Shutdown the server #0
    sContext.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext);
    EXPECT_EQ(iRet, 0);

    // Just in case...
    appcontrol.TerminateServerProcess(tServerID0);
    appcontrol.TerminateServerProcess(tServerID1);
}
 
TEST(SDV_Control_Test, StartupShutdownMultipleServerInstances)
{
    CAppControlHelper appcontrol;

    SContext sContext0;
    sContext0.bSilent = true;
    sContext0.bServerSilent = true;
    SContext sContext1;
    sContext1.uiInstanceID = 1234;
    sContext1.bSilent = true;
    sContext1.bServerSilent = true;
    SContext sContext2;
    sContext2.uiInstanceID = 5678;
    sContext2.bSilent = true;
    sContext2.bServerSilent = true;

    // Startup the server #0
    sContext0.seqCmdLine = { "STARTUP" };
    int iRet = StartupSDVServer(sContext0);
    EXPECT_EQ(iRet, 0);
    sdv::process::TProcessID tServerID0 = GetServerProcessID();

    // Startup the server #1
    sContext1.seqCmdLine = { "STARTUP" };
    iRet = StartupSDVServer(sContext1);
    EXPECT_EQ(iRet, 0);
    sdv::process::TProcessID tServerID1 = GetServerProcessID();

    // Startup the server #2
    sContext2.seqCmdLine = { "STARTUP" };
    iRet = StartupSDVServer(sContext2);
    EXPECT_EQ(iRet, 0);
    sdv::process::TProcessID tServerID2 = GetServerProcessID();

    // Shutdown the server #0
    sContext0.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext0);
    EXPECT_EQ(iRet, 0);

    // Shutdown the server #1
    sContext1.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext1);
    EXPECT_EQ(iRet, 0);

    // Shutdown the server #2
    sContext2.seqCmdLine = { "SHUTDOWN" };
    iRet = ShutdownSDVServer(sContext2);
    EXPECT_EQ(iRet, 0);

    // Just in case...
    appcontrol.TerminateServerProcess(tServerID0);
    appcontrol.TerminateServerProcess(tServerID1);
    appcontrol.TerminateServerProcess(tServerID2);
}

