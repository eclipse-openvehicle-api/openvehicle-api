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
#include "../../../sdv_services/process_control/process_control.h"
#include "../../../global/ipc_named_mutex.h"
#include "../../../global/exec_dir_helper.h"
#include "generated/test_component.h"
#include <interfaces/app.h>
#include <support/app_control.h>
#include <support/mem_access.h>
#include <support/sdv_core.h>

// The test use the following instance numbers (all configured as main application):
//      - instance 199 server without additional listener configured (channel: DEFAULT_CHANNEL_199)
//      - instance 200 server with additional listener configured (channels: DEFAULT_CHANNEL_200 and private_channel_200_202)
//      - instance 201 client with 199-default connection configured (channel: DEFAULT_CHANNEL_199)
//      - instance 202 client with 200-special connection configured (channel: private_channel_200_202)

TEST(AppConnect_UDS, ServerStart_DefaultUnixDomainSockets)
{
    // The default instance
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 199
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 199u);

    // Within the server instance all objects should be accessible.
    // NOTE: The complex service is running isolated.

    // Test utility
    SStages sStages{};
    std::string ss;
    auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
    EXPECT_TRUE(ptrUtilityHelloObject);
    auto pHelloObject = ptrUtilityHelloObject.GetInterface<IHello>();
    EXPECT_TRUE(pHelloObject);
    if (pHelloObject) ss = pHelloObject->SayHello(sStages);
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
    if (pHelloObject) ss = pHelloObject->SayHello(sStages);
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
    if (pHelloObject) ss = pHelloObject->SayHello(sStages);
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
    if (pHelloObject) ss = pHelloObject->SayHello(sStages);
    EXPECT_FALSE(ss.empty());
    EXPECT_TRUE(sStages.bDevice);
    EXPECT_TRUE(sStages.bBasicService);
    EXPECT_FALSE(sStages.bComplexService);
    EXPECT_TRUE(sStages.bSystemService);
    EXPECT_TRUE(sStages.bUtility);
    std::cout << "Basic service reports:" << std::endl << ss;

    //// Test complex service object - executes SayHello on Complex Service, Device, Utility and System Service
    //sStages = {};
    //ss.clear();
    //pHelloObject = sdv::core::GetObject<IHello>("TestObject_ComplexHelloService");
    //EXPECT_TRUE(pHelloObject);
    //if (pHelloObject) ss = pHelloObject->SayHello(sStages);
    //EXPECT_FALSE(ss.empty());
    //EXPECT_TRUE(sStages.bDevice);
    //EXPECT_TRUE(sStages.bBasicService);
    //EXPECT_TRUE(sStages.bComplexService);
    //EXPECT_TRUE(sStages.bSystemService);
    //EXPECT_TRUE(sStages.bUtility);
    //std::cout << "Basic service reports:" << std::endl << ss;

    control.Shutdown();
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);
}

TEST(AppConnect_UDS, ServerStart_PrivateUnixDomainSockets)
{
    // The default instance
    sdv::app::CAppControl control;
    EXPECT_FALSE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(control.GetInstanceID(), 0u);

    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 200
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(control.IsRunning());
    EXPECT_EQ(control.GetAppContext(), sdv::app::EAppContext::main);
    EXPECT_EQ(control.GetInstanceID(), 200u);

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

TEST(AppConnect_UDS, ServerApp_Control_UnixDomainSockets)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"199"}, sdv::process::EProcessRights::default_rights);
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

TEST(AppConnect_UDS, ClientConnect_DefaultUnixDomainSockets)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"199"}, sdv::process::EProcessRights::default_rights);
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

    // When startup succeeds an automatic connection to the server 199 should have been made.
    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 201
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);

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

TEST(AppConnect_UDS, ClientConnect_PrivateUnixDomainSockets)
{
    // The lifetime of the server app is managed through a named mutex being available for locking. Lock the mutex
    ipc::named_mutex mtx("app_connect_test_mutex");
    std::unique_lock<ipc::named_mutex> lock(mtx);

    // Use the process control class to manage the server process
    // Enable access bypass to allow its usage.
    CProcessControl procctrl;
    procctrl.EnableProcessControlAccessBypass();
    procctrl.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(procctrl.GetObjectState(), sdv::EObjectState::initialized);

    // Execute the server process
    sdv::process::TProcessID tServerID = procctrl.Execute(
        (GetExecDirectory() / "AppConnect_Server").generic_u8string(), {"200"}, sdv::process::EProcessRights::default_rights);
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

    // When startup succeeds an automatic connection to the server 200 should have been made.
    bool bResult = control.Startup(R"toml([Application]
Mode = "Main"
Instance = 202
[Console]
RedirectMon = true
)toml");
    EXPECT_TRUE(bResult);

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
