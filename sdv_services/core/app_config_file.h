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

#ifndef APP_CONFIG_FILE_H
#define APP_CONFIG_FILE_H

#include <filesystem>
#include <string>
#include <vector>
#include <interfaces/config.h>

/**
 * @brief Parameter vector type holding parameter name and parameter value paires.
 */
using TParameterVector = std::vector<std::pair<std::string, sdv::any_t>>;

/**
 * @brief Application configuration file loading and saving the configuration in a TOML file.
 * @details The configuration file is a TOML and follows the specification of TOML 1.0 (https://toml.io/en/v1.0.0). The
 * configuration file starts with the "Configuration" table, which identifies that this a configuration file. The table needs to
 * contain the version of the configuration. For example:
 * @code
 *  [Configuration]
 *  Version = 100
 * @endcode
 *
 * The configuration consists of several sections:
 *  - Modules          Modules can contain component classes, which are only made accessible, but not automatically loaded. This
 *                     section applies to standalone applications only. Server applications use installations to provide accessible
 *                     component classes.
 *  - Classes          The classes that are present in a module and contain specific class information.
 *  - Components       The components that are started when loading the configuration.
 *
 * The modules specify the location of the modules relative to the executable or with an absolute path. They are only available for
 * standalone applications and are ignored by server based application, who install the modules rather than provide them in a
 * configuration. A module has the following attribute:
 *  - Path             [Compulsory] Identifying the module path of components which should be made accessible. The path can be
 *                     relative to the application executable (or relocation path) or can be an absolute path to the module in the
 *                     system.
 * Example:
 * @code
 *    [[Module]]
 *    Path = "my_module.sdv"
 * @endcode
 *
 * Classes are used to specify default parameter for a component instance. These parameter can be overridden by the component instance
 * when specifying the parameter with identical name in the component section. The class has the following attributes:
 *  - Path             [Optional] Identifying the module path of the component. Only used for standalone applications. If not
 *                     specified the component class must be known in the system (e.g. through the module section or the component
 *                     section).
 *  - Name             [Compulsory] Name of the class.
 *  - Aliases          [Optional] Array of strings with alternative names for this class.
 *  - DefaultName      [Optional] Name to use when instantiating the component. If not specified, the class name will be used.
 *  - Type             [Compulsory] Object type this class represents.
 *  - Singleton        [Optional] Boolean specifying whether the component allows more than one instantiation. Default is false.
 *  - Dependencies     [Optional] Array of strings with component instances that this class depends on.
 *  - Parameters       [Optional] Table containing component instance specific parameters which are default parameters of all
 *                     instances of the components with of this class.
 * Example:
 * @code 
 *    [[Class]]
 *    Path = "my_module.sdv"
 *    Name = "MyComponent"
 *    Aliases = ["Component1", "ComponentA"]
 *    DefaultName = "MyComp"
 *    Type = "Device"
 *    Singleton = false
 *    Dependencies = ["Component15", "Component16"]
 *    [Class.Parameters]
 *    AttributeA = "default_blep"                     
 *    AttributeB = 1234
 *    AttributeC = 4567.7890
 *@endcode
 *
 * Components are class instances that are instantiated during the loading of the configuration. They are stored as table array
 * and contain as a minimum a class name. They can have instance specific parameters, which are stored as a table within the
 * component. Components are instantiated in the order of appearance. A component can have the following attributes:
 *  - Path             [Optional] Identifying the module path of the component. Only used for standalone applications. If not
 *                     specified the component class must be known in the system (e.g. through the module section or the class
 *                     section).
 *  - Class            [Compulsory] Name of the class that has to be instantiated.
 *  - Name             [Optional] Name of the component instance. If not available, the class name determines the component name.
 *  - Parameters       [Optional] Table containing the component instance specific parameters. They override default component class
 *                     parameters.
 * Example:
 * @code
 *    [[Component]]
 *    Path = "my_module.sdv"
 *    Class = "MyComponent"
 *    Name = "MyPersonalComponent"
 *    [Component.Parameters]
 *    AttributeA = "blep"                     
 *    AttributeB = 123
 *    AttributeC = 456.789
 *    AttributeD = [1, 2, 3, 4]
 * @endcode
 */
class CAppConfigFile
{
public:
    /**
     * @brief default constructor.
     */
    CAppConfigFile() = default;

    /**
     * @brief Constructor with the config path. The config file is automatically loaded.
     * @param[in] rpathConfigFile Path to the configuration. The path must be a file name when running in main, isolation or
     * maintenance mode. Otherwise the path should contain the location of the config file.
     * The file path will be located in the root directory of the instance.
     */
    CAppConfigFile(const std::filesystem::path& rpathConfigFile);

    /**
     * @brief Clear the configuration.
     */
    void Clear();

    /**
     * @brief Is the configuration loaded or created?
     * @return Returns whether the file is loaded.
     */
    bool IsLoaded() const;

    /**
     * @brief Get the configuration file path.
     * @return The config file path.
     */
    const std::filesystem::path& ConfigPath() const;

    /**
     * @brief Load the application config file.
     * @remarks When there is no config file, this is not an error.
     * @return Returns whether the loading was successful.
     */
    bool LoadConfigFile();

    /**
     * @brief Load the application config from string.
     * @param[in] rssConfig Reference to the configuration string.
     * @return Returns whether the loading was successful.
     */
    bool LoadConfigFromString(const std::string& rssConfig);

    /**
     * @brief Save the application config file (or create when not existing yet).
     * @param[in] rpathTarget Reference to a target path. When set, the file is saved to the target file. If not set, the original
     * file is used.
     * @param[in] bSafeParamsOnly When set, only the parameters are stored in the configuration.
     * @return Returns whether the saving was successful (returns 'true' when there is no change is needed).
     */
    bool SaveConfigFile(const std::filesystem::path& rpathTarget = std::filesystem::path(), bool bSafeParamsOnly = false) const;

    /**
     * @brief Update the configuration string with the stored configuration.
     * @param[in, out] rssContent Reference to the string providing the current configuration being updated by this function.
     * @param[in, out] rbChanged Reference to the boolean being set when the string was changed.
     * @param[in] bSaveParamsOnly When set, only the parameters are stored in the configuration.
     * @return Returns whether the update was successful (even when no change occurred).
     */
    bool UpdateConfigString(std::string& rssContent, bool& rbChanged, bool bSaveParamsOnly = false) const;

    /**
     * @brief Module structure (only used in standalone applications).
     */
    struct SModule
    {
        std::filesystem::path pathModule;           ///< Component module path.
    };

    ///**
    // * @brief Component class structure (containing default parameters).
    // */
    //struct SClass
    //{
    //    std::filesystem::path   pathModule;         ///< Optional class module path. Only used in standalone applications.
    //    std::string             ssClassName;        ///< Class name of the component
    //    std::string             ssParameterTOML;    ///< Parameter configuration (excluding [Parameters]-group).
    //};

    /**
     * @brief Component structure
     */
    struct SComponent
    {
        std::filesystem::path   pathModule;         ///< Optional component module path. Only used in standalone applications.
        std::string             ssClassName;        ///< Class name of the component.
        std::string             ssInstanceName;     ///< Optional instance name. If not provided, will be identical to the class
                                                    ///< name.
        std::string             ssParameterTOML;    ///< Parameter configuration (excluding [Parameters]-group).
    };

    /**
     * @brief Get the component list.
     * @return Reference to the vector containing the components in order of the configuration.
     */
    const std::vector<SComponent>& GetComponentList() const;

    /**
     * @brief Get the class list.
     * @remarks Only available when running as standalone application.
     * @return Reference to the vector containing the classes in order of the configuration.
     */
    const std::vector<sdv::SClassInfo>& GetClassList() const;

    /**
     * @brief Get the module list.
     * @remarks Only available when running as standalone application.
     * @return Reference to the vector containing the modules in order of the configuration.
     */
    const std::vector<SModule>& GetModuleList() const;

    /**
     * @brief Insert a component at the provided location.
     * @param[in] nIndex The index location to insert the component before or when larger than the component list, at the end of 
     * the component list.
     * @param[in] rpathModule Reference to the path of the module. Only used when running as standalone application.
     * @param[in] rssClassName Reference to the string containing the module class name of the class to be instantiated when
     * loading the configuration.
     * @param[in] rssInstanceName Reference to the string containing an optional instantiating name of the component instance. Using
     * additional names allows multiple instantiation of the same component.
     * @param[in] rvecParameters Reference to the optional parameter vector belonging to the object instance.
     * @return Returns whether inserting the component was successful. If a component with the same class and instance names and
     * the same path is installed, the function succeeds. In all other cases, having a duplicate class or instance name will result
     * in a function failure.
     */
    bool InsertComponent(size_t nIndex, const std::filesystem::path& rpathModule, const std::string& rssClassName,
        const std::string& rssInstanceName, const TParameterVector& rvecParameters);

    /**
     * @brief Remove the component with the provided instance name.
     * @param[in] rssInstanceName Reference to the instance name of the component. If the component uses the class name for its
     * instance, the class name will be used for the search.
     */
    void RemoveComponent(const std::string& rssInstanceName);

    /**
     * @brief Insert a module into the configuration. Modules listed in the configuration are loaded, but components are not
     * started automatically. This might be useful if the module contains utilities that might be loaded on demand.
     * @remarks Modules can only be added when running as standalone application.
     * @remarks The modules are only added once. If a module is present in either the component or the module list, it is not added
     * again, and the function returns 'true'.
     * @param[in] nIndex The index location to insert the module before or when larger than the module list, at the end of the
     * module list.
     * @param[in] rpathModule Reference to the path of the module.
     * @return Returns whether inserting the module was successful. If a module with the same path is installed, the function
     * succeeds.
     */
    bool InsertModule(size_t nIndex, const std::filesystem::path& rpathModule);

    /**
     * @brief Remove the module with the provided module name.
     * @remarks Removing the module will also remove any components that are registered for the module.
     * @remarks Modules can only be removed when running as standalone application.
     * @param[in] rpathModule Reference to the path of the module.
     */
    void RemoveModule(const std::filesystem::path& rpathModule);

    enum class EMergeResult
    {
        successful,
        partly_successfull,
        not_successful
    };

    /**
     * @brief Merge a configuration into this configuration.
     * @details All modules, classes and components will be added or updated. If a classes already exist, the class parameters will
     * be overwritten (or removed if present before). If a component already exist, the component parameters will be updated (new
     * parameters added, parameters existing in both configurations updated and other parameters left unchanged).
     * @param[in] rpathConfigFile Reference to the config file to merge.
     * @return Returns whether the configuration could be merged.
    */
    EMergeResult MergeConfigFile(const std::filesystem::path& rpathConfigFile);

private:
    std::filesystem::path           m_pathConfigFile;       ///< Path to the configuration file. If running as main, isolated or
                                                            ///< maintenance application, the config file must be located at the
                                                            ///< installation directory.
    bool                            m_bLoaded = false;      ///< Set when the configuration has loaded.
    std::vector<SComponent>         m_vecComponentList;     ///< Component list containing instantiation info for components.
    std::vector<sdv::SClassInfo>    m_vecClassList;         ///< Class list containing default parameters for component classes.
    std::vector<SModule>            m_vecModuleList;        ///< Module list containing the module paths.
};


#endif // !defined APP_CONFIG_FILE_H
