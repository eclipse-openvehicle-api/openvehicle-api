#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include "../../../sdv_services/core/local_shutdown_request.h"
#include "../../../global/base64.h"

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Invalid_ApplicationMode)
{
    // Prevent error reporting on std::cerr - they will influence test outcome.
    auto* pCErr = std::cerr.rdbuf();
    std::ostringstream sstreamCErr;
    std::cerr.rdbuf(sstreamCErr.rdbuf());

    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Invalid\"", nullptr);
    EXPECT_FALSE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    std::cerr.rdbuf(pCErr);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Default_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Standalone_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_External_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Isolated_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup(R"code(
[Application]
Mode = "Isolated"

[Console]
Report = "Silent"
)code", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Main_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Essential_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Maintenance_NoConfig)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Default_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Standalone_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_External_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Isolated_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2005
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = pControl->Startup(ssConfig, nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Main_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Essential_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, Startup_Maintenance_DefineInstance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"\nInstance=2005", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Default)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Standalone)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_External)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Isolated)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2007
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = pControl->Startup(ssConfig, nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Main)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Essential)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_NoEventHandler, RunLoop_Maintenance)
{
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"\nInstance=2007", nullptr);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    // Loop not allowed...
    EXPECT_THROW(pControl->RunLoop(), sdv::XAccessDenied);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

