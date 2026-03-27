 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

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
 * @brief Open trunk example service
 * @details This complex service checks the speed of the vehicle. in case the vehicle is oving, it prohibits the opening of the trunk.
 * Input events from basic service: vehicle speed in km/h
 * Output call for basic service:	opening the trunk
 */
class CTrunkExampleService : public sdv::CSdvObject
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
    {}

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(vss::Vehicle::SpeedService::IVSS_SetSpeed_Event)
        SDV_INTERFACE_ENTRY(ITrunkKitService)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_function)
    DECLARE_OBJECT_CLASS_NAME("Open Trunk Service")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

    /**
     * @brief Set vehicleSpeed signal
     * @param[in] value vehicle speed in km/h
     */
    virtual void SetSpeed(float value) override;

    /**
     * @brief Save call to open the trunk. Opening the trunk is only allowed when the vehicle is not moving
     * @return Returns whether the trunk could be opened or not.
     */
    virtual bool PopTrunk() override;

private:
    float	            m_Speed = 0.0;                                          ///< Speed 
    vss::Vehicle::Body::TrunkService::IVSS_SetOpen* m_pTrunkSvc = nullptr;      ///< Trunk
};

DEFINE_SDV_OBJECT(CTrunkExampleService)

#endif // !define TRUNK_COMPLEX_SERVICE_EXAMPLE_H
