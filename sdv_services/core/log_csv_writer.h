#ifndef LOG_CSV_WRITER_H
#define LOG_CSV_WRITER_H

#include <iostream>
#include <chrono>
#include <string>
#include <interfaces/log.h>
#include <mutex>

/**
 * @brief Writes CSV entries to a given stream.
 */
class CLogCSVWriter
{
public:
    /**
     * @brief Constructs a CSV writer.
     * @param[in] ostream stream the CSV writer will write to.
     * @param[in] separator sets the separator of the CSV file, default is ';'.
     */
    CLogCSVWriter(std::shared_ptr<std::ostream> ostream, const char separator = ';');

    /**
     * @brief Creates a log entry in the log.
     * @param[in] message message of the log entry.
     * @param[in] severity severity of the log entry.
     * @param[in] timestamp timestamp of the log entry.
     */
    void Write(const std::string& message, const sdv::core::ELogSeverity severity,
        const std::chrono::time_point<std::chrono::system_clock> timestamp);

    /**
     * @brief Creates a string of the given timestamp including nano seconds.
     * @param[in] timestamp Timestamp to be converted to a string.
     * @return Returns the string representing the timestamp including nano seconds.
     */
    std::string GetDateTime(const std::chrono::time_point<std::chrono::system_clock> timestamp);

    /**
     * @brief Creates string from the given severity.
     * @param[in] severity Severity to be converted to a string.
     * @return Returns the string representing the severity.
     */
    static std::string GetSeverityString(sdv::core::ELogSeverity severity);

    /**
     * @brief Exception if an invalid separator is set.
     */
    struct SForbiddenSeparatorException : public std::runtime_error
    {
        /**
         * @brief Construct a forbidden separator Exception object.
         * @param[in] message The reason for throwing this exception.
         */
        SForbiddenSeparatorException(const char* message)
            : std::runtime_error(message)
        {}
    };

private:
    static std::string Escape(const std::string& toEscape);

    std::shared_ptr<std::ostream>   m_streamOutput;     ///< Output stream
    const char                      m_cSeparator;       ///< The separator character
    mutable std::mutex              m_mtxOutput;        ///< Protect stream access
};

#endif // !defined LOG_CSV_WRITER_H