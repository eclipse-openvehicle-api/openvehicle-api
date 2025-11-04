#include "module_control.h"
#include "sdv_core.h"
#include <algorithm>
#include "../../global/exec_dir_helper.h"
#include "toml_parser/parser_toml.h"
#include "toml_parser//parser_node_toml.h"

/// @cond DOXYGEN_IGNORE
#ifdef _WIN32
#include <Shlobj.h>
#elif defined __unix__
#include <ctime>
#include <utime.h>
#else
#error OS is not supported!
#endif
/// @endcond

CModuleControl::CModuleControl()
{}

CModuleControl::~CModuleControl()
{
    try
    {
        UnloadAll(std::vector<sdv::core::TModuleID>());
    }
    catch (const sdv::XSysExcept&)
    {
    }
    catch (const std::exception&)
    {
    }

}

bool CModuleControl::AddModuleSearchDir(const sdv::u8string& ssDir)
{
    // Add initial paths if not done so already
    AddCurrentPath();

    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);

    std::filesystem::path pathDir(ssDir.c_str());

    // Relative paths are always relative to the executable
    if (pathDir.is_relative())
        pathDir = GetExecDirectory() / ssDir.c_str();

    // Remove any indirections
    pathDir = pathDir.lexically_normal();

    // If the current path is not a directory, it cannot be added
    if (!std::filesystem::is_directory(pathDir)) return false;

    // Check whether the path is already in the list
    if (std::find(m_lstSearchPaths.begin(), m_lstSearchPaths.end(), pathDir) != m_lstSearchPaths.end())
        return true; // This is not an error

    // Add the path
    m_lstSearchPaths.push_back(pathDir);

    return true;
}

sdv::sequence<sdv::u8string> CModuleControl::GetModuleSearchDirs() const
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    sdv::sequence<sdv::u8string> seqSearchDirs;
    for (const std::filesystem::path& rpathSearchDir : m_lstSearchPaths)
        seqSearchDirs.push_back(rpathSearchDir.generic_u8string());
    return seqSearchDirs;
}

sdv::sequence<sdv::core::SModuleInfo> CModuleControl::GetModuleList() const
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    sdv::sequence<sdv::core::SModuleInfo> seqModuleInfos;

    // Build the module information sequence
    for (const std::shared_ptr<CModuleInst>& ptrModule : m_lstModules)
    {
        if (!ptrModule) continue;
        seqModuleInfos.push_back(ptrModule->GetModuleInfo());
    }
    return seqModuleInfos;
}

sdv::sequence<sdv::SClassInfo> CModuleControl::GetClassList(/*in*/ sdv::core::TModuleID tModuleID) const
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    sdv::sequence<sdv::SClassInfo> seqClassInfos;
    auto itModule = std::find_if(m_lstModules.begin(), m_lstModules.end(), [&](const std::shared_ptr<CModuleInst>& ptrModule)
        {
            return ptrModule && ptrModule->GetModuleID() == tModuleID;
        });
    if (itModule == m_lstModules.end()) return seqClassInfos; // Cannot find the module.

    // Request the class infos from the module instance.
    std::vector<std::string> vecClasses = (*itModule)->GetAvailableClasses();
    for (const std::string& rssClass : vecClasses)
    {
        auto optClassInfo = (*itModule)->GetClassInfo(rssClass);
        if (!optClassInfo) continue;
        seqClassInfos.push_back(*optClassInfo);
    }
    return seqClassInfos;
}

sdv::core::TModuleID CModuleControl::Load(const sdv::u8string& ssModulePath)
{
    if(ssModulePath.empty())
        return 0;

    // Add initial paths if not done so already
    AddCurrentPath();

    // Core services bypass.
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    std::filesystem::path pathModule;
    if (ssModulePath == "core_services.sdv")
        pathModule = static_cast<std::string>(ssModulePath);
    else
    {
        if (GetAppControl().IsMainApplication() || GetAppControl().IsIsolatedApplication())
        {
            // Check the installation for the module.
            pathModule = GetAppConfig().FindInstalledModule(static_cast<std::string>(ssModulePath));
        }
        else
        {
            try
            {
                // Get the search paths if the module path is relative
                std::list<std::filesystem::path> lstSearchPaths;
                std::filesystem::path pathSupplied(static_cast<std::string>(ssModulePath));
                if (pathSupplied.is_relative())
                    lstSearchPaths = m_lstSearchPaths;

                // Add an empty path to allow the OS to search when our own search paths could not find the module.
                lstSearchPaths.push_back(std::filesystem::path());

                // Run through the search paths and try to find the module.
                for (const std::filesystem::path& rpathDirectory : lstSearchPaths)
                {
                    // Compose the path
                    std::filesystem::path pathModuleTemp;
                    if (rpathDirectory.is_absolute())
                        pathModuleTemp = (rpathDirectory / pathSupplied).lexically_normal();
                    else
                    {
                        if (rpathDirectory.empty())
                            pathModuleTemp = pathSupplied.lexically_normal();
                        else
                            pathModuleTemp = (GetExecDirectory() / rpathDirectory / pathSupplied).lexically_normal();
                    }

                    if (std::filesystem::exists(pathModuleTemp))
                    {
                        pathModule = pathModuleTemp;
                        break;
                    }
                }
            }
            catch (const std::exception& re)
            {
                SDV_LOG_ERROR("Supplied path to module load is not valid \"", ssModulePath, "\": ", re.what());
            }
        }
    }

    // Check for an exisiting module. If existing, return the existing module.
    for (const std::shared_ptr<CModuleInst>& rptrModule : m_lstModules)
    {
        if (rptrModule && rptrModule->GetModulePath() == pathModule)
            return rptrModule->GetModuleID();
    }

    // Create a new instance (even if the module could not be found).
    std::shared_ptr<CModuleInst> ptrModule = std::make_shared<CModuleInst>(static_cast<std::string>(ssModulePath), pathModule);
    m_lstModules.push_back(ptrModule);
    m_setConfigModules.insert(ptrModule->GetModuleID());
    if (!ptrModule->IsValid())
    {
        SDV_LOG_ERROR("The module was not found \"", ssModulePath, "\"");
        return 0;   // Do not return the module ID of not loadable modules.
    }
    return ptrModule->GetModuleID();
}

bool CModuleControl::Unload(sdv::core::TModuleID tModuleID)
{
    // Cannot unload the core services
    if (tModuleID == sdv::core::tCoreLibModule)
        return true;

    return ContextUnload(tModuleID, false);
}

bool CModuleControl::HasActiveObjects(sdv::core::TModuleID tModuleID) const
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    auto itModule = std::find_if(m_lstModules.begin(), m_lstModules.end(), [&](const std::shared_ptr<CModuleInst>& ptrModule)
        {
            return ptrModule && ptrModule->GetModuleID() == tModuleID;
        });
    if (itModule == m_lstModules.end()) return false; // Cannot find the module; no active objects known.

    // Redirect the call to the module instance.
    return (*itModule)->HasActiveObjects();
}

std::shared_ptr<CModuleInst> CModuleControl::FindModuleByClass(const std::string& rssClass)
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    for (const std::shared_ptr<CModuleInst>& ptrModule : m_lstModules)
    {
        if (!ptrModule) continue;
        std::vector<std::string> vecClasses = ptrModule->GetAvailableClasses();
        for (const std::string& rssClassLocal : vecClasses)
        {
            if (rssClassLocal == rssClass) return ptrModule;
            auto optClassInfo = ptrModule->GetClassInfo(rssClass);
            if (!optClassInfo) continue;
            if (std::find(optClassInfo->seqClassAliases.begin(), optClassInfo->seqClassAliases.end(), rssClass) !=
                optClassInfo->seqClassAliases.end())
                return ptrModule;
        }
    }

    // For main and isolated applications, check whether the module is in one of the installation manifests.
    auto optManifest = GetAppConfig().FindInstalledComponent(rssClass);
    if (!optManifest) return nullptr;
    auto ssManifest = GetAppConfig().FindInstalledModuleManifest(optManifest->pathRelModule);
    if (ssManifest.empty()) return nullptr;
    lock.unlock();

    // Load the module
    return GetModule(ContextLoad(optManifest->pathRelModule, ssManifest));
}

std::shared_ptr<CModuleInst> CModuleControl::GetModule(sdv::core::TModuleID tModuleID) const
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    auto itModule = std::find_if(m_lstModules.begin(), m_lstModules.end(), [&](const std::shared_ptr<CModuleInst>& ptrModule)
        {
            return ptrModule && ptrModule->GetModuleID() == tModuleID;
        });
    if (itModule == m_lstModules.end()) return nullptr; // Cannot find the module.
    return *itModule;
}

void CModuleControl::UnloadAll(const std::vector<sdv::core::TModuleID>& rvecIgnoreModules)
{
    // Force unloading all modules in reverse order of their loading
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    std::list<std::shared_ptr<CModuleInst>> lstCopy = m_lstModules;
    lock.unlock();
    while (lstCopy.size())
    {
        std::shared_ptr<CModuleInst> ptrModule = std::move(lstCopy.back());
        lstCopy.pop_back();
        if (!ptrModule) continue;

        // On the ignore list?
        if (std::find(rvecIgnoreModules.begin(), rvecIgnoreModules.end(), ptrModule->GetModuleID()) != rvecIgnoreModules.end())
            continue;

        // Unload... -> call the module control function to remove it from the list.
        ContextUnload(ptrModule->GetModuleID(), true);
    }
}

void CModuleControl::ResetConfigBaseline()
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    m_setConfigModules.clear();
}

std::string CModuleControl::SaveConfig(const std::set<std::filesystem::path>& rsetIgnoreModule)
{
    std::stringstream sstream;
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);

    // Add all the loaded modules
    for (const std::shared_ptr<CModuleInst>& rptrModule : m_lstModules)
    {
        if (m_setConfigModules.find(rptrModule->GetModuleID()) != m_setConfigModules.end() &&
            rsetIgnoreModule.find(rptrModule->GetModuleConfigPath()) != rsetIgnoreModule.end())
        {
            sstream << std::endl;
            sstream << "[[Module]]" << std::endl;
            sstream << "Path = \"" << rptrModule->GetModuleConfigPath().generic_u8string() << "\"" << std::endl;
        }
    }
    return sstream.str();
}

sdv::core::TModuleID CModuleControl::ContextLoad(const std::filesystem::path& rpathModule, const std::string& rssManifest)
{
    if (GetAppControl().IsMaintenanceApplication()) return 0;   // Not allowed

    // Run through the manifest and check for complex services, applications and utilities.
    // TODO EVE: Temporary suppression of cppcheck warning.
    // cppcheck-suppress variableScope
    [[maybe_unused]] bool bIsolationNeeded = false;
    try
    {
        size_t nIndex = 0;
        size_t nUnsupportedObjectCount = 0;
        CParserTOML parser(rssManifest);
        do
        {
            std::shared_ptr<CNode> ptrComponentType =
                parser.GetRoot().GetDirect(std::string("Component[") + std::to_string(nIndex++) + "].Type");
            if (!ptrComponentType) break;
            std::string ssType = static_cast<std::string>(ptrComponentType->GetValue());
            if (ssType == "Device") continue; // Okay
            if (ssType == "BasicService") continue; // Okay
            if (ssType == "Proxy") continue; // Okay
            if (ssType == "Stub") continue; // Okay
            if (ssType == "ComplexService" || ssType == "Application" || ssType == "Utility")
            {
                bIsolationNeeded = true;
                continue; // Okay
            }

            // Not okay
            nUnsupportedObjectCount++;
        } while (true);
        if (nIndex == nUnsupportedObjectCount) return 0;    // No object to load.
    }
    catch (const sdv::toml::XTOMLParseException& /*rexcept*/)
    {
        return 0;
    }

    // TODO: If the module to load needs isolation...
    // If this is a main application: isolate modules that should be isolated
    // If this is an isolated application: check for another loaded module that needs isolation already. If this do not allow
    // this module to load.

    // Load the module
    return Load(rpathModule.generic_u8string());
}

bool CModuleControl::ContextUnload(sdv::core::TModuleID tModuleID, bool bForce)
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    auto itModule = std::find_if(m_lstModules.begin(), m_lstModules.end(), [&](const std::shared_ptr<CModuleInst>& ptrModule)
        {
            return ptrModule && ptrModule->GetModuleID() == tModuleID;
        });
    if (itModule == m_lstModules.end()) return true; // Cannot find the module to unload. This is not an error!
    m_setConfigModules.erase(tModuleID);        // Remove from config if part of it.

    // Check whether it is possible to unload.
    bool bSuccess = (*itModule)->Unload(bForce);
    if (bSuccess)
        m_lstModules.erase(itModule);

    return bSuccess;
}

std::shared_ptr<CModuleInst> CModuleControl::FindModuleByPath(const std::filesystem::path& rpathModule) const
{
    // Find the module instance
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);
    auto itModule = std::find_if(m_lstModules.begin(), m_lstModules.end(), [&](const std::shared_ptr<CModuleInst>& ptrModule)
        {
            return ptrModule && ptrModule->GetModulePath() == rpathModule;
        });
    if (itModule == m_lstModules.end()) return nullptr; // Cannot find the module
    return *itModule;
}

void CModuleControl::AddCurrentPath()
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModules);

    // Do this only once.
    if (!m_lstSearchPaths.empty()) return;

    // Add the core directory
    std::filesystem::path pathCoreDir = GetCoreDirectory().lexically_normal();
    m_lstSearchPaths.push_back(pathCoreDir);

    // Add the exe dir
    std::filesystem::path pathExeDir = GetExecDirectory().lexically_normal();
    if (pathExeDir != pathCoreDir) m_lstSearchPaths.push_back(pathExeDir);
}

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

CModuleControl& CModuleControlService::GetModuleControl()
{
    return ::GetModuleControl();
}

bool CModuleControlService::EnableModuleControlAccess()
{
    return GetAppControl().IsStandaloneApplication() ||
        GetAppControl().IsEssentialApplication();
}

#endif
