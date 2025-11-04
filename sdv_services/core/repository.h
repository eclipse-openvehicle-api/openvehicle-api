
#ifndef VAPI_REPOSITORY_H
#define VAPI_REPOSITORY_H

#include <list>
#include <map>
#include <mutex>
#include <string>
#include <set>
#include <shared_mutex>
#include <interfaces/repository.h>
#include <support/component_impl.h>
#include <condition_variable>
#include "module.h"
#include "object_lifetime_control.h"
#include "iso_monitor.h"

/**
 * @brief repository service providing functionality to load modules, create objects and access exiting objects
 */
class CRepository :
    public sdv::IInterfaceAccess, public sdv::core::IObjectAccess, public sdv::core::IRepositoryUtilityCreate,
    public sdv::core::IRepositoryMarshallCreate, public sdv::core::IRepositoryControl, public sdv::core::IRegisterForeignObject,
    public sdv::core::IRepositoryInfo, public IObjectDestroyHandler, public sdv::core::ILinkCoreRepository
{
public:
    /**
     * @brief Default constructor
     */
    CRepository() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::IObjectAccess)
        SDV_INTERFACE_ENTRY(sdv::core::IRepositoryMarshallCreate)
        SDV_INTERFACE_ENTRY(sdv::core::IRepositoryUtilityCreate)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Switch all components in config mode.
     */
    void SetConfigMode();

    /**
     * @brief Switch all components in running mode.
     */
    void SetRunningMode();

    /**
     * @brief Get the object instance previously created through the repository service. Overload of IObjectAccess::GetObject.
     * @param[in] ssObjectName The name of the requested object.
     * @return Returns the IInterfaceAccess interface of the object instance if found and nullptr otherwise.
     */
    virtual sdv::IInterfaceAccess* GetObject(/*in*/ const sdv::u8string& ssObjectName) override;

    /**
     * @brief Get the object instance previously created through the repository service.
     * @attention Utilities cannot be returned by this function.
     * @param[in] tObjectID The ID of the requested object.
     * @return Returns the IInterfaceAccess interface of the object instance if found and nullptr otherwise.
     */
    virtual sdv::IInterfaceAccess* GetObjectByID(/*in*/ sdv::core::TObjectID tObjectID) override;

    /**
     * @brief Creates an utility object from a previously loaded module. Overload of IRepositoryUtilityCreate::CreateUtility.
     * @attention Utilities are standalone objects. Use IObjectDestroy to destroy the utility.
     * @param[in] ssClassName The name of the object class to be created.
     * @param[in] ssObjectConfig Optional configuration handed over to the object upon creation via IObjectControl.
     * @return Returns the IInterfaceAccess interface of newly created utility.
     */
    virtual sdv::IInterfaceAccess* CreateUtility(/*in*/ const sdv::u8string& ssClassName,
        /*in*/ const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Create a proxy object for the interface with the supplied ID. If successful, this object is initialized, but
     * not linked to any other object within the system. Overload of sdv::core::IRepositoryMarshallCreate::CreateProxyObject.
     * @details Create a proxy object with the name "proxy_<ifc id>". "ifc_id" is the decimal value of an interface ID.
     * @param[in] id The interface ID to create the object for.
     * @return Returns the interface to the proxy object. Destruction of the object can be achieved through IObjectDestroy.
     */
    virtual sdv::IInterfaceAccess* CreateProxyObject(/*in*/ sdv::interface_id id) override;

    /**
     * @brief Create a stub object for the interface with the supplied ID. If successful, this object is initialized, but
     * not linked to any other object within the system. Overload of sdv::core::IRepositoryMarshallCreate::CreateStubObject.
     * @details Create a stub object with the name "stub_<ifc id>". "ifc_id" is the decimal value of an interface ID.
     * @param[in] id The interface ID to create the object for.
     * @return Returns the interface to the stub object. Destruction of the object can be achieved through IObjectDestroy.
     */
    virtual sdv::IInterfaceAccess* CreateStubObject(/*in*/ sdv::interface_id id) override;

protected:
    /**
     * @brief Create an object and all its objects it depends on. Overload of Overload of
     * sdv::core::IRepositoryControl::CreateObject.
     * @details For standalone and essential applications, this function allows the creation of system, device and service
     * objects if the module was loaded previously. For the main and isolated application, this function allows the
     * creation of complex services only and only those that are in the installation. For the isolated application only one
     * complex service can be created. External apps, utilities, and proxy and stub objects cannot be created at all using
     * this function.
     * Objects that the to be create object is depending on will be created as well. For the main application this is
     * limited to complex services. Isolated applications cannot load other services; this is taken over by the main
     * application.
     * @param[in] ssClassName The name of the object class to be created. For the main application, the class string could
     * be empty for the main application if the object was defined in the installation.
     * @param[in] ssObjectName Name of the object, required to be unique. For standalone and essential applications, the
     * name string can be empty, in which case the object might either provide a name proposal or the name is the same as
     * the class name. Use the returned object ID to request the name of the object.
     * @param[in] ssObjectConfig Optional configuration handed over to the object upon creation via IObjectControl. Only
     * valid for standalone, essential and isolated applications.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    virtual sdv::core::TObjectID CreateObject(/*in*/ const sdv::u8string& ssClassName, /*in*/ const sdv::u8string& ssObjectName,
        /*in*/ const sdv::u8string& ssObjectConfig) override;
public:
    /**
     * @brief Creates an object from a previously loaded module. Provide the module ID to explicitly define what module to
     * use during object creation. Overload of sdv::core::IRepositoryControl::CreateObjectFromModule.
     * @param[in] tModuleID Module ID that contains the object class to create the object with.
     * @param[in] ssClassName The name of the object class to be created.
     * @param[in] ssObjectName Optional name of the object - required to be unique. If not supplied, the object might
     * either provide a name proposal or the name is the same as the class name. Use the returned object ID to request
     * the name of the object.
     * @param[in] ssObjectConfig Optional configuration handed over to the object upon creation via IObjectControl.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    virtual sdv::core::TObjectID CreateObjectFromModule(/*in*/ sdv::core::TModuleID tModuleID,
        /*in*/ const sdv::u8string& ssClassName, /*in*/ const sdv::u8string& ssObjectName,
        /*in*/ const sdv::u8string& ssObjectConfig) override;

protected:
    /**
     * @brief Destroy a previously created object with the supplied name. Overload of sdv::core::IRepositoryControl::DestroyObject.
     * @details For standalone and essential applications previously created system, device and service objects can be
     * destroyed. For the main and isolated applications, only the complex service can be destroyed. For isolated
     * applications a destruction of the object will end the application.
     * @param[in] ssObjectName The name of the object to destroy.
     * @return Returns whether the object destruction was successful.
     */
    virtual bool DestroyObject(/*in*/ const sdv::u8string& ssObjectName) override;

public:
    /**
     * @brief Create an object and all its objects it depends on. Internal function not accessible through the interface.
     * @param[in] ssClassName The name of the object class to be created. For the main application, the class string could
     * be empty for the main application if the object was defined in the installation.
     * @param[in] ssObjectName Name of the object, required to be unique. For standalone and essential applications, the
     * name string can be empty, in which case the object might either provide a name proposal or the name is the same as
     * the class name. Use the returned object ID to request the name of the object.
     * @param[in] ssObjectConfig Optional configuration handed over to the object upon creation via IObjectControl. Only
     * valid for standalone, essential and isolated applications.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    sdv::core::TObjectID CreateObject2(/*in*/ const sdv::u8string& ssClassName, /*in*/ const sdv::u8string& ssObjectName,
        /*in*/ const sdv::u8string& ssObjectConfig);

    /**
     * @brief Destroy a previously created object with the supplied name. Internal function not accessible through the interface.
     * @details For standalone and essential applications previously created system, device and service objects can be
     * destroyed. For the main and isolated applications, only the complex service can be destroyed. For isolated
     * applications a destruction of the object will end the application.
     * @param[in] ssObjectName The name of the object to destroy.
     * @return Returns whether the object destruction was successful.
     */
    bool DestroyObject2(/*in*/ const sdv::u8string& ssObjectName);

    /**
     * @brief Register as foreign object and make it public to the system with the given name. Overload of
     * sdv::core::IRegisterForeignObject::RegisterObject.
     * @param[in] pObjectIfc Interface of the object to be registered.
     * @param[in] ssObjectName The name under which the object - required to be unique.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    virtual sdv::core::TObjectID RegisterObject(/*in*/ sdv::IInterfaceAccess* pObjectIfc,
        /*in*/ const sdv::u8string& ssObjectName) override;

    /**
     * @brief Register the core repository.
     * @param[in] pCoreRepository Pointer to the proxy interface of the core repository.
     */
    virtual void LinkCoreRepository(/*in*/ sdv::IInterfaceAccess* pCoreRepository) override;

    /**
     * @brief Unlink a previously linked core repository.
     */
    virtual void UnlinkCoreRepository() override;

    /**
     * @brief Find the class information of an object with the supplied name. Overload of
     * sdv::core::IRepositoryControl::IRepositoryInfo::FindClass.
     * @param[in] ssClassName Object class name.
     * @return The object class information.
     */
    virtual sdv::SClassInfo FindClass(/*in*/ const sdv::u8string& ssClassName) const override;

    /**
     * @brief Get a list of all the instantiated objects. Overload of sdv::core::IRepositoryControl::IRepositoryInfo::GetObjectList.
     * @return Sequence containing the object information structures.
     */
    virtual sdv::sequence<sdv::core::SObjectInfo> GetObjectList() const override;

    /**
     * @brief Get the object info for the requested object. Overload of sdv::core::IRepositoryInfo::GetObjectInfo.
     * @param[in] tObjectID The object ID to return the object information for.
     * @return The object information structure if the object is available or an empty structure if not.
     */
    virtual sdv::core::SObjectInfo GetObjectInfo(/*in*/ sdv::core::TObjectID tObjectID) const override;

    /**
     * @brief Find an object with the supplied name. Only object instances that are in the service list can be found with this
     * function (devices, basic and complex services, and system objects). Overload of sdv::core::IRepositoryInfo::FindObject.
     * @param[in] ssObjectName Object name to search for.
     * @return The object information structure if the object is available or an empty structure if not.
     */
    virtual sdv::core::SObjectInfo FindObject(/*in*/ const sdv::u8string& ssObjectName) const override;

    /**
     * @brief Remove an object instance from the local object map. Overload of IObjectDestroyHandler::OnDestroyObject.
     * @param[in] pObject Interface pointer to the object instance.
     */
    virtual void OnDestroyObject(sdv::IInterfaceAccess* pObject) override;

    /**
     * @brief Destroy all objects from the service map created for a specific module.
     * @param[in] tModuleID Module ID that contains the object class to destroy.
     */
    void DestroyModuleObjects(sdv::core::TModuleID tModuleID);

    /**
     * @brief Destroy all objects from the service map.
     * @remarks In Emergency, the objects should be removed without unloading to prevent any more calls to take place. The reason
     * is, that the system could do an emergency shutdown and part of the system might have been cleaned up already and part not.
     * @param[in] rvecIgnoreObjects Reference to the vector of objects to not destroy.
     * @param[in] bForce Force destruction (remove without calling any more function).
     */
    void DestroyAllObjects(const std::vector<std::string>&rvecIgnoreObjects, bool bForce = false);

    /**
     * @brief Reset the current config baseline.
     */
    void ResetConfigBaseline();

    /**
     * @brief Save the configuration of all components.
     * @return The string containing all the components.
     */
    std::string SaveConfig();

private:
    /**
     * @brief Create an isolated object (object running in a separate process). Only allowed to be called by the main application.
     * @param[in] rsClassInfo Reference to the class information structure of the object.
     * @param[in] rssObjectName Reference to the name of the object - required to be unique.
     * @param[in] rssObjectConfig Reference to an optional configuration handed over to the object upon creation via IObjectControl.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    sdv::core::TObjectID CreateIsolatedObject(const sdv::SClassInfo& rsClassInfo, const sdv::u8string& rssObjectName,
        const sdv::u8string& rssObjectConfig);

    /**
     * @brief Creates an object from the supplied module instance.
     * @param[in] rptrModule Reference to the module that contains the object class to create the object with.
     * @param[in] rsClassInfo Reference to the class information structure of the object.
     * @param[in] rssObjectName Reference to the name of the object - required to be unique.
     * @param[in] rssObjectConfig Reference to an optional configuration handed over to the object upon creation via IObjectControl.
     * @return Returns the object ID when the object creation was successful or 0 when not. On success the object is
     * available through the IObjectAccess interface. If the object already exists (class and object names are identical),
     * the object ID of the existing object is returned.
     */
    sdv::core::TObjectID InternalCreateObject(const std::shared_ptr<CModuleInst>& rptrModule, const sdv::SClassInfo& rsClassInfo,
        const sdv::u8string& rssObjectName, const sdv::u8string& rssObjectConfig);

    /**
     * @brief Create a new unique object ID.
     * @return The created object ID.
     */
    static sdv::core::TObjectID CreateObjectID();

    /**
     * @brief Get a list of depending object instances of a specific class.
     * @param[in] rssClass Reference to the class name.
     * @return Returns a list of object instances.
     */
    std::vector<sdv::core::TObjectID> GetDependingObjectInstancesByClass(const std::string& rssClass);

    /**
     * @brief Object entry
     * @details The object instance information. Objects that are running remotely are represented by their proxy. Objects can have
     * multiple references by stubs.
     */
    struct SObjectEntry
    {
        sdv::core::TObjectID                tObjectID = 0;          ///< Object ID (local to this process).
        sdv::SClassInfo                     sClassInfo;             ///< Object class name.
        std::string                         ssName;                 ///< Object name (can be zero with local objects).
        std::string                         ssConfig;               ///< Object configuration.
        sdv::TInterfaceAccessPtr            ptrObject;              ///< Object interface (could be proxy).
        std::shared_ptr<CModuleInst>        ptrModule;              ///< Module instance.
        bool                                bControlled = false;    ///< When set, the object is controlled.
        bool                                bIsolated = false;      ///< When set, the object is isolated and running in another process.
        std::mutex                          mtxConnect;             ///< Mutex used to wait for connection
        std::condition_variable             cvConnect;              ///< Condition variable used to wait for connection
        std::shared_ptr<CIsoMonitor>        ptrIsoMon;              ///< Object being monitored for shutdown.
    };

    using TObjectMap = std::map<sdv::core::TObjectID, std::shared_ptr<SObjectEntry>>;
    using TOrderedObjectList = std::list<std::shared_ptr<SObjectEntry>>;
    using TObjectIDList = std::list<std::shared_ptr<SObjectEntry>>;
    using TServiceMap = std::map<std::string, TOrderedObjectList::iterator>;
    using TIsolationMap = std::map<std::string, std::shared_ptr<SObjectEntry>>;
    using TLocalObjectMap = std::map<sdv::IInterfaceAccess*, std::shared_ptr<SObjectEntry>>;
    using TConfigSet = std::set<sdv::core::TObjectID>;
    mutable std::shared_mutex       m_mtxObjects;                   ///< Protects against concurrent access.
    TOrderedObjectList              m_lstOrderedServiceObjects;     ///< List of service object IDs in order of creation.
    TServiceMap                     m_mapServiceObjects;            ///< Map of service objects indexed by object name an pointing
                                                                    ///< to the entry in the list.
    TLocalObjectMap                 m_mapLocalObjects;              ///< Map with local objects indexed by object pointer.
    TIsolationMap                   m_mapIsolatedObjects;           ///< Map with isolated objects.
    TObjectMap                      m_mapObjects;                   ///< Map with all objects indexed by the object ID.
    TConfigSet                      m_setConfigObjects;             ///< Set with the objects for storing in the configuration.
    sdv::TInterfaceAccessPtr        m_ptrCoreRepoAccess;            ///< Linked core repository access (proxy interface).
    bool                            m_bIsoObjectLoaded = false;     ///< When set, the isolated object has loaded. Do not allow
                                                                    ///< another object of type complex service or utility to be
                                                                    ///< created.
};

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

/**
* @brief Repository service
*/
class CRepositoryService : public sdv::CSdvObject
{
public:
    CRepositoryService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetRepository())
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IRepositoryInfo, GetRepository())
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableRepositoryObjectControl(), 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IRepositoryControl, GetRepository())
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableRepositoryRegisterForeignApp(), 2)
        SDV_INTERFACE_SECTION(2)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::IRegisterForeignObject, GetRepository())
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableRepositoryLink(), 3)
        SDV_INTERFACE_SECTION(3)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::core::ILinkCoreRepository, GetRepository())
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("RepositoryService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Get access to the repository.
     * @return Returns a reference to the one repository of this module.
     */
    static CRepository& GetRepository();

    /**
     * @brief When set, the repository object control access will be enabled.
     * @return Returns whether object control interface access is granted.
     */
    static bool EnableRepositoryObjectControl();

    /**
    * @brief When set, the foreign app registration into the repository will be enabled.
    * @return Returns whether foreign app rgeistration interface access is granted.
    */
    static bool EnableRepositoryRegisterForeignApp();

    /**
     * @brief When set, the core repository link access will be enabled.
     * @return Returns whether core repository link interface access is granted.
     */
    static bool EnableRepositoryLink();
};

DEFINE_SDV_OBJECT_NO_EXPORT(CRepositoryService)
#endif

#endif // !define VAPI_REPOSITORY_H
