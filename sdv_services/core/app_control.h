#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <interfaces/app.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>
#include "../../global/tracefifo/trace_fifo.h"

///
/// @brief Application control class.
/// @details The application control class is responsible for the startup and shutdown of the system. Since the system is (re-)used
/// in many applications, the startup behavior can be determined by the provided configuration as string as argument to the
/// startup function.
/// The configuration uses the TOML format and is defined as follows:
/// @code
/// # Optional use of customized log handler
/// [LogHandler]
/// Class = ""      # Component class name of a custom logger (optional)
/// Path = ""       # Component module path of a custom logger (optional)
/// Tag = ""        # Program tag to use instead of the name SDV_LOG_<pid>
/// Filter = ""     # Lowest severity filter to use when logging (Trace, Debug, Info, Warning, Error, Fatal). Default severity
///                 # level filter is Info (meaning Debug and Trace messages are not being stored).
/// ViewFilter = "" # Lowest severity filter to use when logging (Trace, Debug, Info, Warning, Error, Fatal). Default severity
///                 # level filter is Error (meaning Debug, Trace, Info and Warning messages are not being shown).
///
/// # Application behavior definition
/// # Mode = "Standalone"  (default) app->no RPC + core services + additional configurations allowed
/// # Mode = "External"    app->RPC client only + local services + target service(s) --> connection information through listener
/// # Mode = "Isolated"    app->RPC client only + local services + target service(s) --> connection information needed
/// # Mode = "Main"        app->RPC server + core services --> access key needed
/// # Mode = "Essential"   app->local services + additional configurations allowed
/// # Mode = "Maintenance" app->RPC client only + local services + maintenance service --> connection information needed + access key
/// # Instance = 1234
/// [Application]
/// Mode = "Main"
/// Instance = 1234         # Optional instance ID to be used with main and isolated applications. Has no influence on other
///                         # applications. Default instance ID is 1000. The connection listener is using the instance ID to allow
///                         # connections from an external application to the main application. Furthermore, the instance ID is
///                         # used to locate the installation of SDV components. The location of the SDV components is relative to
///                         # the executable (unless a target directory is supplied) added with the instance and the installations:
///                         # &lt;exe_path&gt;/&lt;instance&gt;/&lt;installation&gt;
/// InstallDir = "./test"   # Optional custom installation directory to be used with main and isolated applications. Has no
///                         # influence on other applications. The default location for installations is the location of the
///                         # executable. Specifying a different directory will change the location of installations to
///                         # &lt;install_directory&gt;/&lt;instance&gt;/&lt;installation&gt;
///                         # NOTE The directory of the core library and the directory of the running executable are always added
///                         # to the system if they contain an installation manifest.
///
/// # Optional configuration that should be loaded (not for maintenance and isolated applications). This overrides the application
/// # config from the settings (only main application). Automatic saving the configuration is not supported.
/// Config = "abc.toml"
///
/// #Console output
/// [Console]
/// Report = "Silent"       # Either "Silent", "Normal" or "Verbose" for no, normal or extensive messages.
///
/// # Search directories
/// @endcode
///
/// TODO: Add config ignore list (e.g. platform.toml, vehicle_ifc.toml and vehicle_abstract.toml).
///       Add dedicated config (rather than standard config) as startup param.
///
class CAppControl : public sdv::IInterfaceAccess, public sdv::app::IAppContext, public sdv::app::IAppControl,
    public sdv::app::IAppOperation, public sdv::app::IAppShutdownRequest, public sdv::IAttributes
{
public:
    /**
     * @brief Constructor
     */
    CAppControl();

    /**
     * @brief Destructor
     */
    ~CAppControl();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::app::IAppOperation)
        SDV_INTERFACE_ENTRY(sdv::app::IAppContext)
        SDV_INTERFACE_ENTRY(sdv::app::IAppControl)
        SDV_INTERFACE_ENTRY(sdv::app::IAppShutdownRequest)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Return whether the current application is the main application.
     * @return Returns 'true' when the current application is the main application; otherwise returns 'false'.
     */
    bool IsMainApplication() const;

    /**
     * @brief Return whether the current application is an isolated application.
     * @return Returns 'true' when the current application is an isolated application; otherwise returns 'false'.
     */
    bool IsIsolatedApplication() const;

    /**
     * @brief Return whether the current application is a standalone application.
     * @return Returns 'true' when the current application is a standalone application; otherwise returns 'false'.
     */
    bool IsStandaloneApplication() const;

    /**
     * @brief Return whether the current application is an essential application.
     * @return Returns 'true' when the current application is an essential application; otherwise returns 'false'.
     */
    bool IsEssentialApplication() const;

    /**
     * @brief Return whether the current application is a maintenance application.
     * @return Returns 'true' when the current application is a maintenance application; otherwise returns 'false'.
     */
    bool IsMaintenanceApplication() const;

    /**
     * @brief Return whether the current application is an external application.
     * @return Returns 'true' when the current application is an external application; otherwise returns 'false'.
     */
    bool IsExternalApplication() const;

    /**
    * @brief Return the application context mode. Overload of sdv::app::IAppContext::GetContextType.
    * @return The context mode.
    */
    sdv::app::EAppContext GetContextType() const override;

    /**
    * @brief Return the core instance ID. Overload of sdv::app::IAppContext::GetContextType.
    * @return The instance ID.
    */
    uint32_t GetInstanceID() const override;

    /**
    * @brief Return the number of retries to establish a connection.
    * @return Number of retries.
    */            
    uint32_t GetRetries() const override;

    /**
     * @brief Start the application. Overload of sdv::app::IAppControl::Startup.
     * @details The core will prepare for an application start based on the provided configuration. Per default, the
     * application will be in running mode after successful startup.
     * @param[in] ssConfig String containing the configuration to be used by the core during startup. This configuration
     * is optional. If not provided the application runs as standalone application without any RPC support.
     * @param[in] pEventHandler Pointer to the event handler receiving application events. For the handler to receive
     * events, the handler needs to expose the IAppEvent interface. This pointer is optionally and can be NULL.
     * @return Returns 'true' on successful start-up and 'false' on failed startup.
     */
    virtual bool Startup(/*in*/ const sdv::u8string& ssConfig, /*in*/ IInterfaceAccess* pEventHandler) override;

    /**
     * @brief Running loop until shutdown request is triggered.
     */
    virtual void RunLoop() override;

    /**
     * @brief Initiate a shutdown of the application. Overload of sdv::app::IAppControl::Shutdown.
     * @details The objects will be called to shutdown allowing them to clean up and gracefully shutdown. If, for some reason, the
     * object cannot shut down (e.g. pointers are still in use or threads are not finalized), the object will be kept alive and the
     * application state will stay in shutting-down-state. In that case the exception is called. A new call to the shutdown function
     * using the force-flag might force a shutdown. Alternatively the application can wait until the application state changes to
     * not-started.
     * @remarks Application shutdown is only possible when all components are released.
     * @param[in] bForce When set, forces an application shutdown. This might result in loss of data and should only be used as a
     * last resort.
     */
    virtual void Shutdown(/*in*/ bool bForce) override;

    /**
     * @brief Request shutdown. Overload of sdv::app::IAppShutdownRequest::RequestShutdown.
     */
    virtual void RequestShutdown() override;

    /**
     * @brief Get the current operation state. This information is also supplied through the event handler function. Overload of
     * sdv::app::IAppOperation::GetOperationState.
     * @return Returns the operation state of the application.
     */
    virtual sdv::app::EAppOperationState GetOperationState() const override;

    /**
     * @brief Get the current running instance.
     * @details Get the instance. If not otherwise specified, the current instance depends on whether the application is running
     * as main or isolated application, in which case the instance is 1000. In all other cases the instance is 0. A dedicated
     * instance can be supplied through the app control.
     * @return The instance number.
    */
    uint32_t GetInstance() const;

    /**
     * @brief Switch from running mode to the configuration mode. Overload of sdv::app::IAppOperation::SetConfigMode.
     */
    virtual void SetConfigMode() override;

    /**
     * @brief Switch from the configuration mode to the running mode. Overload of sdv::app::IAppOperation::SetRunningMode.
     */
    virtual void SetRunningMode() override;

    /**
     * @brief Get a sequence with the available attribute names. Overload of sdv::IAttributes::GetNames.
     * @return The sequence of attribute names.
     */
    virtual sdv::sequence<sdv::u8string> GetNames() const override;

    /**
     * @brief  Get the attribute value. Overload of sdv::IAttributes::Get.
     * @param[in] ssAttribute Name of the attribute.
     * @return The attribute value or an empty any-value if the attribute wasn't found or didn't have a value.
     */
    virtual sdv::any_t Get(/*in*/ const sdv::u8string& ssAttribute) const override;

    /**
     * @brief Set the attribute value. Overload of sdv::IAttributes::Set.
     * @param[in] ssAttribute Name of the attribute.
     * @param[in] anyAttribute Attribute value to set.
     * @return Returns 'true' when setting the attribute was successful or 'false' when the attribute was not found or the
     * attribute is read-only or another error occurred.
     */
    virtual bool Set(/*in*/ const sdv::u8string& ssAttribute, /*in*/ sdv::any_t anyAttribute) override;

    /**
     * @brief Get the attribute flags belonging to a certain attribute. Overload of sdv::IAttributes::GetFlags.
     * @param[in] ssAttribute Name of the attribute.
     * @return Returns the attribute flags (zero or more EAttributeFlags flags) or 0 when the attribute could not be found.
     */
    virtual uint32_t GetFlags(/*in*/ const sdv::u8string& ssAttribute) const override;

    /**
     * @brief Get the installation directory of user components.
     * @return The location of the user components. Only is valid when used in main and isolated applications.
     */
    std::filesystem::path GetInstallDir() const;

    /**
     * @brief Disable the current auto update feature if enabled in the system settings.
     */
    void DisableAutoConfigUpdate();

    /**
     * @brief Enable the current auto update feature if enabled in the system settings.
     */
    void EnableAutoConfigUpdate();

    /**
     * @brief Trigger the config update if enabled in the system settings.
     */
    void TriggerConfigUpdate();

    /**
     * @brief Should the console output be silent?
     * @return Returns whether the console output is silent.
     */
    bool IsConsoleSilent() const;

    /**
     * @brief Should the console output be verbose?
     * @return Returns whether the verbose console output is activated.
     */
    bool IsConsoleVerbose() const;

private:
    /**
     * @brief Set the operation state and broadcast the state through the event.
     * @param[in] eState The new state the app control is in.
    */
    void BroadcastOperationState(sdv::app::EAppOperationState eState);

    /**
     * @brief Process the application configuration before starting the system
     * @param[in] rssConfig Reference to the configuration content.
     * @return Returns 'true' when processing was successful; false when not.
    */
    bool ProcessAppConfig(const sdv::u8string& rssConfig);

    sdv::app::EAppContext               m_eContextMode = sdv::app::EAppContext::no_context;    ///< The application is running as...
    sdv::app::EAppOperationState        m_eState = sdv::app::EAppOperationState::not_started;  ///< The current operation state.
    sdv::app::IAppEvent*                m_pEvent = nullptr;         ///< Pointer to the app event interface.
    std::string                         m_ssLoggerClass;            ///< Class name of a logger service.
    sdv::core::TModuleID                m_tLoggerModuleID = 0;      ///< ID of the logger module.
    std::filesystem::path               m_pathLoggerModule;         ///< Module name of a custom logger.
    std::string                         m_ssProgramTag;             ///< Program tag to use when logging.
    sdv::core::ELogSeverity             m_eSeverityFilter = sdv::core::ELogSeverity::info;      ///< Severity level filter while logging.
    sdv::core::ELogSeverity             m_eSeverityViewFilter = sdv::core::ELogSeverity::error; ///< Severity level filter while logging.
    uint32_t                            m_uiRetries = 0u;           ///< Number of retries to establish a connection.
    uint32_t                            m_uiInstanceID = 0u;        ///< Instance number.
    std::filesystem::path               m_pathInstallDir;           ///< Location of user component installations.
    std::filesystem::path               m_pathRootDir;              ///< Location of user component root directory.
    std::vector<std::filesystem::path>  m_vecSysConfigs;            ///< The system configurations from the settings file.
    std::filesystem::path               m_pathAppConfig;            ///< The application configuration from the settings file.
    bool                                m_bAutoSaveConfig = false;  ///< System setting for automatic saving of the configuration.
    bool                                m_bEnableAutoSave = false;  ///< When set and when enabled in the system settings, allows
                                                                    ///< the automatic saving of the configuration.
    bool                                m_bRunLoop = false;         ///< Used to detect end of running loop function.
    bool                                m_bSilent = false;          ///< When set, no console reporting takes place.
    bool                                m_bVerbose = false;         ///< When set, extensive console reporting takes place.
    std::filesystem::path               m_pathLockFile;             ///< Lock file path name.
    FILE*                               m_pLockFile = nullptr;      ///< Lock file to test for other instances.
    CTraceFifoStdBuffer                 m_fifoTraceStreamBuffer;    ///< Trace stream buffer to redirect std::log, std::out and
                                                                    ///< std::err when running as service.
};

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

/**
* @brief App config service class.
*/
class CAppControlService : public sdv::CSdvObject
{
public:
    CAppControlService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY_MEMBER(sdv::app::IAppOperation, GetAppControl())
        SDV_INTERFACE_ENTRY_MEMBER(sdv::IAttributes, GetAppControl())
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableAppShutdownRequestAccess(), 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::app::IAppShutdownRequest, GetAppControl())
        SDV_INTERFACE_DEFAULT_SECTION()
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("AppControlService")
    DECLARE_OBJECT_SINGLETON()

    /**
    * @brief Get access to the application control.
    * @return Returns the one global instance of the application config.
    */
    static CAppControl& GetAppControl();

    /**
    * @brief When set, the application shutdown request interface access will be enabled.
    * @return Returns 'true' when the access to the application configuration is granted; otherwise returns 'false'.
    */
    bool EnableAppShutdownRequestAccess() const;
};
DEFINE_SDV_OBJECT_NO_EXPORT(CAppControlService)

#endif

#endif // !defined APP_CONTROL_H
