#ifndef WATCH_DOG_H
#define WATCH_DOG_H

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#endif

#include <interfaces/process.h>
#include <support/interface_ptr.h>
#include <mutex>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>

// Forward declaration
class CConnection;

/**
 * @brief The watch dog monitors the processes and connections.
 * @details The watch dog keeps track of the processes and disconnects when the process the connection is with doesn't exist any
 * more. This method is has advantages to using time-outs. If a process is being debugged and a breakpoint is triggered, all
 * threads within the process are paused, which could cause a timeout by the calling process waiting for an answer. By monitoring
 * the process existence, a pause doesn't cause any error. If the process crashes, it is being removed by the OS, causing the
 * monitor to detect this and the connection being terminated. Disadvantage is, that if the process is in a deadlock, this cannot
 * be detected by this method.
 */
class CWatchDog : public sdv::IInterfaceAccess, public sdv::process::IProcessLifetimeCallback
{
public:
    /**
     * @brief Constructor
     */
    CWatchDog();

    /**
     * @brief Destructor
     */
    ~CWatchDog();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::process::IProcessLifetimeCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Unregister all remaining monitors.
     */
    void Clear();

    /**
     * @brief Add a connection object used for monitoring.
     * @param[in] rptrConnection Reference to the connection shared pointer.
     */
    void AddConnection(const std::shared_ptr<CConnection>& rptrConnection);

    /**
    * @brief Remove a connection.
    * @param[in] pConnection The connection pointer to remove the monitor for.
    * @param[in] bAsync Use asnyhronous removal (use when the removal was triggered by a thread owned by the connection).
    */
    void RemoveConnection(CConnection* pConnection, bool bAsync);

    /**
     * @brief Add a process monitor with the connection object to information.
     * @param[in] tProcessID Process ID of the process to monitor.
     * @param[in] pConnection Pointer to the connection to inform when the process is killed.
     */
    void AddMonitor(sdv::process::TProcessID tProcessID, CConnection* pConnection);

    /**
     * @brief Remove a monitor.
     * @param[in] pConnection The connection pointer to remove the monitor for.
     */
    void RemoveMonitor(const CConnection* pConnection);

private:
    /**
     * @brief Thread function to destroy scheduled connections for destruction.
     */
    void HandleScheduledConnectionDestructions();

    /**
     * @brief Called when the process was terminated. Overload of sdv::process::IProcessLifetimeCallback::ProcessTerminated.
     * @remarks The process return value is not always valid. The validity depends on the support of the underlying system.
     * @param[in] tProcessID The process ID of the process being terminated.
     * @param[in] iRetValue Process return value or 0 when not supported.
     */
    virtual void ProcessTerminated(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ int64_t iRetValue) override;

    std::mutex                  m_mtxConnections;                           ///< Protect the map access.
    std::map<CConnection*, std::shared_ptr<CConnection>>    m_mapConnections;       ///< Connection map.

    std::mutex                  m_mtxMonitors;                              ///< Protect the map access.
    std::multimap<sdv::process::TProcessID, std::weak_ptr<CConnection>> m_mapMonitors; ///< Process monitor.
    std::map<sdv::process::TProcessID, uint32_t>            m_mapProcessMonitors;   ///< Map of process monitor cookies.
    std::condition_variable     m_cvTriggerConnectionDestruction;           ///< Condition variable used to trigger when a
                                                                            ///< connection is scheduled for destruction.
    std::queue<std::shared_ptr<CConnection>> m_queueScheduledConnectionDestructions;     ///< Scheduled connection for destruction.
    std::thread                 m_threadScheduledConnectionDestructions;    ///< Thread processing the scheduled destructions.
    std::atomic_bool            m_bShutdown = false;                        ///< Set when shutting down the watchdog
};

#endif // !defined WATCH_DOG_H