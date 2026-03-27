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

#ifndef CORE_H
#define CORE_H

#include <support/component_impl.h>
#include "app_control.h"
#include "app_settings.h"
#include "module_control.h"
#include "memory.h"
#include "repository.h"
#include "logger_control.h"
#include "logger.h"
#include "app_config.h"

/**
* @brief SDV core instance class containing containing the instances for the core services.
*/
class CSDVCore : public sdv::IInterfaceAccess
{
public:
    /**
    * @brief Constructor
    */
    CSDVCore();

    /**
    * @brief Destructor
    */
    ~CSDVCore();

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetAppControl())
        SDV_INTERFACE_CHAIN_MEMBER(GetAppSettings())
        SDV_INTERFACE_CHAIN_MEMBER(GetModuleControl())
        SDV_INTERFACE_CHAIN_MEMBER(GetMemoryManager())
        SDV_INTERFACE_CHAIN_MEMBER(GetRepository())
        SDV_INTERFACE_CHAIN_MEMBER(GetLoggerControl())
    END_SDV_INTERFACE_MAP()

    /**
    * @brief The one and only instance.
    * @return Reference to this class.
    */
    static CSDVCore& GetInstance();
};

/**
 * @brief Exported function for core access.
 * @return Pointer to the interface of the core library.
 */
extern "C" SDV_SYMBOL_PUBLIC sdv::IInterfaceAccess* SDVCore();

#endif // !defined CORE_H