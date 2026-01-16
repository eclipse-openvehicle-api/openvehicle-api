#include "composer_test_suite.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <string>
#include "../../../global/process_watchdog.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/installation_manifest.cpp"
#include "../../../sdv_services/core/installation_composer.cpp"
#include "../../../sdv_services/core/toml_parser/parser_toml.cpp"
#include "../../../sdv_services/core/toml_parser/lexer_toml.cpp"
#include "../../../sdv_services/core/toml_parser/lexer_toml_token.cpp"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.cpp"
#include "../../../sdv_services/core/toml_parser/character_reader_utf_8.cpp"
#include "../../../sdv_services/core/toml_parser/miscellaneous.cpp"
#include <support/app_control.h>

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    CLocalMemMgr memmgr;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void CInstallPackageComposerTest::SetUp()
{}

void CInstallPackageComposerTest::TearDown()
{
    DeleteTargetTestDir();
}

void CInstallPackageComposerTest::SetUpTestCase()
{
    DeleteTestDirs();

    // Set the application installation directory
    sdv::app::CAppControl::SetComponentInstallDirectory(GetExecDirectory() / "install_package_composer_targets");

    // Make rand random
    std::srand(static_cast<unsigned int>(std::time({})));

    // Create test data
    try
    {
        // Create a dummy file with at the given path.
        auto fnCreateDummyFile = [](const std::filesystem::path& rpathFile, uint32_t uiMinSize = 0)
        {
            std::ofstream stream(rpathFile.native().c_str());
            if (!stream.is_open())
            {
                std::cerr << "Could not open file '" << rpathFile.generic_u8string() << "'" << std::endl;
                ASSERT_TRUE(false);
            }
            uint32_t uiTotalSize = 0;
            do
            {
                uint32_t uiSize = std::rand() & 0x1ffff;
                for (uint32_t uiVal = 0; uiVal < uiSize; uiVal++)
                    stream << uiVal;
                uiTotalSize += uiSize * 4;
            }
            while (uiTotalSize < uiMinSize);
            stream.close();
        };

        // The source directory
        std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources";

        // Create dummy packages (one deliberately with a wrong extension)
        std::filesystem::create_directories(pathSrcDir);
        fnCreateDummyFile(pathSrcDir / "file1.sdv_package");
        fnCreateDummyFile(pathSrcDir / "file2.sdv_package");
        fnCreateDummyFile(pathSrcDir / "file3.sdvpackage");
        fnCreateDummyFile(pathSrcDir / "config.toml");

        // Create 10 dummy packages
        std::filesystem::path pathSrcDummy = pathSrcDir / "dummy_package";
        std::filesystem::create_directories(pathSrcDummy);
        for (size_t n = 0; n < 10; n++)
            fnCreateDummyFile(pathSrcDummy / (std::string("file") + std::to_string(n) + ".bin"));

        // Create the following dummy packages:
        //   subdir1/file10.bin
        //   subdir1/file11.bin
        //   subdir2/file20.bin
        //   subdir2/file21.bin
        //   subdir2/file22.bia
        //   subdir3_xyz/file30.bin
        //   subdir3_xyz/file31.bin
        //   subdir3_xyz/file32a.bia
        //   subdir3_xyz/subdir4/file40a.bin
        //   subdir3_xyz/subdir4/subdir5/file50a.bin
        std::filesystem::path pathSrcDummySubDir1 = pathSrcDummy / "subdir1";
        std::filesystem::create_directories(pathSrcDummySubDir1);
        fnCreateDummyFile(pathSrcDummySubDir1 / "file10.bin");
        fnCreateDummyFile(pathSrcDummySubDir1 / "file11.bin");
        std::filesystem::path pathSrcDummySubDir2 = pathSrcDummy / "subdir2";
        std::filesystem::create_directories(pathSrcDummySubDir2);
        fnCreateDummyFile(pathSrcDummySubDir2 / "file20.bin");
        fnCreateDummyFile(pathSrcDummySubDir2 / "file21.bin");
        fnCreateDummyFile(pathSrcDummySubDir2 / "file22.bia");
        std::filesystem::path pathSrcDummySubDir3 = pathSrcDummy / "subdir3_xyz";
        std::filesystem::create_directories(pathSrcDummySubDir3);
        fnCreateDummyFile(pathSrcDummySubDir3 / "file30.bin");
        fnCreateDummyFile(pathSrcDummySubDir3 / "file31.bin");
        fnCreateDummyFile(pathSrcDummySubDir3 / "file32a.bia");
        std::filesystem::path pathSrcDummySubDir4 = pathSrcDummySubDir3 / "subdir4";
        std::filesystem::create_directories(pathSrcDummySubDir4);
        fnCreateDummyFile(pathSrcDummySubDir4 / "file40a.bin");
        std::filesystem::path pathSrcDummySubDir5 = pathSrcDummySubDir4 / "subdir5";
        std::filesystem::create_directories(pathSrcDummySubDir5);
        fnCreateDummyFile(pathSrcDummySubDir5 / "file50a.bin");

        // Files for filename duplication test
        std::filesystem::path pathSrcDummySubDir1b = pathSrcDummy / "subdir1b";
        std::filesystem::create_directories(pathSrcDummySubDir1b);
        fnCreateDummyFile(pathSrcDummySubDir1b / "file10.bin");
        fnCreateDummyFile(pathSrcDummySubDir1b / "file11.bin");

        // XXL-Files
        std::filesystem::path pathSrcDummyXXL = pathSrcDummy / "xxl";
        std::filesystem::create_directories(pathSrcDummyXXL);
        fnCreateDummyFile(pathSrcDummyXXL / "fileXXLa.bin", 20*1024*1024);
        fnCreateDummyFile(pathSrcDummyXXL / "fileXXLb.bin", 2*10124*1024);

        // Files for file attribute and change time test
        std::filesystem::path pathSrcDummyTimeAttr = pathSrcDummy / "time_attr";
        std::filesystem::create_directories(pathSrcDummyTimeAttr);

        // Create time (for Windows only)
#ifdef _WIN32
        // Use the first presentation of the Comodore 64 (7th of January 1982)
        std::tm sCreateTimeInfo{};
        sCreateTimeInfo.tm_year = 82;
        sCreateTimeInfo.tm_mon = 0;
        sCreateTimeInfo.tm_mday = 7;
        sCreateTimeInfo.tm_hour = 11;
        sCreateTimeInfo.tm_min = 0;
        sCreateTimeInfo.tm_sec = 0;
        std::time_t tmCreateTime = _mkgmtime(&sCreateTimeInfo);
        fnCreateDummyFile(pathSrcDummyTimeAttr / "create_time.bin");
        SetCreateTime(pathSrcDummyTimeAttr / "create_time.bin", static_cast<uint64_t>(tmCreateTime) * 1000000ull);
#endif

        // Change time
        // Use the first presentation of the personal computer (12th of August 1981)
        std::tm sChangeTimeInfo{};
        sChangeTimeInfo.tm_year = 81;
        sChangeTimeInfo.tm_mon = 7;
        sChangeTimeInfo.tm_mday = 12;
        sChangeTimeInfo.tm_hour = 9;
        sChangeTimeInfo.tm_min = 0;
        sChangeTimeInfo.tm_sec = 0;
#ifdef _WIN32
        std::time_t tmChangeTime = _mkgmtime(&sChangeTimeInfo);
#else
        std::time_t tmChangeTime = timegm(&sChangeTimeInfo);
#endif
        fnCreateDummyFile(pathSrcDummyTimeAttr / "change_time.bin");
        SetChangeTime(pathSrcDummyTimeAttr / "change_time.bin", static_cast<uint64_t>(tmChangeTime) * 1000000ull);

        // Read only (for Windows and Linux only)
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
        fnCreateDummyFile(pathSrcDummyTimeAttr / "readonly.bin");
        SetReadOnly(pathSrcDummyTimeAttr / "readonly.bin");
#endif
#ifdef __unix__
        fnCreateDummyFile(pathSrcDummyTimeAttr / "executable.bin");
        SetExecutable(pathSrcDummyTimeAttr / "executable.bin");
#endif

        // Change times and attributes
        fnCreateDummyFile(pathSrcDummyTimeAttr / "all.bin");
#ifdef _WIN32
        SetCreateTime(pathSrcDummyTimeAttr / "all.bin", static_cast<uint64_t>(tmCreateTime) * 1000000ull);
#endif
        SetChangeTime(pathSrcDummyTimeAttr / "all.bin", static_cast<uint64_t>(tmChangeTime) * 1000000ull);
#ifdef __unix__
        SetExecutable(pathSrcDummyTimeAttr / "all.bin");
#endif
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
        SetReadOnly(pathSrcDummyTimeAttr / "all.bin");
#endif
    }
    catch (const std::filesystem::filesystem_error& rexception)
    {
        // Exception occurred.
        std::cerr << "Filesystem exception occurred (create test data): " << rexception.what() << std::endl;
        ASSERT_TRUE(false);
    }

}

void CInstallPackageComposerTest::TearDownTestSuite()
{
    DeleteTestDirs();
}

void CInstallPackageComposerTest::DeleteTargetTestDir()
{
    try
    {
        std::filesystem::path pathTgtDir = GetExecDirectory() / "install_package_composer_targets";
        if (!std::filesystem::exists(pathTgtDir)) return;
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
        auto vecFiles = CollectWildcardPath(pathTgtDir, "**/readonly.bin");
        for (const auto& rpathFile : vecFiles)
            ResetReadOnly(pathTgtDir / rpathFile);
        vecFiles = CollectWildcardPath(pathTgtDir, "**/all.bin");
        for (const auto& rpathFile : vecFiles)
            ResetReadOnly(pathTgtDir / rpathFile);
#endif
        std::filesystem::remove_all(pathTgtDir);
    }
    catch (const std::filesystem::filesystem_error& rexception)
    {
        // Exception occurred.
        std::cerr << "Filesystem exception occurred (delete target test directories): " << rexception.what() << std::endl;
        ASSERT_TRUE(false);
    }

}

void CInstallPackageComposerTest::DeleteTestDirs()
{
    try
    {
        std::filesystem::path pathSrcDir = GetExecDirectory() / "install_package_composer_sources/dummy_package";
        if (!std::filesystem::exists(pathSrcDir)) return;
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
        if (std::filesystem::exists(pathSrcDir / "time_attr/readonly.bin"))
            ResetReadOnly(pathSrcDir / "time_attr/readonly.bin");
        if (std::filesystem::exists(pathSrcDir / "time_attr/all.bin"))
            ResetReadOnly(pathSrcDir / "time_attr/all.bin");
#endif

        std::filesystem::remove_all(pathSrcDir);
    }
    catch (const std::filesystem::filesystem_error& rexception)
    {
        // Exception occurred.
        std::cerr << "Filesystem exception occurred (delete test directories): " << rexception.what() << std::endl;
        ASSERT_TRUE(false);
    }
    DeleteTargetTestDir();
}

bool CInstallPackageComposerTest::AreFilesEqual(const std::filesystem::path& rpath1, const std::filesystem::path& rpath2,
    bool bCheckTimes /*= false*/, bool bCheckAttributes /*= false*/)
{
    std::ifstream stream1(rpath1.native().c_str());
    if (!stream1.is_open()) return false;
    std::ifstream stream2(rpath2.native().c_str());
    if (!stream2.is_open()) return false;

    sdv::pointer<uint8_t> ptrFile1 = sdv::from_stream(stream1.rdbuf());
    sdv::pointer<uint8_t> ptrFile2 = sdv::from_stream(stream2.rdbuf());
    if ((!ptrFile1 && ptrFile2) || (ptrFile1 && !ptrFile2)) return false;
    if (ptrFile1.size() != ptrFile2.size()) return false;
    if (!std::equal(ptrFile1.get(), ptrFile1.get() + ptrFile1.size(), ptrFile2.get())) return false;

    // Check attributes
    if (bCheckAttributes)
    {
#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
        if (IsReadOnly(rpath1) != IsReadOnly(rpath2)) return false;
#endif
#ifdef __unix__
        if (IsExecutable(rpath1) != IsExecutable(rpath2)) return false;
#endif
    }

    // File time and attributes
    if (bCheckTimes)
    {
#ifdef _WIN32
        if (GetCreateTime(rpath1) != GetCreateTime(rpath2)) return false;
#endif
        if (GetChangeTime(rpath1) != GetChangeTime(rpath2)) return false;
    }

    // Content, times and attributes are equal.
    return true;
 }

#ifdef _WIN32
void CInstallPackageComposerTest::SetReadOnly(const std::filesystem::path& rpathFile)
{
    // Get the file attributes
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set the file attributes
    SetFileAttributesW(rpathFile.native().c_str(), sFileAttr.dwFileAttributes | FILE_ATTRIBUTE_READONLY);
}

void CInstallPackageComposerTest::ResetReadOnly(const std::filesystem::path& rpathFile)
{
    // Get the file attributes
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set the file attributes
    SetFileAttributesW(rpathFile.native().c_str(), sFileAttr.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
}

bool CInstallPackageComposerTest::IsReadOnly(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return false;
    return sFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? true : false;
}

#elif defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX
void CInstallPackageComposerTest::SetReadOnly(const std::filesystem::path& rpathFile)
{
    // Enable the immutable capability
    cap_t caps = cap_get_proc();
    {
        perror("cap_get_proc error");
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    cap_value_t capList[1] = {CAP_LINUX_IMMUTABLE};
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capList, CAP_SET) == -1)
    {
        perror("cap_set_flag error");
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
            }

    // Set the immutable flag
    int fd = open(pathFile.native().c_str(), O_RDONLY);
    if (fd == -1)
    {
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    int flags = 0;
    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1)
    {
        perror("ioctl error");
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }

    flags |= FS_IMMUTABLE_FL;
    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1)
    {
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    close(fd);
    cap_free(caps);
}

void CInstallPackageComposerTest::ResetReadOnly(const std::filesystem::path& rpathFile)
{
    // Enable the immutable capability
    cap_t caps = cap_get_proc();
    {
        perror("cap_get_proc error");
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    cap_value_t capList[1] = {CAP_LINUX_IMMUTABLE};
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, capList, CAP_SET) == -1)
    {
        perror("cap_set_flag error");
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }

    // Set the immutable flag
    int fd = open(pathFile.native().c_str(), O_RDONLY);
    if (fd == -1)
    {
        cap_free(caps);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    int flags = 0;
    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1)
    {
        perror("ioctl error");
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }

    flags &= ~FS_IMMUTABLE_FL;
    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1)
    {
        cap_free(caps);
        close(fd);
        sdv::XCannotChangeFileTimesOrAttributes exception;
        exception.ssPath = pathFile.generic_u8string();
        throw exception;
    }
    close(fd);
    cap_free(caps);
}

bool CInstallPackageComposerTest::IsReadOnly(const std::filesystem::path& rpathFile)
{
    struct statx sStat{};
    sStat.stx_mask = STATX_BTIME | STATX_MTIME;
    if (statx(0, rsModuleEntry.pathSrcModule.native().c_str(), 0, STATX_BTIME | STATX_MTIME, &sStat) ||
        sStat.stx_btime.tv_sec > 2524608000 || sStat.stx_btime.tv_nsec > 999999999 || sStat.stx_mtime.tv_sec > 2524608000 ||
        sStat.stx_mtime.tv_nsec > 999999999)
        return false;
    return sStat.stx_attributes_mask & STATX_ATTR_IMMUTABLE && sStat.stx_attributes & STATX_ATTR_IMMUTABLE;
}
#endif

#ifdef __unix__
void CInstallPackageComposerTest::SetExecutable(const std::filesystem::path& rpathFile)
{
    chmod(rpathFile.native().c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

bool CInstallPackageComposerTest::IsExecutable(const std::filesystem::path& rpathFile)
{
    struct stat sStat{};
    if (stat(rpathFile.native().c_str(), &sStat) == 0 && S_ISREG(sStat.st_mode))
    {
        // Check if any execute bit is set
        return (sStat.st_mode & S_IXUSR) || (sStat.st_mode & S_IXGRP) || (sStat.st_mode & S_IXOTH);
    }
    return false;
}
#endif

#ifdef _WIN32
void CInstallPackageComposerTest::SetCreateTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    // Get the file times
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set file times
    HANDLE hFile = CreateFile(rpathFile.native().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    FILETIME sCreationTime = PosixTimeToWindowsTime(uiTimeMicrosec);
    FILETIME sAccessTime = sFileAttr.ftLastAccessTime;
    FILETIME sWriteTime = sFileAttr.ftLastWriteTime;
    SetFileTime(hFile, &sCreationTime, &sAccessTime, &sWriteTime);
    CloseHandle(hFile);
}

uint64_t CInstallPackageComposerTest::GetCreateTime(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return 0ull;
    return WindowsTimeToPosixTime(sFileAttr.ftCreationTime);
}

void CInstallPackageComposerTest::SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    // Get the file times
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return;

    // Set file times
    HANDLE hFile = CreateFile(rpathFile.native().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    FILETIME sCreationTime = sFileAttr.ftCreationTime;
    FILETIME sAccessTime = sFileAttr.ftLastAccessTime;
    FILETIME sWriteTime = PosixTimeToWindowsTime(uiTimeMicrosec);
    SetFileTime(hFile, &sCreationTime, &sAccessTime, &sWriteTime);
    CloseHandle(hFile);
}

uint64_t CInstallPackageComposerTest::GetChangeTime(const std::filesystem::path& rpathFile)
{
    WIN32_FILE_ATTRIBUTE_DATA sFileAttr{};
    if (!GetFileAttributesExW(rpathFile.native().c_str(), GetFileExInfoStandard, &sFileAttr))
        return 0ull;
    return WindowsTimeToPosixTime(sFileAttr.ftLastWriteTime);
}

#else
void CInstallPackageComposerTest::SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec)
{
    struct timespec sTimes[2] = {};
    sTimes[0].tv_sec = static_cast<time_t>(uiTimeMicrosec / 1000000ull); // Set access to change time
    sTimes[0].tv_nsec = uiTimeMicrosec % 1000000ull * 1000ull;
    sTimes[1].tv_sec = static_cast<time_t>(uiTimeMicrosec / 1000000ull); // Set modification time to change time
    sTimes[1].tv_nsec = uiTimeMicrosec % 1000000ull * 1000ull;
    // Test for illegal time values... tv_sec should be below 1st of January 2050; tv_nsec should be below or equal to 999999999.
    if (sTimes[0].tv_sec >= 2524608000 || sTimes[0].tv_nsec > 999999999)
        return;

    // Set the times
    utimensat(AT_FDCWD, rpathFile.native().c_str(), sTimes, 0);
}

uint64_t CInstallPackageComposerTest::GetChangeTime(const std::filesystem::path& rpathFile)
{
    // Test for illegal time values... tv_sec should be below 1st of January 2050; tv_nsec should be below or equal to 999999999.
    struct stat sStat{};
    if (stat(rpathFile.u8string().c_str(), &sStat) || sStat.st_mtim.tv_sec >= 2524608000 ||
        sStat.st_mtim.tv_nsec > 999999999)
        return 0;
    return static_cast<uint64_t>(sStat.st_mtim.tv_sec) * 1000000ull + static_cast<uint64_t>(sStat.st_mtim.tv_nsec) / 1000ull;
}
#endif
