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

#include "app_config_file.h"
#include <support/toml.h>
#include "app_settings.h"
#include "../../global/ipc_named_mutex.h"
#include "toml_parser/parser_toml.h"
#include <support/toml.h>
#include "installation_manifest.h"
#include <set>

CAppConfigFile::CAppConfigFile(const std::filesystem::path& rpathConfigFile)
{
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        m_pathConfigFile = GetAppSettings().GetInstallDir() / rpathConfigFile;
        break;
    default:
        m_pathConfigFile = rpathConfigFile;
        break;
    }

    if (!LoadConfigFile())
    {
        SDV_LOG_ERROR("ERROR: failed to load the configuration file: ", rpathConfigFile.generic_u8string());

    }
}

void CAppConfigFile::Clear()
{
    m_pathConfigFile.clear();
    m_bLoaded = false;
    m_vecComponentList.clear();
    m_vecClassList.clear();
    m_vecModuleList.clear();
}

bool CAppConfigFile::IsLoaded() const
{
    return m_bLoaded;
}

const std::filesystem::path& CAppConfigFile::ConfigPath() const
{
    return m_pathConfigFile;
}

bool CAppConfigFile::LoadConfigFile()
{
    if (!m_pathConfigFile.has_filename() || !std::filesystem::exists(m_pathConfigFile.parent_path()))
        return false;
    if (!std::filesystem::exists(m_pathConfigFile))
    {
        m_bLoaded = true;
        return true;
    }

    std::ifstream fstream(m_pathConfigFile);
    std::string ssConfig((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
    fstream.close();

    return LoadConfigFromString(ssConfig);
}

bool CAppConfigFile::LoadConfigFromString(const std::string& rssConfig)
{
    // Determine whether running in main, isolation or maintenance mode.
    bool bServerApp = false;
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        bServerApp = true;
        break;
    default:
        break;
    }

    try
    {
        // Read the configuration
        toml_parser::CParser parserConfig(rssConfig);

        // Check for the version
        sdv::toml::CNodeCollection nodeConfig(&parserConfig.Root());
        uint32_t uiVersion = nodeConfig.GetDirect("Configuration.Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            SDV_LOG_ERROR("Invalid version of application settings file (expected version ",
                SDVFrameworkInterfaceVersion,
                ", but available version ",
                uiVersion,
                ")");
            return false;
        }

        // Iterate through the module list (if not running as server application).
        if (!bServerApp)
        {
            sdv::toml::CNodeCollection nodeModules = nodeConfig.GetDirect("Module");
            for (size_t nIndex = 0; nIndex < nodeModules.GetCount(); nIndex++)
            {
                sdv::toml::CNodeCollection nodeModule = nodeModules.Get(nIndex);
                SModule sModule;
                sModule.pathModule = nodeModule.GetDirect("Path").GetValueAsPath();
                auto itModule = std::find_if(m_vecModuleList.begin(), m_vecModuleList.end(),
                    [&](const SModule& rsModuleStored) { return rsModuleStored.pathModule == sModule.pathModule; });
                if (itModule == m_vecModuleList.end())
                    m_vecModuleList.push_back(std::move(sModule));
            }
        }

        // Iterate through the class list
        sdv::toml::CNodeCollection arrayClasses = nodeConfig.GetDirect("Class");
        for (size_t nIndex = 0; nIndex < arrayClasses.GetCount(); nIndex++)
        {
            sdv::toml::CNodeCollection tableClass = arrayClasses.Get(nIndex);
            sdv::SClassInfo sClass = TOML2ClassInfo(parserConfig.Root().Cast<toml_parser::CNodeCollection>(), nIndex);
            if (sClass.ssName.empty() || sClass.eType == sdv::EObjectType::undefined) continue;
            auto itClass = std::find_if(m_vecClassList.begin(), m_vecClassList.end(),
                [&](const sdv::SClassInfo& rsClassStored) { return rsClassStored.ssName == sClass.ssName; });
            if (itClass == m_vecClassList.end())
                m_vecClassList.push_back(std::move(sClass));
        }

        // Iterate through the component list
        sdv::toml::CNodeCollection nodeComponents = nodeConfig.GetDirect("Component");
        for (size_t nIndex = 0; nIndex < nodeComponents.GetCount(); nIndex++)
        {
            sdv::toml::CNodeCollection tableComponent = nodeComponents.Get(nIndex);
            SComponent sComponent;
            if (!bServerApp)
                sComponent.pathModule = tableComponent.GetDirect("Path").GetValue().get<std::string>();
            sComponent.ssClassName = tableComponent.GetDirect("Class").GetValue().get<std::string>();
            if (sComponent.ssClassName.empty())
            {
                SDV_LOG_ERROR("Missing class name for the class definition in the configuration file '", m_pathConfigFile, "'.");
                return false;
            }
            sComponent.ssInstanceName = tableComponent.GetDirect("Name").GetValue().get<std::string>();
            // NOTE: The name could be empty. The system will automatically select a name.
            sdv::toml::CNodeCollection tableParams = tableComponent.GetDirect("Parameters");
            if (tableParams.IsValid() && tableParams.GetCount())
                sComponent.ssParameterTOML = tableParams.GetTOML();
            auto itComponent = std::find_if(m_vecComponentList.begin(), m_vecComponentList.end(),
                [&](const SComponent& rsComponentStored) { return rsComponentStored.ssClassName == sComponent.ssClassName &&
                rsComponentStored.ssInstanceName == sComponent.ssInstanceName; });
            if (itComponent == m_vecComponentList.end())
                m_vecComponentList.push_back(std::move(sComponent));
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        SDV_LOG_ERROR("Failed to parse application configuration '", m_pathConfigFile.generic_u8string(), "': ", rexcept.what());
        return false;
    }

    m_bLoaded = true;

    return true;
}

bool CAppConfigFile::SaveConfigFile(const std::filesystem::path& rpathTarget /*= std::filesystem::path()*/,
    bool bSafeParamsOnly /*= false*/) const
{
    // Protect against multiple write actions at the same time.
    std::string ssMutexName = "LockSdvConfigFile_" + std::to_string(GetAppSettings().GetInstanceID()) + "_" +
        rpathTarget.filename().generic_u8string();
    ipc::named_mutex mtx(ssMutexName);
    // Warning of cppcheck for locking a local mutex, which doesn't have any effect. Since this is a named mutex between
    // applciations, the warning is not correct. Suppress warning.
    // cppcheck-suppress localMutex
    std::unique_lock<ipc::named_mutex> lock(mtx);

    std::filesystem::path pathTargetCopy = rpathTarget;
    if (pathTargetCopy.empty()) pathTargetCopy = m_pathConfigFile;
    if (pathTargetCopy.empty()) return false;
    if (!std::filesystem::exists(pathTargetCopy.parent_path())) return false;

    // If the configuration is not existing, create a default configuration file...
    std::string ssConfig;
    if (std::filesystem::exists(pathTargetCopy))
    {
        // Open the existing settings file
        std::ifstream fstream(m_pathConfigFile);
        if (!fstream.is_open())
        {
            if (!GetAppSettings().IsConsoleSilent())
                std::cerr << "ERROR: Cannot open the application configuration file." << std::endl;
            return false;
        }

        // Read the config file
        ssConfig = std::string((std::istreambuf_iterator<char>(fstream)), std::istreambuf_iterator<char>());
        if (ssConfig.empty())
        {
            if (!GetAppSettings().IsConsoleSilent())
                std::cerr << "ERROR: Cannot read the application settings file; will use default." << std::endl;
        }
    }
    bool bChanged = false;
    if (!UpdateConfigString(ssConfig, bChanged, bSafeParamsOnly)) return false;
    if (!bChanged) return true; // No change needed

    std::ofstream fstream(pathTargetCopy, std::ios::trunc);
    if (!fstream.is_open())
    {
        SDV_LOG_ERROR("Cannot write the application settings file '", m_pathConfigFile.generic_u8string(), "'.");
        return false;
    }
    fstream << ssConfig;

    return true;
}

bool CAppConfigFile::UpdateConfigString(std::string& rssContent, bool& rbChanged, bool /*bSaveParamsOnly*/ /*= false*/) const
{
    // Reset change flag
    rbChanged = false;

    // Determine whether running in main, isolation or maintenance mode.
    bool bServerApp = false;
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        bServerApp = true;
        break;
    default:
        break;
    }

    // If the configuration is not existing, create a default configuration file...
    std::string ssConfig = rssContent;
    if (ssConfig.empty())
    {
        ssConfig = R"toml(# Configuration file
[Configuration]
Version = )toml" + std::to_string(SDVFrameworkInterfaceVersion) + R"toml(

# The configuration consists of several sections:
#  - Modules          Modules can contain component classes, which are only made accessible, but not automatically loaded. This
#                     section applies to standalone applications only. Server applications use installations to provide accessible
#                     component classes.
#  - Classes          The classes that are present in a module and contain specific class information.
#  - Components       The components that are started when loading the configuration.
#
#
# The modules specify the location of the modules relative to the executable or with an absolute path. They are only available for
# standalone applications and are ignored by server based application, who install the modules rather than provide them in a
# configuration. A module has the following attribute:
#  - Path             [Compulsory] Identifying the module path of components which should be made accessible. The path can be
#                     relative to the application executable (or relocation path) or can be an absolute path to the module in the
#                     system.
#
# Example:
#    [[Module]]
#    Path = "my_module.sdv"
#
#
# Classes are used to specify default parameter for a component instance. These parameter can be overridden by the component instance
# when specifying the parameter with identical name in the component section. The class has the following attributes:
#  - Path             [Optional] Identifying the module path of the component. Only used for standalone applications. If not
#                     specified the component class must be known in the system (e.g. through the module section or the component
#                     section).
#  - Class            [Compulsory] Name of the class that has the default pateters.
#  - Parameters       [Optional] Table containing component instance specific parameters which are default parameters of all
#                     instances of the components with of this class.
# Example:
#    [[Class]]
#    Path = "my_module.sdv"
#    Class = "MyComponent"
#    [Class.Parameters]
#    AttributeA = "default_blep"                     
#    AttributeB = 1234
#    [Component.Parameters.GroupC]
#    AttributeC1 = 4567.7890
#    AttributeC2 = "This is a string"
#
#
# Components are class instances that are instantiated during the loading of the configuration. They are stored as table array
# and contain as a minimum a class name. They can have instance specific parameters, which are stored as a table within the
# component. Components are instantiated in the order of appearance. A component can have the following attributes:
#  - Path             [Optional] Identifying the module path of the component. Only used for standalone applications. If not
#                     specified the component class must be known in the system (e.g. through the module section or the class
#                     section).
#  - Class            [Compulsory] Name of the class that has to be instantiated.
#  - Name             [Optional] Name of the component instance. If not available, the class name determines the component name.
#  - Parameters       [Optional] Table containing the component instance specific parameters. They override default component class
#                     parameters.
#
# Example:
#    [[Component]]
#    Path = "my_module.sdv"
#    Class = "MyComponent"
#    Name = "MyPersonalComponent"
#    [Component.Parameters]
#    AttributeA = "blep"                     
#    AttributeB = 123
#    [Component.Parameters.GroupC]
#    AttributeC1 = 456.789
#    AttributeC2 = "This is a text"
#

)toml";
        rbChanged = true;
    }

    try
    {
        // Read the configuration
        toml_parser::CParser parserConfig(ssConfig);

        // Check for the version
        sdv::toml::CNodeCollection nodeConfig(&parserConfig.Root());
        uint32_t uiVersion = nodeConfig.GetDirect("Configuration.Version").GetValue();
        if (uiVersion != SDVFrameworkInterfaceVersion)
        {
            SDV_LOG_ERROR("Invalid version of application settings file (expected version ", SDVFrameworkInterfaceVersion,
                ", but available version ", uiVersion, ")");
            return false;
        }

        // Iterate and update through the module list (if not running as server application).
        if (!bServerApp)
        {
            // Remove modules not in the list any more.
            // Modules both in the vector and in the list are removed from the vector
            // Modules leftover in the vector are added to the list
            auto vecModuleListCopy = m_vecModuleList;
            sdv::toml::CNodeCollection nodeModules = nodeConfig.GetDirect("Module");
            for (size_t nIndex = nodeModules.GetCount() - 1; nIndex < nodeModules.GetCount(); --nIndex)
            {
                sdv::toml::CNodeCollection tableModule = nodeModules.Get(nIndex);
                std::filesystem::path pathModule = tableModule.GetDirect("Path").GetValue().get<std::string>();
                auto itModule = std::find_if(vecModuleListCopy.begin(), vecModuleListCopy.end(),
                    [&](const SModule& rsModule) { return pathModule == rsModule.pathModule; });
                if (itModule == vecModuleListCopy.end())
                {
                    tableModule.Delete();
                    rbChanged = true;
                }
                else
                    vecModuleListCopy.erase(itModule);
            }
            for (const SModule& rsModule : vecModuleListCopy)
            {
                sdv::toml::CNodeCollection tableModule = nodeConfig.AddTableArray("Module");
                tableModule.AddValue("Path", rsModule.pathModule);
                rbChanged = true;
            }
        }

        // Iterate through and update the class list
        // Remove classes not in the list any more.
        // Classes both in the vector and in the list are removed from the vector
        // Classes leftover in the vector are added to the list
        auto vecClassListCopy = m_vecClassList;
        sdv::toml::CNodeCollection arrayClasses = nodeConfig.GetDirect("Class");
        for (size_t nIndex = arrayClasses.GetCount() -1; nIndex < arrayClasses.GetCount(); --nIndex)
        {
            sdv::toml::CNodeCollection tableClass = arrayClasses.Get(nIndex);
            std::filesystem::path pathModule;
            if (!bServerApp)
                pathModule = tableClass.GetDirect("Path").GetValue().get<std::string>();
            std::string ssClassName = tableClass.GetDirect("Class").GetValue().get<std::string>();
            auto itClass = std::find_if(vecClassListCopy.begin(),
                vecClassListCopy.end(),
                [&](const sdv::SClassInfo& rsClass) { return ssClassName == rsClass.ssName; });
            if (itClass == vecClassListCopy.end())
            {
                tableClass.Delete();
                rbChanged = true;
            }
            else
            {
                // Update of path
                if (!bServerApp)
                {
                    if (pathModule.generic_u8string() != itClass->ssModulePath)
                    {
                        pathModule = std::filesystem::u8path(static_cast<std::string>(itClass->ssModulePath));
                        sdv::toml::CNode nodePath = tableClass.GetDirect("Path");
                        if (!pathModule.empty())
                        {
                            if (nodePath)
                                nodePath.SetValue(pathModule);
                            else
                                tableClass.AddValue("Path", pathModule);
                        }
                        else if (nodePath)
                            nodePath.Delete();
                        rbChanged = true;
                    }
                }

                // Update the parameters - class parameters are default parameters provided by the object. Overwrite existing
                // default parameters.
                sdv::toml::CNodeCollection tableParams = tableClass.GetDirect("Parameters");
                sdv::u8string ssExistingTOML;
                if (tableParams) ssExistingTOML = tableParams.GetTOML();
                if (ssExistingTOML != itClass->ssDefaultConfig)
                {
                    if (tableParams) tableParams.Delete();
                    if (!itClass->ssDefaultConfig.empty())
                        tableClass.InsertTOML(sdv::toml::npos, itClass->ssDefaultConfig);
                    rbChanged = true;
                }

                vecClassListCopy.erase(itClass);
            }
        }
        for (const sdv::SClassInfo& rsClass : vecClassListCopy)
        {
            sdv::toml::CNodeCollection tableClass = nodeConfig.AddTableArray("Class");
            tableClass.AddValue("Class", rsClass.ssName);
            if (!bServerApp)
                tableClass.AddValue("Path", std::filesystem::u8path(static_cast<std::string>(rsClass.ssModulePath)));
            if (!rsClass.ssDefaultConfig.empty())
                tableClass.InsertTOML(sdv::toml::npos, rsClass.ssDefaultConfig);
            rbChanged = true;
        }

        // Iterate through and update the component list
        // Remove components not in the list any more.
        // Components both in the vector and in the list are removed from the vector
        // Components leftover in the vector are added to the list
        auto vecComponentListCopy = m_vecComponentList;
        sdv::toml::CNodeCollection nodeComponents = nodeConfig.GetDirect("Component");
        for (size_t nIndex = nodeComponents.GetCount() - 1; nIndex < nodeComponents.GetCount(); --nIndex)
        {
            sdv::toml::CNodeCollection tableComponent = nodeComponents.Get(nIndex);
            std::filesystem::path pathModule;
            if (!bServerApp)
                pathModule = tableComponent.GetDirect("Path").GetValue().get<std::string>();
            std::string ssClassName = tableComponent.GetDirect("Class").GetValue().get<std::string>();
            std::string ssInstanceName = tableComponent.GetDirect("Name").GetValue().get<std::string>();

            auto itComponent = std::find_if(vecComponentListCopy.begin(), vecComponentListCopy.end(),
                [&](const SComponent& rsComponent)
                {
                    return ssInstanceName.empty() ? (ssClassName == rsComponent.ssClassName && rsComponent.ssInstanceName.empty()) :
                        ssInstanceName == rsComponent.ssInstanceName;
                });
            if (itComponent == vecComponentListCopy.end())
            {
                // The class and the instance names must be identical
                tableComponent.Delete();
                rbChanged = true;
            }
            else
            {
                // Update of path
                if (!bServerApp)
                {
                    if (pathModule != itComponent->pathModule)
                    {
                        pathModule = itComponent->pathModule;
                        sdv::toml::CNode nodePath = tableComponent.GetDirect("Path");
                        if (!pathModule.empty())
                        {
                            if (nodePath)
                                nodePath.SetValue(pathModule);
                            else
                                tableComponent.AddValue("Path", pathModule);
                        }
                        else if (nodePath)
                            nodePath.Delete();
                        rbChanged = true;
                    }
                }

                // Update the parameters - component parameters should be updated, not simply overwritten. Use the combine function.
                sdv::toml::CNodeCollection tableParams = tableComponent.GetDirect("Parameters");
                sdv::u8string ssExistingTOML;
                if (tableParams) ssExistingTOML = tableParams.GetTOML();
                if (!itComponent->ssParameterTOML.empty())
                {
                    // Update needed?
                    if (tableParams && ssExistingTOML != itComponent->ssParameterTOML)
                    {
                        // Get the underlying node collection
                        toml_parser::CTable* pComponent =
                            dynamic_cast<toml_parser::CTable*>(static_cast<sdv::IInterfaceAccess*>(tableParams.GetInterface()));
                        if (!pComponent)
                        {
                            SDV_LOG_ERROR("Failure to get access to an internal interface within the TOML parser.");
                            return false;
                        }
                        try
                        {
                            toml_parser::CParser parserParams(itComponent->ssParameterTOML);
                            pComponent->Combine(parserParams.Root().Cast<toml_parser::CNodeCollection>());
                        }
                        catch (const toml_parser::XTOMLParseException& /*rexcept*/)
                        {
                            SDV_LOG_WARNING("The parameter TOML cannot be interpreted; no saving possible.");
                        }
                        rbChanged = true;
                    }
                    else if (!tableParams)
                    {
                        // Simply add the parameters
                        tableParams = tableComponent.InsertTable(sdv::toml::npos, "Parameters");
                        tableParams.InsertTOML(sdv::toml::npos, itComponent->ssParameterTOML);
                        rbChanged = true;
                    }
                }
                vecComponentListCopy.erase(itComponent);
            }
        }
        for (const SComponent& rsComponent : vecComponentListCopy)
        {
            sdv::toml::CNodeCollection tableComponent = nodeConfig.AddTableArray("Component");
            if (!bServerApp && !rsComponent.pathModule.empty())
                tableComponent.AddValue("Path", rsComponent.pathModule);
            tableComponent.AddValue("Class", rsComponent.ssClassName);
            if (!rsComponent.ssInstanceName.empty())
                tableComponent.AddValue("Name", rsComponent.ssInstanceName);
            if (!rsComponent.ssParameterTOML.empty())
            {
                sdv::toml::CNodeCollection tableParams = tableComponent.InsertTable(sdv::toml::npos, "Parameters");
                tableParams.InsertTOML(sdv::toml::npos, rsComponent.ssParameterTOML);
            }
            rbChanged = true;
        }

        // Save the configuration file if needed
        if (rbChanged)
        {
            rssContent = parserConfig.GenerateTOML();
            rbChanged  = true;
        }
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        SDV_LOG_ERROR("Failed to parse configuration: ", rexcept.what());
        return false;
    }
    return true;
}

const std::vector<CAppConfigFile::SComponent>& CAppConfigFile::GetComponentList() const
{
    return m_vecComponentList;
}

const std::vector<sdv::SClassInfo>& CAppConfigFile::GetClassList() const
{
    return m_vecClassList;
}

const std::vector<CAppConfigFile::SModule>& CAppConfigFile::GetModuleList() const
{
    return m_vecModuleList;
}

bool CAppConfigFile::InsertComponent(size_t nIndex, const std::filesystem::path& rpathModule, const std::string& rssClassName,
    const std::string& rssInstanceName, const TParameterVector& rvecParameters)
{
    // Valid parameter?
    if (rssClassName.empty()) return false;

    // Check for a duplicate instance name. If there is no instance name, check for a component with the same class name without
    // the instance name. This is not an error!
    if (!rssInstanceName.empty())
    {
        if (std::find_if(m_vecComponentList.begin(), m_vecComponentList.end(), [&](const SComponent& rsComponent)
            { return rsComponent.ssInstanceName == rssInstanceName; }) != m_vecComponentList.end())
            return true;
    }
    else
    {
        if (std::find_if(m_vecComponentList.begin(), m_vecComponentList.end(), [&](const SComponent& rsComponent)
            { return rsComponent.ssInstanceName.empty() && rsComponent.ssClassName == rssClassName; }) != m_vecComponentList.end())
            return true;
    }

    // Check for the module to already be added to the module list. Remove it from the list if it is.
    if (!rpathModule.empty())
    {
        auto itModule = std::find_if(m_vecModuleList.begin(), m_vecModuleList.end(), [&](const SModule& rsModule)
            { return rsModule.pathModule == rpathModule; });
        if (itModule != m_vecModuleList.end()) m_vecModuleList.erase(itModule);
    }

    // Create a parameter TOML
    std::string ssParameterTOML;
    try
    {
        // Create a TOML string
        toml_parser::CParser parser("");
        sdv::toml::CNodeCollection root(&parser.Root());
        
        // Split the key in a group and a parameter name
        auto fnSplit = [](const std::string& rssKey) -> std::pair<std::string, std::string>
        {
            size_t nSeparator = rssKey.find_last_of('.');
            if (nSeparator == std::string::npos) return std::make_pair("", rssKey);
            return std::make_pair(rssKey.substr(0, nSeparator), rssKey.substr(nSeparator + 1));
        };

        // Iterate through the parameters
        std::string ssGroup;
        sdv::toml::CNodeCollection group(root);
        for (const auto& prParameter : rvecParameters)
        {
            // Split the key in a group and parameter name.
            auto prKey = fnSplit(prParameter.first);

            // Need to add a group?
            if (prKey.first != ssGroup)
            {
                group = root.InsertTable(sdv::toml::npos, prKey.first);
                ssGroup = prKey.first;
            }

            // Add the parameter
            group.InsertValue(sdv::toml::npos, prKey.second, prParameter.second);
        }
        ssParameterTOML = parser.GenerateTOML();
    }
    catch (const toml_parser::XTOMLParseException& /*rexcept*/)
    {
        return false;
    }

    // Add the instance
    SComponent sComponent;
    sComponent.pathModule = rpathModule;
    sComponent.ssClassName = rssClassName;
    sComponent.ssInstanceName = rssInstanceName;
    sComponent.ssParameterTOML = std::move(ssParameterTOML);
    m_vecComponentList.insert((nIndex < m_vecComponentList.size() ? m_vecComponentList.begin() + nIndex : m_vecComponentList.end()),
        std::move(sComponent));

    return true;
}

void CAppConfigFile::RemoveComponent(const std::string& rssInstanceName)
{
    // Find and destroy
    auto itComponent = std::find_if(m_vecComponentList.begin(),
        m_vecComponentList.end(),
        [&](const SComponent& rsComponent) { return rsComponent.ssInstanceName == rssInstanceName; });
    if (itComponent != m_vecComponentList.end())
        m_vecComponentList.erase(itComponent);
}

bool CAppConfigFile::InsertModule(size_t nIndex, const std::filesystem::path& rpathModule)
{
    // Check for the proper context; only allowed when running as standalone (not main, isolated or maintenace).
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        return true;
    default:
        break;
    }

    if (rpathModule.empty())
        return false;

    // Check for the module to exist already in either the component or in the module list
    if (std::find_if(m_vecComponentList.begin(),
            m_vecComponentList.end(),
            [&](const SComponent& rsComponent) { return rsComponent.pathModule == rpathModule; })
        != m_vecComponentList.end())
        return true;
    if (std::find_if(m_vecModuleList.begin(),
            m_vecModuleList.end(),
            [&](const SModule& rsModule) { return rsModule.pathModule == rpathModule; })
        != m_vecModuleList.end())
        return true;

    // Add the module to the module list
    SModule sModule;
    sModule.pathModule = rpathModule;
    m_vecModuleList.insert(
        (nIndex < m_vecModuleList.size() ? m_vecModuleList.begin() + nIndex : m_vecModuleList.end()), std::move(sModule));

    return true;
}

void CAppConfigFile::RemoveModule(const std::filesystem::path& rpathModule)
{
    // Check for the proper context; only allowed when running as standalone (not main, isolated or maintenace).
    switch (GetAppSettings().GetContextType())
    {
    case sdv::app::EAppContext::main:
    case sdv::app::EAppContext::isolated:
    case sdv::app::EAppContext::maintenance:
        return;
    default:
        break;
    }

    // Find and destroy (any module and component with the provided path).
    auto itModule = std::find_if(m_vecModuleList.begin(),
        m_vecModuleList.end(),
        [&](const SModule& rsModule) { return rsModule.pathModule == rpathModule; });
    if (itModule != m_vecModuleList.end())
        m_vecModuleList.erase(itModule);
    while (true)
    {
        auto itComponent = std::find_if(m_vecComponentList.begin(),
            m_vecComponentList.end(),
            [&](const SComponent& rsComponent) { return rsComponent.pathModule == rpathModule; });
        if (itComponent == m_vecComponentList.end())
            break;
        m_vecComponentList.erase(itComponent);
    }
}

CAppConfigFile::EMergeResult CAppConfigFile::MergeConfigFile(const std::filesystem::path& rpathConfigFile)
{
    CAppConfigFile configNew(rpathConfigFile);
    if (!configNew.LoadConfigFile()) return EMergeResult::not_successful;

    size_t nSucceeded = 0;
    size_t nFailed = 0;

    // Add modules
    auto vecNewModules = configNew.GetModuleList();
    for (const auto& sNewModule : vecNewModules)
    {
        if (std::find_if(m_vecModuleList.begin(), m_vecModuleList.end(), [&sNewModule](const SModule& rsExistingModule)
            { return sNewModule.pathModule == rsExistingModule.pathModule; }) == m_vecModuleList.end())
            m_vecModuleList.push_back(sNewModule);
        ++nSucceeded;
    }

    // Add/update classes
    auto vecNewClasses = configNew.GetClassList();
    for (const auto& sNewClass : vecNewClasses)
    {
        auto itClass = std::find_if(m_vecClassList.begin(), m_vecClassList.end(),
            [&sNewClass](const sdv::SClassInfo& rsExistingClass)
            { return sNewClass.ssName == rsExistingClass.ssName; });
        if (itClass == m_vecClassList.end())
            m_vecClassList.push_back(sNewClass);
        else
            *itClass = sNewClass;
        ++nSucceeded;
    }

    // Merge components
    auto vecNewComponents = configNew.GetComponentList();
    for (const auto& sNewComponent : vecNewComponents)
    {
        auto itComponent = std::find_if(m_vecComponentList.begin(), m_vecComponentList.end(),
            [&sNewComponent](const SComponent& rsExistingComponent)
            {
                if (sNewComponent.ssInstanceName.empty())
                {
                    if (!rsExistingComponent.ssInstanceName.empty()) return false;
                    return sNewComponent.ssClassName == rsExistingComponent.ssClassName;
                }
                else
                    return sNewComponent.ssInstanceName == rsExistingComponent.ssInstanceName;
            });
        if (itComponent == m_vecComponentList.end())
            m_vecComponentList.push_back(sNewComponent);
        else
        {
            // Only update component information if the components are of identical type
            if (itComponent->ssClassName != sNewComponent.ssClassName)
            {
                ++nFailed;
                continue;
            }
            
            // TODO: Add additional possibilty to replace the parameters
            
            // Merge the parameters of the existing component.
            if (!sNewComponent.ssParameterTOML.empty())
            {
                if (itComponent->ssParameterTOML.empty())
                    itComponent->ssParameterTOML = sNewComponent.ssParameterTOML;
                else
                    try
                    {
                        toml_parser::CParser parserNew(itComponent->ssParameterTOML), parserExisting(sNewComponent.ssParameterTOML);
                        parserExisting.Root().Combine(parserNew.Root().Cast<toml_parser::CNodeCollection>());
                        itComponent->ssParameterTOML = parserExisting.GenerateTOML();
                    }
                    catch (const toml_parser::XTOMLParseException&)
                    {
                        ++nFailed;
                        continue;
                    }
            }
            ++nSucceeded;
        }
    }
    return nFailed ? (nSucceeded ? EMergeResult::partly_successfull : EMergeResult::not_successful) : EMergeResult::successful;
}
