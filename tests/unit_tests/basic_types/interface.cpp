#include <support/string.h>
#include <sstream>
#include "basic_types_test.h"

using CInterfaceTypeTest = CBasicTypesTest;

// Test class
class CTest : public sdv::IInterfaceAccess, public sdv::core::IMemoryAlloc
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    /**
    * @brief Allocate a memory block of the provided length. Overload of IMemoryAlloc::Allocate.
    * @param[in] uiLength The length of the memory block to allocate.
    * @return Smart pointer to the memory allocation or NULL when allocating was not possible.
    */
    virtual sdv::pointer<uint8_t> Allocate(/*in*/ uint32_t /*uiLength*/) override { return sdv::pointer<uint8_t>(); };

};

TEST_F(CInterfaceTypeTest, Constructor)
{
    sdv::interface_t ifc;
    EXPECT_FALSE(ifc);

    CTest test;
    sdv::interface_t ifc2(static_cast<sdv::IInterfaceAccess*>(&test));
    EXPECT_TRUE(ifc2);

    sdv::interface_t ifc3(ifc2);
    EXPECT_TRUE(ifc3);
    EXPECT_EQ(ifc2, ifc3);

    sdv::interface_t ifc4(std::move(ifc3));
    EXPECT_FALSE(ifc3);
    EXPECT_TRUE(ifc4);
    EXPECT_EQ(ifc2, ifc4);

    sdv::interface_t ifc5(nullptr);
    EXPECT_FALSE(ifc5);
}

TEST_F(CInterfaceTypeTest, AssignmentOperator)
{
    sdv::interface_t ifc, ifc2, ifc3;
    EXPECT_FALSE(ifc);
    EXPECT_FALSE(ifc2);
    EXPECT_FALSE(ifc3);

    CTest test;
    ifc = static_cast<sdv::IInterfaceAccess*>(&test);
    EXPECT_TRUE(ifc);

    ifc2 = ifc;
    EXPECT_TRUE(ifc);
    EXPECT_TRUE(ifc2);

    ifc3 = std::move(ifc2);
    EXPECT_TRUE(ifc3);
    EXPECT_FALSE(ifc2);

    ifc3 = nullptr;
    EXPECT_FALSE(ifc3);

    ifc = static_cast<sdv::core::IMemoryAlloc*>(&test);
    EXPECT_TRUE(ifc);
}

TEST_F(CInterfaceTypeTest, Reset)
{
    CTest test;
    sdv::interface_t ifc(static_cast<sdv::IInterfaceAccess*>(&test));
    EXPECT_TRUE(ifc);

    ifc.reset();
    EXPECT_FALSE(ifc);
}

TEST_F(CInterfaceTypeTest, ID)
{
    CTest test;
    sdv::interface_t ifc(static_cast<sdv::IInterfaceAccess*>(&test));
    EXPECT_TRUE(ifc);
    EXPECT_EQ(ifc.id(), sdv::GetInterfaceId<sdv::IInterfaceAccess>());

    ifc = static_cast<sdv::core::IMemoryAlloc*>(&test);
    EXPECT_EQ(ifc.id(), sdv::GetInterfaceId<sdv::core::IMemoryAlloc>());
}

TEST_F(CInterfaceTypeTest, Get)
{
    CTest test;
    sdv::interface_t ifc(static_cast<sdv::IInterfaceAccess*>(&test));
    EXPECT_TRUE(ifc);
    EXPECT_EQ(ifc.template get<sdv::IInterfaceAccess>(), static_cast<sdv::IInterfaceAccess*>(&test));
    EXPECT_EQ(ifc.template get<sdv::core::IMemoryAlloc>(), nullptr);

    ifc = static_cast<sdv::core::IMemoryAlloc*>(&test);
    EXPECT_EQ(ifc.template get<sdv::core::IMemoryAlloc>(), static_cast<sdv::core::IMemoryAlloc*>(&test));
    EXPECT_EQ(ifc.template get<sdv::IInterfaceAccess>(), nullptr);
}

TEST_F(CInterfaceTypeTest, Compare)
{
    CTest test;
    sdv::interface_t ifc(static_cast<sdv::IInterfaceAccess*>(&test));
    sdv::interface_t ifc2(static_cast<sdv::IInterfaceAccess*>(&test));
    sdv::interface_t ifc3(static_cast<sdv::core::IMemoryAlloc*>(&test));
    EXPECT_TRUE(ifc);
    EXPECT_TRUE(ifc2);
    EXPECT_TRUE(ifc3);

    EXPECT_EQ(ifc.compare(ifc2), 0);
    if (sdv::GetInterfaceId<sdv::IInterfaceAccess>() > sdv::GetInterfaceId<sdv::core::IMemoryAlloc>())
        EXPECT_GT(ifc.compare(ifc3), 0);
    else
        EXPECT_LT(ifc.compare(ifc3), 0);
}

TEST_F(CInterfaceTypeTest, CompareOperators)
{
    CTest test;
    sdv::interface_t ifc(static_cast<sdv::IInterfaceAccess*>(&test));
    sdv::interface_t ifc2(static_cast<sdv::IInterfaceAccess*>(&test));
    sdv::interface_t ifc3(static_cast<sdv::core::IMemoryAlloc*>(&test));
    sdv::interface_t ifc4;
    EXPECT_TRUE(ifc);
    EXPECT_TRUE(ifc2);
    EXPECT_TRUE(ifc3);

    EXPECT_EQ(ifc, ifc2);
    EXPECT_NE(ifc2, ifc3);
    if (sdv::GetInterfaceId<sdv::IInterfaceAccess>() > sdv::GetInterfaceId<sdv::core::IMemoryAlloc>())
    {
        EXPECT_GT(ifc2, ifc3);
        EXPECT_GE(ifc2, ifc3);
        EXPECT_LT(ifc3, ifc2);
        EXPECT_LE(ifc3, ifc2);
    } else
    {
        EXPECT_GT(ifc3, ifc2);
        EXPECT_GE(ifc3, ifc2);
        EXPECT_LT(ifc2, ifc3);
        EXPECT_LE(ifc2, ifc3);
    }

    EXPECT_NE(ifc2, nullptr);
    EXPECT_NE(nullptr, ifc2);
    EXPECT_GT(ifc2, nullptr);
    EXPECT_GE(ifc2, nullptr);
    EXPECT_FALSE(ifc2 < nullptr);
    EXPECT_FALSE(ifc2 <= nullptr);
    EXPECT_LT(nullptr, ifc2);
    EXPECT_LE(nullptr, ifc2);
    EXPECT_FALSE(nullptr > ifc2);
    EXPECT_FALSE(nullptr >= ifc2);
    EXPECT_EQ(nullptr, ifc4);
    EXPECT_EQ(ifc4, nullptr);
}
