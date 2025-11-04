#include "includes.h"
#include "../../../global/process_watchdog.h"
#include "../../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../../sdv_executables/sdv_control/startup_shutdown.cpp"
#include "../../../sdv_executables/sdv_control/list_elements.cpp"
#include "../../../sdv_executables/sdv_control/start_stop_service.cpp"

/**
* @brief Main function
*/
#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
