#include "environment.h"

#include <string_view>

#include "../../global/cmdlnparser/cmdlnparser.cpp"

CSdvPackagerEnvironment::CSdvPackagerEnvironment() : m_cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character)) {}

CSdvPackagerEnvironment::CSdvPackagerEnvironment(const std::vector<std::string>& rvecArgs) :
    m_cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character))
{
    // Create a classic argument array
    std::vector<const char*> vecArgPtrs;
    for (const std::string& rssArg : rvecArgs)
        vecArgPtrs.push_back(rssArg.c_str());

    // Call the constructor
    *this = CSdvPackagerEnvironment(vecArgPtrs.size(), &vecArgPtrs.front());
}

bool CSdvPackagerEnvironment::Help() const
{
    return m_bHelp;
}

void CSdvPackagerEnvironment::ShowHelp() const
{
    size_t            nArgumentGroup = 0;
    std::stringstream sstreamText;

    // Show default information:
    sstreamText << "Component installation package utility. This utility can be used to pack, install and uninstall "
                   "SDV component installations on the current machine."
                << std::endl
                << std::endl;

    // Show information based on the command.
    switch (m_eOperatingMode)
    {
    case EOperatingMode::pack:
        nArgumentGroup = 1;
        sstreamText << "PACK command usage:" << std::endl
                    << "    sdv_package PACK <name> <files> [options]" << std::endl
                    << std::endl;
        sstreamText
            << "Provide one or more files to add to the installation onto the command line. Wildcards (**, *, ?) are "
               "supported, allowing multiple files and directories to be added automatically. Instead regular expressions can also "
               "be used by prefixing the paths by \"regex:\". If a specific target location (relative to the installation "
               "directory) is required, this can be added by \"=<directory>\"."
            << std::endl
            << std::endl
            << "Examples:" << std::endl
            << "    *.sdv          - This will pack all SDV files from the input directory." << std::endl
            << "    **/*.*         - This will pack all files from all directories." << std::endl
            << "    abc.sdv=mysub  - This will pack \"abc.sdv\" into target \"mysub\"." << std::endl
            << "    regex:[^/].sdv - This will pack all SDV files from the input directory." << std::endl
            << std::endl
            << "REMARKS: To create updatable components, use the '--set_version' option to set a version." << std::endl;
        break;
    case EOperatingMode::install:
        nArgumentGroup = 2;
        sstreamText << "INSTALL command usage:" << std::endl
                    << "    sdv_package INSTALL <package> [options]" << std::endl
                    << std::endl;
        sstreamText
            << "Install an installation package and update the configuration if applicable." << std::endl
            << std::endl
            << "The configuration uses an optional string following the option to define what to configure. The string is "
               "separated "
               "by the '+' separator followed by a list of components (separated by a comma ',' or a '+') to include in the "
               "configuration. For example to update a local configuration ('--local' option) use:"
            << std::endl
            << "    --config_fileabc.toml+my_component1,my_component2" << std::endl
            << "and a server configuration:" << std::endl
            << "    --user_config+my_component1+my_component2" << std::endl
            << "If the configuration file is not existing, it will be created automatically." << std::endl
            << std::endl
            << "REMARKS: The server based installation can only take place when the system is offline. Use \"sdv_control\" "
               "to install on a running server instance."
            << std::endl;
        break;
    case EOperatingMode::direct_install:
        nArgumentGroup = 3;
        sstreamText << "DIRECT_INSTALL command usage:" << std::endl
                    << "    sdv_package DIRECT_INSTALL <name> <files> [options]" << std::endl
                    << std::endl;
        sstreamText
            << "Provide one or more files to add to the installation onto the command line. Wildcards (**, *, ?) are "
               "supported, allowing multiple files and directories to be added automatically. Instead regular expressions can also "
               "used by prefixing the paths by \"regex:\". If a specific target location (relative to the installation "
               "directory) is required, this can be added by \"=<directory>\"."
            << std::endl
            << std::endl
            << "Examples:" << std::endl
            << "    *.sdv          - This will install all SDV files from the input directory." << std::endl
            << "    **/*.*         - This will install all files from the input directory and sub-directories." << std::endl
            << "    abc.sdv=mysub  - This will install \"abc.sdv\" into target \"mysub\"." << std::endl
            << "    regex:[^/].sdv - This will install all SDV files from the input directory." << std::endl
            << std::endl
            << "The configuration uses an optional string following the option to define what to configure. The string is "
               "separated "
               "by the '+' separator followed by a list of components (separated by a comma ',' or a '+') to include in the "
               "configuration. For example to update a local configuration ('--local' option) use:"
            << std::endl
            << "    --config_fileabc.toml+my_component1,my_component2" << std::endl
            << "and a server configuration:" << std::endl
            << "    --user_config+my_component1+my_component2" << std::endl
            << "If the configuration file is not existing, it will be created automatically." << std::endl
            << std::endl
            << "REMARKS: The server based installation can only take place when the system is offline. Use \"sdv_control\" to "
               "install "
               "on a running server instance."
            << std::endl
            << std::endl
            << "REMARKS: To create updatable components, use the '--set_version' option to set a version." << std::endl;
        break;
    case EOperatingMode::uninstall:
        nArgumentGroup = 4;
        sstreamText << "UNINSTALL command usage:" << std::endl
                    << "    sdv_package UNINSTALL <name> <files> [options]" << std::endl
                    << std::endl;
        sstreamText
            << "Uninstall a previously installed package with the supplied name. Update the configuration files where "
               "applicable."
            << std::endl
            << std::endl
            << "REMARKS: The server based unistallation can only take place when the system is offline. Use \"sdv_control\" to "
               "install on a running server instance."
            << std::endl;
        break;
    case EOperatingMode::verify:
        nArgumentGroup = 5;
        sstreamText << "VERIFY command usage:" << std::endl
                    << "    sdv_package VERIFY <package> [options]" << std::endl
                    << std::endl;
        sstreamText << "Verify the package for corruption and/or tampering." << std::endl;
        break;
    case EOperatingMode::show:
        nArgumentGroup = 6;
        sstreamText << "SHOW command usage:" << std::endl
                    << "    sdv_package SHOW [ALL|INFO|MODULES|COMPONENTS] <package> [options]" << std::endl
                    << std::endl;
        sstreamText << "Show package information. The following information can be selected (multiple selections are possible):"
                    << std::endl
                    << "    ALL            Show all information about the package content." << std::endl
                    << "    INFO           Show package information." << std::endl
                    << "    MODULES        Show the modules contained in the package." << std::endl
                    << "    COMPONENTS     Show the components contained in the package." << std::endl;
        break;
    default:
        sstreamText << "Usage: sdv_package <command> [options]" << std::endl << std::endl;
        sstreamText << "The following commands are available:" << std::endl
                    << "    PACK               Create an installation package." << std::endl
                    << "    INSTALL            Install an installation package." << std::endl
                    << "    DIRECT_INSTALL     Install the components directly without creating an installation package."
                    << std::endl
                    << "    UNINSTALL          Uninstall a previously installed installation package." << std::endl
                    << "    VERIFY             Verify the integrity of the installation package." << std::endl
                    << "    SHOW               Show information about the installation package." << std::endl
                    << std::endl
                    << "For information about the options of each command, provide the '--help' option following the command."
                    << std::endl;
        break;
    }

    m_cmdln.PrintHelp(std::cout, sstreamText.str(), nArgumentGroup);
}

void CSdvPackagerEnvironment::ReportInfo() const
{
    // Do not report anything
    if (m_bSilent) return;

    // Dependable on the verbose and the silent settings, select the output stream for information.
    std::stringstream sstreamDummy;
    std::ostream& rstreamVerbose = !m_bHelp && m_bVerbose ? std::cout : sstreamDummy;
    std::ostream& rstreamNormal  = m_bHelp || m_bSilent ? sstreamDummy : std::cout;

    // Assign additional paths and report information...
    switch (m_eOperatingMode)
    {
    case EOperatingMode::pack:
        if (m_bCreateManifestOnly)
            rstreamNormal << "Creating an installation manifest..." << std::endl;
        else
            rstreamNormal << "Creating an installation package..." << std::endl;
        rstreamVerbose << "Source location: " << m_pathSourceLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Output location: " << m_pathOutputLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation name: " << m_ssInstallName << std::endl;
        rstreamVerbose << "Product name: " << m_ssProductName << std::endl;
        rstreamVerbose << "Package description: " << m_ssDescription << std::endl;
        rstreamVerbose << "Author: " << m_ssAuthor << std::endl;
        rstreamVerbose << "Address: " << m_ssAddress << std::endl;
        rstreamVerbose << "Copyright: " << m_ssCopyrights << std::endl;
        rstreamVerbose << "Version: " << m_ssPackageVersion << std::endl;
        rstreamVerbose << "Keep directory structure: " << (m_bKeepStructure ? "true" : "false") << std::endl;
        break;
    case EOperatingMode::install:
        rstreamNormal << "Installing a package..." << std::endl;
        if (m_bCreateManifestOnly)
            rstreamVerbose << "No file copy, creating manifest only..." << std::endl;
        if (m_bLocal)
            rstreamVerbose << "Local installation..." << std::endl;
        else
            rstreamVerbose << "Instance ID: " << m_uiInstanceID << std::endl;
        rstreamVerbose << "Target base location: " << m_pathTargetLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation root location: " << m_pathRootLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Install location: " << m_pathInstallLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Update: " << (m_bUpdate ? "true" : "false") << std::endl;
        rstreamVerbose << "Overwrite: " << (m_bOverwrite ? "true" : "false") << std::endl;
        if (m_bLocal)
        {
            if (!m_pathConfigLocal.empty())
            {
                bool bInitial = true;
                for (const auto& rpathConfigDir : m_vecLocalConfigDirs)
                {
                    if (bInitial)
                        rstreamVerbose << "Config location: ";
                    else
                        rstreamVerbose << "                 ";
                    bInitial = false;
                    rstreamVerbose << rpathConfigDir.generic_u8string() << std::endl;
                }
                rstreamVerbose << "Config file: " << m_pathConfigLocal.generic_u8string() << std::endl;
            }
        }
        else
        {
            bool bInitial = true;
            for (const auto& rssComponent : m_vecUserConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "User config components: ";
                else
                    rstreamVerbose << "                        ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecPlatformConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Platform config components: ";
                else
                    rstreamVerbose << "                            ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecVehIfcConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Vehicle interface config components: ";
                else
                    rstreamVerbose << "                                     ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecVehAbstrConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Vehicle abstraction config components: ";
                else
                    rstreamVerbose << "                                       ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
        }
        break;
    case EOperatingMode::direct_install:
        rstreamNormal << "Direct installation of modules..." << std::endl;
        rstreamVerbose << "Source location: " << m_pathSourceLocation.generic_u8string() << std::endl;
        if (m_bLocal)
            rstreamVerbose << "Local installation..." << std::endl;
        else
            std::cout << "Instance ID: " << m_uiInstanceID << std::endl;
        rstreamVerbose << "Target base location: " << m_pathTargetLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation root location: " << m_pathRootLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Install location: " << m_pathInstallLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation name: " << m_ssInstallName << std::endl;
        rstreamVerbose << "Product name: " << m_ssProductName << std::endl;
        rstreamVerbose << "Package description: " << m_ssDescription << std::endl;
        rstreamVerbose << "Author: " << m_ssAuthor << std::endl;
        rstreamVerbose << "Address: " << m_ssAddress << std::endl;
        rstreamVerbose << "Copyright: " << m_ssCopyrights << std::endl;
        rstreamVerbose << "Version: " << m_ssPackageVersion << std::endl;
        rstreamVerbose << "Keep directory structure: " << (m_bKeepStructure ? "true" : "false") << std::endl;
        rstreamVerbose << "Update: " << (m_bUpdate ? "true" : "false") << std::endl;
        rstreamVerbose << "Overwrite: " << (m_bOverwrite ? "true" : "false") << std::endl;
        if (!m_pathPackage.empty())
            rstreamVerbose << "Package: " << m_pathPackage.generic_u8string() << std::endl;
        if (m_bLocal)
        {
            if (!m_pathConfigLocal.empty())
            {
                bool bInitial = true;
                for (const auto& rpathConfigDir : m_vecLocalConfigDirs)
                {
                    if (bInitial)
                        rstreamVerbose << "Config location: ";
                    else
                        rstreamVerbose << "                 ";
                    bInitial = false;
                    rstreamVerbose << rpathConfigDir.generic_u8string() << std::endl;
                }
                rstreamVerbose << "Config file: " << m_pathConfigLocal.generic_u8string() << std::endl;
            }
        }
        else
        {
            bool bInitial = true;
            for (const auto& rssComponent : m_vecUserConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "User config components: ";
                else
                    rstreamVerbose << "                        ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecPlatformConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Platform config components: ";
                else
                    rstreamVerbose << "                            ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecVehIfcConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Vehicle interface config components: ";
                else
                    rstreamVerbose << "                                     ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
            bInitial = true;
            for (const auto& rssComponent : m_vecVehAbstrConfigComponents)
            {
                if (bInitial)
                    rstreamVerbose << "Vehicle abstraction config components: ";
                else
                    rstreamVerbose << "                                       ";
                bInitial = false;
                rstreamVerbose << rssComponent << std::endl;
            }
        }
        break;
    case EOperatingMode::uninstall:
        rstreamNormal << "Uninstall installation..." << std::endl;
        rstreamVerbose << "Target base location: " << m_pathTargetLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation root location: " << m_pathRootLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Install location: " << m_pathInstallLocation.generic_u8string() << std::endl;
        rstreamVerbose << "Installation name: " << m_ssInstallName << std::endl;
        if (m_bLocal)
        {
            bool bInitial = true;
            for (const auto& rpathConfigDir : m_vecLocalConfigDirs)
            {
                if (bInitial)
                    rstreamVerbose << "Config location: ";
                else
                    rstreamVerbose << "                 ";
                bInitial = false;
                rstreamVerbose << rpathConfigDir.generic_u8string() << std::endl;
            }
        }
        break;
    case EOperatingMode::verify:
        rstreamNormal << "Verify package integrity..." << std::endl;
        if (!m_pathPackage.empty())
            rstreamVerbose << "Package: " << m_pathPackage.generic_u8string() << std::endl;
        break;
    case EOperatingMode::show:
        rstreamNormal << "Show package information..." << std::endl;
        rstreamVerbose << "Information: ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::info))
            rstreamVerbose << "package ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::modules))
            rstreamVerbose << "modules ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::components))
            rstreamVerbose << "components";
        rstreamVerbose << std::endl;
        rstreamVerbose << "Output: ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::console))
            rstreamVerbose << "console ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::xml))
            rstreamVerbose << "xml ";
        if (m_uiShowFlags & static_cast<uint32_t>(EShowMask::json))
            rstreamVerbose << "json";
        rstreamVerbose << std::endl;
        if (!m_pathPackage.empty())
            rstreamVerbose << "Package: " << m_pathPackage.generic_u8string() << std::endl;
        break;
    default:
        rstreamNormal << "Nothing to do..." << std::endl;
        break;
    }
}


bool CSdvPackagerEnvironment::Silent() const
{
    return m_bSilent;
}

bool CSdvPackagerEnvironment::Verbose() const
{
    return m_bVerbose;
}

bool CSdvPackagerEnvironment::Version() const
{
    return m_bVersion;
}

bool CSdvPackagerEnvironment::CreateManifestOnly() const
{
    return m_bCreateManifestOnly;
}

CSdvPackagerEnvironment::EOperatingMode CSdvPackagerEnvironment::OperatingMode() const
{
    return m_eOperatingMode;
}

bool CSdvPackagerEnvironment::KeepStructure() const
{
    return m_bKeepStructure;
}

const std::vector<CSdvPackagerEnvironment::SModule>& CSdvPackagerEnvironment::ModuleList() const
{
    return m_vecModules;
}

const std::filesystem::path& CSdvPackagerEnvironment::PackagePath() const
{
    return m_pathPackage;
}

const std::filesystem::path& CSdvPackagerEnvironment::SourceLocation() const
{
    return m_pathSourceLocation;
}

const std::filesystem::path& CSdvPackagerEnvironment::OutputLocation() const
{
    return m_pathOutputLocation;
}

const std::filesystem::path& CSdvPackagerEnvironment::TargetLocation() const
{
    return m_pathTargetLocation;
}

const std::filesystem::path& CSdvPackagerEnvironment::RootLocation() const
{
    return m_pathRootLocation;
}

const std::filesystem::path& CSdvPackagerEnvironment::InstallLocation() const
{
    return m_pathInstallLocation;
}

bool CSdvPackagerEnvironment::Local() const
{
    return m_bLocal;
}

uint32_t CSdvPackagerEnvironment::InstanceID() const
{
    return m_uiInstanceID;
}

bool CSdvPackagerEnvironment::Update() const
{
    return m_bUpdate;
}

bool CSdvPackagerEnvironment::Overwrite() const
{
    return m_bOverwrite;
}

const std::filesystem::path& CSdvPackagerEnvironment::LocalConfigFile(std::vector<std::string>& rvecComponents) const
{
    rvecComponents = m_vecConfigLocalComponents;
    return m_pathConfigLocal;
}

const std::vector<std::filesystem::path>& CSdvPackagerEnvironment::LocalConfigLocations() const
{
    return m_vecLocalConfigDirs;
}

bool CSdvPackagerEnvironment::ActivateLocalConfig() const
{
    return m_bActivateLocalConfig;
}

bool CSdvPackagerEnvironment::InsertIntoUserConfig(std::vector<std::string>& rvecComponents) const
{
    rvecComponents = m_vecUserConfigComponents;
    return m_bInsertIntoUserConfig;
}

bool CSdvPackagerEnvironment::InsertIntoPlatformConfig(std::vector<std::string>& rvecComponents) const
{
    rvecComponents = m_vecPlatformConfigComponents;
    return m_bInsertIntoPlatformConfig;
}

bool CSdvPackagerEnvironment::InsertIntoVehicleInterfaceConfig(std::vector<std::string>& rvecComponents) const
{
    rvecComponents = m_vecVehIfcConfigComponents;
    return m_bInsertIntoVehIfcConfig;
}

bool CSdvPackagerEnvironment::InsertIntoVehicleAbstractionConfig(std::vector<std::string>& rvecComponents) const
{
    rvecComponents = m_vecVehAbstrConfigComponents;
    return m_bInsertIntoVehAbstrConfig;
}

const std::string& CSdvPackagerEnvironment::InstallName() const
{
    return m_ssInstallName;
}

const std::string& CSdvPackagerEnvironment::ProductName() const
{
    return m_ssProductName;
}

const std::string& CSdvPackagerEnvironment::Description() const
{
    return m_ssDescription;
}

const std::string& CSdvPackagerEnvironment::Author() const
{
    return m_ssAuthor;
}

const std::string& CSdvPackagerEnvironment::Address() const
{
    return m_ssAddress;
}

const std::string& CSdvPackagerEnvironment::Copyrights() const
{
    return m_ssCopyrights;
}

const std::string& CSdvPackagerEnvironment::PackageVersion() const
{
    return m_ssPackageVersion;
}

uint32_t CSdvPackagerEnvironment::ShowFlags() const
{
    return m_uiShowFlags;
}

bool CSdvPackagerEnvironment::CheckShowFlag(EShowMask eMask) const
{
    return (m_uiShowFlags & static_cast<uint32_t>(eMask)) == static_cast<uint32_t>(eMask);
}

int CSdvPackagerEnvironment::Error() const
{
    return m_nError;
}

const std::string& CSdvPackagerEnvironment::ArgError() const
{
    return m_ssArgError;
}

void CSdvPackagerEnvironment::SplitConfigString(const std::string& rssInput,
    std::filesystem::path& rpath,
    std::vector<std::string>& rvecComponents)
{
    size_t nPos = rssInput.find('+');
    rpath       = std::filesystem::u8path(rssInput.substr(0, nPos));
    while (nPos != std::string::npos)
    {
        size_t nNextPos = rssInput.find_first_of("+,", nPos + 1);
        std::string ss  = rssInput.substr(nPos + 1, nNextPos);
        if (!ss.empty())
            rvecComponents.push_back(ss);
        nPos = nNextPos;
    }
}

bool CSdvPackagerEnvironment::ProcessCommandLine(const std::vector<std::string>& rvecCommands)
{
    // Make upper case string
    auto fnMakeUpper = [](const std::string& rss)
    {
        std::string rssTemp;
        rssTemp.reserve(rss.size());
        for (char c : rss)
            rssTemp += static_cast<char>(std::toupper(c));
        return rssTemp;
    };

    // Process the first command...
    size_t nOptionGroup = 0;
    if (rvecCommands.empty())
    {
        m_eOperatingMode = EOperatingMode::none;
    }
    else if (iequals(rvecCommands[0], "PACK"))
    {
        m_eOperatingMode = EOperatingMode::pack;
        nOptionGroup     = 1;
    }
    else if (iequals(rvecCommands[0], "INSTALL"))
    {
        m_eOperatingMode = EOperatingMode::install;
        nOptionGroup     = 2;
    }
    else if (iequals(rvecCommands[0], "DIRECT_INSTALL"))
    {
        m_eOperatingMode = EOperatingMode::direct_install;
        nOptionGroup     = 3;
    }
    else if (iequals(rvecCommands[0], "UNINSTALL"))
    {
        m_eOperatingMode = EOperatingMode::uninstall;
        nOptionGroup     = 4;
    }
    else if (iequals(rvecCommands[0], "VERIFY"))
    {
        m_eOperatingMode = EOperatingMode::verify;
        nOptionGroup     = 5;
    }
    else if (iequals(rvecCommands[0], "SHOW"))
    {
        m_eOperatingMode = EOperatingMode::show;
        nOptionGroup     = 6;
    }
    else
    {
        m_nError     = CMDLN_ARG_ERR;
        m_ssArgError = "Invalid command \"" + rvecCommands[0] + "\" supplied at the command line.";
        return false;
    }
    size_t nCommandPos = 1; // Start iteration at the second command argument

    // Help requested? Do not continue parsing
    if (m_bHelp)
        return false;

    // Check for silent and verbose flags.
    if (m_bSilent && m_bVerbose)
    {
        m_nError     = CMDLN_SILENT_VERBOSE;
        m_ssArgError = CMDLN_SILENT_VERBOSE_MSG;
        return false;
    }

    // Get the installation name
    auto fnGetInstallName = [rvecCommands, &nCommandPos, this]() -> bool
    {
        // Expecting the installation name
        if (nCommandPos >= rvecCommands.size())
        {
            m_nError     = CMDLN_INSTALL_NAME_MISSING;
            m_ssArgError = CMDLN_INSTALL_NAME_MISSING_MSG;
            return false;
        }
        m_ssInstallName = rvecCommands[nCommandPos];
        nCommandPos++;
        return true;
    };

    // Get the modules to pack/install
    auto fnGetModules = [rvecCommands, &nCommandPos, this]() -> bool
    {
        // Expecting at least one source
        if (nCommandPos >= rvecCommands.size())
        {
            m_nError     = CMDLN_SOURCE_FILE_MISSING;
            m_ssArgError = CMDLN_SOURCE_FILE_MISSING_MSG;
            return false;
        }
        for (;nCommandPos < rvecCommands.size(); nCommandPos++)
        {
            const std::string& rssCommand = rvecCommands[nCommandPos];
            size_t nPos = rssCommand.find('=');
            SModule sModule;
            sModule.ssSearchString = rssCommand.substr(0, nPos);
            if (nPos != std::string::npos)
                sModule.pathRelTarget = std::filesystem::u8path(rssCommand.substr(nPos + 1));
            m_vecModules.push_back(std::move(sModule));
        }
        return true;
    };

    // Cneck for no more commands
    auto fnCheckForNoMoreCommands = [rvecCommands, &nCommandPos, this]() -> bool
    {
        if (nCommandPos != rvecCommands.size())
        {
            m_nError     = CMDLN_TOO_MANY_SOURCE_FILES;
            m_ssArgError = CMDLN_TOO_MANY_SOURCE_FILES_MSG;
            return false;
        }

        return true;
    };

    // Get exactly one installation package
    auto fnGetPackage = [rvecCommands, &nCommandPos, this]() -> bool
    {
        // Expecting exactly one installation package
        if (nCommandPos >= rvecCommands.size())
        {
            m_nError     = CMDLN_SOURCE_FILE_MISSING;
            m_ssArgError = CMDLN_SOURCE_FILE_MISSING_MSG;
            return false;
        }
        m_pathPackage = std::filesystem::u8path(rvecCommands[nCommandPos]);
        nCommandPos++;

        // Check for the correct ending and its existence
        if (m_pathPackage.extension() != ".sdv_package" || !std::filesystem::exists(m_pathPackage)
            || !std::filesystem::is_regular_file(m_pathPackage))
        {
            m_nError     = CMDLN_SOURCE_FILE_ERROR;
            m_ssArgError = CMDLN_SOURCE_FILE_ERROR_MSG;
            return false;
        }

        // Any commands left... then too many source files were supplied
        if (nCommandPos != rvecCommands.size())
        {
            m_nError = CMDLN_TOO_MANY_SOURCE_FILES;
            m_ssArgError = CMDLN_TOO_MANY_SOURCE_FILES_MSG;
        }

        return true;
    };

    // Check the source location
    auto fnCheckSourceLocation = [this]() -> bool
    {
        // Check for the source location
        if (m_pathSourceLocation.empty())
            m_pathSourceLocation = std::filesystem::current_path();
        if (!std::filesystem::is_directory(m_pathSourceLocation))
        {
            m_nError = CMDLN_SOURCE_LOCATION_ERROR;
            m_ssArgError =
                std::string(CMDLN_SOURCE_LOCATION_ERROR_MSG) + "\n  Source location: " + m_pathSourceLocation.generic_u8string();
            return false;
        }
        return true;
    };

    // Check the output location
    auto fnCheckOutputLocation = [this]() -> bool
    {
        // Check for the output location (and create directory if needed)
        if (m_pathOutputLocation.empty())
            m_pathOutputLocation = std::filesystem::current_path();
        try
        {
            if (!std::filesystem::is_directory(m_pathOutputLocation))
                std::filesystem::create_directories(m_pathOutputLocation);
            if (!std::filesystem::is_directory(m_pathOutputLocation))
            {
                m_nError     = CMDLN_OUTPUT_LOCATION_ERROR;
                m_ssArgError = std::string(CMDLN_OUTPUT_LOCATION_ERROR_MSG)
                               + "\n  Output location: " + m_pathOutputLocation.generic_u8string();
                return false;
            }
        }
        catch (const std::filesystem::filesystem_error&)
        {
            m_nError = CMDLN_OUTPUT_LOCATION_ERROR;
            m_ssArgError =
                std::string(CMDLN_OUTPUT_LOCATION_ERROR_MSG) + "\n  Output location: " + m_pathOutputLocation.generic_u8string();
            return false;
        }
        return true;
    };

    // Check the target location
    auto fnCheckTargetLocation = [this]() -> bool
    {
        // Check for the target location (and create directory if needed)
        try
        {
            // If not locally, get the target from the environment variable
            if (!m_bLocal)
            {
#ifdef _WIN32
                const wchar_t* szInstallDir = _wgetenv(L"SDV_COMPONENT_INSTALL");
                if (szInstallDir) m_pathTargetLocation = szInstallDir;
#elif defined __unix__
                const char* szInstallDir = getenv("SDV_COMPONENT_INSTALL");
                if (szInstallDir) m_pathTargetLocation = std::filesystem::u8path(szInstallDir);
#else
    #error OS not supported!
#endif
            }

            // Is the directory existing? If not, creat ethe directory
            if (!m_pathTargetLocation.empty() && !std::filesystem::is_directory(m_pathTargetLocation))
                std::filesystem::create_directories(m_pathTargetLocation);

            // The directory should exist; otherwise error.
            if (m_pathTargetLocation.empty() || !std::filesystem::is_directory(m_pathTargetLocation))
            {
                m_nError = CMDLN_TARGET_LOCATION_ERROR;
                m_ssArgError = std::string(CMDLN_TARGET_LOCATION_ERROR_MSG) + "\n  Target location: " +
                    m_pathTargetLocation.generic_u8string();
                return false;
            }
        }
        catch (const std::filesystem::filesystem_error&)
        {
            m_nError = CMDLN_TARGET_LOCATION_ERROR;
            m_ssArgError =
                std::string(CMDLN_TARGET_LOCATION_ERROR_MSG) + "\n  Target location: " + m_pathTargetLocation.generic_u8string();
            return false;
        }
        return true;
    };

    // Get product
    auto fnGetProduct = [this]()
    {
        if (m_ssProductName.empty())
            m_ssProductName = m_ssInstallName;
    };

    // Following the initial command, several additional information must be supplied.
    switch (m_eOperatingMode)
    {
    case EOperatingMode::pack:
        if (!fnGetInstallName())
            return false;
        if (!fnGetModules())
            return false;
        if (!fnCheckSourceLocation())
            return false;
        if (!fnCheckOutputLocation())
            return false;
        fnGetProduct();
        if (m_pathOutputLocation.empty())
            m_pathPackage = std::filesystem::u8path(m_ssInstallName + ".sdv_package");
        else
            m_pathPackage = m_pathOutputLocation / std::filesystem::u8path(m_ssInstallName + ".sdv_package");
        break;
    case EOperatingMode::direct_install:
        if (!fnGetInstallName())
            return false;
        if (!fnGetModules())
            return false;
        if (!fnCheckSourceLocation())
            return false;
        if (!fnCheckTargetLocation())
            return false;
        fnGetProduct();
        break;
    case EOperatingMode::install:
        if (!fnGetPackage())
            return false;
        if (!fnCheckForNoMoreCommands())
            return false;
        if (!fnCheckTargetLocation())
            return false;
        break;
    case EOperatingMode::uninstall:
        if (!fnGetInstallName())
            return false;
        if (!fnCheckForNoMoreCommands())
            return false;
        if (!fnCheckTargetLocation())
            return false;
        break;
    case EOperatingMode::verify:
        if (!fnGetPackage())
            return false;
        if (!fnCheckForNoMoreCommands())
            return false;
        break;
    case EOperatingMode::show:
        // Expecting one or more show commands followed by one installation package
        while (nCommandPos != rvecCommands.size())
        {
            if (!m_vecModules.empty())
            {
                m_nError     = CMDLN_TOO_MANY_SOURCE_FILES;
                m_ssArgError = CMDLN_TOO_MANY_SOURCE_FILES_MSG;
                return false;
            }
            if (iequals(rvecCommands[nCommandPos], "ALL"))
            {
                m_uiShowFlags |= static_cast<uint32_t>(EShowMask::all);
                nCommandPos++;
            }
            else if (iequals(rvecCommands[nCommandPos], "INFO"))
            {
                m_uiShowFlags |= static_cast<uint32_t>(EShowMask::info);
                nCommandPos++;
            }
            else if (iequals(rvecCommands[nCommandPos], "MODULES"))
            {
                m_uiShowFlags |= static_cast<uint32_t>(EShowMask::modules);
                nCommandPos++;
            }
            else if (iequals(rvecCommands[nCommandPos], "COMPONENTS"))
            {
                m_uiShowFlags |= static_cast<uint32_t>(EShowMask::components);
                nCommandPos++;
            }
            else
            {
                // Check whether preceeded by at least one command
                if (!(m_uiShowFlags & 0x00ff))
                {
                    m_nError     = CMDLN_MISSING_SHOW_COMMAND;
                    m_ssArgError = CMDLN_MISSING_SHOW_COMMAND_MSG;
                    return false;
                }
                if (!fnGetPackage())
                    return false;
            }
        }
        if (m_pathPackage.empty())
        {
            m_nError     = CMDLN_SOURCE_FILE_MISSING;
            m_ssArgError = CMDLN_SOURCE_FILE_MISSING_MSG;
            return false;
        }
        break;
    default:
        break;
    }

    // Check for incompatible options
    auto vecIncompatibleOptions = m_cmdln.IncompatibleArguments(nOptionGroup, false);
    if (!vecIncompatibleOptions.empty() && m_eOperatingMode != EOperatingMode::none)
    {
        m_nError     = CMDLN_INCOMPATIBLE_ARGUMENTS;
        m_ssArgError = std::string("The option '" + vecIncompatibleOptions[0] + "' cannot be used with the " +
            fnMakeUpper(rvecCommands[0]) + " command.");
        return false;
    }

    // Assign root and installation locations
    m_pathRootLocation = m_pathTargetLocation;
    if (!m_bLocal)
        m_pathRootLocation /= std::to_string(m_uiInstanceID);
    m_pathInstallLocation = m_pathRootLocation;

    return true;
}
