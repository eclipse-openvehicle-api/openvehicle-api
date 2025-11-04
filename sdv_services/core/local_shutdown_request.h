#ifndef LOCAL_SHUTDOWN_REQUEST_H
#define LOCAL_SHUTDOWN_REQUEST_H

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#elif defined __unix__
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#else
#error OS is not supported!
#endif

#include <string>
#include <filesystem>
#include "../../global/exec_dir_helper.h"

/**
 * @brief Send a exit loop request using a local signaling solution (used when running as essential or standalone application).
 * @attention There is no feedback that the shutdown was successful!
 * @param[in] uiInstanceID The instance to shut down.
 * @return Returns 'true' when the signal could be triggered, 'false' when no signal exists.
 */
inline bool RequestShutdown(uint32_t uiInstanceID = 1000u)
{
    std::string m_ssSignalName = "SDV_EXIT_LOOP_REQUEST_" + std::to_string(uiInstanceID);
#ifdef _WIN32
    HANDLE hEvent = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, m_ssSignalName.c_str());
    if (hEvent && hEvent != INVALID_HANDLE_VALUE)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
        return true;
    }
#elif defined __unix__
    sem_t* pSemaphore = sem_open(m_ssSignalName.c_str(), 0);
    if (pSemaphore && pSemaphore != SEM_FAILED)
    {
        // Since detecting whether or not the semaphore is triggered can only happen in the wait function, trigger 5 times with
        // each 1 ms apart.
        for (uint32_t uiIndex = 0; uiIndex < 5; uiIndex++)
        {
            sem_post(pSemaphore);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        sem_close(pSemaphore);
        return true;
    }
#endif

    // Obviously there is no signal available... no standalone instance is running.
    return false;
}

/**
 * @brief Initialize the signal to receive a shutdown request and check whether a request was fired.
 */
class CShutdownRequestListener
{
public:
    /**
     * @brief Default constructor
     */
    CShutdownRequestListener() = default;

    /**
     * @brief Constructor.
     * @param[in] uiInstanceID The instance ID to check for.
     */
    CShutdownRequestListener(uint32_t uiInstanceID);

    /**
     * @brief No support for copy constructor.
     * @param[in] rListener Reference to the listener to copy.
     */
    CShutdownRequestListener([[maybe_unused]] const CShutdownRequestListener& rListener) = delete;

    /**
     * @brief Move constructor
     * @param[in] rListener Reference to the listener to move.
     */
    CShutdownRequestListener(CShutdownRequestListener&& rListener) noexcept;

    /**
     * @brief Destructor
     */
    ~CShutdownRequestListener();

    /**
     * @brief Assignment operator is not supported.
     * @param[in] rListener Reference to the listener to copy.
     * @return Reference to the listener class.
     */
    CShutdownRequestListener& operator=([[maybe_unused]] const CShutdownRequestListener& rListener) = delete;

    /**
     * @brief Move operator.
     * @param[in] rListener Reference to the listener to move.
     * @return Reference to the listener class.
     */
    CShutdownRequestListener& operator=(CShutdownRequestListener&& rListener) noexcept;

    /**
     * @brief Check for validity.
     * @return Returns whether the signal was initialized correctly.
     */
    bool IsValid() const;

    /**
     * @brief Check whether a trigger occurred. To be called cyclicly.
     * @param[in] uiWaitForMs The amount of ms to wait for the shutdown request.
     * @return Returns 'true' when triggered; 'false' when not.
     */
    bool HasTriggered(uint32_t uiWaitForMs) const;

private:
    std::string             m_ssSignalName;                     ///< The signal name used for the synchronization.
#ifdef _WIN32
    HANDLE                  m_hEvent = INVALID_HANDLE_VALUE;    ///< The event handle
#elif defined __unix__
    sem_t*                  m_pSemaphore = nullptr;             ///< The semaphore
#endif
};

#ifdef _WIN32

inline CShutdownRequestListener::CShutdownRequestListener(uint32_t uiInstanceID) :
	m_ssSignalName("SDV_EXIT_LOOP_REQUEST_" + std::to_string(uiInstanceID))
{
    // Prevent multiple instances by opening the named event first (this should fail)...
    HANDLE hEvent = OpenEventA(SYNCHRONIZE, FALSE, m_ssSignalName.c_str());
    if (hEvent)
        CloseHandle(hEvent); // Another instance is already running. Do not create a new event.
    else
        m_hEvent = CreateEventA(nullptr, TRUE, FALSE, m_ssSignalName.c_str());
}

inline CShutdownRequestListener::CShutdownRequestListener(CShutdownRequestListener&& rListener) noexcept :
    m_ssSignalName(std::move(rListener.m_ssSignalName)), m_hEvent(rListener.m_hEvent)
{
    rListener.m_ssSignalName.clear();
    rListener.m_hEvent = 0;
}

inline CShutdownRequestListener::~CShutdownRequestListener()
{
    // Free the event object
    if (m_hEvent && m_hEvent != INVALID_HANDLE_VALUE)
        CloseHandle(m_hEvent);
}

inline CShutdownRequestListener& CShutdownRequestListener::operator=(CShutdownRequestListener&& rListener) noexcept
{
    m_ssSignalName = std::move(rListener.m_ssSignalName);
    m_hEvent = rListener.m_hEvent;
    rListener.m_ssSignalName.clear();
    rListener.m_hEvent = 0;
    return *this;
}

inline bool CShutdownRequestListener::IsValid() const
{
    return m_hEvent && m_hEvent != INVALID_HANDLE_VALUE;
}

inline bool CShutdownRequestListener::HasTriggered(uint32_t uiWaitForMs) const
{
    return m_hEvent && m_hEvent != INVALID_HANDLE_VALUE && WaitForSingleObject(m_hEvent, uiWaitForMs) == WAIT_OBJECT_0;
}

#elif defined __unix__

inline CShutdownRequestListener::CShutdownRequestListener(uint32_t uiInstanceID) :
    m_ssSignalName("SDV_EXIT_LOOP_REQUEST_" + std::to_string(uiInstanceID))
{
    // Create semaphore object
    sem_unlink(m_ssSignalName.c_str());
    m_pSemaphore = sem_open(m_ssSignalName.c_str(), O_CREAT | O_EXCL, 0777 /*O_RDWR*/, 0);
}

inline CShutdownRequestListener::CShutdownRequestListener(CShutdownRequestListener&& rListener) noexcept :
    m_ssSignalName(std::move(rListener.m_ssSignalName)), m_pSemaphore(rListener.m_pSemaphore)
{
    rListener.m_ssSignalName.clear();
    rListener.m_pSemaphore = 0;
}

inline CShutdownRequestListener::~CShutdownRequestListener()
{
    if (m_pSemaphore && m_pSemaphore != SEM_FAILED)
        sem_unlink(m_ssSignalName.c_str());
}

inline CShutdownRequestListener& CShutdownRequestListener::operator=(CShutdownRequestListener&& rListener) noexcept
{
    m_ssSignalName = std::move(rListener.m_ssSignalName);
    m_pSemaphore = rListener.m_pSemaphore;
    rListener.m_ssSignalName.clear();
    rListener.m_pSemaphore = nullptr;
    return *this;
}

inline bool CShutdownRequestListener::IsValid() const
{
    return m_pSemaphore && m_pSemaphore != SEM_FAILED;
}

inline bool CShutdownRequestListener::HasTriggered(uint32_t uiWaitForMs) const
{
    // Get the time from the realtime clock
    timespec sTimespec{};
    if (clock_gettime(CLOCK_REALTIME, &sTimespec) == -1)
        return false;
    uint64_t uiTimeNs = sTimespec.tv_nsec + uiWaitForMs * 1000000ull;
    sTimespec.tv_nsec = uiTimeNs % 1000000000ull;
    sTimespec.tv_sec += uiTimeNs / 1000000000ull;

    // Wait for the semaphore
    return m_pSemaphore && m_pSemaphore != SEM_FAILED && sem_timedwait(m_pSemaphore, &sTimespec) == 0;
}

#endif

#endif // !defined LOCAL_SHUTDOWN_REQUEST_H