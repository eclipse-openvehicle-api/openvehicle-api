#include "sdv_core.h"
#include "toml_parser_util.h"    // Insert to create the factory of this utility
#include <fstream>
#include <sstream>

extern sdv::IInterfaceAccess* SDVCore()
{
    static CSDVCore& rcore = CSDVCore::GetInstance();
    return &rcore;
}

CSDVCore::CSDVCore()
{}

CSDVCore::~CSDVCore()
{}

CSDVCore& CSDVCore::GetInstance()
{
    static CSDVCore core;
    return core;
}

CAppControl& CSDVCore::GetAppControl()
{
    return m_appctrl;
}

CModuleControl& CSDVCore::GetModuleControl()
{
    return m_modulectrl;
}

CMemoryManager& CSDVCore::GetMemoryManager()
{
    return m_memmgr;
}

CRepository& CSDVCore::GetRepository()
{
    return m_repository;
}

CLoggerControl& CSDVCore::GetLoggerControl()
{
    return m_loggerctrl;
}

CLogger& CSDVCore::GetDefaultLogger()
{
    return m_defaultlogger;
}

CAppConfig& CSDVCore::GetAppConfig()
{
    return m_appconfig;
}

CAppControl& GetAppControl()
{
    return CSDVCore::GetInstance().GetAppControl();
}

CModuleControl& GetModuleControl()
{
    return CSDVCore::GetInstance().GetModuleControl();
}

CMemoryManager& GetMemoryManager()
{
    return CSDVCore::GetInstance().GetMemoryManager();
}

CRepository& GetRepository()
{
    return CSDVCore::GetInstance().GetRepository();
}

CLoggerControl& GetLoggerControl()
{
    return CSDVCore::GetInstance().GetLoggerControl();
}

CLogger& GetDefaultLogger()
{
    return CSDVCore::GetInstance().GetDefaultLogger();
}

CAppConfig& GetAppConfig()
{
    return CSDVCore::GetInstance().GetAppConfig();
}

std::filesystem::path GetCoreDirectory()
{
    static std::filesystem::path pathCoreDir;
    if (!pathCoreDir.empty()) return pathCoreDir;

#ifdef _WIN32
    // Windows specific
    std::wstring ssPath(32768, '\0');

    MEMORY_BASIC_INFORMATION sMemInfo{};
    if (!VirtualQuery(&pathCoreDir, &sMemInfo, sizeof(sMemInfo))) return pathCoreDir;
    DWORD dwLength = GetModuleFileNameW(reinterpret_cast<HINSTANCE>(sMemInfo.AllocationBase), ssPath.data(), 32767);
    ssPath.resize(dwLength);
    pathCoreDir = std::filesystem::path(ssPath);
    return pathCoreDir.remove_filename();
#elif __linux__
    // Read the maps file. It contains all loaded SOs.
    std::ifstream fstream("/proc/self/maps");
    std::stringstream sstreamMap;
    sstreamMap << fstream.rdbuf();
    std::string ssMap = sstreamMap.str();
    if (ssMap.empty())
        return pathCoreDir; // Some error

   // Find the "core_services.sdv"
    size_t nPos = ssMap.find("core_services.sdv");
    if (nPos == std::string::npos) return pathCoreDir;
    size_t nEnd = nPos;

    // Find the start... runbackwards until the beginning of the line and remember the earliest occurance of a slash
    size_t nBegin = 0;
    while (nPos && ssMap[nPos] != '\n')
    {
        if (ssMap[nPos] == '/')
            nBegin = nPos;
        nPos--;
    }
    if (!nBegin) nBegin = nPos;

    // Return the path
    pathCoreDir = ssMap.substr(nBegin, nEnd - nBegin);

    return pathCoreDir;
#else
#error The OS is not supported!
#endif
}
