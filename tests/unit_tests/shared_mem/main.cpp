#include "gtest/gtest.h"
#include "../../../global/process_watchdog.h"
#define TIME_TRACKING
#include "../../../sdv_services/ipc_shared_mem/connection.cpp"
#include "../../../sdv_services/ipc_shared_mem/channel_mgnt.cpp"
#include "../../../sdv_services/ipc_shared_mem/watchdog.cpp"
#include "../../../sdv_services/ipc_shared_mem/mem_buffer_accessor.cpp"
#include <cstring>

/**
 * @brief Load support modules to publish the needed services.
 */
void LoadSupportServices()
{
    // Load the IPC modules
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("process_control.sdv"), 0u);

    // Create the services
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("ProcessControlService", {}, {}), 0u);
}

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
        if (!argv[iIndex])
            continue;
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
