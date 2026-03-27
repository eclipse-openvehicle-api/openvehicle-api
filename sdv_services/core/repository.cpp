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

#include "repository.h"
#include "support/interface_ptr.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include "object_lifetime_control.h"
#include "../../global/base64.h"
#include "module_control.h"
#include "app_config.h"
#include "app_control.h"
#include "app_settings.h"

// GetRepository might be redirected for unit tests.
#ifndef GetRepository
CRepository& GetRepository()
{
    static CRepository repository;
    return repository;
}
#endif // !defined GetRepository

void CRepository::SetConfigMode()
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    for (auto& rvtObjectEntry : m_mapObjects)
    {
        sdv::IObjectControl* pObjectControl = rvtObjectEntry.second->ptrObject.GetInterface<sdv::IObjectControl>();
        if (pObjectControl) pObjectControl->SetOperationMode(sdv::EOperationMode::configuring);
    }
}

void CRepository::SetRunningMode()
{
    // Create a copy of the object map. This is needed to prevent running in a deadlock while getting IObjectControl or setting the
    // operation mode (which might trigger repository action again).
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    auto mapObjectsCopy = m_mapObjects;
    lock.unlock();

    // Set the operation mode for each object.
    for (auto& rvtObjectEntry : mapObjectsCopy)
    {
        sdv::IObjectControl* pObjectControl = rvtObjectEntry.second->ptrObject.GetInterface<sdv::IObjectControl>();
        if (pObjectControl) pObjectControl->SetOperationMode(sdv::EOperationMode::running);
    }
}

sdv::IInterfaceAccess* CRepository::GetObject(const sdv::u8string& ssObjectName)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    auto itService = m_mapServiceObjects.find(ssObjectName);
    if (itService != m_mapServiceObjects.end()) return (*itService->second)->ptrObject;
    lock.unlock();

    // In case the object is not in the service map and this is a main or isolated application, create the object if the object is
    // known in the installation manifest and is a system object.
    auto optManifest = GetAppConfig().FindInstalledComponent(ssObjectName);
    if (optManifest && optManifest->eType == sdv::EObjectType::system_object)
        return GetObjectByID(CreateObject(optManifest->ssName, optManifest->ssDefaultObjectName, ""));

    // Forward the request to core repository if one is linked here (this can only occur with isolated and external applications).
    if (!m_ptrCoreRepoAccess) return nullptr;
    sdv::core::IObjectAccess* pObjectAccess = m_ptrCoreRepoAccess.GetInterface<sdv::core::IObjectAccess>();
    if (!pObjectAccess) return nullptr;
    return pObjectAccess->GetObject(ssObjectName);
}

sdv::IInterfaceAccess* CRepository::GetObjectByID(/*in*/ sdv::core::TObjectID tObjectID)
{
    // Only controlled objects are allowed to be returned using GetObjectByID.
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    auto itObject = m_mapObjects.find(tObjectID);
    if (itObject != m_mapObjects.end())
        return itObject->second->bControlled ? itObject->second->ptrObject : nullptr;

    // TODO: Deal with overlapping IDs in this and in core process...

    // Forward the request to core repository if one is linked here (this can only occur with isolated and external applications).
    if (!m_ptrCoreRepoAccess) return nullptr;
    sdv::core::IObjectAccess* pObjectAccess = m_ptrCoreRepoAccess.GetInterface<sdv::core::IObjectAccess>();
    if (!pObjectAccess) return nullptr;
    return pObjectAccess->GetObjectByID(tObjectID);
}

sdv::IInterfaceAccess* CRepository::CreateUtility(/*in*/ const sdv::u8string& ssClassName, /*in*/ const sdv::u8string& ssObjectConfig)
{
    if(ssClassName.empty()) return nullptr;

    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);

    // Get a fitting module instance
    std::shared_ptr<CModuleInst> ptrModule = GetModuleControl().FindModuleByClass(ssClassName);
    if (!ptrModule)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }

    // Check the class type
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }
    if (optClassInfo->eType != sdv::EObjectType::utility)
    {
        // Utilities and marshall objects cannot be created using the CreateObject function.
        SDV_LOG_ERROR("Creation of an utility of invalid type \"", sdv::ObjectType2String(optClassInfo->eType),
            "\" requested for class \"", ssClassName, "\"!");
        return nullptr;
    }

    // Check with singleton objects. Utilities cannot be singleton.
    if (optClassInfo->uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
    {
        SDV_LOG_ERROR("Utility creation requested but utility is marked as singleton. This is not possible for utilities!");
        return nullptr;
    }

    // Create the utility locally
    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrObjectEntry)
    {
        SDV_LOG_ERROR("Failed to create object management structure!");
        return nullptr;
    }
    ptrObjectEntry->tObjectID = CreateObjectID();
    ptrObjectEntry->sClassInfo = *optClassInfo;
    ptrObjectEntry->ssName = ssClassName;
    ptrObjectEntry->ssConfig = ssObjectConfig;
    ptrObjectEntry->ptrModule = ptrModule;
    lock.unlock();

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Creating a utility #" << ptrObjectEntry->tObjectID << " of type " << ptrObjectEntry->ssName << std::endl;

    // Create the object
    sdv::TInterfaceAccessPtr ptrObject = ptrModule->CreateObject(ssClassName);
    if (!ptrObject)
        return nullptr;

    // Try to initialize
    auto* pObjectControl = ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl)
    {
        pObjectControl->Initialize(ssObjectConfig);
        if (pObjectControl->GetObjectState() != sdv::EObjectState::initialized)
        {
            // Destroy the object
            ptrModule->DestroyObject(ptrObject);
            return nullptr;
        }
        if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::running)
            pObjectControl->SetOperationMode(sdv::EOperationMode::running);
        else if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::configuring)
            pObjectControl->SetOperationMode(sdv::EOperationMode::configuring);
    }

    // Everything good, assign the object instance
    lock.lock();
    ptrObjectEntry->ptrObject = ptrObject;
    m_mapLocalObjects[ptrObject] = ptrObjectEntry;
    m_mapObjects[ptrObjectEntry->tObjectID] = std::move(ptrObjectEntry);

    return new CObjectLifetimeControl(ptrObject, *this);
}

sdv::IInterfaceAccess* CRepository::CreateProxyObject(/*in*/ sdv::interface_id id)
{
    // Prevent object creation when not started or when shutting down.
    if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::shutting_down ||
        GetAppControl().GetOperationState() == sdv::app::EAppOperationState::not_started)
        return nullptr;

    // Get a fitting module instance
    std::string ssClassName = "Proxy_" + std::to_string(id);
    std::shared_ptr<CModuleInst> ptrModule = GetModuleControl().FindModuleByClass(ssClassName);
    if (!ptrModule && m_ptrCoreRepoAccess)
    {
        // Request the server for the name of the module.
        const sdv::core::IRepositoryInfo* pRepInfo = m_ptrCoreRepoAccess.GetInterface<sdv::core::IRepositoryInfo>();
        if (pRepInfo)
        {
            std::string ssModuleName =
                std::filesystem::u8path(static_cast<std::string>(pRepInfo->FindClass(ssClassName).ssModulePath)).
                    filename().u8string();
            if (!ssModuleName.empty())
            {
                sdv::core::TModuleID tModule = GetModuleControl().Load(ssModuleName);
                if (tModule) ptrModule = GetModuleControl().GetModule(tModule);
            }
        }
    }
    if (!ptrModule)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }

    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);

    // Check the class type
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }
    if (optClassInfo->eType != sdv::EObjectType::proxy)
    {
        // Utilities and marshall objects cannot be created using the CreateObject function.
        SDV_LOG_ERROR("Creation of an object of invalid type \"", sdv::ObjectType2String(optClassInfo->eType),
            "\" requested for class \"", ssClassName, "\"!");
        return nullptr;
    }

    // Check with singleton objects. Marshall objects cannot be singleton.
    if (optClassInfo->uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
    {
        SDV_LOG_ERROR("Marshalling object creation requested but object is marked as singleton. This is not possible for "
            "marshalling objects!");
        return nullptr;
    }

    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrObjectEntry)
    {
        SDV_LOG_ERROR("Failed to create object management structure!");
        return nullptr;
    }
    ptrObjectEntry->tObjectID = CreateObjectID();
    ptrObjectEntry->sClassInfo = *optClassInfo;
    ptrObjectEntry->ssName = ssClassName;
    ptrObjectEntry->ssConfig = "";
    ptrObjectEntry->ptrModule = ptrModule;
    lock.unlock();

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Creating a proxy object #" << ptrObjectEntry->tObjectID << " of type " <<
            ptrObjectEntry->ssName << std::endl;

    // Create the object
    sdv::TInterfaceAccessPtr ptrObject = ptrModule->CreateObject(ssClassName);
    if (!ptrObject)
        return nullptr;

    // Try to initialize
    auto* pObjectControl = ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl)
    {
        pObjectControl->Initialize("");
        if (pObjectControl->GetObjectState() != sdv::EObjectState::initialized)
        {
            // Destroy the object
            ptrModule->DestroyObject(ptrObject);
            return nullptr;
        }
        if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::running)
            pObjectControl->SetOperationMode(sdv::EOperationMode::running);
        else if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::configuring)
            pObjectControl->SetOperationMode(sdv::EOperationMode::configuring);
    }

    // Everything good, assign the object instance
    lock.lock();
    ptrObjectEntry->ptrObject = ptrObject;
    m_mapLocalObjects[ptrObject] = ptrObjectEntry;
    m_mapObjects[ptrObjectEntry->tObjectID] = std::move(ptrObjectEntry);

    return new CObjectLifetimeControl(ptrObject, *this);
}

sdv::IInterfaceAccess* CRepository::CreateStubObject(/*in*/ sdv::interface_id id)
{
    // Prevent object creation when not started or when shutting down.
    if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::shutting_down ||
        GetAppControl().GetOperationState() == sdv::app::EAppOperationState::not_started)
        return nullptr;

    // Get a fitting module instance
    std::string ssClassName = "Stub_" + std::to_string(id);
    std::shared_ptr<CModuleInst> ptrModule = GetModuleControl().FindModuleByClass(ssClassName);
    if (!ptrModule && m_ptrCoreRepoAccess)
    {
        // Request the server for the name of the module.
        const sdv::core::IRepositoryInfo* pRepInfo = m_ptrCoreRepoAccess.GetInterface<sdv::core::IRepositoryInfo>();
        if (pRepInfo)
        {
            std::string ssModuleName =
                std::filesystem::u8path(static_cast<std::string>(pRepInfo->FindClass(ssClassName).ssModulePath)).
                    filename().u8string();
            if (!ssModuleName.empty())
            {
                sdv::core::TModuleID tModule = GetModuleControl().Load(ssModuleName);
                if (tModule) ptrModule = GetModuleControl().GetModule(tModule);
            }
        }
    }
    if (!ptrModule)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }

    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);

    // Check the class type
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return nullptr;
    }
    if (optClassInfo->eType != sdv::EObjectType::stub)
    {
        // Utilities and marshall objects cannot be created using the CreateObject function.
        SDV_LOG_ERROR("Creation of an object of invalid type \"", sdv::ObjectType2String(optClassInfo->eType),
            "\" requested for class \"", ssClassName, "\"!");
        return nullptr;
    }

    // Check with singleton objects. Marshall objects cannot be singleton.
    if (optClassInfo->uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
    {
        SDV_LOG_ERROR("Marshalling object creation requested but object is marked as singleton. This is not possible for "
            "marshalling objects!");
        return nullptr;
    }

    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrObjectEntry)
    {
        SDV_LOG_ERROR("Failed to create object management structure!");
        return nullptr;
    }
    ptrObjectEntry->tObjectID = CreateObjectID();
    ptrObjectEntry->sClassInfo = *optClassInfo;
    ptrObjectEntry->ssName = ssClassName;
    ptrObjectEntry->ssConfig = "";
    ptrObjectEntry->ptrModule = ptrModule;
    lock.unlock();

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Creating a stub #" << ptrObjectEntry->tObjectID << " of type " << ptrObjectEntry->ssName << std::endl;

    // Create the object
    sdv::TInterfaceAccessPtr ptrObject = ptrModule->CreateObject(ssClassName);
    if (!ptrObject)
        return nullptr;

    // Try to initialize
    auto* pObjectControl = ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl)
    {
        pObjectControl->Initialize("");
        if (pObjectControl->GetObjectState() != sdv::EObjectState::initialized)
        {
            // Destroy the object
            ptrModule->DestroyObject(ptrObject);
            return nullptr;
        }
        if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::running)
            pObjectControl->SetOperationMode(sdv::EOperationMode::running);
        else if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::configuring)
            pObjectControl->SetOperationMode(sdv::EOperationMode::configuring);
    }

    // Everything good, assign the object instance
    lock.lock();
    ptrObjectEntry->ptrObject = ptrObject;
    m_mapLocalObjects[ptrObject] = ptrObjectEntry;
    m_mapObjects[ptrObjectEntry->tObjectID] = std::move(ptrObjectEntry);

    return new CObjectLifetimeControl(ptrObject, *this);
}

sdv::core::TObjectID CRepository::CreateObject(const sdv::u8string& ssClassName, const sdv::u8string& ssObjectName,
    const sdv::u8string& ssObjectConfig)
{


    /*
    TODO EVE
    Check for...
    - main app and iso: only complex service
    - ...
    - running in config mode?

    Build dependency list and do these checks on dependent objects as well
    */
    return CreateObject2(ssClassName, ssObjectName, ssObjectConfig);
}

sdv::core::TObjectID CRepository::CreateObject2(const sdv::u8string& ssClassName, const sdv::u8string& ssObjectName,
    const sdv::u8string& ssObjectConfig)
{
    // TODO EVE: Link to core repo. Allow only creation of one object if not already created before...
    // Add support for automatic app control shutdown when utility is closed in case of isolated app.

    // Prevent object creation when not started or when shutting down.
    if (GetAppControl().GetOperationState() == sdv::app::EAppOperationState::shutting_down ||
        GetAppControl().GetOperationState() == sdv::app::EAppOperationState::not_started)
        return false;

    if(ssClassName.empty()) return false;

    // Get a fitting module instance
    std::shared_ptr<CModuleInst> ptrModule = GetModuleControl().FindModuleByClass(ssClassName);
    if (!ptrModule)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return false;
    }

    // TODO: Request the parameters from the class info and create a config with them. Then add the parameters from the supplied
    // config and overwrite existing and extend non-existing parameters. Then create a new config string.

    // Check the class type
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return false;
    }
    bool bError = false;        // Creation not allowed
    bool bIsolate = false;      // Creation should be isolated
    bool bDeviceAndBasicServiceAllowed = GetAppSettings().IsMainApplication() || GetAppSettings().IsStandaloneApplication() ||
        GetAppSettings().IsEssentialApplication();
    bool bComplexServiceAllowed = !GetAppSettings().IsMaintenanceApplication() &&
        ((!GetAppSettings().IsIsolatedApplication() && !GetAppSettings().IsExternalApplication()) || !m_bIsoObjectLoaded);
    switch (optClassInfo->eType)
    {
    case sdv::EObjectType::system_object:
        break;
    case sdv::EObjectType::device:
    case sdv::EObjectType::platform_abstraction:
    case sdv::EObjectType::vehicle_bus:
        bError = !bDeviceAndBasicServiceAllowed;
        break;
    case sdv::EObjectType::basic_service:
    case sdv::EObjectType::sensor:
    case sdv::EObjectType::actuator:
        bError = !bDeviceAndBasicServiceAllowed;
        break;
    case sdv::EObjectType::complex_service:
    case sdv::EObjectType::vehicle_function:
        bIsolate = GetAppSettings().IsMainApplication();
        bError = !bComplexServiceAllowed;
        m_bIsoObjectLoaded = true;
        break;
    default:
        bError = true;
        break;
    }
    if (bError)
    {
        // Utilities and marshall objects cannot be created using the CreateObject function.
        SDV_LOG_ERROR("Creation of an object of invalid/unallowed type \"", sdv::ObjectType2String(optClassInfo->eType),
            "\" requested for class \"", ssClassName, "\"!");
        return 0;
    }

    // Check for an object name. If not existing get the default name (being either one specified by the object or the class name).
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    std::string ssObjectName2 = ssObjectName;
    if (ssObjectName2.empty())
        ssObjectName2 = optClassInfo->ssDefaultObjectName;
    if (ssObjectName2.empty())
        ssObjectName2 = ssClassName;

    // Check for an object with the same name.
    auto itPreviousService = m_mapServiceObjects.find(ssObjectName2);
    if (itPreviousService != m_mapServiceObjects.end())
    {
        // Object entry is valid?
        if (!*itPreviousService->second)
        {
            // This should not occur... there is a previous object in the service map, but the object is empty.
            SDV_LOG_ERROR("Object creation requested for class \"", ssClassName, "\", but object with the same name \"",
                ssObjectName2, "\" was already instantiated, but cannot be found!");
            return 0;
        }

        // Trying to create an object with the same name is not an error if the classes are identical.
        if ((*itPreviousService->second)->sClassInfo.ssName == ssClassName)
            return (*itPreviousService->second)->tObjectID;

        // Object name was already used by another class. This is an error.
        SDV_LOG_ERROR("Object creation requested for class \"", ssClassName, "\", but object with the same name \"",
            ssObjectName2, "\" was already instantiated for class \"", (*itPreviousService->second)->sClassInfo.ssName,
            "\"!");
        return 0;
    }

    // Check with singleton objects if the object was already instantiated.
    if (optClassInfo->uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
    {
        for (const auto& rprService : m_mapServiceObjects)
        {
            if (!*rprService.second)
                continue;
            if ((*rprService.second)->ptrModule && (*rprService.second)->ptrModule->GetModuleID() == ptrModule->GetModuleID()
                && (*rprService.second)->sClassInfo.ssName == ssClassName)
            {
                SDV_LOG_ERROR("Object creation requested but object from the same class \"", ssClassName, 
                    "\" was already instantiated and only one instance is allowed!");
                return 0;
            }
        }
    }

    // Unlock; the next function will lock again.
    lock.unlock();

    // Create an isolated object.
    if (bIsolate)
        return CreateIsolatedObject(*optClassInfo, ssObjectName2, ssObjectConfig);

    // Create an internal object.
    return InternalCreateObject(ptrModule, *optClassInfo, ssObjectName2, ssObjectConfig);
}

sdv::core::TObjectID CRepository::CreateObjectFromModule(/*in*/ sdv::core::TModuleID tModuleID,
    /*in*/ const sdv::u8string& ssClassName, /*in*/ const sdv::u8string& ssObjectName, /*in*/ const sdv::u8string& ssObjectConfig)
{
    if(ssClassName.empty()) return false;

    // Get a fitting module instance
    std::shared_ptr<CModuleInst> ptrModule = GetModuleControl().GetModule(tModuleID);
    if (!ptrModule)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return false;
    }

    // Check the class type
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo)
    {
        SDV_LOG_ERROR("Object creation requested but object class was not found \"", ssClassName, "\"!");
        return false;
    }
    bool bError = false;
    bool bDeviceAndBasicServiceAllowed = GetAppSettings().IsMainApplication() || GetAppSettings().IsStandaloneApplication() ||
        GetAppSettings().IsEssentialApplication();
    bool bComplexServiceAllowed = !GetAppSettings().IsMaintenanceApplication() &&
        ((!GetAppSettings().IsIsolatedApplication() && !GetAppSettings().IsExternalApplication()) || !m_bIsoObjectLoaded);
    switch (optClassInfo->eType)
    {
    case sdv::EObjectType::system_object:
        break;
    case sdv::EObjectType::device:
    case sdv::EObjectType::platform_abstraction:
    case sdv::EObjectType::vehicle_bus:
        // Allowed?
        bError = !bDeviceAndBasicServiceAllowed;
        break;
    case sdv::EObjectType::basic_service:
    case sdv::EObjectType::sensor:
    case sdv::EObjectType::actuator:
        // Allowed?
        bError = !bDeviceAndBasicServiceAllowed;
        break;
    case sdv::EObjectType::complex_service:
    case sdv::EObjectType::vehicle_function:
        // Isolation (which is needed for the main application) is not supported for direct creation.
        bError = !bComplexServiceAllowed || GetAppSettings().IsMainApplication();
        m_bIsoObjectLoaded = true;
        break;
    default:
        bError = true;
        break;
    }
    if (bError)
    {
        // Utilities and marshall objects cannot be created using the CreateObject function.
        SDV_LOG_ERROR("Creation of an object of invalid type \"", sdv::ObjectType2String(optClassInfo->eType),
            "\" requested for class \"", ssClassName, "\"!");
        return 0;
    }

    // Check for an object name. If not existing get the default name (being either one specified by the object or the class name).
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    std::string ssObjectName2 = ssObjectName;
    if (ssObjectName2.empty())
        ssObjectName2 = optClassInfo->ssDefaultObjectName;
    if (ssObjectName2.empty())
        ssObjectName2 = ssClassName;

    // Check for an object with the same name.
    auto itPreviousService = m_mapServiceObjects.find(ssObjectName2);
    if (itPreviousService != m_mapServiceObjects.end())
    {
        // Object entry is valid?
        if (!*itPreviousService->second)
        {
            // This should not occur... there is a previous object in the service map, but the object is empty.
            SDV_LOG_ERROR("Object creation requested for class \"",
                ssClassName,
                "\", but object with the same name \"",
                ssObjectName2,
                "\" was already instantiated, but cannot be found!");
            return 0;
        }

        // Trying to create an object with the same name is not an error if the classes are identical.
        if ((*itPreviousService->second)->sClassInfo.ssName == ssClassName)
            return (*itPreviousService->second)->tObjectID;

        // Object name was already used by another class. This is an error.
        SDV_LOG_ERROR("Object creation requested for class \"",
            ssClassName,
            "\", but object with the same name \"",
            ssObjectName2,
            "\" was already instantiated for class \"",
            (*itPreviousService->second)->sClassInfo.ssName,
            "\"!");
        return 0;
    }

    // Check with singleton objects if the object was already instantiated.
    if (optClassInfo->uiFlags & static_cast<uint32_t>(sdv::EObjectFlags::singleton))
    {
        for (const auto& rprService : m_mapServiceObjects)
        {
            if (!*rprService.second)
                continue;
            if ((*rprService.second)->ptrModule && (*rprService.second)->ptrModule->GetModuleID() == ptrModule->GetModuleID()
                && (*rprService.second)->sClassInfo.ssName == ssClassName)
            {
                SDV_LOG_ERROR("Object creation requested but object from the same class \"",
                    ssClassName,
                    "\" was already instantiated and only one instance is allowed!");
                return 0;
            }
        }
    }

    // Unlock; the next function will lock again.
    lock.unlock();

    // Create an internal object.
    return InternalCreateObject(ptrModule, *optClassInfo, ssObjectName2, ssObjectConfig);
}

bool CRepository::DestroyObject(/*in*/ const sdv::u8string& ssObjectName)
{
    return DestroyObject2(ssObjectName);
}

bool CRepository::DestroyObject2(/*in*/ const sdv::u8string& ssObjectName)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
    auto itService = m_mapServiceObjects.find(ssObjectName);
    if (itService == m_mapServiceObjects.end()) return false;

    // Print info
    auto ptrObjectEntry = *itService->second;
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Destroy a " << sdv::ObjectType2String(ptrObjectEntry->sClassInfo.eType) << " #" << ptrObjectEntry->tObjectID <<
            " of type " << ptrObjectEntry->sClassInfo.ssName << " with the name " << ssObjectName << std::endl;

    // Remove the service
    m_lstOrderedServiceObjects.erase(itService->second);
    m_mapServiceObjects.erase(itService);

    // Remove from isolated service list
    m_mapIsolatedObjects.erase(ssObjectName);

    // Remove the object from the list and map.
    if (!ptrObjectEntry) return false;   // Failure: should not happen!
    m_mapObjects.erase(ptrObjectEntry->tObjectID);
    lock.unlock();

    // Try to destroy objects that depend on this object first
    auto fnDestroyDependingObjects = [&](const std::vector<sdv::core::TObjectID>& rvecDependingObjects)
    {
        // Iterate through the vector and find the corresponding object.
        for (sdv::core::TObjectID tDependingObjectID : rvecDependingObjects)
        {
            lock.lock();
            auto itObject = m_mapObjects.find(tDependingObjectID);
            if (itObject == m_mapObjects.end())
            {
                // Since this function is iterative, it might be that the object was removed already by a previous action.
                // Simply ignore this.
                lock.unlock();
                continue;
            }
            std::shared_ptr<SObjectEntry> ptrDependingObject = itObject->second;
            lock.unlock();

            // Only controlled objects can be destroyed this way.
            if (ptrDependingObject->bControlled) DestroyObject(ptrDependingObject->ssName);
        }
    };
    fnDestroyDependingObjects(GetDependingObjectInstancesByClass(ptrObjectEntry->sClassInfo.ssName));
    if (ptrObjectEntry->sClassInfo.ssName != ptrObjectEntry->sClassInfo.ssDefaultObjectName)
        fnDestroyDependingObjects(GetDependingObjectInstancesByClass(ptrObjectEntry->sClassInfo.ssDefaultObjectName));

    // Shutdown the object (not all objects expose IObjectControl).
    auto* pObjectControl = ptrObjectEntry->ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl && pObjectControl->GetObjectState() != sdv::EObjectState::destruction_pending)
        pObjectControl->Shutdown();

    // Destroy the object (not all objects have a module and some are wrapped by an isolation monitor).
    if (ptrObjectEntry->ptrModule && ptrObjectEntry->ptrObject)
    {
        sdv::IInterfaceAccess* pObject = ptrObjectEntry->ptrObject;
        if (ptrObjectEntry->ptrIsoMon)
            pObject = ptrObjectEntry->ptrIsoMon->GetContainedObject();
        ptrObjectEntry->ptrModule->DestroyObject(pObject);
    }

    return true;
}

sdv::core::TObjectID CRepository::RegisterObject(IInterfaceAccess* pObjectIfc, const sdv::u8string& ssObjectName)
{
    // Check for an interface
    if (!pObjectIfc)
    {
        SDV_LOG_ERROR("Object registration requested for an NULL object instance!");
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);

    // Do not allow the creation of more objects in local context if this is an isolated or external application.
    // But do allow the registration of multiple objects... these are only accessible locally and not by the rest of the system.
    m_bIsoObjectLoaded = true;

    // Check for an object with none or the same name.
    // TODO: If running in isolated/external app process, check whether the name is a duplicate in the core process
    if (ssObjectName.empty())
    {
        SDV_LOG_ERROR("Object registration requested for an object instance without name!");
        return false;
    }

    // Check if an isolated application wants to register
    auto itIsolatedApp = m_mapIsolatedObjects.find(ssObjectName);
    if (itIsolatedApp != m_mapIsolatedObjects.end())
    {
        auto ptrIsolatedObject = itIsolatedApp->second;
        lock.unlock();
        if (!ptrIsolatedObject) return 0; // Already deleted
        if (ptrIsolatedObject->ptrObject) return 0; // only one time registration allowed.
        if (GetAppSettings().IsConsoleVerbose())
            std::cout << "Registering the isolated object #" << ptrIsolatedObject->tObjectID << " with the name " << ssObjectName << std::endl;
        ptrIsolatedObject->ptrObject = pObjectIfc;
        std::unique_lock<std::mutex> lockObject(ptrIsolatedObject->mtxConnect);
        ptrIsolatedObject->cvConnect.notify_all();
        return ptrIsolatedObject->tObjectID;
    }

    // If running locally (not in the isolation map, only services can be registered).
    if (m_mapServiceObjects.find(ssObjectName) != m_mapServiceObjects.end())
    {
        SDV_LOG_ERROR("Object registration requested for an object instance, but another object with the same name was already "
            "instantiated for name \"", ssObjectName, "\"!");
        return false;
    }

    // Reserve the object
    sdv::core::TObjectID tObjectID = CreateObjectID();
    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    m_mapServiceObjects[ssObjectName] = m_lstOrderedServiceObjects.insert(m_lstOrderedServiceObjects.end(), ptrObjectEntry);
    m_mapObjects[tObjectID] = ptrObjectEntry;
    ptrObjectEntry->tObjectID = tObjectID;
    ptrObjectEntry->ssName = ssObjectName;
    ptrObjectEntry->ptrObject = pObjectIfc;
    ptrObjectEntry->bControlled = true;

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Registering a application object #" << tObjectID << " with the name " << ssObjectName << std::endl;

    return tObjectID;
}

void CRepository::LinkCoreRepository(/*in*/ sdv::IInterfaceAccess* pCoreRepository)
{
    if (!pCoreRepository) return;
    m_ptrCoreRepoAccess = pCoreRepository;
}

void CRepository::UnlinkCoreRepository()
{
    m_ptrCoreRepoAccess = nullptr;
}

sdv::SClassInfo CRepository::FindClass(/*in*/ const sdv::u8string& ssClassName) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);

    // For main and isolated applications, search in the installation.
    if (GetAppSettings().IsMainApplication() || GetAppSettings().IsIsolatedApplication())
    {
        auto optManifest = GetAppConfig().FindInstalledComponent(ssClassName);
        if (!optManifest) return {};
        return *optManifest;
    }

    // Get the information through the module.
    auto ptrModule = GetModuleControl().FindModuleByClass(ssClassName);
    if (!ptrModule) return {};
    auto optClassInfo = ptrModule->GetClassInfo(ssClassName);
    if (!optClassInfo) return {};
    return *optClassInfo;
}

sdv::sequence<sdv::core::SObjectInfo> CRepository::GetObjectList() const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    sdv::sequence<sdv::core::SObjectInfo> seqObjects;

    // If there is a linked core, use the core to get object list
    if (m_ptrCoreRepoAccess)
    {
        const sdv::core::IRepositoryInfo* pCoreRepoInfo = m_ptrCoreRepoAccess.GetInterface<sdv::core::IRepositoryInfo>();
        return pCoreRepoInfo ? pCoreRepoInfo->GetObjectList() : seqObjects;
    }

    // Add object function
    auto fnAddObject = [&seqObjects](const std::shared_ptr<SObjectEntry>& rptrObjectEntry)
    {
        if (!rptrObjectEntry) return;

        sdv::core::SObjectInfo sInfo{};
        sInfo.tObjectID = rptrObjectEntry->tObjectID;
        sInfo.tModuleID = rptrObjectEntry->ptrModule ? rptrObjectEntry->ptrModule->GetModuleID() : 0;
        sInfo.sClassInfo = rptrObjectEntry->sClassInfo;
        sInfo.ssObjectName = rptrObjectEntry->ssName;
        sInfo.ssObjectConfig = rptrObjectEntry->ssConfig;
        sInfo.uiFlags = 0;
        if (rptrObjectEntry->bControlled)
            sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_controlled);
        if (!rptrObjectEntry->ptrModule)
            sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_foreign);
        if (rptrObjectEntry->bIsolated)
            sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_isolated);

        seqObjects.push_back(sInfo);
    };

    // Add controlled object information
    for (const auto& rptrService : m_lstOrderedServiceObjects)
        fnAddObject(rptrService);

    // Add local objects
    for (const auto& prLocalObject : m_mapLocalObjects)
        fnAddObject(prLocalObject.second);

    return seqObjects;
}

sdv::core::SObjectInfo CRepository::GetObjectInfo(/*in*/ sdv::core::TObjectID tObjectID) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);

    // If there is a linked core, use the core to get object info
    if (m_ptrCoreRepoAccess)
    {
        const sdv::core::IRepositoryInfo* pCoreRepoInfo = m_ptrCoreRepoAccess.GetInterface<sdv::core::IRepositoryInfo>();
        return pCoreRepoInfo ? pCoreRepoInfo->GetObjectInfo(tObjectID) : sdv::core::SObjectInfo{};
    }

    // Use the internal object map for this.
    auto itObject = m_mapObjects.find(tObjectID);
    if (itObject == m_mapObjects.end()) return {};
    if (!itObject->second) return {};

    // Fill in information
    sdv::core::SObjectInfo sInfo{};
    sInfo.tObjectID = itObject->second->tObjectID;
    sInfo.tModuleID = itObject->second->ptrModule ? itObject->second->ptrModule->GetModuleID() : 0;
    sInfo.sClassInfo = itObject->second->sClassInfo;
    sInfo.ssObjectName = itObject->second->ssName;
    sInfo.ssObjectConfig = itObject->second->ssConfig;
    sInfo.uiFlags = 0;
    if (itObject->second->bControlled)
        sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_controlled);
    if (!itObject->second->ptrModule)
        sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_foreign);
    if (!itObject->second->bIsolated)
        sInfo.uiFlags |= static_cast<uint32_t>(sdv::core::EObjectInfoFlags::object_isolated);

    return sInfo;
}

sdv::core::SObjectInfo CRepository::FindObject(/*in*/ const sdv::u8string& ssObjectName) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);

    // If there is a linked core, use the core to get object info
    if (m_ptrCoreRepoAccess)
    {
        const sdv::core::IRepositoryInfo* pCoreRepoInfo = m_ptrCoreRepoAccess.GetInterface<sdv::core::IRepositoryInfo>();
        return pCoreRepoInfo ? pCoreRepoInfo->FindObject(ssObjectName) : sdv::core::SObjectInfo{};
    }

    // Use internal service map for this.
    auto itService = m_mapServiceObjects.find(ssObjectName);
    if (itService == m_mapServiceObjects.end()) return {};
    auto ptrObjectEntry = *itService->second;
    if (!ptrObjectEntry) return {};
    lock.unlock();
    return GetObjectInfo(ptrObjectEntry->tObjectID);
}

void CRepository::OnDestroyObject(sdv::IInterfaceAccess* pObject)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);

    auto itMapObject = m_mapLocalObjects.find(pObject);
    if (itMapObject == m_mapLocalObjects.end()) return;
    auto ptrObjectEntry = itMapObject->second;
    m_mapLocalObjects.erase(itMapObject);

    // Remove the object from the map.
    if (!ptrObjectEntry) return;    // Failure: should not happen!
    m_mapObjects.erase(ptrObjectEntry->tObjectID);
    lock.unlock();

    // Shutdown the object
    auto* pObjectControl = ptrObjectEntry->ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl && pObjectControl->GetObjectState() != sdv::EObjectState::destruction_pending)
        pObjectControl->Shutdown();

    // Destroy the object
    if (ptrObjectEntry->ptrModule)
        ptrObjectEntry->ptrModule->DestroyObject(ptrObjectEntry->ptrObject);
}

void CRepository::DestroyModuleObjects(sdv::core::TModuleID tModuleID)
{
    // For all objects of the module.
    while (true)
    {
        std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
        auto itService = std::find_if(m_mapServiceObjects.begin(), m_mapServiceObjects.end(), [&](const auto& rprService)
            {
                return *rprService.second && (*rprService.second)->ptrModule &&
                    (*rprService.second)->ptrModule->GetModuleID() == tModuleID;
            });
        if (itService == m_mapServiceObjects.end()) return;
        auto ptrObjectEntry = *itService->second;
        m_lstOrderedServiceObjects.erase(itService->second);
        m_mapServiceObjects.erase(itService);

        // Remove the object from the map.
        if (!ptrObjectEntry) continue;
        m_mapObjects.erase(ptrObjectEntry->tObjectID);
        lock.unlock();

        // Shutdown the object
        auto* pObjectControl = ptrObjectEntry->ptrObject.GetInterface<sdv::IObjectControl>();
        if (pObjectControl && pObjectControl->GetObjectState() != sdv::EObjectState::destruction_pending)
            pObjectControl->Shutdown();

        // Destroy the object
        if (ptrObjectEntry->ptrModule)
            ptrObjectEntry->ptrModule->DestroyObject(ptrObjectEntry->ptrObject);
    }
}

void CRepository::DestroyAllObjects(const std::vector<std::string>& rvecIgnoreObjects, bool bForce /*= false*/)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    auto lstCopy = m_lstOrderedServiceObjects;
    lock.unlock();

    // Destroy the objects in reverse order
    while (lstCopy.size())
    {
        std::string ssObjectName = lstCopy.back() ? lstCopy.back()->ssName : std::string{};
        lstCopy.pop_back();
        if (ssObjectName.empty()) continue;

        // On the ignore list?
        if (std::find(rvecIgnoreObjects.begin(), rvecIgnoreObjects.end(), ssObjectName) != rvecIgnoreObjects.end())
            continue;

        // Destroy the object... -> call the repository function to remove it from the list.
        DestroyObject(ssObjectName);

        // Needs force!
        if (bForce)
        {
            // Still there...
            lock.lock();
            auto itService = m_mapServiceObjects.find(ssObjectName);
            if (itService == m_mapServiceObjects.end())
            {
                lock.unlock();
                continue;
            }

            // Remove the service
            auto ptrObjectEntry = *itService->second;
            m_lstOrderedServiceObjects.erase(itService->second);
            m_mapServiceObjects.erase(itService);

            // Remove the object from the list and map.
            if (!ptrObjectEntry)
            {
                lock.unlock();
                continue;   // Failure: should not happen!
            }
            m_mapObjects.erase(ptrObjectEntry->tObjectID);
            lock.unlock();
        }
    }
}

void CRepository::ResetConfigBaseline()
{
    // Reset the configuration set.
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
    m_setConfigObjects.clear();
    for (const auto& rvtObject : m_mapObjects)
        m_setConfigObjects.insert(rvtObject.first);
    lock.unlock();

    GetModuleControl().ResetConfigBaseline();
}

sdv::core::EConfigProcessResult CRepository::StartFromConfig(const CAppConfigFile& rconfig, bool bAllowPartialLoad)
{
    // First load the modules
    sdv::core::EConfigProcessResult eResult = GetModuleControl().LoadModulesFromConfig(rconfig, bAllowPartialLoad);
    if (eResult == sdv::core::EConfigProcessResult::failed) return eResult;

    bool bRunsAsServer = false;
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        bRunsAsServer = true;
    default:
        break;
    }

    // Load all the components from the component list
    auto vecComponents = rconfig.GetComponentList();
    size_t nSuccess = 0, nFail = 0;
    std::vector<std::string> vecLoadedObjects;
    for (const auto& rsComponent : vecComponents)
    {
        // If there a path is stored with the component and not running a server, load and get the module ID and then start a
        // specific component. If there is no path stored, use the default object creation function.
        sdv::core::TObjectID tObjectID = 0;
        if (!bRunsAsServer && !rsComponent.pathModule.empty())
        {
            sdv::core::TModuleID tModuleID = GetModuleControl().Load(rsComponent.pathModule.generic_u8string());
            if (tModuleID)
                tObjectID = CreateObjectFromModule(tModuleID, rsComponent.ssClassName, rsComponent.ssInstanceName, rsComponent.ssParameterTOML);
        } else
            tObjectID = CreateObject(rsComponent.ssClassName, rsComponent.ssInstanceName, rsComponent.ssParameterTOML);

        if (tObjectID)
        {
            ++nSuccess;
            // Request the object name. This name is unique, but might be assigned automatically.
            vecLoadedObjects.push_back(GetObjectInfo(tObjectID).ssObjectName);
        } else
            ++nFail;
        if (!bAllowPartialLoad && nFail)
            break;
    }

    // Destroy the objects when an error occurred during loading.
    if (!bAllowPartialLoad && nFail)
    {
        for (const std::string& rssName : vecLoadedObjects)
            DestroyObject(rssName);
        return sdv::core::EConfigProcessResult::failed;
    }

    // TODO: The class list contains the parameters to set for each component.
    //// Load the class information from the class list
    // auto vecClasses = rconfig.GetClassList();
    // for (const auto& rsClass : vecClasses)
    //{

    //    if (rsClass.ssModulePath.empty())
    //        continue;
    //    sdv::core::TModuleID tModuleID = Load(std::filesystem::u8path(rsClass.ssModulePath));
    //    if (tModuleID)
    //        ++nSuccess;
    //    else
    //        ++nFail;
    //}

    //if (!bAllowPartialLoad && nFail)
    //    return sdv::core::EConfigProcessResult::failed;
    if (!nFail)
        return sdv::core::EConfigProcessResult::successful;
    if (nSuccess)
        return sdv::core::EConfigProcessResult::partially_successful;
    return sdv::core::EConfigProcessResult::failed;
}

std::string CRepository::SaveConfig()
{
    std::stringstream sstream;
    //std::shared_lock<std::shared_mutex> lock(m_mtxObjects);
    //std::set<std::filesystem::path> setModules;
    //for (const std::shared_ptr<SObjectEntry>& rptrObject : m_lstOrderedServiceObjects)
    //{
    //    if (m_setConfigObjects.find(rptrObject->tObjectID) != m_setConfigObjects.end())
    //    {
    //        sstream << std::endl;
    //        sstream << "[[Component]]" << std::endl;
    //        sstream << "Path = \"" << rptrObject->ptrModule->GetModulePath().generic_u8string() << "\"" << std::endl;
    //        setModules.insert(rptrObject->ptrModule->GetModulePath());
    //        sstream << "Class = \"" << rptrObject->sClassInfo.ssName << "\"" << std::endl;
    //        sstream << "Name = \"" << rptrObject->ssName << "\"" << std::endl;
    //        // TODO: attributes...
    //    }
    //}

    //// Add the modules
    //sstream << GetModuleControl().SaveConfig(setModules);

    return sstream.str();
}

sdv::core::TObjectID CRepository::CreateIsolatedObject(const sdv::SClassInfo& rsClassInfo, const sdv::u8string& rssObjectName,
    const sdv::u8string& rssObjectConfig)
{
    // Check whether running as main application.
    if (!GetAppSettings().IsMainApplication()) return 0;

    if (rssObjectName.empty())
    {
        SDV_LOG_ERROR("Object creation requested for class \"", rsClassInfo.ssName, "\", but no object name was provided!");
        return 0;
    }

    // Check the object type and define whether this should be a controlled object
    bool bControlled = false;
    switch (rsClassInfo.eType)
    {
    case sdv::EObjectType::complex_service:
    case sdv::EObjectType::vehicle_function:
        bControlled = true;
        break;
    case sdv::EObjectType::utility:
        break;
    default:
        return 0;
    }

    // Create server connection (to the repository service object... not to this class!).
    sdv::com::IConnectionControl* pConnectionControl =
        sdv::TInterfaceAccessPtr(GetObject("CommunicationControl")).GetInterface<sdv::com::IConnectionControl>();
    if (!pConnectionControl) return 0;
    sdv::u8string ssConnectionString;
    sdv::com::TConnectionID tConnection =
        pConnectionControl->CreateServerConnection(sdv::com::EChannelType::local_channel,
            sdv::core::GetObject("RepositoryService"), 5000, ssConnectionString);
    if (!tConnection.uiControl) return 0;
    if (ssConnectionString.empty()) return 0;

    // Create the isolation process configuration
    std::stringstream sstreamConfig;
    sstreamConfig << "[Isolation]" << std::endl;
    sstreamConfig << "Class = \"" << rsClassInfo.ssName << "\"" << std::endl;
    sstreamConfig << "Object = \"" << rssObjectName << "\"" << std::endl << std::endl;
    if (!rssObjectConfig.empty())
    {
        toml_parser::CParser parserConfig(rssObjectConfig);
        sstreamConfig << parserConfig.GenerateTOML("Isolation.Config");
    }
    toml_parser::CParser parserConnection(ssConnectionString);
    sstreamConfig << parserConnection.GenerateTOML("Isolation.Connection");

    // Create command line arguments
    sdv::sequence<sdv::u8string> seqArguments;
    seqArguments.push_back(Base64EncodePlainText(sstreamConfig.str()));
    if (GetAppSettings().IsConsoleVerbose())
        seqArguments.push_back("--verbose");
    else
        seqArguments.push_back("--silent");
    seqArguments.push_back("--instance" + std::to_string(GetAppSettings().GetInstanceID()));
    seqArguments.push_back("--install_dir" + GetAppSettings().GetRootDir().generic_u8string());

    sdv::core::TObjectID tObjectID = CreateObjectID();
    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrObjectEntry)
    {
        SDV_LOG_ERROR("Object creation failed for class \"", rsClassInfo.ssName, "\"!");
        return 0;
    }
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
    ptrObjectEntry->tObjectID = tObjectID;
    ptrObjectEntry->sClassInfo = rsClassInfo;
    ptrObjectEntry->ssName = rssObjectName;
    ptrObjectEntry->ssConfig = rssObjectConfig;
    ptrObjectEntry->bControlled = bControlled;
    ptrObjectEntry->bIsolated = true;
    if (bControlled)
        m_mapServiceObjects[rssObjectName] = m_lstOrderedServiceObjects.insert(m_lstOrderedServiceObjects.end(), ptrObjectEntry);
    m_mapObjects[tObjectID] = ptrObjectEntry;
    m_mapIsolatedObjects[rssObjectName] = ptrObjectEntry;
    lock.unlock();

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Creating an isolated " << sdv::ObjectType2String(rsClassInfo.eType) << " #" << tObjectID << " of type " <<
            rsClassInfo.ssName << std::endl;

    // Start the isolation process
    sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
    if (!pProcessControl) return 0;
    sdv::process::TProcessID tIsoApp = pProcessControl->Execute("sdv_iso", seqArguments, sdv::process::EProcessRights::reduced_rights);
    if (!tIsoApp) return 0;

    // Wait for connection (at the most 30 seconds).
    std::unique_lock<std::mutex> lockConnect(ptrObjectEntry->mtxConnect);
    ptrObjectEntry->cvConnect.wait_for(lockConnect, std::chrono::milliseconds(30000));

    // Return the ID if there is a pointer in the mean time.
    return ptrObjectEntry->ptrObject ? ptrObjectEntry->tObjectID : 0;
}

sdv::core::TObjectID CRepository::InternalCreateObject(const std::shared_ptr<CModuleInst>& rptrModule,
    const sdv::SClassInfo& rsClassInfo, const sdv::u8string& rssObjectName, const sdv::u8string& rssObjectConfig)
{
    if (rssObjectName.empty())
    {
        SDV_LOG_ERROR("Object creation requested for class \"", rsClassInfo.ssName, "\", but no object name was provided!");
        return 0;
    }

    // Check for an object name. If not existing get the default name (being either one specified by the object or the class name).
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
    // Reserve the object and return the lock
    sdv::core::TObjectID tObjectID = CreateObjectID();
    auto ptrObjectEntry = std::make_shared<SObjectEntry>();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrObjectEntry)
    {
        SDV_LOG_ERROR("Object creation failed for class \"", rsClassInfo.ssName, "\"!");
        return 0;
    }
    m_mapServiceObjects[rssObjectName] = m_lstOrderedServiceObjects.insert(m_lstOrderedServiceObjects.end(), ptrObjectEntry);
    m_mapObjects[tObjectID] = ptrObjectEntry;
    ptrObjectEntry->tObjectID = tObjectID;
    ptrObjectEntry->sClassInfo = rsClassInfo;
    ptrObjectEntry->ssName = rssObjectName;
    ptrObjectEntry->ssConfig = rssObjectConfig;
    ptrObjectEntry->ptrModule = rptrModule;
    ptrObjectEntry->bControlled = true;
    lock.unlock();

    // Print info
    if (GetAppSettings().IsConsoleVerbose())
        std::cout << "Creating a " << sdv::ObjectType2String(rsClassInfo.eType) << " #" << tObjectID << " of type " <<
            rsClassInfo.ssName << " with the name " << rssObjectName << std::endl;

    // Create the object
    sdv::TInterfaceAccessPtr ptrObject = rptrModule->CreateObject(rsClassInfo.ssName);
    if (!ptrObject)
    {
        // Destroy the object again
        DestroyObject(rssObjectName);
        return 0;
    }

    // Try to initialize
    auto* pObjectControl = ptrObject.GetInterface<sdv::IObjectControl>();
    if (pObjectControl)
    {
        pObjectControl->Initialize(rssObjectConfig);
        if (pObjectControl->GetObjectState() != sdv::EObjectState::initialized)
        {
            // Shutdown the object (even if the initialization didn't work properly).
            pObjectControl->Shutdown();

            // Destroy the object
            rptrModule->DestroyObject(ptrObject);

            // Destroy the object again
            DestroyObject(rssObjectName);
            return 0;
        }
        switch (GetAppControl().GetOperationState())
        {
        case sdv::app::EAppOperationState::running:
            pObjectControl->SetOperationMode(sdv::EOperationMode::running);
            break;
        case sdv::app::EAppOperationState::configuring:
        case sdv::app::EAppOperationState::initializing:
        default:
            pObjectControl->SetOperationMode(sdv::EOperationMode::configuring);
            break;
        }
    }

    // For an one isolated object: add iso control and store this as pointer in object entry...
    if ((rsClassInfo.eType == sdv::EObjectType::complex_service || rsClassInfo.eType == sdv::EObjectType::vehicle_function) &&
        GetAppSettings().IsIsolatedApplication())
    {
        ptrObjectEntry->ptrIsoMon = std::make_shared<CIsoMonitor>(ptrObject);
        ptrObjectEntry->ptrObject = ptrObjectEntry->ptrIsoMon.get();
    }
    else
    {
        // Everything good, assign the object instance
        ptrObjectEntry->ptrObject = std::move(ptrObject);
    }

    return tObjectID;
}

sdv::core::TObjectID CRepository::CreateObjectID()
{
    static sdv::core::TObjectID tCurrent = 0;
    if (!tCurrent)
    {
        std::srand(static_cast<unsigned int>(time(0)));
        tCurrent = 0;
        while (!tCurrent) tCurrent = std::rand();
    }
    return ++tCurrent;
}

std::vector<sdv::core::TObjectID> CRepository::GetDependingObjectInstancesByClass(const std::string& rssClass)
{
    // Run through the object list and find objects that have this class in their dependencies list.
    std::vector<sdv::core::TObjectID> vecDependingObjects;
    std::unique_lock<std::shared_mutex> lock(m_mtxObjects);
    for (const auto& rvtObject : m_mapObjects)
    {
        if (std::find(rvtObject.second->sClassInfo.seqDependencies.begin(),
            rvtObject.second->sClassInfo.seqDependencies.end(), rssClass) !=
            rvtObject.second->sClassInfo.seqDependencies.end())
            vecDependingObjects.push_back(rvtObject.first);
    }
    return vecDependingObjects;
}

bool CRepositoryService::EnableRepositoryObjectControl()
{
    return GetAppSettings().IsMainApplication() ||
        GetAppSettings().IsStandaloneApplication() ||
        GetAppSettings().IsEssentialApplication() ||
        GetAppSettings().IsIsolatedApplication();
}

bool CRepositoryService::EnableRepositoryRegisterForeignApp()
{
    return GetAppSettings().IsMainApplication() ||
        GetAppSettings().IsStandaloneApplication() ||
        GetAppSettings().IsEssentialApplication() ||
        GetAppSettings().IsIsolatedApplication();
}

bool CRepositoryService::EnableRepositoryLink()
{
    return GetAppSettings().IsIsolatedApplication() || GetAppSettings().IsExternalApplication();
}
