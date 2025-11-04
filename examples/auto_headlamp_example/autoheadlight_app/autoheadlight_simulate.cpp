#include "autoheadlight_simulate.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

CAutoHeadlightAppSimulate::~CAutoHeadlightAppSimulate()
{
    ResetSignalsSimDatalink();
    Shutdown();
}

bool CAutoHeadlightAppSimulate::Initialize()
{
    if (m_bInitialized)
    {
        return true;
    }

	if (!IsSDVFrameworkEnvironmentSet())
	{
		// if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory		
		m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
		std::cout << "framework runtime directory set\n";
	}
	auto bResult = m_appcontrol.Startup("");
	m_appcontrol.SetConfigMode();
    if (!m_appcontrol.AddConfigSearchDir("config"))
    {
        m_appcontrol.Shutdown();
        return false;
    }    

    bResult &= m_appcontrol.LoadConfig("data_dispatch_example.toml") == sdv::core::EConfigProcessResult::successful;
    bResult &= m_appcontrol.LoadConfig("task_timer_example.toml") == sdv::core::EConfigProcessResult::successful;

    bResult &= RegisterSignalsSimDatalink(); //register signals

    bResult &= m_appcontrol.LoadConfig("autoheadlight_vd_bs.toml") == sdv::core::EConfigProcessResult::successful;
    bResult &= m_appcontrol.LoadConfig("autoheadlight_cs.toml") == sdv::core::EConfigProcessResult::successful;
    if (!bResult)
    {
		SDV_LOG_ERROR("One or more configurations could not be loaded. Cannot continue.");
        m_appcontrol.Shutdown();
        return false;
    }

    if (!GetAccessToServices())
    {
        return false;
    }

    return true;
}

void CAutoHeadlightAppSimulate::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

bool CAutoHeadlightAppSimulate::GetAccessToServices()
{
    m_VisualCurrentLatitude = m_dispatch.Subscribe(headlight::dsFCurrentLatitude, [&](sdv::any_t value) {  CAutoHeadlightAppSimulate::CallbackToSetCurrentLatitude(value); });
    m_VisualCurrentLongitude = m_dispatch.Subscribe(headlight::dsFCurrentLongitude, [&](sdv::any_t value) { CAutoHeadlightAppSimulate::CallbackToSetCurrentLongitude(value); });

    // BASIC SERVICES
    auto pCurrentLatitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLatitude_Service").GetInterface<vss::Vehicle::Position::CurrentLatitudeService::IVSS_GetCurrentLatitude>();
    if (!pCurrentLatitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLatitude': [CAutoHeadlightService]");
        return false;
    }

    auto pCurrentLongitudeSvc = sdv::core::GetObject("Vehicle.Position.CurrentLongitude_Service").GetInterface<vss::Vehicle::Position::CurrentLongitudeService::IVSS_GetCurrentLongitude>();
    if (!pCurrentLongitudeSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_GetCurrentLongitude': [CAutoHeadlightService]");
        return false;
    }

    if (pCurrentLatitudeSvc)
        pCurrentLatitudeSvc->RegisterOnSignalChangeOfFCurrentLatitude(static_cast<vss::Vehicle::Position::CurrentLatitudeService::IVSS_SetCurrentLatitude_Event*> (this));

    if (pCurrentLongitudeSvc)
        pCurrentLongitudeSvc->RegisterOnSignalChangeOfFCurrentLongitude(static_cast<vss::Vehicle::Position::CurrentLongitudeService::IVSS_SetCurrentLongitude_Event*> (this));
    
    // COMPLEX SERVICE

    m_pIAutoheadlightComplexService = sdv::core::GetObject("Auto Headlight Service").GetInterface<IAutoheadlightService>();

    if (!m_pIAutoheadlightComplexService)
    {
        SDV_LOG_ERROR("Console ERROR: Could not get complex service interface 'IAutoheadlightService'");
        return false;
    }
    
    m_bInitialized = true;
    return true;
}

void CAutoHeadlightAppSimulate::ExecuteTestRun()
{
    if (!m_bInitialized) 
        return;

    // Switch to running mode.
    m_appcontrol.SetRunningMode();
    m_bRunning = true;

    bool bRunUntilBreak = true;
    while (bRunUntilBreak)
    { 
        for (const GPS& position : m_DriveWayData)
        {
            m_signalCurrentLatitude.Write<float>(position.latitude);
            m_signalCurrentLongitude.Write<float>(position.longitude);

            // Check for a key
            if (!KeyHit())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }

            // Get a keyboard value (if there is any).
            char c = GetChar();
            if (c == 'x' || c == 'X')
            {
                bRunUntilBreak = false;
                break;
            }
        }
    }
}

bool CAutoHeadlightAppSimulate::RegisterSignalsSimDatalink()
{
    std::string msg = "Signals Registered: ";

    m_signalCurrentLatitude = m_dispatch.RegisterRxSignal(headlight::dsFCurrentLatitude);
    m_signalCurrentLongitude = m_dispatch.RegisterRxSignal(headlight::dsFCurrentLongitude);
    m_signalHeadlight = m_dispatch.RegisterTxSignal(headlight::dsBHeadLightLowBeam, false);

    if (m_signalCurrentLatitude && m_signalCurrentLongitude && m_signalHeadlight)
    {
        std::cout << "Registration was successful\n";
    }
    else
    {
        std::cout << "ATTENTION! Registration failed\n";
        return false;
    }

    auto allSignals = m_dispatch.GetRegisteredSignals();
    msg.append("(");
    msg.append(std::to_string(allSignals.size()));
    msg.append(")\n");
    std::cout << msg.c_str();
    
    return true;
}

void CAutoHeadlightAppSimulate::ResetSignalsSimDatalink()
{
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

    if(m_VisualCurrentLatitude)
    {
        m_VisualCurrentLatitude.Reset();
    }

    if(m_VisualCurrentLongitude)
    {
        m_VisualCurrentLongitude.Reset();
    }
}

bool CAutoHeadlightAppSimulate::IsSDVFrameworkEnvironmentSet()
{
	const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
	if (envVariable)
	{
		std::cout << "framework runtime directory already set\n";
		return true;
	}
	return false;
}

void CAutoHeadlightAppSimulate::SetCurrentLatitude(float value)
{
    m_fBasicServiceCurrentLatitude = value;
}

void CAutoHeadlightAppSimulate::SetCurrentLongitude(float value)
{
    m_fBasicServiceCurrentLongitude = value;
}

void CAutoHeadlightAppSimulate::CallbackToSetCurrentLatitude(sdv::any_t value)
{
    m_fDataLinkCurrentLatitude = value.get<float>();
}

void CAutoHeadlightAppSimulate::CallbackToSetCurrentLongitude(sdv::any_t value)
{
    m_fDataLinkCurrentLongitude= value.get<float>();
}

bool CAutoHeadlightAppSimulate::KeyHit()
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

char CAutoHeadlightAppSimulate::GetChar()
{
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    return getchar();
#endif
}
