#include <gtest/gtest.h>
#include <limits>
#include <functional>
#include "../../../sdv_services/core/toml_parser/lexer_toml.h"
#include "../../../sdv_services/core/toml_parser/exception.h"

/* Requirements Lexer
 * - Lexer has to ignore all whitespaces outside of comments, basic strings and string literals
 * - The Lexer has to recognize line comments
 *  - Control characters other than tab (U+0000 to U+0008, U+000A to U+001F, U+007F) are not permitted in comments
 * - The Lexer has to recognize key-value pairs
 *  - with value data types
 *      - token_quoted_string
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
 *  - Basic token_quoted_string
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
 *  - Multiline Basic token_quoted_string
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
 *      + invalid token_quoted_string
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
 * + when Peek or Consume would access a token after EOF, they return an empty token,
 *          if lexing was terminated, they return a TerminatedToken
 * + Exceptions thrown by the CharacterReaderare caught and result in a token_terminated-Token and no further lexing will be done
 */

TEST(TOMLLexerTest, Keys)
{
    using namespace std::string_literals;
    // U+1F92B is the Finger-On-Lips Shushing emoji with UTF-8 byte representation 0xF09FA4AB
    toml_parser::CLexer lexer(R"(
        key1 = "value1"
        "key\u0032" = "value2"
        "key3\U0001F92B" = "abc"
        '' = "valid"
        'literal "key"' = "also valid"
        "quoted 'key'" = "also valid"
        _0xabcd = 0xabcd
        1234 = 1234
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    // key30xF09FA4AB, '=', abc, NewLine
    // , '=', valid, NewLine
    // literal "key", '=', also valid
    const toml_parser::CToken& key1 = lexer.Consume(1);
    const toml_parser::CToken& key2 = lexer.Consume(3);
    const toml_parser::CToken& key3 = lexer.Consume(3);
    const toml_parser::CToken& key4 = lexer.Consume(3);
    const toml_parser::CToken& key5 = lexer.Consume(3);
    const toml_parser::CToken& key6 = lexer.Consume(3);
    const toml_parser::CToken& key7 = lexer.Consume(3);
    const toml_parser::CToken& key8 = lexer.Consume(3);
    ASSERT_TRUE(key1);
    ASSERT_TRUE(key2);
    ASSERT_TRUE(key3);
    ASSERT_TRUE(key4);
    ASSERT_TRUE(key5);
    ASSERT_TRUE(key6);
    ASSERT_TRUE(key7);
    ASSERT_TRUE(key8);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key1.Category());
    EXPECT_EQ("key1", key1.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key2.Category());
    EXPECT_EQ("key\u0032", key2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key3.Category());
    EXPECT_EQ(u8"key3\U0001F92B",key3.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key4.Category());
    EXPECT_EQ("", key4.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key5.Category());
    EXPECT_EQ("literal \"key\"", key5.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key6.Category());
    EXPECT_EQ("quoted 'key'", key6.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key7.Category());
    EXPECT_EQ("_0xabcd", key7.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, key8.Category());
    EXPECT_EQ("1234", key8.StringValue());
}

TEST(TOMLLexerTest, SyntaxToken_NewLine)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        key1 = "value"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value, NewLine
    const toml_parser::CToken& newLine1 = lexer.Peek(0);
    const toml_parser::CToken& newLine2 = lexer.Peek(4);
    ASSERT_TRUE(newLine1);
    ASSERT_TRUE(newLine2);

    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, newLine1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, newLine2.Category());
}

TEST(TOMLLexerTest, SyntaxToken_Bracket)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        [table]
        array = [[1,2],[3,4]]
        emptyArray = []
        [["table array"]]
        )"s);
    // Tokens:
    // NewLine
    // '[', table, ']', NewLine
    // array, '=', '[', '[', 1, ',', 2, ']', ',', '[', 3, ',', 4, ']', ']', NewLine
    // emptyArray, '=', '[', ']', NewLine
    // '[[', "table array", ']]', NewLine
    const toml_parser::CToken& tableBracketOpen		    = lexer.Peek(1);
    const toml_parser::CToken& tableBracketClose		    = lexer.Peek(3);
    const toml_parser::CToken& arrayOfArraysBracketOpen    = lexer.Peek(7);
    const toml_parser::CToken& array1BracketOpen		    = lexer.Peek(8);
    const toml_parser::CToken& array1BracketClose		    = lexer.Peek(12);
    const toml_parser::CToken& array2BracketOpen		    = lexer.Peek(14);
    const toml_parser::CToken& array2BracketClose		    = lexer.Peek(18);
    const toml_parser::CToken& arrayOfArraysBracketClose   = lexer.Peek(19);
    const toml_parser::CToken& emptyArrayOpen			    = lexer.Peek(23);
    const toml_parser::CToken& emptyArrayClose		        = lexer.Peek(24);
    const toml_parser::CToken& tableArrayBracketOpen		= lexer.Peek(26);
    const toml_parser::CToken& tableArrayBracketClose      = lexer.Peek(28);

    ASSERT_TRUE(tableBracketOpen);
    ASSERT_TRUE(tableBracketClose);
    ASSERT_TRUE(arrayOfArraysBracketOpen);
    ASSERT_TRUE(array1BracketOpen);
    ASSERT_TRUE(array1BracketClose);
    ASSERT_TRUE(array2BracketOpen);
    ASSERT_TRUE(array2BracketClose);
    ASSERT_TRUE(arrayOfArraysBracketClose);
    ASSERT_TRUE(emptyArrayOpen);
    ASSERT_TRUE(emptyArrayClose);
    ASSERT_TRUE(tableArrayBracketOpen);
    ASSERT_TRUE(tableArrayBracketClose);

    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_open, tableBracketOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_close, tableBracketClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_open, arrayOfArraysBracketOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_open, array1BracketOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_close, array1BracketClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_open, array2BracketOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_close, array2BracketClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_close, arrayOfArraysBracketClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayBracketOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayBracketClose.Category());
}

TEST(TOMLLexerTest, SyntaxToken_Assignment)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexerNormal(R"(
        key = "value"
        )"s);
    // Tokens:
    // NewLine
    // key, '=', value, NewLine
    const toml_parser::CToken& rKeyNormal = lexerNormal.Consume(1);
    const toml_parser::CToken& rAssignmentNormal = lexerNormal.Consume();
    const toml_parser::CToken& rValueNormal = lexerNormal.Consume();
    EXPECT_TRUE(rKeyNormal);
    EXPECT_TRUE(rAssignmentNormal);
    EXPECT_TRUE(rValueNormal);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNormal.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNormal.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, rValueNormal.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, rValueNormal.StringType());
    EXPECT_EQ(rKeyNormal.StringValue(), "key");
    EXPECT_EQ(rValueNormal.StringValue(), "value");

    toml_parser::CLexer lexerNoSpace(R"(key="value")"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpace = lexerNoSpace.Consume();
    const toml_parser::CToken& rAssignmentNoSpace = lexerNoSpace.Consume();
    const toml_parser::CToken& rValueNoSpace = lexerNoSpace.Consume();
    EXPECT_TRUE(rKeyNoSpace);
    EXPECT_TRUE(rAssignmentNoSpace);
    EXPECT_TRUE(rValueNoSpace);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpace.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpace.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, rValueNoSpace.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, rValueNoSpace.StringType());
    EXPECT_EQ(rKeyNoSpace.StringValue(), "key");
    EXPECT_EQ(rValueNoSpace.StringValue(), "value");
}

TEST(TOMLLexerTest, Comments)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexerNormal(R"(
        #pre
        key = "value" #follow
        #next
        )"s);
    // Tokens:
    // NewLine
    // key, '=', value, NewLine
    const toml_parser::CToken& rKeyNormal        = lexerNormal.Consume(2);
    const toml_parser::CToken& rAssignmentNormal = lexerNormal.Consume();
    const toml_parser::CToken& rValueNormal      = lexerNormal.Consume();
    EXPECT_TRUE(rKeyNormal);
    EXPECT_TRUE(rAssignmentNormal);
    EXPECT_TRUE(rValueNormal);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNormal.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNormal.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, rValueNormal.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, rValueNormal.StringType());
    EXPECT_EQ(rKeyNormal.StringValue(), "key");
    EXPECT_EQ(rValueNormal.StringValue(), "value");

    toml_parser::CLexer lexerNoSpaceString(R"(key="value"#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceString        = lexerNoSpaceString.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceString = lexerNoSpaceString.Consume();
    const toml_parser::CToken& rValueNoSpaceString      = lexerNoSpaceString.Consume();
    EXPECT_TRUE(rKeyNoSpaceString);
    EXPECT_TRUE(rAssignmentNoSpaceString);
    EXPECT_TRUE(rValueNoSpaceString);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceString.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceString.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, rValueNoSpaceString.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, rValueNoSpaceString.StringType());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceString.Next().Category());
    EXPECT_EQ(rKeyNoSpaceString.StringValue(), "key");
    EXPECT_EQ(rValueNoSpaceString.StringValue(), "value");

    toml_parser::CLexer lexerNoSpaceInteger(R"(key=1234#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceInteger        = lexerNoSpaceInteger.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceInteger = lexerNoSpaceInteger.Consume();
    const toml_parser::CToken& rValueNoSpaceInteger      = lexerNoSpaceInteger.Consume();
    EXPECT_TRUE(rKeyNoSpaceInteger);
    EXPECT_TRUE(rAssignmentNoSpaceInteger);
    EXPECT_TRUE(rValueNoSpaceInteger);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceInteger.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceInteger.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, rValueNoSpaceInteger.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceInteger.Next().Category());
    EXPECT_EQ(rKeyNoSpaceInteger.StringValue(), "key");
    EXPECT_EQ(rValueNoSpaceInteger.IntegerValue(), 1234);

    toml_parser::CLexer lexerNoSpaceBoolean(R"(key=true#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceBoolean        = lexerNoSpaceBoolean.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceBoolean = lexerNoSpaceBoolean.Consume();
    const toml_parser::CToken& rValueNoSpaceBoolean      = lexerNoSpaceBoolean.Consume();
    EXPECT_TRUE(rKeyNoSpaceBoolean);
    EXPECT_TRUE(rAssignmentNoSpaceBoolean);
    EXPECT_TRUE(rValueNoSpaceBoolean);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceBoolean.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceBoolean.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_boolean, rValueNoSpaceBoolean.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceBoolean.Next().Category());
    EXPECT_EQ(rKeyNoSpaceBoolean.StringValue(), "key");
    EXPECT_EQ(rValueNoSpaceBoolean.BooleanValue(), true);

    toml_parser::CLexer lexerNoSpaceFloat(R"(key=123.456e78#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceFloat        = lexerNoSpaceFloat.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceFloat = lexerNoSpaceFloat.Consume();
    const toml_parser::CToken& rValueNoSpaceFloat      = lexerNoSpaceFloat.Consume();
    EXPECT_TRUE(rKeyNoSpaceFloat);
    EXPECT_TRUE(rAssignmentNoSpaceFloat);
    EXPECT_TRUE(rValueNoSpaceFloat);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceFloat.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceFloat.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, rValueNoSpaceFloat.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceFloat.Next().Category());
    EXPECT_EQ(rKeyNoSpaceFloat.StringValue(), "key");
    EXPECT_EQ(rValueNoSpaceFloat.FloatValue(), 123.456e78);

    toml_parser::CLexer lexerNoSpaceInlineTable(R"(key={x=1,y=2,z=3}#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceInlineTable        = lexerNoSpaceInlineTable.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceInlineTable = lexerNoSpaceInlineTable.Consume();
    const toml_parser::CToken& rValueNoSpaceInlineTable      = lexerNoSpaceInlineTable.Consume();
    EXPECT_TRUE(rKeyNoSpaceInlineTable);
    EXPECT_TRUE(rAssignmentNoSpaceInlineTable);
    EXPECT_TRUE(rValueNoSpaceInlineTable);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceInlineTable.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceInlineTable.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, rValueNoSpaceInlineTable.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceInlineTable.Next(12).Category());
    EXPECT_EQ(rKeyNoSpaceInlineTable.StringValue(), "key");

    toml_parser::CLexer lexerNoSpaceArray(R"(key=[1,2,3]#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rKeyNoSpaceArray        = lexerNoSpaceArray.Consume();
    const toml_parser::CToken& rAssignmentNoSpaceArray = lexerNoSpaceArray.Consume();
    const toml_parser::CToken& rValueNoSpaceArray      = lexerNoSpaceArray.Consume();
    EXPECT_TRUE(rKeyNoSpaceArray);
    EXPECT_TRUE(rAssignmentNoSpaceArray);
    EXPECT_TRUE(rValueNoSpaceArray);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceArray.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, rAssignmentNoSpaceArray.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_array_open, rValueNoSpaceArray.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rValueNoSpaceArray.Next(6).Category());
    EXPECT_EQ(rKeyNoSpaceArray.StringValue(), "key");

    toml_parser::CLexer lexerNoSpaceTable(R"([key]#follow)"s);
    // Tokens:
    // key, '=', value
    const toml_parser::CToken& rNoSpaceTableOpen = lexerNoSpaceTable.Consume();
    EXPECT_TRUE(rNoSpaceTableOpen);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_open, rNoSpaceTableOpen.Category());
    const toml_parser::CToken& rKeyNoSpaceTable = lexerNoSpaceTable.Consume();
    EXPECT_TRUE(rKeyNoSpaceTable);
    EXPECT_EQ(rKeyNoSpaceTable.StringValue(), "key");
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, rKeyNoSpaceTable.Category());
    const toml_parser::CToken& rNoSpaceTableClose = lexerNoSpaceTable.Consume();
    EXPECT_TRUE(rNoSpaceTableClose);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_close, rNoSpaceTableClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_comment, rNoSpaceTableClose.Next().Category());
}

TEST(TOMLLexerTest, SyntaxToken_Dot)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& dot1 = lexer.Peek(2);
    const toml_parser::CToken& dot2 = lexer.Peek(8);
    const toml_parser::CToken& dot3 = lexer.Peek(14);
    const toml_parser::CToken& dot4 = lexer.Peek(20);
    const toml_parser::CToken& dot5 = lexer.Peek(22);
    const toml_parser::CToken& dot6 = lexer.Peek(29);
    ASSERT_TRUE(dot1);
    ASSERT_TRUE(dot2);
    ASSERT_TRUE(dot3);
    ASSERT_TRUE(dot4);
    ASSERT_TRUE(dot5);
    ASSERT_TRUE(dot6);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot5.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_dot, dot6.Category());
}

TEST(TOMLLexerTest, SyntaxToken_Braces)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        inlineTable = { name = "Some Name", type = "Some Type" }
        nestedInlineTable = { a = { a = 0, b = "b"}, e = { c = 0, d = "d"} }
        emptyTable = {}
        )"s);
    // Tokens:
    // NewLine
    // inlineTable, '=', '{', name, '=', Some Name, ',', type, '=', Some Type, '}', NewLine
    // nestedInlineTable, '=', '{', a, '=', '{', a, '=', 0, ',' b '=', b, '}', ',' b, '=', '{', c, '=', 0, ',' d, '=', d, '}' '}',
    // NewLine emptyTable, '=', '{', '}', NewLine
    const toml_parser::CToken& braceOpen = lexer.Peek(3);
    const toml_parser::CToken& braceClose = lexer.Peek(11);
    const toml_parser::CToken& nestedOpen1 = lexer.Peek(15);
    const toml_parser::CToken& nestedOpen2 = lexer.Peek(18);
    const toml_parser::CToken& nestedClose2 = lexer.Peek(26);
    const toml_parser::CToken& nestedOpen3 = lexer.Peek(30);
    const toml_parser::CToken& nestedClose3 = lexer.Peek(38);
    const toml_parser::CToken& nestedClose1 = lexer.Peek(39);
    const toml_parser::CToken& emptyTableOpen	 = lexer.Peek(43);
    const toml_parser::CToken& emptyTableClose = lexer.Peek(44);
    ASSERT_TRUE(braceOpen);
    ASSERT_TRUE(braceClose);
    ASSERT_TRUE(nestedOpen1);
    ASSERT_TRUE(nestedOpen2);
    ASSERT_TRUE(nestedClose2);
    ASSERT_TRUE(nestedOpen3);
    ASSERT_TRUE(nestedClose3);
    ASSERT_TRUE(nestedClose1);
    ASSERT_TRUE(emptyTableOpen);
    ASSERT_TRUE(emptyTableClose);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, braceOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_close, braceClose.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, nestedOpen1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_close, nestedClose1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, nestedOpen2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_close, nestedClose2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, nestedOpen3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_close, nestedClose3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_open, emptyTableOpen.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_inline_table_close, emptyTableClose.Category());

    const toml_parser::CToken& a = lexer.Peek(16);
    const toml_parser::CToken& a_a		= lexer.Peek(19);
    const toml_parser::CToken& a_b		= lexer.Peek(23);
    const toml_parser::CToken& e			= lexer.Peek(28);
    const toml_parser::CToken& e_c		= lexer.Peek(31);
    const toml_parser::CToken& e_d		= lexer.Peek(35);
    const toml_parser::CToken& emptyTable = lexer.Peek(41);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, a.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, a_a.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, a_b.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, e.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, e_c.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, e_d.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, emptyTable.Category());
}

TEST(TOMLLexerTest, SyntaxToken_ArrayTable)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        [[tableArray]]
        [[fruits.varieties]]
        [["this is also a 'key'"]]
        [['this is also a "key"']]
        [[a."b.c"]]
        )"s);
    // Tokens:
    // NewLine
    // '[[', tableArray, ']]', NewLine
    // '[[', fruits, '.', varieties, ']]', Newline
    // '[[', "this is also a 'key'", ']]', NewLine
    // '[[', 'this is also a "key"', ']]', NewLine
    // '[[', a, '.', "b.c", ']]', Newline
    const toml_parser::CToken& tableArrayOpen1  = lexer.Peek(1);
    const toml_parser::CToken& tableArrayName1  = lexer.Peek(2);
    const toml_parser::CToken& tableArrayClose1 = lexer.Peek(3);
    const toml_parser::CToken& tableArrayOpen2  = lexer.Peek(5);
    const toml_parser::CToken& tableArrayName2a = lexer.Peek(6);
    const toml_parser::CToken& tableArrayName2b = lexer.Peek(8);
    const toml_parser::CToken& tableArrayClose2 = lexer.Peek(9);
    const toml_parser::CToken& tableArrayOpen3  = lexer.Peek(11);
    const toml_parser::CToken& tableArrayName3  = lexer.Peek(12);
    const toml_parser::CToken& tableArrayClose3 = lexer.Peek(13);
    const toml_parser::CToken& tableArrayOpen4  = lexer.Peek(15);
    const toml_parser::CToken& tableArrayName4  = lexer.Peek(16);
    const toml_parser::CToken& tableArrayClose4 = lexer.Peek(17);
    const toml_parser::CToken& tableArrayOpen5  = lexer.Peek(19);
    const toml_parser::CToken& tableArrayName5a = lexer.Peek(20);
    const toml_parser::CToken& tableArrayName5b = lexer.Peek(22);
    const toml_parser::CToken& tableArrayClose5 = lexer.Peek(23);
    ASSERT_TRUE(tableArrayOpen1);
    ASSERT_TRUE(tableArrayName1);
    ASSERT_TRUE(tableArrayClose1);
    ASSERT_TRUE(tableArrayOpen2);
    ASSERT_TRUE(tableArrayName2a);
    ASSERT_TRUE(tableArrayName2b);
    ASSERT_TRUE(tableArrayClose2);
    ASSERT_TRUE(tableArrayOpen3);
    ASSERT_TRUE(tableArrayName3);
    ASSERT_TRUE(tableArrayClose3);
    ASSERT_TRUE(tableArrayOpen4);
    ASSERT_TRUE(tableArrayName4);
    ASSERT_TRUE(tableArrayClose4);
    ASSERT_TRUE(tableArrayOpen5);
    ASSERT_TRUE(tableArrayName5a);
    ASSERT_TRUE(tableArrayName5b);
    ASSERT_TRUE(tableArrayClose5);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayOpen1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName1.Category());
    EXPECT_EQ(tableArrayName1.StringValue(), "tableArray");
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayClose1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayOpen2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName2a.Category());
    EXPECT_EQ(tableArrayName2a.StringValue(), "fruits");
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName2b.Category());
    EXPECT_EQ(tableArrayName2b.StringValue(), "varieties");
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayClose2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayOpen3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName3.Category());
    EXPECT_EQ(tableArrayName3.StringValue(), "this is also a 'key'");
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayClose3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayOpen4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName4.Category());
    EXPECT_EQ(tableArrayName4.StringValue(), "this is also a \"key\"");
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayClose4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_open, tableArrayOpen5.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName5a.Category());
    EXPECT_EQ(tableArrayName5a.StringValue(), "a");
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, tableArrayName5b.Category());
    EXPECT_EQ(tableArrayName5b.StringValue(), "b.c");
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_table_array_close, tableArrayClose5.Category());
}

TEST(TOMLLexerTest, Datatype_Integer)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
        intErr1 = _1234
        intErr2 = 1__234
        intErr3 = 1234_
        intErr4 = +0xabcd
        intErr5 = -0xabcd
        intErr6 = 0xabc__d
        intErr7 = 0x_abcd
        intErr8 = 0xabcd_
        intErr9 = 0_xabcd
        intErr10 = 0XDEADBEEF
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
    // intErr1, '=', _1234, NewLine
    // intErr2, '=', 1__234, NewLine
    // intErr3, '=', 1234_, NewLine
    // intErr4, '=', +0xabcd, NewLine
    // intErr5, '=', -0xabcd, NewLine
    // intErr6, '=', 0xabc__d, NewLine
    // intErr7, '=', 0x_abcd, NewLine
    // intErr8, '=', 0xabcd_, NewLine
    // intErr9, '=', 0_xabcd, NewLine
    // intErr10, '=', 0XDEADBEEF, NewLine
    const toml_parser::CToken& int1  = lexer.Consume(3);
    const toml_parser::CToken& int2  = lexer.Consume(3);
    const toml_parser::CToken& int3  = lexer.Consume(3);
    const toml_parser::CToken& int4  = lexer.Consume(3);
    const toml_parser::CToken& int5  = lexer.Consume(3);
    const toml_parser::CToken& int6  = lexer.Consume(3);
    const toml_parser::CToken& int7  = lexer.Consume(3);
    const toml_parser::CToken& int8  = lexer.Consume(3);
    const toml_parser::CToken& int9  = lexer.Consume(3);
    const toml_parser::CToken& int10 = lexer.Consume(3);
    const toml_parser::CToken& hex1  = lexer.Consume(3);
    const toml_parser::CToken& hex2  = lexer.Consume(3);
    const toml_parser::CToken& hex3  = lexer.Consume(3);
    const toml_parser::CToken& oct1  = lexer.Consume(3);
    const toml_parser::CToken& oct2  = lexer.Consume(3);
    const toml_parser::CToken& bin1  = lexer.Consume(3);
    const toml_parser::CToken& bin2  = lexer.Consume(3);
    const toml_parser::CToken& bin3  = lexer.Consume(3);
    const toml_parser::CToken& intErr1 = lexer.Consume(3);
    const toml_parser::CToken& intErr2 = lexer.Consume(3);
    const toml_parser::CToken& intErr3 = lexer.Consume(3);
    const toml_parser::CToken& intErr4 = lexer.Consume(3);
    const toml_parser::CToken& intErr5 = lexer.Consume(3);
    const toml_parser::CToken& intErr6 = lexer.Consume(3);
    const toml_parser::CToken& intErr7 = lexer.Consume(3);
    const toml_parser::CToken& intErr8 = lexer.Consume(3);
    const toml_parser::CToken& intErr9 = lexer.Consume(3);
    const toml_parser::CToken& intErr10 = lexer.Consume(3);
    ASSERT_TRUE(int1);
    ASSERT_TRUE(int2);
    ASSERT_TRUE(int3);
    ASSERT_TRUE(int4);
    ASSERT_TRUE(int5);
    ASSERT_TRUE(int6);
    ASSERT_TRUE(int7);
    ASSERT_TRUE(int8);
    ASSERT_TRUE(int9);
    ASSERT_TRUE(int10);
    ASSERT_TRUE(hex1);
    ASSERT_TRUE(hex2);
    ASSERT_TRUE(hex3);
    ASSERT_TRUE(oct1);
    ASSERT_TRUE(oct2);
    ASSERT_TRUE(bin1);
    ASSERT_TRUE(bin2);
    ASSERT_TRUE(bin3);
    ASSERT_TRUE(intErr1);
    ASSERT_TRUE(intErr2);
    ASSERT_TRUE(intErr3);
    ASSERT_TRUE(intErr4);
    ASSERT_TRUE(intErr5);
    ASSERT_TRUE(intErr6);
    ASSERT_TRUE(intErr7);
    ASSERT_TRUE(intErr8);
    ASSERT_TRUE(intErr9);
    ASSERT_TRUE(intErr10);
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int1.Category());
    EXPECT_EQ(1, int1.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int2.Category());
    EXPECT_EQ(13, int2.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int3.Category());
    EXPECT_EQ(-37, int3.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int4.Category());
    EXPECT_EQ(1000, int4.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int5.Category());
    EXPECT_EQ(5349221, int5.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int6.Category());
    EXPECT_EQ(5349221, int6.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int7.Category());
    EXPECT_EQ(12345, int7.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int8.Category());
    EXPECT_EQ(0, int8.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int9.Category());
    EXPECT_EQ(0, int9.IntegerValue());
    EXPECT_EQ(int8.IntegerValue(), int9.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, int10.Category());
    EXPECT_EQ(0, int10.IntegerValue());
    EXPECT_EQ(int8.IntegerValue(), int10.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, hex1.Category());
    EXPECT_EQ(0xDEADBEEF, hex1.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, hex2.Category());
    EXPECT_EQ(0xDEADBEEF, hex2.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, hex3.Category());
    EXPECT_EQ(0xDEADBEEF, hex3.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, oct1.Category());
    EXPECT_EQ(01234567, oct1.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, oct2.Category());
    EXPECT_EQ(0755, oct2.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, bin1.Category());
    EXPECT_EQ(0xFF, bin1.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, bin2.Category());
    EXPECT_EQ(0x01, bin2.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_integer, bin3.Category());
    EXPECT_EQ(0x55, bin3.IntegerValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr5.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr6.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr7.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr8.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr9.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, intErr10.Category());
}

TEST(TOMLLexerTest, Datatype_Float)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        float1 = +1.0
        float2 = 3.1415
        float3 = -0.01
        float4 = 5e+22
        float5 = 1e06
        float6 = -2E-2
        float7 = 6.626e-34
        float8 = 6.6_2_6e-3_4
        float9 = 224_617.445_991_228
        float10 = -0.0
        float11 = +0.0
        specialFloat12 = inf
        specialFloat13 = +inf
        specialFloat14 = -inf
        specialFloat15 = nan
        specialFloat16 = +nan
        specialFloat17 = -nan
        errFloat18 = 224__617.445_991_228
        errFloat19 = _224_617.445_991_228
        errFloat20 = 224_617_.445_991_228
        errFloat21 = 224_617._445_991_228
        errFloat22 = 224_617.445_991_228_
        errFloat23 = 224_617.445_991__228
        errFloat24 = 6.626_e-34
        errFloat25 = 6.626e_-34
        errFloat26 = 6.626e-_34
        errFloat27 = 6.626e-34_
        errFloat28 = 6.626_e-34
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
    // float8, '=', 6.6_2_6e-3_4, Newline
    // float9, '=', 224617.445991228, Newline
    // float10, '=', -0.0f, Newline
    // float11, '=', 0.0f, Newline
    // specialFloat12, '=', inf, NewLine
    // specialFloat13, '=', inf, NewLine
    // specialFloat14, '=', -inf, NewLine
    // specialFloat15, '=', qnan, NewLine
    // specialFloat16, '=', qnan, NewLine
    // specialFloat17, '=', -qnan, NewLine
    // errFloat18, '=', 224__617.445_991_228, NewLine
    // errFloat19, '=', _224_617.445_991_228, NewLine
    // errFloat20, '=', 224_617_.445_991_228, NewLine
    // errFloat21, '=', 224_617._445_991_228, NewLine
    // errFloat22, '=', 224_617.445_991_228_, NewLine
    // errFloat23, '=', 224_617.445_991__228, NewLine
    // errFloat24, '=', 6.626_e-34, NewLine
    // errFloat25, '=', 6.626e_-34, NewLine
    // errFloat26, '=', 6.626e-_34, NewLine
    // errFloat27, '=', 6.626e-34_, NewLine
    // errFloat28, '=', 6.626_e-34, NewLine
    const toml_parser::CToken& float1		   = lexer.Consume(3);
    const toml_parser::CToken& float2		   = lexer.Consume(3);
    const toml_parser::CToken& float3		   = lexer.Consume(3);
    const toml_parser::CToken& float4		   = lexer.Consume(3);
    const toml_parser::CToken& float5		   = lexer.Consume(3);
    const toml_parser::CToken& float6		   = lexer.Consume(3);
    const toml_parser::CToken& float7		   = lexer.Consume(3);
    const toml_parser::CToken& float8		   = lexer.Consume(3);
    const toml_parser::CToken& float9		   = lexer.Consume(3);
    const toml_parser::CToken& float10	   = lexer.Consume(3);
    const toml_parser::CToken& float11	   = lexer.Consume(3);
    const toml_parser::CToken& specialFloat12 = lexer.Consume(3);
    const toml_parser::CToken& specialFloat13  = lexer.Consume(3);
    const toml_parser::CToken& specialFloat14  = lexer.Consume(3);
    const toml_parser::CToken& specialFloat15  = lexer.Consume(3);
    const toml_parser::CToken& specialFloat16  = lexer.Consume(3);
    const toml_parser::CToken& specialFloat17  = lexer.Consume(3);
    const toml_parser::CToken& errFloat18      = lexer.Consume(3);
    const toml_parser::CToken& errFloat19      = lexer.Consume(3);
    const toml_parser::CToken& errFloat20      = lexer.Consume(3);
    const toml_parser::CToken& errFloat21      = lexer.Consume(3);
    const toml_parser::CToken& errFloat22      = lexer.Consume(3);
    const toml_parser::CToken& errFloat23      = lexer.Consume(3);
    const toml_parser::CToken& errFloat24      = lexer.Consume(3);
    const toml_parser::CToken& errFloat25      = lexer.Consume(3);
    const toml_parser::CToken& errFloat26      = lexer.Consume(3);
    const toml_parser::CToken& errFloat27      = lexer.Consume(3);
    const toml_parser::CToken& errFloat28      = lexer.Consume(3);
    ASSERT_TRUE(float1);
    ASSERT_TRUE(float2);
    ASSERT_TRUE(float3);
    ASSERT_TRUE(float4);
    ASSERT_TRUE(float5);
    ASSERT_TRUE(float6);
    ASSERT_TRUE(float7);
    ASSERT_TRUE(float8);
    ASSERT_TRUE(float9);
    ASSERT_TRUE(float10);
    ASSERT_TRUE(float11);
    ASSERT_TRUE(specialFloat12);
    ASSERT_TRUE(specialFloat13);
    ASSERT_TRUE(specialFloat14);
    ASSERT_TRUE(specialFloat15);
    ASSERT_TRUE(specialFloat16);
    ASSERT_TRUE(specialFloat17);
    ASSERT_TRUE(errFloat18);
    ASSERT_TRUE(errFloat19);
    ASSERT_TRUE(errFloat20);
    ASSERT_TRUE(errFloat21);
    ASSERT_TRUE(errFloat22);
    ASSERT_TRUE(errFloat23);
    ASSERT_TRUE(errFloat24);
    ASSERT_TRUE(errFloat25);
    ASSERT_TRUE(errFloat26);
    ASSERT_TRUE(errFloat27);
    ASSERT_TRUE(errFloat28);
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float1.Category());
    EXPECT_EQ(1.0, float1.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float2.Category());
    EXPECT_EQ(3.1415, float2.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float3.Category());
    EXPECT_EQ(-0.01, float3.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float4.Category());
    EXPECT_EQ(5e+22, float4.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float5.Category());
    EXPECT_EQ(1e+06, float5.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float6.Category());
    EXPECT_EQ(-2e-02, float6.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float7.Category());
    EXPECT_EQ(6.626e-34, float7.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float8.Category());
    EXPECT_EQ(6.626e-34, float8.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float9.Category());
    EXPECT_EQ(224617.445991228, float9.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float10.Category());
    EXPECT_EQ(-0.0, float10.FloatValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, float11.Category());
    EXPECT_EQ(+0.0, float11.FloatValue());

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
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat12.Category());
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat12.FloatValue())); // inf
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat13.Category());
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat13.FloatValue())); // +inf
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat14.Category());
    EXPECT_EQ(0xFFF0000000000000ull, fnMakeUInt(specialFloat14.FloatValue())); // -inf
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat15.Category());
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat15.FloatValue()) & 0xFFF0000000000000ull); // nan
    EXPECT_NE(0, fnMakeUInt(specialFloat15.FloatValue()) & 0x000FFFFFFFFFFFFFull);
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat16.Category());
    EXPECT_EQ(0x7FF0000000000000ull, fnMakeUInt(specialFloat16.FloatValue()) & 0xFFF0000000000000ull);
    EXPECT_NE(0, fnMakeUInt(specialFloat16.FloatValue()) & 0x000FFFFFFFFFFFFFull);
    EXPECT_EQ(toml_parser::ETokenCategory::token_float, specialFloat17.Category());
    EXPECT_EQ(0xFFF0000000000000ull, fnMakeUInt(specialFloat17.FloatValue()) & 0xFFF0000000000000ull);
    EXPECT_NE(0, fnMakeUInt(specialFloat17.FloatValue()) & 0x000FFFFFFFFFFFFFull);
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat18.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat19.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat20.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat21.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat22.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat23.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat24.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat25.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat26.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat27.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, errFloat28.Category());
}

TEST(TOMLLexerTest, Datatype_Boolean)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        key1 = true
        key2 = false
        )"s);
    // Tokens:
    // NewLine
    // key1, =, 1, NewLine
    // key2, =, 0, NewLine
    const toml_parser::CToken& trueToken	= lexer.Consume(3);
    const toml_parser::CToken& falseToken = lexer.Consume(3);
    ASSERT_TRUE(trueToken);
    ASSERT_TRUE(falseToken);
    EXPECT_EQ(toml_parser::ETokenCategory::token_boolean, trueToken.Category());
    EXPECT_TRUE(trueToken.BooleanValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_boolean, falseToken.Category());
    EXPECT_FALSE(falseToken.BooleanValue());
}

TEST(TOMLLexerTest, Datatype_String_BasicString)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& string		 = lexer.Consume(3);
    const toml_parser::CToken& string_esc1 = lexer.Consume(3);
    const toml_parser::CToken& string_esc2 = lexer.Consume(3);
    const toml_parser::CToken& string_esc3 = lexer.Consume(3);
    const toml_parser::CToken& string_esc4 = lexer.Consume(3);
    const toml_parser::CToken& string_esc5 = lexer.Consume(3);
    const toml_parser::CToken& string_esc6 = lexer.Consume(3);
    const toml_parser::CToken& string_esc7 = lexer.Consume(3);
    const toml_parser::CToken& string_esc8 = lexer.Consume(3);
    const toml_parser::CToken& string_esc9 = lexer.Consume(3);
    ASSERT_TRUE(string);
    ASSERT_TRUE(string_esc1);
    ASSERT_TRUE(string_esc2);
    ASSERT_TRUE(string_esc3);
    ASSERT_TRUE(string_esc4);
    ASSERT_TRUE(string_esc5);
    ASSERT_TRUE(string_esc6);
    ASSERT_TRUE(string_esc7);
    ASSERT_TRUE(string_esc8);
    ASSERT_TRUE(string_esc9);
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string.StringType());
    EXPECT_EQ("this is a string", string.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc1.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc1.StringType());
    EXPECT_EQ("Backspace\b", string_esc1.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc2.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc2.StringType());
    EXPECT_EQ("Tab\t", string_esc2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc3.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc3.StringType());
    EXPECT_EQ("Linefeed\n", string_esc3.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc4.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc4.StringType());
    EXPECT_EQ("Formfeed\f", string_esc4.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc5.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc5.StringType());
    EXPECT_EQ("carriage return\r", string_esc5.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc6.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc6.StringType());
    EXPECT_EQ("quote\"", string_esc6.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc7.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc7.StringType());
    EXPECT_EQ("backslash\\", string_esc7.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc8.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc8.StringType());
    EXPECT_EQ("unicode hiragana 'no': の", string_esc8.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc9.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, string_esc9.StringType());
    EXPECT_EQ("Musical eighth note: 𝅘𝅥𝅮", string_esc9.StringValue());
}

TEST(TOMLLexerTest, Datatype_String_BasicStringMultiline)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& string1	 = lexer.Consume(3);
    const toml_parser::CToken& string2	 = lexer.Consume(3);
    const toml_parser::CToken& string3	 = lexer.Consume(3);
    const toml_parser::CToken& string4	 = lexer.Consume(3);
    const toml_parser::CToken& string_esc1 = lexer.Consume(3);
    const toml_parser::CToken& string_esc2 = lexer.Consume(3);
    const toml_parser::CToken& string_esc3 = lexer.Consume(3);
    const toml_parser::CToken& string_esc4 = lexer.Consume(3);
    const toml_parser::CToken& string_esc5 = lexer.Consume(3);
    const toml_parser::CToken& string_esc6 = lexer.Consume(3);
    const toml_parser::CToken& string_esc7 = lexer.Consume(3);
    const toml_parser::CToken& string_esc8 = lexer.Consume(3);
    const toml_parser::CToken& string_esc9 = lexer.Consume(3);
    ASSERT_TRUE(string1);
    ASSERT_TRUE(string2);
    ASSERT_TRUE(string3);
    ASSERT_TRUE(string4);
    ASSERT_TRUE(string_esc1);
    ASSERT_TRUE(string_esc2);
    ASSERT_TRUE(string_esc3);
    ASSERT_TRUE(string_esc4);
    ASSERT_TRUE(string_esc5);
    ASSERT_TRUE(string_esc6);
    ASSERT_TRUE(string_esc7);
    ASSERT_TRUE(string_esc8);
    ASSERT_TRUE(string_esc9);

    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string1.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string1.StringType());
    EXPECT_EQ("this is a\nmultiline string", string1.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string2.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string2.StringType());
    EXPECT_EQ("this is a nice multiline string", string2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string3.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string3.StringType());
    EXPECT_EQ("Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\".", string3.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string4.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string4.StringType());
    EXPECT_EQ("\"This,\" she said, \"is just a pointless statement.\"", string4.StringValue());

    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc1.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc1.StringType());
    EXPECT_EQ("Backspace\b multiline", string_esc1.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc2.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc2.StringType());
    EXPECT_EQ("Tab\tmultiline", string_esc2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc3.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc3.StringType());
    EXPECT_EQ("Linefeed\nmultiline", string_esc3.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc4.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc4.StringType());
    EXPECT_EQ("Formfeed\fmultiline", string_esc4.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc5.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc5.StringType());
    EXPECT_EQ("carriage return\rmultiline", string_esc5.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc6.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc6.StringType());
    EXPECT_EQ("quote\" multiline", string_esc6.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc7.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc7.StringType());
    EXPECT_EQ("backslash\\multiline", string_esc7.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc8.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc8.StringType());
    EXPECT_EQ("unicode hiragana 'no': の multiline", string_esc8.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, string_esc9.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_quoted, string_esc9.StringType());
    EXPECT_EQ("Musical eighth note: 𝅘𝅥𝅮 multiline", string_esc9.StringValue());
}

TEST(TOMLLexerTest, Datatype_String_LiteralString)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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

    const toml_parser::CToken& winpath  = lexer.Consume(3);
    const toml_parser::CToken& winpath2 = lexer.Consume(3);
    const toml_parser::CToken& quoted	  = lexer.Consume(3);
    const toml_parser::CToken& regex	  = lexer.Consume(3);
    ASSERT_TRUE(winpath);
    ASSERT_TRUE(winpath2);
    ASSERT_TRUE(quoted);
    ASSERT_TRUE(regex);
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, winpath.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::literal_string, winpath.StringType());
    EXPECT_EQ("C:\\Users\\nodejs\\templates", winpath.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, winpath2.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::literal_string, winpath2.StringType());
    EXPECT_EQ("\\\\ServerX\\admin$\\system32\\", winpath2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, quoted.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::literal_string, quoted.StringType());
    EXPECT_EQ("Tom \"Dubs\" Preston-Werner", quoted.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, regex.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::literal_string, regex.StringType());
    EXPECT_EQ("<\\i\\c*\\s*>", regex.StringValue());
}

TEST(TOMLLexerTest, Datatype_String_LiteralStringMultiline)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& regex2 = lexer.Consume(3);
    const toml_parser::CToken& lines	= lexer.Consume(3);
    const toml_parser::CToken& quot15 = lexer.Consume(3);
    const toml_parser::CToken& apos15 = lexer.Consume(3);
    const toml_parser::CToken& str	= lexer.Consume(3);
    ASSERT_TRUE(regex2);
    ASSERT_TRUE(lines);
    ASSERT_TRUE(quot15);
    ASSERT_TRUE(apos15);
    ASSERT_TRUE(str);
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, regex2.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_literal, regex2.StringType());
    EXPECT_EQ("I [dw]on't need \\d{2} apples", regex2.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, lines.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_literal, lines.StringType());
    EXPECT_EQ("The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", lines.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, quot15.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_literal, quot15.StringType());
    EXPECT_EQ("Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", quot15.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, apos15.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::quoted_string, apos15.StringType());
    EXPECT_EQ("Here are fifteen apostrophes: '''''''''''''''", apos15.StringValue());
    EXPECT_EQ(toml_parser::ETokenCategory::token_string, str.Category());
    EXPECT_EQ(toml_parser::ETokenStringType::multi_line_literal, str.StringType());
    EXPECT_EQ("'That,' she said, 'is still pointless.'", str.StringValue());
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
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& invBareKey	 = lexer.Consume(1); // only [a-zA-Z0-9-_]allowed for bare key
    const toml_parser::CToken& invQuotedKey1 = lexer.Consume(3); // \u has to be followed by 4 times [0-9A-Fa-F]
    const toml_parser::CToken& invQuotedKey2 = lexer.Consume(3); // \U has to be followed by 8 times [0-9A-Fa-F]
    const toml_parser::CToken& invQuotedKey3 = lexer.Consume(3); // any escape sequence except specified in the TOML secs are invalid
    ASSERT_TRUE(invBareKey);    // only [a-zA-Z0-9-_]allowed for bare key
    ASSERT_TRUE(invQuotedKey1); // \u has to be followed by 4 times [0-9A-Fa-F]
    ASSERT_TRUE(invQuotedKey2); // \U has to be followed by 8 times [0-9A-Fa-F]
    ASSERT_TRUE(invQuotedKey3); // any escape sequence except specified in the TOML secs are invalid
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invBareKey.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invQuotedKey1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invQuotedKey2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invQuotedKey3.Category());
}

TEST(TOMLLexerTest, Invalid_String)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer1(R"(key1 = "invalid escape sequence \h)"s);
    toml_parser::CLexer lexer2(R"(key2 = """invalid escape sequence \h""")"s);
    toml_parser::CLexer lexer3(R"(key3 = "\u000")"s);
    toml_parser::CLexer lexer4(R"(key4 = "\U0000000")"s);
    // Tokens:
    // key1, '=', ErrorToken
    // key2, '=', ErrorToken
    // key3, '=', ErrorToken
    // key4, '=', ErrorToken
    const toml_parser::CToken& invString1 = lexer1.Consume(2);
    const toml_parser::CToken& invString2 = lexer2.Consume(2);
    const toml_parser::CToken& invString3 = lexer3.Consume(2);
    const toml_parser::CToken& invString4 = lexer4.Consume(2);
    ASSERT_TRUE(invString1);
    ASSERT_TRUE(invString2);
    ASSERT_TRUE(invString3);
    ASSERT_TRUE(invString4);
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invString1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invString2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invString3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invString4.Category());
}

TEST(TOMLLexerTest, Invalid_Integer)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& invInteger3  = lexer.Consume(3); // no leading zeros
    const toml_parser::CToken& invInteger4  = lexer.Consume(3); // invalid prefix
    const toml_parser::CToken& invInteger5  = lexer.Consume(3); // underscores only allowed between two digits
    const toml_parser::CToken& invInteger6  = lexer.Consume(3); // underscores only allowed between two digits
    const toml_parser::CToken& invInteger7  = lexer.Consume(3); // underscores not allowed after prefix
    const toml_parser::CToken& invInteger8  = lexer.Consume(3); // underscores not allowed after prefix
    const toml_parser::CToken& invInteger9  = lexer.Consume(3); // underscores not allowed after prefix
    const toml_parser::CToken& invInteger10 = lexer.Consume(3); // no leading + allowed
    const toml_parser::CToken& invInteger11 = lexer.Consume(3); // no leading + allowed
    const toml_parser::CToken& invInteger12 = lexer.Consume(3); // no leading + allowed
    const toml_parser::CToken& invInteger13 = lexer.Consume(3); // no negative number representation allowed with hex format
    const toml_parser::CToken& invInteger14 = lexer.Consume(3); // no negative number representation allowed with ocatal format
    const toml_parser::CToken& invInteger15 = lexer.Consume(3); // no negative number representation allowed with binary format
    ASSERT_TRUE(invInteger3);                                    // no leading zeros
    ASSERT_TRUE(invInteger4);                                    // invalid prefix
    ASSERT_TRUE(invInteger5);                                    // underscores only allowed between two digits
    ASSERT_TRUE(invInteger6);                                    // underscores only allowed between two digits
    ASSERT_TRUE(invInteger7);                                    // underscores not allowed after prefix
    ASSERT_TRUE(invInteger8);                                    // underscores not allowed after prefix
    ASSERT_TRUE(invInteger9);                                    // underscores not allowed after prefix
    ASSERT_TRUE(invInteger10);                                   // no leading + allowed
    ASSERT_TRUE(invInteger11);                                   // no leading + allowed
    ASSERT_TRUE(invInteger12);                                   // no leading + allowed
    ASSERT_TRUE(invInteger13);                                   // no negative number representation allowed with hex format
    ASSERT_TRUE(invInteger14);                                   // no negative number representation allowed with ocatal format
    ASSERT_TRUE(invInteger15);                                   // no negative number representation allowed with binary format
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger5.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger6.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger7.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger8.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger9.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger10.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger11.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger12.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger13.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger14.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invInteger15.Category());
}

TEST(TOMLLexerTest, Invalid_Float)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
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
    const toml_parser::CToken& invFloat1 = lexer.Consume(3); // decimal point has to be between two digits
    const toml_parser::CToken& invFloat2 = lexer.Consume(3); // decimal point has to be between two digits
    const toml_parser::CToken& invFloat3 = lexer.Consume(3); // decimal point has to be between two digits
    const toml_parser::CToken& invFloat4 = lexer.Consume(3); // special float values are always lower case
    const toml_parser::CToken& invFloat5 = lexer.Consume(3); // special float values are always lower case
    ASSERT_TRUE(invFloat1); // decimal point has to be between two digits
    ASSERT_TRUE(invFloat2); // decimal point has to be between two digits
    ASSERT_TRUE(invFloat3); // decimal point has to be between two digits
    ASSERT_TRUE(invFloat4); // special float values are always lower case
    ASSERT_TRUE(invFloat5); // special float values are always lower case
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invFloat1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invFloat2.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invFloat3.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invFloat4.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invFloat5.Category());
}

TEST(TOMLLexerTest, Invalid_Boolean)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        invBool1 = True
        invBool2 = False
        )"s);
    // Tokens:
    // NewLine
    // invBool1, '=', ErrorToken, NewLine
    // invBool2, '=', ErrorToken, NewLine
    const toml_parser::CToken& invBool1 = lexer.Consume(3); // keyword for true has to be lower case
    const toml_parser::CToken& invBool2 = lexer.Consume(3); // keyword for false has to be lower case
    ASSERT_TRUE(invBool1); // keyword for true has to be lower case
    ASSERT_TRUE(invBool2); // keyword for false has to be lower case
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invBool1.Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_error, invBool2.Category());
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
    toml_parser::CLexer lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    ASSERT_TRUE(lexer.Peek());
    ASSERT_TRUE(lexer.Peek());
    ASSERT_TRUE(lexer.Peek(1));
    ASSERT_TRUE(lexer.Peek(1));
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, lexer.Peek().Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, lexer.Peek().Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, lexer.Peek(1).Category());
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, lexer.Peek(1).Category());
}

TEST(TOMLLexerTest, Consume_Advance)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    auto ptr = lexer.Consume();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, ptr.Category());
    ptr = lexer.Consume();
    ASSERT_TRUE(ptr);
    EXPECT_EQ(toml_parser::ETokenCategory::token_key, ptr.Category());
    ptr = lexer.Consume(2);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_new_line, ptr.Category());
    ptr = lexer.Consume(1);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(toml_parser::ETokenCategory::token_syntax_assignment, ptr.Category());
}

TEST(TOMLLexerTest, PeekConsume_BoundsCheck)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"(
        key1 = "value1"
        key2 = "value2"
        )"s);
    // Tokens:
    // NewLine
    // key1, '=', value1, NewLine
    // key2, '=', value2, NewLine
    EXPECT_FALSE(lexer.Consume(10));
    EXPECT_FALSE(lexer.Consume());
    EXPECT_FALSE(lexer.Peek());
    EXPECT_FALSE(lexer.Peek(2));
    EXPECT_FALSE(lexer.Peek(0));
    EXPECT_FALSE(lexer.Consume(0));
}

TEST(TOMLLexerTest, PeekConsume_EmptyInput)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer(R"()"s);
    // Tokens:
    EXPECT_FALSE(lexer.Peek());
    EXPECT_FALSE(lexer.Peek(1));
    EXPECT_FALSE(lexer.Consume());
    EXPECT_FALSE(lexer.Consume(1));
}

TEST(TOMLLexerTest, ExceptionHandling)
{
    using namespace std::string_literals;
    toml_parser::CLexer lexer;
    try
    {
        lexer.Feed(std::string("") + static_cast<char>(0xC1));
    }
    catch (const sdv::toml::XTOMLParseException&)
    {
    }

    // Tokens:
    // token_terminated-Token
    EXPECT_EQ(toml_parser::ETokenCategory::token_terminated, lexer.Consume().Category());
    EXPECT_FALSE(lexer.Consume());
    EXPECT_FALSE(lexer.Peek());
    EXPECT_FALSE(lexer.Peek(2));
    EXPECT_FALSE(lexer.Peek(0));
    EXPECT_FALSE(lexer.Consume(0));
}

TEST(TOMLLexerTest, DISABLED_RegenerateTOML)
{
    using namespace std::string_literals;
    std::string ssOrginal = R"(
        # original comments
        key1 = "value1"                 # follow comments
        "key\u0032" = "value2"
        "key3\U0001F92B" = "abc"
        '' = "valid"
        'literal "key"' = "also valid"
        "quoted 'key'" = "also valid"
        _0xabcd = 0xabcd
        1234 = 1234
        [table]
        array = [[1,2],[3,4]]
        emptyArray = []
        [["table array"]]
        fruit.name = "banana"     # this is best practice
        fruit. color = "yellow"    # same as fruit.color
        fruit . flavor = "banana"   # same as fruit.flavor
        a.b.c = 2
        [dog."tater.man"]
        inlineTable = { name = "Some Name", type = "Some Type" }
        nestedInlineTable = { a = { a = 0, b = "b"}, e = { c = 0, d = "d"} }
        emptyTable = {}
        [[tableArray]]
        [[fruits.varieties]]
        [["this is also a 'key'"]]
        [['this is also a "key"']]
        [[a."b.c"]]
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
        intErr1 = _1234
        intErr2 = 1__234
        intErr3 = 1234_
        intErr4 = +0xabcd
        intErr5 = -0xabcd
        intErr6 = 0xabc__d
        intErr7 = 0x_abcd
        intErr8 = 0xabcd_
        intErr9 = 0_xabcd
        intErr10 = 0XDEADBEEF
        float1 = +1.0
        float2 = 3.1415
        float3 = -0.01
        float4 = 5e+22
        float5 = 1e06
        float6 = -2E-2
        float7 = 6.626e-34
        float8 = 6.6_2_6e-3_4
        float9 = 224_617.445_991_228
        float10 = -0.0
        float11 = +0.0
        specialFloat12 = inf
        specialFloat13 = +inf
        specialFloat14 = -inf
        specialFloat15 = nan
        specialFloat16 = +nan
        specialFloat17 = -nan
        errFloat18 = 224__617.445_991_228
        errFloat19 = _224_617.445_991_228
        errFloat20 = 224_617_.445_991_228
        errFloat21 = 224_617._445_991_228
        errFloat22 = 224_617.445_991_228_
        errFloat23 = 224_617.445_991__228
        errFloat24 = 6.626_e-34
        errFloat25 = 6.626e_-34
        errFloat26 = 6.626e-_34
        errFloat27 = 6.626e-34_
        errFloat28 = 6.626_e-34
        key1 = true
        key2 = false
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
        winpath  = 'C:\Users\nodejs\templates'
        winpath2 = '\\ServerX\admin$\system32\'
        quoted   = 'Tom "Dubs" Preston-Werner'
        regex    = '<\i\c*\s*>'
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
        keyä = "value1"
        "key\u123" = "value2"
        "key\U1234" = "value3"
        "key\k" = "value4"
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
        invFloat1 = .7
        invFloat2 = 1.
        invFloat3 = 3.e4
        invFloat4 = Inf
        invFloat5 = NaN
        invBool1 = True
        invBool2 = False
        )"s;

    toml_parser::CLexer lexer(ssOrginal);

    // TODO EVE
    //std::string ssCopy = lexer.GenerateTOML();

    //EXPECT_EQ(ssOrginal, ssCopy);
}
