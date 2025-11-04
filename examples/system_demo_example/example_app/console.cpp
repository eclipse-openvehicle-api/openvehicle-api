#include "console.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

#include "vss_vehiclechassisrearaxlerowwheel_bs_tx.h"
#include "vss_vehiclechassissteeringwheelangle_bs_rx.h"
#include "vss_vehiclesoftwareapplicationisactivecounter_bs_tx.h"
#include "vss_vehiclespeed_bs_rx.h"

/**
 * @brief Key hit check. Windows uses the _kbhit function; POSIX emulates this.
 * @return Returns whether a key has been pressed.
 */
inline bool KeyHit()
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

/**
 * @brief Get the character from the keyboard buffer if pressed.
 * @return Returns the character from the keyboard buffer.
 */
char GetChar()
{
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    return getchar();
#endif
}

const CConsole::SConsolePos g_sTitle{ 1, 1 };
const CConsole::SConsolePos g_sSeparator1{ 2, 1 };
const CConsole::SConsolePos g_sDLDescription{ 4, 1 };
const CConsole::SConsolePos g_sDLSteeringWheel{ 6, 3 };
const CConsole::SConsolePos g_sDLVehicleSpeed{ 7, 3 };
const CConsole::SConsolePos g_sDLRearAxle{ 6, 41 };
const CConsole::SConsolePos g_sDLAliveCounter{ 7, 41 };
const CConsole::SConsolePos g_sSeparator2{ 9, 1 };
const CConsole::SConsolePos g_sBSDescription{ 11, 1 };
const CConsole::SConsolePos g_sBSSteeringWheel{ 13, 3 };
const CConsole::SConsolePos g_sBSVehicleSpeed{ 14, 3 };
const CConsole::SConsolePos g_sSeparator3{ 16, 1 };
const CConsole::SConsolePos g_sCSDescription{ 18, 1 };
const CConsole::SConsolePos g_sCSActivated{ 20, 3 };
const CConsole::SConsolePos g_sCSActive{ 21, 3 };
const CConsole::SConsolePos g_sCSRearAxle{ 20, 41 };
const CConsole::SConsolePos g_sSeparator4{ 23, 1 };
const CConsole::SConsolePos g_sControlDescription{ 25, 1 };
const CConsole::SConsolePos g_sCursor{ 26, 1 };

CConsole::CConsole(bool bMonitorDatalink) : m_bMonitorDatalink(bMonitorDatalink)
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

void CConsole::PrintHeader(const bool bServer, const bool bSimulate)
{
    // Clear the screen...
    std::cout << "\x1b[2J";

    // Create titles
    std::string title = "System demo example";
    std::string dataLinkTitle = "Data link signal values:";    
    if (bServer)
        title.append("  -  Connected to running server");
    else
    {
        if (bSimulate)
        {
            title.append("  -  run as standalone application, simulation mode");
            dataLinkTitle = "Dispatch service signal values:"; 
        }           
        else
        {
            title.append("  -  run as standalone application including data link");
        }
    }

    // Print the titles
    PrintText(g_sTitle, title);
    PrintText(g_sSeparator1, "============================================================================");
    PrintText(g_sDLDescription, dataLinkTitle.c_str());
    PrintText(g_sSeparator2, "----------------------------------------------------------------------------");
    PrintText(g_sBSDescription, "Basic services event values:");
    PrintText(g_sSeparator3, "----------------------------------------------------------------------------");
    PrintText(g_sCSDescription, "Counter steering example service values:");
    PrintText(g_sSeparator4, "============================================================================");
    PrintText(g_sControlDescription, "Press 'X' to quit; 'T' to toggle the service activity...");
}

bool CConsole::PrepareDataConsumers()
{
    //////////////////////////////////////
    // DATA LINK

    // Request the data link signals from the dispatch service. This only works for standalone applications, since the data dispatch
    // service needs to be accessible from within the same process (no IPC marshalling is provided). For server based applications,
    // the data link layer is inacessible by any application (and complex service), hence data cannot be received if connected to a
    // server. The m_bMonitorDataLink flag determines whether data from the data link should can be received or not.
    // NOTE: registering signals and timer for data link data can only occur during configuration time. During the execution, no new
    // signals can be registered.
    if (m_bMonitorDatalink)
    {
        sdv::core::CDispatchService dispatch;
        m_signalRearAxleAngle = dispatch.RegisterTxSignal(demo::dsAxleAngle, 0);
        m_signalCounter = dispatch.RegisterTxSignal(demo::dsLiveCounter, 0);
        m_signalSteeringWheel = dispatch.Subscribe(demo::dsWheelAngle, [&](sdv::any_t value) { DataLinkCallbackSteeringWheelAngle(value); });
        m_signalSpeed = dispatch.Subscribe(demo::dsVehicleSpeed, [&](sdv::any_t value) { DataLinkCallbackVehicleSpeed(value); });
        if (!m_signalRearAxleAngle || !m_signalCounter || !m_signalSteeringWheel || !m_signalSpeed)
        {
            std::cerr << "Console ERROR: TX register and RX subscription failed" << std::endl;
            return false;
        }
    }

    //////////////////////////////////////
    // BASIC SERVICES

    // Request the basic service for the steering wheel.
    auto pSteeringWheelSvc = sdv::core::GetObject("Vehicle.Chassis.SteeringWheel.Angle_Service").GetInterface<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_GetSteeringWheel>();
    if (!pSteeringWheelSvc)
    {
        std::cerr << "Console ERROR: Could not get basic service interface 'IVSS_SetSteeringAngle'" << std::endl;
        return false;
    }

    // Request the basic service for the vehicle speed.
    auto pVehSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (!pVehSpeedSvc)
    {
        std::cerr << "Console ERROR: Could not get basic service interface 'IVSS_SetSpeed'" << std::endl;
        return false;
    }

    // Register steering wheel change event handler.
    pSteeringWheelSvc->RegisterOnSignalChangeOfWheelAngle(static_cast<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event*> (this));

    // Register vehicle speed change event handler.
    pVehSpeedSvc->RegisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));


    //////////////////////////////////////
    // COMPLEX SERVICE

    m_pCounterSteeringSvc = sdv::core::GetObject("Counter Steering Example Service").GetInterface<ICounterSteeringService>();
    if (!m_pCounterSteeringSvc)
    {
        std::cerr << "Console ERROR: Could not get complex service interface 'ICounterSteeringService'" << std::endl;
        return false;
    }

    return true;

}

void CConsole::RunUntilBreak()
{
    // Run until break
    bool bRunning = true;
    while (bRunning)
    {
        // Update and display the data from data link, basic services and complex service.
        UpdateData();

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
        case 't':
        case 'T':
            if (m_pCounterSteeringSvc) m_pCounterSteeringSvc->ActivateService(!m_pCounterSteeringSvc->IsActivated());
            break;
        case 'x':
        case 'X':
            bRunning = false;
            break;
        default:
            break;
        }
    }

    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    // Unregister the data link signalss
    if (m_signalSteeringWheel) m_signalSteeringWheel.Reset();
    if (m_signalSpeed) m_signalSpeed.Reset();
    if (m_signalRearAxleAngle) m_signalRearAxleAngle.Reset();
    if (m_signalCounter) m_signalCounter.Reset();
}

void CConsole::DataLinkCallbackSteeringWheelAngle(sdv::any_t value)
{
    m_fDLSteeringWheelAngle = value.get<float>();
}

void CConsole::DataLinkCallbackVehicleSpeed(sdv::any_t value)
{
    m_fDLVehicleSpeed = value.get<float>();
}

void CConsole::UpdateData()
{
    // Print data link data
    if (m_bMonitorDatalink)
    {
        PrintValue(g_sDLSteeringWheel, "Steering Angle RX", m_fDLSteeringWheelAngle, "rad");
        PrintValue(g_sDLVehicleSpeed, "Vehicle Speed RX", m_fDLVehicleSpeed, "m/s");
        PrintValue(g_sDLRearAxle, "Rear axle angle TX", m_signalRearAxleAngle.Read().get<float>(), "deg");
        PrintValue(g_sDLAliveCounter, "Alive counter TX", m_signalCounter.Read().get<float>(), "");
    }
    else
        PrintText(g_sDLSteeringWheel, "Data link signals are unavailable!");

    // Print basic service event values
    PrintValue(g_sBSSteeringWheel, "Steering Angle", m_fSteeringWheelAngle * 57.296f , "deg");
    PrintValue(g_sBSVehicleSpeed, "Vehicle Speed RX", m_fVehicleSpeed * 3.6f , "km/h");

    // Get complex service information
    if (m_pCounterSteeringSvc)
    {
        PrintValue(g_sCSActivated, "Service activated", m_pCounterSteeringSvc->IsActivated(), "");
        PrintValue(g_sCSActive, "Counter steering active", m_pCounterSteeringSvc->CounterSteeringActive(), "");
        PrintValue(g_sCSRearAxle, "Rear axle angle", m_pCounterSteeringSvc->RearAxleAngle(), "deg");
    }
}

void CConsole::SetSteeringWheel(float value)
{
    m_fSteeringWheelAngle = value;
}

void CConsole::SetSpeed(float value)
{
    m_fVehicleSpeed = value;
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
    std::lock_guard<std::mutex> lock(m_mtxPrintToConsole);
    SetCursorPos(sPos);
    std::cout << rssText;
}
