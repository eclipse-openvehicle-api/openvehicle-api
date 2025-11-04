#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/typedef_entity.cpp"

using CParserTypedefTest = CParserTest;

TEST_F(CParserTypedefTest, ParsingTypeDefinition)
{
    EXPECT_TRUE(CParser("typedef int8 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hello;").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingIndirectTypeDefinition)
{
    EXPECT_TRUE(CParser("typedef int8 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hi; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hi; typedef hi hello;").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingTypeDefinitionArrayDirect)
{
    EXPECT_TRUE(CParser("typedef int8 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hello[2];").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingTypeDefinitionArrayUnbound)
{
    EXPECT_TRUE(CParser("typedef int8 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef uint8 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef char hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef octet hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef int16 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef uint16 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef short hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef unsigned short hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef int32 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef uint32 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef long hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef unsigned long hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef int64 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef uint64 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef long long hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef unsigned long long hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef float hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef double hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef long double hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef wchar hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef char16 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef char32 hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef string hi[]; const hi hello = {\"1\", \"2\"};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef u8string hi[]; const hi hello = {u8\"1\", u8\"2\"};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef u16string hi[]; const hi hello = {u\"1\", u\"2\"};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef u32string hi[]; const hi hello = {U\"1\", U\"2\"};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef wstring hi[]; const hi hello = {L\"1\", L\"2\"};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef fixed hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_TRUE(CParser("typedef boolean hi[]; const hi hello = {1, 2};").Parse().Root()->Find("hi"));
    EXPECT_THROW(CParser("typedef boolean hi[]; hi hello = {1, 2};").Parse(), CCompileException);
    EXPECT_THROW(CParser("typedef boolean hi[]; typedef hi hi; ho hello = {1, 2};").Parse(), CCompileException);
}

TEST_F(CParserTypedefTest, ParsingTypeDefinitionArrayIndirect)
{
    EXPECT_TRUE(CParser("const int32 size = 2; typedef int8 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef uint8 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef char hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef octet hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef int16 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef uint16 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef short hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef unsigned short hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef int32 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef uint32 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef long hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef unsigned long hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef int64 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef uint64 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef long long hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef unsigned long long hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef float hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef double hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef long double hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef wchar hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef char16 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef char32 hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef string hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef u8string hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef u16string hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef u32string hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef fixed hello[2 * size];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("const int32 size = 2; typedef boolean hello[2 * size];").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingIndirectArrayTypeDefinition)
{
    EXPECT_TRUE(CParser("typedef int8 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hi[2]; typedef hi hello;").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingIndirectTypeArrayDefinition)
{
    EXPECT_TRUE(CParser("typedef int8 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hi; typedef hi hello[2];").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingIndirectArrayTypeArrayDefinition)
{
    EXPECT_TRUE(CParser("typedef int8 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint8 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef octet hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int16 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint16 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef short hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned short hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int32 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint32 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef int64 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef uint64 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long long hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef unsigned long long hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef float hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef double hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef long double hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef wchar hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char16 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef char32 hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef string hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u8string hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u16string hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef u32string hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef fixed hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
    EXPECT_TRUE(CParser("typedef boolean hi[2]; typedef hi hello[2];").Parse().Root()->Find("hello"));
}

TEST_F(CParserTypedefTest, ParsingAnonymousStructDefinition)
{
    // Anonymous struct definitions are not allowed in IDL
    EXPECT_THROW(CParser("typedef struct { const long hello = 10; } S1_t;").Parse(), CCompileException);
    EXPECT_THROW(CParser("typedef struct { const long hello = 10; } S1_t[10];").Parse().Root(), CCompileException);

    // Named struct definitions are allowed in IDL
    EXPECT_TRUE(CParser("typedef struct S1 { const long hello = 10; } S1_t;").Parse().Root()->Find("S1_t"));
    EXPECT_TRUE(CParser("typedef struct S1 { const long hello = 10; } S1_t[10];").Parse().Root()->Find("S1_t"));
}
