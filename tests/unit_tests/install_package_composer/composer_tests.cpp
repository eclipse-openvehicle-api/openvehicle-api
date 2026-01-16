#include "composer_test_suite.h"
#include "../../../sdv_services/core/installation_composer.h"
#include "../../../global/exec_dir_helper.h"
#include <fstream>

TEST_F(CInstallPackageComposerTest, Instantiation)
{
    CInstallComposer composer;

    // No files...
    EXPECT_FALSE(composer.Compose("Instantiation_Test"));
}

#ifdef _WIN32
TEST_F(CInstallPackageComposerTest, TimeConversionWindows)
{
    // Use the current time
    SYSTEMTIME sSysTime{};
    GetSystemTime(&sSysTime);
    FILETIME sFileTime{};
    EXPECT_TRUE(SystemTimeToFileTime(&sSysTime, &sFileTime));

    // Convert to POSIX time in micro-seconds
    uint64_t uiPosixTime = WindowsTimeToPosixTime(sFileTime);

    // Request the time struct from the posix time
    time_t tPosixTime = static_cast<time_t>(uiPosixTime / 1000000ull);
    struct tm* psTime = gmtime(&tPosixTime);
    ASSERT_NE(psTime, nullptr);

    // Compare the result of the conversion
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wYear), static_cast<uint32_t>(psTime->tm_year) + 1900);
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wMonth), static_cast<uint32_t>(psTime->tm_mon) + 1);
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wDay), static_cast<uint32_t>(psTime->tm_mday));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wHour), static_cast<uint32_t>(psTime->tm_hour));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wMinute), static_cast<uint32_t>(psTime->tm_min));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wSecond), static_cast<uint32_t>(psTime->tm_sec));

    // Request the POSIX time
    tPosixTime = time(nullptr);
    uiPosixTime = static_cast<uint64_t>(tPosixTime) * 1000000ull;

    // Convert to FILETIME
    sFileTime = PosixTimeToWindowsTime(uiPosixTime);

    // Request the time struct from the posix time
    psTime = gmtime(&tPosixTime);
    ASSERT_NE(psTime, nullptr);

    // Get the system time
    EXPECT_TRUE(FileTimeToSystemTime(&sFileTime, &sSysTime));

    // Compare the result of the conversion
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wYear), static_cast<uint32_t>(psTime->tm_year) + 1900);
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wMonth), static_cast<uint32_t>(psTime->tm_mon) + 1);
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wDay), static_cast<uint32_t>(psTime->tm_mday));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wHour), static_cast<uint32_t>(psTime->tm_hour));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wMinute), static_cast<uint32_t>(psTime->tm_min));
    EXPECT_EQ(static_cast<uint32_t>(sSysTime.wSecond), static_cast<uint32_t>(psTime->tm_sec));

    // Convert to-and-from for 10 files
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathFile = GetExecDirectory() / "install_package_composer_sources" / "dummy_package" /
            (std::string("file") + std::to_string(n) + ".bin");
        WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
        if (!GetFileAttributesExW(pathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        {
            std::cerr << "Cannot get file attributes..." << std::endl;
            EXPECT_TRUE(false);
            continue;
        }
        uint64_t uiCreationDate = WindowsTimeToPosixTime(sFileAttr.ftCreationTime);
        FILETIME sCreationTime = PosixTimeToWindowsTime(uiCreationDate);

        // Equality should be in micro-seconds resolution (time divided by 10).
        uint64_t uiOrgTime = static_cast<uint64_t>(sFileAttr.ftCreationTime.dwHighDateTime) << 32ull |
            static_cast<uint64_t>(sFileAttr.ftCreationTime.dwLowDateTime);
        uint64_t uiConvTime = static_cast<uint64_t>(sCreationTime.dwHighDateTime) << 32ull |
            static_cast<uint64_t>(sCreationTime.dwLowDateTime);
        EXPECT_EQ(uiOrgTime / 10ull, uiConvTime / 10ull);
    }
}
#endif

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedPackage");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedPackage");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackage.sdv_package", "ComposeFileBasedPackage"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackage.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedPackage");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackage", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackage");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageSubDirs)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageSubDirs";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file0.bin").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file1.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file10.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file11.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file20.bin", "subdir").size(), 1);

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedPackageSubDirs");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file0.bin",  pathTgtFileDir / "file0.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file1.bin",  pathTgtFileDir / "subdir" / "file1.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir" / "file20.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedPackageSubDirs");

    // The amount of modules should be 5
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);

    // The name of the module should fit to the expected file
    if (vecModules.size() >= 5)
    {
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file0.bin")), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file1.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file10.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file11.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file20.bin"), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageSubDirs)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageSubDirs";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file0.bin").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file1.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file10.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file11.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file20.bin", "subdir").size(), 1);

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageSubDirs.sdv_package", "ComposeFileBasedPackageSubDirs"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageSubDirs.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file0.bin",  pathTgtFileDir / "file0.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file1.bin",  pathTgtFileDir / "subdir" / "file1.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir" / "file20.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedPackageSubDirs");

    // The amount of modules should be 
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);

    // The name of the module should fit to the expected file
    if (vecModules.size() >= 5)
    {
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file0.bin")), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file1.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file10.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file11.bin"), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file20.bin"), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageSubDirs)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageSubDirs";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file0.bin").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file1.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file10.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file11.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file20.bin", "subdir").size(), 1);

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackageSubDirs", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file0.bin", pathTgtFileDir / "file0.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file1.bin", pathTgtFileDir / "subdir" / "file1.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir" / "file20.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackageSubDirs");

    // The amount of modules should be 5
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);

    // The name of the module should fit to the expected file
    if (vecModules.size() >= 5)
    {
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file0.bin")), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file1.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file10.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file11.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file20.bin"), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposePackageInvalidSubDirs)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file.bin").size(), 0);                           // No module available
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file0.bin").size(), 1); // OK
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file1.bin", "").size(), 1);                      // OK
#if _WIN32
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file2.bin", "/").size(), 1); // OK
#else
    EXPECT_THROW(composer.AddModule(pathSrcFileDir, "file2.bin", "/"), sdv::XInvalidPath);  // Relative target path is absolute
#endif
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file3.bin", ".").size(), 1);              // OK
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file4.bin", "./").size(), 1);             // OK
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "../dummy_package/file5.bin").size(), 0);  // Parent directory cannot be used
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file6.bin", "subdir/..").size(), 1);      // OK
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file7.bin", "subdir1/subdir2/../..").size(), 1);              // OK
    EXPECT_THROW(composer.AddModule(pathSrcFileDir, "file8.bin", ".."), sdv::XInvalidPath); // Points to parent
    EXPECT_THROW(composer.AddModule(pathSrcFileDir, "file9.bin", "subdir1/../../subdir"), sdv::XInvalidPath);   // Points to parent

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposePackageInvalidSubDirs");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageUsingWildcards)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageUsingWildcards";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1").size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n").size(), 2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*").size(), 3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*").size(), 2);  // Wildcards for directories, all files with file4... are included.

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedPackageUsingWildcards");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedPackageUsingWildcards");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageUsingWildcards)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageUsingWildcards";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1").size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n").size(), 2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*").size(), 3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*").size(), 2);  // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageUsingWildcards.sdv_package", "ComposeFileBasedPackageUsingWildcards"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageUsingWildcards.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedPackageUsingWildcards");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageUsingWildcards)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageUsingWildcards";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1").size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n").size(), 2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*").size(), 3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*").size(), 2);  // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackageUsingWildcards", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackageUsingWildcards");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposePackageMemoryBasedUsingWildcardsKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposePackageMemoryBasedUsingWildcardsKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards) | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n", "", uiFlags).size(), 2);  // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*", "", uiFlags).size(), 3);  // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*", "", uiFlags).size(), 2);  // Wildcards for directories, all files with file4... are included.

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposePackageMemoryBasedUsingWildcardsKeepStructure");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
	EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposePackageMemoryBasedUsingWildcardsKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposePackageFileBasedUsingWildcardsKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposePackageFileBasedUsingWildcardsKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards)
                       | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n", "", uiFlags).size(),
        2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*", "", uiFlags).size(),
        3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*", "", uiFlags).size(),
        2); // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposePackageFileBasedUsingWildcardsKeepStructure.sdv_package", "ComposePackageFileBasedUsingWildcardsKeepStructure"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposePackageFileBasedUsingWildcardsKeepStructure.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(
        pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposePackageFileBasedUsingWildcardsKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")),
        vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageUsingWildcardsKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageUsingWildcardsKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards)
                       | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2?.*n", "", uiFlags).size(),
        2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir?_*/*", "", uiFlags).size(),
        3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "**/file4*", "", uiFlags).size(),
        2); // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackageUsingWildcardsKeepStructure", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(
        pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackageUsingWildcardsKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")),
        vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageUsingRegex)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageUsingRegex";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(), 2);  // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(), 3);  // Wildcards for directory and file, all files from subdir3_xyz included.
                                                                                             // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(), 2);  // Wildcards for directories, all files with file4... are included.

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedPackageUsingRegex");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedPackageUsingRegex");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageUsingRegex)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageUsingRegex";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(),
        2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(),
        3); // Wildcards for directory and file, all files from subdir3_xyz included.
            // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(),
        2); // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageUsingRegex.sdv_package", "ComposeFileBasedPackageUsingRegex"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageUsingRegex.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedPackageUsingRegex");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageUsingRegex)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageUsingRegex";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(),
        2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(),
        3); // Wildcards for directory and file, all files from subdir3_xyz included.
            // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(),
        2); // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackageUsingRegex", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackageUsingRegex");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageUsingRegExKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageUsingRegExKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex) | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(), 2);  // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(), 3);  // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(), 2);  // Wildcards for directories, all files with file4... are included.

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedPackageUsingRegExKeepStructure");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedPackageUsingRegExKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageUsingRegExKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageUsingRegExKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex) | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(), 2);  // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(), 3);  // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(), 2);  // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageUsingRegExKeepStructure.sdv_package", "ComposeFileBasedPackageUsingRegExKeepStructure"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageUsingRegExKeepStructure.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedPackageUsingRegExKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")), vecModules.end());
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageUsingRegExKeepStructure)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageUsingRegExKeepStructure";

    // Add modules using wildcards
    // NOTE: The wildcard parsing is tested by the path_match unit test.
    uint32_t uiFlags = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex)
                       | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);
    // No wildcards, but all files in a directory
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/[^/]+", "", uiFlags).size(), 2);
    // File/directory wildcards '*' and '?'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file2\\w\\.[^/]+n", "", uiFlags).size(),
        2); // Wildcards for file, <file>.bin included, <file>.bia not.
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir\\w_[^/]+/[^/]+", "", uiFlags).size(),
        3); // Wildcards for directory and file, all files from subdir3_xyz included.
    // Directory wildcard '**'
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, ".*file4[^/]*", "", uiFlags).size(),
        2); // Wildcards for directories, all files with file4... are included.

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("ComposeDirectPackageUsingRegExKeepStructure", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir1" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir1" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir2" / "file20.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file21.bin", pathTgtFileDir / "subdir2" / "file21.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file30.bin", pathTgtFileDir / "subdir3_xyz" / "file30.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file31.bin", pathTgtFileDir / "subdir3_xyz" / "file31.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir3_xyz" / "file32a.bia", pathTgtFileDir / "subdir3_xyz" / "file32a.bia"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file4.bin", pathTgtFileDir / "file4.bin"));
    EXPECT_TRUE(AreFilesEqual(
        pathSrcFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin", pathTgtFileDir / "subdir3_xyz" / "subdir4" / "file40a.bin"));

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeDirectPackageUsingRegExKeepStructure");

    // The amount of modules should be 9
    auto vecModules = manifest.ModuleList();
    ASSERT_EQ(vecModules.size(), 9);

    // The name of the module should fit to the expected file
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file10.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir1/file11.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file20.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir2/file21.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file30.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file31.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/file32a.bia")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file4.bin")), vecModules.end());
    EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir3_xyz/subdir4/file40a.bin")),
        vecModules.end());
}

TEST_F(CInstallPackageComposerTest, AddModuleSourcePathTesting)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";

    // Add modules using wildcards
    uint32_t uiFlagsFlat = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards);
    uint32_t uiFlagsKeepStructure = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards) | static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::keep_structure);

    // Base relative (fail)
    EXPECT_THROW(composer.AddModule(pathSrcFileDir.lexically_relative(GetExecDirectory()), "subdir1", "", uiFlagsFlat), sdv::XInvalidPath);
    EXPECT_THROW(composer.AddModule(pathSrcFileDir.lexically_relative(GetExecDirectory()), "subdir1", "", uiFlagsKeepStructure), sdv::XInvalidPath);

    // Base empty, module relative (fail)
    EXPECT_THROW(composer.AddModule("", "subdir1", "", uiFlagsFlat), sdv::XInvalidPath);
    EXPECT_THROW(composer.AddModule("", "subdir1", "", uiFlagsKeepStructure), sdv::XInvalidPath);

    // Base empty, module absolute, keep structure (fail)
    EXPECT_THROW(composer.AddModule("", (pathSrcFileDir / "subdir1").generic_u8string(), "", uiFlagsKeepStructure), sdv::installation::XMissingBasePath);

    // Base empty, module absolute, not keeping structure (okay)
    EXPECT_EQ(composer.AddModule("", (pathSrcFileDir / "subdir1").generic_u8string(), "", uiFlagsFlat).size(), 2);

    // Base absolute, module relative, keeping structure (okay)
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2", "", uiFlagsKeepStructure).size(), 3);

    // Base absolute, module relative, not keeping structure (okay)
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir3_xyz", "", uiFlagsFlat).size(), 3);

    // Base absolute, module absolute, bot not from base (fail)
    EXPECT_THROW(composer.AddModule(pathTgtPckDir, (pathSrcFileDir / "subdir3_xyz/subdir4").generic_u8string(), "", uiFlagsFlat), sdv::XInvalidPath);

    // Base absolute, module absolute, keeping structure (okay)
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, (pathSrcFileDir / "subdir3_xyz/subdir4").generic_u8string(), "", uiFlagsKeepStructure).size(), 1);

    // Base absolute, module absolute, not keeping structure (okay)
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, (pathSrcFileDir / "subdir3_xyz/subdir4/subdir5").generic_u8string(), "", uiFlagsFlat).size(), 1);

    // Base absolute, module empty, keeping structure (okay)
    CInstallComposer composer2;
    EXPECT_EQ(composer2.AddModule(pathSrcFileDir / "subdir3_xyz", "", "", uiFlagsKeepStructure).size(), 5);

    // Base absolute, module empty, not keeping structure (okay)
    CInstallComposer composer3;
    EXPECT_EQ(composer3.AddModule(pathSrcFileDir / "subdir3_xyz", "", "", uiFlagsFlat).size(), 5);

    // Base absolute, module same as base (request all files from that one directory) (okay)
    CInstallComposer composer4;
    EXPECT_EQ(composer4.AddModule(pathSrcFileDir, pathSrcFileDir.generic_u8string(), "", uiFlagsKeepStructure).size(), 10);
    CInstallComposer composer5;
    EXPECT_EQ(composer5.AddModule(pathSrcFileDir, pathSrcFileDir.generic_u8string(), "", uiFlagsFlat).size(), 10);
}

#ifdef _WIN32
TEST_F(CInstallPackageComposerTest, AddModuleWindowsPathTesting)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathSrcFileDirW(pathSrcFileDir, std::filesystem::path::format::native_format);

    // Add modules using wildcards
    uint32_t uiFlagsWildcards = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::wildcards);
    uint32_t uiFlagsRegex = static_cast<uint32_t>(CInstallComposer::EAddModuleFlags::regex);

    // Base absolute native, module relative generic
    CInstallComposer composer1;
    EXPECT_EQ(composer1.AddModule(pathSrcFileDirW, "subdir1/*", "", uiFlagsWildcards).size(), 2);
    EXPECT_EQ(composer1.AddModule(pathSrcFileDirW, "subdir2/[^/]+", "", uiFlagsRegex).size(), 3);

    // Base absolute generic, module relative native (wildcards only)
    CInstallComposer composer2;
    EXPECT_EQ(composer2.AddModule(pathSrcFileDir, "subdir1\\*", "", uiFlagsWildcards).size(), 2);

    // Base absolute native, module relative native (wildcards only)
    CInstallComposer composer3;
    EXPECT_EQ(composer3.AddModule(pathSrcFileDirW, "subdir1\\*", "", uiFlagsWildcards).size(), 2);

    // Base absolute native, module absolute generic
    CInstallComposer composer4;
    EXPECT_EQ(composer4.AddModule(pathSrcFileDirW, (pathSrcFileDir / "subdir1/*").generic_u8string(), "", uiFlagsWildcards).size(), 2);
    EXPECT_EQ(composer4.AddModule(pathSrcFileDirW, (pathSrcFileDir / "subdir2/[^/]+").generic_u8string(), "", uiFlagsRegex).size(), 3);

    // Base absolute generic, module absolute native (wildcards only)
    CInstallComposer composer5;
    EXPECT_EQ(composer5.AddModule(pathSrcFileDir, pathSrcFileDirW.string() + "\\subdir1\\*", "", uiFlagsWildcards).size(), 2);

    // Base absolute native, module absolute native (wildcards only)
    CInstallComposer composer6;
    EXPECT_EQ(composer6.AddModule(pathSrcFileDirW, pathSrcFileDirW.string() + "\\subdir1\\*", "", uiFlagsWildcards).size(), 2);

}
#endif

TEST_F(CInstallPackageComposerTest, AddModulePathMultipleTimesAndOverlapping)
{
    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";

    // Multiple times same file same target dir (nothing added)
    CInstallComposer composer1;
    EXPECT_EQ(composer1.AddModule(pathSrcFileDir, "subdir1").size(), 2);
    EXPECT_THROW(composer1.AddModule(pathSrcFileDir, "subdir1"), sdv::XDuplicateFile);

    // Multiple times same file different target dir (okay)
    CInstallComposer composer2;
    EXPECT_EQ(composer2.AddModule(pathSrcFileDir, "subdir1", "A").size(), 2);
    EXPECT_EQ(composer2.AddModule(pathSrcFileDir, "subdir1", "B").size(), 2);

    // Different files with same name in same target dir (nothing added)
    CInstallComposer composer3;
    EXPECT_EQ(composer3.AddModule(pathSrcFileDir, "subdir1").size(), 2);
    EXPECT_THROW(composer3.AddModule(pathSrcFileDir, "subdir1b"), sdv::XDuplicateFile);

    // Different files with same name in different target dir (okay)
    CInstallComposer composer4;
    EXPECT_EQ(composer4.AddModule(pathSrcFileDir, "subdir1", "A").size(), 2);
    EXPECT_EQ(composer4.AddModule(pathSrcFileDir, "subdir1b", "B").size(), 2);
}

TEST_F(CInstallPackageComposerTest, TestSpecificFileAttributes)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "TestSpecificFileAttributes";

    // Add modules
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/readonly.bin").size(), 1);
#endif
#ifdef __unix__
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/executable.bin").size(), 1);
#endif
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/all.bin").size(), 1);

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("TestSpecificFileAttributes");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/readonly.bin", pathTgtFileDir / "readonly.bin", false, true));
#endif
#ifdef __unix__
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/executable.bin", pathTgtFileDir / "executable.bin", false, true));
#endif
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/all.bin", pathTgtFileDir / "all.bin", false, true));
}

TEST_F(CInstallPackageComposerTest, TestSpecificFileTimes)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "TestSpecificFileTimes";

    // Add modules
#ifdef _WIN32
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/create_time.bin").size(), 1);
#endif
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/change_time.bin").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "time_attr/all.bin").size(), 1);

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("TestSpecificFileTimes");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

#ifdef _WIN32
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/create_time.bin", pathTgtFileDir / "create_time.bin", true));
#endif
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/change_time.bin", pathTgtFileDir / "change_time.bin", true));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "time_attr/all.bin", pathTgtFileDir / "all.bin", true));
}

TEST_F(CInstallPackageComposerTest, TestXXLFiles)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "TestXXLFiles";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "xxl").size(), 2);

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("TestXXLFiles");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "xxl/fileXXLa.bin", pathTgtFileDir / "fileXXLa.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "xxl/fileXXLb.bin", pathTgtFileDir / "fileXXLb.bin"));
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedExtractFileBasedPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedExtractFileBasedPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedExtractFileBasedPackage");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Save the file to the disk
    std::ofstream fstream(pathSrcDir / "ComposeMemoryBasedExtractFileBasedPackage.sdv_package", std::ios::out | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    EXPECT_NO_THROW(sdv::to_stream(ptrPackage, fstream.rdbuf()));
    fstream.close();

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeMemoryBasedExtractFileBasedPackage.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedExtractFileBasedPackage");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedExtractMemoryBasedPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedExtractMemoryBasedPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedExtractMemoryBasedPackage.sdv_package", "ComposeFileBasedExtractMemoryBasedPackage"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Read the file from the disk
    std::ifstream fstream(pathSrcDir / "ComposeFileBasedExtractMemoryBasedPackage.sdv_package", std::ios::in | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    sdv::pointer<uint8_t> ptrPackage;
    EXPECT_NO_THROW((ptrPackage = sdv::from_stream(fstream.rdbuf())));
    fstream.close();

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedExtractMemoryBasedPackage");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedExtractManifest)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedExtractManifest";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedExtractManifest");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Save the file to the disk
    std::ofstream fstream(pathSrcDir / "ComposeMemoryBasedExtractManifest.sdv_package", std::ios::out | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    EXPECT_NO_THROW(sdv::to_stream(ptrPackage, fstream.rdbuf()));
    fstream.close();

    CInstallManifest manifest;
    try
    {
        // Start extractor
        CInstallComposer extractor;
        manifest = extractor.ExtractInstallManifest(pathSrcDir / "ComposeMemoryBasedExtractManifest.sdv_package");
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeMemoryBasedExtractManifest");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedExtractManifest)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedExtractManifest";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(
            pathSrcDir / "ComposeFileBasedExtractManifest.sdv_package", "ComposeFileBasedExtractManifest"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Read the file from the disk
    std::ifstream fstream(pathSrcDir / "ComposeFileBasedExtractManifest.sdv_package", std::ios::in | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    sdv::pointer<uint8_t> ptrPackage;
    EXPECT_NO_THROW((ptrPackage = sdv::from_stream(fstream.rdbuf())));
    fstream.close();

    CInstallManifest manifest;
    try
    {
        // Start extractor
        CInstallComposer extractor;
        manifest = extractor.ExtractInstallManifest(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeFileBasedExtractManifest");

    // The amount of modules should be 10
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }
}

TEST_F(CInstallPackageComposerTest, DetectPackageCorruptionHeader)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "DetectPackageCorruptionHeader";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("DetectPackageCorruptionHeader");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Store the header
    uint8_t rguiHeader[72];
    std::copy(ptrPackage.get(), ptrPackage.get() + 72, rguiHeader);

    // Corrupt the package at the header (first 72 bytes)
    for (size_t nPos = 0; nPos < 72; nPos++)
    {
        // Restore header
        std::copy(std::begin(rguiHeader), std::end(rguiHeader), ptrPackage.get());

        // Corrupt one byte
        ptrPackage.get()[nPos] = ~ptrPackage.get()[nPos];

        // Either extract fails or an exception occurs
        try
        {
            // Start extractor
            CInstallComposer extractor;
            CInstallManifest manifest = extractor.Extract(ptrPackage, pathTgtPckDir);
            if (!manifest.IsValid())
                std::cerr << "Extraction of a corruption at position #" << nPos << " didn't cause an error!" << std::endl;
            EXPECT_FALSE(manifest.IsValid());
        }
        catch (const sdv::XSysExcept& /*rexcept*/)
        {
            ASSERT_TRUE(true);
        }
    }
}

TEST_F(CInstallPackageComposerTest, DetectPackageCorruptionContent)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "DetectPackageCorruptionContent";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("DetectPackageCorruptionContent");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Test 25 times
    for (size_t n = 0; n < 25; n++)
    {
        // Corrupt the package at the content (from 100 until the size minus 32)
        size_t nPos = static_cast<size_t>(std::rand()) * (ptrPackage.size() - 132) / RAND_MAX + 100;
        uint8_t uiStoredByte = ptrPackage.get()[nPos];
        ptrPackage.get()[nPos] = ~uiStoredByte;

        // Delete the target directories
        DeleteTargetTestDir();

        // Either extract fails or an exception occurs
        try
        {
            // Start extractor
            CInstallComposer extractor;
            CInstallManifest manifest = extractor.Extract(ptrPackage, pathTgtPckDir);
            if (manifest.IsValid()) std::cerr << "Extraction of a corruption at position #" << nPos << " didn't cause an error!" << std::endl;
            EXPECT_FALSE(manifest.IsValid());
        }
        catch (const sdv::XSysExcept& /*rexcept*/)
        {
            ASSERT_TRUE(true);
        }

        // Restore the byte
        ptrPackage.get()[nPos] = uiStoredByte;
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedVerifyIntegrity)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedVerifyIntegrity";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeMemoryBasedVerifyIntegrity");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Save the file to the disk
    std::ofstream fstream(pathSrcDir / "ComposeMemoryBasedVerifyIntegrity.sdv_package", std::ios::out | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    EXPECT_NO_THROW(sdv::to_stream(ptrPackage, fstream.rdbuf()));
    fstream.close();

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Verify(pathSrcDir / "ComposeMemoryBasedVerifyIntegrity.sdv_package"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedVerifyIntegrity)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedVerifyIntegrity";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(
            pathSrcDir / "ComposeFileBasedVerifyIntegrity.sdv_package", "ComposeFileBasedVerifyIntegrity"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Read the file from the disk
    std::ifstream fstream(pathSrcDir / "ComposeFileBasedVerifyIntegrity.sdv_package", std::ios::in | std::ios::binary);
    ASSERT_TRUE(fstream.is_open());
    sdv::pointer<uint8_t> ptrPackage;
    EXPECT_NO_THROW((ptrPackage = sdv::from_stream(fstream.rdbuf())));
    fstream.close();

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Verify(ptrPackage));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, VerifyIntegrityCorruptionHeader)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "VerifyIntegrityCorruptionHeader";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("VerifyIntegrityCorruptionHeader");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Store the header
    uint8_t rguiHeader[72];
    std::copy(ptrPackage.get(), ptrPackage.get() + 72, rguiHeader);

    // Corrupt the package at the header (first 72 bytes)
    for (size_t nPos = 0; nPos < 72; nPos++)
    {
        // Restore header
        std::copy(std::begin(rguiHeader), std::end(rguiHeader), ptrPackage.get());

        // Corrupt one byte
        ptrPackage.get()[nPos] = ~ptrPackage.get()[nPos];

        // Either extract fails or an exception occurs
        try
        {
            // Start extractor
            CInstallComposer extractor;
            bool bRes = extractor.Verify(ptrPackage);
            if (bRes) std::cerr << "Extraction of a corruption at position #" << nPos << " didn't cause an error!" << std::endl;
            EXPECT_FALSE(bRes);
        }
        catch (const sdv::XSysExcept& /*rexcept*/)
        {
            ASSERT_TRUE(true);
        }
    }
}

TEST_F(CInstallPackageComposerTest, VerifyIntegrityCorruptionContent)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "VerifyIntegrityCorruptionContent";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        EXPECT_EQ(composer.AddModule(pathSrcFileDir, pathModule.generic_u8string()).size(), 1);
    }

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("VerifyIntegrityCorruptionContent");
        EXPECT_TRUE(ptrPackage);
        EXPECT_GT(ptrPackage.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Test 25 times
    for (size_t n = 0; n < 25; n++)
    {
        // Corrupt the package at the content (from 100 until the size minus 32)
        size_t nPos = static_cast<size_t>(std::rand()) * (ptrPackage.size() - 132) / RAND_MAX + 100;
        uint8_t uiStoredByte = ptrPackage.get()[nPos];
        ptrPackage.get()[nPos] = ~uiStoredByte;

        // Delete the target directories
        DeleteTargetTestDir();

        // Either extract fails or an exception occurs
        try
        {
            // Start extractor
            CInstallComposer extractor;
            bool bRes = extractor.Verify(ptrPackage);
            if (bRes) std::cerr << "Extraction of a corruption at position #" << nPos << " didn't cause an error!" << std::endl;
            EXPECT_FALSE(bRes);
        }
        catch (const sdv::XSysExcept& /*rexcept*/)
        {
            ASSERT_TRUE(true);
        }

        // Restore the byte
        ptrPackage.get()[nPos] = uiStoredByte;
    }
}

TEST_F(CInstallPackageComposerTest, ComposeSDVPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory();
    std::filesystem::path pathTgtPckDir = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeSDVPackage";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "UnitTest_InstallPackageComposer_Component1.sdv").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "UnitTest_InstallPackageComposer_Component2.sdv").size(), 1);

    sdv::pointer<uint8_t> ptrPackage;
    try
    {
        // Compose package as binary stream
        ptrPackage = composer.Compose("ComposeSDVPackage");
        EXPECT_TRUE(ptrPackage);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackage, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Read the manifest
    CInstallManifest manifest;
    EXPECT_TRUE(manifest.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifest.InstallName(), "ComposeSDVPackage");

    // The amount of modules should be 2
    auto vecModules = manifest.ModuleList();
    EXPECT_EQ(vecModules.size(), 2);
    auto vecComponents = manifest.ComponentList();
    EXPECT_EQ(vecComponents.size(), 3);

    // The name of the module should fit to the expected file
    std::string szssComponents[] = { "DummyService #1", "DummyDevice", "DummyService #2" };
    for (auto& rssComponent : szssComponents)
    {
        EXPECT_NE(std::find_if(vecComponents.begin(), vecComponents.end(), [&](const CInstallManifest::SComponent& rsComponent)
            {
                return rsComponent.ssClassName == rssComponent;
            }), vecComponents.end());
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageVersionNotAllowed)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageVersionNotAllowed";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    sdv::pointer<uint8_t> ptrPackageV0;
    try
    {
        // Compose package as binary stream
        ptrPackageV0 = composer.Compose("ComposeMemoryBasedPackageVersionNotAllowed");
        EXPECT_TRUE(ptrPackageV0);
        EXPECT_GT(ptrPackageV0.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version 1.0
    sdv::pointer<uint8_t> ptrPackageV1;
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        ptrPackageV1 = composer.Compose("ComposeMemoryBasedPackageVersionNotAllowed");
        EXPECT_TRUE(ptrPackageV1);
        EXPECT_GT(ptrPackageV1.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV0, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(ptrPackageV0, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(ptrPackageV1, pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageVersionNotAllowed)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageVersionNotAllowed";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionNotAllowedV0.sdv_package", "ComposeFileBasedPackageVersionNotAllowed"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version V1
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionNotAllowedV1.sdv_package", "ComposeFileBasedPackageVersionNotAllowed"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionNotAllowedV0.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionNotAllowedV0.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionNotAllowedV1.sdv_package", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageVersionNotAllowed)
{
    // Start composer
    CInstallComposer composerV0, composerV1;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageVersionNotAllowed";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composerV0.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
        vecFiles = composerV1.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    composerV1.AddProperty("Version", "1.0");

    // Install version V0
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV0.ComposeDirect("ComposeDirectPackageVersionNotAllowed", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_FALSE(composerV0.ComposeDirect("ComposeDirectPackageVersionNotAllowed", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (not allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_FALSE(composerV1.ComposeDirect("ComposeDirectPackageVersionNotAllowed", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageVersionUpdate)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageVersionUpdate";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    sdv::pointer<uint8_t> ptrPackageV0;
    try
    {
        // Compose package as binary stream
        ptrPackageV0 = composer.Compose("ComposeMemoryBasedPackageVersionUpdate");
        EXPECT_TRUE(ptrPackageV0);
        EXPECT_GT(ptrPackageV0.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version 1.0
    sdv::pointer<uint8_t> ptrPackageV1;
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        ptrPackageV1 = composer.Compose("ComposeMemoryBasedPackageVersionUpdate");
        EXPECT_TRUE(ptrPackageV1);
        EXPECT_GT(ptrPackageV1.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version 2.0
    sdv::pointer<uint8_t> ptrPackageV2;
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "2.0");
        ptrPackageV2 = composer.Compose("ComposeMemoryBasedPackageVersionUpdate");
        EXPECT_TRUE(ptrPackageV2);
        EXPECT_GT(ptrPackageV2.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV1, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install same version V1 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(ptrPackageV1, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV2, pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageVersionUpdate)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageVersionUpdate";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV0.sdv_package", "ComposeFileBasedPackageVersionUpdate"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version V1
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV1.sdv_package", "ComposeFileBasedPackageVersionUpdate"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version V2
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "2.0");
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV2.sdv_package", "ComposeFileBasedPackageVersionUpdate"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV1.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install same version V1 (not allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_FALSE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV1.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionUpdateV2.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageVersionUpdate)
{
    // Start composer
    CInstallComposer composerV0, composerV1, composerV2;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageVersionUpdate";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composerV0.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
        vecFiles = composerV1.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
        vecFiles = composerV2.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    composerV1.AddProperty("Version", "1.0");
    composerV2.AddProperty("Version", "2.0");

    // Install version V0
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV0.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (not allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_FALSE(composerV0.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV1.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (not allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_FALSE(composerV0.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install same version V1 (not allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_FALSE(composerV1.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept&)
    {
        // Exception was triggered correctly.
        ASSERT_TRUE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV2.ComposeDirect("ComposeDirectPackageVersionUpdate", pathTgtPckDir, CInstallComposer::EUpdateRules::update_when_new).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeMemoryBasedPackageVersionOverwrite)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcFileDir = GetExecDirectory() / "install_package_composer_sources" / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeMemoryBasedPackageVersionOverwrite";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    sdv::pointer<uint8_t> ptrPackageV0;
    try
    {
        // Compose package as binary stream
        ptrPackageV0 = composer.Compose("ComposeMemoryBasedPackageVersionOverwrite");
        EXPECT_TRUE(ptrPackageV0);
        EXPECT_GT(ptrPackageV0.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version 1.0
    sdv::pointer<uint8_t> ptrPackageV1;
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        ptrPackageV1 = composer.Compose("ComposeMemoryBasedPackageVersionOverwrite");
        EXPECT_TRUE(ptrPackageV1);
        EXPECT_GT(ptrPackageV1.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version 2.0
    sdv::pointer<uint8_t> ptrPackageV2;
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "2.0");
        ptrPackageV2 = composer.Compose("ComposeMemoryBasedPackageVersionOverwrite");
        EXPECT_TRUE(ptrPackageV2);
        EXPECT_GT(ptrPackageV2.size(), nTotalSize);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV1, pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV0, pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(ptrPackageV2, pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeFileBasedPackageVersionOverwrite)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeFileBasedPackageVersionOverwrite";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }

    // Create a package without version
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV0.sdv_package", "ComposeFileBasedPackageVersionOverwrite"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version V1
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "1.0");
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV1.sdv_package", "ComposeFileBasedPackageVersionOverwrite"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Create a package with version V2
    try
    {
        // Compose package as binary stream
        composer.AddProperty("Version", "2.0");
        EXPECT_TRUE(composer.Compose(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV2.sdv_package", "ComposeFileBasedPackageVersionOverwrite"));
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install version V0
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV1.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV0.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Start extractor
        CInstallComposer extractor;
        EXPECT_TRUE(extractor.Extract(pathSrcDir / "ComposeFileBasedPackageVersionOverwriteV2.sdv_package", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "EXTRACT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, ComposeDirectPackageVersionOverwrite)
{
    // Start composer
    CInstallComposer composerV0, composerV1, composerV2;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "ComposeDirectPackageVersionOverwrite";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composerV0.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
        vecFiles = composerV1.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
        vecFiles = composerV2.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    composerV1.AddProperty("Version", "1.0");
    composerV2.AddProperty("Version", "2.0");

    // Install version V0
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV0.ComposeDirect("ComposeDirectPackageVersionOverwrite", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install same version V0 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV0.ComposeDirect("ComposeDirectPackageVersionOverwrite", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V1 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV1.ComposeDirect("ComposeDirectPackageVersionOverwrite", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install old version V0 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV0.ComposeDirect("ComposeDirectPackageVersionOverwrite", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Install new version V2 (allowed)
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composerV2.ComposeDirect("ComposeDirectPackageVersionOverwrite", pathTgtPckDir, CInstallComposer::EUpdateRules::overwrite).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST_F(CInstallPackageComposerTest, UninstallPackage)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "UninstallPackage";

    // Add modules
    size_t nTotalSize = 0;
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathModule = std::string("file") + std::to_string(n) + ".bin";
        nTotalSize += std::filesystem::file_size(pathSrcFileDir / pathModule);
        auto vecFiles = composer.AddModule(pathSrcFileDir, pathModule.generic_u8string());
        ASSERT_EQ(vecFiles.size(), 1);
        EXPECT_EQ(vecFiles[0], pathModule);
    }
    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("UninstallPackage", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathSrcFile = pathSrcFileDir / (std::string("file") + std::to_string(n) + ".bin");
        std::filesystem::path pathTgtFile = pathTgtFileDir / (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_TRUE(AreFilesEqual(pathSrcFile, pathTgtFile));
    }

    // Read the manifest
    CInstallManifest manifestInstalled;
    EXPECT_TRUE(manifestInstalled.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifestInstalled.InstallName(), "UninstallPackage");

    // The amount of modules should be 10
    auto vecModules = manifestInstalled.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }

    // Check for the target directory to exist
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "UninstallPackage"));

    // Uninstall the package
    CInstallManifest manifestUninstalled;
    try
    {
        manifestUninstalled = composer.Remove("UninstallPackage", pathTgtPckDir);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "REMOVE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Installation package name should be identical
    EXPECT_EQ(manifestUninstalled.InstallName(), "UninstallPackage");

    // The amount of modules should be 10
    vecModules = manifestUninstalled.ModuleList();
    EXPECT_EQ(vecModules.size(), 10);

    // The name of the module should fit to the expected file
    for (size_t n = 0; n < 10; n++)
    {
        std::filesystem::path pathRelTgtFile = (std::string("file") + std::to_string(n) + ".bin");
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), pathRelTgtFile), vecModules.end());
    }

    // Check for the target directory to have been removed.
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "UninstallPackage"));
}

TEST_F(CInstallPackageComposerTest, UninstallPackageSubDirs)
{
    // Start composer
    CInstallComposer composer;

    // Source and target directories
    std::filesystem::path pathSrcDir     = GetExecDirectory() / "install_package_composer_sources";
    std::filesystem::path pathSrcFileDir = pathSrcDir / "dummy_package";
    std::filesystem::path pathTgtPckDir  = GetExecDirectory() / "install_package_composer_targets";
    std::filesystem::path pathTgtFileDir = pathTgtPckDir / "UninstallPackageSubDirs";

    // Add modules
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file0.bin").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "file1.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file10.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir1/file11.bin", "subdir").size(), 1);
    EXPECT_EQ(composer.AddModule(pathSrcFileDir, "subdir2/file20.bin", "subdir").size(), 1);

    try
    {
        // Compose package as binary stream
        EXPECT_TRUE(composer.ComposeDirect("UninstallPackageSubDirs", pathTgtPckDir).IsValid());
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "COMPOSE DIRECT EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Compare the generated files - names, content, attributes and file-times.
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file0.bin", pathTgtFileDir / "file0.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "file1.bin", pathTgtFileDir / "subdir" / "file1.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file10.bin", pathTgtFileDir / "subdir" / "file10.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir1" / "file11.bin", pathTgtFileDir / "subdir" / "file11.bin"));
    EXPECT_TRUE(AreFilesEqual(pathSrcFileDir / "subdir2" / "file20.bin", pathTgtFileDir / "subdir" / "file20.bin"));

    // Read the manifest
    CInstallManifest manifestInstalled;
    EXPECT_TRUE(manifestInstalled.Load(pathTgtFileDir));

    // Installation package name should be identical
    EXPECT_EQ(manifestInstalled.InstallName(), "UninstallPackageSubDirs");

    // The amount of modules should be 5
    auto vecModules = manifestInstalled.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);

    // The name of the module should fit to the expected file
    if (vecModules.size() >= 5)
    {
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file0.bin")), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file1.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file10.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file11.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file20.bin"), vecModules.end());
    }

    // Check for the target directory to exist
    EXPECT_TRUE(std::filesystem::exists(pathTgtPckDir / "UninstallPackageSubDirs"));

    // Uninstall the package
    CInstallManifest manifestUninstalled;
    try
    {
        manifestUninstalled = composer.Remove("UninstallPackageSubDirs", pathTgtPckDir);
    }
    catch (const sdv::XSysExcept& rexcept)
    {
        std::cerr << "REMOVE EXCEPTION: " << rexcept.what() << std::endl;
        ASSERT_FALSE(true);
    }

    // Installation package name should be identical
    EXPECT_EQ(manifestUninstalled.InstallName(), "UninstallPackageSubDirs");

    // The amount of modules should be 5
    vecModules = manifestUninstalled.ModuleList();
    EXPECT_EQ(vecModules.size(), 5);

    // The name of the module should fit to the expected file
    if (vecModules.size() >= 5)
    {
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("file0.bin")), vecModules.end());
        EXPECT_NE(std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file1.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file10.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file11.bin"), vecModules.end());
        EXPECT_NE(
            std::find(vecModules.begin(), vecModules.end(), std::filesystem::path("subdir") / "file20.bin"), vecModules.end());
    }

    // Check for the target directory to have been removed.
    EXPECT_FALSE(std::filesystem::exists(pathTgtPckDir / "UninstallPackageSubDirs"));
}
