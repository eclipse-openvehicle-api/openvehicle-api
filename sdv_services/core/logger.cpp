#include "logger.h"
#include <sstream>
#include "sdv_core.h"
#include "../../global/exec_dir_helper.h"

#ifdef __unix__
#include <syslog.h>
#endif
#ifdef __GNUC__
#include <unistd.h> // for getpid
#endif

CLogger::~CLogger()
{
#ifdef __unix__
    if (m_bLogOpen) closelog();
#endif
}

void CLogger::Log(sdv::core::ELogSeverity eSeverity, /*in*/ const sdv::u8string& ssSrcFile, /*in*/ uint32_t uiSrcLine,
    /*in*/ sdv::process::TProcessID tProcessID, /*in*/ const sdv:: u8string& ssObjectName, /*in*/ const sdv::u8string& ssMessage)
{
    if (static_cast<uint32_t>(eSeverity) >= static_cast<uint32_t>(m_eViewFilter))
    {
        if (tProcessID) std::clog << "[PID#" << static_cast<int64_t>(tProcessID) << "] ";
        if (!ssObjectName.empty()) std::clog << ssObjectName << " ";
        switch (eSeverity)
        {
        case sdv::core::ELogSeverity::debug:
            std::clog << "Debug: " << ssMessage << std::endl;
            break;
        case sdv::core::ELogSeverity::trace:
            std::clog << "Trace: " << ssMessage << std::endl;
            break;
        case sdv::core::ELogSeverity::info:
            std::clog << "Info: " << ssMessage << std::endl;
            break;
        case sdv::core::ELogSeverity::warning:
            std::clog << "Warning: " << ssMessage << std::endl;
            break;
        case sdv::core::ELogSeverity::error:
            std::clog << "Error: " << ssMessage << std::endl;
            break;
        case sdv::core::ELogSeverity::fatal:
            std::clog << "Fatal: " << ssMessage << std::endl;
            break;
        default:
            break;
        }
    }

    // Apply the log filter
    if (static_cast<uint32_t>(eSeverity) < static_cast<uint32_t>(m_eFilter)) return;

    std::unique_lock<std::recursive_mutex> lock(m_mtxLogger);
    if (m_ssProgramtag.empty())
        m_ssProgramtag = std::string("SDV_LOG_") + std::to_string(getpid());

    sdv::u8string ssExtendedMessage;
    ssExtendedMessage += ssSrcFile + "(" + std::to_string(uiSrcLine) + "): " + ssMessage;

#ifdef _WIN32
    if (!m_ptrWriter)
    {
        std::string logFileName = m_ssProgramtag + "_" +
            GetDateTime(std::chrono::system_clock::now()) + ".log";

        auto path = GetExecDirectory() / logFileName;
        auto stream = std::make_shared<std::ofstream>(path, std::ios_base::out | std::ios_base::app);
        if (!stream->is_open())
        {
            std::cerr << "ERROR: Log file cannot be created!" << std::endl;
            return;
        }
        m_ptrWriter = std::move(std::make_unique<CLogCSVWriter>(stream));
        if (!m_ptrWriter)
        {
            std::cerr << "ERROR: Cannot create CSV write object!" << std::endl;
            return;
        }
    }
    m_ptrWriter->Write(ssExtendedMessage, eSeverity, std::chrono::system_clock::now());
#elif defined __unix__
    if (!m_bLogOpen)
        openlog(m_ssProgramtag.c_str(), LOG_PID | LOG_CONS, LOG_USER);
    m_bLogOpen = true;
    switch(eSeverity)
    {
    case sdv::core::ELogSeverity::trace   : syslog(LOG_NOTICE, "%s", ssExtendedMessage.c_str());  break;
    case sdv::core::ELogSeverity::debug   : syslog(LOG_DEBUG,  "%s", ssExtendedMessage.c_str());  break;
    case sdv::core::ELogSeverity::info    : syslog(LOG_INFO, "%s", ssExtendedMessage.c_str());    break;
    case sdv::core::ELogSeverity::warning : syslog(LOG_WARNING, "%s", ssExtendedMessage.c_str()); break;
    case sdv::core::ELogSeverity::error   : syslog(LOG_ERR, "%s", ssExtendedMessage.c_str());     break;
    case sdv::core::ELogSeverity::fatal   : syslog(LOG_CRIT, "%s", ssExtendedMessage.c_str());    break;
    default: std::cerr << "ERROR: Logging level is undefined!";     break;
    }
#else
#error OS not supported!
#endif
}

void CLogger::SetProgramTag(const sdv::u8string& ssTag)
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxLogger);

    if (m_ssProgramtag.empty())
        m_ssProgramtag = ssTag;
}

sdv::u8string CLogger::GetProgramTag() const
{
    return m_ssProgramtag;
}

void CLogger::SetLogFilter(/*in*/ sdv::core::ELogSeverity eSeverity, /*in*/ sdv::core::ELogSeverity eViewSeverity)
{
    m_eFilter = eSeverity;
    m_eViewFilter = eViewSeverity;
}

sdv::core::ELogSeverity CLogger::GetLogFilter() const
{
    return m_eFilter;
}

sdv::core::ELogSeverity CLogger::GetViewFilter() const
{
    return m_eViewFilter;
}

std::string CLogger::GetDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp)
{
    using namespace std::chrono;
    std::stringstream output;
    microseconds	  micros = duration_cast<microseconds>(timestamp.time_since_epoch());

    std::string timeFormat = "_%Y.%m.%d_%H-%M-%S";

    std::time_t time = system_clock::to_time_t(timestamp);
    output << std::put_time(std::localtime(&time), timeFormat.c_str());
    return output.str();
}

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

CLogger& CLoggerService::GetDefaultLogger()
{
    return ::GetDefaultLogger();
}

#endif