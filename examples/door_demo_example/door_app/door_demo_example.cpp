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
#include "../door_app/include/door_application.h"
#include "../door_app/include/console.h"

int main()
{
    CDoorControl appobj;
    if (!appobj.Initialize(appobj.UserInputNumberOfDoors()))
    {
        std::cout << "ERROR: Failed to initialize application control." << std::endl;
        return 0;
    }

    CConsole visual_obj;
    visual_obj.PrintHeader(appobj.GetNumberOfDoors());
    visual_obj.PrepareDataConsumers();
    visual_obj.StartUpdateDataThread();
    
    appobj.SetRunningMode();
    appobj.RunUntilBreak();

    visual_obj.StopUpdateDataThread();
    visual_obj.ResetSignals();

    appobj.Shutdown();
    return 0;
}
