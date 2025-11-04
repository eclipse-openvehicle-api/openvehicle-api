#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include "../../../global/exec_dir_helper.h"

TEST(CoreLibrary_AppControl, SetModuleSearchPath)
{
    sdv::core::IModuleControlConfig* pModuleConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
    ASSERT_NE(pModuleConfig, nullptr);
    bool bResult = pModuleConfig->AddModuleSearchDir("../../bin");
    EXPECT_TRUE(bResult);
    sdv::sequence<sdv::u8string> seqSearchDirs = pModuleConfig->GetModuleSearchDirs();
    std::filesystem::path pathModuleDir = (GetExecDirectory() / "../../bin").lexically_normal();
    auto itDir = std::find_if(seqSearchDirs.begin(), seqSearchDirs.end(),
        [&](const sdv::u8string& rssDir)
        {
            std::filesystem::path pathDir = static_cast<std::string>(rssDir);
            if (pathDir.is_relative())
                pathDir = (GetExecDirectory() / pathDir).lexically_normal();
            return pathDir == pathModuleDir;
        });
    EXPECT_NE(itDir, seqSearchDirs.end());
}

TEST(CoreLibrary_AppControl, SetModuleSearchPathMgntClass)
{
    sdv::app::CAppControl appcontrol;
    bool bResult = appcontrol.AddModuleSearchDir("../../bin");
    EXPECT_TRUE(bResult);
}

