#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>
#include <queue>
#include <deque>
#include <functional>
#include <memory>
#include <condition_variable>
#include <cstdint>
#include <list>
#include "../flags.h"

/**
 * @brief Job scheduler that uses a dynamic threadpool to schedule the task.
 */
class CTaskScheduler
{
public:
    /**
     * @brief Constructor
     * @param[in] nMinIdle The amount of idle threads that should stay present if there is nothing to process at the moment.
     * @param[in] nMaxBusy The maximum amount of threads that is used for processing.
     */
    CTaskScheduler(size_t nMinIdle = 4, size_t nMaxBusy = 32);

    /**
     * @brief Destructor
     */
    ~CTaskScheduler();

    /**
     * @brief Schedule flags to influence the scheduling.
     */
    enum class EScheduleFlags
    {
        normal = 0x0,   ///< If thread level threshold has been reached, queue the call at the end of the queue.
        priority = 0x1, ///< If thread level threshold has been reached, insert the call at the begin of the queue.
        no_queue = 0x2, ///< If thread level threshold has been reached, fail the schedule call.
    };

    /**
     * @brief Schedule the asynchronous execution of the task.
     * @details If an idle thread is available, the thread will execute the task. If no thread is available and the maximum thread
     * level hasn't been reached, a new thread will be started that schedules the task. If the maximum thread level has been
     * exceeded the task will be placed in the task list based on its priority and allowance. If queue is not allowed (set by the
     * no_queue flag), the scheduling will fail.
     * @param[in] fnTask The task to schedule.
     * @param[in] flags Zero or more flags to use for scheduling the task.
     * @return Returns whether the scheduling was successful or not.
     */
    bool Schedule(std::function<void()> fnTask, hlpr::flags<EScheduleFlags> flags = EScheduleFlags::normal);

    /**
     * @brief Wait until the execution of all threads has been finalized. This will also remove all idle threads.
     * @attention Do not call from a task function - that will cause a deadlock.
     */
    void WaitForExecution();

    /**
     * @brief Get the current amount of threads (idle + processing).
     * @return The amount of threads.
     */
    size_t GetThreadCount() const;

    /**
     * @brief Get the maximum amount of threads that were processing at one time.
     * @return The amount of threads.
     */
    size_t GetMaxThreadCount() const;

    /**
     * @brief Get the current amount of processing threads.
     * @return The amount of threads.
     */
    size_t GetBusyThreadCount() const;

    /**
     * @brief Get the current amount of idle threads.
     * @return The amount of threads.
     */
    size_t GetIdleThreadCount() const;

private:
    /**
     * @brief Helper class for the thread scheduling
     */
    struct CThread
    {
    public:
        /**
         * @brief Constructor starting the thread.
         */
        CThread();

        /**
         * @brief Destructor stopping the thread.
         */
        ~CThread();

        /**
         * @brief Schedule an execution.
         * @param[in] fnTask The execution task.
         */
        void Execute(std::function<void()> fnTask);

    private:
        /**
         * @brief The thread function to execute.
         */
        void ThreadFunc();

        std::thread             m_thread;               ///< The thread that executes the tasks.
        bool                    m_bShutdown = false;    ///< Set when the thread should terminate.
        bool                    m_bStarted = false;     ///< Set when the thread has started.
        std::function<void()>   m_fnTask;               ///< The task to execute (will be updated with new tasks before execution).
        std::mutex              m_mtxSyncStart;         ///< The startup synchronization mutex.
        std::condition_variable m_cvStarted;            ///< Triggered by the thread to indicate that it has started.
        std::mutex              m_mtxSyncExecute;       ///< The execute synchronization mutex.
        std::condition_variable m_cvExecute;            ///< Triggers the thread to indicate that there is a task to execute or
                                                        ///< shutdown has been requested.
    };

    /**
     * @brief Execute a task using the provided thread. After the execution, other tasks from the task list will be executed as
     * well. If after all executions are finalized and if there are enough idle threads, removes the thread from
     * the list. Otherwise adds the thread to the idle queue.
     * @param[in] itThread The thread to use for the execution.
     * @param[in] fnTask The task to execute.
    */
    void Execute(std::list<std::shared_ptr<CThread>>::iterator itThread, std::function<void()> fnTask);

    size_t                                  m_nMinIdle = 4;         ///< The minimal required amount of idle threads
    size_t                                  m_nMaxBusy = 32;        ///< The maximum allowed amount of busy threads
    size_t                                  m_nMaxThreads = 0;      ///< The maximum amount threads at the same time.
    mutable std::mutex                      m_mtxQueueAccess;       ///< Sync access to queue, list and double-ended-queue.
    std::queue<std::list<std::shared_ptr<CThread>>::iterator> m_queueIdleThreads; ///< Idle thread queue.
    std::list<std::shared_ptr<CThread>>     m_lstThreads;           ///< List with all threads.
    std::deque<std::function<void()>>       m_dequeTasks;           ///< Double ended task queue.
};

#endif // !defined THREAD_POOL_H