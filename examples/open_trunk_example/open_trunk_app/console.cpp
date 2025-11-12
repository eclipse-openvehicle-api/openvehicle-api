#include "console.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

const CConsole::SConsolePos g_sTitle{ 1, 1 };
const CConsole::SConsolePos g_sSubTitle{ 3, 1 };
const CConsole::SConsolePos g_sSeparator1{ 5, 1 };
const CConsole::SConsolePos g_sDataLink{ 7, 1 };
const CConsole::SConsolePos g_sDataLinkSpeed{ 8, 1 };
const CConsole::SConsolePos g_sSeparator2{ 10, 1 };
const CConsole::SConsolePos g_sVehicleDevice{ 12, 1 };
const CConsole::SConsolePos g_sVehicleDeviceSpeed{ 13, 1 };
const CConsole::SConsolePos g_sSeparator3{ 15, 1 };
const CConsole::SConsolePos g_sBasicService{ 17, 1 };
const CConsole::SConsolePos g_sBasicServiceSpeed{ 18, 1 };
const CConsole::SConsolePos g_sSeparator4{ 20, 1 };
const CConsole::SConsolePos g_sComment1{ 22, 1 };
const CConsole::SConsolePos g_sComment2{ 23, 1 };
const CConsole::SConsolePos g_sComment3{ 24, 1 };
const CConsole::SConsolePos g_sComment4{ 25, 1 };
const CConsole::SConsolePos g_sComment5{ 26, 1 };
const CConsole::SConsolePos g_sSeparator5{ 28, 1 };
const CConsole::SConsolePos g_sComplexServcie1{ 30, 1 };
const CConsole::SConsolePos g_sComplexServcie2{ 31, 1 };
const CConsole::SConsolePos g_sSeparator6{ 33, 1 };
const CConsole::SConsolePos g_sControlDescription{ 35, 1 };
const CConsole::SConsolePos g_sCursor{ 36, 1 };

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

void CConsole::PrintHeader(uint32_t uiInstance)
{
    // Clear the screen...
    std::cout << "\x1b[2J";

    std::string subTtitle = "Standalone application, this is not Mixed-Critical mode!";
    if (uiInstance != 0)
    {
        subTtitle = "Connected to core instance ";
        subTtitle.append(std::to_string(uiInstance));
        subTtitle.append(", this is NOT Mixed-Critical mode yet!");
    }
    // Print the titles
    PrintText(g_sTitle, "Open Trunk example: Open trunk when vehicle is not moving");
    PrintText(g_sSubTitle, subTtitle);
    PrintText(g_sSeparator1, "============================================================================");
    PrintText(g_sDataLink, "Data dispatch service:");
    PrintText(g_sDataLinkSpeed, "Data Link not available");
    PrintText(g_sSeparator2, "----------------------------------------------------------------------------");
    PrintText(g_sVehicleDevice, "Vehicle Device:");
    PrintText(g_sVehicleDeviceSpeed, "Vehicle Device Interface not available.");
    PrintText(g_sSeparator3, "----------------------------------------------------------------------------");
    PrintText(g_sBasicService, "Basic Service:");
    PrintText(g_sBasicServiceSpeed, "Basic Service Interface not available.");
    PrintText(g_sSeparator4, "============================================================================");
    PrintText(g_sComment1, "The complex service which checks the speed of the vehicle can be seen");
    PrintText(g_sComment2, "as an ASIL A/B component and will block the call from QM.");
    PrintText(g_sComment3, "The extern apllication can be seen as a QM function.");
    PrintText(g_sComment4, "If this example would run in a mixed critical environment the connection");
    PrintText(g_sComment5, "from QM to basic service interface would be forbidden.");
    PrintText(g_sSeparator5, "============================================================================");
    PrintText(g_sComplexServcie1, "Basic Service Interface not available.");
    PrintText(g_sComplexServcie2, "Complex Service Interface not available.");
    PrintText(g_sSeparator6, "----------------------------------------------------------------------------");
    PrintText(g_sControlDescription, "Press 'X' to quit; 'C' to clear screen, '1' or '2' to open trunk ");
}

bool CConsole::PrepareDataConsumers()
{
    // Subscribe for the speed and trunk signal
    sdv::core::CDispatchService dispatch;
    m_SignalSpeed = dispatch.Subscribe(trunk::dsVehicleSpeed, [&](sdv::any_t value) { CallbackSpeed(value); });
    if (m_SignalSpeed)
        PrintValue(g_sDataLinkSpeed, "Vehicle Speed RX", m_SpeedDL, "m/s");

    // Registrate for the vehicle device & basic service of the speed. 
    auto vehicleDevice = sdv::core::GetObject("Vehicle.Speed_Device").GetInterface<vss::Vehicle::SpeedDevice::IVSS_Speed>();
    if (vehicleDevice)
    {
        PrintValue(g_sVehicleDeviceSpeed, "Vehicle Speed RX", m_SpeedVD, "m/s");        
        vehicleDevice->RegisterSpeedEvent(dynamic_cast<vss::Vehicle::SpeedDevice::IVSS_WriteSpeed_Event*> (this));
    }

    auto basicService = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (basicService)
    {
        PrintValue(g_sBasicServiceSpeed, "Vehicle Speed RX", m_SpeedBS, "km/h");
        basicService->RegisterOnSignalChangeOfVehicleSpeed(dynamic_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
    }

    // Request the basic service for opening the drunk.
    m_pTrunkSvc = sdv::core::GetObject("Vehicle.Body.Trunk_Service").GetInterface<vss::Vehicle::Body::TrunkService::IVSS_SetOpen>();
    if (m_pTrunkSvc)
        PrintText(g_sComplexServcie1, "Basic Service available");

    m_pITrunkComplexService = sdv::core::GetObject("Open Trunk Service").GetInterface<ITrunkKitService>();
    if (m_pITrunkComplexService)
        PrintText(g_sComplexServcie2, "Complex Service available");

    return true;
}

void CConsole::ResetSignals()
{
    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    auto vehicleDevice = sdv::core::GetObject("Vehicle.Speed_Device").GetInterface<vss::Vehicle::SpeedDevice::IVSS_Speed>();
    if (vehicleDevice)
        vehicleDevice->UnregisterSpeedEvent(dynamic_cast<vss::Vehicle::SpeedDevice::IVSS_WriteSpeed_Event*> (this));

    auto basicService = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (basicService)
        basicService->UnregisterOnSignalChangeOfVehicleSpeed(dynamic_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));

    // Unregister the data link signalss
    if (m_SignalSpeed)
        m_SignalSpeed.Reset();
}

void CConsole::CallbackSpeed(sdv::any_t value)
{
    if (m_SpeedDL != value.get<float>())
    {
        m_SpeedDL = value.get<float>();
        PrintValue(g_sDataLinkSpeed, "Vehicle Speed RX", m_SpeedDL, "m/s");
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


void CConsole::WriteSpeed(float value)
{
    if (m_SpeedVD != value)
    {
        m_SpeedVD = value;
        PrintValue(g_sVehicleDeviceSpeed, "Vehicle Speed RX", m_SpeedVD, "km/h");
    }
}

void CConsole::SetSpeed(float value)
{
    if (m_SpeedBS != value)
    {
        m_SpeedBS = value;
        PrintValue(g_sBasicServiceSpeed, "Vehicle Speed RX", m_SpeedBS, "km/h");
    }
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
        case 'c':
        case 'C':
            PrintText(g_sComplexServcie1, "                                        ");
            PrintText(g_sComplexServcie2, "                                        ");
            break;
        case '1':
            if (m_pTrunkSvc)
            {
                if (m_pTrunkSvc->SetOpen(true))
                    PrintText(g_sComplexServcie1, "Open trunk via basic service - will not be available in Mixed-Critical mode!");
                else
                    PrintText(g_sComplexServcie1, "Open trunk via basic service failed.");
            }
            break;
        case '2':
            if (m_pITrunkComplexService)
            {
                if (m_pITrunkComplexService->PopTrunk())
                    PrintText(g_sComplexServcie2, "Safety open trunk via complex service.");
                else
                    PrintText(g_sComplexServcie2, "Safety open trunk via complex service failed, car is moving");
            }
            break;
        case 'x':
        case 'X':
            bRunning = false;
            break;
        default:
            break;
        }
    }
}
