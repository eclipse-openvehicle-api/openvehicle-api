#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <cstdint>

enum class EVerbosityMode
{
    report_none = -1,       ///< Do not report anything.
    report_errors = 0,      ///< Report errors only (default).
    report_all = 10,        ///< Report all intermediate stepts.
};

/**
 * @brief Log control class used to control the log data.
 * @remarks Logging is done to std::clog ostream.
 */
class CLogControl
{
public:
    /**
     * @brief Constructor
     */
    CLogControl();

    /**
     * @brief Set verbosity mode.
     * @param[in] eMode The verbosity mode to set.
     */
    void SetVerbosityMode(EVerbosityMode eMode);

    /**
    * @brief Get verbosity mode.
    * @return Returns the current verbosity mode.
    */
    EVerbosityMode GetVerbosityMode() const;

    /**
     * @{
     * @brief Increase and decrease indentation.
     */
    void IncreaseIndent();
    void DecreaseIndent();
    /**
     * @}
     */

    /**
     * @brief Report information; indentation will be inserted automatically after a newline.
     * @remarks Will only be shown when the verbosity mode is on report_all. Errors will be shown when the verbosity mode is on
     * report_all and report_errors.
     * @param[in] rssText Reference to the string to log.
     * @param[in] bError When set, the text indicates an error.
     */
    void Log(const std::string& rssText, bool bError = false) const;

private:
    int32_t         m_iIndent = -1;                                     ///< Indentation after newline
    mutable bool    m_bNewline = true;                                  ///< When set, start inserting an indentation
    EVerbosityMode  m_eVerbosityMode = EVerbosityMode::report_errors;   ///< The level of verbosity while reporting
};

/**
 * @brief One global instance of log control.
 */
extern CLogControl g_log_control;

/**
 * @brief Logger class responsible for reporting the progress and errors. The logger class derives from std::ostream to provide
 * stream access reporting.
 */
class CLog : public std::ostream
{
public:
    /**
     * @brief Constructor forming a test string by streaming.
     * @tparam TArgs Argument types within the parameter pack.
     * @param[in] tArgs Arguments forming the task description.
     */
    template <typename... TArgs>
    CLog(TArgs... tArgs);

    /**
     * @brief Destructor
     */
    ~CLog();

private:
    /**
     * @brief Compose a string without any arguments; doing nothing... ending the unpacking of the parameter pack of the variadic
     * template.
     * @param[in, out] rss Reference to the string stream to fill.
     */
    static void ComposeString([[maybe_unused]] std::stringstream& rss) {}

    /**
     * @brief Compose a string with one or more arguments.
     * @tparam TArg Type of the first argument.
     * @tparam TArgs Types of additional arguments
     * @param[in, out] rss Reference to the string stream to fill.
     * @param[in] tArg First argument.
     * @param[in] tAdditionalArgs Optional additional arguments as parameter pack.
     */
    template <typename TArg, typename... TArgs>
    static void ComposeString(std::stringstream& rss, TArg tArg, TArgs... tAdditionalArgs)
    {
        rss << tArg;
        ComposeString(rss, tAdditionalArgs...);
    }

    /**
     * @brief Logger string buf implementing dedicated streaming to the log using std::ostream.
     */
    class CLogStringBuf : public std::stringbuf
    {
    protected:
        /**
         * @brief Synchronize the controlled character sequence. Overload of std::stringbuf::sync.
         * @return int Returns 0 on success; -1 otherwise.
         */
        virtual int sync() override;
    };

    std::string     m_ssTask;       ///< Task description
    CLogStringBuf   m_buffer;       ///< The logger specific string buffer.
};

template <typename... TArgs>
inline CLog::CLog(TArgs... tArgs) : std::ostream(&m_buffer)
{
    std::stringstream sstreamTaskDescr;
    ComposeString(sstreamTaskDescr, tArgs...);
    m_ssTask = sstreamTaskDescr.str();

    g_log_control.IncreaseIndent();
    if (!m_ssTask.empty())
        *this << "Entering: " << m_ssTask << std::endl;
}

#endif // !defined(LOGGER_H)