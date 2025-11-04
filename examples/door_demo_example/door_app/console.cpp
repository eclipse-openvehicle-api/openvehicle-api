#include "include/console.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

const CConsole::SConsolePos g_sTitle{ 1, 1 };
const CConsole::SConsolePos g_sSubTitle{ 2, 1 };
const CConsole::SConsolePos g_sSeparator11{ 4, 1 };
const CConsole::SConsolePos g_sSeparator12{ 6, 1 };
const CConsole::SConsolePos g_sFrontLeftDoorIsOpen{ 7, 1 };
const CConsole::SConsolePos g_sFrontRightDoorIsOpen{ 8, 1 };
const CConsole::SConsolePos g_sRearLeftDoorIsOpen{ 9, 1 };
const CConsole::SConsolePos g_sRearRightDoorIsOpen{ 10, 1 };
const CConsole::SConsolePos g_sFrontLeftDoorIsLocked{ 7, 42 };
const CConsole::SConsolePos g_sFrontRightDoorIsLocked{ 8, 42 };
const CConsole::SConsolePos g_sRearLeftDoorIsLocked{ 9, 42 };
const CConsole::SConsolePos g_sRearRightDoorIsLocked{ 10, 42 };
const CConsole::SConsolePos g_sSeparator21{ 12, 1 };
const CConsole::SConsolePos g_sSeparator22{ 14, 1 };
const CConsole::SConsolePos g_sVehicleDevice{ 15, 1 };
const CConsole::SConsolePos g_sSeparator31{ 17, 1 };
const CConsole::SConsolePos g_sSeparator32{ 19, 1 };
const CConsole::SConsolePos g_sBasicServiceL1{ 20, 1 };
const CConsole::SConsolePos g_sBasicServiceR1{ 21, 1 };
const CConsole::SConsolePos g_sBasicServiceL2{ 22, 1 };
const CConsole::SConsolePos g_sBasicServiceR2{ 23, 1 };
const CConsole::SConsolePos g_sSeparator41{ 25, 1 };
const CConsole::SConsolePos g_sSeparator42{ 26, 1 };
const CConsole::SConsolePos g_sComplexService{ 27, 1 };
const CConsole::SConsolePos g_sControlDescription{ 29, 1 };
const CConsole::SConsolePos g_sCursor{ 30, 1 };

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

void CConsole::PrintHeader(const uint32_t numberOfDoors)
{
    // Clear the screen...
    std::cout << "\x1b[2J";

    std::string title = "Door demo example: Data link attached (4 doors) ";
    if (numberOfDoors <= 4)
    {
        title = "Door demo example: Vehicle has ";
        title.append(std::to_string(numberOfDoors));
        title.append(" doors.");
    }

    // Print the titles
    PrintText(g_sTitle, title.c_str());
    PrintText(g_sSubTitle,    "   Doors are locked automatically after 2 seconds when all doors are closed.");
    PrintText(g_sSeparator11, "============================================================================");
    PrintText(g_sSeparator12, "Data dispatch service:");
    PrintText(g_sFrontLeftDoorIsOpen, "Front Left  Door:.. not available");
    PrintText(g_sFrontRightDoorIsOpen, "Front Right Door:.. not available");
    PrintText(g_sRearLeftDoorIsOpen, "Rear  Left  Door:.. not available");
    PrintText(g_sRearRightDoorIsOpen, "Rear  Right Door:.. not available");
    PrintText(g_sSeparator21, "----------------------------------------------------------------------------");
    PrintText(g_sSeparator22, "Vehicle device:");
    PrintText(g_sVehicleDevice, "Vehicle Device Interface not available.");
    PrintText(g_sSeparator31, "----------------------------------------------------------------------------");
    PrintText(g_sSeparator32, "Basic services:");
    PrintText(g_sBasicServiceL1, "Basic Service Interface not available.");
    PrintText(g_sSeparator41, "----------------------------------------------------------------------------");
    PrintText(g_sSeparator42, "Complex service:");
    PrintText(g_sComplexService, "Complex Service Interface not available.");
    if (!m_isExternalApp)
    {
        PrintText(g_sControlDescription, "Press 'X' to quit; '1', '2', '3', '4' to toggle doors...");
    }
    else
    {
        title.append("          [Connected to an instance]");
        PrintText(g_sTitle, title.c_str());
        PrintText(g_sControlDescription, "Press 'X' to quit; Doors are toggled automatically");
    }

}

bool CConsole::PrepareDataConsumers()
{
    if (!m_isExternalApp)
    {
        if(!(PrepareDataConsumersForStandAlone()))
        {
            return false;
        }
    }

    auto basicServiceL1 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_GetIsOpen>();
    if (!basicServiceL1)
    {
        SDV_LOG_ERROR("Could not get interface 'LeftService::IVSS_IsOpen': [CConsole]");
        return false;
    }
    else
    {
        /* Interface exists -> Clean the line for Console window */
        PrintText(g_sBasicServiceL1, "                                      "); 
    }
    basicServiceL1->RegisterOnSignalChangeOfLeftDoorIsOpen01(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event*> (this));
    bool value = false;
    PrintValue(g_sFrontLeftDoorIsLocked, "Front Left  Latch:", value, (value ? "locked" : "unlocked"));

    // all other doors are optional
    auto basicServiceR1 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Right_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_GetIsOpen>();
    if (basicServiceR1)
    {
        basicServiceR1->RegisterOnSignalChangeOfRightDoorIsOpen01(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event*> (this));
        PrintValue(g_sFrontRightDoorIsLocked, "Front Right Latch:", value, (value ? "locked" : "unlocked"));
    }

    auto basicServiceL2 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Left_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_GetIsOpen>();
    if (basicServiceL2)
    {
        basicServiceL2->RegisterOnSignalChangeOfLeftDoorIsOpen02(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event*> (this));
        PrintValue(g_sRearLeftDoorIsLocked, "Rear  Left  Latch:", value, (value ? "locked" : "unlocked"));
    }

    auto basicServiceR2 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Right_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_GetIsOpen>();
    if (basicServiceR2)
    {
        basicServiceR2->RegisterOnSignalChangeOfRightDoorIsOpen02(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event*> (this));
        PrintValue(g_sRearRightDoorIsLocked, "Rear  Right Latch:", value, (value ? "locked" : "unlocked"));
    }

    m_pDoorService = sdv::core::GetObject("Doors Example Service").GetInterface<IDoorService>();
    if (!m_pDoorService)
    {
        SDV_LOG_ERROR("Console ERROR: Could not get complex service interface 'IDoorService'");
        return false;
    }

    if (m_isExternalApp)
    {
        PrintText(g_sFrontLeftDoorIsLocked, "                                        ");
        PrintText(g_sFrontRightDoorIsLocked, "                                        ");
        PrintText(g_sRearLeftDoorIsLocked, "                                        ");
        PrintText(g_sRearRightDoorIsLocked, "                                        ");
        PrintText(g_sFrontLeftDoorIsOpen, "External Application, no dispatch service.");
        PrintText(g_sFrontRightDoorIsOpen, "                                        ");
        PrintText(g_sRearLeftDoorIsOpen, "                                        ");
        PrintText(g_sRearRightDoorIsOpen, "                                        ");
    }

    return true;
}

void CConsole::ResetSignals()
{
    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    // Registrate for the vehicle device & basic service of the front left door. Front left door mzust exist, the others are optional
    auto vehicleDevice = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen>();
    if (vehicleDevice)
        vehicleDevice->UnregisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event*> (this));

    auto basicServiceL1 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_GetIsOpen>();
    if (basicServiceL1)
        basicServiceL1->UnregisterOnSignalChangeOfLeftDoorIsOpen01(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event*> (this));

    auto basicServiceR1 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Right_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_GetIsOpen>();
    if (basicServiceR1)
        basicServiceR1->UnregisterOnSignalChangeOfRightDoorIsOpen01(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event*> (this));

    auto basicServiceL2 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Left_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_GetIsOpen>();
    if (basicServiceL2)
        basicServiceL2->UnregisterOnSignalChangeOfLeftDoorIsOpen02(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event*> (this));

    auto basicServiceR2 = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Right_Service").GetInterface<vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_GetIsOpen>();
    if (basicServiceR2)
        basicServiceR2->UnregisterOnSignalChangeOfRightDoorIsOpen02(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event*> (this));

    // Unregister the data link signalss
    if (m_SignalFrontLeftDoorIsOpen) 
        m_SignalFrontLeftDoorIsOpen.Reset();
    if (m_SignalFrontRightDoorIsOpen) 
        m_SignalFrontRightDoorIsOpen.Reset();
    if (m_SignalRearLeftDoorIsOpen) 
        m_SignalRearLeftDoorIsOpen.Reset();
    if (m_SignalRearRightDoorIsOpen)
        m_SignalRearRightDoorIsOpen.Reset();
    if (m_SignalFrontLeftDoorIsLocked) 
        m_SignalFrontLeftDoorIsLocked.Reset();
    if (m_SignalFrontRightDoorIsLocked) 
        m_SignalFrontRightDoorIsLocked.Reset();
    if (m_SignalRearLeftDoorIsLocked) 
        m_SignalRearLeftDoorIsLocked.Reset();
    if (m_SignalRearRightDoorIsLocked)
        m_SignalRearRightDoorIsLocked.Reset();
}

void CConsole::StartUpdateDataThread()
{
    if (m_bThreadStarted) 
        return;

    m_bThreadStarted = true;

    m_bRunning = true;
    m_threadReadTxSignals = std::thread(&CConsole::UpdateDataThreadFunc, this);
}

void CConsole::StopUpdateDataThread()
{
    // Stop running and wait for any thread to finalize
    m_bRunning = false;
    if (m_threadReadTxSignals.joinable())
        m_threadReadTxSignals.join();
}

void CConsole::SetExternalApp()
{
    m_isExternalApp =  true;
}

void CConsole::WriteIsOpen(bool value)
{
    PrintValue(g_sVehicleDevice, "Front Left Door:", value, (value ? "open" : "closed"));
}

void CConsole::SetIsOpenL1(bool value)
{
    PrintValue(g_sBasicServiceL1, "Front Left  Door:", value, (value ? "open" : "closed"));
}

void CConsole::SetIsOpenR1(bool value)
{
    PrintValue(g_sBasicServiceR1, "Front Right Door:", value, (value ? "open" : "closed"));
}

void CConsole::SetIsOpenL2(bool value)
{
    PrintValue(g_sBasicServiceL2, "Rear  Left  Door:", value, (value ? "open" : "closed"));
}

void CConsole::SetIsOpenR2(bool value)
{
    PrintValue(g_sBasicServiceR2, "Rear  Right Door:", value, (value ? "open" : "closed"));
}

bool CConsole::PrepareDataConsumersForStandAlone()
{
    // Subscribe for the door and if available get TX signal. Either both exists or none of them
    sdv::core::CDispatchService dispatch;
    m_SignalFrontLeftDoorIsOpen = dispatch.Subscribe(doors::dsLeftDoorIsOpen01, [&](sdv::any_t value) {  CallbackFrontLeftDoorIsOpen(value); });
    if(m_SignalFrontLeftDoorIsOpen)
        m_SignalFrontLeftDoorIsLocked = dispatch.RegisterTxSignal(doors::dsLeftLatch01, 0);

    m_SignalFrontRightDoorIsOpen = dispatch.Subscribe(doors::dsRightDoorIsOpen01, [&](sdv::any_t value) { CallbackFrontRightDoorIsOpen(value); });
    if(m_SignalFrontRightDoorIsOpen)
        m_SignalFrontRightDoorIsLocked = dispatch.RegisterTxSignal(doors::dsRightLatch01, 0);

    m_SignalRearLeftDoorIsOpen = dispatch.Subscribe(doors::dsLeftDoorIsOpen02, [&](sdv::any_t value) {CallbackRearLeftDoorIsOpen(value); });
    if(m_SignalRearLeftDoorIsOpen)
        m_SignalRearLeftDoorIsLocked = dispatch.RegisterTxSignal(doors::dsLeftLatch02, 0);

    m_SignalRearRightDoorIsOpen = dispatch.Subscribe(doors::dsRightDoorIsOpen02, [&](sdv::any_t value) { CallbackRearRightDoorIsOpen(value); });
    if(m_SignalRearRightDoorIsOpen)
        m_SignalRearRightDoorIsLocked = dispatch.RegisterTxSignal(doors::dsRightLatch02, 0);

    // Validate: Either both exists or none of them
    if (m_SignalFrontLeftDoorIsOpen != m_SignalFrontLeftDoorIsLocked)
    {
        SDV_LOG_ERROR("Console ERROR: m_SignalFrontLeftDoorIsOpen != m_SignalFrontLeftDoorIsLocked do not match, failed");
        return false;
    }
    if (m_SignalFrontRightDoorIsOpen != m_SignalFrontRightDoorIsLocked)
    {
        SDV_LOG_ERROR("Console ERROR: m_SignalFrontRightDoorIsOpen != m_SignalFrontRightDoorIsLocked do not match, failed");
        return false;
    }
    if (m_SignalRearLeftDoorIsOpen != m_SignalRearLeftDoorIsLocked)
    {
        SDV_LOG_ERROR("Console ERROR: m_SignalRearLeftDoorIsOpen != m_SignalRearLeftDoorIsLockeddo not match, failed");
        return false;
    }
    if (m_SignalRearRightDoorIsOpen != m_SignalRearRightDoorIsLocked)
    {
        SDV_LOG_ERROR("Console ERROR: m_SignalRearRightDoorIsOpen != m_SignalRearRightDoorIsLocked do not match, failed");
        return false;
    }

    // Registrate for the vehicle device & basic service of the front left door. Front left door mzust exist, the others are optional
    auto vehicleDevice = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen>();
    if (!vehicleDevice)
    {
        SDV_LOG_ERROR("Could not get interface 'LeftDevice::IVSS_IsOpen': [CConsole]");
        return false;
    } 
    vehicleDevice->RegisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event*> (this));
    return true;
}

void CConsole::CallbackFrontLeftDoorIsOpen(sdv::any_t value)
{
    m_FrontLeftDoorIsOpen = value.get<bool>();
    PrintValue(g_sFrontLeftDoorIsOpen, "Front Left  Door:", m_FrontLeftDoorIsOpen, (m_FrontLeftDoorIsOpen ? "open" : "closed"));   
}
void CConsole::CallbackFrontRightDoorIsOpen(sdv::any_t value)
{
    m_FrontRightDoorIsOpen = value.get<bool>();
    PrintValue(g_sFrontRightDoorIsOpen, "Front Right Door:", m_FrontRightDoorIsOpen, (m_FrontRightDoorIsOpen ? "open" : "closed"));
}
void CConsole::CallbackRearLeftDoorIsOpen(sdv::any_t value)
{
    m_RearLeftDoorIsOpen = value.get<bool>();
    PrintValue(g_sRearLeftDoorIsOpen, "Rear  Left  Door:", m_RearLeftDoorIsOpen, (m_RearLeftDoorIsOpen ? "open" : "closed"));
}
void CConsole::CallbackRearRightDoorIsOpen(sdv::any_t value)
{
    m_RearRightDoorIsOpen = value.get<bool>();
    PrintValue(g_sRearRightDoorIsOpen, "Rear  Right Door:", m_RearRightDoorIsOpen, (m_RearRightDoorIsOpen ? "open" : "closed"));
}

void CConsole::UpdateDataThreadFunc()
{
    bool bDoorsAreLocked = true;
    bool bFirstStatusCheck = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (m_bRunning)
    {
        if (!m_isExternalApp)
        {
            UpdateTXSignal(g_sFrontLeftDoorIsLocked, "Front Left  Latch:", m_SignalFrontLeftDoorIsLocked, m_FrontLeftDoorIsLocked);
            UpdateTXSignal(g_sFrontRightDoorIsLocked, "Front Right Latch:", m_SignalFrontRightDoorIsLocked, m_FrontRightDoorIsLocked);
            UpdateTXSignal(g_sRearLeftDoorIsLocked, "Rear  Left  Latch:", m_SignalRearLeftDoorIsLocked, m_RearLeftDoorIsLocked);
            UpdateTXSignal(g_sRearRightDoorIsLocked, "Rear  Right Latch:", m_SignalRearRightDoorIsLocked, m_RearRightDoorIsLocked);
        }

        if (m_pDoorService)
        {
            auto latch = m_pDoorService->GetDoorsStatus();
            if ((bDoorsAreLocked != latch) || (bFirstStatusCheck))
            {
                bDoorsAreLocked = latch;
                PrintText(g_sComplexService, bDoorsAreLocked ? "All doors are locked" :"All doors are unlocked");
                bFirstStatusCheck = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void CConsole::UpdateTXSignal(SConsolePos sPos, const std::string& label, sdv::core::CSignal& signal, bool& value)
{
    // signal may be optional (door may not exist)
    if (signal)
    {
        auto frontLeftDoorIsLocked = value;
        value = signal.Read().get<bool>();
        if (frontLeftDoorIsLocked != value)
        {
            PrintValue(sPos, label , value, (value ? "locked" : "unlocked"));
        }
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
    while (text.length() < 47)
    {
        text.append(" ");
    }

    std::lock_guard<std::mutex> lock(m_mtxPrintToConsole);    
    SetCursorPos(sPos);
    std::cout << text;
}
