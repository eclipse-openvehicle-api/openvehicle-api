#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../../../global/process_watchdog.h"
#include "../global/exec_dir_helper.h"
#ifdef _WIN32
#include <windows.h>
#endif

class VSSUtilTest : public ::testing::Test
{
public:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

    static void SetUpTestCase()
    {
        std::cout << "SetUpTestCase()" << std::endl;

        std::vector<std::string> subFolders = { "generated0", "generated1", "generated2" };
        for (auto subFolder : subFolders)
        {
            auto generatedPath = GetExecDirectory() / subFolder;
            try
            {
                if (std::filesystem::is_directory(generatedPath))
                {
                    for (const auto& entry : std::filesystem::directory_iterator(generatedPath))
                    {
                        std::filesystem::remove_all(entry.path());
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                std::cout << "catch (const std::filesystem::filesystem_error& e)" << std::endl;
                std::cout << "Error removing directory: " << e.what() << '\n';
            }
        }
    }

    static void TearDownTestSuite() {}
};


std::uintmax_t CountNumberOfFiles(const std::filesystem::path allFilesPath)
{
    std::uintmax_t allFilesCount = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(allFilesPath))
    {
        if (std::filesystem::is_regular_file(entry.status()))
        {
            ++allFilesCount;
        }
    }
    return allFilesCount;
}

void  DeleteExistingHeaderFiles(const std::filesystem::path allFilesPath)
{
    std::string doNotDeleteSignalIdentifier = "signal_identifier.h";

    if (std::filesystem::is_directory(allFilesPath))
    {
        for (const auto& entry : std::filesystem::directory_iterator(allFilesPath))
        {
            if (entry.path().extension() == ".h")
            {
                if (entry.path().filename() != doNotDeleteSignalIdentifier)
                {
                    try
                    {
                        std::filesystem::remove(entry.path());
                        std::cout << "File deleted: " << entry.path().filename().string() << std::endl;
                    } catch (const std::filesystem::filesystem_error&)
                    {
                        std::cout << "Failed to delete: " << entry.path().filename().string() << std::endl;
                    }
                }
            }
        }
    }
}

void DeletePSAndSerdesFolder(const std::filesystem::path allFilesPath)
{
    auto psFolder = allFilesPath / "ps";
    auto serdesolder = allFilesPath / "serdes";
    if (std::filesystem::is_directory(psFolder))
    {
        try
        {
            std::filesystem::remove_all(psFolder);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cout << "catch (const std::filesystem::filesystem_error& e)" << std::endl;
            std::cout << "Error removing directory: " << e.what() << '\n';
        }
    }
    if (std::filesystem::is_directory(serdesolder))
    {
        try
        {
            std::filesystem::remove_all(serdesolder);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cout << "catch (const std::filesystem::filesystem_error& e)" << std::endl;
            std::cout << "Error removing directory: " << e.what() << '\n';
        }
    }
}

std::filesystem::path GetExportFolder(std::filesystem::path folder)
{
    while (!folder.empty())
    {
        auto exportFolder = folder / "export";
        if (std::filesystem::is_directory(exportFolder))
            return exportFolder;
        folder = folder.parent_path();
    }

    return {};
}

/**
 * @brief Build the command line by quoting all arguments. This is the one argument function.
 * @param[in] rssArg The argument to quote.
 * @return The quoted argument.
 */
inline std::string BuildCommandLine(const std::string& rssArg)
{
    return std::string("\"") + rssArg + std::string("\"");
}

/**
 * @brief Build the command line by quoting all arguments. This is the multiple argument function.
 * @tparam ...TArgs Argument types. Needs to be implicitly convertable to std::string.
 * @param[in] rssArg The one argument to quote.
 * @param[in] rgtArgs The other arguments to quote.
 * @return The command line with the quoted arguments.
 */
template <typename... TArgs>
inline std::string BuildCommandLine(const std::string& rssArg, const TArgs&... rgtArgs)
{
    return std::string("\"") + rssArg + std::string("\" ") + BuildCommandLine(rgtArgs...);
}

/**
 * @brief Platform specific implementation for executing command having space in the command path and in the parameters.
 * @tparam ...TArgs
 * @param[in] rssCommand The command line string.
 * @param[in] rgtArgs List of string arguments.
 * @return The result of the application or -1 when the application could not be started.
 */
template <typename... TArgs>
inline int ExecuteCommand(const std::string& rssCommand, const TArgs&... rgtArgs)
{
    // Build the command line by quoting each argument.
    std::string ssFullCommand = BuildCommandLine(rssCommand, rgtArgs...);

    // NOTE: The std::system command is a bit peculiar when dealing with spaces int he path name under Windows. Even though each
    // command line argument is quoted, the quotes are removed before the command line is supplied to CMD /C. To solve this issue
    // add additional quotes at the beginning and end of the complete command line: \"app arg1 arg2 arg3\". See also:
    // https://stackoverflow.com/questions/27975969/how-to-run-an-executable-with-spaces-using-stdsystem-on-windows
    // --> This seems to work with the std::system of MSVC, not not of MINGW!
    // --> For MINGW there is no solution... use platform specific solution is the advice.

#ifdef _WIN32
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    // Convert std::string to LPSTR
    LPSTR szCommand = const_cast<char*>(ssFullCommand.c_str());

    // Start the child process
    if (!CreateProcessA(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
        return -1;
    }

    // Wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get the exit code
    int iRet = 0;
    GetExitCodeProcess(pi.hProcess, reinterpret_cast<DWORD*>(&iRet));

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return iRet;
#elif defined __unix__
    return std::system(ssFullCommand.c_str());
#else
#error The OS is not supported!
#endif
}

int CreateCode(const std::string& document, const std::string& subfolder, const bool bCreateComponents)
{
    auto vssUtility = (GetExecDirectory() / "../../bin/sdv_vss_util").lexically_normal();
    auto documentPath = (GetExecDirectory() / "../../bin" / document).lexically_normal();
    auto outputFolder = (GetExecDirectory() / subfolder).lexically_normal();

    // VSS_Sheet_Single.csv -O${VSSUTIL_OUTPUT_DIRECTORY}/generated/  --version1.0.0.2  --prefixdemo  --enable_components
    if (bCreateComponents)
    {
        return ExecuteCommand(vssUtility.generic_string(), documentPath.generic_string(), std::string("-O") + outputFolder.generic_string(),
            std::string("--enable_components"), std::string("--prefixdemo"), std::string("--version1.2.3.4"), std::string("--silent"));//
    }


    return ExecuteCommand(vssUtility.generic_string(), documentPath.generic_string(), std::string("-O") + outputFolder.generic_string(),
        std::string("--prefixdemo"),  std::string("--version1.2.3.4"), std::string("--silent"));
}

int CompileIDLFiles(const std::filesystem::path allFilesPath)
{
    bool noFilesFound = true;
    int result = 0;
    auto idlCompiler = GetExecDirectory() / "../../bin/sdv_idl_compiler";
    auto idlFiles = allFilesPath / "vss_files";

    for (const auto& entry : std::filesystem::directory_iterator(idlFiles))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".idl") continue;
        noFilesFound = false;

        std::cout << "Call IDL Compiler for file " << entry.path().filename().string() << std::endl;

        // C:/Data/out/build/x64-Debug/tests/bin/../../bin/sdv_idl_compiler "../tests/bin/generated2/vss_files/VSS_VehicleVehicleDrivetrainTransmissionIsParkLockEngaged_tx.idl"
        // -O../tests/bin/generated2/vss_files -I../tests/bin/generated2/vss_files -I../../../../export
        result += ExecuteCommand(idlCompiler.lexically_normal().generic_string(),
            (idlFiles.lexically_normal() / entry.path().filename()).generic_string(),
            std::string("-O") + idlFiles.lexically_normal().generic_string(),
            std::string("-I") + idlFiles.lexically_normal().generic_string(),
            std::string("-I") + GetExportFolder(allFilesPath).lexically_normal().generic_string());
    }
    if (noFilesFound)
    {
        return 1;
    }
    return result;
}

TEST_F(VSSUtilTest, CreateCodeValidateGeneratedFilesUniqueSignalNamesNoComponents)
{
    EXPECT_EQ(0, CreateCode("vss_unique_names.csv", "generated0", false));

    // files created by VSS_DifferentVDandBSSingleSignals.csv
    // find expected IDL files and check number of files in folder 'generated0'
    std::filesystem::path allFilesPath = GetExecDirectory() / "generated0";
    std::filesystem::path vssFilePath = allFilesPath / "vss_files";
    std::filesystem::path signalIdentifierVDFilePath = vssFilePath / "signal_identifier.h";
    std::filesystem::path summaryVDFilePath = vssFilePath / "summary.txt";

    EXPECT_TRUE(std::filesystem::exists(summaryVDFilePath));
    EXPECT_TRUE(std::filesystem::exists(signalIdentifierVDFilePath));

    std::uintmax_t allFilesCount = CountNumberOfFiles(allFilesPath);

    // 4 signals, but they are combined in the same vss definition so we have 1 rx and 1 tx
    uint32_t signals = 4; // 8 header files *.h + 0 cpp files *.cpp  + 0 cmake files + 8 *.idl + 2 * identifier.h + 2 * summary.txt
    uint32_t expected = 2 * signals + 2;
    EXPECT_EQ(allFilesCount, expected);

    auto result = CompileIDLFiles(allFilesPath);
    EXPECT_EQ(0, result);

    DeletePSAndSerdesFolder(vssFilePath); // created by the idl compiler
    allFilesCount = CountNumberOfFiles(allFilesPath);
    EXPECT_EQ(allFilesCount, expected + (2 * signals)); // + compiled header files
}

TEST_F(VSSUtilTest, ValidateGeneratedFilesUniqueSignalNames)
{
    EXPECT_EQ(0, CreateCode("vss_unique_names.csv", "generated1", true));

    // files created by VSS_DifferentVDandBSSingleSignals.csv
    // find expected IDL files and check number of files in folder 'generated1'
    std::filesystem::path allFilesPath = GetExecDirectory() / "generated1";
    std::filesystem::path vssFilePath = allFilesPath / "vss_files";
    std::filesystem::path signalIdentifierVDFilePath = vssFilePath / "signal_identifier.h";
    std::filesystem::path summaryVDFilePath = vssFilePath / "summary.txt";

    EXPECT_TRUE(std::filesystem::exists(summaryVDFilePath));
    EXPECT_TRUE(std::filesystem::exists(signalIdentifierVDFilePath));

    std::uintmax_t allFilesCount = CountNumberOfFiles(allFilesPath);

    // 4 signals, but they are combined in the same vss definition so we have 1 rx and 1 tx
    uint32_t signals = 4; // 2 header files *.h + 2 cpp files *.cpp  + 2 cmake files + 2 *.idl + 2 * identifier.h + 2 * summary.txt
    uint32_t expected =      2 * signals        + 2 * signals        + 2 *signals       + (2 * signals) + 2;
    EXPECT_EQ(allFilesCount, expected);

    auto result = CompileIDLFiles(allFilesPath);
    EXPECT_EQ(0, result);

    DeletePSAndSerdesFolder(vssFilePath); // created by the idl compiler
    allFilesCount = CountNumberOfFiles(allFilesPath);
    EXPECT_EQ(allFilesCount, expected + (2 * signals)); // + compiled header files
}

TEST_F(VSSUtilTest, ValidateGeneratedFilesCombinedSignalNames)
{
    EXPECT_EQ(0, CreateCode("vss_multiple_names.csv", "generated2", true));

    // files created by VSS_DifferentVDandBSMultipleSignals.csv
    // find expected IDL files and check number of files in folder 'generated2'
    std::filesystem::path allFilesPath = GetExecDirectory() / "generated2";
    std::filesystem::path vssFilePath = allFilesPath / "vss_files";
    std::filesystem::path signalIdentifierVDFilePath = vssFilePath / "signal_identifier.h";
    std::filesystem::path summaryVDFilePath = vssFilePath / "summary.txt";

    EXPECT_TRUE(std::filesystem::exists(summaryVDFilePath));
    EXPECT_TRUE(std::filesystem::exists(signalIdentifierVDFilePath));

    std::uintmax_t allFilesCount = CountNumberOfFiles(allFilesPath);

    // 4 signals, but they are combined in the same vss definition so we have 1 rx and 1 tx
    uint32_t signals = 2; // 2 signals = 4 *.h + 4 *.cpp + 4 cmakelists + 4 *.idl + 2 * identifier.h + 2 * summary.txt
    uint32_t expected = (2 * signals) + (2 * signals) + (2 * signals) + (2 * signals) + 2;
    EXPECT_EQ(allFilesCount, expected);

    EXPECT_EQ(0, CompileIDLFiles(allFilesPath));

    DeletePSAndSerdesFolder(vssFilePath); // created by the idl compiler
    allFilesCount = CountNumberOfFiles(allFilesPath);
    EXPECT_EQ(allFilesCount, expected + (2 * signals));
}

extern "C" int main(int argc, char* argv[])
{
    CProcessWatchdog watchdog;

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}
