#include "../../include/gtest_custom.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/pssup.h>
#include "../../../sdv_services/ipc_connect/listener.h"
#include "../../../sdv_services/ipc_connect/client.h"

TEST(IPC_Connect_Test, InstantiateLocalDefaultListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialization_pending);
    listener.Initialize(R"code([Listener]
Type = "Local"
)code");
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialized);

    // Shutdown
    listener.Shutdown();
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test, InstantiateLocalDedicatedListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialization_pending);
    listener.Initialize(R"code([Listener]
Type = "Local"
Instance = 1234
)code");
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialized);

    // Shutdown
    listener.Shutdown();
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test, InstantiateLocalDefaultClientNoListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start client
    CClient client;
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialization_pending);
    client.Initialize("");
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::TObjectPtr ptrClient = client.Connect(R"code([Client]
Type = "local"
)code");
    EXPECT_FALSE(ptrClient);

    // Shutdown
    ptrClient.Clear();
    client.Shutdown();
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}

TEST(IPC_Connect_Test, InstantiateLocalSpecificClientNoListener)
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start client
    CClient client;
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialization_pending);
    client.Initialize("");
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::TObjectPtr ptrClient = client.Connect(R"code([Client]
Type = "local"
Instance = 1234
)code");
    EXPECT_FALSE(ptrClient);

    // Shutdown
    ptrClient.Clear();
    client.Shutdown();
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}

// Disabled the following test due to an unidentified crash/heap corruption occurring with MINGW on the build-server.
// Bug-report #610009 describes this issue: https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/610009
#ifdef _WIN32
TEST(IPC_Connect_Test, DISABLED_InstantiateLocalDefaultClientAndListener)
#else
TEST(IPC_Connect_Test, InstantiateLocalDefaultClientAndListener)
#endif
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialization_pending);
    listener.Initialize(R"code([Listener]
Type = "Local"
)code");
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialized);

    // Start client
    CClient client;
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialization_pending);
    client.Initialize("");
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::TObjectPtr ptrClient = client.Connect(R"code([Client]
Type = "Local"
)code");
    EXPECT_TRUE(ptrClient);

    // The client is a pointer to the repository. Request the module control service
    sdv::core::IObjectAccess* pObjectAccess = ptrClient.GetInterface<sdv::core::IObjectAccess>();
    EXPECT_NE(pObjectAccess, nullptr);
    sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
        pModuleInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService")).GetInterface<sdv::core::IModuleInfo>();
    EXPECT_NE(pModuleInfo, nullptr);
    if (pModuleInfo)
    {
        EXPECT_FALSE(pModuleInfo->GetModuleList().empty());
    }

    // Shutdown
    ptrClient.Clear();
    client.Shutdown();
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::destruction_pending);
    listener.Shutdown();
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}

// Disabled the following test due to an unidentified crash/heap corruption occurring with MINGW on the build-server.
// Bug-report #610009 describes this issue: https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/610009
#ifdef _WIN32
TEST(IPC_Connect_Test, DISABLED_InstantiateLocalSpecificClientAndListener)
#else
TEST(IPC_Connect_Test, InstantiateLocalSpecificClientAndListener)
#endif
{
    // Initialize system
    sdv::app::CAppControl appcontrol;
    ASSERT_TRUE(appcontrol.Startup(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config"));
    ASSERT_TRUE(appcontrol.IsRunning());
    sdv::core::IModuleControl* pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_NE(pModuleControl, nullptr);
    EXPECT_NE(pModuleControl->Load("ipc_com.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("ipc_shared_mem.sdv"), 0);
    EXPECT_NE(pModuleControl->Load("core_ps.sdv"), 0);
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_NE(pRepositoryControl, nullptr);
    EXPECT_NE(pRepositoryControl->CreateObject("CommunicationControl", {}, {}), 0);
    EXPECT_NE(pRepositoryControl->CreateObject("DefaultSharedMemoryChannelControl", {}, {}), 0);

    // Start listener
    CListener listener;
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialization_pending);
    listener.Initialize(R"code([Listener]
Type = "Local"
Instance = 1234
)code");
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::initialized);

    // Start client
    CClient client;
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialization_pending);
    client.Initialize("");
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::initialized);
    sdv::TObjectPtr ptrClient = client.Connect(R"code([Client]
Type = "Local"
Instance = 1234
)code");
    EXPECT_TRUE(ptrClient);

    // The client is a pointer to the repository. Request the module control service
    sdv::core::IObjectAccess* pObjectAccess = ptrClient.GetInterface<sdv::core::IObjectAccess>();
    EXPECT_NE(pObjectAccess, nullptr);
    sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
        pModuleInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService")).GetInterface<sdv::core::IModuleInfo>();
    EXPECT_NE(pModuleInfo, nullptr);
    if (pModuleInfo)
    {
        EXPECT_FALSE(pModuleInfo->GetModuleList().empty());
    }

    // Shutdown
    ptrClient.Clear();
    client.Shutdown();
    EXPECT_EQ(client.GetStatus(), sdv::EObjectStatus::destruction_pending);
    listener.Shutdown();
    EXPECT_EQ(listener.GetStatus(), sdv::EObjectStatus::destruction_pending);
    appcontrol.Shutdown();
}
