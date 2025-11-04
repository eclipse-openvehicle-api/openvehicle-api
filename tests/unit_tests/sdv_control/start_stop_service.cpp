#include "includes.h"
#include "../../../sdv_executables/sdv_control/list_elements.h"
#include "../../../sdv_executables/sdv_control/start_stop_service.h"

/*
* The following tests are needed: (- not implemented; + implemented)
* + Start complex service by class and name
* - Start complex service by name
* + Stop complex service by name
* - Start complex service depending on other services
* - Stop complex service depending on other services
* - Stop complex service being dependent on by other services - other services stop as well
* - Stop complex service being dependent on by other component - fails
* - Start complex service depending on non-existing service - fails
* - Start other component - fails
* - Stop other component - fails
*/

/**
 * @brief Find the SDVControl_Test_PreInstalled_ComplexService component.
 * @param[in] rAppControl Reference to the app control to create a context.
 * @return Return whether the service was found or not.
 */
inline bool ComplexServiceFound(const CAppControlHelper& rAppControl)
{
    std::stringstream sstreamTable;
    if (ListElements(rAppControl.CreateContext("LIST COMPONENTS"), sstreamTable) != 0) return false;
    auto vecTable = InterpretTable(sstreamTable.str());
    for (const auto& rvecRow : vecTable)
    {
        if (rvecRow.size() < 2) continue;
        if (rvecRow[2] == "SDVControl_Test_PreInstalled_ComplexService")
            return true;
    }
    return false;
}

TEST(SDV_Control_Test, DestroyComplexService)
{
    CAppControlHelper appcontrol(true, 2007);
    EXPECT_TRUE(appcontrol.ServerAutoStarted());

    // Find the SDVControl_Test_PreInstalled_ComplexService component
    EXPECT_TRUE(ComplexServiceFound(appcontrol));

    // Destroy the object
    EXPECT_EQ(StopService(appcontrol.CreateContext("STOP SDVControl_Test_PreInstalled_ComplexService")), 0);

    // Find the SDVControl_Test_PreInstalled_ComplexService component
    EXPECT_FALSE(ComplexServiceFound(appcontrol));
}

TEST(SDV_Control_Test, RestartComplexService)
{
    CAppControlHelper appcontrol(true, 2007);
    EXPECT_TRUE(appcontrol.ServerAutoStarted());

    // Find the SDVControl_Test_PreInstalled_ComplexService component
    EXPECT_TRUE(ComplexServiceFound(appcontrol));

    // Destroy the object
    EXPECT_EQ(StopService(appcontrol.CreateContext("STOP SDVControl_Test_PreInstalled_ComplexService")), 0);

    // Find the SDVControl_Test_PreInstalled_ComplexService component
    EXPECT_FALSE(ComplexServiceFound(appcontrol));

    // Destroy the object
    EXPECT_EQ(StartService(appcontrol.CreateContext("START SDVControl_Test_PreInstalled_ComplexService SDVControl_Test_PreInstalled_ComplexService")), 0);

    // Find the SDVControl_Test_PreInstalled_ComplexService component
    EXPECT_TRUE(ComplexServiceFound(appcontrol));
}
