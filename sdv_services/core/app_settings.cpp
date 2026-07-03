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
#include <optional>

const std::string g_ssSettingsTemplate = R"toml(# Settings file
[Settings]
Version = )toml" + std::to_string(SDVFrameworkInterfaceVersion)
                                       + R"toml(

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
# Example:
#   AppConfig = "app_config.toml"
AppConfig = ""

# A list of zero or more listener definitions that should be instantiated during startup of the
# main application. If no listener definition is available, the default shared-memory listener 
# is being instantiated.
#
# Example:
#   [[Settings.Listener]]
#   Name = "SharedMemListener"
#   [Settings.Listerner.Provider]
#   Name = "DefaultSharedMemory"
#   [Settings.Listener.IpcChannel]
#   Name = "CHANNEL_1234"
#   Size = 1024
#
# A list of zero or more client connections that should be instantiated during startup.
#
# Example:
#   [[Settings.Connection]]
#   Name = "SharedMemConnection"
#   [Settings.Connection.Provider]
#   Name = "DefaultSharedMemory"
#   [Settings.Listener.IpcChannel]
#   Name = "CHANNEL_1234"

)toml";



CAppSettings& GetAppSettings()
{
    static CAppSettings app_settings;
    return app_settings;
}

CAppSettings::CAppSettings()
{
#ifdef _WIN32
    // Windows specific
    std::wstring ssPath(32768, '\0');

    MEMORY_BASIC_INFORMATION sMemInfo{};
    if (VirtualQuery(&m_pathFrameworkDir, &sMemInfo, sizeof(sMemInfo)))
    {
        DWORD dwLength = GetModuleFileNameW(reinterpret_cast<HINSTANCE>(sMemInfo.AllocationBase), ssPath.data(), 32767);
        ssPath.resize(dwLength);
        m_pathFrameworkDir = std::filesystem::path(ssPath).remove_filename();
    }

#elif __linux__
    // Read the maps file. It contains all loaded SOs.
    std::ifstream fstream("/proc/self/maps");
    std::stringstream sstreamMap;
    sstreamMap << fstream.rdbuf();
    std::string ssMap = sstreamMap.str();
    if (!ssMap.empty())
    {
        // Find the "core_services.sdv"
        size_t nPos = ssMap.find("core_services.sdv");
        if (nPos != std::string::npos)
        {
            size_t nEnd = nPos;

            // Find the start... runbackwards until the beginning of the line and remember the earliest occurance of a slash
            size_t nBegin = 0;
            while (nPos && ssMap[nPos] != '\n')
            {
                if (ssMap[nPos] == '/') nBegin = nPos;
                nPos--;
            }
            if (!nBegin) nBegin = nPos;

            // Return the path
            m_pathFrameworkDir = ssMap.substr(nBegin, nEnd - nBegin);
        }
}
#else
    #error The OS is not supported!
#endif
}

CAppSettings::~CAppSettings()
{}

bool CAppSettings::ProcessAppStartupConfig(const sdv::u8string& rssConfig)
{
    // Initialize the parameter map
    InitParamMap();

    // Process configuration
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
    std::string ssConsoleRerporting = tableStartupConfig.GetDirect("Console.Report").GetValueAsString();
    if (ssConsoleRerporting == "Silent")
        m_eConsoleReporting = EAppConsoleReporting::silent;
    if (ssConsoleRerporting == "Verbose")
        m_eConsoleReporting = EAppConsoleReporting::verbose;

    // Get the application-mode
    std::string ssApplication = tableStartupConfig.GetDirect("Application.Mode").GetValue();
    if (ssApplication.empty())
        ssApplication = "Standalone";
    if (ssApplication == "Standalone")
        m_eAppContextType = sdv::app::EAppContext::standalone;
    else if (ssApplication == "External")
        m_eAppContextType = sdv::app::EAppContext::external;
    else if (ssApplication == "Isolated")
        m_eAppContextType = sdv::app::EAppContext::isolated;
    else if (ssApplication == "Main")
        m_eAppContextType = sdv::app::EAppContext::main;
    else if (ssApplication == "Essential")
        m_eAppContextType = sdv::app::EAppContext::essential;
    else if (ssApplication == "Maintenance")
        m_eAppContextType = sdv::app::EAppContext::maintenance;
    else
    {
        if (!IsConsoleSilent())
            std::cerr << "ERROR: Failed to process startup config: invalid application-mode specified for core library: "
                      << ssApplication << std::endl;
        return false;
    }

    // Redirect monitoring? Only for main application
    if (m_eAppContextType == sdv::app::EAppContext::main)
        m_bRedirectMon = tableStartupConfig.GetDirect("Console.RedirectMon");

    // Report the outstanding error (if there is one...)
    if (!ssError.empty())
    {
        if (!IsConsoleSilent())
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

    // Get the severity level filter for the logger
    auto fnTranslateSevFilter = [this](const std::string& rssLogFilter, sdv::core::ELogSeverity eDefaultSeverity) -> std::optional<sdv::core::ELogSeverity>
    {
        if (rssLogFilter.empty()) return eDefaultSeverity;
        else if (rssLogFilter == "Trace") return sdv::core::ELogSeverity::trace;
        else if (rssLogFilter == "Debug") return sdv::core::ELogSeverity::debug;
        else if (rssLogFilter == "Info") return sdv::core::ELogSeverity::info;
        else if (rssLogFilter == "Warning") return sdv::core::ELogSeverity::warning;
        else if (rssLogFilter == "Error") return sdv::core::ELogSeverity::error;
        else if (rssLogFilter == "Fatal") return sdv::core::ELogSeverity::fatal;

        if (!IsConsoleSilent())
            std::cerr << "ERROR: Failed to process application log: invalid severity level filter: '" << rssLogFilter << "'"
                        << std::endl;
        return {};
    };
    sdv::core::ELogSeverity eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::error;
    switch (m_eAppContextType)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::external:
        eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::info;
        break;
    default:
        break;
    }
    auto optFilter = fnTranslateSevFilter(tableStartupConfig.GetDirect("LogHandler.Filter").GetValue(),
        sdv::core::ELogSeverity::info);
    if (!optFilter) return false;
    m_eSeverityFilter = *optFilter;
    optFilter = fnTranslateSevFilter(tableStartupConfig.GetDirect("LogHandler.ViewFilter").GetValue(),
        eLogDefaultViewSeverityFilter);
    if (!optFilter) return false;
    m_eSeverityViewFilter = *optFilter;

    // Get the optional instance ID.
    sdv::any_t anyInstanceID = tableStartupConfig.GetDirect("Application.Instance").GetValue();
    if (anyInstanceID) m_uiInstanceID = anyInstanceID;
    else
        m_uiInstanceID = 1000u;

    // Number of attempts to establish a connection to a running instance.
    sdv::toml::CNode nodeRetries = tableStartupConfig.GetDirect("Connections.Retries");
    if (nodeRetries)
    {
        m_uiConnectRetries = nodeRetries.GetValue();
        if (m_uiConnectRetries > 30)
            m_uiConnectRetries = 30;
        else if (m_uiConnectRetries < 3)
            m_uiConnectRetries = 3;
    }

    // Main and isolated apps specific information.
    if (IsMainApplication() || IsIsolatedApplication() || IsExternalApplication() || IsMaintenanceApplication())
    {
        // Get the optional installation directory - either from the startup parameters (relative to exe), or the environment
        // variable (relative to framework) or the exe when nothing else is specified.
        m_pathRootDir = tableStartupConfig.GetDirect("Application.InstallDir").GetValueAsPath();
        if (!m_pathRootDir.empty() && m_pathRootDir.is_relative())
            m_pathRootDir = GetExecDirectory() / m_pathRootDir;
        if (m_pathRootDir.empty())
        {
#ifdef _WIN32
            const wchar_t* szInstallDir = _wgetenv(L"SDV_COMPONENT_INSTALL");
            if (!szInstallDir) szInstallDir = _wgetenv(L"SDV_FRAMEWORK_RUNTIME");
            if (szInstallDir) m_pathRootDir = szInstallDir;
#elif defined __unix__
            const char* szInstallDir = getenv("SDV_COMPONENT_INSTALL");
            if (!szInstallDir) szInstallDir = getenv("SDV_FRAMEWORK_RUNTIME");
            if (szInstallDir) m_pathRootDir = std::filesystem::u8path(szInstallDir);
#else
    #error OS not supported!
#endif
        }
        if (!m_pathRootDir.empty() && m_pathRootDir.is_relative())
            m_pathRootDir = GetExecDirectory() / m_pathRootDir;
        if (m_pathRootDir.empty())
            m_pathRootDir = GetExecDirectory();

        // Derive installation directory from the root directory
        m_pathInstallDir = m_pathRootDir / std::to_string(m_uiInstanceID);
        try
        {
            std::filesystem::create_directories(m_pathInstallDir);
        }
        catch (const std::filesystem::filesystem_error& rexcept)
        {
            if (!IsConsoleSilent())
            {
                std::cerr << "Cannot create installation directory: " << m_pathInstallDir << std::endl;
                std::cerr << "  Reason: " << rexcept.what() << std::endl;
            }
            return false;
        }
    }

    // Set the default communication provider to shared memory. This might be overwritten by the settings.
    m_ssDefaultComProvider = "DefaultSharedMemory";

    // Read the settings... if existing.
    // Maintenance, main and isolated applications cannot load specific configs. The others can specify a configuration file, but
    // not auto-updateable.
    switch (m_eAppContextType)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::external:
    case sdv::app::EAppContext::maintenance:
        if (!LoadSettings()) return false;
        break;
    default:
        m_pathUserConfig = tableStartupConfig.GetDirect("Application.Config").GetValueAsPath();
        break;
    }

    // The applications use the SDV_FRAMEWORK_RUNTIME, SDV_COMPONENT_INSTALL, SDV_FRAMEWORK_DEV_TOOLS and SDV_FRAMEWORK_DEV_INCLUDE
    // environment variables to locate the core components. The core can, however, be executed in an executable from a different
    // location. If the environment variable contains a relative path, the path to the core components is relative to the
    // executable. Since the core can start other executables, the path would be relative to the other executables, which mights
    // reside in a different location than the executable starting the core. Change the paths in the environment variables to an
    // absolute path to prevent an invalid path to the core components.
    auto fnMakeAbsolutePathEnvVariable = [](const std::string& rssVariable)
    {
        std::filesystem::path pathEnv;
#ifdef _WIN32
        const wchar_t* szEnvPath = _wgetenv(sdv::MakeWString(rssVariable).c_str());
        if (szEnvPath)
            pathEnv = szEnvPath;
#elif defined __unix__
        const char* szEnvPath = getenv(rssVariable.c_str());
        if (szEnvPath)
            pathEnv     = szEnvPath;
#else
    #error The OS is not supported!
#endif
        if (!pathEnv.empty() && pathEnv.is_relative())
        {
            pathEnv = (GetExecDirectory() / pathEnv).lexically_normal();
#ifdef _WIN32
            // NOTE: In windows there are two environment variable stacks which need to be updated.
            std::ignore = SetEnvironmentVariable(sdv::MakeWString(rssVariable).c_str(), pathEnv.native().c_str());
            std::ignore = _wputenv((sdv::MakeWString(rssVariable) + L"=" + pathEnv.native()).c_str());
#elif defined __unix__
            std::ignore = setenv(rssVariable.c_str(), pathEnv.generic_u8string().c_str(), 1);
#else
    #error The OS is not supported!
#endif
        }
    };
    fnMakeAbsolutePathEnvVariable("SDV_FRAMEWORK_RUNTIME");
    fnMakeAbsolutePathEnvVariable("SDV_COMPONENT_INSTALL");
    fnMakeAbsolutePathEnvVariable("SDV_FRAMEWORK_DEV_TOOLS");
    fnMakeAbsolutePathEnvVariable("SDV_FRAMEWORK_DEV_INCLUDE");

    return true;
}

bool CAppSettings::LoadSettings()
{
    // Check for the proper context
    switch (m_eAppContextType)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::external:
    case sdv::app::EAppContext::maintenance:
        break;
    default:
        return true; // Not an error...
    }

    // If the template is not existing, this is not an error... use the default template
    std::string ssSettings;
    if (!std::filesystem::exists(m_pathInstallDir / "settings.toml"))
        ssSettings = g_ssSettingsTemplate;
    else
    {
        std::ifstream fstream(m_pathInstallDir / "settings.toml");
        ssSettings = std::string((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
        fstream.close();
    }

    try
    {
        // Read the configuration
        toml_parser::CParser parserSettings(ssSettings);

        // If there is no "Settings" table, this is not an e    rror...
        sdv::toml::CNodeCollection tableSettings(parserSettings.Root().GetNodeDirect("Settings"));
        if (!tableSettings) return true;

        // Check for the version
        uint32_t uiVersion = tableSettings.GetDirect("Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            if (!IsConsoleSilent())
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

        // Reset the default communication provider.
        m_ssDefaultComProvider.clear();

        // Read the listener configurations
        m_mapListeners.clear();
        sdv::toml::CNodeCollection arrayListeners = tableSettings.GetDirect("Listener");
        for (size_t nIndex = 0; nIndex < arrayListeners.GetCount(); nIndex++)
        {
            // Get the listener configuration
            sdv::toml::CNodeCollection tableListener = arrayListeners.Get(nIndex);
            if (!tableListener.IsValid())
            {
                if (!IsConsoleSilent()) std::cerr << "ERROR: Invalid listener configuration element." << std::endl;
                continue;
            }

            // Get the configuration elements
            std::string ssName = tableListener.GetDirect("Name").GetValueAsString();
            std::string ssProvider = tableListener.GetDirect("Provider").GetTOML();
            std::string ssProviderName = tableListener.GetDirect("Provider.Name").GetValueAsString();
            std::string ssChannelConfig = tableListener.GetDirect("IpcChannel").GetTOML();
            if (ssName.empty())
            {
                if (!IsConsoleSilent()) std::cerr << "ERROR: Missing listener name." << std::endl;
                continue;
            }
            if (ssProvider.empty() || ssProviderName.empty())
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Missing listener provider name." << std::endl;
                continue;
            }
            std::stringstream sstreamConfig;
            sstreamConfig << "[Provider]" << std::endl << ssProvider << std::endl << "[IpcChannel]" << std::endl <<
                ssChannelConfig << std::endl;
            if (!AddListenerConfig(ssName, sstreamConfig.str()))
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Could not add listener configuration (likely duplicate configuration)." << std::endl;
                continue;
            }

            // If running as main application, use the default config to extract the default communication provider.
            if (m_eAppContextType == sdv::app::EAppContext::main && ssName == "Default")
                m_ssDefaultComProvider = ssProviderName;
        }

        // Is there a default listener. If not, add one based on the default shared memory component.
        if (m_mapListeners.find("Default") == m_mapListeners.end())
            AddListenerConfig("Default", DefaultListenerConfig());

        // Read the connection configurations
        m_vecConnections.clear();
        sdv::toml::CNodeCollection arrayConnections = tableSettings.GetDirect("Connection");
        for (size_t nIndex = 0; nIndex < arrayConnections.GetCount(); nIndex++)
        {
            // Get the listener configuration
            sdv::toml::CNodeCollection tableConnection = arrayConnections.Get(nIndex);
            if (!tableConnection.IsValid())
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Invalid connection configuration element." << std::endl;
                continue;
            }

            // Get the configuration elements
            std::string ssName = tableConnection.GetDirect("Name").GetValueAsString();
            std::string ssProvider = tableConnection.GetDirect("Provider").GetTOML();
            std::string ssProviderName = tableConnection.GetDirect("Provider.Name").GetValueAsString();
            std::string ssChannelConfig = tableConnection.GetDirect("IpcChannel").GetTOML();
            if (ssName.empty())
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Missing connection name." << std::endl;
                continue;
            }
            if (ssProvider.empty() || ssProviderName.empty())
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Missing connection provider name." << std::endl;
                continue;
            }
            std::stringstream sstreamConfig;
            sstreamConfig << "[Provider]" << std::endl << ssProvider << std::endl << "[IpcChannel]" << std::endl
                << ssChannelConfig << std::endl;
            if (!AddConnectionConfig(ssName, sstreamConfig.str()))
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Could not add connection configuration (likely duplicate configuration)." << std::endl;
                continue;
            }

            // If not running as main application, use the default config to extract the default communication provider.
            if (m_eAppContextType != sdv::app::EAppContext::main && ssName == "Default")
                m_ssDefaultComProvider = ssProviderName;

        }

        // Is there a default connection (only valid for external applications). If not, add one based on the default shared memory.
        auto itConnection = std::find_if(m_vecConnections.begin(),
            m_vecConnections.end(),
            [](const auto& rprConnection) { return rprConnection.first == "Default"; });
        if (itConnection == m_vecConnections.end())
            AddConnectionConfig("Default", DefaultConnectionConfig(),
                m_vecConnections.size() ? m_vecConnections.front().first : "");

        // If there is no default communication provider, use the default shared memory provider.
        if (m_ssDefaultComProvider.empty())
            m_ssDefaultComProvider = "DefaultSharedMemory";
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        if (!IsConsoleSilent())
            std::cerr << "ERROR: Failed to parse application settings: " << rexcept.what() << std::endl;
        return false;
    }
    return true;
}

bool CAppSettings::SaveSettings()
{
    // Check for the proper context
    switch (m_eAppContextType)
    {
    case sdv::app::EAppContext::maintenance:
        break;
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    default:
        return true;    // Not an error...
    }

    // Protect against multiple write actions at the same time.
    ipc::named_mutex mtx("LockSdvAppSettings_" + std::to_string(m_uiInstanceID));
    // Warning of cppcheck for locking a local mutex, which doesn't have any effect. Since this is a named mutex between
    // applciations, the warning is not correct. Suppress warning.
    // cppcheck-suppress localMutex
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // If the template is not existing, create a default settings file...
    std::string ssSettings;
    bool bChangeDetected = false;
    if (!std::filesystem::exists(m_pathInstallDir / "settings.toml"))
    {
        bChangeDetected = true;
        ssSettings = std::move(g_ssSettingsTemplate);
        m_bUpdatePlatformConfig = true;
        m_bUpdateVehIfcConfig = true;
        m_bUpdateVehAbstrConfig = true;
        m_bUpdateUserConfig = true;
    }
    else
    {
        // Open the existing settings file
        std::ifstream fstream(m_pathInstallDir / "settings.toml");
        if (!fstream.is_open())
        {
            if (!IsConsoleSilent())
                std::cerr << "ERROR: Cannot open the application settings file." << std::endl;
            return false;
        }

        // Read the settings file
        ssSettings = std::string((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
        if (ssSettings.empty())
        {
            if (!IsConsoleSilent())
                std::cerr << "ERROR: Cannot read the application settings file; will use default." << std::endl;
            ssSettings = std::move(g_ssSettingsTemplate);
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
            if (!IsConsoleSilent())
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
            if (!IsConsoleSilent())
                std::cerr << "ERROR: Invalid 'Settings' table." << std::endl;
            return false;
        }

        uint32_t uiVersion = tableSettings.GetDirect("Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            if (!IsConsoleSilent())
                std::cerr << "ERROR: Invalid version of application settings file (expected version "
                            << SDVFrameworkInterfaceVersion << ", but available version " << uiVersion << ")" << std::endl;
            return false;
        }

        // Generic update config file function
        auto fnUpdateConfig = [&](const std::string &rssConfigKey, const std::filesystem::path& rpathConfigFile)
        {
            sdv::toml::CNode nodeConfig = tableSettings.GetDirect(rssConfigKey);
            if (nodeConfig.GetValue().empty())
            {
                bChangeDetected = true;
                if (nodeConfig)
                    nodeConfig.Delete();
                nodeConfig = tableSettings.AddValue(rssConfigKey, rpathConfigFile);
                if (!nodeConfig)
                {
                    if (!IsConsoleSilent()) std::cerr << "ERROR: Cannot insert the \"Settings." << rssConfigKey <<
                        "\" value; cannot process further." << std::endl;
                    return false;
                }
            }
            else if (nodeConfig.GetValue() != rpathConfigFile)
            {
                bChangeDetected = true;
                if (!nodeConfig.SetValue(rpathConfigFile))
                {
                    if (!IsConsoleSilent()) std::cerr << "ERROR: Cannot update the \"Settings." << rssConfigKey <<
                        "\" value; cannot process further." << std::endl;
                    return false;
                }
            }
            return true;
        };

        // Update the configuration path values.
        if (m_bUpdatePlatformConfig && !fnUpdateConfig("PlatformConfig", m_pathPlatformConfig)) return false;
        if (m_bUpdateVehIfcConfig && !fnUpdateConfig("VehIfcConfig", m_pathVehIfcConfig)) return false;
        if (m_bUpdateVehAbstrConfig && !fnUpdateConfig("VehAbstrConfig", m_pathVehAbstrConfig)) return false;
        if (m_bUpdateUserConfig && !fnUpdateConfig("AppConfig", m_pathUserConfig)) return false;

        // Update the listener configuration - do not add "Default" config
        sdv::toml::CNode nodeListeners = tableSettings.GetDirect("Listener");
        if (nodeListeners && nodeListeners.GetType() != sdv::toml::ENodeType::node_array)
        {
            nodeListeners.Delete();
            bChangeDetected = true;
        }

        // Check whether each listener in the array is also present in the map.
        // - if not or if different, remove it from the array
        // - remove it from the map if identical
        sdv::toml::CNodeCollection arrayListeners(nodeListeners);
        auto mapListenersCopy = m_mapListeners;
        size_t nListenerCount = arrayListeners.GetCount(); // Do not request anything from arrayListeners during the for
                                                           // loop. When deleting the last entry, the array is deleted as well,
                                                           // rendering the the interface pointer to void.
        for (size_t nIndex = 0; nIndex < nListenerCount; ++nIndex)
        {
            // Find the listener
            sdv::toml::CNode nodeListener = arrayListeners.Get(nIndex);

            // Check for a correct listener
            if (nodeListener.GetType() != sdv::toml::ENodeType::node_table)
            {
                nodeListener.Delete();
                nIndex--;
                nListenerCount--;
                bChangeDetected = true;
                continue;
            }

            // Check for an identical listener
            sdv::toml::CNodeCollection tableListener = nodeListener;
            if (!tableListener) break; // Should not occur

            // Get the configuration elements
            std::string ssStoredName = tableListener.GetDirect("Name").GetValueAsString();
            std::string ssStoredProvider = tableListener.GetDirect("Provider").GetTOML();
            std::string ssStoredChannelConfig = tableListener.GetDirect("IpcChannel").GetTOML();
            std::stringstream sstreamStoredConfig;
            sstreamStoredConfig << "[Provider]" << std::endl << ssStoredProvider << std::endl << "[IpcChannel]" << std::endl
                << ssStoredChannelConfig << std::endl;
            auto itListener = mapListenersCopy.find(ssStoredName);
            std::string ssConfig;
            if (itListener != mapListenersCopy.end())
                ssConfig = itListener->second;

            // Check for identical configuration
            if (!toml_parser::CompareEqual(ssConfig, sstreamStoredConfig.str()))
            {
                // Config is different the entry and try again.
                nodeListener.Delete();
                nIndex--;
                nListenerCount--;
                bChangeDetected = true;
                continue;
            }

            // The node and the map entry are identical. Remove this entry from the map.
            mapListenersCopy.erase(itListener);
        }

        // Add each leftover listener from the map to the array.
        for (const auto& rvtListener : mapListenersCopy)
        {
            // Skip the "Default" configuration
            if (rvtListener.first == "Default" && rvtListener.second == DefaultListenerConfig()) continue;
            sdv::toml::CNodeCollection tableListener = tableSettings.AddTableArray("Listener");
            
            // Add config
            tableListener.AddValue("Name", rvtListener.first);
            tableListener.AddTOML(rvtListener.second);
            bChangeDetected = true;
        }

        // Update the connection configuration - do not add "Default" config
        sdv::toml::CNode nodeConnections = tableSettings.GetDirect("Connection");
        if (nodeConnections && nodeConnections.GetType() != sdv::toml::ENodeType::node_array)
        {
            nodeConnections.Delete();
            bChangeDetected = true;
        }

        // Check whether each connection in the array is also present in the vector
        // - if not or if different, remove it from the array
        sdv::toml::CNodeCollection arrayConnections(nodeConnections);
        size_t nConnectionCount = arrayConnections.GetCount();  // Do not request anything from arrayConnections during the
                                                                // for-loop. When deleting the last entry, the array is deleted as
                                                                // well, rendering the the interface pointer to void.
        for (size_t nIndex = 0; nIndex < nConnectionCount; ++nIndex)
        {
            // Find the connection
            sdv::toml::CNode nodeConnection = arrayConnections.Get(nIndex);

            // Check for a correct connection
            if (nodeConnection.GetType() != sdv::toml::ENodeType::node_table)
            {
                nodeConnection.Delete();
                nIndex--;
                nConnectionCount--;
                bChangeDetected = true;
                continue;
            }

            // Check for an identical connection
            sdv::toml::CNodeCollection tableConnection = nodeConnection;
            if (!tableConnection) break; // Should not occur

            // Get the configuration elements
            std::string ssStoredName = tableConnection.GetDirect("Name").GetValueAsString();
            std::string ssStoredProvider = tableConnection.GetDirect("Provider").GetTOML();
            std::string ssStoredChannelConfig = tableConnection.GetDirect("IpcChannel").GetTOML();
            std::stringstream sstreamStoredConfig;
            sstreamStoredConfig << "[Provider]" << std::endl << ssStoredProvider << std::endl << "[IpcChannel]" << std::endl <<
                ssStoredChannelConfig << std::endl;
            auto itConnection = std::find_if(m_vecConnections.begin(), m_vecConnections.end(), [&](const auto& rprConnection)
                { return rprConnection.first == ssStoredName; });
            std::string ssConfig;
            if (itConnection != m_vecConnections.end())
                ssConfig = itConnection->second;

            // Check for identical configuration
            if (!toml_parser::CompareEqual(ssConfig, sstreamStoredConfig.str()))
            {
                // Config is different. Remove the entry and try again.
                nodeConnection.Delete();
                nIndex--;
                nConnectionCount--;
                bChangeDetected = true;
                continue;
            }
        }

        // Request the array once more. Could be deleted in the mean time.
        nodeConnections = tableSettings.GetDirect("Connection");
        arrayConnections = sdv::toml::CNodeCollection(nodeConnections);

        // Check for each connection in the vector if present at the correct position in the map
        // - if not correct position, take the TOML and remove and reinsert at the correct position
        // - if not available, add to the array
        //size_t nParentIndex = 0;
        size_t nOffset = 0;
        for (size_t nIndex = 0; nIndex < m_vecConnections.size(); ++nIndex)
        {
            // Skip the "Default" configuration
            if (m_vecConnections[nIndex].first == "Default" && m_vecConnections[nIndex].second == DefaultConnectionConfig())
            {
                nOffset++;
                continue;
            }


            // Get the connection name
            std::string ssName = m_vecConnections[nIndex].first;

            // Check index
            std::string ssToml;
            bool bFoundAndCorrect = false;
            for (size_t nIndex2 = nIndex - nOffset; nIndex2 < arrayConnections.GetCount(); ++nIndex2)
            {
                sdv::toml::CNodeCollection tableConnection(arrayConnections.Get(nIndex2));
                std::string ssStoredName = tableConnection.GetDirect("Name").GetValueAsString();
                if (ssStoredName == ssName)
                {
                    if (nIndex2 != nIndex - nOffset)
                        ssToml = tableConnection.GetTOML();
                    else
                        bFoundAndCorrect = true;
                    break;
                }
            }

            // Found and correct? Process next
            if (bFoundAndCorrect) continue;

            // Is there a TOML? Then add a connection with the TOML
            // Otherwise create a new connection
            if (!ssToml.empty())
                arrayConnections.InsertTOML("", ssToml);
            else
            {
                sdv::toml::CNodeCollection tableConnection;
                if(arrayConnections)
                    tableConnection = arrayConnections.InsertTable(arrayConnections.GetNodeNameByIndex(0), "Connection");
                else
                    tableConnection = tableSettings.InsertTableArray("", "Connection");
                tableConnection.AddValue("Name", ssName);
                tableConnection.AddTOML(m_vecConnections[nIndex].second);
            }

            bChangeDetected = true;
        }

        // Save the settings file if needed
        if (bChangeDetected)
        {
            std::ofstream fstream(m_pathInstallDir / "settings.toml", std::ios::trunc);
            if (!fstream.is_open())
            {
                if (!IsConsoleSilent())
                    std::cerr << "ERROR: Cannot write the application settings file." << std::endl;
                return false;
            }
            fstream << parserSettings.GenerateTOML();

            m_bUpdatePlatformConfig = false;
            m_bUpdateVehIfcConfig = false;
            m_bUpdateVehAbstrConfig = false;
            m_bUpdateUserConfig = false;
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        if (!IsConsoleSilent())
            std::cerr << "ERROR: Failed to parse application settings: " << rexcept.what() << std::endl;
        return false;
    }
    return true;
}

bool CAppSettings::IsMainApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::main;
}

bool CAppSettings::IsIsolatedApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::isolated;
}

bool CAppSettings::IsStandaloneApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::standalone;
}

bool CAppSettings::IsEssentialApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::essential;
}

bool CAppSettings::IsMaintenanceApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::maintenance;
}

bool CAppSettings::IsExternalApplication() const
{
    return m_eAppContextType == sdv::app::EAppContext::external;
}

void CAppSettings::SetContextType(sdv::app::EAppContext eContextType)
{
    m_eAppContextType = eContextType;
}

sdv::app::EAppContext CAppSettings::GetContextType() const
{
    return m_eAppContextType;
}

void CAppSettings::SetInstanceID(uint32_t uiID)
{
    m_uiInstanceID = uiID;
}

uint32_t CAppSettings::GetInstanceID() const
{
    return m_uiInstanceID;
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
    return m_eConsoleReporting == EAppConsoleReporting::silent;
}

bool CAppSettings::IsConsoleVerbose() const
{
    return m_eConsoleReporting == EAppConsoleReporting::verbose;
}

void CAppSettings::SetConsoleReporting(EAppConsoleReporting eReporting)
{
    m_eConsoleReporting = eReporting;
}

CAppSettings::EAppConsoleReporting CAppSettings::GetConsoleReporting() const
{
    return m_eConsoleReporting;
}

bool CAppSettings::RedirectMonitorToConsole() const
{
    return m_bRedirectMon;
}

std::filesystem::path CAppSettings::GetFrameworkDir() const
{
    return m_pathFrameworkDir;
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
    // Is running as main or maintenance application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return {};

    switch (eType)
    {
    case EConfigType::platform_config:
        if (!m_pathPlatformConfig.empty()) return m_pathPlatformConfig;
        return m_bUpdatePlatformConfig ? "" : "platform.toml";
    case EConfigType::vehicle_interface_config:
        if (!m_pathVehIfcConfig.empty()) return m_pathVehIfcConfig;
        return m_bUpdateVehIfcConfig ? "" : "vehicle_ifc.toml";
    case EConfigType::vehicle_abstraction_config:
        if (!m_pathVehAbstrConfig.empty()) return m_pathVehAbstrConfig;
        return m_bUpdateVehAbstrConfig ? "" : "vehicle_abstract.toml";
    case EConfigType::user_config:
        if (!m_pathUserConfig.empty()) return m_pathUserConfig;
        return m_bUpdateUserConfig ? "" : "app_config.toml";
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
        m_bUpdatePlatformConfig = true;
        if (!m_pathPlatformConfig.empty()) return true;
        m_pathPlatformConfig = "platform.toml";
        break;
    case EConfigType::vehicle_interface_config:
        m_bUpdateVehIfcConfig = true;
        if (!m_pathVehIfcConfig.empty()) return true;
        m_pathVehIfcConfig = "vehicle_ifc.toml";
        break;
    case EConfigType::vehicle_abstraction_config:
        m_bUpdateVehAbstrConfig = true;
        if (!m_pathVehAbstrConfig.empty()) return true;
        m_pathVehAbstrConfig = "vehicle_abstract.toml";
        break;
    case EConfigType::user_config:
        m_bUpdateUserConfig = true;
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
        m_bUpdatePlatformConfig = true;
        break;
    case EConfigType::vehicle_interface_config:
        m_pathVehIfcConfig.clear();
        m_bUpdateVehIfcConfig = true;
        break;
    case EConfigType::vehicle_abstraction_config:
        m_pathVehAbstrConfig.clear();
        m_bUpdateVehAbstrConfig = true;
        break;
    case EConfigType::user_config:
        m_pathUserConfig.clear();
        m_bUpdateUserConfig = true;
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
    switch (m_eAppContextType)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::external:
    case sdv::app::EAppContext::maintenance:
        // Must be a filename only.
        if (!rpathConfig.has_filename() || rpathConfig.has_parent_path())
            return false;
        break;
    default:
        break;
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

const std::string& CAppSettings::GetDefaultComProvider() const
{
    return m_ssDefaultComProvider;
}

sdv::sequence<sdv::u8string> CAppSettings::GetListeners() const
{
    // Is running as main or maintenance application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return {};

    // Add the listener names
    sdv::sequence<sdv::u8string> seqListeners;
    for (const auto& rvtListener : m_mapListeners)
        seqListeners.push_back(rvtListener.first);

    return seqListeners;
}

sdv::u8string CAppSettings::GetListenerConfig(/*in*/ const sdv::u8string& ssName) const
{
    // Is running as main or maintenance application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return {};

    // Find the listener
    auto itListener = m_mapListeners.find(ssName);
    if (itListener == m_mapListeners.end()) return {};
    return itListener->second;
}

bool CAppSettings::AddListenerConfig(/*in*/ const sdv::u8string& ssName, /*in*/ const sdv::u8string& ssConfig)
{
    // Is running as main or maintenance application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return {};

    // Add the listener
    m_mapListeners[ssName] = ssConfig;

    return true;
}

bool CAppSettings::RemoveListenerConfig(/*in*/ const sdv::u8string& ssName)
{
    // Is running as main or maintenance application?
    if (!IsMainApplication() && !IsMaintenanceApplication()) return false;

    // Erase a listener if existing
    return m_mapListeners.erase(ssName) ? true : false;
}

sdv::sequence<sdv::u8string> CAppSettings::GetConnections() const
{
    // Is running as main, external or maintenance application?
    if (!IsMainApplication() && !IsExternalApplication() && !IsMaintenanceApplication()) return {};

    // Add the connection names
    sdv::sequence<sdv::u8string> seqConnections;
    for (const auto& rprConnection : m_vecConnections)
        seqConnections.push_back(rprConnection.first);

    return seqConnections;
}

sdv::u8string CAppSettings::GetConnectionConfig(/*in*/ const sdv::u8string& ssName) const
{
    // Is running as main, external or maintenance application?
    if (!IsMainApplication() && !IsExternalApplication() && !IsMaintenanceApplication()) return {};

    // Find the connection
    auto itConnection = std::find_if(m_vecConnections.begin(), m_vecConnections.end(), [&](const auto& rprConnection)
        { return rprConnection.first == ssName; });
    if (itConnection == m_vecConnections.end()) return {};
    return itConnection->second;
}

bool CAppSettings::AddConnectionConfig(/*in*/ const sdv::u8string& ssName, /*in*/ const sdv::u8string& ssConfig,
    /*in*/ const sdv::u8string& ssInsertBefore /*= std::string()*/)
{
    // Is running as main, external or maintenance application?
    if (!IsMainApplication() && !IsExternalApplication() && !IsMaintenanceApplication()) return {};

    // Check for an existing connection; erase if existing.
    auto itConnection = std::find_if(m_vecConnections.begin(), m_vecConnections.end(), [&](const auto& rprConnection)
        { return rprConnection.first == ssName; });
    if (itConnection != m_vecConnections.end()) m_vecConnections.erase(itConnection);

    // Find the connection to insert before
    auto itInsertBefore = std::find_if(m_vecConnections.begin(), m_vecConnections.end(), [&](const auto& rprConnection)
        { return rprConnection.first == ssInsertBefore; });

    // Add the connection
    m_vecConnections.insert(itInsertBefore, std::make_pair(ssName, ssConfig));

    return true;
}

bool CAppSettings::RemoveConnectionConfig(/*in*/ const sdv::u8string& ssName)
{
    // Is running as main, external or maintenance application?
    if (!IsMainApplication() && !IsExternalApplication() && !IsMaintenanceApplication()) return false;

    // Check for an existing connection
    auto itConnection = std::find_if(m_vecConnections.begin(), m_vecConnections.end(), [&](const auto& rprConnection)
        { return rprConnection.first == ssName; });
    if (itConnection != m_vecConnections.end())
        m_vecConnections.erase(itConnection);
    else
        return false;
    return true;
}

uint32_t CAppSettings::GetConnectRetries() const
{
    return m_uiConnectRetries;
}

void CAppSettings::Reset()
{
    m_eAppContextType = sdv::app::EAppContext::no_context;
    m_pathLoggerModule.clear();
    m_ssLoggerClass.clear();
    m_ssProgramTag.clear();
    m_eSeverityFilter = sdv::core::ELogSeverity::info;
    m_eSeverityViewFilter = sdv::core::ELogSeverity::error;
    m_uiInstanceID = 0u;
    m_uiConnectRetries = 5u;
    m_eConsoleReporting = EAppConsoleReporting::normal;
    m_bRedirectMon        = false;
    m_pathRootDir.clear();
    m_pathInstallDir.clear();
    m_pathPlatformConfig.clear();
    m_pathVehIfcConfig.clear();
    m_pathVehAbstrConfig.clear();
    m_pathUserConfig.clear();
    m_bUpdatePlatformConfig = false;
    m_bUpdateVehIfcConfig = false;
    m_bUpdateVehAbstrConfig = false;
    m_bUpdateUserConfig = false;
    m_mapListeners.clear();
    m_vecConnections.clear();
    m_ssDefaultComProvider.clear();
}

std::string CAppSettings::DefaultListenerConfig() const
{
    return R"toml([Provider]
Name = "DefaultSharedMemory"
[IpcChannel]
Name = "DEFAULT_CHANNEL_)toml" + std::to_string(m_uiInstanceID) + R"toml("
Size = 1024
)toml";
}

std::string CAppSettings::DefaultConnectionConfig() const
{
    return R"toml([Provider]
Name = "DefaultSharedMemory"
[IpcChannel]
Name = "DEFAULT_CHANNEL_)toml" + std::to_string(m_uiInstanceID) + R"toml("
)toml";
}

CAppSettings& CAppSettingsService::GetAppSettings()
{
    return ::GetAppSettings();
}
