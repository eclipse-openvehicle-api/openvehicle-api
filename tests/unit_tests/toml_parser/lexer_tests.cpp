#include <gtest/gtest.h>
#include <limits>
#include "../../../sdv_services/core/toml_parser/lexer_toml.h"
#include "../../../sdv_services/core/toml_parser/exception.h"

/* Requirements Lexer
 * - Lexer has to ignore all whitespaces outside of comments, basic strings and string literals
 * - The Lexer has to recognize line comments
 *  - Control characters other than tab (U+0000 to U+0008, U+000A to U+001F, U+007F) are not permitted in comments
 * - The Lexer has to recognize key-value pairs
 *  - with value data types
 *      - token_string
 *      - token_integer
 *      - token_float
 *      - token_boolean
 *      - Offset Date-Time (not included)
 *      - Local Date-Time (not included)
 *      - Local Date (not included)
 *      - Local Time (not included)
 *      - Array
 *      - Inline Table
 *  - Keys are on the left of the equals sign and values are on the right
 *  - Keys are always interpreted as strings
 *  - The key, equals sign, and value must be on the same line (though some values can be broken over multiple lines)
 *  - There must be a newline (or EOF) after a key-value pair. (See Inline Table for exceptions.)
 *  - keys
 *      - bare keys may contain only 'A-Za-z0-9_-'; must at least be 1 character long
 *      - quoted keys may be basic strings or string literals; may be empty
 *      - dotted keys are bare keys and dotted keys joined by a dot '.'
 * - Strings
 *  - may only contain valid UTF-8 characters
 *  - Basic token_string
 *      - surrounded by '"'
 *      - chracters that must be escaped: '"', '\', and the control characters other than tab (U+0000 to U+0008, U+000A to U+001F,
 U+007F)
 *          \b         - backspace       (U+0008)
            \t         - tab             (U+0009)
            \n         - linefeed        (U+000A)
            \f         - form feed       (U+000C)
            \r         - carriage return (U+000D)
            \"         - quote           (U+0022)
            \\         - backslash       (U+005C)
            \uXXXX     - unicode         (U+XXXX)
            \UXXXXXXXX - unicode         (U+XXXXXXXX)
            Any Unicode character may be escaped with the \uXXXX or \UXXXXXXXX forms. The escape codes must be valid Unicode scalar
 values. All other escape sequences not listed above are reserved; if they are used, TOML should produce an error.
 *  - Multiline Basic token_string
        - surrounded by three quotation marks '"""' on each side
        - A newline immediately following the opening delimiter will be trimmed
        - All other whitespace and newline characters remain intact
        - When the last non-whitespace character on a line is an unescaped \, it will be trimmed along with all whitespace
            (including newlines) up to the next non-whitespace character or closing delimiter
 *  - Literal
 *  - Mulitline Literal
 */


/*
 * Requirements Lexer
 * - Tokenize input
 *  - find identifiers (keys; bare and quoted)
 *      + Keys
 *  - find syntax tokens ([, ], =, ., {, }, newline, [[, ]]) outside of literals
 *      + NewLine
 *      + Brackets []
 *      + Assignment =
 *      + Dot .
 *      + Braces {}
 *      + TableArray [[]]
 *  - find and convert/unescape literals (strings, integers, floats, bools, dates, times)
 *      + token_integer
 *      + token_float
 *      + token_boolean
 *      + Strings
 *      - Offset Date-Time
 *      - Local Date-Time
 *      - Local Date
 *      - Local Time
 *  - ignore whitespaces outside strings
 *  - ignore comments
 *  - invalid input results in an token_error-Token
 *      + invalid bare key
 *      + invalid quoted key
 *      + invalid token_string
 *      + invalid token_integer
 *      + invalid token_float
 *      + invalid token_boolean
 *      - invalid Offset Date-Time
 *      - invalid Local Date-Time
 *      - invalid Local Date
 *      - invalid Local Time
 *  ? Line and Position of tokens in source file
 * + Peek() returns the next token without advancing the read location
 * + Peek(n) returns the next n-th token without advancing the read location
 * + Consume() returns the next token and advances the read location directly after the returned token
 * + Consume(n) returns the next n-th token and advances the read location directly after the returned token
 * + if Consume(n) or Peek(n) are given a n < 1, they return an token_empty-Token
 * + when Peek or Consume would access a token after token_eof, they return a token_eof-Token,
 *          if lexing was terminated, they return a TerminatedToken
 * + Exceptions thrown by the CharacterReaderare caught and result in a token_terminated-Token and no further lexing will be done
 */

TEST(TOMLLexerTest, Keys)
{
    using namespace std::string_literals;
    // U+1F92B is the Finger-On-Lips Shushing emoji with UTF-8 byte representation 0xF09FA4AB
    CLexerTOML lexer(R"(
        key1 = "value1"
        "key\u0032" = "value2"
        "key3\U0001F92B" = "abc"
        '' = "valid"
        'literal "key"' = "also valid"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    // key30xF09FA4AB, '=', abc, NewLine
    // , '=', valid, NewLine
    // literal "key", '=', also valid
    // token_eof
    CLexerTOML::SToken key1 = lexer.Consume(2);
    CLexerTOML::SToken key2 = lexer.Consume(4);
    CLexerTOML::SToken key3 = lexer.Consume(4);
    CLexerTOML::SToken key4 = lexer.Consume(4);
    CLexerTOML::SToken key5 = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, key1.eCategory);
    EXPECT_EQ("key1", key1.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, key2.eCategory);
    EXPECT_EQ("\"key2\"", key2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, key3.eCategory);
    EXPECT_EQ(std::string("\"key3") + static_cast<char>(0xF0) + static_cast<char>(0x9F) + static_cast<char>(0xA4)
                  + static_cast<char>(0xAB) + '\"',
              key3.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, key4.eCategory);
    EXPECT_EQ("''", key4.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, key5.eCategory);
    EXPECT_EQ("'literal \"key\"'", key5.ssContentString);
}

TEST(TOMLLexerTest, SyntaxToken_NewLine)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key1 = "value"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value, NewLine
    // token_eof
    CLexerTOML::SToken newLine1 = lexer.Peek(1);
    CLexerTOML::SToken newLine2 = lexer.Peek(5);

    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, newLine1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, newLine2.eCategory);
}

TEST(TOMLLexerTest, SyntaxToken_Bracket)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        [table]
        array = [[1,2],[3,4]]
        emptyArray = []
        )"s);
    // Tokens:
    // NewLine
    // '[', table, ']', NewLine
    // array, '=', '[', '[', 1, ',', 2, ']', ',', '[', 3, ',', 4, ']', ']', NewLine
    // emptyArray, '=', '[', ']', NewLine
    // token_eof
    CLexerTOML::SToken tableBracketOpen		   = lexer.Peek(2);
    CLexerTOML::SToken tableBracketClose		   = lexer.Peek(4);
    CLexerTOML::SToken arrayOfArraysBracketOpen  = lexer.Peek(8);
    CLexerTOML::SToken array1BracketOpen		   = lexer.Peek(9);
    CLexerTOML::SToken array1BracketClose		   = lexer.Peek(13);
    CLexerTOML::SToken array2BracketOpen		   = lexer.Peek(15);
    CLexerTOML::SToken array2BracketClose		   = lexer.Peek(19);
    CLexerTOML::SToken arrayOfArraysBracketClose = lexer.Peek(20);
    CLexerTOML::SToken emptyArrayOpen			   = lexer.Peek(24);
    CLexerTOML::SToken emptyArrayClose		   = lexer.Peek(25);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_open, tableBracketOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_close, tableBracketClose.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_open, arrayOfArraysBracketOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_open, array1BracketOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_close, array1BracketClose.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_open, array2BracketOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_close, array2BracketClose.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_array_close, arrayOfArraysBracketClose.eCategory);
}

TEST(TOMLLexerTest, SyntaxToken_Assignment)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key = "value"
        )"s);
    // Tokens:
    // NewLine
    // key, '=', value, NewLine
    // token_eof
    CLexerTOML::SToken assignment = lexer.Peek(3);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_assignment, assignment.eCategory);
}

TEST(TOMLLexerTest, SyntaxToken_Dot)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        fruit.name = "banana"     # this is best practice
        fruit. color = "yellow"    # same as fruit.color
        fruit . flavor = "banana"   # same as fruit.flavor
        a.b.c = 2
        [dog."tater.man"]
        )"s);
    // Tokens:
    // NewLine
    // fruit, '.', name, '=', banana, NewLine
    // fruit, '.', color, '=', yellow, NewLine
    // fruit, '.', flavor, '=', banana, NewLine
    // a, '.', b, '.', c, '=', 2, NewLine
    // '[', dog, '.', tater.man, ']', NewLine
    // token_eof
    CLexerTOML::SToken dot1 = lexer.Peek(3);
    CLexerTOML::SToken dot2 = lexer.Peek(9);
    CLexerTOML::SToken dot3 = lexer.Peek(15);
    CLexerTOML::SToken dot4 = lexer.Peek(21);
    CLexerTOML::SToken dot5 = lexer.Peek(23);
    CLexerTOML::SToken dot6 = lexer.Peek(30);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot4.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot5.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_dot, dot6.eCategory);
}

TEST(TOMLLexerTest, SyntaxToken_Braces)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        inlineTable = { name = "Some Name", type = "Some Type" }
        nestedInlineTable = { a = { a = 0, b = "b"}, e = { c = 0, d = "d"} }
        emptyTable = {}
        )"s);
    // Tokens:
    // NewLine
    // inlineTable, '=', '{', name, '=', Some Name, ',', type, '=', Some Type, '}', NewLine
    // nestedInlineTable, '=', '{', a, '=', '{', a, '=', 0, ',' b '=', b, '}', ',' b, '=', '{', c, '=', 0, ',' d, '=', d, '}' '}',
    // NewLine emptyTable, '=', '{', '}', NewLine token_eof
    CLexerTOML::SToken braceOpen = lexer.Peek(4);
    CLexerTOML::SToken braceClose = lexer.Peek(12);
    CLexerTOML::SToken nestedOpen1 = lexer.Peek(16);
    CLexerTOML::SToken nestedOpen2 = lexer.Peek(19);
    CLexerTOML::SToken nestedClose2 = lexer.Peek(27);
    CLexerTOML::SToken nestedOpen3 = lexer.Peek(31);
    CLexerTOML::SToken nestedClose3 = lexer.Peek(39);
    CLexerTOML::SToken nestedClose1 = lexer.Peek(40);
    CLexerTOML::SToken emptyTableOpen	 = lexer.Peek(44);
    CLexerTOML::SToken emptyTableClose = lexer.Peek(45);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_open, braceOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_close, braceClose.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_open, nestedOpen1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_close, nestedClose1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_open, nestedOpen2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_close, nestedClose2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_open, nestedOpen3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_close, nestedClose3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_open, emptyTableOpen.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_inline_table_close, emptyTableClose.eCategory);

    CLexerTOML::SToken a = lexer.Peek(17);
    CLexerTOML::SToken a_a		= lexer.Peek(20);
    CLexerTOML::SToken a_b		= lexer.Peek(24);
    CLexerTOML::SToken e			= lexer.Peek(29);
    CLexerTOML::SToken e_c		= lexer.Peek(32);
    CLexerTOML::SToken e_d		= lexer.Peek(36);
    CLexerTOML::SToken emptyTable = lexer.Peek(42);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, a.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, a_a.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, a_b.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, e.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, e_c.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, e_d.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, emptyTable.eCategory);
}

TEST(TOMLLexerTest, SyntaxToken_ArrayTable)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        [[tableArray]]
        [[fruits.varieties]]
        )"s);
    // Tokens:
    // NewLine
    // '[[', tableArray, ']]', NewLine
    // '[[', fruits, '.', varieties, ']]', Newline
    // token_eof
    CLexerTOML::SToken tableArrayOpen1  = lexer.Peek(2);
    CLexerTOML::SToken tableArrayClose1 = lexer.Peek(4);
    CLexerTOML::SToken tableArrayOpen2  = lexer.Peek(6);
    CLexerTOML::SToken tableArrayClose2 = lexer.Peek(10);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_array_open, tableArrayOpen1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_array_close, tableArrayClose1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_array_open, tableArrayOpen2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_table_array_close, tableArrayClose2.eCategory);
}

TEST(TOMLLexerTest, Datatype_Integer)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        int1 = 1
        int2 = +13
        int3 = -37
        int4 = 1_000
        int5 = 5_349_221
        int6 = 53_49_221
        int7 = 1_2_3_4_5
        int8 = 0
        int9 = +0
        int10 = -0
        hex1 = 0xDEADBEEF
        hex2 = 0xdeadbeef
        hex3 = 0xdead_beef
        oct1 = 0o01234567
        oct2 = 0o755
        bin1 = 0b11111111
        bin2 = 0b00000001
        bin3 = 0b01010101
        )"s);
    // Tokens:
    // NewLine
    // int1, '=', 1, Newline
    // int2, '=', 13, NewLine
    // int3, '=', -37, NewLine
    // int4, '=', 1000, NewLine
    // int5, '=', 5349221, NewLine
    // int6, '=', 5349221, NewLine
    // int7, '=', 12345, NewLine
    // int8, '=', 0, NewLine
    // int9, '=', 0, NewLine
    // int10, '=', 0, NewLine
    // hex1, '=', 0xDEADBEEF, NewLine
    // hex2, '=', 0xDEADBEEF, NewLine
    // hex3, '=', 0xDEADBEEF, NewLine
    // oct1, '=', 01234567, NewLine
    // oct2, '=', 0755, NewLine
    // bin1, '=', 0xFF, NewLine
    // bin2, '=', 0x01, NewLine
    // bin3, '=', 0x55, NewLine
    // token_eof
    CLexerTOML::SToken int1  = lexer.Consume(4);
    CLexerTOML::SToken int2  = lexer.Consume(4);
    CLexerTOML::SToken int3  = lexer.Consume(4);
    CLexerTOML::SToken int4  = lexer.Consume(4);
    CLexerTOML::SToken int5  = lexer.Consume(4);
    CLexerTOML::SToken int6  = lexer.Consume(4);
    CLexerTOML::SToken int7  = lexer.Consume(4);
    CLexerTOML::SToken int8  = lexer.Consume(4);
    CLexerTOML::SToken int9  = lexer.Consume(4);
    CLexerTOML::SToken int10 = lexer.Consume(4);
    CLexerTOML::SToken hex1  = lexer.Consume(4);
    CLexerTOML::SToken hex2  = lexer.Consume(4);
    CLexerTOML::SToken hex3  = lexer.Consume(4);
    CLexerTOML::SToken oct1  = lexer.Consume(4);
    CLexerTOML::SToken oct2  = lexer.Consume(4);
    CLexerTOML::SToken bin1  = lexer.Consume(4);
    CLexerTOML::SToken bin2  = lexer.Consume(4);
    CLexerTOML::SToken bin3  = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int1.eCategory);
    EXPECT_EQ(1, int1.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int2.eCategory);
    EXPECT_EQ(13, int2.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int3.eCategory);
    EXPECT_EQ(-37, int3.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int4.eCategory);
    EXPECT_EQ(1000, int4.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int5.eCategory);
    EXPECT_EQ(5349221, int5.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int6.eCategory);
    EXPECT_EQ(5349221, int6.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int7.eCategory);
    EXPECT_EQ(12345, int7.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int8.eCategory);
    EXPECT_EQ(0, int8.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int9.eCategory);
    EXPECT_EQ(0, int9.iContentInteger);
    EXPECT_EQ(int8.iContentInteger, int9.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, int10.eCategory);
    EXPECT_EQ(0, int10.iContentInteger);
    EXPECT_EQ(int8.iContentInteger, int10.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, hex1.eCategory);
    EXPECT_EQ(0xDEADBEEF, hex1.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, hex2.eCategory);
    EXPECT_EQ(0xDEADBEEF, hex2.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, hex3.eCategory);
    EXPECT_EQ(0xDEADBEEF, hex3.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, oct1.eCategory);
    EXPECT_EQ(01234567, oct1.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, oct2.eCategory);
    EXPECT_EQ(0755, oct2.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, bin1.eCategory);
    EXPECT_EQ(0xFF, bin1.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, bin2.eCategory);
    EXPECT_EQ(0x01, bin2.iContentInteger);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_integer, bin3.eCategory);
    EXPECT_EQ(0x55, bin3.iContentInteger);
}

TEST(TOMLLexerTest, Datatype_Float)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        float1 = +1.0
        float2 = 3.1415
        float3 = -0.01
        float4 = 5e+22
        float5 = 1e06
        float6 = -2E-2
        float7 = 6.626e-34
        float8 = 224_617.445_991_228
        float9 = -0.0
        float10 = +0.0
        specialFloat1 = inf
        specialFloat2 = +inf
        specialFloat3 = -inf
        specialFloat4 = nan
        specialFloat5 = +nan
        specialFloat6 = -nan
        )"s);
    // Tokens:
    // NewLine
    // float1, '=', 1.0, Newline
    // float2, '=', 3.1415, Newline
    // float3, '=', -0.01, Newline
    // float4, '=', 5e+22, Newline
    // float5, '=', 1e+06, Newline
    // float6, '=', -2e-02, Newline
    // float7, '=', 6.626e-34, Newline
    // float8, '=', 224617.445991228, Newline
    // float9, '=', -0.0f, Newline
    // float10, '=', 0.0f, Newline
    // specialFloat1, '=', inf, NewLine
    // specialFloat2, '=', inf, NewLine
    // specialFloat3, '=', -inf, NewLine
    // specialFloat4, '=', qnan, NewLine
    // specialFloat5, '=', qnan, NewLine
    // specialFloat6, '=', -qnan, NewLine
    // token_eof
    CLexerTOML::SToken float1		   = lexer.Consume(4);
    CLexerTOML::SToken float2		   = lexer.Consume(4);
    CLexerTOML::SToken float3		   = lexer.Consume(4);
    CLexerTOML::SToken float4		   = lexer.Consume(4);
    CLexerTOML::SToken float5		   = lexer.Consume(4);
    CLexerTOML::SToken float6		   = lexer.Consume(4);
    CLexerTOML::SToken float7		   = lexer.Consume(4);
    CLexerTOML::SToken float8		   = lexer.Consume(4);
    CLexerTOML::SToken float9		   = lexer.Consume(4);
    CLexerTOML::SToken float10	   = lexer.Consume(4);
    CLexerTOML::SToken specialFloat1 = lexer.Consume(4);
    CLexerTOML::SToken specialFloat2 = lexer.Consume(4);
    CLexerTOML::SToken specialFloat3 = lexer.Consume(4);
    CLexerTOML::SToken specialFloat4 = lexer.Consume(4);
    CLexerTOML::SToken specialFloat5 = lexer.Consume(4);
    CLexerTOML::SToken specialFloat6 = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float1.eCategory);
    EXPECT_EQ(1.0, float1.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float2.eCategory);
    EXPECT_EQ(3.1415, float2.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float3.eCategory);
    EXPECT_EQ(-0.01, float3.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float4.eCategory);
    EXPECT_EQ(5e+22, float4.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float5.eCategory);
    EXPECT_EQ(1e+06, float5.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float6.eCategory);
    EXPECT_EQ(-2e-02, float6.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float7.eCategory);
    EXPECT_EQ(6.626e-34, float7.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float8.eCategory);
    EXPECT_EQ(224617.445991228, float8.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float9.eCategory);
    EXPECT_EQ(-0.0, float9.dContentFloatingpoint);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, float10.eCategory);
    EXPECT_EQ(+0.0, float10.dContentFloatingpoint);
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

    auto fnMakeUInt = [](double d)
    {
        union
        {
            double      d;
            uint64_t    ui;
        } temp{ d };
        return temp.ui;
    };
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat1.eCategory);
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat1.dContentFloatingpoint));
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat2.eCategory);
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat2.dContentFloatingpoint));
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat3.eCategory);
    EXPECT_EQ(0xFFF0000000000000ull, fnMakeUInt(specialFloat3.dContentFloatingpoint));
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat4.eCategory);
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat4.dContentFloatingpoint) & 0xFFF0000000000000ull);
    EXPECT_NE(0, fnMakeUInt(specialFloat4.dContentFloatingpoint) & 0x000FFFFFFFFFFFFFull);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat5.eCategory);
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat5.dContentFloatingpoint) & 0xFFF0000000000000ull);
    EXPECT_NE(0, fnMakeUInt(specialFloat5.dContentFloatingpoint) & 0x000FFFFFFFFFFFFFull);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_float, specialFloat6.eCategory);
    EXPECT_EQ(0xFFF0000000000000ull, fnMakeUInt(specialFloat6.dContentFloatingpoint) & 0xFFF0000000000000ull);
    EXPECT_NE(0, fnMakeUInt(specialFloat6.dContentFloatingpoint) & 0x000FFFFFFFFFFFFFull);
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
}

TEST(TOMLLexerTest, Datatype_Boolean)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key1 = true
        key2 = false
        )"s);
    // Tokens:
    // NewLine
    // key1, =, 1, NewLine
    // key2, =, 0, NewLine
    // token_eof
    CLexerTOML::SToken trueToken	= lexer.Consume(4);
    CLexerTOML::SToken falseToken = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_boolean, trueToken.eCategory);
    EXPECT_TRUE(trueToken.bContentBoolean);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_boolean, falseToken.eCategory);
    EXPECT_FALSE(falseToken.bContentBoolean);
}

TEST(TOMLLexerTest, Datatype_String_BasicString)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        basicString = "this is a string"
        basicString_esc1 = "Backspace\b"
        basicString_esc2 = "Tab\t"
        basicString_esc3 = "Linefeed\n"
        basicString_esc4 = "Formfeed\f"
        basicString_esc5 = "carriage return\r"
        basicString_esc6 = "quote\""
        basicString_esc7 = "backslash\\"
        basicString_esc8 = "unicode hiragana 'no': \u306e"
        basicString_esc9 = "Musical eighth note: \U0001D160"
        )"s);
    // Tokens:
    // NewLine
    // basicString, '=', this is a string, NewLine
    // basicString_esc1, '=', Bachspac, Newline
    // basicString_esc2, '=', Tab\t, Newline
    // basicString_esc3, '=', Linefeed\n, Newline
    // basicString_esc4, '=', Formfeed\f, Newline
    // basicString_esc5, '=', carriage return\r, Newline
    // basicString_esc6, '=', quote", Newline
    // basicString_esc7, '=', backslash\, Newline
    // basicString_esc8, '=', の, Newline
    // basicString_esc9, '=', 𝅘𝅥𝅮, Newline
    // token_eof
    CLexerTOML::SToken string		 = lexer.Consume(4);
    CLexerTOML::SToken string_esc1 = lexer.Consume(4);
    CLexerTOML::SToken string_esc2 = lexer.Consume(4);
    CLexerTOML::SToken string_esc3 = lexer.Consume(4);
    CLexerTOML::SToken string_esc4 = lexer.Consume(4);
    CLexerTOML::SToken string_esc5 = lexer.Consume(4);
    CLexerTOML::SToken string_esc6 = lexer.Consume(4);
    CLexerTOML::SToken string_esc7 = lexer.Consume(4);
    CLexerTOML::SToken string_esc8 = lexer.Consume(4);
    CLexerTOML::SToken string_esc9 = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string.eCategory);
    EXPECT_EQ("this is a string", string.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc1.eCategory);
    EXPECT_EQ("Backspace\b", string_esc1.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc2.eCategory);
    EXPECT_EQ("Tab\t", string_esc2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc3.eCategory);
    EXPECT_EQ("Linefeed\n", string_esc3.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc4.eCategory);
    EXPECT_EQ("Formfeed\f", string_esc4.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc5.eCategory);
    EXPECT_EQ("carriage return\r", string_esc5.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc6.eCategory);
    EXPECT_EQ("quote\"", string_esc6.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc7.eCategory);
    EXPECT_EQ("backslash\\", string_esc7.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc8.eCategory);
    EXPECT_EQ("unicode hiragana 'no': の", string_esc8.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc9.eCategory);
    EXPECT_EQ("Musical eighth note: 𝅘𝅥𝅮", string_esc9.ssContentString);
}

TEST(TOMLLexerTest, Datatype_String_BasicStringMultiline)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        basicStringMultiline1 = """
this is a
multiline string"""
        basicStringMultiline2 = """\
            this is a nice \

            multiline string"""
        basicStringMultiline3 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""
        basicStringMultiline4 = """"This," she said, "is just a pointless statement.""""
        basicStringMultiline_esc1 = """\
            Backspace\b \
            multiline"""
        basicStringMultiline_esc2 = """\
            Tab\t\
            multiline"""
        basicStringMultiline_esc3 = """\
            Linefeed\n\
            multiline"""
        basicStringMultiline_esc4 = """\
            Formfeed\f\
            multiline"""
        basicStringMultiline_esc5 = """\
            carriage return\r\
            multiline"""
        basicStringMultiline_esc6 = """\
            quote\" \
            multiline"""
        basicStringMultiline_esc7 = """\
            backslash\\\
            multiline"""
        basicStringMultiline_esc8 = """\
            unicode hiragana 'no': \u306e \
            multiline"""
        basicStringMultiline_esc9 = """\
            Musical eighth note: \U0001D160 \
            multiline"""
        )"s);
    // Tokens:
    // NewLine
    // basicStringMultiline1, '=', this is a\nmultiline string, NewLine
    // basicStringMultiline2, '=', this is a nice multiline string, NewLine
    // basicStringMultiline3, '=', Here are fifteen quotation marks: """""""""""""""., NewLine
    // basicStringMultiline4, '=', "This," she said, "is just a pointless statement.", NewLine
    // basicStringMultiline_esc1, '=', Backspac multiline, Newline
    // basicStringMultiline_esc2, '=', Tab\tmultiline, Newline
    // basicStringMultiline_esc3, '=', Linefeed\nmultiline, Newline
    // basicStringMultiline_esc4, '=', Formfeed\fmultiline, Newline
    // basicStringMultiline_esc5, '=', carriage return\rmultiline, Newline
    // basicStringMultiline_esc6, '=', quote" multiline, Newline
    // basicStringMultiline_esc7, '=', backslash\multiline, Newline
    // basicStringMultiline_esc8, '=', unicode hiragana 'no': の multiline, Newline
    // basicStringMultiline_esc9, '=', Musical eighth note: 𝅘𝅥𝅮 multiline, Newline
    // token_eof
    CLexerTOML::SToken string1	 = lexer.Consume(4);
    CLexerTOML::SToken string2	 = lexer.Consume(4);
    CLexerTOML::SToken string3	 = lexer.Consume(4);
    CLexerTOML::SToken string4	 = lexer.Consume(4);
    CLexerTOML::SToken string_esc1 = lexer.Consume(4);
    CLexerTOML::SToken string_esc2 = lexer.Consume(4);
    CLexerTOML::SToken string_esc3 = lexer.Consume(4);
    CLexerTOML::SToken string_esc4 = lexer.Consume(4);
    CLexerTOML::SToken string_esc5 = lexer.Consume(4);
    CLexerTOML::SToken string_esc6 = lexer.Consume(4);
    CLexerTOML::SToken string_esc7 = lexer.Consume(4);
    CLexerTOML::SToken string_esc8 = lexer.Consume(4);
    CLexerTOML::SToken string_esc9 = lexer.Consume(4);

    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string1.eCategory);
    EXPECT_EQ("this is a\nmultiline string", string1.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string2.eCategory);
    EXPECT_EQ("this is a nice multiline string", string2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string3.eCategory);
    EXPECT_EQ("Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", string3.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string4.eCategory);
    EXPECT_EQ("\"This,\" she said, \"is just a pointless statement.\"", string4.ssContentString);

    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc1.eCategory);
    EXPECT_EQ("Backspace\b multiline", string_esc1.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc2.eCategory);
    EXPECT_EQ("Tab\tmultiline", string_esc2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc3.eCategory);
    EXPECT_EQ("Linefeed\nmultiline", string_esc3.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc4.eCategory);
    EXPECT_EQ("Formfeed\fmultiline", string_esc4.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc5.eCategory);
    EXPECT_EQ("carriage return\rmultiline", string_esc5.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc6.eCategory);
    EXPECT_EQ("quote\" multiline", string_esc6.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc7.eCategory);
    EXPECT_EQ("backslash\\multiline", string_esc7.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc8.eCategory);
    EXPECT_EQ("unicode hiragana 'no': の multiline", string_esc8.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, string_esc9.eCategory);
    EXPECT_EQ("Musical eighth note: 𝅘𝅥𝅮 multiline", string_esc9.ssContentString);
}

TEST(TOMLLexerTest, Datatype_String_LiteralString)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        winpath  = 'C:\Users\nodejs\templates'
        winpath2 = '\\ServerX\admin$\system32\'
        quoted   = 'Tom "Dubs" Preston-Werner'
        regex    = '<\i\c*\s*>'
        )"s);
    // Tokens:
    // NewLine
    // winpath, '=', C:\Users\nodejs\templates, NewLine
    // winpath2, '=', \\ServerX\admin$\system32\, NewLine
    // quoted, '=', Tom "Dubs" Preston-Werner, NewLine
    // regex, '=', <\i\c*\s*>, NewLine

    CLexerTOML::SToken winpath  = lexer.Consume(4);
    CLexerTOML::SToken winpath2 = lexer.Consume(4);
    CLexerTOML::SToken quoted	  = lexer.Consume(4);
    CLexerTOML::SToken regex	  = lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, winpath.eCategory);
    EXPECT_EQ("C:\\Users\\nodejs\\templates", winpath.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, winpath2.eCategory);
    EXPECT_EQ("\\\\ServerX\\admin$\\system32\\", winpath2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, quoted.eCategory);
    EXPECT_EQ("Tom \"Dubs\" Preston-Werner", quoted.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, regex.eCategory);
    EXPECT_EQ("<\\i\\c*\\s*>", regex.ssContentString);
}

TEST(TOMLLexerTest, Datatype_String_LiteralStringMultiline)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
'''
quot15 = '''Here are fifteen quotation marks: """""""""""""""'''
apos15 = "Here are fifteen apostrophes: '''''''''''''''"
str = ''''That,' she said, 'is still pointless.''''
        )"s);
    // Tokens:
    // NewLine
    // regex2, '=', I't need \d{2} apples, NewLine
    // lines, '=', The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n, NewLine
    // quot15, '=', Here are fifteen quotation marks: """"""""""""""", NewLine
    // apos15, '=', Here are fifteen apostropes: ''''''''''''''', NewLine
    // str, '=', 'That,' she said, 'is still pointless.', NewLine
    // token_eof
    CLexerTOML::SToken regex2 = lexer.Consume(4);
    CLexerTOML::SToken lines	= lexer.Consume(4);
    CLexerTOML::SToken quot15 = lexer.Consume(4);
    CLexerTOML::SToken apos15 = lexer.Consume(4);
    CLexerTOML::SToken str	= lexer.Consume(4);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, regex2.eCategory);
    EXPECT_EQ("I [dw]on't need \\d{2} apples", regex2.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, lines.eCategory);
    EXPECT_EQ("The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", lines.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, quot15.eCategory);
    EXPECT_EQ("Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", quot15.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, apos15.eCategory);
    EXPECT_EQ("Here are fifteen apostrophes: '''''''''''''''", apos15.ssContentString);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_string, str.eCategory);
    EXPECT_EQ("'That,' she said, 'is still pointless.'", str.ssContentString);
}

// TEST(TOMLLexerTest, Datatype_OffsetDateTime)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Datatype_DateTime)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Datatype_LocalDate)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Datatype_LocalTime)
// {
//     ASSERT_TRUE(false);
// }

TEST(TOMLLexerTest, Invalid_Key)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        keyä = "value1"
        "key\u123" = "value2"
        "key\U1234" = "value3"
        "key\k" = "value4"
        )"s);
    // Tokens:
    // NewLine
    // ErrorToken, '=', value1, NewLine
    // ErrorToken, '=', value2, NewLine
    // ErrorToken, '=', value3, NewLine
    // ErrorToken, '=', value4, NewLine
    // token_eof
    CLexerTOML::SToken invBareKey	   = lexer.Consume(2); // only [a-zA-Z0-9-_]allowed for bare key
    CLexerTOML::SToken invQuotedKey1 = lexer.Consume(4); // \u has to be followed by 4 times [0-9A-Fa-F]
    CLexerTOML::SToken invQuotedKey2 = lexer.Consume(4); // \U has to be followed by 8 times [0-9A-Fa-F]
    CLexerTOML::SToken invQuotedKey3 = lexer.Consume(4); // any escape sequence except specified in the TOML secs are invalid
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invBareKey.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invQuotedKey1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invQuotedKey2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invQuotedKey3.eCategory);
}

TEST(TOMLLexerTest, Invalid_String)
{
    using namespace std::string_literals;
    CLexerTOML lexer1(R"(key1 = "invalid escape sequence \h)"s);
    CLexerTOML lexer2(R"(key2 = """invalid escape sequence \h""")"s);
    CLexerTOML lexer3(R"(key3 = "\u000")"s);
    CLexerTOML lexer4(R"(key4 = "\U0000000")"s);
    // Tokens:
    // key1, '=', ErrorToken, token_eof
    // key2, '=', ErrorToken, token_eof
    // key3, '=', ErrorToken, token_eof
    // key4, '=', ErrorToken, token_eof
    CLexerTOML::SToken invString1 = lexer1.Consume(3);
    CLexerTOML::SToken invString2 = lexer2.Consume(3);
    CLexerTOML::SToken invString3 = lexer3.Consume(3);
    CLexerTOML::SToken invString4 = lexer4.Consume(3);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invString1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invString2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invString3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invString4.eCategory);
}

TEST(TOMLLexerTest, Invalid_Integer)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        invInteger3 = 01
        invInteger4 = 0a1234
        invInteger5 = 1234_
        invInteger6 = _1234
        invInteger7 = 0x_1234
        invInteger8 = 0o_1234
        invInteger9 = 0b_1010
        invInteger10 = +0x1234
        invInteger11 = +0o1234
        invInteger12 = +0b1010
        invInteger13 = -0x1234
        invInteger14 = -0o1234
        invInteger15 = -0b1010
        )"s);
    // Tokens:
    // NewLine
    // invInteger3, '=', ErrorToken, NewLine
    // invInteger4, '=', ErrorToken, NewLine
    // invInteger5, '=', ErrorToken, NewLine
    // invInteger6, '=', ErrorToken, NewLine
    // invInteger7, '=', ErrorToken, NewLine
    // invInteger8, '=', ErrorToken, NewLine
    // invInteger9, '=', ErrorToken, NewLine
    // invInteger10, '=', ErrorToken, NewLine
    // invInteger11, '=', ErrorToken, NewLine
    // invInteger12, '=', ErrorToken, NewLine
    // invInteger12, '=', ErrorToken, NewLine
    // invInteger12, '=', ErrorToken, NewLine
    // invInteger12, '=', ErrorToken, NewLine
    // token_eof
    CLexerTOML::SToken invInteger3  = lexer.Consume(4); // no leading zeros
    CLexerTOML::SToken invInteger4  = lexer.Consume(4); // invalid prefix
    CLexerTOML::SToken invInteger5  = lexer.Consume(4); // underscores only allowed between two digits
    CLexerTOML::SToken invInteger6  = lexer.Consume(4); // underscores only allowed between two digits
    CLexerTOML::SToken invInteger7  = lexer.Consume(4); // underscores not allowed after prefix
    CLexerTOML::SToken invInteger8  = lexer.Consume(4); // underscores not allowed after prefix
    CLexerTOML::SToken invInteger9  = lexer.Consume(4); // underscores not allowed after prefix
    CLexerTOML::SToken invInteger10 = lexer.Consume(4); // no leading + allowed
    CLexerTOML::SToken invInteger11 = lexer.Consume(4); // no leading + allowed
    CLexerTOML::SToken invInteger12 = lexer.Consume(4); // no leading + allowed
    CLexerTOML::SToken invInteger13 = lexer.Consume(4); // no negative number representation allowed with hex format
    CLexerTOML::SToken invInteger14 = lexer.Consume(4); // no negative number representation allowed with ocatal format
    CLexerTOML::SToken invInteger15 = lexer.Consume(4); // no negative number representation allowed with binary format
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger4.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger5.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger6.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger7.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger8.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger9.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger10.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger11.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger12.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger13.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger14.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invInteger15.eCategory);
}

TEST(TOMLLexerTest, Invalid_Float)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        invFloat1 = .7
        invFloat2 = 1.
        invFloat3 = 3.e4
        invFloat4 = Inf
        invFloat5 = NaN
        )"s);
    // Tokens:
    // NewLine
    // invFloat1, '=', ErrorToken, NewLine
    // invFloat2, '=', ErrorToken, NewLine
    // invFloat3, '=', ErrorToken, NewLine
    // invFloat4, '=', ErrorToken, NewLine
    // invFloat5, '=', ErrorToken, NewLine
    // token_eof
    CLexerTOML::SToken invFloat1 = lexer.Consume(4); // decimal point has to be between two digits
    CLexerTOML::SToken invFloat2 = lexer.Consume(4); // decimal point has to be between two digits
    CLexerTOML::SToken invFloat3 = lexer.Consume(4); // decimal point has to be between two digits
    CLexerTOML::SToken invFloat4 = lexer.Consume(4); // special float values are always lower case
    CLexerTOML::SToken invFloat5 = lexer.Consume(4); // special float values are always lower case
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invFloat1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invFloat2.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invFloat3.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invFloat4.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invFloat5.eCategory);
}

TEST(TOMLLexerTest, Invalid_Boolean)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        invBool1 = True
        invBool2 = False
        )"s);
    // Tokens:
    // NewLine
    // invBool1, '=', ErrorToken, NewLine
    // invBool2, '=', ErrorToken, NewLine
    // token_eof
    CLexerTOML::SToken invBool1 = lexer.Consume(4); // keyword for true has to be lower case
    CLexerTOML::SToken invBool2 = lexer.Consume(4); // keyword for false has to be lower case
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invBool1.eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_error, invBool2.eCategory);
}

// TEST(TOMLLexerTest, Invalid_OffsetDateTime)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Invalid_LocalDateTime)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Invalid_LocalDate)
// {
//     ASSERT_TRUE(false);
// }

// TEST(TOMLLexerTest, Invalid_LocalTime)
// {
//     ASSERT_TRUE(false);
// }

TEST(TOMLLexerTest, Peek_NoAdvance)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    // token_eof
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, lexer.Peek().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, lexer.Peek().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, lexer.Peek(2).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, lexer.Peek(2).eCategory);
}

TEST(TOMLLexerTest, Consume_Advance)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    // token_eof
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, lexer.Consume().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_key, lexer.Consume().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_new_line, lexer.Consume(3).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_syntax_assignment, lexer.Consume(2).eCategory);
}

TEST(TOMLLexerTest, PeekConsume_BoundsCheck)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    // token_eof
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Consume(11).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Consume().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Peek().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Peek(2).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Peek(0).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Peek(-1).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Consume(0).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Consume(-1).eCategory);
}

TEST(TOMLLexerTest, PeekConsume_EmptyInput)
{
    using namespace std::string_literals;
    CLexerTOML lexer(R"()"s);
    // Tokens:
    // token_eof
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Peek().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Peek(1).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Consume().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_eof, lexer.Consume(1).eCategory);
}

TEST(TOMLLexerTest, ExceptionHandling)
{
    using namespace std::string_literals;
    CLexerTOML lexer;
    try
    {
        lexer.Feed(std::string("") + static_cast<char>(0xC1));
    }
    catch (const sdv::toml::XTOMLParseException&)
    {
    }

    // Tokens:
    // token_terminated-Token
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_terminated, lexer.Consume(11).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_terminated, lexer.Consume().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_terminated, lexer.Peek().eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_terminated, lexer.Peek(2).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Peek(0).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Peek(-1).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Consume(0).eCategory);
    EXPECT_EQ(CLexerTOML::ETokenCategory::token_empty, lexer.Consume(-1).eCategory);
}
