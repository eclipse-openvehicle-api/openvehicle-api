/**
 * @file module_control.h
 * @author Sudipta Babu Durjoy FRD DISDS1 (mailto:sudipta.durjoy@zf.com) & Erik Verhoeven FRD DISDS1 (mailto:erik.verhoeven@zf.com)
 * @brief This file contains the implementation for loading VAPI/SDV modules and accessing the exported functions related to
 * VAPI/SDV modules on Windows and Posix
 * @version 2.0
 * @date 2024-04-03
 *
 * @copyright Copyright ZF Friedrichshafen AG (c) 2021-2025
 *
 */

#ifndef MODULE_CONTROL_H
#define MODULE_CONTROL_H

#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <support/component_impl.h>
#include <mutex>
#include <list>
#include <set>
#include "toml_parser/parser_toml.h"
#include "module.h"

/**
 * @brief Module control class
 * @remarks This class is also used to load the object from the core library without reloading the library itself again. It uses
 * the module file name "core_services.sdv" and the ID -1 to identify the library.
 */
class CModuleControl : public sdv::IInterfaceAccess, public sdv::core::IModuleControl, public sdv::core::IModuleControlConfig,
    public sdv::core::IModuleInfo
{
public:
    /**
     * @brief Default constructor
     */
    CModuleControl();

    /**
     * @brief No copy constructor
     * @param[in] rctrl Reference to the module control class.
     */
    CModuleControl(const CModuleControl& rctrl) = delete;

    /**
     * @brief No move constructor
     * @param[in] rctrl Reference to the module control class.
     */
    CModuleControl(CModuleControl&& rctrl) = delete;

    /**
     * @brief Destructor unloads all modules
     */
    virtual ~CModuleControl();

    // Interface map
    // NOTE: Only the sdv::core::IModuleControlConfig interface should be accessible from here, since the module control is
    // accessible through sdv::GetCore() function, which should not make all other interfaces accessible (the system needs to start
    // and access should happen over sdv::core::GetObject()).
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::IModuleControlConfig)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief No assignment operator
     * @return Returns a reference to this class.
     */
    CModuleControl& operator=(const CModuleControl&) = delete;

    /**
     * @brief No move operator
     * @return Returns a reference to this class.
     */
    CModuleControl& operator=(CModuleControl&&) = delete;

    /**
     * @brief Sets a search path to a folder where a module can be found. Overload of
     * sdv::core::IModuleControlConfig::AddModuleSearchDir.
     * @param[in] ssDir Relative or absolute path to an existing folder.
     * To load a module out of the folder only the filename is required.
     * @return returns true if folder exists, otherwise false
     */
    virtual bool AddModuleSearchDir(const sdv::u8string& ssDir) override;

    /**
     * @brief Get a sequence containing the current module search directories.
     * @return The sequence with the module search directories.
     */
    virtual sdv::sequence<sdv::u8string> GetModuleSearchDirs() const override;

    /**
     * @brief Get a list of loaded modules. Overload of sdv::core::IModuleInfo::GetModuleList.
     * @return The module list.
     */
    virtual sdv::sequence<sdv::core::SModuleInfo> GetModuleList() const override;

    /**
    * @brief Get a list of classes exposed by the provided module.
    * @param[in] tModuleID The module ID to request the list of classes for.
    * @return Sequence with the class information structures.
    */
    virtual sdv::sequence<sdv::SClassInfo> GetClassList(/*in*/ sdv::core::TModuleID tModuleID) const override;

    /**
     * @brief Load the module. Overload of sdv::core::IModuleControl::Load.
     * @param[in] ssModulePath File name of the module to load.
     * @return Returns the ID of the module or 0 if the module could not be loaded
     */
    virtual sdv::core::TModuleID Load(const sdv::u8string& ssModulePath) override;

    /**
     * @brief Unload the module. Overload of sdv::core::IModuleControl::Unload.
     * @param[in] tModuleID Id representing a module, which has previous been loaded by Load
     * @return Returns true if module is unloaded successfully otherwise false.
     */
    virtual bool Unload(sdv::core::TModuleID tModuleID) override;

    /**
     * @brief Checking for active objects. Overload of sdv::core::IModuleControl::HasActiveObjects.
     * @param[in] tModuleID Id representing a module, which has previous been loaded by Load
     * @return Returns true if there is an active object otherwise false.
     */
    virtual bool HasActiveObjects(sdv::core::TModuleID tModuleID) const override;

    /**
     * @brief Find a module containing a class with the specified name.
     * @details Find the first module containing a class with the specified name. For main and isolated applications, if the
     * module wasn't loaded, the installation manifests are checked and if a module with the class has been found, the module will
     * be loaded. The order of checking the installation manifest is core-manifest, manifest in executable directory and manifest
     * in supplied installation directory.
     * @remarks Modules of system objects specified in the user installation are not returned.
     * @param[in] rssClass Reference to the class that should be searched for. The class is compared to the class name and the
     * default name in the manifest.
     * @return Returns a smart pointer to the module instance or NULL when the module hasn't been found.
     */
    std::shared_ptr<CModuleInst> FindModuleByClass(const std::string& rssClass);

    /**
     * @brief Get a class instance for the module.
     * @param[in] tModuleID Id representing a module, which has previous been loaded by Load
     * @return Returns a smart pointer to the module instance or NULL when the module hasn't been found.
     */
    std::shared_ptr<CModuleInst> GetModule(sdv::core::TModuleID tModuleID) const;

    /**
     * @brief Unload all modules (with force-flag).
     * @param[in] rvecIgnoreModules Reference to a vector containing all the modules to not unload.
     */
    void UnloadAll(const std::vector<sdv::core::TModuleID>& rvecIgnoreModules);

    /**
     * @brief Reset the current config baseline.
     */
    void ResetConfigBaseline();

    /**
     * @brief Save the configuration of all modules.
     * @param[in] rsetIgnoreModule Set of modules not needing to add.
     * @return The string containing all the modules.
     */
    std::string SaveConfig(const std::set<std::filesystem::path>& rsetIgnoreModule);

    /**
     * @brief Load the module with the supplied module manifest. Only for main and isolated applications.
     * @remarks  When running the main application all complex services, applications and utilities run in their isolated
     * environment. If multiple components are implemented in the module, of which one is suppose to run isolated, all of them run
     * isolated. Isolated applications cannot load other modules if one was loaded already. Maintenance applications cannot load
     * modules. All other applications load modules in their own process. Exception to the rul are proxy and stub components.
     * System objects cannot be loaded this way at all.
     * @param[in] rpathModule Absolute file path of the module to load.
     * @param[in] rssManifest The module manifest deciding on where the module should run.
     * @return Returns the ID of the module or 0 if the module could not be loaded
     */
    sdv::core::TModuleID ContextLoad(const std::filesystem::path& rpathModule, const std::string& rssManifest);

    /**
     * @brief Unload the module.
     * @param[in] tModuleID Id representing a module, which has previous been loaded by ContextLoad.
     * @param[in] bForce When set, the module instance class is cleared whether or not the module is unloaded or not. Use with
     * extreme care (e.g. during shutdown).
     * @return Returns true if module is unloaded successfully otherwise false.
     */
    bool ContextUnload(sdv::core::TModuleID tModuleID, bool bForce);

private:
    /**
     * @brief Find a module by its path.
     * @param[in] rpathModule File path of the module to load. Relative paths are relative to the installation directory for main
     * and isolated applications and relative to the exe for all other applications.
     * @return Returns a smart pointer to the module instance or NULL when the module hasn't been found (wasn't loaded).
     */
    std::shared_ptr<CModuleInst> FindModuleByPath(const std::filesystem::path& rpathModule) const;

    /**
     * @brief HasActiveObjects exported function type definition.
     */
    using TFNHasActiveObjects = bool();

    /**
     * @brief GetModuleFactory exported function type definition.
     */
    using TFNGetModuleFactory = sdv::IInterfaceAccess*(uint32_t);

    /**
     * @brief Add core_services and executable folders to the search paths if not done so already.
     */
    void AddCurrentPath();

    using TConfigSet = std::set<sdv::core::TModuleID>;
    mutable std::recursive_mutex            m_mtxModules;           ///< Control of the loaded library access
    std::list<std::shared_ptr<CModuleInst>> m_lstModules;           ///< List of modules. The order of loading is preserved.
    std::list<std::filesystem::path>        m_lstSearchPaths;       ///< List of search directories.
    TConfigSet                              m_setConfigModules;     ///< Set with the modules for storing in the configuration.
};

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

/**
 * @brief Module control service
 */
class CModuleControlService : public sdv::CSdvObject
{
public:
    CModuleControlService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetModuleControl())
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IModuleInfo, GetModuleControl())
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableModuleControlAccess(), 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IModuleControl, GetModuleControl())
        SDV_INTERFACE_DEFAULT_SECTION()
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("ModuleControlService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Get access to the module control
     * @return Returns the one global instance of the module control.
     */
    static CModuleControl& GetModuleControl();

    /**
     * @brief When set, the module control will be enabled.
     * @return Returns whether access to the module control is granted.
     */
    static bool EnableModuleControlAccess();
};

DEFINE_SDV_OBJECT_NO_EXPORT(CModuleControlService)
#endif


#ifdef _WIN32
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif


#endif // !define MODULE_CONTROL_H
