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

#include "app_settings.h"
#include "toml_parser/parser_toml.h"
#include "../../global/flags.h"
#include "../../global/exec_dir_helper.h"
#include "../../global/ipc_named_mutex.h"
#include <support/toml.h>

CAppSettings& GetAppSettings()
{
    static CAppSettings app_settings;
    return app_settings;
}

CAppSettings::CAppSettings()
{}

CAppSettings::~CAppSettings()
{}

bool CAppSettings::ProcessAppStartupConfig(const sdv::u8string& rssConfig)
{
    toml_parser::CParser parserStartupConfig;
    std::string ssError;
    try
    {
        // Read the configuration
        if (!parserStartupConfig.Process(rssConfig))
            return false;
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        ssError = std::string("ERROR: Failed to parse application configuration: ") + rexcept.what();
    }

    sdv::toml::CNodeCollection tableStartupConfig(&parserStartupConfig.Root());

    // Get the reporting settings (if this succeeded at all...)
    m_bSilent = tableStartupConfig.GetDirect("Console.Report").GetValue().get<std::string>() == "Silent";
    m_bVerbose = tableStartupConfig.GetDirect("Console.Report").GetValue().get<std::string>() == "Verbose";

    // Report the outstanding error (if there is one...)
    if (!ssError.empty())
    {
        if (!m_bSilent)
            std::cerr << ssError << std::endl;
        return false;
    }

    // Allow a custom logger to be defined
    m_pathLoggerModule = tableStartupConfig.GetDirect("LogHandler.Path").GetValue().get<std::string>();
    m_ssLoggerClass = tableStartupConfig.GetDirect("LogHandler.Class").GetValue().get<std::string>();
    if (m_ssLoggerClass.empty())
        m_ssLoggerClass = "DefaultLoggerService";

    // Get an optional program tag for the logger
    m_ssProgramTag = tableStartupConfig.GetDirect("LogHandler.Tag").GetValue().get<std::string>();

    // Get the application-mode
    std::string ssApplication = tableStartupConfig.GetDirect("Application.Mode").GetValue();
    if (ssApplication.empty()) ssApplication = "Standalone";
    if (ssApplication == "Standalone")
        m_eContextMode = sdv::app::EAppContext::standalone;
    else if (ssApplication == "External")
        m_eContextMode = sdv::app::EAppContext::external;
    else if (ssApplication == "Isolated")
        m_eContextMode = sdv::app::EAppContext::isolated;
    else if (ssApplication == "Main")
        m_eContextMode = sdv::app::EAppContext::main;
    else if (ssApplication == "Essential")
        m_eContextMode = sdv::app::EAppContext::essential;
    else if (ssApplication == "Maintenance")
        m_eContextMode = sdv::app::EAppContext::maintenance;
    else
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to process startup config: invalid application-mode specified for core library: "
                      << ssApplication << std::endl;
        return false;
    }

    // Get the severity level filter for the logger
    auto fnTranslateSevFilter = [this](const std::string& rssLogFilter, const sdv::core::ELogSeverity eDefault)
    {
        sdv::core::ELogSeverity eSeverityFilter = eDefault;
        if (rssLogFilter == "Trace")
            eSeverityFilter = sdv::core::ELogSeverity::trace;
        else if (rssLogFilter == "Debug")
            eSeverityFilter = sdv::core::ELogSeverity::debug;
        else if (rssLogFilter == "Info")
            eSeverityFilter = sdv::core::ELogSeverity::info;
        else if (rssLogFilter == "Warning")
            eSeverityFilter = sdv::core::ELogSeverity::warning;
        else if (rssLogFilter == "Error")
            eSeverityFilter = sdv::core::ELogSeverity::error;
        else if (rssLogFilter == "Fatal")
            eSeverityFilter = sdv::core::ELogSeverity::fatal;
        else if (!rssLogFilter.empty())
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to process application log: invalid severity level filter: '" << rssLogFilter << "'"
                          << std::endl;
        }
        return eSeverityFilter;
    };
    sdv::core::ELogSeverity eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::error;
    if (IsMainApplication() || IsIsolatedApplication()) eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::info;
    m_eSeverityFilter = fnTranslateSevFilter(tableStartupConfig.GetDirect("LogHandler.Filter").GetValue(),
        sdv::core::ELogSeverity::info);
    m_eSeverityViewFilter = fnTranslateSevFilter(
        tableStartupConfig.GetDirect("LogHandler.ViewFilter").GetValue(), eLogDefaultViewSeverityFilter);

    // Get the optional instance ID.
    sdv::any_t anyInstanceID = tableStartupConfig.GetDirect("Application.Instance").GetValue();
    if (anyInstanceID) m_uiInstanceID = anyInstanceID;
    else
        m_uiInstanceID = 1000u;

    // Number of attempts to establish a connection to a running instance.
    m_uiRetries = tableStartupConfig.GetDirect("Application.Retries").GetValue();
    if (m_uiRetries > 30)
        m_uiRetries = 30;
    else if (m_uiRetries < 3)
        m_uiRetries = 3;

    // Main and isolated apps specific information.
    if (IsMainApplication() || IsIsolatedApplication() || IsMaintenanceApplication())
    {
        // Get the optional installation directory.
        m_pathRootDir = tableStartupConfig.GetDirect("Application.InstallDir").GetValueAsPath();
        if (!m_pathRootDir.empty())
        {
            if (m_pathRootDir.is_relative())
                m_pathRootDir = GetExecDirectory() / m_pathRootDir;
        }
        else
            m_pathRootDir = GetExecDirectory();
        m_pathInstallDir = m_pathRootDir / std::to_string(m_uiInstanceID);
        try
        {
            std::filesystem::create_directories(m_pathInstallDir);
        }
        catch (const std::filesystem::filesystem_error& rexcept)
        {
            if (!m_bSilent)
            {
                std::cerr << "Cannot create installation directory: " << m_pathInstallDir << std::endl;
                std::cerr << "  Reason: " << rexcept.what() << std::endl;
            }
            return false;
        }
    }

    // Maintenance, main and isolated applications cannot load specific configs. The others can specify a configuration file, but
    // not auto-updateable.
    if (!IsMainApplication() && !IsMaintenanceApplication() && !IsIsolatedApplication())
        m_pathUserConfig = tableStartupConfig.GetDirect("Application.Config").GetValueAsPath();

    // Read the settings... if existing. And only for the main application
    if (IsMainApplication() && !LoadSettingsFile())
        return false;

    return true;
}

bool CAppSettings::LoadSettingsFile()
{
    // Check for the proper context
    switch (m_eContextMode)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        break;
    default:
        return true; // Not an error...
    }

    // If the template is not existing, this is not an error...
    if (!std::filesystem::exists(m_pathInstallDir / "settings.toml"))
        return true;

    std::ifstream fstream(m_pathInstallDir / "settings.toml");
    std::string ssSettings((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
    fstream.close();

    try
    {
        // Read the configuration
        toml_parser::CParser parserSettings(ssSettings);

        // If there is no "Settings" table, this is not an error...
        sdv::toml::CNodeCollection tableSettings(parserSettings.Root().GetNodeDirect("Settings"));
        if (!tableSettings) return true;

        // Check for the version
        uint32_t uiVersion = tableSettings.GetDirect("Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Invalid version of application settings file (expected version "
                          << SDVFrameworkInterfaceVersion << ", but available version " << uiVersion << ")" << std::endl;
            return false;
        }

        // Get the platform config - but only when not specified over the app-control-config.
        if (m_pathPlatformConfig.empty())
            m_pathPlatformConfig = tableSettings.GetDirect("PlatformConfig").GetValueAsPath();
        
        // Get the vehicle interface config - but only when not specified over the app-control-config.
        if (m_pathVehIfcConfig.empty())
            m_pathVehIfcConfig = tableSettings.GetDirect("VehIfcConfig").GetValueAsPath();
        
        // Get the vehicle abstraction config - but only when not specified over the app-control-config.
        if (m_pathVehAbstrConfig.empty())
            m_pathVehAbstrConfig = tableSettings.GetDirect("VehAbstrConfig").GetValueAsPath();

        // Get the application config - but only when not specified over the app-control-config.
        if (m_pathUserConfig.empty())
            m_pathUserConfig  = tableSettings.GetDirect("AppConfig").GetValueAsPath();
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to parse application settings: " << rexcept.what() << std::endl;
        return false;
    }
    return true;
}

bool CAppSettings::SaveSettingsFile()
{
    // Check for the proper context
    switch (m_eContextMode)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        break;
    default:
        return true;    // Not an error...
    }

    // Protect against multiple write actions at the same time.
    ipc::named_mutex mtx("LockSdvAppSettings_" + std::to_string(m_uiInstanceID));
    // Warning of cppcheck for locking a local mutex, which doesn't have any effect. Since this is a named mutex between
    // applciations, the warning is not correct. Suppress warning.
    // cppcheck-suppress localMutex
    std::unique_lock<ipc::named_mutex> lock(mtx);

    const std::string ssSettingsTemplate = R"toml(# Settings file
[Settings]
Version = )toml" + std::to_string(SDVFrameworkInterfaceVersion) + R"toml(

# The system configuration is divided into:
#  platform config     - containing all the components needed to interact with the OS,
#                        middleware, vehicle bus, Ethernet.
#  vehicle interface   - containing the vehicle bus interpretation components like data link
#                        based on DBC and devices for their abstraction.
#  vehicle abstraction - containing the vehicle abstraction services
# The configuration files are loaded exactly in that order, allowing the vehicle interface to
# depend on the platform and the vehicle abstraction to depend on the vehicle interface.
# The configurations are loaded if the PlatformConfig, VehIfcConfig and VehAbstrConfig keywords
# are present and describe a valid configuration file.
# A relative path is relative to the installation directory (being "exe_location/instance_id").
#
# Example:
#   PlatformConfig = "platform.toml"
#   VehIfcConfig = "vehicle_ifc.toml"
#   VehAbstrConfig = "vehicle_abstract.toml"
#
PlatformConfig = ""
VehIfcConfig = ""
VehAbstrConfig = ""

# The application config contains the configuration file that can be updated when services and
# apps are being added to the system (or being removed from the system). Load the application
# config by providing the "AppConfig" keyword as a string value. A relative path is relative to
# the installation directory (being "exe_location/instance_id").
#
# Example
#   AppConfig = "app_config.toml"
AppConfig = ""
)toml";

    // If the template is not existing, create a default settings file...
    std::string ssSettings;
    bool bChangeDetected = false;
    if (!std::filesystem::exists(m_pathInstallDir / "settings.toml"))
    {
        bChangeDetected = true;
        ssSettings = std::move(ssSettingsTemplate);
        m_bPlatformConfig = true;
        m_bVehIfcConfig = true;
        m_bVehAbstrConfig = true;
        m_bUserConfig = true;
    }
    else
    {
        // Open the existing settings file
        std::ifstream fstream(m_pathInstallDir / "settings.toml");
        if (!fstream.is_open())
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Cannot open the application settings file." << std::endl;
            return false;
        }

        // Read the settings file
        ssSettings = std::string((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
        if (ssSettings.empty())
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Cannot read the application settings file; will use default." << std::endl;
            ssSettings = std::move(ssSettingsTemplate);
            bChangeDetected = true;
        }
    }

    try
    {
        // Read the settings
        toml_parser::CParser parserSettings(ssSettings);

        // Check for the version
        sdv::toml::CNodeCollection tableRoot(&parserSettings.Root());
        if (!tableRoot)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Invalid TOML file '" << (m_pathInstallDir / "settings.toml").generic_u8string() << "'"
                          <<
                    std::endl;
            return false;
        }
        sdv::toml::CNodeCollection tableSettings = tableRoot.GetDirect("Settings");
        if (!tableSettings)
            tableSettings = tableRoot.AddTable("Settings");
        if (!tableSettings)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Invalid 'Settings' table." << std::endl;
            return false;
        }

        uint32_t uiVersion = tableSettings.GetDirect("Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Invalid version of application settings file (expected version "
                            << SDVFrameworkInterfaceVersion << ", but available version " << uiVersion << ")" << std::endl;
            return false;
        }

        // Generic update config file function
        auto fnUpdateConfig = [&](const std::string &rssConfigKey, const std::filesystem::path& rpathConfigFile)
        {
            sdv::toml::CNode nodeUserConfig = tableSettings.GetDirect(rssConfigKey);
            if (nodeUserConfig.GetValue().empty())
            {
                bChangeDetected = true;
                if (nodeUserConfig)
                    nodeUserConfig.Delete();
                nodeUserConfig = tableSettings.AddValue(rssConfigKey, rpathConfigFile);
                if (!nodeUserConfig)
                {
                    if (!m_bSilent) std::cerr << "ERROR: Cannot insert the \"Settings." << rssConfigKey <<
                        "\" value; cannot process further." << std::endl;
                    return false;
                }
            }
            else if (nodeUserConfig.GetValue() != rpathConfigFile)
            {
                bChangeDetected = true;
                if (!nodeUserConfig.SetValue(rpathConfigFile))
                {
                    if (!m_bSilent) std::cerr << "ERROR: Cannot update the \"Settings." << rssConfigKey <<
                        "\" value; cannot process further." << std::endl;
                    return false;
                }
            }
            return true;
        };

        // Update the configuration path values.
        if (m_bPlatformConfig && !fnUpdateConfig("PlatformConfig", m_pathPlatformConfig)) return false;
        if (m_bVehIfcConfig && !fnUpdateConfig("VehIfcConfig", m_pathVehIfcConfig)) return false;
        if (m_bVehAbstrConfig && !fnUpdateConfig("VehAbstrConfig", m_pathVehAbstrConfig)) return false;
        if (m_bUserConfig && !fnUpdateConfig("AppConfig", m_pathUserConfig)) return false;

        // Save the settings file if needed
        if (bChangeDetected)
        {
            std::ofstream fstream(m_pathInstallDir / "settings.toml", std::ios::trunc);
            if (!fstream.is_open())
            {
                if (!m_bSilent)
                    std::cerr << "ERROR: Cannot write the application settings file." << std::endl;
                return false;
            }
            fstream << parserSettings.GenerateTOML();

            m_bPlatformConfig = false;
            m_bVehIfcConfig = false;
            m_bVehAbstrConfig = false;
            m_bUserConfig = false;
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to parse application settings: " << rexcept.what() << std::endl;
        return false;
    }
    return true;
}

bool CAppSettings::IsMainApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::main;
}

bool CAppSettings::IsIsolatedApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::isolated;
}

bool CAppSettings::IsStandaloneApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::standalone;
}

bool CAppSettings::IsEssentialApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::essential;
}

bool CAppSettings::IsMaintenanceApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::maintenance;
}

bool CAppSettings::IsExternalApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::external;
}

sdv::app::EAppContext CAppSettings::GetContextType() const
{
    return m_eContextMode;
}

uint32_t CAppSettings::GetInstanceID() const
{
    return m_uiInstanceID;
}

uint32_t CAppSettings::GetRetries() const
{
    return m_uiRetries;
}

std::string CAppSettings::GetLoggerClass() const
{
    return m_ssLoggerClass;
}

std::filesystem::path CAppSettings::GetLoggerModulePath() const
{
    return m_pathLoggerModule;
}

std::string CAppSettings::GetLoggerProgramTag() const
{
    return m_ssProgramTag;
}

sdv::core::ELogSeverity CAppSettings::GetLoggerSeverityFilter() const
{
    return m_eSeverityFilter;
}

sdv::core::ELogSeverity CAppSettings::GetConsoleSeverityFilter() const
{
    return m_eSeverityViewFilter;
}

bool CAppSettings::IsConsoleSilent() const
{
    return m_bSilent;
}

bool CAppSettings::IsConsoleVerbose() const
{
    return m_bVerbose;
}

std::filesystem::path CAppSettings::GetRootDir() const
{
    return m_pathRootDir;
}

std::filesystem::path CAppSettings::GetInstallDir() const
{
    return m_pathInstallDir;
}

std::vector<std::filesystem::path> CAppSettings::GetSystemConfigPaths() const
{
    std::vector<std::filesystem::path> vecSysConfigs;
    if (!m_pathPlatformConfig.empty())
        vecSysConfigs.push_back(m_pathPlatformConfig);
    if (!m_pathVehIfcConfig.empty())
        vecSysConfigs.push_back(m_pathVehIfcConfig);
    if (!m_pathVehAbstrConfig.empty())
        vecSysConfigs.push_back(m_pathVehAbstrConfig);
    return vecSysConfigs;
}

std::filesystem::path CAppSettings::GetConfigPath(EConfigType eType) const
{
    // Is running as main application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return {};

    switch (eType)
    {
    case EConfigType::platform_config:
        if (!m_pathPlatformConfig.empty()) return m_pathPlatformConfig;
        return "platform.toml";
    case EConfigType::vehicle_interface_config:
        if (!m_pathVehIfcConfig.empty()) return m_pathVehIfcConfig;
        return "vehicle_ifc.toml";
    case EConfigType::vehicle_abstraction_config:
        if (!m_pathVehAbstrConfig.empty()) return m_pathVehAbstrConfig;
        return "vehicle_abstract.toml";
    case EConfigType::user_config:
        if (!m_pathUserConfig.empty()) return m_pathUserConfig;
        return "app_config.toml";
    default:
        return {};
    }
}


bool CAppSettings::EnableConfig(EConfigType eType)
{
    // Is running as main application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return false;

    switch (eType)
    {
    case EConfigType::platform_config:
        m_bPlatformConfig = true;
        if (!m_pathPlatformConfig.empty()) return true;
        m_pathPlatformConfig = "platform.toml";
        break;
    case EConfigType::vehicle_interface_config:
        m_bVehIfcConfig = true;
        if (!m_pathVehIfcConfig.empty()) return true;
        m_pathVehIfcConfig = "vehicle_ifc.toml";
        break;
    case EConfigType::vehicle_abstraction_config:
        m_bVehAbstrConfig = true;
        if (!m_pathVehAbstrConfig.empty()) return true;
        m_pathVehAbstrConfig = "vehicle_abstract.toml";
        break;
    case EConfigType::user_config:
        m_bUserConfig = true;
        if (!m_pathUserConfig.empty()) return true;
        m_pathUserConfig = "app_config.toml";
        break;
    default:
        return false;
    }

    return true;
}

bool CAppSettings::DisableConfig(EConfigType eType)
{
    // Is running as main application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return false;

    switch (eType)
    {
    case EConfigType::platform_config:
        m_pathPlatformConfig.clear();
        m_bPlatformConfig = true;
        break;
    case EConfigType::vehicle_interface_config:
        m_pathVehIfcConfig.clear();
        m_bVehIfcConfig = true;
        break;
    case EConfigType::vehicle_abstraction_config:
        m_pathVehAbstrConfig.clear();
        m_bVehAbstrConfig = true;
        break;
    case EConfigType::user_config:
        m_pathUserConfig.clear();
        m_bUserConfig = true;
        break;
    default:
        return false;
    }

    return true;
}

std::filesystem::path CAppSettings::GetUserConfigPath() const
{
    return m_pathUserConfig;
}

bool CAppSettings::SetUserConfigPath(const std::filesystem::path& rpathConfig)
{
    // Is running as server application? Then the user configuration is limited to a filename only.
    if (IsMainApplication() || IsMaintenanceApplication() || IsIsolatedApplication())
    {
        // Must be a filename only.
        if (!rpathConfig.has_filename() || rpathConfig.has_parent_path())
            return false;
    }

    // Assign the path
    m_pathUserConfig = rpathConfig;

    return true;
}

bool CAppSettings::RemoveUserConfigPath()
{
    // Is running as main application?
    if (!IsMainApplication()) return false;

    // Clear the path
    m_pathUserConfig.clear();

    return true;
}

sdv::sequence<sdv::u8string> CAppSettings::GetNames() const
{
    sdv::sequence<sdv::u8string> seqNames = {"app.instance_id", "console.info_level"};
    return seqNames;
}

sdv::any_t CAppSettings::Get(/*in*/ const sdv::u8string& ssAttribute) const
{
    if (ssAttribute == "app.instance_id")
        return sdv::any_t(m_uiInstanceID);
    if (ssAttribute == "console.info_level")
    {
        if (m_bSilent)
            return "silent";
        if (m_bVerbose)
            return "verbose";
        return "normal";
    }
    return {};
}

bool CAppSettings::Set(/*in*/ const sdv::u8string& /*ssAttribute*/, /*in*/ sdv::any_t /*anyAttribute*/)
{
    // Currently there are not setting attributes...
    return false;
}

uint32_t CAppSettings::GetFlags(/*in*/ const sdv::u8string& ssAttribute) const
{
    if (ssAttribute == "app.instance_id")
        return hlpr::flags<sdv::EAttributeFlags>(sdv::EAttributeFlags::read_only);
    if (ssAttribute == "console.info_level")
        return hlpr::flags<sdv::EAttributeFlags>(sdv::EAttributeFlags::read_only);
    return 0u;
}

void CAppSettings::Reset()
{
    m_eContextMode = sdv::app::EAppContext::no_context;
    m_pathLoggerModule.clear();
    m_ssLoggerClass.clear();
    m_ssProgramTag.clear();
    m_eSeverityFilter = sdv::core::ELogSeverity::info;
    m_eSeverityViewFilter = sdv::core::ELogSeverity::error;
    m_uiInstanceID = 0u;
    m_bSilent = false;
    m_bVerbose = false;
    m_pathRootDir.clear();
    m_pathInstallDir.clear();
    m_pathPlatformConfig.clear();
    m_pathVehIfcConfig.clear();
    m_pathVehAbstrConfig.clear();
    m_pathUserConfig.clear();
    m_bPlatformConfig = false;
    m_bVehIfcConfig = false;
    m_bVehAbstrConfig = false;
    m_bUserConfig = false;
}

CAppSettings& CAppSettingsService::GetAppSettings()
{
    return ::GetAppSettings();
}
