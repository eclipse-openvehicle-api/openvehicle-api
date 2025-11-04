#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
#include <support/mem_access.h>
#include <support/app_control.h>
#include "../../../sdv_services/process_control/process_control.cpp"
#include "process_control_ifc.h"
#include <cstdlib>
#include "../../../global/trace.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    std::cout << GetTimestamp() << "Startup process control application..." << std::endl;

    // The first argument is the name of the application

    // The second argument is the operation mode
    if (argc < 2)
    {
        std::cout << GetTimestamp() << "Invalid arguments..." << std::endl;
        return -1;
    }
    EOperatingmode eMode = static_cast<EOperatingmode>(std::atoi(sdv::MakeAnsiString(argv[1]).c_str()));

    sdv::app::CAppControl appcontrol;
    if (!appcontrol.Startup(R"code([Application]
Mode="Maintenance")code"))
    {
        std::cout << GetTimestamp() << "Failed to start app control..." << std::endl;
        return -1;
    }

    int nResult = 0;
    switch (eMode)
    {
    case EOperatingmode::normal_shutdown_1000ms:
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << GetTimestamp() << "Normal shutdown after 1000ms" << std::endl;
        break;
    case EOperatingmode::emergency_exit_1000ms:
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << GetTimestamp() << "Emergency exit after 1000ms" << std::endl;
#ifdef _MSC_VER
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifdef _WIN32
        TerminateProcess(GetCurrentProcess(), static_cast<UINT>(-20));
#else
        std::_Exit(-20);
#endif
        break;
    case EOperatingmode::wait_for_process:
        if (argc < 3)
        {
            std::cout << GetTimestamp() << "Invalid arguments..." << std::endl;
            return -1;
        }
        {
            std::string ssProcessID = sdv::MakeAnsiString(argv[2]);
            char* szEnd = nullptr;
            sdv::process::TProcessID tProcessID = static_cast<sdv::process::TProcessID>(std::strtoull(ssProcessID.c_str(), &szEnd, 10));
            if (!tProcessID)
            {
                std::cout << GetTimestamp() << "Expecting a process ID..." << std::endl;
                nResult = -3;
                break;
            }
            std::cout << GetTimestamp() << "Waiting for process with PID#" << std::dec << tProcessID << std::endl;
            CProcessMonitorHelper monitor;
            CProcessControl control;
            control.Initialize(""); // Needed since local instantiation
            uint32_t uiMon = control.RegisterMonitor(tProcessID, &monitor);
            if (!uiMon)
            {
                std::cout << GetTimestamp() << "Could not register monitor..." << std::endl;
                nResult = -4;
                break;
            }
            if (!monitor.Wait5000ms())
            {
                std::cout << GetTimestamp() << "Wait failed for monitor..." << std::endl;
                nResult = -20;
                control.UnregisterMonitor(uiMon);
                break;
            }
            std::cout << GetTimestamp() << "Return value of first process: " << monitor.GetRetValue() << std::endl;
            switch (monitor.GetRetValue())
            {
            case 0: break;
            case -20: nResult = -10; break;
            default: nResult = -20; break;
            }
            control.UnregisterMonitor(uiMon);
            control.Shutdown(); // Needed to prevent clash with core
        }
        break;
    case EOperatingmode::terminate_process:
        if (argc < 3)
        {
            std::cout << GetTimestamp() << "Invalid arguments..." << std::endl;
            return -1;
        }
        {
            std::string ssProcessID = sdv::MakeAnsiString(argv[2]);
            char* szEnd = nullptr;
            sdv::process::TProcessID tProcessID = static_cast<sdv::process::TProcessID>(std::strtoull(ssProcessID.c_str(), &szEnd, 10));
            if (!tProcessID)
            {
                std::cout << GetTimestamp() << "Expecting a process ID..." << std::endl;
                nResult = -3;
                break;
            }
            std::cout << GetTimestamp() << "Terminate process with PID#" << std::dec << tProcessID << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Make certain, that the first process is actually running...
            CProcessControl control;
            control.Initialize(""); // Needed since local instantiation
            nResult = control.Terminate(tProcessID) ? 0 : -20;
            control.Shutdown(); // Needed to prevent clash with core
        }
        break;
    default: // Unknown mode
        std::cout << GetTimestamp() << "Invalid arguments..." << std::endl;
        nResult = -2;
        break;
    }

    appcontrol.Shutdown();

    // Done....
    return nResult;
}
