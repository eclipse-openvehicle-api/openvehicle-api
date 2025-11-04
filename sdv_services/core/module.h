#ifndef MODULE_H
#define MODULE_H

#include <mutex>
#include <string>
#include <map>
#include <functional>
#include <optional>
#include <interfaces/core.h>
#include <interfaces/module.h>

/**
 * @brief Module instance class
 */
class CModuleInst
{
public:
    /**
     * @brief Constructor
     * @param[in] rpathModuleConfigPath Reference to the path of the module as it was specified in the configuration.
     * @param[in] rpathModule Reference to the absolute path of the module to load. If empty, the module is not instanceable, but
     * is used as a placeholder for rewriting the configuration.
     */
    CModuleInst(const std::filesystem::path& rpathModuleConfigPath, const std::filesystem::path& rpathModule) noexcept;

    /**
     * @brief Copy constructor is not allowed.
     */
    CModuleInst(const CModuleInst&) = delete;

    /**
     * @brief Move constructor is not allowed.
     */
    CModuleInst(CModuleInst&&) = delete;

    /**
     * @brief Destructor
     */
    ~CModuleInst();

    /**
     * @brief Assignment operator is not allowed.
     * @return Returns a reference to this class.
     */
    CModuleInst& operator=(const CModuleInst&) = delete;

    /**
     * @brief Move operator is not allowed.
     * @return Returns a reference to this class.
     */
    CModuleInst& operator=(CModuleInst&&) = delete;

    /**
     * @brief Return whether the module instance was loaded correctly.
     * @return Returns 'true' when the module was lloaded successfully. Otherwise returns 'false'.
     */
    bool IsValid() const noexcept;

    /**
     * @brief Get the default object name.
     * @param[in] ssClassName Name of the class
     * @return Returns the default object name or an empty string if the class doesn't exist.
     */
    std::string GetDefaultObjectName(const std::string& ssClassName) const;

    /**
     * @brief Is the object class marked as singleton (only one instance is allowed)?
     * @param[in] ssClassName Name of the class
     * @return Returns whether the object class is a singleton.
     */
    bool IsSingleton(const std::string& ssClassName) const;

    /**
     * @brief Gets the set of all available classes in the module
     * @returns Returns the vector of all available classes in the module
     */
    std::vector<std::string> GetAvailableClasses() const;

    /**
     * @brief Creates an object instance of a given SDV class via the loaded module
     * @param[in] ssClassName the class from which an object is to be created
     * @return Returns a pointer to the created object or nullptr in case of failure
     */
    sdv::IInterfaceAccess* CreateObject(const std::string& ssClassName);

    /**
     * @brief Destroys an object previously created via CreateObject
     * @param[in] object The object to be destroyed.
     * The object is only destroyed if it has been allocated via the loaded module and has not yet been destroyed.
     */
    void DestroyObject(sdv::IInterfaceAccess* object);

    /**
     * @brief Return the module config path (as it was specified in the configuration).
     * @return The config path of the module.
     */
    std::filesystem::path GetModuleConfigPath() const;

    /**
     * @brief Return the module path.
     * @return The path of the module.
     */
    std::filesystem::path GetModulePath() const;

    /**
     * @brief Return the module ID.
     * @return The path of the module.
     */
    sdv::core::TModuleID GetModuleID() const;

    /**
     * @brief Get the module information.
     * @return Returns the module information structure.
     */
    sdv::core::SModuleInfo GetModuleInfo() const;

    /**
     * @brief Return the class information of the supplied class name.
     * @param[in] rssClassName Reference to string containing the class name.
     * @return Returns the information structure of the requested class or an empty structure when the class could not be found.
    */
    std::optional<sdv::SClassInfo> GetClassInfo(const std::string& rssClassName) const;

    /**
     * @brief Load the module.
     * @param[in] rpathModule Reference to the path of the module to load.
     * @return Returns 'true' when successful; 'false' when not.
     */
    bool Load(const std::filesystem::path& rpathModule) noexcept;

    /**
     * @brief Unload the module and clear the module instance class.
     * @attention If there are any active objects still running, this module is not unloaded. In cooperation with the bForce
     * flag this causes the module to stay in memory indefinitely.
     * @param[in] bForce When set, the module instance class is cleared whether or not the module is unloaded or not. Use with
     * extreme care (e.g. during shutdown).
     * @return Returns 'true' when successfully cleared the module instance class, 'false' when there are still active objects
     * blocking the unloading.
     */
    bool Unload(bool bForce) noexcept;

    /**
     * @brief Return whether active objects created through this module are still running.
     * @return Returns 'true' when active objects are running; 'false' when not.
    */
    bool HasActiveObjects() const;

private:
    /// HasActiveObjects exported function type definition.
    using TFNHasActiveObjects = bool();

    /// GetModuleFactory exported function type definition.
    using TFNGetModuleFactory = sdv::IInterfaceAccess*(uint32_t);

    /// GetManifest exported function type definition.
    using TFNGetManifest = const char*();

    mutable std::recursive_mutex            m_mtxModule;                ///< Access regulation for the class members.
    std::filesystem::path                   m_pathModuleConfigPath;     ///< Module path as it was specified in the configuration.
    std::filesystem::path                   m_pathModule;               ///< Module path or filename.
    sdv::core::TModuleID                    m_tModuleID = 0;            ///< Module handle.
    std::function<TFNHasActiveObjects>      m_fnActiveObjects;          ///< Module exported function for requesting active objects.
    std::function<TFNGetModuleFactory>      m_fnGetFactory;             ///< Module exported function for getting factory.
    std::function<TFNGetManifest>           m_fnGetManifest;            ///< Module exported function for getting manifest.
    sdv::IObjectFactory*                    m_pFactory = nullptr;       ///< Factory interface of the module.
    std::map<std::string, sdv::SClassInfo>  m_mapClassInfo;             ///< Available classes
    uint32_t                                m_uiIfcVersion = 0;         ///< Interface version.
};

#endif // !defined MODULE_H