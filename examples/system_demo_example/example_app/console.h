#ifndef CONSOLE_OUTPUT_H
#define CONSOLE_OUTPUT_H

#include <iostream>
#include <string>
#include <functional>
#include <support/signal_support.h>
#include <support/app_control.h>
#include <support/component_impl.h>
#include <support/timer.h>
#include "signal_names.h"

#ifdef __unix__
#include <termios.h>        // Needed for tcgetattr and fcntl
#include <unistd.h>
#endif

// VSS interfaces - located in ../generated/vss_files/include
#include "vss_vehiclechassisrearaxlerowwheel_bs_tx.h"
#include "vss_vehiclechassissteeringwheelangle_bs_rx.h"
#include "vss_vehiclesoftwareapplicationisactivecounter_bs_tx.h"
#include "vss_vehiclespeed_bs_rx.h"

// Complex service counter steering interface - located in ../generated/example_service
#include "countersteering.h"

/**
 * @brief Console operation class.
 * @details This class retrieves data from the data link, basix services and complex service and presents it in a regular interval.
 * Furthermore, it runs in a loop and allows interaction with the complex service.
 */
class CConsole :
    public vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event,     // Basic service interface
    public vss::Vehicle::SpeedService::IVSS_SetSpeed_Event                                      // Basic service interface
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
     * @attention Monitoring the data link can only occur when running as standalone. When connecting to a server, the datalink is
     * not accessible from the application due to safety and security reasons.
     * @param[in] bMonitorDatalink When set, register the signals to monitor the datalink.
     */
    CConsole(bool bMonitorDatalink);

    /**
     * @brief Destructor
     */
    ~CConsole();

    /**
     * @brief Print the header.
     * @param[in] bServer if true onnected to a running server, otherwise the application is running standalone
     * @param[in] bSimulate if true signals are simulated, otherwise the signal came from data link
     */
    void PrintHeader(const bool bServer, const bool bSimulate);

    /**
     * @brief Prepare the data consumers..
     * @details Normally the application communicates through the complex services. As an example of how it could work, three levels
     * of data consumers are prepared (as an example of how this could work): data link (if the monitor-data-link flag is set),
     * basic service and complex service.
     * @return Returns whether the preparation of the data consumers was successful or not.
     */
    bool PrepareDataConsumers();

    /**
     * @brief Block this thread until CTRL+C is pressed.
     */
    void RunUntilBreak();

private:
    /**
     * @brief Callback function when steering wheel signal has changed in disptach service.
     * @remarks Only called when m_bMonitorDatalink is enabled (when running as standalone).
     * @param[in] value The value of the signal to update.
     */
    void DataLinkCallbackSteeringWheelAngle(sdv::any_t value);

    /**
     * @brief Callback function when speed signal has changed in disptach service.
     * @remarks Only called when m_bMonitorDatalink is enabled (when running as standalone).
     * @param[in] value The value of the signal to update.
     */
    void DataLinkCallbackVehicleSpeed(sdv::any_t value);

    /**
     * @brief Read the data link TX signals, the basic service event values and the complex service values and print them into the
     * console.
     * @remarks Data link data is only shown when m_bMonitorDatalink is enabled (when running as standalone).
     */
    void UpdateData();

    /**
     * @brief Set steering angle event. Overload of
     * vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event::SetSteeringWheel.
     * @remarks This is an event function of the steering wheel basic service.
     * @param[in] value Steering wheel angle in radials (-16...16 rad)
     */
    virtual void SetSteeringWheel(float value) override;

    /**
     * @brief Speed event. Overload of vss::Vehicle::SpeedService::IVSS_SetSpeed_Event::SetSpeed.
     * @remarks This is an event function of the vehicle speed service.
     * @param[in] value Vehicle speed in m/s (0... 128 m/s)
     */
    virtual void SetSpeed(float value) override;

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
     * @param[in] rssUnits Units the value is in.
     */
    template <typename TValue>
    void PrintValue(SConsolePos sPos, const std::string& rssName, TValue tValue, const std::string& rssUnits);

    /**
     * @brief Align string between name and value.
     * @param[in] message Reference to the message to align.
     * @param[in] desiredLength The desired length or 0 when no length is specified.
     * @return The aligned string.
    */
    std::string AlignString(const std::string& message, uint32_t desiredLength = 0);

    mutable std::mutex          m_mtxPrintToConsole;                ///< Mutex to print complete message
    bool                        m_bMonitorDatalink = false;         ///< When set, the console output monitors datalink data.
    sdv::core::CSignal          m_signalSteeringWheel;              ///< steering wheel angle (input) - datalink monitoring
    sdv::core::CSignal          m_signalSpeed;		                ///< speed (input) - datalink monitoring
    sdv::core::CSignal          m_signalRearAxleAngle;              ///< rear angle (output) - datalink monitoring
    sdv::core::CSignal          m_signalCounter;		            ///< simple counter (output) - datalink monitoring

    float                       m_fDLSteeringWheelAngle = 0.0f;	    ///< default value (input signal) - datalink monitoring
    float                       m_fDLVehicleSpeed = 0.0f;	        ///< default value (input signal) - datalink monitoring

    float                       m_fSteeringWheelAngle = 0.0f;       ///< Steering wheel angle - basic service event value
    float                       m_fVehicleSpeed = 0.0f;             ///< Vehicle speed - basic service event value

    ICounterSteeringService*    m_pCounterSteeringSvc = nullptr;    ///< Counter steering service interface pointer.

#ifdef _WIN32
    DWORD                       m_dwConsoleOutMode = 0u;            ///< The console mode before switching on ANSI support.
    DWORD                       m_dwConsoleInMode = 0u;             ///< The console mode before switching on ANSI support.
#elif defined __unix__
    struct termios              m_sTermAttr{};                      ///< The terminal attributes before disabling echo.
    int                         m_iFileStatus = 0;                  ///< The file status flags for STDIN.
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
        " " << std::fixed << std::setprecision(2) << tValue << " " << rssUnits << "  ";

    std::lock_guard<std::mutex> lock(m_mtxPrintToConsole);
    SetCursorPos(sPos);
    std::cout << sstreamValueText.str();
}

template <>
inline void CConsole::PrintValue<bool>(SConsolePos sPos, const std::string& rssName, bool bValue, const std::string& rssUnits)
{
    PrintValue(sPos, rssName, bValue ? "yes" : "no", rssUnits);
}

#endif // !define CONSOLE_OUTPUT_H
