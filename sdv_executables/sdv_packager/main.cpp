#include "../../global/process_watchdog.h"
#include <support/mem_access.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/toml.h>
#include "../../global/exec_dir_helper.h"
#include "../../global/filesystem_helper.h"
#include "../../sdv_services/core/installation_manifest.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include "packager.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int iArgc, const wchar_t* rgszArgv[])
#else
extern "C" int main(int iArgc, const char* rgszArgv[])
#endif
{
    // Workaround for GCC to make certain that POSIX thread library is loaded before the components are loaded.
    // REASON: If the first call to a thread is done in a dynamic library, the application is already classified as single
    // threaded and a termination is initiated.
    // See: https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-leads-to-a-sigsev
    // NOTE EVE 27.05.2025: This task has been taken over by the process watchdog.
    CProcessWatchdog watchdog;

    // If not set, set the runtime location to the EXE directory.
    if (sdv::app::CAppControl::GetFrameworkRuntimeDirectory().empty())
        sdv::app::CAppControl::SetFrameworkRuntimeDirectory(GetExecDirectory());
    if (sdv::app::CAppControl::GetComponentInstallDirectory().empty())
        sdv::app::CAppControl::SetComponentInstallDirectory(GetExecDirectory());

    // Process the command line.
    CSdvPackagerEnvironment environment(iArgc, rgszArgv);

    // Print tool title
    if (!environment.Silent())
    {
        std::cout << "SDV Component Installation Package Utility" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    // Version requested?
    if (environment.Version() || environment.Verbose())
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl << std::endl;

    if (!environment.ArgError().empty() && !environment.Silent())
        std::cerr << "ERROR: " << environment.ArgError() << std::endl;

    if (environment.Help())
    {
        if (!environment.Silent())
        {
            if (environment.Error() != NO_ERROR)
                std::cout << std::endl;
            environment.ShowHelp();
        }
        return environment.Error();
    }
    if (environment.Error()) return CMDLN_ARG_ERR;

    // Report information about the settings
    environment.ReportInfo();

    // Anything to do?
    if (environment.OperatingMode() == CSdvPackagerEnvironment::EOperatingMode::none) return NO_ERROR;

    CPackager packager(environment);
    packager.Execute();
    if (!packager.ArgError().empty() && !environment.Silent())
        std::cerr << "ERROR: " << packager.ArgError() << std::endl;
    return packager.Error();

#if 0
    if (environment.CreateManifestOnly() && environment.Verbose())
        std::cout << "No file copy, creating manifest only..." << std::endl;

    if (!environment.Silent())
        std::cout << "Installation name: " << environment.InstallName() << std::endl;
    if (!environment.CreateManifestOnly())
    {
        if (environment.Verbose())
            std::cout << "Instance ID: " << environment.InstanceID() << std::endl;
    }

    std::filesystem::path pathTargetRoot = environment.TargetLocation();
    pathTargetRoot /= std::to_string(environment.InstanceID());
    if (environment.Verbose())
        std::cout << "Target root location: " << pathTargetRoot.generic_u8string() << std::endl;

    if (environment.Verbose())
    {
        if (environment.OperatingMode() == CSdvPackagerEnvironment::EOperatingMode::pack)
            std::cout << "Installing a package..." << std::endl;
        else
        {
            if (environment.OperatingMode() == CSdvPackagerEnvironment::EOperatingMode::direct_install)
                std::cout << "Direct installation of modules..." << std::endl;
            else if (environment.CreateManifestOnly())
                std::cout << "Creating an installation manifest..." << std::endl;
            else
                std::cout << "Creating an installation package..." << std::endl;
        }
    }

    std::filesystem::path pathInstallLocation = environment.TargetLocation();
    if (!environment.CreateManifestOnly())
    {
        pathInstallLocation /= std::to_string(environment.InstanceID());
        pathInstallLocation /= environment.InstallName();
        if (environment.Verbose())
            std::cout << "Install location: " << pathInstallLocation.generic_u8string() << std::endl;
    }

    if (!environment.ConfigPath().empty())
    {
        if (environment.ConfigPath().is_relative()) environment.ConfigPath() = pathTargetRoot / environment.ConfigPath();
        if (environment.Verbose())
            std::cout << "Config location: " << environment.ConfigPath().parent_path().generic_u8string() << std::endl;
    }

    if (environment.CreateManifestOnly())
        environment.InputLocation() = environment.TargetLocation();
    if (environment.Verbose())
        std::cout << "Source location: "
        << (environment.InputLocation().empty() ? "not provided" : environment.InputLocation().generic_u8string()) << std::endl;


    // ================= START OF OPERATION =================



    // Start the application control
    sdv::app::CAppControl appcontrol;
    std::string ssAppConfigTOML = R"code(
[Application]
Mode="Essential"
)code";
    ssAppConfigTOML += "Instance=" + std::to_string(environment.InstanceID());

    // Start the appcontrol
    if (!appcontrol.Startup(ssAppConfigTOML))
    {
        if (!environment.Silent())
            std::cerr << "ERROR: " << APP_CONTROL_STARTUP_ERROR_MSG << std::endl;
        return APP_CONTROL_STARTUP_ERROR;
    }

    // Create the target directory
    try
    {
        std::filesystem::create_directories(environment.TargetLocation());
    }
    catch (const std::filesystem::filesystem_error& /*rexcept*/)
    {
        if (!environment.Silent())
            std::cerr << "ERROR: " << CREATE_TARGET_DIR_ERROR_MSG << std::endl;
        return CREATE_TARGET_DIR_ERROR;
    }

    if (!environment.CreateManifestOnly())
    {
        // Remove existing installation directory
        try
        {
            if (std::filesystem::exists(pathInstallLocation))
                std::filesystem::remove_all(pathInstallLocation);
        }
        catch (const std::filesystem::filesystem_error& /*rexcept*/)
        {
            if (!environment.Silent())
                std::cerr << "ERROR: " << CANNOT_REMOVE_INSTALL_DIR_MSG << std::endl;
            return CANNOT_REMOVE_INSTALL_DIR;
        }

        // Create the installation directory
        try
        {
            std::filesystem::create_directories(pathInstallLocation);
        }
        catch (const std::filesystem::filesystem_error& /*rexcept*/)
        {
            if (!environment.Silent())
                std::cerr << "ERROR: " << CREATE_INSTALL_DIR_ERROR_MSG << std::endl;
            return CREATE_INSTALL_DIR_ERROR;
        }
    }

    // Create the config directory
    if (!environment.ConfigPath().empty())
    {
        try
        {
            std::filesystem::create_directories(environment.ConfigPath().parent_path());
        }
        catch (const std::filesystem::filesystem_error& /*rexcept*/)
        {
            if (!environment.Silent())
                std::cerr << "ERROR: " << CREATE_CONFIG_DIR_ERROR_MSG << std::endl;
            return CREATE_CONFIG_DIR_ERROR;
        }
    }

    // Add the installation manifest
    CInstallManifest manifest;
    manifest.Create(environment.InstallName());

    // Process all modules
    std::set<std::filesystem::path> setModules;
    for (std::filesystem::path& rpathSearchModule : environment.ModuleList())
    {
        if (environment.Verbose())
            std::cout << "Supplied module: " << rpathSearchModule.generic_u8string() << std::endl;
        if (rpathSearchModule.is_relative()) rpathSearchModule = environment.InputLocation() / rpathSearchModule;

        // Does the path have wildcards?
        bool bHasWildcards = rpathSearchModule.generic_u8string().find_first_of("*?") != std::string::npos;

        // Get a list of modules based on the module search criteria
        std::vector<std::filesystem::path> vecSearchedModules = FindFilesWithWildcards(rpathSearchModule);
        if (vecSearchedModules.empty() && !bHasWildcards)
        {
            if (!environment.Silent())
                std::cerr << "ERROR: " << CMDLN_SOURCE_FILE_ERROR_MSG << " (" << rpathSearchModule.generic_u8string() << ")" <<
                    std::endl;
            return CMDLN_SOURCE_FILE_ERROR;
        }

        // Process each module...
        for (std::filesystem::path& rpathModule : vecSearchedModules)
        {
            // Processed already?
            if (setModules.find(rpathModule) != setModules.end()) continue;
            setModules.insert(rpathModule);

            // Determine the relative path to from source location
            std::filesystem::path pathRelSource;
            try
            {
                pathRelSource = environment.InputLocation().empty() ?
                    rpathModule.filename() : std::filesystem::proximate(rpathModule, environment.InputLocation());
            } catch (std::filesystem::filesystem_error& /*rexcept*/)
            {}
            if (pathRelSource.empty() || pathRelSource.begin()->string() == "..")
            {
                if (!environment.Silent())
                    std::cerr << "ERROR: The source directory must be a parent of the module! The module will not be installed." <<
                        std::endl;
                continue;
            }

            if (!environment.Silent())
                std::cout << "Processing module: " << pathRelSource.generic_u8string() << std::endl;

            if (!std::filesystem::exists(rpathModule) || !std::filesystem::is_regular_file(rpathModule))
            {
                if (!environment.Silent())
                    std::cerr << "ERROR: Module cannot be found or is invalid!" << std::endl;
                continue;
            }

            if (!environment.CreateManifestOnly())
            {
                // Create target module path
                std::filesystem::path pathPargetModule = pathInstallLocation / pathRelSource;

                // Create the target directory if not existing
                try
                {
                    std::filesystem::create_directories(pathPargetModule.parent_path());
                }
                catch (std::filesystem::filesystem_error& /*rexcept*/)
                {
                    if (!environment.Silent())
                        std::cerr << "ERROR: Cannot create the module target directory!" << std::endl;
                    continue;
                }

                // Copy the file to there
                std::filesystem::copy(rpathModule, pathPargetModule);

                if (environment.Verbose())
                    std::cout << "Target module: " << pathPargetModule.generic_u8string() << std::endl;
            }

            // Add the installation manifest
            if (!manifest.AddModule(rpathModule))
            {
                if (!environment.Silent())
                    std::cerr << "ERROR: Cannot store the component manifest!" << std::endl;
                continue;
            }
        }
    }

    if (!manifest.Save(pathInstallLocation))
    {
        if (!environment.Silent())
            std::cerr << "ERROR: " << SAVE_INSTALL_MANIFEST_ERROR_MSG << std::endl;
        return SAVE_INSTALL_MANIFEST_ERROR;
    }
    if (environment.Verbose())
        std::cout << "Saved installation manifest: " << (pathInstallLocation / "install_manifest.toml").generic_u8string() <<
            std::endl;

    // Write config file
    if (!environment.ConfigPath().empty())
    {
        std::ofstream fstream(environment.ConfigPath());
        if (!fstream.is_open())
        {
            if (!environment.Silent())
                std::cerr << "ERROR: " << SAVE_CONFIG_FILE_ERROR_MSG << std::endl;
            return SAVE_CONFIG_FILE_ERROR;
        }
        fstream << "[Configuration]" << std::endl;
        fstream << "Version = " << SDVFrameworkInterfaceVersion << std::endl << std::endl;

        // Add each component that is a system object, device, basic service, complex service or an app...
        // Also add the modules that contain one of the other component types (utility, proxy or stub).
        std::map<std::filesystem::path, bool> mapModules;
        for (const auto& rsComponent : manifest.ComponentList())
        {
            //// Exclude the class from the configuration?
            //if (std::find(environment.ExcludeConfigClassList().begin(), environment.ExcludeConfigClassList().end(), rsComponent.ssClassName) !=
            //    environment.ExcludeConfigClassList().end()) continue;

            // Get or insert the module in the map
            auto itModule = mapModules.find(rsComponent.pathRelModule);
            if (itModule == mapModules.end())
            {
                auto prInsert = mapModules.insert(std::make_pair(rsComponent.pathRelModule, false));
                if (!prInsert.second) continue;
                itModule = prInsert.first;
            }

            // Add the component if it is a system object, device, basic service, complex service or an app.
            switch (rsComponent.eType)
            {
            case sdv::EObjectType::SystemObject:
            case sdv::EObjectType::Device:
            case sdv::EObjectType::BasicService:
            case sdv::EObjectType::ComplexService:
                fstream << "[[Component]]" << std::endl;
                fstream << "Path = \"" << rsComponent.pathRelModule.generic_u8string() << "\"" << std::endl;
                fstream << "Class = \"" << rsComponent.ssClassName << "\"" << std::endl;
                if (rsComponent.ssDefaultObjectName.empty())
                    fstream << "Name = \"" << rsComponent.ssClassName << "\"" << std::endl;
                else
                    fstream << "Name = \"" << rsComponent.ssDefaultObjectName << "\"" << std::endl;
                fstream << std::endl;
                itModule->second = true;    // Module was added through component.
                break;
            default:
                break;
            }
        }

        // Add all modules that were not added through a component already
        for (const auto& rvtModule : mapModules)
        {
            if (rvtModule.second) continue;     // Module added already
            fstream << "[[Module]]" << std::endl;
            fstream << "Path = \"" << rvtModule.first.generic_u8string() << "\"" << std::endl << std::endl;
        }
        fstream.close();

        if (environment.Verbose())
            std::cout << "Saved configuration file: " << environment.ConfigPath().generic_u8string() << std::endl;
    }

    // Write settings file
    if (environment.Settings())
    {
        // Read the existing settings
        std::filesystem::path pathSettings = pathTargetRoot / "settings.toml";
        std::list<std::string> lstSystemConfigs;
        std::string ssAppConfig;
        if (std::filesystem::exists(pathSettings))
        {
            std::ifstream fstream(pathSettings);
            if (fstream.is_open())
            {
                if (environment.Verbose())
                    std::cout << "Reading existing settings file..." << std::endl;
                std::string ssContent((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
                sdv::toml::CTOMLParser parser(ssContent);
                sdv::toml::CNode nodeVersion = parser.GetDirect("Settings.Version");
                if (nodeVersion.GetValue() != SDVFrameworkInterfaceVersion)
                {
                    if (!environment.Silent())
                        std::cout << "ERROR: " << SETTING_FILE_VERSION_INVALID_MSG << " No update taken place. (version " <<
                            static_cast<uint32_t>(nodeVersion.GetValue()) << " detected; version " <<
                            SDVFrameworkInterfaceVersion << " needed)." << std::endl;
                    return SETTING_FILE_VERSION_INVALID;
                }

                // Read the system configurations
                sdv::toml::CNodeCollection nodeSysConfigs = parser.GetDirect("Settings.SystemConfig");
                for (size_t nIndex = 0; nIndex < nodeSysConfigs.GetCount(); nIndex++)
                {
                    sdv::toml::CNode nodeSysConfig = nodeSysConfigs[nIndex];
                    if (nodeSysConfig)
                    {
                        lstSystemConfigs.push_back(nodeSysConfig.GetValue());
                        if (environment.Verbose())
                            std::cout << "Detected existing SystemConfig entry: " <<
                                static_cast<std::string>(nodeSysConfig.GetValue()) << std::endl;
                    }
                }

                // Read the app config
                sdv::toml::CNode nodeAppConfig = parser.GetDirect("Settings.AppConfig");
                if (nodeAppConfig)
                {
                    ssAppConfig = static_cast<std::string>(nodeAppConfig.GetValue());
                    if (environment.Verbose())
                        std::cout << "Detected existing AppConfig entry: " << ssAppConfig << std::endl;
                }
            }
        }

        // (Over)write existing settings file
        std::ofstream fstream(pathSettings);
        if (!fstream.is_open())
        {
            std::cerr << "ERROR: " << SAVE_SETTINGS_FILE_ERROR_MSG << std::endl;
            return SAVE_SETTINGS_FILE_ERROR;
        }

        // TODO... allow partial update of TOML file. Needs parser-updates first to be able to do this. See:
        // https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/580309

        const char* szSettingsTemplatePart1 = R"code(# Settings file
[Settings]
Version = 100

# The system config array can contain zero or more configurations that are loaded at the time
# the system ist started. It is advisable to split the configurations in:
#  platform config     - containing all the components needed to interact with the OS,
#                        middleware, vehicle bus, Ethernet.
#  vehicle interface   - containing the vehicle bus interpretation components like data link
#                        based on DBC and devices for their abstraction.
#  vehicle abstraction - containing the basic services
# Load the system configurations by providing the "SystemConfig" keyword as an array of strings.
# A relative path is relative to the installation directory (being "exe_location/instance_id").
#
# Example:
#   SystemConfig = [ "platform.toml", "vehicle_ifc.toml", "vehicle_abstract.toml" ]
#
)code";
        fstream << szSettingsTemplatePart1;
        if (!lstSystemConfigs.empty())
            fstream << "SystemConfig = [ ";
        for (auto it = lstSystemConfigs.begin(); it != lstSystemConfigs.end(); ++it)
        {
            if (environment.Verbose())
                std::cout << "Storing system config " << *it << " to the settings file." << std::endl;
            fstream << "\"" << *it << "\"";
            if (std::next(it) != lstSystemConfigs.end())
            {
                fstream << ", ";
            }
        }
        if (!lstSystemConfigs.empty())
            fstream << " ]" << std::endl;

        const char* szSettingsTemplatePart2 = R"code(
# The application config contains the configuration file that can be updated when services and
# apps are being added to the system (or being removed from the system). Load the application
# config by providing the "AppConfig" keyword as a string value. A relative path is relative to
# the installation directory (being "exe_location/instance_id").
#
# Example
#   AppConfig = "app_config.toml"
#
)code";
        fstream << szSettingsTemplatePart2;
        if (!environment.ConfigPath().empty())
        {
            if (!ssAppConfig.empty() && ssAppConfig != environment.ConfigPath().filename().generic_u8string())
            {
                if (!environment.Silent())
                    std::cout << "WARNING: The application config file in the settings is being updated from " <<
                    environment.ConfigPath().filename().generic_u8string() << " to " << ssAppConfig << std::endl;
            }
            ssAppConfig = environment.ConfigPath().filename().generic_u8string();
        }
        if (!ssAppConfig.empty())
        {
            if (environment.Verbose())
                std::cout << "Storing application config " << ssAppConfig << " to the settings file." << std::endl;
            fstream << "AppConfig = \"" << ssAppConfig << "\"" << std::endl;
        }
        fstream.close();

        if (environment.Verbose())
            std::cout << "Saved settings file: " << pathSettings.generic_u8string() << std::endl;
    }

    appcontrol.Shutdown();

    if (!environment.Silent())
        std::cout << std::endl << "Done..." << std::endl;

    return NO_ERROR;
#endif
    return NOT_IMPLEMENTED;
}
