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

#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <interfaces/app.h>
#include <support/component_impl.h>

/**
 * @brief Application settings class.
 * @details The application settings class is responsible for interpretation of the initial application startup configuration and
 * reading and writing the application main settings.
 * The startup configuration is a string using the TOML format and is defined as follows:
 * @code
 * # Optional use of customized log handler
 * [LogHandler]
 * Class = ""      # Component class name of a custom logger (optional)
 * Path = ""       # Component module path of a custom logger (optional)
 * Tag = ""        # Program tag to use instead of the name SDV_LOG_<pid>
 * Filter = ""     # Lowest severity filter to use when logging (Trace, Debug, Info, Warning, Error, Fatal). Default severity
 *                 # level filter is Info (meaning Debug and Trace messages are not being stored).
 * ViewFilter = "" # Lowest severity filter to use when logging (Trace, Debug, Info, Warning, Error, Fatal). Default severity
 *                 # level filter is Error (meaning Debug, Trace, Info and Warning messages are not being shown).
 *
 * # Application behavior definition
 * # Mode = "Standalone"  (default) app->no RPC + core services + additional configurations allowed
 * # Mode = "External"    app->RPC client only + local services + target service(s) --> connection information through listener
 * # Mode = "Isolated"    app->RPC client only + local services + target service(s) --> connection information needed
 * # Mode = "Main"        app->RPC server + core services --> access key needed
 * # Mode = "Essential"   app->local services + additional configurations allowed
 * # Mode = "Maintenance" app->RPC client only + local services + maintenance service --> connection information needed + access key
 * # Instance = 1234
 * [Application]
 * Mode = "Main"
 * Instance = 1234         # Optional instance ID to be used with main and isolated applications. Has no influence on other
 *                         # applications. Default instance ID is 1000. The connection listener is using the instance ID to allow
 *                         # connections from an external application to the main application. Furthermore, the instance ID is
 *                         # used to locate the installation of SDV components. The location of the SDV components is relative to
 *                         # the executable (unless a target directory is supplied) added with the instance and the installations:
 *                         # &lt;exe_path&gt;/&lt;instance&gt;/&lt;installation&gt;
 * InstallDir = "./test"   # Optional custom installation directory to be used with main and isolated applications. Has no
 *                         # influence on other applications. The default location for installations is the location of the
 *                         # executable. Specifying a different directory will change the location of installations to
 *                         # &lt;install_directory&gt;/&lt;instance&gt;/&lt;installation&gt;
 *                         # NOTE The directory of the core library and the directory of the running executable are always added
 *                         # to the system if they contain an installation manifest.
 *
 * # Optional configuration that should be loaded (only for local applications).
 * Config = "abc.toml"
 *
 * #Console output
 * [Console]
 * Report = "Silent"       # Either "Silent", "Normal" or "Verbose" for no, normal or extensive messages.
 *
 * # Search directories
 * @endcode
 *
 * @todo Add config ignore list (e.g. platform.toml, vehicle_ifc.toml and vehicle_abstract.toml).
 * @todo Add dedicated config (rather than standard config) as startup param.
 * 
 * The settings file is a TOML file with the following structure:
 * @code
 * [Settings]
 * Version = 100
 * 
 * # The system configuration is divided into:
 * #  platform config     - containing all the components needed to interact with the OS,
 * #                        middleware, vehicle bus, Ethernet.
 * #  vehicle interface   - containing the vehicle bus interpretation components like data link
 * #                        based on DBC and devices for their abstraction.
 * #  vehicle abstraction - containing the vehicle abstraction services
 * # The configuration files are loaded exactly in that order, allowing the vehicle interface to
 * # depend on the platform and the vehicle abstraction to depend on the vehicle interface.
 * # The configurations are loaded if the PlatformConfig, VehIfcConfig and VehAbstrConfig keywords
 * # are present and describe a valid configuration file.
 * # A relative path is relative to the installation directory (being "exe_location/instance_id").
 * #
 * # Example:
 * #   PlatformConfig = "platform.toml"
 * #   VehIfcConfig = "vehicle_ifc.toml"
 * #   VehAbstrConfig = "vehicle_abstract.toml"
 * #
 * PlatformConfig = ""
 * VehIfcConfig = ""
 * VehAbstrConfig = ""
 * 
 * # The application config contains the configuration file that can be updated when services and
 * # apps are being added to the system (or being removed from the system). Load the application
 * # config by providing the "AppConfig" keyword as a string value. A relative path is relative to
 * # the installation directory (being "exe_location/instance_id").
 * #
 * # Example
 * #   AppConfig = "app_config.toml"
 * AppConfig = ""
 * @endcode
 */
class CAppSettings : public sdv::IInterfaceAccess, public sdv::app::IAppContext, public sdv::IAttributes
{
public:
    /**
     * @brief Constructor
     */
    CAppSettings();

    /**
     * @brief Destructor
     */
    ~CAppSettings();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::app::IAppContext)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Process the application starrtup configuration.
     * @param[in] rssConfig Reference to the configuration content (TOML format).
     * @return Returns 'true' when processing was successful; false when not.
     */
    bool ProcessAppStartupConfig(const sdv::u8string& rssConfig);

    /**
     * @brief Load the application settings file.
     * @attention Only works if the application is running in main, isolation or maintenance mode.
     * @remarks When there is no settings file, this is not an error. Default settings will be assumed.
     * @return Returns whether the loading was successful.
     */
    bool LoadSettingsFile();

    /**
     * @brief Save the application settings file (or create when not existing yet).
     * @attention Only works if the application is running in main, isolation or maintenance mode.
     * @return Returns whether the saving was successful.
     */
    bool SaveSettingsFile();

    /**
     * @brief Return whether the current application is the main application.
     * @return Returns 'true' when the current application is the main application; otherwise returns 'false'.
     */
    bool IsMainApplication() const;

    /**
     * @brief Return whether the current application is an isolated application.
     * @return Returns 'true' when the current application is an isolated application; otherwise returns 'false'.
     */
    bool IsIsolatedApplication() const;

    /**
     * @brief Return whether the current application is a standalone application.
     * @return Returns 'true' when the current application is a standalone application; otherwise returns 'false'.
     */
    bool IsStandaloneApplication() const;

    /**
     * @brief Return whether the current application is an essential application.
     * @return Returns 'true' when the current application is an essential application; otherwise returns 'false'.
     */
    bool IsEssentialApplication() const;

    /**
     * @brief Return whether the current application is a maintenance application.
     * @return Returns 'true' when the current application is a maintenance application; otherwise returns 'false'.
     */
    bool IsMaintenanceApplication() const;

    /**
     * @brief Return whether the current application is an external application.
     * @return Returns 'true' when the current application is an external application; otherwise returns 'false'.
     */
    bool IsExternalApplication() const;

    /**
     * @brief Return the application context mode. Overload of sdv::app::IAppContext::GetContextType.
     * @return The context mode.
     */
    sdv::app::EAppContext GetContextType() const override;

    /**
     * @brief Return the core instance ID. Overload of sdv::app::IAppContext::GetContextType.
     * @details Get the instance. If not otherwise specified, the current instance depends on whether the application is running
     * as main or isolated application, in which case the instance is 1000. In all other cases the instance is 0. An instance
     * ID can be supplied through the app startup configuration.
     * @return The core instance ID.
     */
    uint32_t GetInstanceID() const override;

    /**
     * @brief Return the number of retries to establish a connection. Overload of sdv::app::IAppContext::GetRetries.
     * @return Number of retries.
     */
    uint32_t GetRetries() const override;

    /**
     * @brief Get the class name of a logger service, if specified in the application startup configuration.
     * @return The logger class name.
     */
    std::string GetLoggerClass() const;

    /**
     * @brief Get the logger service module path, if specified in the application startup configuration.
     * @return The logger module path.
     */
    std::filesystem::path GetLoggerModulePath() const;

    /**
     * @brief Get the logger program tag, if specified in the application startup configuration.
     * @return The logger program tag.
     */
    std::string GetLoggerProgramTag() const;

    /**
     * @brief Get the logger severity filter, if specified in the application startup configuration.
     * @return The logger severity filter.
     */
    sdv::core::ELogSeverity GetLoggerSeverityFilter() const;

    /**
     * @brief Get the console reporting severity file, if specified in the application startup configuration.
     * @return The console reporting severity filter value.
     */
    sdv::core::ELogSeverity GetConsoleSeverityFilter() const;

    /**
     * @brief Should the console output be silent?
     * @return Returns whether the console output is silent.
     */
    bool IsConsoleSilent() const;

    /**
     * @brief Should the console output be verbose?
     * @return Returns whether the verbose console output is activated.
     */
    bool IsConsoleVerbose() const;

    /**
     * @brief Get the root directory for the application.
     * @remarks Is only valid when used in main, isolated and maintenance applications.
     * @return The location of root directory.
     */
    std::filesystem::path GetRootDir() const;

    /**
     * @brief Get the installation directory of user components (root directory / instance ID).
     * @remarks Is only valid when used in main, isolated and maintenance applications.
     * @return The location of the installation director.
     */
    std::filesystem::path GetInstallDir() const;

    /**
     * @brief Get a vector with the system configuration paths (relative to the installation directory) as specified in the
     * settings file.
     * @return The vector of system configuration paths.
     */
    std::vector<std::filesystem::path> GetSystemConfigPaths() const;

    /**
     * @brief Configuration type
     */
    enum class EConfigType
    {
        platform_config,                ///< Contains the platform configuration
        vehicle_interface_config,       ///< Contains the vehicle interface configuration
        vehicle_abstraction_config,     ///< Contains the vehicle abstraction configuration
        user_config                     ///< Contains the user configuration
    };

    /**
     * @brief Get the stored or default configuration path name.
     * @attention Setting a path is only valid when running as main application.
     * @return The path name dependent on the configuration type. If no path name was configured, the default path name is returned.
     */
    std::filesystem::path GetConfigPath(EConfigType eType) const;

    /**
     * @brief Enable a configuration file in the application settings.
     * @attention The configuration file needs to be located at the root directory of the instance installation.
     * @attention Setting a path is only valid when running as main application.
     * @param[in] eType The configuration type to set the path for.
     * @return Returns 'true' when adding the config file path was successful (or when the path already exists in the settings);
     * otherwise returns 'false'.
     */
    bool EnableConfig(EConfigType eType);

    /**
     * @brief Disable and remove a configuration file from the application settings.
     * @attention The configuration file needs to be located at the root directory of the instance installation.
     * @attention Removing a psth is only valid when running as main application.
     * @param[in] eType The configuration type to remove the path from.
     * @return Returns 'true' when the removal was successful.
     */
    bool DisableConfig(EConfigType eType);

    /**
     * @brief Get the path to the user copnfiguration (relative to the installation directory) as specified in the settings file.
     * @return Path to the user configuration.
     */
    std::filesystem::path GetUserConfigPath() const;

    /**
     * @brief Set the user configuration file into the application settings. If a user configuration file is stored already in the
     * settings file, the configuration file name is replaced by the new configuration file name.
     * @remarks Setting the user config path is only valid for local applications. For server application, the path is managed by
     * enabling the configuration.
     * @param[in] rpathConfig Reference to the path containing the configuration file name.
     * @return Returns 'true' when setting the config file path was successful; otherwise returns 'false'.
     */
    bool SetUserConfigPath(const std::filesystem::path& rpathConfig);

    /**
     * @brief Remove the user configuration file from the application settings.
     * @attention Removing the path is only valid when running as main application.
     * @return Returns 'true' when the removal was successful.
     */
    bool RemoveUserConfigPath();

    /**
     * @brief Get a sequence with the available attribute names. Overload of sdv::IAttributes::GetNames.
     * @return The sequence of attribute names.
     */
    virtual sdv::sequence<sdv::u8string> GetNames() const override;

    /**
     * @brief  Get the attribute value. Overload of sdv::IAttributes::Get.
     * @param[in] ssAttribute Name of the attribute.
     * @return The attribute value or an empty any-value if the attribute wasn't found or didn't have a value.
     */
    virtual sdv::any_t Get(/*in*/ const sdv::u8string& ssAttribute) const override;

    /**
     * @brief Set the attribute value. Overload of sdv::IAttributes::Set.
     * @param[in] ssAttribute Name of the attribute.
     * @param[in] anyAttribute Attribute value to set.
     * @return Returns 'true' when setting the attribute was successful or 'false' when the attribute was not found or the
     * attribute is read-only or another error occurred.
     */
    virtual bool Set(/*in*/ const sdv::u8string& ssAttribute, /*in*/ sdv::any_t anyAttribute) override;

    /**
     * @brief Get the attribute flags belonging to a certain attribute. Overload of sdv::IAttributes::GetFlags.
     * @param[in] ssAttribute Name of the attribute.
     * @return Returns the attribute flags (zero or more EAttributeFlags flags) or 0 when the attribute could not be found.
     */
    virtual uint32_t GetFlags(/*in*/ const sdv::u8string& ssAttribute) const override;

    /**
     * @brief Reset the settings after a shutdown.
     */
    void Reset();

private:
    sdv::app::EAppContext       m_eContextMode = sdv::app::EAppContext::no_context; ///< The application is running as...
    uint32_t                    m_uiInstanceID = 0u;            ///< Instance number.
    uint32_t                    m_uiRetries = 0u;               ///< Number of retries to establish a connection.
    std::string                 m_ssLoggerClass;                ///< Class name of a logger service.
    std::filesystem::path       m_pathLoggerModule;             ///< Module name of a custom logger.
    std::string                 m_ssProgramTag;                 ///< Program tag to use when logging.
    sdv::core::ELogSeverity     m_eSeverityFilter = sdv::core::ELogSeverity::info;      ///< Severity level filter while logging.
    sdv::core::ELogSeverity     m_eSeverityViewFilter = sdv::core::ELogSeverity::error; ///< Severity level filter while logging.
    bool                        m_bSilent = false;              ///< When set, no console reporting takes place.
    bool                        m_bVerbose = false;             ///< When set, extensive console reporting takes place.
    std::filesystem::path       m_pathRootDir;                  ///< Location of user component root directory.
    std::filesystem::path       m_pathInstallDir;               ///< Location of user component installations (root with instance).
    std::filesystem::path       m_pathPlatformConfig;           ///< The platform configuration from the settings file.
    std::filesystem::path       m_pathVehIfcConfig;             ///< The vehicle interface configuration from the settings file.
    std::filesystem::path       m_pathVehAbstrConfig;           ///< The vehicle abstraction configuration from the settings file.
    std::filesystem::path       m_pathUserConfig;               ///< The user configuration from the settings file.
    bool                        m_bPlatformConfig = false;      ///< Platform config was explicitly enabled/disabled.
    bool                        m_bVehIfcConfig = false;        ///< Vehicle interface config was explicitly enabled/disabled.
    bool                        m_bVehAbstrConfig = false;      ///< Vehicle abstraction config was explicitly enabled/disabled.
    bool                        m_bUserConfig = false;          ///< User config was explicitly enabled/disabled.
};

/**
 * @brief Return the application settings class.
 * @return Reference to the application settings.
 */
CAppSettings& GetAppSettings();

/**
 * @brief App settings service class.
 */
class CAppSettingsService : public sdv::CSdvObject
{
public:
    CAppSettingsService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY_MEMBER(sdv::IAttributes, GetAppSettings())
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("AppSettingsService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Get access to the application settings.
     * @return Returns the one global instance of the application config.
     */
    static CAppSettings& GetAppSettings();
};
DEFINE_SDV_OBJECT(CAppSettingsService)

#endif // !defined APP_SETTINGS_H
