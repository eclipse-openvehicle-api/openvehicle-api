/********************************************************************************
 * Copyright (c) 2065-2066 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Denisa Ros - initial API and implementation
 ********************************************************************************/

#include <gtest/gtest.h>
#include "../../../sdv_services/process_control/process_control.h"
#include "../../../global/ipc_named_mutex.h"
#include "../../../global/exec_dir_helper.h"
#include "generated/test_component.h"
#include <interfaces/app.h>
#include <support/app_control.h>
#include <support/mem_access.h>
#include <support/sdv_core.h>
#include <fstream>
#include <sstream>

namespace
{
void ExpectConfiguredProvider(uint32_t instanceId, const std::string& configName, bool isListener, const std::string& expectedProvider)
{
    auto* pConnections = sdv::core::GetObject<sdv::app::IAppConnections>("AppSettingsService");
    if (pConnections)
    {
        const sdv::u8string cfg = isListener ? pConnections->GetListenerConfig(configName)
                                             : pConnections->GetConnectionConfig(configName);
        ASSERT_FALSE(cfg.empty()) << "Missing IPC config '" << configName << "'";

        const std::string cfgStr = cfg;
        EXPECT_NE(cfgStr.find("Name = \"" + expectedProvider + "\""), std::string::npos)
            << "Unexpected provider config for '" << configName << "':\n" << cfgStr;
        return;
    }

    // In main app mode this service is not guaranteed to be directly accessible via repository object lookup.
    // Fallback: validate the installed instance settings file where listener/connection definitions are persisted.
    std::filesystem::path installDir;
    if (const char* pInstall = std::getenv("SDV_COMPONENT_INSTALL"))
        installDir = pInstall;
    else
        installDir = (GetExecDirectory() / ".." / ".." / "bin").lexically_normal();

    const auto settingsPath = installDir / std::to_string(instanceId) / "settings.toml";
    std::ifstream settingsStream(settingsPath);
    ASSERT_TRUE(settingsStream.is_open()) << "Cannot open settings file: " << settingsPath.generic_u8string();

    std::stringstream buffer;
    buffer << settingsStream.rdbuf();
    const std::string content = buffer.str();

    const std::string blockStart = isListener ? "[[Settings.Listener]]" : "[[Settings.Connection]]";
    const std::string providerSection = isListener ? "[Settings.Listener.Provider]" : "[Settings.Connection.Provider]";
    const std::string expectedName = "Name = \"" + configName + "\"";
    const std::string expectedProviderName = "Name = \"" + expectedProvider + "\"";

    size_t pos = content.find(blockStart);
    while (pos != std::string::npos)
    {
        const size_t nextPos = content.find(blockStart, pos + blockStart.size());
        const std::string block = content.substr(pos, (nextPos == std::string::npos) ? std::string::npos : nextPos - pos);
        if (block.find(expectedName) != std::string::npos)
        {
            EXPECT_NE(block.find(providerSection), std::string::npos)
                << "Missing provider section in config block for '" << configName << "'";
            EXPECT_NE(block.find(expectedProviderName), std::string::npos)
                << "Unexpected provider in settings block for '" << configName << "':\n" << block;
            return;
        }
        pos = nextPos;
    }

    FAIL() << "Missing IPC config block '" << configName << "' in " << settingsPath.generic_u8string();
}
} // namespace


// The test use the following instance numbers (all configured as main application):
//      - instance 203 server without additional listener configured (channel: DEFAULT_CHANNEL_203)
//      - instance 204 server with additional listener configured (channels:  private_channel_204_206)
//      - instance 205 client with 203-default connection configured (channel: DEFAULT_CHANNEL_203)
//      - instance 206 client with 204-special connection configured (channel: private_channel_204_206)
TEST(AppConnect_UDS_Tunnel, ServerStart_DefaultUDS_Tunnel)
{
    // The default instance
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 203
[Console]
RedirectMon = true
)toml");

    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 203u);

    // Test utility
    SStages sStages{};
    std::string ss;
    auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
    EXPECT_TRUE(ptrUtilityHelloObject);
    auto pHelloObject = ptrUtilityHelloObject.GetInterface<IHello>();
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_FALSE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Utility reports:" << std::endl << ss;

    // Test system object
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "System service reports:" << std::endl << ss;

    // Test device object - executes SayHello on Utility and System Service
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_HelloDevice");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_TRUE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Device reports:" << std::endl << ss;

    // Test basic service object - executes SayHello on Device, Utility and System Service
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_TRUE(sStages.bDevice);
    EXPECT_TRUE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Basic service reports:" << std::endl << ss;

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(AppConnect_UDS_Tunnel, ServerStart_PrivateUDS_Tunnel)
{
    // The default instance
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 204
[Console]
RedirectMon = true
)toml");

    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 204u);
    ExpectConfiguredProvider(204u, "MySpecialListener", true, "unix_domain_sockets_tunnel");

    // Test utility
    SStages sStages{};
    std::string ss;
    auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
    EXPECT_TRUE(ptrUtilityHelloObject);
    auto pHelloObject = ptrUtilityHelloObject.GetInterface<IHello>();
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_FALSE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Utility reports:" << std::endl << ss;

    // Test system object
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "System service reports:" << std::endl << ss;

    // Test device object - executes SayHello on Utility and System Service
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_HelloDevice");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_TRUE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Device reports:" << std::endl << ss;

    // Test basic service object - executes SayHello on Device, Utility and System Service
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_TRUE(sStages.bDevice);
    EXPECT_TRUE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Basic service reports:" << std::endl << ss;

    //// Test complex service object - executes SayHello on Complex Service, Device, Utility and System Service
    // sStages = {};
    // ss.clear();
    // pHelloObject = sdv::core::GetObject<IHello>("TestObject_ComplexHelloService");
    // EXPECT_TRUE(pHelloObject);
    // if (pHelloObject) ss = pHelloObject->SayHello(sStages);
    // EXPECT_FALSE(ss.empty());
    // EXPECT_TRUE(sStages.bDevice);
    // EXPECT_TRUE(sStages.bBasicService);
    // EXPECT_TRUE(sStages.bComplexService);
    // EXPECT_TRUE(sStages.bSystemService);
    // EXPECT_TRUE(sStages.bUtility);
    // std::cout << "Basic service reports:" << std::endl << ss;

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(AppConnect_UDS_Tunnel, ServerApp_Control_UDS_Tunnel)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_tunnel_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"203"}, sdv::process::EProcessRights::default_rights);
    EXPECT_NE(tServerID, 0u);

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server
    EXPECT_FALSE(procctrl.WaitForTerminate(tServerID, 1));

    // Release the mutex; this should shutdown the server
    lock.unlock();

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server - should be terminated
    EXPECT_TRUE(procctrl.WaitForTerminate(tServerID, 10000));

    // Just in case termination
    procctrl.Terminate(tServerID);

    // End process control
    procctrl.Shutdown();
}

TEST(AppConnect_UDS_Tunnel, ClientConnect_DefaultUDS_Tunnel)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_tunnel_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"203"}, sdv::process::EProcessRights::default_rights);
    EXPECT_NE(tServerID, 0u);

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server
    EXPECT_FALSE(procctrl.WaitForTerminate(tServerID, 1));

    // Start the second system using the default instance. This simulates a client
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    // When startup succeeds an automatic connection to the server 203 should have been made.
    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 205
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);
    ExpectConfiguredProvider(205u, "DefaultChannelTo203", false, "unix_domain_sockets_tunnel");

    // Test utility - not allowed
    SStages sStages{};
    std::string ss;
    auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
    EXPECT_FALSE(ptrUtilityHelloObject);

    // Test system object - not allowed BUT CURRENTLY WORKING
    sStages = {};
    ss.clear();
    auto pHelloObject = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "System service reports:" << std::endl << ss;

    //// Test device object - not allowed BUT CURRENTLY WORKING
    //sStages = {};
    //ss.clear();
    //pHelloObject = sdv::core::GetObject<IHello>("TestObject_HelloDevice");
    //EXPECT_TRUE(pHelloObject);
    //if (pHelloObject)
    //    ss = pHelloObject->SayHello(sStages);
    //EXPECT_FALSE(ss.empty());
    //EXPECT_TRUE(sStages.bDevice);
    //EXPECT_FALSE(sStages.bBasicService);
    //EXPECT_FALSE(sStages.bComplexService);
    //EXPECT_TRUE(sStages.bSystemService);
    //EXPECT_TRUE(sStages.bUtility);
    //std::cout << "Device reports:" << std::endl << ss;

    // Test basic service object - tries to executes SayHello on Device, Utility and System Service, but not allowed
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_TRUE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_FALSE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "Basic service reports:" << std::endl << ss;

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    // Release the mutex; this should shutdown the server
    lock.unlock();

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server - should be terminated
    EXPECT_TRUE(procctrl.WaitForTerminate(tServerID, 10000));

    // Just in case termination
    procctrl.Terminate(tServerID);

    // End process control
    procctrl.Shutdown();
}

TEST(AppConnect_UDS_Tunnel, ClientConnect_PrivateUDS_Tunnel)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_tunnel_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"204"}, sdv::process::EProcessRights::default_rights);
    EXPECT_NE(tServerID, 0u);

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server
    EXPECT_FALSE(procctrl.WaitForTerminate(tServerID, 1));

    // Start the second system using the default instance. This simulates a client
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    // When startup succeeds an automatic connection to the server 204 should have been made.
    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 206
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);
    ExpectConfiguredProvider(206u, "MySpecialConnection", false, "unix_domain_sockets_tunnel");

    // Test utility - not allowed
    SStages sStages{};
    std::string ss;
    auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
    EXPECT_FALSE(ptrUtilityHelloObject);

    // Test system object - not allowed BUT CURRENTLY WORKING
    sStages = {};
    ss.clear();
    auto pHelloObject = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_FALSE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "System service reports:" << std::endl << ss;

    //// Test device object - not allowed BUT CURRENTLY WORKING
    //sStages = {};
    //ss.clear();
    //pHelloObject = sdv::core::GetObject<IHello>("TestObject_HelloDevice");
    //EXPECT_TRUE(pHelloObject);
    //if (pHelloObject)
    //    ss = pHelloObject->SayHello(sStages);
    //EXPECT_FALSE(ss.empty());
    //EXPECT_TRUE(sStages.bDevice);
    //EXPECT_FALSE(sStages.bBasicService);
    //EXPECT_FALSE(sStages.bComplexService);
    //EXPECT_TRUE(sStages.bSystemService);
    //EXPECT_TRUE(sStages.bUtility);
    //std::cout << "Device reports:" << std::endl << ss;

    // Test basic service object  - tries to executes SayHello on Device, Utility and System Service, but not allowed
    sStages = {};
    ss.clear();
    pHelloObject = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject)
        ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_FALSE(sStages.bDevice);
    EXPECT_TRUE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_FALSE(sStages.bSystemService);
    EXPECT_FALSE(sStages.bUtility);
    std::cout << "Basic service reports:" << std::endl << ss;

    //// Test complex service object - executes SayHello on Complex Service, Device, Utility and System Service
    // sStages = {};
    // ss.clear();
    // pHelloObject = sdv::core::GetObject<IHello>("TestObject_ComplexHelloService");
    // EXPECT_TRUE(pHelloObject);
    // if (pHelloObject) ss = pHelloObject->SayHello(sStages);
    // EXPECT_FALSE(ss.empty());
    // EXPECT_TRUE(sStages.bDevice);
    // EXPECT_TRUE(sStages.bBasicService);
    // EXPECT_TRUE(sStages.bComplexService);
    // EXPECT_TRUE(sStages.bSystemService);
    // EXPECT_TRUE(sStages.bUtility);
    // std::cout << "Basic service reports:" << std::endl << ss;

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    // Release the mutex; this should shutdown the server
    lock.unlock();

    // Wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Test for the running server - should be terminated
    EXPECT_TRUE(procctrl.WaitForTerminate(tServerID, 10000));

    // Just in case termination
    procctrl.Terminate(tServerID);

    // End process control
    procctrl.Shutdown();
}
