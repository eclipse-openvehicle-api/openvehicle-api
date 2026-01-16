#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../sdv_services/ipc_com/com_ctrl.cpp"
#include "../../../sdv_services/ipc_com/com_channel.cpp"
#include "../../../sdv_services/ipc_com/marshall_object.cpp"
#include <cstring>

/**
* @brief Main function
*/
#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    // Check for the --gtest_repeat option.
    bool bRepeatEnabled = false;
    for (int iIndex = 0; iIndex < argc; iIndex++)
    {
        if (!argv[iIndex]) continue;
#if defined(_WIN32) && defined(_UNICODE)
        bRepeatEnabled |= std::wcsncmp(argv[iIndex], L"--gtest_repeat", 14) == 0;
#else
        bRepeatEnabled |= std::strncmp(argv[iIndex], "--gtest_repeat", 14) == 0;
#endif
    }

    // When repeat is enabled, do not enable the watchdog.
    if (bRepeatEnabled)
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    else
    {
        CProcessWatchdog watchdog;

        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
}
