#include <gtest/gtest.h>
#include <iostream>

#include <support/sdv_core.h>
#include <support/local_service_access.h>
#include <support/app_control.h>
#include "../../../global/process_watchdog.h"
#include "../../include/logger_test_helper.h"

#ifdef _MSC_VER
#include <process.h>
#elif defined __GNUC__
#include <unistd.h>
#endif


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

TEST(AppLoggerTest, CallbackMultiThread)
{
    std::string prefix = "PA_LoggerTest_";

    auto startCount = GetLoggerFilesCount(prefix);
    {
        SDV_LOG(sdv::core::ELogSeverity::warning, "Warning: trying to log before instantiation!");

        sdv::app::CAppControl appcontrol;
        std::stringstream sstreamAppConfig;
        sstreamAppConfig << "[LogHandler]" << std::endl << "Tag=\"" << prefix << getpid() << "\"";
        bool bResult = appcontrol.Startup(sstreamAppConfig.str());
        EXPECT_TRUE(bResult);
        appcontrol.SetConfigMode();

        SDV_LOG(sdv::core::ELogSeverity::warning, "Warning: trying to log before initialization!");

        std::cout << "Logger test: Initialize Log" << std::endl;

        sdv::core::EConfigProcessResult eResult = appcontrol.LoadConfig("test_logger_userconfig.toml");
        EXPECT_EQ(eResult, sdv::core::EConfigProcessResult::successful);
        appcontrol.SetRunningMode();
    }

    auto endCount = GetLoggerFilesCount(prefix);
#if defined(_WIN32)
    EXPECT_TRUE(endCount > startCount);
#else
    // just avoid unused variable warning
    std::cout << "start:" << std::to_string(startCount) << " end:" << std::to_string(endCount) << std::endl;
#endif
}
