/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <iostream>
#include <ostream>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <string>
#include <sstream>
#include <thread>
#include <queue>
#include <chrono>
#include <atomic>
#include "exec_dir_helper.h"

#ifndef ENABLE_DEBUG_LOG
/**
 * @brief Enable debug log by defining the ENABLE_DEBUG_LOG to a value other than zero.
 */
#define ENABLE_DEBUG_LOG 0
#endif

#ifndef DECOUPLED_DEBUG_LOG
/**
 * @brief When DECOUPLED is set to a value not zero, the logging is decoupled from the writing.
 * @attention Decoupling the logging from the writing might improve the accuracy of the logging, and reduce influencing the program
 * to an absolute minimum. It does, however, cause messages to be written delayed, which might lead to missing the clue when a
 * crash occurs. To log messages synchronously with the program code, set DECOUPLED_DEBUG_LOG to the value zero.
 */
#define DECOUPLED_DEBUG_LOG 1
#endif

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
        CLogger() : m_pathLogFile(GetExecDirectory() / GetExecFilename().replace_extension(".log"))
        {
#if DECOUPLED_DEBUG_LOG == 0
            StartLog();
#endif
        }

        /**
         * @brief Destructor
         */
        ~CLogger()
        {
#if DECOUPLED_DEBUG_LOG != 0
            // Shut down the logger thread
            if (m_threadLogger.joinable())
            {
                m_bShutdown = true;
                m_threadLogger.join();
            }

            // Prevent the logger mutex to be still in use.
            std::unique_lock<std::mutex> lock(m_mtxLogger);
            lock.unlock();
#else
            FinishLog();
#endif
        }

        /**
         * @brief Log an entry. The text will be indented dependable on the depth of the logging.
         * @param[in] rss Reference to the string to log.
         */
        void Log(const std::string& rss)
        {
            // Create the message structure
            SLogMsg sMsg{std::this_thread::get_id(), DepthPerThreadOperation(EDepthOperation::report_only), rss};

#if DECOUPLED_DEBUG_LOG != 0
            std::unique_lock<std::mutex> lock(m_mtxLogger);
            
            // First log entry starts logging
            if (!m_threadLogger.joinable())
            {
                m_threadLogger = std::thread(&CLogger::LogToFileThreadFunc, this);
                while (!m_threadLogger.joinable())
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Add message to the log queue
            m_queueLogger.push(sMsg);
#else
            std::ofstream fstream;
            fstream.open(m_pathLogFile, std::ios_base::out | std::ios_base::app);
            LogMsg(fstream, sMsg);
#endif
        }

        /**
         * @brief Increase the depth. This will indent the log.
         */
        void IncrDepth()
        {
            DepthPerThreadOperation(EDepthOperation::increase);
        }

        /**
         * @brief Decrease the depth. This might decrease the indentation.
         */
        void DecrDepth()
        {
            DepthPerThreadOperation(EDepthOperation::decrease);
        }
    private:
        /**
         * @brief Start the logging
         */
        void StartLog()
        {
            std::ofstream fstream;
            fstream.open(m_pathLogFile, std::ios_base::out | std::ios_base::trunc);

            // Make one line out of it... prevents a cut in the middle when multiple streams are being used in parallel.
            std::stringstream sstreamMsg;
            sstreamMsg << "PID#" << getpid() << ": Starting log of " << GetExecFilename().generic_u8string() << std::endl;
            if (fstream.is_open())
            {
                fstream << sstreamMsg.str();
                fstream.close();
            }
            std::cout << sstreamMsg.str() << std::flush;
        }

        /**
         * @brief Finish the logging
         */
        void FinishLog()
        {
            // Finish logging
            std::ofstream fstream;
            fstream.open(m_pathLogFile, std::ios_base::out | std::ios_base::app);

            // Make one line out of it... prevents a cut in the middle when multiple streams are being used in parallel.
            std::stringstream sstreamMsg;
            sstreamMsg << "PID#" << getpid() << ": Ending log of " << GetExecFilename().generic_u8string() << std::endl;
            if (fstream.is_open())
            {
                fstream << sstreamMsg.str();
                fstream.close();
            }
            std::cout << sstreamMsg.str() << std::flush;
        }

        /**
         * @brief Log structure containing thread and log information.
         */
        struct SLogMsg
        {
            std::thread::id id; ///< Thread ID
            size_t nDepth;      ///< Depth within the calls
            std::string ssMsg;  ///< Message to log
        };

        /**
         * @brief Log a message.
         * @param[in] rfstream Reference to the stream to log to.
         * @param[in] rsMsg Reference to the message to log.
         */
        void LogMsg(std::ofstream& rfstream, const SLogMsg& rsMsg)
        {
            std::string ssIndent(rsMsg.nDepth, '>');
            if (!ssIndent.empty())
                ssIndent += ' ';

            // Make one line out of it... prevents a cut in the middle when multiple streams are being used in parallel.
            std::stringstream sstreamMsg;
            sstreamMsg << "PID#" << getpid() << " THREAD#" << rsMsg.id << ": " << ssIndent << rsMsg.ssMsg << std::endl;
            if (rfstream.is_open())
            {
                rfstream << sstreamMsg.str();
                rfstream.close();
            }

            std::cout << sstreamMsg.str() << std::flush;
        }

#if DECOUPLED_DEBUG_LOG != 0
        /**
         * @brief Log to file thread function.
         */
        void LogToFileThreadFunc()
        {
            // Start logging
            StartLog();

            // Log until shutdown
            while (!m_bShutdown)
            {
                std::unique_lock<std::mutex> lock(m_mtxLogger);

                // Are there any message. If not, pause for 100 ms
                if (m_queueLogger.empty())
                {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                std::queue<SLogMsg> queueLocal = std::move(m_queueLogger);
                lock.unlock();

                // Open the file and log all messages that are in the queue
                std::ofstream fstream;
                fstream.open(m_pathLogFile, std::ios_base::out | std::ios_base::app);
                while (!queueLocal.empty())
                {
                    auto sMsg = std::move(queueLocal.front());
                    queueLocal.pop();
                    LogMsg(fstream, sMsg);
                }
            }

            // Finish logging
            FinishLog();
        }
#endif

        /**
         * @brief Depth operation selection.
         */
        enum class EDepthOperation
        {
            increase,
            decrease,
            report_only,
        };

        /**
         * @brief Do a per thread function depth operation (increase, decrease or report).
         * @param[in] eOperation The operation to do.
         * @return The current function call depth.
         */
        size_t DepthPerThreadOperation(EDepthOperation eOperation)
        {
            thread_local static size_t nDepth = 0;   ///< Depth level for indentation.
            if (eOperation == EDepthOperation::increase)
                nDepth++;
            else if (eOperation == EDepthOperation::decrease && nDepth)
                nDepth--;
            return nDepth;
        }

        std::filesystem::path       m_pathLogFile;          ///< Path to the log file.
#if DECOUPLED_DEBUG_LOG != 0
        std::mutex m_mtxLogger;                             ///< Protect against multiple log entries at the same time.
        std::thread                 m_threadLogger;         ///< Logger thread
        std::atomic_bool            m_bShutdown = false;    ///< When set, terminate the logging thread
        std::queue<SLogMsg>         m_queueLogger;          ///< Queue with messages to log.
#endif
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
            GetLogger().Log(std::string("ENTER FUNCTION: ") + rssFunc + " - FILE " + rssFile + " - LINE " + std::to_string(nLine));
            GetLogger().IncrDepth();
        }

        /**
         * @brief Destructor creating a log entry for leaving the function.
         */
        ~CFuncLogger()
        {
            GetLogger().DecrDepth();
            GetLogger().Log(std::string("LEAVE FUNCTION: ") + m_ssFunc);
        }

        /**
         * @brief Log a function checkpoint.
         * @param[in] rssFunc Reference to the function name.
         * @param[in] nLine The line number of this checkpoint.
         */
        void Checkpoint(const std::string& rssFunc, size_t nLine)
        {
            GetLogger().Log("FUNCTION: " + rssFunc + " - LINE " + std::to_string(nLine) + " - CHECKPOINT #" + std::to_string(m_nCounter++));
        }

        /**
         * @brief Log a string.
         * @param[in] nLine The line number of this checkpoint.
         * @param[in] rss Reference to the string to log.
        */
        void Log(size_t nLine, const std::string& rss) const
        {
            GetLogger().Log("LINE " + std::to_string(nLine) + " - MSG: " + rss);
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

/**
 * @brief Macro to set a checkpoint providing the line number from the compiler.
 */
#define CHECKPOINT() logger.Checkpoint(__FUNCSIG__, __LINE__)

#else
/**
* @brief Macro to create a function logger object using the function name, the file name and the line number from the compiler.
*/
#define FUNC_LOGGER() debug::CFuncLogger logger(__PRETTY_FUNCTION__, __FILE__, __LINE__)

/**
 * @brief Macro to set a checkpoint providing the line number from the compiler.
 */
#define CHECKPOINT() logger.Checkpoint(__PRETTY_FUNCTION__, __LINE__)

#endif

/**
 * @brief Log a message
 */
#define FUNC_LOG(msg) logger.Log(__LINE__, msg)

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