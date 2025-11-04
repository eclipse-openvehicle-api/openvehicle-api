#include "includes.h"
#include "../../../sdv_executables/sdv_control/list_elements.h"

/*
* The following tests are needed: (- not implemented; + implemented)
* - List modules - normal list
* - List modules - short list
* - List modules - ID list
* - List classes - normal list
* - List classes - short list
* + List components - normal list
* - List components - short list
* - List components - ID list
* - List installations - normal list
* - List installations - short list
* - List installed files for an installation
* - List connections - normal list
* - List connections - short list
* - List connections - ID list
* - List information with additional external app
* - List filter
* - List header suppression
* - List dependencies in modules, classes and components
* - List location in components and connections
*/

TEST(SDV_Control_Test, DISABLED_ListComponents)
{
    CAppControlHelper appcontrol(true, 2007);
    EXPECT_TRUE(appcontrol.ServerAutoStarted());

    std::stringstream sstreamTable;
    EXPECT_EQ(ListElements(appcontrol.CreateContext("LIST COMPONENTS"), sstreamTable), 0);
    auto vecTable = InterpretTable(sstreamTable.str());
    std::cout << sstreamTable.str() << std::endl;

    bool bRepositoryFound = false;
    bool bComplexHelloServiceFound = false;
    for (const auto& rvecRow : vecTable)
    {
        if (rvecRow.size() < 2) continue;
        if (rvecRow[2] == "RepositoryService")
            bRepositoryFound = true;
        if (rvecRow[2] == "SDVControl_Test_PreInstalled_ComplexService")
            bComplexHelloServiceFound = true;
    }
    EXPECT_TRUE(bRepositoryFound);
    EXPECT_TRUE(bComplexHelloServiceFound);
}

