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


#include "include.h"

TEST(IPC_Communication_Test_UDS, AssignServerEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create a channel endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("unix_domain_sockets");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    EXPECT_NE(sChannelEndpoint.ssConnectString.find("proto=uds"), std::string::npos);
    sdv::TObjectPtr ptrEndpoint = sChannelEndpoint.pConnection; // Automatic lifetime management
    EXPECT_TRUE(ptrEndpoint);

    // Assign the server endpoint
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrEndpoint, &test, 100, false) != 0u);
    ptrEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, AssignClientEndpoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("unix_domain_sockets");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("unix_domain_sockets");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, false) != 0u);
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy) != 0u);
    EXPECT_NE(pObjectProxy, nullptr);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, AssignClientEndpoint_Reconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("unix_domain_sockets");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("unix_domain_sockets");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, true) != 0u);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    sdv::ipc::IConnect* pClientConnect = ptrClientEndpoint.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnect, nullptr);
    EXPECT_EQ(pClientConnect->GetConnectState(), sdv::ipc::EConnectState::connected);
    control.RemoveConnection(tConnectionID);
    EXPECT_EQ(pClientConnect->GetConnectState(), sdv::ipc::EConnectState::disconnected);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Create another client endpoint
    ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the new client endpoint
    pObjectProxy = nullptr;
    tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, AssignClientEndpoint_FailReconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create a server endpoint
    sdv::ipc::ICreateEndpoint* ptrCreateEndpoint = sdv::core::GetObject<sdv::ipc::ICreateEndpoint>("unix_domain_sockets");
    ASSERT_NE(ptrCreateEndpoint, nullptr);
    sdv::ipc::SChannelEndpoint sChannelEndpoint = ptrCreateEndpoint->CreateEndpoint("");
    EXPECT_NE(sChannelEndpoint.pConnection, nullptr);
    EXPECT_FALSE(sChannelEndpoint.ssConnectString.empty());
    sdv::TObjectPtr ptrServerEndpoint = sChannelEndpoint.pConnection;     // Automatic lifetime management
    EXPECT_TRUE(ptrServerEndpoint);

    // Create a client endpoint
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>("unix_domain_sockets");
    ASSERT_NE(pChannelAccess, nullptr);
    sdv::TObjectPtr ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the server and client endpoints
    CInterfaceTest test;
    EXPECT_TRUE(control.AssignServerEndpoint(ptrServerEndpoint, &test, 100, false) != 0u);
    ptrServerEndpoint.Clear();    // Lifetime taken over by communication control.
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    sdv::ipc::IConnect* pClientConnect = ptrClientEndpoint.GetInterface<sdv::ipc::IConnect>();
    ASSERT_NE(pClientConnect, nullptr);
    EXPECT_EQ(pClientConnect->GetConnectState(), sdv::ipc::EConnectState::connected);
    control.RemoveConnection(tConnectionID);
    EXPECT_EQ(pClientConnect->GetConnectState(), sdv::ipc::EConnectState::disconnected);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Create another client endpoint
    ptrClientEndpoint = pChannelAccess->Access(sChannelEndpoint.ssConnectString);
    EXPECT_TRUE(ptrClientEndpoint);

    // Assign the new client endpoint - this should fail!
    pObjectProxy = nullptr;
    tConnectionID = control.AssignClientEndpoint(ptrClientEndpoint, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID == 0u);
    EXPECT_EQ(pObjectProxy, nullptr);
    ptrClientEndpoint.Clear();    // Lifetime taken over by communication control.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, CreateServerConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, CreateClientConnection)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, CreateClientConnection_FailReconnect)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    sdv::com::TConnectionID tConnectionID = control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID != 0u);
    EXPECT_NE(pObjectProxy, nullptr);

    // Disconnect from the client
    control.RemoveConnection(tConnectionID);

    // Create a new client connection
    pObjectProxy = nullptr;
    tConnectionID = control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy);
    EXPECT_TRUE(tConnectionID == 0u);
    EXPECT_EQ(pObjectProxy, nullptr);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallClientToServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the ISayHello interface
    ISayHello* pSayHello = nullptr;
    EXPECT_NO_THROW(pSayHello = pObjectProxy->GetInterface<ISayHello>());
    ASSERT_NE(pSayHello, nullptr);

    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallInterfaceFromServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRequestHello interface
    IRequestHello* pRequestHello = nullptr;
    EXPECT_NO_THROW(pRequestHello = pObjectProxy->GetInterface<IRequestHello>());
    ASSERT_NE(pRequestHello, nullptr);

    // Request the ISayHello interface
    ISayHello* pSayHello = pRequestHello->Request();

    // Call the interface as if talking from the client
    ASSERT_NE(pSayHello, nullptr);
    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallInterfaceFromClient)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRegisterHelloCallback interface
    IRegisterHelloCallback* pRegisterHelloCallback = nullptr;
    EXPECT_NO_THROW(pRegisterHelloCallback = pObjectProxy->GetInterface<IRegisterHelloCallback>());
    ASSERT_NE(pRegisterHelloCallback, nullptr);

    // Register the callback (stored in the object)
    pRegisterHelloCallback->Register(&test);

    // Call the interface as if talking from the server
    ASSERT_NE(test.m_pHello, nullptr);
    EXPECT_EQ(test.m_pHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallGenericInterfaceFromServer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Request the IRegisterHelloCallback interface
    sdv::IInterfaceAccess* pRegisterHelloCallback = nullptr;
    EXPECT_NO_THROW(pRegisterHelloCallback = pObjectProxy->GetInterface<sdv::IInterfaceAccess>());
    ASSERT_NE(pRegisterHelloCallback, nullptr);

    // Call GetInterface to get the IInterfaceAccess pointer once more (to be able to test marshalling).
    sdv::IInterfaceAccess* pInterfaceAccess = pObjectProxy->GetInterface<sdv::IInterfaceAccess>();
    ASSERT_NE(pInterfaceAccess, nullptr);

    // Request the pSayHello interface (this will be marshalled as well)
    ISayHello* pSayHello = pInterfaceAccess->GetInterface<ISayHello>();

    // Call the interface as if talking from the client
    ASSERT_NE(pSayHello, nullptr);
    EXPECT_EQ(pSayHello->Hello(), "Hello");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.bVal = false;
    EXPECT_FALSE(sStruct.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.sVal = -10;
    sStruct.usVal = 10;
    sStruct.lVal = -20;
    sStruct.ulVal = 20;
    sStruct.llVal = -30;
    sStruct.ullVal = 30;
    sStruct.i8Val = -40;
    sStruct.ui8Val = 40;
    sStruct.i16Val = -50;
    sStruct.ui16Val = 50;
    sStruct.i32Val = -60;
    sStruct.ui32Val = 60;
    sStruct.i64Val = -70;
    sStruct.ui64Val = 70;
    sStruct.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.sVal, -11);
    EXPECT_EQ(sStruct.usVal, 11u);
    EXPECT_EQ(sStruct.lVal, -21);
    EXPECT_EQ(sStruct.ulVal, 21u);
    EXPECT_EQ(sStruct.llVal, -31);
    EXPECT_EQ(sStruct.ullVal, 31u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.ui8Val, 41u);
    EXPECT_EQ(sStruct.i16Val, -51);
    EXPECT_EQ(sStruct.ui16Val, 51u);
    EXPECT_EQ(sStruct.i32Val, -61);
    EXPECT_EQ(sStruct.ui32Val, 61u);
    EXPECT_EQ(sStruct.i64Val, -71);
    EXPECT_EQ(sStruct.ui64Val, 71u);
    EXPECT_EQ(sStruct.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.cVal = 'A';
    sStruct.c16Val = u'D';
    sStruct.c32Val = U'G';
    sStruct.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.cVal, 'B');
    EXPECT_EQ(sStruct.c16Val, u'E');
    EXPECT_EQ(sStruct.c32Val, U'H');
    EXPECT_EQ(sStruct.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.fVal = fVal;
    double dVal = 8765.4321;
    sStruct.dVal = dVal;
    long double ldVal = -1234.4321;
    sStruct.ldVal = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.fVal, fVal - 1000.000f);
    EXPECT_EQ(sStruct.dVal, dVal + 1000.000);
    EXPECT_EQ(sStruct.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.ssVal = "hi";
    sStruct.ss8Val = "huhu";
    sStruct.ss16Val = u"hey";
    sStruct.ss32Val = U"hello";
    sStruct.wssVal = L"servus";
    sStruct.ssFixVal = "hi";
    sStruct.ss8FixVal = "huhu";
    sStruct.ss16FixVal = u"hey";
    sStruct.ss32FixVal = U"hello";
    sStruct.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.ssVal, "hij");
    EXPECT_EQ(sStruct.ss8Val, "huhuv");
    EXPECT_EQ(sStruct.ss16Val, u"heyz");
    EXPECT_EQ(sStruct.ss32Val, U"hellop");
    EXPECT_EQ(sStruct.wssVal, L"servust");
    EXPECT_EQ(sStruct.ssFixVal, "hij");
    EXPECT_EQ(sStruct.ss8FixVal, "huhuv");
    EXPECT_EQ(sStruct.ss16FixVal, u"heyz");
    EXPECT_EQ(sStruct.ss32FixVal, U"hellop");
    EXPECT_EQ(sStruct.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8Val.size(); nIndex++)
        sStruct.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.ptrssVal.resize(3);
    sStruct.ptrssVal[0] = "hello";
    sStruct.ptrssVal[1] = "huhu";
    sStruct.ptrssVal[2] = "hi";
    sStruct.ptr8FixVal.resize(sStruct.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sStruct.ptr8FixVal.size(); nIndex++)
        sStruct.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.ptrssFixVal.resize(sStruct.ptrssFixVal.capacity());
    sStruct.ptrssFixVal[0] = "hi";
    sStruct.ptrssFixVal[1] = "hoi";
    sStruct.ptrssFixVal[2] = "hello";
    sStruct.ptrssFixVal[3] = "hallo";
    sStruct.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.ptr8Val[nIndex], sStruct.ptr8Val[nIndex + sStruct.ptr8Val.size() / 2]);
    EXPECT_EQ(sStruct.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sStruct.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.ptrssVal[nIndex], sStruct.ptrssVal[nIndex + sStruct.ptrssVal.size() / 2]);
    EXPECT_EQ(sStruct.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(sStruct.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.ptrssFixVal.size(), 5);
    EXPECT_EQ(sStruct.ptrssFixVal[0], "hihi");
    EXPECT_EQ(sStruct.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(sStruct.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(sStruct.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(sStruct.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.seq8Val.size(); nIndex++)
        sStruct.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.seqssVal.resize(3);
    sStruct.seqssVal[0] = "hello";
    sStruct.seqssVal[1] = "huhu";
    sStruct.seqssVal[2] = "hi";
    sStruct.seq8FixVal.resize(sStruct.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sStruct.seq8FixVal.size(); nIndex++)
        sStruct.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.seqssFixVal.resize(sStruct.seqssFixVal.capacity());
    sStruct.seqssFixVal[0] = "hi";
    sStruct.seqssFixVal[1] = "hoi";
    sStruct.seqssFixVal[2] = "hello";
    sStruct.seqssFixVal[3] = "hallo";
    sStruct.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sStruct.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.seq8Val[nIndex], sStruct.seq8Val[nIndex + sStruct.seq8Val.size() / 2]);
    EXPECT_EQ(sStruct.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sStruct.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(sStruct.seqssVal[nIndex], sStruct.seqssVal[nIndex + sStruct.seqssVal.size() / 2]);
    EXPECT_EQ(sStruct.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(sStruct.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.seqssFixVal.size(), 5);
    EXPECT_EQ(sStruct.seqssFixVal[0], "hihi");
    EXPECT_EQ(sStruct.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(sStruct.seqssFixVal[2], "hellohello");
    EXPECT_EQ(sStruct.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(sStruct.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    sStruct.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    sStruct.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    sStruct.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.idVal, sdv::GetInterfaceId<IAddValue>());
    EXPECT_EQ(sStruct.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sStruct.ifcVal, nullptr);
    ASSERT_NE(sStruct.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sStruct.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    ASSERT_NE(sStruct.pMultiplyValue, nullptr);
    EXPECT_EQ(sStruct.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.sSubVal.i = 1;
    sStruct.sIndVal.i = 10;
    //sStruct.sUnnamedVal.i = 100;  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.sSubVal.i, 2);
    EXPECT_EQ(sStruct.sIndVal.i, 20);
    //EXPECT_EQ(sStruct.sUnnamedVal.i, 120);  // 12.04.2024 EVE Unnamed structures are not supported by IDL compiler.

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructBooleanArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgbVal[0] = false;
    sStruct.rgbVal[1] = true;
    EXPECT_FALSE(sStruct.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.rgbVal[0]);
    EXPECT_FALSE(sStruct.rgbVal[1]);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructIntegralArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgsVal[0] = -10;
    sStruct.rgsVal[1] = -11;
    sStruct.rgusVal[0] = 10;
    sStruct.rgusVal[1] = 11;
    sStruct.rglVal[0] = -20;
    sStruct.rglVal[1] = -21;
    sStruct.rgulVal[0] = 20;
    sStruct.rgulVal[1] = 21;
    sStruct.rgllVal[0] = -30;
    sStruct.rgllVal[1] = -31;
    sStruct.rgullVal[0] = 30;
    sStruct.rgullVal[1] = 31;
    sStruct.rgi8Val[0] = -40;
    sStruct.rgi8Val[1] = -41;
    sStruct.rgui8Val[0] = 40;
    sStruct.rgui8Val[1] = 41;
    sStruct.rgi16Val[0] = -50;
    sStruct.rgi16Val[1] = -51;
    sStruct.rgui16Val[0] = 50;
    sStruct.rgui16Val[1] = 51;
    sStruct.rgi32Val[0] = -60;
    sStruct.rgi32Val[1] = -61;
    sStruct.rgui32Val[0] = 60;
    sStruct.rgui32Val[1] = 61;
    sStruct.rgi64Val[0] = -70;
    sStruct.rgi64Val[1] = -71;
    sStruct.rgui64Val[0] = 70;
    sStruct.rgui64Val[1] = 71;
    sStruct.rgnVal[0] = 80;
    sStruct.rgnVal[1] = 81;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgsVal[0], -11);
    EXPECT_EQ(sStruct.rgsVal[1], -10);
    EXPECT_EQ(sStruct.rgusVal[0], 11u);
    EXPECT_EQ(sStruct.rgusVal[1], 13u);
    EXPECT_EQ(sStruct.rglVal[0], -21);
    EXPECT_EQ(sStruct.rglVal[1], -20);
    EXPECT_EQ(sStruct.rgulVal[0], 21u);
    EXPECT_EQ(sStruct.rgulVal[1], 23u);
    EXPECT_EQ(sStruct.rgllVal[0], -31);
    EXPECT_EQ(sStruct.rgllVal[1], -30);
    EXPECT_EQ(sStruct.rgullVal[0], 31u);
    EXPECT_EQ(sStruct.rgullVal[1], 33u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.rgi8Val[0]), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(static_cast<uint8_t>(sStruct.rgi8Val[1]), static_cast<uint8_t>(-40));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.rgui8Val[0], 41u);
    EXPECT_EQ(sStruct.rgui8Val[1], 43u);
    EXPECT_EQ(sStruct.rgi16Val[0], -51);
    EXPECT_EQ(sStruct.rgi16Val[1], -50);
    EXPECT_EQ(sStruct.rgui16Val[0], 51u);
    EXPECT_EQ(sStruct.rgui16Val[1], 53u);
    EXPECT_EQ(sStruct.rgi32Val[0], -61);
    EXPECT_EQ(sStruct.rgi32Val[1], -60);
    EXPECT_EQ(sStruct.rgui32Val[0], 61u);
    EXPECT_EQ(sStruct.rgui32Val[1], 63u);
    EXPECT_EQ(sStruct.rgi64Val[0], -71);
    EXPECT_EQ(sStruct.rgi64Val[1], -70);
    EXPECT_EQ(sStruct.rgui64Val[0], 71u);
    EXPECT_EQ(sStruct.rgui64Val[1], 73u);
    EXPECT_EQ(sStruct.rgnVal[0], 81);
    EXPECT_EQ(sStruct.rgnVal[1], 83);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructCharacterArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgcVal[0] = 'A';
    sStruct.rgcVal[1] = 'B';
    sStruct.rgc16Val[0] = u'D';
    sStruct.rgc16Val[1] = u'E';
    sStruct.rgc32Val[0] = U'G';
    sStruct.rgc32Val[1] = U'H';
    sStruct.rgwcVal[0] = L'J';
    sStruct.rgwcVal[1] = L'K';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgcVal[0], 'B');
    EXPECT_EQ(sStruct.rgcVal[1], 'D');
    EXPECT_EQ(sStruct.rgc16Val[0], u'E');
    EXPECT_EQ(sStruct.rgc16Val[1], u'G');
    EXPECT_EQ(sStruct.rgc32Val[0], U'H');
    EXPECT_EQ(sStruct.rgc32Val[1], U'J');
    EXPECT_EQ(sStruct.rgwcVal[0], L'K');
    EXPECT_EQ(sStruct.rgwcVal[1], L'M');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructFloatingPointArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.rgfVal[0] = fVal;
    sStruct.rgfVal[1] = fVal;
    double dVal = 8765.4321;
    sStruct.rgdVal[0] = dVal;
    sStruct.rgdVal[1] = dVal;
    long double ldVal = -1234.4321;
    sStruct.rgldVal[0] = ldVal;
    sStruct.rgldVal[1] = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgfVal[0], fVal - 1000.000f);
    EXPECT_EQ(sStruct.rgfVal[1], fVal + 2000.000f);
    EXPECT_EQ(sStruct.rgdVal[0], dVal + 1000.000);
    EXPECT_EQ(sStruct.rgdVal[1], dVal - 2000.000);
    EXPECT_EQ(sStruct.rgldVal[0], ldVal - 1000.000);
    EXPECT_EQ(sStruct.rgldVal[1], ldVal + 2000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructEnumArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgeHelloVal[0] = EHello::hello;
    sStruct.rgeHelloVal[1] = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgeHelloVal[0], EHello::hallo);
    EXPECT_EQ(sStruct.rgeHelloVal[1], EHello::huhu);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructStringArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgssVal[0] = "hi";
    sStruct.rgssVal[1] = "hi";
    sStruct.rgss8Val[0] = "huhu";
    sStruct.rgss8Val[1] = "huhu";
    sStruct.rgss16Val[0] = u"hey";
    sStruct.rgss16Val[1] = u"hey";
    sStruct.rgss32Val[0] = U"hello";
    sStruct.rgss32Val[1] = U"hello";
    sStruct.rgwssVal[0] = L"servus";
    sStruct.rgwssVal[1] = L"servus";
    sStruct.rgssFixVal[0] = "hi";
    sStruct.rgssFixVal[1] = "hi";
    sStruct.rgss8FixVal[0] = "huhu";
    sStruct.rgss8FixVal[1] = "huhu";
    sStruct.rgss16FixVal[0] = u"hey";
    sStruct.rgss16FixVal[1] = u"hey";
    sStruct.rgss32FixVal[0] = U"hello";
    sStruct.rgss32FixVal[1] = U"hello";
    sStruct.rgwssFixVal[0] = L"servus";
    sStruct.rgwssFixVal[1] = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgssVal[0], "hij");
    EXPECT_EQ(sStruct.rgssVal[1], "hii");
    EXPECT_EQ(sStruct.rgss8Val[0], "huhuv");
    EXPECT_EQ(sStruct.rgss8Val[1], "huhui");
    EXPECT_EQ(sStruct.rgss16Val[0], u"heyz");
    EXPECT_EQ(sStruct.rgss16Val[1], u"heyi");
    EXPECT_EQ(sStruct.rgss32Val[0], U"hellop");
    EXPECT_EQ(sStruct.rgss32Val[1], U"helloi");
    EXPECT_EQ(sStruct.rgwssVal[0], L"servust");
    EXPECT_EQ(sStruct.rgwssVal[1], L"servust");
    EXPECT_EQ(sStruct.rgssFixVal[0], "hij");
    EXPECT_EQ(sStruct.rgssFixVal[1], "hii");
    EXPECT_EQ(sStruct.rgss8FixVal[0], "huhuv");
    EXPECT_EQ(sStruct.rgss8FixVal[1], "huhui");
    EXPECT_EQ(sStruct.rgss16FixVal[0], u"heyz");
    EXPECT_EQ(sStruct.rgss16FixVal[1], u"heyi");
    EXPECT_EQ(sStruct.rgss32FixVal[0], U"hellop");
    EXPECT_EQ(sStruct.rgss32FixVal[1], U"helloi");
    EXPECT_EQ(sStruct.rgwssFixVal[0], L"servust");
    EXPECT_EQ(sStruct.rgwssFixVal[1], L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructPointerArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgptr8Val[0].resize(100);
    sStruct.rgptr8Val[1].resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[0].size(); nIndex++)
        sStruct.rgptr8Val[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[1].size(); nIndex++)
        sStruct.rgptr8Val[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgptrssVal[0].resize(3);
    sStruct.rgptrssVal[1].resize(3);
    sStruct.rgptrssVal[0][0] = "hello";
    sStruct.rgptrssVal[1][0] = "ohhello";
    sStruct.rgptrssVal[0][1] = "huhu";
    sStruct.rgptrssVal[1][1] = "ohhuhu";
    sStruct.rgptrssVal[0][2] = "hi";
    sStruct.rgptrssVal[1][2] = "ohhi";
    sStruct.rgptr8FixVal[0].resize(sStruct.rgptr8FixVal[0].capacity());
    sStruct.rgptr8FixVal[1].resize(sStruct.rgptr8FixVal[1].capacity());
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[0].size(); nIndex++)
        sStruct.rgptr8FixVal[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[1].size(); nIndex++)
        sStruct.rgptr8FixVal[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgptrssFixVal[0].resize(sStruct.rgptrssFixVal[0].capacity());
    sStruct.rgptrssFixVal[1].resize(sStruct.rgptrssFixVal[1].capacity());
    sStruct.rgptrssFixVal[0][0] = "hi";
    sStruct.rgptrssFixVal[1][0] = "ohhi";
    sStruct.rgptrssFixVal[0][1] = "hoi";
    sStruct.rgptrssFixVal[1][1] = "ohhoi";
    sStruct.rgptrssFixVal[0][2] = "hello";
    sStruct.rgptrssFixVal[1][2] = "ohhello";
    sStruct.rgptrssFixVal[0][3] = "hallo";
    sStruct.rgptrssFixVal[1][3] = "ohhallo";
    sStruct.rgptrssFixVal[0][4] = "servus";
    sStruct.rgptrssFixVal[1][4] = "ohservus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgptr8Val[0].size(), 200);
    EXPECT_EQ(sStruct.rgptr8Val[1].size(), 300);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgptr8Val[0][nIndex], sStruct.rgptr8Val[0][nIndex + sStruct.rgptr8Val[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8Val[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgptr8Val[1][nIndex], sStruct.rgptr8Val[1][nIndex + sStruct.rgptr8Val[1].size() / 3]);
    EXPECT_EQ(sStruct.rgptrssVal[0].size(), 6);
    EXPECT_EQ(sStruct.rgptrssVal[1].size(), 9);
    for (size_t nIndex = 0; nIndex < sStruct.rgptrssVal[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgptrssVal[0][nIndex], sStruct.rgptrssVal[0][nIndex + sStruct.rgptrssVal[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgptrssVal[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgptrssVal[1][nIndex], sStruct.rgptrssVal[1][nIndex + sStruct.rgptrssVal[1].size() / 3]);
    EXPECT_EQ(sStruct.rgptr8FixVal[0].size(), 20);
    EXPECT_EQ(sStruct.rgptr8FixVal[1].size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[0].size(); nIndex++)
        EXPECT_EQ(sStruct.rgptr8FixVal[0][nIndex], static_cast<uint8_t>(nIndex << 1));
    for (size_t nIndex = 0; nIndex < sStruct.rgptr8FixVal[1].size(); nIndex++)
        EXPECT_EQ(sStruct.rgptr8FixVal[1][nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.rgptrssFixVal[0].size(), 5);
    EXPECT_EQ(sStruct.rgptrssFixVal[1].size(), 5);
    EXPECT_EQ(sStruct.rgptrssFixVal[0][0], "hihi");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][0], "ohhiohhi");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][1], "hoihoi");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][1], "ohhoiohhoi");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][2], "hellohello");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][2], "ohhelloohhello");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][3], "hallohallo");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][3], "ohhalloohhallo");
    EXPECT_EQ(sStruct.rgptrssFixVal[0][4], "servusservus");
    EXPECT_EQ(sStruct.rgptrssFixVal[1][4], "ohservusohservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructSequenceArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgseq8Val[0].resize(100);
    sStruct.rgseq8Val[1].resize(100);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[0].size(); nIndex++)
        sStruct.rgseq8Val[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[1].size(); nIndex++)
        sStruct.rgseq8Val[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgseqssVal[0].resize(3);
    sStruct.rgseqssVal[1].resize(3);
    sStruct.rgseqssVal[0][0] = "hello";
    sStruct.rgseqssVal[1][0] = "ohhello";
    sStruct.rgseqssVal[0][1] = "huhu";
    sStruct.rgseqssVal[1][1] = "ohhuhu";
    sStruct.rgseqssVal[0][2] = "hi";
    sStruct.rgseqssVal[1][2] = "ohhi";
    sStruct.rgseq8FixVal[0].resize(sStruct.rgseq8FixVal[0].capacity());
    sStruct.rgseq8FixVal[1].resize(sStruct.rgseq8FixVal[1].capacity());
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[0].size(); nIndex++)
        sStruct.rgseq8FixVal[0][nIndex] = static_cast<uint8_t>(nIndex);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[1].size(); nIndex++)
        sStruct.rgseq8FixVal[1][nIndex] = static_cast<uint8_t>(nIndex);
    sStruct.rgseqssFixVal[0].resize(sStruct.seqssFixVal.capacity());
    sStruct.rgseqssFixVal[1].resize(sStruct.seqssFixVal.capacity());
    sStruct.rgseqssFixVal[0][0] = "hi";
    sStruct.rgseqssFixVal[1][0] = "ohhi";
    sStruct.rgseqssFixVal[0][1] = "hoi";
    sStruct.rgseqssFixVal[1][1] = "ohhoi";
    sStruct.rgseqssFixVal[0][2] = "hello";
    sStruct.rgseqssFixVal[1][2] = "ohhello";
    sStruct.rgseqssFixVal[0][3] = "hallo";
    sStruct.rgseqssFixVal[1][3] = "ohhallo";
    sStruct.rgseqssFixVal[0][4] = "servus";
    sStruct.rgseqssFixVal[1][4] = "ohservus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgseq8Val[0].size(), 200);
    EXPECT_EQ(sStruct.rgseq8Val[1].size(), 300);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgseq8Val[0][nIndex], sStruct.rgseq8Val[0][nIndex + sStruct.rgseq8Val[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8Val[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgseq8Val[1][nIndex], sStruct.rgseq8Val[1][nIndex + sStruct.rgseq8Val[1].size() / 3]);
    EXPECT_EQ(sStruct.rgseqssVal[0].size(), 6);
    EXPECT_EQ(sStruct.rgseqssVal[1].size(), 9);
    for (size_t nIndex = 0; nIndex < sStruct.rgseqssVal[0].size() / 2; nIndex++)
        EXPECT_EQ(sStruct.rgseqssVal[0][nIndex], sStruct.rgseqssVal[0][nIndex + sStruct.rgseqssVal[0].size() / 2]);
    for (size_t nIndex = 0; nIndex < sStruct.rgseqssVal[1].size() / 3; nIndex++)
        EXPECT_EQ(sStruct.rgseqssVal[1][nIndex], sStruct.rgseqssVal[1][nIndex + sStruct.rgseqssVal[1].size() / 3]);
    EXPECT_EQ(sStruct.rgseq8FixVal[0].size(), 20);
    EXPECT_EQ(sStruct.rgseq8FixVal[1].size(), 20);
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[0].size(); nIndex++)
        EXPECT_EQ(sStruct.rgseq8FixVal[0][nIndex], static_cast<uint8_t>(nIndex << 1));
    for (size_t nIndex = 0; nIndex < sStruct.rgseq8FixVal[1].size(); nIndex++)
        EXPECT_EQ(sStruct.rgseq8FixVal[1][nIndex], static_cast<uint8_t>(nIndex << 1));
    EXPECT_EQ(sStruct.rgseqssFixVal[0].size(), 5);
    EXPECT_EQ(sStruct.rgseqssFixVal[1].size(), 5);
    EXPECT_EQ(sStruct.rgseqssFixVal[0][0], "hihi");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][0], "ohhiohhi");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][1], "hoihoi");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][1], "ohhoiohhoi");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][2], "hellohello");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][2], "ohhelloohhello");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][3], "hallohallo");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][3], "ohhalloohhallo");
    EXPECT_EQ(sStruct.rgseqssFixVal[0][4], "servusservus");
    EXPECT_EQ(sStruct.rgseqssFixVal[1][4], "ohservusohservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructInterfaceArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    //sStruct.rgidVal[0] = sdv::GetInterfaceId<IMultiplyValue>();
    //sStruct.rgidVal[1] = sdv::GetInterfaceId<IMultiplyValue>();
    //sStruct.rgifcVal[0] = static_cast<IMultiplyValue*>(&sMultiply);
    //sStruct.rgifcVal[1] = static_cast<IMultiplyValue*>(&sMultiply);
    //sStruct.rgpMultiplyValue[0] = &sMultiply;
    //sStruct.rgpMultiplyValue[1] = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.rgidVal[0], sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgidVal[1], sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgifcVal[0].id(), sdv::GetInterfaceId<IAddValue>());
    //EXPECT_EQ(sStruct.rgifcVal[1].id(), sdv::GetInterfaceId<IAddValue>());
    //ASSERT_NE(sStruct.rgifcVal[0], nullptr);
    //ASSERT_NE(sStruct.rgifcVal[1], nullptr);
    //ASSERT_NE(sStruct.rgifcVal[0].get<IAddValue>(), nullptr);
    //ASSERT_NE(sStruct.rgifcVal[1].get<IAddValue>(), nullptr);
    //EXPECT_EQ(sStruct.rgifcVal[0].get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    //EXPECT_EQ(sStruct.rgifcVal[1].get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    //ASSERT_NE(sStruct.rgpMultiplyValue[0], nullptr);
    //ASSERT_NE(sStruct.rgpMultiplyValue[1], nullptr);
    //EXPECT_EQ(sStruct.rgpMultiplyValue[0]->Multiply(5), 625); // 5 * 5 * 5 * 5
    //EXPECT_EQ(sStruct.rgpMultiplyValue[1]->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructSubCompoundArray)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.rgsSubVal[0].i = 1;
    sStruct.rgsSubVal[1].i = 2;
    // Incompatible serdes generation: BUG #398509
    //sStruct.rgsIndVal[0].i = 10;
    //sStruct.rgsIndVal[1].i = 20;
    // Unnamed struct with array is not yet supported due to incorret serdes code generation: BUG #398246
    //sStruct.rgsUnnamedVal[0].i = 100;
    //sStruct.rgsUnnamedVal[1].i = 200;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.rgsSubVal[0].i, 2);
    EXPECT_EQ(sStruct.rgsSubVal[1].i, 4);
    //EXPECT_EQ(sStruct.rgsIndVal[0].i, 20);
    //EXPECT_EQ(sStruct.rgsIndVal[1].i, 40);
    //EXPECT_EQ(sStruct.rgsUnnamedVal[0].i, 120);
    //EXPECT_EQ(sStruct.rgsUnnamedVal[1].i, 240);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tbVal = false;
    EXPECT_FALSE(sStruct.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_TRUE(sStruct.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tsVal = -10;
    sStruct.tusVal = 10;
    sStruct.tlVal = -20;
    sStruct.tulVal = 20;
    sStruct.tllVal = -30;
    sStruct.tullVal = 30;
    sStruct.ti8Val = -40;
    sStruct.tui8Val = 40;
    sStruct.ti16Val = -50;
    sStruct.tui16Val = 50;
    sStruct.ti32Val = -60;
    sStruct.tui32Val = 60;
    sStruct.ti64Val = -70;
    sStruct.tui64Val = 70;
    sStruct.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tsVal, -11);
    EXPECT_EQ(sStruct.tusVal, 11u);
    EXPECT_EQ(sStruct.tlVal, -21);
    EXPECT_EQ(sStruct.tulVal, 21u);
    EXPECT_EQ(sStruct.tllVal, -31);
    EXPECT_EQ(sStruct.tullVal, 31u);
    EXPECT_EQ(static_cast<uint8_t>(sStruct.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    EXPECT_EQ(sStruct.tui8Val, 41u);
    EXPECT_EQ(sStruct.ti16Val, -51);
    EXPECT_EQ(sStruct.tui16Val, 51u);
    EXPECT_EQ(sStruct.ti32Val, -61);
    EXPECT_EQ(sStruct.tui32Val, 61u);
    EXPECT_EQ(sStruct.ti64Val, -71);
    EXPECT_EQ(sStruct.tui64Val, 71u);
    EXPECT_EQ(sStruct.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tcVal = 'A';
    sStruct.tc16Val = u'D';
    sStruct.tc32Val = U'G';
    sStruct.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tcVal, 'B');
    EXPECT_EQ(sStruct.tc16Val, u'E');
    EXPECT_EQ(sStruct.tc32Val, U'H');
    EXPECT_EQ(sStruct.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    float fVal = -1234.5678f;
    sStruct.tfVal = fVal;
    double dVal = 8765.4321;
    sStruct.tdVal = dVal;
    long double ldVal = -1234.4321;
    sStruct.tldVal = ldVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tfVal, fVal - 1000.000f);
    EXPECT_EQ(sStruct.tdVal, dVal + 1000.000);
    EXPECT_EQ(sStruct.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tssVal = "hi";
    sStruct.tss8Val = "huhu";
    sStruct.tss16Val = u"hey";
    sStruct.tss32Val = U"hello";
    sStruct.twssVal = L"servus";
    sStruct.tssFixVal = "hi";
    sStruct.tss8FixVal = "huhu";
    sStruct.tss16FixVal = u"hey";
    sStruct.tss32FixVal = U"hello";
    sStruct.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tssVal, "hij");
    EXPECT_EQ(sStruct.tss8Val, "huhuv");
    EXPECT_EQ(sStruct.tss16Val, u"heyz");
    EXPECT_EQ(sStruct.tss32Val, U"hellop");
    EXPECT_EQ(sStruct.twssVal, L"servust");
    EXPECT_EQ(sStruct.tssFixVal, "hij");
    EXPECT_EQ(sStruct.tss8FixVal, "huhuv");
    EXPECT_EQ(sStruct.tss16FixVal, u"heyz");
    EXPECT_EQ(sStruct.tss32FixVal, U"hellop");
    EXPECT_EQ(sStruct.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaStructPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    //sStruct.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8Val.size(); nIndex++)
    //    sStruct.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tptrssVal.resize(3);
    //sStruct.tptrssVal[0] = "hello";
    //sStruct.tptrssVal[1] = "huhu";
    //sStruct.tptrssVal[2] = "hi";
    //sStruct.tptr8FixVal.resize(sStruct.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8FixVal.size(); nIndex++)
    //    sStruct.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tptrssFixVal.resize(sStruct.tptrssFixVal.capacity());
    //sStruct.tptrssFixVal[0] = "hi";
    //sStruct.tptrssFixVal[1] = "hoi";
    //sStruct.tptrssFixVal[2] = "hello";
    //sStruct.tptrssFixVal[3] = "hallo";
    //sStruct.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tptr8Val[nIndex], sStruct.tptr8Val[nIndex + sStruct.tptr8Val.size() / 2]);
    //EXPECT_EQ(sStruct.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sStruct.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tptrssVal[nIndex], sStruct.tptrssVal[nIndex + sStruct.tptrssVal.size() / 2]);
    //EXPECT_EQ(sStruct.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sStruct.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sStruct.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //EXPECT_EQ(sStruct.tptrssFixVal.size(), 5);
    //EXPECT_EQ(sStruct.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(sStruct.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(sStruct.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(sStruct.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(sStruct.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaStructSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    //sStruct.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8Val.size(); nIndex++)
    //    sStruct.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tseqssVal.resize(3);
    //sStruct.tseqssVal[0] = "hello";
    //sStruct.tseqssVal[1] = "huhu";
    //sStruct.tseqssVal[2] = "hi";
    //sStruct.tseq8FixVal.resize(sStruct.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8FixVal.size(); nIndex++)
    //    sStruct.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //sStruct.tseqssFixVal.resize(sStruct.tseqssFixVal.capacity());
    //sStruct.tseqssFixVal[0] = "hi";
    //sStruct.tseqssFixVal[1] = "hoi";
    //sStruct.tseqssFixVal[2] = "hello";
    //sStruct.tseqssFixVal[3] = "hallo";
    //sStruct.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    //EXPECT_EQ(sStruct.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tseq8Val[nIndex], sStruct.tseq8Val[nIndex + sStruct.tseq8Val.size() / 2]);
    //EXPECT_EQ(sStruct.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sStruct.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sStruct.tseqssVal[nIndex], sStruct.tseqssVal[nIndex + sStruct.tseqssVal.size() / 2]);
    //EXPECT_EQ(sStruct.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sStruct.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sStruct.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //EXPECT_EQ(sStruct.tseqssFixVal.size(), 5);
    //EXPECT_EQ(sStruct.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(sStruct.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(sStruct.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(sStruct.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(sStruct.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};

    sStruct.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    sStruct.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    // Typedef of interfaces current not possible. BUG #399464
    //sStruct.ptMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tidVal, sdv::GetInterfaceId<IAddValue>());
    EXPECT_EQ(sStruct.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sStruct.tifcVal, nullptr);
    ASSERT_NE(sStruct.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sStruct.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //ASSERT_NE(sStruct.tpMultiplyValue, nullptr);
    //EXPECT_EQ(sStruct.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaStructSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaStruct sStruct{};
    sStruct.tsSubVal.i = 1;
    sStruct.tsIndVal.i = 10;
    // Incompatible serdes generation: BUG #398509
    //sStruct.tsUnnamedVal.i = 100;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaStruct(sStruct));
    EXPECT_EQ(sStruct.tsSubVal.i, 2);
    EXPECT_EQ(sStruct.tsIndVal.i, 20);
    // Incompatible serdes generation: BUG #398509
    //EXPECT_EQ(sStruct.tsUnnamedVal.i, 120);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(1);
    uUnion.bVal = false;
    EXPECT_FALSE(uUnion.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_TRUE(uUnion.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(2);
    uUnion.sVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.sVal, -11);
    uUnion.switch_to(3);
    uUnion.usVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.usVal, 11u);
    uUnion.switch_to(4);
    uUnion.lVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.lVal, -21);
    uUnion.switch_to(5);
    uUnion.ulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ulVal, 21u);
    uUnion.switch_to(6);
    uUnion.llVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.llVal, -31);
    uUnion.switch_to(7);
    uUnion.ullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ullVal, 31u);
    uUnion.switch_to(8);
    uUnion.i8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(static_cast<uint8_t>(uUnion.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    uUnion.switch_to(9);
    uUnion.ui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui8Val, 41u);
    uUnion.switch_to(10);
    uUnion.i16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i16Val, -51);
    uUnion.switch_to(11);
    uUnion.ui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui16Val, 51u);
    uUnion.switch_to(12);
    uUnion.i32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i32Val, -61);
    uUnion.switch_to(13);
    uUnion.ui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui32Val, 61u);
    uUnion.switch_to(14);
    uUnion.i64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.i64Val, -71);
    uUnion.switch_to(15);
    uUnion.ui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ui64Val, 71u);
    uUnion.switch_to(23);
    uUnion.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(16);
    uUnion.cVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.cVal, 'B');
    uUnion.switch_to(17);
    uUnion.c16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.c16Val, u'E');
    uUnion.switch_to(18);
    uUnion.c32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.c32Val, U'H');
    uUnion.switch_to(19);
    uUnion.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(20);
    float fVal = -1234.5678f;
    uUnion.fVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.fVal, fVal - 1000.000f);
    uUnion.switch_to(21);
    double dVal = 8765.4321;
    uUnion.dVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.dVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //uUnion.switch_to(22);
    //long double ldVal = -1234.4321;
    //uUnion.ldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(24);
    uUnion.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(26);
    uUnion.ssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ssVal, "hij");
    uUnion.switch_to(27);
    uUnion.ss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss8Val, "huhuv");
    uUnion.switch_to(28);
    uUnion.ss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss16Val, u"heyz");
    uUnion.switch_to(29);
    uUnion.ss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss32Val, U"hellop");
    uUnion.switch_to(30);
    uUnion.wssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wssVal, L"servust");
    uUnion.switch_to(31);
    uUnion.ssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ssFixVal, "hij");
    uUnion.switch_to(32);
    uUnion.ss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss8FixVal, "huhuv");
    uUnion.switch_to(33);
    uUnion.ss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss16FixVal, u"heyz");
    uUnion.switch_to(34);
    uUnion.ss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ss32FixVal, U"hellop");
    uUnion.switch_to(35);
    uUnion.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(36);
    uUnion.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8Val.size(); nIndex++)
        uUnion.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.ptr8Val[nIndex], uUnion.ptr8Val[nIndex + uUnion.ptr8Val.size() / 2]);
    uUnion.switch_to(37);
    uUnion.ptrssVal.resize(3);
    uUnion.ptrssVal[0] = "hello";
    uUnion.ptrssVal[1] = "huhu";
    uUnion.ptrssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < uUnion.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.ptrssVal[nIndex], uUnion.ptrssVal[nIndex + uUnion.ptrssVal.size() / 2]);
    uUnion.switch_to(38);
    uUnion.ptr8FixVal.resize(uUnion.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < uUnion.ptr8FixVal.size(); nIndex++)
        uUnion.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < uUnion.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(uUnion.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    uUnion.switch_to(39);
    uUnion.ptrssFixVal.resize(uUnion.ptrssFixVal.capacity());
    uUnion.ptrssFixVal[0] = "hi";
    uUnion.ptrssFixVal[1] = "hoi";
    uUnion.ptrssFixVal[2] = "hello";
    uUnion.ptrssFixVal[3] = "hallo";
    uUnion.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ptrssFixVal.size(), 5);
    EXPECT_EQ(uUnion.ptrssFixVal[0], "hihi");
    EXPECT_EQ(uUnion.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(uUnion.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(uUnion.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(uUnion.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(36);
    uUnion.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < uUnion.seq8Val.size(); nIndex++)
        uUnion.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < uUnion.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.seq8Val[nIndex], uUnion.seq8Val[nIndex + uUnion.seq8Val.size() / 2]);
    uUnion.switch_to(37);
    uUnion.seqssVal.resize(3);
    uUnion.seqssVal[0] = "hello";
    uUnion.seqssVal[1] = "huhu";
    uUnion.seqssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < uUnion.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(uUnion.seqssVal[nIndex], uUnion.seqssVal[nIndex + uUnion.seqssVal.size() / 2]);
    uUnion.switch_to(38);
    uUnion.seq8FixVal.resize(uUnion.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < uUnion.seq8FixVal.size(); nIndex++)
        uUnion.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < uUnion.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(uUnion.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    uUnion.switch_to(39);
    uUnion.seqssFixVal.resize(uUnion.seqssFixVal.capacity());
    uUnion.seqssFixVal[0] = "hi";
    uUnion.seqssFixVal[1] = "hoi";
    uUnion.seqssFixVal[2] = "hello";
    uUnion.seqssFixVal[3] = "hallo";
    uUnion.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.seqssFixVal.size(), 5);
    EXPECT_EQ(uUnion.seqssFixVal[0], "hihi");
    EXPECT_EQ(uUnion.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(uUnion.seqssFixVal[2], "hellohello");
    EXPECT_EQ(uUnion.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(uUnion.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(25);

    uUnion.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.idVal, sdv::GetInterfaceId<IAddValue>());
    uUnion.switch_to(44);
    uUnion.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(uUnion.ifcVal, nullptr);
    ASSERT_NE(uUnion.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(uUnion.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    uUnion.switch_to(45);
    uUnion.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    ASSERT_NE(uUnion.pMultiplyValue, nullptr);
    EXPECT_EQ(uUnion.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(46);
    uUnion.sSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.sSubVal.i, 2);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(201);
    uUnion.tbVal = false;
    EXPECT_FALSE(uUnion.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_TRUE(uUnion.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(202);
    uUnion.tsVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsVal, -11);
    uUnion.switch_to(203);
    uUnion.tusVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tusVal, 11u);
    uUnion.switch_to(204);
    uUnion.tlVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tlVal, -21);
    uUnion.switch_to(205);
    uUnion.tulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tulVal, 21u);
    uUnion.switch_to(206);
    uUnion.tllVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tllVal, -31);
    uUnion.switch_to(207);
    uUnion.tullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tullVal, 31u);
    uUnion.switch_to(208);
    uUnion.ti8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(static_cast<uint8_t>(uUnion.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    uUnion.switch_to(209);
    uUnion.tui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui8Val, 41u);
    uUnion.switch_to(210);
    uUnion.ti16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti16Val, -51);
    uUnion.switch_to(211);
    uUnion.tui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui16Val, 51u);
    uUnion.switch_to(212);
    uUnion.ti32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti32Val, -61);
    uUnion.switch_to(213);
    uUnion.tui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui32Val, 61u);
    uUnion.switch_to(214);
    uUnion.ti64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.ti64Val, -71);
    uUnion.switch_to(215);
    uUnion.tui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tui64Val, 71u);
    uUnion.switch_to(223);
    uUnion.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(216);
    uUnion.tcVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tcVal, 'B');
    uUnion.switch_to(217);
    uUnion.tc16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tc16Val, u'E');
    uUnion.switch_to(218);
    uUnion.tc32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tc32Val, U'H');
    uUnion.switch_to(219);
    uUnion.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(220);
    float fVal = -1234.5678f;
    uUnion.tfVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tfVal, fVal - 1000.000f);
    uUnion.switch_to(221);
    double dVal = 8765.4321;
    uUnion.tdVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tdVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //uUnion.switch_to(222);
    //long double ldVal = -1234.4321;
    //uUnion.tldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(224);
    uUnion.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(226);
    uUnion.tssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tssVal, "hij");
    uUnion.switch_to(227);
    uUnion.tss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss8Val, "huhuv");
    uUnion.switch_to(228);
    uUnion.tss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss16Val, u"heyz");
    uUnion.switch_to(229);
    uUnion.tss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss32Val, U"hellop");
    uUnion.switch_to(230);
    uUnion.twssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twssVal, L"servust");
    uUnion.switch_to(231);
    uUnion.tssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tssFixVal, "hij");
    uUnion.switch_to(232);
    uUnion.tss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss8FixVal, "huhuv");
    uUnion.switch_to(233);
    uUnion.tss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss16FixVal, u"heyz");
    uUnion.switch_to(234);
    uUnion.tss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tss32FixVal, U"hellop");
    uUnion.switch_to(235);
    uUnion.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaTypeBasedUnionPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(236);
    //uUnion.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8Val.size(); nIndex++)
    //    uUnion.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tptr8Val[nIndex], uUnion.tptr8Val[nIndex + uUnion.tptr8Val.size() / 2]);
    //uUnion.switch_to(237);
    //uUnion.tptrssVal.resize(3);
    //uUnion.tptrssVal[0] = "hello";
    //uUnion.tptrssVal[1] = "huhu";
    //uUnion.tptrssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < uUnion.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tptrssVal[nIndex], uUnion.tptrssVal[nIndex + uUnion.tptrssVal.size() / 2]);
    //uUnion.switch_to(238);
    //uUnion.tptr8FixVal.resize(uUnion.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8FixVal.size(); nIndex++)
    //    uUnion.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < uUnion.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(uUnion.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //uUnion.switch_to(239);
    //uUnion.tptrssFixVal.resize(uUnion.tptrssFixVal.capacity());
    //uUnion.tptrssFixVal[0] = "hi";
    //uUnion.tptrssFixVal[1] = "hoi";
    //uUnion.tptrssFixVal[2] = "hello";
    //uUnion.tptrssFixVal[3] = "hallo";
    //uUnion.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tptrssFixVal.size(), 5);
    //EXPECT_EQ(uUnion.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(uUnion.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(uUnion.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(uUnion.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(uUnion.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaTypeBasedUnionSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(236);
    //uUnion.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8Val.size(); nIndex++)
    //    uUnion.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tseq8Val[nIndex], uUnion.tseq8Val[nIndex + uUnion.tseq8Val.size() / 2]);
    //uUnion.switch_to(237);
    //uUnion.tseqssVal.resize(3);
    //uUnion.tseqssVal[0] = "hello";
    //uUnion.tseqssVal[1] = "huhu";
    //uUnion.tseqssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < uUnion.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(uUnion.tseqssVal[nIndex], uUnion.tseqssVal[nIndex + uUnion.tseqssVal.size() / 2]);
    //uUnion.switch_to(238);
    //uUnion.tseq8FixVal.resize(uUnion.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8FixVal.size(); nIndex++)
    //    uUnion.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < uUnion.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(uUnion.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //uUnion.switch_to(239);
    //uUnion.tseqssFixVal.resize(uUnion.tseqssFixVal.capacity());
    //uUnion.tseqssFixVal[0] = "hi";
    //uUnion.tseqssFixVal[1] = "hoi";
    //uUnion.tseqssFixVal[2] = "hello";
    //uUnion.tseqssFixVal[3] = "hallo";
    //uUnion.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //EXPECT_EQ(uUnion.tseqssFixVal.size(), 5);
    //EXPECT_EQ(uUnion.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(uUnion.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(uUnion.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(uUnion.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(uUnion.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(225);

    uUnion.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tidVal, sdv::GetInterfaceId<IAddValue>());
    uUnion.switch_to(244);
    uUnion.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(uUnion.tifcVal, nullptr);
    ASSERT_NE(uUnion.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(uUnion.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //uUnion.switch_to(245);
    //uUnion.tpMultiplyValue = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    //ASSERT_NE(uUnion.tpMultiplyValue, nullptr);
    //EXPECT_EQ(uUnion.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaTypeBasedUnionSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    UMegaTypeBasedUnion uUnion;
    uUnion.switch_to(246);
    uUnion.tsSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsSubVal.i, 2);
    uUnion.switch_to(247);
    uUnion.tsIndVal.i = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaTypeBasedUnion(uUnion));
    EXPECT_EQ(uUnion.tsIndVal.i, 20);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionBoolean)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(1);
    sUnion.uVal.bVal = false;
    EXPECT_FALSE(sUnion.uVal.bVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_TRUE(sUnion.uVal.bVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionIntegral)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(2);
    sUnion.uVal.sVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.sVal, -11);
    sUnion.switch_to(3);
    sUnion.uVal.usVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.usVal, 11u);
    sUnion.switch_to(4);
    sUnion.uVal.lVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.lVal, -21);
    sUnion.switch_to(5);
    sUnion.uVal.ulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ulVal, 21u);
    sUnion.switch_to(6);
    sUnion.uVal.llVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.llVal, -31);
    sUnion.switch_to(7);
    sUnion.uVal.ullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ullVal, 31u);
    sUnion.switch_to(8);
    sUnion.uVal.i8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(static_cast<uint8_t>(sUnion.uVal.i8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    sUnion.switch_to(9);
    sUnion.uVal.ui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui8Val, 41u);
    sUnion.switch_to(10);
    sUnion.uVal.i16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i16Val, -51);
    sUnion.switch_to(11);
    sUnion.uVal.ui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui16Val, 51u);
    sUnion.switch_to(12);
    sUnion.uVal.i32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i32Val, -61);
    sUnion.switch_to(13);
    sUnion.uVal.ui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui32Val, 61u);
    sUnion.switch_to(14);
    sUnion.uVal.i64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.i64Val, -71);
    sUnion.switch_to(15);
    sUnion.uVal.ui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ui64Val, 71u);
    sUnion.switch_to(23);
    sUnion.uVal.nVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.nVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionCharacter)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(16);
    sUnion.uVal.cVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.cVal, 'B');
    sUnion.switch_to(17);
    sUnion.uVal.c16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.c16Val, u'E');
    sUnion.switch_to(18);
    sUnion.uVal.c32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.c32Val, U'H');
    sUnion.switch_to(19);
    sUnion.uVal.wcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionFloatingPoint)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(20);
    float fVal = -1234.5678f;
    sUnion.uVal.fVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.fVal, fVal - 1000.000f);
    sUnion.switch_to(21);
    double dVal = 8765.4321;
    sUnion.uVal.dVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.dVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //sUnion.switch_to(22);
    //long double ldVal = -1234.4321;
    //sUnion.uVal.ldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.ldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionEnum)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(24);
    sUnion.uVal.eHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.eHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionString)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(26);
    sUnion.uVal.ssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ssVal, "hij");
    sUnion.switch_to(27);
    sUnion.uVal.ss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss8Val, "huhuv");
    sUnion.switch_to(28);
    sUnion.uVal.ss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss16Val, u"heyz");
    sUnion.switch_to(29);
    sUnion.uVal.ss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss32Val, U"hellop");
    sUnion.switch_to(30);
    sUnion.uVal.wssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wssVal, L"servust");
    sUnion.switch_to(31);
    sUnion.uVal.ssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ssFixVal, "hij");
    sUnion.switch_to(32);
    sUnion.uVal.ss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss8FixVal, "huhuv");
    sUnion.switch_to(33);
    sUnion.uVal.ss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss16FixVal, u"heyz");
    sUnion.switch_to(34);
    sUnion.uVal.ss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ss32FixVal, U"hellop");
    sUnion.switch_to(35);
    sUnion.uVal.wssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.wssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionPointer)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(36);
    sUnion.uVal.ptr8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8Val.size(); nIndex++)
        sUnion.uVal.ptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptr8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8Val.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.ptr8Val[nIndex], sUnion.uVal.ptr8Val[nIndex + sUnion.uVal.ptr8Val.size() / 2]);
    sUnion.switch_to(37);
    sUnion.uVal.ptrssVal.resize(3);
    sUnion.uVal.ptrssVal[0] = "hello";
    sUnion.uVal.ptrssVal[1] = "huhu";
    sUnion.uVal.ptrssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptrssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptrssVal.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.ptrssVal[nIndex], sUnion.uVal.ptrssVal[nIndex + sUnion.uVal.ptrssVal.size() / 2]);
    sUnion.switch_to(38);
    sUnion.uVal.ptr8FixVal.resize(sUnion.uVal.ptr8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8FixVal.size(); nIndex++)
        sUnion.uVal.ptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptr8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.ptr8FixVal.size(); nIndex++)
        EXPECT_EQ(sUnion.uVal.ptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    sUnion.switch_to(39);
    sUnion.uVal.ptrssFixVal.resize(sUnion.uVal.ptrssFixVal.capacity());
    sUnion.uVal.ptrssFixVal[0] = "hi";
    sUnion.uVal.ptrssFixVal[1] = "hoi";
    sUnion.uVal.ptrssFixVal[2] = "hello";
    sUnion.uVal.ptrssFixVal[3] = "hallo";
    sUnion.uVal.ptrssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ptrssFixVal.size(), 5);
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[0], "hihi");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[1], "hoihoi");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[2], "hellohello");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[3], "hallohallo");
    EXPECT_EQ(sUnion.uVal.ptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionSequence)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(36);
    sUnion.uVal.seq8Val.resize(100);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8Val.size(); nIndex++)
        sUnion.uVal.seq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seq8Val.size(), 200);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8Val.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.seq8Val[nIndex], sUnion.uVal.seq8Val[nIndex + sUnion.uVal.seq8Val.size() / 2]);
    sUnion.switch_to(37);
    sUnion.uVal.seqssVal.resize(3);
    sUnion.uVal.seqssVal[0] = "hello";
    sUnion.uVal.seqssVal[1] = "huhu";
    sUnion.uVal.seqssVal[2] = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seqssVal.size(), 6);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seqssVal.size() / 2; nIndex++)
        EXPECT_EQ(sUnion.uVal.seqssVal[nIndex], sUnion.uVal.seqssVal[nIndex + sUnion.uVal.seqssVal.size() / 2]);
    sUnion.switch_to(38);
    sUnion.uVal.seq8FixVal.resize(sUnion.uVal.seq8FixVal.capacity());
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8FixVal.size(); nIndex++)
        sUnion.uVal.seq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seq8FixVal.size(), 20);
    for (size_t nIndex = 0; nIndex < sUnion.uVal.seq8FixVal.size(); nIndex++)
        EXPECT_EQ(sUnion.uVal.seq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    sUnion.switch_to(39);
    sUnion.uVal.seqssFixVal.resize(sUnion.uVal.seqssFixVal.capacity());
    sUnion.uVal.seqssFixVal[0] = "hi";
    sUnion.uVal.seqssFixVal[1] = "hoi";
    sUnion.uVal.seqssFixVal[2] = "hello";
    sUnion.uVal.seqssFixVal[3] = "hallo";
    sUnion.uVal.seqssFixVal[4] = "servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.seqssFixVal.size(), 5);
    EXPECT_EQ(sUnion.uVal.seqssFixVal[0], "hihi");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[1], "hoihoi");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[2], "hellohello");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[3], "hallohallo");
    EXPECT_EQ(sUnion.uVal.seqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionInterface)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(25);

    sUnion.uVal.idVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.idVal, sdv::GetInterfaceId<IAddValue>());
    sUnion.switch_to(44);
    sUnion.uVal.ifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sUnion.uVal.ifcVal, nullptr);
    ASSERT_NE(sUnion.uVal.ifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sUnion.uVal.ifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    sUnion.switch_to(45);
    sUnion.uVal.pMultiplyValue = &sMultiply;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    ASSERT_NE(sUnion.uVal.pMultiplyValue, nullptr);
    EXPECT_EQ(sUnion.uVal.pMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionSubCompound)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(46);
    sUnion.uVal.sSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.sSubVal.i, 2);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionBooleanTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(201);
    sUnion.uVal.tbVal = false;
    EXPECT_FALSE(sUnion.uVal.tbVal);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_TRUE(sUnion.uVal.tbVal);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionIntegralTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(202);
    sUnion.uVal.tsVal = -10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsVal, -11);
    sUnion.switch_to(203);
    sUnion.uVal.tusVal = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tusVal, 11u);
    sUnion.switch_to(204);
    sUnion.uVal.tlVal = -20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tlVal, -21);
    sUnion.switch_to(205);
    sUnion.uVal.tulVal = 20;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tulVal, 21u);
    sUnion.switch_to(206);
    sUnion.uVal.tllVal = -30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tllVal, -31);
    sUnion.switch_to(207);
    sUnion.uVal.tullVal = 30;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tullVal, 31u);
    sUnion.switch_to(208);
    sUnion.uVal.ti8Val = -40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(static_cast<uint8_t>(sUnion.uVal.ti8Val), static_cast<uint8_t>(-41));    // Cast needed for ARM compatibility
    sUnion.switch_to(209);
    sUnion.uVal.tui8Val = 40;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui8Val, 41u);
    sUnion.switch_to(210);
    sUnion.uVal.ti16Val = -50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti16Val, -51);
    sUnion.switch_to(211);
    sUnion.uVal.tui16Val = 50;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui16Val, 51u);
    sUnion.switch_to(212);
    sUnion.uVal.ti32Val = -60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti32Val, -61);
    sUnion.switch_to(213);
    sUnion.uVal.tui32Val = 60;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui32Val, 61u);
    sUnion.switch_to(214);
    sUnion.uVal.ti64Val = -70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.ti64Val, -71);
    sUnion.switch_to(215);
    sUnion.uVal.tui64Val = 70;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tui64Val, 71u);
    sUnion.switch_to(223);
    sUnion.uVal.tnVal = 80;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tnVal, 81);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionCharacterTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(216);
    sUnion.uVal.tcVal = 'A';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tcVal, 'B');
    sUnion.switch_to(217);
    sUnion.uVal.tc16Val = u'D';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tc16Val, u'E');
    sUnion.switch_to(218);
    sUnion.uVal.tc32Val = U'G';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tc32Val, U'H');
    sUnion.switch_to(219);
    sUnion.uVal.twcVal = L'J';
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twcVal, L'K');

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionFloatingPointTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(220);
    float fVal = -1234.5678f;
    sUnion.uVal.tfVal = fVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tfVal, fVal - 1000.000f);
    sUnion.switch_to(221);
    double dVal = 8765.4321;
    sUnion.uVal.tdVal = dVal;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tdVal, dVal + 1000.000);
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //sUnion.switch_to(222);
    //long double ldVal = -1234.4321;
    //sUnion.uVal.tldVal = ldVal;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tldVal, ldVal - 1000.000);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionEnumTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(224);
    sUnion.uVal.teHelloVal = EHello::hello;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.teHelloVal, EHello::hallo);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionStringTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(226);
    sUnion.uVal.tssVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tssVal, "hij");
    sUnion.switch_to(227);
    sUnion.uVal.tss8Val = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss8Val, "huhuv");
    sUnion.switch_to(228);
    sUnion.uVal.tss16Val = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss16Val, u"heyz");
    sUnion.switch_to(229);
    sUnion.uVal.tss32Val = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss32Val, U"hellop");
    sUnion.switch_to(230);
    sUnion.uVal.twssVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twssVal, L"servust");
    sUnion.switch_to(231);
    sUnion.uVal.tssFixVal = "hi";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tssFixVal, "hij");
    sUnion.switch_to(232);
    sUnion.uVal.tss8FixVal = "huhu";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss8FixVal, "huhuv");
    sUnion.switch_to(233);
    sUnion.uVal.tss16FixVal = u"hey";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss16FixVal, u"heyz");
    sUnion.switch_to(234);
    sUnion.uVal.tss32FixVal = U"hello";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tss32FixVal, U"hellop");
    sUnion.switch_to(235);
    sUnion.uVal.twssFixVal = L"servus";
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.twssFixVal, L"servust");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaVarBasedUnionPointerTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(236);
    //sUnion.uVal.tptr8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8Val.size(); nIndex++)
    //    sUnion.uVal.tptr8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptr8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptr8Val[nIndex], sUnion.uVal.tptr8Val[nIndex + sUnion.uVal.tptr8Val.size() / 2]);
    //sUnion.switch_to(237);
    //sUnion.uVal.tptrssVal.resize(3);
    //sUnion.uVal.tptrssVal[0] = "hello";
    //sUnion.uVal.tptrssVal[1] = "huhu";
    //sUnion.uVal.tptrssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptrssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptrssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptrssVal[nIndex], sUnion.uVal.tptrssVal[nIndex + sUnion.uVal.tptrssVal.size() / 2]);
    //sUnion.switch_to(238);
    //sUnion.uVal.tptr8FixVal.resize(sUnion.uVal.tptr8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8FixVal.size(); nIndex++)
    //    sUnion.uVal.tptr8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptr8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tptr8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tptr8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //sUnion.switch_to(239);
    //sUnion.uVal.tptrssFixVal.resize(sUnion.uVal.tptrssFixVal.capacity());
    //sUnion.uVal.tptrssFixVal[0] = "hi";
    //sUnion.uVal.tptrssFixVal[1] = "hoi";
    //sUnion.uVal.tptrssFixVal[2] = "hello";
    //sUnion.uVal.tptrssFixVal[3] = "hallo";
    //sUnion.uVal.tptrssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal.size(), 5);
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[0], "hihi");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[1], "hoihoi");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[2], "hellohello");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[3], "hallohallo");
    //EXPECT_EQ(sUnion.uVal.tptrssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

// Using typedefs of template based types produced an error: BUG #398266
TEST(IPC_Communication_Test_UDS, DISABLED_MarshallMegaVarBasedUnionSequenceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(236);
    //sUnion.uVal.tseq8Val.resize(100);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8Val.size(); nIndex++)
    //    sUnion.uVal.tseq8Val[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseq8Val.size(), 200);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8Val.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseq8Val[nIndex], sUnion.uVal.tseq8Val[nIndex + sUnion.uVal.tseq8Val.size() / 2]);
    //sUnion.switch_to(237);
    //sUnion.uVal.tseqssVal.resize(3);
    //sUnion.uVal.tseqssVal[0] = "hello";
    //sUnion.uVal.tseqssVal[1] = "huhu";
    //sUnion.uVal.tseqssVal[2] = "hi";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseqssVal.size(), 6);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseqssVal.size() / 2; nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseqssVal[nIndex], sUnion.uVal.tseqssVal[nIndex + sUnion.uVal.tseqssVal.size() / 2]);
    //sUnion.switch_to(238);
    //sUnion.uVal.tseq8FixVal.resize(sUnion.uVal.tseq8FixVal.capacity());
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8FixVal.size(); nIndex++)
    //    sUnion.uVal.tseq8FixVal[nIndex] = static_cast<uint8_t>(nIndex);
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseq8FixVal.size(), 20);
    //for (size_t nIndex = 0; nIndex < sUnion.uVal.tseq8FixVal.size(); nIndex++)
    //    EXPECT_EQ(sUnion.uVal.tseq8FixVal[nIndex], static_cast<uint8_t>(nIndex << 1));
    //sUnion.switch_to(239);
    //sUnion.uVal.tseqssFixVal.resize(sUnion.uVal.tseqssFixVal.capacity());
    //sUnion.uVal.tseqssFixVal[0] = "hi";
    //sUnion.uVal.tseqssFixVal[1] = "hoi";
    //sUnion.uVal.tseqssFixVal[2] = "hello";
    //sUnion.uVal.tseqssFixVal[3] = "hallo";
    //sUnion.uVal.tseqssFixVal[4] = "servus";
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal.size(), 5);
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[0], "hihi");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[1], "hoihoi");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[2], "hellohello");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[3], "hallohallo");
    //EXPECT_EQ(sUnion.uVal.tseqssFixVal[4], "servusservus");

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionInterfaceTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    struct SMultiply : public IMultiplyValue
    {
        size_t Multiply(size_t n)
        {
            return n * n;
        }
    } sMultiply;

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(225);

    sUnion.uVal.tidVal = sdv::GetInterfaceId<IMultiplyValue>();
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tidVal, sdv::GetInterfaceId<IAddValue>());
    sUnion.switch_to(244);
    sUnion.uVal.tifcVal = static_cast<IMultiplyValue*>(&sMultiply);
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tifcVal.id(), sdv::GetInterfaceId<IAddValue>());
    ASSERT_NE(sUnion.uVal.tifcVal, nullptr);
    ASSERT_NE(sUnion.uVal.tifcVal.get<IAddValue>(), nullptr);
    EXPECT_EQ(sUnion.uVal.tifcVal.get<IAddValue>()->Add(99), 19602); // 99 * 99 + 99 * 99
    // Typedef of interfaces current not possible. BUG #399464
    //sUnion.switch_to(245);
    //sUnion.uVal.tpMultiplyValue = &sMultiply;
    //EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    //ASSERT_NE(sUnion.uVal.tpMultiplyValue, nullptr);
    //EXPECT_EQ(sUnion.uVal.tpMultiplyValue->Multiply(5), 625); // 5 * 5 * 5 * 5

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}

TEST(IPC_Communication_Test_UDS, MarshallMegaVarBasedUnionSubCompoundTypedef)
{
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"

[Console]
Report = "Silent"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());

    // Start communication control
    CCommunicationControl control;
    control.Initialize(sdv::SObjectInfo());
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::initialized);
    control.SetOperationMode(sdv::EOperationMode::configuring);
    EXPECT_EQ(control.GetObjectState(), sdv::EObjectState::configuring);

    // Load the shared memory components
    LoadIPCModules(control, ELocalIpcBackend::sockets);

    // Create the server connection
    CInterfaceTest test;
    sdv::u8string ssConnectionString;
    EXPECT_TRUE(control.CreateServerConnection(sdv::com::EChannelType::local_channel, &test, 100, ssConnectionString) != 0u);

    // Create the client connection
    sdv::IInterfaceAccess* pObjectProxy = nullptr;
    EXPECT_TRUE(control.CreateClientConnection(ssConnectionString, 1000, pObjectProxy) != 0u);
    ASSERT_NE(pObjectProxy, nullptr);

    // Get the target interface
    IMegaTest* pMegaTest = pObjectProxy->GetInterface<IMegaTest>();

    ASSERT_NE(pMegaTest, nullptr);
    SMegaVarBasedUnion sUnion;
    sUnion.switch_to(246);
    sUnion.uVal.tsSubVal.i = 1;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsSubVal.i, 2);
    sUnion.switch_to(247);
    sUnion.uVal.tsIndVal.i = 10;
    EXPECT_NO_THROW(pMegaTest->ProcessMegaVarBasedUnion(sUnion));
    EXPECT_EQ(sUnion.uVal.tsIndVal.i, 20);

    // Cleanup...
    control.Shutdown();
    appcontrol.Shutdown();
}
