#if defined(_WIN32)

#include "../../global/exec_dir_helper.h"

inline int GetLoggerFilesCount(const std::string& prefix)
{
    int count = 0;

    std::string path = GetExecDirectory().generic_string();
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::string filename = entry.path().filename().generic_string().c_str();
        if (filename.rfind(prefix, 0) == 0)
        {
            count++;
        }
    }
    return count;
}

inline void DeleteLoggerFile(const std::string& filename)
{
    auto path = GetExecDirectory() / filename;
    try
    {
        std::filesystem::remove(path);
    } catch (const std::filesystem::filesystem_error&)
    {}
}

inline uint32_t GetErrorCountFromLogFile(const std::string& filename)
{
    uint32_t errorCount = 0;
    auto path = GetExecDirectory() / filename;
    std::ifstream infile(path);
    if (infile.is_open())
    {
        std::string line;
        while (std::getline(infile, line))
        {
            size_t foundStart = line.find("Error;");
            if (foundStart != std::string::npos)
            {
                std::cout << line << std::endl;
                errorCount++;
            }
        }
    }

    if (errorCount != 0)
    {
        std::cout << "Checking " << filename << ", found: " << std::to_string(errorCount) << std::endl;
    }

    return errorCount;
}

inline bool FileNameStartsWith(const std::string& fileName, const std::string& startOfLogName)
{
    if (!startOfLogName.empty())
    {
        if (fileName.rfind(startOfLogName, 0) != 0) // returns std::string::npos (i.e. not found)
        {
            return false;
        }
    }
    return true;
}

inline uint32_t GetStringCountFromLogFiles(const std::string& startOfLogName, const std::string& subString)
{
    uint32_t errorCount = 0;

    auto path = GetExecDirectory();
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if ((entry.path().extension() == ".log") &&
            (FileNameStartsWith(entry.path().filename().string(), startOfLogName)))
        {
            std::ifstream infile(entry.path());
            if (infile.is_open())
            {
                std::string line;
                while (std::getline(infile, line))
                {
                    size_t foundStart = line.find(subString);
                    if (foundStart != std::string::npos)
                    {
                        errorCount++;
                    }
                }
                infile.close();
            }
        }
    }

    return errorCount;
}

inline uint32_t GetErrorCountFromLogFiles(const std::string& logName) { return GetStringCountFromLogFiles(logName, ";Error;"); }
inline uint32_t GetWarningCountFromLogFiles(const std::string& logName) { return GetStringCountFromLogFiles(logName, "Warning;"); }
inline uint32_t GetTraceCountFromLogFiles(const std::string& logName) { return GetStringCountFromLogFiles(logName, "Trace;"); }


#else
inline int GetLoggerFilesCount(const std::string& ) { return 0; }
inline void DeleteLoggerFile(const std::string& ) {	return; }
inline uint32_t GetErrorCountFromLogFile(const std::string&) { return 0; }
inline uint32_t GetErrorCountFromLogFiles(const std::string&) { return 0; }
inline uint32_t GetWarningCountFromLogFiles(const std::string&) { return 0; }
inline uint32_t GetTraceCountFromLogFiles(const std::string&) { return 0; }
#endif
