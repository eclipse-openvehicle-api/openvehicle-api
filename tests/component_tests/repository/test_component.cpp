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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
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
class CTestObjectControl : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}
};

DEFINE_SDV_OBJECT(CTestObjectControl)

/**
* @brief Example component testing IObjectControl - 2nd component to test duplicate instantiation
*/
class CTestObjectControl2 : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object_2")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}
};

DEFINE_SDV_OBJECT(CTestObjectControl2)

/**
 * @brief Example component testing IObjectControl
 */
class CTestObjectControlFail : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_IObjectControlFail")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return false;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}
};

DEFINE_SDV_OBJECT(CTestObjectControlFail)

/**
 * @brief Example component testing chained object creation in Initialize
 */
class CTestObjectCreate : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChain")

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_ssChainedObject, "chained_object", "", "", "Name of the chained object")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        //create new example object with name ssObjectConfig
        auto pRepo = sdv::core::GetObject("RepositoryService");
        if (!pRepo) return false;

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        if (!pCreate) return false;

        if (m_ssChainedObject.empty())
            return false;

        if (!pCreate->CreateObject("Example_Object", m_ssChainedObject, nullptr))
            return false;

        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}

    std::string m_ssChainedObject;  ///< Name of the chained object
};

DEFINE_SDV_OBJECT(CTestObjectCreate)

/**
 * @brief Example component testing chained object creation in Initialize while holding a lock
 */
class CTestObjectCreateLock: public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChainLock")

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_ssChainedObject, "chained_object", "", "", "Name of the chained object")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        //create new example object with name ssObjectConfig while holding lock

        auto pLock = sdv::core::GetObject<ITestLock>("TestLockService");
        if (!pLock) return false;

        struct SAutoLock
        {
            SAutoLock(ITestLock* pLockParam) : m_pLockParam(pLockParam) { pLockParam->Lock(); }
            ~SAutoLock() { m_pLockParam->Unlock(); }
            ITestLock* m_pLockParam = nullptr;
        } sAutoLock(pLock);

        auto pRepo = sdv::core::GetObject("RepositoryService");
        if (!pRepo) return false;

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        if (!pCreate) return false;

        if (m_ssChainedObject.empty()) return false;

        if (!pCreate->CreateObject("Example_Object", m_ssChainedObject, nullptr)) return false;

        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}

    std::string m_ssChainedObject; ///< Name of the chained object
};

DEFINE_SDV_OBJECT(CTestObjectCreateLock)

/**
 * @brief Example component testing chained object creation in Initialize in extra thread holding a lock
 */
class CTestObjectCreateLockThread : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateChainLockThread")

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_ssChainedObject, "chained_object", "", "", "Name of the chained object")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        bool bResult = true;
        auto fun = [this, &bResult]()
        {
            //create new example object with name ssObjectConfig while holding lock

            auto pLock = sdv::core::GetObject<ITestLock>("TestLockService");
            if (!pLock)
            {
                bResult = false;
                return;
            }

            struct SAutoLock
            {
                SAutoLock(ITestLock* pLockParam) : m_pLockParam(pLockParam) { pLockParam->Lock(); }
                ~SAutoLock() { m_pLockParam->Unlock(); }
                ITestLock* m_pLockParam = nullptr;
            } sAutoLock(pLock);

            auto pRepo = sdv::core::GetObject("RepositoryService");
            if (!pRepo)
            {
                bResult = false;
                return;
            }

            auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
            if (!pCreate)
            {
                bResult = false;
                return;
            }

            if (m_ssChainedObject.empty())
            {
                bResult = false;
                return;
            }

            if (!pCreate->CreateObject("Example_Object", m_ssChainedObject, nullptr))
            {
                bResult = false;
                return;
            }
        };

        std::thread testThread(fun);

        testThread.join();

        return bResult;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}

    std::string m_ssChainedObject; ///< Name of the chained object
};

DEFINE_SDV_OBJECT(CTestObjectCreateLockThread)


/**
 * @brief Example component testing IObjectControl
 */
class CTestObjectCreateDuringShutdown : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_CreateDuringShutdown")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown method called before the object is destroyed. Overload of sdv::CSdvObject::OnShutdown.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetObjectState should return EObjectState::destruction_pending
     */
    virtual void OnShutdown() override
    {
        //create new example object
        //this is allowed during shutdown as it's needed in case new proxies are created

        auto pRepo = sdv::core::GetObject("RepositoryService");
        ASSERT_TRUE(pRepo);

        auto pCreate = sdv::TInterfaceAccessPtr(pRepo).GetInterface<sdv::core::IRepositoryControl>();
        ASSERT_TRUE(pCreate);

        ASSERT_FALSE(pCreate->CreateObject("Example_Object", nullptr, nullptr));
    }
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::basic_service)
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::complex_service)
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::utility)
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
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
