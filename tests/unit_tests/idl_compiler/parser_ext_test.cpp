#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/definition_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/struct_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/variable_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/declaration_entity.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.h"

using CParserExtensionTest = CParserTest;

TEST_F(CParserExtensionTest, InterfaceType)
{
    // Check the use of the interface_t and the interface_id types.
    EXPECT_TRUE(CParser("interface I{void func(in interface_t ifc);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("struct S{interface_t ifc;};").Parse().Root()->Find("S::ifc"));
    EXPECT_TRUE(CParser("interface I{void func(in interface_id id);};").Parse().Root()->Find("I::func"));
    EXPECT_TRUE(CParser("struct S{interface_id id;};").Parse().Root()->Find("S::id"));
    EXPECT_TRUE(CParser("interface I{}; struct S{I ifc = null;};").Parse().Root()->Find("S::ifc"));
    EXPECT_THROW(CParser("struct interface_t{};").Parse(), CCompileException);   // not available as def name
    EXPECT_THROW(CParser("struct interface_id{};").Parse(), CCompileException);   // not available as def name
    EXPECT_THROW(CParser("struct null{};").Parse(), CCompileException);   // not available as def name

    // Disable the interface type extension
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--interface_type-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.InterfaceTypeExtension());
    EXPECT_THROW(CParser("interface I{void func(in interface_t ifc);};", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S{interface_t ifc;};", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I{void func(in interface_id id);};", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S{interface_id id;};", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("interface I{}; struct S{I ifc = null;};", env).Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct interface_t{};", env).Parse().Root()->Find("interface_t"));   // available as def name
    EXPECT_TRUE(CParser("struct interface_id{};", env).Parse().Root()->Find("interface_id"));   // available as def name
    EXPECT_TRUE(CParser("struct null{};", env).Parse().Root()->Find("null"));   // available as def name
}

TEST_F(CParserExtensionTest, ExceptionId)
{
    // Check the use of the exception_id type.
    EXPECT_TRUE(CParser("struct S{ exception_id id; };").Parse().Root()->Find("S::id"));
    EXPECT_THROW(CParser("struct exception_id{};").Parse(), CCompileException);   // not available as def name

    // Disable the interface type extension
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--exception_type-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.ExceptionTypeExtension());
    EXPECT_THROW(CParser("struct S{ exception_id id; };", env).Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct exception_id{};", env).Parse().Root()->Find("exception_id"));   // available as def name
}

TEST_F(CParserExtensionTest, PointerType)
{
    // Check the use of the pointer type.
    EXPECT_TRUE(CParser("struct S{ pointer<uint8> ptr; };").Parse().Root()->Find("S::ptr"));
    EXPECT_THROW(CParser("struct pointer{};").Parse(), CCompileException);   // not available as def name

    // Disable the interface type extension
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--pointer_type-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.PointerTypeExtension());
    EXPECT_THROW(CParser("struct S{ pointer<uint8> ptr; };", env).Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct pointer{};", env).Parse().Root()->Find("pointer"));   // available as def name
}

TEST_F(CParserExtensionTest, UnicodeCharType)
{
    // Check the use of the pointer type.
    EXPECT_TRUE(CParser("struct S{ char16 c16; };").Parse().Root()->Find("S::c16"));
    EXPECT_TRUE(CParser("struct S{ char32 c32; };").Parse().Root()->Find("S::c32"));
    EXPECT_THROW(CParser("struct char16{};").Parse(), CCompileException);   // not available as def name
    EXPECT_THROW(CParser("struct char32{};").Parse(), CCompileException);   // not available as def name

    // Disable the interface type extension
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--unicode_char-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.UnicodeExtension());
    EXPECT_THROW(CParser("struct S{ char16 c16; };", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S{ char32 c32; };", env).Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct char16{};", env).Parse().Root()->Find("char16"));   // available as def name
    EXPECT_TRUE(CParser("struct char32{};", env).Parse().Root()->Find("char32"));   // available as def name
}

TEST_F(CParserExtensionTest, UnicodeStringType)
{
    // Check the use of the pointer type.
    EXPECT_TRUE(CParser("struct S{ u8string ss8; };").Parse().Root()->Find("S::ss8"));
    EXPECT_TRUE(CParser("struct S{ u16string ss16; };").Parse().Root()->Find("S::ss16"));
    EXPECT_TRUE(CParser("struct S{ u32string ss32; };").Parse().Root()->Find("S::ss32"));
    EXPECT_THROW(CParser("struct u8string{};").Parse(), CCompileException);   // not available as def name
    EXPECT_THROW(CParser("struct u16string{};").Parse(), CCompileException);   // not available as def name
    EXPECT_THROW(CParser("struct u32string{};").Parse(), CCompileException);   // not available as def name

    // Disable the interface type extension
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--unicode_char-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.UnicodeExtension());
    EXPECT_THROW(CParser("struct S{ u8string ss8; };", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S{ u16string ss16; };", env).Parse(), CCompileException);
    EXPECT_THROW(CParser("struct S{ u32string ss32; };", env).Parse(), CCompileException);
    EXPECT_TRUE(CParser("struct u8string{};", env).Parse().Root()->Find("u8string"));   // available as def name
    EXPECT_TRUE(CParser("struct u16string{};", env).Parse().Root()->Find("u16string"));   // available as def name
    EXPECT_TRUE(CParser("struct u32string{};", env).Parse().Root()->Find("u32string"));   // available as def name
}

TEST_F(CParserExtensionTest, CaseSensitivity)
{
    // Disable case sensitivity
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--case_sensitive-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.CaseSensitiveTypeExtension());

    // Use identical keyword only differencing in case
    EXPECT_TRUE(CParser("struct Struct{};").Parse().Root()->Find("Struct"));
    EXPECT_THROW(CParser("struct Struct{};", env).Parse(), CCompileException);

    // Use identical identifiers only differencing in case
    EXPECT_THROW(CParser("struct S{ int32 i; int32 i;};").Parse(), CCompileException);          // Case sensitive; two vars
    EXPECT_THROW(CParser("struct S{ int32 i; int32 i;};", env).Parse(), CCompileException);     // Case insensitive; two vars
    EXPECT_NO_THROW(CParser("struct S { int32 i; int32 I; };").Parse());                        // Case sensitive; two vars differ in case
    EXPECT_THROW(CParser("struct S { int32 i; int32 I; };", env).Parse(), CCompileException);   // Case insensitive; two vars differ in case
    EXPECT_NO_THROW(CParser("struct S{}; struct S2{ S s; };").Parse());                         // Case sensitive; def and decl differ in scope and case
    EXPECT_NO_THROW(CParser("struct S{}; struct S2{ S s; };", env).Parse());                    // Case insensitive; def and decl differ in scope and case
}

TEST_F(CParserExtensionTest, ContextDependentNames)
{
    // Disable context dependent names
    std::vector<std::string> vecNoContextDeptNamesCaseSensitive = { "idl_compiler_test", "--context_names-" };
    std::vector<std::string> vecNoContextDeptNamesCaseInsensitive = { "idl_compiler_test", "--context_names-", "--case_sensitive-" };
    CIdlCompilerEnvironment envNoContextDeptNamesCaseSensitive(vecNoContextDeptNamesCaseSensitive);
    CIdlCompilerEnvironment envNoContextDeptNamesCaseInsensitive(vecNoContextDeptNamesCaseInsensitive);
    EXPECT_FALSE(envNoContextDeptNamesCaseSensitive.ContextDependentNamesExtension());
    EXPECT_FALSE(envNoContextDeptNamesCaseInsensitive.ContextDependentNamesExtension());
    EXPECT_FALSE(envNoContextDeptNamesCaseInsensitive.CaseSensitiveTypeExtension());

    // Use identical identifiers only differencing in case
    EXPECT_NO_THROW(CParser("struct S2{ struct S{}; S s;};").Parse());                                                                      // Case sensitive; reuse allowed
    EXPECT_NO_THROW(CParser("struct S2{ struct S{}; S s;};", envNoContextDeptNamesCaseSensitive).Parse());                                  // Case sensitive; reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; S s;};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);                // Names differ in case only; no reuse allowed
    EXPECT_NO_THROW(CParser("struct S2{ struct S{}; S S;};").Parse());                                                                      // Context differs; reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; S S;};", envNoContextDeptNamesCaseSensitive).Parse(), CCompileException);                  // Identical names; no reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; S S;};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);                // Identical names; no reuse allowed
    EXPECT_NO_THROW(CParser("struct S2{ int16 s; struct S{};};").Parse());                                                                  // Case sensitive; reuse allowed
    EXPECT_NO_THROW(CParser("struct S2{ int16 s; struct S{};};", envNoContextDeptNamesCaseSensitive).Parse());                              // Case sensitive; reuse allowed
    EXPECT_THROW(CParser("struct S2{ int16 s; struct S{};};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);            // Names differ in case only; no reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; typedef int16 S;};").Parse(), CCompileException);                                          // Identical names with no context change; no reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; typedef int16 S;};", envNoContextDeptNamesCaseSensitive).Parse(), CCompileException);      // Identical names; no reuse allowed
    EXPECT_THROW(CParser("struct S2{ struct S{}; typedef int16 S;};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);    // Identical names; no reuse allowed
    EXPECT_NO_THROW(CParser("struct S{ int16 attribute;};").Parse());                                                                       // Context differs; reuse allowed
    EXPECT_THROW(CParser("struct S{ int16 attribute;};", envNoContextDeptNamesCaseSensitive).Parse(), CCompileException);                   // Keyword is reserved; no reuse allowed
    EXPECT_THROW(CParser("struct S{ int16 attribute;};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);                 // Keyword is reserved; no reuse allowed
    EXPECT_NO_THROW(CParser("struct S{ int16 Attribute;};").Parse());                                                                       // Context differs; reuse allowed
    EXPECT_NO_THROW(CParser("struct S{ int16 Attribute;};", envNoContextDeptNamesCaseSensitive).Parse());                                   // Case sensitive; reuse allowed
    EXPECT_THROW(CParser("struct S{ int16 Attribute;};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);                 // Keyword name differs in case only; no reuse allowed
    EXPECT_THROW(CParser("struct S{ struct attribute{};};").Parse(), CCompileException);                                                    // Keyword is reserved; reuse allowed
    EXPECT_THROW(CParser("struct S{ struct attribute{};};", envNoContextDeptNamesCaseSensitive).Parse(), CCompileException);                // Keyword is reserved; no reuse allowed
    EXPECT_THROW(CParser("struct S{ struct attribute{};};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);              // Keyword is reserved; no reuse allowed
    EXPECT_NO_THROW(CParser("struct S{ struct Attribute{};};").Parse());                                                                    // Case sensitive; reuse allowed
    EXPECT_NO_THROW(CParser("struct S{ struct Attribute{};};", envNoContextDeptNamesCaseSensitive).Parse());                                // Case sensitive; reuse allowed
    EXPECT_THROW(CParser("struct S{ struct Attribute{};};", envNoContextDeptNamesCaseInsensitive).Parse(), CCompileException);              // Keyword name differs in case only; no reuse allowed
}

TEST_F(CParserExtensionTest, MultiDimArray)
{
    // Disable multi dimensional arrays
    CIdlCompilerEnvironment env;
    std::vector<std::string> vec = { "idl_compiler_test", "--multi_dimensional_array-" };
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.MultiDimArrayExtension());

    // Declare arrays
    EXPECT_NO_THROW(CParser("const int16 rguiArr[1] = { 10 };").Parse());
    EXPECT_NO_THROW(CParser("const int16 rguiArr[1] = { 10 };", env).Parse());
    EXPECT_NO_THROW(CParser("struct S { int16 rguiArr[1]; };").Parse());
    EXPECT_NO_THROW(CParser("struct S { int16 rguiArr[1]; };", env).Parse());
    EXPECT_NO_THROW(CParser("const int16 rguiArr[1][1] = { { 10 } };").Parse());
    EXPECT_THROW(CParser("const int16 rguiArr[1][1] = { { 10 } };", env).Parse(), CCompileException);
    EXPECT_NO_THROW(CParser("struct S { int16 rguiArr[1][1]; };").Parse());
    EXPECT_THROW(CParser("struct S { int16 rguiArr[1][1]; };", env).Parse(), CCompileException);
}