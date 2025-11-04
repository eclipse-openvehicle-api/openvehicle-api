#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/enum_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.h"

using CParserEnumTest = CParserTest;

TEST_F(CParserEnumTest, ForwardDeclaration)
{
    // enum <enum_identifier>; --> forward declaration
    EXPECT_TRUE(CParser("enum E1; enum E1 {};").Parse().Root()->Find("E1"));
    EXPECT_TRUE(CParser("enum E1 {}; enum E1;").Parse().Root()->Find("E1"));
    EXPECT_TRUE(CParser("enum E1; enum E1; enum E1; enum E1;").Parse().Root()->Find("E1"));
    EXPECT_TRUE(CParser("enum E1 {}; enum E1; enum E1; ").Parse().Root()->Find("E1"));
    EXPECT_THROW(CParser("enum E1 {}; enum E1; enum E1 {}; ").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, Definition)
{
    // enum <enum_identifier> {...}; --> enum definition
    EXPECT_TRUE(CParser("enum E2 {};").Parse().Root()->Find("E2"));
    EXPECT_THROW(CParser("enum E2 {} e2;").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, AnonymousDefinition)
{
    // enum <enum_identifier> {...}; --> enum definition
    EXPECT_TRUE(CParser("struct S { enum {} e2; };").Parse().Root()->Find("S::e2"));
    // Anonymous definitions at root level are not allowed.
    EXPECT_THROW(CParser("typedef enum {} TE2;").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, SpecificEntryValueAssignment)
{
    EXPECT_TRUE(CParser("enum E2 { test = 10 };").Parse().Root()->Find("E2"));
    EXPECT_TRUE(CParser("enum E2 { test = 10 };").Parse().Root()->Find("E2::test"));
    EXPECT_TRUE(CParser("enum E2 { test = 10, };").Parse().Root()->Find("E2"));
    EXPECT_TRUE(CParser("enum E2 { test = 10, };").Parse().Root()->Find("E2::test"));
    EXPECT_THROW(CParser("enum E2 { test1 = 10, test1 = 20};").Parse(), CCompileException);
    EXPECT_THROW(CParser("enum E2 { test1 = 10, test1 = 10};").Parse(), CCompileException);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 20 };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 20 };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 20);
    EXPECT_EQ(CParser("enum E2 { test1 = 0xa, test2 = 0xf};").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_EQ(CParser("enum E2 { test1 = 0xa, test2 = 0xf};").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 15);
    EXPECT_EQ(CParser("enum E2 { test1 = 20, test2 = 10 };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 20);
    EXPECT_EQ(CParser("enum E2 { test1 = 20, test2 = 10 };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_THROW(CParser("enum E2 { test1 = 10, test2 = 10 };").Parse(), CCompileException);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 2, test3 = 5, test4 = 1, };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 2, test3 = 5, test4 = 1, };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 2, test3 = 5, test4 = 1, };").Parse().Root()->Find("E2::test3")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 5);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 2, test3 = 5, test4 = 1, };").Parse().Root()->Find("E2::test4")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 1);
}

TEST_F(CParserEnumTest, AutomaticEntryValueAssignment)
{
    EXPECT_TRUE(CParser("enum E2 { test };").Parse().Root()->Find("E2"));
    EXPECT_TRUE(CParser("enum E2 { test };").Parse().Root()->Find("E2::test"));
    EXPECT_TRUE(CParser("enum E2 { test, };").Parse().Root()->Find("E2"));
    EXPECT_TRUE(CParser("enum E2 { test, };").Parse().Root()->Find("E2::test"));
    EXPECT_THROW(CParser("enum E2 { test1, test1};").Parse(), CCompileException);
    EXPECT_EQ(CParser("enum E2 { test1, test2 };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 0);
    EXPECT_EQ(CParser("enum E2 { test1, test2 };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 1);
    EXPECT_EQ(CParser("enum E2 { test1 = 0xa, test2};").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_EQ(CParser("enum E2 { test1 = 0xa, test2};").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 11);
    EXPECT_EQ(CParser("enum E2 { test1, test2 = 10 };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 0);
    EXPECT_EQ(CParser("enum E2 { test1, test2 = 10 };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_THROW(CParser("enum E2 { test1 = 9, test2, test3 = 10 };").Parse(), CCompileException);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 8, test3, test4, };").Parse().Root()->Find("E2::test1")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 10);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 8, test3, test4, };").Parse().Root()->Find("E2::test2")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 8);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 8, test3, test4, };").Parse().Root()->Find("E2::test3")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 9);
    EXPECT_EQ(CParser("enum E2 { test1 = 10, test2 = 8, test3, test4, };").Parse().Root()->Find("E2::test4")->Get<CEnumEntry>()->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 11);
}

TEST_F(CParserEnumTest, ValueAssignment)
{
    EXPECT_EQ(CParser("struct S {enum E { test = 1 } eVar = E::test;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 1);
    EXPECT_EQ(CParser("struct S {enum E { test = 1, test2 } rgVar[2] = {E::test, E::test2};};").Parse().Root()->FindValue("S.rgVar[1]")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_THROW(CParser("struct S {enum E { test = 1 } eVar = 1;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("enum E { test1 = 1, test2 = 2 }; struct S {E eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("enum E { test1 = 1, test2 = 2 }; struct S {enum E eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("typedef enum tagE { test1 = 1, test2 = 2 } E; struct S {E eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("typedef enum tagE { test1 = 1, test2 = 2 } E; struct S {E eVar = E::test2, eVar2 = eVar;};").Parse().Root()->FindValue("S.eVar2")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
}

TEST_F(CParserEnumTest, ValueAssignmentInt8)
{
    EXPECT_EQ(CParser("struct S {enum E : int8 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : int8 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : int8 { test1 = -130, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : int8 { test1 = 1, test2 = 130 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentInt16)
{
    EXPECT_EQ(CParser("struct S {enum E : int16 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : int16 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : int16 { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : int16 { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentInt32)
{
    EXPECT_EQ(CParser("struct S {enum E : int32 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : int32 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : int32 { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : int32 { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentInt64)
{
    EXPECT_EQ(CParser("struct S {enum E : int64 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int64_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : int64 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int64_t>(), -2);
    EXPECT_EQ(CParser("struct S {enum E : int64 { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int64_t>(), -2);
    EXPECT_EQ(CParser("struct S {enum E : int64 { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int64_t>(), 2147483648);
}

TEST_F(CParserEnumTest, ValueAssignmentUInt8)
{
    EXPECT_EQ(CParser("struct S {enum E : uint8 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : uint8 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfeu);
    EXPECT_THROW(CParser("struct S {enum E : uint8 { test1 = -130, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : uint8 { test1 = 1, test2 = 130 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 130u);
}

TEST_F(CParserEnumTest, ValueAssignmentUInt16)
{
    EXPECT_EQ(CParser("struct S {enum E : uint16 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : uint16 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffeu);
    EXPECT_THROW(CParser("struct S {enum E : uint16 { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : uint16 { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 32768u);
}

TEST_F(CParserEnumTest, ValueAssignmentUInt32)
{
    EXPECT_EQ(CParser("struct S {enum E : uint32 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : uint32 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffffffeu);
    EXPECT_THROW(CParser("struct S {enum E : uint32 { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : uint32 { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2147483648u);
}

TEST_F(CParserEnumTest, ValueAssignmentUInt64)
{
    EXPECT_EQ(CParser("struct S {enum E : uint64 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 2ull);
    EXPECT_EQ(CParser("struct S {enum E : uint64 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 0xFFFFFFFFFFFFFFFEull);
    EXPECT_EQ(CParser("struct S {enum E : uint64 { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 0xFFFFFFFFFFFFFFFEull);
    EXPECT_EQ(CParser("struct S {enum E : uint64 { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 2147483648ull);
}

TEST_F(CParserEnumTest, ValueAssignmentChar)
{
    EXPECT_EQ(CParser("struct S {enum E : char { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : char { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : char { test1 = -130, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : char { test1 = 1, test2 = 130 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentShort)
{
    EXPECT_EQ(CParser("struct S {enum E : short { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : short { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : short { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : short { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentLong)
{
    EXPECT_EQ(CParser("struct S {enum E : long { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : long { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_THROW(CParser("struct S {enum E : long { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : long { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse(), CCompileException);
}

TEST_F(CParserEnumTest, ValueAssignmentLongLong)
{
    EXPECT_EQ(CParser("struct S {enum E : long long { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), 2);
    EXPECT_EQ(CParser("struct S {enum E : long long { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_EQ(CParser("struct S {enum E : long long { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int32_t>(), -2);
    EXPECT_EQ(CParser("struct S {enum E : long long { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<int64_t>(), 2147483648);
}


TEST_F(CParserEnumTest, ValueAssignmentOctet)
{
    EXPECT_EQ(CParser("struct S {enum E : octet { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : octet { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfeu);
    EXPECT_THROW(CParser("struct S {enum E : octet { test1 = -130, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : octet { test1 = 1, test2 = 130 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 130u);
}

TEST_F(CParserEnumTest, ValueAssignmentUnsignedShort)
{
    EXPECT_EQ(CParser("struct S {enum E : unsigned short { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : unsigned short { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffeu);
    EXPECT_THROW(CParser("struct S {enum E : unsigned short { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : unsigned short { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 32768u);
}

TEST_F(CParserEnumTest, ValueAssignmentUnsignedLong)
{
    EXPECT_EQ(CParser("struct S {enum E : unsigned long { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : unsigned long { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffffffeu);
    EXPECT_THROW(CParser("struct S {enum E : unsigned long { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : unsigned long { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2147483648u);
}

TEST_F(CParserEnumTest, ValueAssignmentUnsignedLongLong)
{
    EXPECT_EQ(CParser("struct S {enum E : unsigned long long { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 2ull);
    EXPECT_EQ(CParser("struct S {enum E : unsigned long long { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 0xFFFFFFFFFFFFFFFEull);
    EXPECT_EQ(CParser("struct S {enum E : unsigned long long { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 0xFFFFFFFFFFFFFFFEull);
    EXPECT_EQ(CParser("struct S {enum E : unsigned long long { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint64_t>(), 2147483648ull);
}

TEST_F(CParserEnumTest, ValueAssignmentChar16)
{
    EXPECT_EQ(CParser("struct S {enum E : char16 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : char16 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffeu);
    EXPECT_THROW(CParser("struct S {enum E : char16 { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : char16 { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 32768u);
}

TEST_F(CParserEnumTest, ValueAssignmentChar32)
{
    EXPECT_EQ(CParser("struct S {enum E : char32 { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
    EXPECT_EQ(CParser("struct S {enum E : char32 { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffffffeu);
    EXPECT_THROW(CParser("struct S {enum E : char32 { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {enum E : char32 { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2147483648u);
}

TEST_F(CParserEnumTest, ValueAssignmentWChar)
{
    if constexpr (sizeof(wchar_t) == 2)
    {
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffeu);
        EXPECT_THROW(CParser("struct S {enum E : wchar { test1 = -32769, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = 1, test2 = 32768 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 32768u);
    }
    else
    {
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2u);
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = -1, test2 = -2 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 0xfffffffeu);
        EXPECT_THROW(CParser("struct S {enum E : wchar { test1 = -2147483649, test2 = -2 } eVar = E::test2;};").Parse(), CCompileException);
        EXPECT_EQ(CParser("struct S {enum E : wchar { test1 = 1, test2 = 2147483648 } eVar = E::test2;};").Parse().Root()->FindValue("S.eVar")->Get<CEnumValueNode>()->Variant().Get<uint32_t>(), 2147483648u);
    }
}

TEST_F(CParserEnumTest, ValueAssignmentInvalidType)
{
    EXPECT_THROW(CParser("struct S {enum E : float { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : double { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : long double { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : fixed { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : string { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : u8string { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : u16string { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : u32string { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {enum E : wstring { test1 = 1, test2 = 2 } eVar = E::test2;};").Parse(), CCompileException);
}