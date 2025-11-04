#ifndef TRIGGER_H
#define TRIGGER_H

#include <support/interface_ptr.h>
#include <support/timer.h>

// Forward declaration
class CDispatchService;
class CSignal;
class CTrigger;

/**
 * @brief Flag indicating whether the execution is part of a periodic update or is spontaneous.
 */
enum class EExecutionFlag
{
    periodic,       ///< Execution is triggered periodically
    spontaneous     ///< Spontaneous execution triggered
};

/**
 * @brief Scheduler class being used to schedule a deferred trigger execution when the minimal time between execution is undercut.
*/
class CScheduler
{
public:
    /**
     * @brief Default constructor
     */
    CScheduler() = default;

    /**
     * @brief Destructor
     */
    ~CScheduler();

    /**
     * @brief Start the scheduler. This will start the scheduler timer at a 1ms rate.
     */
    void Start();

    /**
     * @brief Stop the scheduler. This will stop the scheduler timer and clear all pending schedule jobs.
     */
    void Stop();

    /**
     * @brief Schedule a new trigger execution for the trigger object.
     * @details If a scheduled trigger execution already exists for the object, the trigger execution is not scheduled again. If
     * the already scheduled trigger execution was marked as periodic and the new trigger is a spontenous trigger, the scheduled
     * execution is updated to represent a spontaneous execution (periodic triggers are sometimes not fired when the data hasn't
     * changed).
     * @param[in] pTrigger Pointer to the trigger object.
     * @param[in] eExecFlag Set the trigger execution flag.
     * @param[in] tpDue The due time this trigger is to be executed.
    */
    void Schedule(CTrigger* pTrigger, EExecutionFlag eExecFlag, std::chrono::high_resolution_clock::time_point tpDue);

    /**
     * @brief In case the trigger object will be destroyed, remove all pending schedule jobs from the scheduler.
     * @param[in] pTrigger Pointer to the trigger object.
     */
    void RemoveFromSchedule(const CTrigger* pTrigger);

private:
    /**
     * @brief Evaluate a potential job execution.
     */
    void EvaluateAndExecute();

    /// Map containing the scheduled objects and the execution flag for the object.
    using CObjectMap = std::map<CTrigger*, EExecutionFlag>;

    /// Multi-map containing the scheduled target time and an iterator to the entry of the scheduler object map.
    using CSchedulerMMap = std::multimap<std::chrono::high_resolution_clock::time_point, CObjectMap::iterator>;

    sdv::core::CTaskTimer       m_timer;                ///< 1ms timer
    std::mutex                  m_mtxScheduler;         ///< Protection of the schedule list
    CObjectMap                  m_mapTriggers;          ///< Map with the currently scheduled trigger objects (prevents new
                                                        ///< triggers to be scheduled for the same trigger object).
    CSchedulerMMap              m_mmapScheduleList;     ///< Schedule lst.
};

/**
 * @brief Trigger object, managing the triggering.
 */
class CTrigger : public sdv::IInterfaceAccess, public sdv::core::ITxTrigger, public sdv::IObjectDestroy
{
public:
    /**
    * @brief Constructor
    * @param[in] rDispatchSvc Reference to the dispatch service.
    * @param[in] uiCycleTime When set to any value other than 0, provides a cyclic trigger (ms). Could be 0 if cyclic
    * triggering is not required.
    * @param[in] uiDelayTime When set to any value other than 0, ensures a minimum time between two triggers. Could be 0
    * if minimum time should not be enforced.
    * @param[in] uiBehaviorFlags Zero or more flags from sdv::core::ETxTriggerBehavior.
    * @param[in] pCallback Pointer to the callback interface.
    */
    CTrigger(CDispatchService& rDispatchSvc, uint32_t uiCycleTime, uint32_t uiDelayTime, uint32_t uiBehaviorFlags,
        sdv::core::ITxTriggerCallback* pCallback);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::core::ITxTrigger)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Return whether the trigger object is valid.
     * @return Returns the validity of the trigger.
     */
    bool IsValid() const;

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     */
    virtual void DestroyObject() override;

    /**
     * @brief Add a signal to the trigger object. The signal must be registered as TX signal before.
     * Overload of sdv::core::ITxTrigger::AddSignal.
     * @param[in] ssSignalName Name of the signal.
     * @return Returns whether adding the signal was successful.
     */
    virtual bool AddSignal(/*in*/ const sdv::u8string& ssSignalName) override;

    /**
     * @brief Remove a signal from the trigger object.Overload of sdv::core::ITxTrigger::RemoveSignal.
     * @param[in] ssSignalName Name of the signal.
     */
    virtual void RemoveSignal(/*in*/ const sdv::u8string& ssSignalName) override;

    /**
     * @brief This function is triggered every ms.
     * @param[in] eExecFlag When set, the trigger was caused by the periodic timer.
     */
    void Execute(EExecutionFlag eExecFlag = EExecutionFlag::spontaneous);

private:
    sdv::CLifetimeCookie                m_cookie = sdv::CreateLifetimeCookie(); ///< Lifetime cookie to manage the module lifetime.
    CDispatchService&                   m_rDispatchSvc;                         ///< Reference to dispatch service.
    sdv::core::CTaskTimer               m_timer;                                ///< Task timer event object.
    std::chrono::high_resolution_clock::time_point      m_tpLast;               ///< Timepoint of the last trigger
    size_t                              m_nPeriod = 0ull;                       ///< The period to trigger.
    size_t                              m_nDelay = 0ull;                        ///< The minimum delay between two triggers.
    uint32_t                            m_uiBehaviorFlags = 0ul;                ///< Additional behavior
    size_t                              m_nInactiveRepetition = 0ull;           ///< Count the amount of inactive executions.
    sdv::core::ITxTriggerCallback*      m_pCallback = nullptr;                  ///< Callback pointer
    std::mutex                          m_mtxSignals;                           ///< Signal map protection.
    std::map<sdv::u8string, CSignal*>   m_mapSignals;                           ///< Assigned signals.
};

#endif // ! defined TRIGGER_H