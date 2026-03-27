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

#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <interfaces/app.h>
#include <support/component_impl.h>
#include <support/interface_ptr.h>
#include "../../global/tracefifo/trace_fifo.h"

/**
 * @brief Application control class.
 * @details The application control class is responsible for the startup and shutdown of the system. The startup behavior can be
 * influenced through the provided startup configuration. Dependable on the configuration, the core context is either server
 * operated (for main, isolated or maintenance applications) or locally operated (for standalone, external or essential
 * application).
 * 
 * In case of a server operated startup sequence, the following startup procedure takes place:
 *  - Core services are started
 *  - Installation manifests are loaded (not for maintenance applications)
 *      - This registers the available component classes and corresponding modules
 *  - IPC and RPC services are started
 *  - Switch system to configuration mode
 *  - Installed system configurations are loaded and components are started
 *      - This starts all system, device and interface abstraction level components with corresponding configuration.
 *  - Installed user configuration will be loaded and components are started
 *      - This will start the vehicle functions (in isolated processed where applicable).
 *  - Switch system to running mode
 * 
 * In case of a locally operated startup sequence, the following startup procedure takes place:
 *  - Core services are started
 *  - Core installation manifest is loaded
 *      - This registers the available component classes and corresponding modules for core components
 *  - For external application: RPC client is started 
 *  - Switch system to configuration mode
 *  - Modules from provided configuration are loaded
 *      - This registers the available component classes and corresponding modules
 *  - Installed user configuration will be loaded and components are started
 *      - This will start the vehicle functions.
 *  - Switch system to running mode
 */
class CAppControl : public sdv::IInterfaceAccess, public sdv::app::IAppControl, public sdv::app::IAppOperation,
    public sdv::app::IAppShutdownRequest
{
public:
    /**
     * @brief Constructor
     */
    CAppControl() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::app::IAppOperation)
        SDV_INTERFACE_ENTRY(sdv::app::IAppControl)
        SDV_INTERFACE_ENTRY(sdv::app::IAppShutdownRequest)
    END_SDV_INTERFACE_MAP()

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
     * @brief Switch from running mode to the configuration mode. Overload of sdv::app::IAppOperation::SetConfigMode.
     */
    virtual void SetConfigMode() override;

    /**
     * @brief Switch from the configuration mode to the running mode. Overload of sdv::app::IAppOperation::SetRunningMode.
     */
    virtual void SetRunningMode() override;

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

private:
    /**
     * @brief Set the operation state and broadcast the state through the event.
     * @param[in] eState The new state the app control is in.
    */
    void BroadcastOperationState(sdv::app::EAppOperationState eState);

    sdv::app::EAppOperationState        m_eState = sdv::app::EAppOperationState::not_started;  ///< The current operation state.
    sdv::app::IAppEvent*                m_pEvent = nullptr;         ///< Pointer to the app event interface.
    sdv::core::TModuleID                m_tLoggerModuleID = 0;      ///< ID of the logger module.
    bool                                m_bEnableAutoSave = false;  ///< When set and when enabled in the system settings, allows
                                                                    ///< the automatic saving of the configuration.
    bool                                m_bRunLoop = false;         ///< Used to detect end of running loop function.
    std::filesystem::path               m_pathLockFile;             ///< Lock file path name.
    FILE*                               m_pLockFile = nullptr;      ///< Lock file to test for other instances.
    CTraceFifoStdBuffer                 m_fifoTraceStreamBuffer;    ///< Trace stream buffer to redirect std::log, std::out and
                                                                    ///< std::err when running as service.
    bool                                m_bAutoSaveConfig = false;  ///< System setting for automatic saving of the user configuration.
};

/**
 * @brief Return the application control.
 * @return Reference to the application control.
 */
CAppControl& GetAppControl();

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
        SDV_INTERFACE_SET_SECTION_CONDITION(EnableAppShutdownRequestAccess(), 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY_MEMBER(sdv::app::IAppShutdownRequest, GetAppControl())
        SDV_INTERFACE_DEFAULT_SECTION()
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("AppControlService")
    DECLARE_OBJECT_SINGLETON()

    /**
    * @brief When set, the application shutdown request interface access will be enabled.
    * @return Returns 'true' when the access to the application configuration is granted; otherwise returns 'false'.
    */
    bool EnableAppShutdownRequestAccess() const;
};
DEFINE_SDV_OBJECT(CAppControlService)

#endif // !defined APP_CONTROL_H
