#include <sstream>

#include <support/pointer.h>

#include "basic_types_test.h"

using CPointerSimpleTypeTest = CBasicTypesTest;

TEST_F(CPointerSimpleTypeTest, MakeFixedPointer)
{
    sdv::pointer<uint32_t, 1> ptr0;
    EXPECT_TRUE(ptr0);
}

TEST_F(CPointerSimpleTypeTest, MakeDynamicPointer)
{
    sdv::pointer<uint32_t> ptr0 = sdv::make_ptr<uint32_t>(0);
    EXPECT_TRUE(ptr0);
    sdv::pointer<uint32_t> ptr1 = sdv::make_ptr<uint32_t>();
    EXPECT_TRUE(ptr1);
    sdv::pointer<uint32_t> ptr2 = sdv::make_ptr<uint32_t>(2);
    EXPECT_TRUE(ptr2);
}

TEST_F(CPointerSimpleTypeTest, ConstructorDynamicPointer)
{
    // Default constructor
    sdv::pointer<uint32_t> ptr;
    EXPECT_FALSE(ptr);

    // Allocate
    ptr = sdv::make_ptr<uint32_t>();
    EXPECT_TRUE(ptr);

    // Copy constructor
    sdv::pointer<uint32_t> ptrCopy(ptr);
    EXPECT_TRUE(ptrCopy);

    // Move constructor
    sdv::pointer<uint32_t> ptrMove(std::move(ptr));
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrMove);

    // Destructor
    GetMemMgr().ResetPtrSet();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    sdv::pointer<uint32_t>* pptr = new sdv::pointer<uint32_t>(sdv::make_ptr<uint32_t>());
    EXPECT_NE(pptr, nullptr);
    EXPECT_NE(GetMemMgr().GetPtrCount(), 0);
    delete pptr;
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
}

TEST_F(CPointerSimpleTypeTest, ConstructorFixedPointer)
{
    // Default constructor
    sdv::pointer<uint32_t, 1> ptr;
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0);

    // Copy constructor
    sdv::pointer<uint32_t, 1> ptrCopy1(ptr);
    EXPECT_TRUE(ptrCopy1);
    EXPECT_EQ(ptrCopy1.size(), 0);
    sdv::pointer<uint32_t, 10> ptrCopy10(ptr);
    EXPECT_TRUE(ptrCopy10);
    EXPECT_EQ(ptrCopy10.size(), 0);
    using TCopy2 = sdv::pointer<uint32_t, 2>;
    EXPECT_NO_THROW(TCopy2 ptrCopy2(ptrCopy10));
    ptrCopy10.resize(5);
    EXPECT_EQ(ptrCopy10.size(), 5);
    using TCopy3 = sdv::pointer<uint32_t, 3>;
    EXPECT_THROW(TCopy3 ptrCopy3(ptrCopy10), sdv::XBufferTooSmall);

    // Move constructor
    ptr.resize(1);
    EXPECT_EQ(ptr.size(), 1);
    sdv::pointer<uint32_t, 1> ptrMove(std::move(ptr));
    EXPECT_TRUE(ptr);
    EXPECT_TRUE(ptrMove);
    EXPECT_EQ(ptr.size(), 0);
    EXPECT_EQ(ptrMove.size(), 1);
}

TEST_F(CPointerSimpleTypeTest, ConstructorMixedPointer)
{
    // Default constructor
    sdv::pointer<uint32_t> ptr0;
    sdv::pointer<uint32_t, 1> ptr1;
    sdv::pointer<uint32_t, 5> ptr5;
    EXPECT_FALSE(ptr0);
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr5);

    // Copy constructor
    sdv::pointer<uint32_t> ptrCopy0a(ptr0);
    EXPECT_FALSE(ptrCopy0a);
    EXPECT_EQ(ptrCopy0a.size(), 0);
    sdv::pointer<uint32_t> ptrCopy0b(ptr1);
    EXPECT_TRUE(ptrCopy0b);
    EXPECT_EQ(ptrCopy0b.size(), 0);
    sdv::pointer<uint32_t> ptrCopy0c(ptr5);
    EXPECT_TRUE(ptrCopy0c);
    EXPECT_EQ(ptrCopy0c.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1a(ptr0);
    EXPECT_TRUE(ptrCopy1a);
    EXPECT_EQ(ptrCopy1a.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1b(ptr1);
    EXPECT_TRUE(ptrCopy1b);
    EXPECT_EQ(ptrCopy1b.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1c(ptr5);
    EXPECT_TRUE(ptrCopy1c);
    EXPECT_EQ(ptrCopy1c.size(), 0);
    ptr0.resize(3);
    ptr1.resize(1);
    ptr5.resize(3);
    using TCopy2 = sdv::pointer<uint32_t, 2>;
    EXPECT_THROW(TCopy2 ptrCopy2a(ptr5), sdv::XBufferTooSmall);
    sdv::pointer<uint32_t, 2> ptrCopy2b(ptr1);
    EXPECT_TRUE(ptrCopy2b);
    EXPECT_EQ(ptrCopy2b.size(), 1);
    EXPECT_THROW(TCopy2 ptrCopy2c(ptr5), sdv::XBufferTooSmall);
    sdv::pointer<uint32_t, 3> ptrCopy3a(ptr0);
    EXPECT_TRUE(ptrCopy3a);
    EXPECT_EQ(ptrCopy3a.size(), 3);
    sdv::pointer<uint32_t, 3> ptrCopy3b(ptr1);
    EXPECT_TRUE(ptrCopy3b);
    EXPECT_EQ(ptrCopy3b.size(), 1);
    sdv::pointer<uint32_t, 3> ptrCopy3c(ptr5);
    EXPECT_TRUE(ptrCopy3c);
    EXPECT_EQ(ptrCopy3c.size(), 3);
}

TEST_F(CPointerSimpleTypeTest, AssignmentOperatorsDynamicPointer)
{
    // Create pointer
    sdv::pointer<uint32_t> ptr = sdv::make_ptr<uint32_t>();
    EXPECT_TRUE(ptr);

    // Copy assignment
    sdv::pointer<uint32_t> ptrCopy;
    EXPECT_FALSE(ptrCopy);
    ptrCopy = ptr;
    EXPECT_TRUE(ptrCopy);

    // Move assignment
    sdv::pointer<uint32_t> ptrMove;
    EXPECT_FALSE(ptrMove);
    ptrMove = std::move(ptr);
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrMove);
}

TEST_F(CPointerSimpleTypeTest, AssignmentOperatorsFixedPointer)
{
    // Copy assignment
    sdv::pointer<uint32_t, 3> ptr;
    sdv::pointer<uint32_t, 1> ptrCopy;
    EXPECT_TRUE(ptrCopy);
    ptrCopy = ptr;
    EXPECT_TRUE(ptrCopy);
    ptr.resize(1);
    EXPECT_EQ(ptr.size(), 1);
    ptrCopy = ptr;
    EXPECT_EQ(ptrCopy.size(), 1);
    ptr.resize(2);
    EXPECT_EQ(ptr.size(), 2);
    EXPECT_THROW(ptrCopy = ptr, sdv::XBufferTooSmall);

    // Move assignment
    sdv::pointer<uint32_t, 3> ptrMove;
    EXPECT_TRUE(ptrMove);
    ptrMove = std::move(ptr);
    EXPECT_TRUE(ptr);
    EXPECT_TRUE(ptrMove);
}

TEST_F(CPointerSimpleTypeTest, AssignmentOperatorsMixedPointer)
{
    sdv::pointer<uint32_t>	  ptr0;
    sdv::pointer<uint32_t, 1> ptr1;
    sdv::pointer<uint32_t, 5> ptr5;
    EXPECT_FALSE(ptr0);
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr5);

    // Copy constructor
    sdv::pointer<uint32_t> ptrCopy0a;
    EXPECT_FALSE(ptrCopy0a);
    ptrCopy0a = ptr0;
    EXPECT_FALSE(ptrCopy0a);
    EXPECT_EQ(ptrCopy0a.size(), 0);
    sdv::pointer<uint32_t> ptrCopy0b;
    EXPECT_FALSE(ptrCopy0b);
    ptrCopy0b = ptr1;
    EXPECT_TRUE(ptrCopy0b);
    EXPECT_EQ(ptrCopy0b.size(), 0);
    sdv::pointer<uint32_t> ptrCopy0c;
    EXPECT_FALSE(ptrCopy0c);
    ptrCopy0c = ptr5;
    EXPECT_TRUE(ptrCopy0c);
    EXPECT_EQ(ptrCopy0c.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1a;
    EXPECT_TRUE(ptrCopy1a);
    ptrCopy1a = ptr0;
    EXPECT_TRUE(ptrCopy1a);
    EXPECT_EQ(ptrCopy1a.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1b;
    EXPECT_TRUE(ptrCopy1b);
    ptrCopy1b = ptr1;
    EXPECT_TRUE(ptrCopy1b);
    EXPECT_EQ(ptrCopy1b.size(), 0);
    sdv::pointer<uint32_t, 1> ptrCopy1c;
    EXPECT_TRUE(ptrCopy1c);
    ptrCopy1c = ptr5;
    EXPECT_TRUE(ptrCopy1c);
    EXPECT_EQ(ptrCopy1c.size(), 0);
    ptr0.resize(3);
    ptr1.resize(1);
    ptr5.resize(3);
    sdv::pointer<uint32_t, 2> ptrCopy2a;
    EXPECT_TRUE(ptrCopy2a);
    EXPECT_THROW(ptrCopy2a = ptr5, sdv::XBufferTooSmall);
    sdv::pointer<uint32_t, 2> ptrCopy2b;
    EXPECT_TRUE(ptrCopy2b);
    ptrCopy2b = ptr1;
    EXPECT_TRUE(ptrCopy2b);
    EXPECT_EQ(ptrCopy2b.size(), 1);
    sdv::pointer<uint32_t, 2> ptrCopy2c;
    EXPECT_TRUE(ptrCopy2c);
    EXPECT_THROW(ptrCopy2c = ptr5, sdv::XBufferTooSmall);
    sdv::pointer<uint32_t, 3> ptrCopy3a;
    EXPECT_TRUE(ptrCopy3a);
    ptrCopy3a = ptr0;
    EXPECT_TRUE(ptrCopy3a);
    EXPECT_EQ(ptrCopy3a.size(), 3);
    sdv::pointer<uint32_t, 3> ptrCopy3b;
    EXPECT_TRUE(ptrCopy3b);
    ptrCopy3b = ptr1;
    EXPECT_TRUE(ptrCopy3b);
    EXPECT_EQ(ptrCopy3b.size(), 1);
    sdv::pointer<uint32_t, 3> ptrCopy3c;
    EXPECT_TRUE(ptrCopy3c);
    ptrCopy3c = ptr5;
    EXPECT_TRUE(ptrCopy3c);
    EXPECT_EQ(ptrCopy3c.size(), 3);
}

/**
 * @brief Helper class to access attach and detach functions.
 * @tparam T Type of the variable to store.
*/
template <typename T>
class CPointerHelper : public sdv::pointer<T>
{
public:
    /**
     * @brief Attach a uint8_t buffer.
     * @param[in] rptrBuffer Reference to the pointer to attach.
     */
    void attach(sdv::pointer<uint8_t>&& rptrBuffer)
    {
        sdv::pointer<T>::attach(std::move(rptrBuffer));
    }

    /**
     * @brief Detach the uint8_t buffer.
     * @return Reference to the pointer to detach.
     */
    sdv::pointer<uint8_t>&& detach()
    {
        return std::move(sdv::pointer<T>::detach());
    }
};

TEST_F(CPointerSimpleTypeTest, AttachmentDynamicPointer)
{
    // Create the buffer
    sdv::pointer<uint8_t> ptrBuffer = sdv::make_ptr<uint8_t>(101);
    EXPECT_TRUE(ptrBuffer);
    EXPECT_EQ(ptrBuffer.size(), 101);
    EXPECT_EQ(ptrBuffer.ref_count(), 1);

    // Attach
    CPointerHelper<uint32_t> ptr;
    EXPECT_FALSE(ptr);
    ptr.attach(std::move(ptrBuffer));
    EXPECT_FALSE(ptrBuffer);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 25);
    EXPECT_EQ(ptr.ref_count(), 1);

    // Detach
    ptrBuffer = ptr.detach();
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrBuffer);
    EXPECT_EQ(ptrBuffer.size(), 101);
    EXPECT_EQ(ptrBuffer.ref_count(), 1);
}

TEST_F(CPointerSimpleTypeTest, ResetFunctionStaticPointer)
{
    // Create pointer
    sdv::pointer<uint32_t, 10> ptr;
    ptr.resize(10);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 10);
    ptr.reset();
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0);
}

TEST_F(CPointerSimpleTypeTest, ResetFunction)
{
    // Create pointer
    GetMemMgr().ResetPtrSet();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    sdv::pointer<uint32_t> ptr = sdv::make_ptr<uint32_t>();
    ptr.resize(10);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 10);
    EXPECT_NE(GetMemMgr().GetPtrCount(), 0);
    ptr.reset();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    EXPECT_EQ(ptr.size(), 0);
}

TEST_F(CPointerSimpleTypeTest, SwapFunctionStaticPointer)
{
    // Allocate
    sdv::pointer<uint32_t, 250> ptr1;
    EXPECT_TRUE(ptr1);
    ptr1.resize(100);
    EXPECT_EQ(ptr1.size(), 100);
    sdv::pointer<uint32_t, 200> ptr2;
    EXPECT_TRUE(ptr2);
    ptr2.resize(200);
    EXPECT_EQ(ptr2.size(), 200);

    // Swap two pointers
    ptr1.swap(ptr2);
    EXPECT_EQ(ptr1.size(), 200);
    EXPECT_EQ(ptr2.size(), 100);
    sdv::swap(ptr1, ptr2);
    EXPECT_EQ(ptr1.size(), 100);
    EXPECT_EQ(ptr2.size(), 200);

    // Swap with empty pointer
    sdv::pointer<uint32_t, 150> ptr3;
    ptr3.swap(ptr1);
    EXPECT_TRUE(ptr3);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr3.size(), 100);
    EXPECT_EQ(ptr1.size(), 0);
    sdv::pointer<uint32_t, 150> ptr4;
    EXPECT_THROW(sdv::swap(ptr4, ptr2), sdv::XBufferTooSmall);
}

TEST_F(CPointerSimpleTypeTest, SwapFunctionDynamicPointer)
{
    // Allocate
    sdv::pointer<uint32_t> ptr1 = sdv::make_ptr<uint32_t>(100);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1.size(), 100);
    sdv::pointer<uint32_t> ptr2 = sdv::make_ptr<uint32_t>(200);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.size(), 200);

    // Swap two pointers
    ptr1.swap(ptr2);
    EXPECT_EQ(ptr1.size(), 200);
    EXPECT_EQ(ptr2.size(), 100);
    sdv::swap(ptr1, ptr2);
    EXPECT_EQ(ptr1.size(), 100);
    EXPECT_EQ(ptr2.size(), 200);

    // Swap with empty pointer
    sdv::pointer<uint32_t> ptr3;
    ptr3.swap(ptr1);
    EXPECT_TRUE(ptr3);
    EXPECT_FALSE(ptr1);
    sdv::swap(ptr3, ptr1);
    EXPECT_FALSE(ptr3);
    EXPECT_TRUE(ptr1);
}

TEST_F(CPointerSimpleTypeTest, SwapFunctionMixedPointer)
{
    // Allocate
    sdv::pointer<uint32_t> ptr1 = sdv::make_ptr<uint32_t>(100);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1.size(), 100);
    sdv::pointer<uint32_t, 200> ptr2;
    EXPECT_TRUE(ptr2);
    ptr2.resize(200);
    EXPECT_EQ(ptr2.size(), 200);

    // Swap two pointers
    ptr1.swap(ptr2);
    EXPECT_EQ(ptr1.size(), 200);
    EXPECT_EQ(ptr2.size(), 100);
    sdv::swap(ptr1, ptr2);
    EXPECT_EQ(ptr1.size(), 100);
    EXPECT_EQ(ptr2.size(), 200);
}

TEST_F(CPointerSimpleTypeTest, AccessFunctions)
{
    struct STest
    {
        uint32_t uiValue;
    };

    // Empty pointer
    sdv::pointer<STest> ptr;
    EXPECT_FALSE(ptr);

    // Get
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_THROW(*ptr, sdv::XNullPointer);
    EXPECT_THROW(ptr->uiValue, sdv::XNullPointer);

    // Allocate
    ptr = sdv::make_ptr<STest>(100);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 100);

    // Get function
    EXPECT_NE(ptr.get(), nullptr);

    // Assign the value to the array
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr.get()[uiIndex].uiValue = uiIndex;

    // Reference functions
    EXPECT_EQ((*ptr).uiValue, 0u);
    *ptr = STest{101u};
    EXPECT_EQ(ptr->uiValue, 101u);
    EXPECT_EQ((*ptr.get()).uiValue, 101u);
    ptr->uiValue = 0;
    EXPECT_EQ((*ptr).uiValue, 0u);
    EXPECT_EQ((*ptr.get()).uiValue, 0u);

    // Indexed values
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr[uiIndex] = STest{uiIndex + 200u};
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        EXPECT_EQ(ptr.get()[uiIndex].uiValue, uiIndex + 200u);
    EXPECT_THROW(ptr[100], sdv::XIndexOutOfRange);
}

TEST_F(CPointerSimpleTypeTest, SizeFunctionsStaticPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t, 150> ptr;
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0);
    EXPECT_EQ(ptr.capacity(), 150);
    EXPECT_NO_THROW(ptr.resize(100));
    EXPECT_EQ(ptr.size(), 100);
    EXPECT_EQ(ptr.capacity(), 150);

    // Resize to many
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr[uiIndex] = uiIndex + 1000;
    EXPECT_THROW(ptr[101], sdv::XIndexOutOfRange);

    // Resize smaller
    ptr.resize(50);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 50);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex], uiIndex + 1000);
    EXPECT_THROW(ptr[51], sdv::XIndexOutOfRange);

    // Resize larger (the first 50 should still be valid)
    ptr.resize(150);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 150);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex], uiIndex + 1000);
    EXPECT_THROW(ptr[151], sdv::XIndexOutOfRange);

    // Exceed buffer capacity
    EXPECT_THROW(ptr.resize(151), sdv::XBufferTooSmall);
}

TEST_F(CPointerSimpleTypeTest, SizeFunctionsDynamicPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.size(), 0);
    EXPECT_NO_THROW(ptr.resize(100));
    EXPECT_EQ(ptr.size(), 100);
    EXPECT_EQ(ptr.capacity(), 100);

    // Allocate zero
    ptr = sdv::make_ptr<uint32_t>(0);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0);
    EXPECT_EQ(ptr.capacity(), 0);

    // Resize to many
    ptr.resize(100);
    EXPECT_EQ(ptr.size(), 100);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr[uiIndex] = uiIndex + 1000;
    EXPECT_THROW(ptr[101], sdv::XIndexOutOfRange);

    // Resize smaller
    ptr.resize(50);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 50);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex], uiIndex + 1000);
    EXPECT_THROW(ptr[51], sdv::XIndexOutOfRange);

    // Resize larger (the first 50 should still be valid)
    ptr.resize(150);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 150);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex], uiIndex + 1000);
    EXPECT_THROW(ptr[151], sdv::XIndexOutOfRange);

    // Copy the pointer an resize to giant; both pointers should still be valid.
    sdv::pointer<uint32_t> ptrGiant(ptr);
    EXPECT_EQ(ptr, ptrGiant);
    ptrGiant.resize(1024 * 1024 * 8);
    EXPECT_EQ(ptr, ptrGiant);
}

/**
 * @brief Count class increasing the count on construction and descreasing on destruction.
*/
class CCountLifetime
{
public:
    /**
     * @brief Constructor increasing the counter.
     */
    CCountLifetime()
    {
        m_nCount++;
    }

    /**
     * @brief Copy constructor increasing the counter.
     */
    CCountLifetime(const CCountLifetime&)
    {
        m_nCount++;
    }

    /**
    * @brief Copy constructor increasing the counter.
    */
    CCountLifetime(CCountLifetime&)
    {
        m_nCount++;
    }

    /**
     * @brief Move constructor not increasing the counter.
     */
    CCountLifetime(CCountLifetime&&)
    {}

    /**
     * @brief Destructor descreasing the counter.
     */
    ~CCountLifetime()
    {
        m_nCount--;
    }

    /**
     * @brief Assignment operator
     * @return Reference to this class.
     */
    CCountLifetime& operator=(const CCountLifetime&)
    {
        return *this;
    }

    /**
     * @brief Move operator
     * @return Reference to this class.
     */
    CCountLifetime& operator=(CCountLifetime&&)
    {
        return *this;
    }

    static int m_nCount;        ///< Global lifetime counter.
};

int CCountLifetime::m_nCount = 0;

TEST_F(CPointerSimpleTypeTest, LifetimeSizeFunctionsStaticPointer)
{
    CCountLifetime::m_nCount = 0;

    // Create local scope to test destructor
    {
        sdv::pointer<CCountLifetime, 1000> ptr;
        EXPECT_EQ(ptr.size(), 0);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
        ptr.resize(100);
        EXPECT_EQ(ptr.size(), 100);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
        ptr.resize(20);
        EXPECT_EQ(ptr.size(), 20);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
        ptr.resize(500);
        EXPECT_EQ(ptr.size(), 500);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
        ptr.reset();
        EXPECT_EQ(ptr.size(), 0);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
        ptr.resize(750);
        EXPECT_EQ(ptr.size(), 750);
        EXPECT_EQ(CCountLifetime::m_nCount, 1000);
    }
    EXPECT_EQ(CCountLifetime::m_nCount, 0);
}

TEST_F(CPointerSimpleTypeTest, LifetimeSizeFunctionsDynamicPointer)
{
    CCountLifetime::m_nCount = 0;

    // Create local scope to test destructor
    {
        sdv::pointer<CCountLifetime> ptr;
        EXPECT_EQ(ptr.size(), 0);
        EXPECT_EQ(CCountLifetime::m_nCount, 0);
        ptr.resize(100);
        EXPECT_EQ(ptr.size(), 100);
        EXPECT_EQ(CCountLifetime::m_nCount, 100);
        ptr.resize(20);
        EXPECT_EQ(ptr.size(), 20);
        EXPECT_EQ(CCountLifetime::m_nCount, 20);
        ptr.resize(500);
        EXPECT_EQ(ptr.size(), 500);
        EXPECT_EQ(CCountLifetime::m_nCount, 500);
        ptr.reset();
        EXPECT_EQ(ptr.size(), 0);
        EXPECT_EQ(CCountLifetime::m_nCount, 0);
        ptr.resize(750);
        EXPECT_EQ(ptr.size(), 750);
        EXPECT_EQ(CCountLifetime::m_nCount, 750);
    }
    EXPECT_EQ(CCountLifetime::m_nCount, 0);

    // Create pointer
    CCountLifetime::m_nCount = 0;
    sdv::pointer<CCountLifetime> ptr = sdv::make_ptr<CCountLifetime>();
    ptr.resize(10);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 10u);
    EXPECT_EQ(CCountLifetime::m_nCount, 10);
    ptr.reset();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    EXPECT_EQ(ptr.size(), 0u);
    EXPECT_EQ(CCountLifetime::m_nCount, 0);

    // Allocate
    sdv::pointer<CCountLifetime> ptr1 = sdv::make_ptr<CCountLifetime>(100);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1.size(), 100u);
    sdv::pointer<CCountLifetime, 200> ptr2;
    EXPECT_TRUE(ptr2);
    ptr2.resize(200);
    EXPECT_EQ(ptr2.size(), 200u);

    // Swap two pointers
    ptr1.swap(ptr2);
    EXPECT_EQ(ptr1.size(), 200u);
    EXPECT_EQ(ptr2.size(), 100u);
    sdv::swap(ptr1, ptr2);
    EXPECT_EQ(ptr1.size(), 100u);
    EXPECT_EQ(ptr2.size(), 200u);
}

TEST_F(CPointerSimpleTypeTest, RefCountFunctionDynamicPointer)
    {
    // Empty pointer
    sdv::pointer<uint32_t> ptr;
    EXPECT_EQ(ptr.ref_count(), 0);

    // Allocate
    ptr = sdv::make_ptr<uint32_t>();
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.ref_count(), 1);

    // Copy
    sdv::pointer<uint32_t> ptrCopy(ptr);
    EXPECT_TRUE(ptrCopy);
    EXPECT_EQ(ptr.ref_count(), 2);
    EXPECT_EQ(ptrCopy.ref_count(), 2);
    sdv::pointer<uint32_t> ptrCopy2;
    EXPECT_FALSE(ptrCopy2);
    ptrCopy2 = ptrCopy;
    EXPECT_TRUE(ptrCopy2);
    EXPECT_EQ(ptr.ref_count(), 3);
    EXPECT_EQ(ptrCopy.ref_count(), 3);
    EXPECT_EQ(ptrCopy2.ref_count(), 3);

    // Move
    sdv::pointer<uint32_t> ptrMove(std::move(ptrCopy));
    EXPECT_TRUE(ptrMove);
    EXPECT_EQ(ptr.ref_count(), 3);
    EXPECT_EQ(ptrCopy.ref_count(), 0);
    EXPECT_EQ(ptrCopy2.ref_count(), 3);
    EXPECT_EQ(ptrMove.ref_count(), 3);
    sdv::pointer<uint32_t> ptrMove2;
    EXPECT_FALSE(ptrMove2);
    ptrMove2 = std::move(ptrCopy2);
    EXPECT_EQ(ptr.ref_count(), 3);
    EXPECT_EQ(ptrCopy.ref_count(), 0);
    EXPECT_EQ(ptrCopy2.ref_count(), 0);
    EXPECT_EQ(ptrMove.ref_count(), 3);
    EXPECT_EQ(ptrMove2.ref_count(), 3);

    // Reset
    ptrMove.reset();
    EXPECT_EQ(ptr.ref_count(), 2);
    EXPECT_EQ(ptrMove.ref_count(), 0);
    EXPECT_EQ(ptrMove2.ref_count(), 2);
    ptrMove2.reset();
    EXPECT_EQ(ptr.ref_count(), 1);
    EXPECT_EQ(ptrMove.ref_count(), 0);
    EXPECT_EQ(ptrMove2.ref_count(), 0);
}

TEST_F(CPointerSimpleTypeTest, CompareOperatorStaticPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t, 10> ptrLeft;
    sdv::pointer<uint32_t, 15> ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Resize one pointer and fill with counter
    ptrLeft.resize(10);
    for (size_t nIndex = 0; nIndex < ptrLeft.size(); nIndex++)
        ptrLeft[nIndex] = static_cast<uint32_t>(nIndex);
    EXPECT_EQ(ptrLeft.size(), 10);
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_FALSE(ptrLeft <= ptrRight);
    EXPECT_TRUE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Copy the allocation
    ptrRight = ptrLeft;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Free left allocation
    ptrLeft.reset();
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_TRUE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_FALSE(ptrLeft >= ptrRight);
}

TEST_F(CPointerSimpleTypeTest, CompareOperatorDynamicPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t> ptrLeft, ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Allocate one pointer
    ptrLeft = sdv::make_ptr<uint32_t>(10);
    EXPECT_TRUE(ptrLeft);
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_FALSE(ptrLeft <= ptrRight);
    EXPECT_TRUE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Copy the allocation
    ptrRight = ptrLeft;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Free left allocation
    ptrLeft.reset();
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_TRUE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_FALSE(ptrLeft >= ptrRight);

    // Reallocate left allocation
    ptrLeft = sdv::make_ptr<uint32_t>(10);
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
}

TEST_F(CPointerSimpleTypeTest, CompareOperatorMixedPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t, 10> ptrLeft;
    sdv::pointer<uint32_t> ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Resize one pointer and fill with counter
    ptrLeft.resize(10);
    for (size_t nIndex = 0; nIndex < ptrLeft.size(); nIndex++)
        ptrLeft[nIndex] = static_cast<uint32_t>(nIndex);
    EXPECT_EQ(ptrLeft.size(), 10);
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_FALSE(ptrLeft <= ptrRight);
    EXPECT_TRUE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Copy the allocation
    ptrRight = ptrLeft;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Free left allocation
    ptrLeft.reset();
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
    EXPECT_TRUE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_FALSE(ptrLeft >= ptrRight);
}

TEST_F(CPointerSimpleTypeTest, CompareNullOperatorDynamicPointer)
{
    // Empty pointer
    sdv::pointer<uint32_t> ptr;
    EXPECT_TRUE(ptr == nullptr);
    EXPECT_TRUE(nullptr == ptr);
    EXPECT_FALSE(ptr != nullptr);
    EXPECT_FALSE(nullptr != ptr);
    EXPECT_FALSE(ptr < nullptr);
    EXPECT_FALSE(nullptr < ptr);
    EXPECT_TRUE(ptr <= nullptr);
    EXPECT_TRUE(nullptr <= ptr);
    EXPECT_FALSE(ptr > nullptr);
    EXPECT_FALSE(nullptr > ptr);
    EXPECT_TRUE(ptr >= nullptr);
    EXPECT_TRUE(nullptr >= ptr);

    // Allocate one pointer
    ptr = sdv::make_ptr<uint32_t>(10);
    EXPECT_TRUE(ptr);
    EXPECT_FALSE(ptr == nullptr);
    EXPECT_FALSE(nullptr == ptr);
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_TRUE(nullptr != ptr);
    EXPECT_FALSE(ptr < nullptr);
    EXPECT_TRUE(nullptr < ptr);
    EXPECT_FALSE(ptr <= nullptr);
    EXPECT_TRUE(nullptr <= ptr);
    EXPECT_TRUE(ptr > nullptr);
    EXPECT_FALSE(nullptr > ptr);
    EXPECT_TRUE(ptr >= nullptr);
    EXPECT_FALSE(nullptr >= ptr);

    // Free allocation
    ptr.reset();
    EXPECT_TRUE(ptr == nullptr);
    EXPECT_TRUE(nullptr == ptr);
    EXPECT_FALSE(ptr != nullptr);
    EXPECT_FALSE(nullptr != ptr);
    EXPECT_FALSE(ptr < nullptr);
    EXPECT_FALSE(nullptr < ptr);
    EXPECT_TRUE(ptr <= nullptr);
    EXPECT_TRUE(nullptr <= ptr);
    EXPECT_FALSE(ptr > nullptr);
    EXPECT_FALSE(nullptr > ptr);
    EXPECT_TRUE(ptr >= nullptr);
    EXPECT_TRUE(nullptr >= ptr);
}

