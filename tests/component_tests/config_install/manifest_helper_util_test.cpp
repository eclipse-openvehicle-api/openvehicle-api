#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include <filesystem>
#include "../../../global/exec_dir_helper.h"

TEST(ManifestHelperUtil, Instantiate)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    sdv::core::TModuleID tManifestModule = pModuleControl->Load("manifest_util.sdv");
    ASSERT_NE(tManifestModule, 0);

    sdv::TObjectPtr ptrManifestUtil = sdv::core::CreateUtility("ManifestHelperUtility");
    EXPECT_TRUE(ptrManifestUtil);
    sdv::helper::IModuleManifestHelper* pManifestHelper = ptrManifestUtil.GetInterface<sdv::helper::IModuleManifestHelper>();
    EXPECT_NE(pManifestHelper, nullptr);
    ptrManifestUtil.Clear();

    pModuleControl->Unload(tManifestModule);

    control.Shutdown();
}

TEST(ManifestHelperUtil, NonExistingComponent)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    sdv::core::TModuleID tManifestModule = pModuleControl->Load("manifest_util.sdv");
    ASSERT_NE(tManifestModule, 0);

    sdv::TObjectPtr ptrManifestUtil = sdv::core::CreateUtility("ManifestHelperUtility");
    EXPECT_TRUE(ptrManifestUtil);
    sdv::helper::IModuleManifestHelper* pManifestHelper = ptrManifestUtil.GetInterface<sdv::helper::IModuleManifestHelper>();
    EXPECT_NE(pManifestHelper, nullptr);

    sdv::u8string ssManifest = pManifestHelper->ReadModuleManifest("non_existing_component");
    EXPECT_TRUE(ssManifest.empty());

    ptrManifestUtil.Clear();
    pModuleControl->Unload(tManifestModule);

    control.Shutdown();
}

TEST(ManifestHelperUtil, ExistingComponent)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    sdv::core::TModuleID tManifestModule = pModuleControl->Load("manifest_util.sdv");
    ASSERT_NE(tManifestModule, 0);

    sdv::TObjectPtr ptrManifestUtil = sdv::core::CreateUtility("ManifestHelperUtility");
    EXPECT_TRUE(ptrManifestUtil);
    sdv::helper::IModuleManifestHelper* pManifestHelper = ptrManifestUtil.GetInterface<sdv::helper::IModuleManifestHelper>();
    EXPECT_NE(pManifestHelper, nullptr);

    sdv::u8string ssManifest = pManifestHelper->ReadModuleManifest((GetExecDirectory() /
        "ComponentTest_ConfigInstall_Module.sdv").generic_u8string());
    EXPECT_FALSE(ssManifest.empty());

    ptrManifestUtil.Clear();
    pModuleControl->Unload(tManifestModule);

    control.Shutdown();
}

