#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/exception_entity.cpp"

using CParserExceptionTest = CParserTest;

TEST_F(CParserExceptionTest, ForwardDeclaration)
{
    // exception <exception_identifier>; --> forward declaration
    EXPECT_TRUE(CParser("exception X1; exception X1 {};").Parse().Root()->Find("X1"));
    EXPECT_TRUE(CParser("exception X1 {}; exception X1;").Parse().Root()->Find("X1"));
    EXPECT_TRUE(CParser("exception X1; exception X1; exception X1; exception X1;").Parse().Root()->Find("X1"));
    EXPECT_TRUE(CParser("exception X1 {}; exception X1; exception X1; ").Parse().Root()->Find("X1"));
    EXPECT_THROW(CParser("exception X1 {}; exception X1; exception X1 {}; ").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception X1; struct X1 {};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, Definition)
{
    // exception <interface_identifier> {...}; --> exception definition
    EXPECT_TRUE(CParser("exception X2 {};").Parse().Root()->Find("X2"));
    EXPECT_TRUE(CParser("exception X2 { int32 i; };").Parse().Root()->Find("X2"));
    EXPECT_TRUE(CParser("exception X2 { int32 i; };").Parse().Root()->Find("X2::i"));
}

TEST_F(CParserExceptionTest, DefinitionWithInheritance)
{
    // exception <exception_identifier> : <base_exception,...> {...}; --> exception definition with inheritance
    EXPECT_TRUE(CParser("exception X3base {int32  i = 1;}; exception X3 : X3base {int32 j = 1;};").Parse().Root()->Find("X3::i"));
    EXPECT_THROW(CParser("exception X3base {int32  i = 1;}; struct S3 : X3base {int32 j = 1;};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, VariableDeclaration)
{
    // <struct_identifier> <decl_identifier>; --> exception variable declaration
    EXPECT_TRUE(CParser("exception X{struct S4 {int32  i;}; S4 s4Value;};").Parse().Root()->Find("X::s4Value"));
    // exception <struct_identifier> <decl_identifier>; --> exception variable declaration
    EXPECT_TRUE(CParser("exception X{struct S6 {int32  i;}; struct S6 s6Value;};").Parse().Root()->Find("X::s6Value"));
}

TEST_F(CParserExceptionTest, VarDeclarationAndAssignment)
{
    // <struct_identifier> <decl_identifier> = {...}; --> exception variable declaration and assignment
    EXPECT_TRUE(CParser("exception X{struct S5 {int32  i;}; S5 s5Value = {10};};").Parse().Root()->Find("X::s5Value"));
    EXPECT_TRUE(CParser("exception X{struct S5b {int32  i = 11;}; S5b s5bValue = {10};};").Parse().Root()->Find("X::s5bValue"));
    // exception <struct_identifier> <decl_identifier> = {...}; --> exception variable declaration and assignment
    EXPECT_TRUE(CParser("exception X{struct S7 {int32  i;}; struct S7 s7Value = {10};};").Parse().Root()->Find("X::s7Value"));
}

TEST_F(CParserExceptionTest, DefinitionAndVariableDeclaration)
{
    // exception <struct_identifier> {...} <decl_identifier>; --> exception definition and variable declaration
    EXPECT_TRUE(CParser("exception X{struct S8 {int32  i;} s8Value;};").Parse().Root()->Find("X::s8Value"));
}

TEST_F(CParserExceptionTest, DefinitionWithInheritanceAndVariableDeclaration)
{
    // exception <struct_identifier> : <base_struct,...> {...} <decl_identifier>;  --> exception definition with inheritance and variable declaration
    EXPECT_TRUE(CParser("exception X{struct S9base {int32  i;}; struct S9 : S9base {int32  j;} s9Value;};").Parse().Root()->Find("X::s9Value"));
}

TEST_F(CParserExceptionTest, DefinitionAndVariableDeclarationAndAssignment)
{
    // exception <struct_identifier> {...} <decl_identifier> = {...};  --> exception definition, variable declaration and assignment
    EXPECT_TRUE(CParser("exception X{struct S10 {int32  i;} s10Value = {10};};").Parse().Root()->Find("X::s10Value"));
}

TEST_F(CParserExceptionTest, DefinitionWithInheritanceAndVariableDeclarationAndAssignment)
{
    // exception <struct_identifier> : <base_struct,...> {...} <decl_identifier> = {...}; --> exception definition with inheritance, variable declaration and assignment
    EXPECT_TRUE(CParser("exception X{struct S11base {int32  i;}; struct S11 : S11base {int32  j;} s11Value = {{10}, 10};};").Parse().Root()->Find("X::s11Value"));
    EXPECT_TRUE(CParser("exception X{struct S11base {int32  i;}; struct S11 : S11base {} s11Value = {{10}};};").Parse().Root()->Find("X::s11Value"));
}

TEST_F(CParserExceptionTest, UnnamedDefinitionAndVariableDeclaration)
{
    // exception {...} <decl_identifier>; --> unnamed exception definition and variable declaration
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("exception X{struct {int32 i;} s12Value;};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, UnnamedDefinitionWithInheritanceAndVariableDeclaration)
{
    // exception : <base_struct,...> {...} <decl_identifier>; --> unnamed exception definition with inheritance and variable declaration
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("exception X{struct S13base {int32  i;}; struct : S13base {int32 j;} s13Value;};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, UnnamedDefinitionAndVariableDeclarationAndAssignment)
{
    // exception {...} <decl_identifier> = {...}; --> unnamed exception definition, variable declaration and assignment
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("exception X{struct {int32 i;} s14Value = {10};};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, UnnamedDefinitionWithInheritanceAndVariableDeclarationAndAssignment)
{
    // exception : <base_struct,...> {...} <decl_identifier> = {...};  --> unnamed exception definition with inheritance, variable declaration and assignment
    // NOTE 11.04.2025: Not allowed due to incompatibility in C++ standard
    EXPECT_THROW(CParser("exception X{struct S15base {int32  i;}; struct : S15base {int32 j;} s15Value = {{10}, 10};};").Parse(), CCompileException);
}

TEST_F(CParserExceptionTest, ComplexCombinationWithStructTypedefMultiDimmensionalArrayDeclarations)
{
    const char szCode[] = R"code(
        exception X
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
    CEntityPtr ptrVar = parser.Root()->Find("X::rgarray");
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
