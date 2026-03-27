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

#include "module.h"
#include <support/component_impl.h>
#include <support/local_service_access.h>
#include <functional>
#include <algorithm>
#include "toml_parser/parser_toml.h"
#include "repository.h"

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#pragma push_macro("GetObject")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <objbase.h>

// Resolve conflict
#pragma pop_macro("interface")
#pragma pop_macro("GetObject")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#endif // defined _WIN32

#ifdef __unix__
#include <dlfcn.h>
#endif

CModuleInst::CModuleInst(const std::filesystem::path& rpathModuleConfigPath, const std::filesystem::path& rpathModule) noexcept :
    m_pathModuleConfigPath(rpathModuleConfigPath)
{
    if (!rpathModule.empty())
        Load(rpathModule);
}

CModuleInst::~CModuleInst()
{
    Unload(true);
}

bool CModuleInst::IsValid() const noexcept
{
    return m_tModuleID != 0 && m_pFactory;
}

std::string CModuleInst::GetDefaultObjectName(const std::string& ssClassName) const
{
    auto itClass = m_mapClassInfo.find(ssClassName);
    if (itClass == m_mapClassInfo.end()) return std::string();
    return itClass->second.ssDefaultObjectName.empty() ? itClass->second.ssName : itClass->second.ssDefaultObjectName;
}

bool CModuleInst::IsSingleton(const std::string& ssClassName) const
{
    auto itClass = m_mapClassInfo.find(ssClassName);
    if (itClass == m_mapClassInfo.end()) return false;
    return (itClass->second.uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton)) ? true : false;
}

std::vector<std::string> CModuleInst::GetAvailableClasses() const
{
    std::vector<std::string> vecClasses;
    for (const auto& rvtClass : m_mapClassInfo)
        vecClasses.push_back(rvtClass.first);
    return vecClasses;
}

sdv::IInterfaceAccess* CModuleInst::CreateObject(const std::string& ssClassName)
{
    if(!m_pFactory)
    {
        return nullptr;
    }
    return m_pFactory->CreateObject(ssClassName.c_str());
}

void CModuleInst::DestroyObject(sdv::IInterfaceAccess* object)
{
    if(m_pFactory)
    {
        m_pFactory->DestroyObject(object);
    }
}

std::filesystem::path CModuleInst::GetModuleConfigPath() const
{
    return m_pathModuleConfigPath;
}

std::filesystem::path CModuleInst::GetModulePath() const
{
    return m_pathModule;
}

sdv::core::TModuleID CModuleInst::GetModuleID() const
{
    return m_tModuleID;
}

sdv::core::SModuleInfo CModuleInst::GetModuleInfo() const
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModule);
    sdv::core::SModuleInfo sClass{};
    sClass.tModuleID = m_tModuleID;
    sClass.ssPath = m_pathModule.filename().generic_u8string();
    sClass.ssFilename = m_pathModule.filename().generic_u8string();
    sClass.uiVersion = m_uiIfcVersion;
    sClass.bActive = m_fnActiveObjects ? m_fnActiveObjects() : false;
    return sClass;
}

std::optional<sdv::SClassInfo> CModuleInst::GetClassInfo(const std::string& rssClassName) const
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModule);
    const auto itClass = m_mapClassInfo.find(rssClassName);
    if (itClass == m_mapClassInfo.end()) return {};
    std::optional<sdv::SClassInfo> optClassInfo = itClass->second;

    // Add the module path to the result
    optClassInfo->ssModulePath = m_pathModule.generic_u8string();

    return itClass->second;
}

bool CModuleInst::Load(const std::filesystem::path& rpathModule) noexcept
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModule);

    // Cannot load again.
    if (m_tModuleID) return false;
    m_pathModule = rpathModule;

    // Core services bypass.
    if (rpathModule == "core_services.sdv")
    {
        m_tModuleID = sdv::core::tCoreLibModule;
        m_fnActiveObjects = &::HasActiveObjects;
        m_fnGetFactory = &::GetModuleFactory;
        m_fnGetManifest = &::GetManifest;
    }
    else
    {
#ifdef _WIN32
        m_tModuleID = reinterpret_cast<uint64_t>(LoadLibrary(rpathModule.native().c_str()));
#elif defined __unix__
        m_tModuleID = reinterpret_cast<uint64_t>(dlopen(rpathModule.native().c_str(), RTLD_LAZY));
#else
#error OS not supported!
#endif
        if (!m_tModuleID)
        {
#ifdef _WIN32
            std::string ssError(1024, '\0');
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                &ssError.front(),
                static_cast<DWORD>(ssError.size()), NULL);
            ssError[1023] = '\0';
            ssError.resize(strlen(ssError.c_str()));
#elif defined __unix__
            std::string ssError = dlerror();
#else
#error OS not supported!
#endif
            SDV_LOG_ERROR("Error opening SDV module: \"", rpathModule.generic_u8string(), "\" error: ", ssError);
            return 0;
        }

        // Check whether the module exposes the necessary functions
#ifdef _WIN32
        m_fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(GetProcAddress(reinterpret_cast<HMODULE>(m_tModuleID), "GetModuleFactory"));
        m_fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(GetProcAddress(reinterpret_cast<HMODULE>(m_tModuleID), "HasActiveObjects"));
        m_fnGetManifest = reinterpret_cast<TFNGetManifest*>(GetProcAddress(reinterpret_cast<HMODULE>(m_tModuleID), "GetManifest"));
#elif defined __unix__
        m_fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(dlsym(reinterpret_cast<void*>(m_tModuleID), "GetModuleFactory"));
        m_fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(dlsym(reinterpret_cast<void*>(m_tModuleID), "HasActiveObjects"));
        m_fnGetManifest = reinterpret_cast<TFNGetManifest*>(dlsym(reinterpret_cast<void*>(m_tModuleID), "GetManifest"));
#else
#error OS not supported!
#endif
        if (!m_fnGetFactory || !m_fnActiveObjects || !m_fnGetManifest || !m_fnGetManifest())
        {
            SDV_LOG_ERROR("Error opening SDV module: ", rpathModule.generic_u8string(),
                " error: The module doesn't expose access functions!");
            Unload(true);
            return 0;
        }
    }

    // Get the manifest
    std::string ssManifest = m_fnGetManifest();

    try
    {
        toml_parser::CParser parser(ssManifest);

        // Check for the interface version - currently must be equal.
        auto ptrVersion = parser.Root().Direct("Interface.Version");
        if (ptrVersion) m_uiIfcVersion = ptrVersion->GetValue();
        if (!ptrVersion || m_uiIfcVersion != SDVFrameworkInterfaceVersion)
        {
            // Incompatible interface.
            SDV_LOG_ERROR("Error opening SDV module: ", rpathModule.generic_u8string(), " error: incompatible version ", m_uiIfcVersion,
                " (required ", SDVFrameworkInterfaceVersion, ")");
            Unload(true);
            return false;
        }

        // Get a pointer to the factory for the current interface.
        m_pFactory = sdv::TInterfaceAccessPtr(m_fnGetFactory(SDVFrameworkInterfaceVersion)).GetInterface<sdv::IObjectFactory>();
        if (!m_pFactory)
        {
            SDV_LOG_ERROR("Error opening SDV module: ", rpathModule.generic_u8string(), " error: no object factory available");
            Unload(true);
            return false;
        }

        // Get available classes
        auto ptrClasses = parser.Root().Direct("Class");
        if (!ptrClasses || !ptrClasses->Cast<toml_parser::CArray>())
        {
            SDV_LOG_ERROR("Error opening SDV module: ", rpathModule.generic_u8string(), " error: no components available");
            Unload(true);
            return false;
        }
        for (std::uint32_t uiIndex = 0; uiIndex < ptrClasses->Cast<toml_parser::CArray>()->GetCount(); uiIndex++)
        {
            // Fill in the class info.
            sdv::SClassInfo sClass{};
            auto ptrClass = ptrClasses->Cast<toml_parser::CArray>()->Get(uiIndex);
            if (!ptrClass) continue;
            auto ptrClassName = ptrClass->Direct("Name");
            if (!ptrClassName) continue;
            sClass.ssName = static_cast<std::string>(ptrClassName->GetValue());
            auto ptrAliases = ptrClass->Direct("Aliases");
            if (ptrAliases)
            {
                auto ptrAliasesArray = ptrAliases->Cast<toml_parser::CArray>();
                for (uint32_t uiAliasIndex = 0; ptrAliasesArray && uiAliasIndex < ptrAliasesArray->GetCount(); uiAliasIndex++)
                {
                    auto ptrClassAlias = ptrAliasesArray->Get(uiAliasIndex);
                    if (ptrClassAlias)
                        sClass.seqClassAliases.push_back(static_cast<sdv::u8string>(ptrClassAlias->GetValue()));
                }
            }
            auto ptrDefaultName = ptrClass->Direct("DefaultName");
            if (ptrDefaultName) sClass.ssDefaultObjectName = static_cast<std::string>(ptrDefaultName->GetValue());
            else sClass.ssDefaultObjectName = sClass.ssName;
            auto ptrType = ptrClass->Direct("Type");
            if (!ptrType) continue;
            sClass.eType = sdv::String2ObjectType(ptrType->GetValue());
            if (sClass.eType == sdv::EObjectType::undefined)
                continue;
            auto ptrSingleton = ptrClass->Direct("Singleton");
            if (ptrSingleton && static_cast<bool>(ptrSingleton->GetValue()))
                sClass.uiFlags = static_cast<uint32_t>(sdv::EObjectFlags::singleton);
            auto ptrDependencies = ptrClass->Direct("Dependencies");
            if (ptrDependencies)
            {
                auto ptrDependencyArray = ptrDependencies->Cast<toml_parser::CArray>();
                for (uint32_t uiDependencyIndex = 0; ptrDependencyArray && uiDependencyIndex < ptrDependencyArray->GetCount();
                    uiDependencyIndex++)
                {
                    auto ptrDependsOn = ptrDependencyArray->Get(uiDependencyIndex);
                    if (ptrDependsOn)
                        sClass.seqDependencies.push_back(static_cast<sdv::u8string>(ptrDependsOn->GetValue()));
                }
            }

            m_mapClassInfo[sClass.ssName] = sClass;
        }
    }
    catch (const sdv::toml::XTOMLParseException&)
    {
        SDV_LOG_ERROR("Manifest interpretation error for: \"", m_pathModule.filename().generic_u8string(), "\"");
        return false;
    }

    SDV_LOG_INFO("Successfully loaded SDV module: \"", m_pathModule.filename().generic_u8string(), "\"");

    return true;
}

bool CModuleInst::Unload(bool bForce) noexcept
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModule);

    // Request the repository to unload any running objects
    try
    {
        GetRepository().DestroyModuleObjects(m_tModuleID);
    }
    catch (const sdv::XSysExcept&)
    {
    }
    catch (const std::exception&)
    {
    }

    // Check for a active objects first
    bool bHasActive = m_fnActiveObjects && m_fnActiveObjects();
    if (!bForce && bHasActive)
    {
        SDV_LOG_WARNING("Trying to unload module \"", m_pathModule.filename().generic_u8string(),
            "\" but there are still active objects. Unloading the module will be blocked.");
        return false;
    }

    // Clear the members
    m_fnActiveObjects = nullptr;
    m_fnGetFactory = nullptr;
    m_pFactory = nullptr;
    m_mapClassInfo.clear();
    m_uiIfcVersion = 0u;

    // Unload the module
    if (!bHasActive)
    {
        if (m_tModuleID && m_tModuleID != sdv::core::tCoreLibModule)
        {
#ifdef _WIN32
            FreeLibrary(reinterpret_cast<HMODULE>(m_tModuleID));
#elif defined __unix__
            dlclose(reinterpret_cast<void*>(m_tModuleID));
#else
#error OS not supported!
#endif
            SDV_LOG_INFO("Successfully unloaded SDV module: \"", m_pathModule.filename().generic_u8string(), "\"");
        }
    } else
    {
        SDV_LOG_ERROR("Trying to force to unload module \"", m_pathModule.filename().generic_u8string(),
            "\" but there are still active objects. The module will not be unloaded! "
            "However, the management instance will be destroyed!");
    }

    // Clear other members
    m_tModuleID = 0;
    m_pathModule.clear();

    return true;
}

bool CModuleInst::HasActiveObjects() const
{
    std::unique_lock<std::recursive_mutex> lock(m_mtxModule);

    return m_fnActiveObjects && m_fnActiveObjects();
}

#if defined _WIN32 && defined __GNUC__
#pragma GCC diagnostic pop
#endif
