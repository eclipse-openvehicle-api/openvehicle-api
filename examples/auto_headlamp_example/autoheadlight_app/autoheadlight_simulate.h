#include <iostream>
#include <string>
#include <functional>
#include <support/signal_support.h>
#include <support/app_control.h>
#include <support/component_impl.h>
#include <support/timer.h>
#include "signal_names.h"

// VSS interfaces - located in ../generated/vss_files/include
#include "vss_vehiclepositioncurrentlatitude_bs_rx.h"
#include "vss_vehiclepositioncurrentlongitude_bs_rx.h"
#include "vss_vehiclebodylightfrontlowbeam_bs_tx.h"


// Complex service Headlight interface - located in ../generated/example_service
 #include "autoheadlight_cs_ifc.h"

/**
 * @brief Driveway Simulation utility
 */
class CAutoHeadlightAppSimulate :
    public vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event,     // Basic service interface
    public vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event    // Basic service interface

{
public:

    /**
     * @brief Destructor.
    */
    ~CAutoHeadlightAppSimulate();
    
    /**
     * @brief Initialize the app.
     * @return Return true on success otherwise false
    */
    bool Initialize();

    /**
     * @brief Reset and Stop the app.
    */
    void Shutdown();

    /**
     * @brief Driveway data is provided to complex service and headlight is enabled based on the tunnel data
    */
    void ExecuteTestRun();

private:

    /**
     * @brief Key hit check. Windows uses the _kbhit function; POSIX emulates this.
     * @return Returns whether a key has been pressed.
     */
    bool KeyHit();

    /**
     * @brief Get the character from the keyboard buffer if pressed.
     * @return Returns the character from the keyboard buffer.
     */
    char GetChar();

    /**
     * @brief Access to required services to get information on desired signals
     * @return True if the access to all the services are success
    */
    /**
     * @brief Access to required services to get information on desired signals
     * @return Return true if there was no issue with getting access to services otherwise return false
     */
    bool GetAccessToServices();
    
    /**
     * @brief Register Signals 
     * @return Return true if there was no issue with registering signals otherwise return false
     */
    bool RegisterSignalsSimDatalink();
    
    /**
     * @brief Reset Signals 
     */
    void ResetSignalsSimDatalink();
    
    /**
     * @brief Set the evnironment path to fetch framework binaries
     * @return Return true if there was no issue with setting framework path otherwise return false
    */
    bool IsSDVFrameworkEnvironmentSet();
    
    /**
     * @brief sets the current latitude.
     * @param[in] value current latitude value
     */
    virtual void SetCurrentLatitude(float value) override;
    
    /**
     * @brief sets the current longitude.
     * @param[in] value current longitude value
     */
    virtual void SetCurrentLongitude(float value) override;

    /**
     * @brief Callback function when new latitude value is available
     * @param[in] value The value of the latitude
    */
    void CallbackToSetCurrentLatitude(sdv::any_t value);

    /**
     * @brief Callback function when new longitude value is available
     * @param[in] value The value of the longitude
    */
    void CallbackToSetCurrentLongitude(sdv::any_t value);

    sdv::core::CDispatchService     m_dispatch;                                     ///< Dispatch service 
    bool                            m_bInitialized = false;                         ///< Set when initialized.
    bool                            m_bRunning = false;                             ///< When set, the application is running.
    std::filesystem::path           m_pathFramework;                                ///< Path to the SDV V-API framework.
    sdv::app::CAppControl           m_appcontrol;                                   ///< App-control of SDV V-API.

    sdv::core::CSignal              m_signalCurrentLatitude;                        ///< Signal Current latitude  
    sdv::core::CSignal              m_signalCurrentLongitude;		                ///< Signal Current longitude         
    sdv::core::CSignal              m_signalHeadlight;                              ///< Signal Headlight status

    sdv::core::CSignal              m_VisualCurrentLatitude;                        ///< Signal value visualization purpose : Current latitude subscription
    sdv::core::CSignal              m_VisualCurrentLongitude;		                ///< Signal value visualization purpose : Current longitude subscription       

    float                           m_fDataLinkCurrentLatitude = 0.0f;	            ///< default value (input signal) - datalink monitoring
    float                           m_fDataLinkCurrentLongitude = 0.0f;	            ///< default value (input signal) - datalink monitoring
    bool                            m_bDataLinkHeadlightStatus = false;	            ///< default value (output signal) - datalink monitoring

    float                           m_fBasicServiceCurrentLatitude = 0.0f;          ///< Current Latitude - basic service event value
    float                           m_fBasicServiceCurrentLongitude = 0.0f;         ///< Current Longitude - basic service event value
    bool                            m_bBasicServiceHeadlightStatus = false;	        ///< Headlight Status - basic service event value

    IAutoheadlightService*          m_pIAutoheadlightComplexService = nullptr;      ///< Autoheadlight Service interface pointer.
    
    /**
     * @brief GPS driveway struct for coordinates and text wrt tunnel info
    */
    struct GPS {
        float latitude = 0.0f; ///< Latitude
        float longitude = 0.0f; ///< Longitude
        std::string location = "Before Tunnel"; ///< Text : "Before Tunnel", "Inside Tunnel", "After Tunnel"
    };

    /**
     * @brief Driveway data including the tunnel information
    */
    std::vector<GPS> m_DriveWayData = {
        {47.6495f, 9.4695f, "Before Tunnel"},
        {47.6496f, 9.4696f, "Before Tunnel"},
        {47.6497f, 9.4697f, "Before Tunnel"},
        {47.6498f, 9.4698f, "Before Tunnel"},
        {47.6499f, 9.4699f, "Before Tunnel"},
        {47.6500f, 9.4700f, "Inside Tunnel"},
        {47.6501f, 9.4701f, "Inside Tunnel"},
        {47.6502f, 9.4702f, "Inside Tunnel"},
        {47.6503f, 9.4703f, "Inside Tunnel"},
        {47.6504f, 9.4704f, "Inside Tunnel"},
        {47.6505f, 9.4705f, "Inside Tunnel"},
        {47.6506f, 9.4706f, "After Tunnel"},
        {47.6507f, 9.4707f, "After Tunnel"},
        {47.6508f, 9.4708f, "After Tunnel"},
        {47.6509f, 9.4709f, "After Tunnel"},
        {47.6510f, 9.4710f, "After Tunnel"}
    };
};

