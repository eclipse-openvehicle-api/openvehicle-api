#include "scheduler.h"
#include <cassert>

CTaskScheduler::CTaskScheduler(size_t nMinIdle /*= 4*/, size_t nMaxBusy /*= 32*/) :
    m_nMinIdle(nMinIdle), m_nMaxBusy(nMaxBusy)
{
    // Check min/max values
    assert(nMinIdle > 0);
    if (!m_nMinIdle) m_nMinIdle = 1;
    assert(nMinIdle > 0);
    if (!m_nMaxBusy) m_nMaxBusy = 1;

    // Start the minimal required idle threads
    for (size_t n = 0; n < nMinIdle; n++)
        m_queueIdleThreads.push(
            m_lstThreads.insert(m_lstThreads.end(), std::make_shared<CThread>()));
    m_nMaxThreads = m_lstThreads.size();
}

CTaskScheduler::~CTaskScheduler()
{
    WaitForExecution();
}

bool CTaskScheduler::Schedule(std::function<void()> fnTask, hlpr::flags<EScheduleFlags> flags /*= 0*/)
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);

    // Get a thread - either from the idle queue or new.
    std::list<std::shared_ptr<CThread>>::iterator itThread = m_lstThreads.end();
    if (m_queueIdleThreads.empty())
    {
        // No thread is available. Allowed to make one?
        if (m_lstThreads.size() < m_nMaxBusy)
            itThread = m_lstThreads.insert(m_lstThreads.end(), std::make_shared<CThread>());
        if (m_lstThreads.size() > m_nMaxThreads)
            m_nMaxThreads = m_lstThreads.size();
    }
    else
    {
        itThread = m_queueIdleThreads.front();
        m_queueIdleThreads.pop();
    }

    // Is there a valid thread? The schedule the task. Otherwise queue the task.
    if (itThread != m_lstThreads.end())
    {
        lock.unlock();
        Execute(itThread, fnTask);
    }
    else
    {
        // Queuing allowed?
        if (flags & EScheduleFlags::no_queue) return false; // No scheduling possible.

        // Add the task to the queue
        if (flags & EScheduleFlags::priority)
            m_dequeTasks.push_front(fnTask);
        else
            m_dequeTasks.push_back(fnTask);
    }

    // Successful scheduled.
    return true;
}

void CTaskScheduler::WaitForExecution()
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);

    // Temporarily reduce the idle level to zero.
    size_t nMinIdleTemp = m_nMinIdle;
    m_nMinIdle = 0;

    // Wait until the thread list is empty.
    while (!m_lstThreads.empty())
    {
        // Erase all idle threads.
        while (!m_queueIdleThreads.empty())
        {
            // False positive of CppCheck: content of m_queueIdleThreads represents the iterator of m_lstThreads
            // cppcheck-suppress mismatchingContainerIterator
            m_lstThreads.erase(m_queueIdleThreads.front());
            m_queueIdleThreads.pop();
        }

        // Allow execution threads to finalize its processing
        lock.unlock();

        // Wait shortly
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Check again
        lock.lock();
    }

    // Restore the idle thread amount
    m_nMinIdle = nMinIdleTemp;
}

size_t CTaskScheduler::GetThreadCount() const
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);
    return m_lstThreads.size();
}

size_t CTaskScheduler::GetMaxThreadCount() const
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);
    return m_nMaxThreads;
}

size_t CTaskScheduler::GetBusyThreadCount() const
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);
    return m_lstThreads.size() - m_queueIdleThreads.size();
}

size_t CTaskScheduler::GetIdleThreadCount() const
{
    std::unique_lock<std::mutex> lock(m_mtxQueueAccess);
    return m_queueIdleThreads.size();
}

void CTaskScheduler::Execute(std::list<std::shared_ptr<CThread>>::iterator itThread, std::function<void()> fnTask)
{
    std::shared_ptr<CThread>& rptrThread = *itThread;

    // Call execute.
    rptrThread->Execute([this, fnTask, itThread]()
        {
            // Start with the supplied task.
            std::function<void()> fnTaskLocal = fnTask;

            // Execute tasks for as long as there are any
            do
            {
                fnTaskLocal();

                // Any other task?
                std::unique_lock<std::mutex> lock(m_mtxQueueAccess);
                if (m_dequeTasks.empty()) break;
                fnTaskLocal = std::move(m_dequeTasks.front());
                m_dequeTasks.pop_front();

            } while (true);


            // Prepare for adding this thread to the idle queue. Remove any redundant threads.
            // Attention: it is not possible to terminate this thread, since it still runs the thread func and relies on member
            // variables (which would otherwise be cleared). It is, however, possible to remove all other threads.
            std::unique_lock<std::mutex> lock2(m_mtxQueueAccess);
            while (!m_queueIdleThreads.empty() && m_queueIdleThreads.size() >= m_nMinIdle)
            {
                // False positive of CppCheck: content of m_queueIdleThreads represents the iterator of m_lstThreads
                // cppcheck-suppress mismatchingContainerIterator
                m_lstThreads.erase(m_queueIdleThreads.front());
                m_queueIdleThreads.pop();
            }

            // Add this thread to the idle queue.
            m_queueIdleThreads.push(itThread);
        });
}

CTaskScheduler::CThread::CThread()
{
    // Wait for the start
    std::unique_lock<std::mutex> lockStart(m_mtxSyncStart);
    m_thread = std::thread(&CThread::ThreadFunc, this);
    while (!m_bStarted)
        m_cvStarted.wait_for(lockStart, std::chrono::milliseconds(10));
}

CTaskScheduler::CThread::~CThread()
{
    // Lock to prevent execution still to take place. Then shutdown.
    std::unique_lock<std::mutex> lock(m_mtxSyncExecute);
    m_bShutdown = true;
    m_cvExecute.notify_all();
    lock.unlock();
    if (m_thread.joinable())
        m_thread.join();
}

void CTaskScheduler::CThread::Execute(std::function<void()> fnTask)
{
    // Assign the task and notify for execution.
    std::unique_lock<std::mutex> lock(m_mtxSyncExecute);
    m_fnTask = fnTask;
    m_cvExecute.notify_all();
    lock.unlock();
}

void CTaskScheduler::CThread::ThreadFunc()
{
    // Thread has started (needed, since the condition variable doesn't keep its state).
    m_bStarted = true;

    // Notify the thread has started
    std::unique_lock<std::mutex> lockStart(m_mtxSyncStart);
    m_cvStarted.notify_all();
    lockStart.unlock();

    // Notify the thread has executed
    std::unique_lock<std::mutex> lockExecute(m_mtxSyncExecute);
    while (!m_bShutdown)
    {
        // Wait for an execution task to take place.
        m_cvExecute.wait_for(lockExecute, std::chrono::milliseconds(10));

        // In case there is no task scheduled.
        if (!m_fnTask) continue;

        // Execute the task
        m_fnTask();

        // Task is done, clear the task function
        m_fnTask = {};
    }
}
