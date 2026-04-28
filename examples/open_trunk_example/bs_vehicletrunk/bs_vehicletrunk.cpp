 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include <iostream>
#include "bs_vehicletrunk.h"

namespace
{
constexpr float kMovingSpeedThresholdKmh = 0.0F;
}

/**
 * @brief Constructor
 */
CBasicServiceVehicleTrunk::CBasicServiceVehicleTrunk()
{
	m_ptrOpen = sdv::core::GetObject("Vehicle.Body.Trunk_Device").GetInterface<vss::Vehicle::Body::TrunkDevice::IVSS_WriteOpen>();
	if (!m_ptrOpen)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_WriteOpen': [CBasicServiceVehicleTrunk]");
		throw std::runtime_error("Open device not found");
	}

    // Request the basic service for speed. 
    auto pSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (pSpeedSvc)
    {
        // Register speed change event handler.
        pSpeedSvc->RegisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
    }

	SDV_LOG_TRACE("CBasicServiceVehicleTrunk created.");
}

/**
 * @brief Open
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicServiceVehicleTrunk::SetOpen(bool value)
{
	// Block only opening requests while vehicle is moving; closing must always be allowed.
	if (value && (m_Speed > kMovingSpeedThresholdKmh))
		return false;
	return m_ptrOpen->WriteOpen(value);
}

void CBasicServiceVehicleTrunk::SetSpeed(float value)
{
    m_Speed = value;

    // Safety fallback: if vehicle starts moving, force trunk to closed state.
    if (m_Speed > kMovingSpeedThresholdKmh)
    {
        (void)m_ptrOpen->WriteOpen(false);
    }
}
