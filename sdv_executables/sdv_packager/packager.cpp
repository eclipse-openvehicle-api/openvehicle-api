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

#include "packager.h"
#include <support/component_impl.h>
#include "../../sdv_services/core/app_settings.h"
#include "../../sdv_services/core/app_config.h"
#include "../../sdv_services/core/installation_composer.h"
#include "../global/path_match.h"

CPackager::CPackager(CSdvPackagerEnvironment& renv) : m_env(renv)
{}

bool CPackager::Execute()
{
    switch (m_env.OperatingMode())
    {
    case CSdvPackagerEnvironment::EOperatingMode::pack:
        if (!Pack()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::install:
        if (!Unpack()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::direct_install:
        if (!Copy()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::configure:
        if (!Configure()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::uninstall:
        if (!Remove()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::verify:
        if (!CheckIntegrity()) return false;
        break;
    case CSdvPackagerEnvironment::EOperatingMode::show:
        if (!ShowContent()) return false;
        break;
    default:
        return false;
        break;
    }
    return true;
}

int CPackager::Error() const
{
    return m_nError;
}

const std::string& CPackager::ArgError() const
{
    return m_ssArgError;
}

bool CPackager::Pack()
{
    try
    {
        CInstallComposer composer;
        size_t nCount = 0;
        for (const CSdvPackagerEnvironment::SModule& rsModule : m_env.ModuleList())
        {
            // Check for regular expression or wildcard search string
            std::string ssSearchString;
            uint32_t uiFlags = 0;
            bool bExpectFile = true; // When set, expect at least one file for the addition
            if (rsModule.ssSearchString.substr(0, 6) == "regex:")
            {
                ssSearchString = rsModule.ssSearchString.substr(6);
                uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
                bExpectFile = false;
            }
            else
            {
                ssSearchString = rsModule.ssSearchString;

                // Check for a wildcard character
                bExpectFile = ssSearchString.find_first_of("*?") != std::string::npos;

                // Wildcards are default...
                uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards);
            }

            // Should the directory structure be maintained
            if (m_env.KeepStructure())
                uiFlags |= static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);

            // Add the module
            auto vecFiles = composer.AddModule(m_env.SourceLocation(), ssSearchString, rsModule.pathRelTarget, uiFlags);
            if (bExpectFile && !vecFiles.size())
            {
                m_nError = CMDLN_SOURCE_FILE_ERROR;
                m_ssArgError = CMDLN_SOURCE_FILE_ERROR_MSG;
                return false;
            }

            // Report the files if requested
            if (m_env.Verbose())
            {
                for (const auto& rpathFile : vecFiles)
                    std::cout << "Adding " << rpathFile.generic_u8string() << "..." << std::endl;
            }
            nCount += vecFiles.size();
        }

        if (!nCount)
        {
            m_nError = NO_SOURCE_FILES;
            m_ssArgError = NO_SOURCE_FILES_MSG;
            return false;
        }

        // Add additional information as properties
        if (!m_env.ProductName().empty())
            composer.AddProperty("Product", m_env.ProductName());
        if (!m_env.Description().empty())
            composer.AddProperty("Description", m_env.Description());
        if (!m_env.Author().empty())
            composer.AddProperty("Author", m_env.Author());
        if (!m_env.Address().empty())
            composer.AddProperty("Address", m_env.Address());
        if (!m_env.Copyrights().empty())
            composer.AddProperty("Copyrights", m_env.Copyrights());
        if (!m_env.PackageVersion().empty())
            composer.AddProperty("Version", m_env.PackageVersion());

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Added " << nCount << " files..." << std::endl;

        // Create manifest only? Or compose package.
        if (m_env.CreateManifestOnly())
        {
            if (m_env.Verbose())
                std::cout << "Create installation manifest..." << std::endl;
            CInstallManifest manifest = composer.ComposeInstallManifest(m_env.InstallName());
            std::filesystem::path pathOutputLocation = m_env.OutputLocation().empty() ? "." : m_env.OutputLocation();
            return manifest.Save(pathOutputLocation);
        }
        else
        {
            if (m_env.Verbose())
                std::cout << "Compose package..." << std::endl;
            return composer.Compose(m_env.PackagePath(), m_env.InstallName());
        }
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_CREATION_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
}

bool CPackager::Unpack()
{
    CInstallManifest manifest;
    try
    {
        if (m_env.Verbose())
            std::cout << "Read package..." << std::endl;
        CInstallComposer extractor;
        CInstallComposer::EUpdateRules eUpdateRule = CInstallComposer::EUpdateRules::not_allowed;
        if (m_env.Overwrite())
            eUpdateRule = CInstallComposer::EUpdateRules::overwrite;
        if (m_env.Update())
            eUpdateRule = CInstallComposer::EUpdateRules::update_when_new;
        manifest = extractor.Extract(m_env.PackagePath(), m_env.InstallLocation(), eUpdateRule);

        if (m_env.Verbose())
        {
            auto vecFiles = manifest.ModuleList();
            for (const auto& rpathFile : vecFiles)
                std::cout << "Extracting " << rpathFile.generic_u8string() << "..." << std::endl;

            std::cout << "Product name: " << *manifest.Property("Product") << std::endl;
            std::cout << "Package description: " << *manifest.Property("Description") << std::endl;
            std::cout << "Author: " << *manifest.Property("Author") << std::endl;
            std::cout << "Address: " << *manifest.Property("Address") << std::endl;
            std::cout << "Copyright: " << *manifest.Property("Copyrights") << std::endl;
            std::cout << "Version: " << *manifest.Property("Version") << std::endl;
        }

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Unpacked " << manifest.ModuleList().size() << " files..." << std::endl;

        // Update the configuration
        if (!ConfigureFromManifest(manifest)) return false;
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return true;
}

bool CPackager::Copy()
{
    CInstallManifest manifest;
    try
    {
        if (m_env.Verbose())
            std::cout << "Direct installation..." << std::endl;

        CInstallComposer composer;
        size_t nCount = 0;
        for (const CSdvPackagerEnvironment::SModule& rsModule : m_env.ModuleList())
        {
            // Check for regular expression or wildcard search string
            std::string ssSearchString;
            uint32_t uiFlags = 0;
            bool bExpectFile = true; // When set, expect at least one file for the addition
            if (rsModule.ssSearchString.substr(0, 6) == "regex:")
            {
                ssSearchString = rsModule.ssSearchString.substr(6);
                uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
                bExpectFile = false;
            }
            else
            {
                ssSearchString = rsModule.ssSearchString;

                // Check for a wildcard character
                bExpectFile = ssSearchString.find_first_of("*?") != std::string::npos;

                // Wildcards are default...
                uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards);
            }

            // Should the directory structure be maintained
            if (m_env.KeepStructure())
                uiFlags |= static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);

            // Add the module
            auto vecFiles = composer.AddModule(m_env.SourceLocation(), ssSearchString, rsModule.pathRelTarget, uiFlags);
            if (bExpectFile && !vecFiles.size())
            {
                m_nError = CMDLN_SOURCE_FILE_ERROR;
                m_ssArgError = CMDLN_SOURCE_FILE_ERROR_MSG;
                return false;
            }

            // Report the files if requested
            if (m_env.Verbose())
            {
                for (const auto& rpathFile : vecFiles)
                    std::cout << "Adding " << rpathFile.generic_u8string() << "..." << std::endl;
            }
            nCount += vecFiles.size();
        }

        if (!nCount)
        {
            m_nError = NO_SOURCE_FILES;
            m_ssArgError = NO_SOURCE_FILES_MSG;
            return false;
        }

        // Add additional information as properties
        if (!m_env.ProductName().empty())
            composer.AddProperty("Product", m_env.ProductName());
        if (!m_env.Description().empty())
            composer.AddProperty("Description", m_env.Description());
        if (!m_env.Author().empty())
            composer.AddProperty("Author", m_env.Author());
        if (!m_env.Address().empty())
            composer.AddProperty("Address", m_env.Address());
        if (!m_env.Copyrights().empty())
            composer.AddProperty("Copyrights", m_env.Copyrights());
        if (!m_env.PackageVersion().empty())
            composer.AddProperty("Version", m_env.PackageVersion());

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Added " << nCount << " files..." << std::endl;
        CInstallComposer::EUpdateRules eUpdateRule = CInstallComposer::EUpdateRules::not_allowed;
        if (m_env.Overwrite())
            eUpdateRule = CInstallComposer::EUpdateRules::overwrite;
        if (m_env.Update())
            eUpdateRule = CInstallComposer::EUpdateRules::update_when_new;
        manifest = composer.ComposeDirect(m_env.InstallName(), m_env.InstallLocation(), eUpdateRule);

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Copied " << manifest.ModuleList().size() << " files..." << std::endl;

        // Update the configuration
        if (!ConfigureFromManifest(manifest)) return false;
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return true;
}

bool CPackager::Configure()
{
    if (m_env.Verbose())
        std::cout << "Configure installation..." << std::endl;

    std::vector<std::filesystem::path> vecConfigFiles;
    for (const std::string& rssConfigFileSearchString : m_env.ConfigFileList())
    {
        // Check for regular expression or wildcard search string
        std::string ssSearchString;
        EPathMatchAlgorithm eAlgorithm = EPathMatchAlgorithm::wildcards;
        if (rssConfigFileSearchString.substr(0, 6) == "regex:")
        {
            ssSearchString = rssConfigFileSearchString.substr(6);
            eAlgorithm = EPathMatchAlgorithm::regex;
        }
        else
            ssSearchString = rssConfigFileSearchString;

        std::filesystem::path pathBasePath = m_env.SourceLocation();
        if (pathBasePath.empty())
        {
            // If no base path is supplied, the file path needs to be absolute.
            if (!std::filesystem::path(ssSearchString).is_absolute())
            {
                m_nError = CMDLN_SOURCE_LOCATION_ERROR;
                m_ssArgError = CMDLN_SOURCE_LOCATION_ERROR_MSG;
                return false;
            }
        }
        else
        {
            // The base path needs to be absolute.
            if (!pathBasePath.is_absolute() || !std::filesystem::exists(pathBasePath) ||
                !std::filesystem::is_directory(pathBasePath))
            {
                m_nError = CMDLN_SOURCE_LOCATION_ERROR;
                m_ssArgError = CMDLN_SOURCE_LOCATION_ERROR_MSG;
                return false;
            }
        }

        // Check for the config search path
        if (ssSearchString.empty())
        {
            // Base path must be present
            if (pathBasePath.empty())
            {
                m_nError = CMDLN_SOURCE_LOCATION_ERROR;
                m_ssArgError = CMDLN_SOURCE_LOCATION_ERROR_MSG;
                return false;
            }
        }

        // If the pattern is defined using an absolute path, this path needs to correspond with the base path.
        std::filesystem::path pathConfigFile(ssSearchString);
        if (pathConfigFile.is_absolute())
        {
            auto itBasePart = pathBasePath.begin();
            auto itConfigPart = pathConfigFile.begin();
            while (itBasePart != pathBasePath.end())
            {
                if (itConfigPart == pathConfigFile.end() || *itBasePart != *itConfigPart)
                {
                    m_nError = CMDLN_SOURCE_LOCATION_ERROR;
                    m_ssArgError = CMDLN_SOURCE_LOCATION_ERROR_MSG;
                    return false;
                }

                // Next part
                itBasePart++;
                itConfigPart++;
            }
        }

        // Get the list of files
        auto vecFiles = CollectWildcardPath(pathBasePath, ssSearchString, eAlgorithm);

        // For each file, check whether the file is somewhere within the base path (if provided) and add the file to the file list.
        for (const std::filesystem::path& rpathFile : vecFiles)
        {
            // Check whether the files have already been added
            if (std::find_if(vecConfigFiles.begin(), vecConfigFiles.end(), [&](const std::filesystem::path& rpath) -> bool
                { return rpath == rpathFile; }) != vecConfigFiles.end())
                continue;

            // Add the file
            vecConfigFiles.push_back(rpathFile);
        }
    }

    // Check whether there are configuration files to merge
    if (vecConfigFiles.empty())
    {
        m_nError = NO_SOURCE_FILES;
        m_ssArgError = NO_SOURCE_FILES_MSG;
        return false;
    }

    // Detemine the target to update
    std::filesystem::path pathTargetConfig;
    CSdvPackagerEnvironment::CComponentVector vecUnused;
    if (m_env.Local())
        pathTargetConfig = m_env.LocalConfigFile(vecUnused);
    else
    {
        if (m_env.InsertIntoPlatformConfig(vecUnused))
        {
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::platform_config);
            pathTargetConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::platform_config);
        }
        if (m_env.InsertIntoVehicleInterfaceConfig(vecUnused))
        {
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::vehicle_interface_config);
            pathTargetConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config);
        }
        if (m_env.InsertIntoVehicleAbstractionConfig(vecUnused))
        {
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::vehicle_abstraction_config);
            pathTargetConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config);
        }
        if (m_env.InsertIntoUserConfig(vecUnused))
        {
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::user_config);
            pathTargetConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::user_config);
        }
    }
    if (pathTargetConfig.empty())
    {
        m_nError = CMDLN_MISSING_TARGET;
        m_ssArgError = CMDLN_MISSING_TARGET_MSG;
        return false;
    }
    if (!GetAppSettings().SaveSettingsFile())
    {
        SDV_LOG_ERROR("Failed to save application settings. Cannot configure components!");
        return false;
    }

    // Open the target location
    if (m_env.Verbose())
        std::cout << "Updating configuration " << pathTargetConfig.generic_u8string() << "..." << std::endl;
    CAppConfigFile configTarget(pathTargetConfig);
    if (!configTarget.LoadConfigFile())
    {
        m_nError = CANNOT_READ_CONFIG;
        m_ssArgError = CANNOT_READ_CONFIG_MSG;
        return false;
    }

    // Merge all files
    for (const auto& rpathFile : vecConfigFiles)
    {
        // Report the files if requested
        if (m_env.Verbose())
            std::cout << "Merging " << rpathFile.generic_u8string() << "..." << std::endl;
        switch (configTarget.MergeConfigFile(rpathFile))
        {
        case CAppConfigFile::EMergeResult::not_successful:
            m_nError = FAILED_UPDATING_CONFIG;
            m_ssArgError = FAILED_UPDATING_CONFIG_MSG;
            return false;
        case CAppConfigFile::EMergeResult::partly_successfull:
            m_nError = PARTLY_FAILED_UPDATING_CONFIG;
            m_ssArgError = PARTLY_FAILED_UPDATING_CONFIG_MSG;
            return false;
        default:
            break;
        }
    }

    return configTarget.SaveConfigFile();
}

bool CPackager::Remove()
{
    CInstallManifest manifest;
    try
    {
        if (m_env.Verbose())
            std::cout << "Remove installation..." << std::endl;

        CInstallComposer composer;
        manifest = composer.Remove(m_env.InstallName(), m_env.InstallLocation());

        // Report the files if requested
        if (m_env.Verbose())
        {
            auto vecFiles = manifest.ModuleList();
            for (const auto& rpathFile : vecFiles)
                std::cout << "Removing " << rpathFile.generic_u8string() << "..." << std::endl;
        }

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Removed " << manifest.ModuleList().size() << " files..." << std::endl;
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return manifest.IsValid();
}

bool CPackager::CheckIntegrity()
{
    CInstallManifest manifest;
    try
    {
        if (m_env.Verbose())
            std::cout << "Verify package..." << std::endl;

        CInstallComposer verifier;
        bool bRet = verifier.Verify(m_env.PackagePath());

        // Report count if requested
        if (!m_env.Silent())
            std::cout << "Verification of package " << (bRet ? "was successful..." : "has failed...") << std::endl;
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return true;
}

bool CPackager::ShowContent()
{
    CInstallManifest manifest;
    try
    {
        if (m_env.Verbose())
            std::cout << "Show package information..." << std::endl;

        CInstallComposer extractor;
        manifest = extractor.ExtractInstallManifest(m_env.PackagePath());

        bool bConsole = m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::console);

        // Currently no support for XML and JSON.
        if (!bConsole) return manifest.IsValid();


        bool bSimple = m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::console_simple);

        // Information
        if (m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info))
        {
            // Show information about the package
            std::cout << std::endl;
            if (!bSimple && !m_env.Silent())
                std::cout << "Information:" << std::endl;

            std::vector<std::vector<std::string>> vecTable;
            auto vecProperties = manifest.PropertyList();
            if (bSimple)
            {
                // No header, one column
                std::vector<std::string> vecInitial = {std::string("Installation=") + manifest.InstallName()};
                vecTable.push_back(vecInitial);
                for (const auto& prProperty : vecProperties)
                {
                    std::vector<std::string> vecLine = {prProperty.first + "=" + prProperty.second};
                    vecTable.push_back(vecLine);
                }
            } else
            {
                // Header and multiple columns
                std::vector<std::string> vecHeader = {"Name", "Value"};
                vecTable.push_back(vecHeader);
                std::vector<std::string> vecInitial = {"Installation", manifest.InstallName()};
                vecTable.push_back(vecInitial);
                for (const auto& prProperty : vecProperties)
                {
                    std::vector<std::string> vecLine = {prProperty.first, prProperty.second};
                    vecTable.push_back(vecLine);
                }
            }
            DrawTable(vecTable, bSimple);
        }

        // Modules
        if (m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules))
        {
            // Show module information
            std::cout << std::endl;
            if (!bSimple && !m_env.Silent())
                std::cout << "Modules:" << std::endl;

            std::vector<std::vector<std::string>> vecTable;
            if (!bSimple)
            {
                // Header only when not simple
                std::vector<std::string> vecHeader = {"Filename"};
                vecTable.push_back(vecHeader);
            }
            auto vecModules = manifest.ModuleList();
            for (const auto& pathModule : vecModules)
            {
                std::vector<std::string> vecLine = {pathModule.generic_u8string()};
                vecTable.push_back(vecLine);
            }
            DrawTable(vecTable, bSimple);
        }

        // Components
        if (m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components))
        {
            // Show component information
            std::cout << std::endl;
            if (!bSimple && !m_env.Silent())
                std::cout << "Components:" << std::endl;

            std::vector<std::vector<std::string>> vecTable;
            if (!bSimple)
            {
                // Header only when not simple
                std::vector<std::string> vecHeader = {"Class", "Alias", "Type", "Dependency"};
                vecTable.push_back(vecHeader);
            }
            auto vecClasses = manifest.ClassList();
            for (const auto& sComponent : vecClasses)
            {
                if (bSimple)
                {
                    // Simple - no header and onle column containing all classes and associated aliases.
                    vecTable.push_back(std::vector<std::string>{sComponent.ssName});
                    for (const auto& rssAlias : sComponent.seqClassAliases)
                        vecTable.push_back(std::vector<std::string>{rssAlias});
                }
                else
                {
                    // Not simple - header and multiple columns allowed
                    // Components can have a list of aliases and a list of dependencies. Add extra lines for each additional
                    // alias and dependcy.
                    size_t nIndex = 0;
                    std::vector<std::string> vecLine;
                    bool bAliasEnd = sComponent.seqClassAliases.empty(), bDependencyEnd = sComponent.seqDependencies.empty();
                    do
                    {
                        vecLine.push_back(nIndex ? "" : sComponent.ssName);
                        vecLine.push_back(bAliasEnd ? "" : sComponent.seqClassAliases[nIndex]);
                        if (!nIndex)
                            vecLine.push_back(sdv::ObjectType2String(sComponent.eType));
                        else
                            vecLine.push_back("");
                        vecLine.push_back(bDependencyEnd ? "" : sComponent.seqDependencies[nIndex]);
                        vecTable.push_back(vecLine);

                        nIndex++;
                        bAliasEnd = nIndex >= sComponent.seqClassAliases.size();
                        bDependencyEnd = nIndex >= sComponent.seqDependencies.size();
                    } while (!bAliasEnd && !bDependencyEnd);
                }
            }
            DrawTable(vecTable, bSimple);
        }
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return manifest.IsValid();
}

bool CPackager::ConfigureFromManifest(const CInstallManifest& rmanifest)
{
    // Get the class list.
    std::vector<sdv::SClassInfo> vecClasses = rmanifest.ClassList();
    CSdvPackagerEnvironment::CComponentVector vecManifestComponents, vecAddedToConfig;
    for (const sdv::SClassInfo& rsClass : vecClasses)
        vecManifestComponents.push_back(
            std::make_pair(rsClass.ssName, ""));

    // Add modules, classes and components if requested.
    if (m_env.Local())
    {
        CSdvPackagerEnvironment::CComponentVector vecComponentsToAdd;
        std::filesystem::path pathConfig = m_env.LocalConfigFile(vecComponentsToAdd);
        if (vecComponentsToAdd.empty())
            vecComponentsToAdd = vecManifestComponents; // If non are specified, add all components.
        if (!pathConfig.empty())
            WriteConfig(vecClasses, pathConfig, vecComponentsToAdd, true, vecAddedToConfig);
    }
    else
    {
        // Make certain that the settings contain the configurations if there are components to add.
        CSdvPackagerEnvironment::CComponentVector vecComponentsToAdd;
        if (m_env.InsertIntoPlatformConfig(vecComponentsToAdd))
        {
            if (vecComponentsToAdd.empty())
                vecComponentsToAdd = vecManifestComponents; // If non are specified, add all components.
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::platform_config);
            auto pathConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::platform_config);
            WriteConfig(vecClasses, pathConfig, vecComponentsToAdd, false, vecAddedToConfig);
        }
        if (m_env.InsertIntoVehicleInterfaceConfig(vecComponentsToAdd))
        {
            if (vecComponentsToAdd.empty())
                vecComponentsToAdd = vecManifestComponents; // If non are specified, add all components.
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::vehicle_interface_config);
            auto pathConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::vehicle_interface_config);
            WriteConfig(vecClasses, pathConfig, vecComponentsToAdd, false, vecAddedToConfig);
        }
        if (m_env.InsertIntoVehicleAbstractionConfig(vecComponentsToAdd))
        {
            if (vecComponentsToAdd.empty())
                vecComponentsToAdd = vecManifestComponents; // If non are specified, add all components.
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::vehicle_abstraction_config);
            auto pathConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::vehicle_abstraction_config);
            WriteConfig(vecClasses, pathConfig, vecComponentsToAdd, false, vecAddedToConfig);
        }
        if (m_env.InsertIntoUserConfig(vecComponentsToAdd))
        {
            if (vecComponentsToAdd.empty())
                vecComponentsToAdd = vecManifestComponents; // If non are specified, add all components.
            GetAppSettings().EnableConfig(CAppSettings::EConfigType::user_config);
            auto pathConfig = GetAppSettings().GetConfigPath(CAppSettings::EConfigType::user_config);
            WriteConfig(vecClasses, pathConfig, vecComponentsToAdd, true, vecAddedToConfig);
        }
        if (!GetAppSettings().SaveSettingsFile())
        {
            SDV_LOG_ERROR("Failed to save application settings. Cannot configure components!");
            return false;
        }
    }

    return true;
}

void CPackager::WriteConfig(const std::vector<sdv::SClassInfo>& rvecAllClasses, const std::filesystem::path& rpathConfig,
    const CSdvPackagerEnvironment::CComponentVector& rvecComponents, bool bUserConfig,
    CSdvPackagerEnvironment::CComponentVector& rvecAddedToConfig)
{
    CAppConfigFile file(rpathConfig);
    if (std::filesystem::exists(rpathConfig) && !file.IsLoaded())
    {
        if (!m_env.Silent())
            std::cerr << "ERROR: Failed to read existing configuration file '" << rpathConfig.generic_u8string() << "'" <<
                std::endl;
        return;
    }

    // Find the class with the corresponding class name
    auto fnFindClass = [&](const std::string& rssClassName) -> std::optional<sdv::SClassInfo>
    {
        auto itClass = std::find_if(rvecAllClasses.begin(), rvecAllClasses.end(), [&](const sdv::SClassInfo& rsClass)
            { return rsClass.ssName == rssClassName; });
        return itClass != rvecAllClasses.end() ? *itClass : std::optional<sdv::SClassInfo>();
    };

    // Add the components to the configuration.
    // For local installation, add any component. For server installation, only add complex services for user congiguration
    // and devices, basic services and system services for the other configurations. For server installation, the component
    // must be present in the installation and be added only once.
    std::vector<std::string> vecNotInInstallation, vecNotAllowed, vecDuplicate, vecFailed, vecSucceeded;
    CSdvPackagerEnvironment::CComponentVector vecComponentsCopy = rvecComponents;
    if (rvecComponents.empty()) return; // Nothing to do...
    for (const auto& rprComponent : vecComponentsCopy)
    {
        // For server components, get the corresponding class and check whether installation is allowed.
        if (!m_env.Local())
        {
            const auto& optClass = fnFindClass(rprComponent.first);
            if (!optClass)
            {
                vecNotInInstallation.push_back(rprComponent.first);
                continue;
            }
            enum class ECompatibility {compatible, incompatible, silent_incompatible} eIncompatible = ECompatibility::incompatible;
            switch (optClass->eType)
            {
            case sdv::EObjectType::device:
            case sdv::EObjectType::platform_abstraction:
            case sdv::EObjectType::vehicle_bus:
            case sdv::EObjectType::basic_service:
            case sdv::EObjectType::sensor:
            case sdv::EObjectType::actuator:
            case sdv::EObjectType::system_object:
                eIncompatible = bUserConfig ? ECompatibility::incompatible : ECompatibility::compatible;
                break;
            case sdv::EObjectType::complex_service:
            case sdv::EObjectType::vehicle_function:
            case sdv::EObjectType::utility:
                eIncompatible = bUserConfig ? ECompatibility::compatible : ECompatibility::incompatible;
                break;
            case sdv::EObjectType::proxy:
            case sdv::EObjectType::stub:
                eIncompatible = ECompatibility::silent_incompatible;
                break;
            default:
                break;
            }
            if (eIncompatible == ECompatibility::incompatible && !rvecComponents.empty())
            {
                // Only add to incompatible list if not extracted from manifest list.
                vecNotAllowed.push_back(rprComponent.first);
            }
            if (eIncompatible != ECompatibility::compatible)
                continue;
            std::string ssInstanceName = rprComponent.second;
            if (ssInstanceName.empty())
                ssInstanceName = optClass->ssDefaultObjectName;
            if (ssInstanceName.empty())
                ssInstanceName = rprComponent.first;
            auto itInstalledComponent = std::find_if(rvecAddedToConfig.begin(), rvecAddedToConfig.end(),
                [&](const auto& rprInstalledComponent) { return rprInstalledComponent.second == ssInstanceName; });
            if (itInstalledComponent != rvecAddedToConfig.end())
            {
                // Only add to duplicate list if not extracted from manifest list.
                if (!rvecComponents.empty())
                    vecDuplicate.push_back(ssInstanceName);
                continue;
            }
            rvecAddedToConfig.push_back(std::make_pair(rprComponent.first, ssInstanceName));
        }

        // Find the object instrance parameters
        TParameterVector vecParameters = m_env.ObjectParameters(rprComponent.first);

        // Add the component to the configuration...
        if (file.InsertComponent(sdv::toml::npos, std::filesystem::path(), rprComponent.first, rprComponent.second,
            vecParameters))
            vecSucceeded.push_back(rprComponent.first);
        else
            vecFailed.push_back(rprComponent.first);
    }

    // Report warnings.
    if (!vecNotInInstallation.empty())
    {
        if (!m_env.Silent())
            std::cerr << "WARNING: in the configuration file '" << rpathConfig.generic_u8string() << "' the following "
                "component classes were not part of the installation and cannot be added: ";
        bool bInitial = true;
        for (const std::string& rssName : vecNotInInstallation)
        {
            std::cerr << (bInitial ? "" : ", ") << rssName;
            bInitial = false;
        }
        std::cerr << std::endl;
    }
    if (!vecNotAllowed.empty())
    {
        if (!m_env.Silent())
            std::cerr << "WARNING: in the configuration file '" << rpathConfig.generic_u8string() << "' the following "
                "component classes are of invalid type and cannot be added: ";
        bool bInitial = true;
        for (const std::string& rssName : vecNotAllowed)
        {
            std::cerr << (bInitial ? "" : ", ") << rssName;
            bInitial = false;
        }
        std::cerr << std::endl;
    }
    if (!vecDuplicate.empty())
    {
        if (!m_env.Silent())
            std::cerr << "WARNING: in the configuration file '" << rpathConfig.generic_u8string() << "' the following "
                "component classes are duplicated and cannot be added: ";
        bool bInitial = true;
        for (const std::string& rssName : vecDuplicate)
        {
            std::cerr << (bInitial ? "" : ", ") << rssName;
            bInitial = false;
        }
        std::cerr << std::endl;
    }
    if (!vecFailed.empty())
    {
        if (!m_env.Silent())
            std::cerr << "WARNING: in the configuration file '" << rpathConfig.generic_u8string() << "' the following "
                "component classes cannot be added due ot a technical issue: ";
        bool bInitial = true;
        for (const std::string& rssName : vecFailed)
        {
            std::cerr << (bInitial ? "" : ", ") << rssName;
            bInitial = false;
        }
        std::cerr << std::endl;
    }

    // Save the configuration file.
    file.SaveConfigFile();

    // Report count if requested
    if (!m_env.Silent())
        std::cout << "Added " << vecSucceeded.size() << " components to '" << rpathConfig.generic_u8string() << "'" <<
            std::endl;
}

void CPackager::DrawTable(const std::vector<std::vector<std::string>>& rvecInfoTable, bool bSimple)
{
    if (rvecInfoTable.empty()) return;  // No columns

    // Simple? One column only
    if (bSimple)
    {
        for (const auto& rvecLine : rvecInfoTable)
        {
            if (!rvecLine.empty())
                std::cout << rvecLine[0] << std::endl;
        }
        return;
    }

    // Measure the sizes of each column.
    std::vector<size_t> vecSizes;
    for (const auto& rvecLine : rvecInfoTable)
    {
        if (vecSizes.size() < rvecLine.size())
            vecSizes.resize(rvecLine.size());
        for (size_t nIndex = 0; nIndex < rvecLine.size(); nIndex++)
        {
            if (rvecLine[nIndex].size() > vecSizes[nIndex])
                vecSizes[nIndex] = rvecLine[nIndex].size();
        }
    }

    auto fnPrintValue = [](std::stringstream& rsstream, const std::string& rssValue, size_t nLength)
    {
        rsstream << rssValue;
        if (rssValue.size() < nLength)
            rsstream << std::string(nLength - rssValue.size(), ' ');
    };

    // Print first line
    bool bInitialLine = true;
    std::stringstream sstream;
    for (const auto& rvecLine : rvecInfoTable)
    {
        // Print values
        bool bInitialCol = true;
        for (size_t nIndex = 0; nIndex < rvecLine.size(); nIndex++)
        {
            // Add apace or separator
            if (bInitialCol)
                sstream << " ";
            else
                sstream << " | ";
            bInitialCol = false;

            // Print the value
            bool bLast = nIndex == vecSizes.size() - 1;
            fnPrintValue(sstream, rvecLine[nIndex], bLast ? 0 : vecSizes[nIndex]);
        }

        // Less values than sizes... then print additional space
        for (size_t nIndex = rvecLine.size(); nIndex < vecSizes.size(); nIndex++)
        {
            // Add apace or separator
            if (bInitialCol)
                sstream << " ";
            else
                sstream << " | ";
            bInitialCol = false;

            // Print the empty space
            bool bLast = nIndex == vecSizes.size() - 1;
            fnPrintValue(sstream, "", bLast ? 0 : vecSizes[nIndex]);
        }

        // End of line
        sstream << std::endl;

        // Initial line... then print dashes
        if (bInitialLine)
        {
            bool bInitialCol1stLine = true;
            for (size_t nSize : vecSizes)
            {
                // Add apace or separator
                if (bInitialCol1stLine)
                    sstream << "-";
                else
                    sstream << "-+-";
                bInitialCol1stLine = false;
                               
                // Print dashes
                sstream << std::string(nSize, '-');
            }

            // End of line
            sstream << "-" << std::endl;
            bInitialLine = false;
        }
    }

    // Stream the table
    std::cout << sstream.str();
}

