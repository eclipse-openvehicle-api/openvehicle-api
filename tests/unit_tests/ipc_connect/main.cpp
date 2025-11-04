#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../sdv_services/ipc_connect/listener.cpp"
#include "../../../sdv_services/ipc_connect/client.cpp"

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
