/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

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
class CSimulationTaskTimerService : public sdv::CSdvObject, public sdv::core::ITaskTimer, public sdv::core::ITimerSimulationStep
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
        SDV_INTERFACE_ENTRY(sdv::core::ITaskTimer)
        SDV_INTERFACE_ENTRY(sdv::core::ITimerSimulationStep)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("SimulationTaskTimerService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

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
    std::mutex                                                     m_mtxTasks;  ///< Mutex for tasks
    std::map<CSimulationTimer*, std::unique_ptr<CSimulationTimer>> m_mapTasks;  ///< Set to get the active tasks
};

DEFINE_SDV_OBJECT(CSimulationTaskTimerService)

#endif // !define SIMULATION_TASK_TIMER_H
