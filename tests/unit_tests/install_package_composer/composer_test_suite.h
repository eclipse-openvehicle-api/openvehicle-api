#ifndef COMPOSER_TEST_SUITE_H
#define COMPOSER_TEST_SUITE_H

#include <gtest/gtest.h>
#include <filesystem>
#include <regex>
#include <string>

#include "../../../global/localmemmgr.h"

namespace sdv
{
    namespace core
    {
        using TModuleID = uint64_t;
    }
}

/**
 * @brief Install package composer test suite class.
 */
class CInstallPackageComposerTest : public ::testing::Test
{
public:
    /**
     * @brief Setup test test
     */
    virtual void SetUp();

    /**
     * @brief Tear down test
     */
    virtual void TearDown();

    /**
     * @brief Setup test case
     */
    static void SetUpTestCase();

    /**
     * @brief Tear down test case
     */
    static void TearDownTestSuite();

    /**
     * @brief Delete target test directory
     */
    static void DeleteTargetTestDir();

    /**
     * @brief Delete all test directories
     */
    static void DeleteTestDirs();

    /**
     * @brief Check whether the content of one file is identical to the content of the other file.
     * @param[in] rpath1 Reference to the path of the first file.
     * @param[in] rpath2 Reference to the path of the second file.
     * @param[in] bCheckTimes Check creation and change times (if supported by OS).
     * @param[in] bCheckAttributes Check file attributes (if asupported by OS).
     * @return Returns wether the files are identical and if requested times and attributes fit.
     */
    bool AreFilesEqual(const std::filesystem::path& rpath1, const std::filesystem::path& rpath2, bool bCheckTimes = false,
        bool bCheckAttributes = false);

#if defined _WIN32 || (defined __linux__ && COMPOSER_SUPPORT_READONLY_LINUX)
    /**
     * @brief Set the file to read-only.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static void SetReadOnly(const std::filesystem::path& rpathFile);

    /**
     * @brief Set the file to writable.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static void ResetReadOnly(const std::filesystem::path& rpathFile);

    /**
     * @brief Get the file read-only-state.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static bool IsReadOnly(const std::filesystem::path& rpathFile);
#endif

#ifdef __unix__
    /**
     * @brief Set the file to executable.
     * @remarks Not available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static void SetExecutable(const std::filesystem::path& rpathFile);

    /**
     * @brief Get the file to executable state.
     * @remarks Not available for WIndows.
     * @param[in] rpathFile Reference to the path of the file.
     */
    static bool IsExecutable(const std::filesystem::path& rpathFile);
#endif

#ifdef _WIN32
    /**
     * @brief Set the file creation time.
     * @remarks Only available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     * @param[in] uiTimeMicrosec The unix epoch time in microseconds.
     */
    static void SetCreateTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec);

    /**
     * @brief Get the file creation time.
     * @remarks Only available for Windows.
     * @param[in] rpathFile Reference to the path of the file.
     * @return Returns the unix epoch time in microseconds.
     */
    static uint64_t GetCreateTime(const std::filesystem::path& rpathFile);
#endif

    /**
     * @brief Set the file change time.
     * @param[in] rpathFile Reference to the path of the file.
     * @param[in] uiTimeMicrosec The unix epoch time in microseconds.
     */
    static void SetChangeTime(const std::filesystem::path& rpathFile, uint64_t uiTimeMicrosec);

    /**
     * @brief Get the file change time.
     * @param[in] rpathFile Reference to the path of the file.
     * @return Returns the unix epoch time in microseconds.
     */
    static uint64_t GetChangeTime(const std::filesystem::path& rpathFile);
};

#endif // !defined COMPOSER_TEST_SUITE_H