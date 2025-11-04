
#ifndef LOCK_DOORS_THREAD_H
#define LOCK_DOORS_THREAD_H

/**
 * @brief Thread to lock the doors (or execute any other callback the thread gets) after a certain time. can be restarted
 */
class LockDoorsThread
{

public:

    /**
     * @brief Constructor, set the callback function
     * @param[in] callback function to be called
     * @param[in] m_running when set to false end thread
     * @param[in] m_timeIn100MilliSeconds  timespan * 100 in milliseconds before callback should be executed
     */
    LockDoorsThread(std::function<void()> callback)
                : m_callback(callback), m_running(false), m_timeIn100MilliSeconds(0) {}

    /**
     * @brief Stsrt thread
     * @param[in] timeSpan the time span before the callback will be executed
     */                
    void start(uint32_t timeSpan) 
    {
        if (m_running) 
            return;
        
        m_timeIn100MilliSeconds = timeSpan;
        m_counter = 0;
        m_running = true;
        m_thread = std::thread(&LockDoorsThread::run, this);
    }

    /**
    * @brief Stop thread
    */
    void stop() 
    {
        m_running = false;
        if (m_thread.joinable()) 
        {
            m_thread.join();
        }
    }

    /**
     * @brief Stop and restart thread with new time setting
     * @param[in] timeSpan the time span before the callback will be executed
     */
    void restart(uint32_t timeSpan)
    {
        stop();
        m_counter = 0;
        start(timeSpan);
    }

    /**
     * @brief destructor
     */    
    ~LockDoorsThread() 
    {
        stop();
    }

    private:

    /**
     * @brief thread loop. If time span is reach, execute callback and reset. Only one execution required
     */    
    void run() 
    {
        while (m_running && m_counter <= m_timeIn100MilliSeconds)
        {
            m_counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (m_counter > m_timeIn100MilliSeconds && m_callback)
        {
            m_callback();
            m_counter = 0;
        }
        m_running = false;
    }

    std::function<void()> m_callback; ///< callback function
    std::atomic<bool> m_running;      ///< status if loop is running
    std::thread m_thread;             ///< timer  thread
    uint32_t m_counter;               ///< loop counter
    uint32_t m_timeIn100MilliSeconds; ///< time before the callback will be executed
};

#endif // !define LOCK_DOORS_THREAD_H
