#include "dispatchservice.h"
#include "trigger.h"
#include "signal.h"

CScheduler::~CScheduler()
{
    Stop();
}

void CScheduler::Start()
{
    // Start the timer at 1ms rate
    m_timer = sdv::core::CTaskTimer(1, [&]() { EvaluateAndExecute(); });
}

void CScheduler::Stop()
{
    // Stop the timer
    m_timer.Reset();

    // Clear the schedule lists
    std::unique_lock<std::mutex> lock(m_mtxScheduler);
    m_mapTriggers.clear();
    m_mmapScheduleList.clear();
}

void CScheduler::Schedule(CTrigger* pTrigger, EExecutionFlag eExecFlag, std::chrono::high_resolution_clock::time_point tpDue)
{
    if (!pTrigger) return;
    if (!m_timer) return;

    // Check whether a job is scheduled already for this trigger object
    std::unique_lock<std::mutex> lock(m_mtxScheduler);
    auto itObject = m_mapTriggers.find(pTrigger);
    if (itObject != m_mapTriggers.end())
    {
        // Trigger execution of the object already found. Update the periodic flag if not set necessary
        if (eExecFlag == EExecutionFlag::spontaneous)
            itObject->second = eExecFlag;
        return;
    }

    // Insert the object into the scheduled object map.
    itObject = m_mapTriggers.emplace(pTrigger, eExecFlag).first;
    if (itObject == m_mapTriggers.end()) return; // Should not happen

    // Schedule the execution
    m_mmapScheduleList.emplace(tpDue, itObject);
}

void CScheduler::EvaluateAndExecute()
{
    // Run until there is nothing to execute for the moment
    while (true)
    {
        // Check the schedule list
        std::unique_lock<std::mutex> lock(m_mtxScheduler);
        auto itJob = m_mmapScheduleList.begin();
        if (itJob == m_mmapScheduleList.end()) return; // Nothing to do

        // Get the current time and check whether to execute
        std::chrono::high_resolution_clock::time_point tpNow = std::chrono::high_resolution_clock::now();
        if (tpNow < itJob->first) return; // No time yet

        // This job is being scheduled; copy the information and remove the entries from the scheduler
        CTrigger* pTrigger = itJob->second->first;
        EExecutionFlag eExecFlag = itJob->second->second;
        m_mapTriggers.erase(itJob->second);
        m_mmapScheduleList.erase(itJob);
        lock.unlock();

        // Execute the trigger
        pTrigger->Execute(eExecFlag);
    }
}

void CScheduler::RemoveFromSchedule(const CTrigger* pTrigger)
{
    // Search for the job in the scheduled list
    std::unique_lock<std::mutex> lock(m_mtxScheduler);
    auto itJob = std::find_if(m_mmapScheduleList.begin(), m_mmapScheduleList.end(),
        [&](const auto& rvtJob) { return rvtJob.second->first == pTrigger; });
    if (itJob == m_mmapScheduleList.end()) return;  // No scheduled jobs found

    // Remove the job
    m_mapTriggers.erase(itJob->second);
    m_mmapScheduleList.erase(itJob);
}

CTrigger::CTrigger(CDispatchService& rDispatchSvc, uint32_t uiCycleTime, uint32_t uiDelayTime, uint32_t uiBehaviorFlags,
    sdv::core::ITxTriggerCallback* pCallback) :
    m_rDispatchSvc(rDispatchSvc), m_tpLast{}, m_nPeriod(uiCycleTime), m_nDelay(uiDelayTime), m_uiBehaviorFlags(uiBehaviorFlags),
    m_pCallback(pCallback)
{
    // Start the timer if this triggr should be perodic
    if (uiCycleTime)
        m_timer = sdv::core::CTaskTimer(uiCycleTime, [&]() { Execute(EExecutionFlag::periodic); });
}

bool CTrigger::IsValid() const
{
    if (!m_pCallback) return false;
    if (m_nPeriod && !m_timer) return false;
    if (!m_nPeriod && !(m_uiBehaviorFlags & static_cast<uint32_t>(sdv::core::ISignalTransmission::ETxTriggerBehavior::spontaneous)))
        return false;
    return true;
}

void CTrigger::DestroyObject()
{
    // Stop the timer
    m_timer.Reset();

    // Remove all signals
    std::unique_lock<std::mutex> lock(m_mtxSignals);
    while (!m_mapSignals.empty())
    {
        CSignal* pSignal = m_mapSignals.begin()->second;
        m_mapSignals.erase(m_mapSignals.begin());
        if (!pSignal) continue;
        lock.unlock();

        pSignal->RemoveTrigger(this);

        lock.lock();
    }
    lock.unlock();

    // Remove the object (this will destruct this object).
    m_rDispatchSvc.RemoveTxTrigger(this);
}

bool CTrigger::AddSignal(/*in*/ const sdv::u8string& ssSignalName)
{
    CSignal* pSignal = m_rDispatchSvc.FindSignal(ssSignalName, sdv::core::ESignalDirection::sigdir_tx);
    if (pSignal && pSignal->GetDirection() == sdv::core::ESignalDirection::sigdir_tx)
    {
        std::unique_lock<std::mutex> lock(m_mtxSignals);
        pSignal->AddTrigger(this);
        m_mapSignals.insert(std::make_pair(pSignal->GetName(), pSignal));
        return true;
    }
    return false;
}

void CTrigger::RemoveSignal(/*in*/ const sdv::u8string& ssSignalName)
{
    std::unique_lock<std::mutex> lock(m_mtxSignals);
    auto itSignal = m_mapSignals.find(ssSignalName);
    if (itSignal == m_mapSignals.end()) return;
    CSignal* pSignal = itSignal->second;
    m_mapSignals.erase(itSignal);
    lock.unlock();

    if (pSignal) pSignal->RemoveTrigger(this);
}

void CTrigger::Execute(EExecutionFlag eExecFlag /*= EExecutionFlag::spontaneous*/)
{
    if (m_rDispatchSvc.GetStatus() != sdv::EObjectStatus::running) return;

    // Check for allowed execution
    if (eExecFlag == EExecutionFlag::spontaneous &&
        !(m_uiBehaviorFlags & static_cast<uint32_t>(sdv::core::ISignalTransmission::ETxTriggerBehavior::spontaneous)))
        return;

    // Is there a delay time, check for the delay
    std::chrono::high_resolution_clock::time_point tpNow = std::chrono::high_resolution_clock::now();
    if (m_nDelay)
    {
        // Calculate earliest execution time
        std::chrono::high_resolution_clock::time_point tpAllowedExecution = m_tpLast + std::chrono::milliseconds(m_nDelay);

        // Is execution allowed already; if not, schedule execution for later.
        if (tpNow < tpAllowedExecution)
        {
            m_rDispatchSvc.GetScheduler().Schedule(this, eExecFlag, tpAllowedExecution);
            return;
        }
    }

    // Check for the active flag. If periodic send when active is enabled, check the data for its state and update the repition
    // counter.
    if (m_uiBehaviorFlags & static_cast<uint32_t>(sdv::core::ISignalTransmission::ETxTriggerBehavior::periodic_if_active))
    {
        // Check whether content is active
        std::unique_lock<std::mutex> lock(m_mtxSignals);
        bool bDefault = true;
        for (const auto& rvtSignal : m_mapSignals)
            bDefault &= rvtSignal.second->EqualsDefaultValue();

        // Reset or increase repetition counter based on activity
        m_nInactiveRepetition = bDefault ? m_nInactiveRepetition + 1 : 0;

        // Based on the inactive repitions, decide to execute.
        // FIXED VALUE: Inactive repitions is set to 1... could be defined dynamic in the future
        if (eExecFlag == EExecutionFlag::periodic && m_nInactiveRepetition > 1) return;
    }

    // Execution is allowed, update execution timepoint
    m_tpLast = tpNow;

    // Execute the trigger
    if (m_pCallback) m_pCallback->Execute();
}
