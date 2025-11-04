#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <iostream>
#include <ostream>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <string>
#include <sstream>
#include "exec_dir_helper.h"

/**
 * @brief Enable debug log by defining it and assigning it the value 1.
 */
#define ENABLE_DEBUG_LOG 1

/**
 * @brief Debug namespace
 */
namespace debug
{
    /**
     * @brief Logger class, creating a log file at the location of the executable and using the same filename as the executable
     * with the .log extension.
     */
    class CLogger
    {
    public:
        /**
         * @brief Constructor, creating the file. If the file exists, it will be overwritten.
         */
        CLogger()
        {
            m_pathLogFile = GetExecDirectory() / GetExecFilename().replace_extension(".log");
            std::unique_lock<std::mutex> lock(m_mtxLogger);
            std::ofstream fstream(m_pathLogFile, std::ios_base::out | std::ios_base::trunc);
            if (fstream.is_open())
            {
                fstream << "Starting log of " << GetExecFilename().generic_u8string() << std::endl;
                fstream.close();
            }
            std::clog << "Starting log of " << GetExecFilename().generic_u8string() << std::flush << std::endl;
        }

        /**
         * @brief Destructor
         */
        ~CLogger()
        {
            std::unique_lock<std::mutex> lock(m_mtxLogger);
            std::ofstream fstream(m_pathLogFile, std::ios_base::out | std::ios_base::app);
            if (fstream.is_open())
            {
                fstream << "End of log..." << std::endl;
                fstream.close();
            }
            std::clog << "End of log..." << std::flush << std::endl;
        }

        /**
         * @brief Log an entry. The text will be indented dependable on the depth of the logging.
         * @param[in] rss Reference to the string to log.
         */
        void Log(const std::string& rss)
        {
            std::unique_lock<std::mutex> lock(m_mtxLogger);
            std::ofstream fstream(m_pathLogFile, std::ios_base::out | std::ios_base::app);
            std::string ssIndent(m_nDepth, '>');
            if (!ssIndent.empty()) ssIndent += ' ';
            if (fstream.is_open())
            {
                fstream << ssIndent << rss << std::endl;
                fstream.close();
            }
            std::clog << ssIndent << rss << std::flush << std::endl;
        }

        /**
         * @brief Increase the depth. This will indent the log.
         */
        void IncrDepth()
        {
            m_nDepth++;
        }

        /**
         * @brief Decrease the depth. This might decrease the indentation.
         */
        void DecrDepth()
        {
            if (m_nDepth)
                m_nDepth--;
        }
    private:
        std::filesystem::path   m_pathLogFile;  ///< Path to the log file.
        std::mutex              m_mtxLogger;    ///< Protect against multiple log entries at the same time.
        size_t                  m_nDepth;       ///< Depth level for indentation.
    };

    /**
     * @brief Global logger function. A call to this function will cause an instance to be available.
     * @return Reference to the logger.
    */
    inline CLogger& GetLogger()
    {
        static CLogger logger;
        return logger;
    }

    /**
     * @brief Function logger class logging the entering and leaving of the function if places at the beginning of the function.
     */
    class CFuncLogger
    {
    public:
        /**
         * @brief Constructor creating a log entry for entering the function
         * @param[in] rssFunc Reference to the function name.
         * @param[in] rssFile Reference to the source file the function is implemented in.
         * @param[in] nLine Reference to the line the function logger object is created.
        */
        CFuncLogger(const std::string& rssFunc, const std::string& rssFile, size_t nLine) : m_ssFunc(rssFunc)
        {
            GetLogger().Log(std::string("Enter function:") + rssFunc + " - file " + rssFile + " - line " + std::to_string(nLine));
            GetLogger().IncrDepth();
        }

        /**
         * @brief Destructor creating a log entry for leaving the function.
         */
        ~CFuncLogger()
        {
            GetLogger().DecrDepth();
            GetLogger().Log(std::string("Leave function:") + m_ssFunc);
        }

        /**
         * @brief Log a function checkpoint.
         * @param[in] nLine The line number of this checkpoint.
         */
        void Checkpoint(size_t nLine)
        {
            GetLogger().Log(std::string("Checkpoint #") + std::to_string(m_nCounter++) + " - line " + std::to_string(nLine));
        }

        /**
         * @brief Log a string.
         * @param[in] rss Reference to the string to log.
        */
        void Log(const std::string& rss) const
        {
            GetLogger().Log(rss);
        }

    private:
        std::string     m_ssFunc;           ///< Name of the function.
        size_t          m_nCounter = 1;     ///< Checkpoint counter.
    };

} // namespace debug

#if ENABLE_DEBUG_LOG == 1

#ifdef _MSC_VER
/**
 * @brief Macro to create a function logger object using the function name, the file name and the line number from the compiler.
 */
#define FUNC_LOGGER() debug::CFuncLogger logger(__FUNCSIG__, __FILE__, __LINE__)
#else
/**
* @brief Macro to create a function logger object using the function name, the file name and the line number from the compiler.
*/
#define FUNC_LOGGER() debug::CFuncLogger logger(__PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

/**
 * @brief Macro to set a checkpoint providing the line number from the compiler.
 */
#define CHECKPOINT() logger.Checkpoint(__LINE__)

/**
 * @brief Log a message
 */
#define FUNC_LOG(msg) logger.Log(msg)

#else
/**
 * @brief Macro to create a function logger object using the function name, the file name and the line number from the compiler
 * (disabled).
 */
#define FUNC_LOGGER()

/**
 * @brief Macro to set a checkpoint providing the line number from the compiler (disabled).
 */
#define CHECKPOINT()

 /**
 * @brief Log a message (disabled)
 */
#define FUNC_LOG(msg)

#endif


#endif // !defined DEBUG_LOG_H