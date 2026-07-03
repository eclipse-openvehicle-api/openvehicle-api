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

#include "../../global/process_watchdog.h"
#include <support/sdv_core.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/exec_dir_helper.h"
#include <support/mem_access.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <iostream>
#include <array>
#include <algorithm>
#include <thread>
#include "context.h"
#include "startup_shutdown.h"
#include "list_elements.h"
#include "start_stop_service.h"
#include "installation.h"
#include "connection_config.h"
#include "../error_msg.h"

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

    CCommandLine cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    SContext sContext;
    bool bError = false;
    bool bHelp = false;
    bool bVersion = false;
    std::string ssArgError;
    try
    {
        // Argument groups 1 - 10:
        //  STARTUP
        //  SHUTDOWN
        //  LIST
        //  INSTALL
        //  UPDATE
        //  UNINSTALL
        //  START
        //  STOP
        //  LISTENER
        //  CONNECTION
        auto& rArgHelpDef =
            cmdln.DefineOption("?", bHelp, "Show help. Use <COMMAND> --help for specific help on the command.");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", sContext.bSilent, "Do not show any information on std::cout. Not compatible "
            "with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", sContext.bVerbose, "Provide verbose information. Not compatible with "
            "'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("version", bVersion, "Show version information.");
        cmdln.DefineSubOption("instance", sContext.uiInstanceID, "The instance ID of the SDV instance (default ID is 1000).");
        cmdln.DefineSubOption("server_silent", sContext.bServerSilent, "Only used with STARTUP command: Server is started using "
            "silent option. Not compatible with 'server_verbose'.", true, 1);
        cmdln.DefineSubOption("server_verbose", sContext.bServerVerbose, "Only used with STARTUP command: Server is started using "
            "verbose option. Not compatible with 'server_silent'.", true, 1);
        cmdln.DefineSubOption("install_dir", sContext.pathInstallDir, "Only used with STARTUP command: Installation directory "
            "(absolute or relative to the sdv_core executable).", true, 1);
        cmdln.DefineSubOption("no_header", sContext.bListNoHdr, "Only used with LIST command: Do not print a header for the "
            "listing table.", true, 3);
        cmdln.DefineSubOption("short", sContext.bListShort, "Only used with LIST command: Print only the most essential "
            "information as one column.", true, 3);
        cmdln.DefineSubOption(
            "insert_before", sContext.ssInsertBeforeConnection, "Name of the connection to insert before.", true, 10);
        cmdln.DefineDefaultArgument(sContext.seqCmdLine, "COMMAND");

        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        ssArgError = rsExcept.what();
        bHelp = true;
        bError = true;
    }

    if (!sContext.bSilent)
    {
        std::cout << "SDV Server Control Utility" << std::endl;
        std::cout << "Copyright (C): 2022-2026 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    if (!ssArgError.empty() && !sContext.bSilent)
        std::cerr << "ERROR: " << ssArgError << std::endl;

    if (!bHelp && !bVersion && sContext.seqCmdLine.empty())
    {
        if (!sContext.bSilent)
            std::cerr << "ERROR: Missing command!" << std::endl;
        bHelp = true;
        bError = true;
    }

    enum class ECommand { unknown, startup, shutdown, list, install, update, uninstall, start, stop, listener, connection } eCommand = ECommand::unknown;
    if (!sContext.seqCmdLine.empty())
    {
        if (iequals(sContext.seqCmdLine[0], "STARTUP")) eCommand = ECommand::startup;
        else if (iequals(sContext.seqCmdLine[0], "SHUTDOWN")) eCommand = ECommand::shutdown;
        else if (iequals(sContext.seqCmdLine[0], "LIST")) eCommand = ECommand::list;
        else if (iequals(sContext.seqCmdLine[0], "INSTALL")) eCommand = ECommand::install;
        else if (iequals(sContext.seqCmdLine[0], "UPDATE")) eCommand = ECommand::update;
        else if (iequals(sContext.seqCmdLine[0], "UNINSTALL")) eCommand = ECommand::uninstall;
        else if (iequals(sContext.seqCmdLine[0], "START")) eCommand = ECommand::start;
        else if (iequals(sContext.seqCmdLine[0], "STOP")) eCommand = ECommand::stop;
        else if (iequals(sContext.seqCmdLine[0], "LISTENER")) eCommand = ECommand::listener;
        else if (iequals(sContext.seqCmdLine[0], "CONNECTION")) eCommand = ECommand::connection;
        else
        {
            if (!sContext.bSilent)
                std::cerr << "ERROR: Invalid command: " << sContext.seqCmdLine[0] << std::endl;
            bHelp = true;
            bError = true;
        }
    }

    if (bHelp)
    {
        if (!sContext.bSilent)
        {
            if (bError)
                std::cout << std::endl;
            switch (eCommand)
            {
            case ECommand::startup:
            case ECommand::shutdown:
                StartupShutdownHelp(sContext);
                break;
            case ECommand::start:
            case ECommand::stop:
                StartStopServiceHelp(sContext);
                break;
            case ECommand::list:
                ListHelp(sContext);
                break;
            case ECommand::install:
            case ECommand::update:
            case ECommand::uninstall:
                InstallationHelp(sContext);
                break;
            case ECommand::listener:
                ListenerHelp(sContext);
                break;
            case ECommand::connection:
                ConnectionHelp(sContext);
                break;
            default:
                cmdln.PrintHelp(std::cout, R"text(Supported commands:
    STARTUP    Start the core application server
    SHUTDOWN   Stop the core application server
    LIST       List information about modules/classes/components/installations/listeners/connections.
    INSTALL    Install a new application or service.
    UPDATE     Update an existing installation.
    UNINSTALL  Uninstall an installed application or service.
    START      Start a service (complex services and vehicle functions only).
    STOP       Stop a service (complex services and vehicle functions only).
    LISTENER   Add or remove a communication listener configuration.
    CONNECTION Add or remove a connection configuration.
)text");
                break;
            }
        }
        return bError ? CMDLN_ARG_ERR : NO_ERROR;
    }
    if (bError) return CMDLN_ARG_ERR;

    if (bVersion)
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl;

    if (sContext.seqCmdLine.empty()) return NO_ERROR;    // Done.

    // Start the application control
    if (sContext.bVerbose)
        std::cout << "Starting local application control.." << std::endl;
    sdv::app::CAppControl appcontrol;
    std::string ssAppConfig = std::string(R"toml(
[Application]
Mode = "Maintenance"
Instance = )toml") + std::to_string(sContext.uiInstanceID) + R"toml(
)toml";
    if (!appcontrol.Startup(ssAppConfig))
    {
        if (!sContext.bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_STARTUP_ERROR_MSG << std::endl;
        return APP_CONTROL_STARTUP_ERROR;
    }

    int iRet = 0;
    switch (eCommand)
    {
    case ECommand::startup:
        iRet = StartupSDVServer(sContext);
        break;
    case ECommand::shutdown:
        iRet = ShutdownSDVServer(sContext);
        break;
    case ECommand::list:
        iRet = ListElements(sContext);
        break;
    case ECommand::start:
        iRet = StartService(sContext);
        break;
    case ECommand::stop:
        iRet = StopService(sContext);
        break;
    case ECommand::install:
        iRet = Install(sContext);
        break;
    case ECommand::update:
        iRet = Update(sContext);
        break;
    case ECommand::uninstall:
        iRet = Uninstall(sContext);
        break;
    case ECommand::listener:
        iRet = ConfigureListener(sContext);
        break;
    case ECommand::connection:
        iRet = ConfigureConnection(sContext);
        break;
    default:
        std::cout << "Command missing :-(" << std::endl;
        break;
    }

    if (sContext.bVerbose) std::cout << "Shutting down local application control.." << std::endl;
    appcontrol.Shutdown();

    return iRet;
}