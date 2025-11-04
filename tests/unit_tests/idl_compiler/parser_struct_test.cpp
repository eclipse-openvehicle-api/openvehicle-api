#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/definition_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/struct_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/variable_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/declaration_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.h"

using CParserStructTest = CParserTest;

TEST_F(CParserStructTest, ForwardDeclaration)
{
    // struct <struct_identifier>; --> forward declaration
    EXPECT_TRUE(CParser("struct S1; struct S1 {};").Parse().Root()->Find("S1"));
    EXPECT_TRUE(CParser("struct S1 {}; struct S1;").Parse().Root()->Find("S1"));
    EXPECT_TRUE(CParser("struct S1; struct S1; struct S1; struct S1;").Parse().Root()->Find("S1"));
    EXPECT_TRUE(CParser("struct S1 {}; struct S1; struct S1; ").Parse().Root()->Find("S1"));
    EXPECT_THROW(CParser("struct S1 {}; struct S1; struct S1 {}; ").Parse(), CCompileException);
}

TEST_F(CParserStructTest, Definition)
{
    // struct <struct_identifier> {...}; --> struct definition
    EXPECT_TRUE(CParser("struct S2 {};").Parse().Root()->Find("S2"));
    EXPECT_TRUE(CParser("struct S2 { int32 i; };").Parse().Root()->Find("S2"));
    EXPECT_TRUE(CParser("struct S2 { int32 i; };").Parse().Root()->Find("S2::i"));
}

TEST_F(CParserStructTest, DefinitionWithInheritance)
{
    // struct <struct_identifier> : <base_struct,...> {...}; --> struct definition with inheritance
    EXPECT_TRUE(CParser("struct S3base {int32 i;}; struct S3 : S3base {int32 j;};").Parse().Root()->Find("S3"));
    EXPECT_TRUE(CParser("struct S3base_1 {int32 i;}; struct S3base_2 {int32 i;}; struct S3_12 : S3base_1, S3base_2 {int32 j;};").Parse().Root()->Find("S3_12"));
    EXPECT_TRUE(CParser("struct S3base_1 {}; struct S3base_2 : S3base_1 {}; struct S3_12 : S3base_2 {};").Parse().Root()->Find("S3_12"));
    EXPECT_TRUE(CParser("struct S3base_1 {}; struct S3base_2 : S3base_1 {}; struct S3_12 : S3base_1, S3base_2 {};").Parse().Root()->Find("S3_12"));
    // S3base is not defined
    EXPECT_THROW(CParser("struct S3 : S3base {};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S3base; struct S3 : S3base {};").Parse(), CCompileException);
    // Invalid base struct
    EXPECT_THROW(CParser("const uint32_t S1 = 10; struct S2 : S1 {};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, DuplicateNameThroughInheritance)
{
    EXPECT_THROW(CParser("struct S1 {const char c = 'C';}; struct S2 : S1 {const int32  c = 10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S1; struct S1 {const char c = 'C';}; struct S2 : S1 {const int32  c = 10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S1 {const char c = 'C';}; struct S2 : S1 {}; struct S3 : S2 {const int32  c = 10;};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, DeclarationsAndTypedefs)
{
    const char szCode[] = R"code(
struct S
{
    struct S1
    {
        int32 a;
        int32 b;
        int32 c;
    };
    typedef S1 TS1;
    struct S2 : TS1
    {
        int32 d;
        int32 e;
        int32 f;
    };
    typedef S2 TS2;
    struct S3
    {
        TS1 ts1Value;
        TS2 ts2Value;
    };
    typedef S3 TS3;
    TS3 ts3Value;
    typedef TS3 TS4;
    TS4 ts4Value;
};
)code";
    CParser parser(szCode);
    parser.Parse();
    EXPECT_TRUE(parser.Root()->Find("S::S1"));
    EXPECT_TRUE(parser.Root()->Find("S::S1::a"));
    EXPECT_TRUE(parser.Root()->Find("S::S1::b"));
    EXPECT_TRUE(parser.Root()->Find("S::S1::c"));
    EXPECT_TRUE(parser.Root()->Find("S::TS1"));
    EXPECT_TRUE(parser.Root()->Find("S::TS1::a"));
    EXPECT_TRUE(parser.Root()->Find("S::TS1::b"));
    EXPECT_TRUE(parser.Root()->Find("S::TS1::c"));
    EXPECT_TRUE(parser.Root()->Find("S::S2"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::a"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::b"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::c"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::d"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::e"));
    EXPECT_TRUE(parser.Root()->Find("S::S2::f"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::a"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::b"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::c"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::d"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::e"));
    EXPECT_TRUE(parser.Root()->Find("S::TS2::f"));
    EXPECT_TRUE(parser.Root()->Find("S::S3"));
    EXPECT_TRUE(parser.Root()->Find("S::TS3"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts1Value"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts1Value.a"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts1Value.b"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts1Value.c"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.a"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.b"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.c"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.d"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.e"));
    EXPECT_TRUE(parser.Root()->Find("S::ts3Value.ts2Value.f"));
    EXPECT_TRUE(parser.Root()->Find("S::TS4"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts1Value"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts1Value.a"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts1Value.b"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts1Value.c"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.a"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.b"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.c"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.d"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.e"));
    EXPECT_TRUE(parser.Root()->Find("S::ts4Value.ts2Value.f"));
}

TEST_F(CParserStructTest, VariableDeclaration)
{
    // <struct_identifier> <decl_identifier>; --> struct variable declaration
    EXPECT_TRUE(CParser("struct S{struct S4 {int32 i;}; S4 s4Value;};").Parse().Root()->Find("S::s4Value"));
    EXPECT_TRUE(CParser("struct S{struct S4 {int32 i;}; S4 s4Value;};").Parse().Root()->Find("S::s4Value.i"));
    EXPECT_TRUE(CParser("struct S{struct S4 {int32 i;}; typedef S4 S5; S5 s5Value;};").Parse().Root()->Find("S::s5Value.i"));
    EXPECT_TRUE(CParser("struct S{struct S4 {struct S5{int32 i;} s5Value;}; typedef S4 S6; S6 s6Value;};").Parse().Root()->Find("S::s6Value.s5Value.i"));
    EXPECT_TRUE(CParser("struct S{struct S4 {struct S5{int32 i;} s5Value;}; typedef S4 S6; S6 s6Value;}; const S sValue = {{{10}}};").Parse().Root()->Find("sValue.s6Value.s5Value.i"));
    // struct <struct_identifier> <decl_identifier>; --> struct variable declaration
    EXPECT_TRUE(CParser("struct S{struct S6 {int32 i;}; struct S6 s6Value;};").Parse().Root()->Find("S::s6Value"));
}

TEST_F(CParserStructTest, VarDeclarationAndAssignment)
{
    // <struct_identifier> <decl_identifier> = {...}; --> struct variable declaration and assignment
    EXPECT_TRUE(CParser("struct S{struct S5 {int32 i;}; S5 s5Value = {10};};").Parse().Root()->Find("S::s5Value"));
    EXPECT_TRUE(CParser("struct S{struct S5b {int32 i = 11;}; S5b s5bValue = {10};};").Parse().Root()->Find("S::s5bValue"));
    // struct <struct_identifier> <decl_identifier> = {...}; --> struct variable declaration and assignment
    EXPECT_TRUE(CParser("struct S{struct S7 {int32 i;}; struct S7 s7Value = {10};};").Parse().Root()->Find("S::s7Value"));
}

TEST_F(CParserStructTest, VarDeclarationAndAssignmentOfStructDecl)
{
    EXPECT_EQ(CParser("struct S1{int32 i;}; struct S2{ S1 sValue1 = {10}; S1 sValue2 = sValue1; };").Parse().Root()->FindValueVariant("S2::sValue2.i").Get<int32_t>(), 10);
    EXPECT_EQ(CParser("struct S1{int32 i[2];}; struct S2{ S1 sValue1 = {{10, 20}}; S1 sValue2 = sValue1; };").Parse().Root()->FindValueVariant("S2::sValue2.i[1]").Get<int32_t>(), 20);
    EXPECT_EQ(CParser("struct S1{int32 i[2][2];}; struct S2{ S1 sValue1 = {{{10, 20}, {30, 40}}}; S1 sValue2 = sValue1; };").Parse().Root()->FindValueVariant("S2::sValue2.i[1][1]").Get<int32_t>(), 40);
    EXPECT_EQ(CParser("struct S1{typedef int32 ti[2]; ti i[2];}; struct S2{ S1 sValue1 = {{{10, 20}, {30, 40}}}; S1 sValue2 = sValue1; };").Parse().Root()->FindValueVariant("S2::sValue2.i[1][1]").Get<int32_t>(), 40);
    EXPECT_EQ(CParser("typedef struct tagS1 {int32 i;} S1; struct S2{ S1 sValue1 = {10}; S1 sValue2 = sValue1; };").Parse().Root()->FindValueVariant("S2::sValue2.i").Get<int32_t>(), 10);
    EXPECT_EQ(CParser("typedef struct tagS1 {int32 i;} S1; struct S2{ S1 sValue1[2] = {{10}, {20}}; S1 sValue2 = sValue1[1]; };").Parse().Root()->FindValueVariant("S2::sValue2.i").Get<int32_t>(), 20);
    EXPECT_EQ(CParser("typedef struct tagS1 {int32 i;} S1; const int32 index = 2; struct S2{ S1 sValue1[index] = {{10}, {20}}; S1 sValue2 = sValue1[1]; };").Parse().Root()->FindValueVariant("S2::sValue2.i").Get<int32_t>(), 20);
    EXPECT_EQ(CParser("typedef struct tagS1 {int32 i;} S1; struct S2{ S1 sValue1[2] = {{10}, {20}};}; struct S3{S1 sValue2 = S2.sValue1[1]; };").Parse().Root()->FindValueVariant("S3::sValue2.i").Get<int32_t>(), 20);
}

TEST_F(CParserStructTest, DefinitionAndVariableDeclaration)
{
    // struct <struct_identifier> {...} <decl_identifier>; --> struct definition and variable declaration
    EXPECT_TRUE(CParser("struct S{struct S8 {int32 i;} s8Value;};").Parse().Root()->Find("S::s8Value"));
}

TEST_F(CParserStructTest, DefinitionWithInheritanceAndVariableDeclaration)
{
    // struct <struct_identifier> : <base_struct,...> {...} <decl_identifier>;  --> struct definition with inheritance and variable declaration
    EXPECT_TRUE(CParser("struct S{struct S9base {int32 i;}; struct S9 : S9base {int32  j;} s9Value;};").Parse().Root()->Find("S::s9Value"));
}

TEST_F(CParserStructTest, DefinitionAndVariableDeclarationAndAssignment)
{
    // struct <struct_identifier> {...} <decl_identifier> = {...};  --> struct definition, variable declaration and assignment
    EXPECT_TRUE(CParser("struct S{struct S10 {int32 i;} s10Value = {10};};").Parse().Root()->Find("S::s10Value"));
}

TEST_F(CParserStructTest, DefinitionWithInheritanceAndVariableDeclarationAndAssignment)
{
    // struct <struct_identifier> : <base_struct,...> {...} <decl_identifier> = {...}; --> struct definition with inheritance, variable declaration and assignment
    EXPECT_TRUE(CParser("struct S{struct S11base {int32 i;}; struct S11 : S11base {int32  j;} s11Value = {{10}, 10};};").Parse().Root()->Find("S::s11Value"));
    EXPECT_TRUE(CParser("struct S{struct S11base {int32 i;}; struct S11 : S11base {} s11Value = {{10}};};").Parse().Root()->Find("S::s11Value"));
}

TEST_F(CParserStructTest, UnnamedDefinitionAndVariableDeclaration)
{
    // struct {...} <decl_identifier>; --> unnamed struct definition and variable declaration
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("struct S{struct {int32 i;} s12Value;};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, UnnamedDefinitionWithInheritanceAndVariableDeclaration)
{
    // struct : <base_struct,...> {...} <decl_identifier>; --> unnamed struct definition with inheritance and variable declaration
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("struct S{struct S13base {int32 i;}; struct : S13base {int32 j;} s13Value;};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, UnnamedDefinitionAndVariableDeclarationAndAssignment)
{
    // struct {...} <decl_identifier> = {...}; --> unnamed struct definition, variable declaration and assignment
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("struct S{struct {int32 i;} s14Value = {10};};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, UnnamedDefinitionWithInheritanceAndVariableDeclarationAndAssignment)
{
    // struct : <base_struct,...> {...} <decl_identifier> = {...};  --> unnamed struct definition with inheritance, variable declaration and assignment
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("struct S{struct S15base {int32 i;}; struct : S15base {int32 j;} s15Value = {{10}, 10};};").Parse(), CCompileException);
}

TEST_F(CParserStructTest, ComplexCombinationWithStructTypedefMultiDimmensionalArrayDeclarations)
{
    const char szCode[] = R"code(
        struct S
        {
            const int32 a = 2;
            typedef int32 intarray[a];
            typedef intarray initintarray[3][1];
            initintarray rgarray[4] = {{{{1,2}}, {{3,4}}, {{5,6}}},
                {{{7,8}},{{9,10}}, {{a,b}}},
                {{{13,14}}, {{15,16}}, {{17,18}}},
                {{{19,20}},{{21,22}}, {{23,24}}} };
            const int32 b = 4;
        };
    )code";

    CParser parser(szCode);
    EXPECT_NO_THROW(parser.Parse());
    CEntityPtr ptrVar = parser.Root()->Find("S::rgarray");
    ASSERT_TRUE(ptrVar);
    CDeclarationEntity* pVar = ptrVar->Get<CDeclarationEntity>();
    ASSERT_NE(pVar, nullptr);
    CValueNodePtr ptrVal = ptrVar->ValueRef();
    ASSERT_TRUE(ptrVal);
    CArrayValueNode* pInitIntArrayVal = ptrVal->Get<CArrayValueNode>();
    ASSERT_NE(pInitIntArrayVal, nullptr);
    EXPECT_EQ(pInitIntArrayVal->GetSize(), 4);
    CValueNodePtr ptrIntArrayVal = (*pInitIntArrayVal)[3];
    ASSERT_TRUE(ptrIntArrayVal);
    CArrayValueNode* pIntArrayVal = ptrIntArrayVal->Get<CArrayValueNode>();
    ASSERT_NE(pIntArrayVal, nullptr);
    EXPECT_EQ(pIntArrayVal->GetSize(), 3);
    CValueNodePtr ptrIntArrayVal_2 = (*pIntArrayVal)[2];
    ASSERT_TRUE(ptrIntArrayVal_2);
    CArrayValueNode* pIntArrayVal_2 = ptrIntArrayVal_2->Get<CArrayValueNode>();
    ASSERT_NE(pIntArrayVal_2, nullptr);
    EXPECT_EQ(pIntArrayVal_2->GetSize(), 1);
    CValueNodePtr ptrIntArrayVal_3 = (*pIntArrayVal_2)[0];
    ASSERT_TRUE(ptrIntArrayVal_3);
    CArrayValueNode* pIntArrayVal_3 = ptrIntArrayVal_3->Get<CArrayValueNode>();
    ASSERT_NE(pIntArrayVal_3, nullptr);
    EXPECT_EQ(pIntArrayVal_3->GetSize(), 2);
    CValueNodePtr ptrIntVal = (*pIntArrayVal_3)[1];
    ASSERT_TRUE(ptrIntVal);
    CSimpleTypeValueNode* pIntVal = ptrIntVal->Get<CSimpleTypeValueNode>();
    ASSERT_NE(pIntVal, nullptr);
    EXPECT_EQ(pIntVal->Variant().Get<int32_t>(), 24);
}

TEST_F(CParserStructTest, StringAssignmentInStruct)
{
    // Tests bug fix: #380904
    const char szCode[] = R"code(
    struct SStringTest
    {
        string ssVal = "hello";
        u8string ss8Val = u8"hello";
    };
    )code";

    CParser parser(szCode);
    EXPECT_NO_THROW(parser.Parse());
}

