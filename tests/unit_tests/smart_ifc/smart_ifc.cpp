#include "../../include/gtest_custom.h"
#include "generated/smart_ifc.h"
#include <support/interface_ptr.h>

/**
 * @brief Helper class to test the smart pointer classes.
 */
class CSmartInterfaceHelper : public sdv::IInterfaceAccess, public IOther, public sdv::IObjectDestroy, public sdv::IObjectLifetime
{
public:
    /**
     * @brief Constructor
     * @param[in] bEnableObjectDestroy When set, the sdv::IObjectDestroy interface is exposed through the interface map.
     * @param[in] bEnableObjectRefCnt Object lifetime interface exposed through the interface map.
    */
    CSmartInterfaceHelper(bool bEnableObjectDestroy = false, bool bEnableObjectRefCnt = false) :
        m_bEnableObjectDestroy(bEnableObjectDestroy), m_bEnableObjectRefCnt(bEnableObjectRefCnt)
    {}

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(IOther)
        SDV_INTERFACE_SET_SECTION_CONDITION(m_bEnableObjectDestroy, 1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_SET_SECTION_CONDITION(m_bEnableObjectRefCnt, 2)
        SDV_INTERFACE_SECTION(2)
        SDV_INTERFACE_ENTRY(sdv::IObjectLifetime)
        SDV_INTERFACE_DEFAULT_SECTION()
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
     */
    virtual void DestroyObject() override
    {
        if (m_bDestroyCalled) m_bDestroyCalledTooMany = true;
        m_bDestroyCalled = true;
    }

    /**
    * @brief Increment the lifetime. Needs to be balanced by a call to Decrement. Overload of IObjectLifetime::Increment.
    */
    virtual void Increment() override
    {
        m_iCounter++;
    }

    /**
    * @brief Decrement the lifetime. If the lifetime reaches zero, the object will be destroyed (through the exposed
    * IObjectDestroy interface). Overload of IObjectLifetime::Decrement.
    * @return Returns 'true' if the object was destroyed, false if not.
    */
    virtual bool Decrement() override
    {
        m_iCounter--;
        if (!m_iCounter)
        {
            DestroyObject();
            return true;
        }
        if (m_iCounter < 0) m_bDecrementCalledTooMany = true;
        return false;
    }

    /**
    * Get the current lifetime count. Overload of IObjectLifetime::GetCount.
    */
    virtual uint32_t GetCount() const override
    {
        return static_cast<uint32_t>(m_iCounter);
    }

    /**
     * @brief Test function. Overload of IOther::Test.
     */
    virtual void Test() override
    {
        m_bTest = true;
    }

    /**
     * @brief Return whether destroy was called at least one time.
     * @return Returns the destroy-object-flag.
     */
    bool IsDestroyCalled() const { return m_bDestroyCalled; }

    /**
     * @brief Return when destroyed was called twice or more - which is an error.
     * @return Returns the destroy-object-too-many-times-flag.
     */
    bool IsDestroyCalledTooManyTimes() const { return m_bDestroyCalledTooMany; }

    /**
    * @brief Return when decrement was called too many times (more than increment) - which is an error.
    * @return Returns the decrement-object-too-many-times-flag.
    */
    bool IsDecrementCalledTooManyTimes() const { return m_bDecrementCalledTooMany; }

    /**
     * @brief Hast the test function been called?
     * @return The test function flag.
     */
    bool TestCalled() const { return m_bTest; }

private:
    bool    m_bEnableObjectDestroy = false;         ///< When enabled, object destroy is exposed.
    bool    m_bEnableObjectRefCnt = false;          ///< When ebabled, object reference counting is exposed.
    bool    m_bDestroyCalled = false;               ///< Flag set when DestroyObject is called.
    bool    m_bDestroyCalledTooMany = false;        ///< Flag set when DestroyObject is called more than once.
    bool    m_bDecrementCalledTooMany = false;      ///< Flag set when Decrement is called more than Increment.
    bool    m_bTest = false;                        ///< Flag set by the a call to the test function.
    int32_t m_iCounter = 0;                         ///< Object lifetime counter
};

TEST(Smart_Interface_Test, IInterfaceAccess_Self)
{
    CSmartInterfaceHelper helper;

    sdv::IInterfaceAccess* pAccess = &helper;
    ASSERT_NE(pAccess, nullptr);

    sdv::interface_t tInterface = pAccess->GetInterface(sdv::GetInterfaceId<sdv::IInterfaceAccess>());
    EXPECT_TRUE(tInterface);
    EXPECT_EQ(tInterface.get<sdv::IInterfaceAccess>(), pAccess);
}

TEST(Smart_Interface_Test, IInterfaceAccess_Other)
{
    CSmartInterfaceHelper helper;

    sdv::IInterfaceAccess* pAccess = &helper;
    ASSERT_NE(pAccess, nullptr);

    sdv::interface_t tInterface = pAccess->GetInterface(sdv::GetInterfaceId<IOther>());
    EXPECT_TRUE(tInterface);
    ASSERT_NE(tInterface.get<IOther>(), nullptr);
    EXPECT_EQ(tInterface.get<IOther>(), static_cast<IOther*>(&helper));
    EXPECT_EQ(tInterface.get<IOther>(), pAccess->GetInterface<IOther>());
    EXPECT_FALSE(helper.TestCalled());
    tInterface.get<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_Declaration)
{
    sdv::TInterfaceAccessPtr ptrAccess;
    EXPECT_FALSE(ptrAccess);
    EXPECT_FALSE(ptrAccess.IsValid());
    EXPECT_EQ(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    EXPECT_EQ(ptrAccess.GetInterface<sdv::IInterfaceAccess>(), nullptr);
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_AssignmentConstructor)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrAccess(&helper);
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_CopyConstructor)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrInitial(&helper);
    EXPECT_TRUE(ptrInitial);
    sdv::TInterfaceAccessPtr ptrAccess(ptrInitial);
    EXPECT_TRUE(ptrInitial);
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_MoveConstructor)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrInitial(&helper);
    EXPECT_TRUE(ptrInitial);
    sdv::TInterfaceAccessPtr ptrAccess(std::move(ptrInitial));
    EXPECT_FALSE(ptrInitial);
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_AssignmentOperator)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrAccess;
    EXPECT_FALSE(ptrAccess);
    ptrAccess = &helper;
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_CopyOperator)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrInitial(&helper);
    EXPECT_TRUE(ptrInitial);
    sdv::TInterfaceAccessPtr ptrAccess;
    EXPECT_FALSE(ptrAccess);
    ptrAccess = ptrInitial;
    EXPECT_TRUE(ptrInitial);
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, TInterfaceAccessPtr_MoveOperator)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrInitial(&helper);
    EXPECT_TRUE(ptrInitial);
    sdv::TInterfaceAccessPtr ptrAccess;
    EXPECT_FALSE(ptrAccess);
    ptrAccess = std::move(ptrInitial);
    EXPECT_FALSE(ptrInitial);
    EXPECT_TRUE(ptrAccess);
    EXPECT_TRUE(ptrAccess.IsValid());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrAccess), nullptr);
    ASSERT_NE(ptrAccess.GetInterface<IOther>(), nullptr);
    EXPECT_FALSE(helper.TestCalled());
    ptrAccess.GetInterface<IOther>()->Test();
    EXPECT_TRUE(helper.TestCalled());
}

TEST(Smart_Interface_Test, IObjectDestroy_NoInterface)
{
    CSmartInterfaceHelper helper;

    sdv::TInterfaceAccessPtr ptrAccess(&helper);
    EXPECT_TRUE(ptrAccess);

    // Get interface
    sdv::IObjectDestroy* pObjectDestroy = ptrAccess.GetInterface<sdv::IObjectDestroy>();
    EXPECT_EQ(pObjectDestroy, nullptr);
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, IObjectDestroy)
{
    CSmartInterfaceHelper helper(true);

    sdv::TInterfaceAccessPtr ptrAccess(&helper);
    EXPECT_TRUE(ptrAccess);

    // Get interface
    sdv::IObjectDestroy* pObjectDestroy = ptrAccess.GetInterface<sdv::IObjectDestroy>();
    ASSERT_NE(pObjectDestroy, nullptr);

    // Destroy the object
    pObjectDestroy->DestroyObject();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Destroy the object (simulates illegal call to pObjectDestroy, which normally would be destroyed with the previous call).
    pObjectDestroy->DestroyObject();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_TRUE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, IObjectLifetime)
{
    CSmartInterfaceHelper helper(true, true);

    sdv::TInterfaceAccessPtr ptrAccess(&helper);
    EXPECT_TRUE(ptrAccess);

    // Get interface
    sdv::IObjectLifetime* pObjectLifetime = ptrAccess.GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pObjectLifetime, nullptr);
    EXPECT_EQ(pObjectLifetime->GetCount(), 0u);
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());

    // Increment
    pObjectLifetime->Increment();
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());

    // Increment
    pObjectLifetime->Increment();
    EXPECT_EQ(pObjectLifetime->GetCount(), 2u);
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());

    // Decrement
    pObjectLifetime->Decrement();
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());

    // Decrement - this will destroy the object
    pObjectLifetime->Decrement();
    EXPECT_EQ(pObjectLifetime->GetCount(), 0u);
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());

    // Decrement - one too many
    pObjectLifetime->Decrement();
    EXPECT_EQ(pObjectLifetime->GetCount(), static_cast<uint32_t>(-1));
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_TRUE(helper.IsDecrementCalledTooManyTimes());
}

TEST(Smart_Interface_Test, ObjectLifetimeWrapper_CreateEmpty)
{
    sdv::IInterfaceAccess* pObject = sdv::CObjectLifetimeWrapper::CreateWrapper(nullptr);
    EXPECT_EQ(pObject, nullptr);
}

TEST(Smart_Interface_Test, ObjectLifetimeWrapper_Create)
{
    CSmartInterfaceHelper helper(true);
    sdv::IInterfaceAccess* pObject = sdv::CObjectLifetimeWrapper::CreateWrapper(&helper);
    ASSERT_NE(pObject, nullptr);

    // Helper class should not expose object lifetime
    sdv::IObjectLifetime* pObjectLifetime = sdv::CInterfacePtr(&helper).GetInterface<sdv::IObjectLifetime>();
    EXPECT_EQ(pObjectLifetime, nullptr);

    // Wrapper should expose object lifetime interface
    pObjectLifetime = sdv::CInterfacePtr(pObject).GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pObjectLifetime, nullptr);

    // Helper class exposes IObjectDestroy
    sdv::IObjectDestroy* pObjectDestroy = sdv::CInterfacePtr(&helper).GetInterface<sdv::IObjectDestroy>();
    ASSERT_NE(pObjectDestroy, nullptr);

    // Wrapper should also expose object destroy
    pObjectDestroy = sdv::CInterfacePtr(pObject).GetInterface<sdv::IObjectDestroy>();
    EXPECT_NE(pObjectDestroy, nullptr);

    // Status quo
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);

    // Increment
    pObjectLifetime->Increment();
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 2u);

    // Decrement
    pObjectLifetime->Decrement();
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);

    // Decrement
    pObjectLifetime->Decrement();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
}

TEST(Smart_Interface_Test, ObjectLifetimeWrapper_DestroyObject)
{
    CSmartInterfaceHelper helper(true);
    sdv::IInterfaceAccess* pObject = sdv::CObjectLifetimeWrapper::CreateWrapper(&helper);
    ASSERT_NE(pObject, nullptr);

    // Helper class should not expose object lifetime
    sdv::IObjectLifetime* pObjectLifetime = sdv::CInterfacePtr(&helper).GetInterface<sdv::IObjectLifetime>();
    EXPECT_EQ(pObjectLifetime, nullptr);

    // Wrapper should expose object lifetime interface
    pObjectLifetime = sdv::CInterfacePtr(pObject).GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pObjectLifetime, nullptr);

    // Helper class exposes IObjectDestroy
    sdv::IObjectDestroy* pObjectDestroy = sdv::CInterfacePtr(&helper).GetInterface<sdv::IObjectDestroy>();
    ASSERT_NE(pObjectDestroy, nullptr);

    // Wrapper should also expose object destroy
    pObjectDestroy = sdv::CInterfacePtr(pObject).GetInterface<sdv::IObjectDestroy>();
    EXPECT_NE(pObjectDestroy, nullptr);

    // Status quo
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);

    // Increment
    pObjectLifetime->Increment();
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 2u);

    // Call object destroy on the wrapper.
    pObjectDestroy->DestroyObject();
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
    EXPECT_EQ(pObjectLifetime->GetCount(), 1u);

    // Call object destroy on the wrapper.
    pObjectDestroy->DestroyObject();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDecrementCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_Declaration)
{
    sdv::TObjectPtr ptrObject;
    EXPECT_FALSE(ptrObject);
    EXPECT_FALSE(ptrObject.IsValid());
    EXPECT_EQ(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
    EXPECT_EQ(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
}

TEST(Smart_Interface_Test, TObjectPtr_AssignmentConstructor)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrObject(&helper);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_CopyConstructor)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrInitial(&helper);
        EXPECT_TRUE(ptrInitial);
        sdv::TObjectPtr ptrObject(ptrInitial);
        EXPECT_TRUE(ptrInitial);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_MoveConstructor)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrInitial(&helper);
        EXPECT_TRUE(ptrInitial);
        sdv::TObjectPtr ptrObject(std::move(ptrInitial));
        EXPECT_FALSE(ptrInitial);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_AssignmentOperator)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrObject;
        EXPECT_FALSE(ptrObject);
        ptrObject = &helper;
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_CopyOperator)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrInitial(&helper);
        EXPECT_TRUE(ptrInitial);
        sdv::TObjectPtr ptrObject;
        EXPECT_FALSE(ptrObject);
        ptrObject = ptrInitial;
        EXPECT_TRUE(ptrInitial);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_MoveOperator)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrInitial(&helper);
        EXPECT_TRUE(ptrInitial);
        sdv::TObjectPtr ptrObject;
        EXPECT_FALSE(ptrObject);
        ptrObject = std::move(ptrInitial);
        EXPECT_FALSE(ptrInitial);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_Clear)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    {
        sdv::TObjectPtr ptrObject(&helper);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);

        EXPECT_FALSE(helper.IsDestroyCalled());
        EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
        ptrObject.Clear();
        EXPECT_TRUE(helper.IsDestroyCalled());
        EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
    }

    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_Detach)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Use scope
    sdv::TInterfaceAccessPtr ptrAccess;
    {
        sdv::TObjectPtr ptrObject(&helper);
        EXPECT_TRUE(ptrObject);
        EXPECT_TRUE(ptrObject.IsValid());
        EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(ptrObject), nullptr);
        EXPECT_NE(ptrObject.GetInterface<sdv::IObjectDestroy>(), nullptr);

        EXPECT_FALSE(helper.IsDestroyCalled());
        EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
        ptrAccess = ptrObject.Detach();
        EXPECT_TRUE(ptrAccess);
        EXPECT_FALSE(helper.IsDestroyCalled());
        EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
    }

    // Object is not destroyed yet
    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());

    // Explicitly destroy the object
    sdv::IObjectDestroy* pDestroy = ptrAccess.GetInterface<sdv::IObjectDestroy>();
    ASSERT_NE(pDestroy, nullptr);
    pDestroy->DestroyObject();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
}

TEST(Smart_Interface_Test, TObjectPtr_RefCnt)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
    EXPECT_EQ(helper.GetCount(), 0u);

    sdv::TObjectPtr ptrObject(&helper);
    EXPECT_EQ(helper.GetCount(), 0u); // Not exposed by helper and therefore not used

    sdv::IObjectLifetime* pLifetime = ptrObject.GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pLifetime, nullptr);
    EXPECT_EQ(pLifetime->GetCount(), 1u);

    ptrObject.Clear();
    EXPECT_EQ(helper.GetCount(), 0u);
    EXPECT_TRUE(helper.IsDestroyCalled());
}

TEST(Smart_Interface_Test, TObjectPtr_RefCntTransfer)
{
    CSmartInterfaceHelper helper(true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
    EXPECT_EQ(helper.GetCount(), 0u);

    sdv::TObjectPtr ptrObject(&helper);
    EXPECT_EQ(helper.GetCount(), 0u); // Not exposed by helper and therefore not used

    sdv::IObjectLifetime* pLifetime = ptrObject.GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pLifetime, nullptr);
    EXPECT_EQ(pLifetime->GetCount(), 1u);

    sdv::TObjectPtr ptrObject2(static_cast<sdv::IInterfaceAccess*>(ptrObject));
    EXPECT_EQ(helper.GetCount(), 0u); // Not exposed by helper and therefore not used
    ASSERT_NE(pLifetime, nullptr);
    EXPECT_EQ(pLifetime->GetCount(), 2u);

    ptrObject.Clear();
    EXPECT_EQ(pLifetime->GetCount(), 1u);
    EXPECT_EQ(helper.GetCount(), 0u);

    ptrObject2.Clear();
    EXPECT_TRUE(helper.IsDestroyCalled());
}

TEST(Smart_Interface_Test, TObjectPtr_RefCntObject)
{
    CSmartInterfaceHelper helper(true, true);

    EXPECT_FALSE(helper.IsDestroyCalled());
    EXPECT_FALSE(helper.IsDestroyCalledTooManyTimes());
    EXPECT_EQ(helper.GetCount(), 0u);

    sdv::TObjectPtr ptrObject(&helper);
    EXPECT_EQ(helper.GetCount(), 1u);

    sdv::IObjectLifetime* pLifetime = ptrObject.GetInterface<sdv::IObjectLifetime>();
    ASSERT_NE(pLifetime, nullptr);
    EXPECT_EQ(pLifetime->GetCount(), 1u);

    sdv::TObjectPtr ptrObject2(static_cast<sdv::IInterfaceAccess*>(ptrObject));
    EXPECT_EQ(helper.GetCount(), 2u);
    ASSERT_NE(pLifetime, nullptr);
    EXPECT_EQ(pLifetime->GetCount(), 2u);

    ptrObject.Clear();
    EXPECT_EQ(pLifetime->GetCount(), 1u);
    EXPECT_EQ(helper.GetCount(), 1u);

    ptrObject2.Clear();
    EXPECT_TRUE(helper.IsDestroyCalled());
    EXPECT_EQ(helper.GetCount(), 0u);
}