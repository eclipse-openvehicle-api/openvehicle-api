#include "tasktimer.h"
#include <fstream>
#include <functional>

CTimer::CTimer(CTaskTimerService& rtimersvc, uint32_t uiPeriod, sdv::core::ITaskExecute* pExecute) :
    m_rtimersvc(rtimersvc), m_pExecute(pExecute)
{
    if (!pExecute) return;

#ifdef _WIN32
    // Use a lambda function without capture as function pointer.
    // NOTE: Casting is unsafe... but a necessity when working with Windows API functions.
    m_uiTimerID = timeSetEvent(uiPeriod, 0, [](UINT, UINT, DWORD_PTR dwUser, DWORD_PTR, DWORD_PTR)
        {
            CTimer* pTimer = reinterpret_cast<CTimer*>(dwUser);
            pTimer->ExecuteCallback();
        }, reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
#elif defined __unix__
    // Configure the signal event for timer expiration
    sigevent sev{};
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = [](sigval sv)
    {
        CTimer* pTimer = reinterpret_cast<CTimer*>(sv.sival_ptr);
        std::unique_lock<std::mutex> lock(pTimer->m_mtxExecution);
        if (!pTimer->m_bRunning) return;
        sdv::core::ITaskExecute* pExecuteLocal = reinterpret_cast<sdv::core::ITaskExecute*>(pTimer->m_pExecute);
        pExecuteLocal->Execute();
    };
    sev.sigev_value.sival_ptr = this;
    sev.sigev_notify_attributes = nullptr;

    // Create the timer
    if (timer_create(CLOCK_MONOTONIC, &sev, &m_timerid) != 0)
        return;

    //split up period in full seconds and remaining nanoseconds
    //find out how many full seconds period contains
    uint32_t uiFullSeconds = uiPeriod / 1000;
    //determine remainder in nanoseconds
    uint32_t uiRemainderNS = (uiPeriod % 1000)*1000000;

    // Configure the timer
    itimerspec its{};
    its.it_value.tv_sec = uiFullSeconds;
    its.it_value.tv_nsec = uiRemainderNS;
    its.it_interval.tv_sec = uiFullSeconds;
    its.it_interval.tv_nsec = uiRemainderNS;

    // Start the timer
    if (timer_settime(m_timerid, 0, &its, nullptr) != 0)
    {
        timer_delete(m_timerid);
        return;
    }

    m_bRunning = true;
#endif
}

void CTimer::DestroyObject()
{
#ifdef _WIN32
    // Terminate the timer
    if (m_uiTimerID)
        timeKillEvent(m_uiTimerID);
    m_uiTimerID = 0ul;
#elif defined __unix__
    // Terminate the timer
    if (m_bRunning)
    {
        std::unique_lock<std::mutex> lock(m_mtxExecution);
        m_bRunning = false;
        timer_delete(m_timerid);
        m_timerid = 0ul;
        lock.unlock();

        // Sleep 20ms to allow all outstanding tasks to end.
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
#endif

    // Delete the object
    m_rtimersvc.RemoveTimer(this);
}

CTimer::operator bool() const
{
#ifdef _WIN32
    return m_uiTimerID ? true : false;
#elif defined __unix__
    return m_bRunning;
#endif
}

#ifdef _WIN32
void CTimer::ExecuteCallback()
{
    if (m_rtimersvc.GetStatus() != sdv::EObjectStatus::running) return;
    if (!m_pExecute) return;
    if (!m_bPrioritySet)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    m_bPrioritySet = true;
    if (m_pExecute) m_pExecute->Execute();
}
#endif

CTaskTimerService::CTaskTimerService()
{
}

CTaskTimerService::~CTaskTimerService()
{
}

void CTaskTimerService::Initialize(/*in*/ const sdv::u8string& /*ssObjectConfig*/)
{
#ifdef _WIN32
    m_eObjectStatus = sdv::EObjectStatus::initializing;
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // set up time resolution and maybe some other things
    timeBeginPeriod(1);
#endif

    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CTaskTimerService::GetStatus() const
{
    return m_eObjectStatus;
}

void CTaskTimerService::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
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

void CTaskTimerService::Shutdown()
{
#ifdef _WIN32
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;
    timeEndPeriod(1);
#endif
    m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
}

sdv::IInterfaceAccess* CTaskTimerService::CreateTimer(uint32_t uiPeriod, sdv::IInterfaceAccess* pTask)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    if (!uiPeriod) return nullptr;
    if (!pTask) return nullptr;
    sdv::core::ITaskExecute* pExecute = pTask->GetInterface<sdv::core::ITaskExecute>();
    if (!pExecute) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxTasks);
    auto ptrTimer = std::make_unique<CTimer>(*this, uiPeriod, pExecute);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrTimer)
        return nullptr;
    CTimer* pObject = ptrTimer.get();
    if (pObject)
        m_mapTasks.try_emplace(pObject, std::move(ptrTimer));
    return pObject;
}

void CTaskTimerService::RemoveTimer(CTimer* pTimer)
{
    std::unique_lock<std::mutex> lock(m_mtxTasks);
    m_mapTasks.erase(pTimer);
}
