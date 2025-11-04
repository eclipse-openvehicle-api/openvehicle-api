#ifndef MOCK_H
#define MOCK_H

#include <interfaces/app.h>
#define DO_NOT_INCLUDE_IN_UNIT_TEST
#include "../../../sdv_services/core/app_control.h"
#include "../../../sdv_services/core/sdv_core.h"

class CMock
{
public:
    //void DestroyModuleObjects(sdv::core::TModuleID) {}
    bool IsStandaloneApplication() { return true; }
    bool IsEssentialApplication() { return false; }
    bool IsMainApplication() { return false; }
    bool IsIsolatedApplication() { return false; }
    bool IsMaintenanceApplication() { return false; }
    bool IsExternalApplication() { return false; }
    bool IsConsoleSilent() { return true; }
    bool IsConsoleVerbose() { return false; }
    uint32_t GetInstanceID() { return 1000u; }
    void RequestShutdown() {}
    sdv::app::EAppOperationState GetOperationState() const { return sdv::app::EAppOperationState::running; }
    std::filesystem::path GetInstallDir() const { return std::filesystem::path(); }
    std::filesystem::path FindInstalledModule(const std::filesystem::path&) const { return {}; }
    std::optional<CInstallManifest::SComponent> FindInstalledComponent(const std::string&) const { return {}; }
    std::string FindInstalledModuleManifest(const std::filesystem::path&) { return {}; }
};

inline CMock& GetMock()
{
    static CMock mock;
    return mock;
}

#define CAppControl CMock
#define GetAppControl GetMock
#define GetAppConfig GetMock

#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include "../../../sdv_services/core/module_control.h"
#include "../../../sdv_services/core/repository.h"
#include "../../../sdv_services/core/app_config.h"

inline std::filesystem::path GetCoreDirectory() { return "../../bin"; }

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

#endif // !defined MOCK_H