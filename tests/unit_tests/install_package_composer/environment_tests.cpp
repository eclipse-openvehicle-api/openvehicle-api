#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#endif

#include <gtest/gtest.h>
#include "../../../global/localmemmgr.h"
#include "../../../sdv_executables/sdv_packager/environment.cpp"
#include "../../../global/exec_dir_helper.h"
#include "composer_test_suite.h"

class CCmdlnStr: public std::vector<std::string>
{
public:
    CCmdlnStr(const std::string& rssString = {})
    {
        // Add the current exe as initial command line.
        push_back(GetExecFilename().generic_u8string());

        // Parse through the string and check for whitespace
        std::string ssCurrent;
        bool bInQuote = false;
        size_t nPos = 0;
        while (nPos < rssString.size())
        {
            // Get the character.
            char c = rssString[nPos];
            nPos++;

            switch (c)
            {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '\v':
                if (bInQuote)
                    ssCurrent += c;
                else
                {
                    if (!ssCurrent.empty())
                        push_back(ssCurrent);
                    ssCurrent.clear();
                }
                break;
            case '\\':
                if (nPos < rssString.size() && rssString[nPos] == '\"')
                {
                    nPos++;
                    ssCurrent += '\"';
                    break;
                }
                ssCurrent += c;
                break;
            case '\"':
                bInQuote = !bInQuote;
                break;
            default:
                ssCurrent += c;
                break;
            }
        }
        if (!ssCurrent.empty())
            push_back(ssCurrent);
    }
};

using CEnvironmentTest = CInstallPackageComposerTest;

TEST_F(CEnvironmentTest, Instantiation)
{
    CCmdlnStr cmdln;

    // Instantiate
    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdln));

    // There is nothing to do...
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, InvalidCommand)
{
    CCmdlnStr cmdln("blah");

    // Instantiate
    try
    {
        CSdvPackagerEnvironment environment(cmdln);
    }
    catch (const CSdvPackagerEnvironment::SEnvironmentException& rexcept)
    {
        EXPECT_EQ(rexcept.nCode, CMDLN_ARG_ERR);
    }
}

TEST_F(CEnvironmentTest, Help)
{
    CCmdlnStr cmdlnGeneral("--help");
    CCmdlnStr cmdlnPack("PACK --help");
    CCmdlnStr cmdlnInstall("INSTALL --help");
    CCmdlnStr cmdlnDirectInstall("DIRECT_INSTALL --help");
    CCmdlnStr cmdlnUninstall("UNINSTALL --help");
    CCmdlnStr cmdlnVerify("VERIFY --help");
    CCmdlnStr cmdlnShow("SHOW --help");
    CCmdlnStr cmdlnBlah("blah --help");

    // Instantiate
    try
    {
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnGeneral).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnPack).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnInstall).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnDirectInstall).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnUninstall).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnVerify).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnShow).Help());
        EXPECT_TRUE(CSdvPackagerEnvironment(cmdlnBlah).Help());
    }
    catch (const CSdvPackagerEnvironment::SEnvironmentException&)
    {
        EXPECT_TRUE(false);
    }
}

TEST_F(CEnvironmentTest, Command)
{
    CCmdlnStr cmdlnGeneral("--help");
    CCmdlnStr cmdlnPack("PACK --help");
    CCmdlnStr cmdlnPackLC("pack --help");
    CCmdlnStr cmdlnInstall("INSTALL --help");
    CCmdlnStr cmdlnInstallLC("install --help");
    CCmdlnStr cmdlnDirectInstall("DIRECT_INSTALL --help");
    CCmdlnStr cmdlnDirectInstallLC("direct_install --help");
    CCmdlnStr cmdlnUninstall("UNINSTALL --help");
    CCmdlnStr cmdlnUninstallLC("uninstall --help");
    CCmdlnStr cmdlnVerify("VERIFY --help");
    CCmdlnStr cmdlnVerifyLC("verify --help");
    CCmdlnStr cmdlnShow("SHOW --help");
    CCmdlnStr cmdlnShowLC("show --help");
    CCmdlnStr cmdlnBlah("blah --help");

    // Instantiate
    try
    {
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnGeneral).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::none);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnPack).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnPackLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnInstall).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnInstallLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnDirectInstall).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnDirectInstallLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnUninstall).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnUninstallLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnVerify).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnVerifyLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnShow).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnShowLC).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
        EXPECT_EQ(CSdvPackagerEnvironment(cmdlnBlah).OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::none);
    }
    catch (const CSdvPackagerEnvironment::SEnvironmentException&)
    {
        EXPECT_TRUE(false);
    }
}

TEST_F(CEnvironmentTest, ReportingMode)
{
    CCmdlnStr cmdlnNone;
    CCmdlnStr cmdlnSilent1("--silent");
    CCmdlnStr cmdlnSilent2("-s");
    CCmdlnStr cmdlnVerbose1("--verbose");
    CCmdlnStr cmdlnVerbose2("-v");
    CCmdlnStr cmdlnBoth1("--silent --verbose");
    CCmdlnStr cmdlnBoth2("--silent -v");
    CCmdlnStr cmdlnBoth3("-s --verbose");
    CCmdlnStr cmdlnBoth4("-s -v");

    // No reporting flag
    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNone));
    EXPECT_FALSE(environment.Silent());
    EXPECT_FALSE(environment.Verbose());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    // Silent flag
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnSilent1));
    EXPECT_TRUE(environment.Silent());
    EXPECT_FALSE(environment.Verbose());
    EXPECT_EQ(environment.Error(), NO_ERROR);
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnSilent2));
    EXPECT_TRUE(environment.Silent());
    EXPECT_FALSE(environment.Verbose());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    // Verbose flag
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnVerbose1));
    EXPECT_FALSE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), NO_ERROR);
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnVerbose1));
    EXPECT_FALSE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    // Silent and verbose flag
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnBoth1));
    EXPECT_TRUE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), CMDLN_SILENT_VERBOSE);
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnBoth2));
    EXPECT_TRUE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), CMDLN_SILENT_VERBOSE);
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnBoth3));
    EXPECT_TRUE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), CMDLN_SILENT_VERBOSE);
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnBoth4));
    EXPECT_TRUE(environment.Silent());
    EXPECT_TRUE(environment.Verbose());
    EXPECT_EQ(environment.Error(), CMDLN_SILENT_VERBOSE);
}

TEST_F(CEnvironmentTest, Version)
{
    CCmdlnStr cmdln("--version");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdln));
    EXPECT_TRUE(environment.Version());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_InstallName)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing("PACK");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.Error(), CMDLN_INSTALL_NAME_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_SourceFile)
{
    std::filesystem::path pathSrcFile1 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathSrcFile2 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file2.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"-O") + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile1.generic_u8string() + "\" \"" + pathSrcFile2.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    ASSERT_EQ(environment.ModuleList().size(), 2);
    EXPECT_EQ(environment.ModuleList()[0].ssSearchString, pathSrcFile1.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[1].ssSearchString, pathSrcFile2.generic_u8string());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_SourceFileWithTarget)
{
    std::filesystem::path pathSrcFile1 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathSrcFile2 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file2.bin";
    std::filesystem::path pathOutDir   = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"-O") + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile1.generic_u8string() + "=special_target\" \""
                             + pathSrcFile2.generic_u8string() + "=special_target/xyz\" \"-O" + pathOutDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    ASSERT_EQ(environment.ModuleList().size(), 2);
    EXPECT_EQ(environment.ModuleList()[0].ssSearchString, pathSrcFile1.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[0].pathRelTarget, std::filesystem::path("special_target"));
    EXPECT_EQ(environment.ModuleList()[1].ssSearchString, pathSrcFile2.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[1].pathRelTarget, std::filesystem::path("special_target/xyz"));
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_Directories)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnWrongSourceDir(std::string("PACK abc file1.bin \"-I") + (pathSrcDir / "blah").generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnNoSourceDir(std::string("PACK abc file1.bin \"-O") + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnNoTargetDir(std::string("PACK abc file1.bin \"-I") + pathSrcDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc file1.bin \"-I") + pathSrcDir.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWrongSourceDir));
    EXPECT_EQ(environment.SourceLocation(), pathSrcDir / "blah");
    EXPECT_EQ(environment.OutputLocation(), pathOutDir);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNoSourceDir));
    EXPECT_EQ(environment.SourceLocation(), std::filesystem::current_path());
    EXPECT_EQ(environment.OutputLocation(), pathOutDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNoTargetDir));
    EXPECT_EQ(environment.SourceLocation(), pathSrcDir);
    EXPECT_EQ(environment.OutputLocation(), std::filesystem::current_path());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.SourceLocation(), pathSrcDir);
    EXPECT_EQ(environment.OutputLocation(), pathOutDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_KeepStructure)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --keep_structure");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_FALSE(environment.KeepStructure());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_TRUE(environment.KeepStructure());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_CreateManifestOnly)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --create_manifest_only");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_FALSE(environment.CreateManifestOnly());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_TRUE(environment.CreateManifestOnly());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_SetInformation)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() +
        "\" --set_productdef \"--set_descriptionCoooolllll!!!\" \"--set_authorLisa Simpson\" \"--set_address742 Evergreen Terrace Springfield\" "
        "\"--set_copyright(C) 2025 This is mine\" --set_version1.2.3.4");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.ProductName(), "abc");
    EXPECT_TRUE(environment.Description().empty());
    EXPECT_TRUE(environment.Author().empty());
    EXPECT_TRUE(environment.Address().empty());
    EXPECT_TRUE(environment.Copyrights().empty());
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.ProductName(), "def");
    EXPECT_EQ(environment.Description(), "Coooolllll!!!");
    EXPECT_EQ(environment.Author(), "Lisa Simpson");
    EXPECT_EQ(environment.Address(), "742 Evergreen Terrace Springfield");
    EXPECT_EQ(environment.Copyrights(), "(C) 2025 This is mine");
    EXPECT_EQ(environment.PackageVersion(), "1.2.3.4");
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, PACK_ProductVersion)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnEmpty(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version");
    CCmdlnStr cmdlnMajorOnly(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version1");
    CCmdlnStr cmdlnMajorMinorOnly(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version1.2");
    CCmdlnStr cmdlnMinorOnly(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version.2");
    CCmdlnStr cmdlnPatchWithText(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version1.2.3a");
    CCmdlnStr cmdlnText(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_versiona.b.c");
    CCmdlnStr cmdlnNumberThenText(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version1a.2b.3c");
    CCmdlnStr cmdlnTextThenNumber(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_versiona1.b2.c3");
    CCmdlnStr cmdlnStandard(std::string("PACK abc \"") + pathSrcFile.generic_u8string() + "\" \"-O" + pathOutDir.generic_u8string() + "\" --set_version1.2.3.123");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnEmpty));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMajorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "1");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMajorMinorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "1.2");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMinorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), ".2");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPatchWithText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "1.2.3a");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "a.b.c");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNumberThenText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "1a.2b.3c");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTextThenNumber));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "a1.b2.c3");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnStandard));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::pack);
    EXPECT_EQ(environment.PackageVersion(), "1.2.3.123");
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_SourceFile)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathPackage2 = GetExecDirectory() / "install_package_composer_sources" / "file2.sdv_package";
    std::filesystem::path pathPackageWrong = GetExecDirectory() / "install_package_composer_sources" / "file3.sdvpackage";

    CCmdlnStr cmdlnMissing("INSTALL");
    CCmdlnStr cmdlnOnePackage(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnTwoPackages(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"" + pathPackage2.generic_u8string() + "\"");
    CCmdlnStr cmdlnWrongPackage(std::string("INSTALL \"") + pathPackageWrong.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOnePackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_EQ(environment.PackagePath(), pathPackage1.generic_u8string());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTwoPackages));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_EQ(environment.Error(), CMDLN_TOO_MANY_SOURCE_FILES);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWrongPackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_Local)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnLocalMissingTarget(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --local");
    CCmdlnStr cmdlnLocalShort(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" -L \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnLocalLong(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalMissingTarget));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), CMDLN_TARGET_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_Directories)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnLocalMissingTarget(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --local");
    CCmdlnStr cmdlnTargetMissingLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalMissingTarget));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.TargetLocation().empty());
    EXPECT_EQ(environment.Error(), CMDLN_TARGET_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTargetMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_InstanceID)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --instance1234");
    CCmdlnStr cmdlnWithLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\" --instance1234");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_EQ(environment.InstanceID(), 1000u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWithLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);
}

TEST_F(CEnvironmentTest, INSTALL_UpdateOverwrite)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnUpdateWithOverwrite(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" -W -P");
    CCmdlnStr cmdlnUpdateShort(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" -P");
    CCmdlnStr cmdlnUpdateLong(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --update");
    CCmdlnStr cmdlnOverwriteShort(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" -W");
    CCmdlnStr cmdlnOverwriteLong(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --overwrite");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateWithOverwrite));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), CMDLN_UPDATE_OVERWRITE_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOverwriteShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOverwriteLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_LocalConfig)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathPackage1 = pathSrcDir / "file1.sdv_package";
    std::filesystem::path pathConfig = pathSrcDir / "config.toml";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnConfigDirMissingLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigFileMissingLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"--config_file" +
        pathConfig.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigActivateConfigMissingLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" \"--config_file" + pathConfig.generic_u8string() + "\" --activate_config \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigActivateConfigMissingConfig(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --local --activate_config \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailableExtendedPlus(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "+abc+def\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailableExtendedComma(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "+abc,def\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    std::vector<std::string> vecComponents;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigDirMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_TRUE(environment.LocalConfigFile(vecComponents).empty());
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigFileMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), pathConfig);
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigActivateConfigMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), pathConfig);
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigActivateConfigMissingConfig));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_TRUE(environment.LocalConfigFile(vecComponents).empty());
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailableExtendedPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailableExtendedComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, INSTALL_ServerConfig)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathPackage1 = pathSrcDir / "file1.sdv_package";
    std::filesystem::path pathTgtDir   = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnUserConfig(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --user_config");
    CCmdlnStr cmdlnUserConfigComponentsComma(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --user_config+abc,def");
    CCmdlnStr cmdlnUserConfigComponentsPlus(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --user_config+abc+def");
    CCmdlnStr cmdlnPlatform(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --platform_config");
    CCmdlnStr cmdlnPlatformComponentsComma(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --platform_config+abc,def");
    CCmdlnStr cmdlnPlatformComponentsPlus(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --platform_config+abc+def");
    CCmdlnStr cmdlnInterface(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --interface_config");
    CCmdlnStr cmdlnInterfaceComponentsComma(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --interface_config+abc,def");
    CCmdlnStr cmdlnInterfaceComponentsPlus(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --interface_config+abc+def");
    CCmdlnStr cmdlnAbstract(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --abstract_config");
    CCmdlnStr cmdlnAbstractComponentsComma(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --abstract_config+abc,def");
    CCmdlnStr cmdlnAbstractComponentsPlus(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --abstract_config+abc+def");
    CCmdlnStr cmdlnAll(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --user_config --platform_config --interface_config --abstract_config");
    CCmdlnStr cmdlnUserLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() + "\" --user_config --local \"-T" +
        pathTgtDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnPlatformLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --platform_config --local \"-T" + pathTgtDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnInterfaceLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --interface_config --local \"-T" + pathTgtDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAbstractLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --abstract_config --local \"-T" + pathTgtDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAllLocal(std::string("INSTALL \"") + pathPackage1.generic_u8string() +
        "\" --user_config --platform_config --interface_config --abstract_config --local \"-T" + pathTgtDir.generic_u8string() +
        "\"");

    CSdvPackagerEnvironment environment;
    std::vector<std::string> vecComponents;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfig));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfigComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfigComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatform));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterface));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstract));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAll));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAllLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_InstallName)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";

    CCmdlnStr cmdlnMissing("DIRECT_INSTALL");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.Error(), CMDLN_INSTALL_NAME_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_KeepStructure)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --keep_structure");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.KeepStructure());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.KeepStructure());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_SetInformation)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --set_productdef "
        "\"--set_descriptionCoooolllll!!!\" \"--set_authorLisa Simpson\" \"--set_address742 Evergreen Terrace Springfield\" "
        "\"--set_copyright(C) 2025 This is mine\" --set_version1.2.3.4");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.ProductName(), "abc");
    EXPECT_TRUE(environment.Description().empty());
    EXPECT_TRUE(environment.Author().empty());
    EXPECT_TRUE(environment.Address().empty());
    EXPECT_TRUE(environment.Copyrights().empty());
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.ProductName(), "def");
    EXPECT_EQ(environment.Description(), "Coooolllll!!!");
    EXPECT_EQ(environment.Author(), "Lisa Simpson");
    EXPECT_EQ(environment.Address(), "742 Evergreen Terrace Springfield");
    EXPECT_EQ(environment.Copyrights(), "(C) 2025 This is mine");
    EXPECT_EQ(environment.PackageVersion(), "1.2.3.4");
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_ProductVersion)
{
    std::filesystem::path pathSrcFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" ");
    CCmdlnStr cmdlnEmpty(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version");
    CCmdlnStr cmdlnMajorOnly(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version1");
    CCmdlnStr cmdlnMajorMinorOnly(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version1.2");
    CCmdlnStr cmdlnMinorOnly(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version.2");
    CCmdlnStr cmdlnPatchWithText(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version1.2.3a");
    CCmdlnStr cmdlnText(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_versiona.b.c");
    CCmdlnStr cmdlnNumberThenText(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version1a.2b.3c");
    CCmdlnStr cmdlnTextThenNumber(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_versiona1.b2.c3");
    CCmdlnStr cmdlnStandard(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"  --set_version1.2.3.123");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnEmpty));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.PackageVersion().empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMajorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "1");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMajorMinorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "1.2");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMinorOnly));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), ".2");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPatchWithText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "1.2.3a");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "a.b.c");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNumberThenText));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "1a.2b.3c");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTextThenNumber));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "a1.b2.c3");
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnStandard));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.PackageVersion(), "1.2.3.123");
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_SourceFile)
{
    std::filesystem::path pathSrcFile1 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathSrcFile2 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file2.bin";
    std::filesystem::path pathOutDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing("DIRECT_INSTALL abc");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile1.generic_u8string() + "\" \"" + pathSrcFile2.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    ASSERT_EQ(environment.ModuleList().size(), 2);
    EXPECT_EQ(environment.ModuleList()[0].ssSearchString, pathSrcFile1.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[1].ssSearchString, pathSrcFile2.generic_u8string());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_SourceFileWithTarget)
{
    std::filesystem::path pathSrcFile1 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file1.bin";
    std::filesystem::path pathSrcFile2 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" / "file2.bin";
    std::filesystem::path pathOutDir   = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing("DIRECT_INSTALL abc");
    CCmdlnStr cmdlnAvailable(
        std::string("DIRECT_INSTALL abc \"") + pathSrcFile1.generic_u8string() + "=special_target\" \"" + pathSrcFile2.generic_u8string() + "=special_target/xyz\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    ASSERT_EQ(environment.ModuleList().size(), 2);
    EXPECT_EQ(environment.ModuleList()[0].ssSearchString, pathSrcFile1.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[0].pathRelTarget, std::filesystem::path("special_target"));
    EXPECT_EQ(environment.ModuleList()[1].ssSearchString, pathSrcFile2.generic_u8string());
    EXPECT_EQ(environment.ModuleList()[1].pathRelTarget, std::filesystem::path("special_target/xyz"));
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_Local)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnLocalShort(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" -L \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnLocalLong(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_Directories)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnWrongSourceDir(std::string("DIRECT_INSTALL abc file1.bin \"-I") + (pathSrcDir / "blah").generic_u8string() + "\"");
    CCmdlnStr cmdlnNoSourceDir("DIRECT_INSTALL abc file1.bin");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc file1.bin \"-I") + pathSrcDir.generic_u8string() + "\" ");
    CCmdlnStr cmdlnLocalMissingTarget(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --local");
    CCmdlnStr cmdlnTargetMissingLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnLocalAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWrongSourceDir));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.SourceLocation(), pathSrcDir / "blah");
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnNoSourceDir));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.SourceLocation(), std::filesystem::current_path());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.SourceLocation(), pathSrcDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalMissingTarget));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.TargetLocation().empty());
    EXPECT_EQ(environment.Error(), CMDLN_TARGET_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTargetMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_InstanceID)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --instance1234");
    CCmdlnStr cmdlnWithLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\" --instance1234");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_EQ(environment.InstanceID(), 1000u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWithLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_UpdateOverwrite)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");
    CCmdlnStr cmdlnUpdateWithOverwrite(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" -W -P");
    CCmdlnStr cmdlnUpdateShort(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" -P");
    CCmdlnStr cmdlnUpdateLong(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --update");
    CCmdlnStr cmdlnOverwriteShort(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" -W");
    CCmdlnStr cmdlnOverwriteLong(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --overwrite");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateWithOverwrite));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), CMDLN_UPDATE_OVERWRITE_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUpdateLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Update());
    EXPECT_FALSE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOverwriteShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOverwriteLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Update());
    EXPECT_TRUE(environment.Overwrite());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_LocalConfig)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcDir2 = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir2 / "file1.bin";
    std::filesystem::path pathConfig = pathSrcDir / "config.toml";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnConfigDirMissingLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigFileMissingLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"--config_file" +
        pathConfig.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigActivateConfigMissingLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() +
        "\" \"--config_file" + pathConfig.generic_u8string() + "\" --activate_config \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnConfigActivateConfigMissingConfig(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() +
        "\" --local --activate_config \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailableExtendedPlus(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "+abc+def\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailableExtendedComma(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" \"--config_dir" +
        pathSrcDir.generic_u8string() + "," + pathTargetDir.generic_u8string() + "\" \"--config_file" +
        pathConfig.filename().generic_u8string() + "+abc,def\" --activate_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    std::vector<std::string> vecComponents;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigDirMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_TRUE(environment.LocalConfigFile(vecComponents).empty());
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigFileMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), pathConfig);
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigActivateConfigMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), pathConfig);
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigActivateConfigMissingConfig));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.LocalConfigLocations().empty());
    EXPECT_TRUE(environment.LocalConfigFile(vecComponents).empty());
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailableExtendedPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailableExtendedComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.LocalConfigFile(vecComponents), "config.toml");
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_TRUE(environment.ActivateLocalConfig());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, DIRECT_INSTALL_ServerConfig)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFile = pathSrcDir / "file1.bin";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\"");
    CCmdlnStr cmdlnUserConfig(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config");
    CCmdlnStr cmdlnUserConfigComponentsComma(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config+abc,def");
    CCmdlnStr cmdlnUserConfigComponentsPlus(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config+abc+def");
    CCmdlnStr cmdlnPlatform(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --platform_config");
    CCmdlnStr cmdlnPlatformComponentsComma(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --platform_config+abc,def");
    CCmdlnStr cmdlnPlatformComponentsPlus(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --platform_config+abc+def");
    CCmdlnStr cmdlnInterface(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --interface_config");
    CCmdlnStr cmdlnInterfaceComponentsComma(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --interface_config+abc,def");
    CCmdlnStr cmdlnInterfaceComponentsPlus(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --interface_config+abc+def");
    CCmdlnStr cmdlnAbstract(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --abstract_config");
    CCmdlnStr cmdlnAbstractComponentsComma(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --abstract_config+abc,def");
    CCmdlnStr cmdlnAbstractComponentsPlus(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --abstract_config+abc+def");
    CCmdlnStr cmdlnAll(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config --platform_config --interface_config --abstract_config");
    CCmdlnStr cmdlnUserLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnPlatformLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --platform_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnInterfaceLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --interface_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAbstractLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --abstract_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAllLocal(std::string("DIRECT_INSTALL abc \"") + pathSrcFile.generic_u8string() + "\" --user_config --platform_config --interface_config --abstract_config --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    std::vector<std::string> vecComponents;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfig));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfigComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserConfigComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatform));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterface));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstract));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractComponentsComma));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractComponentsPlus));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_EQ(vecComponents.size(), 2);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAll));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_FALSE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnUserLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnPlatformLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInterfaceLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAbstractLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_FALSE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_FALSE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAllLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::direct_install);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.InsertIntoUserConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoPlatformConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleInterfaceConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_TRUE(environment.InsertIntoVehicleAbstractionConfig(vecComponents));
    EXPECT_TRUE(vecComponents.empty());
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);
}

TEST_F(CEnvironmentTest, UNINSTALL_InstallName)
{
    CCmdlnStr cmdlnMissing("UNINSTALL");
    CCmdlnStr cmdlnAvailable("UNINSTALL abc");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_EQ(environment.Error(), CMDLN_INSTALL_NAME_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, UNINSTALL_Local)
{
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnLocalShort(std::string("UNINSTALL abc -L \"-T") + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnLocalLong(std::string("UNINSTALL abc --local \"-T") + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalShort));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalLong));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, UNINSTALL_InstanceID)
{
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnMissing("UNINSTALL abc");
    CCmdlnStr cmdlnAvailable("UNINSTALL abc --instance1234");
    CCmdlnStr cmdlnWithLocal(std::string("UNINSTALL abc --local \"-T") + pathTargetDir.generic_u8string() + "\" --instance1234");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_EQ(environment.InstanceID(), 1000u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWithLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.InstanceID(), 1234u);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);
}

TEST_F(CEnvironmentTest, UNINSTALL_Directories)
{
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnLocalMissingTarget("UNINSTALL abc --local");
    CCmdlnStr cmdlnTargetMissingLocal(std::string("UNINSTALL abc \"-T") + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("UNINSTALL abc --local \"-T") + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnLocalMissingTarget));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_TRUE(environment.TargetLocation().empty());
    EXPECT_EQ(environment.Error(), CMDLN_TARGET_LOCATION_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTargetMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.TargetLocation(), pathTargetDir);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, UNINSTALL_LocalConfig)
{
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathPackage1 = pathSrcDir / "file1.sdv_package";
    std::filesystem::path pathConfig = pathSrcDir / "config.toml";
    std::filesystem::path pathTargetDir = GetExecDirectory() / "install_package_composer_targets";

    CCmdlnStr cmdlnConfigDirMissingLocal(std::string("UNINSTALL abc \"--config_dir") + pathSrcDir.generic_u8string() + "," +
        pathTargetDir.generic_u8string() + "\" \"-T" + pathTargetDir.generic_u8string() + "\"");
    CCmdlnStr cmdlnAvailable(std::string("UNINSTALL abc \"--config_dir") + pathSrcDir.generic_u8string() + "," +
        pathTargetDir.generic_u8string() + "\" --local \"-T" + pathTargetDir.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnConfigDirMissingLocal));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_FALSE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.Error(), CMDLN_INCOMPATIBLE_ARGUMENTS);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAvailable));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::uninstall);
    EXPECT_TRUE(environment.Local());
    EXPECT_EQ(environment.LocalConfigLocations().size(), 2);
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, VERIFY_SourceFile)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathPackage2 = GetExecDirectory() / "install_package_composer_sources" / "file2.sdv_package";
    std::filesystem::path pathPackageWrong = GetExecDirectory() / "install_package_composer_sources" / "file3.sdvpackage";

    CCmdlnStr cmdlnMissing("VERIFY");
    CCmdlnStr cmdlnOnePackage(std::string("VERIFY \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnTwoPackages(std::string("VERIFY \"") + pathPackage1.generic_u8string() + "\" \"" + pathPackage2.generic_u8string() + "\"");
    CCmdlnStr cmdlnWrongPackage(std::string("VERIFY \"") + pathPackageWrong.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOnePackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
    ASSERT_EQ(environment.PackagePath(), pathPackage1.generic_u8string());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTwoPackages));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
    EXPECT_EQ(environment.Error(), CMDLN_TOO_MANY_SOURCE_FILES);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWrongPackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::verify);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_ERROR);
}

TEST_F(CEnvironmentTest, SHOW_Command)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathPackage2 = GetExecDirectory() / "install_package_composer_sources" / "file2.sdv_package";
    std::filesystem::path pathPackageWrong = GetExecDirectory() / "install_package_composer_sources" / "file3.sdvpackage";

    CCmdlnStr cmdlnMissing(std::string("SHOW \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnInvalid(std::string("SHOW NOTHING \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnAll(std::string("SHOW ALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnAllLC(std::string("SHOW all \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnInfo(std::string("SHOW INFO \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnInfoLC(std::string("SHOW info \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnModules(std::string("SHOW MODULES \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnModulesLC(std::string("SHOW modules \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnComponents(std::string("SHOW COMPONENTS \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnComponentsLC(std::string("SHOW components \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnCombined(std::string("SHOW ALL INFO MODULES COMPONENTS \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnCombinedLC(std::string("SHOW all info modules components \"") + pathPackage1.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_EQ(environment.ShowFlags(), 0u);
    EXPECT_EQ(environment.Error(), CMDLN_MISSING_SHOW_COMMAND);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInvalid));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_EQ(environment.ShowFlags(), 0u);
    EXPECT_EQ(environment.Error(), CMDLN_MISSING_SHOW_COMMAND);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAll));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnAllLC));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInfo));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnInfoLC));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnModules));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnModulesLC));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnComponents));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnComponentsLC));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnCombined));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnCombinedLC));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::all));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::info));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::modules));
    EXPECT_TRUE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::components));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::xml));
    EXPECT_FALSE(environment.CheckShowFlag(CSdvPackagerEnvironment::EShowMask::json));
    EXPECT_EQ(environment.Error(), NO_ERROR);
}

TEST_F(CEnvironmentTest, SHOW_SourceFile)
{
    std::filesystem::path pathPackage1 = GetExecDirectory() / "install_package_composer_sources" / "file1.sdv_package";
    std::filesystem::path pathPackage2 = GetExecDirectory() / "install_package_composer_sources" / "file2.sdv_package";
    std::filesystem::path pathPackageWrong = GetExecDirectory() / "install_package_composer_sources" / "file3.sdvpackage";

    CCmdlnStr cmdlnMissing("SHOW ALL");
    CCmdlnStr cmdlnOnePackage(std::string("SHOW ALL \"") + pathPackage1.generic_u8string() + "\"");
    CCmdlnStr cmdlnTwoPackages(std::string("SHOW ALL \"") + pathPackage1.generic_u8string() + "\" \"" + pathPackage2.generic_u8string() + "\"");
    CCmdlnStr cmdlnWrongPackage(std::string("SHOW ALL \"") + pathPackageWrong.generic_u8string() + "\"");

    CSdvPackagerEnvironment environment;
    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnMissing));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_MISSING);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnOnePackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    ASSERT_EQ(environment.PackagePath(), pathPackage1.generic_u8string());
    EXPECT_EQ(environment.Error(), NO_ERROR);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnTwoPackages));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_EQ(environment.Error(), CMDLN_TOO_MANY_SOURCE_FILES);

    EXPECT_NO_THROW(environment = CSdvPackagerEnvironment(cmdlnWrongPackage));
    EXPECT_EQ(environment.OperatingMode(), CSdvPackagerEnvironment::EOperatingMode::show);
    EXPECT_EQ(environment.Error(), CMDLN_SOURCE_FILE_ERROR);
}
