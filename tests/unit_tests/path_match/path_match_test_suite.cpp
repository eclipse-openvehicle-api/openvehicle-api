#include "path_match_test_suite.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../global/process_watchdog.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <string>

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void CPathMatchTest::SetUp()
{
}

void CPathMatchTest::TearDown()
{
}

void CPathMatchTest::SetUpTestCase()
{
    // Clean up, just in case...
    TearDownTestSuite();

    // Make rand random
    std::srand(static_cast<unsigned int>(std::time({})));

    // Create test data
    try
    {
        // Create a dummy file with at the given path.
        auto fnCreateDummyFile = [](const std::filesystem::path& rpathFile)
        {
            std::ofstream stream(rpathFile.native().c_str());
            if (!stream.is_open())
            {
                std::cerr << "Could not open file '" << rpathFile.generic_u8string() << "'" << std::endl;
                ASSERT_TRUE(false);
            }
            uint32_t uiSize = std::rand() & 0x1ffff;
            for (uint32_t uiVal = 0; uiVal < uiSize; uiVal++)
                stream << uiVal;
            stream.close();
        };

        // The source directory
        std::filesystem::path pathSrc = GetExecDirectory() / "path_match_test";
        std::filesystem::create_directories(pathSrc);

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
        std::filesystem::path pathSrcSubDir1 = pathSrc / "subdir1";
        std::filesystem::create_directories(pathSrcSubDir1);
        fnCreateDummyFile(pathSrcSubDir1 / "file10.bin");
        fnCreateDummyFile(pathSrcSubDir1 / "file11.bin");
        std::filesystem::path pathSrcSubDir2 = pathSrc / "subdir2";
        std::filesystem::create_directories(pathSrcSubDir2);
        fnCreateDummyFile(pathSrcSubDir2 / "file20.bin");
        fnCreateDummyFile(pathSrcSubDir2 / "file21.bin");
        fnCreateDummyFile(pathSrcSubDir2 / "file22.bia");
        std::filesystem::path pathSrcSubDir3 = pathSrc / "subdir3_xyz";
        std::filesystem::create_directories(pathSrcSubDir3);
        fnCreateDummyFile(pathSrcSubDir3 / "file30.bin");
        fnCreateDummyFile(pathSrcSubDir3 / "file31.bin");
        fnCreateDummyFile(pathSrcSubDir3 / "file32a.bia");
        std::filesystem::path pathSrcSubDir4 = pathSrcSubDir3 / "subdir4";
        std::filesystem::create_directories(pathSrcSubDir4);
        fnCreateDummyFile(pathSrcSubDir4 / "file40a.bin");
        std::filesystem::path pathSrcSubDir5 = pathSrcSubDir4 / "subdir5";
        std::filesystem::create_directories(pathSrcSubDir5);
        fnCreateDummyFile(pathSrcSubDir5 / "file50a.bin");
    }
    catch (const std::filesystem::filesystem_error& rexception)
    {
        // Exception occurred.
        std::cerr << "Filesystem exception occurred (create test data): " << rexception.what() << std::endl;
        ASSERT_TRUE(false);
    }
}

void CPathMatchTest::TearDownTestSuite()
{
    try
    {
        std::filesystem::remove_all(GetExecDirectory() / "path_match_test");
    }
    catch (const std::filesystem::filesystem_error& rexception)
    {
        // Exception occurred.
        std::cerr << "Filesystem exception occurred (delete target test directories): " << rexception.what() << std::endl;
        ASSERT_TRUE(false);
    }
}
