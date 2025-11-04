#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include "../../../global/exec_dir_helper.h"
#include <stdlib.h>
#ifdef __unix__
#include <spawn.h>
#include <sys/wait.h>
#endif

/**
 * @brief Helper class to store and restore the current state of various core library specific settings and to execute the test in
 * a specific subdirectory.
 * @remarks There is a difference between the usage of the path variable in Windows and in Posix. In Windows, environment'
 * variables being changed by the application are local to the application and are applied directly. In Posix, the ld.so
 * environment variables being changed by the application are only valid for any process starting after the change, but not for a
 * running process. Therefore the implementation under Posix would need to explicitly start the test in its own executable with the
 * variable changed.
 */
class CTestExecute
{
public:

#ifdef _WIN32
    /**
     * @brief The name of the test application to execute.
     */
    static constexpr char szTestAppName[] = "UnitTest_CoreLoader_TestApp.exe";
#else
    /**
     * @brief The name of the test application to execute.
     */
    static constexpr char szTestAppName[] = "UnitTest_CoreLoader_TestApp";
#endif

    /**
     * @brief Test executor constructor
     * @details Create a specific test sub directory and copy the test applicatin into it. Store the current directory and move to
     * the location of the executable. Store the current PATH/LD_LIBRARY_PATH environment variable and reset the variable. Store
     * the current value of an existing SDV_FRAMEWORK_RUNTIME environment variable and remove the variable.
     */
    CTestExecute()
    {
        // Get the timestring used for renaming files
        auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream sstreamTime;
        sstreamTime << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");

        // Create a test directory
        m_pathExeDir = GetExecDirectory() / (std::string("core_loader_test_") + sstreamTime.str());
        if (!std::filesystem::create_directory(m_pathExeDir)) return;

        // Copy the test application
        std::filesystem::copy(GetExecDirectory() / szTestAppName, m_pathExeDir / szTestAppName);

        // Store the current directory and move to the directory of the executable
        m_pathCurrentDir = std::filesystem::current_path();
        std::filesystem::current_path(m_pathExeDir);

        // Store and remove the current PATH/LD_LIBRARY_PATH variable
#ifdef _WIN32
        const wchar_t* szPath = _wgetenv(L"PATH");
        if (szPath) m_ssPath = szPath;
        // TODO: It could be that the path to the core lib is already available. Scan the path list and remove any to the core
        // lib. Reassign the path.
        // _wputenv(L"PATH=");
#elif defined __unix__
        const char* szPath = getenv("LD_LIBRARY_PATH");
        if (szPath) m_ssPath = szPath;
        setenv("LD_LIBRARY_PATH", "", true);
#else
#error OS not supported!
#endif

        // Store and remove the current SDV_FRAMEWORK_RUNTIME variable
#ifdef _WIN32
        const wchar_t* szFrameworkDir = _wgetenv(L"SDV_FRAMEWORK_RUNTIME");
        if (szFrameworkDir)
        {
            m_ssFrameworkDir = szFrameworkDir;
            _wputenv(L"SDV_FRAMEWORK_RUNTIME=");
        }
#elif defined __unix__
        const char* szFrameworkDir = getenv("SDV_FRAMEWORK_RUNTIME");
        if (szFrameworkDir)
        {
            m_ssFrameworkDir = szFrameworkDir;
            unsetenv("SDV_FRAMEWORK_RUNTIME");
        }
#else
#error OS not supported!
#endif
    }

    /**
     * @brief Test executor destructor.
     * @details Return to the previous current-path location. Restore the previous PATH/LD_LIBRARY_PATH environment variable. If a
     * previous existing SDV_FRAMEWORK_RUNTIME environment variable was existing, restore this variable. Remove the test directory.
     */
    ~CTestExecute()
    {
        // Restore the current directory
        std::filesystem::current_path(m_pathCurrentDir);

        // Delete the test directory
        if (!m_pathExeDir.empty())
        {
            try
            {
                std::filesystem::remove_all(m_pathExeDir);
            } catch (const std::filesystem::filesystem_error&)
            {}
        }

        // Restore the SDV_FRAMEWORK_RUNTIME variable
        if (!m_ssFrameworkDir.empty())
        {
#ifdef _WIN32
            _wputenv((std::wstring(L"SDV_FRAMEWORK_RUNTIME=") + m_ssFrameworkDir).c_str());
#elif defined __unix__
            setenv("SDV_FRAMEWORK_RUNTIME", m_ssFrameworkDir.c_str(), true);
#else
#error OS not supported!
#endif
        }

        // Restore the PATH/LD_LIBRARY_PATH variable
#ifdef _WIN32
        _wputenv((std::wstring(L"PATH=") + m_ssPath).c_str());
#elif defined __unix__
        setenv("LD_LIBRARY_PATH", m_ssPath.c_str(), true);
#else
#error OS not supported!
#endif
    }

    /**
     * @brief Run the test app process and check the result.
     * @return Returns a 1 when the test app process could run and provided a positive result. Returns 0 when the test app process
     * could run, but didn't provide a positive result. Returns -1 when the test app process couldn't run.
     */
    int Execute()
    {
        // Start the process and wait for it to finish
#ifdef _WIN32
        PROCESS_INFORMATION sProcessInfo{};
        STARTUPINFO sStartupInfo{};
        sStartupInfo.cb = sizeof(sStartupInfo);
        wchar_t szCommandLine[32768] = {};
        wcscpy(szCommandLine, (m_pathExeDir / szTestAppName).native().c_str());
        //wcscpy(szCommandLine, std::filesystem::path(szTestAppName).native().c_str());
        if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &sStartupInfo, &sProcessInfo))
        {
            std::cerr << "Failed to create the test process... (" << GetLastError() << ")" << std::endl;
            return -1;
        }
        if (WaitForSingleObject(sProcessInfo.hProcess, INFINITE) != WAIT_OBJECT_0)
        {
            std::cerr << "Process was not started..." << std::endl;
            return -1;
        }
        DWORD dwExitCode = 0xffffffff;
        GetExitCodeProcess(sProcessInfo.hProcess, &dwExitCode);
        if (dwExitCode)
        {
            std::cerr << "Process exited with exit code: 0x" << std::hex << dwExitCode << std::endl;
            return -1;
        }
        if (sProcessInfo.hProcess) CloseHandle(sProcessInfo.hProcess);
        if (sProcessInfo.hThread) CloseHandle(sProcessInfo.hThread);
#elif defined __unix__
        pid_t processID = 0;
        char* szArg = nullptr;
        int spawnResult = posix_spawn(&processID, (m_pathExeDir / szTestAppName).native().c_str(), nullptr, nullptr,
            &szArg, environ);
        if (spawnResult != 0) return -1;
        int status = 0;
        do
        {
            if (waitpid(processID, &status, WUNTRACED | WCONTINUED) == -1)
                return -1;
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
#else
#error OS is not supported!
#endif

        // Read the result from the result file
        bool bResult = false;
        std::ifstream fstream(m_pathExeDir / "CoreLoading_result.txt");
        if (!fstream.eof())
            fstream >> bResult;

        // Done.
        return bResult ? 1 : 0;
    }

    /**
     * @brief Add the core directory to the PATH/DL_LIBRARY_PATH environment variable.
     */
    void AddCoreToSearchPath()
    {
        // Store and remove the current PATH/LD_LIBRARY_PATH variable
#ifdef _WIN32
        std::wstringstream sstreamPath;
        sstreamPath << L"PATH=" << m_ssPath;
        if (!m_ssPath.empty())
            sstreamPath << L";";
        sstreamPath << (GetExecDirectory() / "../../bin").lexically_normal().native();
        _wputenv(sstreamPath.str().c_str());
#elif defined __unix__
        setenv("LD_LIBRARY_PATH", (GetExecDirectory() / "../../bin").lexically_normal().native().c_str(), true);
#else
#error OS not supported!
#endif
    }

    /**
     * @brief Set framework environment variable.
     */
    void SetFrameworkEnvVar()
    {
#ifdef _WIN32
        _wputenv((std::wstring(L"SDV_FRAMEWORK_RUNTIME=") + (GetExecDirectory() / "../../bin").lexically_normal().native()).c_str());
#elif defined __unix__
        setenv("SDV_FRAMEWORK_RUNTIME", (GetExecDirectory() / "../../bin").lexically_normal().native().c_str(), true);
#else
#error OS not supported!
#endif
    }

    /**
     * @brief Return the location of the executable directory.
     * @return The executable directory path.
    */
    std::filesystem::path GetExeDir() const
    {
        return m_pathExeDir;
    }

private:
    std::filesystem::path   m_pathExeDir;       ///< Execution path directory
    std::filesystem::path   m_pathCurrentDir;   ///< The current directory.
#ifdef _WIN32
    std::wstring        m_ssPath;               ///< The current PATH environment variable before changing.
    std::wstring        m_ssFrameworkDir;       ///< The current SDV_FRAMEWORK_RUNTIME environment variable before changing.
#elif __unix__
    std::string        m_ssPath;                ///< The current PATH environment variable before changing.
    std::string        m_ssFrameworkDir;        ///< The current SDV_FRAMEWORK_RUNTIME environment variable before changing.
#else
#error OS not supported!
#endif
};

TEST(CoreLoaderTest, FailedLoading)
{
    CTestExecute executor;
    EXPECT_EQ(executor.Execute(), 0);
}

TEST(CoreLoaderTest, SearchPathLoading)
{
    CTestExecute executor;
    executor.AddCoreToSearchPath();
    EXPECT_EQ(executor.Execute(), 1);
}

TEST(CoreLoaderTest, EnvVarLoading)
{
    CTestExecute executor;
    executor.SetFrameworkEnvVar();
    EXPECT_EQ(executor.Execute(), 1);
}

TEST(CoreLoaderTest, CfgFileLoadingRel)
{
    CTestExecute executor;

    std::ofstream fstream(executor.GetExeDir() / "sdv_core_reloc.toml");
    fstream << R"cfg(# Some bogus information
unknown_var = 2

# Core library path
directory = "../../../bin")cfg";
    fstream.close();

    EXPECT_EQ(executor.Execute(), 1);
}

TEST(CoreLoaderTest, CfgFileLoadingAbs)
{
    CTestExecute executor;

    std::ofstream fstream(executor.GetExeDir() / "sdv_core_reloc.toml");
    fstream << R"cfg(# Some bogus information
unknown_var = 2

# Core library path
directory = ")cfg" << (executor.GetExeDir() / "../../../bin").lexically_normal().generic_string() << "\"";
    fstream.close();

    EXPECT_EQ(executor.Execute(), 1);
}

#ifdef _WIN32
TEST(CoreLoaderTest, CfgFileLoadingRelWin)
{
    CTestExecute executor;

    std::ofstream fstream(executor.GetExeDir() / "sdv_core_reloc.toml");
    fstream << R"cfg(# Some bogus information
unknown_var = 2

# Core library path
directory = "..\\..\\..\\bin")cfg";
    fstream.close();

    EXPECT_EQ(executor.Execute(), 1);
}

TEST(CoreLoaderTest, CfgFileLoadingAbsWin)
{
    CTestExecute executor;

    // Convert UTF-16 to ANSI ignoring all non-convertable characters and add an escape character for each backslash
    auto fnSimpleMakeAnsi = [](const std::wstring& rss)
    {
        std::string ss;
        for (wchar_t c : rss)
        {
            if (c == '\\')
                ss.push_back(static_cast<char>(c));
            ss.push_back(static_cast<char>(c));
        }
        return ss;
    };

    std::string ssDir = fnSimpleMakeAnsi((executor.GetExeDir() / "../../../bin").lexically_normal().native());

    std::ofstream fstream(executor.GetExeDir() / "sdv_core_reloc.toml");
    fstream << R"cfg(# Some bogus information
unknown_var = 2

# Core library path
directory = ")cfg" << ssDir << "\"";
    fstream.close();

    EXPECT_EQ(executor.Execute(), 1);
}
#endif

TEST(CoreLoaderTest, IdenticalLocationLoading)
{
    CTestExecute executor;

    std::filesystem::copy(GetExecDirectory() / "../../bin/core_services.sdv", executor.GetExeDir() / "core_services.sdv");

    EXPECT_EQ(executor.Execute(), 1);
}
