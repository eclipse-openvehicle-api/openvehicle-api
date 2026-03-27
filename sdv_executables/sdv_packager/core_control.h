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

#ifndef CORE_CONTROL_H

#include <interfaces/core.h>
#include <interfaces/config.h>
#include <interfaces/app.h>
#include <support/component_impl.h>

// Core control service implementation functions targeting sdv_packager.

// Prevent including app_control.h, module_control.h, repository.h, logger.h
#define APP_CONTROL_H
#define MODULE_CONTROL_H
#define REPOSITORY_H
#define LOGGER_H
#define LOGGER_CONTROL_H

/**
 * @brief CAppControl redefined
 */
class CAppControl : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Application control is only in configuration state.
     * @return Returns the operation state of the application.
     */
    virtual sdv::app::EAppOperationState GetOperationState() const
    {
        return sdv::app::EAppOperationState::configuring;
    }
};

/**
 * @brief Return the application control.
 * @return Reference to the application control.
 */
inline CAppControl& GetAppControl()
{
    static CAppControl app_control;
    return app_control;
}

/**
 * @brief CModuleControl redefined
 */
class CModuleControl : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()
};

/**
 * @brief Return the module control.
 * @return Reference to the module control.
 */
inline CModuleControl& GetModuleControl()
{
    static CModuleControl module_control;
    return module_control;
}

class CAppConfigFile;

/**
 * @brief CRepository redefined
 */
class CRepository : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Mock function, starting the components from the configuration.
     * @return Returns the result, which is always failed.
     */
    sdv::core::EConfigProcessResult StartFromConfig(const CAppConfigFile&, bool) { return sdv::core::EConfigProcessResult::failed; }

    /**
     * @brief Mock function, resetting the configuration baseline.
     */
    void ResetConfigBaseline() {}
};

/**
 * @brief Return the repository.
 * @return Reference to the repository.
 */
inline CRepository& GetRepository()
{
    static CRepository repository;
    return repository;
}

/**
 * @brief CLoggerControl redefined
 */
class CLoggerControl : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()
};

/**
 * @brief Return the logger control.
 * @return Reference to the logger control.
 */
inline CLoggerControl& GetLoggerControl()
{
    static CLoggerControl logger_control;
    return logger_control;
}


#endif // !defined CORE_CONTROL_H