#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include <filesystem>
#include "../../../global/exec_dir_helper.h"
#include "../../../global/base64.h"

#ifdef _WIN32
#include <Shlobj.h>
#endif

class CModuleControl_Install : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        DeleteTestDirs();
    }

    virtual void TearDown()
    {
        DeleteTestDirs();
    }

    static void SetUpTestCase() {}
    static void TearDownTestSuite() {}

    void DeleteTestDirs()
    {
        try
        {
            std::filesystem::remove_all(GetExecDirectory() / "module_test1");
        }
        catch (const std::filesystem::filesystem_error&)
        {}
        try
        {
            std::filesystem::remove_all(GetExecDirectory() / "module_test2");
        }
        catch (const std::filesystem::filesystem_error&)
        {}
        try
        {
            std::filesystem::remove_all(GetDefaultInstallDir());
        }
        catch (const std::filesystem::filesystem_error&)
        {}
    }

    /**
     * @brief Default installation directory.
     * @return The default installation directory.
     */
    std::filesystem::path GetDefaultInstallDir();
};

inline std::filesystem::path CModuleControl_Install::GetDefaultInstallDir()
{
//    // The default directory for Windows is located at $ProgramData/ and for Posix at the executable.
    std::filesystem::path pathInstallDir;
//#ifdef _WIN32
//    wchar_t* szPath = nullptr;
//    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, 0, &szPath);
//    if (SUCCEEDED(hr) && szPath)
//        pathInstallDir = szPath;
//    if (szPath) CoTaskMemFree(szPath);
//#elif defined __unix__
    pathInstallDir = GetExecDirectory();
//#else
//#error OS is not supported!
//#endif
    // Append the installation directory with /<instance_ID>/.
    pathInstallDir /= "2006";

    return pathInstallDir;
}

TEST_F(CModuleControl_Install, MainApp_DefaultInstallDir)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2006");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetDefaultInstallDir()));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, IsolatedApp_DefaultInstallDir)
{
    sdv::app::CAppControl control;
    std::string ssConnectionString = Base64EncodePlainText("");
    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2006
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = control.Startup(ssConfig);
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetDefaultInstallDir()));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, OtherApp_DefaultInstallDir)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_FALSE(std::filesystem::exists(GetDefaultInstallDir()));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, MainApp_BlockDefaultInstallDir)
{
    // Prevent error reporting on std::cerr - they will influence test outcome.
    auto* pCErr = std::cerr.rdbuf();
    std::ostringstream sstreamCErr;
    std::cerr.rdbuf(sstreamCErr.rdbuf());

    // Create a blocking file... cannot create directory!
    std::ofstream fstream(GetDefaultInstallDir());
    ASSERT_TRUE(fstream.is_open());
    fstream << "Dummy string...";
    fstream.close();

    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=2006");
    EXPECT_FALSE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetDefaultInstallDir()));

    std::cerr.rdbuf(pCErr);
}

TEST_F(CModuleControl_Install, IsolatedApp_BlockDefaultInstallDir)
{
    // Prevent error reporting on std::cerr - they will influence test outcome.
    auto* pCErr = std::cerr.rdbuf();
    std::ostringstream sstreamCErr;
    std::cerr.rdbuf(sstreamCErr.rdbuf());

    std::ofstream fstream(GetDefaultInstallDir());
    ASSERT_TRUE(fstream.is_open());
    fstream << "Dummy string...";
    fstream.close();

    sdv::app::CAppControl control;
    bool bResult = control.Startup("[Application]\nMode=\"Isolated\"\nInstance=2006");
    EXPECT_FALSE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetDefaultInstallDir()));

    std::cerr.rdbuf(pCErr);
}

TEST_F(CModuleControl_Install, OtherApp_BlockDefaultInstallDir)
{
    std::ofstream fstream(GetDefaultInstallDir());
    ASSERT_TRUE(fstream.is_open());
    fstream << "Dummy string...";
    fstream.close();

    sdv::app::CAppControl control;
    bool bResult = control.Startup("");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetDefaultInstallDir()));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, MainApp_CustomInstallDirRelative)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(R"code(
[Application]
Mode="Main"
Instance=2006
InstallDir = "module_test1"
)code");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetExecDirectory() / "module_test1"));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, IsolatedApp_CustomInstallDirRelative)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(std::string(R"code(
[Application]
Mode="Isolated"
Instance=2006
InstallDir = "module_test1"
Connection = ")code") + Base64EncodePlainText("test") + "\"");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetExecDirectory() / "module_test1"));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, OtherApp_CustomInstallDirRelative)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(R"code([Application]
Install.Dir = "module_test1"
)code");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "module_test1"));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, MainApp_CustomInstallDirAbsolute)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(std::string(R"code(
[Application]
Mode="Main"
Instance=2006
InstallDir = ")code") + (GetExecDirectory() / "module_test2").generic_u8string() + "\"");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetExecDirectory() / "module_test2"));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, IsolatedApp_CustomInstallDirAbsolute)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(std::string(R"code(
[Application]
Mode="Isolated"
Instance=2006
InstallDir = ")code") + (GetExecDirectory() / "module_test2").generic_u8string() + "\"" + R"code(
Connection = ")code" + Base64EncodePlainText("test") + "\"");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_TRUE(std::filesystem::exists(GetExecDirectory() / "module_test2"));

    control.Shutdown();
}

TEST_F(CModuleControl_Install, OtherApp_CustomInstallDirAbsolute)
{
    sdv::app::CAppControl control;
    bool bResult = control.Startup(std::string(R"code([Application]
InstallDir = ")code") + (GetExecDirectory() / "module_test2").generic_u8string() + "\"");
    EXPECT_TRUE(bResult);

    // The default installation directory should be created.
    EXPECT_FALSE(std::filesystem::exists(GetExecDirectory() / "module_test2"));

    control.Shutdown();
}

