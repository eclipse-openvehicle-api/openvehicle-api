#include <functional>
#include <gtest/gtest.h>
#include <limits>

#include "../../../sdv_services/core/toml_parser/exception.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"

TEST(TOMLMiscellaneousTests, Hex2Dec)
{
    // No string
    std::string ssEmpty;
    EXPECT_THROW(toml_parser::HexadecimalToDecimal(ssEmpty), sdv::toml::XTOMLParseException);

    // Invalid string
    std::string ssNoNumber = "xyz";
    EXPECT_THROW(toml_parser::HexadecimalToDecimal(ssNoNumber), sdv::toml::XTOMLParseException);

    // Parse value
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("10"), 16u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("a"), 10u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("AbCd1234"), 2882343476u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("0"), 0u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("7fffffff"), 2147483647u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("80000000"), 2147483648u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("FFFFFFFF"), 4294967295u);

    // Parse up to max value
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("0FFFFFFFF"), 4294967295u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("FFFFFFFFA"), 4294967295u);

    // Parse until unknown character
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("0xyz"), 0u);
    EXPECT_EQ(toml_parser::HexadecimalToDecimal("10xyz"), 16u);
}

TEST(TOMLMiscellaneousTests, Dec2Dec)
{
    // No string
    std::string ssEmpty;
    EXPECT_THROW(toml_parser::DecimalToDecimal(ssEmpty), sdv::toml::XTOMLParseException);

    // Invalid string
    std::string ssNoNumber = "f123";
    EXPECT_THROW(toml_parser::DecimalToDecimal(ssNoNumber), sdv::toml::XTOMLParseException);

    // Parse value
    EXPECT_EQ(toml_parser::DecimalToDecimal("16"), 16u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("10"), 10u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("2882343476"), 2882343476u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("0"), 0u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("2147483647"), 2147483647u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("2147483648"), 2147483648u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("4294967295"), 4294967295u);

    // Parse up to max value
    EXPECT_EQ(toml_parser::DecimalToDecimal("04294967295"), 4294967295u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("42949672950"), 4294967295u);

    // Parse until unknown character
    EXPECT_EQ(toml_parser::DecimalToDecimal("0xyz"), 0u);
    EXPECT_EQ(toml_parser::DecimalToDecimal("10xyz"), 10u);
}

TEST(TOMLMiscellaneousTests, Oct2Dec)
{
    // No string
    std::string ssEmpty;
    EXPECT_THROW(toml_parser::OctalToDecimal(ssEmpty), sdv::toml::XTOMLParseException);

    // Invalid string
    std::string ssNoNumber = "890";
    EXPECT_THROW(toml_parser::OctalToDecimal(ssNoNumber), sdv::toml::XTOMLParseException);

    // Parse value
    EXPECT_EQ(toml_parser::OctalToDecimal("20"), 16u);
    EXPECT_EQ(toml_parser::OctalToDecimal("12"), 10u);
    EXPECT_EQ(toml_parser::OctalToDecimal("25363211064"), 2882343476u);
    EXPECT_EQ(toml_parser::OctalToDecimal("0"), 0u);
    EXPECT_EQ(toml_parser::OctalToDecimal("17777777777"), 2147483647u);
    EXPECT_EQ(toml_parser::OctalToDecimal("20000000000"), 2147483648u);
    EXPECT_EQ(toml_parser::OctalToDecimal("37777777777"), 4294967295u);

    // Parse up to max value
    EXPECT_EQ(toml_parser::OctalToDecimal("037777777777"), 4294967295u);
    EXPECT_EQ(toml_parser::OctalToDecimal("377777777770"), 4294967295u);

    // Parse until unknown character
    EXPECT_EQ(toml_parser::OctalToDecimal("0xyz"), 0u);
    EXPECT_EQ(toml_parser::OctalToDecimal("12xyz"), 10u);
}

TEST(TOMLMiscellaneousTests, Bin2Dec)
{
    // No string
    std::string ssEmpty;
    EXPECT_THROW(toml_parser::BinaryToDecimal(ssEmpty), sdv::toml::XTOMLParseException);

    // Invalid string
    std::string ssNoNumber = "234";
    EXPECT_THROW(toml_parser::BinaryToDecimal(ssNoNumber), sdv::toml::XTOMLParseException);

    // Parse value
    EXPECT_EQ(toml_parser::BinaryToDecimal("10000"), 16u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("1010"), 10u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("10101011110011010001001000110100"), 2882343476u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("0"), 0u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("1111111111111111111111111111111"), 2147483647u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("10000000000000000000000000000000"), 2147483648u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("11111111111111111111111111111111"), 4294967295u);

    // Parse up to max value
    EXPECT_EQ(toml_parser::BinaryToDecimal("011111111111111111111111111111111"), 4294967295u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("111111111111111111111111111111110"), 4294967295u);

    // Parse until unknown character
    EXPECT_EQ(toml_parser::BinaryToDecimal("0xyz"), 0u);
    EXPECT_EQ(toml_parser::BinaryToDecimal("1010yz"), 10u);
}

TEST(TOMLMiscellaneousTests, UnicodeCharacter)
{
    // No string
    std::string ssEmpty;
    EXPECT_THROW(toml_parser::EscapedUnicodeCharacterToUTF8(ssEmpty), sdv::toml::XTOMLParseException);

    // Invalid string
    std::string ssNoNumber = "xyz";
    EXPECT_THROW(toml_parser::EscapedUnicodeCharacterToUTF8(ssNoNumber), sdv::toml::XTOMLParseException);

    // Conversion
    EXPECT_EQ(toml_parser::EscapedUnicodeCharacterToUTF8("042f"), u8"\u042f");
    EXPECT_EQ(toml_parser::EscapedUnicodeCharacterToUTF8("0000042f"), u8"\U0000042f");
    EXPECT_EQ(toml_parser::EscapedUnicodeCharacterToUTF8("0001F600"), u8"\U0001F600");
}

TEST(TOMLMiscellaneousTests, SplitKeyStringEmpty)
{
    std::string ssKeyEmpty;
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKeyEmpty);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitKeyFirstPartOnly)
{
    std::string ssKey = "abc";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "_abc";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "_abc");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "-abc";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "-abc");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "1234";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1234");
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Although invalid bare key
    ssKey = "abc/";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid bare key
    ssKey = "abc\\";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid bare key
    ssKey = "abc def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitStandardBareKey)
{
    std::string ssKey = "abc.def";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "def");

    ssKey = "_abc._def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "_abc");
    EXPECT_EQ(prSplittedKey.second, "_def");

    ssKey = "-abc.-def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "-abc");
    EXPECT_EQ(prSplittedKey.second, "-def");

    ssKey = "1234.5678";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1234");
    EXPECT_EQ(prSplittedKey.second, "5678");

    ssKey = "1234.5678.90";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1234");
    EXPECT_EQ(prSplittedKey.second, "5678.90");

    // Invalid bare key
    ssKey = "/abc./def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid bare key
    ssKey = "\\abc.\\def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitBareKeyWithSpace)
{
    std::string ssKey = " abc.def";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "def");

    ssKey = "abc . def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, " def");

    ssKey = "\tabc.\tdef";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "\tdef");

    ssKey = "abc\n.\ndef";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "\ndef");

    // Invalid key
    ssKey = "abc def . ghi jkl";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitLiteralKey)
{
    std::string ssKey = "'abc'";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "'abc.def'";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc.def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "'abc'.'def'";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "'def'");

    ssKey = "'\"cool\" key'.'very \"cool\" key'";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "\"cool\" key");
    EXPECT_EQ(prSplittedKey.second, "'very \"cool\" key'");

    ssKey = "'abc/def'";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc/def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "'abc\\ndef'";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\\ndef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "'abc.def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "cool' 'key";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitQuotedKey)
{
    std::string ssKey = "\"abc\"";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc.def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc.def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\".\"def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "\"def\"");

    ssKey = "\"'cool' key\".\"very 'cool' key\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "'cool' key");
    EXPECT_EQ(prSplittedKey.second, "\"very 'cool' key\"");

    ssKey = "\"abc/def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc/def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "\"abc.def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "cool\" \"key";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitEscapedQuotedKey)
{
    std::string ssKey = "\"abc\\bdef\"";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\bdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\tdef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\tdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\ndef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\ndef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\fdef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\fdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\rdef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\rdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\\"def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\"def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\\\def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc\\def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\u042fdef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, u8"abc\u042fdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\U0000042fdef\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, u8"abc\U0000042fdef");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "\"abc\\U0001F600def\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, u8"abc\U0001F600def");
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "\"abc\\uxyz\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "\"abc\\Uxyz\"";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SplitTableKey)
{
    std::string ssKey = "abc.def";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "def");

    ssKey = "abc.def.ghi";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "def.ghi");

    ssKey = ".abc.def.ghi";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "def.ghi");
}

TEST(TOMLMiscellaneousTests, SplitArrayKey)
{
    std::string ssKey = "abc[1]";
    auto prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "[1]");

    ssKey = "abc[1][2]";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "[1][2]");

    ssKey = "[1]";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1");
    EXPECT_TRUE(prSplittedKey.second.empty());

    ssKey = "[1][2]";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1");
    EXPECT_EQ(prSplittedKey.second, "[2]");

    ssKey = "[1].abc";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1");
    EXPECT_EQ(prSplittedKey.second, "abc");

    ssKey = ".[1].abc";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "1");
    EXPECT_EQ(prSplittedKey.second, "abc");

    ssKey = "abc [ 1 ] [ 2 ] [ 3 ] . def";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_EQ(prSplittedKey.first, "abc");
    EXPECT_EQ(prSplittedKey.second, "[ 1 ] [ 2 ] [ 3 ] . def");

    // Invalid key
    ssKey = "[1]abc";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "[1.2][2]";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "[1";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());

    // Invalid key
    ssKey = "[1[2]]";
    prSplittedKey = toml_parser::SplitNodeKey(ssKey);
    EXPECT_TRUE(prSplittedKey.first.empty());
    EXPECT_TRUE(prSplittedKey.second.empty());
}

TEST(TOMLMiscellaneousTests, SmartQuoteBareKeys)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::smart_key), "abc");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::smart_key), "123");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::smart_key), "ABC");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::smart_key), "abc_def");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::smart_key), "ABC-DEF");
}

TEST(TOMLMiscellaneousTests, SmartQuoteSpecialCharsKeys)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::smart_key), "\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::smart_key), "\"abc def\"");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::smart_key), "\".abc\"");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::smart_key),
        "\"\\u00B5\""); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::smart_key), "\"\\u00A1\"");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::smart_key), "\"abc/\"");
}

TEST(TOMLMiscellaneousTests, SmartQuoteEscapeCharsKeys)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::smart_key), "\"abc\\tdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::smart_key), "\"abc\\\\def\"");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::smart_key), "\"\\\"abc\\\"\"");
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::smart_key), "\"'abc'\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::smart_key), "\"abc\\bdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::smart_key), "\"abc\\ndef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::smart_key), "\"abc\\fdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::smart_key), "\"abc\\rdef\"");
}

TEST(TOMLMiscellaneousTests, SmartQuoteControlCharsKeys)
{
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::smart_key), "\"\\u0000\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::smart_key), "\"\\u0001\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::smart_key), "\"\\u0002\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::smart_key), "\"\\u0003\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::smart_key), "\"\\u0004\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::smart_key), "\"\\u0005\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::smart_key), "\"\\u0006\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::smart_key), "\"\\u0007\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::smart_key), "\"\\u000B\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::smart_key), "\"\\u000E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::smart_key), "\"\\u000F\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::smart_key), "\"\\u0010\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::smart_key), "\"\\u0011\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::smart_key), "\"\\u0012\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::smart_key), "\"\\u0013\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::smart_key), "\"\\u0014\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::smart_key), "\"\\u0015\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::smart_key), "\"\\u0016\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::smart_key), "\"\\u0017\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::smart_key), "\"\\u0018\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::smart_key), "\"\\u0019\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::smart_key), "\"\\u001A\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::smart_key), "\"\\u001B\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::smart_key), "\"\\u001C\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::smart_key), "\"\\u001D\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::smart_key), "\"\\u001E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::smart_key), "\"\\u001F\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::smart_key), "\"\\u007F\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, SmartQuoteText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::smart_text), "\"abc\"");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::smart_text), "\"123\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::smart_text), "\"ABC\"");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::smart_text), "\"abc_def\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::smart_text), "\"ABC-DEF\"");
}

TEST(TOMLMiscellaneousTests, SmartQuoteSpecialCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::smart_text), "\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::smart_text), "\"abc def\"");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::smart_text), "\".abc\"");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::smart_text),
        "\"\\u00B5\""); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::smart_text), "\"\\u00A1\"");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::smart_text), "\"abc/\"");
}

TEST(TOMLMiscellaneousTests, SmartQuoteEscapeCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::smart_text), "\"abc\\tdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::smart_text), "'abc\\def'");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::smart_text), "'\"abc\"'");      // becomes literal text
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::smart_text), "\"'abc'\"");        // becomes literal text
    EXPECT_EQ(toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::smart_text), "\"abc\\bdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::smart_text), "\"abc\\ndef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::smart_text), "\"abc\\fdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::smart_text), "\"abc\\rdef\"");
}

TEST(TOMLMiscellaneousTests, SmartQuoteControlCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::smart_text), "\"\\u0000\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::smart_text), "\"\\u0001\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::smart_text), "\"\\u0002\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::smart_text), "\"\\u0003\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::smart_text), "\"\\u0004\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::smart_text), "\"\\u0005\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::smart_text), "\"\\u0006\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::smart_text), "\"\\u0007\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::smart_text), "\"\\u000B\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::smart_text), "\"\\u000E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::smart_text), "\"\\u000F\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::smart_text), "\"\\u0010\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::smart_text), "\"\\u0011\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::smart_text), "\"\\u0012\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::smart_text), "\"\\u0013\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::smart_text), "\"\\u0014\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::smart_text), "\"\\u0015\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::smart_text), "\"\\u0016\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::smart_text), "\"\\u0017\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::smart_text), "\"\\u0018\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::smart_text), "\"\\u0019\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::smart_text), "\"\\u001A\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::smart_text), "\"\\u001B\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::smart_text), "\"\\u001C\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::smart_text), "\"\\u001D\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::smart_text), "\"\\u001E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::smart_text), "\"\\u001F\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::smart_text), "\"\\u007F\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, QuotedText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::quoted_text), "\"abc\"");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::quoted_text), "\"123\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::quoted_text), "\"ABC\"");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::quoted_text), "\"abc_def\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::quoted_text), "\"ABC-DEF\"");
}

TEST(TOMLMiscellaneousTests, QuotedSpecialCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::quoted_text), "\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::quoted_text), "\"abc def\"");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::quoted_text), "\".abc\"");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::quoted_text),
        "\"\\u00B5\""); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::quoted_text), "\"\\u00A1\"");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::quoted_text), "\"abc/\"");
}

TEST(TOMLMiscellaneousTests, QuotedEscapeCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::quoted_text), "\"abc\\tdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::quoted_text), "\"abc\\\\def\"");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::quoted_text), "\"\\\"abc\\\"\"");
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::quoted_text), "\"'abc'\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::quoted_text), "\"abc\\bdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::quoted_text), "\"abc\\ndef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::quoted_text), "\"abc\\fdef\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::quoted_text), "\"abc\\rdef\"");
}

TEST(TOMLMiscellaneousTests, QuotedControlCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::quoted_text), "\"\\u0000\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::quoted_text), "\"\\u0001\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::quoted_text), "\"\\u0002\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::quoted_text), "\"\\u0003\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::quoted_text), "\"\\u0004\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::quoted_text), "\"\\u0005\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::quoted_text), "\"\\u0006\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::quoted_text), "\"\\u0007\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::quoted_text), "\"\\u000B\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::quoted_text), "\"\\u000E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::quoted_text), "\"\\u000F\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::quoted_text), "\"\\u0010\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::quoted_text), "\"\\u0011\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::quoted_text), "\"\\u0012\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::quoted_text), "\"\\u0013\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::quoted_text), "\"\\u0014\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::quoted_text), "\"\\u0015\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::quoted_text), "\"\\u0016\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::quoted_text), "\"\\u0017\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::quoted_text), "\"\\u0018\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::quoted_text), "\"\\u0019\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::quoted_text), "\"\\u001A\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::quoted_text), "\"\\u001B\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::quoted_text), "\"\\u001C\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::quoted_text), "\"\\u001D\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::quoted_text), "\"\\u001E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::quoted_text), "\"\\u001F\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::quoted_text), "\"\\u007F\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, LiteralText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::literal_text), "'abc'");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::literal_text), "'123'");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::literal_text), "'ABC'");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::literal_text), "'abc_def'");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::literal_text), "'ABC-DEF'");
}

TEST(TOMLMiscellaneousTests, LiteralSpecialCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::literal_text), "''");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::literal_text), "'abc def'");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::literal_text), "'.abc'");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::literal_text),
        u8"'\u00B5'"); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::literal_text), u8"'\u00A1'");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::literal_text), "'abc/'");
}

TEST(TOMLMiscellaneousTests, LiteralEscapeCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::literal_text), "\"abc\\tdef\"");   // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::literal_text), "'abc\\def'");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::literal_text), "'\"abc\"'");
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::literal_text), "\"'abc'\"");          // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::literal_text), "\"abc\\bdef\"");   // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::literal_text), "\"abc\\ndef\"");   // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::literal_text), "\"abc\\fdef\"");   // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::literal_text), "\"abc\\rdef\"");   // Becomes quoted
}

TEST(TOMLMiscellaneousTests, LiteralControlCharsText)
{
    // All become quoted insteda of literal (due to control character).
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::literal_text), "\"\\u0000\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::literal_text), "\"\\u0001\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::literal_text), "\"\\u0002\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::literal_text), "\"\\u0003\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::literal_text), "\"\\u0004\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::literal_text), "\"\\u0005\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::literal_text), "\"\\u0006\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::literal_text), "\"\\u0007\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::literal_text), "\"\\u000B\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::literal_text), "\"\\u000E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::literal_text), "\"\\u000F\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::literal_text), "\"\\u0010\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::literal_text), "\"\\u0011\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::literal_text), "\"\\u0012\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::literal_text), "\"\\u0013\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::literal_text), "\"\\u0014\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::literal_text), "\"\\u0015\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::literal_text), "\"\\u0016\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::literal_text), "\"\\u0017\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::literal_text), "\"\\u0018\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::literal_text), "\"\\u0019\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::literal_text), "\"\\u001A\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::literal_text), "\"\\u001B\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::literal_text), "\"\\u001C\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::literal_text), "\"\\u001D\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::literal_text), "\"\\u001E\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::literal_text), "\"\\u001F\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::literal_text), "\"\\u007F\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, MultiLineQuotedText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"123\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"ABC\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc_def\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"ABC-DEF\"\"\"");
}

TEST(TOMLMiscellaneousTests, MultiLineQuotedSpecialCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc def\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\".abc\"\"\"");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::multi_line_quoted_text),
        "\"\"\"\\u00B5\"\"\""); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u00A1\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc/\"\"\"");
}

TEST(TOMLMiscellaneousTests, MultiLineQuotedEscapeCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\\tdef\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\\\\def\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\\"abc\\\"\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"'abc'\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\\bdef\"\"\"");
    // Multi line
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\ndef\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\\fdef\"\"\"");
    // Multi line
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"abc\rdef\"\"\"");
}

TEST(TOMLMiscellaneousTests, MultiLineQuotedControlCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0000\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0001\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0002\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0003\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0004\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0005\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0006\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0007\"\"\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u000B\"\"\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u000E\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u000F\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0010\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0011\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0012\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0013\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0014\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0015\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0016\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0017\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0018\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u0019\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001A\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001B\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001C\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001D\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001E\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u001F\"\"\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::multi_line_quoted_text), "\"\"\"\\u007F\"\"\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, MultiLineLiteralText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc'''");
    EXPECT_EQ(toml_parser::QuoteText("123", toml_parser::EQuoteRequest::multi_line_literal_text), "'''123'''");
    EXPECT_EQ(toml_parser::QuoteText("ABC", toml_parser::EQuoteRequest::multi_line_literal_text), "'''ABC'''");
    EXPECT_EQ(toml_parser::QuoteText("abc_def", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc_def'''");
    EXPECT_EQ(toml_parser::QuoteText("ABC-DEF", toml_parser::EQuoteRequest::multi_line_literal_text), "'''ABC-DEF'''");
}

TEST(TOMLMiscellaneousTests, MultiLineLiteralSpecialCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("", toml_parser::EQuoteRequest::multi_line_literal_text), "''''''");
    EXPECT_EQ(toml_parser::QuoteText("abc def", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc def'''");
    EXPECT_EQ(toml_parser::QuoteText(".abc", toml_parser::EQuoteRequest::multi_line_literal_text), "'''.abc'''");
#ifndef __GNUC__
    EXPECT_EQ(toml_parser::QuoteText(u8"µ", toml_parser::EQuoteRequest::multi_line_literal_text),
        u8"'''\u00B5'''"); // Only supported on Windows with MS compiler
#endif
    EXPECT_EQ(toml_parser::QuoteText(u8"\u00a1", toml_parser::EQuoteRequest::multi_line_literal_text), u8"'''\u00A1'''");
    EXPECT_EQ(toml_parser::QuoteText("abc/", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc/'''");
}

TEST(TOMLMiscellaneousTests, MultiLineLiteralEscapeCharsText)
{
    EXPECT_EQ(toml_parser::QuoteText("abc\tdef", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"abc\\tdef\"\"\""); // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\\def", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc\\def'''");
    EXPECT_EQ(toml_parser::QuoteText("\"abc\"", toml_parser::EQuoteRequest::multi_line_literal_text), "'''\"abc\"'''");
    EXPECT_EQ(toml_parser::QuoteText("'abc'", toml_parser::EQuoteRequest::multi_line_literal_text), "''''abc''''");
    EXPECT_EQ(
        toml_parser::QuoteText("abc\bdef", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"abc\\bdef\"\"\""); // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\ndef", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc\ndef'''");
    EXPECT_EQ(toml_parser::QuoteText("abc\fdef", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"abc\\fdef\"\"\""); // Becomes quoted
    EXPECT_EQ(toml_parser::QuoteText("abc\rdef", toml_parser::EQuoteRequest::multi_line_literal_text), "'''abc\rdef'''");
}

TEST(TOMLMiscellaneousTests, MultiLineLiteralControlCharsText)
{
    // All become quoted insteda of literal (due to control character).
    EXPECT_EQ(toml_parser::QuoteText(std::string(1, '\0'), toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0000\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0001", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0001\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0002", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0002\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0003", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0003\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0004", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0004\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0005", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0005\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0006", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0006\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0007", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0007\"\"\"");
    // 0008 = backspace (\b), 0009 = tab (\t), 000a = linefeed (\n)
    EXPECT_EQ(toml_parser::QuoteText("\u000b", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u000B\"\"\"");
    // 000c = form feed (\f), 000d = carriage return (\r)
    EXPECT_EQ(toml_parser::QuoteText("\u000e", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u000E\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u000f", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u000F\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0010", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0010\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0011", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0011\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0012", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0012\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0013", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0013\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0014", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0014\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0015", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0015\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0016", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0016\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0017", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0017\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0018", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0018\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u0019", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u0019\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001a", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001A\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001b", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001B\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001c", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001C\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001d", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001D\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001e", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001E\"\"\"");
    EXPECT_EQ(toml_parser::QuoteText("\u001f", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u001F\"\"\"");
    // 0020..0021 are treated as characters (partly quotation needed)
    // 0022 = quote (\")
    // 0023..005b are treated as characters (partly quotation needed)
    // 005c = backslash (\\)
    // 005d..007e are treated as characters (partly quotation needed)
    EXPECT_EQ(toml_parser::QuoteText("\u007f", toml_parser::EQuoteRequest::multi_line_literal_text), "\"\"\"\\u007F\"\"\"");
    // 0080... and higher are treated as unicode character (quotation needed)
}

TEST(TOMLMiscellaneousTests, ExtractBareKeyName)
{
    EXPECT_EQ(toml_parser::ExtractKeyName(""), "");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc.def"), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc"), "abc");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc.def.ghi"), "ghi");
    EXPECT_EQ(toml_parser::ExtractKeyName("_abc.def"), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("_abc._def"), "_def");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc-.def"), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc-.def-"), "def-");
    EXPECT_EQ(toml_parser::ExtractKeyName("1234"), "1234");
}

TEST(TOMLMiscellaneousTests, ExtractQuotedKeyName)
{
    EXPECT_EQ(toml_parser::ExtractKeyName("\"\""), "");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"abc\""), "abc");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc.\"def\""), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"abc.def\""), "abc.def");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"abc\\tdef\""), "abc\tdef");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"'This' is a \\\"very\\\" quoted key!\""), "'This' is a \"very\" quoted key!");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"[1]\""), "[1]");
    EXPECT_EQ(toml_parser::ExtractKeyName("\"abc"), "");        // Failure
    EXPECT_EQ(toml_parser::ExtractKeyName("abc\"def\""), "");   // Failure
}

TEST(TOMLMiscellaneousTests, ExtractLiteralKeyName)
{
    EXPECT_EQ(toml_parser::ExtractKeyName("''"), "");
    EXPECT_EQ(toml_parser::ExtractKeyName("'abc'"), "abc");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc.'def'"), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("'abc.def'"), "abc.def");
    EXPECT_EQ(toml_parser::ExtractKeyName("'abc def'"), "abc def");
    EXPECT_EQ(toml_parser::ExtractKeyName("'abc\\tdef'"), "abc\\tdef");    // No escape  uences supported
    EXPECT_EQ(toml_parser::ExtractKeyName("'This is a \"very\" literal key!'"), "This is a \"very\" literal key!");
    EXPECT_EQ(toml_parser::ExtractKeyName("'[1]'"), "[1]");
    EXPECT_EQ(toml_parser::ExtractKeyName("'abc"), "");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc'def"), ""); // Failure
}

TEST(TOMLMiscellaneousTests, ExtractIndexKeyName)
{
    EXPECT_EQ(toml_parser::ExtractKeyName("[0]"), "0");
    EXPECT_EQ(toml_parser::ExtractKeyName("[0][1]"), "1");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0][1]"), "1");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0].def"), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0].\"def\""), "def");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0].def[1]"), "1");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0].\"def\"[1]"), "1");
    EXPECT_EQ(toml_parser::ExtractKeyName("abc[0]def"), "");    // Failure
}
