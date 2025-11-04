#ifndef INSTALL_PACKAGING_H
#define INSTALL_PACKAGING_H

#include <interfaces/config.h>
#include "installation_manifest.h"
#include <list>
#include <filesystem>

/// When enabled, support the read-only flag for files. By default this flag is not enabled due to limited support by the OS.
///  - The Windows OS provides native support for this flag
///  - The Linux allows installing additional libraries providing support, but this is not supported by all Linux versions.
///  - The Posix API doesn't provide support for the read-only flag.
#define COMPOSER_SUPPORT_READONLY_LINUX 0

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface
#pragma push_macro("GetObject")
#undef GetObject

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <objbase.h>

// Resolve conflict
#pragma pop_macro("GetObject")
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

/**
 * @brief Convert the Windows FILETIME to the POSIX file time in micro-seconds.
 * @details The Windows FILETIME structure counts with 100ns resolution and starts on the 1st of January 1601. The POSIX time
 * counts with 1 second resolution and starts on the 1st of January 1970. V-API uses 1 micro-second resolution.
 * @param[in] sWinTime The WIndows file time.
 * @return The POSIX time in micro-seconds.
 */
inline uint64_t WindowsTimeToPosixTime(FILETIME sWinTime)
{
    const uint64_t uiTicksPerMicroSecond = 10ull;      // Windows counts in 100ns; POSIX in seconds
    const uint64_t uiEpochDifference = 11644473600000000ull;  // Windows starts counting on 1st Jan. 1601; POSIX on 1st Jan. 1970
    return ((static_cast<uint64_t>(sWinTime.dwHighDateTime) << 32 | static_cast<uint64_t>(sWinTime.dwLowDateTime)) /
        uiTicksPerMicroSecond) - uiEpochDifference;
}

/**
 * @brief Convert the POSIX file time in micro-seconds to the Windows FILETIME.
 * @details The Windows FILETIME structure counts with 100ns resolution and starts on the 1st of January 1601. The POSIX time
 * counts with 1 second resolution and starts on the 1st of January 1970. V-API uses 1 micro-second resolution.
 * @param[in] uiTime The POSIX time in micro-seconds.
 * @return The Windows FILETIME.
 */
inline FILETIME PosixTimeToWindowsTime(uint64_t uiTime)
{
    FILETIME sWinTime{};
    const uint64_t uiTicksPerMicroSecond = 10ull;      // Windows counts in 100ns; POSIX in seconds
    const uint64_t uiEpochDifference = 11644473600000000ull;  // Windows starts counting on 1st Jan. 1601; POSIX on 1st Jan. 1970
    sWinTime.dwLowDateTime = static_cast<uint32_t>(((uiTime + uiEpochDifference) * uiTicksPerMicroSecond) & 0xffffffffull);
    sWinTime.dwHighDateTime = static_cast<uint32_t>(((uiTime + uiEpochDifference) * uiTicksPerMicroSecond) >> 32ull);
    return sWinTime;
}
#endif

/**
 * @brief Installation packager class
 */
class CInstallComposer
{
public:
    /**
     * @brief Default constructor.
     */
    CInstallComposer() = default;

    /**
     * @brief Destructor, finalizing the installation if not done so before.
     */
    ~CInstallComposer();

    /**
     * @brief Clear the current package to allow a new package composition.
     */
    void Clear();

    /**
     * @brief Flags to use with the AddModule function.
     */
    enum class EAddModuleFlags : uint32_t
    {
        wildcards = 0x00,           ///< Add all modules that fit the modules path using wildcards to search for modules; could also
                                    ///< point to a directory. Wildcards are '*', '**' and '?'. The '*' will allow matching zero or
                                    ///< more characters until the next separator ('.' or '/'). A '**' allows matching of zero or
                                    ///< more characters across the directory separator '/'. A '?' allows matching one character.
                                    ///< Examples (using path: /dir1/dir2/dir3/dir4/file.txt)
                                    ///<  - Pattern /dir1/**/*.txt         - match
                                    ///<  - Pattern /dir?/dir?/**/dir4     - match
                                    ///<  - Pattern /**/*.txt              - match
        regex = 0x01,               ///< Add all modules that fit the modules path using regular expression to search for modules;
                                    ///< could also point to a directory.
        keep_structure = 0x10,      ///< Maintain the directory structure when adding modules (requires a valid base directory).
    };

    /**
     * @brief Installation update rules.
    */
    enum class EUpdateRules
    {
        not_allowed,        ///< Not allowed to update an existing installation.
        update_when_new,    ///< Allow to update when the installation is newer.
        overwrite,          ///< Always allow to update.
    };

    /**
     * @brief Add a module to the installation.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @attention This function is part of the composer. Since it is loading a module to retrieve the component manifest of the
     * module, module code could be executed. it therefore imposes a security risk. Do not call this function in sdv_core!
     * @details Add a module to the installation package.
     * @param[in] rpathBasePath Reference to the base path to start searching for modules. Could be empty if the module path is
     * absolute, in which case the directory structure cannot be stored at the target.
     * @param[in] rssModulePath Reference to the string containing the module path to install. Depending on the options, this path
     * could contain a pattern with wildcards or regular expressions. If the path is relative, the module base path must be valid.
     * The module path is optional. If not supplied, the base path must be provided and all files within all subdirectories will
     * be added.
     * @param[in] rpathRelTargetDir Reference to the relative target directory the module(s) should be stored at.
     * @param[in] uiFlags Zero or more flags from EAddModuleFlags.
     * @return Returns the a vector of modules that were added. In case of using wildcards or regular expression, this could also
     * be an empty list, which is not an error.
     */
    std::vector<std::filesystem::path> AddModule(const std::filesystem::path& rpathBasePath, const std::string& rssModulePath,
        const std::filesystem::path& rpathRelTargetDir = ".", uint32_t uiFlags = 0);

    /**
     * @brief Add a property value. This value will be added to the installation manifest.
     * @details Add a property value, which will be included in the installation manifest. The properties "Description" and
     * "Version" are used during package management.
     * @param[in] rssName Name of the property. Spaces are allowed. Quotes are not allowed.
     * @param[in] rssValue Property value.
     */
    void AddProperty(const std::string& rssName, const std::string& rssValue);

    /**
     * @brief Compose the package in memory.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @attention This function is part of the composer. Since it is loading a module to retrieve the component manifest of the
     * module, module code could be executed. it therefore imposes a security risk. Do not call this function in sdv_core!
     * @details Compose a package from all the modules added through the AddModule function. Additionally add an installation
     * manifest containing the module and component details. If the module is an SDV module, the component manifest will
     * automatically extracted from the component and added to the installation manifest.
     * @param[in] rssInstallName Reference to the string containing the installation name.
     * @return Returns a buffer to the package content.
     */
    sdv::pointer<uint8_t> Compose(const std::string& rssInstallName) const;

    /**
     * @brief Compose the package to disk.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @attention This function is part of the composer. Since it is loading a module to retrieve the component manifest of the
     * module, module code could be executed. it therefore imposes a security risk. Do not call this function in sdv_core!
     * @details Compose a package from all the modules added through the AddModule function. Additionally add an installation
     * manifest containing the module and component details. If the module is an SDV module, the component manifest will
     * automatically extracted from the component and added to the installation manifest.
     * @param[in] rpathPackage Reference to the path receiving the package content. Any existing package will be overwritten.
     * @param[in] rssInstallName Reference to the string containing the installation name.
     * @return Returns whether the package composing was successful.
     */
    bool Compose(const std::filesystem::path& rpathPackage, const std::string& rssInstallName) const;

    /**
     * @brief Compose the installation directly at the target directory (without package composing and extracting).
     * @throw Could throw a sdv::XSysExcept based exception.
     * @attention This function is part of the composer. Since it is loading a module to retrieve the component manifest of the
     * module, module code could be executed. it therefore imposes a security risk. Do not call this function in sdv_core!
     * @details Compose a package from all the modules added through the AddModule function. Additionally add an installation
     * manifest containing the module and component details. If the module is an SDV module, the component manifest will
     * automatically extracted from the component and added to the installation manifest.
     * @param[in] rssInstallName Reference to the string containing the installation name.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @param[in] eUpdateRule Decide how to deal with updating an existing installation.
     * @return Returns the installation manifest when the package extraction was successful; or an empty manifest when not.
     */
    CInstallManifest ComposeDirect(const std::string& rssInstallName, const std::filesystem::path& rpathInstallDir,
        EUpdateRules eUpdateRule = EUpdateRules::not_allowed) const;

    /**
     * @brief Compose an installation manifest.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @attention This function is part of the composer. Since it is loading a module to retrieve the component manifest of the
     * module, module code could be executed. it therefore imposes a security risk. Do not call this function in sdv_core!
     * @details Compose a package from all the modules added through the AddModule function. Additionally add an installation
     * manifest containing the module and component details. If the module is an SDV module, the component manifest will
     * automatically extracted from the component and added to the installation manifest.
     * @param[in] rssInstallName Reference to the string containing the installation name.
     * @return Returns an initialized installation manifest if successful, or an empty manifest if not.
     */
    CInstallManifest ComposeInstallManifest(const std::string& rssInstallName) const;

    /**
     * @brief Extract a package to an installation directory.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @pre An installation directory must be available.
     * @param[in] rptrPackage Reference to the pointer containing the package content.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @param[in] eUpdateRule Decide how to deal with updating an existing installation.
     * @return Returns the installation manifest when the package extraction was successful; or an empty manifest when not.
     */
    static CInstallManifest Extract(const sdv::pointer<uint8_t>& rptrPackage, const std::filesystem::path& rpathInstallDir,
        EUpdateRules eUpdateRule = EUpdateRules::not_allowed);

    /**
     * @brief Extract a package to an installation directory.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @pre An installation directory must be available.
     * @param[in] rpathPackage Reference to the path of the package file.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @param[in] eUpdateRule Decide how to deal with updating an existing installation.
     * @return Returns the installation manifest when the package extraction was successful; or an empty manifest when not.
     */
    static CInstallManifest Extract(const std::filesystem::path& rpathPackage, const std::filesystem::path& rpathInstallDir,
        EUpdateRules eUpdateRule = EUpdateRules::not_allowed);

    /**
     * @brief Remove an installation.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @param[in] rssInstallName Reference to the string containing the installation name.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @return Returns the installation manifest of the removed installation when the removal was successful; or an empty manifest
     * when not.
     */
    static CInstallManifest Remove(const std::string& rssInstallName, const std::filesystem::path& rpathInstallDir);

    /**
     * @brief Verify the integrity of an installation package.
     * @throw Could throw a sdv::XSysExcept based exception with information about the integrity violation.
     * @param[in] rptrPackage Reference to the pointer containing the package content.
     * @return Returns 'true' when the package extraction was successful; 'false' when not.
     */
    static bool Verify(const sdv::pointer<uint8_t>& rptrPackage);

    /**
     * @brief Verify the integrity of an installation package.
     * @throw Could throw a sdv::XSysExcept based exception with information about the integrity violation.
     * @param[in] rpathPackage Reference to the path of the package file.
     * @return Returns 'true' when the package extraction was successful; 'false' when not.
     */
    static bool Verify(const std::filesystem::path& rpathPackage);

    /**
     * @brief Extract an installation manifest from a package.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @param[in] rptrPackage Reference to the pointer containing the package content.
     * @return Returns 'true' when the package extraction was successful; 'false' when not.
     */
    static CInstallManifest ExtractInstallManifest(const sdv::pointer<uint8_t>& rptrPackage);

    /**
     * @brief Extract an installation manifest from a package.
     * @throw Could throw a sdv::XSysExcept based exception.
     * @param[in] rpathPackage Reference to the path of the package file.
     * @return Returns 'true' when the package extraction was successful; 'false' when not.
     */
    static CInstallManifest ExtractInstallManifest(const std::filesystem::path& rpathPackage);

private:
    /**
     * @brief File entry (modules and companion files).
     */
    struct SFileEntry
    {
        std::filesystem::path   pathSrcModule;      ///! Source path to the module.
        std::filesystem::path   pathRelDir;         ///< Relative directory within the installation.
    };

    /**
     * @brief Compose and serialize a package header.
     * @param[in, out] rptrPackage Reference to a pointer object receiving the serialized header. The pointer object will be
     * adjusted in size to fit the header.
     * @param[in] rmanifest Reference to the installation manifest to add.
     * @return The checksum of data stored in the package pointer.
     */
    static uint32_t SerializePackageHeader(sdv::pointer<uint8_t>& rptrPackage, const CInstallManifest& rmanifest);

    /**
     * @brief Serialize a module into a package BLOB.
     * @param[in] uiChecksumInit The initial checksum to start calculating the BLOB checksum with.
     * @param[in, out] rptrPackage Reference to a pointer object receiving the BLOB. The pointer object will be extended to fit
     * the BLOB, which will be placed following the data stored in the pointer buffer. The existing data will not be touched.
     * @param[in] rsFile Reference to the module entry to add as a BLOB.
     * @return The checksum of data stored in the package pointer.
     */
    static uint32_t SerializeModuleBLOB(uint32_t uiChecksumInit, sdv::pointer<uint8_t>& rptrPackage, const SFileEntry& rsFile);

    /**
     * @brief Serialize a final BLOB. This marks the end of the BLOBs within the package.
     * @param[in] uiChecksumInit The initial checksum to start calculating the BLOB checksum with.
     * @param[in, out] rptrPackage Reference to a pointer object receiving the BLOB. The pointer object will be extended to fit
     * the BLOB, which will be placed following the data stored in the pointer buffer. The existing data will not be touched.
     * @return The checksum of data stored in the package pointer.
     */
    static uint32_t SerializeFinalBLOB(uint32_t uiChecksumInit, sdv::pointer<uint8_t>& rptrPackage);

    /**
     * @brief Serialize the package footer.
     * @param[in] uiChecksum The checksum of the data to store in the footer.
     * @param[in, out] rptrPackage Reference to a pointer object receiving the footer. The pointer object will be extended to fit
     * the footer, which will be placed following the data stored in the pointer buffer. The existing data will not be touched.
     */
    static void SerializePackageFooter(uint32_t uiChecksum, sdv::pointer<uint8_t>& rptrPackage);

    /**
     * @brief Extracts the header from the file.
     * @details This function reads the header from the file from the current position and checks whether the content fits the
     * checksum.
     * @param[in, out] rfstream Reference to the opened package stream at the initial position.
     * @param[out] ruiChecksum Reference to the variable receiving the checksum calculated over the complete header. This checksum
     * is used as input for the checksum of the following structures.
     * @return Returns the content of the package header.
     */
    static sdv::installation::SPackageHeader DeserializeHeader(std::ifstream& rfstream, uint32_t& ruiChecksum);

    /**
     * @brief Extracts the header from the file.
     * @details This function reads the header from the file from the current position and checks whether the content fits the
     * checksum.
     * @param[in] rptrPackage Reference to the package containing the header.
     * @param[out] rnOffset Reference to the variable receiving the offset location following the header.
     * @param[out] ruiChecksum Reference to the variable receiving the checksum calculated over the complete header. This checksum
     * is used as input for the checksum of the following structures.
     * @return Returns the content of the package header.
     */
    static sdv::installation::SPackageHeader DeserializeHeader(const sdv::pointer<uint8_t>& rptrPackage, size_t& rnOffset,
        uint32_t& ruiChecksum);

    /**
     * @brief Extracts the BLOB from the file.
     * @details This function reads the BLOB from the file from the current position and checks whether the content fits the
     * checksum.
     * @param[in, out] rfstream Reference to the opened package stream at the current position.
     * @param[in, out] ruiChecksum Reference to the variable receiving the checksum calculated over the complete BLOB. This
     * checksum is used as input for the checksum of the following structures.
     * @return Returns the content of the package header.
     */
    static sdv::installation::SPackageBLOB DeserializeBLOB(std::ifstream& rfstream, uint32_t& ruiChecksum);

    /**
     * @brief Extracts the BLOB from the file.
     * @details This function reads the BLOB from the file from the current position and checks whether the content fits the
     * checksum.
     * @param[in] rptrPackage Reference to the package containing the BLOB.
     * @param[in, out] rnOffset Reference to the variable containing the offset to the BLOB and receiving the offset following the
     * BLOB.
     * @param[in, out] ruiChecksum Reference to the variable receiving the checksum calculated over the complete BLOB. This
     * checksum is used as input for the checksum of the following structures.
     * @return Returns the content of the package header.
     */
    static sdv::installation::SPackageBLOB DeserializeBLOB(const sdv::pointer<uint8_t>& rptrPackage, size_t& rnOffset, uint32_t& ruiChecksum);

    /**
     * @brief Check the final checksum using the package footer.
     * @param[in, out] rfstream Reference to the opened package stream at the current position.
     * @param[in] uiChecksum The calculated checksum of the package content to use for the calculation of the checksum of the
     * footer.
     */
    static void DeserializeFinalChecksum(std::ifstream& rfstream, uint32_t uiChecksum);

    /**
     * @brief Check the final checksum using the package footer.
     * @param[in] rptrPackage Reference to the package containing the footer with the checksum.
     * @param[in] nOffset The variable containing the offset to the footer.
     * @param[in] uiChecksum The calculated checksum of the package content to use for the calculation of the checksum of the
     * footer.
     */
    static void DeserializeFinalChecksum(const sdv::pointer<uint8_t>& rptrPackage, size_t nOffset, uint32_t uiChecksum);

    /**
     * @brief Store the module from the supplied BLOB.
     * @param[in] rsModuleBLOB Reference to the BLOB structure containing the module. If the BLOB doesn't contain the module,
     * nothing occurs.
     * @param[in] rpathLocation Reference to the path containing the location to store the module to.
     */
    static void StoreModuleBLOB(const sdv::installation::SPackageBLOB& rsModuleBLOB, const std::filesystem::path& rpathLocation);

    /**
     * @brief Store the installation manifest at the provided location and set the creation time for the manifest file.
     * @param[in] uiCreationTime The time of the package creation (will be used for the manifest file).
     * @param[in] rpathLocation Reference to the path containing the location to store the manifest to.
     * @param[in] rmanifest Reference to the manifest to store.
     */
    static void StoreManifest(const std::filesystem::path& rpathLocation, const CInstallManifest& rmanifest, int64_t uiCreationTime);

    /**
     * @brief Set the file to read-only.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static void SetReadOnly(const std::filesystem::path& rpathFile);

    /**
     * @brief Get the file read-only-state.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static bool IsReadOnly(const std::filesystem::path& rpathFile);

    /**
     * @brief Set the file to executable.
     * @remarks Not available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static void SetExecutable(const std::filesystem::path& rpathFile);

    /**
     * @brief Get the file to executable state.
     * @remarks Not available for WIndows.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static bool IsExecutable(const std::filesystem::path& rpathFile);

    /**
     * @brief Set the file creation time.
     * @remarks Only available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     * @param[in] uiTimeMicrosec The unix epoch time in microseconds.
     */
    static void SetCreateTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec);

    /**
     * @brief Get the file creation time.
     * @remarks Only available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     * @return Returns the unix epoch time in microseconds.
     */
    static uint64_t GetCreateTime(const std::filesystem::path& rpathFile);

    /**
     * @brief Set the file change time.
     * @param[in] rpathFile Reference to the path of the file.
     * @param[in] uiTimeMicrosec The unix epoch time in microseconds.
     */
    static void SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec);

    /**
     * @brief Get the file change time.
     * @param[in] rpathFile Reference to the path of the file.
     * @return Returns the unix epoch time in microseconds.
     */
    static uint64_t GetChangeTime(const std::filesystem::path& rpathFile);

    /**
     * @brief Check whether thr installed version can be overwritten. This is the case when the version number of the current
     * package is larger than the version number of the installed package.
     * @remarks Returns 'true' if the current installation doesn't have an installation manifest.
     * @remarks If there is no version stored in the installation manifest, the version number is considered 0.0.0.
     * @param[in] rpathInstall Reference to the installation directory of the current installation. This directory must contain a
     * "install_manifest.toml" file.
     * @param[in] sVersionNew The version to use for comparison.
     * @param[in] eUpdateRule Decide how to deal with updating an existing installation.
     * @return Returns whether the existing installation can be updated (removed and installed again).
     */
    static bool UpdateExistingInstallation(const std::filesystem::path& rpathInstall, sdv::installation::SPackageVersion sVersionNew, EUpdateRules eUpdateRule);

    std::list<SFileEntry>               m_lstFiles;         ///< List of modules added to the package
    std::map<std::string, std::string>  m_mapProperties;    ///< Property map.
};

#endif // !defined INSTALL_PACKAGING_H