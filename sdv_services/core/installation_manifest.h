#ifndef INSTALL_MANIFEST_H
#define INSTALL_MANIFEST_H

#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <interfaces/core.h>
#include <interfaces/config.h>
#include <map>
#include <cstdlib>

/**
 * @brief Check whether a relative path is directing to a parent of the base path.
 * @details Detect whether a relative path joined to the base path is referring to a parent of the base path. For example, the
 * base path is "/home/user/project" and the relative path is "..". Both joined together would make "/home/user", which is a parent
 * of the base path.
 * @param[in] rpathBase Reference to the base path.
 * @param[in] rpathRelative Reference to the relative path following the base.
 * @return Returns whether the relative path is a parent of the base (not the base or a child of the base).
 */
inline bool IsParentPath(const std::filesystem::path& rpathBase, const std::filesystem::path& rpathRelative)
{
    auto pathAbsBase = std::filesystem::weakly_canonical(rpathBase);
    auto pathAbsPotentialParent = std::filesystem::weakly_canonical(rpathBase / rpathRelative);

    // Check if pathAbsPotentialParent supersedes pathAbsBase. If not, it points to or derives from a parent.
    auto itBase = pathAbsBase.begin();
    auto itPotentialParent = pathAbsPotentialParent.begin();
    while (itBase != pathAbsBase.end())
    {
        // If the potential path has finished, it is pointing to a parent.
        if (itPotentialParent == pathAbsPotentialParent.end()) return true;

        // If the path-parts are not equal, the potential parent part is really deriving from a parent.
        if (*itBase != *itPotentialParent) return true;

        // Check next
        itBase++;
        itPotentialParent++;
    }

    // Even if the potential path might be still have more parts, they derive from the base path.
    return false;
}

/**
 * @brief Check whether the relative path directs to a parent path.
 * @details Detect whether a relative path directs to a parent path. For example with "./dir1/dir2/../dir3"; this is not the case,
 * but with "./dir1/dir2/../../../dir3" this is the case.
 * @param[in] rpathRelative Reference to the relative path to check for.
 * @return Returns whether the relative path refers to the
 */
inline bool RefersToRelativeParent(const std::filesystem::path& rpathRelative)
{
    int iDepth = 0;
    for (const auto& pathPart : rpathRelative)
    {
        if (pathPart == "..")
            --iDepth;
        else if (pathPart != "." && pathPart != "")
            ++iDepth;
        if (iDepth < 0) return true;   // Not allowed to be negative.
    }
    return false;
}

/**
 * @brief Class managing the installation manifest.
 * @details The installation manifest is a TOML file with the name "install_manifest.toml" and the following format:
 * @code
 * [Installation]
 * Version = 100                        # Installation manifest version.
 * Name = "Duck"                        # Name of the installation. This typically is identical to the installation directory name.
 *
 * [Properties]
 * Product = "Wild goose components"    # Product name
 * Description = "Mallard installation" # Description
 * Author = "Nils Holgerson"            # Author
 * Address = """Vildgåsvägen 7
 * Skanör med Falsterbo
 * Skåne län, 123 45
 * Sverige"""                           # Address
 * Copyrights = "(C) 2025 Wild goose"   # Copyrights
 * Version = "0.1.2.3"                  # Package version
 * 
 * [[Module]]
 * Path = "mallard.sdv                  # Relative path to the module
 *
 * [[Module.Component]]                 # Component manifest
 * Class = "Mallard class"              # The name of the class
 * Aliases = ["Duck", "Pont duck"]      # Optional list of aliases
 * DefaultName = "Duck"                 # Optional default name for the class instance
 * Type = "Complex service"             # Component type (Device, BasicService, ComplexService, App, Proxy, Stub, Utility)
 * Singleton = false                    # Optional singleton flag
 * Dependencies = ["Bird", "Animal"]    # Optional list of dependencies
 *
 * [[Module.Component]]                 # Another component manifest
 * #...
 *
 * [[Module]]                           # Another module
 * Path = "large/greylag_goose.sdv      # Relative path to the module
 * @endcode
 * @remarks The installation directory path is used to create relative paths to the modules. It is not stored in the manifest
 * itself allowing the manifest to be copied from one location to another as long as the relative path to the modules is maintained
 * (meaning copying the modules along with the manifest).
 */
class CInstallManifest
{
public:
    /**
     * @brief Manifest information belonging to the component.
     */
    struct SComponent
    {
        std::filesystem::path pathRelModule;            ///< Relative module path (relative to the installation directory).
        std::string ssManifest;                         ///< Component manifest.
        std::string ssClassName;                        ///< String representing the class name.
        sdv::sequence<sdv::u8string> seqAliases;        ///< Sequence containing zero or more class name aliases.
        std::string ssDefaultObjectName;                ///< The default object name.
        sdv::EObjectType eType;                         ///< Type of object.
        uint32_t uiFlags;                               ///< Zero or more object flags from EObjectFlags.
        sdv::sequence<sdv::u8string> seqDependencies;   ///< This component depends on...
    };

    /**
     * @brief Default constructor.
     */
    CInstallManifest() = default;

    /**
     * @brief Is this a valid installation manifest (installation directory path is known and a name is given)?
     * @return Returns whether the installation manifest is valid.
     */
    bool IsValid() const;

    /**
     * @brief Clear the current manifest to start a new manifest.
     */
    void Clear();

    /**
     * @brief Get the installation name.
     * @return Reference to the string holding the installation name.
     */
    const std::string& InstallName() const;

    /**
     * @brief Get the package version from the version property.
     * @remarks The version property can hold additional information, which is not available through this function.
     * @return Structure containing the version information.
     */
    sdv::installation::SPackageVersion Version() const;

    /**
     * @brief Get the installation directory path.
     * @return Reference to the path to the installation.
     */
    const std::filesystem::path& InstallDir() const;

    /**
     * @brief Create a new manifest.
     * @param[in] rssInstallName Reference to the string containing the installaltion name.
     * @return Returns whether manifest creation was successful.
     */
    bool Create(const std::string& rssInstallName);

    /**
     * @brief Load a manifest TOML file.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @param[in] bBlockSystemObjects When set, system objects are not stored in the repository.
     * @return Returns whether loading the TOML file was successful.
     */
    bool Load(const std::filesystem::path& rpathInstallDir, bool bBlockSystemObjects = false);

    /**
     * @brief Save a manifest TOML file.
     * @param[in] rpathInstallDir Reference to the installation directory.
     * @return Returns whether saving the TOML file was successful.
     */
    bool Save(const std::filesystem::path& rpathInstallDir) const;

    /**
     * @brief Read a manifest TOML string.
     * @param[in] rssManifest Reference to the string containing the manifest.
     * @param[in] bBlockSystemObjects When set, system objects are not stored in the repository.
     * @return Returns whether reading the TOML string was successful.
     */
    bool Read(const std::string& rssManifest, bool bBlockSystemObjects = false);

    /**
     * @brief Write a manifest TOML string.
     * @return The manifest TOML string when successful or an empty string when not.
     */
    std::string Write() const;

    /**
     * @brief Add a module to the installation manifest (if the module contains components).
     * @attention This function should not be called by the core application, since it imposes a security risk!
     * @param[in] rpathModulePath Reference to the module path.
     * @param[in] rpathRelTargetDir The relative target directory the module should be stored at.
     * @return Returns 'true' when successful; 'false' when not.
     */
    bool AddModule(const std::filesystem::path& rpathModulePath, const std::filesystem::path& rpathRelTargetDir = ".");

    /**
     * @brief Find the module stored in the installation manifest.
     * @pre Only successful for manifests having an installation directory.
     * @param[in] rpathRelModule Reference to the path containing the relative path to a module.
     * @return Returns the full path if the module was found or an empty path when not.
     */
    std::filesystem::path FindModule(const std::filesystem::path& rpathRelModule) const;

    /**
     * @brief Find the module manifest.
     * @pre Only successful for manifests having an installation directory.
     * @param[in] rpathRelModule Reference to the path containing the relative path to a module.
     * @return Returns the string containing the module manifest.
     */
    std::string FindModuleManifest(const std::filesystem::path& rpathRelModule) const;

    /**
     * @brief Find the component stored in this installation manifest.
     * @param[in] rssClass Reference to the class name of the component.
     * @return The component manifest information.
     */
    std::optional<SComponent> FindComponentByClass(const std::string& rssClass) const;

    /**
     * @brief Get a vector of all components stored in this installation manifest.
     * @return The component manifest vector.
     */
    std::vector<SComponent> ComponentList() const;

    /**
     * @brief Get the module list.
     * @return Returns a vector with paths to all modules relative to the installation directory.
     */
    std::vector<std::filesystem::path> ModuleList() const;

    /**
     * @brief Get the property list.
     * @return Returns a vector with properties and the corresponding values.
     */
    std::vector<std::pair<std::string, std::string>> PropertyList() const;

    /**
     * @brief Set a property value.
     * @details Set a property value, which will be included in the installation manifest. The properties "Description" and
     * "Version" are used during package management.
     * @remarks Adding a property with the same name as a previously added property will replace the previous property value.
     * @param[in] rssName Name of the property. Spaces are allowed. Quotes are not allowed.
     * @param[in] rssValue Property value.
     */
    void Property(const std::string& rssName, const std::string& rssValue);

    /**
     * @brief Get a property value.
     * @param[in] rssName Name of the property. Spaces are allowed. Quotes are not allowed.
     * @return Returns the property value if existing.
     */
    std::optional<std::string> Property(const std::string& rssName) const;

private:
    /**
     * @brief Checks the name for adherence to the bare key rule of TOML. If a bare key is not possible, a quoted key is required.
     * @remarks TOML defines for bare keys that they can only be composed of ASCII letters, ASCII digits, underscores and dashes.
     * @param[in] rssName Name to check for.
     * @return Returns whether quotes are required.
    */
    static bool NeedQuotedName(const std::string& rssName);

    /**
     * @brief Manifest information belonging to the module.
     */
    struct SModule
    {
        /**
         * @brief Constructor
         * @param[in] rpathRelModule Reference to the relative module path.
         * @param[in] rssManifest Reference to the manifest file.
         * @param[in] bBlockSystemObjects Set when to block system objects.
         */
        SModule(const std::filesystem::path& rpathRelModule, const std::string& rssManifest, bool bBlockSystemObjects);

        std::filesystem::path   pathRelModule;      ///< Relative module path (relative to the installation directory).
        std::string             ssManifest;         ///< Manifest containing the components.
        std::vector<SComponent> vecComponents;      ///< Vector with contained components
    };

    std::string                         m_ssInstallName;                ///< Installation name.
    mutable std::filesystem::path       m_pathInstallDir;               ///< Installation directory when install manifest was
                                                                        ///< loaded or saved.
    bool                                m_bBlockSystemObjects = false;  ///< When set, do not store system objects.
    std::vector<SModule>                m_vecModules;                   ///< Vector containing the modules.
    std::map<std::string, std::string>  m_mapProperties;                ///< Property map.
};

/**
 * @brief Interpret a version number as string.
 * @details A version string is composed of: major.minor.patch (numbers only; characters and whitespace are ignored).
 * @param rssVersion Reference to the version string.
 * @return The interpreted version.
 */
inline sdv::installation::SPackageVersion InterpretVersionString(const std::string& rssVersion)
{
    // 
    sdv::installation::SPackageVersion sVersion{};
    size_t nPos      = rssVersion.find('.');
    sVersion.uiMajor = static_cast<uint32_t>(std::atoi(rssVersion.substr(0, nPos).c_str()));
    size_t nStart    = nPos;
    if (nStart != std::string::npos)
    {
        nStart++;
        nPos             = rssVersion.find('.', nPos + 1);
        sVersion.uiMinor = static_cast<uint32_t>(std::atoi(rssVersion.substr(nStart, nPos).c_str()));
    }
    nStart = nPos;
    if (nStart != std::string::npos)
    {
        nStart++;
        sVersion.uiPatch = static_cast<uint32_t>(std::atoi(rssVersion.substr(nStart).c_str()));
    }
    return sVersion;
}

/**
 * @brief Equality operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator==(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor == sSecond.uiMajor && sFirst.uiMinor == sSecond.uiMinor && sFirst.uiPatch == sSecond.uiPatch;
}

/**
 * @brief Equal or larger than operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator>=(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor > sSecond.uiMajor
           || (sFirst.uiMajor == sSecond.uiMajor
               && (sFirst.uiMinor > sSecond.uiMinor ||
                   (sFirst.uiMinor == sSecond.uiMinor && sFirst.uiPatch >= sSecond.uiPatch)));
}

/**
 * @brief Larger than operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator>(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor > sSecond.uiMajor
           || (sFirst.uiMajor == sSecond.uiMajor
               && (sFirst.uiMinor > sSecond.uiMinor || (sFirst.uiMinor == sSecond.uiMinor && sFirst.uiPatch > sSecond.uiPatch)));
}

/**
 * @brief Inequality operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator!=(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor != sSecond.uiMajor || sFirst.uiMinor != sSecond.uiMinor || sFirst.uiPatch != sSecond.uiPatch;
}

/**
 * @brief Equality operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator<(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor < sSecond.uiMajor
           || (sFirst.uiMajor == sSecond.uiMajor
               && (sFirst.uiMinor < sSecond.uiMinor || (sFirst.uiMinor == sSecond.uiMinor && sFirst.uiPatch < sSecond.uiPatch)));
}

/**
 * @brief Equality operator for package version.
 * @param[in] sFirst First package version used for the comparison.
 * @param[in] sSecond Second package version used for the comparison.
 * @return The result of the comparison.
 */
constexpr inline bool operator<=(sdv::installation::SPackageVersion sFirst, sdv::installation::SPackageVersion sSecond)
{
    return sFirst.uiMajor < sSecond.uiMajor
           || (sFirst.uiMajor == sSecond.uiMajor
               && (sFirst.uiMinor < sSecond.uiMinor || (sFirst.uiMinor == sSecond.uiMinor && sFirst.uiPatch <= sSecond.uiPatch)));
}

#endif // !defined INSTALL_MANIFEST_H
