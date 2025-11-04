#include <iostream>
#include <cstdlib> // for std::strtol
#include "trunk_application.h"
#include "console.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
{
    uint32_t uiInstance = 0;
    if (argc < 2)
    {
        std::cout << "Missing instance number to connect to, '0' will run as standalone application" << std::endl;
        return 1;
    }
    try
    {
        uiInstance = std::stoi(argv[1]);
    }
    catch (const std::exception& )
    {
        uiInstance = 0;
    }

#else
extern "C" int main(int argc, char* argv[])
{
    uint32_t uiInstance = 0;
    if (argc < 2) 
    {
        std::cout << "Missing instance number to connect to, '0' will run as standalone application" << std::endl;
        return 1;
    }
    try 
    {
        uiInstance = std::stoi(argv[1]);
    }
    catch (const std::exception& ) 
    {
        uiInstance = 0;
    }
#endif
    CTrunkControl appobj;
    if (!appobj.Initialize(uiInstance))
    {
        std::cout << "ERROR: Failed to initialize application control." << std::endl;
        return 0;
    }

    CConsole visual_obj;
    visual_obj.PrintHeader(uiInstance);
    visual_obj.PrepareDataConsumers();

    appobj.SetRunningMode();
    visual_obj.RunUntilBreak();

    visual_obj.ResetSignals();

    appobj.Shutdown();
    return 0;
}
