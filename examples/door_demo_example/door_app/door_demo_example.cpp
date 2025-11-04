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
