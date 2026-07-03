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

TEST(ParamUtils, ResolveText_FailObjectName)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Invalid parameters interface
    std::string ssText = sdv::core::ResolveText("$(InvalidObject:Application.InstallDir)");
    EXPECT_TRUE(ssText.empty());
    bool bExcept = false;
    try
    {
        sdv::core::ResolveText("$(InvalidObject:Application.InstallDir)", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Invalid parameter name
    ssText = sdv::core::ResolveText("$(AppSettingsService:ParamDoesNotExist)");
    EXPECT_TRUE(ssText.empty());
    bExcept = false;
    try
    {
        sdv::core::ResolveText("$(AppSettingsService:ParamDoesNotExist)", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    control.Shutdown();
}

TEST(ParamUtils, ResolveText_EscapingDollarCharacter)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Not escaping $ - this is allowed!
    std::string ssText = sdv::core::ResolveText("There is a $ in this text.");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(ssText, "There is a $ in this text.");
    bool bExcept = false;
    ssText.clear();
    try
    {
        ssText = sdv::core::ResolveText("There is a $ in this text.", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_FALSE(bExcept);
    EXPECT_EQ(ssText, "There is a $ in this text.");

    // Missing (-character - this is the same as not escaping $
    ssText = sdv::core::ResolveText("$AppSettingsService:Application.InstallDir)");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(ssText, "$AppSettingsService:Application.InstallDir)");
    bExcept = false;
    ssText.clear();
    try
    {
        ssText = sdv::core::ResolveText("$AppSettingsService:Application.InstallDir)", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_FALSE(bExcept);
    EXPECT_EQ(ssText, "$AppSettingsService:Application.InstallDir)");

    // Missing )-character - this is an error
    ssText = sdv::core::ResolveText("$(AppSettingsService:Application.InstallDir");
    EXPECT_TRUE(ssText.empty());
    bExcept = false;
    try
    {
        sdv::core::ResolveText("$(AppSettingsService:Application.InstallDir", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Invalid object-param pair
    ssText = sdv::core::ResolveText("$(Application.InstallDir)");
    EXPECT_TRUE(ssText.empty());
    bExcept = false;
    try
    {
        sdv::core::ResolveText("$(Application.InstallDir)", true, false);
    }
    catch (const std::runtime_error&)
    {
        bExcept = true;
    }
    EXPECT_TRUE(bExcept);

    // Resolvinf during first iteration
    ssText = sdv::core::ResolveText("$(AppSettingsService:Application.Mode)");
    EXPECT_EQ(ssText, "Main");

    // Resolving during second iteration
    ssText = sdv::core::ResolveText("\\$(AppSettingsService:Application.Mode)");
    EXPECT_EQ(ssText, "Main");

    // No further iteration
    ssText = sdv::core::ResolveText("\\$(AppSettingsService:Application.Mode)", false);
    EXPECT_EQ(ssText, "$(AppSettingsService:Application.Mode)");

    control.Shutdown();
}

TEST(ParamUtils, ResolveText_Object)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2010");
    ASSERT_TRUE(bResult);

    // Request the install directory through parameters
    std::string ssText = sdv::core::ResolveText("$(AppSettingsService:Application.InstallDir)/MyPackage");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(std::filesystem::relative(ssText, control.GetAppDirectory()), "../../bin/2010/MyPackage");

    // Request the install directory through parameters
    ssText = sdv::core::ResolveText("The application mode is \"$( AppSettingsService : Application.Mode )\" and the instance numbver is $(AppSettingsService:Application.Instance).");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(ssText, "The application mode is \"Main\" and the instance numbver is 2010.");

    ssText = sdv::core::ResolveText("The application mode is \"$(AppSettingsService:Application.Mode)\" and the instance number "
        "is $(AppSettingsService:Application.Instance).");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(ssText, "The application mode is \"Main\" and the instance number is 2010.");

    // Request the install directory through parameters
    ssText = sdv::core::ResolveText("Valid text with a \\$ in the text...");
    EXPECT_FALSE(ssText.empty());
    EXPECT_EQ(ssText, "Valid text with a $ in the text...");

    control.Shutdown();
}

// TODO: Recursive test
// e.g. \$($(MyObject1:Name):$(MyObject2:ParamName))
