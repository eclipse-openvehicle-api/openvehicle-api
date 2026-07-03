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

#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/app_settings.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include <support/toml.h>

// Load settings
    // Invalid file
    // No file
    // Config paths
    // Listeners
    // Connections
// Save settings
    // No file
    // Config paths
    // Listeners
    // Connections

std::filesystem::path GetSettingsFilePath()
{
    // The settings file is located at the exe directory with sub-directory 2000.
    return GetExecDirectory() / "2000" / "settings.toml";
}

toml_parser::CParser ReadSettingsFile()
{
    std::ifstream fstream(GetSettingsFilePath());
    if (!fstream.is_open()) return {};
    toml_parser::CParser parser(std::string((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>()));
    fstream.close();
    return parser;
}

bool WriteSettingsFile(const std::string& rssSettings)
{
    std::filesystem::create_directories(GetSettingsFilePath().remove_filename());
    std::ofstream fstream(GetSettingsFilePath(), std::ios::trunc);
    if (!fstream.is_open()) return false;
    fstream << rssSettings;
    return true;
}

void DeleteSettingsFile()
{
    if (std::filesystem::exists(GetSettingsFilePath().remove_filename()))
        std::filesystem::remove_all(GetSettingsFilePath().remove_filename());
}

TEST(AppSettingsTest_SettingsConfig, LoadInvalidConfig)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    const std::string ssInvalidToml = R"toml([Settings]
Version = hundred  # Should be valid number)toml";
    const std::string ssInvalidValue = R"toml([Settings]
Version = "hundred"  # Invalid value)toml";
    const std::string ssInvalidVersion = R"toml([Settings]
Version = 99)toml";
    const std::string ssNoVersion = R"toml([Settings])toml";

    WriteSettingsFile(ssInvalidToml);
    EXPECT_FALSE(settings.LoadSettings());
    WriteSettingsFile(ssInvalidValue);
    EXPECT_FALSE(settings.LoadSettings());
    WriteSettingsFile(ssInvalidVersion);
    EXPECT_FALSE(settings.LoadSettings());
    WriteSettingsFile(ssNoVersion);
    EXPECT_FALSE(settings.LoadSettings());
}

TEST(AppSettingsTest_SettingsConfig, LoadNoConfig)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    // Read empty file - this is not an error
    EXPECT_TRUE(settings.LoadSettings());
}

TEST(AppSettingsTest_SettingsConfig, SaveConfigWrongMode)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_TRUE(settings.SaveSettings());

    // Verify for a file
    EXPECT_FALSE(std::filesystem::exists(GetSettingsFilePath()));
}

TEST(AppSettingsTest_SettingsConfig, SaveDefaultConfig)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_TRUE(settings.SaveSettings());

    // Verify for a file
    EXPECT_TRUE(std::filesystem::exists(GetSettingsFilePath()));

    // Verify settings
    auto parser = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings(parser.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings);
    EXPECT_EQ(tableSettings.GetDirect("Version").GetValue(), 100u);
    sdv::toml::CNode nodePlatformConfig = tableSettings.GetDirect("PlatformConfig");
    EXPECT_TRUE(nodePlatformConfig);
    sdv::toml::CNode nodeVehIfcConfig = tableSettings.GetDirect("VehIfcConfig");
    EXPECT_TRUE(nodeVehIfcConfig);
    sdv::toml::CNode nodeVehAbstrConfig = tableSettings.GetDirect("VehAbstrConfig");
    EXPECT_TRUE(nodeVehAbstrConfig);
    sdv::toml::CNode nodeAppConfig = tableSettings.GetDirect("AppConfig");
    EXPECT_TRUE(nodeAppConfig);
    sdv::toml::CNodeCollection arrayListener = tableSettings.GetDirect("Listener");
    EXPECT_FALSE(arrayListener);
    sdv::toml::CNodeCollection arrayConnection = tableSettings.GetDirect("Connection");
    EXPECT_FALSE(arrayConnection);
}

TEST(AppSettingsTest_SettingsConfig, LoadSettings_SystemConfigs)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::platform_config), "platform.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config), "vehicle_ifc.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config), "vehicle_abstract.toml");

    WriteSettingsFile(R"toml([Settings]
Version = 100
PlatformConfig = "abc.toml"
VehIfcConfig = "def.toml"
VehAbstrConfig = "ghi.toml")toml");

    // Read settings file
    EXPECT_TRUE(settings.LoadSettings());

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::platform_config), "abc.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config), "def.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config), "ghi.toml");
}

TEST(AppSettingsTest_SettingsConfig, SaveSettings_SystemConfigs)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::platform_config), "platform.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config), "vehicle_ifc.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config), "vehicle_abstract.toml");
    settings.DisableConfig(CAppSettings::EConfigType::platform_config);
    settings.DisableConfig(CAppSettings::EConfigType::vehicle_interface_config);
    settings.DisableConfig(CAppSettings::EConfigType::vehicle_abstraction_config);
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::platform_config), "");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config), "");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config), "");

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings(parser.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings);
    EXPECT_TRUE(tableSettings.GetDirect("PlatformConfig").GetValueAsPath().empty());
    EXPECT_TRUE(tableSettings.GetDirect("VehIfcConfig").GetValueAsPath().empty());
    EXPECT_TRUE(tableSettings.GetDirect("VehAbstrConfig").GetValueAsPath().empty());

    settings.EnableConfig(CAppSettings::EConfigType::platform_config);
    settings.EnableConfig(CAppSettings::EConfigType::vehicle_interface_config);
    settings.EnableConfig(CAppSettings::EConfigType::vehicle_abstraction_config);
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::platform_config), "platform.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config), "vehicle_ifc.toml");
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config), "vehicle_abstract.toml");

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser2 = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings2(parser2.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings2);
    EXPECT_EQ(tableSettings2.GetDirect("PlatformConfig").GetValueAsPath(), "platform.toml");
    EXPECT_EQ(tableSettings2.GetDirect("VehIfcConfig").GetValueAsPath(), "vehicle_ifc.toml");
    EXPECT_EQ(tableSettings2.GetDirect("VehAbstrConfig").GetValueAsPath(), "vehicle_abstract.toml");
}

TEST(AppSettingsTest_SettingsConfig, LoadSettings_UserConfig)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::user_config), "app_config.toml");

    WriteSettingsFile(R"toml([Settings]
Version = 100
AppConfig = "xyz.toml")toml");

    // Read settings file
    EXPECT_TRUE(settings.LoadSettings());

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::user_config), "xyz.toml");

    settings.Reset();

    // For all configurations except main, isolated and maintenance, the user config is supplied through the app config.
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Standalone"
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string()) + R"toml(
Config = "klm.toml")toml"));

    EXPECT_EQ(settings.GetUserConfigPath(), "klm.toml");
}

TEST(AppSettingsTest_SettingsConfig, SaveSettings_UserConfig)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::user_config), "app_config.toml");
    settings.DisableConfig(CAppSettings::EConfigType::user_config);
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::user_config), "");

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings(parser.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings);
    EXPECT_TRUE(tableSettings.GetDirect("AppConfig").GetValueAsPath().empty());

    settings.EnableConfig(CAppSettings::EConfigType::user_config);
    EXPECT_EQ(settings.GetConfigPath(CAppSettings::EConfigType::user_config), "app_config.toml");

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser2 = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings2(parser2.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings2);
    EXPECT_EQ(tableSettings2.GetDirect("AppConfig").GetValueAsPath(), "app_config.toml");
}

TEST(AppSettingsTest_SettingsConfig, LoadSettings_Listeners)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    // Default listener shoule be available.
    EXPECT_EQ(settings.GetListeners().size(), 1u);

    WriteSettingsFile(R"toml([Settings]
Version = 100

[[Settings.Listener]]
Name = "MyListener1"
[Settings.Listener.Provider]
Name = "MyListenerProvider1"
[Settings.Listener.IpcChannel]
abc = "def"

[[Settings.Listener]]
Name = "MyListener2"
[Settings.Listener.Provider]
Name = "MyListenerProvider2"
[Settings.Listener.IpcChannel]
xyz = 1234
)toml");

    // Read settings file - note: the listeners are part of a listener map. The order is not fixed.
    // There will be an additional default listener.
    EXPECT_TRUE(settings.LoadSettings());
    auto seqListeners = settings.GetListeners();
    ASSERT_EQ(seqListeners.size(), 3u);
    EXPECT_TRUE(seqListeners[0] == "MyListener1" || seqListeners[1] == "MyListener1" || seqListeners[2] == "MyListener1");
    EXPECT_TRUE(seqListeners[0] == "MyListener2" || seqListeners[1] == "MyListener2" || seqListeners[2] == "MyListener2");
    EXPECT_TRUE(seqListeners[0] == "Default" || seqListeners[1] == "Default" || seqListeners[2] == "Default");
    EXPECT_TRUE(settings.GetListenerConfig("MyListener0").empty());
    EXPECT_TRUE(toml_parser::CompareEqual(settings.GetListenerConfig("MyListener1"), R"toml([Provider]
Name = "MyListenerProvider1"

[IpcChannel]
abc = "def"
)toml"));
    EXPECT_TRUE(toml_parser::CompareEqual(settings.GetListenerConfig("MyListener2"), R"toml([Provider]
Name = "MyListenerProvider2"

[IpcChannel]
xyz = 1234
)toml"));
}

TEST(AppSettingsTest_SettingsConfig, SaveSettings_Listeners)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetListeners().size(), 1u);

    EXPECT_TRUE(settings.AddListenerConfig("MyListener2", R"toml([Provider]
Name = "MyListenerProvider2"
[IpcChannel]
abc = "def")toml"));
    EXPECT_EQ(settings.GetListeners().size(), 2u);

    EXPECT_TRUE(settings.AddListenerConfig("MyListener1", R"toml([Provider]
Name = "MyListenerProvider1"
[IpcChannel]
xyz = 1234)toml"));
    EXPECT_EQ(settings.GetListeners().size(), 3u);

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings(parser.Root().GetNodeDirect("Settings"));
    [[maybe_unused]] std::string ss = parser.GenerateTOML();
    EXPECT_TRUE(tableSettings);
    sdv::toml::CNodeCollection tableListener = tableSettings.GetDirect("Listener[0]");
    if (tableListener.GetDirect("Name").GetValueAsString() != "MyListener1")
        tableListener = tableSettings.GetDirect("Listener[1]");
    EXPECT_TRUE(tableListener);
    EXPECT_EQ(tableListener.GetDirect("Name").GetValueAsString(), "MyListener1");
    EXPECT_EQ(tableListener.GetDirect("Provider.Name").GetValueAsString(), "MyListenerProvider1");
    EXPECT_EQ(tableListener.GetDirect("IpcChannel.xyz").GetValue(), 1234);
    tableListener = tableSettings.GetDirect("Listener[0]");
    if (tableListener.GetDirect("Name").GetValueAsString() != "MyListener2")
        tableListener = tableSettings.GetDirect("Listener[1]");
    EXPECT_TRUE(tableListener);
    EXPECT_EQ(tableListener.GetDirect("Name").GetValueAsString(), "MyListener2");
    EXPECT_EQ(tableListener.GetDirect("Provider.Name").GetValueAsString(), "MyListenerProvider2");
    EXPECT_EQ(tableListener.GetDirect("IpcChannel.abc").GetValueAsString(), "def");

    // Remove one listener
    settings.RemoveListenerConfig("MyListener1");
    EXPECT_EQ(settings.GetListeners().size(), 2u);
    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser2 = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings2(parser2.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings2);
    tableListener = tableSettings2.GetDirect("Listener[0]");
    EXPECT_TRUE(tableListener);
    EXPECT_EQ(tableListener.GetDirect("Name").GetValueAsString(), "MyListener2");
    EXPECT_EQ(tableListener.GetDirect("Provider.Name").GetValueAsString(), "MyListenerProvider2");
    EXPECT_EQ(tableListener.GetDirect("IpcChannel.abc").GetValueAsString(), "def");
}

TEST(AppSettingsTest_SettingsConfig, LoadSettings_Connections)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Main"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    // Default connection only
    EXPECT_EQ(settings.GetConnections().size(), 1);

    WriteSettingsFile(R"toml([Settings]
Version = 100

[[Settings.Connection]]
Name = "MyConnection1"
[Settings.Connection.Provider]
Name = "MyConnectionProvider1"
[Settings.Connection.IpcChannel]
abc = "def"

[[Settings.Connection]]
Name = "MyConnection2"
[Settings.Connection.Provider]
Name = "MyConnectionProvider2"
[Settings.Connection.IpcChannel]
xyz = 1234
)toml");

    // Read settings file - note: the order of connections is preserved in the settings file.
    // There will be an additional default connection.
    EXPECT_TRUE(settings.LoadSettings());
    auto seqConnections = settings.GetConnections();
    ASSERT_EQ(seqConnections.size(), 3u);
    EXPECT_EQ(seqConnections[0], "Default");
    EXPECT_EQ(seqConnections[1], "MyConnection1");
    EXPECT_EQ(seqConnections[2], "MyConnection2");
    EXPECT_TRUE(settings.GetConnectionConfig("MyConnection0").empty());
    EXPECT_TRUE(toml_parser::CompareEqual(settings.GetConnectionConfig("MyConnection1"), R"toml([Provider]
Name = "MyConnectionProvider1"

[IpcChannel]
abc = "def"
)toml"));
    EXPECT_TRUE(toml_parser::CompareEqual(settings.GetConnectionConfig("MyConnection2"), R"toml([Provider]
Name = "MyConnectionProvider2"

[IpcChannel]
xyz = 1234
)toml"));
}

TEST(AppSettingsTest_SettingsConfig, SaveSettings_Connections)
{
    // Settings for the main application with a specific instance ID
    CAppSettings settings;
    settings.SetConsoleReporting(CAppSettings::EAppConsoleReporting::silent);
    DeleteSettingsFile(); // Needed to prevent settings file parsing during startup.
    EXPECT_TRUE(settings.ProcessAppStartupConfig(R"toml([Application]
Mode = "Maintenance"
Instance = 2000
InstallDir = )toml" + toml_parser::QuoteText(GetExecDirectory().generic_u8string())));

    EXPECT_EQ(settings.GetConnections().size(), 1u);

    EXPECT_TRUE(settings.AddConnectionConfig("MyConnection2", R"toml([Provider]
Name = "MyConnectionProvider2"
[IpcChannel]
abc = "def")toml"));
    EXPECT_EQ(settings.GetConnections().size(), 2u);

    EXPECT_TRUE(settings.AddConnectionConfig("MyConnection1", R"toml([Provider]
Name = "MyConnectionProvider1"
[IpcChannel]
xyz = 1234)toml"));
    EXPECT_EQ(settings.GetConnections().size(), 3u);

    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings(parser.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings);
    sdv::toml::CNodeCollection tableConnection = tableSettings.GetDirect("Connection[0]");
    EXPECT_TRUE(tableConnection);
    EXPECT_EQ(tableConnection.GetDirect("Name").GetValueAsString(), "MyConnection2");
    EXPECT_EQ(tableConnection.GetDirect("Provider.Name").GetValueAsString(), "MyConnectionProvider2");
    EXPECT_EQ(tableConnection.GetDirect("IpcChannel.abc").GetValueAsString(), "def");
    tableConnection = tableSettings.GetDirect("Connection[1]");
    EXPECT_TRUE(tableConnection);
    EXPECT_EQ(tableConnection.GetDirect("Name").GetValueAsString(), "MyConnection1");
    EXPECT_EQ(tableConnection.GetDirect("Provider.Name").GetValueAsString(), "MyConnectionProvider1");
    EXPECT_EQ(tableConnection.GetDirect("IpcChannel.xyz").GetValue(), 1234);

    // Remove one listener
    settings.RemoveConnectionConfig("MyConnection1");
    EXPECT_EQ(settings.GetConnections().size(), 2u);
    EXPECT_TRUE(settings.SaveSettings());

    // Verify settings
    auto parser2 = ReadSettingsFile();
    sdv::toml::CNodeCollection tableSettings2(parser2.Root().GetNodeDirect("Settings"));
    EXPECT_TRUE(tableSettings2);
    tableConnection = tableSettings2.GetDirect("Connection[0]");
    EXPECT_TRUE(tableConnection);
    EXPECT_EQ(tableConnection.GetDirect("Name").GetValueAsString(), "MyConnection2");
    EXPECT_EQ(tableConnection.GetDirect("Provider.Name").GetValueAsString(), "MyConnectionProvider2");
    EXPECT_EQ(tableConnection.GetDirect("IpcChannel.abc").GetValueAsString(), "def");
}
