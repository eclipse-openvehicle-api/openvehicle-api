#ifndef TRUNK_COMPLEX_SERVICE_EXAMPLE_H
#define TRUNK_COMPLEX_SERVICE_EXAMPLE_H

#include <iostream>

// SDV framework support
#include <support/component_impl.h>
#include <support/signal_support.h>

// VSS interfaces - located in ../interfaces
#include "../generated/vss_files/vss_vehiclespeed_bs_rx.h"
#include "../generated/vss_files/vss_vehiclebodytrunk_bs_tx.h"

// Complex service trunk interface - located in ../generated/trunk_service
#include "trunkkit.h"


/**
 * @brief Open trunk service: opens the trunk if vehicle is not moving
 */
class CTrunkExampleService : public sdv::CSdvObject
    , public sdv::IObjectControl
    , public vss::Vehicle::SpeedService::IVSS_SetSpeed_Event
    , public ITrunkKitService
{
public:
    /**
     * @brief Constructor
     */
    CTrunkExampleService(){}

    /**
     * @brief Destructor
     */
    ~CTrunkExampleService()
    {
        // Just in case...
        Shutdown();
    }

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(vss::Vehicle::SpeedService::IVSS_SetSpeed_Event)
        SDV_INTERFACE_ENTRY(ITrunkKitService)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("Open Trunk Service")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Set vehicleSpeed signal
     * @param[in] value vehicleSpeed
     */
    virtual void SetSpeed(float value) override;

    /**
     * @brief Save call to open the trunk. Opening the trunk is onkly allowed when the vehicle is not moving
     * @return Returns whether the trunk could be opened or not.
     */
    virtual bool PopTrunk() override;

private:

    sdv::EObjectStatus  m_eStatus = sdv::EObjectStatus::initialization_pending; ///< Current object status

    float	            m_Speed = 0.0;                                          ///< Speed 
    vss::Vehicle::Body::TrunkService::IVSS_SetOpen* m_pTrunkSvc = nullptr;      ///< Trunk
};

DEFINE_SDV_OBJECT(CTrunkExampleService)

#endif // !define TRUNK_COMPLEX_SERVICE_EXAMPLE_H
