#include <gtest/gtest.h>
#include <support/app_control.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include "../../../global/exec_dir_helper.h"
#include "generated/test_component.h"

inline sdv::process::TProcessID GetProcessID()
{
    static sdv::process::TProcessID tProcessID = 0;
    if (!tProcessID)
    {
        sdv::process::IProcessInfo* pProcessInfo = sdv::core::GetObject<sdv::process::IProcessInfo>("ProcessControlService");
        if (!pProcessInfo) return 0;
        tProcessID = pProcessInfo->GetProcessID();
    }
    return tProcessID;
}

/**
 * @brief Automatically configure and start the system to do tests on the test component.
 */
class CIsolatedComponentTest :  public ::testing::Test, public sdv::app::CAppControl
{
public:
    static void SetUpTestCase() {}
    static void TearDownTestSuite() {}

    /**
     * @brief Setup test case
     */
    virtual void SetUp()
    {
        bool bRet = Startup(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Main"
Instance = 2005
)code");

        if (!bRet)
            std::cout << "Failed to start the application for instance 1234." << std::endl;
    }

    /**
     * @brief Tear down test case
     */
    virtual void TearDown()
    {
        Shutdown();
    }
};

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST_F(CIsolatedComponentTest, DISABLED_LoadNonIsolatedSystemService)
{
    // Load the hello system service
    IHello* pHello = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
    ASSERT_NE(pHello, nullptr);

    EXPECT_EQ(pHello->SayHello(), "Hello from system service");
    sdv::process::TProcessID tPID = pHello->GetPID();
    EXPECT_NE(tPID, 0u);
    EXPECT_EQ(tPID, GetProcessID());    // Runs in main process
}

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST_F(CIsolatedComponentTest, DISABLED_LoadNonIsolatedDevice)
{
    // Load the hello device
    IHello* pHello = sdv::core::GetObject<IHello>("TestObject_HelloDevice");
    ASSERT_NE(pHello, nullptr);

    EXPECT_EQ(pHello->SayHello(), "Hello from device");
    sdv::process::TProcessID tPID = pHello->GetPID();
    EXPECT_NE(tPID, 0u);
    EXPECT_EQ(tPID, GetProcessID());    // Runs in main process
}

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST_F(CIsolatedComponentTest, DISABLED_LoadNonIsolatedBasicService)
{
    // Load the hello basic service
    IHello* pHello = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
    ASSERT_NE(pHello, nullptr);

    EXPECT_EQ(pHello->SayHello(), "Hello from basic service");
    sdv::process::TProcessID tPID = pHello->GetPID();
    EXPECT_NE(tPID, 0u);
    EXPECT_EQ(tPID, GetProcessID());    // Runs in main process
}

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST_F(CIsolatedComponentTest, DISABLED_LoadIsolatedComplexService)
{
    // Load the hello basic service
    IHello* pHello = sdv::core::GetObject<IHello>("TestObject_ComplexHelloService");
    ASSERT_NE(pHello, nullptr);

    EXPECT_EQ(pHello->SayHello(), "Hello from complex service");
    sdv::process::TProcessID tPID = pHello->GetPID();
    EXPECT_NE(tPID, 0u);
    EXPECT_NE(tPID, GetProcessID());    // Runs in isolated process
}



/*
TEST the following:

- Main app crashes or ends... iso app also ends
- Iso app crashes/ends, object in main app will be removed
- iso and exetrnal apps are not allowed to create objects in main app
- dependencies
-   on startup create objects the object is dependent on first
-   on shutdown shutdown dependent objects first

// TODO EVE
// The following issues are still present:
//  Lifetime of remote object
//  Lifetime of iso process
//  Print information during verbose
//  Test when error occurs
//  Utilities during startup and when running (currently disabled: see CreateUtility)
//  Enabling the automatic test execution and compilation of repo tests: see CMakeLists in ComponentTest_Repository



*/