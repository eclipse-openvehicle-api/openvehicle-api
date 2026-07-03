/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include <gtest/gtest.h>
#include <support/app_control.h>
#include <support/local_service_access.h>

TEST(ParamUtils, GetParameter_FailParamInterface)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Get the app settings service
    sdv::TInterfaceAccessPtr ptrSystemSettings = sdv::core::GetObject("AppSettingsService");
    ASSERT_TRUE(ptrSystemSettings);

    // Request the IParameters interface
    sdv::IParameters* pParameters = ptrSystemSettings.GetInterface<sdv::IParameters>();
    ASSERT_NE(pParameters, nullptr);

    // Invalid parameters interface
    sdv::any_t anyFail = sdv::core::GetParameter(nullptr, "Application.InstallDir");
    EXPECT_TRUE(anyFail.empty());
    bool bExcept = false;
    try
    {
        sdv::core::GetParameter(nullptr, "Application.InstallDir", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Invalid parameters name
    anyFail = sdv::core::GetParameter(pParameters, "ParamDoesNotExist");
    EXPECT_TRUE(anyFail.empty());
    bExcept = false;
    try
    {
        sdv::core::GetParameter(pParameters, "ParamDoesNotExist", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_FailObjectInterface)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Get the app settings service
    sdv::TInterfaceAccessPtr ptrSystemSettings = sdv::core::GetObject("AppSettingsService");
    ASSERT_TRUE(ptrSystemSettings);

    // Invalid object interface
    sdv::any_t anyFail = sdv::core::GetParameter(sdv::TInterfaceAccessPtr(), "Application.InstallDir");
    EXPECT_TRUE(anyFail.empty());
    bool bExcept = false;
    try
    {
        sdv::core::GetParameter(sdv::TInterfaceAccessPtr(), "Application.InstallDir", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Invalid parameters name
    anyFail = sdv::core::GetParameter(ptrSystemSettings, "ParamDoesNotExist");
    EXPECT_TRUE(anyFail.empty());
    bExcept = false;
    try
    {
        sdv::core::GetParameter(ptrSystemSettings, "ParamDoesNotExist", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_FailObjectName)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Invalid object name
    sdv::any_t anyFail = sdv::core::GetParameter("InvalidObject", "Application.InstallDir");
    EXPECT_TRUE(anyFail.empty());
    bool bExcept = false;
    try
    {
        sdv::core::GetParameter("InvalidObject", "Application.InstallDir", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Invalid parameters name
    anyFail = sdv::core::GetParameter("AppSettingsService", "ParamDoesNotExist");
    EXPECT_TRUE(anyFail.empty());
    bExcept = false;
    try
    {
        sdv::core::GetParameter("AppSettingsService", "ParamDoesNotExist", false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_ParamInterface)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Get the app settings service
    sdv::TInterfaceAccessPtr ptrSystemSettings = sdv::core::GetObject("AppSettingsService");
    ASSERT_TRUE(ptrSystemSettings);

    // Request the IParameters interface
    sdv::IParameters* pParameters = ptrSystemSettings.GetInterface<sdv::IParameters>();
    ASSERT_NE(pParameters, nullptr);

    // Request the install directory through parameters
    sdv::any_t anyInstall = sdv::core::GetParameter(pParameters, "Application.InstallDir");
    EXPECT_FALSE(anyInstall.empty());
    EXPECT_EQ(std::filesystem::relative(anyInstall.get<std::string>(), control.GetAppDirectory()), "../../bin/2010");

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_ObjectInterface)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Get the app settings service
    sdv::TInterfaceAccessPtr ptrSystemSettings = sdv::core::GetObject("AppSettingsService");
    ASSERT_TRUE(ptrSystemSettings);

    // Request the install directory through parameters
    sdv::any_t anyInstall = sdv::core::GetParameter(ptrSystemSettings, "Application.InstallDir");
    EXPECT_FALSE(anyInstall.empty());
    EXPECT_EQ(std::filesystem::relative(anyInstall.get<std::string>(), control.GetAppDirectory()), "../../bin/2010");

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_ObjectName)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Request the install directory through parameters
    sdv::any_t anyInstall = sdv::core::GetParameter("AppSettingsService", "Application.InstallDir");
    EXPECT_FALSE(anyInstall.empty());
    EXPECT_EQ(std::filesystem::relative(anyInstall.get<std::string>(), control.GetAppDirectory()), "../../bin/2010");

    control.Shutdown();
}

TEST(ParamUtils, GetParameter_EnumExpand)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Request the mode through parameters
    sdv::any_t anyMode = sdv::core::GetParameter("AppSettingsService", "Application.Mode");
    EXPECT_FALSE(anyMode.empty());
    EXPECT_EQ(static_cast<uint64_t>(anyMode), static_cast<uint64_t>(sdv::app::EAppContext::main));

    // Request the expanded mode through parameters
    std::string ssMode = sdv::core::GetParameterExpand("AppSettingsService", "Application.Mode");
    EXPECT_FALSE(ssMode.empty());
    EXPECT_EQ(ssMode, "Main");

    control.Shutdown();
}

