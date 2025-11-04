#include "packager.h"
#include "../../sdv_services/core/installation_composer.h"

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
                uiFlags        = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
                bExpectFile    = false;
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
                m_nError     = CMDLN_SOURCE_FILE_ERROR;
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
            m_nError     = NO_SOURCE_FILES;
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
                uiFlags        = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
                bExpectFile    = false;
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
                m_nError     = CMDLN_SOURCE_FILE_ERROR;
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
            m_nError     = NO_SOURCE_FILES;
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
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return true;
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
        m_nError     = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return true;
}

void DrawTable(const std::vector<std::vector<std::string>>& rvecInfoTable, bool bSimple)
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
        bool bSimple = m_env.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::console_simple);

        // Currently no support for XML and JSON.
        if (bConsole)
        {
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
                auto vecComponents = manifest.ComponentList();
                for (const auto& sComponent : vecComponents)
                {
                    if (bSimple)
                    {
                        // Simple - no header and onle column containing all classes and associated aliases.
                        vecTable.push_back(std::vector<std::string>{sComponent.ssClassName});
                        for (const auto& rssAlias : sComponent.seqAliases)
                            vecTable.push_back(std::vector<std::string>{rssAlias});
                    }
                    else
                    {
                        // Not simple - header and multiple columns allowed
                        // Components can have a list of aliases and a list of dependencies. Add extra lines for each additional
                        // alias and dependcy.
                        size_t nIndex  = 0;
                        std::vector<std::string> vecLine;
                        bool bAliasEnd = sComponent.seqAliases.empty(), bDependencyEnd = sComponent.seqDependencies.empty();
                        do
                        {
                            vecLine.push_back(nIndex ? "" : sComponent.ssClassName);
                            vecLine.push_back(bAliasEnd ? "" : sComponent.seqAliases[nIndex]);
                            if (!nIndex)
                            {
                                switch (sComponent.eType)
                                {
                                case sdv::EObjectType::SystemObject:
                                    vecLine.push_back("System object");
                                    break;
                                case sdv::EObjectType::Device:
                                    vecLine.push_back("Device");
                                    break;
                                case sdv::EObjectType::BasicService:
                                    vecLine.push_back("Basic service");
                                    break;
                                case sdv::EObjectType::ComplexService:
                                    vecLine.push_back("Complex service");
                                    break;
                                case sdv::EObjectType::Application:
                                    vecLine.push_back("Application");
                                    break;
                                case sdv::EObjectType::Proxy:
                                    vecLine.push_back("Proxy object");
                                    break;
                                case sdv::EObjectType::Stub:
                                    vecLine.push_back("Stub object");
                                    break;
                                case sdv::EObjectType::Utility:
                                    vecLine.push_back("Utility");
                                    break;
                                default:
                                    vecLine.push_back("Unknown object");
                                    break;
                                }
                            }
                            else
                                vecLine.push_back("");
                            vecLine.push_back(bDependencyEnd ? "" : sComponent.seqDependencies[nIndex]);
                            vecTable.push_back(vecLine);

                            nIndex++;
                            bAliasEnd      = nIndex >= sComponent.seqAliases.size();
                            bDependencyEnd = nIndex >= sComponent.seqDependencies.size();
                        } while (!bAliasEnd && !bDependencyEnd);
                    }
                }
                DrawTable(vecTable, bSimple);
            }
        }
    }
    catch (const sdv::XSysExcept& rexception)
    {
        m_nError     = PACKAGE_READ_ERROR;
        m_ssArgError = rexception.what();
        return false;
    }
    return manifest.IsValid();
}
