#include "app_control.h"
#include "module_control.h"
#include "repository.h"
#include "sdv_core.h"
#include "../../global/exec_dir_helper.h"
#include "../../global/base64.h"
#include "../../global/flags.h"
#include "../../global/tracefifo/trace_fifo.cpp"
#include "toml_parser/parser_toml.h"
#include "local_shutdown_request.h"

const char* szSettingsTemplate = R"code(# Settings file
[Settings]
Version = 100

# The system config array can contain zero or more configurations that are loaded at the time
# the system ist started. It is advisable to split the configurations in:
#  platform config     - containing all the components needed to interact with the OS,
#                        middleware, vehicle bus, Ethernet.
#  vehicle interface   - containing the vehicle bus interpretation components like data link
#                        based on DBC and devices for their abstraction.
#  vehicle abstraction - containing the basic services
# Load the system configurations by providing the "SystemConfig" keyword as an array of strings.
# A relative path is relative to the installation directory (being "exe_location/instance_id").
#
# Example:
#   SystemConfig = [ "platform.toml", "vehicle_ifc.toml", "vehicle_abstract.toml" ]
#

# The application config contains the configuration file that can be updated when services and
# apps are being added to the system (or being removed from the system). Load the application
# config by providing the "AppConfig" keyword as a string value. A relative path is relative to
# the installation directory (being "exe_location/instance_id").
#
# Example
#   AppConfig = "app_config.toml"
)code";

/**
 * @brief Specific exit handler helping to shut down after startup. In case the shutdown wasn't explicitly executed.
 */
void ExitHandler()
{
    // Check if the system was properly shut down
    switch (GetAppControl().GetOperationState())
    {
    case sdv::app::EAppOperationState::shutting_down:
    case sdv::app::EAppOperationState::not_started:
        break;
    default:
        GetAppControl().Shutdown(true);
        break;
    }
}

CAppControl::CAppControl()
{}

CAppControl::~CAppControl()
{}

bool CAppControl::IsMainApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::main;
}

bool CAppControl::IsIsolatedApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::isolated;
}

bool CAppControl::IsStandaloneApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::standalone;
}

bool CAppControl::IsEssentialApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::essential;
}

bool CAppControl::IsMaintenanceApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::maintenance;
}

bool CAppControl::IsExternalApplication() const
{
    return m_eContextMode == sdv::app::EAppContext::external;
}

sdv::app::EAppContext CAppControl::GetContextType() const
{
    return m_eContextMode;
}

uint32_t CAppControl::GetInstanceID() const
{
    return m_uiInstanceID;
}

uint32_t CAppControl::GetRetries() const
{
    return m_uiRetries;
}

bool CAppControl::Startup(/*in*/ const sdv::u8string& ssConfig, /*in*/ IInterfaceAccess* pEventHandler)
{
    m_pEvent = pEventHandler ? pEventHandler->GetInterface<sdv::app::IAppEvent>() : nullptr;

    // Intercept the logging...
    std::stringstream sstreamCOUT, sstreamCLOG, sstreamCERR;
    std::streambuf* pstreambufCOUT = std::cout.rdbuf(sstreamCOUT.rdbuf());
    std::streambuf* pstreambufCLOG = std::clog.rdbuf(sstreamCLOG.rdbuf());
    std::streambuf* pstreambufCERR = std::cerr.rdbuf(sstreamCERR.rdbuf());

    // Write initial startup message
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    SDV_LOG_INFO("SDV Application start at ", std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S %Z"));

    BroadcastOperationState(sdv::app::EAppOperationState::initializing);

    // Process the application config.
    bool bRet = ProcessAppConfig(ssConfig);

    // Undo logging interception
    sstreamCOUT.rdbuf()->pubsync();
    sstreamCLOG.rdbuf()->pubsync();
    sstreamCERR.rdbuf()->pubsync();
    std::cout.rdbuf(pstreambufCOUT);
    std::clog.rdbuf(pstreambufCLOG);
    std::cerr.rdbuf(pstreambufCERR);

    // Process config error...
    if (!bRet)
    {
        std::cout << sstreamCOUT.str();
        std::clog << sstreamCLOG.str();
        std::cerr << sstreamCERR.str();
        if (!m_bSilent)
            std::cerr << "ERROR: Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Open the stream buffer and attach the streams if the application control is initialized as main app.
    if (m_eContextMode == sdv::app::EAppContext::main)
    {
        m_fifoTraceStreamBuffer.SetInstanceID(m_uiInstanceID);
        m_fifoTraceStreamBuffer.Open(1000);
        std::cout << "**********************************************" << std::endl;
    }

    // Stream the starting logging to the output streams...
    std::cout << sstreamCOUT.str();
    std::clog << sstreamCLOG.str();
    std::cerr << sstreamCERR.str();

    // Check for a correctly opened stream buffer
    if (m_eContextMode == sdv::app::EAppContext::main && !m_fifoTraceStreamBuffer.IsOpened())
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Log streaming could not be initialized; cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Allow only one instance if running as main application
    if (m_eContextMode == sdv::app::EAppContext::main)
    {
        m_pathLockFile = GetExecDirectory() / ("sdv_core_" + std::to_string(m_uiInstanceID) + ".lock");
#ifdef _WIN32
        // Checkf or the existence of a lock file. If existing, do not continue.
        if (std::filesystem::exists(m_pathLockFile)) return false;

        // Create temp file
        m_pLockFile = _wfopen(m_pathLockFile.native().c_str(), L"a+TD");
#elif defined __unix__
        // Create a lock file if not existing.
        m_pLockFile = fopen(m_pathLockFile.native().c_str(), "a+");
        if (!m_pLockFile || lockf(fileno(m_pLockFile), F_TLOCK, 0) != 0)
        {
            // File exists already or could not be opened.
            // No continuation possible.
            if (m_pLockFile) fclose(m_pLockFile);
            m_pLockFile = nullptr;
            return false;
        }
#else
#error OS is not supported!
#endif
    }

    std::string ssErrorString;
    auto fnLoadModule = [&ssErrorString](const sdv::u8string& rssModule) -> bool
    {
        bool bLocalRet = GetModuleControl().Load(rssModule);
        if (!bLocalRet) ssErrorString = std::string("Failed to load module '") + rssModule + "'.";
        return bLocalRet;
    };
    auto fnCreateObject = [&ssErrorString](const sdv::u8string& rssClass, const sdv::u8string& rssObject, const sdv::u8string& rssConfig) -> bool
    {
        bool bLocalRet = GetRepository().CreateObject2(rssClass, rssObject, rssConfig);
        if (!bLocalRet)
        {
            ssErrorString = std::string("Failed to instantiate a new object from class '") + rssClass + "'";
            if (!rssObject.empty())
                ssErrorString += std::string(" with name '") + rssObject + "'.";
        }
        return bLocalRet;
    };

    // Load the core services
    if (!fnLoadModule("core_services.sdv"))
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to load the Core Services. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Load the logger module if one is specified
    if (!m_pathLoggerModule.empty())
    {
        m_tLoggerModuleID = fnLoadModule(m_pathLoggerModule.u8string());
        if (!m_tLoggerModuleID)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to load the custom logger. Cannot continue!" << std::endl;
            Shutdown(true);
            return false;
        }
    }

    // Start the logger and assign it to the logger control.
    fnCreateObject(m_ssLoggerClass, m_ssLoggerClass, "");
    sdv::IInterfaceAccess* pLoggerObj = GetRepository().GetObject(m_ssLoggerClass);
    if (!pLoggerObj)
    {
        GetRepository().DestroyObject2(m_ssLoggerClass);
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to start the logger. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    sdv::core::ILoggerConfig* pLoggerConfig = pLoggerObj->GetInterface<sdv::core::ILoggerConfig>();
    sdv::core::ILogger* pLogger = pLoggerObj->GetInterface<sdv::core::ILogger>();
    if (!pLoggerConfig || !pLogger)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to start the logger. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }
    if (!m_ssProgramTag.empty())
        pLoggerConfig->SetProgramTag(m_ssProgramTag);
    pLoggerConfig->SetLogFilter(m_eSeverityFilter, m_eSeverityViewFilter);
    GetLoggerControl().SetLogger(pLogger);

    // Create the core service objects
    bRet = fnCreateObject("AppControlService", "AppControlService", "");
    bRet = bRet && fnCreateObject("RepositoryService", "RepositoryService", "");
    bRet = bRet && fnCreateObject("ModuleControlService", "ModuleControlService", "");
    bRet = bRet && fnCreateObject("ConfigService", "ConfigService", "");
    if (!bRet)
    {
        if (!m_bSilent)
        {
            std::cerr << "ERROR: Failed to start the Core Services. Cannot continue!" << std::endl;
            if (!ssErrorString.empty())
                std::cerr << "REASON: " << ssErrorString << std::endl;
        }
        Shutdown(true);
        return false;
    }

    // Load specific services
    bool bLoadRPCClient = false, bLoadRPCServer = false;
    bool bLoadInstallationManifests = false;
    switch (m_eContextMode)
    {
    case sdv::app::EAppContext::standalone:
        break;
    case sdv::app::EAppContext::external:
        bLoadRPCClient = true;
        break;
    case sdv::app::EAppContext::isolated:
        bLoadInstallationManifests = true;
        bLoadRPCClient = true;
        break;
    case sdv::app::EAppContext::main:
        bLoadInstallationManifests = true;
        bLoadRPCClient = true;
        bLoadRPCServer = true;
        break;
    case sdv::app::EAppContext::essential:
        //bLoadRPCClient = true;
        break;
    case sdv::app::EAppContext::maintenance:
        bLoadRPCClient = true;
        break;
    default:
        if (!m_bSilent)
            std::cerr << "ERROR: Invalid Run-As mode. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Load installation manifests for main and isolated applications.
    if (bLoadInstallationManifests)
    {
        if (!GetAppConfig().LoadInstallationManifests())
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to load installation manifests. Cannot continue!" << std::endl;
            Shutdown(true);
            return false;
        }
    }

    // Load process control
    if (bRet) bRet = fnLoadModule("process_control.sdv") ? true : false;
    if (bRet) bRet = fnCreateObject("ProcessControlService", "", "");

    // Load RPC components
    if (bRet && bLoadRPCServer)
    {
        // Load hardware identification
        bRet = fnLoadModule("hardware_ident.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("HardwareIdentificationService", "", "");

        // Load shared memory channel
        if (bRet) bRet = fnLoadModule("ipc_shared_mem.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("DefaultSharedMemoryChannelControl", "", "");

        // Load IPC service
        if (bRet) bRet = fnLoadModule("ipc_com.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("CommunicationControl", "", "");

        // Load IPC service and create listener local connections
        if (bRet) bRet = fnLoadModule("ipc_listener.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("ConnectionListenerService", "", R"code([Listener]
Type = "Local"
)code");

        // Load IPC service
        if (bRet) bRet = fnLoadModule("ipc_connect.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("ConnectionService", "", "");

        // Load proxy/stub for core interfaces
        if (bRet) bRet = fnLoadModule("core_ps.sdv") ? true : false;

//        // Start the listener
//        if (bRet) bRet = fnLoadModule("ipc_listener.sdv") ? true : false;
//        if (bRet) bRet = GetRepository().CreateObject("ConnectionListenerService", "ConnectionListenerService", R"code([Listener]
//Type="local"
//Instance=)code" + std::to_string(GetInstanceID()));

        if (!bRet)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to load IPC server components. Cannot continue!" << std::endl;
            if (!ssErrorString.empty())
                std::cerr << "REASON: " << ssErrorString << std::endl;
            Shutdown(true);
            return false;
        }
    }
    else if (bRet && bLoadRPCClient)
    {
        // Interpret the connect string...


        // Load hardware identification
        bRet = fnLoadModule("hardware_ident.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("HardwareIdentificationService", "", "");

        // Load shared memory channel
        if (bRet) bRet = fnLoadModule("ipc_shared_mem.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("DefaultSharedMemoryChannelControl", "", "");

        // Load IPC service
        if (bRet) bRet = fnLoadModule("ipc_com.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("CommunicationControl", "", "");

        // Load IPC service
        if (bRet) bRet = fnLoadModule("ipc_connect.sdv") ? true : false;
        if (bRet) bRet = fnCreateObject("ConnectionService", "", "");

        // Load proxy/stub for core interfaces
        if (bRet) bRet = fnLoadModule("core_ps.sdv") ? true : false;

        if (!bRet)
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to load IPC client components. Cannot continue!" << std::endl;
            Shutdown(true);
            return false;
        }
    }

    if (!bRet)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to start core components. Cannot continue!" << std::endl;
        if (!ssErrorString.empty())
            std::cerr << "REASON: " << ssErrorString << std::endl;
        Shutdown(true);
        return false;
    }

    // Register the exit handler to force a proper shutdown in case the application exits without a call to shutdown.
    std::atexit(ExitHandler);

    BroadcastOperationState(sdv::app::EAppOperationState::initialized);

    SetConfigMode();

    if (IsMainApplication())
    {
        // Load system configs - they need to be present completely
        for (const std::filesystem::path& rpathConfig : m_vecSysConfigs)
        {
            sdv::core::EConfigProcessResult eResult = GetAppConfig().LoadConfig(rpathConfig.generic_u8string());
            if (eResult != sdv::core::EConfigProcessResult::successful)
            {
                if (!m_bSilent)
                    std::cerr << "ERROR: Cannot load or partly load the system configuration: " <<
                    rpathConfig.generic_u8string() << std::endl;
                Shutdown(true);
                return false;
            }
        }

        // The system configs should not be stored once more.
        GetAppConfig().ResetConfigBaseline();

        // Load the application config - they can also be partly there
        if (!m_pathAppConfig.empty())
        {
            sdv::core::EConfigProcessResult eResult = GetAppConfig().LoadConfig(m_pathAppConfig.generic_u8string());
            if (eResult == sdv::core::EConfigProcessResult::failed)
            {
                if (!m_bSilent)
                    std::cerr << "ERROR: Cannot load application configuration: " << m_pathAppConfig.generic_u8string() << std::endl;
                Shutdown(true);
                return false;
            }
            else if (eResult != sdv::core::EConfigProcessResult::partially_successful)
            {
                if (!m_bSilent)
                    std::cerr << "WARNING: Partially could not load application configuration: " <<
                    m_pathAppConfig.generic_u8string() << std::endl;
            }
        }
    }

    SetRunningMode();

    return true;
}

void CAppControl::RunLoop()
{
    // Only allowed to run during configuration and running mode.
    switch (m_eState)
    {
    case sdv::app::EAppOperationState::configuring :
    case sdv::app::EAppOperationState::running :
        break;
    default:
        throw sdv::XInvalidState();
    }

    // Treat local running differently from main and isolated app running.
    // Note... Maintenance apps should not use the loop
    bool bLocal = true;
    switch (m_eContextMode)
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
        bLocal = false;
        break;
    case sdv::app::EAppContext::maintenance:
        throw sdv::XAccessDenied();
        break;
    default:
        bLocal = true;
        break;
    }

    CShutdownRequestListener listener;
    if (bLocal)
    {
        listener = std::move(CShutdownRequestListener(m_uiInstanceID));
        if (!listener.IsValid())
            throw sdv::XAccessDenied(); // Another instance is already running.
    }

    if (m_bVerbose)
        std::cout << "Entering loop" << std::endl;

    m_bRunLoop = true;
    while (m_bRunLoop)
    {
        // Check and wait 2 milliseconds.
        if (bLocal)
            m_bRunLoop = !listener.HasTriggered(2);
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

        // Inform about running.
        if (m_pEvent)
        {
            sdv::app::SAppEvent sEvent{};
            sEvent.uiEventID = sdv::app::EVENT_RUNNING_LOOP;
            sEvent.uiInfo = 0u;
            m_pEvent->ProcessEvent(sEvent);
        }
    }
    if (m_bVerbose)
        std::cout << "Leaving loop" << std::endl;
}

void CAppControl::Shutdown(/*in*/ bool bForce)
{
    if (m_eState == sdv::app::EAppOperationState::not_started) return;

    // Shutdown the running loop
    if (m_bRunLoop)
    {
        m_bRunLoop = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    BroadcastOperationState(sdv::app::EAppOperationState::shutting_down);

    // Set the config state for all objects
    GetRepository().SetConfigMode();

    // Disable automatic configuration saving.
    m_bAutoSaveConfig = false;

    // Destroy all objects... this should also remove any registered services, except the custom logger.
    GetRepository().DestroyAllObjects(std::vector<std::string>({ m_ssLoggerClass }), bForce);

    // Unload all modules... this should destroy all running services, except the custom logger
    GetModuleControl().UnloadAll(std::vector<sdv::core::TModuleID>({ m_tLoggerModuleID }));

    // Remove all installation manifests
    GetAppConfig().UnloadInstallatonManifests();

    // End logging
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    if (bForce) SDV_LOG_INFO("Forced shutdown of application.");
    SDV_LOG_INFO("SDV Application end at ", std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S %Z"));
    GetLoggerControl().SetLogger(nullptr);

    // Unload all modules... this should also destroy the custom logger.
    GetModuleControl().UnloadAll(std::vector<sdv::core::TModuleID>());

    // Release the lock on the instance (only for main application)
    if (m_pLockFile)
    {
#ifdef __unix__
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
        lockf(fileno(m_pLockFile), F_ULOCK, 0);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif
        fclose(m_pLockFile);
        m_pLockFile = nullptr;
        try
        {
            if (std::filesystem::exists(m_pathLockFile))
                std::filesystem::remove(m_pathLockFile);
        } catch (const std::filesystem::filesystem_error&)
        {}
        m_pathLockFile.clear();
    }

    // End trace streaming
    if (m_eContextMode == sdv::app::EAppContext::main)
    {
        std::cout << "**********************************************" << std::endl;

        // Shutdown log streaming
        m_fifoTraceStreamBuffer.Close();
    }

    BroadcastOperationState(sdv::app::EAppOperationState::not_started);
    m_eContextMode = sdv::app::EAppContext::no_context;
    m_uiInstanceID = 0u;
    m_pEvent = nullptr;
    m_ssLoggerClass.clear();
    m_tLoggerModuleID = 0;
    m_pathLoggerModule.clear();
    m_ssProgramTag.clear();
    m_eSeverityFilter = sdv::core::ELogSeverity::info;
    m_pathInstallDir.clear();
    m_pathRootDir.clear();
    m_vecSysConfigs.clear();
    m_pathAppConfig.clear();
    m_bAutoSaveConfig = false;
    m_bEnableAutoSave = false;
    m_bSilent = false;
    m_bVerbose = false;
}

void CAppControl::RequestShutdown()
{
    SDV_LOG_INFO("Shutdown request received.");
    m_bRunLoop = false;
}

sdv::app::EAppOperationState CAppControl::GetOperationState() const
{
    return m_eState;
}

uint32_t CAppControl::GetInstance() const
{
    return m_uiInstanceID;
}

void CAppControl::SetConfigMode()
{
    GetRepository().SetConfigMode();
    if (m_eState == sdv::app::EAppOperationState::running || m_eState == sdv::app::EAppOperationState::initialized)
        BroadcastOperationState(sdv::app::EAppOperationState::configuring);
}

void CAppControl::SetRunningMode()
{
    GetRepository().SetRunningMode();
    if (m_eState == sdv::app::EAppOperationState::configuring || m_eState == sdv::app::EAppOperationState::initialized)
        BroadcastOperationState(sdv::app::EAppOperationState::running);
}

sdv::sequence<sdv::u8string> CAppControl::GetNames() const
{
    sdv::sequence<sdv::u8string> seqNames = {"app.instance_id", "console.info_level"};
    return seqNames;
}

sdv::any_t CAppControl::Get(/*in*/ const sdv::u8string& ssAttribute) const
{
    if (ssAttribute == "app.instance_id") return sdv::any_t(m_uiInstanceID);
    if (ssAttribute == "console.info_level")
    {
        if (m_bSilent) return "silent";
        if (m_bVerbose) return "verbose";
        return "normal";
    }
    return {};
}

bool CAppControl::Set(/*in*/ const sdv::u8string& /*ssAttribute*/, /*in*/ sdv::any_t /*anyAttribute*/)
{
    // Currently there are not setting attributes...
    return false;
}

uint32_t CAppControl::GetFlags(/*in*/ const sdv::u8string& ssAttribute) const
{
    if (ssAttribute == "app.instance_id") return hlpr::flags<sdv::EAttributeFlags>(sdv::EAttributeFlags::read_only);
    if (ssAttribute == "console.info_level") return hlpr::flags<sdv::EAttributeFlags>(sdv::EAttributeFlags::read_only);
    return 0u;
}

std::filesystem::path CAppControl::GetInstallDir() const
{
    return m_pathInstallDir;
}

void CAppControl::DisableAutoConfigUpdate()
{
    m_bEnableAutoSave = false;
}

void CAppControl::EnableAutoConfigUpdate()
{
    m_bEnableAutoSave = true;
}

void CAppControl::TriggerConfigUpdate()
{
    if (!m_bAutoSaveConfig || !m_bEnableAutoSave) return;
    if (m_pathAppConfig.empty()) return;

    if (!GetAppConfig().SaveConfig(m_pathAppConfig.generic_u8string()))
        SDV_LOG_ERROR("Failed to automatically save the configuration ", m_pathAppConfig.generic_u8string());
}

bool CAppControl::IsConsoleSilent() const
{
    return m_bSilent;
}

bool CAppControl::IsConsoleVerbose() const
{
    return m_bVerbose;
}

void CAppControl::BroadcastOperationState(sdv::app::EAppOperationState eState)
{
    m_eState = eState;
    if (m_pEvent)
    {
        sdv::app::SAppEvent sEvent{};
        sEvent.uiEventID = sdv::app::EVENT_OPERATION_STATE_CHANGED;
        sEvent.uiInfo = static_cast<uint32_t>(eState);
        m_pEvent->ProcessEvent(sEvent);
    }
}

bool CAppControl::ProcessAppConfig(const sdv::u8string& rssConfig)
{
    CParserTOML parserConfig;
    std::string ssError;
    try
    {
        // Read the configuration
        if (!parserConfig.Process(rssConfig)) return false;

    } catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        ssError = std::string("ERROR: Failed to parse application configuration: ") + rexcept.what();
    }

    // Get the reporting settings (if this succeeded at all...)
    auto ptrReport = parserConfig.GetRoot().GetDirect("Console.Report");
    if (ptrReport && ptrReport->GetValue() == "Silent") m_bSilent = true;
    if (ptrReport && ptrReport->GetValue() == "Verbose") m_bVerbose = true;

    // Report the outstanding error (if there is one...)
    if (!ssError.empty())
    {
        if (!m_bSilent)
            std::cerr << ssError << std::endl;
        return false;
    }

    // Allow a custom logger to be defined
    std::filesystem::path pathLoggerModule;
    std::string ssLoggerClass;
    std::shared_ptr<CNode> ptrLogHandlerPath = parserConfig.GetRoot().GetDirect("LogHandler.Path");
    std::shared_ptr<CNode> ptrLogHandlerClass = parserConfig.GetRoot().GetDirect("LogHandler.Class");
    if (ptrLogHandlerPath && !ptrLogHandlerClass)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to process application log: custom logger handler module path supplied, but no class "
                "defined!" << std::endl;
        return false;
    }
    if (!ptrLogHandlerPath && ptrLogHandlerClass)
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to process application log: custom logger handler class supplied, but no module "
                "defined!" << std::endl;
        return false;
    }
    if (ptrLogHandlerPath)
    {
        m_pathLoggerModule = static_cast<std::string>(ptrLogHandlerPath->GetValue());
        m_ssLoggerClass = static_cast<std::string>(ptrLogHandlerClass->GetValue());
    } else
    {
        // Default logger
        m_ssLoggerClass = "DefaultLoggerService";
    }

    // Get an optional program tag for the logger
    std::shared_ptr<CNode> ptrLogPogramTag = parserConfig.GetRoot().GetDirect("LogHandler.Tag");
    if (ptrLogPogramTag) m_ssProgramTag = static_cast<std::string>(ptrLogPogramTag->GetValue());

    // Get the application-mode
    std::string ssApplication = "Standalone";
    std::shared_ptr<CNode> ptrApplication = parserConfig.GetRoot().GetDirect("Application.Mode");
    if (ptrApplication) ssApplication = static_cast<std::string>(ptrApplication->GetValue());
    if (ssApplication == "Standalone") m_eContextMode = sdv::app::EAppContext::standalone;
    else if (ssApplication == "External") m_eContextMode = sdv::app::EAppContext::external;
    else if (ssApplication == "Isolated") m_eContextMode = sdv::app::EAppContext::isolated;
    else if (ssApplication == "Main") m_eContextMode = sdv::app::EAppContext::main;
    else if (ssApplication == "Essential") m_eContextMode = sdv::app::EAppContext::essential;
    else if (ssApplication == "Maintenance") m_eContextMode = sdv::app::EAppContext::maintenance;
    else
    {
        if (!m_bSilent)
            std::cerr << "ERROR: Failed to process startup config: invalid application-mode specified for core library: " <<
            ssApplication << std::endl;
        return false;
    }

    // Get the severity level filter for the logger
    auto fnTranslateSevFilter = [this](const std::string& rssLogFilter, const sdv::core::ELogSeverity eDefault)
    {
        sdv::core::ELogSeverity eSeverityFilter = eDefault;
        if (rssLogFilter == "Trace") eSeverityFilter = sdv::core::ELogSeverity::trace;
        else if (rssLogFilter == "Debug") eSeverityFilter = sdv::core::ELogSeverity::debug;
        else if (rssLogFilter == "Info") eSeverityFilter = sdv::core::ELogSeverity::info;
        else if (rssLogFilter == "Warning") eSeverityFilter = sdv::core::ELogSeverity::warning;
        else if (rssLogFilter == "Error") eSeverityFilter = sdv::core::ELogSeverity::error;
        else if (rssLogFilter == "Fatal") eSeverityFilter = sdv::core::ELogSeverity::fatal;
        else if (!rssLogFilter.empty())
        {
            if (!m_bSilent)
                std::cerr << "ERROR: Failed to process application log: invalid severity level filter: '" << rssLogFilter <<
                "'" << std::endl;
        }
        return eSeverityFilter;
    };
    sdv::core::ELogSeverity eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::error;
    if (IsMainApplication() || IsIsolatedApplication())
        eLogDefaultViewSeverityFilter = sdv::core::ELogSeverity::info;
    std::shared_ptr<CNode> ptrLogSeverityFilter = parserConfig.GetRoot().GetDirect("LogHandler.Filter");
    m_eSeverityFilter = fnTranslateSevFilter(ptrLogSeverityFilter ? ptrLogSeverityFilter->GetValue() : "",
        sdv::core::ELogSeverity::info);
    ptrLogSeverityFilter = parserConfig.GetRoot().GetDirect("LogHandler.ViewFilter");
    m_eSeverityViewFilter = fnTranslateSevFilter(ptrLogSeverityFilter ? ptrLogSeverityFilter->GetValue() : "",
        eLogDefaultViewSeverityFilter);

    // Get the optional instance ID.
    std::shared_ptr<CNode> ptrInstance = parserConfig.GetRoot().GetDirect("Application.Instance");
    if (ptrInstance)
        m_uiInstanceID = ptrInstance->GetValue();
    else
        m_uiInstanceID = 1000u;
    // Number of attempts to establish a connection to a running instance.
    std::shared_ptr<CNode> ptrRetries = parserConfig.GetRoot().GetDirect("Application.Retries");
    if (ptrRetries)
    {
        m_uiRetries = ptrRetries->GetValue();
        if (m_uiRetries > 30)
            m_uiRetries = 30;
        else if (m_uiRetries < 3)
            m_uiRetries = 3;
    }

    // Main and isolated apps specific information.
    if (IsMainApplication() || IsIsolatedApplication())
    {
        // Get the optional installation directory.
        std::shared_ptr<CNode> ptrInstallDir = parserConfig.GetRoot().GetDirect("Application.InstallDir");
        if (ptrInstallDir)
        {
            m_pathRootDir = ptrInstallDir->GetValue();
            if (m_pathRootDir.is_relative())
                m_pathRootDir = GetExecDirectory() / m_pathRootDir;
        }
        else
            m_pathRootDir = GetExecDirectory() / std::to_string(m_uiInstanceID);
        m_pathInstallDir = m_pathRootDir;
        try
        {
            std::filesystem::create_directories(m_pathRootDir);
        }
        catch (const std::filesystem::filesystem_error& rexcept)
        {
            if (!m_bSilent)
            {
                std::cerr << "Cannot create root directory: " << m_pathRootDir << std::endl;
                std::cerr << "  Reason: " << rexcept.what() << std::endl;
            }
            return false;
        }
    }

    // Maintenance and isolated applications cannot load specific configs. The others can specify a configuration file, but
    // not auto-updateable.
    if (!IsMaintenanceApplication() && !IsIsolatedApplication())
    {
        auto ptrConfigFile = parserConfig.GetRoot().GetDirect("Application.Config");
        if (ptrConfigFile)
            m_pathAppConfig = ptrConfigFile->GetValue();
    }

    // Read the settings... if existing. And only for the main application
    if (IsMainApplication())
    {
        // If the template is not existing, create the template...
        if (!std::filesystem::exists(m_pathRootDir / "settings.toml"))
        {
            std::ofstream fstream(m_pathRootDir / "settings.toml");
            if (!fstream.is_open())
            {
                if (!m_bSilent)
                    std::cerr << "ERROR: Failed to store application settings." << std::endl;
                return false;
            }
            fstream << szSettingsTemplate;
            fstream.close();
        }
        else
        {
            std::ifstream fstream(m_pathRootDir / "settings.toml");
            std::string ssContent((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
            try
            {
                // Read the configuration
                CParserTOML parserSettings(ssContent);

                // Check for the version
                auto ptrVersion = parserSettings.GetRoot().GetDirect("Settings.Version");
                if (!ptrVersion)
                {
                    if (!m_bSilent)
                        std::cerr << "ERROR: Missing version in application settings file." << std::endl;
                    return false;
                }
                if (ptrVersion->GetValue() != SDVFrameworkInterfaceVersion)
                {
                    if (!m_bSilent)
                        std::cerr << "ERROR: Invalid version of application settings file (expected version " <<
                            SDVFrameworkInterfaceVersion << ", but available version " <<
                            static_cast<uint32_t>(ptrVersion->GetValue()) << ")" << std::endl;
                    return false;
                }

                // Read the system configurations
                auto ptrSystemConfigs = parserSettings.GetRoot().GetDirect("Settings.SystemConfig");
                if (ptrSystemConfigs && ptrSystemConfigs->GetArray())
                {
                    for (uint32_t uiIndex = 0; uiIndex < ptrSystemConfigs->GetArray()->GetCount(); uiIndex++)
                    {
                        auto ptrSystemConfig = ptrSystemConfigs->GetArray()->Get(uiIndex);
                        if (!ptrSystemConfig) continue;
                        m_vecSysConfigs.push_back(static_cast<std::string>(ptrSystemConfig->GetValue()));
                    }
                }

                // Get the application config - but only when not specified over the app-control-config.
                if (m_pathAppConfig.empty())
                {
                    auto ptrAppConfig = parserSettings.GetRoot().GetDirect("Settings.AppConfig");
                    if (ptrAppConfig)
                    {
                        // Path available. Enable auto-update.
                        m_pathAppConfig = static_cast<std::string>(ptrAppConfig->GetValue());
                        m_bAutoSaveConfig = true;
                    }
                }
            }
            catch (const sdv::toml::XTOMLParseException& rexcept)
            {
                if (!m_bSilent)
                    std::cerr << "ERROR: Failed to parse application settings: " << rexcept.what() << std::endl;
                return false;
            }
        }
    }

    return true;
}

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

CAppControl& CAppControlService::GetAppControl()
{
    return ::GetAppControl();
}

bool CAppControlService::EnableAppShutdownRequestAccess() const
{
    return ::GetAppControl().IsMainApplication() || ::GetAppControl().IsIsolatedApplication();
}

#endif
