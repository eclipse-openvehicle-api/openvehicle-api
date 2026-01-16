#ifndef CONSOLE_OUTPUT_H
#define CONSOLE_OUTPUT_H

#include <iostream>
#include <string>
#include <functional>
#include <support/signal_support.h>
#include <support/app_control.h>
#include <support/component_impl.h>
#include <support/timer.h>
#include <atomic>
#include "signal_names.h"
#include <fcntl.h>
#include "vss_vehiclepositioncurrentlatitude_vd_rx.h"
#include "vss_vehiclepositioncurrentlongitude_vd_rx.h"
#include "vss_vehiclepositioncurrentlatitude_bs_rx.h"
#include "vss_vehiclepositioncurrentlongitude_bs_rx.h"
#include "vss_vehiclebodylightfrontlowbeam_bs_tx.h"

// Complex service Headlight interface - located in ../generated/example_service
#include "autoheadlight_cs_ifc.h"

#ifdef __unix__
#include <termios.h>        // Needed for tcgetattr and fcntl
#include <unistd.h>
#endif

/**
 * @brief Console operation class.
 * @details This class retrieves RX data from the data dispatch service, vehicle device & basic service of front left door on event change.
 * Furthermore, it shows TX value by polling the RX signals.
 */
class CConsole :
    public vss::Vehicle::Position::CurrentLatitudeDevice::IVSS_WriteCurrentLatitude_Event,    // Vehicle Device interface
    public vss::Vehicle::Position::CurrentLongitudeDevice::IVSS_WriteCurrentLongitude_Event,  // Vehicle Device interface
    public vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event,     // Basic service interface
    public vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event    // Basic service interface
{
public:
    /**
     * @brief Screen position structure
     */
    struct SConsolePos
    {
        uint32_t uiRow;     ///< Row position (starts at 1)
        uint32_t uiCol;     ///< Column position (starts at 1)
    };

    /**
     * @brief Constructor
     */
    CConsole();

    /**
     * @brief Destructor
     */
    ~CConsole();

    /**
     * @brief Print the header.
     */
    void PrintHeader();

    /**
     * @brief Prepare the data consumers..
     * @details Gets all signals (2 RX signals [GPS] and 1 TX signals [headlight beam)
     * @return Returns whether the preparation of the data consumers was successful or not.
     */
    bool PrepareDataConsumers();

    /**
     * @brief For gracefully shutdown all signals need to be reset.
     */
    void ResetSignals();

    /**
     * @brief Starts thread for polling the TX signals
     */
    void StartUpdateDataThread();

    /**
     * @brief Stops thread
     */
    void StopUpdateDataThread();


private:

    /**
     * @brief sets the current latitude.
     * @param[in] value current latitude value
     */
    void WriteCurrentLatitude(float value) override;

    /**
     * @brief sets the current longitude.
     * @param[in] value current longitude value
     */
    void WriteCurrentLongitude(float value) override;

    /**
     * @brief sets the current latitude.
     * @param[in] value current latitude value
     */
    void SetCurrentLatitude(float value) override;

    /**
     * @brief sets the current longitude.
     * @param[in] value current longitude value
     */
    void SetCurrentLongitude(float value) override;

    /**
    * @brief Register Signals
    * @return Return true if there was no issue with registering signals otherwise return false
    */
    bool RegisterSignals();

    /**
     * @brief Callback function when new latitude value is available
     * @param[in] value The value of the latitude
    */
    void CallbackCurrentLatitude(sdv::any_t value);

    /**
     * @brief Callback function when new longitude value is available
     * @param[in] value The value of the longitude
    */
    void CallbackCurrentLongitude(sdv::any_t value);


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

    /**
     * @brief Read the data link TX signals and print them into the console.
     */
    void UpdateDataThreadFunc();

    /**
     * @brief Update the signal on the console output depending on the signal
     * @details Check if the signal is valid. If invalid, ignore it.
     */
    void UpdateTXSignal(SConsolePos sPos, const std::string& label, sdv::core::CSignal& signal, bool& value);

    /**
     * @brief Get the cursor position of the console.
     * @return The cursor position.
     */
    SConsolePos GetCursorPos() const;

    /**
     * @brief Set the current cursor position for the console.
     * @param[in] sPos Console position to place the current cursor at.
     */
    void SetCursorPos(SConsolePos sPos);

    /**
     * @brief Print text at a specific location.
     * @param[in] sPos The location to print text at.
     * @param[in] rssText Reference to the text to print.
    */
    void PrintText(SConsolePos sPos, const std::string& rssText);

    /**
     * @brief Print a value string at a specific location.
     * @tparam TValue Type of value.
     * @param[in] sPos The location to print the value at.
     * @param[in] rssName Reference to the value.
     * @param[in] tValue The value.
     * @param[in] rssStatus Status, becuse we have signals of type bool
     */
    template <typename TValue>
    void PrintValue(SConsolePos sPos, const std::string& rssName, TValue tValue, const std::string& rssStatus);

    /**
     * @brief Align string between name and value.
     * @param[in] message Reference to the message to align.
     * @param[in] desiredLength The desired length or 0 when no length is specified.
     * @return The aligned string.
    */
    std::string AlignString(const std::string& message, uint32_t desiredLength = 0);

    mutable std::mutex  m_mtxPrintToConsole;                ///< Mutex to print complete message
    std::thread         m_threadReadTxSignals;              ///< Simulation datalink thread.
    bool                m_bThreadStarted = false;           ///< Set when initialized.
    std::atomic_bool    m_bRunning = false;                 ///< When set, the application is running.
    mutable std::mutex  m_mPrintToConsole;                  ///< Mutex to print complete message

    sdv::core::CSignal  m_signalCurrentLatitude;            ///< Signal Current latitude  
    sdv::core::CSignal  m_signalCurrentLongitude;		    ///< Signal Current longitude         
    sdv::core::CSignal  m_signalHeadlight;                  ///< Signal Headlight status

    float               m_fCurrentLongitude = 0.0f;         //< Current latitude
    float               m_fCurrentLatitude = 0.0f;          //< Current longitude   
    bool                m_bHeadLight = true;	            ///< Head light

    float               m_fVehicleDeviceCurrentLatitude = 0.0f;       ///< Current latitude (Vehicle Device)
    float               m_fVehicleDeviceCurrentLongitude = 0.0;       ///< Current longitude  (Vehicle Device)   
    
    float               m_fBasicServiceCurrentLatitude = 0.0f;	      ///< Current latitude (basic Service)
    float               m_fBasicServiceCurrentLongitude = 0.0;        ///< Current longitude (basic Service)  

    IAutoheadlightService* m_pIAutoheadlightComplexService = nullptr; ///< Autoheadlight Service interface pointer.


#ifdef _WIN32
    DWORD               m_dwConsoleOutMode = 0u;            ///< The console mode before switching on ANSI support.
    DWORD               m_dwConsoleInMode = 0u;             ///< The console mode before switching on ANSI support.
#elif defined __unix__
    struct termios      m_sTermAttr {};                     ///< The terminal attributes before disabling echo.
    int                 m_iFileStatus = 0;                  ///< The file status flags for STDIN.
#else
#error The OS is not supported!
#endif

};

template <typename TValue>
inline void CConsole::PrintValue(SConsolePos sPos, const std::string& rssName, TValue tValue, const std::string& rssUnits)
{
    const size_t nValueNameLen = 26;
    std::stringstream sstreamValueText;
    sstreamValueText << rssName <<
        std::string(nValueNameLen - std::min(rssName.size(), static_cast<size_t>(nValueNameLen - 1)) - 1, '.') <<
        " " << std::fixed << std::setprecision(6) << tValue << " " << rssUnits << "  ";

    std::lock_guard<std::mutex> lock(m_mPrintToConsole);
    SetCursorPos(sPos);
    std::cout << sstreamValueText.str();
}

template <>
inline void CConsole::PrintValue<bool>(SConsolePos sPos, const std::string& rssName, bool bValue, const std::string& rssUnits)
{
    PrintValue(sPos, rssName, bValue ? "" : "", rssUnits);
}


#endif // !define CONSOLE_OUTPUT_H
