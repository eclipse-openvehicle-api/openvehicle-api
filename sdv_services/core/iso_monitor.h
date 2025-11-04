#ifndef ISOLATION_OBJECT_MONITOR_H
#define ISOLATION_OBJECT_MONITOR_H

#include <support/component_impl.h>

/**
 * @brief Isolation object monitor. If shutdown is called, the application leaves the running loop.
 * @remarks Only valid when running in an isolated application.
 */
class CIsoMonitor : public sdv::IInterfaceAccess, public sdv::IObjectControl
{
public:
    /**
     * @brief Constructor
     * @param[in] pObject Pointer to the object to monitor
     */
    CIsoMonitor(sdv::IInterfaceAccess* pObject);

    /**
     * @brief Destructor
     */
    ~CIsoMonitor();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrObject)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(/*in*/ const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    virtual void SetOperationMode(/*in*/ sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown() override;

    /**
     * @brief Get the contained object from the isolation monitor.
     * @return The contained object.
     */
    sdv::IInterfaceAccess* GetContainedObject();

private:
    sdv::TInterfaceAccessPtr        m_ptrObject;                    ///< Smart pointer to the object.
    sdv::IObjectControl*            m_pObjectControl = nullptr;     ///< Pointer to the object control of the application
    sdv::EObjectStatus              m_eObjectStatus = sdv::EObjectStatus::initialization_pending; ///< Object status (in case there is no object control).
};

#endif // !defined ISOLATION_OBJECT_MONITOR_H