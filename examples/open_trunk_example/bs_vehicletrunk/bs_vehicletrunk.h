 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#ifndef __VSS_GENERATED__BS_VEHICLETRUNK_H_20260415_164751_648__
#define __VSS_GENERATED__BS_VEHICLETRUNK_H_20260415_164751_648__

#include <iostream>
#include <support/component_impl.h>
#include "../generated/vss_files/vss_vehiclebodytrunk_vd_tx.h"
#include "../generated/vss_files/vss_vehiclebodytrunk_bs_tx.h"
#include "../generated/vss_files/vss_vehiclespeed_bs_rx.h"

/**
 * @brief basic service VehicleTrunk
 */
class CBasicServiceVehicleTrunk
	: public sdv::CSdvObject
	, public vss::Vehicle::Body::TrunkService::IVSS_SetOpen
    , public vss::Vehicle::SpeedService::IVSS_SetSpeed_Event	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(vss::Vehicle::Body::TrunkService::IVSS_SetOpen)
        SDV_INTERFACE_ENTRY(vss::Vehicle::SpeedService::IVSS_SetSpeed_Event)		
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::actuator)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Body.Trunk_Service")

	/**
	* @brief Constructor
	*/
	CBasicServiceVehicleTrunk();
	
	/**
	 * @brief Set trunk signal
	 * @param[in] value 
     * @return true on success otherwise false
	 */
	bool SetOpen(bool value) override;

    /**
     * @brief Set vehicleSpeed signal
     * @param[in] value vehicle speed in km/h
     */
    virtual void SetSpeed(float value) override;	

private:
    float m_Speed = 1.0;   ///< Speed, start with representing a moveing vehicle.
	vss::Vehicle::Body::TrunkDevice::IVSS_WriteOpen* m_ptrOpen = nullptr;  ///< Trunk	
};

DEFINE_SDV_OBJECT(CBasicServiceVehicleTrunk)

#endif // !define __VSS_GENERATED__BS_VEHICLETRUNK_H_20260415_164751_648__

