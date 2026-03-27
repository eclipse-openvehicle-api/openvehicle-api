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
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/installation_manifest.h"

// Prevent including app_settings.h and app_control.h
#define APP_SETTINGS_H
#define APP_CONTROL_H
#define LOGGER_H
#define LOGGER_CONTROL_H

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
    uint32_t GetInstanceID() { return 1000u; }
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
    void RequestShutdown() {}
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

#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include "../../../sdv_services/core/module_control.h"
#include "../../../sdv_services/core/repository.h"
#include "../../../sdv_services/core/app_config.h"

//inline std::filesystem::path GetCoreDirectoryMock() { return "../../bin"; }
//#define GetCoreDirectory GetCoreDirectoryMock

class CHelper
{
public:
    CHelper(CModuleControl& modulectrl, CRepository& repository) { m_pModuleControl = &modulectrl; m_pRepository = &repository; }
    ~CHelper() {}

    static CModuleControl& GetModuleControl()
    {
        if (!m_pModuleControl) throw std::bad_exception();
        return *m_pModuleControl;
    }

    static CRepository& GetRepository()
    {
        if (!m_pRepository) throw std::bad_exception();
        return *m_pRepository;
    }

    inline static CModuleControl* m_pModuleControl = nullptr;
    inline static CRepository* m_pRepository = nullptr;
};

inline CModuleControl& GetModuleControl() { return CHelper::GetModuleControl(); }
inline CRepository& GetRepository() { return CHelper::GetRepository(); }
#define GetModuleControl GetModuleControl
#define GetRepository GetRepository

#endif // !defined MOCK_H