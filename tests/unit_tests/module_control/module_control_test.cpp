#include <gtest/gtest.h>
#include "mock.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../global/tracefifo/trace_fifo.cpp"

TEST(ModuleControlTest, OpenInvalidFileName)
{
    CModuleControl control;
    EXPECT_EQ(0, control.Load((GetExecDirectory() / "foobar42.sdv").generic_u8string().c_str()));
}

TEST(ModuleControlTest, OpenNullptrFileName)
{
    CModuleControl control;
    EXPECT_EQ(0, control.Load(nullptr));
}

TEST(ModuleControlTest, OpenNotAPathFileName)
{
    CModuleControl control;
    EXPECT_EQ(0, control.Load("ads�pwb�g�a"));
}

TEST(ModuleControlTest, AddResourceModulePathAbsolut)
{
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir((GetExecDirectory() / "../../bin").generic_u8string().c_str()));
    EXPECT_FALSE(control.AddModuleSearchDir((GetExecDirectory() / "../../something").generic_u8string().c_str()));
}
TEST(ModuleControlTest, AddResourceModulePathRelative)
{
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir("../bin"));
    EXPECT_FALSE(control.AddModuleSearchDir("../soemething"));
}

TEST(ModuleControlTest, AddResourceModulePathRelativeAndLoadModule)
{
    // Create a test subdirectory
    try
    {
        std::filesystem::remove_all(GetExecDirectory() / "unittest_module_control_rel");
        std::filesystem::create_directories(GetExecDirectory() / "unittest_module_control_rel");
        std::filesystem::copy_file(GetExecDirectory() / "../../bin/data_dispatch_service.sdv",
            GetExecDirectory() / "unittest_module_control_rel" / "test_rel.sdv");
    } catch (const std::filesystem::filesystem_error&)
    {}

    CModuleControl control;
    EXPECT_EQ(0, control.Load("test_rel.sdv"));

    EXPECT_TRUE(control.AddModuleSearchDir("unittest_module_control_rel"));

    sdv::core::TModuleID tModuleID = control.Load("test_rel.sdv");
    EXPECT_NE(tModuleID, 0u);
    EXPECT_NE(control.GetModule(tModuleID), nullptr);

    EXPECT_TRUE(control.Unload(tModuleID));

    // Clean up...
    try
    {
        std::filesystem::remove_all(GetExecDirectory() / "unittest_module_control_rel");
    } catch (const std::filesystem::filesystem_error&)
    {}
}


TEST(ModuleControlTest, AddResourceModulePathAbsolutAndLoadModule)
{
    // Create a test subdirectory
    try
    {
        std::filesystem::remove_all(GetExecDirectory() / "unittest_module_control_abs");
        std::filesystem::create_directories(GetExecDirectory() / "unittest_module_control_abs");
        std::filesystem::copy_file(GetExecDirectory() / "../../bin/data_dispatch_service.sdv",
            GetExecDirectory() / "unittest_module_control_abs" / "test_abs.sdv");
    } catch (const std::filesystem::filesystem_error&)
    {}

    CModuleControl control;
    EXPECT_EQ(0, control.Load("test.abs.sdv"));

    EXPECT_TRUE(control.AddModuleSearchDir((GetExecDirectory() / "unittest_module_control_abs").generic_u8string().c_str()));

    sdv::core::TModuleID tModuleID = control.Load("test_abs.sdv");

    EXPECT_NE(tModuleID, 0u);
    EXPECT_NE(control.GetModule(tModuleID), nullptr);

    EXPECT_TRUE(control.Unload(tModuleID));

    try
    {
        std::filesystem::remove_all(GetExecDirectory() / "unittest_module_control_abs");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST(ModuleControlTest, LoadModuleNoResourcePath)
{
    CModuleControl control;

    sdv::core::TModuleID tModuleID = control.Load((GetExecDirectory() / "../../bin/data_dispatch_service.sdv").generic_u8string().c_str());

    EXPECT_NE(tModuleID, 0u);
    EXPECT_NE(control.GetModule(tModuleID), nullptr);

    EXPECT_TRUE(control.Unload(tModuleID));
}

TEST(ModuleControlTest, HappyPath)
{
    //load repository service module
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir("../../bin"));

    sdv::core::TModuleID tModuleID = control.Load("data_dispatch_service.sdv");
    EXPECT_NE(tModuleID, 0u);
    std::shared_ptr<CModuleInst> ptrModule = control.GetModule(tModuleID);
    ASSERT_NE(ptrModule, nullptr);

    //create an object via factory and verify HasActiveObjects functionality
    EXPECT_FALSE(ptrModule->HasActiveObjects());
    auto pService = ptrModule->CreateObject("DataDispatchService");
    EXPECT_NE(pService, nullptr);
    EXPECT_TRUE(ptrModule->HasActiveObjects());
    EXPECT_NO_THROW(ptrModule->DestroyObject(pService));
    EXPECT_TRUE(control.Unload(tModuleID));
}

TEST(ModuleControlTest, OpenCloseDuplicateModule)
{
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir("../../bin"));

    sdv::core::TModuleID tModuleID_1 = control.Load("data_dispatch_service.sdv");
    EXPECT_NE(0, tModuleID_1);
    sdv::core::TModuleID tModuleID_2 = control.Load("data_dispatch_service.sdv");
    EXPECT_NE(0, tModuleID_2);
    EXPECT_EQ(tModuleID_1, tModuleID_2);

    // The decision to unload is based on running objects.
    EXPECT_TRUE(control.Unload(tModuleID_1));   // No more running objects; unload successful.
    EXPECT_TRUE(control.Unload(tModuleID_2));   // No module any more, unload succeeds since no running objects.
    EXPECT_TRUE(control.Unload(tModuleID_1));   // No module any more, unload succeeds since no running objects.
}

TEST(ModuleControlTest, AccessModuleAfterClose)
{
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir("../../bin"));

    sdv::core::TModuleID tModuleID = control.Load("data_dispatch_service.sdv");

    EXPECT_NE(tModuleID, 0u);
    EXPECT_NE(control.GetModule(tModuleID), nullptr);

    EXPECT_TRUE(control.Unload(tModuleID));
    EXPECT_EQ(control.GetModule(tModuleID), nullptr);
}

TEST(ModuleControlTest, LoadUnloadMultiModule)
{
    CModuleControl control;
    EXPECT_TRUE(control.AddModuleSearchDir("../../bin"));

    //load and unload module
    sdv::core::TModuleID tModuleID_1 = control.Load("data_dispatch_service.sdv");
    EXPECT_NE(tModuleID_1, 0);
    EXPECT_NE(control.GetModule(tModuleID_1), nullptr);

    EXPECT_TRUE(control.Unload(tModuleID_1));
    EXPECT_EQ(control.GetModule(tModuleID_1), nullptr);

    //load same module again
    sdv::core::TModuleID tModuleID_2 = control.Load("data_dispatch_service.sdv");
    EXPECT_NE(tModuleID_2, 0);
    EXPECT_NE(control.GetModule(tModuleID_2), nullptr);

    //load another module
    sdv::core::TModuleID tModuleID_3 = control.Load("core_ps.sdv");
    EXPECT_NE(tModuleID_3, 0);
    EXPECT_NE(control.GetModule(tModuleID_3), nullptr);
    ASSERT_NE(tModuleID_2, tModuleID_3);

    //unload other module - original module should still be loaded properly
    EXPECT_TRUE(control.Unload(tModuleID_3));

    EXPECT_EQ(control.GetModule(tModuleID_3), nullptr);

    EXPECT_NE(control.GetModule(tModuleID_2), nullptr);

    //unload remaining module
    EXPECT_TRUE(control.Unload(tModuleID_2));
    EXPECT_EQ(control.GetModule(tModuleID_2), nullptr);
}

