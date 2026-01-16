#ifndef IPC_NAMED_MUTEX_H
#define IPC_NAMED_MUTEX_H

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif
//#include <WinSock2.h>
#include <Windows.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#elif defined __unix__
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#else
#error OS is not supported!
#endif

#include <string>
#include <cstdlib>

/**
 * @brief Inter-process-communication namespace
 */
namespace ipc
{
    /**
     * @brief Named mutex allowing interprocess synchronization. This mutex class can be used with the standard C++ library.
     */
    class named_mutex
    {
    public:
#ifdef _WIN32
        /// Native handle type.
        using native_handle_type = HANDLE;
#elif defined __unix__
        /// Native handle type.
        using native_handle_type = sem_t*;
#else
#error OS is not supported!
#endif

        /**
         * @brief Named mutex constructor. Opens or creates a named mutex. This mutex is unlocked.
         * @param[in] szName Pointer to the zero terminated name of the mutex. Can be null to automatically generate a name.
         */
        named_mutex(const char* szName = nullptr) noexcept;

        /**
         * @brief Copy constructor is deleted.
         */
        named_mutex(const named_mutex&) = delete;

        /**
         * @brief Move constructor.
         * @param[in] rmtx Reference to the mutex to move from.
         */
        named_mutex(named_mutex&& rmtx) noexcept;

        /**
         * @brief Destructor
         */
        ~named_mutex();

        /**
         * @brief Assignment operator is deleted.
         * @return Reference to this class.
         */
        named_mutex& operator=(const named_mutex&) = delete;

        /**
         * @brief Move operator.
         * @param[in] rmtx Reference to the mutex to move from.
         * @return Reference to this class.
         */
        named_mutex& operator=(named_mutex&& rmtx) noexcept;

        /**
         * @brief Locks the mutex, blocks if the mutex is not available.
         */
        void lock();

        /**
         * @brief Tries to lock the mutex, returns if the mutex is not available.
         * @return Returns 'true' when the lock was successful; 'false' when not.
         */
        bool try_lock();

        /**
         * @brief Unlocks the mutex.
         */
        void unlock();

        /**
         * @brief Return the mutex name.
         * @return Reference to the name of the mutex.
         */
        const std::string& name() const;

        /**
         * @brief Return the underlying implementation-defined native handle object.
         * @return Handle to the mutex.
         */
        native_handle_type native_handle();

    private:
        std::string         m_ssName;           ///< Name of the mutex.
        native_handle_type  m_handle;           ///< Mutex handle.
    };

#ifdef _WIN32
    inline named_mutex::named_mutex(const char* szName) noexcept : m_handle(nullptr)
    {
        if (szName)
            m_ssName = szName;
        else
        {
            std::srand(static_cast<unsigned>(std::time(nullptr))); // Use current time as seed for random generator
            uint32_t uiRand = 0;
            while (!uiRand) uiRand = std::rand();
            m_ssName = std::string("NAMED_MUTEX_") + std::to_string(uiRand);
        }
        m_handle = CreateMutexA(nullptr, FALSE, (std::string("Global\\") + m_ssName).c_str());
    }

    inline named_mutex::named_mutex(named_mutex&& rmtx) noexcept : m_handle(rmtx.m_handle)
    {
        rmtx.m_handle = nullptr;
    }

    inline named_mutex::~named_mutex()
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_handle);
    }

    // False positive warning of CppCheck: m_handle is not assigned a value. This is not the case. Suppress the warning.
    // cppcheck-suppress operatorEqVarError
    inline named_mutex& named_mutex::operator=(named_mutex&& rmtx) noexcept
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_handle);
        m_handle = rmtx.m_handle;
        rmtx.m_handle = nullptr;
        return *this;
    }

    inline void named_mutex::lock()
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
            WaitForSingleObject(m_handle, INFINITE);
    }

    inline bool named_mutex::try_lock()
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
            return WaitForSingleObject(m_handle, 0) == WAIT_OBJECT_0;
        else
            return false;
    }

    inline void named_mutex::unlock()
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
            ReleaseMutex(m_handle);
    }

    inline const std::string& named_mutex::name() const
    {
        return m_ssName;
    }

    inline named_mutex::native_handle_type named_mutex::native_handle()
    {
        return m_handle;
    }

#elif defined __unix__

    inline named_mutex::named_mutex(const char* szName) noexcept : m_handle(nullptr)
    {
        if (szName)
            m_ssName = szName;
        else
        {
            std::srand(static_cast<unsigned>(std::time(nullptr))); // Use current time as seed for random generator
            uint32_t uiRand = 0;
            while (!uiRand) uiRand = std::rand();
            m_ssName = std::string("NAMED_MUTEX_") + std::to_string(uiRand);
        }
        m_handle = sem_open(m_ssName.c_str(), O_CREAT, 0777 /*O_RDWR*/, 1);
    }

    inline named_mutex::named_mutex(named_mutex&& rmtx) noexcept : m_handle(rmtx.m_handle)
    {
        rmtx.m_handle = nullptr;
    }

    inline named_mutex::~named_mutex()
    {
        if (m_handle)
            sem_close(m_handle);
    }

    // False positive warning of CppCheck: m_handle is not assigned a value. This is not the case. Suppress the warning.
    // cppcheck-suppress operatorEqVarError
    inline named_mutex& named_mutex::operator=(named_mutex&& rmtx) noexcept
    {
        if (m_handle)
            sem_close(m_handle);
        m_handle = rmtx.m_handle;
        rmtx.m_handle = nullptr;
        return *this;
    }

    inline void named_mutex::lock()
    {
        if (m_handle)
            sem_wait(m_handle);
    }

    inline bool named_mutex::try_lock()
    {
        return m_handle ? sem_trywait(m_handle) >= 0 : false;
    }

    inline void named_mutex::unlock()
    {
        if (m_handle)
            sem_post(m_handle);
    }

    inline const std::string& named_mutex::name() const
    {
        return m_ssName;
    }

    inline named_mutex::native_handle_type named_mutex::native_handle()
    {
        return m_handle;
    }
#else
#error OS is not supported!
#endif
}

#endif // !defined IPC_NAMED_MUTEX_H