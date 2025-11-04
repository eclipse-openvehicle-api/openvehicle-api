#include <gtest/gtest.h>
#include <interfaces/repository.h>
#include <support/local_service_access.h>
#include <support/app_control.h>
#include "../../../global/process_watchdog.h"
#include "../../include/basic_test_helper.h"
#include "generated/test_component.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

const uint32_t LoopCount = 100; //< amount of loops used for concurrency tests. more loops means more thorough deadlock check at the cost of increased runtime

// used to supply external objects for testing pruposes. Implements IObjectControl to make sure this is NOT called for externally supplied objects
class TestExternalObject : public sdv::IInterfaceAccess, public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        FAIL() << "Error: Initialize should not be called by Repo Service!";
//        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
    }

    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    virtual void Shutdown()
    {
        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

TEST(RepositoryTest, LoadNonexistentModule)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    EXPECT_FALSE(pModuleControl->Load((GetExecDirectory() / "TestFooBar.sdv").generic_u8string()));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, CreateNonexistantClass)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    EXPECT_FALSE(pRepositoryControl->CreateObject("TestFooBar", nullptr, nullptr));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, GetNonexistantObject)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("Example_Object", nullptr, nullptr);
    EXPECT_TRUE(bRes);
    EXPECT_EQ(nullptr, pObjectAccess->GetObject("TestFooBar"));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateAndGet)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("Example_Object", nullptr, nullptr);
    EXPECT_TRUE(bRes);
    EXPECT_NE(nullptr, pObjectAccess->GetObject("Example_Object"));

    control.SetRunningMode();
    control.Shutdown();

    EXPECT_EQ(nullptr, pObjectAccess->GetObject("Example_Object"));
}

TEST(RepositoryTest, InstantiateInitFail)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("TestObject_IObjectControlFail", nullptr, nullptr);
    EXPECT_FALSE(bRes);
    EXPECT_EQ(nullptr, pObjectAccess->GetObject("TestObject_IObjectControlFail"));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateDuplicateCreateObjectName)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    // Instantiating the same object twice is allowed (only one instance will be created).
    EXPECT_TRUE(pRepositoryControl->CreateObject("Example_Object", nullptr, nullptr));
    EXPECT_TRUE(pRepositoryControl->CreateObject("Example_Object", nullptr, nullptr));
    EXPECT_NE(nullptr, pObjectAccess->GetObject("Example_Object"));

    // Instantiating a different object with the same name as the previous object is not allowed.
    EXPECT_FALSE(pRepositoryControl->CreateObject("Example_Object_2", "Example_Object", nullptr));

    // Instantiating a different object with a completely different name is allowed
    EXPECT_TRUE(pRepositoryControl->CreateObject("Example_Object_2", "abracadabra", nullptr));
    EXPECT_NE(nullptr, pObjectAccess->GetObject("abracadabra"));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateDuplicateExternalObjectName)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    sdv::core::IRegisterForeignObject* pExternalObject = sdv::core::GetObject<sdv::core::IRegisterForeignObject>("RepositoryService");
    ASSERT_TRUE(pExternalObject);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    //no two objects of same name allowed! Second Register fails, Get returns instance of first Register
    TestExternalObject testObj1;
    EXPECT_TRUE(pExternalObject->RegisterObject(&testObj1, "Example_Object"));
    TestExternalObject testObj2;
    EXPECT_FALSE(pExternalObject->RegisterObject(&testObj2, "Example_Object"));
    EXPECT_EQ(&testObj1, pObjectAccess->GetObject("Example_Object"));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateDuplicateObjectNameMixed)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    sdv::core::IRegisterForeignObject* pExternalObject = sdv::core::GetObject<sdv::core::IRegisterForeignObject>("RepositoryService");
    ASSERT_TRUE(pExternalObject);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    //no two objects of same name allowed! Register fails, Get returns instance previously created
    bool bRes = pRepositoryControl->CreateObject("Example_Object", nullptr, nullptr);
    EXPECT_TRUE(bRes);

    TestExternalObject testObj2;
    EXPECT_FALSE(pExternalObject->RegisterObject(&testObj2, "Example_Object"));

    EXPECT_NE(nullptr, pObjectAccess->GetObject("Example_Object"));

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateTwoNamedInstances)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes1 = pRepositoryControl->CreateObject("Example_Object", "Obj_1", nullptr);
    EXPECT_TRUE(bRes1);
    EXPECT_NE(nullptr, pObjectAccess->GetObject("Obj_1"));

    bool bRes2 = pRepositoryControl->CreateObject("Example_Object", "Obj_2", nullptr);
    EXPECT_TRUE(bRes2);
    EXPECT_NE(nullptr, pObjectAccess->GetObject("Obj_2"));

    EXPECT_EQ(nullptr, pObjectAccess->GetObject("Example_Object"));

    control.SetRunningMode();
    control.Shutdown();

    EXPECT_EQ(nullptr, pObjectAccess->GetObject("Obj_1"));
    EXPECT_EQ(nullptr, pObjectAccess->GetObject("Obj_2"));
}

TEST(RepositoryTest, ChainCreateSimple)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("TestObject_CreateChain", nullptr, "ChainedObject");
    EXPECT_TRUE(bRes);
    EXPECT_NE(nullptr, pObjectAccess->GetObject("TestObject_CreateChain"));
    EXPECT_NE(nullptr, pObjectAccess->GetObject("ChainedObject"));

    control.SetRunningMode();
    control.Shutdown();

    EXPECT_EQ(nullptr, pObjectAccess->GetObject("TestObject_CreateChain"));
    EXPECT_EQ(nullptr, pObjectAccess->GetObject("ChainedObject"));
}

TEST(RepositoryTest, ChainCreateParallel)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    std::thread testThread([pRepositoryControl, pObjectAccess]()
        {
            for (uint32_t i = 0; i < LoopCount; ++i)
            {
                std::string count = std::to_string(i);
                bool bRes = pRepositoryControl->CreateObject("TestObject_CreateChain", "Bar_" + count, "BarFoo_" + count);
                EXPECT_TRUE(bRes);
                EXPECT_NE(nullptr, pObjectAccess->GetObject("BarFoo_" + count));
            }
        });

    for (uint32_t i = 0; i < LoopCount; ++i)
    {
        std::string count = std::to_string(i);
        bool bRes = pRepositoryControl->CreateObject("TestObject_CreateChain", "Foo_" + count, "FooBar_" + count);
        EXPECT_TRUE(bRes);
        EXPECT_NE(nullptr, pObjectAccess->GetObject("FooBar_" + count));
    }

    testThread.join();


    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, ChainCreateParallelLock)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("TestLockService", nullptr, nullptr);
    ASSERT_TRUE(bRes);
    auto pLock = sdv::core::GetObject("TestLockService").GetInterface<ITestLock>();

    ASSERT_NE(pLock, nullptr);

    //attempt to create deadlock by taking lock during initialize and by taking lock before call to create object

    std::thread testThread([&]()
        {
            for (uint32_t i = 0; i < LoopCount; ++i)
            {
                std::string count = std::to_string(i);
                //locks using TestLockService during construction
                bRes = pRepositoryControl->CreateObject("TestObject_CreateChainLock", "Bar_" + count, "BarFoo_" + count);
                EXPECT_TRUE(bRes);
                EXPECT_NE(nullptr, pObjectAccess->GetObject("BarFoo_" + count));
            }
        });

    for (uint32_t i = 0; i < LoopCount; ++i)
    {
        std::string count = std::to_string(i);
        pLock->Lock();
        bRes = pRepositoryControl->CreateObject("TestObject_CreateChain", "Foo_" + count, "FooBar_" + count);
        EXPECT_TRUE(bRes);
        EXPECT_NE(nullptr, pObjectAccess->GetObject("FooBar_" + count));
        pLock->Unlock();
    }

    testThread.join();


    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, ChainCreateParallelThreadLock)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    bool bRes = pRepositoryControl->CreateObject("TestLockService", nullptr, nullptr);
    ASSERT_TRUE(bRes);
    auto pLock = sdv::core::GetObject("TestLockService").GetInterface<ITestLock>();

    ASSERT_NE(pLock, nullptr);

    //attempt to create deadlock by taking lock during initialize and by taking lock before call to create object

    std::thread testThread([&]()
        {
            for (uint32_t i = 0; i < LoopCount; ++i)
            {
                std::string count = std::to_string(i);
                //locks using TestLockService during construction in a seperate thread
                bRes = pRepositoryControl->CreateObject("TestObject_CreateChainLockThread", "Bar_" + count, "BarFoo_" + count);
                EXPECT_TRUE(bRes);
                EXPECT_NE(nullptr, pObjectAccess->GetObject("BarFoo_" + count));
            }
        });

    for (uint32_t i = 0; i < LoopCount; ++i)
    {
        std::string count = std::to_string(i);
        pLock->Lock();
        bRes = pRepositoryControl->CreateObject("TestObject_CreateChain", "Foo_" + count, "FooBar_" + count);
        EXPECT_TRUE(bRes);
        EXPECT_NE(nullptr, pObjectAccess->GetObject("FooBar_" + count));
        pLock->Unlock();
    }

    testThread.join();


    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, InstantiateDuringShutdown)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto pModuleControl = sdv::core::GetObject<sdv::core::IModuleControl>("ModuleControlService");
    ASSERT_TRUE(pModuleControl);
    auto pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    ASSERT_TRUE(pRepositoryControl);
    auto pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    ASSERT_TRUE(pObjectAccess);

    ASSERT_TRUE(pModuleControl->Load((GetExecDirectory() / "ComponentTest_Repository_test_module.sdv").generic_u8string()));

    //object creates new object during shutdown
    //this is needed in case new proxies are requested during shutdown

    bool bRes = pRepositoryControl->CreateObject("TestObject_CreateDuringShutdown", nullptr, nullptr);
    EXPECT_TRUE(bRes);

    control.SetRunningMode();
    control.Shutdown();
}

TEST(RepositoryTest, CreateUtility)
{
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Essential"
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    auto ptrUtility = sdv::core::CreateUtility("TOMLParserUtility");
    ASSERT_TRUE(ptrUtility);
    ptrUtility.Clear();
    ASSERT_FALSE(ptrUtility);

    control.SetRunningMode();
    control.Shutdown();
}

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST(RepositoryTest, DISABLED_MainApplication_GetInstalledAndLoadedComponent)
{
    // Start the app control. The application automatically loads the installation manifests and the application configuration,
    // starting automatically the objects.
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Main"
Instance = 2005
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    // The example object is part of the installation and should have automatically instantiated through the configuration.
    auto pInterfaceAccess = sdv::core::GetObject<sdv::IInterfaceAccess>("Example_Object_2");
    EXPECT_NE(pInterfaceAccess, nullptr);

    control.SetRunningMode();
    control.Shutdown();
}

// TODO EVE: Disabled until the implementation of configuration installation is finished. See
// https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/705891
TEST(RepositoryTest, DISABLED_MainApplication_GetInstalledComponent)
{
    // Start the app control. The application automatically loads the installation manifests, but since there is no application
    // configuration, it doesn't load the objects.
    sdv::app::CAppControl control(R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Main"
Instance = 2005
AppConfig = "" # override application config
)code");
    ASSERT_TRUE(control.IsRunning());
    control.SetConfigMode();

    // The example object is part of the installation and should have be automatically instantiated.
    auto pInterfaceAccess = sdv::core::GetObject<sdv::IInterfaceAccess>("Example_Object_2");
    EXPECT_NE(pInterfaceAccess, nullptr);

    control.SetRunningMode();
    control.Shutdown();
}

