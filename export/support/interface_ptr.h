/**
 *
 * @file      interface_ptr.h
 * @brief     This file provides all necessary definitions to use and implement interface maps.
 * @version   0.1
 * @date      2022.11.14
 * @author    Thomas.pfleiderer@zf.com
 * @copyright Copyright ZF Friedrichshaven AG (c) 2022
 *
 */
#ifndef INTERFACE_IMPL_H
#define INTERFACE_IMPL_H

#include <atomic>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeinfo> // AXIVION Same Line AutosarC++19_03-A16.2.2

#ifndef DONT_LOAD_CORE_TYPES
#include "../interfaces/core.h"
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    /**
     * @brief Internal namespace
     */
    namespace internal
    {
        /**
         * @brief Helper struct for section selection during interface map processing.
         */
        class CSectionSelector
        {
        public:
            /**
             * @brief Any processing of interfaces after this the call to this function belongs to the default section.
             */
            void DefineDefaultSection()
            {
                m_uiSection = -1;
            }

            /**
             * @brief Any processing of interfaces after the call of this function, belongs to the section with the provided
             * section number.
             * @param[in] uiSection The section number the interface table entries belong to.
             */
            void DefineSection(int uiSection)
            {
                m_uiSection = uiSection;
            }

            /**
             * Select the section that is supported now.
             */
            void Select(int uiSection)
            {
                m_uiUseSection = uiSection;
            }

            /**
             * @brief Is the current section selected.
             * @return Returns whether the section is currently within the selection.
             */
            bool Selected() const
            {
                return m_uiSection < 0 || m_uiSection == m_uiUseSection;
            }

        private:
            int m_uiUseSection = -1;        ///< The number of the section to process the interface table entries for.
            int m_uiSection    = -1;        ///< The number of the section the interface table entries belong to.
        };
    }
}


/**
 * @brief Interface map containing the supported interface definitions.
 * @pre The class using the interface map should derive from IInterfaceAccess.
 */
#define BEGIN_SDV_INTERFACE_MAP()                                                                                                  \
    /**                                                                                                                            \
     * @brief Gets the requested interface                                                                                         \
     * @param[in] idInterface The id representing the requested interface                                                          \
     * @return Returns the IInterfaceAccess pointer to this if the requested interface is found, nullptr otherwise                 \
     */                                                                                                                            \
    sdv::interface_t GetInterface(sdv::interface_id idInterface) override                                                          \
    {                                                                                                                              \
        if (!idInterface) return nullptr;                                                                                          \
        ::sdv::internal::CSectionSelector selector;                                                                                \
        if (selector.Selected())                                                                                                   \
        {

/**
 * @brief Using a namespace in the interface map.
 */
#define SDV_INTERFACE_USE_NAMESPACE(namespace_name) using namespace namespace_name;

/**
 * @brief Interface entry containing the interface this class is derived from.
 * @param ifc The interface to return.
 */
#define SDV_INTERFACE_ENTRY(ifc)                                                                                                   \
            if (idInterface == sdv::GetInterfaceId<ifc>())                                                                         \
                return static_cast<ifc*>(this);

/**
 * @brief Interface entry containing the interface a member class is derived from.
 * @param ifc The interface to return. The member must derive from this interface.
 * @param member_or_pointer The interface of the member to return.
 */
#define SDV_INTERFACE_ENTRY_MEMBER(ifc, member_or_pointer)                                                                         \
            {                                                                                                                      \
                sdv::interface_t tifc = sdv::SInterfaceMemberHelper<ifc>::InterfaceMapEntryHelper(member_or_pointer, idInterface); \
                if (tifc)                                                                                                          \
                    return tifc;                                                                                                   \
            }

/**
 * @brief Interface entry containing the interface this class is derived from. The indirect implementation solves
 * ambiguous casts when the class is derived from the same interface more than once.
 * @param ifc The interface to return.
 * @param ifc_cast The interface to use for casting before casting to the interface. This is usually one of the
 * interfaces/classes derived from the interface
 */
#define SDV_INTERFACE_ENTRY_INDIRECT(ifc, ifc_cast)                                                                                \
            if (idInterface == sdv::GetInterfaceId<ifc>())                                                                         \
                return static_cast<ifc*>(static_cast<ifc_cast*>(this));

/**
 * @brief Interface entry containing the interface this class is definitely not supporting (used to overload an entry
 * in an interface map in a base class).
 * @attention When the interface map was chained, this will end the current processing for the chained map, but the
 * calling map will still continue the processing.
 * @param ifc The interface to deny.
 */
#define SDV_INTERFACE_DENY_ENTRY(ifc)                                                                                              \
            if (idInterface == sdv::GetInterfaceId<ifc>())                                                                         \
                return nullptr;

/**
 * @brief Chain the interface map of a base class.
 * @param base_class The class implementing the GetInterface function.
 */
#define SDV_INTERFACE_CHAIN_BASE(base_class)                                                                                       \
            {                                                                                                                      \
                sdv::interface_t ifc = base_class::GetInterface(idInterface);                                                      \
                if (ifc)                                                                                                           \
                    return ifc;                                                                                                    \
            }

/**
 * @brief Chain the interface map of a member as reference or as pointer.
 * @param member_or_pointer The member implementing the GetInterface function.
 */
#define SDV_INTERFACE_CHAIN_MEMBER(member_or_pointer)                                                                              \
            {                                                                                                                      \
                sdv::interface_t ifc = sdv::InterfaceMapChainHelper(member_or_pointer, idInterface);                               \
                if (ifc)                                                                                                           \
                    return ifc;                                                                                                    \
            }

/**
 * @brief Conditional check; when true continue the checking for interfaces.
 * @param condition The condition to be checked. When 'false' processing will be stopped; otherwise processing
 *  continues. Example: @code SDV_INTERFACE_CHECK_CONDITION(CheckFunc()) @endcode
 */
#define SDV_INTERFACE_CHECK_CONDITION(condition)                                                                                   \
            if (!(condition))                                                                                                      \
                return nullptr;

/**
 * @brief Use the condition, to select a section to process.
 * @param condition Condition to be checked. When 'true' processing will be limited to the section.
 * Example: @code SDV_INTERFACE_PROCESS_SECTION(CheckFunc(), 1) @endcode
 * @param section_number The section to be processed when the condition is true.
 */
#define SDV_INTERFACE_SET_SECTION_CONDITION(condition, section_number)                                                             \
            if (condition)                                                                                                         \
                selector.Select(section_number);                                                                                   \
        }                                                                                                                          \
        if (selector.Selected())                                                                                                   \
        {

/**
 * @brief Set section to process.
 * @param section_number The section to process.
 */
#define SDV_INTERFACE_SET_SECTION(section_number)                                                                                  \
            selector.Select(section_number);                                                                                       \
        }                                                                                                                          \
        if (selector.Selected())                                                                                                   \
        {


/**
 * @brief Define that the interface map entries following this statement are to be processed regardless of the section
 * selection.
 */
#define SDV_INTERFACE_DEFAULT_SECTION()                                                                                            \
        }                                                                                                                          \
        selector.DefineDefaultSection();                                                                                           \
        if (selector.Selected())                                                                                                   \
        {

/**
 * @brief Define that the interface map entries following this statement belong to a dedicated section and should only
 * be processed when that section is selected.
 * @param section_number Begin of a section with the section number.
 */
#define SDV_INTERFACE_SECTION(section_number)                                                                                      \
        }                                                                                                                          \
        selector.DefineSection(section_number);                                                                                    \
        if (selector.Selected())                                                                                                   \
        {


/**
 * @brief End of interface map.
 */
#define END_SDV_INTERFACE_MAP()                                                                                                    \
        }                                                                                                                          \
        return nullptr; /* None found */                                                                                           \
    }

// NOLINTEND(cppcoreguidelines-macro-usage)

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    /**
     * @brief Interface access wrapper class.
     * @tparam IInterfaceAccess Interface type to wrap
     */
    class CInterfacePtr
    {
    public:
        /**
         * @brief Construct a new CInterfacePtr object.
         * @param[in] pInterface Pointer to the interface to wrap or nullptr when no interface is to be wrapped yet.
         */
        CInterfacePtr(IInterfaceAccess* pInterface = nullptr) : m_pInterface(pInterface)
        {}

        /**
         * @brief Copy construct a new CInterfacePtr object
         * @param[in] rptrInterface Reference to the CInterfacePtr object to copy from.
         */
        CInterfacePtr(const CInterfacePtr& rptrInterface) : m_pInterface(rptrInterface.m_pInterface.load())
        {}

        /**
         * @brief Move construct a new CInterfacePtr object
         * @param[in] rptrInterface Reference to the CInterfacePtr object to move from.
         */
        CInterfacePtr(CInterfacePtr&& rptrInterface) noexcept
        {
            IInterfaceAccess* pInterface = rptrInterface.m_pInterface.load();
            rptrInterface.m_pInterface = nullptr;
            m_pInterface = pInterface;
        }

        /**
         * @brief Default destructor.
         */
        virtual ~CInterfacePtr() = default;

        /**
         * @brief Assignment operator
         * @param[in] pInterface Pointer to the interface to wrap or nullptr to clear the wrapping.
         * @return Returns *this
         */
        CInterfacePtr& operator=(IInterfaceAccess* pInterface)
        {
            m_pInterface = pInterface;
            return *this;
        }

        /**
         * @brief Assignment operator
         * @param[in] rptrInterface Reference to the CInterfacePtr object to copy from.
         * @return Returns *this
         */
        CInterfacePtr& operator=(const CInterfacePtr& rptrInterface)
        {
            m_pInterface = rptrInterface.m_pInterface.load();
            return *this;
        }

        /**
         * @brief Move operator
         * @param[in] rptrInterface Reference to the CInterfacePtr object to move from.
         * @return Returns *this
         */
        CInterfacePtr& operator=(CInterfacePtr&& rptrInterface) noexcept
        {
            IInterfaceAccess* pInterface = rptrInterface.m_pInterface.load();
            rptrInterface.m_pInterface = nullptr;
            m_pInterface = pInterface;
            return *this;
        }

        /**
         * @brief Get a pointer to the interface
         */
        operator IInterfaceAccess*()
        {
            return m_pInterface;
        }

        /**
         * @brief Get a pointer to the interface
         */
        operator const IInterfaceAccess*() const
        {
            return m_pInterface;
        }

        /**
         * @brief Is there a valid interface?
         * @return Returns 'true' when an interface is wrapped; 'false' otherwise.
         */
        operator bool() const
        {
            return static_cast<bool>(m_pInterface);
        }

        /**
         * @brief Return whether there is a valid interface.
         * @return Returns 'true' when an interface is wrapped; 'false' otherwise.
         */
        bool IsValid() const
        {
            return static_cast<bool>(m_pInterface);
        }

        /**
         * @brief Gets an interface by using the IInterfaceAccess::GetInterface function.
         * @tparam TIfc The interface type to request.
         * @return Returns a pointer to the requested interface if the object has itregistered, nullptr otherwise
         */
        template <typename TIfc>
        TIfc* GetInterface() const
        {
            return m_pInterface ? m_pInterface.load()->GetInterface(GetInterfaceId<TIfc>()).template get<TIfc>() : nullptr;
        }

    private:
        /**
         * @brief Contained pointer to the interface.
         */
        std::atomic<IInterfaceAccess*> m_pInterface{nullptr};
    };

    /**
     * @brief Specialization for sdv::IInterfaceAccess interface.
     * @tparam TIfc The interface type to request.
     * @return Returns a pointer to the requested interface if the object has itregistered, nullptr otherwise
     */
    template <>
    inline IInterfaceAccess* CInterfacePtr::GetInterface<IInterfaceAccess>() const
    {
        return m_pInterface;
    }

    /**
     * @brief IInterfaceAccess smart pointer.
     */
    using TInterfaceAccessPtr = CInterfacePtr;

    /**
     * @brief Helper function for interface chaining using a member variable.
     * @tparam TMember Type of the member variable.
     * @param[in] rtMember Reference to the member to chain to.
     * @param[in] idInterface ID of the interface to request.
     * @return Pointer to the interface.
     */
    template <typename TMember>
    inline sdv::interface_t InterfaceMapChainHelper(TMember& rtMember, interface_id idInterface)
    {
        return rtMember.GetInterface(idInterface);
    }

    // Warning of cppchgeck for a potential const variable cannot be applied due to the non-const nature of interfaces. Suppress
    // warning.
    // cppcheck-suppress constParameterReference
    /**
     * @brief Helper function for interface chaining using a member variable.
     * @tparam IInterfaceAccess Type of the interface wrapped by the member variable.
     * @param[in] rptrMember Reference to the interface wrapper class to chain to.
     * @param[in] idInterface ID of the interface to request.
     * @return Pointer to the interface.
     */
    inline sdv::interface_t InterfaceMapChainHelper(CInterfacePtr& rptrMember, interface_id idInterface)
    {
        return rptrMember ? static_cast<IInterfaceAccess*>(rptrMember)->GetInterface(idInterface) : nullptr;
    }

    /**
     * @brief Helper function for interface chaining using a member variable.
     * @tparam TMember Type of the member variable.
     * @param[in] ptMember Pointer to the member to chain to.
     * @param[in] idInterface ID of the interface.
     * @return Pointer to the interface.
     */
    template <typename TMember>
    inline sdv::interface_t InterfaceMapChainHelper(TMember* ptMember, interface_id idInterface)
    {
        return ptMember ? ptMember->GetInterface(idInterface) : nullptr;
    }

    /**
     * @brief Helper function for interface chaining using a member variable.
     * @tparam TMember Type of the member variable.
     * @param[in] rptrMember Reference to the shared pointer to the member to chain to.
     * @param[in] idInterface ID of the interface.
     * @return Pointer to the interface.
     */
    template <typename TMember>
    inline sdv::interface_t InterfaceMapChainHelper(std::shared_ptr<TMember>& rptrMember, interface_id idInterface)
    {
        return rptrMember ? rptrMember->GetInterface(idInterface) : nullptr;
    }

    /**
     * @brief Helper function for interface chaining using a member variable.
     * @tparam TMember Type of the member variable.
     * @param[in] rweakMember Reference to the weak pointer to the member to chain to.
     * @param[in] idInterface ID of the interface.
     * @return Pointer to the interface.
     */
    template <typename TMember>
    inline sdv::interface_t InterfaceMapChainHelper(std::weak_ptr<TMember>& rweakMember, interface_id idInterface)
    {
        std::shared_ptr<TMember> ptrMember = rweakMember.lock();
        return InterfaceMapChainHelper(ptrMember, idInterface);
    }

    /**
     * @brief Interface member helper class.
     * @tparam TInterface The interface the member is to implement.
    */
    template <typename TInterface>
    struct SInterfaceMemberHelper
    {
        /**
         * @brief Helper function for interface entries in the interface map.
         * @tparam TMember Type of the member variable.
         * @param[in] rtMember Reference to the member implementing the interface.
         * @param[in] idInterface ID of the interface.
         * @return Pointer to the interface.
         */
        template <typename TMember>
        static sdv::interface_t InterfaceMapEntryHelper(TMember& rtMember, interface_id idInterface)
        {
            static_assert(std::is_same_v<TInterface, TMember> || std::is_base_of_v<TInterface, TMember>);
            if (sdv::GetInterfaceId<TInterface>() == idInterface)
            {
                return static_cast<TInterface*>(&rtMember);
            }
            return nullptr;
        }

        /**
         * @brief Helper function for interface entries in the interface map.
         * @tparam TMember Type of the member variable.
         * @param[in] ptMember Pointer to the member implementing the interface.
         * @param[in] idInterface ID of the interface.
         * @return Pointer to the interface.
         */
        template <typename TMember>
        static sdv::interface_t InterfaceMapEntryHelper(TMember* ptMember, interface_id idInterface)
        {
            static_assert(std::is_same_v<TInterface, TMember> || std::is_base_of_v<TInterface, TMember>);
            if (sdv::GetInterfaceId<TInterface>() == idInterface)
            {
                return static_cast<TInterface*>(ptMember);
            }
            return nullptr;
        }

        /**
         * @brief Helper function for interface entries in the interface map.
         * @tparam TMember Type of the member variable.
         * @param[in] rptrMember Reference to the shared pointer to the member implementing the interface.
         * @param[in] idInterface ID of the interface.
         * @return Pointer to the interface.
         */
        template <typename TMember>
        static sdv::interface_t InterfaceMapEntryHelper(std::shared_ptr<TMember>& rptrMember, interface_id idInterface)
        {
            static_assert(std::is_same_v<TInterface, TMember> || std::is_base_of_v<TInterface, TMember>);
            if (sdv::GetInterfaceId<TInterface>() == idInterface)
            {
                return static_cast<TInterface*>(rptrMember.get());
            }
            return nullptr;
        }

        /**
         * @brief Helper function for interface entries in the interface map.
         * @tparam TMember Type of the member variable.
         * @param[in] rweakMember Reference to the weak pointer to the member implementing the interface.
         * @param[in] idInterface ID of the interface.
         * @return Pointer to the interface.
         */
        template <typename TMember>
        static sdv::interface_t InterfaceMapEntryHelper(std::weak_ptr<TMember>& rweakMember, interface_id idInterface)
        {
            static_assert(std::is_same_v<TInterface, TMember> || std::is_base_of_v<TInterface, TMember>);
            std::shared_ptr<TMember> ptrMember = rweakMember.lock();
            return InterfaceMapEntryHelper<TMember>(ptrMember, idInterface);
        }
    };

    /**
     * @brief Helper class to implement the IObjectLifetime interface for objects not supporting this.
     */
    class CObjectLifetimeWrapper : public IInterfaceAccess, public IObjectLifetime, public IObjectDestroy
    {
    public:
        /**
         * @brief Default constructor
         */
        CObjectLifetimeWrapper() = default;

        /**
         * @brief Destructor automatically destroying the object.
         */
        ~CObjectLifetimeWrapper()
        {}

        // Interface map
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_ENTRY(IObjectLifetime)
            SDV_INTERFACE_ENTRY(IObjectDestroy)
            SDV_INTERFACE_CHAIN_MEMBER(m_pObject)
        END_SDV_INTERFACE_MAP()

        /**
         * @brief Create a object lifetime wrapper object.
         * @details Create an object lifetime wrapper object and assign the provided interface to the wrapper. The wrapper object
         * blocks calls to IObjectDestroy since it will take ober the management of object lifetime.
         * @param[in] pObject Interface of the object to wrap. This object needs to expose IObjectDestroy and could expose
         * IObjectLifetime.
         * @return Returns an interface giving access to a IObjectLifetime interface, forwarding all other interface calls.
         */
        static IInterfaceAccess* CreateWrapper(IInterfaceAccess* pObject)
        {
            if (!pObject) return nullptr;   // Nothing to manage
            std::unique_ptr<CObjectLifetimeWrapper> ptrWrapper = std::make_unique<CObjectLifetimeWrapper>();
            // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
            // exception was triggered).
            // cppcheck-suppress knownConditionTrueFalse
            if (!ptrWrapper)
                return nullptr;
            ptrWrapper->Increment();
            ptrWrapper->m_pObject = pObject;

            // Store the pointer internally - this will keep the pointer alive, but invalidates ptrWrapper.
            std::unique_ptr<CObjectLifetimeWrapper>& rptrKeepAlive = ptrWrapper->m_ptrKeepAlive;
            rptrKeepAlive = std::move(ptrWrapper);

            // Return the interface pointer
            return rptrKeepAlive.get();
        }

        /**
         * @brief Increment the lifetime. Needs to be balanced by a call to Decrement. Overload of IObjectLifetime::Increment.
         */
        virtual void Increment() override
        {
            m_iCounter++;
        }

        /**
         * @brief Decrement the lifetime. If the lifetime reaches zero, the object will be destroyed (through the exposed
         * IObjectDestroy interface). Overload of IObjectLifetime::Decrement.
         * @return Returns 'true' if the object was destroyed, false if not.
         */
        virtual bool Decrement() override
        {
            if (!m_iCounter) return false;
            m_iCounter--;
            if (!m_iCounter)
            {
                IObjectDestroy* pObjectDestroy = CInterfacePtr(m_pObject).GetInterface<IObjectDestroy>();
                if (pObjectDestroy) pObjectDestroy->DestroyObject();
                m_pObject = nullptr;
                m_ptrKeepAlive.reset();
                return true;
            }
            return false;
        }

        /**
         * Get the current lifetime count. Overload of IObjectLifetime::GetCount.
         * @remarks The GetCount function returns a momentary value, which can be changed at any moment.
         * @return Returns the current counter value.
         */
        virtual uint32_t GetCount() const override
        {
            return static_cast<uint32_t>(m_iCounter);
        }

        /**
         * @brief Destroy the object. Overload of IObjectDestroy::DestroyObject.
         * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
         */
        virtual void DestroyObject() override
        {
            // Destroy object is only allowed when the counter is 1.
            if (static_cast<int32_t>(m_iCounter) != 1)
                std::cerr << "Trying to destroy an object having references." << std::endl;
            if (static_cast<int32_t>(m_iCounter) > 0)
                Decrement();
        }

        IInterfaceAccess*                       m_pObject = nullptr;    ///< The interface pointer.
        std::atomic<int32_t>                    m_iCounter = 0;         ///< The lifetime counter.
        std::unique_ptr<CObjectLifetimeWrapper> m_ptrKeepAlive;         ///< The one instance of this wrapper class.
    };

    /**
     * @brief Object smart pointer. Implements destruction of the object.
     */
    class CObjectPtr
    {
    public:
        /**
         * @brief Construct a new CObjectPtr object.
         * @param[in] pInterface Pointer to the interface to wrap or nullptr when no interface is to be wrapped yet.
         */
        CObjectPtr(IInterfaceAccess* pInterface = nullptr)
        {
            Assign(pInterface);
        }

        /**
         * @brief Copy construct a new CInterfacePtr object
         * @param[in] rptrInterface Reference to the CInterfacePtr object to copy from.
         */
        CObjectPtr(const CObjectPtr& rptrInterface) : m_ptrObject(rptrInterface.m_ptrObject)
        {
            IObjectLifetime* pObjectLifetime = GetInterface<IObjectLifetime>();
            if (pObjectLifetime) pObjectLifetime->Increment();
        }

        /**
         * @brief Move construct a new CInterfacePtr object
         * @param[in] rptrInterface Reference to the CInterfacePtr object to move from.
         */
        CObjectPtr(CObjectPtr&& rptrInterface) noexcept : m_ptrObject(std::move(rptrInterface.m_ptrObject))
        {}

        /**
         * @brief Destructor.
         */
        virtual ~CObjectPtr()
        {
            Clear();
        }

        /**
         * @brief Assignment operator
         * @param[in] pInterface Pointer to the interface to wrap or nullptr to clear the wrapping.
         * @return Returns *this
         */
        CObjectPtr& operator=(IInterfaceAccess* pInterface)
        {
            Assign(pInterface);
            return *this;
        }

        /**
         * @brief Assignment operator
         * @param[in] rptrInterface Reference to the CInterfacePtr object to copy from.
         * @return Returns *this
         */
        CObjectPtr& operator=(const CObjectPtr& rptrInterface)
        {
            Clear();
            m_ptrObject = rptrInterface.m_ptrObject;
            IObjectLifetime* pObjectLifetime = GetInterface<IObjectLifetime>();
            if (pObjectLifetime) pObjectLifetime->Increment();
            return *this;
        }

        /**
         * @brief Move operator
         * @param[in] rptrInterface Reference to the CInterfacePtr object to move from.
         * @return Returns *this
         */
        CObjectPtr& operator=(CObjectPtr&& rptrInterface) noexcept
        {
            Attach(rptrInterface.Detach());
            return *this;
        }

        /**
         * @brief Get a pointer to the interface
         */
        operator IInterfaceAccess*()
        {
            return m_ptrObject;
        }

        /**
         * @brief Get a pointer to the interface
         */
        operator const IInterfaceAccess*() const
        {
            return m_ptrObject;
        }

        /**
         * @brief Get a pointer to the interface
         */
        operator CInterfacePtr() const
        {
            return m_ptrObject;
        }

        /**
         * @brief Is there a valid interface?
         * @return Returns 'true' when an interface is wrapped; 'false' otherwise.
         */
        operator bool() const
        {
            return IsValid();
        }

        /**
         * @brief Return whether there is a valid interface.
         * @return Returns 'true' when an interface is wrapped; 'false' otherwise.
         */
        bool IsValid() const
        {
            return static_cast<bool>(m_ptrObject);
        }

        /**
         * @brief Gets an interface by using the IInterfaceAccess::GetInterface function.
         * @tparam TIfc The interface type to request.
         * @return Returns a pointer to the requested interface if the object has itregistered, nullptr otherwise
         */
        template <typename TIfc>
        TIfc* GetInterface() const
        {
            return m_ptrObject.GetInterface<TIfc>();
        }

        /**
         * @brief Assign the object to this object pointer and increment the lifetime counter.
         * @param[in] pObject Interface pointer to the object.
         */
        void Assign(IInterfaceAccess* pObject)
        {
            Clear();
            if (!pObject) return;

            IObjectLifetime* pLifetime = CInterfacePtr(pObject).GetInterface<IObjectLifetime>();
            if (pLifetime)  // Object provides lifetime management. Use this...
            {
                m_ptrObject = pObject;
                pLifetime->Increment();
            }
            else // Create a lifetime management wrapper. Do not increase count... this has already been done during creation
                m_ptrObject = CObjectLifetimeWrapper::CreateWrapper(pObject);
        }

        /**
         * @brief Destroy the object and clear the interface.
         */
        void Clear()
        {
            IObjectLifetime* pLifetime = m_ptrObject.GetInterface<IObjectLifetime>();
            if (pLifetime)  // Object provides lifetime management. Use this...
                pLifetime->Decrement();
            m_ptrObject = nullptr;
        }

        /**
         * @brief Attach an object which is managed by lifetime. The lifetime counter is not increased.
         * @remarks Attachment only succeeds when the object exposes the IObjectLifetime pointer.
         * @param[in] pObject Pointer to the object.
         */
        void Attach(IInterfaceAccess* pObject)
        {
            Clear();
            if (CInterfacePtr(pObject).GetInterface<IObjectLifetime>())
                m_ptrObject = pObject;
        }

        /**
         * @brief Detach the pointer without change the object lifetime. The internal pointer will beeome NULL.
         * @attention Detaching the pointer means it is not managed any more. A call to IObjectLifetime::Decrement needs to be done
         * manually.
         * @return The detached interface pointer.
         */
        CInterfacePtr Detach()
        {
            if (!m_ptrObject) return CInterfacePtr();
            CInterfacePtr ptr = std::move(m_ptrObject);
            m_ptrObject = nullptr;
            return ptr;
        }

    private:
        CInterfacePtr   m_ptrObject;    ///< The object pointer.
    };

    /**
     * @brief IObjectAccess smart pointer.
     */
    using TObjectPtr = CObjectPtr;

} // namespace sdv


#ifndef DONT_LOAD_CORE_TYPES
#include "mem_access.h"
#endif

#endif // !defined INTERFACE_IMPL_H
