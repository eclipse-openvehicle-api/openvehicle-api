#ifndef OBJECT_LIFETIME_CONTROL_H
#define OBJECT_LIFETIME_CONTROL_H

#include <support/interface_ptr.h>

/**
 * @brief Object destroy handler - callback interface
 */
interface IObjectDestroyHandler
{
    /**
     * @brief Inform about an object destruction. This function might delete the calling class, so no other function must be
     * called after the call to this function.
     * @param[in] pObject Pointer to the object being destroyed.
     */
    virtual void OnDestroyObject(sdv::IInterfaceAccess* pObject) = 0;
};

/**
 * @brief Wrapper class around the object to allow objects to be destroyed through sdv::IObjectDestroy.
 */
class CObjectLifetimeControl : public sdv::IInterfaceAccess, public sdv::IObjectDestroy, public sdv::IObjectLifetime
{
public:
    /**
     * @brief Constructor assigning the object instance.
     * @param[in] pObject Interface pointer to the object instance. Must not be nullptr.
     * @param[in] rHandler Reference to the handler being informed about the object destruction.
     * @param[in] bAutoDestroy When set, the wrapper class will be destroyed automatically when the object is destroyed.
     */
    CObjectLifetimeControl(sdv::IInterfaceAccess* pObject, IObjectDestroyHandler& rHandler, bool bAutoDestroy = true);

    /**
     * @brief Virtual destructor
     */
    virtual ~CObjectLifetimeControl() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_SET_SECTION_CONDITION(!m_pObjectLifetime, 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_SET_SECTION_CONDITION(m_pObjectLifetime, 2)
        SDV_INTERFACE_SECTION(2)
        SDV_INTERFACE_ENTRY(sdv::IObjectLifetime)
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_DENY_ENTRY(sdv::IObjectControl)   // Do not allow users to directly access the object control.
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrObject)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
     */
    virtual void DestroyObject() override;

    /**
     * @brief Increment the lifetime. Needs to be balanced by a call to Decrement. Overload of sdv::IObjectLifetime::Increment.
     */
    virtual void Increment() override;

    /**
     * @brief Decrement the lifetime. If the lifetime reaches zero, the object will be destroyed (through the exposed
     * IObjectDestroy interface). Overload of sdv::IObjectLifetime::Decrement.
     * @return Returns 'true' if the object was destroyed, false if not.
     */
    virtual bool Decrement() override;

    /**
     * @brief Get the current lifetime count. Overload of sdv::IObjectLifetime::GetCount.
     * @remarks The GetCount function returns a momentary value, which can be changed at any moment.
     * @return Returns the current counter value.
     */
    virtual uint32_t GetCount() const override;

private:
    IObjectDestroyHandler&      m_rHandler;                     ///< Reference to the handler class being informed about the
                                                                ///< destruction.
    sdv::IObjectDestroy*        m_pObjectDestroy = nullptr;     ///< IObjectDestroy interface is exposed by object
    sdv::IObjectLifetime*       m_pObjectLifetime = nullptr;    ///< IObjectLifetime interface is exposed by object
    sdv::TInterfaceAccessPtr    m_ptrObject;                    ///< Interface pointer to the object.
    bool                        m_bAutoDestroy = false;         ///< When set, the lifetime wrapper object is destroyed after an
                                                                ///< object destroy call.
};

#endif // !defined OBJECT_LIFETIME_CONTROL_H