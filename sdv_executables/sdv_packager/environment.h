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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <functional>
#include "../../global/cmdlnparser/cmdlnparser.h"
#include "../error_msg.h"
#include <interfaces/config.h>
#include "../../sdv_services/core/app_config_file.h"
#include "../../sdv_services/core/toml_parser/parser_toml.h"
#include <support/toml.h>

/**
 * @brief Compare whether two characters are identical when both are converted to lower case.
 * @remarks Cannot deal with Unicode letters comprising of more than one characters.
 * @param[in] cLeft First character
 * @param[in] cRight Second character
 * @return The case independent equality of the characters.
 */
inline bool ichar_equals(char cLeft, char cRight)
{
    return std::tolower(static_cast<unsigned char>(cLeft)) ==
        std::tolower(static_cast<unsigned char>(cRight));
}

/**
 * @brief Compare for case-independent equality.
 * @param[in] rssLeft Reference to the left string.
 * @param[in] rssRight Reference to the right string.
 * @return The case-independent equality of the strings.
 */
inline bool iequals(const std::string& rssLeft, const std::string& rssRight)
{
    return std::equal(rssLeft.begin(), rssLeft.end(), rssRight.begin(), rssRight.end(), ichar_equals);
}

/**
 * @brief Environment access class.
 */
class CSdvPackagerEnvironment
{
public:
    /**
     * @brief Component vector containing the component class names and optionally the component instantiation names.
     */
    using CComponentVector = std::vector<std::pair<std::string, std::string>>;

    /**
     * @brief Environment exception triggered on error during command line processing.
     */
    struct SEnvironmentException : std::exception
    {
        /**
         * @brief Constructor
         * @param[in] nCodeParam Exception code
         * @param[in] rssDescriptionParam Reference to the exception description text.
         */
        SEnvironmentException(int nCodeParam, const std::string& rssDescriptionParam) :
            nCode(nCodeParam), ssDescription(rssDescriptionParam)
        {}

        /**
         * @brief Returns the explanatory string.
         * @return Pointer to the explanatory string as C-string.
         */
        virtual const char* what() const noexcept
        {
            return ssDescription.c_str();
        }

        int             nCode = 0;          ///< Exception code
        std::string     ssDescription;      ///< Exception description
    };

    /**
     * @brief Operation mode the packager utility is running in.
     */
    enum class EOperatingMode
    {
        none,               ///< No mode selected
        pack,               ///< Packing operation (default)
        install,            ///< Installation
        direct_install,     ///< Direct installation.
        configure,          ///< Configure
        uninstall,          ///< Remove an installed package.
        verify,             ///< Verify integrity
        show,               ///< Show content
    };

    /**
     * @brief Show information mask (multiple can be combined).
     */
    enum class EShowMask
    {
        all = 0x00ff,                   ///< Show all information
        info = 0x0001,                  ///< Show package information
        modules = 0x0010,               ///< Show modules
        components = 0x0020,            ///< Show components
        console = 0x01000000,           ///< Show as console output (extended form)
        console_simple = 0x03000000,    ///< Show as console output (simple form)
        xml = 0x10000000,               ///< Provide as XML
        json = 0x20000000,              ///< Provide as JSON
    };

    /**
     * @brief Product version structure
     */
    struct SVersion
    {
        size_t  nMajor = 0;     ///< Major version number
        size_t  nMinor = 0;     ///< Minor version number
        size_t  nPatch = 0;     ///< Patch level
    };

    /**
     * @brief Module structure containing the module search string and relative target path.
     * @details The module structure is used for module selection during packing and direct installation - hence the search string
     * and potential target directory.
     */
    struct SModule
    {
        std::string             ssSearchString;     ///< Module path; can contain wildcards and regular expression (therefore of
                                                    ///< type std::string).
        std::filesystem::path   pathRelTarget;      ///< Relative target path. Default is empty.
    };

    /**
     * @brief Default constructor
     */
    CSdvPackagerEnvironment();

    /**
     * @brief Constructor with program arguments.
     * @param[in] rvecArgs Reference to the vector with program arguments
     */
    CSdvPackagerEnvironment(const std::vector<std::string>& rvecArgs);

    /**
     * @brief Constructor with program arguments.
     * @tparam TCharType Character type.
     * @param[in] nArgs The amount of arguments.
     * @param[in] rgszArgs Array of arguments.
     */
    template <typename TCharType>
    CSdvPackagerEnvironment(size_t nArgs, const TCharType** rgszArgs);

    /**
     * @brief Help was requested on the command line.
     * @return Returns 'true' when help was requested. Otherwise returns 'false'.
     */
    bool Help() const;

    /**
     * @brief Show command line help.
     */
    void ShowHelp() const;

    /**
     * @brief Report information about the command and the settings dependable on the reporting flag.
     */
    void ReportInfo() const;

    /**
     * @brief Return whether silent mode was requested for the console output.
     * @return Returns whether the silent mode is switched on.
     */
    bool Silent() const;

    /**
     * @brief Return whether verbose mode was requested for the console output.
     * @return Returns whether the verbose mode is switched on.
     */
    bool Verbose() const;

    /**
     * @brief Version information was requested on the command line.
     * @return Returns 'true' when version info was requested. Otherwise returns 'false'.
     */
    bool Version() const;

    /**
     * @brief Create a manifest during the creation phase.
     * @return Returns whether a manifest should be the only output of the creation.
     */
    bool CreateManifestOnly() const;

    /**
     * @brief The operating mode the packager is running in.
     * @return Returns the current operating mode based on the command line settings.
     */
    EOperatingMode OperatingMode() const;

    /**
     * @brief Keep the current structure during the installation. This requires a source input directory to be set.
     * @return Returns whether a structure should be kept relative to the source directory.
     */
    bool KeepStructure() const;

    /**
     * @brief List of modules or packages to create or install respectively.
     * @return Returns a reference to the variable containing the list of modules or packages. Could contain wildcards and regular
     * expression strings.
     */
    const std::vector<SModule>& ModuleList() const;

    /**
     * @brief List of files to configure the system.
     * @return Returns a reference to the variable containing the list of files. Could contain wildcards and regular expression 
     * strings.
     */
    const std::vector<std::string>& ConfigFileList() const;

    /**
     * @brief Get the package path. Available for installation, uninstallation, verification and content showing commands.
     * @return Returns a reference to the variable containing the package path.
     */
    const std::filesystem::path& PackagePath() const;

    /**
     * @brief During package creation, the source location of the modules and files to create the package for (or directly install).
     * During package installation, source location of the package.
     * @return Returns a reference to the variable containing the source location.
     */
    const std::filesystem::path& SourceLocation() const;

    /**
     * @brief During package creation, the output location of the package file.
     * @return Returns a reference to the variable containing the output location.
     */
    const std::filesystem::path& OutputLocation() const;

    /**
     * @brief During package installation, base path to the package target location.
     * @return Returns a reference to the variable containing the target location.
     */
    const std::filesystem::path& TargetLocation() const;

    /**
     * @brief During package installation, base path to the package root location (including instance).
     * @return Returns a reference to the variable containing the root location.
     */
    const std::filesystem::path& RootLocation() const;

    /**
     * @brief During package installation, base path to the package installation location (including package name).
     * @return Returns a reference to the variable containing the installation location.
     */
    const std::filesystem::path& InstallLocation() const;

    /**
     * @brief Set when the local flag has been supplied on the command line.
     * @return Returns whether local system operation is enabled.
    */
    bool Local() const;

    /**
     * @brief The instance to use to install the package to.
     * @return The instance ID.
     */
    uint32_t InstanceID() const;

    /**
     * @brief Update if an older version has been found.
     * @return Returns whether the update flag was specified.
     */
    bool Update() const;

    /**
     * @brief Overwrite any version that might be installed before.
     * @return Returns whether the overwrite flags was specified.
     */
    bool Overwrite() const;

    /**
     * @brief Return the configuration file to create or update during the installation.
     * @param[out] rvecComponents Reference to a list of component names to add to the configuration. If no components are listed, all components
     * are to be added (if a configuration file was supplied).
     * @return Returns a reference to the variable containing the path to the configuration file or empty when no file was supplied.
     */
    const std::filesystem::path& LocalConfigFile(CComponentVector& rvecComponents) const;

    /**
     * @brief During package installation, the configuration file locations during local operation.
     * @return Returns a reference to the variable containing the vector with configuration locations.
     */
    const std::vector<std::filesystem::path>& LocalConfigLocations() const;

    /**
     * @brief Add the components to the user configuration.
     * @remarks Only complex services will be inserted.
     * @param[out] rvecComponents Reference to a list of component names to add to the configuration. If no components are listed, all components
     * are to be added (if a the function returns true).
     * @return Returns true if a configuration update is requested; false otherwise.
     */
    bool InsertIntoUserConfig(CComponentVector& rvecComponents) const;

    /**
     * @brief Add the components to the platform configuration.
     * @param[out] rvecComponents Reference to a list of component names to add to the configuration. If no components are listed, all components
     * are to be added (if a the function returns true).
     * @return Returns true if a configuration update is requested; false otherwise.
     */
    bool InsertIntoPlatformConfig(CComponentVector& rvecComponents) const;

    /**
     * @brief Add the components to the vehicle interface configuration.
     * @param[out] rvecComponents Reference to a list of component names to add to the configuration. If no components are listed, all components
     * are to be added (if a the function returns true).
     * @return Returns true if a configuration update is requested; false otherwise.
     */
    bool InsertIntoVehicleInterfaceConfig(CComponentVector& rvecComponents) const;

    /**
     * @brief Add the components to the vehicle abstraction configuration.
     * @param[out] rvecComponents Reference to a list of component names to add to the configuration. If no components are listed, all components
     * are to be added (if a the function returns true).
     * @return Returns true if a configuration update is requested; false otherwise.
     */
    bool InsertIntoVehicleAbstractionConfig(CComponentVector& rvecComponents) const;

    /**
     * @brief Get the object parameter map for the provided object instance.
     * @param[in] rssObjectInstance Reference to the object instance name to get the parameter map for.
     * @return Returns a reference to the parameter vector. It is guaranteed that the parameters are unique. Can be empty when
     * there are no parameters for the object instance.
     */
    const TParameterVector& ObjectParameters(const std::string& rssObjectInstance) const;

    /**
     * @brief The installation name for the package creation.
     * @return Returns a reference to the variable containing the installation name.
     */
    const std::string& InstallName() const;

    /**
     * @brief Get the product name. If not explicitly provided, this will be the installation name.
     * @return Returns a reference to the variable containing the product name.
     */
    const std::string& ProductName() const;

    /**
     * @brief Get the product description.
     * @return Returns a reference to the variable containing the product description or empty if no description was provided.
     */
    const std::string& Description() const;

    /**
     * @brief Get the product author.
     * @return Returns a reference to the variable containing the product author or empty if no author was provided.
     */
    const std::string& Author() const;

    /**
     * @brief Get the product author's address.
     * @return Returns a reference to the variable containing the product author's address or empty if no address was provided.
     */
    const std::string& Address() const;

    /**
     * @brief Get the product copyrights.
     * @return Returns a reference to the variable containing the product copyrights or empty if no copyrights were provided.
     */
    const std::string& Copyrights() const;

    /**
     * @brief Get the package version string (format: major.minor.patch).
     * @return Returns a reference to the variable containing the package version string or empty if no version string was provided.
     */
    const std::string& PackageVersion() const;

    /**
     * @brief Get the show flags.
     * @return The shhow flags.
    */
    uint32_t ShowFlags() const;

    /**
     * @brief Check whether the show flags contain certain flags.
     * @param[in] eMask Show flag mask to check for.
     * @return Returns true when the flags contaion exactly the provided mask.
    */
    bool CheckShowFlag(EShowMask eMask) const;

    /**
     * @brief Was a commandline parse error or a logical error with command line arguments detected?
     * @return Returns the result of the command line parsing. If no error occurs, NO_ERROR is returned.
     */
    int Error() const;

    /**
     * @brief Parse error that might have occurred.
     * @return Returns a reference to the variable containing the error text.
     */
    const std::string& ArgError() const;

private:
    /**
     * @brief Split a configuration string in path and components.
     * @details A configuration string can consist of &lt;path&gt;+component,component,component... The path might be optional. If
     * missing, the string starts with '+'. The components might also be optional. If missing, the '+' is also not needed.
     * @param[in] rssInput Reference to the configuration string.
     * @param[out] rpath Reference to the path to be returned.
     * @param[out] rvecComponents Reference to the vector containing the component class names and optionally component
     * instantiation names (or empty when the name should be extracted from the component class).
     */
    void SplitConfigString(const std::string& rssInput, std::filesystem::path& rpath, CComponentVector& rvecComponents);

    /**
     * @brief Process the parsed environment settings.
     * @param[in] rvecCommands Commands list from the command line.
     * @return Returns true on successful processing, false when an error has occurred.
     */
    bool ProcessCommandLine(const std::vector<std::string>& rvecCommands);

    CCommandLine                m_cmdln;                            ///< Command line parsing class.
    bool                        m_bHelp = false;                    ///< Set when help was requested on the command line.
    bool                        m_bSilent = false;                  ///< Set when silent mode is enabled.
    bool                        m_bVerbose = false;                 ///< Set when verbose reporting mode is enabled.
    bool                        m_bVersion = false;                 ///< Set when version information was requested.
    bool                        m_bKeepStructure = false;           ///< When set, keep the source directory structure.
    bool                        m_bCreateManifestOnly = false;      ///< Create a manifest only.
    bool                        m_bLocal = false;                   ///< When set, the installation is on a local instance only.
    bool                        m_bUpdate = false;                  ///< Update an existing installation.
    bool                        m_bOverwrite = false;               ///< Overwrite an existing installation.
    EOperatingMode              m_eOperatingMode = EOperatingMode::none; ///< Operating mode of the packager utility.
    std::vector<SModule>        m_vecModules;                       ///< List of modules (module search strings).
    std::vector<std::string>    m_vecConfigFiles;                   ///< List of configuration files (file search strings).
    std::filesystem::path       m_pathSourceLocation;               ///< Path to the input location.
    std::filesystem::path       m_pathOutputLocation;               ///< Path to the output location.
    std::filesystem::path       m_pathTargetLocation;               ///< Path to the target location (at installation).
    std::filesystem::path       m_pathRootLocation;                 ///< Target root location (includes the instance for server
                                                                    ///< location).
    std::filesystem::path       m_pathInstallLocation;              ///< Target install location (root location with installation
                                                                    ///< name).
    std::filesystem::path       m_pathPackage;                      ///< Path to the package during installation, uninstallation,
                                                                    ///< integrity checking andcontent showing. 
    uint32_t                    m_uiInstanceID = 1000u;             ///< Instance number (optional).
    std::string                 m_ssInstallName;                    ///< Installation name.
    std::string                 m_ssProductName;                    ///< Product name (default is package name).
    std::string                 m_ssDescription;                    ///< Product description
    std::string                 m_ssAuthor;                         ///< Author
    std::string                 m_ssAddress;                        ///< Company address
    std::string                 m_ssCopyrights;                     ///< Copyright
    std::string                 m_ssPackageVersion;                 ///< Package version string
    std::filesystem::path       m_pathConfigLocal;                  ///< Configuration file path (local only).
    CComponentVector            m_vecConfigLocalComponents;         ///< List of components to add to the configuration file.
    std::vector<std::filesystem::path> m_vecLocalConfigDirs;        ///< List of directories to scan for the component to uninstall.
    CComponentVector            m_vecUserConfigComponents;          ///< User configuration (server only).
    CComponentVector            m_vecPlatformConfigComponents;      ///< Platform configuration (server only).
    CComponentVector            m_vecVehIfcConfigComponents;        ///< Vehicle interface configuration (server only).
    CComponentVector            m_vecVehAbstrConfigComponents;      ///< Vehicle abstraction configuration (server only).
    std::map<std::string, TParameterVector> m_mapParameters;        ///< Map with parameters associated to their instances.
    bool                        m_bInsertIntoUserConfig = false;    ///< When set, insert the components into the user config
                                                                    ///< (server only).
    bool                        m_bInsertIntoPlatformConfig = false;///< When set, insert the components into the platform config
                                                                    ///< (server only).
    bool                        m_bInsertIntoVehIfcConfig = false;  ///< When set, insert the components into the vehicle interface
                                                                    ///< config (server only).
    bool                        m_bInsertIntoVehAbstrConfig = false;///< When set, insert the components into the vehicle
                                                                    ///< abstraction config (server only).
    uint32_t                    m_uiShowFlags = 0;                  ///< Show package information bitmask.
    int                         m_nError = NO_ERROR;                ///< Error code after processing the command line.
    std::string                 m_ssArgError;                       ///< Error text after processing the command line.
};

template <typename TCharType>
CSdvPackagerEnvironment::CSdvPackagerEnvironment(size_t nArgs, const TCharType** rgszArgs) :
    m_cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character))
{
    m_cmdln.PrintMaxWidth(80);
    m_cmdln.PrintSyntax(false);

    try
    {
        std::vector<std::string> vecCommands;

        // COMMANDS (selective argument groups 1-7):
        //   PACK                       Pack modules and files into an installation package.
        //                              Usage: sdv_packager PACK <package name> <files> <options>
        //   INSTALL                    Install a package into the system.
        //                              Usage: sdv_packager INSTALL <package path> <options>
        //   DIRECT_INSTALL             Directly install modules and files into the system (without the creation of a package).
        //                              Usage: sdv_packager DIRECT_INSTALL <package name> <files> <options>
        //   CONFIGURE                  Install a configuration into the system.
        //                              Usage: sdv_packager CONFIGURE <config files> <options>
        //   UNINSTALL                  Remove an installation from the system.
        //                              Usage: sdv_packager UNINSTALL <package name> <options>
        //   VERIFY                     Verify the consistency and the integrity of an installation package.
        //                              Usage: sdv_packager VERIFY <package path> <options>
        //   SHOW                       Show package information.
        //                              Usage: sdv_packager SHOW <ALL|INFO|MODULES|COMPONENTS> <package path> <options>
        m_cmdln.DefineDefaultArgument(vecCommands, "COMMAND <...> <options>");

        // ARGUMENT SELECTION GROUP #0 - General options:
        //     -?, --help              Show help
        //     -s, --silent            Silent mode
        //     -v, --verbose           Verbose mode
        //     --version               Show version
        auto& rArgHelpDef = m_cmdln.DefineOption("?", m_bHelp, "Show help", true, 0, 1, 2, 3, 4, 5, 6, 7);
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = m_cmdln.DefineOption("s", m_bSilent, "Do not show any information on STDOUT. Not compatible with "
            "'--verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = m_cmdln.DefineOption("v", m_bVerbose, "Provide verbose information. Not compatible with "
            "'--silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        m_cmdln.DefineSubOption("version", m_bVersion, "Show version information.");

        // ARGUMENT SELECTION GROUP #2 & #3, #4 & #5: General options during installation/uninstallation
        //     -L, --local                  Local installation
        //     --instance<id>               Instance ID to use for installation
        auto& rLocalArgDef = m_cmdln.DefineOption("L", m_bLocal, "Target the local system.", true, 2, 3, 4, 5);
        rLocalArgDef.AddSubOptionName("local");
        auto& rInstance = m_cmdln.DefineSubOption("instance", m_uiInstanceID, "The instance ID of the SDV server instance when not "
            "targeting the local system (default ID is 1000).", true, 2, 3, 4, 5);

        // ARGUMENT SELECTION GROUP #1 - Packing:
        //     -O<path>                     Optional destination location
        //     --signature<path>            Path to the file to use to sign the package (not implemented yet)
        m_cmdln.DefineGroup("Package creation");
        m_cmdln.DefineOption("O", m_pathOutputLocation, "The output location (optional, default is current directory).", true, 1);

        // ARGUMENT SECLECTION GROUP #1 & #3 - Packing and direct installation:
        //     -I<path>                     Optional source location
        //     --keep_structure             Maintain original directory structure
        //     --create_manifest_only       Create a manifest file only
        //     --set_product<string>        Set the product name (if not identical to installation name).
        //     --set_description<string>    Set the package description
        //     --set_author<string>         Set author string
        //     --set_address<string>        Set address string (multi-line)
        //     --set_copyright<string>      Set copyright string
        //     --set_version<string>        Set product version string (also use for updates).
        m_cmdln.DefineOption("I", m_pathSourceLocation, "The source location (optional, default is current directory).", true,
            1, 3);
        m_cmdln.DefineSubOption("keep_structure", m_bKeepStructure, "Maintain the directory structure within the package. In "
            "combination with the source location (current directory or directory provided by '-I' option).", true, 1, 3);
        m_cmdln.DefineSubOption("create_manifest_only", m_bCreateManifestOnly, "Create manifest, but does not copy module files "
            "from specified location. If provided, the manifest will be stored in the output directory (current directory or "
            "directory provided by '-O' option).", true, 1);
        m_cmdln.DefineSubOption("set_product", m_ssProductName, "Set the product name (default is package name).", true, 1, 3);
        m_cmdln.DefineSubOption("set_description", m_ssDescription, "Set the package description.", true, 1, 3);
        m_cmdln.DefineSubOption("set_author", m_ssAuthor, "Set author name.", true, 1, 3);
        m_cmdln.DefineSubOption("set_address", m_ssAddress, "Set author address information.", true, 1, 3);
        m_cmdln.DefineSubOption("set_copyright", m_ssCopyrights, "Set copyright information.", true, 1, 3);
        m_cmdln.DefineSubOption("set_version", m_ssPackageVersion, "Set package version (needed to allow updates).", true, 1, 3);

        // ARGUMENT SELECTION GROUP #2 & #3 & #5- Installation from package and direct installation:
        //     -T<path>                     Optional target location
        //     -P, --update                 Use update if a previous package with older version has been found
        //     -W, --overwrite              Use overwrite if a previous package has been found
        //     --verify_signed              Verify whether the package is signed and if not return an error (not implemented)
        m_cmdln.DefineGroup("Installation / uninstallation");
        auto& rTargetLoc = m_cmdln.DefineOption("T", m_pathTargetLocation, "The target location for package installation. For "
            "server based installations, this is the root location containing the instance ID sub-directories with the "
            "installations. For local installations, this is the real target directory for installing the content.",
            true, 2, 3, 5);
        auto& rArgDefUpdate = m_cmdln.DefineOption("P", m_bUpdate, "Update the installation if an older version is existing.",
            true, 2, 3);
        rArgDefUpdate.AddSubOptionName("update");
        auto& rArgDefOverwrite = m_cmdln.DefineOption("W", m_bOverwrite, "Overwrite the installation if an installation is "
            "existing (regardless of the installed version).", true, 2, 3);
        rArgDefOverwrite.AddSubOptionName("overwrite");

        // Configuration
        // ARGUMENT SELECTION GROUP #2 & #3, #4 & #5 - Update from package and directly and uninstallation:
        //     --config_dir<paths...>       Local instance only; directory to configuration files to update.
        m_cmdln.DefineGroup("Configuration update");
        auto& rConfigDir = m_cmdln.DefineSubOption("config_dir", m_vecLocalConfigDirs, "One or more configuration directories "
            "to scan for components being updated. Use with local systems only ('--local' option).", true, 2, 3, 4, 5);

        //     --config_file<path>[+component[=name]+....]]                     Local instance only; configuration file.
        //     --user_config[+component[=name]+....]]                           Server instance only; user configuration.
        //     --platform_config[+component[=name]+....]]                       Server instance only; platform config.
        //     --interface_config[+component[=name]+....]]                      Server instance only; vehicle interface config.
        //     --abstract_config[+component[=name]+....]]                       Server instance only; vehicle abstraction config.
        std::string ssConfigFileLocal;
        auto& rConfigFile = m_cmdln.DefineSubOption("config_file", ssConfigFileLocal, "Update a user configuration file. For "
            "usage, see explanatory text above. Use with local systems only ('--local' option).", true, 2, 3);
        std::string ssUserConfigServer, ssPlatformConfigServer, ssInterfaceConfigServer, ssAbstrConfigServer;
        auto& rPlatformConfig = m_cmdln.DefineSubOption("platform_config", ssPlatformConfigServer, "Update the platform "
            "configuration defining which components of this installation are used to interact with the hardware platform. For "
            "usage, see explanatory text above. Use on server only.", true, 2, 3);
        auto& rInterfaceConfig = m_cmdln.DefineSubOption("interface_config", ssInterfaceConfigServer, "Update the vehicle "
            "interface configuration defining which components of this installation represent the vehicle interface. For usage, "
            "see explanatory text above. Use on server only.", true, 2, 3);
        auto& rAbstrConfig = m_cmdln.DefineSubOption("abstract_config", ssAbstrConfigServer, "Update the vehicle abstraction "
            "configuration defining which components of this installation represent an abstracted vehicle to the application "
            "components. For usage, see explanatory text above. Use on server only.", true, 2, 3);
        auto& rUserConfig = m_cmdln.DefineSubOption("user_config", ssUserConfigServer, "Update the user configuration defining "
            "which components of this installation need to be instantiated automatically. User configuration can only contain "
            "complex services. For usage, see explanatory text above. Use on server only.", true, 2, 3);
        m_cmdln.DefineSubOption("config_file", m_pathConfigLocal, "Update a user configuration file. usage, see explanatory text "
            "above. Use with local systems only ('--local' option).", true, 4);
        m_cmdln.DefineSubOption("platform_config", m_bInsertIntoPlatformConfig, "Update the platform "
            "configuration defining which components of this installation are used to interact with the hardware platform. Use "
            "on server only.", true, 4);
        m_cmdln.DefineSubOption("interface_config", m_bInsertIntoVehIfcConfig, "Update the vehicle "
            "interface configuration defining which components of this installation represent the vehicle interface. Use on server "
            "only.", true, 4);
        m_cmdln.DefineSubOption("abstract_config", m_bInsertIntoVehAbstrConfig, "Update the vehicle abstraction "
            "configuration defining which components of this installation represent an abstracted vehicle to the application "
            "components. Use on server only.", true, 4);
        m_cmdln.DefineSubOption("user_config", m_bInsertIntoUserConfig, "Update the user configuration defining "
            "which components of this installation need to be instantiated automatically. User configuration can only contain "
            "complex services. Use on server only.", true, 4);

        //      --parameters<component_name>:<param>=<value>[,<param>=<value>...]   Parameters for an added component.
        //      --param_file<TOML_file>                                             Parameters from a file for an added component.
        std::vector<std::string> vecParametersRaw;
        m_cmdln.DefineSubOption("parameters", vecParametersRaw, "Parameters for a component instance. For  usage, see explanatory "
            "text above.", true, 2, 3);
        std::vector<std::filesystem::path> vecParamFiles;
        m_cmdln.DefineSubOption("param_file", vecParamFiles, "Parameters for one or more component instances located in a "
            "parameter TOML file. For usage, see explanatory text above.", true, 2, 3);

        // ARGUMENT SELECTION GROUP #6 - Package verififcation
        m_cmdln.DefineGroup("Verification options");
        //     --verify_signed              Verify whether the package is signed and if not return an error (not implemented)

        // ARGUMENT SELECTION GROUP #7 - Show package information
        //     --show_simple
        //     --xml (not implemented)
        //     --json (not implemented)
        m_cmdln.DefineGroup("Show options");
        bool bShowSimple = false;
        m_cmdln.DefineSubOption("show_simple", bShowSimple, "Show package information in simple format.", true, 7);

        m_cmdln.Parse(static_cast<size_t>(nArgs), rgszArgs);

        // Process the command line
        if (!ProcessCommandLine(vecCommands)) return;

        // Update show mask
        if (bShowSimple)
            m_uiShowFlags |= static_cast<uint32_t>(EShowMask::console_simple);
        else // TODO: if not XML and JSON, add console
            m_uiShowFlags |= static_cast<uint32_t>(EShowMask::console);

        // Split the configuration strings, unless in CONFIGURE mode
        if (!ssConfigFileLocal.empty())
            SplitConfigString(ssConfigFileLocal, m_pathConfigLocal, m_vecConfigLocalComponents);
        if (rUserConfig.OptionAvailableOnCommandLine())
        {
            m_bInsertIntoUserConfig = true;
            std::filesystem::path rpathTemp;
            SplitConfigString(ssUserConfigServer, rpathTemp, m_vecUserConfigComponents);
            if (!rpathTemp.empty())
            {
                m_nError = CMDLN_INVALID_CONFIG_STRING;
                m_ssArgError = CMDLN_INVALID_CONFIG_STRING_MSG;
                return;
            }
        }
        if (rPlatformConfig.OptionAvailableOnCommandLine())
        {
            m_bInsertIntoPlatformConfig = true;
            std::filesystem::path rpathTemp;
            SplitConfigString(ssPlatformConfigServer, rpathTemp, m_vecPlatformConfigComponents);
            if (!rpathTemp.empty())
            {
                m_nError = CMDLN_INVALID_CONFIG_STRING;
                m_ssArgError = CMDLN_INVALID_CONFIG_STRING_MSG;
                return;
            }
        }
        if (rInterfaceConfig.OptionAvailableOnCommandLine())
        {
            m_bInsertIntoVehIfcConfig = true;
            std::filesystem::path rpathTemp;
            SplitConfigString(ssInterfaceConfigServer, rpathTemp, m_vecVehIfcConfigComponents);
            if (!rpathTemp.empty())
            {
                m_nError = CMDLN_INVALID_CONFIG_STRING;
                m_ssArgError = CMDLN_INVALID_CONFIG_STRING_MSG;
                return;
            }
        }
        if (rAbstrConfig.OptionAvailableOnCommandLine())
        {
            m_bInsertIntoVehAbstrConfig = true;
            std::filesystem::path rpathTemp;
            SplitConfigString(ssAbstrConfigServer, rpathTemp, m_vecVehAbstrConfigComponents);
            if (!rpathTemp.empty())
            {
                m_nError = CMDLN_INVALID_CONFIG_STRING;
                m_ssArgError = CMDLN_INVALID_CONFIG_STRING_MSG;
                return;
            }
        }

        // Check for the local flag and if set, this is not compatible to server options.
        // Also if not set, local options are not allowed.
        if (m_bLocal)
        {
            if (rInstance.OptionAvailableOnCommandLine())
            {
                m_nError = CMDLN_INCOMPATIBLE_ARGUMENTS;
                m_ssArgError = "The option '--instance' cannot be used with with local installations ('--local' option).";
                return;
            }
            if (rUserConfig.OptionAvailableOnCommandLine() || rPlatformConfig.OptionAvailableOnCommandLine() ||
                rInterfaceConfig.OptionAvailableOnCommandLine() || rAbstrConfig.OptionAvailableOnCommandLine())
            {
                m_nError = CMDLN_INCOMPATIBLE_ARGUMENTS;
                m_ssArgError = "The configuration options '--user_config', '--platform_config', '--interface_config' and "
                    "'--abstract_config' cannot be used with with local installations ('--local' option).";
                return;
            }
            if (!rTargetLoc.OptionAvailableOnCommandLine())
            {
                m_nError = CMDLN_TARGET_LOCATION_ERROR;
                m_ssArgError = "The target location option '-T' must be used when specifying local installations ('--local' "
                    "option).";
                return;
            }
        } else
        {
            if (rConfigFile.OptionAvailableOnCommandLine())
            {
                m_nError = CMDLN_INCOMPATIBLE_ARGUMENTS;
                m_ssArgError = "The configuration file option '--config_path' can only be used with with local installations "
                    "('--local' option).";
                return;
            }
            if (rConfigDir.OptionAvailableOnCommandLine())
            {
                m_nError = CMDLN_INCOMPATIBLE_ARGUMENTS;
                m_ssArgError = "The configuration file option '--config_dir' can only be used with with local installations "
                    "('--local' option).";
                return;
            }
        }

        // Add a parameter to the current instance. If the parameter is already present, the parameter will not be overwritten.
        auto itInstance = m_mapParameters.end();
        auto fnAddParameter = [&](const std::string& rssObjectInstance, const std::string& rssKey,
            const sdv::any_t& rany) -> bool
        {
            if (itInstance == m_mapParameters.end() || itInstance->first != rssObjectInstance)
            {
                // Find or create the instance
                itInstance = m_mapParameters.find(rssObjectInstance);
                if (itInstance == m_mapParameters.end())
                {
                    auto prInsert = m_mapParameters.emplace(rssObjectInstance, std::vector<std::pair<std::string, sdv::any_t>>());
                    if (!prInsert.second || prInsert.first == m_mapParameters.end())
                        return false;
                    itInstance = prInsert.first;
                }
            }
            if (itInstance == m_mapParameters.end()) return false;

            // Check for the existence
            if (std::find_if(itInstance->second.begin(), itInstance->second.end(), [&](const auto& prParameter)
                { return prParameter.first == rssKey; }) != itInstance->second.end()) return true;  // Not an error...

            // Add the parameter
            itInstance->second.push_back(std::make_pair(rssKey, rany));

            return true;
        };

        // Parameters are only valid when a configuration is supplied.
        if (!vecParametersRaw.empty() || !vecParamFiles.empty())
        {
            // Parameters can only be added when a configuration is created.
            if (!m_bInsertIntoUserConfig && !m_bInsertIntoPlatformConfig && !m_bInsertIntoVehIfcConfig
                && !m_bInsertIntoVehAbstrConfig && ssConfigFileLocal.empty())
            {
                m_nError = CMDLN_INCOMPATIBLE_ARGUMENTS;
                m_ssArgError = CMDLN_INCOMPATIBLE_ARGUMENTS_MSG;
                return;
            }
        }

        // Process command line parameters
        // The list of raw parameters are started with a component instance, followed by parameters.
        std::string ssInstance;
        for (const std::string& rssParameterRaw : vecParametersRaw)
        {
            size_t nInstanceSep = rssParameterRaw.find_first_of(':');
            if (nInstanceSep == std::string::npos)
                nInstanceSep = 0;
            else
            {
                ssInstance = rssParameterRaw.substr(0, nInstanceSep);
                ++nInstanceSep;
            }

            size_t nAssignment = rssParameterRaw.find_first_of('=', nInstanceSep);
            if (nAssignment == nInstanceSep || nAssignment == std::string::npos)
            {
                m_nError = CMDLN_INVALID_PARAM_STRING;
                m_ssArgError = CMDLN_INVALID_PARAM_STRING_MSG;
                return;
            }
            std::string ssParamName = rssParameterRaw.substr(nInstanceSep, nAssignment - nInstanceSep);
            std::string ssParamValue = rssParameterRaw.substr(nAssignment + 1);
            if (ssParamName.empty()) // An empty name is not allowed. An empty value is allowed.
            {
                m_nError = CMDLN_INVALID_PARAM_STRING;
                m_ssArgError = CMDLN_INVALID_PARAM_STRING_MSG;
                return;
            }

            // Add the parameter to the instance in the map.
            sdv::any_t anyValue;
            if (!ssParamValue.empty()) anyValue = ssParamValue;
            if (!fnAddParameter(ssInstance, ssParamName, anyValue))
            {
                m_nError = CMDLN_INVALID_PARAM_STRING;
                m_ssArgError = CMDLN_INVALID_PARAM_STRING_MSG;
                return;
            }
        }

        // Process parameter files
        for (std::filesystem::path& rpathParamFile : vecParamFiles)
        {
            try
            {
                // Load the file
                std::ifstream fstream(rpathParamFile);
                if (!fstream.is_open())
                {
                    m_nError = CMDLN_INVALID_PARAM_FILE;
                    m_ssArgError = CMDLN_INVALID_PARAM_FILE_MSG;
                    return;
                }
                std::string ssContent((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
                fstream.close();

                // Split the parameter path from the object instance.
                auto fnSplit = [](const std::string& rssNodePath) -> std::pair<std::string, std::string>
                {
                    size_t nSeparator = rssNodePath.find_first_of('.');
                    if (nSeparator == std::string::npos)
                        return std::make_pair(rssNodePath, std::string());
                    return std::make_pair(rssNodePath.substr(0, nSeparator), rssNodePath.substr(nSeparator + 1));
                };

                // Parse the file
                toml_parser::CParser parser(ssContent);
                sdv::toml::CNodeCollection root(&parser.Root());

                // Lambda function to iterative add parameters
                std::function<void(const sdv::toml::CNodeCollection&)> fnProcessObjectInstance =
                    [&](const sdv::toml::CNodeCollection& rTable)
                {
                    for (size_t nNodeIndex = 0; nNodeIndex < rTable.GetCount(); nNodeIndex++)
                    {
                        sdv::toml::CNode node = rTable.Get(nNodeIndex);
                        switch (node.GetType())
                        {
                        case sdv::toml::ENodeType::node_boolean:
                        case sdv::toml::ENodeType::node_integer:
                        case sdv::toml::ENodeType::node_floating_point:
                        case sdv::toml::ENodeType::node_string:
                            {
                                auto prParameter = fnSplit(node.GetQualifiedPath());
                                if (!prParameter.first.empty() && !prParameter.second.empty())
                                    fnAddParameter(prParameter.first, prParameter.second, node.GetValue());
                            }
                            break;
                        case sdv::toml::ENodeType::node_table:
                            {
                                sdv::toml::CNodeCollection group(node);
                                if (!group.IsValid())
                                    break;
                                fnProcessObjectInstance(group);
                            }
                            break;
                        default:
                            // Ignore...
                            break;
                        }
                    }
                };
                fnProcessObjectInstance(root);
            }
            catch (const sdv::toml::XTOMLParseException& /*rxExcept*/)
            {
                m_nError = CMDLN_INVALID_PARAM_FILE;
                m_ssArgError = CMDLN_INVALID_PARAM_FILE_MSG;
                return;
            }
        }

        // Check update and overwrite
        if (m_bUpdate && m_bOverwrite)
        {
            m_nError = CMDLN_UPDATE_OVERWRITE_ERROR;
            m_ssArgError = CMDLN_UPDATE_OVERWRITE_ERROR_MSG;
            return;
        }
    } catch (const SArgumentParseException& rsExcept)
    {
        // If not all fits, but help is requested, this is okay...
        // Otherwise throw an environment exception.
        if (!m_bHelp)
        {
            m_nError = CMDLN_ARG_ERR;
            m_ssArgError = rsExcept.what();
            return;
        }
    }
}

#endif // !defined ENVIRONMENT_H