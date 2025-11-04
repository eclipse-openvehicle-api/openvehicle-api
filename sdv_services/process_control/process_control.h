/**
 * @file process_control.h
 * @author Erik Verhoeven
 * @brief
 * @version 1.0
 * @date 2024-08-16
 *
 * @copyright Copyright ZF Friedrichshafen AG (c) 2024
 *
 */

#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <interfaces/process.h>
#include <support/component_impl.h>
#include <map>
#include <set>
#include <condition_variable>

/**
 * @brief Process control service class
 */
class CProcessControl : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::process::IProcessInfo,
    public sdv::process::IProcessLifetime, public sdv::process::IProcessControl
{
public:
    /**
     * @brief Default constructor
     */
    CProcessControl() = default;

    /**
     * @brief Destructor
     */
    virtual ~CProcessControl() override;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::process::IProcessInfo)
        SDV_INTERFACE_ENTRY(sdv::process::IProcessLifetime)
        SDV_INTERFACE_CHECK_CONDITION(AllowProcessControl())
        SDV_INTERFACE_ENTRY(sdv::process::IProcessControl)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("ProcessControlService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Check for the application context mode being main or standalone.
     * @return Returns 'true' when the process control is allowed; otherwise returns 'false'.
     */
    bool AllowProcessControl() const;

    /**
     * @brief Gets the process ID of that currently in. Overload of sdv::process::IProcessInfo::GetProcessID.
     * @return Return the process ID.
     */
    sdv::process::TProcessID GetProcessID() const override;

    /**
    * @brief Register a process lifetime monitor. Overload of sdv::process::IProcessLifetime::RegisterMonitor.
    * @param[in] tProcessID Process ID to monitor the lifetime for.
    * @param[in] pMonitor Pointer to the monitor interface. The monitor should expose the IProcessLifetimeCallback
    * interface.
    * @return Returns a non-zero cookie when successful; zero when not.
    */
    virtual uint32_t RegisterMonitor(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ sdv::IInterfaceAccess* pMonitor) override;

    /**
    * @brief Unregistered a previously registered monitor. Overload of sdv::process::IProcessLifetime::UnregisterMonitor.
    * @param[in] uiCookie The cookie from the monitor registration.
    */
    virtual void UnregisterMonitor(/*in*/ uint32_t uiCookie) override;

    /**
    * @brief Wait for a process to finalize. Overload of sdv::process::IProcessLifetime::WaitForTerminate.
    * @param[in] tProcessID The process ID to wait for.
    * @param[in] uiWaitMs Maximum time to wait in ms. Could be 0xffffffff to wait indefintely.
    * @return Returns 'true' when the process was terminated (or isn't running), 'false' when still running and a timeout
    * has occurred.
    */
    virtual bool WaitForTerminate(/*in*/ sdv::process::TProcessID tProcessID, /*in*/ uint32_t uiWaitMs) override;

    /**
    * @brief Execute a process. Overload of sdv::process::IProcessControl::Execute.
    * @param[in] ssModule Module name of the process executable.
    * @param[in] seqArgs Instantiation arguments to supply to the process.
    * @param[in] eRights The process rights during instantiation.
    * @return Returns the process ID or 0 when process creation failed.
    */
    virtual sdv::process::TProcessID Execute(/*in*/ const sdv::u8string& ssModule,
        /*in*/ const sdv::sequence<sdv::u8string>& seqArgs, /*in*/ sdv::process::EProcessRights eRights) override;

    /**
    * @brief Terminate the process. Overload of sdv::process::IProcessControl::Terminate.
    * @attention Use this function as a last resort only. The process will be killed and anything unsaved will render invalid.
    * @param[in] tProcessID The process ID of the process to terminate.
    * @return Returns 'true' if termination was successful; returns 'false' if termination was not possible or not allowed.
    */
    virtual bool Terminate(/*in*/ sdv::process::TProcessID tProcessID) override;

  private:
    /**
     * @brief Monitor thread function.
     */
    void MonitorThread();

    sdv::EObjectStatus      m_eObjectStatus = sdv::EObjectStatus::initialization_pending;       ///< Object status.

      std::mutex            m_mtxProcessThreadShutdown;                                         ///< Synchronize access
#ifdef _WIN32
    std::map<sdv::process::TProcessID, std::pair<HANDLE,HANDLE>>    m_mapProcessThreadShutdown; ///< Map with process IDs and event handles
#elif __unix__
      std::set<sdv::process::TProcessID>    m_setProcessThreadShutdown;                         ///< Set with process IDs
#else
#error OS is not supported!
#endif

      /**
       * @brief Process helper structure
       */
      struct SProcessHelper
      {
          sdv::process::TProcessID  tProcessID = 0;             ///< Process ID
#ifdef _WIN32
          HANDLE                    hProcess = 0;               ///< process handle
#elif defined __unix__
          bool                      bNotAChild = false;         ///< When set, the process is not a child of the monitor process.
#else
#error OS is not supported!
#endif
          bool                      bRunning = true;            ///< Set when the process is running and not terminated yet.
          int64_t                   iRetVal = 0;                ///< Process return value.
          std::map<uint32_t, sdv::process::IProcessLifetimeCallback*> mapAssociatedMonitors;    ///< Map with associated monitors.
          std::mutex                mtxProcess;                 ///< Mutex for process access.
          std::condition_variable   cvWaitForProcess;           ///< Condition variable to wait for process termination.
      };
      mutable std::mutex            m_mtxProcesses;             ///< Access control for monitor map.
      std::map<sdv::process::TProcessID, std::shared_ptr<SProcessHelper>> m_mapProcesses;   ///< Monitor map
      uint32_t                      m_uiNextMonCookie = 1;      ///< Next monitor cookie
      std::map<uint32_t, std::shared_ptr<SProcessHelper>> m_mapMonitors;    ///< Map with monitors.
      bool                          m_bShutdown = false;        ///< Set to shutdown the monitor thread.
      std::thread                   m_threadMonitor;            ///< Monitor thread.
};
DEFINE_SDV_OBJECT(CProcessControl)

#endif // !define PROCESS_CONTROL_H
