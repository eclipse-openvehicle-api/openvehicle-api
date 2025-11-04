#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/interface_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/attribute_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/operation_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/parameter_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/exception_entity.h"

using CParserInterfaceTest = CParserTest;

TEST_F(CParserInterfaceTest, ForwardDeclaration)
{
    // interface <interface_identifier>; --> forward declaration
    EXPECT_TRUE(CParser("interface I1; interface I1 {};").Parse().Root()->Find("I1"));
    EXPECT_TRUE(CParser("interface I1 {}; interface I1;").Parse().Root()->Find("I1"));
    EXPECT_TRUE(CParser("interface I1; interface I1; interface I1; interface I1;").Parse().Root()->Find("I1"));
    EXPECT_TRUE(CParser("interface I1 {}; interface I1; interface I1; ").Parse().Root()->Find("I1"));
    EXPECT_THROW(CParser("interface I1 {}; interface I1; interface I1 {}; ").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, Definition)
{
    // interface <interface_identifier> {...}; --> interface definition
    EXPECT_TRUE(CParser("interface I2 {};").Parse().Root()->Find("I2"));
    EXPECT_TRUE(CParser("interface I2 { const int32 i = 1; };").Parse().Root()->Find("I2"));
    EXPECT_TRUE(CParser("interface I2 { const int32 i = 1; };").Parse().Root()->Find("I2::i"));
}

TEST_F(CParserInterfaceTest, DefinitionWithInheritance)
{
    // interface <interface_identifier> : <base_interface,...> {...}; --> interface definition with inheritance
    EXPECT_TRUE(CParser("interface I3base {const int32  i = 1;}; interface I3 : I3base {const int32 j = 1;};").Parse().Root()->Find("I3"));
    EXPECT_TRUE(CParser("interface I3base_1 {const int32  i = 1;}; interface I3base_2 {const int32  i = 2;}; interface I3_12 : I3base_1, I3base_2 {const int32 j = 1;};").Parse().Root()->Find("I3_12"));
    EXPECT_TRUE(CParser("interface I3base_1 {}; interface I3base_2 : I3base_1 {}; interface I3_12 : I3base_2 {};").Parse().Root()->Find("I3_12"));
    EXPECT_TRUE(CParser("interface I3base_1 {}; interface I3base_2 : I3base_1 {}; interface I3_12 : I3base_1, I3base_2 {};").Parse().Root()->Find("I3_12"));
    // I3base is not defined
    EXPECT_THROW(CParser("interface I3 : I3base {};").Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I3base; interface I3 : I3base {};").Parse(), CCompileException);
    // Invalid base interface
    EXPECT_THROW(CParser("const uint32_t I1 = 10; interface I2 : I1 {};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, DuplicateNameThroughInheritance)
{
    EXPECT_THROW(CParser("interface I1 {const char c = 'C';}; interface I2 : I1 {const int32  c = 10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I1; interface I1 {const char c = 'C';}; interface I2 : I1 {const int32  c = 10;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I1 {const char c = 'C';}; interface I2 : I1 {}; interface I3 : I2 {const int32  c = 10;};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, SimpleTypeAttributeDefinition)
{
    EXPECT_TRUE(CParser("interface I{attribute int8 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute int16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute int32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute int64 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute uint8 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute uint16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute uint32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute uint64 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute octet attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute short attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute long long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute unsigned short attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute unsigned long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute unsigned long long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute char attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute wchar attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute char16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute char32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute float attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute double attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute long double attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute fixed attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute u8string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute u16string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute u32string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{attribute wstring attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_THROW(CParser("interface I{attribute void attr;};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, ComplexTypeAttributeDefinition)
{
    EXPECT_TRUE(CParser("struct S{}; interface I{attribute S attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("struct S{}; interface I{attribute struct S attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("typedef struct tagS {} S; interface I{attribute S attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("typedef int32 TI; interface I{attribute TI attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("typedef int32 TI[10]; interface I{attribute TI attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_THROW(CParser("typedef int32 TI[]; interface I{attribute TI attr;};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, AttributeWithExceptionDefinition)
{
    EXPECT_TRUE(CParser("exception S1{}; interface I{attribute int32 attr raises(S1);};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr getraises(S1) setraises(S2);};").Parse().Root()->Find("I::attr"));
    EXPECT_NO_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr raises(S1) setraises(S2);};").Parse());
    EXPECT_NO_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr raises(S1) getraises(S2);};").Parse());
    EXPECT_NO_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr setraises(S1) raises(S2);};").Parse());
    EXPECT_NO_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr getraises(S1) raises(S2);};").Parse());
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr setraises(S1) setraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr getraises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{attribute int32 attr raises(S1) raises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I{attribute int32 attr raises(S1);};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, SimpleTypeReadOnlyAttributeDefinition)
{
    EXPECT_TRUE(CParser("interface I{readonly attribute int8 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute int16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute int32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute int64 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute uint8 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute uint16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute uint32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute uint64 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute octet attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute short attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute long long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute unsigned short attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute unsigned long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute unsigned long long attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute char attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute wchar attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute char16 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute char32 attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute float attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute double attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute long double attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute fixed attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute u8string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute u16string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute u32string attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("interface I{readonly attribute wstring attr;};").Parse().Root()->Find("I::attr"));
    EXPECT_THROW(CParser("interface I{readonly attribute void attr;};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, ReadOnlyAttributeWithExceptionDefinition)
{
    EXPECT_TRUE(CParser("exception S1{}; interface I{readonly attribute int32 attr raises(S1);};").Parse().Root()->Find("I::attr"));
    EXPECT_TRUE(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr getraises(S1);};").Parse().Root()->Find("I::attr"));
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr setraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr getraises(S1) setraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr raises(S1) setraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr raises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr setraises(S1) raises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr getraises(S1) raises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr setraises(S1) setraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr getraises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{readonly attribute int32 attr raises(S1) raises(S2);};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, MultipleAttributeDefinition)
{
    EXPECT_TRUE(CParser("interface I{attribute int32 attr1, attr2, attr3;};").Parse().Root()->Find("I::attr3"));
    EXPECT_TRUE(CParser("exception S1{}; exception S2{}; exception S3; interface I{attribute int32 attr1 raises(S1), attr2 getraises(S1) setraises(S2), attr3;};").Parse().Root()->Find("I::attr3"));
}

TEST_F(CParserInterfaceTest, SimpleTypeOperationDefinition)
{
    EXPECT_TRUE(CParser("interface I{void func(in int8 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in int16 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in int32 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in int64 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in uint8 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in uint16 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in uint32 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in uint64 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in octet var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in short var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in long var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in long long var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in unsigned short var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in unsigned long var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in unsigned long long var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in char var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in wchar var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in char16 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in char32 var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in float var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in double var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in long double var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in fixed var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in string var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in u8string var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in u16string var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in u32string var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func(in wstring var);};").Parse().Root()->Find("I::func"));
    EXPECT_THROW(CParser("interface I{void func(in void);};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, ReturnSimpleTypeOperationDefinition)
{
    EXPECT_TRUE(CParser("interface I{int8 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int16 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int32 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int64 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint8 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint16 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint32 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint64 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{octet func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{short func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long long func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned short func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned long func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned long long func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{wchar func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char16 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char32 func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{float func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{double func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long double func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{fixed func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{string func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u8string func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u16string func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u32string func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{wstring func();};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func();};").Parse().Root()->Find("I::func"));
}

TEST_F(CParserInterfaceTest, ReturnSimpleTypeConstOperationDefinition)
{
    EXPECT_TRUE(CParser("interface I{int16 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int32 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int64 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint8 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint16 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint32 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{uint64 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{octet func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{short func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long long func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned short func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned long func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{int8 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{unsigned long long func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{wchar func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char16 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{char32 func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{float func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{double func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{long double func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{fixed func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{string func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u8string func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u16string func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{u32string func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{wstring func() const;};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface I{void func() const;};").Parse().Root()->Find("I::func"));
}

TEST_F(CParserInterfaceTest, SimpleTypeOperationMegaDefinition)
{
    EXPECT_TRUE(CParser("interface I{void func(in int8 var1, in int16 var2, in int32 var3, in int64 var4, in uint8 var5,"
        " in uint16 var6, in uint32 var7, in uint64 var8, in octet var9, in short var10, in long var11, in long long var12,"
        " in unsigned short var13, in unsigned long var14, in unsigned long long var15, in char var16, in wchar var17,"
        " in char16 var18, in char32 var19, in float var20, in double var21, in long double var22, in fixed var23,"
        " in string var24, in u8string var25, in u16string var26, in u32string var27, in wstring var28);};").Parse().
            Root()->Find("I::func"));
}

TEST_F(CParserInterfaceTest, ComplexTypeOperationDefinition)
{
    EXPECT_TRUE(CParser("struct S{}; interface I{void func(in S var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("struct S{}; interface I{void func(in struct S var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("typedef struct tagS {} S; interface I{void func(in S var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("typedef int32 TI; interface I{void func(in TI var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("typedef int32 TI[10]; interface I{void func(in TI var);};").Parse().Root()->Find("I::func"));
    EXPECT_THROW(CParser("typedef int32 TI[]; interface I{void func(in TI var);};").Parse(), CCompileException);
    EXPECT_TRUE(CParser("interface IRet{}; interface I{void func(in IRet var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface IRet{}; interface I{void func(out IRet var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface IRet{}; interface I{void func(inout IRet var);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("interface IRet{}; interface I{IRet func();};").Parse().Root()->Find("I::func"));
}

TEST_F(CParserInterfaceTest, OperationWithExceptionDefinition)
{
    EXPECT_TRUE(CParser("exception S1{}; interface I{void func(in int32 var) raises(S1);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("exception S1{}; interface I{void func(in int32 var) const raises(S1);};").Parse().Root()->Find("I::func"));
    EXPECT_THROW(CParser("exception S1{}; interface I{void func(in int32 var) getraises(S1);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; interface I{void func(in int32 var) const getraises(S1);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; interface I{void func(in int32 var) raises(S1) const;};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; interface I{void func(in int32 var) getraises(S1) const;};").Parse(), CCompileException);
    EXPECT_NO_THROW(CParser("exception S1{}; interface I{void func(in int32 var) raises(S1);};").Parse());
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{void func(in int32 var) setraises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{void func(in int32 var) raises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{void func(in int32 var) getraises(S1) raises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{void func(in int32 var) getraises(S1) getraises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("exception S1{}; exception S2{}; interface I{void func(in int32 var) raises(S1) raises(S2);};").Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I{void func(in int32 var) raises(S1);};").Parse(), CCompileException);
}

TEST_F(CParserInterfaceTest, LocalInterface)
{
    EXPECT_THROW(CParser("interface I{uint8[] Test(in int8 p);};").Parse(), CCompileException);
}

