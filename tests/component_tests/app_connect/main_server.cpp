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

#include "../../../global/process_watchdog.h"
#include "../../../global/ipc_named_mutex.h"
#include <atomic>
#include <interfaces/ipc.h>
#include <support/sdv_core.h>
#include <support/app_control.h>

/**
 * @brief Main function
 */
#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    // Workaround for GCC to make certain that POSIX thread library is loaded before the components are loaded.
    // REASON: If the first call to a thread is done in a dynamic library, the application is already classified as single
    // threaded and a termination is initiated.
    // See: https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-leads-to-a-sigsev
    // NOTE EVE 27.05.2025: in release builds, starting and ending the thread right after each other causes incorrect behavior and
    // leads in some cases to create a deadlock in the join-function. The solution is to add delays in the thread processing.
    std::atomic_bool bThreadStarted = false;
    std::thread thread = std::thread(
        [&]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            bThreadStarted = true;
        });
    while (!bThreadStarted)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (thread.joinable())
        thread.join();

    std::cout << "RuntimeDirectory = " << sdv::app::CAppControl::GetFrameworkRuntimeDirectory().generic_u8string() << std::endl;
    std::cout << "InstallDirectory = " << sdv::app::CAppControl::GetComponentInstallDirectory().generic_u8string() << std::endl;

    std::cout << "Connect test application" << std::endl;
    std::cout << "Copyright (C): 2026 ZF Friedrichshafen AG" << std::endl;
    std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;

    // Get the instance number from the command line.
    if (argc != 2)
    {
        std::cout << "Missing argument; usage: AppConnect_Client <instance_number>" << std::endl;
        return -1;
    }
#if defined(_WIN32) && defined(_UNICODE)
    size_t nInstance = wcstoul(argv[1], nullptr, 10);
#else
    size_t nInstance = atoi(argv[1]);
#endif
    std::cout << "Listening for connections on instance #" << nInstance << std::endl;

    sdv::app::CAppControl control;
    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = )toml" + std::to_string(nInstance));
    if (!bResult)
    {
        std::cout << "Failed to start system..." << std::endl;
        return -2;
    }

    // Lifetime of the server is managed by the locking of a named mutex. As long as locking doesn't work, the server should run.
    // Test every 250ms
    ipc::named_mutex mtx("app_connect_test_mutex");
    bool bRunning = true;
    while (bRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (mtx.try_lock())
        {
            bRunning = false;
            mtx.unlock();
        }
    }

    control.Shutdown();
    std::cout << "Connect test application terminated... (instance #" << nInstance << ")" << std::endl;
    return 0;
}
