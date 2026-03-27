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
#include <support/mem_access.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/toml.h>
#include "../../global/exec_dir_helper.h"
#include "../../global/filesystem_helper.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include "packager.h"
#include "../../sdv_services/core/toml_parser/miscellaneous.h"
#include "../../sdv_services/core/app_settings.h"

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

    // Process the command line.
    CSdvPackagerEnvironment environment(iArgc, rgszArgv);

    // Print tool title
    if (!environment.Silent())
    {
        std::cout << "SDV Component Installation Package Utility" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    // Version requested?
    if (environment.Version() || environment.Verbose())
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl << std::endl;

    if (!environment.ArgError().empty() && !environment.Silent())
        std::cerr << "ERROR: " << environment.ArgError() << std::endl;

    if (environment.Help())
    {
        if (!environment.Silent())
        {
            if (environment.Error() != NO_ERROR)
                std::cout << std::endl;
            environment.ShowHelp();
        }
        return environment.Error();
    }
    if (environment.Error()) return CMDLN_ARG_ERR;

    // Report information about the settings
    environment.ReportInfo();

    // Anything to do?
    if (environment.OperatingMode() == CSdvPackagerEnvironment::EOperatingMode::none) return NO_ERROR;

    // Create a settings startup configuration
    std::stringstream sstreamStartupConfig;
    sstreamStartupConfig << "[Application]" << std::endl;
    if (environment.Local())
    {
        sstreamStartupConfig << "Mode = \"Standalone\"" << std::endl;
    }
    else
    {
        sstreamStartupConfig << "Mode = \"Maintenance\"" << std::endl;
        sstreamStartupConfig << "Instance = " << environment.InstanceID() << std::endl;
        if (!environment.TargetLocation().empty())
            sstreamStartupConfig << "InstallDir = \"" << environment.TargetLocation().generic_u8string() << "\"" << std::endl;
    }
    sstreamStartupConfig << "[Console]" << std::endl;
    if (environment.Silent())
        sstreamStartupConfig << "Report = \"Silent\"" << std::endl;
    else if (environment.Verbose())
        sstreamStartupConfig << "Report = \"Verbose\"" << std::endl;
    else
        sstreamStartupConfig << "Report = \"Normal\"" << std::endl;

    // Process the application startup configuration
    if (!GetAppSettings().ProcessAppStartupConfig(sstreamStartupConfig.str()))
    {
        std::cerr << "ERROR: Failed to process the startup configuration; cannot continue!" << std::endl;
        return -1;
    }

    // If running as server, load the settings file
    if (!environment.Local() && !GetAppSettings().LoadSettingsFile())
    {
        std::cerr << "ERROR: Failed to load the application settings file; cannot continue!" << std::endl;
        return -1;
    }

    // Enable the packager.
    CPackager packager(environment);
    packager.Execute();
    if (!packager.ArgError().empty() && !environment.Silent())
        std::cerr << "ERROR: " << packager.ArgError() << std::endl;
    return packager.Error();
}
