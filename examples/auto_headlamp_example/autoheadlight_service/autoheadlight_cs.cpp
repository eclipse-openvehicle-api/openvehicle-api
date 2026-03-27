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
#include "autoheadlight_cs.h"


CAutoHeadlightService::CAutoHeadlightService()
{}

CAutoHeadlightService::~CAutoHeadlightService()
{}

bool CAutoHeadlightService::OnInitialize()
{
    // Request the basic service for the headlight.
    m_pHeadlightSvc = sdv::core::GetObject("Vehicle.Body.Light.Front.LowBeam_Service").GetInterface<vss::Vehicle::Body::Light::Front::LowBeamService::IVSS_SetHeadLightLowBeam>();
    if (!m_pHeadlightSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetHeadlightLowBeam': [CAutoHeadlightService]");
        return false;
    }

    // Request the basic service for the steering wheel.
    auto pCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (!pCurrentLatitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLatitude': [CAutoHeadlightService]");
        return false;
    }

    // Request the basic service for the vehicle speed.
    auto pCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (!pCurrentLongitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLongitude': [CAutoHeadlightService]");
        return false;
    }

    // Register Current Latitude change event handler.
    pCurrentLatitudeSvc->RegisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    // Register Current Longitude change event handler.
    pCurrentLongitudeSvc->RegisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this));

 
    // Swap the bounding box the make certain that min is less than max
    if (m_SGPSBoundingBox.fTunnelMinLat > m_SGPSBoundingBox.fTunnelMaxLat)
        std::swap(m_SGPSBoundingBox.fTunnelMinLat, m_SGPSBoundingBox.fTunnelMaxLat);
    if (m_SGPSBoundingBox.fTunnelMinLon > m_SGPSBoundingBox.fTunnelMaxLon)
        std::swap(m_SGPSBoundingBox.fTunnelMinLon, m_SGPSBoundingBox.fTunnelMaxLon);

    SDV_LOG_INFO("AutoHeadlightService: Initialized Successfully");
    return true;
}

void CAutoHeadlightService::OnShutdown()
{
    // Unregister the Current latitude event handler.
    auto pCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (pCurrentLatitudeSvc)
        pCurrentLatitudeSvc->UnregisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    // Unregister the vehicle speed event handler.
    auto pCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (pCurrentLongitudeSvc)
        pCurrentLongitudeSvc->UnregisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this));
}

void CAutoHeadlightService::SetCurrentLatitude(float value)
{
    if (m_fCurrentLatitude == value) 
        return;

    m_fCurrentLatitude = value;
    ProcessHeadlightBasedOnEgoPosition();
}

void CAutoHeadlightService::SetCurrentLongitude(float value)
{
    if (m_fCurrentLongitude == value) 
        return;
        
    m_fCurrentLongitude = value;
    ProcessHeadlightBasedOnEgoPosition();
}

bool CAutoHeadlightService::IsinTunnel() const
{
    // Check if vehicle is within the tunnel bounds
    if (m_fCurrentLatitude >= m_SGPSBoundingBox.fTunnelMinLat && m_fCurrentLatitude <= m_SGPSBoundingBox.fTunnelMaxLat &&
        m_fCurrentLongitude >= m_SGPSBoundingBox.fTunnelMinLon && m_fCurrentLongitude <= m_SGPSBoundingBox.fTunnelMaxLon) 
    {
        return true;
    } 

    return false;
}

bool CAutoHeadlightService::GetHeadlightStatus() const
{
    return m_bHeadlight;
}

void CAutoHeadlightService::ProcessHeadlightBasedOnEgoPosition()
{    
    auto isInTunnel = IsinTunnel();
    if (isInTunnel && !m_bHeadlight)
    {
        // switch on headlight
        m_bHeadlight =  true;
        m_pHeadlightSvc->SetHeadLightLowBeam(m_bHeadlight);
    } 
    if (!IsinTunnel() && m_bHeadlight)
    {
        // switch off headlight
        m_bHeadlight = false;
        m_pHeadlightSvc->SetHeadLightLowBeam(m_bHeadlight);
    }
}

IAutoheadlightService::SGPSBoundBox CAutoHeadlightService::GetGPSBoundBox() const
{
    SGPSBoundBox tunnel;
    tunnel.fTunnelMinLat = m_SGPSBoundingBox.fTunnelMinLat;
    tunnel.fTunnelMaxLat = m_SGPSBoundingBox.fTunnelMaxLat;
    tunnel.fTunnelMinLon = m_SGPSBoundingBox.fTunnelMinLon;
    tunnel.fTunnelMaxLon = m_SGPSBoundingBox.fTunnelMaxLon;

    return tunnel;
}
