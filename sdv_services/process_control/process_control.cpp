#include "process_control.h"
#include "../../global/base64.h"
#include "../../global/exec_dir_helper.h"
#include <support/local_service_access.h>
#include <interfaces/app.h>

#ifdef __GNUC__
#include <unistd.h>
#endif

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <objbase.h>
#include <wchar.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#elif defined __unix__
//#include <spawn.h>
#include <sys/wait.h>
#include <signal.h>
#include <spawn.h>
#else
#error OS is not supported!
#endif

#include "../../global/trace.h"

CProcessControl::~CProcessControl()
{
    Shutdown();
}

void CProcessControl::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    if (m_eObjectStatus != sdv::EObjectStatus::initialization_pending) return;

    // Without monitor no trigger...
    m_threadMonitor = std::thread(&CProcessControl::MonitorThread, this);

    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CProcessControl::GetStatus() const
{
    return m_eObjectStatus;
}

void CProcessControl::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::configuring;
        break;
    case sdv::EOperationMode::running:
        if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
            m_eObjectStatus = sdv::EObjectStatus::running;
        break;
    default:
        break;
    }
}

void CProcessControl::Shutdown()
{
    // TODO: Close process handles
    if (m_eObjectStatus == sdv::EObjectStatus::destruction_pending) return;
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Shutdown the monitor
    m_bShutdown = true;
    if (m_threadMonitor.joinable())
        m_threadMonitor.join();

    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

bool CProcessControl::AllowProcessControl() const
{
    const sdv::app::IAppContext* pAppContext = sdv::core::GetCore<sdv::app::IAppContext>();
    return pAppContext && (pAppContext->GetContextType() == sdv::app::EAppContext::main ||
        pAppContext->GetContextType() == sdv::app::EAppContext::maintenance ||
        pAppContext->GetContextType() == sdv::app::EAppContext::essential);
}

sdv::process::TProcessID CProcessControl::GetProcessID() const
{
#ifdef _WIN32
    return static_cast<uint64_t>(GetCurrentProcessId());
#elif defined __unix__
    return static_cast<uint64_t>(getpid());
#else
#error OS is not supported!
#endif
}

uint32_t CProcessControl::RegisterMonitor(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ sdv::IInterfaceAccess* pMonitor)
{
    if (!tProcessID || !pMonitor) return 0;
    sdv::process::IProcessLifetimeCallback* pCallback =
        sdv::TInterfaceAccessPtr(pMonitor).GetInterface<sdv::process::IProcessLifetimeCallback>();
    if (!pCallback) return 0;

    std::unique_lock<std::mutex> lock(m_mtxProcesses);

    SDV_LOG_TRACE(GetTimestamp(), "Registering... (PID#", tProcessID,  ")");

    // Find the process
    auto itProcess = m_mapProcesses.find(tProcessID);
    if (itProcess == m_mapProcesses.end())
    {
#if _WIN32
        // Get the process handle.
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(tProcessID));
        if (!hProcess) return 0; // Cannot monitor
#elif defined __unix__
        // Use "kill" to detect the existence of the process
        // Notice: this doesn't kill the process.
        if (kill(tProcessID, 0) != 0) return 0; // Cannot monitor
#else
#error OS is not supported!
#endif

        // Create a new process structure
        auto ptrNewProcess = std::make_shared<SProcessHelper>();
        // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
        // exception was triggered).
        // cppcheck-suppress knownConditionTrueFalse
        if (!ptrNewProcess)
        {
#ifdef _WIN32
            CloseHandle(hProcess);
#endif
            SDV_LOG_TRACE(GetTimestamp(), "Could not add monitor... (PID#", tProcessID, ")");
            return 0;
        }
        ptrNewProcess->tProcessID = tProcessID;
#ifdef _WIN32
        ptrNewProcess->hProcess = hProcess;
#endif
        auto prInsert = m_mapProcesses.insert(std::make_pair(tProcessID, ptrNewProcess));
        if (!prInsert.second)
        {
#ifdef _WIN32
            CloseHandle(hProcess);
#endif
            SDV_LOG_TRACE(GetTimestamp(), "Could not add monitor... (PID#", tProcessID, ")");
            return 0;   // Could not insert
        }
        SDV_LOG_TRACE(GetTimestamp(), "Monitor now added... (PID#", tProcessID, ")");
        itProcess = prInsert.first;
    }
    auto ptrProcess = itProcess->second;

    // Create a new cookie
    uint32_t uiCookie = m_uiNextMonCookie++;

    // Register the new monitor
    m_mapMonitors[uiCookie] = ptrProcess;
    lock.unlock();

    // Add the monitor to the process as well
    std::unique_lock<std::mutex> lockProcess(ptrProcess->mtxProcess);
    ptrProcess->mapAssociatedMonitors[uiCookie] = pCallback;

    // Monitor thread running?
    // Already terminated?
    if (!ptrProcess->bRunning)
        pCallback->ProcessTerminated(tProcessID, ptrProcess->iRetVal);

    return uiCookie;
}

void CProcessControl::UnregisterMonitor(/*in*/ uint32_t uiCookie)
{
    if (!uiCookie) return;

    std::unique_lock<std::mutex> lock(m_mtxProcesses);

    // Find the monitor and remove the monitor
    auto itMonitor = m_mapMonitors.find(uiCookie);
    if (itMonitor == m_mapMonitors.end()) return;
    auto ptrProcess = itMonitor->second;
    m_mapMonitors.erase(itMonitor);

    lock.unlock();

    // Removing the monitor from the map might be problematic when the call is done in a callback. Simply update the pointer
    // instead (this will not affect the map order).
    std::unique_lock<std::mutex> lockProcess(ptrProcess->mtxProcess);
    ptrProcess->mapAssociatedMonitors.erase(uiCookie);
}

bool CProcessControl::WaitForTerminate(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ uint32_t uiWaitMs)
{
    if (!tProcessID) return true;     // Non-existent processes are terminated :-)

    std::unique_lock<std::mutex> lock(m_mtxProcesses);

    // Find the process
    auto itProcess = m_mapProcesses.find(tProcessID);
    if (itProcess == m_mapProcesses.end()) return true;     // Non-existent processes are terminated :-)
    auto ptrProcess = itProcess->second;

    lock.unlock();

    // Already terminated?
    std::unique_lock<std::mutex> lockProcess(ptrProcess->mtxProcess);
    if (!ptrProcess->bRunning) return true;

    // Wait for termination
    std::chrono::high_resolution_clock::time_point tpStart = std::chrono::high_resolution_clock::now();
	bool bTimeout = false;
    // False warning from cppcheck. bRunning is set by the monitor thread. Suppress warning.
    // cppcheck-suppress knownConditionTrueFalse
    while (ptrProcess->bRunning)
    {
        ptrProcess->cvWaitForProcess.wait_for(lockProcess, std::chrono::milliseconds(10));
        if (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tpStart).count() > static_cast<double>(uiWaitMs))
		{
			bTimeout = true;
            break;
		}
    }

    return !bTimeout;
}

sdv::process::TProcessID CProcessControl::Execute(/*in*/ const sdv::u8string& ssModule,
    /*in*/ const sdv::sequence<sdv::u8string>& seqArgs, /*in*/ sdv::process::EProcessRights eRights)
{
    if (!AllowProcessControl()) return 0;
    if (ssModule.empty()) return 0;
    sdv::process::TProcessID tProcessID = 0;

    // Update rights
    bool bReduceRights = false;
    const sdv::app::IAppContext* pAppContext = sdv::core::GetCore<sdv::app::IAppContext>();
    if (!pAppContext) return 0;

    switch (eRights)
    {
    case sdv::process::EProcessRights::default_rights:
        // Default implementation would be reduced rights
        bReduceRights = pAppContext->GetContextType() == sdv::app::EAppContext::main;
        break;
    case sdv::process::EProcessRights::reduced_rights:
        bReduceRights = true;
    break;
    case sdv::process::EProcessRights::parent_rights:
        break;
    default:
        break;
    }

    // Check for the existence of the module
    std::filesystem::path pathModule(static_cast<std::string>(ssModule));
#ifdef _WIN32
    if (!pathModule.has_extension())
        pathModule.replace_extension(".exe");
#endif
    if (pathModule.is_relative())
    {
        // Get the module serch dirs
        sdv::sequence<sdv::u8string> seqSearchDirs;
        const sdv::core::IModuleControlConfig* pModuleControlConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
        if (pModuleControlConfig)
            seqSearchDirs = pModuleControlConfig->GetModuleSearchDirs();

        // Add the current directory as well.
        seqSearchDirs.insert(seqSearchDirs.begin(), ".");

        // Now find the module
        bool bFound = false;
        for (const sdv::u8string& rssPath : seqSearchDirs)
        {
            std::filesystem::path pathModuleTemp = std::filesystem::path(static_cast<std::string>(rssPath)) / pathModule;
            if (std::filesystem::exists(pathModuleTemp))
            {
                pathModule = pathModuleTemp;
                bFound = true;
                break;
            }
        }

        // Found?
        if (!bFound) return 0;
    }

#ifdef _WIN32
    // The command line is one string. If containing spaces, include quotes
    std::wstringstream sstreamCommandline;
    sstreamCommandline << pathModule.native();
    for (auto& rssArg : seqArgs)
    {
        sstreamCommandline << L" ";
        bool bQuotes = rssArg.find(" ") != std::string::npos;
        if (bQuotes) sstreamCommandline << L'\"';
        sstreamCommandline << sdv::MakeWString(rssArg);
        if (bQuotes) sstreamCommandline << L'\"';
    }

    // Command line string
    std::wstring ssCommandLine = sstreamCommandline.str();
    if (ssCommandLine.size() > 32768) return 0;
    ssCommandLine.reserve(32768);

    /* First get a handle to the current process's primary token */
    HANDLE hProcessToken = 0;
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &hProcessToken);

    // Update rights
    if (bReduceRights)
    {
        // Create a restricted token with all privileges removed.
        // NOTE: This doesn't restrict file system access. To do so, use a separate user (e.g. guest user).
        HANDLE hRestrictedToken = 0;
        CreateRestrictedToken(hProcessToken, DISABLE_MAX_PRIVILEGE, 0, 0, 0, 0, 0, 0, &hRestrictedToken);
        CloseHandle(hProcessToken);
        hProcessToken = hRestrictedToken;
    }

    // Start the process
    STARTUPINFOW sStartInfo{};
    PROCESS_INFORMATION sProcessInfo{};
    bool bRes = CreateProcessAsUserW(hProcessToken,         // Process token
        nullptr,                                            // No module name (use command line)
        &ssCommandLine.front(),                             // Command line
        nullptr,                                            // Process handle not inheritable
        nullptr,                                            // Thread handle not inheritable
        FALSE,                                              // Set handle inheritance to FALSE
        0,                                                  // No creation flags
        nullptr,                                            // Use parent's environment block
        GetExecDirectory().native().c_str(),                // Use parent's starting directory
        &sStartInfo,                                        // Pointer to STARTUPINFO structure
        &sProcessInfo);                                     // Pointer to PROCESS_INFORMATION structure
    if (!bRes) return 0;
    tProcessID = sProcessInfo.dwProcessId;
    CloseHandle(sProcessInfo.hThread);
    CloseHandle(hProcessToken);

#elif defined __unix__

    // Create the argument list
    std::vector<char*> vecArgs;
    auto seqTempArgs = seqArgs;
    std::string ssModuleTemp = pathModule.native();
    seqTempArgs.insert(seqTempArgs.begin(), &ssModuleTemp.front());
    for (auto& rssArg : seqTempArgs)
        vecArgs.push_back(&rssArg.front());
    vecArgs.push_back(nullptr);

    // Create environment variable list
    std::vector<char*> vecEnv;
    vecEnv.push_back(nullptr);

    // TODO: Update rights and environment vars

    // Fork the process
    int pid = vfork();
    switch (pid)
    {
    case -1:    // Error called in parent process; cannot continue
        return 0;
    case 0:     // Child process is executing with pid == 0
    {
        // Reduce rights
        if (bReduceRights)
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
            setgid(getgid());
            setuid(getuid());
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }

        execve(pathModule.native().c_str(), &vecArgs.front(), &vecEnv.front());
        std::abort(); // Child process only comes here on error
    }
    default:
        // Parent process is executing with pid != 0
        tProcessID = static_cast<sdv::process::TProcessID>(pid);
        break;
    }
#else
#error OS is not supported!
#endif

    // Create a new process structure
    auto ptrNewProcess = std::make_shared<SProcessHelper>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrNewProcess)
    {
        // TODO Terminate process and close handles...
        return 0;
    }
    ptrNewProcess->tProcessID = tProcessID;
#ifdef _WIN32
    ptrNewProcess->hProcess = sProcessInfo.hProcess;
#endif
    m_mapProcesses[tProcessID] = ptrNewProcess;

    return tProcessID;
}

bool CProcessControl::Terminate(/*in*/ sdv::process::TProcessID tProcessID)
{
    if (!AllowProcessControl()) return false;

    if (!tProcessID) return false;

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(tProcessID));
    if (!hProcess) return 0; // Cannot terminate
    TerminateProcess(hProcess, static_cast<UINT>(-100));
    CloseHandle(hProcess);
#elif defined __unix__
    // Send a signal (SIGUSR1) to the child process.
    if (kill(tProcessID, SIGUSR1) != 0) return false;
#else
#error OS is not supported!
#endif

    return true;
}

void CProcessControl::MonitorThread()
{
    while (!m_bShutdown)
    {
        // Run through the list of processes and determine which process has been terminated.
        std::unique_lock<std::mutex> lock(m_mtxProcesses);
        std::vector<std::shared_ptr<SProcessHelper>> m_vecTerminatedProcesses;
        for (auto& vtProcess : m_mapProcesses)
        {
            // Update only once
            if (!vtProcess.second->bRunning) continue;

#ifdef _WIN32
            DWORD dwExitCode = 0;
            if (GetExitCodeProcess(vtProcess.second->hProcess, &dwExitCode) && dwExitCode != STILL_ACTIVE)
//            if (WaitForSingleObject(vtProcess.second->hProcess, 0) == WAIT_OBJECT_0)
            {
                // Get the exit code
                //DWORD dwExitCode = 0;
                //GetExitCodeProcess(vtProcess.second->hProcess, &dwExitCode);
                vtProcess.second->iRetVal = static_cast<int>(dwExitCode);   // Do not convert to 64-bit!

                // Close the process handle
                CloseHandle(vtProcess.second->hProcess);
                vtProcess.second->hProcess = 0;
                m_vecTerminatedProcesses.push_back(vtProcess.second);

                TRACE(GetTimestamp(), "Adding process PID#", vtProcess.second->tProcessID, " to termination map with exit code ", vtProcess.second->iRetVal);
            }
#elif defined __unix__
            // There are two ways of checking whether a process is still running. If the monitor process is the parent process, the
            // waitpid function should be used to request the process state. If the process was not created by the parent process,
            // the kill function returns information about the process.
            // After termination a process will stay dorment until the parent process has received the exit value of the process.
            // This is also the reason why the kill function is not working with the parent process, since it is only returning
            // a result after the process is not available at all any more (also not dorment).
            // Use a flag to indicate that the process is not a child process of the monitor process.
            if (!vtProcess.second->bNotAChild)
            {
                // Check with waitpid first
                int iStatus = 0;
#ifdef WCONTINUED
                pid_t pid = waitpid(vtProcess.second->tProcessID, &iStatus, WCONTINUED | WNOHANG);
#else
                pid_t pid = waitpid(vtProcess.second->tProcessID, &iStatus, WNOHANG);
#endif
                switch (pid)
                {
                case -1:
                    // Not a child process of the monitor process. Or a signal is returned to the calling process. Use kill instead.
                    vtProcess.second->bNotAChild = true;
                    SDV_LOG_TRACE(getpid(), " Process ", vtProcess.second->tProcessID, " is not child process...");
                    break;
                case 0:
                    // Process still running, no status available.
                    break;
                default:
                    if (WIFEXITED(iStatus))
                    {
                        m_vecTerminatedProcesses.push_back(vtProcess.second);
                        vtProcess.second->iRetVal = static_cast<int8_t>(WEXITSTATUS(iStatus));
                        SDV_LOG_TRACE(getpid(), " Normal exit detected for process ", vtProcess.second->tProcessID);
                    }
                    else if (WIFSIGNALED(iStatus))
                    {
                        m_vecTerminatedProcesses.push_back(vtProcess.second);
                        // Note: The status is not reported by the process, since it was terminated without return value.
                        //vtProcess.second->iRetVal = static_cast<int8_t>(WTERMSIG(iStatus));
                        vtProcess.second->iRetVal = -100;
                        SDV_LOG_TRACE(getpid(), " Signalled stop detected for process ", vtProcess.second->tProcessID);
                    }
                    else if (WIFSTOPPED(iStatus))
                    {
                        m_vecTerminatedProcesses.push_back(vtProcess.second);
                        vtProcess.second->iRetVal = static_cast<int8_t>(WSTOPSIG(iStatus));
                        SDV_LOG_TRACE(getpid(), " Terminate exit detected for process ", vtProcess.second->tProcessID);
                    }
                    SDV_LOG_TRACE(GetTimestamp(), "Exit detected with exit code ", vtProcess.second->iRetVal);
                }
            }

            // For a process not being the child of the monitor process, check with the kill function.
            // Note: check for the bNotAChild flag once more, since it might be set by the waitpid analysis.
            if (vtProcess.second->bNotAChild)
            {
                // Use "kill" to detect the existence of the process
                // Notice: this doesn't kill the process.
                kill(vtProcess.first, 0);
                if (errno == ESRCH)
                {
                    m_vecTerminatedProcesses.push_back(vtProcess.second);

                    SDV_LOG_TRACE(getpid(), " Non-child exit detected for process ", vtProcess.second->tProcessID);

                    // No exit code available...
                }
            }
#else
#error OS is not supported!
#endif
        }

        // Allow changes
        lock.unlock();

        // Is there a vector of terminated process IDs?
        for (auto ptrTerminatedProcess : m_vecTerminatedProcesses)
        {
            std::unique_lock<std::mutex> lockProcess(ptrTerminatedProcess->mtxProcess);

            // Reset the running flag
            ptrTerminatedProcess->bRunning = false;

            // Call the callback functions
            // Note: The unregister function might change the pointers in the map. Make a copy.
            auto mapAssociatedMonitorsCopy = ptrTerminatedProcess->mapAssociatedMonitors;
            lockProcess.unlock();
            for (auto& rvtMonitor : mapAssociatedMonitorsCopy)
            {
                // Check whether the monitor still exists
                lockProcess.lock();
                auto itMonitor = ptrTerminatedProcess->mapAssociatedMonitors.find(rvtMonitor.first);
                if (itMonitor == ptrTerminatedProcess->mapAssociatedMonitors.end() || !itMonitor->second)
                {
                    // Monitor not available...
                    lockProcess.unlock();
                    continue;
                }

                // Allow updates to take place
                lockProcess.unlock();

                // Call callback
                rvtMonitor.second->ProcessTerminated(ptrTerminatedProcess->tProcessID, ptrTerminatedProcess->iRetVal);
            }

            // Inform all waiting processes
            ptrTerminatedProcess->cvWaitForProcess.notify_all();
        }

        // Wait for 100ms until next check
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

