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
#include "../../../sdv_services/core/app_settings.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"

// Load startup config
    // Invalid config
    // Empty config (default config)
    // Application types
    // Instance ID
    // Log handler
    // Console output
    // Config path

TEST(AppSettingsTest_StartupConfig, InvalidConfig)
{
    const std::string ssInvalidToml = R"toml([Application]
Mode = Standalone  # Missing quotes)toml";
    const std::string ssInvalidValue = R"toml([Application]
Mode = "Supadupa"  # Invalid value)toml";
    const std::string ssAdditionalValues = R"toml([Application]
Mode = "Standalone"
SpecialAction = "Fly to the moon"  # Additional value)toml";

    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    EXPECT_FALSE(settings.ProcessAppStartupConfig(ssInvalidToml));
    EXPECT_FALSE(settings.ProcessAppStartupConfig(ssInvalidValue));
    EXPECT_TRUE(settings.ProcessAppStartupConfig(ssAdditionalValues));
}

TEST(AppSettingsTest_StartupConfig, DefaultConfig)
{
    const std::string ssEmptyToml;

    CAppSettings settings;
    EXPECT_EQ(settings.GetInstanceID(), 0u);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(ssEmptyToml));
    EXPECT_TRUE(settings.IsStandaloneApplication());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::error);
    EXPECT_EQ(settings.GetConsoleReporting(), CAppSettings::EAppConsoleReporting::normal);
    EXPECT_EQ(settings.GetInstanceID(), 1000u);
}

TEST(AppSettingsTest_StartupConfig, ApplicationTypes)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(""));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_TRUE(settings.IsStandaloneApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "External")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::external);
    EXPECT_TRUE(settings.IsExternalApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Standalone")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_TRUE(settings.IsStandaloneApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Isolated")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_TRUE(settings.IsIsolatedApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::main);
    EXPECT_TRUE(settings.IsMainApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Essential")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_TRUE(settings.IsEssentialApplication());
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance")toml"));
    EXPECT_EQ(settings.GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_TRUE(settings.IsMaintenanceApplication());
}

TEST(AppSettingsTest_StartupConfig, InstanceID)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    EXPECT_EQ(settings.GetInstanceID(), 0u);

    // Change type to main; no instance ID yet
    settings.SetContextType(sdv::app::EAppContext::main);
    EXPECT_EQ(settings.GetInstanceID(), 0u);

    // Set instance ID, but no type
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Instance = 1234)toml"));
    EXPECT_EQ(settings.GetInstanceID(), 1234u);

    // Change types, only main, isolated and maintenance should provide the instance ID.
    settings.SetContextType(sdv::app::EAppContext::standalone);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.SetContextType(sdv::app::EAppContext::external);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.SetContextType(sdv::app::EAppContext::isolated);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.SetContextType(sdv::app::EAppContext::main);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.SetContextType(sdv::app::EAppContext::essential);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.SetContextType(sdv::app::EAppContext::maintenance);
    EXPECT_EQ(settings.GetInstanceID(), 1234u);
    settings.Reset();
    EXPECT_EQ(settings.GetInstanceID(), 0u);
}

TEST(AppSettingsTest_StartupConfig, LogHandler)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    
    // No config
    EXPECT_TRUE(settings.GetLoggerClass().empty());
    EXPECT_TRUE(settings.GetLoggerModulePath().empty());
    EXPECT_TRUE(settings.GetLoggerProgramTag().empty());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::error);

    // Default config
    EXPECT_TRUE(settings.ProcessAppStartupConfig(""));
    EXPECT_EQ(settings.GetLoggerClass(), "DefaultLoggerService");
    EXPECT_TRUE(settings.GetLoggerModulePath().empty());
    EXPECT_TRUE(settings.GetLoggerProgramTag().empty());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::error);

    // Default logger when main or isolated
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main")toml"));
    EXPECT_EQ(settings.GetLoggerClass(), "DefaultLoggerService");
    EXPECT_TRUE(settings.GetLoggerModulePath().empty());
    EXPECT_TRUE(settings.GetLoggerProgramTag().empty());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::info);

    // Default logger when main or isolated
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Isolated")toml"));
    EXPECT_EQ(settings.GetLoggerClass(), "DefaultLoggerService");
    EXPECT_TRUE(settings.GetLoggerModulePath().empty());
    EXPECT_TRUE(settings.GetLoggerProgramTag().empty());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::info);

    // Default logger for all others
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Standalone")toml"));
    EXPECT_EQ(settings.GetLoggerClass(), "DefaultLoggerService");
    EXPECT_TRUE(settings.GetLoggerModulePath().empty());
    EXPECT_TRUE(settings.GetLoggerProgramTag().empty());
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::error);

    // Set severity level for logger
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Trace")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::trace);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Debug")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::debug);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Info")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Warning")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::warning);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Error")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::error);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Fatal")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::fatal);
    EXPECT_FALSE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Filter = "Wrong")toml"));
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::fatal);
    
    // Set severity level for console
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Trace")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::trace);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Debug")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::debug);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Info")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::info);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Warning")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::warning);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Error")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::error);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Fatal")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::fatal);
    EXPECT_FALSE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
ViewFilter = "Wrong")toml"));
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::fatal);

    // Explicit logger
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([LogHandler]
Class = "logger_class"
Path = "logger.xyz"
Tag = "logger_tag"
Filter = "Trace"
ViewFilter = "Warning"
UnkownValue = "hi")toml"));
    EXPECT_EQ(settings.GetLoggerClass(), "logger_class");
    EXPECT_EQ(settings.GetLoggerModulePath(), "logger.xyz");
    EXPECT_EQ(settings.GetLoggerProgramTag(), "logger_tag");
    EXPECT_EQ(settings.GetLoggerSeverityFilter(), sdv::core::ELogSeverity::trace);
    EXPECT_EQ(settings.GetConsoleSeverityFilter(), sdv::core::ELogSeverity::warning);
}

TEST(AppSettingsTest_StartupConfig, ConnectionRetries)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);

    // Default is 5 retries
    EXPECT_EQ(settings.GetConnectRetries(), 5u);
    EXPECT_TRUE(settings.ProcessAppStartupConfig(""));
    EXPECT_EQ(settings.GetConnectRetries(), 5u);

    // Set 0 retries - this is not an error and minimizes to 3
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Connections]
Retries = 0)toml"));
    EXPECT_EQ(settings.GetConnectRetries(), 3u);

    // Set 10 retries
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Connections]
Retries = 10)toml"));
    EXPECT_EQ(settings.GetConnectRetries(), 10u);

    // Set 100 retries - this is not an error and maximizes to 30
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Connections]
Retries = 100)toml"));
    EXPECT_EQ(settings.GetConnectRetries(), 30u);
}

TEST(AppSettingsTest_StartupConfig, InstallDir)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);

    // Empty install directory
    EXPECT_TRUE(settings.GetRootDir().empty());
    EXPECT_TRUE(settings.GetInstallDir().empty());

    // Install directory has significance with main, isolated and maintenance apps only.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));
    EXPECT_TRUE(settings.GetRootDir().empty());
    EXPECT_TRUE(settings.GetInstallDir().empty());

    // Use main and a specific instance ID
    std::filesystem::remove_all(GetExecDirectory() / "2000");   // Needed to prevent settings file parsing.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));
    EXPECT_EQ(settings.GetRootDir(), GetExecDirectory());
    EXPECT_EQ(settings.GetInstallDir(), GetExecDirectory() / "2000");

    // Use isolated and a specific instance ID
    std::filesystem::remove_all(GetExecDirectory() / "2001"); // Needed to prevent settings file parsing.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Isolated"
Instance = 2001
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));
    EXPECT_EQ(settings.GetRootDir(), GetExecDirectory());
    EXPECT_EQ(settings.GetInstallDir(), GetExecDirectory() / "2001");
    
    // Use maintenance and a specific instance ID
    std::filesystem::remove_all(GetExecDirectory() / "2002"); // Needed to prevent settings file parsing.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2002
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));
    EXPECT_EQ(settings.GetRootDir(), GetExecDirectory());
    EXPECT_EQ(settings.GetInstallDir(), GetExecDirectory() / "2002");

    // Relative path
    std::filesystem::remove_all(GetExecDirectory() / "2003"); // Needed to prevent settings file parsing.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2003
InstallDir = "..")toml"));
    EXPECT_EQ(settings.GetRootDir(), GetExecDirectory() / "..");
    EXPECT_EQ(settings.GetInstallDir(), GetExecDirectory() / ".." / "2003");
}

TEST(AppSettingsTest_StartupConfig, CustomConfigFile)
{
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);

    // Default empty
    EXPECT_TRUE(settings.GetUserConfigPath().empty());

    // Application config path has no significance with main, isolated and maintenance apps only.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Config = "abc.def")toml"));
    EXPECT_EQ(settings.GetUserConfigPath(), "abc.def");

    // Use main
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Config = "def.hij")toml"));
    EXPECT_EQ(settings.GetUserConfigPath(), "abc.def");

    // Use isolated
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Isolated"
Config = "hij.klm")toml"));
    EXPECT_EQ(settings.GetUserConfigPath(), "abc.def");

    // Use maintenance
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Config = "klm.nop")toml"));
    EXPECT_EQ(settings.GetUserConfigPath(), "abc.def");
}