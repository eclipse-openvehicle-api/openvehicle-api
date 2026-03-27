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

#include "installation_manifest.h"
#include <support/component_impl.h>
#include <support/serdes.h>
#include <support/toml.h>

#if defined _WIN32 && defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#ifdef __unix__
#include <dlfcn.h>
#endif

std::string ClassInfo2TOML(const sdv::SClassInfo& rsClass)
{
    toml_parser::CParser parser("");
    sdv::toml::CNodeCollection nodeRoot(&parser.Root());
    auto nodeClass = nodeRoot.AddTableArray("Class");
    if (!rsClass.ssModulePath.empty())
        nodeClass.AddValue("Path", rsClass.ssModulePath);
    if (rsClass.ssName.empty()) return {};
    nodeClass.AddValue("Name", rsClass.ssName);
    if (!rsClass.seqClassAliases.empty())
    {
        auto nodeAliases = nodeClass.AddArray("Aliases");
        for (const auto& rssAlias : rsClass.seqClassAliases)
            nodeAliases.AddValue("", rssAlias);
    }
    if (!rsClass.ssDefaultObjectName.empty() && rsClass.ssDefaultObjectName != rsClass.ssName)
        nodeClass.AddValue("DefaultName", rsClass.ssDefaultObjectName);
    if (!rsClass.ssDefaultConfig.empty())
        nodeClass.AddTOML(rsClass.ssDefaultConfig);
    if (rsClass.eType == sdv::EObjectType::undefined) return {};
    nodeClass.AddValue("Type", ::sdv::ObjectType2String(rsClass.eType));
    if (rsClass.uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
        nodeClass.AddValue("Singleton", true);
    if (!rsClass.seqDependencies.empty())
    {
        auto nodeDependencies = nodeClass.AddArray("Dependencies");
        for (const auto& rssDependency : rsClass.seqDependencies)
            nodeDependencies.AddValue("" , rssDependency);
    }

    return parser.GenerateTOML();
}

sdv::SClassInfo TOML2ClassInfo(const std::string& rssTOML, size_t nIndex /*= 0*/)
{
    toml_parser::CParser parser(rssTOML);
    return TOML2ClassInfo(parser.Root().Cast<toml_parser::CNodeCollection>(), nIndex);
}

sdv::SClassInfo TOML2ClassInfo(const std::shared_ptr<toml_parser::CNodeCollection>& rptrTOML, size_t nIndex /*= 0*/)
{
    if (!rptrTOML) return {};
    sdv::toml::CNodeCollection nodeRoot(rptrTOML.get());
    sdv::toml::CNodeCollection nodeClass = nodeRoot.GetDirect("Class[" + std::to_string(nIndex) + "]");
    if (!nodeClass)
        nodeClass = nodeRoot.GetDirect("Class");
    if (!nodeClass) return {};
    
    sdv::SClassInfo sClass{};
    sClass.ssModulePath = nodeClass.GetDirect("Path").GetValueAsString();
    sClass.ssName = nodeClass.GetDirect("Name").GetValueAsString();
    sdv::toml::CNodeCollection nodeAliases = nodeClass.GetDirect("Aliases");
    for (size_t nAliasIndex = 0; nAliasIndex < nodeAliases.GetCount(); nAliasIndex++)
        sClass.seqClassAliases.push_back(nodeAliases.Get(nAliasIndex).GetValueAsString());
    sClass.ssDefaultObjectName = nodeClass.GetDirect("DefaultName").GetValueAsString();
    sClass.ssDefaultConfig = nodeClass.GetDirect("Parameters").GetTOML();
    sClass.eType = sdv::String2ObjectType(nodeClass.GetDirect("Type").GetValue());
    if (static_cast<bool>(nodeClass.GetDirect("Singleton").GetValue()))
        sClass.uiFlags = static_cast<uint32_t>(sdv::EObjectFlags::singleton);
    sdv::toml::CNodeCollection nodeDependencies = nodeClass.GetDirect("Dependencies");
    for (size_t nDependencyIndex = 0; nDependencyIndex < nodeDependencies.GetCount(); nDependencyIndex++)
        sClass.seqDependencies.push_back(nodeDependencies.Get(nDependencyIndex).GetValueAsString());
    if (sClass.ssName.empty() || sClass.eType == sdv::EObjectType::undefined)
        return {};

    return sClass;
}

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
    toml_parser::CParser parser(rssManifest);
    sdv::toml::CNodeCollection nodeRoot(&parser.Root());

    // Get the installation version - must be identical to the interface version
    if (nodeRoot.GetDirect("Installation.Version").GetValue() != SDVFrameworkInterfaceVersion) return false;

    // Get the installation name
    m_ssInstallName = nodeRoot.GetDirect("Installation.Name").GetValueAsString();
    if (m_ssInstallName.empty()) return false;

    // Get installation properties. The properties are optional
    sdv::toml::CNodeCollection nodeProperties = nodeRoot.GetDirect("Properties");
    for (size_t nIndex = 0; nIndex < nodeProperties.GetCount(); nIndex++)
    {
        auto nodeProperty = nodeProperties.Get(nIndex);
        if (nodeProperty)
            m_mapProperties[nodeProperty.GetName()] = nodeProperty.GetValueAsString();
    }

    // Build the module list
    sdv::toml::CNodeCollection nodeModules = nodeRoot.GetDirect("Module");
    if (!nodeModules) return true;   // No modules in the manifest
    for (size_t nIndex = 0; nIndex < nodeModules.GetCount(); nIndex++)
    {
        // Get the module
        sdv::toml::CNodeCollection nodeModule = nodeModules.Get(nIndex);
        if (!nodeModule) continue;

        // Get the module path
        std::filesystem::path pathModule = nodeModule.GetDirect("Path").GetValueAsPath();
        std::string ssModuleManifest;

        // Get the class list (if available) and get the fitting TOML for the classes.
        sdv::toml::CNodeCollection nodeClasses = nodeModule.GetDirect("Class");
        if (nodeClasses) ssModuleManifest = nodeClasses.GetTOML();

        // Add the module
        m_vecModules.push_back(SModule(pathModule, ssModuleManifest, m_bBlockSystemObjects));
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
        toml_parser::CParser parser(rsEntry.ssManifest);

        // Add the module manifest as part of the installation manifest.
        sstream << parser.GenerateTOML("Module") << std::endl;
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
        toml_parser::CParser parser(ssManifest);
        auto ptrInterfaceNode = parser.Root().Direct("Interface.Version");
        if (!ptrInterfaceNode) return false;
        if (ptrInterfaceNode->GetValue() != SDVFrameworkInterfaceVersion) return false;
        auto ptrComponentsNode = parser.Root().Direct("Class");
        if (!ptrComponentsNode) return true;    // No component available in the manifest
        if (!ptrComponentsNode->Cast<toml_parser::CArray>()) return false;
        ssComponentsManifest = ptrComponentsNode->GenerateTOML();
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

std::optional<sdv::SClassInfo> CInstallManifest::FindComponentByClass(const std::string& rssClass) const
{
    // Search for the correct module
    sdv::SClassInfo sRet{};
    auto itModule = std::find_if(m_vecModules.begin(), m_vecModules.end(), [&](const SModule& rsEntry)
        {
            return std::find_if(rsEntry.vecClasses.begin(), rsEntry.vecClasses.end(),
                [&](const sdv::SClassInfo& sClass)
                {
                    // Note, use the class, alias and the default object name for searching...
                    if (sClass.ssName == rssClass ||
                        std::find(sClass.seqClassAliases.begin(), sClass.seqClassAliases.end(), rssClass) !=
                            sClass.seqClassAliases.end())
                    {
                        sRet = sClass;
                        return true;
                    }
                    return false;
                }) != rsEntry.vecClasses.end();
        });
    if (itModule != m_vecModules.end()) return sRet;
    return {};
}

std::vector<sdv::SClassInfo> CInstallManifest::ClassList() const
{
    std::vector<sdv::SClassInfo> vecClasses;
    for (const auto& rsModule : m_vecModules)
    {
        for (const auto& rsComponent : rsModule.vecClasses)
            vecClasses.push_back(rsComponent);
    }
    return vecClasses;
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
        if (static_cast<uint8_t>(c) > 127u || (!std::isalnum(c) && c != '_' && c != '-'))
            return true;
    }
    return false;
}

CInstallManifest::SModule::SModule(const std::filesystem::path& rpathRelModule, const std::string& rssManifest,
    bool bBlockSystemObjects) : pathRelModule(rpathRelModule), ssManifest(rssManifest)
{
    // Parse the manifest and extract information from them...
    toml_parser::CParser parser(rssManifest);
    auto ptrClasses = parser.Root().Direct("Class");
    if (!ptrClasses) return; // No objects...
    auto ptrClassArray = ptrClasses->Cast<toml_parser::CArray>();
    if (!ptrClassArray) return; // No objects...
    for (uint32_t uiIndex = 0; uiIndex < ptrClassArray->GetCount(); uiIndex++)
    {
        //auto ptrClass = ptrClassArray->Get(uiIndex);
        //if (!ptrClass) continue;

        //// Fill in the component structure
        //sdv::SClassInfo sClass{};
        ////sClass.pathModule = rpathModule;
        //sClass.ssModulePath = rpathRelModule.generic_u8string();
        ////sClass.ssManifest = ptrClass->GenerateTOML(toml_parser::CGenContext("Class"));
        //auto ptrClassName = ptrClass->Direct("Name");
        //if (!ptrClassName) continue;
        //sClass.ssName = ptrClassName->GetValue();
        //auto ptrAliases = ptrClass->Direct("Aliases");
        //if (ptrAliases)
        //{
        //    auto ptrAliasesArray = ptrAliases->Cast<toml_parser::CArray>();
        //    for (uint32_t uiAliasIndex = 0; ptrAliasesArray && uiAliasIndex < ptrAliasesArray->GetCount(); uiAliasIndex++)
        //    {
        //        auto ptrClassAlias = ptrAliasesArray->Get(uiAliasIndex);
        //        if (ptrClassAlias)
        //            sClass.seqClassAliases.push_back(static_cast<sdv::u8string>(ptrClassAlias->GetValue()));
        //    }
        //}
        //auto ptrDefaultName = ptrClass->Direct("DefaultName");
        //if (ptrDefaultName) sClass.ssDefaultObjectName = ptrDefaultName->GetValue();
        //else sClass.ssDefaultObjectName = sClass.ssName;
        //auto ptrType = ptrClass->Direct("Type");
        //if (!ptrType) continue;
        //sClass.eType = sdv::String2ObjectType(ptrType->GetValue());
        //if (sClass.eType == sdv::EObjectType::Undefined) continue;
        //if (bBlockSystemObjects && sClass.eType == sdv::EObjectType::system_object) continue;
        //auto ptrSingleton = ptrClass->Direct("Singleton");
        //if (ptrSingleton && static_cast<bool>(ptrSingleton->GetValue()))
        //    sClass.uiFlags = static_cast<uint32_t>(sdv::EObjectFlags::singleton);
        //auto ptrDependencies = ptrClass->Direct("Dependencies");
        //if (ptrDependencies)
        //{
        //    auto ptrDependencyArray = ptrDependencies->Cast<toml_parser::CArray>();
        //    for (uint32_t uiDependencyIndex = 0; ptrDependencyArray && uiDependencyIndex < ptrDependencyArray->GetCount();
        //        uiDependencyIndex++)
        //    {
        //        auto ptrDependsOn = ptrDependencyArray->Get(uiDependencyIndex);
        //        if (ptrDependsOn)
        //            sClass.seqDependencies.push_back(static_cast<sdv::u8string>(ptrDependsOn->GetValue()));
        //    }
        //}

        //vecClasses.push_back(sClass);

        auto sClass = TOML2ClassInfo(parser.Root().Cast<toml_parser::CNodeCollection>(), uiIndex);
        if (bBlockSystemObjects && sClass.eType == sdv::EObjectType::system_object) continue;
        if (sClass.eType == sdv::EObjectType::undefined) continue;
        sClass.ssModulePath = rpathRelModule.generic_u8string();
        vecClasses.push_back(sClass);
    }
}

#if defined _WIN32 && defined __GNUC__
#pragma GCC diagnostic pop
#endif