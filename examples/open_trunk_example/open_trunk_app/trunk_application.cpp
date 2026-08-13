 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include "trunk_application.h"
#include "../generated/vss_files/signal_identifier.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

bool CTrunkControl::LoadConfigFile(const std::string& inputMsg, const std::string& configFileName)
{
    std::string msg = inputMsg;
    if (m_appcontrol.LoadConfig(configFileName) == sdv::core::EConfigProcessResult::successful)
    {
         msg.append("ok\n");
        std::cout << msg.c_str();
        return true;
    }

    msg.append("FAILED.\n");
    std::cout << msg.c_str();
    return false;
}

bool CTrunkControl::IsSDVFrameworkEnvironmentSet()
{
    const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
    if (envVariable)
    {
        return true;
    }

    return false;
}

bool CTrunkControl::Initialize(bool bSimulate, uint32_t uiInstance)
{
    if (m_bInitialized) 
        return true;

    if (!IsSDVFrameworkEnvironmentSet())
    {
        // if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
        m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
    }

    if (uiInstance != 0 && !bSimulate)
    {
        std::stringstream sstreamAppConfig;
        sstreamAppConfig << "[Application]" << std::endl;
        sstreamAppConfig << "Mode=\"External\"" << std::endl;
        sstreamAppConfig << "Instance=" << uiInstance << std::endl;
        sstreamAppConfig << "Retries=" << 6 << std::endl;
        sstreamAppConfig << "[Console]" << std::endl;
        sstreamAppConfig << "Report=\"Silent\"" << std::endl;
        if (!m_appcontrol.Startup(sstreamAppConfig.str()))
            return false;
    }
    else
    {
        if (!m_appcontrol.Startup(""))
            return false; 

        // Switch to config mode.
        m_appcontrol.SetConfigMode();
        bool bResult = LoadConfigFile("Load dispatch service: ", "data_dispatch_trunk.toml");
        bResult &= LoadConfigFile("Load task_timer_trunk: ", "task_timer_trunk.toml");

        if (bSimulate)
        {
            sdv::core::CDispatchService dispatch;
            m_signalSpeed = dispatch.RegisterRxSignal("CAN_Input.Speed");
            m_signalTrunk = dispatch.RegisterTxSignal("CAN_Output.OpenTrunk", 0);          
        }
        else
        {
            bResult &= LoadConfigFile("Load can_com_simulation_trunk: ", "can_com_simulation_trunk.toml");
            bResult &= LoadConfigFile("Load data_link_trunk: ", "data_link_trunk.toml");
        }

        bResult &= LoadConfigFile("Load trunk_vehicle_device_and_basic_service: ", "trunk_vehicle_device_and_basic_service.toml");
        //bResult &= LoadConfigFile("Load trunk service (complex service): ", "complex_service_trunk.toml");

        if (!bResult) 
        {
		    SDV_LOG_ERROR("One or more configurations could not be loaded. Cannot continue.");
            return false;
        }
    }

    m_bInitialized = true;
    return true;
}


void CTrunkControl::Shutdown()
{
    m_bSimulateRunning = false;
    if (m_threadSimulate.joinable())
        m_threadSimulate.join();

    if (m_signalSpeed)
    {
        m_signalSpeed.Reset();
    }; 
    if (m_signalTrunk)
    {
        m_signalTrunk.Reset();
    };     

    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

void CTrunkControl::SetRunningMode()
{
    m_appcontrol.SetRunningMode();
}
void CTrunkControl::StartSimulation()
{
    m_bSimulateRunning = true;
    m_threadSimulate = sdv::core::secure_thread(&CTrunkControl::SimulateThreadFunction, this);     
}


void CTrunkControl::SimulateThreadFunction()
{
    bool bIsMoving = true;
    uint32_t maxLoopCount  = 20;     
    m_signalSpeed.Write(0);

    while (m_bSimulateRunning)
    {      
        uint32_t loop  = 0;
        uint32_t speed  = 0;  
        while (loop < maxLoopCount && m_bSimulateRunning)
        {
            if (bIsMoving)
            {
                speed = loop * 5;
            }

            m_signalSpeed.Write(speed);
            loop++;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }        

        bIsMoving = !bIsMoving;
    }
}
