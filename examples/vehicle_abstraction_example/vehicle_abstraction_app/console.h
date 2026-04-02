 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#ifndef CONSOLE_OUTPUT_H
#define CONSOLE_OUTPUT_H

#include <iostream>
#include <string>
#include <functional>
#include <support/signal_support.h>
#include <support/app_control.h>
#include <support/component_impl.h>
#include <support/timer.h>
#include "../generated/vss_files/signal_identifier.h"
#include <fcntl.h>

#include "../generated/vss_files/vss_vehiclespeed_vd_rx.h"
#include "../generated/vss_files/vss_vehiclespeed_bs_rx.h"

#ifdef __unix__
#include <termios.h>        // Needed for tcgetattr and fcntl
#include <unistd.h>
#endif

/**
 * @brief Console operation class.
 * @details This class retrieves RX data from the data dispatch service, platform abstraction & basic service of speed on event change.
 * Furthermore, it shows TX value by polling the RX signals.
 */
 class CConsole : public vss::Vehicle::SpeedDevice::IVSS_WriteSpeed_Event
    , public vss::Vehicle::SpeedService::IVSS_SetSpeed_Event
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
     * @return Returns whether the preparation of the data consumers was successful or not.
     */
    bool PrepareDataConsumers();

    /**
    * @brief Run loop as long as user input does not exit
    */
    void RunUntilBreak();

    /**
     * @brief Let the user decide which unit the input data should have
     * @return Return true if inout data should be km/h, otherwise false
     */    
    bool SelectInputDataUnits();  

    /**
     * @brief Write vehicleSpeed signal
     * @param[in] value vehicleSpeed
     */
    void WriteSpeed( float value)  override; 

    /**
     * @brief Set vehicleSpeed signal
     * @param[in] value vehicleSpeed
     */
    void SetSpeed( float value)  override;

    /**
     * @brief For gracefully shutdown all signals need to be reset.
     */
    void ResetSignals();

private:

    /**
     * @brief Callback function for speed.
     * @param[in] value The value of the signal to update.
     */
    void CallbackSpeed(sdv::any_t value);

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
     * @param[in] rssStatus Status, because we have signals of type bool
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

    mutable std::mutex  m_mtxPrintToConsole;             ///< Mutex to print complete message
    bool                m_bRunning         = false;      ///< When set, the application is running.
    std::string         m_DataUnit         = "[ --- Input data in m/s --- ]";
    std::string         m_Unit             = "m/s";    

    sdv::core::CSignal  m_SignalSpeed;                   ///< Signal to subscribe to the speed signal in dispatch service

    float               m_SpeedDataLink    = 0.0;        ///< Data Link value, either km/h or m/s
    float               m_PlatformSpeed    = 0.0;        ///< Generalized Speed 
    float               m_BasicSpeed       = 0.0;        ///< Generalized Speed    

#ifdef _WIN32
    DWORD               m_dwConsoleOutMode = 0u;         ///< The console mode before switching on ANSI support.
    DWORD               m_dwConsoleInMode  = 0u;         ///< The console mode before switching on ANSI support.
#elif defined __unix__
    struct termios      m_sTermAttr{};                   ///< The terminal attributes before disabling echo.
    int                 m_iFileStatus = 0;               ///< The file status flags for STDIN.
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
inline void CConsole::PrintValue<bool>(SConsolePos sPos, const std::string& rssName, bool bValue, const std::string& rssStatus)
{;
    PrintValue(sPos, rssName, bValue ? "" : "", rssStatus);
}

#endif // !define CONSOLE_OUTPUT_H
