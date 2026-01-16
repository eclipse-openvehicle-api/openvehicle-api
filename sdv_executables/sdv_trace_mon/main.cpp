#include <atomic>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/tracefifo/trace_fifo.cpp"
#include "../../global/exec_dir_helper.h"
#include "../error_msg.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <signal.h>
#endif

/**
 * @brief Add a slag to the string.
 * @param[in] rssString The string to add a flag to.
 * @param[in] rssFlag The flag text to add.
 */
void AddFlagString(std::string& rssString, const std::string& rssFlag)
{
    if (!rssString.empty()) rssString += " | ";
    rssString += rssFlag;
}

/**
 * @brief Application shutdown signalled
 */
bool bShutdownSignalled = false;

#ifdef _WIN32
/**
 * @brief Console application control handler (CTRL+C, close, shutdown, etc.).
 * @param[in] dwCtrlType The control type triggered by the system.
 * @return Returns TRUE when the signal was handled.
 */
static BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        bShutdownSignalled = true;
        return TRUE;
        break;
    default:
        return FALSE;
    }
}
#elif defined __unix__
/**
 * @brief Signal handler to catch signal eevents.
 * @param[in] iSigNum The signal type that was triggered.
 */
static void SignalHandler(int iSigNum)
{
    if (iSigNum == SIGINT)
        bShutdownSignalled = true;
}
#endif

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
    // NOTE EVE 27.05.2025: in release builds, starting and ending the thread right after each other causes incorrect behavior and
    // leads in some cases to create a deadlock in the join-function. The solution is to add delays in the thread processing.
    std::atomic_bool bThreadStarted = false;
    std::thread thread = std::thread([&]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            bThreadStarted = true;
        });
    while (!bThreadStarted) {std::this_thread::sleep_for(std::chrono::milliseconds(100));}
    if (thread.joinable())
        thread.join();

    // If not set, set the runtime location to the EXE directory.
    if (sdv::app::CAppControl::GetFrameworkRuntimeDirectory().empty())
        sdv::app::CAppControl::SetFrameworkRuntimeDirectory(GetExecDirectory());
    if (sdv::app::CAppControl::GetComponentInstallDirectory().empty())
        sdv::app::CAppControl::SetComponentInstallDirectory(GetExecDirectory());

    CCommandLine cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    bool bHelp = false;
    bool bVersion = false;
    uint32_t uiInstanceID = 1000u;
    std::string ssArgError;
    try
    {
        auto& rArgDef = cmdln.DefineOption("?", bHelp, "Show help. Use <COMMAND> --help for specific help on the command.");
        rArgDef.AddSubOptionName("help");
        cmdln.DefineSubOption("version", bVersion, "Show version information.");
        cmdln.DefineSubOption("instance", uiInstanceID, "The instance ID of the SDV instance (default ID is 1000).");

        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        ssArgError = rsExcept.what();
        bHelp = true;
    }

    std::cout << "SDV Server Log Monitor Utility" << std::endl;
    std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
    std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;

    if (bVersion)
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl;
    if (bHelp)
    {
        cmdln.PrintHelp(std::cout, "Monitor log messages from a sdv-server.\n");
        return NO_ERROR;
    }
    std::cout << "Monitoring instance #" << uiInstanceID << std::endl;

    CTraceFifoReader reader(uiInstanceID);

#ifdef _WIN32
    // Register the console control handler
    if (!SetConsoleCtrlHandler(&ControlHandler, TRUE))
    {
        std::cerr << "ERROR: " << TRACE_MON_REG_HNDLR_ERROR_MSG << std::endl;
        return TRACE_MON_REG_HNDLR_ERROR;
    }
#elif defined __unix__
    // Install our signal handler
    struct sigaction sSigAction{};
    sSigAction.sa_handler = SignalHandler;
    sSigAction.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sSigAction, NULL) == -1)
    {
        std::cerr << "ERROR: " << TRACE_MON_REG_HNDLR_ERROR_MSG << std::endl;
        return TRACE_MON_REG_HNDLR_ERROR;
    }
#endif

    // Open the reader.
    if (!reader.Open())
    {
        std::cerr << "ERROR: " << TRACE_MON_FIFO_OPEN_ERROR_MSG << std::endl;
        return TRACE_MON_FIFO_OPEN_ERROR;
    }

    // Trace messages
    while (!bShutdownSignalled)
    {
        std::string ssMsg = reader.WaitForMessage(250);
        if (!ssMsg.empty())
            std::cout << ssMsg;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(&ControlHandler, FALSE);
#endif

    reader.Close();
    std::cout << "Done..." << std::endl;

    return NO_ERROR;
}