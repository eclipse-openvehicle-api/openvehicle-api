#include "installation_composer.h"
#include <ctime>
#include <fstream>
#include <utility>
#include <regex>
#include <interfaces/serdes/config_serdes.h>
#include "../global/path_match.h"

#ifdef _WIN32
#include <Windows.h>
#elif defined __unix__
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <utime.h>
#if defined __linux__
#include <linux/fs.h>
#if COMPOSER_SUPPORT_READONLY_LINUX
#include <sys/capability.h>
#endif
#endif
#else
#error OS is not supported!
#endif

CInstallComposer::~CInstallComposer()
{}

void CInstallComposer::Clear()
{
    m_lstFiles.clear();
}

std::vector<std::filesystem::path> CInstallComposer::AddModule(const std::filesystem::path& rpathBasePath,
    const std::string& rssModulePath, const std::filesystem::path& rpathRelTargetDir /*= "."*/, uint32_t uiFlags /*= 0*/)
{
    std::vector<std::filesystem::path> vecFilesReturned;

    // Check for the flags
    bool bKeepStructure = (uiFlags & static_cast<uint32_t>(EAddModuleFlags::keep_structure)) ? true : false;
    EPathMatchAlgorithm eAlgorithm = EPathMatchAlgorithm::wildcards;
    if (uiFlags & static_cast<uint32_t>(EAddModuleFlags::regex))
        eAlgorithm = EPathMatchAlgorithm::regex;

    if (rpathBasePath.empty())
    {
        // If no base path is supplied, the module path needs to be absolute.
        if (!std::filesystem::path(rssModulePath).is_absolute())
        {
            sdv::XInvalidPath exception;
            exception.ssPath = rssModulePath;
            throw exception;
        }

        // If the structure needs to be maintained, the base path needs to be supplied.
        if (bKeepStructure) throw sdv::installation::XMissingBasePath();
    }
    else
    {
        // If a base path is supplied, the path needs to be absolute.
        if (!rpathBasePath.is_absolute() || !std::filesystem::exists(rpathBasePath) || !std::filesystem::is_directory(rpathBasePath))
        {
            sdv::XInvalidPath exception;
            exception.ssPath = rpathRelTargetDir.generic_u8string();
            throw exception;
        }
    }

    // Check for the module path
    std::string ssModulePathCopy = rssModulePath;
    if (ssModulePathCopy.empty())
    {
        // Base path must be present
        if (rpathBasePath.empty())
            throw sdv::installation::XMissingBasePath();
    }

    // Check for a relative target path
    if (!rpathRelTargetDir.empty() && !rpathRelTargetDir.is_relative())
    {
        sdv::XInvalidPath exception;
        exception.ssPath = rpathRelTargetDir.generic_u8string();
        throw exception;
    }
    if (RefersToRelativeParent(rpathRelTargetDir))
    {
        sdv::XInvalidPath exception;
        exception.ssPath = rpathRelTargetDir.generic_u8string();
        throw exception;
    }

    // If the pattern is defined using an absolute path, this path needs to correspond with the base path.
    std::filesystem::path pathModulePath(rssModulePath);
    if (pathModulePath.is_absolute())
    {
        auto itBasePart = rpathBasePath.begin();
        auto itModulePart = pathModulePath.begin();
        while (itBasePart != rpathBasePath.end())
        {
            if (itModulePart == pathModulePath.end() ||
                *itBasePart != *itModulePart)
            {
                sdv::XInvalidPath exception;
                exception.ssPath = rssModulePath;
                throw exception;
            }

            // Next part
            itBasePart++;
            itModulePart++;
        }
    }

    // Get the list of files
    auto vecFiles = CollectWildcardPath(rpathBasePath, ssModulePathCopy, eAlgorithm);

    // For each file, check whether the file is somewhere within the base path (if provided) and add the file to the module list.
    for (const std::filesystem::path& rpathFile : vecFiles)
    {
        // Get the relative source directory if a base path exists.
        std::filesystem::path pathRelSourceDir;
        if (!rpathBasePath.empty() && rpathFile.is_absolute())
        {
            pathRelSourceDir = rpathFile.parent_path().lexically_relative(rpathBasePath);
            if (pathRelSourceDir.empty()) continue; // No path available
            if (*pathRelSourceDir.begin() == "..") continue; // File is not relative to base path (or higher up the base path).
        }
        else
            pathRelSourceDir = rpathFile.parent_path();

        // Create the relative target directory path
        std::filesystem::path pathRelTargetDirCopy = rpathRelTargetDir;
        if (bKeepStructure && !pathRelSourceDir.empty())
            pathRelTargetDirCopy /= pathRelSourceDir;
        pathRelTargetDirCopy = pathRelTargetDirCopy.lexically_normal();

        // Check whether the files have already been added
        if (std::find_if(m_lstFiles.begin(), m_lstFiles.end(), [&](const SFileEntry& rEntry) -> bool
            {
                return rEntry.pathRelDir / rEntry.pathSrcModule.filename() == pathRelTargetDirCopy / rpathFile.filename();
            }) != m_lstFiles.end())
        {
            sdv::XDuplicateFile exception;
            exception.ssFilePath = (pathRelTargetDirCopy / rpathFile.filename()).generic_u8string();
            throw exception;
        }

        // Add the file
        m_lstFiles.push_back(SFileEntry{ rpathBasePath / rpathFile, pathRelTargetDirCopy });
        vecFilesReturned.push_back(rpathFile);
    }

    return vecFilesReturned;
}

void CInstallComposer::AddProperty(const std::string& rssName, const std::string& rssValue)
{
    m_mapProperties[rssName] = rssValue;
}

sdv::pointer<uint8_t> CInstallComposer::Compose(const std::string& rssInstallName) const
{
    if (m_lstFiles.empty()) return {};

    // Installation manifest
    CInstallManifest manifest;
    if (!manifest.Create(rssInstallName))
    {
        sdv::installation::XFailedManifestCreation exception;
        exception.ssInstallName = rssInstallName;
        throw exception;
    }

    // Create the manifest entries for the files
    for (const auto& rsFileEntry : m_lstFiles)
    {
        if (!std::filesystem::exists(rsFileEntry.pathSrcModule) || !std::filesystem::is_regular_file(rsFileEntry.pathSrcModule))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }

        // Add the module to the manifest
        if (!manifest.AddModule(rsFileEntry.pathSrcModule, rsFileEntry.pathRelDir))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }
    }

    // Add the properties to the manifest
    for (const auto& rvtProperty : m_mapProperties)
        manifest.Property(rvtProperty.first, rvtProperty.second);

    // Fill in the header
    sdv::pointer<uint8_t> ptrPackage;
    uint32_t uiChecksum = SerializePackageHeader(ptrPackage, manifest);

    // Create the file description structures
    for (const auto& rsFileEntry : m_lstFiles)
        uiChecksum = SerializeModuleBLOB(uiChecksum, ptrPackage, rsFileEntry);

    // Add the final BLOB
    uiChecksum = SerializeFinalBLOB(uiChecksum, ptrPackage);

    // Add the footer
    SerializePackageFooter(uiChecksum, ptrPackage);

    return ptrPackage;
}

bool CInstallComposer::Compose(const std::filesystem::path& rpathPackage, const std::string& rssInstallName) const
{
    if (m_lstFiles.empty()) return false;

    // Stream the package content to the file.
    std::ofstream fstream(rpathPackage.native().c_str(), std::ios::binary);
    if (!fstream.is_open())
    {
        sdv::XCannotOpenFile exception;
        exception.ssPath = rpathPackage.generic_u8string();
        throw exception;
    }

    // Installation manifest
    CInstallManifest manifest;
    if (!manifest.Create(rssInstallName))
    {
        sdv::installation::XFailedManifestCreation exception;
        exception.ssInstallName = rssInstallName;
        throw exception;
    }

    // Create the manifest entries for the files
    for (const auto& rsFileEntry : m_lstFiles)
    {
        if (!std::filesystem::exists(rsFileEntry.pathSrcModule) || !std::filesystem::is_regular_file(rsFileEntry.pathSrcModule))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }

        // Add the module to the manifest
        if (!manifest.AddModule(rsFileEntry.pathSrcModule, rsFileEntry.pathRelDir))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }
    }

    // Add the properties to the manifest
    for (const auto& rvtProperty : m_mapProperties)
        manifest.Property(rvtProperty.first, rvtProperty.second);

    // Stream the header
    sdv::pointer<uint8_t> ptrPackage;
    uint32_t uiChecksum = SerializePackageHeader(ptrPackage, manifest);
    sdv::to_stream(ptrPackage, fstream.rdbuf());

    // Create the file description structures
    for (const auto& rsFileEntry : m_lstFiles)
    {
        // Add the package content
        ptrPackage.reset();
        uiChecksum = SerializeModuleBLOB(uiChecksum, ptrPackage, rsFileEntry);
        sdv::to_stream(ptrPackage, fstream.rdbuf());
    }

    // Add the final BLOB
    ptrPackage.reset();
    uiChecksum = SerializeFinalBLOB(uiChecksum, ptrPackage);
    sdv::to_stream(ptrPackage, fstream.rdbuf());

    // Add the footer
    ptrPackage.reset();
    SerializePackageFooter(uiChecksum, ptrPackage);
    sdv::to_stream(ptrPackage, fstream.rdbuf());

    fstream.close();

    return true;
}

CInstallManifest CInstallComposer::ComposeDirect(const std::string& rssInstallName,
    const std::filesystem::path& rpathInstallDir, EUpdateRules eUpdateRule /*= EUpdateRules::not_allowed*/) const
{
    if (m_lstFiles.empty()) return {};

    // Check for the target path
    std::filesystem::path pathTargetDir = rpathInstallDir / std::filesystem::u8path(rssInstallName);

    // Check whether there are any files in the target directory
    bool bExistingInstallation = false;
    try
    {
        if (std::filesystem::is_directory(pathTargetDir))
        {
            for (auto const& sDirEntry : std::filesystem::directory_iterator{pathTargetDir})
            {
                if (sDirEntry.path() != ".." && sDirEntry.path() != ".")
                {
                    bExistingInstallation = true;
                    break;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotRemoveDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Is there an existing installation?
    if (bExistingInstallation)
    {
        // Get the version number
        const auto& itVersion = m_mapProperties.find("Version");
        sdv::installation::SPackageVersion sVersionNew{};
        if (itVersion != m_mapProperties.end())
            sVersionNew = InterpretVersionString(itVersion->second);

        // Allowed to update?
        if (!UpdateExistingInstallation(pathTargetDir, sVersionNew, eUpdateRule))
        {
            // Not allowed to update.
            sdv::installation::XDuplicateInstall exception;
            exception.ssInstallName = rssInstallName;
            throw exception;
        }

        // TODO EVE: Uninstall from the configuration...

        // Delete the directory (this represents the uninstallation process).
        try
        {
            std::filesystem::remove_all(pathTargetDir);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            sdv::XCannotRemoveDir exception;
            exception.ssPath = pathTargetDir.generic_u8string();
            throw exception;
        }
    }

    // Create the target directory
    try
    {
        std::filesystem::create_directories(pathTargetDir);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotCreateDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // CReate the installation manifest
    CInstallManifest manifest;
    if (!manifest.Create(rssInstallName))
    {
        sdv::installation::XFailedManifestCreation exception;
        exception.ssInstallName = rssInstallName;
        throw exception;
    }

    // Create the manifest entries for the files
    for (const auto& rsFileEntry : m_lstFiles)
    {
        if (!std::filesystem::exists(rsFileEntry.pathSrcModule) || !std::filesystem::is_regular_file(rsFileEntry.pathSrcModule))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }

        // Add the module to the manifest
        if (!manifest.AddModule(rsFileEntry.pathSrcModule, rsFileEntry.pathRelDir))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }
    }

    // Add the properties to the manifest
    for (const auto& rvtProperty : m_mapProperties)
        manifest.Property(rvtProperty.first, rvtProperty.second);

    // Store the installation manifest (with the current creation date).
    StoreManifest(pathTargetDir, manifest, static_cast<uint64_t>(std::time(nullptr)) * 1000000ull);

    // Copy the files
    for (const auto& rsFile : m_lstFiles)
    {
        std::filesystem::path pathFile =
            (pathTargetDir / rsFile.pathRelDir / rsFile.pathSrcModule.filename()).lexically_normal().u8string();
        bool bAttrReadonly             = IsReadOnly(rsFile.pathSrcModule);
        bool bAttrExecutable           = IsExecutable(rsFile.pathSrcModule);
        uint64_t uiCreationDate        = GetCreateTime(rsFile.pathSrcModule);
        uint64_t uiChangeDate          = GetChangeTime(rsFile.pathSrcModule);

        // Target directory exists?
        std::filesystem::path pathDir = pathFile.parent_path();
        if (!std::filesystem::exists(pathDir) || !std::filesystem::is_directory(pathDir))
        {
            try
            {
                std::filesystem::create_directories(pathDir);
            }
            catch (const std::filesystem::filesystem_error&)
            {
                sdv::XCannotCreateDir exception;
                exception.ssPath = pathDir.generic_u8string();
                throw exception;
            }
        }

        // Copy file content
        std::ifstream fstreamSource(rsFile.pathSrcModule.native().c_str(), std::ios::binary);
        if (!fstreamSource.is_open())
        {
            sdv::XCannotOpenFile exception;
            exception.ssPath = rsFile.pathSrcModule.generic_u8string();
            throw exception;
        }
        std::ofstream fstreamTarget(pathFile.native().c_str(), std::ios::binary);
        if (!fstreamTarget.is_open())
        {
            sdv::XCannotOpenFile exception;
            exception.ssPath = pathFile.generic_u8string();
            throw exception;
        }
        fstreamTarget << fstreamSource.rdbuf();
        fstreamSource.close();
        fstreamTarget.close();

            // Set file times and attributes (read-only last) - so far as supported by OS
        SetCreateTime(pathFile, uiCreationDate);
        SetChangeTime(pathFile, uiChangeDate);
        if (bAttrExecutable) SetExecutable(pathFile);
        if (bAttrReadonly) SetReadOnly(pathFile);
    }

    return manifest;
}

CInstallManifest CInstallComposer::ComposeInstallManifest(const std::string& rssInstallName) const
{
    if (m_lstFiles.empty()) return {};

    // Installation manifest
    CInstallManifest manifest;
    if (!manifest.Create(rssInstallName))
    {
        sdv::installation::XFailedManifestCreation exception;
        exception.ssInstallName = rssInstallName;
        throw exception;
    }

    // Add the files to the manifest
    for (const auto& rsFileEntry : m_lstFiles)
    {
        if (!std::filesystem::exists(rsFileEntry.pathSrcModule) || !std::filesystem::is_regular_file(rsFileEntry.pathSrcModule))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }

        // Add the module to the manifest
        if (!manifest.AddModule(rsFileEntry.pathSrcModule, rsFileEntry.pathRelDir))
        {
            sdv::installation::XModuleNotFound exception;
            exception.ssPath = rsFileEntry.pathSrcModule.generic_u8string();
            throw exception;
        }
    }

    // Add the properties to the manifest
    for (const auto& rvtProperty : m_mapProperties)
        manifest.Property(rvtProperty.first, rvtProperty.second);

    return manifest;
}

CInstallManifest CInstallComposer::Extract(const sdv::pointer<uint8_t>& rptrPackage, const std::filesystem::path& rpathInstallDir,
    EUpdateRules eUpdateRule /*= EUpdateRules::not_allowed*/)
{
    uint32_t uiChecksum = 0;
    size_t nOffset = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(rptrPackage, nOffset, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();

    // Check for the target path
    std::filesystem::path pathTargetDir = rpathInstallDir / std::filesystem::u8path(manifest.InstallName());

    // Check whether there are any files in the target directory
    bool bExistingInstallation = false;
    try
    {
        if (std::filesystem::is_directory(pathTargetDir))
        {
            for (auto const& sDirEntry : std::filesystem::directory_iterator{pathTargetDir})
            {
                if (sDirEntry.path() != ".." && sDirEntry.path() != ".")
                {
                    bExistingInstallation = true;
                    break;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotRemoveDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Is there an existing installation?
    if (bExistingInstallation)
    {
        // Allowed to update?
        if (!UpdateExistingInstallation(pathTargetDir, manifest.Version(), eUpdateRule))
        {
            // Not allowed to update.
            sdv::installation::XDuplicateInstall exception;
            exception.ssInstallName = manifest.InstallName();
            throw exception;
        }

        // TODO EVE: Uninstall from the configuration...

        // Delete the directory (this represents the uninstallation process).
        try
        {
            std::filesystem::remove_all(pathTargetDir);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            sdv::XCannotRemoveDir exception;
            exception.ssPath = pathTargetDir.generic_u8string();
            throw exception;
        }
    }

    // Create the target directory
    try
    {
        std::filesystem::create_directories(pathTargetDir);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotCreateDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Store the installation manifest.
    StoreManifest(pathTargetDir, manifest, sHeader.uiCreationDate);

    // Iterate through the BLOBs
    bool bFinal = false;
    while (!bFinal && nOffset < rptrPackage.size())
    {
        sdv::installation::SPackageBLOB sBLOB = DeserializeBLOB(rptrPackage, nOffset, uiChecksum);
        switch (sBLOB.get_switch())
        {
        case sdv::installation::EPackageBLOBType::binary_file:
            StoreModuleBLOB(sBLOB, pathTargetDir);
            break;
        case sdv::installation::EPackageBLOBType::final_entry:
            bFinal = true;
            break;
        default:
            // Do not know how to deal with this section...
            break;
        }
    }

    // When no final BLOB was extracted, the package is corrupted
    if (!bFinal)
    {
        sdv::XBufferTooSmall exception;
        exception.uiSize = nOffset;
        exception.uiCapacity = rptrPackage.size();
        throw exception;
    }

    // Check for final checksum
    DeserializeFinalChecksum(rptrPackage, nOffset, uiChecksum);

    return manifest;
}

CInstallManifest CInstallComposer::Extract(const std::filesystem::path& rpathPackage, const std::filesystem::path& rpathInstallDir,
    EUpdateRules eUpdateRule /*= EUpdateRules::not_allowed*/)
{
    // Stream the package content from the file.
    std::ifstream fstream(rpathPackage, std::ios::binary);
    if (!fstream.is_open())
    {
        sdv::XCannotOpenFile exception;
        exception.ssPath = rpathPackage.generic_u8string();
        throw exception;
    }

    uint32_t uiChecksum = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(fstream, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();

    // Check for the target path
    std::filesystem::path pathTargetDir = rpathInstallDir / std::filesystem::u8path(manifest.InstallName());

    // Check whether there are any files in the target directory
    bool bExistingInstallation = false;
    try
    {
        if (std::filesystem::is_directory(pathTargetDir))
        {
            for (auto const& sDirEntry : std::filesystem::directory_iterator{pathTargetDir})
            {
                if (sDirEntry.path() != ".." && sDirEntry.path() != ".")
                {
                    bExistingInstallation = true;
                    break;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotRemoveDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Is there an existing installation?
    if (bExistingInstallation)
    {
        // Allowed to update?
        if (!UpdateExistingInstallation(pathTargetDir, manifest.Version(), eUpdateRule))
        {
            // Not allowed to update.
            sdv::installation::XDuplicateInstall exception;
            exception.ssInstallName = manifest.InstallName();
            throw exception;
        }

        // TODO EVE: Uninstall from the configuration...

        // Delete the directory (this represents the uninstallation process).
        try
        {
            std::filesystem::remove_all(pathTargetDir);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            sdv::XCannotRemoveDir exception;
            exception.ssPath = pathTargetDir.generic_u8string();
            throw exception;
        }
    }

    // Create the target directory
    try
    {
        std::filesystem::create_directories(pathTargetDir);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotCreateDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Store the installation manifest.
    StoreManifest(pathTargetDir, manifest, sHeader.uiCreationDate);

    // Iterate through the BLOBs
    bool bFinal = false;
    while (!bFinal)
    {
        sdv::installation::SPackageBLOB sBLOB = DeserializeBLOB(fstream, uiChecksum);
        switch (sBLOB.get_switch())
        {
        case sdv::installation::EPackageBLOBType::binary_file:
            StoreModuleBLOB(sBLOB, pathTargetDir);
            break;
        case sdv::installation::EPackageBLOBType::final_entry:
            bFinal = true;
            break;
        default:
            // Do not know how to deal with this section...
            break;
        }
    }

    // When no final BLOB was extracted, the package is corrupted
    if (!bFinal)
    {
        sdv::XFileCorrupt exception;
        exception.ssPath = rpathPackage.generic_u8string();
        throw exception;
    }

    // Check for final checksum
    DeserializeFinalChecksum(fstream, uiChecksum);

    // Close the stream
    fstream.close();

    return manifest;
}

CInstallManifest CInstallComposer::Remove(const std::string& rssInstallName, const std::filesystem::path& rpathInstallDir)
{
    // Check for the target path
    std::filesystem::path pathTargetDir = rpathInstallDir / std::filesystem::u8path(rssInstallName);

    // Read the installation manifest.
    CInstallManifest manifest;
    if (!manifest.Load(pathTargetDir)) return {};

    // Check whether there are any files in the target directory
    bool bExistingInstallation = false;
    try
    {
        if (std::filesystem::is_directory(pathTargetDir))
        {
            for (auto const& sDirEntry : std::filesystem::directory_iterator{pathTargetDir})
            {
                if (sDirEntry.path() != ".." && sDirEntry.path() != ".")
                {
                    bExistingInstallation = true;
                    break;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        sdv::XCannotRemoveDir exception;
        exception.ssPath = pathTargetDir.generic_u8string();
        throw exception;
    }

    // Is there an existing installation?
    if (bExistingInstallation)
    {
        // TODO EVE: Uninstall from the configuration...

        // Delete the directory (this represents the uninstallation process).
        try
        {
            std::filesystem::remove_all(pathTargetDir);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            sdv::XCannotRemoveDir exception;
            exception.ssPath = pathTargetDir.generic_u8string();
            throw exception;
        }
    }

    return manifest;
}

bool CInstallComposer::Verify(const sdv::pointer<uint8_t>& rptrPackage)
{
    uint32_t uiChecksum = 0;
    size_t nOffset = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(rptrPackage, nOffset, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();

    // Iterate through the BLOBs
    bool bFinal = false;
    while (!bFinal && nOffset < rptrPackage.size())
    {
        sdv::installation::SPackageBLOB sBLOB = DeserializeBLOB(rptrPackage, nOffset, uiChecksum);
        switch (sBLOB.get_switch())
        {
        case sdv::installation::EPackageBLOBType::binary_file:
            break;
        case sdv::installation::EPackageBLOBType::final_entry:
            bFinal = true;
            break;
        default:
            // Do not know how to deal with this section...
            break;
        }
    }

    // When no final BLOB was extracted, the package is corrupted
    if (!bFinal)
    {
        sdv::XBufferTooSmall exception;
        exception.uiSize     = nOffset;
        exception.uiCapacity = rptrPackage.size();
        throw exception;
    }

    // Check for final checksum
    DeserializeFinalChecksum(rptrPackage, nOffset, uiChecksum);

    return true;
}

bool CInstallComposer::Verify(const std::filesystem::path& rpathPackage)
{
    // Stream the package content from the file.
    std::ifstream fstream(rpathPackage, std::ios::binary);
    if (!fstream.is_open()) return false;

    uint32_t uiChecksum = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(fstream, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();

    // Iterate through the BLOBs
    bool bFinal = false;
    while (!bFinal)
    {
        sdv::installation::SPackageBLOB sBLOB = DeserializeBLOB(fstream, uiChecksum);
        switch (sBLOB.get_switch())
        {
        case sdv::installation::EPackageBLOBType::binary_file:
            break;
        case sdv::installation::EPackageBLOBType::final_entry:
            bFinal = true;
            break;
        default:
            // Do not know how to deal with this section...
            break;
        }
    }

    // When no final BLOB was extracted, the package is corrupted
    if (!bFinal) return false;

    // Check for final checksum
    DeserializeFinalChecksum(fstream, uiChecksum);

    // Close the stream
    fstream.close();

    return true;
}

CInstallManifest CInstallComposer::ExtractInstallManifest(const sdv::pointer<uint8_t>& rptrPackage)
{
    uint32_t uiChecksum = 0;
    size_t nOffset      = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(rptrPackage, nOffset, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();

    return manifest;
}

CInstallManifest CInstallComposer::ExtractInstallManifest(const std::filesystem::path& rpathPackage)
{
    // Stream the package content from the file.
    std::ifstream fstream(rpathPackage, std::ios::binary);
    if (!fstream.is_open())
    {
        sdv::XCannotOpenFile exception;
        exception.ssPath = rpathPackage.generic_u8string();
        throw exception;
    }

    uint32_t uiChecksum = 0;

    // Extract the header
    sdv::installation::SPackageHeader sHeader = DeserializeHeader(fstream, uiChecksum);

    // Read the manifest
    CInstallManifest manifest;
    if (!manifest.Read(sHeader.ssManifest) || !manifest.IsValid() || manifest.InstallName().empty())
        throw sdv::installation::XInvalidManifest();
    
    // Close the stream
    fstream.close();

    return manifest;
}

uint32_t CInstallComposer::SerializePackageHeader(sdv::pointer<uint8_t>& rptrPackage, const CInstallManifest& rmanifest)
{
    // Fill in the header
    sdv::installation::SPackageHeader sHeader{};
    sdv::installation::SPackageHeaderChecksum sHeaderChecksum{};
    sHeader.eEndian = sdv::GetPlatformEndianess();
    sHeader.uiVersion = SDVFrameworkInterfaceVersion;
    const uint8_t rguiSignature[] = { 'S', 'D', 'V', '_', 'I', 'P', 'C', 'K' };
    std::copy(std::begin(rguiSignature), std::end(rguiSignature), sHeader.rguiSignature);
    sHeader.uiCreationDate = static_cast<uint64_t>(std::time(nullptr)) * 1000000ull;
    sHeader.ssManifest = rmanifest.Write();
    if (sHeader.ssManifest.empty())
    {
        sdv::installation::XFailedManifestCreation exception;
        exception.ssInstallName = rmanifest.InstallName();
        throw exception;
    }

    // Calculate the size of the header (including padding and checksum)
    size_t nSize = 0;
    sdv::ser_size(sHeader, nSize);
    sdv::ser_size(sHeaderChecksum, nSize);
    if (nSize % 8) nSize += 8 - nSize % 8;
    sHeader.uiOffset = static_cast<uint32_t>(nSize);

    // Serialize the package header
    sdv::serializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> serializer;
    rptrPackage.reset();
    serializer.attach(std::move(rptrPackage));
    serializer << sHeader;

    // Add padding
    while (serializer.offset() < nSize - sizeof(sdv::installation::SPackageHeaderChecksum))
    {
        uint8_t uiPadding = 0;
        serializer << uiPadding;
    }

    // Fill in and serialize the checksum
    sHeaderChecksum.uiChecksum = serializer.checksum();
    serializer << sHeaderChecksum;

    // Detach and return the current checksum calculated over the header (including header checksum)
    uint32_t uiChecksum = serializer.checksum();
    serializer.detach(rptrPackage);

    return uiChecksum;
}

uint32_t CInstallComposer::SerializeModuleBLOB(uint32_t uiChecksumInit, sdv::pointer<uint8_t>& rptrPackage,
    const SFileEntry& rsFile)
{
    // Fill in the module BLOB
    sdv::installation::SPackageBLOB sBLOB{};
    sBLOB.switch_to(sdv::installation::EPackageBLOBType::binary_file);
    sBLOB.sFileDesc.ssFileName = (rsFile.pathRelDir / rsFile.pathSrcModule.filename()).lexically_normal().u8string();
    sBLOB.sFileDesc.bAttrReadonly = IsReadOnly(rsFile.pathSrcModule);
    sBLOB.sFileDesc.bAttrExecutable = IsExecutable(rsFile.pathSrcModule);
    sBLOB.sFileDesc.uiCreationDate = GetCreateTime(rsFile.pathSrcModule);
    sBLOB.sFileDesc.uiChangeDate = GetChangeTime(rsFile.pathSrcModule);
    sBLOB.uiChecksumInit = uiChecksumInit;

    // File content
    std::ifstream fstream(rsFile.pathSrcModule.native().c_str(), std::ios::binary);
    if (!fstream.is_open())
    {
        sdv::XCannotOpenFile exception;
        exception.ssPath = rsFile.pathSrcModule.generic_u8string();
        throw exception;
    }
    sBLOB.sFileDesc.ptrContent = sdv::from_stream(fstream.rdbuf());
    fstream.close();

    // Calculate the size of the BLOB (including padding and checksum)
    sdv::installation::SPackageBLOBChecksum sBLOBChecksum{};
    size_t nSize = 0;
    sdv::ser_size(sBLOB, nSize);
    sdv::ser_size(sBLOBChecksum, nSize);
    if (nSize % 8) nSize += 8 - nSize % 8;
    sBLOB.uiBLOBSize = static_cast<uint32_t>(nSize);

    // Serialize the BLOB
    sdv::serializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> serializer;
    size_t nOffset = rptrPackage.size();
    serializer.attach(std::move(rptrPackage), nOffset, uiChecksumInit);
    serializer << sBLOB;

    // Add padding
    while (serializer.offset() - nOffset < nSize - sizeof(sdv::installation::SPackageBLOBChecksum))
    {
        uint8_t uiPadding = 0;
        serializer << uiPadding;
    }

    // Fill in and serialize the checksum
    sBLOBChecksum.uiChecksum = serializer.checksum();
    serializer << sBLOBChecksum;

    // Detach and return the current checksum calculated over the BLOB (including BLOB content checksum)
    uint32_t uiChecksum = serializer.checksum();
    serializer.detach(rptrPackage);
    return uiChecksum;
}

uint32_t CInstallComposer::SerializeFinalBLOB(uint32_t uiChecksumInit, sdv::pointer<uint8_t>& rptrPackage)
{
    // Fill in the module BLOB
    sdv::installation::SPackageBLOB sBLOB{};
    sBLOB.switch_to(sdv::installation::EPackageBLOBType::final_entry);
    sBLOB.uiChecksumInit = uiChecksumInit;

    // Calculate the size of the BLOB (including padding and checksum)
    sdv::installation::SPackageBLOBChecksum sBLOBChecksum{};
    size_t nSize = 0;
    sdv::ser_size(sBLOB, nSize);
    sdv::ser_size(sBLOBChecksum, nSize);
    if (nSize % 8) nSize += 8 - nSize % 8;
    sBLOB.uiBLOBSize = static_cast<uint32_t>(nSize);

    // Serialize the BLOB
    sdv::serializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> serializer;
    size_t nOffset = rptrPackage.size();
    serializer.attach(std::move(rptrPackage), nOffset, uiChecksumInit);
    serializer << sBLOB;

    // Add padding
    while (serializer.offset() - nOffset < nSize - sizeof(sdv::installation::SPackageBLOBChecksum))
    {
        uint8_t uiPadding = 0;
        serializer << uiPadding;
    }

    // Fill in and serialize the checksum
    sBLOBChecksum.uiChecksum = serializer.checksum();
    serializer << sBLOBChecksum;

    // Detach and return the current checksum calculated over the BLOB (including BLOB content checksum)
    uint32_t uiChecksum = serializer.checksum();
    serializer.detach(rptrPackage);
    return uiChecksum;
}

void CInstallComposer::SerializePackageFooter(uint32_t uiChecksum, sdv::pointer<uint8_t>& rptrPackage)
{
    // Fill in and serialize the checksum
    sdv::serializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> serializer;
    size_t nOffset = rptrPackage.size();
    serializer.attach(std::move(rptrPackage), nOffset, uiChecksum);
    sdv::installation::SPackageFooter sFooter{};
    sFooter.uiChecksum = uiChecksum;
    serializer << sFooter;

    // Detach the package
    serializer.detach(rptrPackage);
}

sdv::installation::SPackageHeader CInstallComposer::DeserializeHeader(std::ifstream& rfstream, uint32_t& ruiChecksum)
{
    // Prepare the buffer
    sdv::pointer<uint8_t> ptrHeader;
    size_t nPartialSize = offsetof(sdv::installation::SPackageHeader, rguiSignature) +
        sizeof(sdv::installation::SPackageHeader::rguiSignature);
    ptrHeader.resize(std::max(nPartialSize, sizeof(sdv::installation::SPackageHeader)));

    // Partially read the header
    rfstream.read(reinterpret_cast<char*>(ptrHeader.get()), nPartialSize);

    // NOTE: The serialization process of plain data is identical to casting. Therefore, the beginning of the data fits the
    // SPackageHeader structure until the dynamic types are added.
    const sdv::installation::SPackageHeader* psPartialHeader = sdv::cast<sdv::installation::SPackageHeader>(ptrHeader);
    if (!psPartialHeader) throw sdv::installation::XIncompatiblePackage();

    // Check for the correct version of the package
    const uint8_t rguiSignature[] = { 'S', 'D', 'V', '_', 'I', 'P', 'C', 'K' };
    if (psPartialHeader->eEndian != sdv::GetPlatformEndianess() ||
        psPartialHeader->uiVersion != SDVFrameworkInterfaceVersion ||
        !std::equal(std::begin(rguiSignature), std::end(rguiSignature), psPartialHeader->rguiSignature))
        throw sdv::installation::XIncompatiblePackage();

    // Check the offset (max 32k, min at least the size of the two header structs).
    sdv::installation::SPackageHeader sHeader{};
    sdv::installation::SPackageHeaderChecksum sHeaderChecksum{};
    if (sHeader.uiOffset > 32768)
        throw sdv::installation::XIncompatiblePackage();
    size_t nMinSize = 0;
    sdv::ser_size(sHeader, nMinSize);
    sdv::ser_size(sHeaderChecksum, nMinSize);
    if (psPartialHeader->uiOffset < nMinSize)
        throw sdv::installation::XIncompatiblePackage();

    // Extend the buffer and read the header data (after the resize, the psPartialHeader pointer could be invalidated).
    uint32_t uiHeaderSize = psPartialHeader->uiOffset;
    ptrHeader.resize(uiHeaderSize);
    if (!ptrHeader)
    {
        sdv::XBufferTooSmall exception;
        exception.uiSize = uiHeaderSize;
        exception.uiCapacity = ptrHeader.size();
        throw exception;
    }
    rfstream.read(reinterpret_cast<char*>(ptrHeader.get() + nPartialSize), uiHeaderSize - nPartialSize);

    // Deserialize into the header
    sdv::deserializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> deserializer;
    deserializer.attach(ptrHeader);
    deserializer >> sHeader;

    // Skip padding
    while (deserializer.offset() < uiHeaderSize - sizeof(sdv::installation::SPackageHeaderChecksum))
    {
        uint8_t uiPadding = 0;
        deserializer >> uiPadding;
    }

    // Deserialize and check the header checksum
    uint64_t uiChecksum = deserializer.checksum();
    deserializer >> sHeaderChecksum;
    if (uiChecksum != sHeaderChecksum.uiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Update the checksum variables
    ruiChecksum = deserializer.checksum(); // Checksum including checksum-structure of the header

    return sHeader;
}

sdv::installation::SPackageHeader CInstallComposer::DeserializeHeader(const sdv::pointer<uint8_t>& rptrPackage, size_t& rnOffset,
    uint32_t& ruiChecksum)
{
    // Deserialize the package header
    sdv::deserializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> deserializer;
    deserializer.attach(rptrPackage);
    sdv::installation::SPackageHeader sHeader{};
    sdv::installation::SPackageHeaderChecksum sHeaderChecksum{};
    deserializer >> sHeader;

    // Check for the correct version of the package
    const uint8_t rguiSignature[] = { 'S', 'D', 'V', '_', 'I', 'P', 'C', 'K' };
    if (sHeader.eEndian != sdv::GetPlatformEndianess() ||
        sHeader.uiVersion != SDVFrameworkInterfaceVersion ||
        !std::equal(std::begin(rguiSignature), std::end(rguiSignature), sHeader.rguiSignature))
        throw sdv::installation::XIncompatiblePackage();

    // Check the offset (max package size, min at least the size of the two header structs).
    if (sHeader.uiOffset > rptrPackage.size() - sizeof(sdv::installation::SPackageFooter))
        throw sdv::installation::XIncompatiblePackage();
    size_t nMinSize = 0;
    sdv::ser_size(sHeader, nMinSize);
    sdv::ser_size(sHeaderChecksum, nMinSize);
    if (sHeader.uiOffset < nMinSize)
        throw sdv::installation::XIncompatiblePackage();

    // Skip padding
    while (deserializer.offset() < sHeader.uiOffset - sizeof(sdv::installation::SPackageHeaderChecksum))
    {
        uint8_t uiPadding = 0;
        deserializer >> uiPadding;
    }

    // Deserialize and check the header checksum
    uint64_t uiChecksum = deserializer.checksum();
    deserializer >> sHeaderChecksum;
    if (uiChecksum != sHeaderChecksum.uiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Update the offset and checksum variables
    rnOffset = deserializer.offset();
    ruiChecksum = deserializer.checksum(); // Checksum including checksum-structure of the header

    return sHeader;
}

sdv::installation::SPackageBLOB CInstallComposer::DeserializeBLOB(std::ifstream& rfstream, uint32_t& ruiChecksum)
{
    // Prepare the buffer
    sdv::installation::SPackageBLOB sBLOB{};
    sdv::installation::SPackageBLOBChecksum sBLOBChecksum{};
    sdv::pointer<uint8_t> ptrBLOB;
    // NOTE: The macro offsetof works, but should not be used with non-POD structures.
    size_t nPartialSize = reinterpret_cast<uint64_t>(&sBLOB.uiBLOBSize) - reinterpret_cast<uint64_t>(&sBLOB) + sizeof(uint32_t);
    ptrBLOB.resize(std::max(nPartialSize, sizeof(sdv::installation::SPackageBLOB)));

    // Partially read the header
    rfstream.read(reinterpret_cast<char*>(ptrBLOB.get()), nPartialSize);

    // NOTE: The serialization process of plain data is identical to casting. Therefore, the beginning of the data fits the
    // SPackageBLOB structure until the dynamic types are added.
    const sdv::installation::SPackageBLOB* psPartialBLOB = sdv::cast<sdv::installation::SPackageBLOB>(ptrBLOB);
    if (!psPartialBLOB) throw sdv::installation::XIncompatiblePackage();

    // Check for a correct initial checksum
    if (psPartialBLOB->uiChecksumInit != ruiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Check the size
    size_t nMinSize = 0;
    sdv::ser_size(sBLOB, nMinSize);
    sdv::ser_size(sBLOBChecksum, nMinSize);
    if (psPartialBLOB->uiBLOBSize < nMinSize)
        throw sdv::installation::XIncompatiblePackage();

    // Extend the buffer and read the BLOB data (after the resize, the psPartialBLOB pointer could be invalidated).
    uint32_t uiBLOBSize = psPartialBLOB->uiBLOBSize;
    if (uiBLOBSize > 24*1024*1024)
        throw sdv::installation::XIncompatiblePackage();    // Safety
    ptrBLOB.resize(uiBLOBSize);
    if (!ptrBLOB)
    {
        sdv::XBufferTooSmall exception;
        exception.uiSize = uiBLOBSize;
        exception.uiCapacity = ptrBLOB.size();
        throw exception;
    }
    rfstream.read(reinterpret_cast<char*>(ptrBLOB.get() + nPartialSize), static_cast<size_t>(uiBLOBSize) - nPartialSize);
    if (rfstream.gcount() != static_cast<std::streamsize>(uiBLOBSize) - static_cast<std::streamsize>(nPartialSize))
        throw sdv::installation::XIncompatiblePackage();

    // Deserialize into the BLOB structure
    sdv::deserializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> deserializer;
    deserializer.attach(ptrBLOB);
    deserializer.jump(0, ruiChecksum);
    deserializer >> sBLOB;

    // Skip padding
    while (deserializer.offset() < uiBLOBSize - sizeof(sdv::installation::SPackageBLOBChecksum))
    {
        uint8_t uiPadding = 0;
        deserializer >> uiPadding;
    }

    // Deserialize and check the BLOB checksum
    uint64_t uiChecksum = deserializer.checksum();
    deserializer >> sBLOBChecksum;
    if (uiChecksum != sBLOBChecksum.uiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Update the checksum variables
    ruiChecksum = deserializer.checksum(); // Checksum including checksum-structure of the BLOB

    return sBLOB;
}

sdv::installation::SPackageBLOB CInstallComposer::DeserializeBLOB(const sdv::pointer<uint8_t>& rptrPackage, size_t& rnOffset,
    uint32_t& ruiChecksum)
{
    // Deserialize the BLOB
    sdv::deserializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> deserializer;
    deserializer.attach(rptrPackage);
    deserializer.jump(rnOffset, ruiChecksum);
    sdv::installation::SPackageBLOB sBLOB{};
    sdv::installation::SPackageBLOBChecksum sBLOBChecksum{};
    deserializer >> sBLOB;
    if (sBLOB.uiBLOBSize + rnOffset > rptrPackage.size())
        throw sdv::installation::XIncompatiblePackage();    // Safety

    // Check the initial checksum
    if (sBLOB.uiChecksumInit != ruiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Skip padding
    while (deserializer.offset() - rnOffset < sBLOB.uiBLOBSize - sizeof(sdv::installation::SPackageBLOBChecksum))
    {
        uint8_t uiPadding = 0;
        deserializer >> uiPadding;
    }

    // Deserialize and check the header checksum
    uint64_t uiChecksum = deserializer.checksum();
    deserializer >> sBLOBChecksum;
    if (uiChecksum != sBLOBChecksum.uiChecksum)
        throw sdv::installation::XIncorrectCRC();

    // Update the offset and checksum variables
    rnOffset = deserializer.offset();
    ruiChecksum = deserializer.checksum(); // Checksum including checksum-structure of the BLOB

    return sBLOB;
}

void CInstallComposer::DeserializeFinalChecksum(std::ifstream& rfstream, uint32_t uiChecksum)
{
    // Prepare the buffer
    sdv::pointer<uint8_t> ptrFooter;
    ptrFooter.resize(sizeof(sdv::installation::SPackageFooter));

    // Read the footer
    rfstream.read(reinterpret_cast<char*>(ptrFooter.get()), sizeof(sdv::installation::SPackageFooter));

    // Cast to the footer
    const sdv::installation::SPackageFooter* psFooter = sdv::cast<sdv::installation::SPackageFooter>(ptrFooter);
    if (!psFooter) throw sdv::installation::XIncompatiblePackage();

    if (uiChecksum != psFooter->uiChecksum)
        throw sdv::installation::XIncorrectCRC();
}

void CInstallComposer::DeserializeFinalChecksum(const sdv::pointer<uint8_t>& rptrPackage, size_t nOffset, uint32_t uiChecksum)
{
    // Deserialize the footer
    sdv::deserializer<sdv::GetPlatformEndianess(), sdv::crcCRC32C> deserializer;
    deserializer.attach(rptrPackage);
    deserializer.jump(nOffset, uiChecksum);
    sdv::installation::SPackageFooter sFooter{};
    deserializer >> sFooter;

    if (uiChecksum != sFooter.uiChecksum)
        throw sdv::installation::XIncorrectCRC();
}

void CInstallComposer::StoreModuleBLOB(const sdv::installation::SPackageBLOB& rsModuleBLOB,
    const std::filesystem::path& rpathLocation)
{
    if (rsModuleBLOB.get_switch() != sdv::installation::EPackageBLOBType::binary_file) return;

    // File path
    if (IsParentPath(rpathLocation, static_cast<std::string>(rsModuleBLOB.sFileDesc.ssFileName)))
    {
        sdv::XInvalidPath exception;
        exception.ssPath = rsModuleBLOB.sFileDesc.ssFileName;
        throw exception;

    }
    std::filesystem::path pathFile = rpathLocation / static_cast<std::string>(rsModuleBLOB.sFileDesc.ssFileName);

    // Target directory exists?
    std::filesystem::path pathDir = pathFile.parent_path();
    if (!std::filesystem::exists(pathDir) || !std::filesystem::is_directory(pathDir))
    {
        try
        {
            std::filesystem::create_directories(pathDir);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            sdv::XCannotCreateDir exception;
            exception.ssPath = pathDir.generic_u8string();
            throw exception;
        }
    }

    // File content
    std::ofstream fstream(pathFile.native().c_str(), std::ios::binary);
    if (!fstream.is_open())
    {
        sdv::XCannotOpenFile exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    sdv::to_stream(rsModuleBLOB.sFileDesc.ptrContent, fstream.rdbuf());
    fstream.close();

    // Set file times and attributes (read-only last) - so far as supported by OS
    SetCreateTime(pathFile, rsModuleBLOB.sFileDesc.uiCreationDate);
    SetChangeTime(pathFile, rsModuleBLOB.sFileDesc.uiChangeDate);
    if (rsModuleBLOB.sFileDesc.bAttrExecutable)
        SetExecutable(pathFile);
    if (rsModuleBLOB.sFileDesc.bAttrReadonly)
        SetReadOnly(pathFile);
}

void CInstallComposer::StoreManifest(const std::filesystem::path& rpathLocation, const CInstallManifest& rmanifest, int64_t uiCreationTime)
{
    // Save the manifest to the target directory
    if (!rmanifest.Save(rpathLocation))
    {
        sdv::installation::XFailedSaveManifest exception;
        exception.ssInstallName = rmanifest.InstallName();
        exception.ssPath = (rpathLocation / "install_manifest.toml").generic_u8string();
        throw exception;
    }

    // Set the creation and change times (for supporting OSes) for the manifest file.
    SetCreateTime(rpathLocation / "install_manifest.toml", uiCreationTime);
    SetChangeTime(rpathLocation / "install_manifest.toml", uiCreationTime);
}

#ifdef _WIN32
void CInstallComposer::SetReadOnly(const std::filesystem::path& rpathFile)
{
    // Get the file attributes
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set the file attributes
    SetFileAttributesW(rpathFile.native().c_str(), sFileAttr.dwFileAttributes | FILE_ATTRIBUTE_READONLY);
}

bool CInstallComposer::IsReadOnly(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return false;
    return (sFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? true : false;
}

#elif defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX
void CInstallComposer::SetReadOnly(const std::filesystem::path& rpathFile)
{
    // Enable the immutable capability
    cap_t caps = cap_get_proc();
    {
        perror("cap_get_proc error");
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    cap_value_t capList[1] = {CAP_LINUX_IMMUTABLE};
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capList, CAP_SET) == -1)
    {
        perror("cap_set_flag error");
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }

    // Set the immutable flag
    int fd = open(pathFile.native().c_str(), O_RDONLY);
    if (fd == -1)
    {
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    int flags = 0;
    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1)
    {
        perror("ioctl error");
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }

    flags |= FS_IMMUTABLE_FL;
    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1)
    {
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    close(fd);
    cap_free(caps);
}

bool CInstallComposer::IsReadOnly(const std::filesystem::path& rpathFile)
{
    struct statx sStat{};
    sStat.stx_mask = STATX_BTIME | STATX_MTIME;
    if (statx(0, rsModuleEntry.pathSrcModule.native().c_str(), 0, STATX_BTIME | STATX_MTIME, &sStat) ||
        sStat.stx_btime.tv_sec > 2524608000 || sStat.stx_btime.tv_nsec > 999999999 || sStat.stx_mtime.tv_sec > 2524608000 ||
        sStat.stx_mtime.tv_nsec > 999999999)
        return false;
    return sStat.stx_attributes_mask & STATX_ATTR_IMMUTABLE && sStat.stx_attributes & STATX_ATTR_IMMUTABLE;
}

#else
void CInstallComposer::SetReadOnly(const std::filesystem::path& /*rpathFile*/)
{
    // Do nothing, since not supported by OS.
}

bool CInstallComposer::IsReadOnly(const std::filesystem::path& /*rpathFile*/)
{
    // Do nothing, since not supported by OS.
    return false;
}

#endif

#ifdef __unix__
void CInstallComposer::SetExecutable(const std::filesystem::path& rpathFile)
{
    chmod(rpathFile.native().c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

bool CInstallComposer::IsExecutable(const std::filesystem::path& rpathFile)
{
    struct stat sStat{};
    if (stat(rpathFile.native().c_str(), &sStat) == 0 && S_ISREG(sStat.st_mode))
    {
        // Check if any execute bit is set
        return (sStat.st_mode & S_IXUSR) || (sStat.st_mode & S_IXGRP) || (sStat.st_mode & S_IXOTH);
    }
    return false;
}

#else
void CInstallComposer::SetExecutable(const std::filesystem::path& /*rpathFile*/)
{
    // Do nothing, since not supported by OS.
}

bool CInstallComposer::IsExecutable(const std::filesystem::path& /*rpathFile*/)
{
    // Do nothing, since not supported by OS.
    return false;
}
#endif

#ifdef _WIN32
void CInstallComposer::SetCreateTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    // Get the file times
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set file times
    HANDLE hFile = CreateFile(rpathFile.native().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    FILETIME sCreationTime = PosixTimeToWindowsTime(uiTimeMicrosec);
    FILETIME sAccessTime = sFileAttr.ftLastAccessTime;
    FILETIME sWriteTime = sFileAttr.ftLastWriteTime;
    SetFileTime(hFile, &sCreationTime, &sAccessTime, &sWriteTime);
    CloseHandle(hFile);
}

uint64_t CInstallComposer::GetCreateTime(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return 0ull;
    return WindowsTimeToPosixTime(sFileAttr.ftCreationTime);
}

void CInstallComposer::SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    // Get the file times
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set file times
    HANDLE hFile = CreateFile(rpathFile.native().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    FILETIME sCreationTime = sFileAttr.ftCreationTime;
    FILETIME sAccessTime = sFileAttr.ftLastAccessTime;
    FILETIME sWriteTime = PosixTimeToWindowsTime(uiTimeMicrosec);
    SetFileTime(hFile, &sCreationTime, &sAccessTime, &sWriteTime);
    CloseHandle(hFile);
}

uint64_t CInstallComposer::GetChangeTime(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return 0ull;
    return WindowsTimeToPosixTime(sFileAttr.ftLastWriteTime);
}

#else
void CInstallComposer::SetCreateTime(const std::filesystem::path& /*rpathFile*/, uint64_t /*uiTimeMicrosec*/)
{
    // Do nothing, since not supported by OS.
}

uint64_t CInstallComposer::GetCreateTime(const std::filesystem::path& /*rpathFile*/)
{
    // Do nothing, since not supported by OS.
    return 0ull;
}

void CInstallComposer::SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    struct timespec sTimes[2] = {};
    sTimes[0].tv_sec = static_cast<time_t>(uiTimeMicrosec / 1000000ull); // Set access to change time
    sTimes[0].tv_nsec = uiTimeMicrosec % 1000000ull * 1000ull;
    sTimes[1].tv_sec = static_cast<time_t>(uiTimeMicrosec / 1000000ull); // Set modification time to change time
    sTimes[1].tv_nsec = uiTimeMicrosec % 1000000ull * 1000ull;
    // Test for illegal time values... tv_sec should be below 1st of January 2050; tv_nsec should be below or equal to 999999999.
    if (sTimes[0].tv_sec >= 2524608000 || sTimes[0].tv_nsec > 999999999)
        return;

    // Set the times
    utimensat(AT_FDCWD, rpathFile.native().c_str(), sTimes, 0);
}

uint64_t CInstallComposer::GetChangeTime(const std::filesystem::path& rpathFile)
{
    // Test for illegal time values... tv_sec should be below 1st of January 2050; tv_nsec should be below or equal to 999999999.
    struct stat sStat{};
    if (stat(rpathFile.u8string().c_str(), &sStat) || sStat.st_mtim.tv_sec >= 2524608000 ||
        sStat.st_mtim.tv_nsec > 999999999)
        return 0;
    return static_cast<uint64_t>(sStat.st_mtim.tv_sec) * 1000000ull + static_cast<uint64_t>(sStat.st_mtim.tv_nsec) / 1000ull;
}
#endif

bool CInstallComposer::UpdateExistingInstallation(const std::filesystem::path& rpathInstall,
    sdv::installation::SPackageVersion sVersionNew, EUpdateRules eUpdateRule)
{
    // Determine what to do.
    switch (eUpdateRule)
    {
    case EUpdateRules::overwrite:
        return true;
    case EUpdateRules::update_when_new:
        break;
    case EUpdateRules::not_allowed:
    default:
        return false;
    }

    // Check if the new version number is higher.
    std::filesystem::path pathOldManifest = rpathInstall / "install_manifest.toml";
    if (!std::filesystem::is_regular_file(pathOldManifest))
        return true;
    try
    {
        CInstallManifest manifestOld;
        manifestOld.Load(rpathInstall);
        return sVersionNew > manifestOld.Version();
    } catch (const sdv::XSysExcept&)
    {
        // Invalid installation - do not allow updating
        return false;
    }
}
