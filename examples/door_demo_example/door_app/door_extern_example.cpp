#include <iostream>
#include "../door_app/include/door_extern_application.h"
#include "../door_app/include/console.h"

int main()
{
    CDoorExternControl appobj;
    if (!appobj.Initialize())
    {
        std::cout << "ERROR: Failed to initialize application control." << std::endl;
        return 0;
    }

    CConsole visual_obj;
    visual_obj.SetExternalApp();
    visual_obj.PrintHeader(4);
    visual_obj.PrepareDataConsumers();
    visual_obj.StartUpdateDataThread();

    appobj.RunUntilBreak();

    visual_obj.StopUpdateDataThread();
    visual_obj.ResetSignals();

    appobj.Shutdown();
    return 0;
}
