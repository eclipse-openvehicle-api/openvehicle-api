#include "app_config.h"
#include "sdv_core.h"
#include "../../global/exec_dir_helper.h"
#include <interfaces/com.h>
#include <interfaces/process.h>
#include <interfaces/ipc.h>
#include <support/crc.h>

#if __unix__
#include <utime.h>
#endif

bool CAppConfig::LoadInstallationManifests()
{
    if (!GetAppControl().IsMainApplication() && !GetAppControl().IsIsolatedApplication()) return false;

    std::filesystem::path pathCore = GetCoreDirectory();
    std::filesystem::path pathExe = GetExecDirectory();
    std::filesystem::path pathInstall = GetAppControl().GetInstallDir();

    // Load the core manifest (should work in any case).
    m_manifestCore = CInstallManifest();
    if (!m_manifestCore.Load(pathCore)) return false;

    // Check whether the exe path is identical to the core. If so, skip the manifest. If there is no manifest, this is not an error.
    if (pathCore != pathExe)
    {
        m_manifestExe = CInstallManifest();
        m_manifestExe.Load(pathExe);
    }
    else
        m_manifestExe.Clear();

    // Get the user manifests. If there is no manifest, this is not an error.
    for (auto const& dir_entry : std::filesystem::directory_iterator{ pathInstall })
    {
        if (!dir_entry.is_directory()) continue;
        if (!std::filesystem::exists(dir_entry.path() / "install_manifest.toml")) continue;
        CInstallManifest manifestUser;
        if (manifestUser.Load(dir_entry.path(), true))
            m_vecUserManifests.push_back(manifestUser);
    }

    return true;
}

void CAppConfig::UnloadInstallatonManifests()
{
    if (!GetAppControl().IsMainApplication() && !GetAppControl().IsIsolatedApplication()) return;

    m_vecUserManifests.clear();
    m_manifestExe.Clear();
    m_manifestCore.Clear();
}

sdv::core::EConfigProcessResult CAppConfig::ProcessConfig(/*in*/ const sdv::u8string& ssContent)
{
    if (GetAppControl().GetOperationState() != sdv::app::EAppOperationState::configuring)
        return sdv::core::EConfigProcessResult::failed;

    // Reset the current baseline
    ResetConfigBaseline();

    toml_parser::CParser parser(ssContent);

    // Check for config file compatibility
    auto ptrConfigVersion = parser.Root().Direct("Configuration.Version");
    if (!ptrConfigVersion)
    {
        SDV_LOG_ERROR("Configuration version must be present in the configuration file.");
        return sdv::core::EConfigProcessResult::failed;
    }
    if (ptrConfigVersion->GetValue() != SDVFrameworkInterfaceVersion)
    {
        SDV_LOG_ERROR("Incompatible configuration file version.");
        return sdv::core::EConfigProcessResult::failed;
    }

    // If this is not a main, isolated or maintenance application, load all modules in the component and module sections.
    std::map<std::filesystem::path, sdv::core::TModuleID> mapModules;
    size_t nLoadable = 0;
    size_t nNotLoadable = 0;
    if (!GetAppControl().IsMainApplication() && !GetAppControl().IsIsolatedApplication() &&
        !GetAppControl().IsMaintenanceApplication())
    {
        // Load all modules in the component section
        auto ptrComponents = parser.Root().Direct("Component");
        if (ptrComponents && ptrComponents->Cast<toml_parser::CArray>())
        {
            for (uint32_t uiIndex = 0; uiIndex < ptrComponents->Cast<toml_parser::CArray>()->GetCount(); uiIndex++)
            {
                auto ptrComponent = ptrComponents->Cast<toml_parser::CArray>()->Get(uiIndex);
                if (ptrComponent)
                {
                    // Get the information from the component.
                    std::filesystem::path pathModule;
                    auto ptrModule = ptrComponent->Direct("Path");
                    if (ptrModule) pathModule = static_cast<std::string>(ptrModule->GetValue());

                    // If there is no path, this is allowed... skip this module
                    if (pathModule.empty()) continue;

                    // Load the module.
                    sdv::core::TModuleID tModuleID = GetModuleControl().Load(pathModule.generic_u8string());
                    if (tModuleID)
                    {
                        mapModules[pathModule] = tModuleID;
                        nLoadable++;
                    }
                    else
                    {
                        SDV_LOG_ERROR("Failed to load module: ", pathModule);
                        nNotLoadable++;
                        continue;
                    }
                }
            }
        }

        // Load all modules from the module section.
        auto ptrModules = parser.Root().Direct("Module");
        if (ptrModules && ptrModules->Cast<toml_parser::CArray>())
        {
            for (uint32_t uiIndex = 0; uiIndex < ptrModules->Cast<toml_parser::CArray>()->GetCount(); uiIndex++)
            {
                auto ptrModule = ptrModules->Cast<toml_parser::CArray>()->Get(uiIndex);
                if (ptrModule)
                {
                    // Get the information from the component.
                    std::filesystem::path pathModule;
                    auto ptrModulePath = ptrModule->Direct("Path");
                    if (ptrModulePath) pathModule = static_cast<std::string>(ptrModulePath->GetValue());

                    if (pathModule.empty())
                    {
                        SDV_LOG_ERROR("Missing module path for component configuration.");
                        nNotLoadable++;
                        continue;
                    }

                    // Load the module.
                    sdv::core::TModuleID tModuleID = GetModuleControl().Load(pathModule.generic_u8string());
                    if (tModuleID)
                    {
                        mapModules[pathModule] = tModuleID;
                        nLoadable++;
                    }
                    else
                    {
                        SDV_LOG_ERROR("Failed to load module: ", pathModule);
                        nNotLoadable++;
                        continue;
                    }
                }
            }
        }
    }

    // Start all components from the component section
    std::filesystem::path pathLastModule;
    auto ptrComponents = parser.Root().Direct("Component");
    if (ptrComponents && ptrComponents->Cast<toml_parser::CArray>())
    {
        for (uint32_t uiIndex = 0; uiIndex < ptrComponents->Cast<toml_parser::CArray>()->GetCount(); uiIndex++)
        {
            auto ptrComponent = ptrComponents->Cast<toml_parser::CArray>()->Get(uiIndex);
            if (ptrComponent)
            {
                // Get the information from the component.
                std::filesystem::path pathModule;
                auto ptrModule = ptrComponent->Direct("Path");
                if (ptrModule) pathModule = static_cast<std::string>(ptrModule->GetValue());
                std::string ssClass;
                auto ptrClass = ptrComponent->Direct("Class");
                if (ptrClass) ssClass = static_cast<std::string>(ptrClass->GetValue());
                std::string ssName;
                auto ptrName = ptrComponent->Direct("Name");
                if (ptrName) ssName = static_cast<std::string>(ptrName->GetValue());

                // If there is a path, store it. If there is none, take the last stored
                if (!pathModule.empty())
                    pathLastModule = pathModule;
                else
                    pathModule = pathLastModule;

                if (pathModule.empty())
                {
                    SDV_LOG_ERROR("Missing module path for component configuration.");
                    nNotLoadable++;
                    continue;
                }
                if (ssClass.empty())
                {
                    SDV_LOG_ERROR("Missing component class name in the configuration.");
                    nNotLoadable++;
                    continue;
                }

                // In case of a main, isolated or maintenance application, ignore the module. In all other cases, the module was
                // loaded; get the module ID.
                sdv::core::TObjectID tObjectID = 0;
                if (!GetAppControl().IsMainApplication() && !GetAppControl().IsIsolatedApplication() &&
                    !GetAppControl().IsMaintenanceApplication())
                {
                    auto itModule = mapModules.find(pathModule);
                    if (itModule == mapModules.end()) continue; // Module was not loaded before...
                    tObjectID = GetRepository().CreateObjectFromModule(itModule->second, ssClass, ssName, ptrComponent->GenerateTOML());
                }
                else
                    tObjectID = GetRepository().CreateObject2(ssClass, ssName, ptrComponent->GenerateTOML());

                if (!tObjectID)
                {
                    SDV_LOG_ERROR("Failed to load component: ", ssClass);
                    nNotLoadable++;
                    continue;
                }
                else
                    nLoadable++;
            }
        }
    }
    if (!nNotLoadable)
        return sdv::core::EConfigProcessResult::successful;
    if (!nLoadable)
        return sdv::core::EConfigProcessResult::failed;
    return sdv::core::EConfigProcessResult::partially_successful;
}

sdv::core::EConfigProcessResult CAppConfig::LoadConfig(/*in*/ const sdv::u8string& ssConfigPath)
{
    if (GetAppControl().GetOperationState() != sdv::app::EAppOperationState::configuring)
        return sdv::core::EConfigProcessResult::failed;
    if (ssConfigPath.empty())
        return sdv::core::EConfigProcessResult::failed;

    std::filesystem::path pathConfig;
    if (GetAppControl().IsMainApplication())
    {
        pathConfig = GetAppControl().GetInstallDir() / static_cast<std::string>(ssConfigPath);
        if (!std::filesystem::exists(pathConfig) || !std::filesystem::is_regular_file(pathConfig))
            pathConfig.clear();
    }
    else
    {
        AddCurrentPath();

        try
        {
            // Get the search paths if the module path is relative
            std::list<std::filesystem::path> lstSearchPaths;
            std::filesystem::path pathSupplied(static_cast<std::string>(ssConfigPath));
            if (pathSupplied.is_relative())
                lstSearchPaths = m_lstSearchPaths;

            // Add an empty path to allow the OS to search when our own search paths could not find the module.
            lstSearchPaths.push_back(std::filesystem::path());

            // Run through the search paths and try to find the config file.
            for (const std::filesystem::path& rpathDirectory : lstSearchPaths)
            {
                // Compose the path
                std::filesystem::path pathConfigTemp;
                if (rpathDirectory.is_absolute())
                    pathConfigTemp = (rpathDirectory / pathSupplied).lexically_normal();
                else
                {
                    if (rpathDirectory.empty())
                        pathConfigTemp = pathSupplied.lexically_normal();
                    else
                        pathConfigTemp = (GetExecDirectory() / rpathDirectory / pathSupplied).lexically_normal();
                }
                if (std::filesystem::exists(pathConfigTemp))
                {
                    pathConfig = pathConfigTemp;
                    m_pathLastSearchDir = rpathDirectory;
                    break;
                }
            }
        }
        catch (const std::exception& re)
        {
            SDV_LOG_ERROR("Supplied path to config load is not valid \"", ssConfigPath, "\": ", re.what());
            return sdv::core::EConfigProcessResult::failed;
        }
    }
    if (pathConfig.empty())
    {
        SDV_LOG_ERROR("Configuration file was not found \"", ssConfigPath, "\"");
        return sdv::core::EConfigProcessResult::failed;
    }

    std::ifstream fstream(pathConfig);
    if (!fstream.is_open()) return sdv::core::EConfigProcessResult::failed;
    std::string ssContent((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
    return ProcessConfig(ssContent);
}

bool CAppConfig::SaveConfig(/*in*/ const sdv::u8string& ssConfigPath) const
{
    std::filesystem::path pathConfig = static_cast<std::string>(ssConfigPath);
    if (pathConfig.is_relative())
    {
        if (GetAppControl().IsMainApplication())
            pathConfig = GetAppControl().GetInstallDir() / pathConfig;
        else
        {
            if (m_pathLastSearchDir.empty())
                pathConfig = GetExecDirectory() / pathConfig;
            else
                pathConfig = m_pathLastSearchDir / pathConfig;
        }
    }

    std::ofstream fstream(pathConfig, std::ios::out | std::ios::trunc);
    if (!fstream.is_open())
    {
        SDV_LOG_ERROR("Cannot open config file \"", ssConfigPath, "\"");
        return false;
    }

    // Write header
    fstream << "[Configuration]" << std::endl;
    fstream << "Version = " << SDVFrameworkInterfaceVersion << " # Configuration file version." << std::endl;

    // Write all objects and modules
    fstream << GetRepository().SaveConfig();

    fstream.close();

    return true;
}

bool CAppConfig::AddConfigSearchDir(/*in*/ const sdv::u8string& ssDir)
{
    if (GetAppControl().GetOperationState() != sdv::app::EAppOperationState::configuring) return false;

    // Add initial paths if not done so already
    AddCurrentPath();

    std::unique_lock<std::mutex> lock(m_mtxSearchPaths);

    std::filesystem::path pathDir(ssDir.c_str());

    // Relative paths are always relative of the executable
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

void CAppConfig::ResetConfigBaseline()
{
    GetRepository().ResetConfigBaseline();
}

#ifdef _MSC_VER
    // Prevent bogus warning about uninitialized memory for the variable *hFile.
    #pragma warning(push)
    #pragma warning(disable : 6001)
#endif

bool CAppConfig::Install(/*in*/ const sdv::u8string& ssInstallName, /*in*/ const sdv::sequence<sdv::installation::SFileDesc>& seqFiles)
{
    // Installations can only be done in the main application.
    if (!GetAppControl().IsMainApplication()) return false;

    std::unique_lock<std::mutex> lock(m_mtxInstallations);

    // Check whether the installation already exists. If so, cancel the installation.
    if (m_mapInstallations.find(ssInstallName) != m_mapInstallations.end())
    {
        throw sdv::installation::XDuplicateInstall();
    }

    // Rollback class - automatically reverses the loading.
    class CRollback
    {
    public:
        ~CRollback()
        {
            bool bSuccess = true;
            for (sdv::core::TModuleID tModuleID : m_vecLoadedModules)
			{
				try
				{
					bSuccess &= GetModuleControl().ContextUnload(tModuleID, true);
				}
				catch (const sdv::XSysExcept&)
				{
				}
				catch (const std::exception&)
				{
				}
			}
            if (!bSuccess)
            {
                SDV_LOG_ERROR("Failed to rollback \"", m_pathInstallRoot, "\".");
                return;
            }

            if (!m_pathInstallRoot.empty())
            {
                try
                {
                    std::filesystem::remove_all(m_pathInstallRoot);
                }
                catch (const std::filesystem::filesystem_error& rexcept)
                {
                    SDV_LOG_ERROR("Failed to rollback \"", m_pathInstallRoot, "\": ", rexcept.what());
                }
            }
        }
        void Commit()
        {
            // Do not roll back.
            m_pathInstallRoot.clear();
            m_vecLoadedModules.clear();
        }
        void SetInstallPath(const std::filesystem::path& rPathInstallDir) { m_pathInstallRoot = rPathInstallDir; }
        void AddLoadedModule(sdv::core::TModuleID tModuleID) { m_vecLoadedModules.push_back(tModuleID); }
    private:
        std::filesystem::path               m_pathInstallRoot;      ///< Installation root directory
        std::vector<sdv::core::TModuleID>   m_vecLoadedModules;     ///< loaded modules
    } rollback;

    std::filesystem::path pathInstallation = GetAppControl().GetInstallDir() / static_cast<std::string>(ssInstallName);
    std::vector<std::filesystem::path> vecComponents;
    try
    {
        // A sub-directory with the name must not already exist, if so, delete it.
        if (std::filesystem::exists(pathInstallation))
            std::filesystem::remove_all(pathInstallation);

        // Create a directory for the installation
        std::filesystem::create_directories(pathInstallation);
        rollback.SetInstallPath(pathInstallation);

        // Add each file
        for (const sdv::installation::SFileDesc& rsFileDesc : seqFiles)
        {
            // File path
            std::filesystem::path pathFileDir = pathInstallation;
            //if (!rsFileDesc.ssRelativeDir.empty())
            //{
            //    std::filesystem::path pathTemp = static_cast<std::string>(rsFileDesc.ssRelativeDir);
            //    if (pathTemp.is_absolute() || !pathTemp.is_relative())
            //        throw sdv::installation::XIncorrectPath();
            //    pathFileDir /= pathTemp;
            //    if (!std::filesystem::exists(pathFileDir))
            //        std::filesystem::create_directories(pathFileDir);
            //}
            std::filesystem::path pathFile = pathFileDir;
            if (std::filesystem::exists(pathFile))
                throw sdv::installation::XIncorrectPath();

            // TODO EVE
            //// Check the file CRC
            //sdv::crcCRC32C crc;
            //uint32_t uiCRC = crc.calc_checksum(rsFileDesc.ptrContent.get(), rsFileDesc.ptrContent.size());
            //if (uiCRC != rsFileDesc.uiCRC32c)
            //    throw sdv::installation::XIncorrectCRC();

            // Create the file
            std::ofstream fstream(pathFile, std::ios_base::out | std::ios_base::binary);
            if (!fstream.is_open()) throw sdv::installation::XIncorrectPath();
            if (rsFileDesc.ptrContent.size())
                fstream.write(reinterpret_cast<const char*>(rsFileDesc.ptrContent.get()), rsFileDesc.ptrContent.size());
            fstream.close();

            // Set file times
            // Note: use dedicated OS functions. C++11 filesystem library doesn't define a proper way to convert between UNIX time
            // and the time defined by the C++11 functions. This deficit has been solved in C++20.

#ifdef _WIN32
            // Windows uses file access for this.
            HANDLE hFile = CreateFile(pathFile.native().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == nullptr || hFile == INVALID_HANDLE_VALUE)
            {
                SDV_LOG_ERROR("Cannot create files or directories in \"", pathInstallation, "\"");
                throw sdv::installation::XIncorrectPath();
            }

            // Set the filetime on the file.
            // The filetime starts counting from 1st of January 1601. The resolution is 100ns.
            // The UNIX time starts counting from 1st of January 1970. The resolution is 1s.
            // Both times ignore leap seconds.
            // Thus the UNIX time is running 11644473600 seconds behind the filetime and is a factor 10000000 larger.
            ULARGE_INTEGER sCreateTime{}, sChangeTime{};
            sCreateTime.QuadPart = (rsFileDesc.uiCreationDate + 11644473600ull) * 10000000ull;
            FILETIME ftCreateTime{ sCreateTime.LowPart, sChangeTime.HighPart };
            sChangeTime.QuadPart = (rsFileDesc.uiChangeDate + 11644473600ull) * 10000000ull;
            FILETIME ftChangeTime{ sChangeTime.LowPart, sChangeTime.HighPart };
            BOOL bRes = SetFileTime(hFile, &ftCreateTime, nullptr, &ftChangeTime);

            // Close our handle.
            CloseHandle(hFile);

            if (!bRes)
            {
                SDV_LOG_ERROR("Cannot create files or directories in \"", pathInstallation, "\"");
                throw sdv::installation::XIncorrectPath();
            }
#elif defined __unix__
            // Note: POSIX doesn't define the creation time.
            utimbuf sTimes{};
            sTimes.actime = std::time(0);
            sTimes.modtime = static_cast<time_t>(rsFileDesc.uiChangeDate);
            if (utime(pathFile.native().c_str(), &sTimes))
            {
                SDV_LOG_ERROR("Cannot create files or directories in \"", pathInstallation, "\"");
                throw sdv::installation::XIncorrectPath();
            }
#endif

            // Set attributes
            if (rsFileDesc.bAttrExecutable)
                std::filesystem::permissions(pathFile, std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec |
                    std::filesystem::perms::others_exec, std::filesystem::perm_options::add);
            if (rsFileDesc.bAttrReadonly)
                std::filesystem::permissions(pathFile, std::filesystem::perms::owner_write | std::filesystem::perms::group_write |
                    std::filesystem::perms::others_write, std::filesystem::perm_options::remove);

            // Component paths are stored
            //if (rsFileDesc.bAttrComponent) vecComponents.push_back(pathFile);
        }
    }
    catch (const std::filesystem::filesystem_error& rexcept)
    {
        SDV_LOG_ERROR("Cannot create files or directories in \"", pathInstallation, "\": ", rexcept.what());
        throw sdv::installation::XIncorrectPath();
    }

    // TODO: Restriction. Only complex services will be started dynamically and will be added to the configuration. Utilities will
    // also be installed but will not be started. Devices and basic services will not be available this way.

    // TODO:
    // For each component, get the manifest using the manifest helper utility.
    // Note: to get the manifest, the component module needs to be loaded and the GetManifest exported function is called. Loading
    // a component module causes part of the component to start already and dependencies to be resolved. If this malfunctions, the
    // component could crash or do other malicious things. OR if on purpose, an application or complex service component, that
    // normally would run in an isolated environment, gets access to the memory and everything else within the process. If this
    // module is not isolated from the core, it could access the core and do nasty things. THEREFORE, the manifest helper utility is
    // started as a component in its own isolated environment and then loading the component. No component code is executed within
    // the core process.
    sdv::TObjectPtr ptrManifestUtil = sdv::core::CreateUtility("ManifestHelperUtility");
    sdv::helper::IModuleManifestHelper* pManifestHelper = ptrManifestUtil.GetInterface<sdv::helper::IModuleManifestHelper>();
    if (!pManifestHelper)
    {
        SDV_LOG_ERROR("Manifest helper utility cannot be loaded.");
        throw sdv::installation::XComponentNotLoadable();
    }

    // Get the manifest of all components
    std::map<std::filesystem::path, sdv::u8string> mapComponentManifests;
    for (const std::filesystem::path& rpathComponent : vecComponents)
    {
        sdv::u8string ssManifest = pManifestHelper->ReadModuleManifest(rpathComponent.generic_u8string());
        if (!ssManifest.empty())
            mapComponentManifests[rpathComponent] = ssManifest;
    }

    // Check whether there ary any manifests
    if (mapComponentManifests.empty())
    {
        SDV_LOG_ERROR("No component in installation", ssInstallName, ".");
        throw sdv::installation::XNoManifest();
    }

    // Load the modules
    bool bSuccess = true;
    for (const auto& rvtModule : mapComponentManifests)
    {
        sdv::core::TModuleID tModuleID = GetModuleControl().ContextLoad(rvtModule.first, rvtModule.second);
        if (!tModuleID)
        {
            bSuccess = false;
            break;
        }

        // TODO: What about the dependent modules that were started as well. They are not rolled back...
        rollback.AddLoadedModule(tModuleID);
    }
    if (!bSuccess)
    {
        SDV_LOG_ERROR("Manifest helper utility cannot be loaded.");
        throw sdv::installation::XComponentNotLoadable();
    }

    // Add the installation to the installation map
    //SInstallation sInstallation{ seqFiles };
    //for (sdv::core::)
    //m_mapInstallations[ssInstallName] = SInstallation{ seqFiles };

    // Commit the changes - prevent rollback
    rollback.Commit();

    return true;
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

bool CAppConfig::Update(/*in*/ const sdv::u8string& ssInstallName, /*in*/ const sdv::sequence<sdv::installation::SFileDesc>& seqFiles)
{
    // TODO: as soon as the configuration is updatable as well, the update uses the same configuration and possibly updates the
    // configuration.
    // Until the config update feature is available, the update can stop and update and start the installation by uninstall and
    // install.

    bool bRet = Uninstall(ssInstallName);
    if (bRet) bRet = Install(ssInstallName, seqFiles);
    return bRet;
}

bool CAppConfig::Uninstall(/*in*/ const sdv::u8string& /*ssInstallName*/)
{
    // Installations can only be done in the main application.
    if (!GetAppControl().IsMainApplication()) return false;


    return false;
}

sdv::sequence<sdv::u8string> CAppConfig::GetInstallations() const
{
    return sdv::sequence<sdv::u8string>();
}

sdv::sequence<sdv::installation::SFileDesc> CAppConfig::GetInstallationFiles(/*in*/ const sdv::u8string& /*ssInstallName*/) const
{
    return sdv::sequence<sdv::installation::SFileDesc>();
}

std::filesystem::path CAppConfig::FindInstalledModule(const std::filesystem::path& rpathRelModule)
{
    // Search for the module in the following order:
    // - Core manifest
    // - Exe manifest
    // - User manifests

    std::filesystem::path pathModule = m_manifestCore.FindModule(rpathRelModule);
    if (pathModule.empty())
        pathModule = m_manifestExe.FindModule(rpathRelModule);
    for (const CInstallManifest& rmanifest : m_vecUserManifests)
    {
        if (!pathModule.empty()) break;
        pathModule = rmanifest.FindModule(rpathRelModule);
    }
    return pathModule;
}

std::string CAppConfig::FindInstalledModuleManifest(const std::filesystem::path& rpathRelModule)
{
    // Search for the module in the following order:
    // - Core manifest
    // - Exe manifest
    // - User manifests

    std::string ssManifest = m_manifestCore.FindModuleManifest(rpathRelModule);
    if (ssManifest.empty())
        ssManifest = m_manifestExe.FindModuleManifest(rpathRelModule);
    for (const CInstallManifest& rmanifest : m_vecUserManifests)
    {
        if (!ssManifest.empty()) break;
        ssManifest = rmanifest.FindModuleManifest(rpathRelModule);
    }
    return ssManifest;
}

std::optional<CInstallManifest::SComponent> CAppConfig::FindInstalledComponent(const std::string& rssClass) const
{
    // For main and isolated applications, check whether the module is in one of the installation manifests.
    if (!GetAppControl().IsMainApplication() && !GetAppControl().IsIsolatedApplication()) return {};

    // Search for the component in the following order:
    // - Core manifest
    // - Exe manifest
    // - User manifests
    auto optManifest = m_manifestCore.FindComponentByClass(rssClass);
    if (!optManifest) optManifest = m_manifestExe.FindComponentByClass(rssClass);
    for (const CInstallManifest& rmanifest : m_vecUserManifests)
    {
        if (optManifest) break;
        optManifest = rmanifest.FindComponentByClass(rssClass);
    }
    return optManifest;
}

void CAppConfig::AddCurrentPath()
{
    std::unique_lock<std::mutex> lock(m_mtxSearchPaths);

    // Do this only once.
    if (!m_lstSearchPaths.empty()) return;

    // Add the core directory
    std::filesystem::path pathCoreDir = GetCoreDirectory().lexically_normal();
    m_lstSearchPaths.push_back(pathCoreDir / "config/");

    // Add the exe dir
    std::filesystem::path pathExeDir = GetExecDirectory().lexically_normal();
    if (pathExeDir != pathCoreDir) m_lstSearchPaths.push_back(pathExeDir / "config/");
}

#ifndef DO_NOT_INCLUDE_IN_UNIT_TEST

CAppConfig& CAppConfigService::GetAppConfig()
{
    return ::GetAppConfig();
}

bool CAppConfigService::EnableAppConfigAccess()
{
    return GetAppControl().IsStandaloneApplication() ||
        GetAppControl().IsEssentialApplication();
}

bool CAppConfigService::EnableAppInstallAccess()
{
    return GetAppControl().IsMainApplication();
}

#endif // !defined DO_NOT_INCLUDE_IN_UNIT_TEST