#include <sstream>

#include <support/pointer.h>
#include <cstring>

#include "basic_types_test.h"

// There are some versions of GCC that produce bogus warnings for -Wstringop-overflow (e.g. version 9.4 warns, 11.4 not - changing
// the compile order without changing the logical behavior, will produce different results).
// See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100477
// And https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115074
// Suppress this warning for the string class.
// NOTE 03.08.2025: Additional bogus warnigs/errors are suppressed for newer versions of the compiler.
#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
    #pragma GCC diagnostic ignored "-Warray-bounds"
#endif

using CPointerTypeTest = CBasicTypesTest;

TEST_F(CPointerTypeTest, MakeFixedPointer)
{
    sdv::pointer<uint32_t, 1> ptr0;
    EXPECT_TRUE(ptr0);
}

TEST_F(CPointerTypeTest, MakeDynamicPointer)
{
    sdv::pointer<uint32_t> ptr0 = sdv::make_ptr<uint32_t>(0);
    EXPECT_TRUE(ptr0);
    sdv::pointer<uint32_t> ptr1 = sdv::make_ptr<uint32_t>();
    EXPECT_TRUE(ptr1);
    sdv::pointer<uint32_t> ptr2 = sdv::make_ptr<uint32_t>(2);
    EXPECT_TRUE(ptr2);
}

TEST_F(CPointerTypeTest, ConstructorDynamicPointer)
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

TEST_F(CPointerTypeTest, ConstructorFixedPointer)
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

TEST_F(CPointerTypeTest, ConstructorMixedPointer)
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

TEST_F(CPointerTypeTest, AssignmentOperatorsDynamicPointer)
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

TEST_F(CPointerTypeTest, AssignmentOperatorsFixedPointer)
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

TEST_F(CPointerTypeTest, AssignmentOperatorsMixedPointer)
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

TEST_F(CPointerTypeTest, AttachmentDynamicPointer)
{
    // Functions are protected. Derive the class to access the functions
    class CDerivedPtr : public sdv::pointer<uint32_t>
    {
    public:
        void attach(pointer<uint8_t>&& rptrBuffer) { sdv::pointer<uint32_t>::attach(std::move(rptrBuffer)); }
        pointer<uint8_t>&& detach() { return sdv::pointer<uint32_t>::detach(); }
    };

    // Create the buffer
    sdv::pointer<uint8_t>  ptrBuffer = sdv::make_ptr<uint8_t>(101);
    EXPECT_TRUE(ptrBuffer);
    EXPECT_EQ(ptrBuffer.size(), 101);
    EXPECT_EQ(ptrBuffer.ref_count(), 1);

    // Attach
    CDerivedPtr ptr;
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

TEST_F(CPointerTypeTest, ResetFunctionStaticPointer)
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

TEST_F(CPointerTypeTest, ResetFunction)
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

TEST_F(CPointerTypeTest, SwapFunctionStaticPointer)
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

TEST_F(CPointerTypeTest, SwapFunctionDynamicPointer)
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

TEST_F(CPointerTypeTest, SwapFunctionMixedPointer)
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

TEST_F(CPointerTypeTest, AccessFunctions)
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

TEST_F(CPointerTypeTest, SizeFunctionsStaticPointer)
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

TEST_F(CPointerTypeTest, SizeFunctionsDynamicPointer)
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

TEST_F(CPointerTypeTest, RefCountFunctionDynamicPointer)
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

TEST_F(CPointerTypeTest, CompareOperatorStaticPointer)
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

TEST_F(CPointerTypeTest, CompareOperatorDynamicPointer)
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

TEST_F(CPointerTypeTest, CompareOperatorMixedPointer)
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

TEST_F(CPointerTypeTest, CompareNullOperatorDynamicPointer)
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

TEST_F(CPointerTypeTest, StreamOperatorStaticPointer)
{
    // Stream one number
    std::stringstream sstream;
    sdv::pointer<uint32_t, 1> ptr;
    ptr.resize(1);
    *ptr = 100;
    sstream << ptr;
    EXPECT_EQ(sstream.str(), "100");

    // Stream multiple numbers
    std::stringstream sstream2;
    sdv::pointer<uint32_t, 10> ptr2;
    ptr2.resize(10);
    for (uint32_t uiIndex = 0u; uiIndex < 10u; uiIndex++)
        ptr2[uiIndex] = 100u + uiIndex;
    sstream2 << ptr2;
    EXPECT_EQ(sstream2.str(), "100101102103104105106107108109");
}

TEST_F(CPointerTypeTest, StreamOperatorDynamicPointer)
{
    // Stream one number
    std::stringstream  sstream;
    sdv::pointer<uint32_t> ptr;
    ptr	 = sdv::make_ptr<uint32_t>();
    *ptr = 100;
    sstream << ptr;
    EXPECT_EQ(sstream.str(), "100");

    // Stream multiple numbers
    std::stringstream sstream2;
    ptr = sdv::make_ptr<uint32_t>(10);
    for (uint32_t uiIndex = 0u; uiIndex < 10u; uiIndex++)
        ptr[uiIndex] = 100u + uiIndex;
    sstream2 << ptr;
    EXPECT_EQ(sstream2.str(), "100101102103104105106107108109");
}

TEST_F(CPointerTypeTest, MakePointerFromStream)
{
    std::stringstream sstream;
    sstream << "Hello, this is a text which should be used to check the from-stream functionality!";
    sdv::pointer<uint8_t> ptrData = sdv::from_stream(sstream.rdbuf());
    ASSERT_EQ(ptrData.size(), sstream.str().size());
    // NOTE: Since the C-string is zero base, it is not possible to do a simple string compare.
    EXPECT_EQ(memcmp(sstream.str().c_str(), ptrData.get(), ptrData.size()), 0);
}

TEST_F(CPointerTypeTest, StorePointerIntoStream)
{
    char sz[] = "Hello, this is a text which should be used to check the to-stream functionality!";
    sdv::pointer<uint8_t> ptrData;
    ptrData.resize(sizeof(sz));
    ASSERT_EQ(ptrData.size(), sizeof(sz));
    std::copy(std::begin(sz), std::end(sz), ptrData.get());
    std::stringstream sstream;
    sdv::to_stream(ptrData, sstream.rdbuf());
    // NOTE: Since the C-string is zero base, it is not possible to do a simple string compare.
    EXPECT_EQ(memcmp(sstream.str().c_str(), sz, ptrData.size()), 0);
}

TEST_F(CPointerTypeTest, CastFromPointer)
{
    struct STest
    {
        int32_t     i32 = 10;
        uint16_t    ui16 = 99;
        char        sz[2] = { 'a', 'b' };
        int64_t     i64 = -98765432;
        uint64_t    ui64 = 123456789;
    } test;

    // Test for exact size
    sdv::pointer<uint8_t> ptrNoOffset;
    ptrNoOffset.resize(sizeof(test));
    ASSERT_EQ(ptrNoOffset.size(), sizeof(test));
    std::memcpy(ptrNoOffset.get(), &test, sizeof(STest));
    STest* pTest = sdv::cast<STest>(ptrNoOffset);
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test for smaller size
    sdv::pointer<uint8_t> ptrSmaller;
    ptrSmaller.resize(sizeof(test));
    ASSERT_EQ(ptrSmaller.size(), sizeof(test));
    std::memcpy(ptrSmaller.get(), &test, sizeof(STest));
    ptrSmaller.resize(sizeof(STest) - 1);
    pTest = sdv::cast<STest>(ptrSmaller);
    EXPECT_EQ(pTest, nullptr);

    // Test for larger size
    sdv::pointer<uint8_t> ptrLarger;
    ptrLarger.resize(sizeof(test) + 100);
    ASSERT_GT(ptrLarger.size(), sizeof(test));
    std::memcpy(ptrLarger.get(), &test, sizeof(STest));
    pTest = sdv::cast<STest>(ptrLarger);
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test with offset
    sdv::pointer<uint8_t> ptrOffset;
    ptrOffset.resize(sizeof(test) + 100);
    ASSERT_GT(ptrOffset.size(), sizeof(test));
    std::memcpy(ptrOffset.get() + 100, &test, sizeof(STest));
    pTest = sdv::cast<STest>(ptrOffset, 100);
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test with offset, beyond size
    sdv::pointer<uint8_t> ptrOffsetBeyond;
    ptrOffsetBeyond.resize(sizeof(test) + 100);
    ASSERT_GT(ptrOffsetBeyond.size(), sizeof(test));
    std::memcpy(ptrOffsetBeyond.get() + 100, &test, sizeof(STest));
    pTest = sdv::cast<STest>(ptrOffsetBeyond, 101);
    EXPECT_EQ(pTest, nullptr);
}

TEST_F(CPointerTypeTest, ConstCastFromPointer)
{
    struct STest
    {
        int32_t     i32 = 10;
        uint16_t    ui16 = 99;
        char        sz[2] = { 'a', 'b' };
        int64_t     i64 = -98765432;
        uint64_t    ui64 = 123456789;
    } test;

    // Test for exact size
    sdv::pointer<uint8_t> ptrNoOffset;
    ptrNoOffset.resize(sizeof(test));
    ASSERT_EQ(ptrNoOffset.size(), sizeof(test));
    std::memcpy(ptrNoOffset.get(), &test, sizeof(STest));
    const STest* pTest = sdv::cast<STest>(static_cast<const sdv::pointer<uint8_t>&>(ptrNoOffset));
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test for smaller size
    sdv::pointer<uint8_t> ptrSmaller;
    ptrSmaller.resize(sizeof(test));
    ASSERT_EQ(ptrSmaller.size(), sizeof(test));
    std::memcpy(ptrSmaller.get(), &test, sizeof(STest));
    ptrSmaller.resize(sizeof(STest) - 1);
    pTest = sdv::cast<STest>(static_cast<const sdv::pointer<uint8_t>&>(ptrSmaller));
    EXPECT_EQ(pTest, nullptr);

    // Test for larger size
    sdv::pointer<uint8_t> ptrLarger;
    ptrLarger.resize(sizeof(test) + 100);
    ASSERT_GT(ptrLarger.size(), sizeof(test));
    std::memcpy(ptrLarger.get(), &test, sizeof(STest));
    pTest = sdv::cast<STest>(static_cast<const sdv::pointer<uint8_t>&>(ptrLarger));
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test with offset
    sdv::pointer<uint8_t> ptrOffset;
    ptrOffset.resize(sizeof(test) + 100);
    ASSERT_GT(ptrOffset.size(), sizeof(test));
    std::memcpy(ptrOffset.get() + 100, &test, sizeof(STest));
    pTest = sdv::cast<STest>(static_cast<const sdv::pointer<uint8_t>&>(ptrOffset), 100);
    EXPECT_NE(pTest, nullptr);
    EXPECT_EQ(std::memcmp(&test, pTest, sizeof(STest)), 0);

    // Test with offset, beyond size
    sdv::pointer<uint8_t> ptrOffsetBeyond;
    ptrOffsetBeyond.resize(sizeof(test) + 100);
    ASSERT_GT(ptrOffsetBeyond.size(), sizeof(test));
    std::memcpy(ptrOffsetBeyond.get() + 100, &test, sizeof(STest));
    pTest = sdv::cast<STest>(static_cast<const sdv::pointer<uint8_t>&>(ptrOffsetBeyond), 101);
    EXPECT_EQ(pTest, nullptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
