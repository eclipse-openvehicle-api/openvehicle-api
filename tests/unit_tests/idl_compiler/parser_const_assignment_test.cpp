#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/declaration_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/constvariant.inl"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.h"

using CParserConstAssignmentTest = CParserTest;

TEST_F(CParserConstAssignmentTest, ConstAssignment)
{
    EXPECT_TRUE(CParser("const long hello = 10 * 10;").Parse().Root()->Find("hello"));
}

TEST_F(CParserConstAssignmentTest, NestedConstAssignment)
{
    EXPECT_TRUE(CParser("module Test { const long hello = 10; };").Parse().Root()->Find("Test::hello"));
}

TEST_F(CParserConstAssignmentTest, ConstMissingType)
{
    EXPECT_THROW(CParser("const hello = 10 * 10;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstMissingSemiColon)
{
    EXPECT_THROW(CParser("const long hello = 10 * 10").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstMissingAssignmentOperator)
{
    EXPECT_THROW(CParser("const long hello;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstArray)
{
    EXPECT_EQ(CParser("const long rglHello[2] = {1, 2};").Parse().Root()->Find<CVariableEntity>("rglHello")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_EQ(CParser("const long rglHello[] = {1, 2};").Parse().Root()->Find<CVariableEntity>("rglHello")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_EQ(CParser("const long rglHello[] = {1, 2}; const long rglHello2[rglHello[1]] = {rglHello[0], rglHello[1]};").
        Parse().Root()->Find<CVariableEntity>("rglHello2")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_THROW(CParser("const long rglHello[2] = {1, 2, 3};").Parse(), CCompileException);
    EXPECT_THROW(CParser("const long rglHello[2] = {1};").Parse(), CCompileException);
    EXPECT_THROW(CParser("const long rglHello[2] = {1, 2;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const long rglHello[2] = 1, 2};").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, MultipleConstAssignments)
{
    EXPECT_TRUE(CParser("const int8 iHello = 10, iHello2 = 20;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = 10, iHello2 = 20;").Parse().Root()->Find("iHello2"));
    EXPECT_EQ(CParser("const long rglHello[] = {1, 2}, rglHello2[rglHello[1]] = {rglHello[0], rglHello[1]};").
        Parse().Root()->Find<CVariableEntity>("rglHello2")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_THROW(CParser("const int8 iHello = 10, const int iHello2 = 20;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 10, int iHello2 = 20;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 10, iHello2;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentInt8)
{
    EXPECT_TRUE(CParser("const int8 iHello = 10;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = 10l;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = 10ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = 'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = L'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = u'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = U'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = '\\0\\0\\0A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = true;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = false;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = TRUE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = FALSE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = nullptr;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = NULL;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = -128;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 iHello = +127;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int8 rgiHello[2] = {+127, -127};").Parse().Root()->Find("rgiHello"));
    EXPECT_THROW(CParser("const int8 iHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = 128;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 iHello = L'\\u20ac';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int8 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentInt16)
{
    EXPECT_TRUE(CParser("const int16 iHello = 10;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = 10l;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = 10ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = 'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = L'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = u'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = U'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = '\\0\\0BA';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = true;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = false;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = TRUE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = FALSE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = nullptr;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = NULL;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = -32768;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 iHello = +32767;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int16 rgiHello[2] = {+32767, -32767};").Parse().Root()->Find("rgiHello"));
    EXPECT_THROW(CParser("const int16 iHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 iHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 iHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 iHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 iHello = 32768;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 iHello = u'\\U00010437';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int16 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentInt32)
{
    EXPECT_TRUE(CParser("const int32 iHello = 10;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = 10l;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = 10ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = 'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = L'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = u'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = U'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = 'DCBA';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = true;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = false;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = TRUE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = FALSE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = nullptr;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = NULL;").Parse().Root()->Find("iHello"));
    // NOTE: The value -2147483648 would still fit in an integer. The C++-parsing, however, sees the minus operator not as part
    // of the number and therefore does a calculation of "unary minus" on number "2147483648", which doesn't fit in the 32-bits
    // any more.
    EXPECT_TRUE(CParser("const int32 iHello = -2147483647;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = -2147483648ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 iHello = +2147483647;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int32 rgiHello[2] = {+2147483647, -2147483647};").Parse().Root()->Find("rgiHello"));
    EXPECT_THROW(CParser("const int32 iHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int32 iHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int32 iHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int32 iHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int32 iHello = 2147483648;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int32 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentInt64)
{
    EXPECT_TRUE(CParser("const int64 iHello = 10;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = 10l;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = 10ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = 'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = L'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = u'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = U'A';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = 'DCBA';").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = true;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = false;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = TRUE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = FALSE;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = nullptr;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = NULL;").Parse().Root()->Find("iHello"));
    // NOTE: The value -9223372036854775808 would still fit in an integer. The C++-parsing, however, sees the minus operator not
    // as part of the number and therefore does a calculation of "unary minus" on number "9223372036854775808", which doesn't fit
    // in the 64-bits any more.
    EXPECT_THROW(CParser("const int64 iHello = -9223372036854775808ll;").Parse(), CCompileException);
    EXPECT_TRUE(CParser("const int64 iHello = -9223372036854775807ll - 1;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 iHello = +9223372036854775807ll;").Parse().Root()->Find("iHello"));
    EXPECT_TRUE(CParser("const int64 rgiHello[2] = {+9223372036854775807ll, -9223372036854775807ll};").Parse().Root()->Find("rgiHello"));
    EXPECT_THROW(CParser("const int64 iHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int64 iHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int64 iHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int64 iHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int64 iHello = 9223372036854775808ll;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const int64 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUInt8)
{
    EXPECT_TRUE(CParser("const uint8 uiHello = 10;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = 10l;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = 10ll;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = 'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = L'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = u'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = U'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = '\\0\\0\\0A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = true;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = false;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = TRUE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = FALSE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = nullptr;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = NULL;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = +0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = -0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 uiHello = +255;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint8 rguiHello[2] = {0, +255};").Parse().Root()->Find("rguiHello"));
    EXPECT_THROW(CParser("const uint8 uiHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = 256;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = L'\\u20ac';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint8 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUInt16)
{
    EXPECT_TRUE(CParser("const uint16 uiHello = 10;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = 10l;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = 10ll;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = 'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = L'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = u'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = U'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = '\\0\\0BA';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = true;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = false;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = TRUE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = FALSE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = nullptr;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = NULL;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = +0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = -0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 uiHello = +65535;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint16 rguiHello[2] = {0, +65535};").Parse().Root()->Find("rguiHello"));
    EXPECT_THROW(CParser("const uint16 uiHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = 65536;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = u'\\U00010437';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint16 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUInt32)
{
    EXPECT_TRUE(CParser("const uint32 uiHello = 10;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = 10l;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = 10ll;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = 'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = L'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = u'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = U'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = 'DCBA';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = true;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = false;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = TRUE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = FALSE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = nullptr;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = NULL;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = +0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = -0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 uiHello = +4294967295;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint32 rguiHello[2] = {0, +4294967295};").Parse().Root()->Find("rguiHello"));
    EXPECT_THROW(CParser("const uint32 uiHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint32 uiHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint32 uiHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint32 uiHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint32 uiHello = 4294967296;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint32 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUInt64)
{
    EXPECT_TRUE(CParser("const uint64 uiHello = 10;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = 10l;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = 10ll;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = 'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = L'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = u'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = U'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = 'DCBA';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = true;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = false;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = TRUE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = FALSE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = nullptr;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = NULL;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = +0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = -0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 uiHello = +18446744073709551615ull;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const uint64 rguiHello[2] = {0, +18446744073709551615ull};").Parse().Root()->Find("rguiHello"));
    EXPECT_THROW(CParser("const uint64 uiHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint64 uiHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint64 uiHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint64 uiHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint64 uiHello = 18446744073709551616ull;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const uint64 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmenChar)
{
    EXPECT_TRUE(CParser("const char cHello = 10;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = 10l;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = 10ll;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = 'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = L'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = u'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = U'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = '\\0\\0\\0A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = true;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = false;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = TRUE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = FALSE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = nullptr;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = NULL;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = -128;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char cHello = +127;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char rgcHello[2] = {-127, +127};").Parse().Root()->Find("rgcHello"));
    EXPECT_THROW(CParser("const char cHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char cHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char cHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char cHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char cHello = 128;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char cHello = L'\\u20ac';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentChar16)
{
    EXPECT_TRUE(CParser("const char16 cHello = 10;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = 10l;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = 10ll;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = 'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = L'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = u'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = U'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = '\\0\\0BA';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = true;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = false;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = TRUE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = FALSE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = nullptr;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = NULL;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = +0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = -0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 cHello = +65535;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char16 rgcHello[2] = {0, +65535};").Parse().Root()->Find("rgcHello"));
    EXPECT_THROW(CParser("const char16 cHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 cHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 cHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 cHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 cHello = 65536;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 cHello = u'\\U00010437';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char16 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentChar32)
{
    EXPECT_TRUE(CParser("const char32 cHello = 10;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = 10l;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = 10ll;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = 'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = L'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = u'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = U'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = 'DCBA';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = true;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = false;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = TRUE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = FALSE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = nullptr;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = NULL;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = +0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 cHello = -0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const char32 rgcHello[2] = {0, +4294967295};").Parse().Root()->Find("rgcHello"));
    EXPECT_THROW(CParser("const char32 cHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char32 cHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char32 cHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char32 cHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char32 cHello = 4294967296;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const char32 uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentWChar)
{
    EXPECT_TRUE(CParser("const wchar cHello = 10;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = 10l;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = 10ll;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = 'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = L'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = u'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = U'A';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = '\\0\\0BA';").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = true;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = false;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = TRUE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = FALSE;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = nullptr;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = NULL;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = +0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = -0;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar cHello = +65535;").Parse().Root()->Find("cHello"));
    EXPECT_TRUE(CParser("const wchar rgcHello[2] = {0, +65535};").Parse().Root()->Find("rgcHello"));
    EXPECT_THROW(CParser("const wchar cHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wchar cHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wchar cHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wchar cHello = 10.0d;").Parse(), CCompileException);
#ifdef _MSC_VER
    EXPECT_THROW(CParser("const wchar cHello = 65536;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wchar cHello = u'\\U00010437';").Parse(), CCompileException);
#else
    EXPECT_THROW(CParser("const wchar cHello = 4294967296;").Parse(), CCompileException);
#endif
    EXPECT_THROW(CParser("const wchar uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentOctet)
{
    EXPECT_TRUE(CParser("const octet uiHello = 10;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = 10l;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = 10ll;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = 'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = L'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = u'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = U'A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = '\\0\\0\\0A';").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = true;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = false;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = TRUE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = FALSE;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = nullptr;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = NULL;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = +0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = -0;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet uiHello = +255;").Parse().Root()->Find("uiHello"));
    EXPECT_TRUE(CParser("const octet rguiHello[2] = {0, +255};").Parse().Root()->Find("rguiHello"));
    EXPECT_THROW(CParser("const octet uiHello = 10.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = 10.0f;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = 10.0l;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = 10.0d;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = 256;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = L'\\u20ac';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const octet uiHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentFloat)
{
    EXPECT_TRUE(CParser("const float fHello = 111.11;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = -2.22;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = 0X1.BC70A3D70A3D7P+6;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = .18973e+39;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = 10ull;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = 'A';").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = L'A';").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = u'A';").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = U'A';").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = '\\0\\0\\0A';").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = true;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = false;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = TRUE;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = FALSE;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = nullptr;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = NULL;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser((std::string("const float fHello = ") + std::to_string(std::numeric_limits<float>::min()) + ";").c_str()).Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser((std::string("const float fHello = ") + std::to_string(std::numeric_limits<float>::max()) + ";").c_str()).Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser((std::string("const float fHello = ") + std::to_string(std::numeric_limits<float>::lowest()) + ";").c_str()).Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float fHello = 10.0d;").Parse().Root()->Find("fHello"));
    EXPECT_TRUE(CParser("const float rgfHello[2] = {10.0d, 20.0d};").Parse().Root()->Find("rgfHello"));
    EXPECT_THROW(CParser("const float fHello = 3.40283e+38;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const float fHello = -3.40283e+38;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const float fHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentDouble)
{
    EXPECT_TRUE(CParser("const double ldHello = 111.11;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = -2.22;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = 0X1.BC70A3D70A3D7P+6;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = .18973e+39;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = 10ull;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = 'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = L'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = u'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = U'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = '\\0\\0\\0A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = true;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = false;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = TRUE;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = FALSE;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = nullptr;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double ldHello = NULL;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser((std::string("const double dHello = ") + std::to_string(std::numeric_limits<double>::min()) + ";").c_str()).Parse().Root()->Find("dHello"));
    EXPECT_TRUE(CParser((std::string("const double dHello = ") + std::to_string(std::numeric_limits<double>::max()) + ";").c_str()).Parse().Root()->Find("dHello"));
    EXPECT_TRUE(CParser((std::string("const double dHello = ") + std::to_string(std::numeric_limits<double>::lowest()) + ";").c_str()).Parse().Root()->Find("dHello"));
    EXPECT_TRUE(CParser("const double ldHello = 10.0d;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const double rgldHello[2] = {10.0d, 20.0d};").Parse().Root()->Find("rgldHello"));
    EXPECT_THROW(CParser("const double ldHello = 1.7977e+308;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const double ldHello = -1.7977e+308;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const double ldHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentLongDouble)
{
    EXPECT_TRUE(CParser("const long double ldHello = 111.11;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = -2.22;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = 0X1.BC70A3D70A3D7P+6;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = .18973e+39;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = 10ull;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = 'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = L'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = u'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = U'A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = '\\0\\0\\0A';").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = true;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = false;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = TRUE;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = FALSE;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = nullptr;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = NULL;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser((std::string("const long double ldHello = ") + std::to_string(std::numeric_limits<long double>::min()) + ";").c_str()).Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser((std::string("const long double ldHello = ") + std::to_string(std::numeric_limits<long double>::max()) + ";").c_str()).Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser((std::string("const long double ldHello = ") + std::to_string(std::numeric_limits<long double>::lowest()) + ";").c_str()).Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double ldHello = 10.0d;").Parse().Root()->Find("ldHello"));
    EXPECT_TRUE(CParser("const long double rgldHello[2] = {10.0d, 20.0d};").Parse().Root()->Find("rgldHello"));
    EXPECT_THROW(CParser("const long double ldHello = 1.18974e+4932;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const long double ldHello = -1.18974e+4932;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const long double ldHello = u8\"\\u20ac\";").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentFixed)
{
    EXPECT_TRUE(CParser("const fixed fixHello = 111.11d;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = -2.22D;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 0X1.BC70A3D70A3D7P+6;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 10ull;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 'A';").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = L'A';").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = u'A';").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = U'A';").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = '\\0\\0\\0A';").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = true;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = false;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = TRUE;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = FALSE;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = nullptr;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = NULL;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = -2147483647;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 2147483648;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 0.0000000004656612873077392578125d;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed fixHello = 10.0d;").Parse().Root()->Find("fixHello"));
    EXPECT_TRUE(CParser("const fixed rgfixHello[2] = {10.0d, 20.0d};").Parse().Root()->Find("rgfixHello"));
    EXPECT_THROW(CParser("const fixed fHello = 0.2147484000e+10;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const fixed fHello = -0.2147484000e+10;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const fixed fHello = u8\"\\u20ac\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const fixed fixHello = .18973e+39;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentBoolean)
{
    EXPECT_TRUE(CParser("const boolean bHello = true;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = false;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = TRUE;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = FALSE;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = 1;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = 0;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = '\\1';").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = '\\0\\0\\0\\1';").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = -1;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = 2;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean bHello = 1.0;").Parse().Root()->Find("bHello"));
    EXPECT_TRUE(CParser("const boolean rgbHello[2] = {1.0, 2.0};").Parse().Root()->Find("rgbHello"));
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentString)
{
    EXPECT_TRUE(CParser("const string ssHello = \"hello\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const string ssHello = \"hello\" \"hello2\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const string rgssHello[2] = {\"hello\", \"hello2\" \"hello3\"};").Parse().Root()->Find("rgssHello"));
    EXPECT_THROW(CParser("const string ssHello = u8\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = u\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = U\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = L\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = \"hello\" u8\"hello2\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = true;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = 1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = 1.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = 'A';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const string ssHello = nullptr;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUtf8String)
{
    EXPECT_TRUE(CParser("const u8string ssHello = u8\"hello\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u8string ssHello = u8\"hello\" u8\"hello2\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u8string rgssHello[2] = {u8\"hello\", u8\"hello2\" u8\"hello3\"};").Parse().Root()->Find("rgssHello"));
    EXPECT_THROW(CParser("const u8string ssHello = \"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = u\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = U\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = L\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = u8\"hello\" \"hello2\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = true;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = 1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = 1.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = 'A';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u8string ssHello = nullptr;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUtf16String)
{
    EXPECT_TRUE(CParser("const u16string ssHello = u\"hello\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u16string ssHello = u\"hello\" u\"hello2\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u16string rgssHello[2] = {u\"hello\", u\"hello2\" u\"hello3\"};").Parse().Root()->Find("rgssHello"));
    EXPECT_THROW(CParser("const u16string ssHello = \"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = u8\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = U\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = L\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = u\"hello\" \"hello2\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = true;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = 1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = 1.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = 'A';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u16string ssHello = nullptr;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentUtf32String)
{
    EXPECT_TRUE(CParser("const u32string ssHello = U\"hello\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u32string ssHello = U\"hello\" U\"hello2\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const u32string rgssHello[2] = {U\"hello\", U\"hello2\" U\"hello3\"};").Parse().Root()->Find("rgssHello"));
    EXPECT_THROW(CParser("const u32string ssHello = \"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = u8\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = u\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = L\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = U\"hello\" \"hello2\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = true;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = 1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = 1.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = 'A';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const u32string ssHello = nullptr;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentWideString)
{
    EXPECT_TRUE(CParser("const wstring ssHello = L\"hello\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const wstring ssHello = L\"hello\" L\"hello2\";").Parse().Root()->Find("ssHello"));
    EXPECT_TRUE(CParser("const wstring rgssHello[2] = {L\"hello\", L\"hello2\" L\"hello3\"};").Parse().Root()->Find("rgssHello"));
    EXPECT_THROW(CParser("const wstring ssHello = \"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = u8\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = u\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = U\"hello\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = L\"hello\" \"hello2\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = true;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = 1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = 1.0;").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = 'A';").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello = nullptr;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentNumericIdentifiers)
{
    EXPECT_TRUE(CParser("const int32 iHello1 = 100;\nconst int32 iHello2 = iHello1;").Parse().Root()->Find("iHello2"));
    EXPECT_TRUE(CParser("const int32 iHello1 = 100;\nconst int32 iHello2 = iHello1; const int64 rgiHello3[] = {iHello1, iHello2};").Parse().Root()->Find("rgiHello3"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nconst int32 iHello2 = TEST::iHello1;").Parse().Root()->Find("iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nconst int32 iHello2 = ::TEST::iHello1;").Parse().Root()->Find("iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nmodule TEST2{const int32 iHello2 = TEST::iHello1;};").Parse().Root()->Find("TEST2::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nmodule TEST2{const int32 iHello2 = ::TEST::iHello1;};").Parse().Root()->Find("TEST2::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100; const int32 iHello2 = TEST::iHello1;};").Parse().Root()->Find("TEST::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100; const int32 iHello2 = iHello1;};").Parse().Root()->Find("TEST::iHello2"));

}

TEST_F(CParserConstAssignmentTest, ConstAssignmentStringIdentifiers)
{
    EXPECT_TRUE(CParser("const wstring ssHello1 = L\"hello\";\nconst wstring ssHello2 = ssHello1;").Parse().Root()->Find("ssHello2"));
    EXPECT_TRUE(CParser("const wstring ssHello1 = L\"hello\";\nconst wstring ssHello2 = ssHello1; const wstring ssHello3[] = {ssHello1, ssHello2};").Parse().Root()->Find("ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nconst wstring ssHello2 = TEST::ssHello1;").Parse().Root()->Find("ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nconst wstring ssHello2 = ::TEST::ssHello1;").Parse().Root()->Find("ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nmodule TEST2{const wstring ssHello2 = TEST::ssHello1;};").Parse().Root()->Find("TEST2::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nmodule TEST2{const wstring ssHello2 = ::TEST::ssHello1;};").Parse().Root()->Find("TEST2::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\"; const wstring ssHello2 = TEST::ssHello1;};").Parse().Root()->Find("TEST::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\"; const wstring ssHello2 = ssHello1;};").Parse().Root()->Find("TEST::ssHello2"));
    EXPECT_THROW(CParser("const wstring ssHello1 = L\"hello\";\nconst wstring ssHello2 = ssHello1 \"this doesn't work\";").Parse(), CCompileException);
    EXPECT_THROW(CParser("const wstring ssHello1 = L\"hello\";\nconst wstring ssHello2 = \"this doesn't work\" ssHello1;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ComplexConstAssignment)
{
    EXPECT_TRUE(CParser("struct S {int32 i;}; const S s1 = {10};").Parse().Root()->Find("s1"));
    EXPECT_TRUE(CParser("struct S {int32 i;}; const struct S s1 = {10};").Parse().Root()->Find("s1"));
    EXPECT_THROW(CParser("interface I {}; const I i1;").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception Ex {}; const Ex ex1;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentOperatorsIntegral)
{
    // Parenthesis while calculating
    CParser parserParenthesis(R"code(
        const int32 a = 10;
        const int32 b = 5;
        const int32 c = b*b;
        const int64 e = 10;
        const int16 f = a * (c + e);
    )code");
    EXPECT_NO_THROW(parserParenthesis.Parse());
    const CVariableEntity* pParenthesisEntityResult = parserParenthesis.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pParenthesisEntityResult, nullptr);
    EXPECT_EQ(pParenthesisEntityResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 350);

    // Operators + - / * %
    CParser parserArithmetic(R"code(
        const int64 f = 10 + 11 * 35 % 10 -2;
    )code");
    EXPECT_NO_THROW(parserArithmetic.Parse());
    const CVariableEntity* pArithmeticResult = parserArithmetic.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pArithmeticResult, nullptr);
    EXPECT_EQ(pArithmeticResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 13);

    // Operators << >> & | ~
    CParser parserBitManipulation(R"code(
        const uint8 f = 2 << 2 | 7 >> 1 & (~ 120 & 7);
    )code");
    EXPECT_NO_THROW(parserBitManipulation.Parse());
    const CVariableEntity* pBitManipulationResult = parserBitManipulation.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pBitManipulationResult, nullptr);
    EXPECT_EQ(pBitManipulationResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int64_t>(), 11);

    // Operators && || < > <= >= == !=
    CParser parserCondition(R"code(
        const boolean a = (20 || 10 && 1) < 2;
        const boolean b = 50 + 7 >= 57;
        const boolean c = 50 + 8 > 57;
        const boolean d = 50 + 7 <= 57;
        const boolean e = 10 != 0;
        const int32 f = a + b + c + d + e;
    )code");
    EXPECT_NO_THROW(parserCondition.Parse());
    const CVariableEntity* pConditionResult = parserCondition.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pConditionResult, nullptr);
    EXPECT_EQ(pConditionResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<uint8_t>(), 5);

    // Div by zero /
    EXPECT_THROW(CParser("const long i = 0; const unsigned long j = 10/i;").Parse(), CCompileException);

    // Div by zero %
    EXPECT_THROW(CParser("const long i = 0; const unsigned long j = 10%i;").Parse(), CCompileException);
}

TEST_F(CParserConstAssignmentTest, ConstAssignmentOperatorsFloatingPoint)
{
    // Parenthesis while calculating
    CParser parserParenthesis(R"code(
        const double a = 10;
        const double b = 5;
        const double c = b*b;
        const double e = 10;
        const double f = a * (c + e);
    )code");
    EXPECT_NO_THROW(parserParenthesis.Parse());
    const CVariableEntity* pParenthesisEntityResult = parserParenthesis.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pParenthesisEntityResult, nullptr);
    EXPECT_EQ(pParenthesisEntityResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<float>(), 350);

    // Operators + - / * %
    CParser parserArithmetic(R"code(
        const float f = 10 + 11 * 35 % 10 -2;
    )code");
    EXPECT_NO_THROW(parserArithmetic.Parse());
    const CVariableEntity* pArithmeticResult = parserArithmetic.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pArithmeticResult, nullptr);
    EXPECT_EQ(pArithmeticResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<long double>(), 13);

    // Operators && || < > <= >= == !=
    CParser parserCondition(R"code(
        const boolean a = (20.0 || 10.0 && 1.0) < 2.0;
        const boolean b = 50.0 + 7.0 >= 57.0;
        const boolean c = 50.0 + 8.0 > 57.0;
        const boolean d = 50.0 + 7.0 <= 57.0;
        const boolean e = 10.0 != 0.0;
        const int32 f = a + b + c + d + e;
    )code");
    EXPECT_NO_THROW(parserCondition.Parse());
    const CVariableEntity* pConditionResult = parserCondition.Root()->Find<CVariableEntity>("f");
    ASSERT_NE(pConditionResult, nullptr);
    EXPECT_EQ(pConditionResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<uint8_t>(), 5);

    // Div by zero /
    EXPECT_THROW(CParser("const float i = 0; const long double j = 10/i;").Parse(), CCompileException);

    // Div by zero %
    EXPECT_THROW(CParser("const double i = 0; const float j = 10%i;").Parse(), CCompileException);
}

