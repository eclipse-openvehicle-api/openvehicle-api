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
#include <fcntl.h>
#include "../interfaces/vss_vehiclechassisdooraxle01left_vd_rx.h"
#include "../interfaces/vss_vehiclechassisdooraxle01left_bs_rx.h"
#include "../interfaces/vss_vehiclechassisdooraxle01right_bs_rx.h"
#include "../interfaces/vss_vehiclechassisdooraxle02left_bs_rx.h"
#include "../interfaces/vss_vehiclechassisdooraxle02right_bs_rx.h"

#ifdef __unix__
#include <termios.h>        // Needed for tcgetattr and fcntl
#include <unistd.h>
#endif

#include "../generated/door_service/door_ifc.h"

/**
 * @brief Console operation class.
 * @details This class retrieves RX data from the data dispatch service, vehicle device & basic service of front left door on event change.
 * Furthermore, it shows TX value by polling the RX signals.
 */
class CConsole : public vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event
    , public vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event
    , public vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event
    , public vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event
    , public vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event
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
     * @param[in] numberOfDoors number of doors
     */
    void PrintHeader(const uint32_t numberOfDoors);

    /**
     * @brief Prepare the data consumers..
     * @details Gets all signals (4 RX signals showing Open/Closed doors and 4 TX signals if the doors are locked)
     * Open/Closed is done as input (user) while locking the doors is done automatically by complex service when all doors are closed
     * Need to work independent from the number of doors (1-4)
     * @return Returns whether the preparation of the data consumers was successful or not.
     */
    bool PrepareDataConsumers();


    /**
     * @brief Write leftDoorIsOpen signal
     * @param[in] value leftDoorIsOpen
     */
    void WriteIsOpen(bool value) override;

    /**
     * @brief Set leftDoorIsOpen signal (front door)
     * @param[in] value leftDoorIsOpen
     */
    void SetIsOpenL1(bool value) override;

    /**
     * @brief Set rightDoorIsOpen signal (front door)
     * @param[in] value rightDoorIsOpen
     */
    void SetIsOpenR1(bool value) override;

    /**
     * @brief Set leftDoorIsOpen signal (rear door)
     * @param[in] value leftDoorIsOpen
     */
    void SetIsOpenL2(bool value) override;

    /**
     * @brief Set rightDoorIsOpen signal (rear door)
     * @param[in] value rightDoorIsOpen
     */
    void SetIsOpenR2(bool value) override;

    /**
     * @brief For gracefully shutdown all signals need to be reset.
     */
    void ResetSignals();

    /**
     * @brief Starts thread for polling the TX signals (if the doors are locked by the complex service)
     */
    void StartUpdateDataThread();

    /**
     * @brief Stops thread
     */
    void StopUpdateDataThread();

    /**
     * @brief Used to set a flag for when we use external App
     */
    void SetExternalApp();
    

private:

    /**
     * @brief Prepare the data consumers for Standalone application
     * @return Returns whether the preparation of the Standalone data consumers was successful or not.
     */
    bool PrepareDataConsumersForStandAlone();

    /**
     * @brief Callback function when front left door is opened or closed (by user).
     * @param[in] value The value of the signal to update.
     */
    void CallbackFrontLeftDoorIsOpen(sdv::any_t value);

    /**
     * @brief Callback function when front right door is opened or closed (by user).
     * @param[in] value The value of the signal to update.
     */
    void CallbackFrontRightDoorIsOpen(sdv::any_t value);

    /**
     * @brief Callback function when rear left door is opened or closed (by user).
     * @param[in] value The value of the signal to update.
     */
    void CallbackRearLeftDoorIsOpen(sdv::any_t value);

    /**
     * @brief Callback function when rear right door is opened or closed (by user).
     * @param[in] value The value of the signal to update.
     */
    void CallbackRearRightDoorIsOpen(sdv::any_t value);

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
    bool                m_bThreadStarted = false;           ///< Set when initialized.
    bool                m_bRunning = false;                 ///< When set, the application is running.
    bool                m_isExternalApp = false;            ///< True when we have an external application
    mutable std::mutex  m_mPrintToConsole;                  ///< Mutex to print complete message
    
    std::thread         m_threadReadTxSignals;              ///< Simulation datalink thread.

    sdv::core::CSignal  m_SignalFrontLeftDoorIsOpen;        ///< Front Left Door signal (RX input) - open / closed
    sdv::core::CSignal  m_SignalFrontRightDoorIsOpen;       ///< Front Right Door signal (RX input) - open / closed
    sdv::core::CSignal  m_SignalRearLeftDoorIsOpen;         ///< Rear Left Door signal (RX input) - open / closed
    sdv::core::CSignal  m_SignalRearRightDoorIsOpen;        ///< Rear Right Door signal (RX input) - open / closed

    bool                m_FrontLeftDoorIsOpen = false;	    ///< Front Left Door value (RX input signal) - open / closed
    bool                m_FrontRightDoorIsOpen = false;	    ///< Front Right Door value (RX input signal)  - open / closed
    bool                m_RearLeftDoorIsOpen = false;	    ///< Rear Left Door value (RX input signal)  - open / closed
    bool                m_RearRightDoorIsOpen = false;      ///< Rear Right Door value (RX input signal)  - open / closed

    sdv::core::CSignal  m_SignalFrontLeftDoorIsLocked;      ///< Front Left Door signal (TX output) - locked / unlocked
    sdv::core::CSignal  m_SignalFrontRightDoorIsLocked;     ///< Front Right Door signal (TX output) - locked / unlocked
    sdv::core::CSignal  m_SignalRearLeftDoorIsLocked;       ///< Rear Left Door signal (TX output) - locked / unlocked
    sdv::core::CSignal  m_SignalRearRightDoorIsLocked;      ///< Rear Right Door signal (TX output) - locked / unlocked

    bool                m_FrontLeftDoorIsLocked = false;	///< Front Left Door value (TX output) - locked / unlocked
    bool                m_FrontRightDoorIsLocked = false;	///< Front Right Door value (TX output) - locked / unlocked
    bool                m_RearLeftDoorIsLocked = false;	    ///< Rear Left Door value (TX output) - locked / unlocked
    bool                m_RearRightDoorIsLocked = false;    ///< Rear Right Door value (TX output) - locked / unlocked
	
	IDoorService* 		m_pDoorService = nullptr;			///< Door service interface pointer.

#ifdef _WIN32
    DWORD               m_dwConsoleOutMode = 0u;            ///< The console mode before switching on ANSI support.
    DWORD               m_dwConsoleInMode = 0u;             ///< The console mode before switching on ANSI support.
#elif defined __unix__
    struct termios      m_sTermAttr{};                      ///< The terminal attributes before disabling echo.
    int                 m_iFileStatus = 0;                  ///< The file status flags for STDIN.
#else
#error The OS is not supported!
#endif

};

template <typename TValue>
inline void CConsole::PrintValue(SConsolePos sPos, const std::string& rssName, TValue tValue, const std::string& rssUnits)
{
    std::string endName = " ";
    const size_t nEndNameLen = 14 - rssUnits.size();
    const size_t nValueNameLen = 26;
    std::stringstream sstreamValueText;
    sstreamValueText << rssName <<
        std::string(nValueNameLen - std::min(rssName.size(), static_cast<size_t>(nValueNameLen - 1)) - 1, '.') <<
        " " << std::fixed << std::setprecision(2) << tValue << " " << rssUnits <<
        std::string(nEndNameLen - std::min(endName.size(), static_cast<size_t>(nEndNameLen - 1)) - 1, ' ');

    std::lock_guard<std::mutex> lock(m_mPrintToConsole);
    SetCursorPos(sPos);
    std::cout << sstreamValueText.str();
}

template <>
inline void CConsole::PrintValue<bool>(SConsolePos sPos, const std::string& rssName, bool bValue, const std::string& rssStatus)
{;
    PrintValue(sPos, rssName, bValue ? "" : "", rssStatus);
}

#endif // !define CONSOLE_OUTPUT_H
