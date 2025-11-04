#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include "../../../sdv_services/core/local_shutdown_request.h"
#include "../../../global/base64.h"

TEST(CoreLibrary_AppControl_MgntClass, Startup_Invalid_ApplicationMode)
{
    // Prevent error reporting on std::cerr - they will influence test outcome.
    auto* pCErr = std::cerr.rdbuf();
    std::ostringstream sstreamCErr;
    std::cerr.rdbuf(sstreamCErr.rdbuf());

    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Invalid\"");
    EXPECT_FALSE(bResult);
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    std::cerr.rdbuf(pCErr);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Default_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Standalone_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Standalone\"");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_External_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"External\"");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::external);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Isolated_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup(R"code(
[Application]
Mode = "Isolated"

[Console]
Report = "Silent"
)code");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Main_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Main\"");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Essential_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Essential\"");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::essential);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Maintenance_NoConfig)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Maintenance\"");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(control.GetInstanceID(), 1000u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Default_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Standalone_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Standalone\"\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_External_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"External\"\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::external);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Isolated_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2005
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = control.Startup(ssConfig);
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Main_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Essential_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Essential\"\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::essential);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, Startup_Maintenance_DefineInstance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Maintenance\"\nInstance=2005");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(control.GetInstanceID(), 2005u);

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Default)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Standalone)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Standalone\"\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_External)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"External\"\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::external);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Isolated)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2007
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = control.Startup(ssConfig);
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Main)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Essential)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Essential\"\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::essential);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    EXPECT_TRUE(control.RunLoop());
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_MgntClass, RunLoop_Maintenance)
{
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup("[Application]\nMode=\"Maintenance\"\nInstance=2007");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(control.GetInstanceID(), 2007u);

    // Loop not allowed...
    EXPECT_FALSE(control.RunLoop());

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

