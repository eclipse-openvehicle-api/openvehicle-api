#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/declaration_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/variable_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/constvariant.inl"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.h"

using CParserVarAssignmentTest = CParserTest;

TEST_F(CParserVarAssignmentTest, ParsingVarAssignment)
{
    EXPECT_TRUE(CParser("struct S {long hello = 10 * 10;};").Parse().Root()->Find("S::hello"));
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentOutOfScope)
{
    EXPECT_THROW(CParser("long hello = 10 * 10;").Parse(), CCompileException);
    EXPECT_THROW(CParser("module m {long hello = 10 * 10;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingConstMissingSemiColon)
{
    EXPECT_THROW(CParser("struct S {long hello = 10 * 10};};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingConstNoAssignmentOperator)
{
    EXPECT_TRUE(CParser("struct S {long hello;};").Parse().Root()->Find("S::hello"));
}

TEST_F(CParserVarAssignmentTest, ParsingVarArray)
{
    EXPECT_EQ(CParser("struct S {long rglHello[2] = {1, 2};};").Parse().Root()->Find<CVariableEntity>("S::rglHello")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_THROW(CParser("struct S {long rglHello[] = {1, 2};};").Parse(), CCompileException);
    EXPECT_EQ(CParser("struct S {const long rglHello[2] = {1, 2}; long rglHello2[rglHello[1]] = {rglHello[0], rglHello[1]};};").
        Parse().Root()->Find<CVariableEntity>("S::rglHello2")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_THROW(CParser("struct S {long rglHello[2] = {1, 2, 3};};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {long rglHello[2] = {1};};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {long rglHello[2] = {1, 2;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {long rglHello[2] = 1, 2};};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingMultipleVarAssignments)
{
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10, iHello2 = 20;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10, iHello2 = 20;};").Parse().Root()->Find("S::iHello2"));
    EXPECT_EQ(CParser("struct S {const long rglHello[2] = {1, 2}, rglHello2[rglHello[1]] = {rglHello[0], rglHello[1]};};").
        Parse().Root()->Find<CVariableEntity>("S::rglHello2")->ValueRef()->Get<CArrayValueNode>()->GetSize(), 2);
    EXPECT_THROW(CParser("struct S {int8 iHello = 10, int iHello2 = 20;};").Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10, iHello2;};").Parse().Root()->Find("S::iHello2"));
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentInt8)
{
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10l;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = 10ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = 'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = L'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = u'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = U'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = true;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = false;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = TRUE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = FALSE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = nullptr;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = NULL;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = -128;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 iHello = +127;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int8 rgiHello[2] = {+127, -127};};").Parse().Root()->Find("S::rgiHello"));
    EXPECT_THROW(CParser("struct S {int8 iHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 iHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 iHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 iHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 iHello = 128;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 iHello = L'\\u20ac';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int8 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentInt16)
{
    EXPECT_TRUE(CParser("struct S {int16 iHello = 10;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = 10l;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = 10ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = 'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = L'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = u'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = U'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = '\\0\\0BA';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = true;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = false;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = TRUE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = FALSE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = nullptr;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = NULL;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = -32768;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 iHello = +32767;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int16 rgiHello[2] = {+32767, -32767};};").Parse().Root()->Find("S::rgiHello"));
    EXPECT_THROW(CParser("struct S {int16 iHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 iHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 iHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 iHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 iHello = 32768;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 iHello = u'\\U00010437';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int16 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentInt32)
{
    EXPECT_TRUE(CParser("struct S {int32 iHello = 10;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = 10l;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = 10ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = 'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = L'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = u'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = U'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = 'DCBA';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = true;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = false;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = TRUE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = FALSE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = nullptr;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = NULL;};").Parse().Root()->Find("S::iHello"));
    // NOTE: The value -2147483648 would still fit in an integer. The C++-parsing, however, sees the minus operator not as part
    // of the number and therefore does a calculation of "unary minus" on number "2147483648", which doesn't fit in the 32-bits
    // any more.
    EXPECT_TRUE(CParser("struct S {int32 iHello = -2147483647;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = -2147483648ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 iHello = +2147483647;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int32 rgiHello[2] = {+2147483647, -2147483647};};").Parse().Root()->Find("S::rgiHello"));
    EXPECT_THROW(CParser("struct S {int32 iHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int32 iHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int32 iHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int32 iHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int32 iHello = 2147483648;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int32 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentInt64)
{
    EXPECT_TRUE(CParser("struct S {int64 iHello = 10;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = 10l;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = 10ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = 'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = L'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = u'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = U'A';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = 'DCBA';};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = true;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = false;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = TRUE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = FALSE;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = nullptr;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = NULL;};").Parse().Root()->Find("S::iHello"));
    // NOTE: The value -9223372036854775808 would still fit in an integer. The C++-parsing, however, sees the minus operator not
    // as part of the number and therefore does a calculation of "unary minus" on number "9223372036854775808", which doesn't fit
    // in the 64-bits any more.
    EXPECT_THROW(CParser("struct S {int64 iHello = -9223372036854775808ll;};").Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct S {int64 iHello = -9223372036854775807ll - 1;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 iHello = +9223372036854775807ll;};").Parse().Root()->Find("S::iHello"));
    EXPECT_TRUE(CParser("struct S {int64 rgiHello[2] = {+9223372036854775807ll, -9223372036854775807ll};};").Parse().Root()->Find("S::rgiHello"));
    EXPECT_THROW(CParser("struct S {int64 iHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int64 iHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int64 iHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int64 iHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int64 iHello = 9223372036854775808ll;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {int64 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUInt8)
{
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = 10;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = 10l;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = 10ll;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = 'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = L'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = u'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = U'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = true;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = false;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = TRUE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = FALSE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = nullptr;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = NULL;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = +0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = -0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 uiHello = +255;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint8 rguiHello[2] = {0, +255};};").Parse().Root()->Find("S::rguiHello"));
    EXPECT_THROW(CParser("struct S {uint8 uiHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = 256;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = L'\\u20ac';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint8 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUInt16)
{
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = 10;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = 10l;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = 10ll;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = 'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = L'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = u'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = U'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = '\\0\\0BA';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = true;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = false;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = TRUE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = FALSE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = nullptr;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = NULL;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = +0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = -0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 uiHello = +65535;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint16 rguiHello[2] = {0, +65535};};").Parse().Root()->Find("S::rguiHello"));
    EXPECT_THROW(CParser("struct S {uint16 uiHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = 65536;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = u'\\U00010437';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint16 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUInt32)
{
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = 10;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = 10l;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = 10ll;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = 'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = L'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = u'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = U'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = 'DCBA';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = true;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = false;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = TRUE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = FALSE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = nullptr;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = NULL;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = +0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = -0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 uiHello = +4294967295;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint32 rguiHello[2] = {0, +4294967295};};").Parse().Root()->Find("S::rguiHello"));
    EXPECT_THROW(CParser("struct S {uint32 uiHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint32 uiHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint32 uiHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint32 uiHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint32 uiHello = 4294967296;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint32 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUInt64)
{
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = 10;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = 10l;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = 10ll;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = 'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = L'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = u'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = U'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = 'DCBA';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = true;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = false;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = TRUE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = FALSE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = nullptr;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = NULL;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = +0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = -0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 uiHello = +18446744073709551615ull;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {uint64 rguiHello[2] = {0, +18446744073709551615ull};};").Parse().Root()->Find("S::rguiHello"));
    EXPECT_THROW(CParser("struct S {uint64 uiHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint64 uiHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint64 uiHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint64 uiHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint64 uiHello = 18446744073709551616ull;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {uint64 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingConstAssignmenChar)
{
    EXPECT_TRUE(CParser("struct S {char cHello = 10;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = 10l;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = 10ll;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = 'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = L'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = u'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = U'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = true;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = false;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = TRUE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = FALSE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = nullptr;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = NULL;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = -128;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char cHello = +127;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char rgcHello[2] = {-127, +127};};").Parse().Root()->Find("S::rgcHello"));
    EXPECT_THROW(CParser("struct S {char cHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char cHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char cHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char cHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char cHello = 128;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char cHello = L'\\u20ac';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentChar16)
{
    EXPECT_TRUE(CParser("struct S {char16 cHello = 10;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = 10l;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = 10ll;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = 'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = L'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = u'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = U'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = '\\0\\0BA';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = true;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = false;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = TRUE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = FALSE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = nullptr;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = NULL;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = +0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = -0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 cHello = +65535;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char16 rgcHello[2] = {0, +65535};};").Parse().Root()->Find("S::rgcHello"));
    EXPECT_THROW(CParser("struct S {char16 cHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 cHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 cHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 cHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 cHello = 65536;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 cHello = u'\\U00010437';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char16 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentChar32)
{
    EXPECT_TRUE(CParser("struct S {char32 cHello = 10;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = 10l;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = 10ll;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = 'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = L'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = u'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = U'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = 'DCBA';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = true;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = false;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = TRUE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = FALSE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = nullptr;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = NULL;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = +0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 cHello = -0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {char32 rgcHello[2] = {0, +4294967295};};").Parse().Root()->Find("S::rgcHello"));
    EXPECT_THROW(CParser("struct S {char32 cHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char32 cHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char32 cHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char32 cHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char32 cHello = 4294967296;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {char32 uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentWChar)
{
    EXPECT_TRUE(CParser("struct S {wchar cHello = 10;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = 10l;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = 10ll;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = 'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = L'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = u'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = U'A';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = '\\0\\0BA';};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = true;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = false;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = TRUE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = FALSE;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = nullptr;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = NULL;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = +0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = -0;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar cHello = +65535;};").Parse().Root()->Find("S::cHello"));
    EXPECT_TRUE(CParser("struct S {wchar rgcHello[2] = {0, +65535};};").Parse().Root()->Find("S::rgcHello"));
    EXPECT_THROW(CParser("struct S {wchar cHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wchar cHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wchar cHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wchar cHello = 10.0d;};").Parse(), CCompileException);
#ifdef _MSC_VER
    EXPECT_THROW(CParser("struct S {wchar cHello = 65536;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wchar cHello = u'\\U00010437';};").Parse(), CCompileException);
#else
    EXPECT_THROW(CParser("struct S {wchar cHello = 4294967296;};").Parse(), CCompileException);
#endif
    EXPECT_THROW(CParser("struct S {wchar uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentOctet)
{
    EXPECT_TRUE(CParser("struct S {octet uiHello = 10;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = 10l;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = 10ll;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = 'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = L'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = u'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = U'A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = true;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = false;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = TRUE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = FALSE;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = nullptr;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = NULL;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = +0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = -0;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet uiHello = +255;};").Parse().Root()->Find("S::uiHello"));
    EXPECT_TRUE(CParser("struct S {octet rguiHello[2] = {0, +255};};").Parse().Root()->Find("S::rguiHello"));
    EXPECT_THROW(CParser("struct S {octet uiHello = 10.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = 10.0f;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = 10.0l;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = 10.0d;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = 256;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = L'\\u20ac';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {octet uiHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentFloat)
{
    EXPECT_TRUE(CParser("struct S {float fHello = 111.11;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = -2.22;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = 0X1.BC70A3D70A3D7P+6;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = .18973e+39;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = 10ull;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = 'A';};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = L'A';};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = u'A';};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = U'A';};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = true;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = false;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = TRUE;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = FALSE;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = nullptr;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = NULL;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser((std::string("struct S {float fHello = ") + std::to_string(std::numeric_limits<float>::min()) + ";};").c_str()).Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser((std::string("struct S {float fHello = ") + std::to_string(std::numeric_limits<float>::max()) + ";};").c_str()).Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser((std::string("struct S {float fHello = ") + std::to_string(std::numeric_limits<float>::lowest()) + ";};").c_str()).Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float fHello = 10.0d;};").Parse().Root()->Find("S::fHello"));
    EXPECT_TRUE(CParser("struct S {float rgfHello[2] = {10.0d, 20.0d};};").Parse().Root()->Find("S::rgfHello"));
    EXPECT_THROW(CParser("struct S {float fHello = 3.40283e+38;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {float fHello = -3.40283e+38;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {float fHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentDouble)
{
    EXPECT_TRUE(CParser("struct S {double ldHello = 111.11;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = -2.22;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = 0X1.BC70A3D70A3D7P+6;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = .18973e+39;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = 10ull;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = 'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = L'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = u'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = U'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = true;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = false;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = TRUE;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = FALSE;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = nullptr;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = NULL;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser((std::string("struct S {double dHello = ") + std::to_string(std::numeric_limits<double>::min()) + ";};").c_str()).Parse().Root()->Find("S::dHello"));
    EXPECT_TRUE(CParser((std::string("struct S {double dHello = ") + std::to_string(std::numeric_limits<double>::max()) + ";};").c_str()).Parse().Root()->Find("S::dHello"));
    EXPECT_TRUE(CParser((std::string("struct S {double dHello = ") + std::to_string(std::numeric_limits<double>::lowest()) + ";};").c_str()).Parse().Root()->Find("S::dHello"));
    EXPECT_TRUE(CParser("struct S {double ldHello = 10.0d;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {double rgldHello[2] = {10.0d, 20.0d};};").Parse().Root()->Find("S::rgldHello"));
    EXPECT_THROW(CParser("struct S {double ldHello = 1.7977e+308;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {double ldHello = -1.7977e+308;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {double ldHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentLongDouble)
{
    EXPECT_TRUE(CParser("struct S {long double ldHello = 111.11;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = -2.22;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = 0X1.BC70A3D70A3D7P+6;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = .18973e+39;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = 10ull;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = 'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = L'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = u'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = U'A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = true;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = false;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = TRUE;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = FALSE;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = nullptr;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = NULL;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser((std::string("struct S {long double ldHello = ") + std::to_string(std::numeric_limits<long double>::min()) + ";};").c_str()).Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser((std::string("struct S {long double ldHello = ") + std::to_string(std::numeric_limits<long double>::max()) + ";};").c_str()).Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser((std::string("struct S {long double ldHello = ") + std::to_string(std::numeric_limits<long double>::lowest()) + ";};").c_str()).Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double ldHello = 10.0d;};").Parse().Root()->Find("S::ldHello"));
    EXPECT_TRUE(CParser("struct S {long double rgldHello[2] = {10.0d, 20.0d};};").Parse().Root()->Find("S::rgldHello"));
    EXPECT_THROW(CParser("struct S {long double ldHello = 1.18974e+4932;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {long double ldHello = -1.18974e+4932;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {long double ldHello = u8\"\\u20ac\";};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentFixed)
{
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 111.11d;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = -2.22D;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 0X1.BC70A3D70A3D7P+6;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 10ull;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 'A';};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = L'A';};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = u'A';};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = U'A';};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = '\\0\\0\\0A';};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = true;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = false;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = TRUE;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = FALSE;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = nullptr;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = NULL;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = -2147483647;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 2147483648;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 0.0000000004656612873077392578125d;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed fixHello = 10.0d;};").Parse().Root()->Find("S::fixHello"));
    EXPECT_TRUE(CParser("struct S {fixed rgfixHello[2] = {10.0d, 20.0d};};").Parse().Root()->Find("S::rgfixHello"));
    EXPECT_THROW(CParser("struct S {fixed fHello = 0.2147484000e+10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {fixed fHello = -0.2147484000e+10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {fixed fHello = u8\"\\u20ac\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {fixed fixHello = .18973e+39;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentBoolean)
{
    EXPECT_TRUE(CParser("struct S {boolean bHello = true;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = false;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = TRUE;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = FALSE;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = 1;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = 0;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = '\\1';};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = '\\0\\0\\0\\1';};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = -1;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = 2;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean bHello = 1.0;};").Parse().Root()->Find("S::bHello"));
    EXPECT_TRUE(CParser("struct S {boolean rgbHello[2] = {1.0, 2.0};};").Parse().Root()->Find("S::rgbHello"));
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentString)
{
    EXPECT_TRUE(CParser("struct S {string ssHello = \"hello\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {string ssHello = \"hello\" \"hello2\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {string rgssHello[2] = {\"hello\", \"hello2\" \"hello3\"};};").Parse().Root()->Find("S::rgssHello"));
    EXPECT_THROW(CParser("struct S {string ssHello = u8\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = u\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = U\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = L\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = \"hello\" u8\"hello2\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = true;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = 1;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = 1.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = 'A';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {string ssHello = nullptr;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUtf8String)
{
    EXPECT_TRUE(CParser("struct S {u8string ssHello = u8\"hello\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u8string ssHello = u8\"hello\" u8\"hello2\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u8string rgssHello[2] = {u8\"hello\", u8\"hello2\" u8\"hello3\"};};").Parse().Root()->Find("S::rgssHello"));
    EXPECT_THROW(CParser("struct S {u8string ssHello = \"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = u\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = U\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = L\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = u8\"hello\" \"hello2\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = true;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = 1;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = 1.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = 'A';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u8string ssHello = nullptr;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUtf16String)
{
    EXPECT_TRUE(CParser("struct S {u16string ssHello = u\"hello\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u16string ssHello = u\"hello\" u\"hello2\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u16string rgssHello[2] = {u\"hello\", u\"hello2\" u\"hello3\"};};").Parse().Root()->Find("S::rgssHello"));
    EXPECT_THROW(CParser("struct S {u16string ssHello = \"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = u8\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = U\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = L\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = u\"hello\" \"hello2\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = true;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = 1;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = 1.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = 'A';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u16string ssHello = nullptr;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentUtf32String)
{
    EXPECT_TRUE(CParser("struct S {u32string ssHello = U\"hello\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u32string ssHello = U\"hello\" U\"hello2\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {u32string rgssHello[2] = {U\"hello\", U\"hello2\" U\"hello3\"};};").Parse().Root()->Find("S::rgssHello"));
    EXPECT_THROW(CParser("struct S {u32string ssHello = \"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = u8\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = u\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = L\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = U\"hello\" \"hello2\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = true;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = 1;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = 1.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = 'A';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {u32string ssHello = nullptr;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentWideString)
{
    EXPECT_TRUE(CParser("struct S {wstring ssHello = L\"hello\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {wstring ssHello = L\"hello\" L\"hello2\";};").Parse().Root()->Find("S::ssHello"));
    EXPECT_TRUE(CParser("struct S {wstring rgssHello[2] = {L\"hello\", L\"hello2\" L\"hello3\"};};").Parse().Root()->Find("S::rgssHello"));
    EXPECT_THROW(CParser("struct S {wstring ssHello = \"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = u8\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = u\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = U\"hello\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = L\"hello\" \"hello2\";};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = true;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = 1;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = 1.0;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = 'A';};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {wstring ssHello = nullptr;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentNumericIdentifiers)
{
    EXPECT_TRUE(CParser("struct S {const int32 iHello1 = 100;\nint32 iHello2 = iHello1;};").Parse().Root()->Find("S::iHello2"));
    EXPECT_TRUE(CParser("struct S {const int32 iHello1 = 100;\nconst int32 iHello2 = iHello1; int64 rgiHello3[2] = {iHello1, iHello2};};").Parse().Root()->Find("S::rgiHello3"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nstruct S {int32 iHello2 = TEST::iHello1;};").Parse().Root()->Find("S::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nstruct S {int32 iHello2 = ::TEST::iHello1;};").Parse().Root()->Find("S::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nmodule TEST2{struct S {int32 iHello2 = TEST::iHello1;};};").Parse().Root()->Find("TEST2::S::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100;};\nmodule TEST2{struct S {int32 iHello2 = ::TEST::iHello1;};};").Parse().Root()->Find("TEST2::S::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100; struct S {int32 iHello2 = TEST::iHello1;};};").Parse().Root()->Find("TEST::S::iHello2"));
    EXPECT_TRUE(CParser("module TEST {const int32 iHello1 = 100; struct S {int32 iHello2 = iHello1;};};").Parse().Root()->Find("TEST::S::iHello2"));
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentStringIdentifiers)
{
    EXPECT_TRUE(CParser("struct S {const wstring ssHello1 = L\"hello\";\nwstring ssHello2 = ssHello1;};").Parse().Root()->Find("S::ssHello2"));
    EXPECT_TRUE(CParser("struct S {const wstring ssHello1 = L\"hello\";\nconst wstring ssHello2 = ssHello1; wstring rgssHello3[2] = {ssHello1, ssHello2};};").Parse().Root()->Find("S::rgssHello3"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nstruct S {wstring ssHello2 = TEST::ssHello1;};").Parse().Root()->Find("S::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nstruct S {wstring ssHello2 = ::TEST::ssHello1;};").Parse().Root()->Find("S::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nmodule TEST2{struct S {wstring ssHello2 = TEST::ssHello1;};};").Parse().Root()->Find("TEST2::S::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\";};\nmodule TEST2{struct S {wstring ssHello2 = ::TEST::ssHello1;};};").Parse().Root()->Find("TEST2::S::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\"; struct S { wstring ssHello2 = TEST::ssHello1;};};").Parse().Root()->Find("TEST::S::ssHello2"));
    EXPECT_TRUE(CParser("module TEST {const wstring ssHello1 = L\"hello\"; struct S { wstring ssHello2 = ssHello1;};};").Parse().Root()->Find("TEST::S::ssHello2"));
    EXPECT_THROW(CParser("struct S {const wstring ssHello1 = L\"hello\";\nwstring ssHello2 = ssHello1 \"this doesn't work\";};};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S {const wstring ssHello1 = L\"hello\";\nwstring ssHello2 = \"this doesn't work\" ssHello1;};};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentOperatorsIntegral)
{
    // Parenthesis while calculating
    CParser parserParenthesis(R"code(struct S {
        int32 a = 10;
        int32 b = 5;
        int32 c = b*b;
        int64 e = 10;
        int16 f = a * (c + e);
    };)code");
    EXPECT_NO_THROW(parserParenthesis.Parse());
    const CVariableEntity* pParenthesisEntityResult = parserParenthesis.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pParenthesisEntityResult, nullptr);
    // Since 'f' is dynamic (based on variables), the result is 0.
    EXPECT_EQ(pParenthesisEntityResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 0);

    // Operators + - / * %
    CParser parserArithmetic(R"code(
        struct S {int64 f = 10 + 11 * 35 % 10 -2;};
    )code");
    EXPECT_NO_THROW(parserArithmetic.Parse());
    const CVariableEntity* pArithmeticResult = parserArithmetic.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pArithmeticResult, nullptr);
    EXPECT_EQ(pArithmeticResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int32_t>(), 13);

    // Operators << >> & | ~
    CParser parserBitManipulation(R"code(
        struct S {uint8 f = 2 << 2 | 7 >> 1 & (~ 120 & 7); };
    )code");
    EXPECT_NO_THROW(parserBitManipulation.Parse());
    const CVariableEntity* pBitManipulationResult = parserBitManipulation.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pBitManipulationResult, nullptr);
    EXPECT_EQ(pBitManipulationResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<int64_t>(), 11);

    // Operators && || < > <= >= == !=
    CParser parserCondition(R"code(struct S {
        boolean a = (20 || 10 && 1) < 2;
        boolean b = 50 + 7 >= 57;
        boolean c = 50 + 8 > 57;
        boolean d = 50 + 7 <= 57;
        boolean e = 10 != 0;
        int32 f = a + b + c + d + e;
    };)code");
    EXPECT_NO_THROW(parserCondition.Parse());
    const CVariableEntity* pConditionResult = parserCondition.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pConditionResult, nullptr);
    // Since 'f' is dynamic (based on variables), the result is 0.
    EXPECT_EQ(pConditionResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<uint8_t>(), 0);

    // Div by zero /
    EXPECT_THROW(CParser("struct S {long i = 0; struct S {unsigned long j = 10/i;};").Parse(), CCompileException);

    // Div by zero %
    EXPECT_THROW(CParser("struct S {long i = 0; struct S {unsigned long j = 10%i;};").Parse(), CCompileException);
}

TEST_F(CParserVarAssignmentTest, ParsingVarAssignmentOperatorsFloatingPoint)
{
    // Parenthesis while calculating
    CParser parserParenthesis(R"code(struct S {
        double a = 10;
        double b = 5;
        double c = b*b;
        double e = 10;
        double f = a * (c + e);
    };)code");
    EXPECT_NO_THROW(parserParenthesis.Parse());
    const CVariableEntity* pParenthesisEntityResult = parserParenthesis.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pParenthesisEntityResult, nullptr);
    // Since 'f' is dynamic (based on variables), the result is 0.0.
    EXPECT_EQ(pParenthesisEntityResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<float>(), 0.0);

    // Operators + - / * %
    CParser parserArithmetic(R"code(
        struct S {float f = 10 + 11 * 35 % 10 -2; };
    )code");
    EXPECT_NO_THROW(parserArithmetic.Parse());
    const CVariableEntity* pArithmeticResult = parserArithmetic.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pArithmeticResult, nullptr);
    EXPECT_EQ(pArithmeticResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<long double>(), 13);

    // Operators && || < > <= >= == !=
    CParser parserCondition(R"code(struct S {
        boolean a = (20.0 || 10.0 && 1.0) < 2.0;
        boolean b = 50.0 + 7.0 >= 57.0;
        boolean c = 50.0 + 8.0 > 57.0;
        boolean d = 50.0 + 7.0 <= 57.0;
        boolean e = 10.0 != 0.0;
        int32 f = a + b + c + d + e;
    };)code");
    EXPECT_NO_THROW(parserCondition.Parse());
    const CVariableEntity* pConditionResult = parserCondition.Root()->Find<CVariableEntity>("S::f");
    ASSERT_NE(pConditionResult, nullptr);
    // Since 'f' is dynamic (based on variables), the result is 0.
    EXPECT_EQ(pConditionResult->ValueRef()->Get<CSimpleTypeValueNode>()->Variant().Get<uint8_t>(), 0.0);

    // Div by zero /
    EXPECT_THROW(CParser("struct S {float i = 0; struct S {long double j = 10/i;};").Parse(), CCompileException);

    // Div by zero %
    EXPECT_THROW(CParser("struct S {double i = 0; struct S {float j = 10%i;};").Parse(), CCompileException);
}



// Scoped names... relative to a parent....

// Missing type - okay
// Invalid type
// Scoped name
//  Available
//  Unavailable
//  Wrong type
// Missing assignment operator - okay
// Expression
//  With every possible type
//  Missing expression - okay
//  Expression type automatic conversion
//  Expression type does not match
//  Uses identifier within same module
//  Uses identifier in parent/sub module
// Missing ';' - okay
// Additional data types
// Correct expression

