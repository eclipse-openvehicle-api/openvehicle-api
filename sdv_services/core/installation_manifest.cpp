#include "installation_manifest.h"
#include "toml_parser/parser_toml.h"
#include <support/serdes.h>

#if defined _WIN32 && defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#ifdef __unix__
#include <dlfcn.h>
#endif

/**
 * @brief Read the module manifest from the binary.
 * @param[in] rpathModule Reference to the module to read the manifest from.
 * @return The manifest, if existing or an empty string when not.
*/
std::string ReadModuleManifest(const std::filesystem::path& rpathModule)
{
    if (rpathModule.extension() != ".sdv") return {};
    if (!std::filesystem::exists(rpathModule)) return {};

    // Load the module
#ifdef _WIN32
    sdv::core::TModuleID tModuleID = reinterpret_cast<sdv::core::TModuleID>(LoadLibrary(rpathModule.native().c_str()));
#elif defined __unix__
    sdv::core::TModuleID tModuleID = reinterpret_cast<sdv::core::TModuleID>(dlopen(rpathModule.native().c_str(), RTLD_LAZY));
#else
#error OS not supported!
#endif
    if (!tModuleID) return {};

    // Check whether the module exposes the necessary functions
    using TFNHasActiveObjects = bool();
    using TFNGetModuleFactory = sdv::IInterfaceAccess*(uint32_t);
    using TFNGetManifest = const char*();

#ifdef _WIN32
    std::function<TFNGetModuleFactory> fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "GetModuleFactory"));
    std::function<TFNHasActiveObjects> fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "HasActiveObjects"));
    std::function<TFNGetManifest> fnGetManifest = reinterpret_cast<TFNGetManifest*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "GetManifest"));
#elif defined __unix__
    std::function<TFNGetModuleFactory> fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(dlsym(reinterpret_cast<void*>(tModuleID), "GetModuleFactory"));
    std::function<TFNHasActiveObjects> fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(dlsym(reinterpret_cast<void*>(tModuleID), "HasActiveObjects"));
    std::function<TFNGetManifest> fnGetManifest = reinterpret_cast<TFNGetManifest*>(dlsym(reinterpret_cast<void*>(tModuleID), "GetManifest"));
#else
#error OS not supported!
#endif

    // Check for functions and the correct version
    std::string ssManifest;
    if (fnGetFactory && fnActiveObjects && fnGetManifest && fnGetManifest())
        ssManifest = fnGetManifest();

    // Release the library
#ifdef _WIN32
    FreeLibrary(reinterpret_cast<HMODULE>(tModuleID));
#elif defined __unix__
    dlclose(reinterpret_cast<void*>(tModuleID));
#else
#error OS not supported!
#endif

    // Return the manifest
    return ssManifest;
}

bool CInstallManifest::IsValid() const
{
    // Must have an installation name to be valid.
    return !m_ssInstallName.empty();
}

void CInstallManifest::Clear()
{
    m_ssInstallName.clear();
    m_vecModules.clear();
    m_bBlockSystemObjects = false;
}

const std::string& CInstallManifest::InstallName() const
{
    return m_ssInstallName;
}

sdv::installation::SPackageVersion CInstallManifest::Version() const
{
    auto optProperty = Property("Version");
    if (!optProperty) return {};

    // A property string is composed of: major.minor.patch (numbers only; characters and whitespace are ignored).
    sdv::installation::SPackageVersion sVersion{};
    size_t nPos = optProperty->find('.');
    sVersion.uiMajor = static_cast<uint32_t>(std::atoi(optProperty->substr(0, nPos).c_str()));
    size_t nStart = nPos;
    if (nStart != std::string::npos)
    {
        nStart++;
        nPos = optProperty->find('.', nPos + 1);
        sVersion.uiMinor = static_cast<uint32_t>(std::atoi(optProperty->substr(nStart, nPos).c_str()));
    }
    nStart = nPos;
    if (nStart != std::string::npos)
    {
        nStart++;
        sVersion.uiPatch = static_cast<uint32_t>(std::atoi(optProperty->substr(nStart).c_str()));
    }
    return sVersion;
}

const std::filesystem::path& CInstallManifest::InstallDir() const
{
    return m_pathInstallDir;
}

bool CInstallManifest::Create(const std::string& rssInstallName)
{
    Clear();

    m_ssInstallName = rssInstallName;

    return true;
}

bool CInstallManifest::Load(const std::filesystem::path& rpathInstallDir, bool bBlockSystemObjects /*= false*/)
{
    if (rpathInstallDir.empty() || !std::filesystem::exists(rpathInstallDir) ||
        !std::filesystem::is_directory(rpathInstallDir)) return {};
    m_pathInstallDir = rpathInstallDir;

    // Create the manifest file
    std::ifstream fstream(rpathInstallDir / "install_manifest.toml");
    if (!fstream.is_open()) return false;

    // Read the manifest in memory
    std::stringstream sstreamManifest;
    sstreamManifest << fstream.rdbuf();
    fstream.close();

    return Read(sstreamManifest.str(), bBlockSystemObjects);
}

bool CInstallManifest::Save(const std::filesystem::path& rpathInstallDir) const
{
    if (!IsValid() || rpathInstallDir.empty()) return false;
    m_pathInstallDir = rpathInstallDir;

    // Create the manifest file
    std::ofstream fstream(rpathInstallDir / "install_manifest.toml", std::ios_base::out | std::ios_base::trunc);
    if (!fstream.is_open()) return false;

    std::string ssManifest = Write();
    if (!ssManifest.empty()) fstream << ssManifest;

    fstream.close();

    return !ssManifest.empty();
}

bool CInstallManifest::Read(const std::string& rssManifest, bool bBlockSystemObjects /*= false*/)
{
    Clear();
    m_bBlockSystemObjects = bBlockSystemObjects;

    // Parse the manifest
    CParserTOML parser(rssManifest);

    // Get the installation version - must be identical to the interface version
    auto ptrInstallVersionNode = parser.GetRoot().GetDirect("Installation.Version");
    if (!ptrInstallVersionNode || ptrInstallVersionNode->GetValue() != SDVFrameworkInterfaceVersion) return false;

    // Get the installation name
    auto ptrInstallNameNode = parser.GetRoot().GetDirect("Installation.Name");
    if (!ptrInstallNameNode) return false;
    m_ssInstallName = static_cast<std::string>(ptrInstallNameNode->GetValue());
    if (m_ssInstallName.empty()) return false;

    // Get installation properties. The properties are optional
    auto ptrProperties = parser.GetRoot().GetDirect("Properties");
    std::shared_ptr<CTable> ptrPropertyTable;
    if (ptrProperties) ptrPropertyTable = ptrProperties->GetTable();
    if (ptrPropertyTable)
    {
        for (uint32_t uiIndex = 0; uiIndex < ptrPropertyTable->GetCount(); uiIndex++)
        {
            auto ptrProperty = ptrPropertyTable->Get(uiIndex);
            if (ptrProperty)
                m_mapProperties[ptrProperty->GetName()] = static_cast<std::string>(ptrProperty->GetValue());
        }
    }

    // Build the module list
    auto ptrModulesNode = parser.GetRoot().GetDirect("Module");
    if (!ptrModulesNode) return true;   // No modules in the manifest
    auto ptrModuleArrayNode = ptrModulesNode->GetArray();
    if (!ptrModuleArrayNode) return false;  // Must be array
    for (uint32_t uiModuleIndex = 0; uiModuleIndex < ptrModuleArrayNode->GetCount(); uiModuleIndex++)
    {
        // Get the module
        auto ptrModule = ptrModuleArrayNode->Get(uiModuleIndex);
        if (!ptrModule) continue;

        // Get the module path
        auto ptrModulePath = ptrModule->GetDirect("Path");
        if (!ptrModulePath) continue;
        std::filesystem::path pathModule = static_cast<std::string>(ptrModulePath->GetValue());
        std::string ssModuleManifest;

        // Get the component list (if available)
        auto ptrModuleComponents = ptrModule->GetDirect("Component");
        if (ptrModuleComponents)
        {
            // The module manifest contains the TOML text of the component array
            auto ptrModuleComponentArray = ptrModuleComponents->GetArray();
            if (ptrModuleComponentArray)
                ssModuleManifest = ptrModuleComponents->CreateTOMLText();
        }

        // Add the module
        m_vecModules.push_back(SModule(pathModule, ssModuleManifest,
            m_bBlockSystemObjects));
    }

    return true;
}

std::string CInstallManifest::Write() const
{
    if (!IsValid()) return {};

    std::stringstream sstream;

    // Add the installation section
    sstream << "[Installation]" << std::endl << "Version = " << SDVFrameworkSubbuildVersion << std::endl << "Name = \"" <<
        m_ssInstallName << "\"" << std::endl << std::endl;

    // Add the properties section (if there are any properties)
    if (!m_mapProperties.empty())
    {
        sstream << "[Properties]" << std::endl;
        for (const auto& rvtProperty : m_mapProperties)
        {
            if (NeedQuotedName(rvtProperty.first))
                sstream << "\"" << rvtProperty.first << "\"";
            else
                sstream << rvtProperty.first;
            sstream << " = \"" << rvtProperty.second << "\"" << std::endl;
        }
        sstream << std::endl;
    }

    // Add the modules
    for (const SModule& rsEntry : m_vecModules)
    {
        sstream << "[[Module]]" << std::endl << "Path=\"" << rsEntry.pathRelModule.generic_u8string() << "\"" << std::endl;

        // Read the module manifest
        CParserTOML parser(rsEntry.ssManifest);

        // Add the module manifest as part of the installation manifest.
        sstream << parser.CreateTOMLText("Module") << std::endl;
    }

    return sstream.str();
}

bool CInstallManifest::AddModule(const std::filesystem::path& rpathModulePath,
    const std::filesystem::path& rpathRelTargetDir /*= "."*/)
{
    if (!IsValid()) return false;

    // Check for the existence of the module.
    if (!std::filesystem::exists(rpathModulePath)) return false;
    if (!std::filesystem::is_regular_file(rpathModulePath)) return false;

    // Check for a relative path
    if (!rpathRelTargetDir.is_relative()) return false;
    if (RefersToRelativeParent(rpathRelTargetDir)) return false;

    // Read the manifest...
    std::string ssManifest = ReadModuleManifest(rpathModulePath);

    // Read the component manifest if existing
    std::string ssComponentsManifest;
    if (!ssManifest.empty())
    {
        // Check the interface version for compatibility
        CParserTOML parser(ssManifest);
        auto ptrInterfaceNode = parser.GetRoot().GetDirect("Interface.Version");
        if (!ptrInterfaceNode) return false;
        if (ptrInterfaceNode->GetValue() != SDVFrameworkInterfaceVersion) return false;
        auto ptrComponentsNode = parser.GetRoot().GetDirect("Component");
        if (!ptrComponentsNode) return true;    // No component available in the manifest
        if (!ptrComponentsNode->GetArray()) return false;
        ssComponentsManifest = ptrComponentsNode->CreateTOMLText();
    }

    // Store path and component
    std::filesystem::path pathRelModule = (rpathRelTargetDir / rpathModulePath.filename()).lexically_normal();
    m_vecModules.push_back(SModule(pathRelModule,
        ssComponentsManifest, m_bBlockSystemObjects));

    return true;
}

std::filesystem::path CInstallManifest::FindModule(const std::filesystem::path& rpathRelModule) const
{
    if (!IsValid() || m_pathInstallDir.empty())
        return {};

    // Search for the correct module
    auto itModule = std::find_if(
        m_vecModules.begin(), m_vecModules.end(), [&](const SModule& rsEntry) { return rsEntry.pathRelModule == rpathRelModule; });
    if (itModule != m_vecModules.end())
        return m_pathInstallDir / rpathRelModule;
    return {};
}

std::string CInstallManifest::FindModuleManifest(const std::filesystem::path& rpathRelModule) const
{
    if (!IsValid() || m_pathInstallDir.empty())
        return {};

    // Search for the correct module
    auto itModule = std::find_if(
        m_vecModules.begin(), m_vecModules.end(), [&](const SModule& rsEntry) { return rsEntry.pathRelModule == rpathRelModule; });
    if (itModule != m_vecModules.end())
        return itModule->ssManifest;
    return {};
}

std::optional<CInstallManifest::SComponent> CInstallManifest::FindComponentByClass(const std::string& rssClass) const
{
    // Search for the correct module
    SComponent sRet{};
    auto itModule = std::find_if(m_vecModules.begin(), m_vecModules.end(), [&](const SModule& rsEntry)
        {
            return std::find_if(rsEntry.vecComponents.begin(), rsEntry.vecComponents.end(),
                [&](const SComponent& sComponent)
                {
                    // Note, use the class, alias and the default object name for searching...
                    if (sComponent.ssClassName == rssClass ||
                        std::find(sComponent.seqAliases.begin(), sComponent.seqAliases.end(), rssClass) !=
                            sComponent.seqAliases.end())
                    {
                        sRet = sComponent;
                        return true;
                    }
                    return false;
                }) != rsEntry.vecComponents.end();
        });
    if (itModule != m_vecModules.end()) return sRet;
    return {};
}

std::vector<CInstallManifest::SComponent> CInstallManifest::ComponentList() const
{
    std::vector<SComponent> vecComponents;
    for (const auto& rsModule : m_vecModules)
    {
        for (const auto& rsComponent : rsModule.vecComponents)
            vecComponents.push_back(rsComponent);
    }
    return vecComponents;
}

std::vector<std::filesystem::path> CInstallManifest::ModuleList() const
{
    std::vector<std::filesystem::path> vecModules;
    for (const auto& rsModule : m_vecModules)
        vecModules.push_back(rsModule.pathRelModule);
    return vecModules;
}

std::vector<std::pair<std::string, std::string>> CInstallManifest::PropertyList() const
{
    return std::vector<std::pair<std::string, std::string>>(m_mapProperties.begin(), m_mapProperties.end());
}

void CInstallManifest::Property(const std::string& rssName, const std::string& rssValue)
{
    // Check for a quote in the name... if existing then do not allow insertion.
    if (rssName.find_first_of("\"'") != std::string::npos)
        return;
    m_mapProperties[rssName] = rssValue;
}

std::optional<std::string> CInstallManifest::Property(const std::string& rssName) const
{
    auto itProperty = m_mapProperties.find(rssName);
    if (itProperty == m_mapProperties.end())
        return {};
    return itProperty->second;
}

bool CInstallManifest::NeedQuotedName(const std::string& rssName)
{
    for (char c : rssName)
    {
        if (!std::isalnum(c) && c != '_' && c != '-')
            return true;
    }
    return false;
}

CInstallManifest::SModule::SModule(const std::filesystem::path& rpathRelModule, const std::string& rssManifest,
    bool bBlockSystemObjects) : pathRelModule(rpathRelModule), ssManifest(rssManifest)
{
    // Parse the manifest and extract information from them...
    CParserTOML parser(rssManifest);
    auto ptrComponents = parser.GetRoot().GetDirect("Component");
    if (!ptrComponents) return; // No objects...
    auto ptrComponentArray = ptrComponents->GetArray();
    if (!ptrComponentArray) return; // No objects...
    for (uint32_t uiIndex = 0; uiIndex < ptrComponentArray->GetCount(); uiIndex++)
    {
        auto ptrComponent = ptrComponentArray->Get(uiIndex);
        if (!ptrComponent) continue;

        // Fill in the component structure
        SComponent sComponent{};
        //sComponent.pathModule = rpathModule;
        sComponent.pathRelModule = rpathRelModule;
        sComponent.ssManifest = ptrComponent->CreateTOMLText("Component");
        auto ptrClassName = ptrComponent->GetDirect("Class");
        if (!ptrClassName) continue;
        sComponent.ssClassName = static_cast<std::string>(ptrClassName->GetValue());
        auto ptrAliases = ptrComponent->GetDirect("Aliases");
        if (ptrAliases)
        {
            auto ptrAliasesArray = ptrAliases->GetArray();
            for (uint32_t uiAliasIndex = 0; ptrAliasesArray && uiAliasIndex < ptrAliasesArray->GetCount(); uiAliasIndex++)
            {
                auto ptrClassAlias = ptrAliasesArray->Get(uiAliasIndex);
                if (ptrClassAlias)
                    sComponent.seqAliases.push_back(static_cast<sdv::u8string>(ptrClassAlias->GetValue()));
            }
        }
        auto ptrDefaultName = ptrComponent->GetDirect("DefaultName");
        if (ptrDefaultName) sComponent.ssDefaultObjectName = static_cast<std::string>(ptrDefaultName->GetValue());
        else sComponent.ssDefaultObjectName = sComponent.ssClassName;
        auto ptrType = ptrComponent->GetDirect("Type");
        if (!ptrType) continue;
        std::string ssType = static_cast<std::string>(ptrType->GetValue());
        if (ssType == "System") sComponent.eType = sdv::EObjectType::SystemObject;
        else if (ssType == "Device") sComponent.eType = sdv::EObjectType::Device;
        else if (ssType == "BasicService") sComponent.eType = sdv::EObjectType::BasicService;
        else if (ssType == "ComplexService") sComponent.eType = sdv::EObjectType::ComplexService;
        else if (ssType == "App") sComponent.eType = sdv::EObjectType::Application;
        else if (ssType == "Proxy") sComponent.eType = sdv::EObjectType::Proxy;
        else if (ssType == "Stub") sComponent.eType = sdv::EObjectType::Stub;
        else if (ssType == "Utility") sComponent.eType = sdv::EObjectType::Utility;
        else continue;
        if (bBlockSystemObjects && sComponent.eType == sdv::EObjectType::SystemObject) continue;
        auto ptrSingleton = ptrComponent->GetDirect("Singleton");
        if (ptrSingleton && static_cast<bool>(ptrSingleton->GetValue()))
            sComponent.uiFlags = static_cast<uint32_t>(sdv::EObjectFlags::singleton);
        auto ptrDependencies = ptrComponent->GetDirect("Dependencies");
        if (ptrDependencies)
        {
            auto ptrDependencyArray = ptrDependencies->GetArray();
            for (uint32_t uiDependencyIndex = 0; ptrDependencyArray && uiDependencyIndex < ptrDependencyArray->GetCount();
                uiDependencyIndex++)
            {
                auto ptrDependsOn = ptrDependencyArray->Get(uiDependencyIndex);
                if (ptrDependsOn)
                    sComponent.seqDependencies.push_back(static_cast<sdv::u8string>(ptrDependsOn->GetValue()));
            }
        }

        vecComponents.push_back(sComponent);
    }
}

#if defined _WIN32 && defined __GNUC__
#pragma GCC diagnostic pop
#endif