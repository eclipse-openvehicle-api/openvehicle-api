#ifndef PROCESS_WATCHDOG_H
#define PROCESS_WATCHDOG_H

#include <thread>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#ifdef _WIN32
// Prevent reassignment of "interface"
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Include windows headers
#include <WinSock2.h>
#include <Windows.h>
#include <objbase.h>

// Use previous assignment of "interface"
#pragma pop_macro("interface")

// Remove "GetObject" assignment
#ifdef GetObject
#undef GetObject
#endif

// Remove "GetClassInfo" assignment
#ifdef GetClassInfo
#undef GetClassInfo
#endif

#elif defined __unix__
#include <signal.h>
#include <unistd.h>
#else
#error The OS is not supported!
#endif

/**
 * @brief Process watchdog class; ends the process when runtime duration has superseded (as is the case when a deadlock has
 * occurred).
 */
class CProcessWatchdog
{
public:
    /**
     * @brief Constructor
     * @param[in] iWatchdogTimeS The duration the watchdog is monitoring before process termination in seconds (default 300s).
     */
    CProcessWatchdog(int64_t iWatchdogTimeS = 300ll)
    {
        m_threadWatchdog = std::thread(&CProcessWatchdog::WatchdogThreadFunc, this, iWatchdogTimeS );
    }

    /**
     * @brief Destructor; cancels the watch thread.
     */
    ~CProcessWatchdog()
    {
        m_bTerminateWatchdog = true;
        if (m_threadWatchdog.joinable())
            m_threadWatchdog.join();
    }

private:
#ifdef _WIN32
#elif defined __linux__
    /**
     * @brief Check whether the debugger is present and running the application.
     * @return Returns true when the debugger is present or false otherwise.
     */
    bool IsDebuggerPresent()
    {
        std::string ssFilename = "/proc/self/status"; // For the current process
        std::ifstream fstreamStatus(ssFilename);
        if (!fstreamStatus.is_open()) return false; // Cannot open file, assume not debugged

        std::string ssLine;
        while (std::getline(fstreamStatus, ssLine))
        {
            if (ssLine.substr(0, 9) == "TracerPid")
            {
                std::stringstream sstreamLine(ssLine);
                std::string ssKey;
                int iTracerPid = 0;
                sstreamLine >> ssKey >> iTracerPid;
                return iTracerPid != 0;
            }
        }
        return false; // TracerPid not found
    }
#else
    /**
     * @brief Check whether the debugger is present and running the application.
     * @return Returns true when the debugger is present or false otherwise.
     */
    bool IsDebuggerPresent()
    {
        // No implementation available to check for a debugger.
        return false;
    }
#endif

    /**
     * @brief Watch thread function.
     * @param[in] iWatchdogTimeS  The duration the watchdog is monitoring before process termination in seconds.
     */
    void WatchdogThreadFunc(int64_t iWatchdogTimeS)
    {
        // Run for the most the set time; then terminate...
        auto tpStart = std::chrono::steady_clock::now();
        while (!m_bTerminateWatchdog)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto tpNow = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(tpNow - tpStart).count() > iWatchdogTimeS)
            {
                // Do not end the process when a debugger is present.
                if (IsDebuggerPresent()) continue;

                std::cerr << "WATCHDOG TERMINATION ENFORCED!!!" << std::endl;
                std::cerr.flush();
#ifdef _WIN32
                // Get the current process handle
                HANDLE hProcess = GetCurrentProcess();
                // Terminate the current process with exit code -100
                TerminateProcess(hProcess, static_cast<UINT>(-100));
#elif defined __unix__

                // Send SIGTERM signal to the current process
                kill(getpid(), SIGTERM);
#else
#error The OS is not supported!
#endif
            }
        }
    }

    std::atomic_bool    m_bTerminateWatchdog = false;       ///< When set, allows the thread to terminate.
    std::thread         m_threadWatchdog;                   ///< The watchdog thread.
};

#endif // !defined PROCESS_WATCHDOG_H
