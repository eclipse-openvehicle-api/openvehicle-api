#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/union_entity.cpp"

using CParserUnionTest = CParserTest;

TEST_F(CParserUnionTest, ForwardDeclaration)
{
    // type based union: union <union_identifier>; --> forward declaration
    EXPECT_TRUE(CParser("union U1; union U1 switch (int32) {};").Parse().Root()->Find("U1"));
    EXPECT_TRUE(CParser("union U1 switch (int32) {}; union U1;").Parse().Root()->Find("U1"));
    EXPECT_TRUE(CParser("union U1; union U1; union U1; union U1;").Parse().Root()->Find("U1"));
    EXPECT_TRUE(CParser("union U1 switch (int32) {}; union U1; union U1; ").Parse().Root()->Find("U1"));
    EXPECT_THROW(CParser("union U1 switch (int32) {}; union U1; union U1 switch (int32) {}; ").Parse(), CCompileException);

    // variable based union: struct {union <union_identifier>; --> forward declaration}
    EXPECT_TRUE(CParser("struct S { uint32 uiVal; union U1 switch (uiVal) {};};").Parse().Root()->Find("S::U1"));
    EXPECT_TRUE(CParser("struct S { union U1; uint32 uiVal; union U1 switch (uiVal) {};};").Parse().Root()->Find("S::U1"));
}

TEST_F(CParserUnionTest, Definition)
{
    // union <union_identifier> switch(<case_identifier>){...}; --> union definition
    EXPECT_TRUE(CParser("union U2 switch (int32) {};").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(boolean) { case true: int32 i; case false: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(boolean) { case true: int32 i; case false: int64 j; };").Parse().Root()->Find("U2::i"));
    EXPECT_TRUE(CParser("union U2 switch(int32) { case 1: int32 i; case 2: int64 j; default: float f; };").Parse().Root()->Find("U2::f"));
    EXPECT_TRUE(CParser("union U2 switch(int32) { default: float f; case 1: int32 i; case 2: int64 j; };").Parse().Root()->Find("U2::f"));
    EXPECT_THROW(CParser("union U2 switch(int32) { default: float f; case 1: int32 i; case 2: int64 j; default: double d; };").Parse(), CCompileException);
    EXPECT_THROW(CParser("union U2 switch(int32) { default: float f; case 1: int32 i; case 2: int64 j; default: double d; };").Parse(), CCompileException);
}

TEST_F(CParserUnionTest, VariableDeclaration)
{
    // <union_identifier> <decl_identifier>; --> union variable declaration
    EXPECT_TRUE(CParser("struct S{union U4 switch(int32){case 10: int32 i;}; U4 u4Value;};").Parse().Root()->Find("S::u4Value"));
    // union <union_identifier> <decl_identifier>; --> union variable declaration
    EXPECT_TRUE(CParser("struct S{union U6 switch(int32){case 10: int32 i;}; union U6 u6Value;};").Parse().Root()->Find("S::u6Value"));
}

TEST_F(CParserUnionTest, IntegralSwitchType)
{
    // Use any type of integral value in case
    EXPECT_TRUE(CParser("union U2 switch(boolean) { case true: int32 i; case false: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(int8) { case 10: int32 i; case -10: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(int16) { case 10: int32 i; case -10: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(int32) { case 10: int32 i; case -10: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(int64) { case 10: int32 i; case -10: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(uint8) { case 10: int32 i; case 20: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(uint16) { case 10: int32 i; case 20: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(uint32) { case 10: int32 i; case 20: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(uint64) { case 10: int32 i; case 20: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(char) { case 'a': int32 i; case 'b': int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(char16) { case u'a': int32 i; case u'b': int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(char32) { case U'a': int32 i; case U'A': int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("union U2 switch(wchar) { case L'a': int32 i; case L'b': int64 j; };").Parse().Root()->Find("U2"));

    // Use other type of value in case
    EXPECT_THROW(CParser("union U2 switch(float) { case 0.0: int32 i; case 1.1: int64 j; };").Parse(), CCompileException);
}

TEST_F(CParserUnionTest, IntegralTypeMixing)
{
    // Use const values in case
    EXPECT_TRUE(CParser("const int32 iVal1 = 10, iVal2 = 20; union U2 switch(int32) { case iVal1: int32 i; case iVal2: int64 j; };").Parse().Root()->Find("U2"));

    // Use calculated const value in case
    EXPECT_TRUE(CParser("const int32 iVal1 = 10 + 10, iVal2 = 30; union U2 switch(int32) { case iVal1: int32 i; case iVal2: int64 j; };").Parse().Root()->Find("U2"));

    // Use calculated value in case
    EXPECT_TRUE(CParser("const int32 iVal1 = 10; union U2 switch(int32) { case iVal1: int32 i; case iVal1 + 1: int64 j; };").Parse().Root()->Find("U2"));

    // Use declaration of const values following the switch case
    EXPECT_TRUE(CParser("union U2 switch(int32) { case iVal1: int32 i; case iVal2: int64 j; }; const int32 iVal1 = 10, iVal2 = 20;").Parse().Root()->Find("U2"));

    // Use typedef value in case
    EXPECT_TRUE(CParser("typedef int32 TCaseInt; const TCaseInt iVal1 = 10, iVal2 = 20; union U2 switch(TCaseInt) { case iVal1: int32 i; case iVal2: int64 j; };").Parse().Root()->Find("U2"));
    EXPECT_TRUE(CParser("typedef int32 TCaseInt; const int32 iVal1 = 10, iVal2 = 20; union U2 switch(TCaseInt) { case iVal1: int32 i; case iVal2: int64 j; };").Parse().Root()->Find("U2"));

    // Use mixed const values in case
    EXPECT_TRUE(CParser("const int8 iVal1 = 10; const int64 iVal2 = 20; union U2 switch(int32) { case iVal1: int32 i; case iVal2: int64 j; };").Parse().Root()->Find("U2"));
}

TEST_F(CParserUnionTest, EnumSwitchtype)
{
    // Use enum values in case
    EXPECT_TRUE(CParser("enum EValues {ten, twenty}; union U2 switch(EValues) { case ten: int32 i; case twenty: int64 j; };").Parse().Root()->Find("U2"));

    // Use definition of enum following the switch case
    EXPECT_THROW(CParser("union U2 switch(EValues) { case ten: int32 i; case twenty: int64 j; }; enum EValues {ten, twenty};").Parse(), CCompileException);

    EXPECT_TRUE(CParser("enum EValues; union U2 switch(EValues) { case ten: int32 i; case twenty: int64 j; }; enum EValues {ten, twenty};").Parse().Root()->Find("U2"));
}

TEST_F(CParserUnionTest, DuplicateCaseLabel)
{
    // Use values in case
    EXPECT_THROW(CParser("union U2 switch(int32) { case 10: int32 i; case 10: int64 j; };").Parse(), CCompileException);

    // Use const values in case
    EXPECT_THROW(CParser("const int32 iVal1 = 10, iVal2 = 10; union U2 switch(int32) { case iVal1: int32 i; case iVal2: int64 j; };").Parse(), CCompileException);

    // Use enum values in case
    EXPECT_THROW(CParser("enum EValues {ten, twenty}; union U2 switch(EValues) { case ten: int32 i; case ten: int64 j; };").Parse(), CCompileException);

    // Duplicate default
    EXPECT_THROW(CParser("union U2 switch(int32) { default: double d; case 10: int32 i; case 20: int64 j; default: float f; };").Parse(), CCompileException);
}

TEST_F(CParserUnionTest, DuplicateVariables)
{
    EXPECT_THROW(CParser("union U2 switch(int32) { case 10: int32 i; case 20: int64 j; case 30: int32 i; };").Parse(), CCompileException);
    EXPECT_THROW(CParser("union U2 switch(int32) { case 10: int32 i; case 20: int64 j; case 30: int64 i; };").Parse(), CCompileException);
    EXPECT_THROW(CParser("union U2 switch(int32) { case 10: int32 i; case 20: int64 j; default: int32 i; };").Parse(), CCompileException);
    EXPECT_THROW(CParser("union U2 switch(int32) { case 10: int32 i; case 20: int64 j; default: int64 i; };").Parse(), CCompileException);
}

TEST_F(CParserUnionTest, MemberSwitchLabelType)
{
    // Use const value in switch label
    EXPECT_THROW(CParser("const int32 iVal = 10; union U2 switch(iVal) { case 10: int32 i; case 20: int64 j; };").Parse(), CCompileException);

    // Use value in switch label
    EXPECT_TRUE(CParser("struct S {int32 iVal = 10; union U2 switch(iVal) { case 10: int32 i; case 20: int64 j; } u2Var;};").Parse().Root()->Find("S::u2Var"));

    // Use succeeding declared value in switch label
    EXPECT_TRUE(CParser("struct S {union U2 switch(iVal) { case 10: int32 i; case 20: int64 j; }u2Var; int32 iVal = 10;};").Parse().Root()->Find("S::u2Var"));

    // Use enum values in switch label
    EXPECT_TRUE(CParser("struct S {enum EValues {ten, twenty}; union U2 switch(EValues) { case ten: int32 i; case twenty: int64 j; } u2Var;};").Parse().Root()->Find("S::u2Var"));
}

TEST_F(CParserUnionTest, UnnamedUnionDefinition)
{
    // Declaration of unnamed union at global scope is not allowed.

    // Use type in switch label
    EXPECT_THROW(CParser(R"code(
union switch(int32)
{
case 10:
    int32 i;
case 20:
    int64 j;
} u1;
};)code").Parse(), CCompileException);

    // Declaration of unnamed union within struct.

    // Use type in switch label
    EXPECT_TRUE(CParser(R"code(
struct S
{
    union switch(int32)
    {
    case 10:
        int32 i;
    case 20:
        int64 j;
    } u2;
};)code").Parse().Root()->Find("S::u2"));

    // Use value in switch label
    EXPECT_TRUE(CParser(R"code(
struct S
{
    int32 iVal = 10;
    union switch(iVal)
    {
    case 10:
        int32 i;
    case 20:
        int64 j;
    } u2;
};)code").Parse().Root()->Find("S::u2"));
}

TEST_F(CParserUnionTest, AnonymousUnionDeclaration)
{
    // Declaration of unnamed union within struct.

    // Use type in switch label
    CParser parser1(R"code(
struct S
{
    union switch(int32)
    {
    case 10:
        int32 i;
    case 20:
        int64 j;
    };
};)code");
    EXPECT_NO_THROW(parser1.Parse());
    EXPECT_TRUE(parser1.Root()->Find("S::i"));
    EXPECT_TRUE(parser1.Root()->Find("S::j"));

    // Use value in switch label
    CParser parser2(R"code(
struct S
{
    int32 iVal = 10;
    union switch(iVal)
    {
    case 10:
        int32 i;
    case 20:
        int64 j;
    };
};)code");
    EXPECT_NO_THROW(parser2.Parse());
    EXPECT_TRUE(parser2.Root()->Find("S::iVal"));
    EXPECT_TRUE(parser2.Root()->Find("S::i"));
    EXPECT_TRUE(parser2.Root()->Find("S::j"));
}

// Disable this struct until support for anonymous declarations are implemented: PBI #397894
TEST_F(CParserUnionTest, DISABLED_AnonymousDeclaration)
{
    // Use type in switch label on root level
    EXPECT_THROW(CParser("union switch(int32) { case 10: int32 i; case 20: int64 j; };").Parse(), CCompileException);

    // Use type in switch label
    EXPECT_TRUE(CParser("struct S {union switch(int32) { case 10: int32 i; case 20: int64 j; };};").Parse().Root()->Find("S::i"));

    // Use type in switch label
    char szCodeTypeLabel[] = R"code(
struct S
{
    union switch (int32)
    {
    case 10:
        union switch (int32)
        {
            case 1: int32 a;
            case 2: int32 b;
            case 3: int32 c;
        };
    case 20:
        struct
        {
            int32 d;
            int32 e;
            int32 f;
        };
    case 30:
        int32 g;
    };
};
)code";
    CParser parserTypeLabel(szCodeTypeLabel);
    parserTypeLabel.Parse();
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::a"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::b"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::c"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::d"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::e"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::f"));
    EXPECT_TRUE(parserTypeLabel.Root()->Find("S::g"));

    // Use value in switch label
    EXPECT_TRUE(CParser("struct S {int32 iVal = 10; union switch(iVal) { case 10: int32 i; case 20: int64 j; };};").Parse().Root()->Find("S::i"));
    EXPECT_TRUE(CParser("struct S {union switch(iVal) { case 10: int32 i; case 20: int64 j; }; int32 iVal = 10;};").Parse().Root()->Find("S::i"));
    EXPECT_THROW(CParser("const int32 iVal = 10; struct S {union switch(iVal) { case 10: int32 i; case 20: int64 j; };};").Parse(), CCompileException);

    // Use value in switch label
    char szCodeVarLabel[] = R"code(
struct S
{
    int32 iVal = 10;
    int32 iVal2 = 1;
    union switch (iVal)
    {
    case 10:
        union switch (iVal2)
        {
            case 1: int32 a;
            case 2: int32 b;
            case 3: int32 c;
        };
    case 20:
        struct
        {
            int32 d;
            int32 e;
            int32 f;
        };
    case 30:
        int32 g;
    };
};
struct S2
{
    S sValue;
};
)code";
    CParser parserVarLabel(szCodeVarLabel);
    parserVarLabel.Parse();
    EXPECT_TRUE(parserVarLabel.Root()->Find("S"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::iVal"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::iVal2"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::a"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::b"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::c"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::d"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::e"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::f"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S::g"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.iVal"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.iVal2"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.a"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.b"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.c"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.d"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.e"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.f"));
    EXPECT_TRUE(parserVarLabel.Root()->Find("S2::sValue.g"));
}

/*

Test:
- Union with nested anonymous/named/assigned struct/union in case - not allowed following C++ rules
- Struct with anonymous union with/without switch variable - allowed
- Struct with multiple anonymous unions with/without switch variable - allowed
- Struct with anonymous struct with declaration - allowed
- Struct with anonymous union with/without switch variable with declaration - allowed
- Struct with two union definition, second using the declaration of the first definition in case statement - allowed
*/


///**
// * @brief Union with complex type as a variable.
// */
union UComplex1
{
    ~UComplex1() {}
    int32_t i;        ///< Simple type
    struct SComplex
    {
        sdv::u8string    ss1;     ///< String var
        int8_t        i8;     ///< int var
    } sComplex;     ///< Complex type
    sdv::u8string ss2;    ///< String type
};

// ///**
// // * @brief Union with complex type as a variable.
// // */
// union UComplex2
// {
//     ~UComplex2() {}
//     int32_t i;        ///< Simple type
//     struct
//     {
//         sdv::u8string    ss1;     ///< String var
//         int8_t        i8;     ///< int var
//     };     ///< Complex type
//     sdv::u8string ss2;    ///< String type
// };

