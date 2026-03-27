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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "app_control.h"
#include "installation_manifest.h"
#include <interfaces/config.h>
#include <mutex>
#include <list>
#include <filesystem>
#include <vector>
#include <map>
#include "app_config_file.h"

// @cond DOXYGEN_IGNORE
// Components are installed using two procedures:
// Procedure 1: by loading a configuration file
//  mainly done when running standalone or in essential mode
// Procedure 2: by installing using an interface
//  mainly done when running as server
//
// The installation of components must be executed using a separated process to ensure the components are not interfering with
// the main process during installation.
// The component information is stored in the manifest, exposed through an exported function, and read during installation and
// instantiation.
// Installed files have attributes: creation and change dates, executable file flag, CRC, relative sub-directory, compoonent flag.
// The installation might need companion files to be installed in various relative sub-directories.
// @endcond

/**
 * @brief Get the location of the core_services.sdv.
 * @return Path to the directory containing the loaded core directory.
 */
std::filesystem::path GetCoreDirectory();

/**
 * @brief Application configuration service
 * @details In the configuration system objects, devices, basic services, complex services and apps are defined and will be started
 * suring the load process. The objects are loaded in this order (system objects first, apps last) unless dependencies define
 * a different order. Utilities, proxy and stub objects are not specified here, since they can be started 'on-the-fly' if needed.
 * Loading a configuration extends the current configuration. Saving a configuration includes all objects since the last
 * configuration not including the components present before the last load.
 *
 * For the main application there are several configuration files:
 *      - Platform configuration (OS support, middleware support, vehicle bus and Ethernet interface configuration - system objects)
 *      - Vehicle interface configuration (DBC, port assignments - devices)
 *      - Vehicle abstraction interface configuration (basic services)
 *      - Application configuration (complex services and applications)
 *
 * If components are added dynamically (or removed dynamically) they are added to and removed from the application configuration.
 * The configuration files are located in the installation directory.
 *
 * TODO: Add additional interface function: set component operation mode - config, running...
 *       When switching to config mode, switch the apps, complex services, basic services, devices and system objects (in that order) to config mode.
 *       When switching back to running mode, enable running mode for the system objects, devices, base services, complex services and apps (in that order).
 *
 */
class CAppConfig : public sdv::IInterfaceAccess, public sdv::core::IConfig, public sdv::installation::IAppInstall
{
public:
    CAppConfig() = default;

    // Interface map
    // Note: only add globally accessible interfaces here (which are not limited by the type of application).
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Load/reload the installation manifests for core, executable and user components.
     * @details The installation manifests contain the module and component class information that is used for finding component
     * classes. Standalone applications only load the core manifest. Server applications additionally load the manifest according to
     * their instance ID.
     * @return Returns 'true when successful; 'false' when not.
     */
    bool LoadInstallationManifests();

    /**
     * @brief Unload all manifest during shutdown.
     */
    void UnloadInstallatonManifests();

    /**
     * @brief Load the configuration files listed in the settings.
     * @details Load the configuration files stored in the settings file (for server applications) or provided through the startup
     * configuration (for local applications). For server applications, the system configurations are loaded and provided to the
     * repository for starting the components. After the system configuration, the user configuration is loaded. The module section
     * and path information in the configuration files is ignored (if available at all) for server applications. For local
     * applications there are no system configurations to load and only the user configuration is loaded which was provided during
     * startup. The loading of the modules precedes the loading and starting of the components.
     * @return Returns 'true' when the loading was successful. 'False' when not.
     */
    bool LoadAppConfigs();

    /**
     * @brief Save the configuration files listed in the settings.
     * @remarks When running in maintenance mode, all configurations are updated. In all other modes, only the
     * user application config is updated.
     */
    void SaveAppConfigs();

    /**
     * @brief Process the provided configuration by loading modules and creating objects/stubs/proxies defined in the
     * configuration string. Processing a configuration resets the baseline before processing. Overload of
     * sdv::core::IConfig::ProcessConfig.
     * @attention Configuration changes can only occur when the system is in configuration mode.
     * @param[in] ssContent The contents of the configuration file (TOML).
     * @return Returns a config process result enum value.
     */
    virtual sdv::core::EConfigProcessResult ProcessConfig(/*in*/ const sdv::u8string& ssContent) override;

    /**
     * @brief Read file pointed to by the provided file path and load modules and create objects/stubs/proxies as defined
     * in the configuration file. Loading a configuration resets the baseline before processing. Overload of
     * sdv::core::IConfig::LoadConfig.
     * @attention Configuration changes can only occur when the system is in configuration mode.
     * @param[in] ssConfigPath Path to the file containing the configuration (TOML). The path can be an absolute as well as a
     * relative path. In case a relative path is provided, the current directory is searched as well as all directories
     * supplied through the AddConfigSearchDir function.
     * @return Returns a config process result enum value.
     */
    virtual sdv::core::EConfigProcessResult LoadConfig(/*in*/ const sdv::u8string& ssConfigPath) override;

    /**
     * @brief Save a configuration file pointed to by the provided file path. All components are saved that were added after
     * the last baseline with the configuration specific settings. Overload of sdv::core::IConfig::SaveConfig.
     * @attention Configuration changes can only occur when the system is in configuration mode.
     * @param[in] ssConfigPath Path to the file containing the configuration (TOML). The path can be an absolute as well as a
     * relative path. In case a relative path is provided, the configuration is stored relative to the executable directory.
     * @return Returns 'true' on success (or no changes detected); 'false' otherwise.
     */
    virtual bool SaveConfig(/*in*/ const sdv::u8string& ssConfigPath) const override;

    /**
     * @brief Generate the configuration TOML string. Overload of sdv::core::IConfig::GenerateConfigString.
     * @return The generated configuration string.
     */
    virtual sdv::u8string GenerateConfigString() const override;

    /**
     * @brief Close the current configuration. Overload of sdv::core::IConfig::CloseConfig.
     * @details This will close und unload the components and modules from the current configuration as well as dependent
     * components that builds on top of the components being closed. Components that the current configuration depends on
     * are not closed.
     */
    virtual void CloseConfig() override;

    /**
     * @brief Add a search path to a folder where a config file can be found. Overload of
     * sdv::core::IConfig::AddDirectorySearchDir.
     * @param[in] ssDir Relative or absolute path to an existing folder.
     * @return Returns 'true' on success; 'false' otherwise.
     */
    virtual bool AddConfigSearchDir(/*in*/ const sdv::u8string& ssDir) override;

    /**
     * @brief Reset the configuration baseline. Overload of sdv::core::IConfig::ResetConfigBaseline.
     * @remarks When running as server, the config baseline is limited to the user configuration, which only consists of
     * complex services. For standalone setups, the configuration can also contain basic services and devices.
     * @details The configuration baseline determines what belongs to the current configuration. Any object being added
     * after this baseline will be stored in a configuration file.
     */
    virtual void ResetConfigBaseline() override;

    /**
     * @brief Make an installation onto the system. Overload of sdv::installation::IAppInstall::Install.
     * @details Make an installation with from component modules and supporting files. At least one component module must
     * be provided for this installation to be successful (component attribute flag must be set). Components are only
     * installed, not activated.
     * @remarks The system needs to run in configuration mode.
     * @param[in] ssInstallName Name of the installation. Must be unique to the system.
     * @param[in] seqFiles Files belonging to the installation. This could be component modules as well as supporting files.
     * @return Returns whether the installation was successful - installation name was unique and at least one loadable
     * component was provided.
     */
    virtual bool Install(/*in*/ const sdv::u8string& ssInstallName,
        /*in*/ const sdv::sequence<sdv::installation::SFileDesc>& seqFiles) override;

    /**
     * @brief Update an installation. Overload of sdv::installation::IAppInstall::Update.
     * @details Stops a component if the component is running, makes an update and if the component was running, starts
     * the component again.
     * @todo: Currently limited to complex services only.
     * @remarks The system needs to run in configuration mode.
     * @param[in] ssInstallName Name of the installation. Must be unique to the system.
     * @param[in] seqFiles Files belonging to the installation. This could be component modules as well as supporting files.
     * @return Returns whether the installation was successful - installation name was unique and at least one loadable
     * component was provided.
     */
    virtual bool Update(/*in*/ const sdv::u8string& ssInstallName,
        /*in*/ const sdv::sequence<sdv::installation::SFileDesc>& seqFiles) override;

    /**
     * @brief Uninstall of a previous installation. Overload of sdv::installation::IAppInstall::Uninstall.
     * @details Stops a component if the component is running and removes the files and modules.
     * @todo: Currently limited to complex services only.
     * @remarks The system needs to run in configuration mode.
     * @param[in] ssInstallName Installation name.
     * @return Returns whether the uninstallation was successful.
     */
    virtual bool Uninstall(/*in*/ const sdv::u8string& ssInstallName) override;

    /**
     * @brief Get a sequence of installations. Overload of sdv::installation::IAppInstall::GetInstallations.
     * @return The sequence with installations.
     */
    virtual sdv::sequence<sdv::u8string> GetInstallations() const override;

    /**
     * @brief Get the installed files from the installation. Overload of sdv::installation::IAppInstall::GetInstallationFiles.
     * @param[in] ssInstallName The installation to get the files for.
     * @return Sequence containing the file structures without the binary file content.
     */
    virtual sdv::sequence<sdv::installation::SFileDesc> GetInstallationFiles(/*in*/ const sdv::u8string& ssInstallName) const override;

    /**
     * @brief Find the module stored in the installation manifest (core, exe or user installations).
     * @param[in] rpathRelModule Reference to the path containing the relative path to a module.
     * @return Returns the full path if the module was found or an empty path when not.
     */
    std::filesystem::path FindInstalledModule(const std::filesystem::path& rpathRelModule);

    /**
     * @brief Find the module stored in the installation manifest (core, exe or user installations).
     * @param[in] rpathRelModule Reference to the path containing the relative path to a module.
     * @return Returns the module manifest if the module was found or an empty string when not.
     */
    std::string FindInstalledModuleManifest(const std::filesystem::path& rpathRelModule);

    /**
     * @brief Search for the installed component with the specific class name.
     * @details Find the first component containing a class with the specified name. The order of checking the installation manifest
     * is core-manifest, manifest in executable directory and manifest in supplied installation directory. For standalone
     * applications only the core and executable manifests are checked.
     * @remarks Components of system objects specified in the user installation are not returned.
     * @param[in] rssClass Reference to the class that should be searched for. The class is compared to the class name and the
     * default name in the manifest.
     * @return Optionally returns the component class information.
     */
    std::optional<sdv::SClassInfo> FindInstalledComponent(const std::string& rssClass) const;

    /**
     * @brief Remove any components that are mentioned in the manifest from the configurations available in the settings file.
     * @remarks Only available when running as maintenance or main application. If running as main application limited to removal
     * from the user configuration only.
     * @param[in] rManifest Reference to the manifest file containing the installation components.
     * @return Returns whether the removal was successful (also when there was nothing to remove) or not successful (when the
     * components were in one of the system configurations and the application is not running as maintenance application).
     */
    bool RemoveFromConfig(const CInstallManifest& rManifest);

private:
    /**
     * @brief For standalone applications, compose a proper absolute configuration file path based a relative path and the search
     * directories provided.
     * @remarks The path could point to a non-existing file (in case of the save configuration call). The parent directory needs
     * to exist.
     * @param[in] rpathConfigFile Reference to the path containing a relatice or absolute path name to the configuration file.
     * @return Returns a fully qualified path to the configuration file. Or an empty path in case of failure.
     */
    std::filesystem::path ComposeConfigPathStandaloneApp(const std::filesystem::path& rpathConfigFile) const;

    /**
     * @brief Add config folders of the core_services and the executable to the search paths if not done so already.
     */
    void AddCurrentPath();

    /**
     * @brief Installation structure
     */
    struct SInstallation
    {
        sdv::sequence<sdv::installation::SFileDesc>  seqFiles;       ///< Companion files
    };

    std::mutex                                      m_mtxSearchPaths;       ///< Access protection to directory list.
    std::list<std::filesystem::path>                m_lstSearchPaths;       ///< List of search directories.
    std::filesystem::path                           m_pathLastSearchDir;    ///< The last search directory to also save the file to.
    std::mutex                                      m_mtxInstallations;     ///< Access protection to the installations.
    CInstallManifest                                m_manifestCore;         ///< Install manifest for core components (main and isolated apps).
    CInstallManifest                                m_manifestExe;          ///< Install manifest for exe components (main and isolated apps).
    std::vector<CInstallManifest>                   m_vecUserManifests;     ///< Install manifests for user components (main and isolated apps).
    std::map<std::string, SInstallation>            m_mapInstallations;     ///< Installation map.
    std::vector<CAppConfigFile>                     m_vecSysConfigs;        ///< System configurations.
    CAppConfigFile                                  m_configUserConfig;     ///< User configuration file.
};

/**
 * @brief Return the application config class.
 * @return Reference to the application config class.
 */
CAppConfig& GetAppConfig();

/**
 * @brief App config service class.
 */
class CAppConfigService : public sdv::CSdvObject
{
public:
    /**
     * @brief Default constructor
     */
    CAppConfigService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetAppConfig())
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableAppConfigAccess(), 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IConfig, GetAppConfig())
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableAppInstallAccess(), 2)
        SDV_INTERFACE_SECTION(2)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::installation::IAppInstall, GetAppConfig())
        SDV_INTERFACE_DEFAULT_SECTION()
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("ConfigService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Get access to the application config.
     * @return Returns the one global instance of the application config.
     */
    static CAppConfig& GetAppConfig();

    /**
     * @brief When set, the application control access will be enabled.
     * @return Returns 'true' when the access to the application configuration is granted; otherwise returns 'false'.
     */
    static bool EnableAppConfigAccess();

    /**
    * @brief When set, the application install will be enabled.
    * @return Returns whether access to the application install is granted.
    */
    static bool EnableAppInstallAccess();
};
DEFINE_SDV_OBJECT(CAppConfigService)

#endif // !defined APP_CONFIG_H