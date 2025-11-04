#include "autoheadlight_console.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

const CConsole::SConsolePos g_sTitle{ 1, 1 };
const CConsole::SConsolePos g_sLatitudeMin{ 3, 1 };
const CConsole::SConsolePos g_sLatitudeMax{ 3, 30 };
const CConsole::SConsolePos g_sLongitudeMin{ 4, 1 };
const CConsole::SConsolePos g_sLongitudeMax{ 4, 30 };
const CConsole::SConsolePos g_sSeparator11{ 6, 1 };
const CConsole::SConsolePos g_sSeparator12{ 8, 1 };
const CConsole::SConsolePos g_sDispatchService1{ 9, 1 };
const CConsole::SConsolePos g_sDispatchService2{ 10, 1 };
const CConsole::SConsolePos g_sDispatchService4{ 12, 1 };
const CConsole::SConsolePos g_sSeparator21{ 14, 1 };
const CConsole::SConsolePos g_sSeparator22{ 16, 1 };
const CConsole::SConsolePos g_sVehicleDevice1{ 17, 1 };
const CConsole::SConsolePos g_sVehicleDevice2{ 18, 1 };
const CConsole::SConsolePos g_sSeparator31{ 20, 1 };
const CConsole::SConsolePos g_sSeparator32{ 22, 1 };
const CConsole::SConsolePos g_sBasicService1{ 23, 1 };
const CConsole::SConsolePos g_sBasicService2{ 24, 1 };
const CConsole::SConsolePos g_sSeparator4{ 26, 1 };
const CConsole::SConsolePos g_sComplexService1{ 28, 1 };
const CConsole::SConsolePos g_sComplexService2{ 29, 1 };
const CConsole::SConsolePos g_sComplexService3{ 30, 1 };
const CConsole::SConsolePos g_sSeparator5{ 32, 1 };
const CConsole::SConsolePos g_sControlDescription{ 34, 1 };
const CConsole::SConsolePos g_sCursor{ 35, 1 };

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
    PrintText(g_sTitle, "Headlight");
    PrintText(g_sSeparator11, "============================================================================");
    PrintText(g_sSeparator12, "Data dispatch service:");
    PrintText(g_sSeparator21, "----------------------------------------------------------------------------");
    PrintText(g_sSeparator22, "Vehicle device:");
    PrintText(g_sSeparator31, "----------------------------------------------------------------------------");
    PrintText(g_sSeparator32, "Basic services:");
    PrintText(g_sSeparator4, "----------------------------------------------------------------------------");
    PrintText(g_sComplexService1, "Complex service:");
    PrintText(g_sSeparator5, "----------------------------------------------------------------------------");
    PrintText(g_sControlDescription, "Press 'X' to quit.");
}

bool CConsole::PrepareDataConsumers()
{
    // Vehicle Device
    auto pVDCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Device").GetInterface<vss::Vehicle::Position::CurrentLatitudeDevice::IVSS_CurrentLatitude>();
    if (!pVDCurrentLatitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetVDCurrentLatitude': [CAutoHeadlightService]");
        return false;
    }

    auto pVDCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Device").GetInterface<vss::Vehicle::Position::CurrentLongitudeDevice::IVSS_CurrentLongitude>();
    if (!pVDCurrentLongitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetVDCurrentLongitude': [CAutoHeadlightService]");
        return false;
    }

    if (pVDCurrentLatitudeSvc)
        pVDCurrentLatitudeSvc->RegisterCurrentLatitudeEvent(static_cast<vss::Vehicle::Position::CurrentLatitudeDevice::IVSS_WriteCurrentLatitude_Event*> (this));

    if (pVDCurrentLongitudeSvc)
        pVDCurrentLongitudeSvc->RegisterCurrentLongitudeEvent(static_cast<vss::Vehicle::Position::CurrentLongitudeDevice::IVSS_WriteCurrentLongitude_Event*> (this));

    // BASIC SERVICES
    auto pBSCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (!pBSCurrentLatitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetBSCurrentLatitude': [CAutoHeadlightService]");
        return false;
    }

    auto pBSCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (!pBSCurrentLongitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetBSCurrentLongitude': [CAutoHeadlightService]");
        return false;
    }

    if (pBSCurrentLatitudeSvc)
        pBSCurrentLatitudeSvc->RegisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    if (pBSCurrentLongitudeSvc)
        pBSCurrentLongitudeSvc->RegisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this));

    RegisterSignals();
    UpdateTXSignal(g_sDispatchService4, "Headlight:", m_signalHeadlight, m_bHeadLight);

    m_pIAutoheadlightComplexService = sdv::core::GetObject("Auto Headlight Service").GetInterface<IAutoheadlightService>();

    if (!m_pIAutoheadlightComplexService)
    {
        SDV_LOG_ERROR("Console ERROR: Could not get complex service interface 'IAutoheadlightService'");
        return false;
    }

    auto tunnel = m_pIAutoheadlightComplexService->GetGPSBoundBox();

    std::string minLatitude = "Tunnel Latitude: ";
    std::string maxLatitude = " - ";
    std::string minLongitude = "       Longitude: ";
    std::string maxLongitude = " - ";
    minLatitude.append(std::to_string(tunnel.fTunnelMinLat));
    maxLatitude.append(std::to_string(tunnel.fTunnelMaxLat));
    minLongitude.append(std::to_string(tunnel.fTunnelMinLon));
    maxLongitude.append(std::to_string(tunnel.fTunnelMaxLon));

    PrintText(g_sLatitudeMin, minLatitude);
    PrintText(g_sLatitudeMax, maxLatitude);
    PrintText(g_sLongitudeMin, minLongitude);
    PrintText(g_sLongitudeMax, maxLongitude);

    return true;
}


bool  CConsole::RegisterSignals()
{
    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    sdv::core::CDispatchService dispatch;
    m_signalCurrentLatitude = dispatch.Subscribe(headlight::dsFCurrentLatitude, [&](sdv::any_t value) {  CallbackCurrentLatitude(value); });
    m_signalCurrentLongitude = dispatch.Subscribe(headlight::dsFCurrentLongitude, [&](sdv::any_t value) { CallbackCurrentLongitude(value); });
    m_signalHeadlight = dispatch.RegisterTxSignal(headlight::dsBHeadLightLowBeam, false);

    return true;
}


void CConsole::CallbackCurrentLatitude(sdv::any_t value)
{
    m_fCurrentLatitude = value.get<float>();
    PrintValue(g_sDispatchService1, "Latitude: ", m_fCurrentLatitude, " N");
}
void CConsole::CallbackCurrentLongitude(sdv::any_t value)
{
    m_fCurrentLongitude = value.get<float>();
    PrintValue(g_sDispatchService2, "Longitude: ", m_fCurrentLongitude, " E");
}


void CConsole::ResetSignals()
{
    // Set the cursor position at the end
    SetCursorPos(g_sCursor);

    // Vehicle Device
    auto pVDCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Device").GetInterface<vss::Vehicle::Position::CurrentLatitudeDevice::IVSS_CurrentLatitude>();
    if (pVDCurrentLatitudeSvc)
        pVDCurrentLatitudeSvc->UnregisterCurrentLatitudeEvent(static_cast<vss::Vehicle::Position::CurrentLatitudeDevice::IVSS_WriteCurrentLatitude_Event*> (this));

    auto pVDCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Device").GetInterface<vss::Vehicle::Position::CurrentLongitudeDevice::IVSS_CurrentLongitude>();
    if (pVDCurrentLongitudeSvc)
        pVDCurrentLongitudeSvc->UnregisterCurrentLongitudeEvent(static_cast<vss::Vehicle::Position::CurrentLongitudeDevice::IVSS_WriteCurrentLongitude_Event*> (this));

    // BASIC SERVICES
    auto pBSCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (pBSCurrentLatitudeSvc)
        pBSCurrentLatitudeSvc->RegisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    auto pBSCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (pBSCurrentLongitudeSvc)
        pBSCurrentLongitudeSvc->RegisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this)); 


    if (m_signalCurrentLatitude)
    {
        m_signalCurrentLatitude.Reset();
    }
    if (m_signalCurrentLongitude)
    {
        m_signalCurrentLongitude.Reset();
    }
    if (m_signalHeadlight)
    {
        m_signalHeadlight.Reset();
    }
}


void CConsole::WriteCurrentLatitude(float value)
{
    m_fVehicleDeviceCurrentLatitude = value;
    PrintValue(g_sVehicleDevice1, "Latitude: ", m_fVehicleDeviceCurrentLatitude, " N");
}

void CConsole::WriteCurrentLongitude(float value)
{
    m_fVehicleDeviceCurrentLongitude = value;
    PrintValue(g_sVehicleDevice2, "Longitude: ", m_fVehicleDeviceCurrentLongitude, " E");
}

void CConsole::SetCurrentLatitude(float value)
{
    m_fBasicServiceCurrentLatitude = value;
    PrintValue(g_sBasicService1, "Latitude: ", m_fBasicServiceCurrentLatitude, " N");
}

void CConsole::SetCurrentLongitude(float value)
{
    m_fBasicServiceCurrentLongitude = value;
    PrintValue(g_sBasicService2, "Longitude: ", m_fBasicServiceCurrentLongitude, " E");
}

CConsole::SConsolePos CConsole::GetCursorPos() const
{
    SConsolePos sPos{};
    std::cout << "\033[6n";

    char buff[128];
    int indx = 0;
    for (;;) {
        int cc = std::cin.get();
        buff[indx] = (char)cc;
        indx++;
        if (cc == 'R') {
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


void CConsole::UpdateTXSignal(SConsolePos sPos, const std::string& label, sdv::core::CSignal& signal, bool& value)
{
    if (signal)
    {
        auto headlight = value;
        value = signal.Read().get<bool>();
        if (headlight != value)
        {
            PrintValue(sPos, label, value, (value ? "on >>>>>>>>>>>>>>>>>>>>>>" : "off                       "));
        }
    }
}

void CConsole::UpdateDataThreadFunc()
{
    static auto oldLight = m_pIAutoheadlightComplexService->GetHeadlightStatus();
    static auto olsIsInTunnel = m_pIAutoheadlightComplexService->IsinTunnel();
    PrintValue(g_sComplexService2, "Light: ", olsIsInTunnel, (olsIsInTunnel ? "on" : "off")); 
    PrintValue(g_sComplexService3, "Is in tunnel: ", oldLight, (oldLight ? "yes" : "no"));
    while (m_bRunning)
    {
        UpdateTXSignal(g_sDispatchService4, "Headlight:", m_signalHeadlight, m_bHeadLight);
        auto light = m_pIAutoheadlightComplexService->GetHeadlightStatus();
        if (oldLight != light)
        {
            PrintValue(g_sComplexService3, "Is in tunnel: ", light, (light ? "yes" : "no"));
            oldLight = light;
        }
        auto isInTunnel = m_pIAutoheadlightComplexService->IsinTunnel();
        if (olsIsInTunnel != isInTunnel)
        {
            PrintValue(g_sComplexService2, "Headlight: ", isInTunnel, (isInTunnel ? "on" : "off"));
            olsIsInTunnel = isInTunnel;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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
