#ifndef LOGGER_H
#define LOGGER_H

#include <interfaces/log.h>
#include <support/interface_ptr.h>
#include <support/component_impl.h>
#include "log_csv_writer.h"

/**
 * @brief Logger class implementation for Windows to enable logging in a logfile.
 */
class CLogger : public sdv::IInterfaceAccess, public sdv::core::ILogger, public sdv::core::ILoggerConfig
{
public:
    /**
     * @brief Default constructor
     */
    CLogger() = default;

    /**
     * @brief Destructor
     */
    ~CLogger();

    // Interface table
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::ILogger)
        SDV_INTERFACE_ENTRY(sdv::core::ILoggerConfig)
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
        /*in*/ sdv::process::TProcessID tProcessID, /*in*/ const sdv:: u8string& ssObjectName, /*in*/ const sdv::u8string& ssMessage) override;

    /**
     * @brief Initialize the logging from SDV platform abstraction. Overload of sdv::core::ILoggerConfig::SetProgramTag.
     * @details This function needs to be called before starting to log.
     * @param[in] ssTag Provided tag to create log.
     */
    virtual void SetProgramTag(/*in*/ const sdv::u8string& ssTag) override;

    /**
     * @brief Get the program tag used for logging. Overload of sdv::core::ILoggerConfig::GetProgramTag.
     * @return The string containing the program tag.
     */
    virtual sdv::u8string GetProgramTag() const override;

    /**
     * @brief Filter the log messages based on severity. Overload of sdv::core::ILoggerConfig::SetLogFilter.
     * @param[in] eSeverity The severity level to use as a lowest level for logging. Default is "info" meaning, that
     * debug and trace messages will not be logged.
     * @param[in] eViewSeverity The severity level to use as a lowest level for viewing. Default is "error" meaning, that
     * debug, trace, info, warning and error messages will not be shown in console output.
     */
    virtual void SetLogFilter(/*in*/ sdv::core::ELogSeverity eSeverity, /*in*/ sdv::core::ELogSeverity eViewSeverity) override;

    /**
     * @brief Get the current log severity filter level. Overload of sdv::core::ILoggerConfig::GetLogFilter.
     * @return The severity level of the log filter.
     */
    virtual sdv::core::ELogSeverity GetLogFilter() const override;

    /**
     * @brief Get the current view severity level. Overload of sdv::core::ILoggerConfig::GetViewFilter.
     * @return The severity level of the view filter.
     */
    virtual sdv::core::ELogSeverity GetViewFilter() const override;

private:
    /**
     * @brief Creates a string of the given timestamp without nano seconds.
     * @param[in] timestamp Timestamp to be converted to a string.
     * @return Returns the string representing the timestamp without nano seconds.
     */
    static std::string GetDateTime(std::chrono::time_point<std::chrono::system_clock> timestamp);

    std::recursive_mutex            m_mtxLogger;                                    ///< Mutex for logging
    sdv::core::ELogSeverity         m_eFilter = sdv::core::ELogSeverity::info;      ///< Severity filter for logging
    sdv::core::ELogSeverity         m_eViewFilter = sdv::core::ELogSeverity::info;  ///< Severity filter for viewing
    std::string                     m_ssProgramtag;                                 ///< Program tag to use for logging.
#ifdef _WIN32
    std::unique_ptr<CLogCSVWriter>  m_ptrWriter;                                    ///< Pointer to CSVWriter
#elif defined __unix__
    bool                            m_bLogOpen = false;                             ///< Check if the logger is opened.
#else
#error OS currently not supported!
#endif
};

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

/**
* @brief Logger service
*/
class CLoggerService : public sdv::CSdvObject
{
public:
    CLoggerService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetDefaultLogger())
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("DefaultLoggerService")
    DECLARE_OBJECT_SINGLETON()

    /**
    * @brief Get access to the repository.
    * @return Returns a reference to the one repository of this module.
    */
    static CLogger& GetDefaultLogger();
};

DEFINE_SDV_OBJECT_NO_EXPORT(CLoggerService)
#endif

#endif // !defined LOGGER_H