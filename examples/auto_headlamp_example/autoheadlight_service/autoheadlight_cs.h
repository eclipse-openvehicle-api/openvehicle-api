#ifndef COMPLEX_SERVICE_EXAMPLE_H
#define COMPLEX_SERVICE_EXAMPLE_H

// C++ library
#include <iostream>


// SDV framework support
#include <support/component_impl.h>
#include <support/signal_support.h>
#include <support/timer.h>
#include <support/toml.h>

// VSS interfaces - located in ../generated/vss_files/include
#include "vss_vehiclepositioncurrentlatitude_bs_rx.h"
#include "vss_vehiclepositioncurrentlongitude_bs_rx.h"
#include "vss_vehiclebodylightfrontlowbeam_bs_tx.h"


// Complex service Headlight interface - located in ../generated/example_service
#include "autoheadlight_cs_ifc.h"

/**
 * @brief Auto Headlight service
 * @details This complex service enables the headlight if the vehicle position is detected inside the tunnel and disables the headlight
 * if vehicle is detected outside the tunnel. This also checks if the time based on summer and winter season and enables it accordingly.(time aspect will be developed later)
 * 
 * Input events from basic service: CurrentLatitude 
 *									CurrentLongitude 
 * Output calls for basic service:	headlight (true or false)
 *			
 * Input calls for applications:    autoheadlight enabled (true/false)
 * Output info for applications:	headlight status (true/false)
 *                                  Inside the tunnel (true/false)
 */
class CAutoHeadlightService :
    public sdv::CSdvObject,
    public sdv::IObjectControl,
    public IAutoheadlightService,
    public vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event,
    public vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event
{
public:
    /**
     * @brief Constructor
     */
    CAutoHeadlightService();

    /**
     * @brief Destructor
     */
    ~CAutoHeadlightService();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event)
        SDV_INTERFACE_ENTRY(IAutoheadlightService)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("Auto Headlight Service")
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

private:

    /**
     * @brief Set Current latitude event. Overload of vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetBSCurrentLatitude_Event
     * @param[in] value Current latitude value in float
     */
     void SetCurrentLatitude(float value) override;

    /**
     * @brief Set Current Longitude event. Overload of vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetBSCurrentLongitude_Event
     * @param[in] value Current latitude value in float
     */
     void SetCurrentLongitude(float value) override;

    /**
     * @brief Gets status of vehicle position if it is inside tunnel or not. Overload of IAutoheadlightService::IsinTunnel.
     * @return Returns true if position of vehicle is inside tunnel and false if outside.
     */
     bool IsinTunnel() const override;

    /**
     * @brief Get status of headlights. Overload of IAutoheadlightService::GetHeadlightStatus.
     * @return Returns the status of headlights (true if switched on , false if not switched on)
     */
     bool GetHeadlightStatus() const override;

     /**
      * @brief Get the GPS bounding box
      * @return Returns the bounding box structure
      */
     SGPSBoundBox GetGPSBoundBox() const override;

    /**
     * @brief Update the headlight status based on the vehicle position with respect to tunnel.
     */
    void ProcessHeadlightBasedOnEgoPosition();

    /**
     * @brief Load GPS bounding box from the configuration file
     */
    bool LoadGPSBounds(const sdv::u8string& rssObjectConfig);

    sdv::EObjectStatus      m_eStatus = sdv::EObjectStatus::initialization_pending; ///< Current object status
    volatile float          m_fCurrentLatitude = 0.0;       ///< Current Latitude
    volatile float          m_fCurrentLongitude = 0.0;      ///< Current Longitude
    volatile bool           m_bHeadlight = false;           ///< Headlight status
    SGPSBoundBox            m_SGPSBoundingBox;              ///< Tunnel bounding box coordinates


    ///< Headlight interface.
    vss::Vehicle::Body::Light::Front::LowBeamService::IVSS_SetHeadLightLowBeam* m_pHeadlightSvc = nullptr;
};

DEFINE_SDV_OBJECT(CAutoHeadlightService)

#endif // !define COMPLEX_SERVICE_EXAMPLE_H
