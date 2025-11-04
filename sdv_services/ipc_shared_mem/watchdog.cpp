#include "watchdog.h"
#include "connection.h"
#include <vector>
#include "../../global/trace.h"

#ifdef __unix__
#include <signal.h>
#endif

CWatchDog::CWatchDog()
{
    m_threadScheduledConnectionDestructions = std::thread(&CWatchDog::HandleScheduledConnectionDestructions, this);
}

CWatchDog::~CWatchDog()
{
    Clear();
}

void CWatchDog::Clear()
{
    sdv::process::IProcessLifetime* pMonitorMgnt = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    if (pMonitorMgnt)
    {
        std::unique_lock<std::mutex> lockMonitors(m_mtxMonitors);
        for (const auto& rvtProcessMonitor : m_mapProcessMonitors)
            pMonitorMgnt->UnregisterMonitor(rvtProcessMonitor.second);
        m_mapProcessMonitors.clear();
    }


    // Shift the connections to a local variable to be able to delete the connections without being in the lock region.
    std::unique_lock<std::mutex> lockConnections(m_mtxConnections);
    auto mapConnections = std::move(m_mapConnections);
    lockConnections.unlock();

    // Remove the connections.
    mapConnections.clear();

    // Finalize the asnyhronous destructions
    m_bShutdown = true;
    if (m_threadScheduledConnectionDestructions.joinable())
        m_threadScheduledConnectionDestructions.join();
}

void CWatchDog::AddConnection(const std::shared_ptr<CConnection>& rptrConnection)
{
    if (!rptrConnection) return;

#if ENABLE_REPORTING > 0
    TRACE("Registering connection ", rptrConnection->IsServer() ? "server" : "client");
#endif

    std::unique_lock<std::mutex> lock(m_mtxConnections);
    m_mapConnections.try_emplace(rptrConnection.get(), rptrConnection);
}

void CWatchDog::RemoveConnection(CConnection* pConnection, bool bAsync)
{
#if ENABLE_REPORTING > 0
    if (bAsync)
        TRACE("Scheduling destruction connection ", pConnection->IsServer() ? "server" : "client");
    else
        TRACE("Destroying connection ", pConnection->IsServer() ? "server" : "client");
#endif

    // Find the connection and move it in a local variable to release it when outside the lock region.
    std::unique_lock<std::mutex> lockConnections(m_mtxConnections);
    auto itConnection = m_mapConnections.find(pConnection);
    if (itConnection == m_mapConnections.end()) return;
    auto ptrConnection = std::move(itConnection->second);
    m_mapConnections.erase(itConnection);
    if (bAsync)
    {
        // Shift the connection into the queue.
        m_queueScheduledConnectionDestructions.push(std::move(ptrConnection));
        m_cvTriggerConnectionDestruction.notify_all();
    }
    lockConnections.unlock();

    // Release the connection
    ptrConnection.reset();
}

void CWatchDog::AddMonitor(sdv::process::TProcessID tProcessID, CConnection* pConnection)
{
    if (!tProcessID || !pConnection) return;

    sdv::process::IProcessLifetime* pMonitorMgnt = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    if (!pMonitorMgnt) return;

    std::unique_lock<std::mutex> lock(m_mtxMonitors);

#if ENABLE_REPORTING > 0
    TRACE("Registering ", pConnection->IsServer() ? "server" : "client", " watchdog monitor for PID#", tProcessID);
#endif

    // Add the process monitor if not already assigned
    m_mapProcessMonitors.try_emplace(tProcessID, pMonitorMgnt->RegisterMonitor(tProcessID, this));

#if ENABLE_REPORTING > 0
    TRACE("Registering watchdog for PID#", tProcessID);
#endif

    // Add a monitor entry
    m_mapMonitors.insert(std::make_pair(tProcessID, pConnection->shared_from_this()));
}

void CWatchDog::RemoveMonitor(const CConnection* pConnection)
{
    if (!pConnection) return;

    std::unique_lock<std::mutex> lock(m_mtxMonitors);

#if ENABLE_REPORTING > 0
    TRACE("Removing ", pConnection->IsServer() ? "server" : "client", " watchdog monitor");
#endif

    // Earse all monitors for the provided connection
    auto itMonitor = m_mapMonitors.begin();
    while (itMonitor != m_mapMonitors.end())
    {
        std::shared_ptr<CConnection> ptrConnection = itMonitor->second.lock();
        if (ptrConnection.get() == pConnection)
            itMonitor = m_mapMonitors.erase(itMonitor);
        else
            itMonitor++;
    }
}

void CWatchDog::HandleScheduledConnectionDestructions()
{
    while (!m_bShutdown)
    {
        std::unique_lock<std::mutex> lock(m_mtxMonitors);
        m_cvTriggerConnectionDestruction.wait_for(lock, std::chrono::milliseconds(100));
        while (m_queueScheduledConnectionDestructions.size())
        {
            std::shared_ptr<CConnection> ptrConnection = std::move(m_queueScheduledConnectionDestructions.front());
            m_queueScheduledConnectionDestructions.pop();
            lock.unlock();

#if ENABLE_REPORTING > 0
            TRACE("Final destroying connection ", ptrConnection->IsServer() ? "server" : "client");
#endif

            ptrConnection.reset();
            lock.lock();
        }
    }
}

void CWatchDog::ProcessTerminated(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ int64_t /*iRetValue*/)
{
    std::unique_lock<std::mutex> lock(m_mtxMonitors);

    // Unregister the monitor
    sdv::process::IProcessLifetime* pMonitorMgnt = sdv::core::GetObject<sdv::process::IProcessLifetime>("ProcessControlService");
    if (!pMonitorMgnt) return;
    auto itProcessMonitor = m_mapProcessMonitors.find(tProcessID);
    if (itProcessMonitor == m_mapProcessMonitors.end()) return;
    pMonitorMgnt->UnregisterMonitor(itProcessMonitor->second);
    m_mapProcessMonitors.erase(itProcessMonitor);

    // Find the monitor in the map, remove it and add the connection to a to-be-disconnected vector.
    std::vector<std::shared_ptr<CConnection>> vecDisconnectedConnections;
    auto itMonitor = m_mapMonitors.find(tProcessID);
    while (itMonitor != m_mapMonitors.end() && itMonitor->first == tProcessID)
    {
#if ENABLE_REPORTING > 0
        TRACE("Removing watchdog for PID#", tProcessID);
#endif
        // Add the connection to the vector (if the connection was not removed before).
        auto ptrConnection = itMonitor->second.lock();
        if (ptrConnection) vecDisconnectedConnections.push_back(ptrConnection);

        // Remove the monitor
        itMonitor = m_mapMonitors.erase(itMonitor);
    }

    lock.unlock();

    // Inform the connection about the removed process.
    for (auto& rptrConnection : vecDisconnectedConnections)
    {
        rptrConnection->SetStatus(sdv::ipc::EConnectStatus::disconnected_forced);

#if ENABLE_REPORTING > 0
        TRACE("Forced disconnection for PID#", tProcessID);
#endif
    }
}
