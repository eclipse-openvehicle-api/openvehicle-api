#ifndef MOCK_H
#define MOCK_H

#define DO_NOT_INCLUDE_IN_UNIT_TEST
#include "../../../sdv_services/core/module_control.h"
#include "../../../sdv_services/core/module.h"
#include "../../../sdv_services/core/sdv_core.h"
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_services/core/app_config.h"

class CMock
{
public:
    CMock() : m_root("root")
    {
        auto ptrElement = std::make_shared<CNormalTable>("Install");
        m_root.AddElement(ptrElement);
        auto ptrValue = std::make_shared<CStringNode>("Directory", "install/test");
        ptrElement->AddElement(ptrValue);
    }
    void DestroyModuleObjects(sdv::core::TModuleID) {}
    bool IsStandaloneApplication() { return true; }
    bool IsEssentialApplication() { return false; }
    bool IsMainApplication() { return false; }
    bool IsIsolatedApplication() { return false; }
    bool IsMaintenanceApplication() { return false; }
    bool IsConsoleSilent() { return true; }
    bool IsConsoleVerbose() { return false; }
    uint32_t GetInstanceID() { return 1234u; }
    sdv::app::EAppOperationState GetOperationState() const { return sdv::app::EAppOperationState::running; }
    std::filesystem::path GetInstallDir() const { return GetExecDirectory(); }
    std::filesystem::path FindInstalledModule(const std::filesystem::path&) const { return {}; }
    std::optional<CInstallManifest::SComponent> FindInstalledComponent(const std::string&) const { return {}; }
    std::string FindInstalledModuleManifest(const std::filesystem::path&) { return {}; }
    sdv::core::TObjectID CreateObjectFromModule(sdv::core::TModuleID, const sdv::u8string&, const sdv::u8string&, const sdv::u8string&) { return 0; }
    sdv::core::TObjectID CreateObject2(const sdv::u8string&, const sdv::u8string&, const sdv::u8string&) { return 0; }
    std::string SaveConfig() { return {}; }
    void ResetConfigBaseline() {}
    sdv::core::TModuleID Load(const sdv::u8string&) { return 0; }
    sdv::core::TModuleID ContextLoad(const std::filesystem::path&, const sdv::u8string&)  { return 0; }
    bool ContextUnload(sdv::core::TModuleID, bool) { return false; }

    CNormalTable    m_root;
    bool            m_bIsMain = false;
    bool            m_bIsIsolated = false;
};

inline CMock& GetMock()
{
    static CMock mock;
    return mock;
}

#define CRepository CMock
#define CAppControl CMock
#define GetRepository GetMock
#define GetAppControl GetMock
#define GetAppConfig GetMock
#define GetModuleControl GetMock

inline std::filesystem::path GetCoreDirectory() { return "../../bin"; }

#endif // !defined MOCK_H