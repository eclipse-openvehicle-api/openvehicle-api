#include "startup_shutdown.h"
#include "../../global/cmdlnparser/cmdlnparser.h"
#include <interfaces/process.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <interfaces/com.h>
#include <interfaces/app.h>
#include "../error_msg.h"

sdv::process::TProcessID g_tServerProcessID = 0;

/**
 * @brief Process lifetime monitor struct
 */
struct SProcessLifetimeCallback : sdv::IInterfaceAccess, sdv::process::IProcessLifetimeCallback
{
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::process::IProcessLifetimeCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Called when the process was terminated.
     * @remarks The process return value is not always valid. The validity depends on the support of the underlying system.
     * @param[in] tProcessID The process ID of the process being terminated.
     * @param[in] iRetValueParam Process return value or 0 when not supported.
     */
    virtual void ProcessTerminated([[maybe_unused]] /*in*/ sdv::process::TProcessID tProcessID, /*in*/ int64_t iRetValueParam) override
    {
        iRetValue = iRetValueParam;
    }

    /**
     * @brief Get the process return value or 0 when no value was set.
     * @return The process return value.
     */
    int64_t GetRetValue() const
    {
        return iRetValue;
    }

    int64_t     iRetValue = 0;      ///< Process return value to be updated by the process monitor.
};

void StartupShutdownHelp(const SContext& rsContext)
{
    // First and only argument should be "STARTUP" or "SHUTDOWN".
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing startup/shutdown command..." << std::endl;
        return;
    }
    if (rsContext.seqCmdLine.size() > 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: too many commands following startup/shutdown command..." << std::endl;
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "STARTUP"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control STARTUP [options...]\n\n"
            "Start the SDV server with the supplied instance ID. If no instance ID is supplied, use the default instance "
            "ID #1000.\n\n"
            "Options:\n"
            " --server_silent   Server is started using silent option. Not compatible with 'server_verbose'.\n"
            " --server_verbose  Server is started using verbose option. Not compatible with 'server_silent'.\n"
            " --install_dir     Installation directory (absolute or relative to the sdv_core executable).\n\n");
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "SHUTDOWN"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control SHUTDOWN [options...]\n\nStop the SDV server with the supplied "
            "instance ID. If no instance ID is supplied, use the default instance ID #1000.\n\n");
        return;
    }
    if (!rsContext.bSilent)
        std::cerr << "ERROR: invalid startup/shutdown command..." << std::endl;
}

int StartupSDVServer(const SContext& rsContext)
{
    if (rsContext.seqCmdLine.size() != 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid arguments following STARTUP command." << std::endl;
        return CMDLN_ARG_ERR;
    }

    if (!rsContext.bSilent)
        std::cout << "Starting the SDV Server..." << std::endl;

    // Get the process control service
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    sdv::process::IProcessLifetime* pProcessLifetime = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    if (!pProcessControl || !pProcessLifetime)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << PROCESS_CONTROL_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return PROCESS_CONTROL_SERVICE_ACCESS_ERROR;
    }

    sdv::sequence<sdv::u8string> seqArgTemp;
    if (rsContext.bVerbose)
        std::cout << "Instance ID: " << rsContext.uiInstanceID << std::endl;
    seqArgTemp.push_back("-server");
    seqArgTemp.push_back("--instance" + std::to_string(rsContext.uiInstanceID));
    seqArgTemp.push_back("--no_banner");
    if (rsContext.bServerSilent) seqArgTemp.push_back("--silent");
    if (rsContext.bServerVerbose) seqArgTemp.push_back("--verbose");
    if (!rsContext.pathInstallDir.empty())
        seqArgTemp.push_back("--install_dir\"" + rsContext.pathInstallDir.generic_u8string() + "\"");
    g_tServerProcessID = pProcessControl->Execute("sdv_core", seqArgTemp, sdv::process::EProcessRights::parent_rights);
    if (!g_tServerProcessID)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << STARTUP_CORE_ERROR_MSG << std::endl;
        return STARTUP_CORE_ERROR;
    }

    // Register the process monitor...
    SProcessLifetimeCallback sProcessLifetimeCallback;
    uint32_t uiCookie = pProcessLifetime->RegisterMonitor(g_tServerProcessID, &sProcessLifetimeCallback);
    if (!uiCookie)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << REGISTER_PROCESS_MONITOR_ERROR_MSG << std::endl;
        return REGISTER_PROCESS_MONITOR_ERROR;
    }

    if (rsContext.bVerbose)
        std::cout << "Process ID: " << g_tServerProcessID << std::endl;

    // Wait for 3 seconds to see if the process terminates... (which should not occur).
    bool bTerminated = pProcessLifetime->WaitForTerminate(g_tServerProcessID, 3000);
    pProcessLifetime->UnregisterMonitor(uiCookie);
    if (bTerminated)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CORE_NOT_STARTED_MSG << " Instance: #" << rsContext.uiInstanceID << " Exit code: " <<
                sProcessLifetimeCallback.iRetValue << std::endl;
        return CORE_NOT_STARTED;
    }

    // Try to connect
    auto ptrRepository = sdv::com::ConnectToLocalServerRepository(rsContext.uiInstanceID);
    if (!ptrRepository)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << " SDV Server instance #:" << rsContext.uiInstanceID <<
                std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }

    if (!rsContext.bSilent)
        std::cout << "SDV Server #" << rsContext.uiInstanceID << " has successfully started..." << std::endl;
    return NO_ERROR;
}

int ShutdownSDVServer(const SContext& rsContext)
{
    if (rsContext.seqCmdLine.size() != 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid arguments following SHUTDOWN command." << std::endl;
        return CMDLN_ARG_ERR;
    }

    if (!rsContext.bSilent)
        std::cout << "Connecting to the SDV #" << rsContext.uiInstanceID << " server..." << std::endl;

    // Try to connect
    auto ptrRepository = sdv::com::ConnectToLocalServerRepository(rsContext.uiInstanceID);
    if (!ptrRepository)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }

    if (!rsContext.bSilent)
        std::cout << "Requesting shutdown..." << std::endl;
    sdv::core::IObjectAccess* pObjAccess = ptrRepository.GetInterface<sdv::core::IObjectAccess>();
    sdv::app::IAppShutdownRequest* pShutdownRequest = nullptr;
    if (pObjAccess)
        pShutdownRequest = sdv::TInterfaceAccessPtr(pObjAccess->GetObject("AppControlService")).
            GetInterface<sdv::app::IAppShutdownRequest>();
    if (pShutdownRequest)
        pShutdownRequest->RequestShutdown();
    ptrRepository.Clear();
    if (!pShutdownRequest)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return APP_CONTROL_SERVICE_ACCESS_ERROR;
    }

    if (!rsContext.bSilent)
        std::cout << "Shutdown initiated..." << std::endl;
    return NO_ERROR;
}

sdv::process::TProcessID GetServerProcessID()
{
    return g_tServerProcessID;
}
