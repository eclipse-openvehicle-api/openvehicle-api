#include <iostream>
#include "complex_service.h"


void CTrunkExampleService::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    m_eStatus = sdv::EObjectStatus::initializing;

    // Request the basic service for speed. 
    auto pSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (pSpeedSvc)
    {
        // Register speed change event handler.
        pSpeedSvc->RegisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
    }

    // Request the basic service for opening the trunk
    m_pTrunkSvc = sdv::core::GetObject("Vehicle.Body.Trunk_Service").GetInterface<vss::Vehicle::Body::TrunkService::IVSS_SetOpen>();

    if ((!pSpeedSvc) || (!m_pTrunkSvc))
    {
        SDV_LOG_ERROR("Could not get interfaces : [CTrunkExampleService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    m_eStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CTrunkExampleService::GetStatus() const
{
    return m_eStatus;
}

void CTrunkExampleService::SetOperationMode(sdv::EOperationMode /*eMode*/)
{
    // Not applicable
}

void CTrunkExampleService::Shutdown()
{
    // Unregister trunk change event handler.
    auto pSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (pSpeedSvc)
        pSpeedSvc->UnregisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
}

void CTrunkExampleService::SetSpeed(float value)
{
    m_Speed = value;
}

bool CTrunkExampleService::PopTrunk()
{
    if (m_Speed == 0.0)
    {
        return m_pTrunkSvc->SetOpen(true);
    }

    return false;
}

