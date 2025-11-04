#include <iostream>
#include "autoheadlight_cs.h"


CAutoHeadlightService::CAutoHeadlightService()
{
}

CAutoHeadlightService::~CAutoHeadlightService()
{
    Shutdown();
}

void CAutoHeadlightService::Initialize(const sdv::u8string& ssObjectConfig)
{
    m_eStatus = sdv::EObjectStatus::initializing;

    // Request the basic service for the headlight.
    m_pHeadlightSvc = sdv::core::GetObject("Vehicle.Body.Light.Front.LowBeam_Service").GetInterface<vss::Vehicle::Body::Light::Front::LowBeamService::IVSS_SetHeadLightLowBeam>();
    if (!m_pHeadlightSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetHeadlightLowBeam': [CAutoHeadlightService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Request the basic service for the steering wheel.
    auto pCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (!pCurrentLatitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLatitude': [CAutoHeadlightService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Request the basic service for the vehicle speed.
    auto pCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (!pCurrentLongitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLongitude': [CAutoHeadlightService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Register Current Latitude change event handler.
    pCurrentLatitudeSvc->RegisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    // Register Current Longitude change event handler.
    pCurrentLongitudeSvc->RegisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this));

 
    if(LoadGPSBounds(ssObjectConfig))
    {
        SDV_LOG_INFO("AutoHeadlightService: GPS bounds loaded Successfully");
        m_eStatus = sdv::EObjectStatus::initialized;
    }
    else
    {
        SDV_LOG_ERROR("AutoHeadlightService: GPS bounds could not be loaded");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
   
    SDV_LOG_INFO("AutoHeadlightService: Initialized Successfully");
}

sdv::EObjectStatus CAutoHeadlightService::GetStatus() const
{
    return m_eStatus;
}

void CAutoHeadlightService::SetOperationMode(sdv::EOperationMode /*eMode*/)
{
    // Not applicable
}

void CAutoHeadlightService::Shutdown()
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


bool CAutoHeadlightService::LoadGPSBounds(const sdv::u8string& rssObjectConfig)
{
    try
    {
        sdv::toml::CTOMLParser config(rssObjectConfig.c_str());

        sdv::toml::CNode fStartLatNode = config.GetDirect("tunnel_start_lat");
        float  fTunnelStartLat = 0.0;     ///< Tunnel Start Latitude
        if (fStartLatNode.GetType() == sdv::toml::ENodeType::node_floating_point)
        {
            fTunnelStartLat = static_cast<float>(fStartLatNode.GetValue());
        }
    
        sdv::toml::CNode fStartLonNode = config.GetDirect("tunnel_start_lon");
        float  fTunnelStartLon = 0.0;     ///< Tunnel Start Longitude
        if (fStartLonNode.GetType() == sdv::toml::ENodeType::node_floating_point)
        {
            fTunnelStartLon = static_cast<float>(fStartLonNode.GetValue());
        }

        sdv::toml::CNode fEndLatNode = config.GetDirect("tunnel_end_lat");
        float  fTunnelEndLat = 0.0;       ///< Tunnel End Latitude
        if (fEndLatNode.GetType() == sdv::toml::ENodeType::node_floating_point)
        {
            fTunnelEndLat = static_cast<float>(fEndLatNode.GetValue());
        }

        sdv::toml::CNode fEndLonNode = config.GetDirect("tunnel_end_lon");
        float  fTunnelEndLon = 0.0;       ///< Tunnel End Longitude
        if (fEndLonNode.GetType() == sdv::toml::ENodeType::node_floating_point)
        {
            fTunnelEndLon = static_cast<float>(fEndLonNode.GetValue());
        }

        // Calculate bounding box
        m_SGPSBoundingBox.fTunnelMinLat = std::min(fTunnelStartLat, fTunnelEndLat);
        m_SGPSBoundingBox.fTunnelMaxLat = std::max(fTunnelStartLat, fTunnelEndLat);
        m_SGPSBoundingBox.fTunnelMinLon = std::min(fTunnelStartLon, fTunnelEndLon);
        m_SGPSBoundingBox.fTunnelMaxLon = std::max(fTunnelStartLon, fTunnelEndLon);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        SDV_LOG_ERROR("Parsing error: ", e.what());
        return false;
    }

    return true;
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
