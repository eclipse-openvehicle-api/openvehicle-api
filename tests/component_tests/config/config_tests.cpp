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

#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include <filesystem>
#include "../../../global/exec_dir_helper.h"
#include "../../../global/base64.h"

#ifdef _WIN32
#include <Shlobj.h>
#endif

// TODO:
// - Load config (server/standalone)
// - Load config with dependencies (server/standalone)
// - Close config (server/standalone)
// - Load + reload same config (server/standalone)
// - Load + load different config (server/standalone)
// - Save config (standalone)
// - Start + Load + Shutdown + Start (server)
// - System config + load user config (server)
// - System config + reload user config (server)
// - System config + load and close and load different user config (server)
// - System config with changed parameters + load config (server)

// Application config tests
// Load config file - Save config file - identical?
// Load config file - add module - save config file - module added?
// Load config file - add and delete module - identical?
// Load config file - add component - save config file - module added?
// Load config file - add and delete component - identical?
// Load config file - load another config file - Save config file - identical to last config file?
// Load config file - reset baseline - add module - save config file - only module in config?
// Load config file - reset baseline - add component - save config file - only component in config?
// Add module - save config - only module in config?
// Add component - save config - only component in config?
// Add config file in Startup - config loaded?
// For main app: add system config file in settings - config loaded?
// For main app: add multiple system config file in settings - config loaded?
// For main app: add application config file in settings - config loaded?
// For main app: add multiple system config files and an application config in settings - configs loaded?
// For main app: add application config file in settings - config loaded? - add component - auto save config - config updated with
// additional compoment? For main app: add multiple system config files and an application config file in settings - config loaded?
// - add component - auto save config - config updated with additional compoment? For main app: add system config file in settings -
// add config in application startup - configs loaded? For main app: add multiple system config file in settings - add config in
// application startup - configs loaded? For main app: add application config file in settings - add config in application startup -
// startup config loaded (and otherone not)? For main app: add application config file in settings - add config in application
// startup - add service - startup config not updated? Settings config also not? Test search algorithm with multiple modules with
// similar name in different location for standalone and essential Test search algorithm with multiple modules with similar name in
// different location for main and isolated Test configuration saving of module path shoudl not change when module is found
// somewhere else. Test not existing config Test partially existing config (some modules/components do not exist) Test multiple
// manifest for loading components

TEST(ConfigTest, LoadConfigStandalone)
{
//    sdv::app::CAppControl control;
//    bool bResult = control.Startup("[Application]\nMode = \"Standalone\"");
//    EXPECT_TRUE(bResult);
//
//    std::string ssConfig = R"toml([Configuration]
//Version = 100
//
//)toml";
//
//    control
//        .LoadConfig("")
//
//    control.Shutdown();
}

TEST(ConfigTest, LoadConfigServer)
{
    //sdv::app::CAppControl control;
    //bool bResult = control.Startup("[Application]\nMode=\"Main\"\nInstance=4000");
    //EXPECT_TRUE(bResult);

    //control.Shutdown();
}

