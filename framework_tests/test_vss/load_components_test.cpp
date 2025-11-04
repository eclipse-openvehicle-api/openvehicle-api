#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <support/signal_support.h>
#include <interfaces/dispatch.h>
#include <support/app_control.h>
#include <signal_identifier.h>
#include <vss_vehiclechassisvehicleboolean_bs_rx.h>
#include <vss_vehiclechassisvehicleboolean_bs_tx.h>
#include <vss_vehiclechassisvehicleboolean_vd_rx.h>
#include <vss_vehiclechassisvehicleboolean_vd_tx.h>
#include <vss_vehiclechassisvehiclefloat_bs_rx.h>
#include <vss_vehiclechassisvehiclefloat_bs_tx.h>
#include <vss_vehiclechassisvehiclefloat_vd_rx.h>
#include <vss_vehiclechassisvehiclefloat_vd_tx.h>
#include <vss_vehiclechassisvehicledouble_bs_rx.h>
#include <vss_vehiclechassisvehicledouble_bs_tx.h>
#include <vss_vehiclechassisvehicledouble_vd_rx.h>
#include <vss_vehiclechassisvehicledouble_vd_tx.h>
#include <vss_vehiclechassisvehiclelongdouble_vd_rx.h>
#include <vss_vehiclechassisvehiclelongdouble_bs_tx.h>
#include <vss_vehiclechassisvehiclelongdouble_bs_rx.h>
#include <vss_vehiclechassisvehiclelongdouble_vd_tx.h>
#include <vss_vehiclechassisvehiclechar16_bs_rx.h>
#include <vss_vehiclechassisvehiclechar16_bs_tx.h>
#include <vss_vehiclechassisvehiclechar16_vd_rx.h>
#include <vss_vehiclechassisvehiclechar16_vd_tx.h>
#include <vss_vehiclechassisvehiclechar32_bs_rx.h>
#include <vss_vehiclechassisvehiclechar32_bs_tx.h>
#include <vss_vehiclechassisvehiclechar32_vd_rx.h>
#include <vss_vehiclechassisvehiclechar32_vd_tx.h>
#include <vss_vehiclechassisvehiclechar_bs_rx.h>
#include <vss_vehiclechassisvehiclechar_bs_tx.h>
#include <vss_vehiclechassisvehiclechar_vd_rx.h>
#include <vss_vehiclechassisvehiclechar_vd_tx.h>
#include <vss_vehiclechassisvehicledouble_bs_rx.h>
#include <vss_vehiclechassisvehicledouble_bs_tx.h>
#include <vss_vehiclechassisvehicledouble_vd_rx.h>
#include <vss_vehiclechassisvehicledouble_vd_tx.h>
#include <vss_vehiclechassisvehicleint16_bs_rx.h>
#include <vss_vehiclechassisvehicleint16_bs_tx.h>
#include <vss_vehiclechassisvehicleint16_vd_rx.h>
#include <vss_vehiclechassisvehicleint16_vd_tx.h>
#include <vss_vehiclechassisvehicleint32_bs_rx.h>
#include <vss_vehiclechassisvehicleint32_bs_tx.h>
#include <vss_vehiclechassisvehicleint32_vd_rx.h>
#include <vss_vehiclechassisvehicleint32_vd_tx.h>
#include <vss_vehiclechassisvehicleint64_bs_rx.h>
#include <vss_vehiclechassisvehicleint64_bs_tx.h>
#include <vss_vehiclechassisvehicleint64_vd_rx.h>
#include <vss_vehiclechassisvehicleint64_vd_tx.h>
#include <vss_vehiclechassisvehicleint8_bs_rx.h>
#include <vss_vehiclechassisvehicleint8_bs_tx.h>
#include <vss_vehiclechassisvehicleint8_vd_rx.h>
#include <vss_vehiclechassisvehicleint8_vd_tx.h>
#include <vss_vehiclechassisvehiclelonglong_bs_rx.h>
#include <vss_vehiclechassisvehiclelonglong_bs_tx.h>
#include <vss_vehiclechassisvehiclelonglong_vd_rx.h>
#include <vss_vehiclechassisvehiclelonglong_vd_tx.h>
#include <vss_vehiclechassisvehiclelong_bs_rx.h>
#include <vss_vehiclechassisvehiclelong_bs_tx.h>
#include <vss_vehiclechassisvehiclelong_vd_rx.h>
#include <vss_vehiclechassisvehiclelong_vd_tx.h>
#include <vss_vehiclechassisvehiclenative_bs_rx.h>
#include <vss_vehiclechassisvehiclenative_bs_tx.h>
#include <vss_vehiclechassisvehiclenative_vd_rx.h>
#include <vss_vehiclechassisvehiclenative_vd_tx.h>
#include <vss_vehiclechassisvehicleshort_bs_rx.h>
#include <vss_vehiclechassisvehicleshort_bs_tx.h>
#include <vss_vehiclechassisvehicleshort_vd_rx.h>
#include <vss_vehiclechassisvehicleshort_vd_tx.h>
#include <vss_vehiclechassisvehiclestring_bs_rx.h>
#include <vss_vehiclechassisvehiclestring_bs_tx.h>
#include <vss_vehiclechassisvehiclestring_vd_rx.h>
#include <vss_vehiclechassisvehiclestring_vd_tx.h>
#include <vss_vehiclechassisvehicleu16string_bs_rx.h>
#include <vss_vehiclechassisvehicleu16string_bs_tx.h>
#include <vss_vehiclechassisvehicleu16string_vd_rx.h>
#include <vss_vehiclechassisvehicleu16string_vd_tx.h>
#include <vss_vehiclechassisvehicleu32string_bs_rx.h>
#include <vss_vehiclechassisvehicleu32string_bs_tx.h>
#include <vss_vehiclechassisvehicleu32string_vd_rx.h>
#include <vss_vehiclechassisvehicleu32string_vd_tx.h>
#include <vss_vehiclechassisvehicleu8string_bs_rx.h>
#include <vss_vehiclechassisvehicleu8string_bs_tx.h>
#include <vss_vehiclechassisvehicleu8string_vd_rx.h>
#include <vss_vehiclechassisvehicleu8string_vd_tx.h>
#include <vss_vehiclechassisvehicleuint16_bs_rx.h>
#include <vss_vehiclechassisvehicleuint16_bs_tx.h>
#include <vss_vehiclechassisvehicleuint16_vd_rx.h>
#include <vss_vehiclechassisvehicleuint16_vd_tx.h>
#include <vss_vehiclechassisvehicleuint32_bs_rx.h>
#include <vss_vehiclechassisvehicleuint32_bs_tx.h>
#include <vss_vehiclechassisvehicleuint32_vd_rx.h>
#include <vss_vehiclechassisvehicleuint32_vd_tx.h>
#include <vss_vehiclechassisvehicleuint64_bs_rx.h>
#include <vss_vehiclechassisvehicleuint64_bs_tx.h>
#include <vss_vehiclechassisvehicleuint64_vd_rx.h>
#include <vss_vehiclechassisvehicleuint64_vd_tx.h>
#include <vss_vehiclechassisvehicleuint8_bs_rx.h>
#include <vss_vehiclechassisvehicleuint8_bs_tx.h>
#include <vss_vehiclechassisvehicleuint8_vd_rx.h>
#include <vss_vehiclechassisvehicleuint8_vd_tx.h>
#include <vss_vehiclechassisvehicleunsignedlonglong_bs_rx.h>
#include <vss_vehiclechassisvehicleunsignedlonglong_bs_tx.h>
#include <vss_vehiclechassisvehicleunsignedlonglong_vd_rx.h>
#include <vss_vehiclechassisvehicleunsignedlonglong_vd_tx.h>
#include <vss_vehiclechassisvehicleunsignedlong_bs_rx.h>
#include <vss_vehiclechassisvehicleunsignedlong_bs_tx.h>
#include <vss_vehiclechassisvehicleunsignedlong_vd_rx.h>
#include <vss_vehiclechassisvehicleunsignedlong_vd_tx.h>
#include <vss_vehiclechassisvehicleunsignedshort_bs_rx.h>
#include <vss_vehiclechassisvehicleunsignedshort_bs_tx.h>
#include <vss_vehiclechassisvehicleunsignedshort_vd_rx.h>
#include <vss_vehiclechassisvehicleunsignedshort_vd_tx.h>
#include <vss_vehiclechassisvehiclewchar_bs_rx.h>
#include <vss_vehiclechassisvehiclewchar_bs_tx.h>
#include <vss_vehiclechassisvehiclewchar_vd_rx.h>
#include <vss_vehiclechassisvehiclewchar_vd_tx.h>
#include <vss_vehiclechassisvehiclewstring_bs_rx.h>
#include <vss_vehiclechassisvehiclewstring_bs_tx.h>
#include <vss_vehiclechassisvehiclewstring_vd_rx.h>
#include <vss_vehiclechassisvehiclewstring_vd_tx.h>      

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

//
//  Test RX Signals
//

TEST(VSSComponentTest, RXBoolean)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalBoolean);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeboolean.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Boolean_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::BooleanService::IVSS_GetSignalTypeBoolean>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const bool expected = true;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeBoolean());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXFloat)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalFloat);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypefloat.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Float_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::FloatService::IVSS_GetSignalTypeFloat>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const float expected = 3.14f;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeFloat());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXDouble)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalDouble);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypedouble.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Double_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::DoubleService::IVSS_GetSignalTypeDouble>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const double expected = 123456.123456;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeDouble());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXLongDouble)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalLongDouble);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypelongdouble.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.LongDouble_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongDoubleService::IVSS_GetSignalTypeLongDouble>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const double expected = 654321.123456;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeLongDouble());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXInt8)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalInt8);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeint8.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int8_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int8Service::IVSS_GetSignalTypeInt8>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const char expected = 'c';
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeInt8());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXInt16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalInt16);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeint16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int16Service::IVSS_GetSignalTypeInt16>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const int16_t expected = 32766;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeInt16());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXInt32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalInt32);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeint32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int32Service::IVSS_GetSignalTypeInt32>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const int32_t expected = 2147483646;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeInt32());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXInt64)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalInt64);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeint64.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int64_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int64Service::IVSS_GetSignalTypeInt64>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const int64_t expected = 9223372036854775806;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeInt64());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalLong);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypelong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Long_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongService::IVSS_GetSignalTypeLong>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const long expected = 2147483646;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeLong());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXLongLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalLongLong);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypelonglong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.LongLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongLongService::IVSS_GetSignalTypeLongLong>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const long long expected = 9223372036854775806LL;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeLongLong());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXShort)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalShort);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeshort.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Short_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::ShortService::IVSS_GetSignalTypeShort>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const short expected = 127;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeShort());

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
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalString);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypestring.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::StringService::IVSS_GetSignalTypeString>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const std::string expected = "some text written";
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeString());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXWString)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalWString);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypewstring.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.WString_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::WStringService::IVSS_GetSignalTypeWString>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const std::wstring expected = L"some text written";
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeWString());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUint8)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUint8);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeuint8.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint8_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint8Service::IVSS_GetSignalTypeUint8>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const uint8_t expected = 254;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUint8());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUint16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUint16);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeuint16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint16Service::IVSS_GetSignalTypeUint16>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const uint16_t expected = 65534;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUint16());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUint32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUint32);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeuint32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint32Service::IVSS_GetSignalTypeUint32>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const uint32_t expected = 4294967294U;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUint32());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUint64)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUint64);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeuint64.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint64_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint64Service::IVSS_GetSignalTypeUint64>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const uint64_t expected = 18446744073709551614ULL;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUint64());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUnsignedLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUnsignedLong);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeunsignedlong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedLongService::IVSS_GetSignalTypeUnsignedLong>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const unsigned long expected = 4294967295;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUnsignedLong());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUnsignedlonglong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUnsignedLongLong);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeunsignedlonglong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedLongLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedLongLongService::IVSS_GetSignalTypeUnsignedLongLong>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const unsigned long long expected = 18446744073709551613ULL;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUnsignedLongLong());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXUnsignedShort)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalUnsignedShort);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeunsignedshort.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedShort_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedShortService::IVSS_GetSignalTypeUnsignedShort>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const unsigned short expected = 255;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeUnsignedShort());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXChar)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalChar);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypechar.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::CharService::IVSS_GetSignalTypeChar>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const char expected = 'C';
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeChar());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXChar16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalChar16);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypechar16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Char16Service::IVSS_GetSignalTypeChar16>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const char32_t expected = u'\uFFFF';
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeChar16());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXChar32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalChar32);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypechar32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Char32Service::IVSS_GetSignalTypeChar32>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const char32_t expected = U'\U0010FFFF';
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeChar32());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXWChar)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalWChar);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypewchar.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.WChar_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::WCharService::IVSS_GetSignalTypeWChar>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const wchar_t expected = L'\xFFEE';
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeWChar());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXNative)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalNative);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypenative.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Native_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::NativeService::IVSS_GetSignalTypeNative>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const size_t expected = 290;
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeNative());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXU8String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalU8String);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeu8string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U8String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U8StringService::IVSS_GetSignalTypeU8String>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const sdv::u8string expected = "some text written";
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeU8String());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXU16String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalU16String);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeu16string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U16String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U16StringService::IVSS_GetSignalTypeU16String>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const sdv::u16string expected = u"some text written";
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeU16String());

    signalRx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, RXU32String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalRx;
    signalRx = dispatch.RegisterRxSignal(testcase4::dsSignalU32String);
    ASSERT_TRUE(signalRx);

    bResult &= appControl.LoadConfig("rxtypeu32string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U32String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U32StringService::IVSS_GetSignalTypeU32String>();
    ASSERT_TRUE(basicService);

    appControl.SetRunningMode();
    const sdv::u32string expected = U"some text written";
    signalRx.Write(expected);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ASSERT_EQ(expected, basicService->GetSignalTypeU32String());

    signalRx.Reset();
    appControl.Shutdown();
}

////
////  Test TX Signals
////

TEST(VSSComponentTest, TXBoolean)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const bool initVal = false;
    const bool expected = !initVal;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalBoolean_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeboolean.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Boolean_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::BooleanService::IVSS_SetSignalTypeBoolean>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeBoolean(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<bool>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<bool>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXFloat)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const float initVal = 3.14f;
    const float expected = initVal * 3.14f;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalFloat_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypefloat.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Float_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::FloatService::IVSS_SetSignalTypeFloat>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeFloat(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<float>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<float>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXDouble)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const double initVal = 123456.123456;
    const double expected = initVal / 3;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalDouble_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypedouble.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Double_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::DoubleService::IVSS_SetSignalTypeDouble>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeDouble(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<double>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<double>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXLongDouble)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const long double initVal = 654321.123456;
    const long double expected = initVal / 3;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalLongDouble_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypelongdouble.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.LongDouble_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongDoubleService::IVSS_SetSignalTypeLongDouble>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeLongDouble(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<long double>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<long double>());

    signalTx.Reset();
    appControl.Shutdown();
}


TEST(VSSComponentTest, TXInt8)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    char initVal = 'c';
    char expected = 'x';
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalInt8_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeint8.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int8_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int8Service::IVSS_SetSignalTypeInt8>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeInt8(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<char>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<char>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXInt16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const int16_t initVal = 32766;
    const int16_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalInt16_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeint16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int16Service::IVSS_SetSignalTypeInt16>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeInt16(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<int16_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<int16_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXInt32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const int32_t initVal = 2147483646;
    const int32_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalInt32_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeint32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int32Service::IVSS_SetSignalTypeInt32>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeInt32(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<int32_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<int32_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXInt64)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const int64_t initVal = 9223372036854775806;
    const int64_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalInt64_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeint64.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Int64_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Int64Service::IVSS_SetSignalTypeInt64>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeInt64(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<int64_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<int64_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const long initVal = 2147483646;
    const long expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalLong_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypelong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Long_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongService::IVSS_SetSignalTypeLong>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeLong(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<long>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<long>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXLongLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const long long initVal = 9223372036854775806LL;
    const long long expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalLongLong_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypelonglong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.LongLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::LongLongService::IVSS_SetSignalTypeLongLong>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeLongLong(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<long long>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<long long>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXShort)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const int16_t initVal = 127;
    const int16_t expected = initVal - 100;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalShort_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeshort.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Short_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::ShortService::IVSS_SetSignalTypeShort>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeShort(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<int16_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<int16_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXString)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const sdv::string initVal = "some initialization string";
    const sdv::string expected = "changed string";
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalString_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypestring.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::StringService::IVSS_SetSignalTypeString>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeString(expected);
    std::cout << "Init: " << initVal << " -> " << signalTx.Read().get<sdv::string>() << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<sdv::string>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXWString)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const sdv::wstring initVal = L"some initialization string";
    const sdv::wstring expected = L"changed string";
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalWString_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypewstring.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.WString_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::WStringService::IVSS_SetSignalTypeWString>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeWString(expected);
    std::wcout << L"Init: " << initVal << L" -> " << signalTx.Read().get<sdv::wstring>() << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<sdv::wstring>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUint8)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const uint8_t initVal = 255;
    const uint8_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUint8_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeuint8.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint8_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint8Service::IVSS_SetSignalTypeUint8>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUint8(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<int16_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<int16_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUint16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const uint16_t initVal = 65534;
    const uint16_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUint16_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeuint16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint16Service::IVSS_SetSignalTypeUint16>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUint16(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<uint16_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<uint16_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUint32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const uint32_t initVal = 4294967294U;
    const uint32_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUint32_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeuint32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint32Service::IVSS_SetSignalTypeUint32>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUint32(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<uint32_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<uint32_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUint64)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const uint64_t initVal = 18446744073709551614ULL;
    const uint64_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUint64_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeuint64.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Uint64_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Uint64Service::IVSS_SetSignalTypeUint64>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUint64(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<uint64_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<uint64_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUnsignedLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const unsigned long initVal = 4294967295;
    const unsigned long expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUnsignedLong_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeunsignedlong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedLongService::IVSS_SetSignalTypeUnsignedLong>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUnsignedLong(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<unsigned long>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<unsigned long>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUnsignedLongLong)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const unsigned long long initVal = 18446744073709551613ULL;
    const unsigned long long expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUnsignedLongLong_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeunsignedlonglong.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedLongLong_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedLongLongService::IVSS_SetSignalTypeUnsignedLongLong>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUnsignedLongLong(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<unsigned long long>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<unsigned long long>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXUnsignedShort)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const unsigned short initVal = 255;
    const unsigned short expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalUnsignedShort_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeunsignedshort.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.UnsignedShort_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::UnsignedShortService::IVSS_SetSignalTypeUnsignedShort>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeUnsignedShort(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<unsigned short>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<unsigned short>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXChar)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const char initVal = 'C';
    const char  expected = 'X';
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalChar_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypechar.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::CharService::IVSS_SetSignalTypeChar>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeChar(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<char>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<char>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXChar16)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const char16_t initVal = u'\uFFFF';
    const char16_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalChar16_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypechar16.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char16_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Char16Service::IVSS_SetSignalTypeChar16>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeChar16(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<char16_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<char16_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXChar32)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const char32_t initVal = U'\U0010FFFF';
    const char32_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalChar32_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypechar32.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Char32_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::Char32Service::IVSS_SetSignalTypeChar32>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeChar32(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<char32_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<char32_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXWChar)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const wchar_t initVal = L'\xFFEE';
    const wchar_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalWChar_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypewchar.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.WChar_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::WCharService::IVSS_SetSignalTypeWChar>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeWChar(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<wchar_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<wchar_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXNative)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const size_t initVal = 314;
    const size_t expected = initVal / 2;
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalNative_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypenative.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.Native_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::NativeService::IVSS_SetSignalTypeNative>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeNative(expected);
    std::cout << "Init: " << std::to_string(initVal) << " -> " << std::to_string(signalTx.Read().get<size_t>()) << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<size_t>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXU8String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const sdv::u8string initVal = "some initialize string";
    const sdv::u8string expected = "changed string";
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalU8String_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeu8string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U8String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U8StringService::IVSS_SetSignalTypeU8String>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeU8String(expected);
    std::cout << "Init: " << initVal << " -> " << signalTx.Read().get<sdv::u8string>() << std::endl;
    ASSERT_EQ(expected, signalTx.Read().get<sdv::u8string>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXU16String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const sdv::u16string initVal = u"some initialize string";
    const sdv::u16string expected = u"changed string";
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalU16String_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeu16string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U16String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U16StringService::IVSS_SetSignalTypeU16String>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeU16String(expected);
    ASSERT_EQ(expected, signalTx.Read().get<sdv::u16string>());

    signalTx.Reset();
    appControl.Shutdown();
}

TEST(VSSComponentTest, TXU32String)
{
    sdv::app::CAppControl appControl;

    auto bResult = InitializeAppControl(&appControl, "data_dispatch_service.toml");
    ASSERT_EQ(bResult, true);

    const sdv::u32string initVal = U"some initialize string";
    const sdv::u32string expected = U"changed string";
    sdv::core::CDispatchService dispatch;
    sdv::core::CSignal signalTx;
    signalTx = dispatch.RegisterTxSignal(testcase4::dsSignalU32String_1, initVal);
    ASSERT_TRUE(signalTx);

    bResult &= appControl.LoadConfig("txtypeu32string.toml") == sdv::core::EConfigProcessResult::successful;
    ASSERT_EQ(bResult, true);

    auto basicService = sdv::core::GetObject("Vehicle.Chassis.Vehicle.U32String_Service").GetInterface<vss::Vehicle::Chassis::Vehicle::U32StringService::IVSS_SetSignalTypeU32String>();
    ASSERT_TRUE(basicService);
    appControl.SetRunningMode();
    basicService->SetSignalTypeU32String(expected);
    ASSERT_EQ(expected, signalTx.Read().get<sdv::u32string>());

    signalTx.Reset();
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
