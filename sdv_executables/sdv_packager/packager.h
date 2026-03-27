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

#ifndef PACKAGER_H
#define PACKAGER_H

#include "core_control.h"
#include "environment.h"
#include "../../sdv_services/core/installation_manifest.h"

/**
 * @brief Packager class.
*/
class CPackager
{
public:
    /**
     * @brief Constructor
     * @param[in] renv Reference to the environment object.
     */
    CPackager(CSdvPackagerEnvironment& renv);

    /**
     * @brief Execute the packager dependable on the environment settings.
     * @return Returns whether the execution was successful.
     */
    bool Execute();

    /**
     * @brief Was a commandline parse error or a logical error with command line arguments detected?
     * @return Returns the result of the command line parsing. If no error occurs, NO_ERROR is returned.
     */
    int Error() const;

    /**
     * @brief Parse error that might have occurred.
     * @return Returns a reference to the member variable containing the error text.
     */
    const std::string& ArgError() const;

private:
    /**
     * @brief Pack files into a package. Uses the environment settings to create the package.
     * @return Returns whether the packing operation was successful.
     */
    bool Pack();

    /**
     * @brief Unpack an installation package into the target location. Uses the environment settings to unpack the package.
     * @return Returns whether the packing opereration was successful.
     */
    bool Unpack();

    /**
     * @brief Copy files from the source location to the target location (direct installation). Uses the environment settings to
     * copy the files.
     * @return Returns whether the copy operation was successful.
     */
    bool Copy();

    /**
     * @brief Configure the system with configuration files from the source location. Uses the environment settings to retrieve the
     * configuration files and the set the target configuration the files should integrate into.
     * @return Returns whether the configuration operation was successful.
     */
    bool Configure();

    /**
     * @brief Remove an installation from the target location. Uses the environment settings to remove the files.
     * @return Returns whether the removal operation was successful.
     */
    bool Remove();

    /**
     * @brief Check package integrity. Uses the environment settings to check the package.
     * @return Returns whether the integrity operation was successful.
     */
    bool CheckIntegrity();

    /**
     * @brief Show package content. Uses the environment settings to show the package content.
     * @return Returns whether the show operation was successful.
     */
    bool ShowContent();

private:
    /**
     * @brief After copying files, it is possible to configure the system using the manifest.
     * @param[in] rmanifest Reference to the manifest holding the objects that were installed.
     * @return Returns whether the copy operation was successful.
     */
    bool ConfigureFromManifest(const CInstallManifest& rmanifest);

    /**
     * @brief Create/update a configuration file
     * @param[in] rvecAllClasses Reference to the vector containing all component classes installed by the packager.
     * @param[in] rpathConfig Reference to the path of the configuration file.
     * @param[in] rvecComponents Reference to the components to add to the configuration.
     * @param[in] bUserConfig Set when the configuration is an user configuration; otherwise the configuration is a system
     * configuration.
     * @param[in, out] rvecAddedToConfig Reference to the vector containing and being updated with all the installed components.
     * This vector is used to prevent adding the component to more than configuration.
     */
    void WriteConfig(const std::vector<sdv::SClassInfo>& rvecAllClasses, const std::filesystem::path& rpathConfig,
        const CSdvPackagerEnvironment::CComponentVector& rvecComponents, bool bUserConfig,
        CSdvPackagerEnvironment::CComponentVector& rvecAddedToConfig);

    /**
     * @brief Draw a table using the two dimensional vector with information.
     * @param[in] rvecInfoTable Reference to the two dimensional vector containing the table information.
     * @param[in] bSimple When set, draws a one column table only. Otherwise all the table information is included.
     */
    static void DrawTable(const std::vector<std::vector<std::string>>& rvecInfoTable, bool bSimple);
    
    CSdvPackagerEnvironment     m_env;                              ///< The packager environment
    int                         m_nError = NO_ERROR;                ///< Error code after processing the command line.
    std::string                 m_ssArgError;                       ///< Error text after processing the command line.
};


#endif // !defined PACKAGER_H