#include "simulationtasktimer.h"
#include <fstream>
#include <functional>

CSimulationTimer::CSimulationTimer(CSimulationTaskTimerService& rtimersvc, uint32_t uiPeriod, sdv::core::ITaskExecute* pExecute) :
    m_rtimersvc(rtimersvc), m_pExecute(pExecute)
{
    if (!pExecute) return;

    m_uiInitializedPeriod = static_cast<uint64_t>(uiPeriod) * 1000;
    m_uiPeriod = static_cast<uint64_t>(uiPeriod) * 1000;
    if(m_uiInitializedPeriod != 0)
        m_bRunning = true;
}

void CSimulationTimer::DestroyObject()
{
    // Delete the object
    m_rtimersvc.RemoveTimer(this);
}

CSimulationTimer::operator bool() const
{
    return m_bRunning;
}

void CSimulationTimer::SimulationStep(uint64_t uiSimulationStep)
{
    while (uiSimulationStep > m_uiPeriod)
    {
        uiSimulationStep -= m_uiPeriod;
        if (m_pExecute) m_pExecute->Execute();
        m_uiPeriod = m_uiInitializedPeriod;
    }

    m_uiPeriod -= uiSimulationStep;
    if (m_uiPeriod == 0)
    {
        if (m_pExecute) m_pExecute->Execute();
        m_uiPeriod = m_uiInitializedPeriod;
    }
}

//#ifdef _WIN32
//void CSimulationTimer::ExecuteCallback()
//{
//    if (m_rtimersvc.GetStatus() != sdv::EObjectStatus::running) return;
//    if (!m_pExecute) return;
//    if (!m_bPrioritySet)
//        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
//    m_bPrioritySet = true;
//
//    if (m_pExecute) m_pExecute->Execute();
//}
//#endif

CSimulationTaskTimerService::CSimulationTaskTimerService()
{
}

CSimulationTaskTimerService::~CSimulationTaskTimerService()
{
}

void CSimulationTaskTimerService::Initialize(/*in*/ const sdv::u8string& /*ssObjectConfig*/)
{
#ifdef _WIN32
    m_eObjectStatus = sdv::EObjectStatus::initializing;
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // set up time resolution and maybe some other things
    timeBeginPeriod(1);
#endif
    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CSimulationTaskTimerService::GetStatus() const
{
    return m_eObjectStatus;
}

void CSimulationTaskTimerService::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
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

void CSimulationTaskTimerService::Shutdown()
{
#ifdef _WIN32
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;
    timeEndPeriod(1);
#endif
    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::IInterfaceAccess* CSimulationTaskTimerService::CreateTimer(uint32_t uiPeriod, sdv::IInterfaceAccess* pTask)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;
    if (!uiPeriod) return nullptr;
    if (!pTask) return nullptr;
    sdv::core::ITaskExecute* pExecute = pTask->GetInterface<sdv::core::ITaskExecute>();
    if (!pExecute) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxTasks);
    auto ptrTimer = std::make_unique<CSimulationTimer>(*this, uiPeriod, pExecute);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrTimer)
        return nullptr;
    CSimulationTimer* pObject = ptrTimer.get();
    if (pObject)
        m_mapTasks.try_emplace(pObject, std::move(ptrTimer));
    return pObject;
}

void CSimulationTaskTimerService::SimulationStep(uint64_t uiSimulationStep)
{
    std::unique_lock<std::mutex> lock(m_mtxTasks);
    for (auto it = m_mapTasks.begin(); it != m_mapTasks.end(); it++)
    {
        it->first->SimulationStep(uiSimulationStep);
    }
}

void CSimulationTaskTimerService::RemoveTimer(CSimulationTimer* pTimer)
{
    std::unique_lock<std::mutex> lock(m_mtxTasks);
    m_mapTasks.erase(pTimer);
}
