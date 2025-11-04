#include <support/sdv_core.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/exec_dir_helper.h"
#include <support/app_control.h>
#include "../error_msg.h"

/**
* @brief Main function of the executable.
* @param[in] iArgc Amount of arguments provided.
* @param[in] rgszArgv Array of C-string pointers. The first entry is the name or path to this executable.
* @return Returns the executable's exit code.
*/
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
    bool bThreadStarted = false;
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
    bool bError = false;
    bool bSilent = false;
    bool bNoBanner = false;
    bool bVerbose = false;
    bool bVersion = false;
    bool bStandalone = false;
    bool bServer = false;
    std::filesystem::path pathConfig;
    uint32_t uiInstanceID = 1000;   // Default instance is 1000
    std::filesystem::path pathInstallDir;
    std::string ssArgError;
    try
    {
        auto& rArgHelpDef = cmdln.DefineOption("?", bHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("no_banner", bNoBanner, "Do not show banner information. Ignored when silent is activated.");
        cmdln.DefineSubOption("version", bVersion, "Show version information.");
        cmdln.DefineSubOption("instance", uiInstanceID, "The instance ID of the SDV instance (default ID is 1000).");
        cmdln.DefineOption("local", bStandalone, "Start the local version (default - no IPC available).");
        cmdln.DefineOption("server", bServer, "Start the server version (not compatible with the local version).");
        cmdln.DefineSubOption("install_dir", pathInstallDir, "Installation directory (absolute or relative to this executable).");
        cmdln.DefineDefaultArgument(pathConfig, "Configuration file to start running (compulsory; applicable for local version only).");
        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        ssArgError = rsExcept.what();
        bHelp = true;
        bError = true;
    }

    if (!bSilent && !bNoBanner)
    {
        std::cout << "SDV core application" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    if (!ssArgError.empty() && !bSilent)
        std::cerr << "ERROR: " << ssArgError << std::endl;

    if (bServer && bStandalone)
    {
        std::cerr << "ERROR: The commandline options -server and -local cannot be supplied at the same time." << std::endl;
        bHelp = true;
        bError = true;
    }
    bStandalone = !bServer;

    if (!pathConfig.empty() && bServer)
    {
        std::cerr << "ERROR: Cannot supply a configuration when running as server." << std::endl;
        bHelp = true;
        bError = true;
    }
    else if (pathConfig.empty() && bStandalone)
    {
        std::cerr << "ERROR: Needing a configuration when running as local application." << std::endl;
        bHelp = true;
        bError = true;
    }

    if (bHelp)
    {
        if (!bSilent)
        {
            if (bError)
                std::cout << std::endl;
            cmdln.PrintHelp(std::cout, "The core application is used to start the service framework for use by applications. The "
                "safe version using component isolation and installation manifests should be started with the -safe option and is "
                "for use in the vehicle. For local simulation the stand-alone version can be used.\n");
        }
        return bError ? CMDLN_ARG_ERR : NO_ERROR;
    }
    if (bError) return CMDLN_ARG_ERR;

    if (bVersion || bVerbose)
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl;
    if (!bSilent)
    {
        std::cout << "Running as: " << (bServer ? "server" : "local") << std::endl;
        std::cout << "Instance ID: " << uiInstanceID << std::endl;
        if (bStandalone)
            std::cout << "Config: " << pathConfig.generic_u8string() << std::endl;
    }
    if (bVerbose)
        std::cout << "Installation directory: " << pathInstallDir.generic_u8string() << std::endl;

    // Create the startup config
    std::stringstream sstreamConfig;
    sstreamConfig << "[Application]" << std::endl;
    sstreamConfig << "Mode = \"" << (bServer ? "Main" : "Standalone") << "\"" << std::endl;
    sstreamConfig << "Instance = " << uiInstanceID << std::endl;
    if (!pathConfig.empty())
        sstreamConfig << "Config = \"" << pathConfig.generic_u8string() << "\"" << std::endl;
    if (!pathInstallDir.empty())
    {
        if (pathInstallDir.is_relative())
            sstreamConfig << "InstallDir = \"" << pathInstallDir.generic_u8string() << "\"" << std::endl;
        else
            sstreamConfig << "InstallDir = \"" << pathInstallDir.lexically_relative(GetExecDirectory()).generic_u8string() <<
            "\"" << std::endl;
    }

    // Add console information
    if (bVerbose || bSilent)
    {
        sstreamConfig << "[Console]" << std::endl;
        sstreamConfig << "Report = ";
        if (bSilent) sstreamConfig << "\"Silent\"";
        else
            sstreamConfig << "\"Verbose\"";
        sstreamConfig << std::endl;
    }

    if (bVerbose)
        std::cout << "Starting up..." << std::endl;

    // Start the application control
    sdv::app::CAppControl appcontrol;
    if (!appcontrol.Startup(sstreamConfig.str()))
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_STARTUP_ERROR_MSG << std::endl;
        return APP_CONTROL_STARTUP_ERROR;
    }

    // Start the running loop
    if (!appcontrol.RunLoop())
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_DUPLICATE_INSTANCE_MSG << std::endl;
        return APP_CONTROL_DUPLICATE_INSTANCE;
    }

    if (bVerbose)
    {
        std::cout << "Shutdown request received..." << std::endl;
        std::cout << "Shutting down..." << std::endl;
    }

    // Shutdwown
    appcontrol.Shutdown();

    return NO_ERROR;
}