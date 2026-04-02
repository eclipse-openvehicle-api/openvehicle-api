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
#include <fstream>
#include <filesystem>
#include <sstream>
#include <ostream>
#include "vehicle_abstraction_application.h"
#include "console.h"


#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain([[maybe_unused]] int argc, [[maybe_unused]] wchar_t* argv[])
{
#else
extern "C" int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
#endif
    CConsole visual_obj;
    CAbstractionControl appobj;
    if (!appobj.Initialize(visual_obj.SelectInputDataUnits()))
    {
        std::cout << "ERROR: Failed to initialize application control." << std::endl;
        return 0;
    }

    visual_obj.PrintHeader();
    visual_obj.PrepareDataConsumers();

    appobj.SetRunningMode();

    visual_obj.RunUntilBreak();

    visual_obj.ResetSignals();

    appobj.Shutdown();
    return 0;
}
