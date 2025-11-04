#ifndef LOGGER_CONTROL_H
#define LOGGER_CONTROL_H

#include <support/interface_ptr.h>
#include <interfaces/log.h>
#include <queue>
#include <mutex>
#include <utility>

/**
 * @brief Logger control class.
 */
class CLoggerControl : public sdv::IInterfaceAccess, public sdv::core::ILogger
{
public:
    /**
     * @brief Default constructor
    */
    CLoggerControl() = default;

    /**
     * @brief Destructor
     */
    ~CLoggerControl();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::ILogger)
    END_SDV_INTERFACE_MAP()

    /**
    * @brief Log a message to the SDV log. Overload of sdv::core::ILogger::Log.
    * @param[in] eSeverity Severity level of the log message which will be logged, e.g. info, warning, error etc.
    * @param[in] ssSrcFile The source file that caused the log entry.
    * @param[in] uiSrcLine The line number in the source file that caused the log entry.
    * @param[in] tProcessID Process ID of the process reporting this log entry.
    * @param[in] ssObjectName Name of the object if the log entry is supplied by a component.
    * @param[in] ssMessage The log message that will be logged.
    */
    virtual void Log(/*in*/ sdv::core::ELogSeverity eSeverity, /*in*/ const sdv::u8string& ssSrcFile, /*in*/ uint32_t uiSrcLine,
        /*in*/ const sdv::process::TProcessID tProcessID, /*in*/ const sdv::u8string& ssObjectName,
        /*in*/ const sdv::u8string& ssMessage) override;

    /**
     * @brief Set the logger to use for logging.
     * @param[in] pLogger
     */
    void SetLogger(sdv::core::ILogger* pLogger);

private:
    /**
     * @brief Log entry to store.
     */
    struct SLogEntry
    {
        sdv::core::ELogSeverity     eSeverity;      ///< Severity level of the log message.
        sdv::u8string               ssSrcFile;      ///< The source file that caused the log entry.
        uint32_t                    uiSrcLine;      ///< The line number in the source file that caused the log entry.
        sdv::process::TProcessID    tProcessID;     ///< Process ID of the process submitting the log entry.
        sdv::u8string               ssObjectName;   ///< Name of the object if the log entry is submitted by an object.
        sdv::u8string               ssMessage;      ///< The log message that will be logged.
    };

    mutable std::mutex          m_mtxPrematureLog;      ///< Protect the logger queue access.
    std::queue<SLogEntry>       m_queuePrematureLog;    ///< Logger queue to store messages before a logger was assigned.
    sdv::core::ILogger*         m_pLogger = nullptr;    ///< Interface for the actual logger.
};

#endif // !defined LOGGER_CONTROL_H