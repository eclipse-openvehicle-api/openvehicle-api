/********************************************************************************
 * Copyright (c) 2025-2026 Contributors to the Eclipse Foundation
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
#include "../../../global/localmemmgr.h"
#include "../../../global/exec_dir_helper.h"
#include <support/param_impl.h>

class CEmptyParameters : public sdv::CSdvParamMap
{
public:
    CEmptyParameters()
    {}

    BEGIN_SDV_PARAM_MAP()
    END_SDV_PARAM_MAP()
};

TEST(ParameterTest, EmptyParamMap)
{
    // Global information
    auto vecParamMap = CEmptyParameters::GetParamMapInfoStatic();
    EXPECT_TRUE(vecParamMap.empty());

    // Local information
    CEmptyParameters param;
    auto seqParams = param.GetParamPaths();
    EXPECT_TRUE(seqParams.empty());
}

class CSimpleParameters : public sdv::CSdvParamMap
{
public:
    CSimpleParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    int             m_i;
    double          m_d;
    std::string     m_ss;
};

TEST(ParameterTest, SimpleParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CSimpleParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 3u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
}

TEST(ParameterTest, SimpleParamMapObjectInfo)
{
    // Instantiation
    CSimpleParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
}

TEST(ParameterTest, SimpleParamMapObjectInfoIndirect)
{
    // Instantiation
    CSimpleParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "int_unit");
    EXPECT_EQ(info.Description(), "My integer");
    EXPECT_EQ(info.DefaultVal(), 10);

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "double_unit");
    EXPECT_EQ(info.Description(), "My double");
    EXPECT_EQ(info.DefaultVal(), 1234.5);

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Unit(), "string_unit");
    EXPECT_EQ(info.Description(), "My string");
    EXPECT_EQ(info.DefaultVal(), "string_value");
}

TEST(ParameterTest, SimpleParamMapGetSet)
{
    // Instantiation
    CSimpleParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10);  // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], 20));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // New value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5);  // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 5432.1));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 5432.1); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value");  // Default value
    EXPECT_TRUE(param.SetParam(seqParams[2], "text_value"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "text_value"); // New value
}

class CRestrictedParameters : public sdv::CSdvParamMap
{
public:
    CRestrictedParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_NUMBER_ENTRY(m_i, "my_integer", 10, >= 0, <= 20, "int_unit", "My integer")
        SDV_PARAM_NUMBER_ENTRY(m_d, "my_double", 1234.5, NO_LIMIT, < 20000, "double_unit", "My double")
        SDV_PARAM_STRING_ENTRY(m_ss, "my_string", "0xabcd", "^0[xX][0-9a-fA-F]+$", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    int             m_i;
    double          m_d;
    std::string     m_ss;
};

TEST(ParameterTest, RestrictedParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CRestrictedParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 3u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_FALSE(ptrParamInfo->NumericLimitLow().first.empty());
    EXPECT_EQ(ptrParamInfo->NumericLimitLow().first, 0);
    EXPECT_TRUE(ptrParamInfo->NumericLimitLow().second);
    EXPECT_EQ(ptrParamInfo->NumericLimitHigh().first, 20);
    EXPECT_TRUE(ptrParamInfo->NumericLimitHigh().second);

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_TRUE(ptrParamInfo->NumericLimitLow().first.empty());
    EXPECT_EQ(ptrParamInfo->NumericLimitHigh().first, 20000);
    EXPECT_FALSE(ptrParamInfo->NumericLimitHigh().second);

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->StringPattern(), "^0[xX][0-9a-fA-F]+$");
}

TEST(ParameterTest, RestrictedParamMapObjectInfo)
{
    // Instantiation
    CRestrictedParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->Numeric());
    EXPECT_FALSE(ptrParamInfo->NumericLimitLow().first.empty());
    EXPECT_EQ(ptrParamInfo->NumericLimitLow().first, 0);
    EXPECT_TRUE(ptrParamInfo->NumericLimitLow().second);
    EXPECT_EQ(ptrParamInfo->NumericLimitHigh().first, 20);
    EXPECT_TRUE(ptrParamInfo->NumericLimitHigh().second);

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->Numeric());
    EXPECT_TRUE(ptrParamInfo->NumericLimitLow().first.empty());
    EXPECT_EQ(ptrParamInfo->NumericLimitHigh().first, 20000);
    EXPECT_FALSE(ptrParamInfo->NumericLimitHigh().second);

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    ASSERT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->StringPattern(), "^0[xX][0-9a-fA-F]+$");
}

TEST(ParameterTest, RestrictedParamMapObjectInfoIndirect)
{
    // Instantiation
    CRestrictedParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    ASSERT_TRUE(info.Numeric());
    EXPECT_FALSE(info.NumericLimitLow().first.empty());
    EXPECT_EQ(info.NumericLimitLow().first, 0);
    EXPECT_TRUE(info.NumericLimitLow().second);
    EXPECT_EQ(info.NumericLimitHigh().first, 20);
    EXPECT_TRUE(info.NumericLimitHigh().second);

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_TRUE(info.NumericLimitLow().first.empty());
    EXPECT_EQ(info.NumericLimitHigh().first, 20000);
    EXPECT_FALSE(info.NumericLimitHigh().second);

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.StringPattern(), "^0[xX][0-9a-fA-F]+$");
}

TEST(ParameterTest, RestrictedParamMapGetSet)
{
    // Instantiation
    CRestrictedParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], 20)); // In limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // New value
    EXPECT_FALSE(param.SetParam(seqParams[0], 21)); // Over limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // Same value
    EXPECT_TRUE(param.SetParam(seqParams[0], 0)); // In limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 0); // New value
    EXPECT_FALSE(param.SetParam(seqParams[0], -1)); // Over limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 0); // Same value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 19999.99)); // In limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 19999.99); // New value
    EXPECT_FALSE(param.SetParam(seqParams[1], 20000)); // Over limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 19999.99); // Same value
    EXPECT_TRUE(param.SetParam(seqParams[1], -20000)); // No limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, -20000); // Same value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "0xabcd"); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[2], "0x1234567890abcdef"));    // Hex value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "0x1234567890abcdef"); // New value
    EXPECT_TRUE(param.SetParam(seqParams[2], "0XFEDCBA987654321"));    // Hex value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "0XFEDCBA987654321"); // New value
    EXPECT_FALSE(param.SetParam(seqParams[2], "0xabcdfg"));    // Invalid value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "0XFEDCBA987654321"); // Same value
    EXPECT_FALSE(param.SetParam(seqParams[2], "100"));    // Invalid value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "0XFEDCBA987654321"); // Same value
}

class CEnumParameters : public sdv::CSdvParamMap
{
public:
    // C-style enum
    enum ECStyle
    {
        one,
        two,
        three
    };

    BEGIN_SDV_LABEL_MAP(ECStyle)
        SDV_LABEL_ENTRY(ECStyle::one, "one")
        SDV_LABEL_ENTRY(ECStyle::two, "two")
        SDV_LABEL_ENTRY(ECStyle::three, "three")
    END_SDV_LABEL_MAP()

    // C++-style enum
    enum class ECppStyle : uint8_t
    {
        four,
        five,
        six
    };

    BEGIN_SDV_LABEL_MAP(ECppStyle)
        SDV_LABEL_ENTRY(ECppStyle::four, "four")
        SDV_LABEL_ENTRY(ECppStyle::five, "five")
        SDV_LABEL_ENTRY(ECppStyle::six, "six")
    END_SDV_LABEL_MAP()

    CEnumParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENUM_ENTRY(m_eCStyle, "C enum", ECStyle::one, "This is a C style enum variable")
        SDV_PARAM_ENUM_ENTRY(m_eCppStyle, "C++ enum", ECppStyle::four, "This is a C++ style enum variable")
    END_SDV_PARAM_MAP()

private:
    ECStyle     m_eCStyle;
    ECppStyle   m_eCppStyle;
};

TEST(ParameterTest, EnumParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CEnumParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 2u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Enum());
    EXPECT_EQ(ptrParamInfo->Name(), "C enum");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C style enum variable");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), CEnumParameters::ECStyle::one);

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Enum());
    EXPECT_EQ(ptrParamInfo->Name(), "C++ enum");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C++ style enum variable");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), CEnumParameters::ECppStyle::four);
}

TEST(ParameterTest, EnumParamMapObjectInfo)
{
    // Instantiation
    CEnumParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C enum");
    EXPECT_EQ(seqParams[1], "C++ enum");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Enum());
    EXPECT_EQ(ptrParamInfo->Name(), "C enum");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C style enum variable");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), CEnumParameters::ECStyle::one);

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Enum());
    EXPECT_EQ(ptrParamInfo->Name(), "C++ enum");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C++ style enum variable");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), CEnumParameters::ECppStyle::four);
}

TEST(ParameterTest, EnumParamMapObjectInfoIndirect)
{
    // Instantiation
    CEnumParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C enum");
    EXPECT_EQ(seqParams[1], "C++ enum");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Enum());
    EXPECT_EQ(info.Name(), "C enum");
    EXPECT_EQ(info.Description(), "This is a C style enum variable");
    EXPECT_EQ(info.DefaultVal(), CEnumParameters::ECStyle::one);

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Enum());
    EXPECT_EQ(info.Name(), "C++ enum");
    EXPECT_EQ(info.Description(), "This is a C++ style enum variable");
    EXPECT_EQ(info.DefaultVal(), CEnumParameters::ECppStyle::four);
}

TEST(ParameterTest, EnumParamMapGetSet)
{
    // Instantiation
    CEnumParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C enum");
    EXPECT_EQ(seqParams[1], "C++ enum");

    using ECStyle = CEnumParameters::ECStyle;
    using ECppStyle = CEnumParameters::ECppStyle;

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, ECStyle::one);                           // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], ECStyle::three)); // In limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, ECStyle::three);                          // New value
    EXPECT_FALSE(param.SetParam(seqParams[0], 4));              // Over limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, ECStyle::three);                          // Same value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, ECppStyle::four);                         // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], sdv::any_t(ECppStyle::six)));  // In limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, ECppStyle::six);                          // New value
    EXPECT_FALSE(param.SetParam(seqParams[1], 10));             // Over limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, ECppStyle::six);                          // Same value
}

class CBitmaskParameters : public sdv::CSdvParamMap
{
public:
    // C-style enum
    enum ECStyle
    {
        bit0 = 1,
        bit1 = 2,
        bit2 = 4
    };

    BEGIN_SDV_LABEL_MAP(ECStyle)
        SDV_LABEL_ENTRY(ECStyle::bit0, "bit0")
        SDV_LABEL_ENTRY(ECStyle::bit1, "bit1")
        SDV_LABEL_ENTRY(ECStyle::bit2, "bit2")
    END_SDV_LABEL_MAP()

    // C++-style enum
    enum class ECppStyle : uint8_t
    {
        bit5 = 8,
        bit6 = 16,
        bit7 = 32
    };

    BEGIN_SDV_LABEL_MAP(ECppStyle)
        SDV_LABEL_ENTRY(ECppStyle::bit5, "bit5")
        SDV_LABEL_ENTRY(ECppStyle::bit6, "bit6")
        SDV_LABEL_ENTRY(ECppStyle::bit7, "bit7")
    END_SDV_LABEL_MAP()

    CBitmaskParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_BITMASK_ENTRY(ECStyle, m_uiCStyle, "C bitmask", 1, "This is a C style enum bitmask")
        SDV_PARAM_BITMASK_ENTRY(ECppStyle, m_uiCppStyle, "C++ bitmask", 8, "This is a C++ style enum bitmask")
    END_SDV_PARAM_MAP()

private:
    uint64_t    m_uiCStyle;
    uint8_t     m_uiCppStyle;
};

TEST(ParameterTest, BitmaskParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CBitmaskParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 2u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Bitmask());
    EXPECT_EQ(ptrParamInfo->Name(), "C bitmask");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C style enum bitmask");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1);

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Bitmask());
    EXPECT_EQ(ptrParamInfo->Name(), "C++ bitmask");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C++ style enum bitmask");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 8);
}

TEST(ParameterTest, BitmaskParamMapObjectInfo)
{
    // Instantiation
    CBitmaskParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C bitmask");
    EXPECT_EQ(seqParams[1], "C++ bitmask");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Bitmask());
    EXPECT_EQ(ptrParamInfo->Name(), "C bitmask");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C style enum bitmask");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1);

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Bitmask());
    EXPECT_EQ(ptrParamInfo->Name(), "C++ bitmask");
    EXPECT_EQ(ptrParamInfo->Description(), "This is a C++ style enum bitmask");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 8);
}

TEST(ParameterTest, BitmaskParamMapObjectInfoIndirect)
{
    // Instantiation
    CBitmaskParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C bitmask");
    EXPECT_EQ(seqParams[1], "C++ bitmask");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Bitmask());
    EXPECT_EQ(info.Name(), "C bitmask");
    EXPECT_EQ(info.Description(), "This is a C style enum bitmask");
    EXPECT_EQ(info.DefaultVal(), 1);

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Bitmask());
    EXPECT_EQ(info.Name(), "C++ bitmask");
    EXPECT_EQ(info.Description(), "This is a C++ style enum bitmask");
    EXPECT_EQ(info.DefaultVal(), 8);
}

TEST(ParameterTest, BitmaskParamMapGetSet)
{
    // Instantiation
    CBitmaskParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "C bitmask");
    EXPECT_EQ(seqParams[1], "C++ bitmask");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 1);                           // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], 5));   // In limit
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 5);                           // New value
    EXPECT_FALSE(param.SetParam(seqParams[0], 8));  // Bits not specified
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 5);                           // Same value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 8);                           // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 40));  // In limit
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 40);                          // New value
    EXPECT_FALSE(param.SetParam(seqParams[1], 42)); // Bits not specified
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 40);                          // Same value
}

class CPathParameters : public sdv::CSdvParamMap
{
public:
    CPathParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_pathOne, "1st path", std::filesystem::u8path("C:\\dir1\abc.def"), "", "The first path")
        SDV_PARAM_ENTRY(m_pathTwo, "2nd path", GetExecDirectory() / GetExecFilename(), "", "The second path")
    END_SDV_PARAM_MAP()

private:
    std::filesystem::path    m_pathOne;
    std::filesystem::path    m_pathTwo;
};

TEST(ParameterTest, PathParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CPathParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 2u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "1st path");
    EXPECT_EQ(ptrParamInfo->Description(), "The first path");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), std::filesystem::path("C:\\dir1\abc.def").generic_u8string());

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "2nd path");
    EXPECT_EQ(ptrParamInfo->Description(), "The second path");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), GetExecDirectory() / GetExecFilename());
}

TEST(ParameterTest, PathParamMapObjectInfo)
{
    // Instantiation
    CPathParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "1st path");
    EXPECT_EQ(seqParams[1], "2nd path");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "1st path");
    EXPECT_EQ(ptrParamInfo->Description(), "The first path");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), std::filesystem::path("C:\\dir1\abc.def").generic_u8string());

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "2nd path");
    EXPECT_EQ(ptrParamInfo->Description(), "The second path");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), GetExecDirectory() / GetExecFilename());
}

TEST(ParameterTest, PathParamMapObjectInfoIndirect)
{
    // Instantiation
    CPathParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "1st path");
    EXPECT_EQ(seqParams[1], "2nd path");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Name(), "1st path");
    EXPECT_EQ(info.Description(), "The first path");
    EXPECT_EQ(info.DefaultVal(), std::filesystem::path("C:\\dir1\abc.def").generic_u8string());

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Name(), "2nd path");
    EXPECT_EQ(info.Description(), "The second path");
    EXPECT_EQ(info.DefaultVal(), GetExecDirectory() / GetExecFilename());
}

TEST(ParameterTest, PathParamMapGetSet)
{
    // Instantiation
    CPathParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 2u);
    EXPECT_EQ(seqParams[0], "1st path");
    EXPECT_EQ(seqParams[1], "2nd path");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, std::filesystem::path("C:\\dir1\abc.def").generic_u8string()); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], "xyz/rtw/pop.lol"));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, std::filesystem::path("xyz/rtw/pop.lol").generic_u8string()); // New value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, GetExecDirectory() / GetExecFilename()); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], GetExecDirectory()));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, GetExecDirectory());                          // New value
}

class CReadOnlyParameters : public sdv::CSdvParamMap
{
public:
    CReadOnlyParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_SET_READONLY()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_RESET_READONLY()
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
        SDV_PARAM_SET_READONLY()
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
        SDV_PARAM_RESET_ATTRIBUTES()
        SDV_PARAM_ENTRY(m_b, "my_boolean", true, "no_unit", "My boolean")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
    std::string m_ss;
    bool m_b;
};

TEST(ParameterTest, ReadOnlyParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CReadOnlyParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 4u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);
    ASSERT_TRUE(vecParamMap[3]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_TRUE(ptrParamInfo->ReadOnly());

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->ReadOnly());

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->ReadOnly());

    // Get parameter #3 global information
    ptrParamInfo = vecParamMap[3];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Name(), "my_boolean");
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->ReadOnly());
}

TEST(ParameterTest, ReadOnlyParamMapObjectInfo)
{
    // Instantiation
    CReadOnlyParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_TRUE(ptrParamInfo->ReadOnly());

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->ReadOnly());

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->ReadOnly());

    // Get parameter #3 global information
    ptrParamInfo = param.FindParamObject(seqParams[3]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->ReadOnly());
}

TEST(ParameterTest, ReadOnlyParamMapObjectInfoIndirect)
{
    // Instantiation
    CReadOnlyParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "int_unit");
    EXPECT_EQ(info.Description(), "My integer");
    EXPECT_EQ(info.DefaultVal(), 10);
    EXPECT_TRUE(info.ReadOnly());

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "double_unit");
    EXPECT_EQ(info.Description(), "My double");
    EXPECT_EQ(info.DefaultVal(), 1234.5);
    EXPECT_FALSE(info.ReadOnly());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Unit(), "string_unit");
    EXPECT_EQ(info.Description(), "My string");
    EXPECT_EQ(info.DefaultVal(), "string_value");
    EXPECT_TRUE(info.ReadOnly());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[3]);
    EXPECT_TRUE(info.Boolean());
    EXPECT_EQ(info.Unit(), "no_unit");
    EXPECT_EQ(info.Description(), "My boolean");
    EXPECT_EQ(info.DefaultVal(), true);
    EXPECT_FALSE(info.ReadOnly());
}

TEST(ParameterTest, ReadOnlyParamMapGetSet)
{
    // Instantiation
    CReadOnlyParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Default value
    EXPECT_FALSE(param.SetParam(seqParams[0], 20));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Same value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 5432.1));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 5432.1); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value"); // Default value
    EXPECT_FALSE(param.SetParam(seqParams[2], "text_value"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value"); // Same value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, true); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[3], false));
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, false); // New value
}

class CTemporaryParameters : public sdv::CSdvParamMap
{
public:
    CTemporaryParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_SET_TEMPORARY()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_RESET_TEMPORARY()
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
        SDV_PARAM_SET_TEMPORARY()
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
        SDV_PARAM_RESET_ATTRIBUTES()
        SDV_PARAM_ENTRY(m_b, "my_boolean", true, "no_unit", "My boolean")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
    std::string m_ss;
    bool m_b;
};

TEST(ParameterTest, TemporaryParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CTemporaryParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 4u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);
    ASSERT_TRUE(vecParamMap[3]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_TRUE(ptrParamInfo->Temporary());

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->Temporary());

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->Temporary());

    // Get parameter #3 global information
    ptrParamInfo = vecParamMap[3];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Name(), "my_boolean");
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->Temporary());
}

TEST(ParameterTest, TemporaryParamMapObjectInfo)
{
    // Instantiation
    CTemporaryParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_TRUE(ptrParamInfo->Temporary());

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->Temporary());

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->Temporary());

    // Get parameter #3 global information
    ptrParamInfo = param.FindParamObject(seqParams[3]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->Temporary());
}

TEST(ParameterTest, TemporaryParamMapObjectInfoIndirect)
{
    // Instantiation
    CTemporaryParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "int_unit");
    EXPECT_EQ(info.Description(), "My integer");
    EXPECT_EQ(info.DefaultVal(), 10);
    EXPECT_TRUE(info.Temporary());

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "double_unit");
    EXPECT_EQ(info.Description(), "My double");
    EXPECT_EQ(info.DefaultVal(), 1234.5);
    EXPECT_FALSE(info.Temporary());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Unit(), "string_unit");
    EXPECT_EQ(info.Description(), "My string");
    EXPECT_EQ(info.DefaultVal(), "string_value");
    EXPECT_TRUE(info.Temporary());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[3]);
    EXPECT_TRUE(info.Boolean());
    EXPECT_EQ(info.Unit(), "no_unit");
    EXPECT_EQ(info.Description(), "My boolean");
    EXPECT_EQ(info.DefaultVal(), true);
    EXPECT_FALSE(info.Temporary());
}

TEST(ParameterTest, TemporaryParamMapGetSet)
{
    // Instantiation
    CTemporaryParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], 20));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // New value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 5432.1));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 5432.1); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value"); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[2], "text_value"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "text_value"); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, true); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[3], false));
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, false); // New value
}

class CLockedParameters : public sdv::CSdvParamMap
{
public:
    CLockedParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_DISABLE_LOCKING()
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
        SDV_PARAM_RESET_ATTRIBUTES()
        SDV_PARAM_ENTRY(m_b, "my_boolean", true, "no_unit", "My boolean")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
    std::string m_ss;
    bool m_b;
};

TEST(ParameterTest, LockedParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CLockedParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 4u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);
    ASSERT_TRUE(vecParamMap[3]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_FALSE(ptrParamInfo->Locked());

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->Locked());

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_FALSE(ptrParamInfo->Locked());

    // Get parameter #3 global information
    ptrParamInfo = vecParamMap[3];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Name(), "my_boolean");
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->Locked());
}

TEST(ParameterTest, LockedParamMapObjectInfo)
{
    // Instantiation
    CLockedParameters param;

    // Lock the parameter map
    param.LockParamMap();

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_TRUE(ptrParamInfo->Locked());

    // Get parameter #1 global information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_FALSE(ptrParamInfo->Locked());

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->Locked());

    // Get parameter #3 global information
    ptrParamInfo = param.FindParamObject(seqParams[3]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Boolean());
    EXPECT_EQ(ptrParamInfo->Unit(), "no_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My boolean");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), true);
    EXPECT_FALSE(ptrParamInfo->Locked());
}

TEST(ParameterTest, LockedParamMapObjectInfoIndirect)
{
    // Instantiation
    CLockedParameters param;

    // Lock the parameter map
    param.LockParamMap();

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get parameter #0 information
    sdv::CSdvParamInfo info(param.GetParamInfo(seqParams[0]));
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "int_unit");
    EXPECT_EQ(info.Description(), "My integer");
    EXPECT_EQ(info.DefaultVal(), 10);
    EXPECT_TRUE(info.Locked());

    // Get parameter #1 global information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "double_unit");
    EXPECT_EQ(info.Description(), "My double");
    EXPECT_EQ(info.DefaultVal(), 1234.5);
    EXPECT_FALSE(info.Locked());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Unit(), "string_unit");
    EXPECT_EQ(info.Description(), "My string");
    EXPECT_EQ(info.DefaultVal(), "string_value");
    EXPECT_TRUE(info.Locked());

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[3]);
    EXPECT_TRUE(info.Boolean());
    EXPECT_EQ(info.Unit(), "no_unit");
    EXPECT_EQ(info.Description(), "My boolean");
    EXPECT_EQ(info.DefaultVal(), true);
    EXPECT_FALSE(info.Locked());
}

TEST(ParameterTest, LockedParamMapGetSet)
{
    // Instantiation
    CLockedParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 4u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");
    EXPECT_EQ(seqParams[3], "my_boolean");

    // Get/set parameter #0 value
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[0], 20));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // New value

    // Get/set parameter #1 value
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[1], 5432.1));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 5432.1); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value"); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[2], "text_value"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "text_value"); // New value

    // Get/set parameter #2 value
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, true); // Default value
    EXPECT_TRUE(param.SetParam(seqParams[3], false));
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, false); // New value

    // Lock the parameter map
    param.LockParamMap();

    // Get/set parameter #0 value
    EXPECT_FALSE(param.SetParam(seqParams[0], 30));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // Same value

    // Get/set parameter #1 value
    EXPECT_TRUE(param.SetParam(seqParams[1], 678.9));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 678.9); // New value

    // Get/set parameter #2 value
    EXPECT_FALSE(param.SetParam(seqParams[2], "book"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "text_value"); // Same value

    // Get/set parameter #2 value
    EXPECT_TRUE(param.SetParam(seqParams[3], true));
    anyVal = param.GetParam(seqParams[3]);
    EXPECT_EQ(anyVal, true); // New value
}

TEST(ParameterTest, DirtyParamMapGetSet)
{
    // Instantiation
    CSimpleParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

    // Get/set parameter #0 value
    auto ptrParamObj = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamObj);
    auto anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 10); // Default value
    EXPECT_FALSE(ptrParamObj->Dirty());
    EXPECT_TRUE(param.SetParam(seqParams[0], 20));
    anyVal = param.GetParam(seqParams[0]);
    EXPECT_EQ(anyVal, 20); // New value
    EXPECT_TRUE(ptrParamObj->Dirty());
    sdv::CSdvParamInfo info = param.GetParamInfo(seqParams[0]);
    EXPECT_TRUE(info.Dirty());

    // Get/set parameter #1 value
    ptrParamObj = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamObj);
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 1234.5); // Default value
    EXPECT_FALSE(ptrParamObj->Dirty());
    EXPECT_TRUE(param.SetParam(seqParams[1], 5432.1));
    anyVal = param.GetParam(seqParams[1]);
    EXPECT_EQ(anyVal, 5432.1); // New value
    EXPECT_TRUE(ptrParamObj->Dirty());
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Dirty());

    // Get/set parameter #2 value
    ptrParamObj = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamObj);
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "string_value"); // Default value
    EXPECT_FALSE(ptrParamObj->Dirty());
    EXPECT_TRUE(param.SetParam(seqParams[2], "text_value"));
    anyVal = param.GetParam(seqParams[2]);
    EXPECT_EQ(anyVal, "text_value"); // New value
    EXPECT_TRUE(ptrParamObj->Dirty());
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.Dirty());

    // Reset dirty flag
    EXPECT_TRUE(param.IsParamDirty(seqParams[0]));
    EXPECT_TRUE(param.IsParamDirty(seqParams[1]));
    EXPECT_TRUE(param.IsParamDirty(seqParams[2]));
    param.ResetParamMapDirtyFlags();
    EXPECT_FALSE(param.IsParamDirty(seqParams[0]));
    EXPECT_FALSE(param.IsParamDirty(seqParams[1]));
    EXPECT_FALSE(param.IsParamDirty(seqParams[2]));
}

class CGroupParameters : public sdv::CSdvParamMap
{
public:
    CGroupParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_GROUP("Number")
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_GROUP("Number.Floating")
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
        SDV_PARAM_NO_GROUP()
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
    std::string m_ss;
};

TEST(ParameterTest, GroupParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CGroupParameters::GetParamMapInfoStatic();
    ASSERT_EQ(vecParamMap.size(), 3u);
    ASSERT_TRUE(vecParamMap[0]);
    ASSERT_TRUE(vecParamMap[1]);
    ASSERT_TRUE(vecParamMap[2]);

    // Get parameter #0 global information
    auto ptrParamInfo = vecParamMap[0];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_integer");
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_EQ(ptrParamInfo->Group(), "Number");

    // Get parameter #1 global information
    ptrParamInfo = vecParamMap[1];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Name(), "my_double");
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_EQ(ptrParamInfo->Group(), "Number.Floating");

    // Get parameter #2 global information
    ptrParamInfo = vecParamMap[2];
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Name(), "my_string");
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->Group().empty());
}

TEST(ParameterTest, GroupParamMapObjectInfo)
{
    // Instantiation
    CGroupParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_string");
    EXPECT_EQ(seqParams[1], "Number.my_integer");
    EXPECT_EQ(seqParams[2], "Number.Floating.my_double");

    // Get parameter #0 global information
    auto ptrParamInfo = param.FindParamObject(seqParams[0]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->String());
    EXPECT_EQ(ptrParamInfo->Unit(), "string_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My string");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), "string_value");
    EXPECT_TRUE(ptrParamInfo->Group().empty());

    // Get parameter #1 information
    ptrParamInfo = param.FindParamObject(seqParams[1]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "int_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My integer");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 10);
    EXPECT_EQ(ptrParamInfo->Group(), "Number");

    // Get parameter #2 global information
    ptrParamInfo = param.FindParamObject(seqParams[2]);
    ASSERT_TRUE(ptrParamInfo);
    EXPECT_TRUE(ptrParamInfo->Numeric());
    EXPECT_EQ(ptrParamInfo->Unit(), "double_unit");
    EXPECT_EQ(ptrParamInfo->Description(), "My double");
    EXPECT_EQ(ptrParamInfo->DefaultVal(), 1234.5);
    EXPECT_EQ(ptrParamInfo->Group(), "Number.Floating");
}

TEST(ParameterTest, GroupParamMapObjectInfoIndirect)
{
    // Instantiation
    CGroupParameters param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_string");
    EXPECT_EQ(seqParams[1], "Number.my_integer");
    EXPECT_EQ(seqParams[2], "Number.Floating.my_double");

    // Get parameter #0 global information
    sdv::CSdvParamInfo info = param.GetParamInfo(seqParams[0]);
    EXPECT_TRUE(info.String());
    EXPECT_EQ(info.Unit(), "string_unit");
    EXPECT_EQ(info.Description(), "My string");
    EXPECT_EQ(info.DefaultVal(), "string_value");
    EXPECT_TRUE(info.Group().empty());

    // Get parameter #1 information
    info = param.GetParamInfo(seqParams[1]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "int_unit");
    EXPECT_EQ(info.Description(), "My integer");
    EXPECT_EQ(info.DefaultVal(), 10);
    EXPECT_EQ(info.Group(), "Number");

    // Get parameter #2 global information
    info = param.GetParamInfo(seqParams[2]);
    EXPECT_TRUE(info.Numeric());
    EXPECT_EQ(info.Unit(), "double_unit");
    EXPECT_EQ(info.Description(), "My double");
    EXPECT_EQ(info.DefaultVal(), 1234.5);
    EXPECT_EQ(info.Group(), "Number.Floating");
}
