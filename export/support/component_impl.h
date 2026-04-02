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

#ifndef COMPONENT_IMPL_H
#define COMPONENT_IMPL_H

#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <string>
#include <sstream>

#include "../interfaces/core_types.h"
#include "../interfaces/core.h"
#include "../interfaces/repository.h"
#include "sdv_core.h"
#include "interface_ptr.h"
#include "local_service_access.h"
#include "param_impl.h"
#include "toml.h"

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
        virtual SClassInfo GetClassInfo() const = 0;

        /**
         * @brief Get the object class name.
         * @return Returns the class name of the object.
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual u8string GetClassName() const = 0;

        /**
         * @brief Get the class name aliases.
         * @return Returns a sequence containing zero or more class name aliases.
         */
        virtual sequence<u8string> GetClassAliases() const = 0;

        /**
         * @brief Get the default object name.
         * @return Returns the default name of the object if exists.
         */
        virtual u8string GetDefaultObjectName() const = 0;

        /**
         * @brief Get the default config TOML string.
         * @param[in] rssTablePrefix Reference to the string containing a table prefix to insert as part of the table key.
         * @return Returns the TOML string containing the default configuration of the object if exists.
         */
        virtual u8string GetDefaultConfig(const u8string& rssTablePrefix = {}) const = 0;

        /**
         * @brief Get the object type.
         * @return Returns the type of the object
         */
        virtual EObjectType GetObjectType() const = 0;

        /**
         * @brief Get the object creation flags.
         * @return Returns the current flags of the object
         */
        virtual uint32_t GetObjectFlags() const = 0;

        /**
         * @brief Get object dependencies.
         * @return Returns a vector containing the class names of the objects this component is dependent on.
         */
        virtual sequence<u8string> GetObjectDependencies() const = 0;

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
         * @brief Get the amount of active SDV objects.
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
         * @brief The object class names implemented in the object. Overload of sdv::IObjectFactory::GetClassNames.
         * @return Sequence with object class names string.
         */
        virtual sequence<u8string> GetClassNames() const override;

        /**
         * @brief Get the class information. Overload of sdv::IObjectFactory::GetClassInfo.
         * @param[in] ssClassName The name of the class object to get the class information for.
         * @return Returns the class information struct.
         */
        virtual SClassInfo GetClassInfo(const u8string& ssClassName) const override;

        /**
         * @brief Create or get the object using the name from the object class info. Overload of
         * sdv::IObjectFactory::CreateObject.
         * @attention The objects lifetime is ended by a call to the DestroyObject function or the unloading of he module.
         * @param[in] ssClassName The name of the class object to instantiate.
         * @return Pointer to IInterfaceAccess interface of the object or NULL when the requested object doesn't exist.
         */
        virtual IInterfaceAccess* CreateObject(const u8string& ssClassName) override;

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
         */
        void BuildManifest();

        /**
         * @brief Expose the object prototype by placing it into the object prototype list.
         * @details Expose the object class to allow class access. This function is called by the constructor of the
         * class object.
         * @param[in] pClassInfo The object class internal interface.
         */
        void ExposeObjectClass(ISdvObjectClassInfo* pClassInfo);

        /**
         * @brief Revoke the object prototype from the object prototype list.
         * @param[in] pClassInfo The object class internal interface.
         */
        void RevokeObjectClass(const ISdvObjectClassInfo* pClassInfo);

        /**
         * @brief Quote the provided text following the string rules of TOML.
         * @param[in] rssText Reference to the text to quote.
         * @return The quoted text string.
         */
        static std::string QuoteText(const std::string& rssText);

    private:
        std::atomic<uint32_t>                       m_uiActiveObjectCount{0};   ///< The amount of active SDV objects.

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
    class CModule : public CSdvObjectAccess, public CObjectFactory
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
    class CSdvObjectClass : public ISdvObjectClassInfo
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
         * @brief Get the class information struct. Overload of ISdvObjectClassInfo::GetClassInfo.
         * @return Returns the class information struct.
         */
         SClassInfo GetClassInfo() const override
        {
            SClassInfo sClassInfo{};
            sClassInfo.ssName = GetClassName();
            sClassInfo.seqClassAliases = GetClassAliases();
            sClassInfo.ssDefaultObjectName = GetDefaultObjectName();
            sClassInfo.ssDefaultConfig = GetDefaultConfig("Parameters");
            sClassInfo.eType = GetObjectType();
            sClassInfo.uiFlags = GetObjectFlags();
            sClassInfo.seqDependencies = GetObjectDependencies();
            return sClassInfo;
        }

        /**
         * @brief Get the object class name. Overload of ISdvObjectClassInfo::GetClassName.
         * @return Returns the class name of the object.
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual u8string GetClassName() const override
        {
            return TSdvObject::GetClassNameStatic();
        }

        /**
         * @brief Get the class name aliases. Overload of ISdvObjectClassInfo::GetClassAliases.
         * @return Returns a sequence containing zero or more class name aliases.
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual sequence<u8string> GetClassAliases() const override
        {
            return TSdvObject::GetClassAliasesStatic();
        }

        /**
         * @brief Get the default config TOML string. Overload of ISdvObjectClassInfo::GetDefaultConfig.
         * @param[in] rssTablePrefix Reference to the string containing a table prefix to insert as part of the table key.
         * @return Returns the TOML string containing the default configuration of the object if exists.
         */
        virtual u8string GetDefaultConfig(const u8string& rssTablePrefix /*= {}*/) const override
        {
            std::vector<std::shared_ptr<CSdvParamInfo>> vecParaminfo = TSdvObject::GetParamMapInfoStatic();
            std::stringstream sstream;
            u8string ssGroup;
            for (const auto& ptrParam : vecParaminfo)
            {
                if (!ptrParam) continue;

                // Only describe readable and non-temporary parameters
                if (ptrParam->ReadOnly() || ptrParam->Temporary()) continue;

                // Is there a value? If not, skip the parameter
                if (ptrParam->DefaultVal().empty()) continue;

                // Need a new group?
                if (ptrParam->Group() != ssGroup)
                {
                    if (sstream.rdbuf()->in_avail())
                        sstream << std::endl;
                    sstream << "[";
                    if (!rssTablePrefix.empty())
                        sstream << rssTablePrefix << ".";
                    sstream << ptrParam->Group() << "]" << std::endl;
                    ssGroup = ptrParam->Group();
                }

                // Store the parameter
                // Quotation of parameter name needed?
                u8string ssName;
                if (ptrParam->Name().find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-")
                    != u8string::npos)
                    ssName = CObjectFactory::QuoteText(ptrParam->Name());
                else
                    ssName = ptrParam->Name();
                sstream << ssName << " = ";
                if (ptrParam->Numeric() || ptrParam->Bitmask() || ptrParam->Enum())
                {
                    std::string ssValue = ptrParam->DefaultVal();
                    sstream << (ssValue.empty() ? "0" : ssValue);
                }
                else if (ptrParam->String())
                    sstream << CObjectFactory::QuoteText(ptrParam->DefaultVal());
                else if (ptrParam->Boolean())
                    sstream << (ptrParam->DefaultVal().get<bool>() ? "true" : "false");
                sstream << std::endl;
            }
            return sstream.str();
        }

        /**
         * @brief Get the default object name. Overload of ISdvObjectClassInfo::GetDefaultObjectName.
         * @return Returns the default name of the object if exists.
         */
        virtual u8string GetDefaultObjectName() const override
        {
            return TSdvObject::GetDefaultObjectNameStatic();
        }

        /**
         * @brief Get the object type. Overload of ISdvObjectClassInfo::GetObjectType.
         * @return Returns the type of the object
         * @remarks Default implementation gets the information from the object class if not defined in the definition.
         */
        virtual EObjectType GetObjectType() const override
        {
            return TSdvObject::GetObjectType();
        }

        /**
         * @brief Get the object creation flags. Overload of ISdvObjectClassInfo::GetObjectFlags.
         * @return Returns the current flags of the object
         */
        uint32_t GetObjectFlags() const override
        {
            uint32_t uiFlags = 0;

            if (TSdvObject::IsSingletonStatic())
                uiFlags |= static_cast<uint32_t>(EObjectFlags::singleton);

            // Currently no other flags defined.

            return uiFlags;
        }

        /**
         * @brief Get object dependencies. Overload of ISdvObjectClassInfo::GetObjectDependencies.
         * @return Returns a vector containing the class names of the objects this component is dependent on.
         */
        virtual sequence<u8string> GetObjectDependencies() const override
        {
            return TSdvObject::GetObjectDependenciesStatic();
        }

        /**
         * @brief Create the SDV object. Overload of ISdvObjectClassInfo::CreateObject.
         * @return Returns an instance of the SDV object or nullptr when the object could not be created (exception thrown during construction).
         */
        virtual std::unique_ptr<CSdvObject> CreateObject() override;
    };

    /**
     * @brief SDV object base class.
     * @details The SDV object class implements all base functionality for is instantiating and controlling the SDV object. A SDV
     * object implementation can derive from this class and overload the following events to control object behavior:
     * - OnPreInitialize - prepare for initialization, no object configuration loaded yet.
     * - OnInitialize - initialize; object configuration is loaded and stored in the parameters.
     * - OnChangeToConfigMode - change to configuration mode; shutdown running instances.
     * - OnChangeToRunningMode - change to running mode, interpret configuration parameters and start running instances.
     * - OnShutdown - shutdown the object
     * - OnParamChanged - parameter has changed
     * - OnParamFlagChanged - parameter flag has changed
     */
    class CSdvObject : public CSdvParamMap, public CSdvObjectAccess, public IObjectControl
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
         * @brief Initialize the object. Overload of IObjectControl::Initialize.
         * @param[in] ssObjectConfig Optional configuration string.
         */
        virtual void Initialize(/*in*/ const u8string& ssObjectConfig) override
        {
            // Not started before or ended completely?
            if (GetObjectState() != EObjectState::initialization_pending && GetObjectState() != EObjectState::destruction_pending)
            {
                m_eObjectState = EObjectState::runtime_error;
                return;
            }

            // Initializing
            m_eObjectState = EObjectState::initializing;

            // Initialize the parameter map.
            InitParamMap();

            // Copy the configuration
            m_ssObjectConfig = ssObjectConfig;

            // Prepare initialization
            OnPreInitialize();

            // Parse the object configuration if there is any.
            if (!ssObjectConfig.empty())
            {
                try
                {
                    // Parse the config TOML.
                    toml::CTOMLParser parser(ssObjectConfig);
                    if (!parser.IsValid())
                    {
                        m_eObjectState = EObjectState::config_error;
                        return;
                    }

                    // Read group function (iterative)
                    u8string ssGroup;
                    std::function<void(const toml::CNodeCollection&)> fnReadGroup;
                    fnReadGroup = [&](const toml::CNodeCollection& rTable)
                    {
                        // Iterate through the config.
                        for (size_t n = 0; n < rTable.GetCount(); n++)
                        {
                            const toml::CNode node = rTable.Get(n);
                            switch (node.GetType())
                            {
                            case toml::ENodeType::node_integer:
                            case toml::ENodeType::node_floating_point:
                            case toml::ENodeType::node_boolean:
                            case toml::ENodeType::node_string:
                                // Ignore the result. If it is not possible to set the parameter, this is not a failure.
                                SetParam(ssGroup + node.GetName(), node.GetValue());
                                break;
                            case toml::ENodeType::node_table:
                                {
                                    const toml::CNodeCollection sub_table= node;
                                    if (!sub_table.IsValid()) break;
                                    ssGroup += sub_table.GetName() + ".";
                                    fnReadGroup(sub_table);
                                }
                            default:
                                // Other node types cannot be processed.
                                break;
                            }
                        }
                    };

                    // Parse the root node
                    fnReadGroup(parser);
                } catch (const toml::XTOMLParseException& rexcept)
                {
                    SDV_LOG(core::ELogSeverity::error, "Cannot process object configuration TOML: ", rexcept.what());
                    m_eObjectState = EObjectState::initialization_failure;
                    return;
                }
            }

            // Let the object initialize itself.
            if (!OnInitialize())
            {
                m_eObjectState = EObjectState::initialization_failure;
                return;
            }

            // Initialization is complete.
            m_eObjectState = EObjectState::initialized;
        }

        /**
         * @brief Get the current state of the object. Overload of IObjectControl::GetObjectState.
         * @return Return the current state of the object.
         */
        virtual EObjectState GetObjectState() const override
        {
            return m_eObjectState;
        }

        /**
         * @brief Set the component operation mode. Overload of IObjectControl::SetOperationMode.
         * @param[in] eMode The operation mode, the component should run in.
         */
        virtual void SetOperationMode(/*in*/ EOperationMode eMode) override
        {
            switch (eMode)
            {
            case EOperationMode::configuring:
                // Allowed?
                if (GetObjectState() != EObjectState::initialized && GetObjectState() != EObjectState::running &&
                    GetObjectState() != EObjectState::runtime_error && GetObjectState() != EObjectState::config_error)
                    break;

                // Inform derived class
                OnChangeToConfigMode();

                // Unlock the parameter map.
                UnlockParamMap();

                m_eObjectState = EObjectState::configuring;
                break;
            case EOperationMode::running:
                // Allowed?
                if (GetObjectState() != EObjectState::initialized && GetObjectState() != EObjectState::configuring &&
                    GetObjectState() != EObjectState::config_error)
                    break;

                // Lock the parameter map.
                LockParamMap();

                // Inform the derived class
                if (!OnChangeToRunningMode())
                {
                    // Cannot enter running mode. Reconfiguration needed.
                    m_eObjectState = EObjectState::config_error;
                    UnlockParamMap();
                    break;
                }

                m_eObjectState = EObjectState::running;
                break;
            default:
                // Wrong operation mode.
                m_eObjectState = EObjectState::runtime_error;
                break;
            }
        }

        /**
         * @brief Get the object configuration for persistence.
         * @return The object configuration as TOML string.
         */
        virtual u8string GetObjectConfig() const override
        {
            // During the initialization, return the stored object configuration.
            if (m_eObjectState == EObjectState::initializing || m_eObjectState == EObjectState::initialization_pending)
                return m_ssObjectConfig;

            // Split path function (splits the group from the parameter names)
            auto fnSplitPath = [](const u8string& rssPath) -> std::pair<u8string, u8string>
            {
                size_t nPos = rssPath.find_last_of('.');
                if (nPos == u8string::npos)
                    return std::make_pair("", rssPath);
                return std::make_pair(rssPath.substr(0, nPos), rssPath.substr(nPos + 1));
            };

            // Create a new configuration string from the parameters.
            auto seqParameters = GetParamPaths();
            if (seqParameters.empty()) return {};

            // Iterate through the list of parameter names and create the TOML entries for it.
            u8string ssGroup;
            toml::CTOMLParser parser("");
            toml::CNodeCollection table(parser);
            for (auto ssParamPath : seqParameters)
            {
                // Get the parameter object
                auto ptrParam = FindParamObject(ssParamPath);
                
                // Read only and temporary parameters are not stored
                if ((!ptrParam->Locked() && ptrParam->ReadOnly()) || !ptrParam->Temporary())
                    continue;

                // Get the value
                any_t anyValue = ptrParam->Get();
                if (anyValue.empty())
                    continue;

                // Split the path in group and parameter name
                auto prParam = fnSplitPath(ssParamPath);

                // Need to add a group?
                if (prParam.first != ssGroup)
                {
                    table = parser.AddTable(prParam.first);
                    ssGroup = prParam.first;
                }

                // Store the value
                table.AddValue(prParam.second, anyValue);
            }

            return parser.GetTOML();
        }

        /**
         * @brief Shutdown called before the object is destroyed. Overload of IObjectControl::Shutdown.
         * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the
         * object! After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
         * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still
         * call it during the shutdown sequence! Any subsequent call to GetObjectState should return EObjectState::destruction_pending
         */
        virtual void Shutdown() override
        {
            // Lock the parameter map - do not allow any more changes
            LockParamMap();

            // Set state
            m_eObjectState = EObjectState::shutdown_in_progress;

            // Inform derived class
            OnShutdown();

            // Set status
            m_eObjectState = EObjectState::destruction_pending;

            // Unlock parameter map to allow re-initialization.
            UnlockParamMap();
        }

        /**
         * @brief Set the object into configuration error state.
         * @pre Works only when in configuration or initialized mode.
         * @post Reset by switching to configuration mode again or by trying to switch to running mode.
         */
        void SetObjectIntoConfigErrorState()
        {
            if (m_eObjectState == EObjectState::initialized || m_eObjectState == EObjectState::configuring)
                m_eObjectState = EObjectState::config_error;
        }

        /**
         * @brief Set the onbject into runtime error state.
         * @pre Works only when in running mode.
         * @post Reset by switching to configuration mode.
         */
        void SetObjectIntoRuntimeErrorState()
        {
            if (m_eObjectState == EObjectState::running)
                m_eObjectState = EObjectState::runtime_error;
        }

        /**
         * @brief Class aliases sequence if not provided.
         * @return An empty sequence of class aliases.
         */
        static sequence<u8string> GetClassAliasesStatic() { return {}; }

        /**
         * @brief Default object name is not set.
         * @return The empty default object name
         */
        static u8string GetDefaultObjectNameStatic() { return {}; }

        /**
         * @brief Return whether the object is a singleton object (only one instance will be created of the object).
         * @return Returns whether the object is a singleton.
         */
        static bool IsSingletonStatic() { return false; }

        /**
         * @brief Get object dependencies.
         * @return Returns a vector containing the class names of the objects this component is dependent on.
         */
        static sequence<u8string> GetObjectDependenciesStatic() { return sequence<u8string>(); }

        /**
         * @brief Pre-initialization event, called before loading the object configuration. Overload this function to implement
         * pre-initialization functionality.
         */
        virtual void OnPreInitialize() {}

        /**
         * @brief Initialization event, called after object configuration was loaded. Overload this function to implement object
         * initialization.
         * @return Returns 'true' when the initialization was successful, 'false' when not.
         */
        virtual bool OnInitialize() { return true; }

        /**
         * @brief Change to configuration mode event. After this a call to this function locked parameters can be changed again.
         * Overload this function to shutdown any running instances and prepare for configuration.
         */
        virtual void OnChangeToConfigMode() {}

        /**
         * @brief Change to running mode event. Parameters were locked before the call to this event. Overload this function to
         * interpret the configuration parameters and start the running instances.
         * @return Returns 'true' when the configuration is valid and the running instances could be started. Otherwise returns
         * 'false'.
         */
        virtual bool OnChangeToRunningMode() { return true; }

        /**
         * @brief Shutdown the object. Overload this function to terminate any running instances and prepare the object for
         * termination or restarting.
         */
        virtual void OnShutdown() {}

        /**
         * @brief Interface map
         */
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_ENTRY(IParameters)
            SDV_INTERFACE_ENTRY(IObjectControl)
        END_SDV_INTERFACE_MAP()

    private:
        std::atomic<EObjectState>   m_eObjectState = EObjectState::initialization_pending;  ///< Object state
        std::string                 m_ssObjectConfig;                                       ///< Copy of the configuration TOML.
    };

    /**
     * @brief The object control wrapper class ensures, that the CSdvObject interface map is always accessible.
     * @tparam The class to instantiate.
     */
    template <class TSdvObject>
    class CObjectInstance : public TSdvObject
    {
        static_assert(std::is_base_of_v<CSdvObject, TSdvObject>, "TSdvObject must derive from sdv::CSdvObject!");

    public:
        // Interface map
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_CHAIN_BASE(CSdvObject)
            SDV_INTERFACE_CHAIN_BASE(TSdvObject)
        END_SDV_INTERFACE_MAP()
    };

    /////////////////////////////////////////
    // Object control class implementation //
    /////////////////////////////////////////

    inline sequence<u8string> CObjectFactory::GetClassNames() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        sequence<u8string> seqClassNames;
        for (const ISdvObjectClassInfo* pClassInfo : m_vecObjectClasses)
        {
            if (!pClassInfo) continue;
            seqClassNames.push_back(pClassInfo->GetClassName());
        }
        return seqClassNames;
    }

    inline SClassInfo CObjectFactory::GetClassInfo(const u8string& ssClassName) const
    {
        if (ssClassName.empty()) return {};

        std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        for (ISdvObjectClassInfo* pClassInfo : m_vecObjectClasses)
        {
            if (!pClassInfo) continue;

            // Check for the class name.
            bool bFound = pClassInfo->GetClassName() == ssClassName;

            // If not found, check for all aliases.
            auto seqClassAliases = pClassInfo->GetClassAliases();
            for (auto itAlias = seqClassAliases.begin(); !bFound && itAlias != seqClassAliases.end(); ++itAlias)
                bFound = *itAlias == ssClassName;
            if (!bFound) continue;

            // Return the class information
            return pClassInfo->GetClassInfo();
        }

        // No class information found
        return {};
    }

    inline IInterfaceAccess* CObjectFactory::CreateObject(const u8string& ssClassName)
    {
        if (ssClassName.empty()) return nullptr;

        std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        for (ISdvObjectClassInfo* pClassInfo : m_vecObjectClasses)
        {
            if (!pClassInfo) continue;

            // Check for the class name.
            bool bFound = pClassInfo->GetClassName() == ssClassName;

            // If not found, check for all aliases.
            auto seqClassAliases = pClassInfo->GetClassAliases();
            for (auto itAlias = seqClassAliases.begin(); !bFound && itAlias != seqClassAliases.end(); ++itAlias)
                bFound = *itAlias == ssClassName;
            if (!bFound) continue;

            ++m_uiActiveObjectCount;
            auto object = pClassInfo->CreateObject();
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
        for(auto iter = m_vecActiveObjects.begin(); iter != m_vecActiveObjects.end(); ++iter)
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

    inline void CObjectFactory::BuildManifest()
    {
        // Start the manifest
        std::stringstream sstream;
        sstream << R"toml(# Module manifest

[Interface]
Version = )toml" << SDVFrameworkInterfaceVersion
                << std::endl
                << std::endl;

        // Add all classes to the manifest
        std::shared_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        for (const ISdvObjectClassInfo* pClassInfo : m_vecObjectClasses)
        {
            sstream << "[[Class]]" << std::endl;
            sstream << "Name = " << QuoteText(pClassInfo->GetClassName()) << std::endl;
            if (pClassInfo->GetClassName().empty())
                continue;
            if (!pClassInfo->GetClassAliases().empty())
            {
                sstream << "Aliases = [";
                bool bInitialAlias = true;
                for (const u8string& rssAlias : pClassInfo->GetClassAliases())
                {
                    if (!bInitialAlias)
                        sstream << ", ";
                    bInitialAlias = false;
                    sstream << QuoteText(rssAlias);
                }
                sstream << "]" << std::endl;
            }
            if (!pClassInfo->GetDefaultObjectName().empty())
                sstream << "DefaultName = " << QuoteText(pClassInfo->GetDefaultObjectName()) << std::endl;
            if (pClassInfo->GetObjectType() == EObjectType::undefined)
                continue;
            sstream << "Type = " << QuoteText(ObjectType2String(pClassInfo->GetObjectType())) << std::endl;
            if (pClassInfo->GetObjectFlags() & static_cast<uint32_t>(EObjectFlags::singleton))
                sstream << "Singleton = true" << std::endl;
            if (!pClassInfo->GetObjectDependencies().empty())
            {
                sstream << "Dependencies = [";
                bool bInitialDependency = true;
                for (const u8string& rssDependsOn : pClassInfo->GetObjectDependencies())
                {
                    if (!bInitialDependency)
                        sstream << ", ";
                    bInitialDependency = false;
                    sstream << "\"" << rssDependsOn << "\"";
                }
                sstream << "]" << std::endl;
            }
            // Assign the parameter table to the class. Prepend [Class.Parameters] to every table.
            std::string ssParameters = pClassInfo->GetDefaultConfig("Class.Parameters");
            if (!ssParameters.empty()) sstream << ssParameters;
        }
        m_ssManifest = sstream.str();
    }

    inline void CObjectFactory::ExposeObjectClass(ISdvObjectClassInfo* pClassInfo)
    {
        if (pClassInfo)
        {
            std::unique_lock<std::shared_mutex> lock(m_mtxObjectClasses);
            m_vecObjectClasses.push_back(pClassInfo);

            // Attention: pObjectClass is a pointer to ISdvObjectClassInfo even if the class was derived.
            // Virtual functions are not available yet at this stage.
        }
    }

    inline void CObjectFactory::RevokeObjectClass(const ISdvObjectClassInfo* pClassInfo)
    {
        std::unique_lock<std::shared_mutex> lock(m_mtxObjectClasses);
        auto itObjectClass = std::find(m_vecObjectClasses.begin(), m_vecObjectClasses.end(), pClassInfo);
        if (itObjectClass != m_vecObjectClasses.end())
            m_vecObjectClasses.erase(itObjectClass);
        // TODO EVE: Updated through cppcheck warning
        //for (auto objectClassIter = m_vecObjectClasses.begin(); objectClassIter != m_vecObjectClasses.end();
        //     objectClassIter++)
        //{
        //    if (*objectClassIter == pClassInfo)
        //    {
        //        m_vecObjectClasses.erase(objectClassIter);
        //        break;
        //    }
        //}
    }

    inline std::string CObjectFactory::QuoteText(const std::string& rssText)
    {
        std::stringstream sstreamQuotedText;
        sstreamQuotedText << "\"";
        for (size_t nPos = 0; nPos < rssText.size(); nPos++)
        {
            uint8_t uiChar = static_cast<uint8_t>(rssText[nPos]);
            uint32_t uiUTFChar = 0;
            switch (uiChar)
            {
            case '\'':
                sstreamQuotedText << '\'';
                break; // Single quote character
            case '\b':
                sstreamQuotedText << "\\b";
                break; // Escape backspace
            case '\t':
                sstreamQuotedText << "\\t";
                break; // Escape tab
            case '\n':
                sstreamQuotedText << "\\n";
                break; // Escape linefeed
            case '\f':
                sstreamQuotedText << "\\f";
                break; // Escape form feed
            case '\r':
                sstreamQuotedText << "\\r";
                break; // Escape carriage return
            case '\"':
                sstreamQuotedText << "\\\"";
                break; // Escape quote
            case '\\':
                sstreamQuotedText << "\\\\";
                break; // Escape backslash
            default:
                // Check for ASCII character
                if (uiChar >= 0x20 && uiChar < 0x7f)
                {
                    // Standard ASCII
                    sstreamQuotedText << static_cast<char>(uiChar);
                    break;
                }

                // Use UNICODE escape character for the quoted text
                if (uiChar <= 0x80) // One byte UTF-8
                    uiUTFChar = static_cast<uint32_t>(uiChar);
                else if (uiChar <= 0xDF) // Two bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00011111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }
                else if (uiChar <= 0xEF) // Three bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00001111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }
                else if (uiChar <= 0xF7) // Four bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00000111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }

                // Stream the UTF character
                if (uiUTFChar <= 0xFFFF)
                    sstreamQuotedText << "\\u" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << uiUTFChar;
                else
                    sstreamQuotedText << "\\U" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << uiUTFChar;
                break;
            }
        }
        sstreamQuotedText << "\"";
        return sstreamQuotedText.str();
    }

    template <class TSdvObject>
    inline std::unique_ptr<CSdvObject> CSdvObjectClass<TSdvObject>::CreateObject()
    {
        std::unique_ptr<CSdvObject> ret;
        try
        {
            ret = std::make_unique<CObjectInstance<TSdvObject>>();
        }
        catch (...)
        {
            SDV_LOG(core::ELogSeverity::error,
                "Failed to instantiate object of class ",
                GetClassName(),
                " - exception thrown during construction! ");
        }
        return ret;
    }

} // namespace sdv

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#ifdef SDV_NO_CLASS_DEFINITION

/**
 * @brief Define the SDV component access functions, but do not define the SDV object.
 * @param sdv_object_class The object class must derive from sdv::CSdvObject.
 */
#define DEFINE_SDV_OBJECT(sdv_object_class)                                                                                        \
    extern "C" bool HasActiveObjects();                                                                                            \
    extern "C" sdv::IInterfaceAccess* GetModuleFactory(uint32_t uiInterfaceVersion);                                               \
    extern "C" const char* GetManifest();

#elif defined SDV_NO_EXPORT_DEFINITION

/**
 * @brief Define the SDV object (derived from CSdvObject) to be exposed to the framework. Do not define export functions.
 * @param sdv_object_class The object class must derive from sdv::CSdvObject.
 */
#define DEFINE_SDV_OBJECT(sdv_object_class)                                                                                        \
    /**                                                                                                                            \
     * @brief Object instantiation helper class.                                                                                   \
     */                                                                                                                            \
    struct SObjectClassInstance_##sdv_object_class                                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        /**                                                                                                                        \
         * @brief Constructor                                                                                                      \
         */                                                                                                                        \
        SObjectClassInstance_##sdv_object_class()                                                                                  \
        {                                                                                                                          \
            /* Enforce derivation of sdv::CSdvObject. */                                                                           \
            static_assert(std::is_base_of<sdv::CSdvObject, sdv_object_class>::value,                                               \
                          "CSdvObject is not base of sdv_object_class");                                                           \
            /* Call the static function once to instantiate the definition. */                                                     \
            GetObjectClassInstance();                                                                                              \
        }                                                                                                                          \
        /**                                                                                                                        \
         * @brief Get the creator class for the object instance.                                                                   \
         * @return The global creator class.                                                                                       \
         */                                                                                                                        \
        static sdv_object_class::TSdvObjectCreator<sdv_object_class>& GetObjectClassInstance()                                     \
        {                                                                                                                          \
            static sdv_object_class::TSdvObjectCreator<sdv_object_class> object_class;                                             \
            return object_class;                                                                                                   \
        }                                                                                                                          \
    };                                                                                                                             \
    /** Global instance of the object helper class. */                                                                             \
    static SObjectClassInstance_##sdv_object_class g_##sdv_object_class;                                                           \
    bool HasActiveObjects();                                                                                                       \
    sdv::IInterfaceAccess* GetModuleFactory(uint32_t uiInterfaceVersion);                                                          \
    const char* GetManifest();

#else

/**
 * @brief Define the SDV object (derived from CSdvObject) to be exposed to the framework.
 * @param sdv_object_class The object class must derive from sdv::CSdvObject.
 */
#define DEFINE_SDV_OBJECT(sdv_object_class)                                                                                        \
    /**                                                                                                                            \
     * @brief Object instantiation helper class.                                                                                   \
     */                                                                                                                            \
    struct SObjectClassInstance_##sdv_object_class                                                                                 \
    {                                                                                                                              \
    public:                                                                                                                        \
        /**                                                                                                                        \
         * @brief Constructor                                                                                                      \
         */                                                                                                                        \
        SObjectClassInstance_##sdv_object_class()                                                                                  \
        {                                                                                                                          \
            /* Enforce derivation of sdv::CSdvObject. */                                                                           \
            static_assert(std::is_base_of<sdv::CSdvObject, sdv_object_class>::value,                                               \
                          "CSdvObject is not base of sdv_object_class");                                                           \
            /* Call the static function once to instantiate the definition. */                                                     \
            GetObjectClassInstance();                                                                                              \
        }                                                                                                                          \
        /**                                                                                                                        \
         * @brief Get the creator class for the object instance.                                                                   \
         * @return The global creator class.                                                                                       \
         */                                                                                                                        \
        static sdv_object_class::TSdvObjectCreator<sdv_object_class>& GetObjectClassInstance()                                     \
        {                                                                                                                          \
            static sdv_object_class::TSdvObjectCreator<sdv_object_class> object_class;                                             \
            return object_class;                                                                                                   \
        }                                                                                                                          \
    };                                                                                                                             \
    /** Global instance of the object helper class. */                                                                             \
    static SObjectClassInstance_##sdv_object_class g_##sdv_object_class;                                                           \
    extern "C" SDV_SYMBOL_PUBLIC bool HasActiveObjects();                                                                          \
    extern "C" SDV_SYMBOL_PUBLIC sdv::IInterfaceAccess* GetModuleFactory(uint32_t uiInterfaceVersion);                             \
    extern "C" SDV_SYMBOL_PUBLIC const char* GetManifest();

#endif // !defined SDV_NO_CLASS_DEFINITION && !defined SDV_NO_EXPORT_DEFINITION

/**
 * @brief Declare the object class type. To be placed in the SDV object class derived from CSdvObject.
 * @param class_type The type of the object (EObjectType).
 */
#define DECLARE_OBJECT_CLASS_TYPE(class_type)                                                                                      \
    /**                                                                                                                            \
     * @brief Get the object class type.                                                                                           \
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
    /**                                                                                                                            \
     * @brief Get the class name.                                                                                                  \
     * @return Returns the string containing the class name.                                                                       \
     */                                                                                                                            \
    static sdv::u8string GetClassNameStatic()                                                                                      \
    {                                                                                                                              \
        return class_name_string;                                                                                                  \
    }

/**
 * @brief Declare the object class aliases. To be placed in the SDV object class derived from CSdvObject.
 * @param ... Multiple UTF-8 strings containing the alias names for the object class.
 */
#define DECLARE_OBJECT_CLASS_ALIAS(...)                                                                                            \
    /**                                                                                                                            \
     * @brief Get the list of class name aliases.                                                                                  \
     * @return Vector containing the class name alias strings.                                                                     \
     */                                                                                                                            \
    static sdv::sequence<sdv::u8string> GetClassAliasesStatic()                                                                    \
    {                                                                                                                              \
        return sdv::sequence<sdv::u8string>({__VA_ARGS__});                                                                        \
    }

 /**
 * @brief Declare the default object name. To be placed in the SDV object class derived from CSdvObject.
 * @param object_name_string The UTF-8 string containing the name of the object.
 */
#define DECLARE_DEFAULT_OBJECT_NAME(object_name_string)                                                                            \
    /**                                                                                                                            \
     * @brief Get the default name to be used during the object instantiation.                                                     \
     * @return String containing the default name.                                                                                 \
     */                                                                                                                            \
    static sdv::u8string GetDefaultObjectNameStatic()                                                                              \
    {                                                                                                                              \
        return object_name_string;                                                                                                 \
    }

/**
 * @brief Declare the object to be a singleton object (only one instance will be created of the object).
 */
#define DECLARE_OBJECT_SINGLETON()                                                                                                 \
    /**                                                                                                                            \
     * @brief Is the class defined as singleton (allows at the most one instance of the class)?                                    \
     * @return Returns whether the class is defined as singleton.                                                                  \
     */                                                                                                                            \
    static bool IsSingletonStatic() { return true; }

/**
 * @brief Declare the object to be a singleton object (only one instance will be created of the object).
 * One or more strings with the class/default names of the objects the component is dependent on.
 */
#define DECLARE_OBJECT_DEPENDENCIES(...)                                                                                           \
    /**                                                                                                                            \
     * @brief Get the list of object dependencies.                                                                                 \
     * @return The sequence containing the dependency strings.                                                                     \
     */                                                                                                                            \
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
