 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include "vehicle_abstraction_application.h"
#include "../generated/vss_files/signal_identifier.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

bool CAbstractionControl::LoadConfigFile(const std::string& inputMsg, const std::string& configFileName)
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

bool CAbstractionControl::IsSDVFrameworkEnvironmentSet()
{
    const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
    if (envVariable)
    {
        return true;
    }

    return false;
}

bool CAbstractionControl::Initialize(bool bSimulate)
{
    if (m_bInitialized) 
        return true;

    m_bSimulate = bSimulate;
    if (!IsSDVFrameworkEnvironmentSet())
    {
        // if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
        m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
    }

    if (!m_appcontrol.Startup(""))
        return false; 

    m_appcontrol.SetConfigMode();
    bool bResult = LoadConfigFile("Load dispatch service: ", "data_dispatch_vehicle_abstraction.toml");
    bResult &= LoadConfigFile("Load task_timer_vehicle: ", "task_timer_vehicle.toml");

    if (bSimulate)
    {
        bResult &= LoadConfigFile("Load can_com_simulation_vehicle_abstraction_kmh: ", "can_com_simulation_vehicle_abstraction_kmh.toml");
        bResult &= LoadConfigFile("Load data_link_vehicle_abstraction: ", "data_link_vehicle_abstraction.toml"); 
        bResult &= LoadConfigFile("Load vehicle_abstraction_kmh: ", "vehicle_abstraction_device_kmh.toml");          
    }
    else
    {
        bResult &= LoadConfigFile("Load can_com_simulation_vehicle_abstraction_ms: ", "can_com_simulation_vehicle_abstraction_ms.toml");
        bResult &= LoadConfigFile("Load data_link_vehicle_abstraction: ", "data_link_vehicle_abstraction.toml"); 
        bResult &= LoadConfigFile("Load vehicle_abstraction_ms: ", "vehicle_abstraction_device_ms.toml");      
    }

    bResult &= LoadConfigFile("Load vehicle_abstraction_basic_service: ", "vehicle_abstraction_basic_service.toml");      

    if (!bResult) 
    {
		SDV_LOG_ERROR("One or more configurations could not be loaded. Cannot continue.");
        return false;
    }

    m_bInitialized = true;

    return true;
}


void CAbstractionControl::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

void CAbstractionControl::SetRunningMode()
{
    m_appcontrol.SetRunningMode();
}

