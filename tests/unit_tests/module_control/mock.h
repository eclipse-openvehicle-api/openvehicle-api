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

#ifndef MOCK_H
#define MOCK_H

#include <interfaces/app.h>
#include <support/component_impl.h>
#include "../../../sdv_services/core/installation_manifest.h"
#include "../../../global/exec_dir_helper.h"

// Prevent including app_settings.h and app_control.h
#define APP_SETTINGS_H
#define APP_CONTROL_H
#define REPOSITORY_H
#define LOGGER_H
#define LOGER_CONTROL_H

/**
 * @brief CAppSettings redefined
 */
class CAppSettings : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    // CAppSettings mocked functions
    sdv::app::EAppContext GetContextType() const { return sdv::app::EAppContext::standalone; }
    bool IsStandaloneApplication() { return true; }
    bool IsEssentialApplication() { return false; }
    bool IsMainApplication() { return false; }
    bool IsIsolatedApplication() { return false; }
    bool IsMaintenanceApplication() { return false; }
    bool IsExternalApplication() const { return false; }
    bool IsConsoleSilent() { return true; }
    bool IsConsoleVerbose() { return false; }
    uint32_t GetInstanceID() { return 1234u; }
    std::filesystem::path GetRootDir() const { return GetExecDirectory(); }
    std::filesystem::path GetInstallDir() const { return GetExecDirectory(); }
    std::vector<std::filesystem::path> GetSystemConfigPaths() const { return {}; }
    std::filesystem::path GetUserConfigPath() const { return {}; }
};

/**
 * @brief Return the application settings class.
 * @return Reference to the application settings.
 */
inline CAppSettings& GetAppSettings()
{
    static CAppSettings app_settings;
    return app_settings;
}

/**
 * @brief CAppControl redefined
 */
class CAppControl : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    // CAppControl mocked functions
    sdv::app::EAppOperationState GetOperationState() const { return sdv::app::EAppOperationState::running; }
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

class CAppConfigFile;

/**
 * @brief CRepository redefined
 */
class CRepository : public sdv::IInterfaceAccess
{
public:
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    // CRepository mocked functions
    void DestroyModuleObjects(sdv::core::TModuleID /*tModuleID*/) {}
    sdv::core::EConfigProcessResult StartFromConfig(const CAppConfigFile&, bool) { return sdv::core::EConfigProcessResult::failed; }
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

#include "../../../sdv_services/core/module_control.h"
#include "../../../sdv_services/core/module.h"
#include "../../../sdv_services/core/app_settings.h"
#include "../../../sdv_services/core/app_config.h"

//inline std::filesystem::path GetCoreDirectoryMock() { return "../../bin"; }
//#define GetCoreDirectory GetCoreDirectoryMock

#endif // !defined MOCK_H