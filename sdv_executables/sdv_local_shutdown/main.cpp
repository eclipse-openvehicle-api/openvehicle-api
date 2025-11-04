#include "../../global/process_watchdog.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/exec_dir_helper.h"
#include "../../sdv_services/core/local_shutdown_request.h"
#include "../error_msg.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int iArgc, const wchar_t* rgszArgv[])
#else
extern "C" int main(int iArgc, const char* rgszArgv[])
#endif
{
    // Workaround for GCC to make certain that POSIX thread library is loaded before the components are loaded.
    // REASON: If the first call to a thread is done in a dynamic library, the application is already classified as single
    // threaded and a termination is initiated.
    // See: https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-leads-to-a-sigsev
    // NOTE EVE 27.05.2025: This task has been taken over by the process watchdog.
    CProcessWatchdog watchdog;

    // If not set, set the runtime location to the EXE directory.
    if (sdv::app::CAppControl::GetFrameworkRuntimeDirectory().empty())
        sdv::app::CAppControl::SetFrameworkRuntimeDirectory(GetExecDirectory());
    if (sdv::app::CAppControl::GetComponentInstallDirectory().empty())
        sdv::app::CAppControl::SetComponentInstallDirectory(GetExecDirectory());

    CCommandLine cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    bool bHelp = false;
    bool bError = false;
    bool bSilent = false;
    bool bVerbose = false;
    bool bVersion = false;
    uint32_t uiInstanceID = 1000;   // Default instance is 1000
    std::string ssArgError;
    try
    {
        auto& rArgHelpDef = cmdln.DefineOption("?", bHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("version", bVersion, "Show version information.");
        cmdln.DefineSubOption("instance", uiInstanceID, "The instance ID of the SDV instance (default ID is 1000).");
        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        ssArgError = rsExcept.what();
        bHelp = true;
        bError = true;
    }

    if (!bSilent)
    {
        std::cout << "SDV Local Instance Shutdown Utility" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    if (!ssArgError.empty())
        std::cerr << "ERROR: " << ssArgError << std::endl;

    if (bHelp)
    {
        if (!bSilent)
        {
            if (bError)
                std::cout << std::endl;
            cmdln.PrintHelp(std::cout, "Request a shutdown for a local instance (sdv_core started as standalone).\n");
        }
        return bError ? CMDLN_ARG_ERR : NO_ERROR;
    }
    if (bError) return CMDLN_ARG_ERR;

    if (bVersion || bVerbose)
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl;

    if (bVerbose)
        std::cout << "Trying to request a shutdown for instance with ID #" << uiInstanceID << "..." << std::endl;

    bool bRet = RequestShutdown(uiInstanceID);

    if (!bRet)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }
    if (!bSilent)
    {
        std::cout << "Shutdown request was sent..." << std::endl;
        std::cout << std::endl << "Done..." << std::endl;
    }

    return NO_ERROR;
}