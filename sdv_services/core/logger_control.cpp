#include "logger_control.h"

#ifdef _WIN32
#include <process.h>    // Needed for _getpid 
#endif

CLoggerControl::~CLoggerControl()
{
    // Reset the logger. This will finialize an open log.
    SetLogger(nullptr);
}

void CLoggerControl::Log(/*in*/ sdv::core::ELogSeverity eSeverity, /*in*/ const sdv::u8string& ssSrcFile, /*in*/ uint32_t uiSrcLine,
    /*in*/ const sdv::process::TProcessID tProcessID, /*in*/ const sdv::u8string& ssObjectName, /*in*/ const sdv::u8string& ssMessage)
{
    std::unique_lock<std::mutex> lock(m_mtxPrematureLog);

    // Is a registered logger available?
    if (m_pLogger)
    {
        sdv::core::ILogger* pLogger = m_pLogger;
        lock.unlock();
        pLogger->Log(eSeverity, ssSrcFile, uiSrcLine, tProcessID, ssObjectName, ssMessage);
        return;
    }

    // Store the log entry for later use. Store at the most 1000 messages.
    if (m_queuePrematureLog.size() >= 1000) return;
    if (m_queuePrematureLog.size() == 999)
    {
#ifdef _WIN32
        sdv::process::TProcessID tProcessIDLocal = _getpid();
#elif defined __unix__
        sdv::process::TProcessID tProcessIDLocal = getpid();
#else
#error The OS currently is not supported!
#endif
        SLogEntry entryTruncated{ sdv::core::ELogSeverity::warning, "", 0, tProcessIDLocal, "", "Log truncated..." };
        m_queuePrematureLog.push(entryTruncated);
        return;
    }
    SLogEntry entry{ eSeverity, ssSrcFile, uiSrcLine, tProcessID, ssObjectName, ssMessage };
    m_queuePrematureLog.push(entry);
}

void CLoggerControl::SetLogger(sdv::core::ILogger* pLogger)
{
    std::unique_lock<std::mutex> lock(m_mtxPrematureLog);
    if (pLogger == m_pLogger) return;

    // When assigning a logger, write the stored entries to the log
#ifdef _WIN32
    sdv::process::TProcessID tProcessID = _getpid();
#elif defined __unix__
    sdv::process::TProcessID tProcessID = getpid();
#else
#error The OS currently is not supported!
#endif
    if (pLogger)
    {
        pLogger->Log(sdv::core::ELogSeverity::info, "", 0, tProcessID, "", "Begin logging...");
        while (!m_queuePrematureLog.empty())
        {
            SLogEntry& rentry = m_queuePrematureLog.front();
            pLogger->Log(rentry.eSeverity, rentry.ssSrcFile, rentry.uiSrcLine, rentry.tProcessID, rentry.ssObjectName,
                rentry.ssMessage);
            m_queuePrematureLog.pop();
        }
    }
    else // Finish the log
    {
        m_pLogger->Log(sdv::core::ELogSeverity::info, "", 0, tProcessID, "", "End logging...");
    }

    // Store the new logger pointer
    m_pLogger = pLogger;
}
