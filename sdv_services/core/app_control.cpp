/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include "app_control.h"
#include "module_control.h"
#include "repository.h"
#include "../../global/base64.h"
#include "../../global/tracefifo/trace_fifo.cpp"
#include "toml_parser/parser_toml.h"
#include "local_shutdown_request.h"
#include "app_settings.h"
#include "logger_control.h"
#include "app_config.h"

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

CAppControl& GetAppControl()
{
    static CAppControl app_control;
    return app_control;
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
    bool bRet = GetAppSettings().ProcessAppStartupConfig(ssConfig);

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
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Open the stream buffer and attach the streams if the application control is initialized as main app.
    if (GetAppSettings().IsMainApplication())
    {
        m_fifoTraceStreamBuffer.SetInstanceID(GetAppSettings().GetInstanceID());
        m_fifoTraceStreamBuffer.Open(1000);
        std::cout << "**********************************************" << std::endl;
    }

    // Stream the starting logging to the output streams...
    std::cout << sstreamCOUT.str();
    std::clog << sstreamCLOG.str();
    std::cerr << sstreamCERR.str();

    // Check for a correctly opened stream buffer
    if (GetAppSettings().IsMainApplication() && !m_fifoTraceStreamBuffer.IsOpened())
    {
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Log streaming could not be initialized; cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Allow only one instance if running as main application
    if (GetAppSettings().IsMainApplication())
    {
        m_pathLockFile = GetExecDirectory() / ("sdv_core_" + std::to_string(GetAppSettings().GetInstanceID()) + ".lock");
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
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Failed to load the Core Services. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Load the logger module if one is specified
    if (!GetAppSettings().GetLoggerModulePath().empty())
    {
        m_tLoggerModuleID = fnLoadModule(GetAppSettings().GetLoggerModulePath().u8string());
        if (!m_tLoggerModuleID)
        {
            if (!GetAppSettings().IsConsoleSilent())
                std::cerr << "ERROR: Failed to load the custom logger. Cannot continue!" << std::endl;
            Shutdown(true);
            return false;
        }
    }

    // Start the logger and assign it to the logger control.
    fnCreateObject(GetAppSettings().GetLoggerClass(), GetAppSettings().GetLoggerClass(), "");
    sdv::IInterfaceAccess* pLoggerObj = GetRepository().GetObject(GetAppSettings().GetLoggerClass());
    if (!pLoggerObj)
    {
        GetRepository().DestroyObject2(GetAppSettings().GetLoggerClass());
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Failed to start the logger. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    sdv::core::ILoggerConfig* pLoggerConfig = pLoggerObj->GetInterface<sdv::core::ILoggerConfig>();
    sdv::core::ILogger* pLogger = pLoggerObj->GetInterface<sdv::core::ILogger>();
    if (!pLoggerConfig || !pLogger)
    {
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Failed to start the logger. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }
    if (!GetAppSettings().GetLoggerProgramTag().empty())
        pLoggerConfig->SetProgramTag(GetAppSettings().GetLoggerProgramTag());
    pLoggerConfig->SetLogFilter(GetAppSettings().GetLoggerSeverityFilter(), GetAppSettings().GetConsoleSeverityFilter());
    GetLoggerControl().SetLogger(pLogger);

    // Create the core service objects
    bRet = fnCreateObject("AppControlService", "AppControlService", "");
    bRet = bRet && fnCreateObject("RepositoryService", "RepositoryService", "");
    bRet = bRet && fnCreateObject("ModuleControlService", "ModuleControlService", "");
    bRet = bRet && fnCreateObject("ConfigService", "ConfigService", "");
    if (!bRet)
    {
        if (!GetAppSettings().IsConsoleSilent())
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
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::standalone:
        break;
    case sdv::app::EAppContext::external:
        bLoadRPCClient = true;
        break;
    case sdv::app::EAppContext::isolated:
        bLoadRPCClient = true;
        break;
    case sdv::app::EAppContext::main:
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
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Invalid Run-As mode. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Load the application settings.
    if (!GetAppSettings().LoadSettingsFile())
    {
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Failed to load application settings file. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
    }

    // Load installation manifests. For standalone applications, only the core manifest is loaded.
    if (!GetAppConfig().LoadInstallationManifests())
    {
        if (!GetAppSettings().IsConsoleSilent())
            std::cerr << "ERROR: Failed to load installation manifests. Cannot continue!" << std::endl;
        Shutdown(true);
        return false;
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
            if (!GetAppSettings().IsConsoleSilent())
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
            if (!GetAppSettings().IsConsoleSilent())
                std::cerr << "ERROR: Failed to load IPC client components. Cannot continue!" << std::endl;
            Shutdown(true);
            return false;
        }
    }

    if (!bRet)
    {
        if (!GetAppSettings().IsConsoleSilent())
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

    if (GetAppSettings().IsMainApplication())
    {
        // Load system configs - they need to be present completely
        for (const std::filesystem::path& rpathConfig : GetAppSettings().GetSystemConfigPaths())
        {
            sdv::core::EConfigProcessResult eResult = GetAppConfig().LoadConfig(rpathConfig.generic_u8string());
            if (eResult != sdv::core::EConfigProcessResult::successful)
            {
                if (!GetAppSettings().IsConsoleSilent())
                    std::cerr << "ERROR: Cannot load or partly load the system configuration: " <<
                    rpathConfig.generic_u8string() << std::endl;
                Shutdown(true);
                return false;
            }
        }

        // The system configs should not be stored once more.
        GetAppConfig().ResetConfigBaseline();

        // Load the application config - they can also be partly there
        if (!GetAppSettings().GetUserConfigPath().empty())
        {
            sdv::core::EConfigProcessResult eResult =
                GetAppConfig().LoadConfig(GetAppSettings().GetUserConfigPath().generic_u8string());
            if (eResult == sdv::core::EConfigProcessResult::failed)
            {
                if (!GetAppSettings().IsConsoleSilent())
                    std::cerr << "ERROR: Cannot load application configuration: " <<
                        GetAppSettings().GetUserConfigPath().generic_u8string() << std::endl;
                Shutdown(true);
                return false;
            }
            else if (eResult != sdv::core::EConfigProcessResult::partially_successful)
            {
                if (!GetAppSettings().IsConsoleSilent())
                    std::cerr << "WARNING: Partially could not load application configuration: " <<
                        GetAppSettings().GetUserConfigPath().generic_u8string() << std::endl;
            }
            m_bAutoSaveConfig = true;
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
    switch (GetAppSettings().GetContextType())
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
        listener = std::move(CShutdownRequestListener(GetAppSettings().GetInstanceID()));
        if (!listener.IsValid())
            throw sdv::XAccessDenied(); // Another instance is already running.
    }

    if (GetAppSettings().IsConsoleVerbose())
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
    if (GetAppSettings().IsConsoleVerbose())
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

    // Update the application settings file
    GetAppSettings().SaveSettingsFile();

    // Destroy all objects... this should also remove any registered services, except the custom logger.
    GetRepository().DestroyAllObjects(std::vector<std::string>({GetAppSettings().GetLoggerClass()}), bForce);

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
    if (GetAppSettings().IsMainApplication())
    {
        std::cout << "**********************************************" << std::endl;

        // Shutdown log streaming
        m_fifoTraceStreamBuffer.Close();
    }

    BroadcastOperationState(sdv::app::EAppOperationState::not_started);
    m_pEvent = nullptr;
    m_tLoggerModuleID = 0;
    m_bAutoSaveConfig = false;
    m_bEnableAutoSave = false;
    GetAppSettings().Reset();
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
    if (GetAppSettings().GetUserConfigPath().empty())
        return;

    if (!GetAppConfig().SaveConfig(GetAppSettings().GetUserConfigPath().generic_u8string()))
        SDV_LOG_ERROR("Failed to automatically save the configuration ", GetAppSettings().GetUserConfigPath().generic_u8string());
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

bool CAppControlService::EnableAppShutdownRequestAccess() const
{
    return ::GetAppSettings().IsMainApplication() || ::GetAppSettings().IsIsolatedApplication();
}
