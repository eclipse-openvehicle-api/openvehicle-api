#include "control.h"

CExampleControl::~CExampleControl()
{
    Shutdown();
}

bool CExampleControl::Initialize()
{
    if (m_bInitialized) return true;

    // Set the SDV V-API framework directory.
    if (!m_pathFramework.empty())
        m_appcontrol.SetFrameworkRuntimeDirectory(m_pathFramework);

    // Start the framework
    std::stringstream sstreamAppConfig;
    sstreamAppConfig << "[Application]" << std::endl;
    sstreamAppConfig << "Mode=\"" << (RunAsStandaloneApp() ? "Standalone" : "External") << "\"" << std::endl;
    sstreamAppConfig << "Instance=" << m_uiInstance << std::endl;
    sstreamAppConfig << "Retries=" << 6 << std::endl;    
    sstreamAppConfig << "[Console]" << std::endl;
    sstreamAppConfig << "Report=";
    switch (m_eReporting)
    {
    case EAppControlReporting::normal:
        sstreamAppConfig << "\"Normal\"";
        break;
    case EAppControlReporting::verbose:
        sstreamAppConfig << "\"Verbose\"";
        break;
    default:
        sstreamAppConfig << "\"Silent\"";
        break;
    }
    sstreamAppConfig << std::endl;
    if (!m_appcontrol.Startup(sstreamAppConfig.str())) return false;

    // Local configurations only when running as standalone.
    if (RunAsStandaloneApp())
    {
        // Start in configuration mode.
        m_appcontrol.SetConfigMode();
        if (!m_appcontrol.AddConfigSearchDir("config"))
        {
            m_appcontrol.Shutdown();
            return false;
        }
    }

    m_bInitialized = true;
    return true;
}

void CExampleControl::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

bool CExampleControl::IsInitialized() const
{
    return m_bInitialized;
}

bool CExampleControl::LoadConfigFile(const std::string& inputMsg, const std::string& configFileName)
{
    if (!m_bInitialized && m_eRunAs == ERunAs::standalone) return false;

    std::string msg = inputMsg;
    if (m_appcontrol.LoadConfig(configFileName) == sdv::core::EConfigProcessResult::successful)
    {
        msg.append("ok\n");
        std::cout << msg.c_str();
        return true;
    }

    msg.append("FAILED.\n");
    std::cout << msg.c_str();
    return false;
}

void CExampleControl::StartTestRun()
{
    if (!m_bInitialized) return;

    // Switch to running mode.
    m_appcontrol.SetRunningMode();

    // Should the datalink be simulated?
    if (m_eRunAs != ERunAs::standalone_simulated) return;

    // Start the simulated datalink
    m_bRunning = true;
    m_threadSimulateDatalink = std::thread(&CExampleControl::SimulateDatalinkThreadFunc, this);
}

void CExampleControl::StopTestRun()
{
    // Stop running and wait for any thread to finalize
    m_bRunning = false;
    if (m_threadSimulateDatalink.joinable())
        m_threadSimulateDatalink.join();
}

bool CExampleControl::HasCommandLineError() const
{
    return m_bCmdLnError;
}

bool CExampleControl::HasRequestedCommandLineHelp() const
{
    return m_bCmdLnHelp;
}

CExampleControl::ERunAs CExampleControl::GetAppOperation() const
{
    return m_eRunAs;
}

bool CExampleControl::IsSimulationMode() const
{
    return m_eRunAs == ERunAs::standalone_simulated;
}

bool CExampleControl::RunAsStandaloneApp() const
{
    return m_eRunAs == ERunAs::standalone || m_eRunAs == ERunAs::standalone_simulated;
}

bool CExampleControl::RunAsServerApp() const
{
    return m_eRunAs == ERunAs::server_connect;
}

bool CExampleControl::RegisterSignalsSimDatalink()
{
    if (!m_bInitialized) return false;
    if (m_eRunAs != ERunAs::standalone_simulated) return true;  // Nothing to do...

    std::string msg = "Register all signals: ";
    sdv::core::CDispatchService dispatch;

    m_signalSteeringWheel = dispatch.RegisterRxSignal(demo::dsWheelAngle);
    m_signalSpeed = dispatch.RegisterRxSignal(demo::dsVehicleSpeed);
    m_signalRearAngle = dispatch.RegisterTxSignal(demo::dsAxleAngle, 0);
    m_signalCounter = dispatch.RegisterTxSignal(demo::dsLiveCounter, 0);

    if (m_signalSteeringWheel && m_signalSpeed && m_signalRearAngle && m_signalCounter)
        std::cout << "Registration was successful\n";
    else
        std::cout << "ATTENTION! Registration failed\n";

    auto allSignals = dispatch.GetRegisteredSignals();
    msg.append("(number of signals == ");
    msg.append(std::to_string(allSignals.size()));
    msg.append(") ok\n");
    std::cout << msg.c_str();
    return true;
}

void CExampleControl::ResetSignalsSimDatalink()
{
    if (m_eRunAs != ERunAs::standalone_simulated) return;  // Nothing to do...

    if (m_signalSteeringWheel)
        m_signalSteeringWheel.Reset();
    if (m_signalSpeed)
        m_signalSpeed.Reset();
    if (m_signalRearAngle)
        m_signalRearAngle.Reset();
    if (m_signalCounter)
        m_signalCounter.Reset();
}

void CExampleControl::SimulateDatalinkThreadFunc()
{
    if (m_eRunAs != ERunAs::standalone_simulated) return;  // Nothing to do...

    // Send fSteeringWheelAngle wheel angel from -16 to 16 radians and vice versa
    // Send fVehicleSpeed from 0 to 12 m/s (43.2 km/h) and vice versa

    float fSteeringWheelAngle = 0.0f;
    float fVehicleSpeed = 0.0f;
    m_signalSpeed.Write(fVehicleSpeed);
    m_signalSteeringWheel.Write(fSteeringWheelAngle);

    float fDeltaSteering = 0.1f;
    float fDeltaSpeed = 0.1f;

    while (m_bRunning)
    {
        fSteeringWheelAngle += fDeltaSteering;
        if (fSteeringWheelAngle >= 15.999f)
        {
            fSteeringWheelAngle = 16.0f;
            fDeltaSteering = -0.1f;
        }
        else if (fSteeringWheelAngle <= -16.0f)
        {
            fSteeringWheelAngle = -16.0f;
            fDeltaSteering = 0.1f;
        }

        fVehicleSpeed += fDeltaSpeed;
        if (fVehicleSpeed >= 12.0f)
        {
            fVehicleSpeed = 12.0f;
            fDeltaSpeed = -1.0f;
        }
        else if (fVehicleSpeed <= 0.0f)
        {
            fVehicleSpeed = 0.0f;
            fDeltaSpeed = 0.1f;
        }

        m_signalSteeringWheel.Write(fSteeringWheelAngle);
        m_signalSpeed.Write(fVehicleSpeed);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}
