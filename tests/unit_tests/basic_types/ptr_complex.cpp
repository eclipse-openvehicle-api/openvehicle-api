#include <sstream>

#include <support/pointer.h>

#include "basic_types_test.h"

using CPointerComplexTypeTest = CBasicTypesTest;

struct SComplexStruct
{
    sdv::string ssHello = {"Hello"};
    uint8_t ui8 = 8u;
    uint32_t ui32 = 32u;
    std::vector<std::string> vecHello = {"Hello1", "Hello2", "Hello3", "Hello4"};
};

bool operator==(const SComplexStruct& rsLeft, const SComplexStruct& rsRight)
{
    return rsLeft.ssHello == rsRight.ssHello && rsLeft.ui8 == rsRight.ui8 && rsLeft.ui32 == rsRight.ui32
           && rsLeft.vecHello == rsRight.vecHello;
}

bool operator<(const SComplexStruct& rsLeft, const SComplexStruct& rsRight)
{
    if (rsLeft.ssHello < rsRight.ssHello) return true;
    if (rsLeft.ssHello > rsRight.ssHello) return false;
    if (rsLeft.ui8 < rsRight.ui8) return true;
    if (rsLeft.ui8 > rsRight.ui8) return false;
    if (rsLeft.ui32 < rsRight.ui32) return true;
    if (rsLeft.ui32 > rsRight.ui32) return false;
    if (rsLeft.vecHello < rsRight.vecHello) return true;
    if (rsLeft.vecHello > rsRight.vecHello) return false;
    return false;
}

template <class U, class V>
inline std::basic_ostream<U, V>& operator<<(std::basic_ostream<U, V>& rstream, const SComplexStruct& rsComplex)
{
    rstream << "{" << rsComplex.ssHello << ", " << rsComplex.ui8 << ", " << rsComplex.ui32 << "{";
    bool bInitial = true;
    for (const std::string& rss : rsComplex.vecHello)
    {
        if (!bInitial) rstream << ", ";
        bInitial = false;
        rstream << rss;
    }
    rstream << "}}";
    return rstream;
}

namespace std
{
    template <>
    struct less<SComplexStruct>
    {
        bool operator()(const SComplexStruct& rsLeft, const SComplexStruct& rsRight) const
        {
            if (rsLeft.ssHello < rsRight.ssHello) return true;
            if (rsLeft.ssHello > rsRight.ssHello) return false;
            if (rsLeft.ui8 < rsRight.ui8) return true;
            if (rsLeft.ui8 > rsRight.ui8) return false;
            if (rsLeft.ui32 < rsRight.ui32) return true;
            if (rsLeft.ui32 > rsRight.ui32) return false;
            if (rsLeft.vecHello < rsRight.vecHello) return true;
            if (rsLeft.vecHello > rsRight.vecHello) return false;
            return false;
        }
    };
}

TEST_F(CPointerComplexTypeTest, MakeFixedPointer)
{
    sdv::pointer<SComplexStruct, 1> ptr0;
    EXPECT_TRUE(ptr0);
}

TEST_F(CPointerComplexTypeTest, MakeDynamicPointer)
{
    sdv::pointer<SComplexStruct> ptr0 = sdv::make_ptr<SComplexStruct>(0);
    EXPECT_TRUE(ptr0);
    sdv::pointer<SComplexStruct> ptr1 = sdv::make_ptr<SComplexStruct>();
    EXPECT_TRUE(ptr1);
    sdv::pointer<SComplexStruct> ptr2 = sdv::make_ptr<SComplexStruct>(2);
    EXPECT_TRUE(ptr2);
}

TEST_F(CPointerComplexTypeTest, ConstructorDynamicPointer)
{
    // Default constructor
    sdv::pointer<SComplexStruct> ptr;
    EXPECT_FALSE(ptr);

    // Allocate
    ptr = sdv::make_ptr<SComplexStruct>();
    EXPECT_TRUE(ptr);

    // Copy constructor
    sdv::pointer<SComplexStruct> ptrCopy(ptr);
    EXPECT_TRUE(ptrCopy);

    // Move constructor
    sdv::pointer<SComplexStruct> ptrMove(std::move(ptr));
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrMove);

    // Destructor
    GetMemMgr().ResetPtrSet();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    sdv::pointer<SComplexStruct>* pptr = new sdv::pointer<SComplexStruct>(sdv::make_ptr<SComplexStruct>());
    EXPECT_NE(pptr, nullptr);
    EXPECT_NE(GetMemMgr().GetPtrCount(), 0);
    delete pptr;
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
}

TEST_F(CPointerComplexTypeTest, ConstructorFixedPointer)
{
    // Default constructor
    sdv::pointer<SComplexStruct, 1> ptr;
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0u);

    // Copy constructor
    sdv::pointer<SComplexStruct, 1> ptrCopy1(ptr);
    EXPECT_TRUE(ptrCopy1);
    EXPECT_EQ(ptrCopy1.size(), 0u);
    sdv::pointer<SComplexStruct, 10> ptrCopy10(ptr);
    EXPECT_TRUE(ptrCopy10);
    EXPECT_EQ(ptrCopy10.size(), 0u);
    using TCopy2 = sdv::pointer<SComplexStruct, 2>;
    EXPECT_NO_THROW(TCopy2 ptrCopy2(ptrCopy10));
    ptrCopy10.resize(5);
    EXPECT_EQ(ptrCopy10.size(), 5u);
    using TCopy3 = sdv::pointer<SComplexStruct, 3>;
    EXPECT_THROW(TCopy3 ptrCopy3(ptrCopy10), sdv::XBufferTooSmall);

    // Move constructor
    ptr.resize(1);
    EXPECT_EQ(ptr.size(), 1u);
    sdv::pointer<SComplexStruct, 1> ptrMove(std::move(ptr));
    EXPECT_TRUE(ptr);
    EXPECT_TRUE(ptrMove);
    EXPECT_EQ(ptr.size(), 0u);
    EXPECT_EQ(ptrMove.size(), 1u);
}

TEST_F(CPointerComplexTypeTest, ConstructorMixedPointer)
{
    // Default constructor
    sdv::pointer<SComplexStruct> ptr0;
    sdv::pointer<SComplexStruct, 1> ptr1;
    sdv::pointer<SComplexStruct, 5> ptr5;
    EXPECT_FALSE(ptr0);
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr5);

    // Copy constructor
    sdv::pointer<SComplexStruct> ptrCopy0a(ptr0);
    EXPECT_FALSE(ptrCopy0a);
    EXPECT_EQ(ptrCopy0a.size(), 0u);
    sdv::pointer<SComplexStruct> ptrCopy0b(ptr1);
    EXPECT_TRUE(ptrCopy0b);
    EXPECT_EQ(ptrCopy0b.size(), 0u);
    sdv::pointer<SComplexStruct> ptrCopy0c(ptr5);
    EXPECT_TRUE(ptrCopy0c);
    EXPECT_EQ(ptrCopy0c.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1a(ptr0);
    EXPECT_TRUE(ptrCopy1a);
    EXPECT_EQ(ptrCopy1a.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1b(ptr1);
    EXPECT_TRUE(ptrCopy1b);
    EXPECT_EQ(ptrCopy1b.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1c(ptr5);
    EXPECT_TRUE(ptrCopy1c);
    EXPECT_EQ(ptrCopy1c.size(), 0u);
    ptr0.resize(3);
    ptr1.resize(1);
    ptr5.resize(3);
    using TCopy2 = sdv::pointer<SComplexStruct, 2>;
    EXPECT_THROW(TCopy2 ptrCopy2a(ptr5), sdv::XBufferTooSmall);
    sdv::pointer<SComplexStruct, 2> ptrCopy2b(ptr1);
    EXPECT_TRUE(ptrCopy2b);
    EXPECT_EQ(ptrCopy2b.size(), 1u);
    EXPECT_THROW(TCopy2 ptrCopy2c(ptr5), sdv::XBufferTooSmall);
    sdv::pointer<SComplexStruct, 3> ptrCopy3a(ptr0);
    EXPECT_TRUE(ptrCopy3a);
    EXPECT_EQ(ptrCopy3a.size(), 3u);
    sdv::pointer<SComplexStruct, 3> ptrCopy3b(ptr1);
    EXPECT_TRUE(ptrCopy3b);
    EXPECT_EQ(ptrCopy3b.size(), 1u);
    sdv::pointer<SComplexStruct, 3> ptrCopy3c(ptr5);
    EXPECT_TRUE(ptrCopy3c);
    EXPECT_EQ(ptrCopy3c.size(), 3u);
}

TEST_F(CPointerComplexTypeTest, AssignmentOperatorsDynamicPointer)
{
    // Create pointer
    sdv::pointer<SComplexStruct> ptr = sdv::make_ptr<SComplexStruct>();
    EXPECT_TRUE(ptr);

    // Copy assignment
    sdv::pointer<SComplexStruct> ptrCopy;
    EXPECT_FALSE(ptrCopy);
    ptrCopy = ptr;
    EXPECT_TRUE(ptrCopy);

    // Move assignment
    sdv::pointer<SComplexStruct> ptrMove;
    EXPECT_FALSE(ptrMove);
    ptrMove = std::move(ptr);
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrMove);
}

TEST_F(CPointerComplexTypeTest, AssignmentOperatorsFixedPointer)
{
    // Copy assignment
    sdv::pointer<SComplexStruct, 3> ptr;
    sdv::pointer<SComplexStruct, 1> ptrCopy;
    EXPECT_TRUE(ptrCopy);
    ptrCopy = ptr;
    EXPECT_TRUE(ptrCopy);
    ptr.resize(1);
    EXPECT_EQ(ptr.size(), 1u);
    ptrCopy = ptr;
    EXPECT_EQ(ptrCopy.size(), 1u);
    ptr.resize(2);
    EXPECT_EQ(ptr.size(), 2u);
    EXPECT_THROW(ptrCopy = ptr, sdv::XBufferTooSmall);

    // Move assignment
    sdv::pointer<SComplexStruct, 3> ptrMove;
    EXPECT_TRUE(ptrMove);
    ptrMove = std::move(ptr);
    EXPECT_TRUE(ptr);
    EXPECT_TRUE(ptrMove);
}

TEST_F(CPointerComplexTypeTest, AssignmentOperatorsMixedPointer)
{
    sdv::pointer<SComplexStruct>	  ptr0;
    sdv::pointer<SComplexStruct, 1> ptr1;
    sdv::pointer<SComplexStruct, 5> ptr5;
    EXPECT_FALSE(ptr0);
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr5);

    // Copy constructor
    sdv::pointer<SComplexStruct> ptrCopy0a;
    EXPECT_FALSE(ptrCopy0a);
    ptrCopy0a = ptr0;
    EXPECT_FALSE(ptrCopy0a);
    EXPECT_EQ(ptrCopy0a.size(), 0u);
    sdv::pointer<SComplexStruct> ptrCopy0b;
    EXPECT_FALSE(ptrCopy0b);
    ptrCopy0b = ptr1;
    EXPECT_TRUE(ptrCopy0b);
    EXPECT_EQ(ptrCopy0b.size(), 0u);
    sdv::pointer<SComplexStruct> ptrCopy0c;
    EXPECT_FALSE(ptrCopy0c);
    ptrCopy0c = ptr5;
    EXPECT_TRUE(ptrCopy0c);
    EXPECT_EQ(ptrCopy0c.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1a;
    EXPECT_TRUE(ptrCopy1a);
    ptrCopy1a = ptr0;
    EXPECT_TRUE(ptrCopy1a);
    EXPECT_EQ(ptrCopy1a.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1b;
    EXPECT_TRUE(ptrCopy1b);
    ptrCopy1b = ptr1;
    EXPECT_TRUE(ptrCopy1b);
    EXPECT_EQ(ptrCopy1b.size(), 0u);
    sdv::pointer<SComplexStruct, 1> ptrCopy1c;
    EXPECT_TRUE(ptrCopy1c);
    ptrCopy1c = ptr5;
    EXPECT_TRUE(ptrCopy1c);
    EXPECT_EQ(ptrCopy1c.size(), 0u);
    ptr0.resize(3);
    ptr1.resize(1);
    ptr5.resize(3);
    sdv::pointer<SComplexStruct, 2> ptrCopy2a;
    EXPECT_TRUE(ptrCopy2a);
    EXPECT_THROW(ptrCopy2a = ptr5, sdv::XBufferTooSmall);
    sdv::pointer<SComplexStruct, 2> ptrCopy2b;
    EXPECT_TRUE(ptrCopy2b);
    ptrCopy2b = ptr1;
    EXPECT_TRUE(ptrCopy2b);
    EXPECT_EQ(ptrCopy2b.size(), 1u);
    sdv::pointer<SComplexStruct, 2> ptrCopy2c;
    EXPECT_TRUE(ptrCopy2c);
    EXPECT_THROW(ptrCopy2c = ptr5, sdv::XBufferTooSmall);
    sdv::pointer<SComplexStruct, 3> ptrCopy3a;
    EXPECT_TRUE(ptrCopy3a);
    ptrCopy3a = ptr0;
    EXPECT_TRUE(ptrCopy3a);
    EXPECT_EQ(ptrCopy3a.size(), 3u);
    sdv::pointer<SComplexStruct, 3> ptrCopy3b;
    EXPECT_TRUE(ptrCopy3b);
    ptrCopy3b = ptr1;
    EXPECT_TRUE(ptrCopy3b);
    EXPECT_EQ(ptrCopy3b.size(), 1u);
    sdv::pointer<SComplexStruct, 3> ptrCopy3c;
    EXPECT_TRUE(ptrCopy3c);
    ptrCopy3c = ptr5;
    EXPECT_TRUE(ptrCopy3c);
    EXPECT_EQ(ptrCopy3c.size(), 3u);
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

 TEST_F(CPointerComplexTypeTest, AttachmentDynamicPointer)
{
    // Create the buffer
    sdv::pointer<uint8_t> ptrBuffer = sdv::make_ptr<uint8_t>(25 * sizeof(SComplexStruct));
    EXPECT_TRUE(ptrBuffer);
    EXPECT_EQ(ptrBuffer.size(), 25 * sizeof(SComplexStruct));
    EXPECT_EQ(ptrBuffer.ref_count(), 1);

    // Attach
    CPointerHelper<SComplexStruct> ptr;
    EXPECT_FALSE(ptr);
    ptr.attach(std::move(ptrBuffer));
    EXPECT_FALSE(ptrBuffer);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 25u);
    EXPECT_EQ(ptr.ref_count(), 1);

    // Detach
    ptrBuffer = ptr.detach();
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptrBuffer);
    EXPECT_EQ(ptrBuffer.size(), 25 * sizeof(SComplexStruct));
    EXPECT_EQ(ptrBuffer.ref_count(), 1);
}

TEST_F(CPointerComplexTypeTest, ResetFunctionStaticPointer)
{
    // Create pointer
    sdv::pointer<SComplexStruct, 10> ptr;
    ptr.resize(10);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 10u);
    ptr.reset();
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0u);
}

TEST_F(CPointerComplexTypeTest, ResetFunction)
{
    // Create pointer
    GetMemMgr().ResetPtrSet();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    sdv::pointer<SComplexStruct> ptr = sdv::make_ptr<SComplexStruct>();
    ptr.resize(10);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 10u);
    EXPECT_NE(GetMemMgr().GetPtrCount(), 0);
    ptr.reset();
    EXPECT_EQ(GetMemMgr().GetPtrCount(), 0);
    EXPECT_EQ(ptr.size(), 0u);
}

TEST_F(CPointerComplexTypeTest, SwapFunctionStaticPointer)
{
    // Allocate
    sdv::pointer<SComplexStruct, 250> ptr1;
    EXPECT_TRUE(ptr1);
    ptr1.resize(100);
    EXPECT_EQ(ptr1.size(), 100u);
    sdv::pointer<SComplexStruct, 200> ptr2;
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

    // Swap with empty pointer
    sdv::pointer<SComplexStruct, 150> ptr3;
    ptr3.swap(ptr1);
    EXPECT_TRUE(ptr3);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr3.size(), 100u);
    EXPECT_EQ(ptr1.size(), 0u);
    sdv::pointer<SComplexStruct, 150> ptr4;
    EXPECT_THROW(sdv::swap(ptr4, ptr2), sdv::XBufferTooSmall);
}

TEST_F(CPointerComplexTypeTest, SwapFunctionDynamicPointer)
{
    // Allocate
    sdv::pointer<SComplexStruct> ptr1 = sdv::make_ptr<SComplexStruct>(100);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1.size(), 100u);
    sdv::pointer<SComplexStruct> ptr2 = sdv::make_ptr<SComplexStruct>(200);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.size(), 200u);

    // Swap two pointers
    ptr1.swap(ptr2);
    EXPECT_EQ(ptr1.size(), 200u);
    EXPECT_EQ(ptr2.size(), 100u);
    sdv::swap(ptr1, ptr2);
    EXPECT_EQ(ptr1.size(), 100u);
    EXPECT_EQ(ptr2.size(), 200u);

    // Swap with empty pointer
    sdv::pointer<SComplexStruct> ptr3;
    ptr3.swap(ptr1);
    EXPECT_TRUE(ptr3);
    EXPECT_FALSE(ptr1);
    sdv::swap(ptr3, ptr1);
    EXPECT_FALSE(ptr3);
    EXPECT_TRUE(ptr1);
}

TEST_F(CPointerComplexTypeTest, SwapFunctionMixedPointer)
{
    // Allocate
    sdv::pointer<SComplexStruct> ptr1 = sdv::make_ptr<SComplexStruct>(100);
    EXPECT_TRUE(ptr1);
    EXPECT_EQ(ptr1.size(), 100u);
    sdv::pointer<SComplexStruct, 200> ptr2;
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

TEST_F(CPointerComplexTypeTest, AccessFunctions)
{
    // Empty pointer
    sdv::pointer<SComplexStruct> ptr;
    EXPECT_FALSE(ptr);

    // Get
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_THROW(*ptr, sdv::XNullPointer);
    EXPECT_THROW(ptr->ssHello, sdv::XNullPointer);

    // Allocate
    ptr = sdv::make_ptr<SComplexStruct>(100);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 100u);

    // Get function
    EXPECT_NE(ptr.get(), nullptr);

    // Assign the value to the array
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
    {
        EXPECT_EQ(ptr.get()[uiIndex].ssHello, "Hello");
        EXPECT_EQ(ptr.get()[uiIndex].ui8, 8u);
        EXPECT_EQ(ptr.get()[uiIndex].ui32, 32u);
        EXPECT_EQ(ptr.get()[uiIndex].vecHello.size(), 4u);
        ptr.get()[uiIndex].ssHello = "Hi" + std::to_string(uiIndex);
        ptr.get()[uiIndex].ui8 = 9u + static_cast<uint8_t>(uiIndex);
        ptr.get()[uiIndex].ui32 = 33u + uiIndex;
        ptr.get()[uiIndex].vecHello = {std::string("Hi1") + std::to_string(uiIndex), std::string("Hi2") + std::to_string(uiIndex)};
    }

    // Reference functions (getter)
    EXPECT_EQ((*ptr).ssHello, "Hi0");
    EXPECT_EQ((*ptr).ui8, 9u);
    EXPECT_EQ((*ptr).ui32, 33u);
    EXPECT_EQ((*ptr).vecHello.size(), 2u);

    // Indexed values (getter)
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
    {
        EXPECT_EQ(ptr[uiIndex].ssHello, std::string("Hi") + std::to_string(uiIndex));
        EXPECT_EQ(ptr[uiIndex].ui8, 9u + static_cast<uint8_t>(uiIndex));
        EXPECT_EQ(ptr[uiIndex].ui32, 33u + uiIndex);
        EXPECT_EQ(ptr[uiIndex].vecHello.size(), 2u);
    }

    // Reference functions (setter)
    (*ptr).ssHello	= "Hello mio";
    (*ptr).ui8 = 255u;
    (*ptr).ui32 = 255u;
    (*ptr).vecHello = {"Hello mio1", "Hello mio2", "Hello mio3"};
    EXPECT_EQ(ptr.get()->ssHello, "Hello mio");
    EXPECT_EQ(ptr.get()->ui8, 255u);
    EXPECT_EQ(ptr.get()->ui32, 255u);
    EXPECT_EQ(ptr.get()->vecHello.size(), 3u);

    // Indexed values (setter)
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
    {
        ptr[uiIndex].ssHello= "Hi miomio" + std::to_string(uiIndex);
        ptr[uiIndex].ui8 = 109u + static_cast<uint8_t>(uiIndex);
        ptr[uiIndex].ui32 = 133u + uiIndex;
        ptr[uiIndex].vecHello = {std::string("Hi miomio1") + std::to_string(uiIndex),
                                 std::string("Hi miomio2") + std::to_string(uiIndex),
                                 std::string("Hi miomio3") + std::to_string(uiIndex),
                                 std::string("Hi miomio4") + std::to_string(uiIndex),
                                 std::string("Hi miomio5") + std::to_string(uiIndex)};
        EXPECT_EQ(ptr.get()[uiIndex].ssHello, std::string("Hi miomio") + std::to_string(uiIndex));
        EXPECT_EQ(ptr.get()[uiIndex].ui8, 109u + static_cast<uint8_t>(uiIndex));
        EXPECT_EQ(ptr.get()[uiIndex].ui32, 133u + uiIndex);
        EXPECT_EQ(ptr.get()[uiIndex].vecHello.size(), 5u);
    }

    // Resizing to 0 and then back to 100 should reset all elements to the default value.
    ptr.resize(0);
    EXPECT_TRUE(ptr);
    ptr.resize(100);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 100u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
    {
        EXPECT_EQ(ptr.get()[uiIndex].ssHello, "Hello");
        EXPECT_EQ(ptr.get()[uiIndex].ui8, 8u);
        EXPECT_EQ(ptr.get()[uiIndex].ui32, 32u);
        EXPECT_EQ(ptr.get()[uiIndex].vecHello.size(), 4u);
    }
}

TEST_F(CPointerComplexTypeTest, SizeFunctionsStaticPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct, 150> ptr;
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0u);
    EXPECT_EQ(ptr.capacity(), 150);
    EXPECT_NO_THROW(ptr.resize(100));
    EXPECT_EQ(ptr.size(), 100u);
    EXPECT_EQ(ptr.capacity(), 150);

    // Resize to many
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr[uiIndex].ui32 = uiIndex + 1000;
    EXPECT_THROW(ptr[101], sdv::XIndexOutOfRange);

    // Resize smaller
    ptr.resize(50);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 50u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex].ui32, uiIndex + 1000);
    EXPECT_THROW(ptr[51], sdv::XIndexOutOfRange);

    // Resize larger (the first 50 should still be valid)
    ptr.resize(150);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 150u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex].ui32, uiIndex + 1000);
    EXPECT_THROW(ptr[151], sdv::XIndexOutOfRange);

    // Exceed buffer capacity
    EXPECT_THROW(ptr.resize(151), sdv::XBufferTooSmall);
}

TEST_F(CPointerComplexTypeTest, SizeFunctionsDynamicPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.size(), 0u);
    EXPECT_NO_THROW(ptr.resize(100));
    EXPECT_EQ(ptr.size(), 100u);
    EXPECT_EQ(ptr.capacity(), 100);

    // Allocate zero
    ptr = sdv::make_ptr<SComplexStruct>(0);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 0u);
    EXPECT_EQ(ptr.capacity(), 0);

    // Resize to many
    ptr.resize(100);
    EXPECT_EQ(ptr.size(), 100u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 100; uiIndex++)
        ptr[uiIndex].ui32 = uiIndex + 1000;
    EXPECT_THROW(ptr[101], sdv::XIndexOutOfRange);

    // Resize smaller
    ptr.resize(50);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 50u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex].ui32, uiIndex + 1000);
    EXPECT_THROW(ptr[51], sdv::XIndexOutOfRange);

    // Resize larger (the first 50 should still be valid)
    ptr.resize(150);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.size(), 150u);
    for (uint32_t uiIndex = 0; ptr.get() && uiIndex < 50; uiIndex++)
        EXPECT_EQ(ptr[uiIndex].ui32, uiIndex + 1000);
    EXPECT_THROW(ptr[151], sdv::XIndexOutOfRange);

    // Copy the pointer an resize to giant; both pointers should still be valid.
    sdv::pointer<SComplexStruct> ptrGiant(ptr);
    EXPECT_EQ(ptr, ptrGiant);
    ptrGiant.resize(1024 * 8);
    EXPECT_EQ(ptr, ptrGiant);
}

TEST_F(CPointerComplexTypeTest, RefCountFunctionDynamicPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct> ptr;
    EXPECT_EQ(ptr.ref_count(), 0);

    // Allocate
    ptr = sdv::make_ptr<SComplexStruct>();
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.ref_count(), 1);

    // Copy
    sdv::pointer<SComplexStruct> ptrCopy(ptr);
    EXPECT_TRUE(ptrCopy);
    EXPECT_EQ(ptr.ref_count(), 2);
    EXPECT_EQ(ptrCopy.ref_count(), 2);
    sdv::pointer<SComplexStruct> ptrCopy2;
    EXPECT_FALSE(ptrCopy2);
    ptrCopy2 = ptrCopy;
    EXPECT_TRUE(ptrCopy2);
    EXPECT_EQ(ptr.ref_count(), 3);
    EXPECT_EQ(ptrCopy.ref_count(), 3);
    EXPECT_EQ(ptrCopy2.ref_count(), 3);

    // Move
    sdv::pointer<SComplexStruct> ptrMove(std::move(ptrCopy));
    EXPECT_TRUE(ptrMove);
    EXPECT_EQ(ptr.ref_count(), 3);
    EXPECT_EQ(ptrCopy.ref_count(), 0);
    EXPECT_EQ(ptrCopy2.ref_count(), 3);
    EXPECT_EQ(ptrMove.ref_count(), 3);
    sdv::pointer<SComplexStruct> ptrMove2;
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

TEST_F(CPointerComplexTypeTest, CompareOperatorStaticPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct, 10> ptrLeft;
    sdv::pointer<SComplexStruct, 15> ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Resize one pointer and fill with counter
    ptrLeft.resize(10);
    for (size_t nIndex = 0; nIndex < ptrLeft.size(); nIndex++)
        ptrLeft[nIndex].ui32 = static_cast<uint32_t>(nIndex);
    EXPECT_EQ(ptrLeft.size(), 10u);
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

TEST_F(CPointerComplexTypeTest, CompareOperatorDynamicPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct> ptrLeft, ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Allocate one pointer
    ptrLeft = sdv::make_ptr<SComplexStruct>(10);
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
    ptrLeft = sdv::make_ptr<SComplexStruct>(10);
    EXPECT_FALSE(ptrLeft == ptrRight);
    EXPECT_TRUE(ptrLeft != ptrRight);
}

TEST_F(CPointerComplexTypeTest, CompareOperatorMixedPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct, 10> ptrLeft;
    sdv::pointer<SComplexStruct> ptrRight;
    EXPECT_TRUE(ptrLeft == ptrRight);
    EXPECT_FALSE(ptrLeft != ptrRight);
    EXPECT_FALSE(ptrLeft < ptrRight);
    EXPECT_TRUE(ptrLeft <= ptrRight);
    EXPECT_FALSE(ptrLeft > ptrRight);
    EXPECT_TRUE(ptrLeft >= ptrRight);

    // Resize one pointer and fill with counter
    ptrLeft.resize(10);
    for (size_t nIndex = 0; nIndex < ptrLeft.size(); nIndex++)
        ptrLeft[nIndex].ui32 = static_cast<uint32_t>(nIndex);
    EXPECT_EQ(ptrLeft.size(), 10u);
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

TEST_F(CPointerComplexTypeTest, CompareNullOperatorDynamicPointer)
{
    // Empty pointer
    sdv::pointer<SComplexStruct> ptr;
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
    ptr = sdv::make_ptr<SComplexStruct>(10);
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
