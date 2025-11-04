#ifndef TEST_WATCHDOG_H
#define TEST_WATCHDOG_H

#include <thread>
#include <cstdint>
#include <iostream>
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
 * @brief Test watchdog class; ends test process when runtime duration has superseded.
 */
class CProcessWatchdog
{
public:
    /**
     * @brief Constructor
     * @param[in] m_iWatchdogTimeS The duration the watchdog is monitoring before process termination in seconds (default 300s).
     */
    CProcessWatchdog(int64_t iWatchdogTimeS = 300ll)
    {
        m_threadWatchdog = std::thread(&CProcessWatchdog::WatchdogThreadFunc, this, iWatchdogTimeS );
    }

    /**
     * @breif Destructor; cancels the watch thread.
     */
    ~CProcessWatchdog()
    {
        m_bTerminateWatchdog = true;
        if (m_threadWatchdog.joinable())
            m_threadWatchdog.join();
    }

private:
    /**
     * @brief Watch thread function.
     * @param[in] iWatchdogTimeS  The duration the watchdog is monitoring before process termination in seconds (default 120s).
     */
    void WatchdogThreadFunc(int64_t iWatchdogTimeS  = 120ll)
    {
        // Run for the most the set time; then terminate...
        auto tpStart = std::chrono::steady_clock::now();
        while (!m_bTerminateWatchdog)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto tpNow = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(tpNow - tpStart).count() > iWatchdogTimeS)
            {
                std::cerr << "ERROR PROCESS WATCHDOG: POTENTIAL DEADLOCK DETECTED - TERMINATION ENFORCED!!!" << std::endl;
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

    bool            m_bTerminateWatchdog = false;       ///< When set, allows the thread to terminate.
    std::thread     m_threadWatchdog;                   ///< The watchdog thread.
};

#endif // !defined TEST_WATCHDOG_H
