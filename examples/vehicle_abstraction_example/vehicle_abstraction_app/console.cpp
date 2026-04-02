 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include "console.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

const CConsole::SConsolePos g_sTitle{ 1, 1 };
const CConsole::SConsolePos g_sSubTitle1{ 2, 1 };
const CConsole::SConsolePos g_sSubTitle2{3, 1};
const CConsole::SConsolePos g_sSeparator1{ 5, 1 };
const CConsole::SConsolePos g_sDataUnit{ 7, 1 };
const CConsole::SConsolePos g_sDataLinkSpeed{ 8, 1 };
const CConsole::SConsolePos g_sSeparator2{ 10, 1 };
const CConsole::SConsolePos g_sDeviceServiceSpeed{ 12, 1 };
const CConsole::SConsolePos g_sSeparator3{ 14, 1 };
const CConsole::SConsolePos g_sBasicServiceSpeed{ 16, 1 };
const CConsole::SConsolePos g_sSeparator4{ 18, 1 };
const CConsole::SConsolePos g_sComment1{ 20, 1 };
const CConsole::SConsolePos g_sComment2{ 21, 1 };
const CConsole::SConsolePos g_sComment3{ 23, 1 };
const CConsole::SConsolePos g_sComment4{ 24, 1 };   
const CConsole::SConsolePos g_sComment5{ 25, 1 };   
const CConsole::SConsolePos g_sComment6{ 27, 1 };  
const CConsole::SConsolePos g_sComment7{ 28, 1 };  
const CConsole::SConsolePos g_sSeparator5{ 30, 1 };

const CConsole::SConsolePos g_sControlDescription{ 32, 1 };
const CConsole::SConsolePos g_sCursor{ 33, 1 };

CConsole::CConsole()
{
#ifdef _WIN32
    // Enable ANSI escape codes
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE && GetConsoleMode(hStdOut, &m_dwConsoleOutMode))
        SetConsoleMode(hStdOut, m_dwConsoleOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn != INVALID_HANDLE_VALUE && GetConsoleMode(hStdIn, &m_dwConsoleInMode))
        SetConsoleMode(hStdIn, m_dwConsoleInMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
#elif defined __unix__
    // Disable echo
    tcgetattr(STDIN_FILENO, &m_sTermAttr);
    struct termios sTermAttrTemp = m_sTermAttr;
    sTermAttrTemp.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &sTermAttrTemp);
    m_iFileStatus = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, m_iFileStatus | O_NONBLOCK);
#else
#error The OS is not supported!
#endif
}

CConsole::~CConsole()
{
    SetCursorPos(g_sCursor);

#ifdef _WIN32
    // Return to the stored console mode
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE)
        SetConsoleMode(hStdOut, m_dwConsoleOutMode);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn != INVALID_HANDLE_VALUE)
        SetConsoleMode(hStdIn, m_dwConsoleInMode);
#elif defined __unix__
    // Return the previous file status flags.
    fcntl(STDIN_FILENO, F_SETFL, m_iFileStatus);

    // Return to previous terminal state
    tcsetattr(STDIN_FILENO, TCSANOW, &m_sTermAttr);
#endif
}

void CConsole::PrintHeader()
{
    // Clear the screen...
    std::cout << "\x1b[2J";
  
    // Print the titles
    PrintText(g_sTitle, "Vehicle Abstraction: ");
    PrintText(g_sSubTitle1, "This example demonstrates that the vehicle function implementation is independent of the vehicle itself.");    
    PrintText(g_sSubTitle2, "CAN bus vehicle 1 => km/h, CAN bus vehicle 2 => m/s.");    
    PrintText(g_sSeparator1, "====================================================================================================================");
    PrintText(g_sDataUnit, m_DataUnit);
    PrintText(g_sDataLinkSpeed, "Data Link not available.");
    PrintText(g_sSeparator2, "--------------------------------------------------------------------------------------------------------------------");
    PrintText(g_sDeviceServiceSpeed, "Platform Abstraction Interface not available.");
    PrintText(g_sSeparator3, "--------------------------------------------------------------------------------------------------------------------");
    PrintText(g_sBasicServiceSpeed, "Basic Service Interface not available.");
    PrintText(g_sSeparator4, "====================================================================================================================");
    PrintText(g_sComment1, "The dispatch service displays the value that is written by the CAN message read from the ASC file.");
    PrintText(g_sComment2, "The speed should be increased by steps of 10 km/h.");
    PrintText(g_sComment3, "The 'Platform Abstraction' component is responsible for the vehicle abstraction and converts the value if necessary.");  	
    PrintText(g_sComment4, "The example contains 2 components for 'Platform Abstraction' with identical output interfaces.");  	
    PrintText(g_sComment5, "Depending on the input (m/s or km/h) the correct 'Platform Abstraction' component must be loaded.");
    PrintText(g_sComment6, "Therefore, only one speed sensor component is required, which always receives the speed in km/h.");
    PrintText(g_sComment7, "The vehicle function does not require any logic that depends on the specific vehicle.");  
    PrintText(g_sSeparator5, "====================================================================================================================");
    PrintText(g_sControlDescription, "Press 'X' to quit;");
}

bool CConsole::PrepareDataConsumers()
{
    sdv::core::CDispatchService dispatch;
    m_SignalSpeed = dispatch.Subscribe(abstraction::dsVehicleSpeed, [&](sdv::any_t value) { CallbackSpeed(value); });
    if (m_SignalSpeed)
    {
        auto unit = m_Unit;
        unit.append("  [ data dispatch service - input from CAN bus ] ");
        PrintValue(g_sDataLinkSpeed, "Vehicle Speed RX", m_SpeedDataLink, unit);     
    }

    auto deviceServiceSpeed = sdv::core::GetObject("Vehicle.Speed_Device").GetInterface<vss::Vehicle::SpeedDevice::IVSS_ReadSpeed>();
    if (deviceServiceSpeed)
    {
        PrintValue(g_sDeviceServiceSpeed, "Vehicle Speed RX", m_PlatformSpeed, "km/h  [ Output of Platform Abstraction, not accessible by application ]   ");
        deviceServiceSpeed->RegisterSpeedEvent(dynamic_cast<vss::Vehicle::SpeedDevice::IVSS_WriteSpeed_Event*> (this));
    }     

    auto basicServiceSpeed = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (basicServiceSpeed)
    {
        PrintValue(g_sBasicServiceSpeed, "Vehicle Speed RX", m_BasicSpeed, "km/h  [ Output of Speed Sensor Service, accessible by application ]   ");
        basicServiceSpeed->RegisterOnSignalChangeOfVehicleSpeed(dynamic_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
    }   

    return true;
}

void CConsole::WriteSpeed( float value)
{
    if (m_PlatformSpeed != value) 
    {
        m_PlatformSpeed = value;
        PrintValue(g_sDeviceServiceSpeed, "Vehicle Speed RX", m_PlatformSpeed, "km/h  [ Output of Platform Abstraction, not accessible by application ]   ");
    }
}

void CConsole::SetSpeed( float value)
{
    if (m_BasicSpeed != value) 
    {
        m_BasicSpeed= value;
        PrintValue(g_sBasicServiceSpeed, "Vehicle Speed RX", m_BasicSpeed, "km/h  [ Output of Speed Sensor Service, accessible by application ]   ");        
    }
}

void CConsole::ResetSignals()
{
    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    auto deviceServiceSpeed = sdv::core::GetObject("Vehicle.Speed_Devicee").GetInterface<vss::Vehicle::SpeedDevice::IVSS_ReadSpeed>();
    if (deviceServiceSpeed)
    {
        deviceServiceSpeed->UnregisterSpeedEvent(dynamic_cast<vss::Vehicle::SpeedDevice::IVSS_WriteSpeed_Event*> (this));
    }

    auto basicServiceSpeed = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (basicServiceSpeed)
    {
        basicServiceSpeed->UnregisterOnSignalChangeOfVehicleSpeed(dynamic_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
    }    

    if (m_SignalSpeed)
        m_SignalSpeed.Reset();    
}

void CConsole::CallbackSpeed(sdv::any_t value)
{
    if (m_SpeedDataLink != value.get<float>())
    {
        m_SpeedDataLink = value.get<float>();

        auto unit = m_Unit;
        unit.append("  [ data dispatch service - input from CAN bus ] ");        
        PrintValue(g_sDataLinkSpeed, "Vehicle Speed RX", m_SpeedDataLink, unit);
    }
}


CConsole::SConsolePos CConsole::GetCursorPos() const
{
    SConsolePos sPos{};
    std::cout << "\033[6n";

    char buff[128];
    int indx = 0;
    for(;;) {
        int cc = std::cin.get();
        buff[indx] = (char)cc;
        indx++;
        if(cc == 'R') {
            buff[indx + 1] = '\0';
            break;
        }
    }
    int iRow = 0, iCol = 0;
    sscanf(buff, "\x1b[%d;%dR", &iRow, &iCol);
    sPos.uiRow = static_cast<uint32_t>(iRow);
    sPos.uiCol = static_cast<uint32_t>(iCol);
    fseek(stdin, 0, SEEK_END);

    return sPos;
}

void CConsole::SetCursorPos(SConsolePos sPos)
{
    std::cout << "\033[" << sPos.uiRow << ";" << sPos.uiCol << "H";
}

void CConsole::PrintText(SConsolePos sPos, const std::string& rssText)
{
    auto text = rssText;
    while (text.length() < 76)
        text.append(" ");

    std::lock_guard<std::mutex> lock(m_mtxPrintToConsole);        
    SetCursorPos(sPos);
    std::cout << text;
}



bool CConsole::KeyHit()
{
#ifdef _WIN32
    return _kbhit();
#elif __unix__
    int ch = getchar();
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
#endif
}

char CConsole::GetChar()
{
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    return getchar();
#endif
}

void CConsole::RunUntilBreak()
{
    bool bRunning = true;

    while (bRunning)
    {
        // Check for a key
        if (!KeyHit())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Get a keyboard value (if there is any).
        char c = GetChar();
        switch (c)
        {
        case 'x':
        case 'X':
            bRunning = false;
            break;
        default:
            break;
        }
    }
}

bool CConsole::SelectInputDataUnits()
{
    // Clear the screen and goto top...
    std::cout << "\x1b[2J\033[0;0H";
    std::cout << "Click '1' to select km/h or '2' to select m/s";

    bool bIsKmh = false;
    char c = '0';    
    while(c != '1' && c != '2')
    {
        c = GetChar();
        switch (c)
        {
        case '1':
            m_DataUnit = "[ --- Input data in km/h --- ]";
            m_Unit = "km/h";        
            bIsKmh = true;    
            break;
        case '2':
            m_DataUnit = "[ --- Input data in m/s --- ]";
            m_Unit = "m/s";        
            bIsKmh = false;
            break;
        default:
            break;
        }
    }
    return bIsKmh;
}