#ifndef PROCESS_CONTROL_IFC_H
#define PROCESS_CONTROL_IFC_H

/**
 * @brief Process control application parameters.
 * @details The operating mode is the first parameter of the application.
 */
enum class EOperatingmode : uint32_t
{
    normal_shutdown_1000ms,         ///< Proper shutdown after 1000ms; exit code = 0
    emergency_exit_1000ms,          ///< Emergency exit after 1000ms; exit code = -20
    wait_for_process,               ///< Wait for process end; then shutdown; exit code = 0 on proper shutdown, -10 on emergency exit, -20 on timeout
    terminate_process,              ///< Terminate the first process; then shutdown; exit code = 0
};

/**
 * @brief Process monitor helper class
 */
class CProcessMonitorHelper : public sdv::IInterfaceAccess, public sdv::process::IProcessLifetimeCallback
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::process::IProcessLifetimeCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Called when the process was terminated. Overload of sdv::process::IProcessLifetimeCallback::ProcessTerminated.
     * @remarks The process return value is not always valid. The validity depends on the support of the underlying system.
     * @param[in] tProcessID The process ID of the process being terminated.
     * @param[in] iRetValue Process return value or 0 when not supported.
     */
    virtual void ProcessTerminated(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ int64_t iRetValue) override
    {
        std::unique_lock<std::mutex> lock(m_mtxTerminate);
        m_tProcessID = tProcessID;
        m_iRetValue = iRetValue;
        m_bCalled = true;
        lock.unlock();
        m_cvTerminate.notify_all();
    }

    /**
     * @brief Wait for 5000ms for the process to terminate.
     * @return Returns 'true' on successful terminate; 'false' due to a timeout.
     */
    bool Wait5000ms()
    {
        std::unique_lock<std::mutex> lock(m_mtxTerminate);
        std::chrono::high_resolution_clock::time_point tpStart = std::chrono::high_resolution_clock::now();
        while (!m_bCalled && std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count() < 5.0)
        {
            m_cvTerminate.wait_for(lock, std::chrono::milliseconds(30));
        }
        std::cout << GetTimestamp() << "Process PID#" << std::dec << m_tProcessID << " has " << (m_bCalled ? "" : "not ") << "terminated." << std::endl;
        return m_bCalled;
    }

    /**
     * @brief Get the process ID (to be called after successful termination).
     * @return The process ID provided by the callback event.
     */
    sdv::process::TProcessID GetProcessID() const
    {
        std::unique_lock<std::mutex> lock(m_mtxTerminate);
        return m_tProcessID;
    }

    /**
     * @brief Get the return value (to be called after successful termination).
     * @return The return value provided by the callback event.
     */
    int64_t GetRetValue() const
    {
        std::unique_lock<std::mutex> lock(m_mtxTerminate);
        return m_iRetValue;
    }

private:
    mutable std::mutex          m_mtxTerminate;         ///< Mutex for process termination conditional variable.
    std::condition_variable     m_cvTerminate;          ///< Process termination conditional variable.
    sdv::process::TProcessID    m_tProcessID = 0;       ///< Process ID of terminated process.
    int64_t                     m_iRetValue = -100;     ///< Process return value.
    bool                        m_bCalled = false;      ///< Set when called (to prevent waiting for condition var).
};





#endif // !defined PROCESS_CONTROL_IFC_H