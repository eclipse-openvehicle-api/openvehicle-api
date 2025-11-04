/**
* @file simulationtasktimer.h
* @author Sudipta Babu Durjoy FRD DISS21 (mailto:sudipta.durjoy@zf.com)
* @brief
* @version 1.0
* @date 2023-05-24
*
* @copyright Copyright ZF Friedrichshafen AG (c) 2023
*
*/

#ifndef SIMULATION_TASK_TIMER_H
#define SIMULATION_TASK_TIMER_H

#include <interfaces/core.h>
#include <interfaces/timer.h>
#include <support/interface_ptr.h>
#include <support/component_impl.h>
#include <map>
#include <set>
#include <fstream>

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <timeapi.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#elif defined __unix__
#include <signal.h>
#include <unistd.h>
#include <mutex>
#include <time.h>
#include <sys/times.h>
#else
#error OS is not supported!
#endif

// Forward declaration
class CSimulationTaskTimerService;

/**
 * @brief Timer object managing the lifetime of the timer.
*/
class CSimulationTimer : public sdv::IInterfaceAccess, public sdv::IObjectDestroy, public sdv::core::ITimerSimulationStep
{
public:
    /**
     * @brief Constructor
     * @param[in] rtimersvc Reference to the task timer service.
     * @param[in] uiPeriod The period of the task timer (must not be 0) in ms.
     * @param[in] pExecute Pointer to the interface containing the execution function.
     */
    CSimulationTimer(CSimulationTaskTimerService& rtimersvc, uint32_t uiPeriod, sdv::core::ITaskExecute* pExecute);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_ENTRY(sdv::core::ITimerSimulationStep)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     */
    virtual void DestroyObject() override;

    /**
     * @brief Method to set the time which has past from the last simulation step.
     * @param[in] uiSimulationStep the time in microseconds which has past from the last simulation step.
     */
    virtual void SimulationStep(/*in*/ uint64_t uiSimulationStep) override;

    /**
     * @brief Operator returning info about the validity of the timer.
    */
    operator bool() const;

private:
//#ifdef _WIN32
//    /**
//     * @brief Execution function called by the timer.
//     */
//    void ExecuteCallback();
//
//#endif

    sdv::CLifetimeCookie         m_cookie = sdv::CreateLifetimeCookie(); ///< Lifetime cookie to manage the module lifetime.
    CSimulationTaskTimerService& m_rtimersvc;                            ///< Reference to the task timer service
    sdv::core::ITaskExecute*     m_pExecute = nullptr;                   ///< Pointer to the execution callback interface.
    uint64_t                     m_uiInitializedPeriod = 0;              ///< Period in microseconds when timer is initialized.
    uint64_t                     m_uiPeriod = 0;                         ///< Decreasing period counter, if 0 execution is called.
    bool                         m_bRunning = false;                     ///< When set, the timer is running.
    bool                         m_bPrioritySet = false;                 ///< When set, the priority of the task was increased.
};

/**
* @brief Task timer class to execute task periodically
*/
class CSimulationTaskTimerService : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::core::ITaskTimer,
    public sdv::core::ITimerSimulationStep
{
public:
    /**
    * @brief Constructor configure and initializes the timer.
    */
    CSimulationTaskTimerService();

    /**
    * @brief Destructor cleans up the timer if there is no active task and delete it after.
    */
    virtual ~CSimulationTaskTimerService() override;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::core::ITaskTimer)
        SDV_INTERFACE_ENTRY(sdv::core::ITimerSimulationStep)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("SimulationTaskTimerService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(/*in*/ const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    virtual void SetOperationMode(/*in*/ sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown() override;

    /**
     * @brief Method to execute the user-defined task periodically until ShutdownTask is called.
     * @param[in] uiPeriod The time period in milliseconds in which the task should executed.
     * @param[in] pTask Interface to the task object exposing the ITaskExecute interface. The object must be kept alive
     * until the timer has been destroyed.
     * @return Returns an interface to the task timer object. Use sdv::IObjectDestroy to terminate the timer.
     */
    virtual sdv::IInterfaceAccess* CreateTimer(uint32_t uiPeriod, sdv::IInterfaceAccess* pTask) override;


    /**
     * @brief Method to set the time which has past from the last simulation step.
     * @param[in] uiSimulationStep the time in microseconds which has past from the last simulation step.
     */
    virtual void SimulationStep(/*in*/ uint64_t uiSimulationStep) override;

    /**
     * @brief Remove the timer from from the timer map.
     * @param[in] pTimer Pointer to the timer object to remove.
     */
    void RemoveTimer(CSimulationTimer* pTimer);

private:
    sdv::EObjectStatus      m_eObjectStatus = sdv::EObjectStatus::initialization_pending; ///< Object operation status
    std::mutex                                                     m_mtxTasks;            ///< Mutex for tasks
    std::map<CSimulationTimer*, std::unique_ptr<CSimulationTimer>> m_mapTasks;            ///< Set to get the active tasks
};

DEFINE_SDV_OBJECT(CSimulationTaskTimerService)

#endif // !define SIMULATION_TASK_TIMER_H
