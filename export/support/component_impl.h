/**
 *
 * @file      component_impl.h
 * @brief     This file provides all necessary definitions to implement SDV object.
 * @version   0.1
 * @date      2022.11.14
 * @author    Thomas.pfleiderer@zf.com
 * @copyright Copyright ZF Friedrichshaven AG (c) 2022
 *
 */
#ifndef COMPONENT_IMPL_H
#define COMPONENT_IMPL_H

#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "../interfaces/core_types.h"
#include "../interfaces/core.h"
#include "../interfaces/repository.h"
#include "interface_ptr.h"
#include "local_service_access.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * @brief Stringize helper.
 */
#define SDV_STRINGIZE_HELPER2(txt) #txt
/**
 * @brief Stringize helper.
 */
#define SDV_STRINGIZE_HELPER(txt) SDV_STRINGIZE_HELPER2(txt)

 // NOLINTEND(cppcoreguidelines-macro-usage)

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

#elif defined __unix__
#else
#error OS is not supported!
#endif // defined _MSC_VER


/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    // Forward declarations
    class CSdvObject;


    ////////////////////////////////////
    // Object control classes         //
    ////////////////////////////////////

    /**
     * @brief SDV object create interface.
     * @remarks This is an internal interface not exposed through IInterfaceAccess.
     */
    struct ISdvObjectClassInfo
    {
    public:
        /**
         * @brief Get the class information struct.
         * @return Returns the class information struct.
         */
        virtual sdv::SClassInfo GetClassInfo() const = 0;

        /**
         * @brief Create the SDV object.
         * @return Returns an instance of the SDV object or nullptr when the object could not be created.
         */
        virtual std::unique_ptr<CSdvObject> CreateObject() = 0;
    };

    /**
     * @brief Top level SDV object to solve ambiguities in the use of IInterfaceAccess. THis class is used by
     * CSdvObjectClass during the instantiation of the SDV object.
     */
    class CSdvObjectAccess : public IInterfaceAccess
    {
    public:
        /**
         * @brief Default constructor
         */
        CSdvObjectAccess() = default;

        CSdvObjectAccess(CSdvObjectAccess&)				= delete;
        CSdvObjectAccess& operator=(CSdvObjectAccess&)	= delete;
        CSdvObjectAccess(CSdvObjectAccess&&)			= delete;
        CSdvObjectAccess& operator=(CSdvObjectAccess&&) = delete;

        /**
         * @brief Default virtual destructor
         */
        virtual ~CSdvObjectAccess() = default;

        /**
         * @brief Get access to the implemented IInterfaceAccess interface.
         * @return Returns a pointer to the implementation of IInterfaceAccess.
         */
        IInterfaceAccess* GetObjectAccess()
        {
            return static_cast<IInterfaceAccess*>(this);
        }

        // Interface map
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    };

    /**
     * @brief Lifetime cookie to automatically manage the module lifetime.
     * @details The lifetime cookie uses an increment and decrement function to increment for every assigned instantiation of
     * the cookie and to decrement every previously instantiated cookie. This allows the classfactory to keep track of its lifetime
     * even with dynamically created objects.
     */
    class CLifetimeCookie
    {
    public:
        /**
         * @brief Default constructor.
        */
        CLifetimeCookie() = default;

        /**
         * @brief Assignment constructor, assigning the increment and decrement functions.
         * @details Assign the increment and decrement functions and increase the incrementation.
         * @param[in] fnLifetimeIncr Increment function object.
         * @param[in] fnLifetimeDecr Decrement function object.
         */
        CLifetimeCookie(std::function<void()> fnLifetimeIncr, std::function<void()> fnLifetimeDecr) :
            m_fnLifetimeIncr(fnLifetimeIncr), m_fnLifetimeDecr(fnLifetimeDecr)
        {
            if (m_fnLifetimeIncr) m_fnLifetimeIncr();
        }

        /**
         * @brief Copy constructor.
         * @details Construct a lifetime cookie object and copy the increment and decrement functions and call the increment
         * function if valid.
         * @param[in] rCookie Reference to the cookie to copy from.
         */
        CLifetimeCookie(const CLifetimeCookie& rCookie) :
            m_fnLifetimeIncr(rCookie.m_fnLifetimeIncr), m_fnLifetimeDecr(rCookie.m_fnLifetimeDecr)
        {
            if (m_fnLifetimeIncr) m_fnLifetimeIncr();
        }

        /**
         * @brief Move constructor.
         * @details Construct a lifetime cookie object and move the functions from the provided cookie. Then reset the functions
         * of the provided cookie.
         * @param[in] rCookie Reference to the cookie to copy from.
         */
        CLifetimeCookie(CLifetimeCookie&& rCookie) noexcept :
            m_fnLifetimeIncr(rCookie.m_fnLifetimeIncr), m_fnLifetimeDecr(rCookie.m_fnLifetimeDecr)
        {
            rCookie.m_fnLifetimeIncr = nullptr;
            rCookie.m_fnLifetimeDecr = nullptr;
        }

        /**
         * @brief Destroy the lifetime cookie.
         * @details Decrement the lifetime by calling the decrement function if valid and destroy the cookie object.
         */
        ~CLifetimeCookie()
        {
            if (m_fnLifetimeDecr) m_fnLifetimeDecr();
        }

        /**
         * @brief Assignment operator.
         * @details Release the current lifetime by calling the decrement function and assign the increment and decrement functions
         * of the provided cookie and call the increment function if valid.
         * @param[in] rCookie Reference to the cookie to copy from.
         * @return Reference to this class.
         */
        CLifetimeCookie& operator=(const CLifetimeCookie& rCookie)
        {
            if (m_fnLifetimeDecr) m_fnLifetimeDecr();
            m_fnLifetimeIncr = rCookie.m_fnLifetimeIncr;
            m_fnLifetimeDecr = rCookie.m_fnLifetimeDecr;
            if (m_fnLifetimeIncr) m_fnLifetimeIncr();
            return *this;
        }

        /**
         * @brief Move operator.
         * @details Release the current lifetime by calling the decrement function and move the functions from the provided cookie.
         * Then reset the functions of the provided cookie.
         * @param[in] rCookie Reference to the cookie to copy from.
         * @return Reference to this class.
         */
        CLifetimeCookie& operator=(CLifetimeCookie&& rCookie) noexcept
        {
            if (m_fnLifetimeDecr) m_fnLifetimeDecr();
            m_fnLifetimeIncr = rCookie.m_fnLifetimeIncr;
            m_fnLifetimeDecr = rCookie.m_fnLifetimeDecr;
            rCookie.m_fnLifetimeIncr = nullptr;
            rCookie.m_fnLifetimeDecr = nullptr;
            return *this;
        }

    private:
        std::function<void()>   m_fnLifetimeIncr;   ///< Lifetime increment function
        std::function<void()>   m_fnLifetimeDecr;   ///< Lifetime decrement function
    };

    /**
     * @brief Global tracking of active objects. Functions in this class will only be called by CSdvObject and CSdvObjectClass
     */
    class CObjectFactory : public IInterfaceAccess, public IObjectFactory
    {
        // Friend class SDV object and definition
        friend class CSdvObject; ///< SDV object base class.
        template <class TSdvObject>
        friend class CSdvObjectClass; ///< SDV object creator class.

    public:
        /**
         * @brief Constructor
         */
        CObjectFactory() = default;

        /**
         * @brief Interface map.
         */
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_ENTRY(IObjectFactory)
        END_SDV_INTERFACE_MAP()

        /**
         * @brief Create a lifetime cookie that keeps track of the module lifetime as long as it is alive.
         * @return Returns a lifetime cookie.
         */
        CLifetimeCookie CreateLifetimeCookie()
        {
            return CLifetimeCookie([this]() { m_uiActiveObjectCount++; },
                [this]() { if (m_uiActiveObjectCount) m_uiActiveObjectCount--; });
        }

        /**
         * @brief Gets the amount of active SDV objects.
         * @return Returns the number of active SDV objects.
         */
        uint32_t GetActiveObjects() const
        {
            return m_uiActiveObjectCount;
        }

        /**
         * @brief Get the module manifest.
         * @return Returns the pointer to a zero terminated string containing the module manifest or NULL when there is no string.
         */
        const char* GetManifest()
        {
            if (m_ssManifest.empty())
                BuildManifest();
            return m_ssManifest.c_str();
        }

        /**
         * @brief Create or get the object using the name from the object class info. Overload of
         * sdv::IObjectFactory::CreateObject.
         * @attention The objects lifetime is ended by a call to the DestroyObject function or the unloading of he module.
         * @param[in] ssClassName The name of the class object to instantiate.
         * @return Pointer to IInterfaceAccess interface of the object or NULL when the requested object doesn't exist.
         */
        virtual IInterfaceAccess* CreateObject(const sdv::u8string& ssClassName) override;

        /**
         * @brief Destroy an instantiated object using the name of the object class info. Overload of
         * sdv::IObjectFactory::DestroyObject.
         * @param[in] object The object to destroy.
         */
        virtual void DestroyObject(IInterfaceAccess* object) override;

        /**
         * @brief Destroys all active objects in reverse order of creation
         */
        virtual void DestroyAllObjects() override;

    protected:
        /**
         * @brief Build the module manifest.
         * @return Returns the pointer to a zero terminated string containing the module manifest or NULL when there is no string.
         */
        void BuildManifest()
        {
            std::stringstream sstream;
            sstream << R"code(# Module manifest

[Interface]
Version = )code" << SDVFrameworkInterfaceVersion
                    << R"code(

)code";
            std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
            for (const ISdvObjectClassInfo* pClassInfo : m_vecObjectClasses)
            {
                sdv::SClassInfo sInfo = pClassInfo->GetClassInfo();
                sstream << "[[Component]]" << std::endl;
                sstream << "Class=\"" << sInfo.ssClassName << "\"" << std::endl;
                if (!sInfo.seqClassAliases.empty())
                {
                    sstream << "Aliases=[";
                    bool bInitialAlias = true;
                    for (const sdv::u8string& rssAlias : sInfo.seqClassAliases)
                    {
                        if (!bInitialAlias)
                            sstream << ", ";
                        bInitialAlias = false;
                        sstream << "\"" << rssAlias << "\"";
                    }
                    sstream << "]" << std::endl;
                }
                if (!sInfo.ssDefaultObjectName.empty())
                    sstream << "DefaultName=\"" << sInfo.ssDefaultObjectName << "\"" << std::endl;
                bool bSkip = false;
                switch (sInfo.eType)
                {
                case sdv::EObjectType::SystemObject:
                    sstream << "Type=\""
                            << "System"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::Device:
                    sstream << "Type=\""
                            << "Device"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::BasicService:
                    sstream << "Type=\""
                            << "BasicService"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::ComplexService:
                    sstream << "Type=\""
                            << "ComplexService"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::Application:
                    sstream << "Type=\""
                            << "App"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::Proxy:
                    sstream << "Type=\""
                            << "Proxy"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::Stub:
                    sstream << "Type=\""
                            << "Stub"
                            << "\"" << std::endl;
                    break;
                case sdv::EObjectType::Utility:
                    sstream << "Type=\""
                            << "Utility"
                            << "\"" << std::endl;
                    break;
                default:
                    bSkip = true;
                    break;
                }
                if (bSkip)
                    continue;
                if (sInfo.uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
                    sstream << "Singleton=true" << std::endl;
                if (!sInfo.seqDependencies.empty())
                {
                    sstream << "Dependencies=[";
                    bool bInitialDependency = true;
                    for (const sdv::u8string& rssDependsOn : sInfo.seqDependencies)
                    {
                        if (!bInitialDependency)
                            sstream << ", ";
                        bInitialDependency = false;
                        sstream << "\"" << rssDependsOn << "\"";
                    }
                    sstream << "]" << std::endl;
                }
            }
            m_ssManifest = sstream.str();
        }

        /**
         * @brief Expose the object prototype by placing it into the object prototype list.
         * @details Expose the object class to allow class access. This function is called by the constructor of the
         * class object.
         * @param[in] pObjectClassInfo The object class internal interface.
         */
        void ExposeObjectClass(ISdvObjectClassInfo* pObjectClassInfo)
        {
            if (pObjectClassInfo)
            {
                std::unique_lock<std::shared_mutex> lock(m_mtxObjectClasses);
                m_vecObjectClasses.push_back(pObjectClassInfo);

                // Attention: pObjectClass is a pointer to ISdvObjectClassInfo even if the class was derived.
                // Virtual functions are not available yet at this stage.
            }
        }

        /**
         * @brief Revoke the object prototype from the object prototype list.
         * @param[in] pObjectClassInfo The object class internal interface.
         */
        void RevokeObjectClass(const ISdvObjectClassInfo* pObjectClassInfo)
        {
            std::unique_lock<std::shared_mutex> lock(m_mtxObjectClasses);
            auto itObjectClass = std::find(m_vecObjectClasses.begin(), m_vecObjectClasses.end(), pObjectClassInfo);
            if (itObjectClass != m_vecObjectClasses.end())
                m_vecObjectClasses.erase(itObjectClass);
            // TODO EVE: Updated through cppcheck warning
            //for (auto objectClassIter = m_vecObjectClasses.begin(); objectClassIter != m_vecObjectClasses.end();
            //     objectClassIter++)
            //{
            //    if (*objectClassIter == pObjectClassInfo)
            //    {
            //        m_vecObjectClasses.erase(objectClassIter);
            //        break;
            //    }
            //}
        }

    private:
        std::atomic<uint32_t>                       m_uiActiveObjectCount{0};	///< The amount of active SDV objects.

        mutable std::mutex                          m_mtxActiveObjects;         ///< Synchronize access to m_vecActiveObjects
        std::vector<std::unique_ptr<CSdvObject>>    m_vecActiveObjects;         ///< List of objects actively kept alive

        mutable std::shared_mutex                   m_mtxObjectClasses;         ///< Synchronize access to the object classes.
        std::vector<ISdvObjectClassInfo*>           m_vecObjectClasses;         ///< List of object classes contained in this module.

        std::string                                 m_ssManifest;               ///< Manifest of this module.
    };

    ////////////////////////////////////
    // Framework module classes       //
    ////////////////////////////////////

    /**
     * @brief Class that manages the object instantiation and module lifetime.
     * @remarks A pointer to an instance of this class is exposed across dll boundaries and used to implement
     * CModuleInstance for SDV DLLs
     * The instance is created by the DEFINE_SDV_OBJECT macro and the macro also registers a helper capable of
     * instantiating the SDV object with the instance (see CObjectFactory for details)
     */
    class CModule
        : public CSdvObjectAccess
        , public CObjectFactory
    {
    public:
        /**
         * @brief Default constructor
         */
        CModule() = default;

        /**
         * @brief Interface map.
         */
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_CHAIN_BASE(CObjectFactory)
        END_SDV_INTERFACE_MAP()

        /**
         * @brief Check whether the requested interface version corresponds to the one implemented here and return this
         * object.
         * @param[in] interfaceVersion The version of module interface that is requested for.
         * @return Returns the IInterfaceAccess interface of the module if the interface versions match, nullptr otherwise
         */
        IInterfaceAccess* GetModuleControl(uint32_t interfaceVersion)
        {
            // No compatibility interfacing available yet.
            return interfaceVersion == SDVFrameworkInterfaceVersion ? GetObjectAccess() : nullptr;
        }
    };

    /**
     * @brief Module access (global per module).
     * @return Reference to the global module.
     */
    inline CModule& GetModule()
    {
        static CModule module;
        return module;
    }

    /**
     * @brief Create a lifetime cookie that keeps track of the module lifetime as long as it is alive.
     * @return Returns a lifetime cookie.
     */
    inline CLifetimeCookie CreateLifetimeCookie()
    {
        return GetModule().CreateLifetimeCookie();
    }

    ////////////////////////////
    // Component object class //
    ////////////////////////////

    /**
     * @brief SDV object creator class. Used to define the SDV class object and register its definition into the system
     * as welll a as providing the creation interface for creating an object instance.
     * @tparam TSdvObject Class type of the SDV object derived from CSdvObject.
     */
    template <class TSdvObject>
    class CSdvObjectClass
        : public ISdvObjectClassInfo
    {
    public:
        /**
         * @brief Constructor assigning the SDV prototype to the module.
         */
        CSdvObjectClass()
        {
            // Add this object definition to the definition list.
            GetModule().ExposeObjectClass(this);
        }

        CSdvObjectClass(CSdvObjectClass&)			  = delete;
        CSdvObjectClass(CSdvObjectClass&&)			  = delete;
        CSdvObjectClass& operator=(CSdvObjectClass&)  = delete;
        CSdvObjectClass& operator=(CSdvObjectClass&&) = delete;

        /**
         * @brief Destructor revoking the object prototype.
         */
        virtual ~CSdvObjectClass()
        {
            GetModule().RevokeObjectClass(this);
        }

        /**
         * @brief Gets the object class name.
         * @return Returns the class name of the object.
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual sdv::u8string GetClassName() const
        {
            return TSdvObject::GetClassNameStatic();
        }

        /**
         * @brief Gets the class name aliases.
         * @return Returns a sequence containing zero or more class name aliases.
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual sdv::sequence<sdv::u8string> GetClassAliases() const
        {
            return TSdvObject::GetClassAliasesStatic();
        }

        /**
         * @brief Gets the default object name.
         * @return Returns the default name of the object if exists.
         */
        virtual sdv::u8string GetDefaultObjectName() const
        {
            return TSdvObject::GetDefaultObjectNameStatic();
        }

        /**
         * @brief Is the object marked as singleton.
         * @return Returns whether the object is a singleton object.
         */
        virtual bool IsSingleton() const
        {
            return TSdvObject::IsSingletonStatic();
        }

        /**
         * @brief Get object dependencies.
         * @return Returns a vector containing the class names of the objects this component is dependent on.
         */
        virtual sdv::sequence<sdv::u8string> GetObjectDependencies() const
        {
            return TSdvObject::GetObjectDependenciesStatic();
        }

        /**
         * @brief Gets the object type.
         * @return Returns the type of the object
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual EObjectType GetObjectType() const
        {
            return TSdvObject::GetObjectType();
        }

    protected:
        /**
         * @brief Helper function to retrieve the object creation flags.
         * @return Returns the current flags of the object
         */
        uint32_t GetObjectFlags() const
        {
            uint32_t flags = 0;

            if (IsSingleton()) flags |= static_cast<uint32_t>(EObjectFlags::singleton);

            // Currently no other flags known.

            return flags;
        }

        /**
         * @brief Get the class information struct. Overload of ISdvObjectClassInfo::GetClassInfo.
         * @return Returns the class information struct.
         */
        sdv::SClassInfo GetClassInfo() const override
        {
            sdv::SClassInfo classInfo{};
            classInfo.ssClassName = GetClassName();
            classInfo.seqClassAliases = GetClassAliases();
            classInfo.ssDefaultObjectName = GetDefaultObjectName();
            classInfo.eType	= GetObjectType();
            classInfo.uiFlags = GetObjectFlags();
            classInfo.seqDependencies = GetObjectDependencies();
            return classInfo;
        }

        /**
         * @brief Create the SDV object. Overload of ISdvObjectClassInfo::CreateObject.
         * @return Returns an instance of the SDV object or nullptr when the object could not be created (exception thrown during construction).
         */
        std::unique_ptr<CSdvObject> CreateObject() override
        {
            std::unique_ptr<CSdvObject> ret;
            try
            {
                ret = std::make_unique<TSdvObject>();
            }
            catch(...)
            {
                SDV_LOG(core::ELogSeverity::error, "Failed to instantiate object of class ", GetClassName(), " - exception thrown during construction! ");
            }
            return ret;
        }
    };

    /**
     * @brief SDV object base class.
     */
    class CSdvObject : public CSdvObjectAccess
    {
    public:
        /**
         * @brief Use the default object creator class.
         */
        template <class TSdvObject>
        using TSdvObjectCreator = CSdvObjectClass<TSdvObject>;

        /**
         * @brief Constructor
         */
        CSdvObject() = default;

        CSdvObject(CSdvObject&) = delete;
        CSdvObject& operator=(CSdvObject&) = delete;
        CSdvObject(CSdvObject&&) = delete;
        CSdvObject& operator=(CSdvObject&&) = delete;

        /**
         * @brief Destructor
         */
        ~CSdvObject() override
        {}

        /**
         * @brief Class aliases sequence if not provided.
         * @return An empty sequence of class aliases.
         */
        static sdv::sequence<sdv::u8string> GetClassAliasesStatic() { return {}; }

        /**
         * @brief Default object name is not set.
         * @return The empty default object name
         */
        static sdv::u8string GetDefaultObjectNameStatic() { return {}; }

        /**
         * @brief Return whether the object is a singleton object (only one instance will be created of the object).
         * @return Returns whether the object is a singleton.
         */
        static bool IsSingletonStatic() { return false; }

        /**
         * @brief Get object dependencies.
         * @return Returns a vector containing the class names of the objects this component is dependent on.
         */
        static sdv::sequence<sdv::u8string> GetObjectDependenciesStatic() { return sdv::sequence<sdv::u8string>(); }

        /**
         * @brief Interface map
         */
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    };

    /////////////////////////////////////////
    // Object control class implementation //
    /////////////////////////////////////////

    inline IInterfaceAccess* CObjectFactory::CreateObject(const sdv::u8string& ssClassName)
    {
        if (ssClassName.empty())
        {
            return nullptr;
        }

        std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        for (ISdvObjectClassInfo* pObjectClassInfo : m_vecObjectClasses)
        {
            if (pObjectClassInfo == nullptr)continue;
            sdv::SClassInfo sClassInfo = pObjectClassInfo->GetClassInfo();

            // Check for the class name.
            bool bFound = sClassInfo.ssClassName == ssClassName;

            // If not found, check for all aliases.
            for (auto itAlias = sClassInfo.seqClassAliases.begin(); !bFound && itAlias != sClassInfo.seqClassAliases.end(); itAlias++)
                bFound = *itAlias == ssClassName;
            if (!bFound) continue;

            ++m_uiActiveObjectCount;
            auto object = pObjectClassInfo->CreateObject();
            lock.unlock();
            if(!object)
            {
                --m_uiActiveObjectCount;
                return nullptr;
            }
            auto ret = object.get()->GetObjectAccess();
            std::unique_lock<std::mutex> lockObjects (m_mtxActiveObjects);
            m_vecActiveObjects.emplace_back(std::move(object));
            return ret;
        }

        return nullptr;
    }

    inline void CObjectFactory::DestroyObject(IInterfaceAccess* object)
    {
        if (object == nullptr)
        {
            return;
        }

        std::unique_lock<std::mutex> lockObjects(m_mtxActiveObjects);
        for(auto iter = m_vecActiveObjects.begin(); iter!= m_vecActiveObjects.end();++iter)
        {
            if(iter->get()->GetObjectAccess() == object)
            {
                auto objectPtr = std::move(*iter);
                m_vecActiveObjects.erase(iter);
                lockObjects.unlock();
                objectPtr = nullptr;
                if (m_uiActiveObjectCount) --m_uiActiveObjectCount;
                return;
            }
        }

    }

    inline void CObjectFactory::DestroyAllObjects()
    {
        std::unique_lock<std::mutex> lockObjects(m_mtxActiveObjects);
        auto objects = std::move(m_vecActiveObjects);
        lockObjects.unlock();
        while(!objects.empty())
        {
            objects.pop_back();
            if (m_uiActiveObjectCount) --m_uiActiveObjectCount;
        }
    }
} // namespace sdv

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * @brief Define the SDV object (derived from CSdvObject) to be exposed to the framework.
 * @param sdv_object_class The object class must derive from sdv::CSdvObject.
 */
#define DEFINE_SDV_OBJECT(sdv_object_class)                                                                                        \
    struct SObjectClassInstance_##sdv_object_class                                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        SObjectClassInstance_##sdv_object_class()                                                                                  \
        {                                                                                                                          \
            /* Enforce derivation of sdv::CSdvObject. */                                                                           \
            static_assert(std::is_base_of<sdv::CSdvObject, sdv_object_class>::value,                                               \
                          "CSdvObject is not base of sdv_object_class");                                                           \
            /* Call the static function once to instantiate the definition. */                                                     \
            GetObjectClassInstance();                                                                                              \
        }                                                                                                                          \
        static sdv_object_class::TSdvObjectCreator<sdv_object_class>& GetObjectClassInstance()                                     \
        {                                                                                                                          \
            static sdv_object_class::TSdvObjectCreator<sdv_object_class> object_class;                                             \
            return object_class;                                                                                                   \
        }                                                                                                                          \
    };                                                                                                                             \
    static SObjectClassInstance_##sdv_object_class g_##sdv_object_class;                                                           \
    extern "C" SDV_SYMBOL_PUBLIC bool HasActiveObjects();                                                                          \
    extern "C" SDV_SYMBOL_PUBLIC sdv::IInterfaceAccess* GetModuleFactory(uint32_t uiInterfaceVersion);                             \
    extern "C" SDV_SYMBOL_PUBLIC const char* GetManifest();

/**
 * @brief Define the SDV object (derived from CSdvObject) to be exposed to the framework. Do not define export functions.
 * @param sdv_object_class The object class must derive from sdv::CSdvObject.
 */
#define DEFINE_SDV_OBJECT_NO_EXPORT(sdv_object_class)                                                                              \
    struct SObjectClassInstance_##sdv_object_class                                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        SObjectClassInstance_##sdv_object_class()                                                                                  \
        {                                                                                                                          \
            /* Enforce derivation of sdv::CSdvObject. */                                                                           \
            static_assert(std::is_base_of<sdv::CSdvObject, sdv_object_class>::value,                                               \
                          "CSdvObject is not base of sdv_object_class");                                                           \
            /* Call the static function once to instantiate the definition. */                                                     \
            GetObjectClassInstance();                                                                                              \
        }                                                                                                                          \
        static sdv_object_class::TSdvObjectCreator<sdv_object_class>& GetObjectClassInstance()                                     \
        {                                                                                                                          \
            static sdv_object_class::TSdvObjectCreator<sdv_object_class> object_class;                                             \
            return object_class;                                                                                                   \
        }                                                                                                                          \
    };                                                                                                                             \
    static SObjectClassInstance_##sdv_object_class g_##sdv_object_class;                                                           \
    bool HasActiveObjects();                                                                                                       \
    sdv::IInterfaceAccess* GetModuleFactory(uint32_t uiInterfaceVersion);                                                          \
    const char* GetManifest();

/**
 * @brief Declare the object class type. To be placed in the SDV object class derived from CSdvObject.
 * \param class_type The type of the object (EObjectType).
 */
#define DECLARE_OBJECT_CLASS_TYPE(class_type)                                                                                      \
    /**                                                                                                                            \
     * @brief Declare the object class type. To be placed in the SDV object class derived from CSdvObject.                         \
     * @return Returns the type of the object (EObjectType).                                                                       \
     */                                                                                                                            \
    constexpr static sdv::EObjectType GetObjectType()                                                                              \
    {                                                                                                                              \
        return class_type;                                                                                                         \
    }

/**
 * @brief Declare the object class name. To be placed in the SDV object class derived from CSdvObject.
 * @param class_name_string The UTF-8 string containing the name of the object.
 */
#define DECLARE_OBJECT_CLASS_NAME(class_name_string)                                                                               \
    static sdv::u8string GetClassNameStatic()                                                                                      \
    {                                                                                                                              \
        return class_name_string;                                                                                                  \
    }

/**
 * @brief Declare the object class aliases. To be placed in the SDV object class derived from CSdvObject.
 * @param ... Multiple UTF-8 strings containing the alias names for the object class.
 */
#define DECLARE_OBJECT_CLASS_ALIAS(...)                                                                                            \
    static sdv::sequence<sdv::u8string> GetClassAliasesStatic()                                                                    \
    {                                                                                                                              \
        return sdv::sequence<sdv::u8string>({__VA_ARGS__});                                                                        \
    }

 /**
 * @brief Declare the default object name. To be placed in the SDV object class derived from CSdvObject.
 * @param object_name_string The UTF-8 string containing the name of the object.
 */
#define DECLARE_DEFAULT_OBJECT_NAME(object_name_string)                                                                            \
    static sdv::u8string GetDefaultObjectNameStatic()                                                                              \
    {                                                                                                                              \
        return object_name_string;                                                                                                 \
    }

/**
 * @brief Declare the object to be a singleton object (only one instance will be created of the object).
 */
#define DECLARE_OBJECT_SINGLETON()                                                                                                 \
    static bool IsSingletonStatic() { return true; }

/**
 * @brief Declare the object to be a singleton object (only one instance will be created of the object).
 * One or more strings with the class/default names of the objects the component is dependent on.
 */
#define DECLARE_OBJECT_DEPENDENCIES(...)                                                                                           \
    static sdv::sequence<sdv::u8string> GetObjectDependenciesStatic() { return sdv::sequence<sdv::u8string>({__VA_ARGS__}); }

 // NOLINTEND(cppcoreguidelines-macro-usage)

/*
 * @brief Returns whether or not instances of objects implemented by this module are running. If none, the module can be unloaded.
 * @remarks Unloading the module with running instances could cause a crash and should be prevented at all costs. Unloading the
 * module removes the code of the objects still running.
 * @return Returns true when object instances are running; otherwise returns 'false'.
 */
inline extern bool HasActiveObjects()
{
    return sdv::GetModule().GetActiveObjects() != 0;
}

/*
 * @brief Get the module factory interface with a specific version.
 * @details This function provides access to the objects being implemented in this module.
 * @param[in] interfaceVersion Request the module factory for a specific interface version. Using another interface version than
 * the one returned from the manifest, might cause the function to fail.
 * @return Returns pointer to the IInterfaceAccess interface of the module factory object.
 */
inline extern sdv::IInterfaceAccess* GetModuleFactory(uint32_t interfaceVersion)
{
    return sdv::GetModule().GetModuleControl(interfaceVersion);
}

/*
 * @brief Get the the module manifest.
 * @details Each module contains a manifest containing general information as well as information about the component classes. This
 * allows installing the component without having to instantiate the classes.
 * @return Returns the pointer to a zero terminated string containing the module manifest or NULL when there is no string.
 */
inline extern const char* GetManifest()
{
    return sdv::GetModule().GetManifest();
}

#endif // !defined COMPONENT_IMPL_H
