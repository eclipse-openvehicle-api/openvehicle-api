 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include <iostream>
#include <chrono>
#include <support/component_impl.h>
#include <support/timer.h>
#include <support/toml.h>

class DemoTimerComponent : public sdv::CSdvObject
{
  public:
	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
	DECLARE_OBJECT_CLASS_NAME("Timer_Example")

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_PeriodicValue, "Timer", 10, "ms", "Periodic timer duration.")
    END_SDV_PARAM_MAP()

    /**
    * @brief initialize function to register, access the task timer interface from platform abstraction. Overload of
    * sdv::CSdvObject::OnInitialize.
    * After initialization 'CreateTimer' function is called to execute the task periodically.
    * @param[in] ssObjectConfig An object configuration is currently not used by this demo component.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
    */
    virtual bool OnInitialize() override
    {
        m_Timer = sdv::core::CTaskTimer(m_PeriodicValue, [&]() {CustomerExecute(); });
        if (!m_Timer)
        {
            SDV_LOG_ERROR("Tasktimer with ", std::to_string(m_PeriodicValue), " milliseconds could not be created.");
            return false;
        }
        else
        {
            SDV_LOG_INFO("Tasktimer created with ", std::to_string(m_PeriodicValue), " milliseconds");
        }
        return true;
    };

    /**
    * @brief Shutdown function is to shutdown the execution of periodic task. Overload of sdv::CSdvObject::OnShutdown.
    * Timer ID of the task is used to shutdown the specific task.
    */
    virtual void OnShutdown() override
    {
        if (m_Timer)
        {
            m_Timer.Reset();
        }
    }

    /**
    * @brief Execute function contains the task to be executed in provided period.
    * This function will be called periodically. Currently it's kept simple which just prints the current timestamp to the console.
    */
	void CustomerExecute() const
	{
		std::cout << "Executing the task with timestamp:" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
	};

  private:
    sdv::core::CTaskTimer m_Timer;             ///< timer 
    uint32_t m_PeriodicValue = 10;             ///< periodix in milliseconds
};

DEFINE_SDV_OBJECT(DemoTimerComponent)
