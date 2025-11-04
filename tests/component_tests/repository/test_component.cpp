#include <gtest/gtest.h>
#include <mutex>
#include <iostream>
#include <functional>
#include <thread>
#include <support/component_impl.h>
#include <support/local_service_access.h>
#include "generated/test_component.h"

inline sdv::process::TProcessID GetProcessID()
{
    static sdv::process::TProcessID tProcessID = 0;
    if (!tProcessID)
    {
        sdv::process::IProcessInfo* pProcessInfo = sdv::core::GetObject<sdv::process::IProcessInfo>("ProcessControlService");
        if (!pProcessInfo) return 0;
        tProcessID = pProcessInfo->GetProcessID();
    }
    return tProcessID;
}

class CTestLockService : public sdv::CSdvObject, public ITestLock
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(ITestLock)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestLockService")

    void Lock() override
    {
        m_mutex.lock();
    }

    void Unlock() override
    {
        m_mutex.unlock();
    }

private:
    std::recursive_mutex m_mutex;
};

DEFINE_SDV_OBJECT(CTestLockService)

/**
 * @brief Example component testing IObjectControl
 */
    class CTestObjectControl
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    ~CTestObjectControl()
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::destruction_pending);
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object")

    /**
     * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectControl)

/**
* @brief Example component testing IObjectControl - 2nd component to test duplicate instantiation
*/
class CTestObjectControl2
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:
    ~CTestObjectControl2()
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::destruction_pending);
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object_2")

    /**
    * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
    * @param[in] ssObjectConfig Optional configuration string.
    */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
    * @brief Gets the current status of the object
    * @return EObjectStatus The current status of the object
    */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
    * @brief Shutdown method called before the object is destroyed.
    * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
    * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
    * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
    * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
    */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectControl2)

/**
 * @brief Example component testing IObjectControl
 */
    class CTestObjectControlFail
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_IObjectControlFail")

    /**
     * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {}

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectControlFail)

/**
 * @brief Example component testing chained object creation in Initialize
 */
    class CTestObjectCreate
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChain")

    /**
     * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize( const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        //create new example object with name ssObjectConfig

        auto pRepo = sdv::core::GetObject("RepositoryService");
        ASSERT_TRUE(pRepo);

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        ASSERT_TRUE(pCreate);

        ASSERT_FALSE(ssObjectConfig.empty());

        ASSERT_TRUE(pCreate->CreateObject("Example_Object", ssObjectConfig,nullptr));

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectCreate)

/**
 * @brief Example component testing chained object creation in Initialize while holding a lock
 */
    class CTestObjectCreateLock
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChainLock")

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        //create new example object with name ssObjectConfig while holding lock

        auto pLock = sdv::core::GetObject<ITestLock>("TestLockService");
        ASSERT_NE(pLock,nullptr);

        struct SAutoLock
        {
            SAutoLock(ITestLock* pLockParam) : m_pLockParam(pLockParam) { pLockParam->Lock(); }
            ~SAutoLock() { m_pLockParam->Unlock(); }
            ITestLock* m_pLockParam = nullptr;
        } sAutoLock(pLock);

        auto pRepo = sdv::core::GetObject("RepositoryService");
        ASSERT_TRUE(pRepo);

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        ASSERT_TRUE(pCreate);

        ASSERT_FALSE(ssObjectConfig.empty());

        ASSERT_TRUE(pCreate->CreateObject("Example_Object", ssObjectConfig, nullptr));

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectCreateLock)

/**
 * @brief Example component testing chained object creation in Initialize in extra thread holding a lock
 */
    class CTestObjectCreateLockThread
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChainLockThread")

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        auto fun = [ssObjectConfig]()
        {
            //create new example object with name ssObjectConfig while holding lock

            auto pLock = sdv::core::GetObject<ITestLock>("TestLockService");
            ASSERT_NE(pLock, nullptr);

            struct SAutoLock
            {
                SAutoLock(ITestLock* pLockParam) : m_pLockParam(pLockParam) { pLockParam->Lock(); }
                ~SAutoLock() { m_pLockParam->Unlock(); }
                ITestLock* m_pLockParam = nullptr;
            } sAutoLock(pLock);

            auto pRepo = sdv::core::GetObject("RepositoryService");
            ASSERT_TRUE(pRepo);

            auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
            ASSERT_TRUE(pCreate);

            ASSERT_FALSE(ssObjectConfig.empty());

            ASSERT_TRUE(pCreate->CreateObject("Example_Object", ssObjectConfig, nullptr));
        };

        std::thread testThread(fun);

        testThread.join();

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectCreateLockThread)


/**
 * @brief Example component testing IObjectControl
 */
    class CTestObjectCreateDuringShutdown
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateDuringShutdown")

    /**
     * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        //create new example object
        //this is allowed during shutdown as it's needed in case new proxies are created

        auto pRepo = sdv::core::GetObject("RepositoryService");
        ASSERT_TRUE(pRepo);

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        ASSERT_TRUE(pCreate);

        ASSERT_FALSE(pCreate->CreateObject("Example_Object", nullptr, nullptr));

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};
DEFINE_SDV_OBJECT(CTestObjectCreateDuringShutdown)

/**
 * @brief Hello device
 */
class CHelloDevice : public sdv::CSdvObject, public IHello
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_HelloDevice")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from device";
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CHelloDevice)

/**
 * @brief Basic hello service
 */
class CBasicHelloService : public sdv::CSdvObject, public IHello
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
    DECLARE_OBJECT_CLASS_NAME("TestObject_BasicHelloService")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from basic service";
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CBasicHelloService)

/**
 * @brief Complex hello service
 */
class CComplexHelloService : public sdv::CSdvObject, public IHello
{
public:
    CComplexHelloService()
    {
        std::cout << "CComplexHelloService constructor" << std::endl;
    }
    ~CComplexHelloService()
    {
        std::cout << "CComplexHelloService destructor" << std::endl;
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("TestObject_ComplexHelloService")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from complex service";
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CComplexHelloService)

/**
 * @brief Hello utlity
 */
class CHelloUtility : public sdv::CSdvObject, public IHello
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("TestObject_HelloUtility")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from utility";
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CHelloUtility)

/**
 * @brief Hello utlity
 */
class CSystemHelloService : public sdv::CSdvObject, public IHello
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("TestObject_SystemHelloService")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from system service";
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CSystemHelloService)
