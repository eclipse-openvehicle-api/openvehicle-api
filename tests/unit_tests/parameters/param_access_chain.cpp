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
#include <support/param_impl.h>

class CSimpleParametersBase : public sdv::CSdvParamMap
{
public:
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
};

class CSimpleParameters : public CSimpleParametersBase
{
public:
    CSimpleParameters()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_CHAIN_BASE(CSimpleParametersBase)
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    std::string m_ss;
};

TEST(ParameterTest, ChainBaseSimpleParamMapStaticInfo)
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

TEST(ParameterTest, ChainBaseSimpleParamMapObjectInfo)
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

TEST(ParameterTest, ChainBaseSimpleParamMapObjectInfoIndirect)
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

TEST(ParameterTest, ChainBaseSimpleParamMapGetSet)
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
}

class CSimpleParametersVirtualBase : virtual public sdv::CSdvParamMap
{
public:
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
};

class CSimpleParametersVirtual : virtual public sdv::CSdvParamMap, public CSimpleParametersVirtualBase
{
public:
    CSimpleParametersVirtual()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_CHAIN_BASE(CSimpleParametersVirtualBase)
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    std::string m_ss;
};

TEST(ParameterTest, VirtualChainBaseSimpleParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CSimpleParametersVirtual::GetParamMapInfoStatic();
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

TEST(ParameterTest, VirtualChainBaseSimpleParamMapObjectInfo)
{
    // Instantiation
    CSimpleParametersVirtual param;

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

TEST(ParameterTest, VirtualChainBaseSimpleParamMapObjectInfoIndirect)
{
    // Instantiation
    CSimpleParametersVirtual param;

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

TEST(ParameterTest, VirtualChainBaseSimpleParamMapGetSet)
{
    // Instantiation
    CSimpleParametersVirtual param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

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
}

class CSimpleParametersMember : public sdv::CSdvParamMap
{
public:
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_i, "my_integer", 10, "int_unit", "My integer")
        SDV_PARAM_ENTRY(m_d, "my_double", 1234.5, "double_unit", "My double")
    END_SDV_PARAM_MAP()

private:
    int m_i;
    double m_d;
};

class CSimpleParametersWithMember : public sdv::CSdvParamMap
{
public:
    CSimpleParametersWithMember()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_CHAIN_MEMBER(m_member)
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    std::string m_ss;
    CSimpleParametersMember m_member;
};

TEST(ParameterTest, MemberChainBaseSimpleParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CSimpleParametersWithMember::GetParamMapInfoStatic();
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

TEST(ParameterTest, MemberChainBaseSimpleParamMapObjectInfo)
{
    // Instantiation
    CSimpleParametersWithMember param;

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

TEST(ParameterTest, MemberChainBaseSimpleParamMapObjectInfoIndirect)
{
    // Instantiation
    CSimpleParametersWithMember param;

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

TEST(ParameterTest, MemberChainBaseSimpleParamMapGetSet)
{
    // Instantiation
    CSimpleParametersWithMember param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

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
}

class CSimpleParametersWithMemberPointer : public sdv::CSdvParamMap
{
public:
    CSimpleParametersWithMemberPointer()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_CHAIN_MEMBER(m_pMember)
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    std::string m_ss;
    CSimpleParametersMember m_member;
    CSimpleParametersMember* m_pMember = &m_member;
};

TEST(ParameterTest, MemberPointerChainBaseSimpleParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CSimpleParametersWithMemberPointer::GetParamMapInfoStatic();
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

TEST(ParameterTest, MemberPointerChainBaseSimpleParamMapObjectInfo)
{
    // Instantiation
    CSimpleParametersWithMemberPointer param;

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

TEST(ParameterTest, MemberPointerChainBaseSimpleParamMapObjectInfoIndirect)
{
    // Instantiation
    CSimpleParametersWithMemberPointer param;

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

TEST(ParameterTest, MemberPointerChainBaseSimpleParamMapGetSet)
{
    // Instantiation
    CSimpleParametersWithMemberPointer param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

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
}

class CSimpleParametersWithMemberSmartPointer : public sdv::CSdvParamMap
{
public:
    CSimpleParametersWithMemberSmartPointer()
    {
        InitParamMap();
    }

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_CHAIN_MEMBER(m_ptrMember)
        SDV_PARAM_ENTRY(m_ss, "my_string", "string_value", "string_unit", "My string")
    END_SDV_PARAM_MAP()

private:
    std::string m_ss;
    std::shared_ptr<CSimpleParametersMember> m_ptrMember = std::make_shared<CSimpleParametersMember>();
};

TEST(ParameterTest, MemberSmartPointerChainBaseSimpleParamMapStaticInfo)
{
    // Get parameter names
    auto vecParamMap = CSimpleParametersWithMemberSmartPointer::GetParamMapInfoStatic();
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

TEST(ParameterTest, MemberSmartPointerChainBaseSimpleParamMapObjectInfo)
{
    // Instantiation
    CSimpleParametersWithMemberSmartPointer param;

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

TEST(ParameterTest, MemberSmartPointerChainBaseSimpleParamMapObjectInfoIndirect)
{
    // Instantiation
    CSimpleParametersWithMemberSmartPointer param;

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

TEST(ParameterTest, MemberSmartPointerChainBaseSimpleParamMapGetSet)
{
    // Instantiation
    CSimpleParametersWithMemberSmartPointer param;

    // Get parameter names
    auto seqParams = param.GetParamPaths();
    ASSERT_EQ(seqParams.size(), 3u);
    EXPECT_EQ(seqParams[0], "my_integer");
    EXPECT_EQ(seqParams[1], "my_double");
    EXPECT_EQ(seqParams[2], "my_string");

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
}
