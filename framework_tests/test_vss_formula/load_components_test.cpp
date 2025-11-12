#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <support/signal_support.h>
#include <interfaces/dispatch.h>
#include <support/app_control.h>
#include <signal_identifier.h>      
#include <vss_vehiclechassisbodyint32_vd_rx.h>   
#include <vss_vehiclechassisbodystring_vd_rx.h>
#include <vss_vehiclechassisbodyint32_bs_rx.h>   
#include <vss_vehiclechassisbodystring_bs_rx.h>    

#ifdef _WIN32
#include <windows.h>
#endif

bool InitializeAppControl(sdv::app::CAppControl* appControl, const std::string& configFileName)
{
    auto bResult = appControl->AddModuleSearchDir("../../bin");
    bResult &= appControl->Startup("");
    appControl->SetConfigMode();
    bResult &= appControl->AddConfigSearchDir("../../tests/bin/config");

    if (!configFileName.empty())
    {
        bResult &= appControl->LoadConfig(configFileName.c_str()) == sdv::core::EConfigProcessResult::successful;
    }

    return bResult;
}


TEST(VSSComponentTest, RXInt32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase5::dsSignalInt32);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxformulatypeint32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Body.Int32_Service").GetInterface<vss::Vehicle::Chassis::Body::Int32Service::IVSS_GetSignalTypeInt32>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const int32_t expected = 123;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected * 2, basicService->GetSignalTypeInt32());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXString)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase5::dsSignalString);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxformulatypestring.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Body.String_Service").GetInterface<vss::Vehicle::Chassis::Body::StringService::IVSS_GetSignalTypeString>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    std::string expected = "some text written";
    signalRx.Write(expected);
    expected.append("_string_added");
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeString());

    signalRx.Reset();
    appControl.Shutdown();
}

extern "C" int main(int argc, char* argv[])
{


    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
