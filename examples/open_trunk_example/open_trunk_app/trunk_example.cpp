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
#include <cstdlib> // for std::strtol
#include "trunk_application.h"
#include "console.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
{
    bool bSimulate = false;
    uint32_t uiInstance = 0;
    if (argc < 2)
    {
        std::cout << "Parameter (instance number to connect to) missing. 0 or 1 will run a standalone application." << std::endl;
        return 1;
    }
    try
    {
        uiInstance = std::stoi(argv[1]);
        if (uiInstance == 1)
        {
            bSimulate = true;
        }
    }
    catch (const std::exception& )
    {
        uiInstance = 0;
    }

#else
extern "C" int main(int argc, char* argv[])
{
    bool bSimulate = false;
    uint32_t uiInstance = 0;
    if (argc < 2) 
    {
        std::cout << "Parameter (instance number to connect to) missing. 0 or 1 will run a standalone application." << std::endl;
        return 1;
    }
    try 
    {
        uiInstance = std::stoi(argv[1]);
        if (uiInstance == 1)
        {
            bSimulate = true;
        }        
    }
    catch (const std::exception& ) 
    {
        uiInstance = 0;
    }
#endif
    CTrunkControl appobj;
    if (!appobj.Initialize(bSimulate, uiInstance))
    {
        std::cout << "ERROR: Failed to initialize application control." << std::endl;
        return 0;
    }

    CConsole visual_obj;
    visual_obj.PrintHeader(bSimulate, uiInstance);
    visual_obj.PrepareDataConsumers();

    appobj.SetRunningMode();
    if (bSimulate)
    {
        appobj.StartSimulation();
        visual_obj.RunUntilBreak();
    }
    else
    {
        visual_obj.RunUntilBreak();
        visual_obj.ResetSignals();
    }

    appobj.Shutdown();
    return 0;
}
