#include "includes.h"
#include "lexer_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/support.h"

using CText2CTextTranslator = CLexerTest;

TEST_F(CText2CTextTranslator, InterpretCTextAscii)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText("Hello", nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText("Hello\\nYou there", nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextUTF8)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText("Hello", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, "Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText("Hello\\nYou there", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, "Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextUTF16)
{
    std::u16string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText("Hello", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, u"Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText("Hello\\nYou there", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, u"Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextUTF32)
{
    std::u32string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText("Hello", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, U"Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText("Hello\\nYou there", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, U"Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextWide)
{
    std::wstring ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText("Hello", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, L"Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText("Hello\\nYou there", nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, L"Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextCodePos)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    EXPECT_NO_THROW(InterpretCText(CCodePos("Hello"), nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "Hello");
    EXPECT_EQ(uiByteCnt, 5u);

    // Text with escape sequence
    EXPECT_NO_THROW(InterpretCText(CCodePos("Hello\\nYou there"), nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "Hello\nYou there");
    EXPECT_EQ(uiByteCnt, 16u);
}

TEST_F(CText2CTextTranslator, InterpretCTextWithDelimiter)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Standard text
    const char szCodeText1[] = "const char* sz = \"This is a text\";";
    EXPECT_NO_THROW(InterpretCText(szCodeText1 + 18, "\"", ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "This is a text");
    EXPECT_EQ(uiByteCnt, 14u);

    // Escape character text
    const char szCodeText2[] = "const char* sz = \"This is a\\ntext\";";
    EXPECT_NO_THROW(InterpretCText(szCodeText2 + 18, "\"", ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "This is a\ntext");
    EXPECT_EQ(uiByteCnt, 15u);

    // Standard char
    const char szCodeChar1[] = "const char c = \'T\';";
    EXPECT_NO_THROW(InterpretCText(szCodeChar1 + 16, "\'", ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "T");
    EXPECT_EQ(uiByteCnt, 1u);

    // Escaped char
    const char szCodeChar2[] = "const char c = \'\\a\';";
    EXPECT_NO_THROW(InterpretCText(szCodeChar2 + 16, "\'", ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "\a");
    EXPECT_EQ(uiByteCnt, 2u);

    // Raw text
    const char szCodeRaw1[] = R"code(abc(this is a delimiter check)abc)code";
    EXPECT_NO_THROW(InterpretCText(szCodeRaw1 + 4, ")abc", ssResult, uiByteCnt, true, true));
    EXPECT_EQ(ssResult, "this is a delimiter check");
    EXPECT_EQ(uiByteCnt, 25u);

    // Raw text with newline
    const char szCodeRaw2[] = R"code(abc(this is a delimiter\check)abc)code";
    EXPECT_NO_THROW(InterpretCText(szCodeRaw2 + 4, ")abc", ssResult, uiByteCnt, true, true));
    EXPECT_EQ(ssResult, "this is a delimiter\\check");
    EXPECT_EQ(uiByteCnt, 25u);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCText)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\\'\\\"\\?\\\\\\a\\b\\f\\n\\r\\t\\v\\265\\40\\xB5\\xb5";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "\'\"?\\\a\b\f\n\r\t\v\265 \265\265");
    EXPECT_EQ(uiByteCnt, 37u);

    // Invalid escapes
    const char szInvalidEscape1[] = "\\p";
    EXPECT_THROW(InterpretCText(szInvalidEscape1, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
    const char szInvalidEscape2[] = "\\400";
    EXPECT_THROW(InterpretCText(szInvalidEscape2, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
    const char szInvalidEscape3[] = "\\xx";
    EXPECT_THROW(InterpretCText(szInvalidEscape3, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCTextUnicodeASCII)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\u0040\\u00ff\\U00000040\\U000000FF";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false, true));
    EXPECT_EQ(ssResult, "@\xff@\xff");
    EXPECT_EQ(uiByteCnt, 32u);

    // Invalid escaped
    const char szInvalidEscaped1[] = "\\u0140";     // ASCII doesn't support this character
    EXPECT_THROW(InterpretCText(szInvalidEscaped1, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
    const char szInvalidEscaped2[] = "\\u040xyz";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped2, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
    const char szInvalidEscaped3[] = "\\U0000000";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped3, nullptr, ssResult, uiByteCnt, false, true), CCompileException);
    const char szInvalidEscaped4[] = "\\uD800";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped4, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped5[] = "\\udfff";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped5, nullptr, ssResult, uiByteCnt, false), CCompileException);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCTextUnicodeUTF8)
{
    std::string ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\u0040\\u00ff\\U00000040\\U000000FF\\u0024\\u00a3\\u0939\\u20AC\\ud55c\\U00010348";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, u8"@\u00ff@\u00ff\u0024\u00a3\u0939\u20AC\ud55c\U00010348");
    // ATTENTION: The following line will run okay with MSVC, but causes an error with GNU-C. This is because GNU-C is translating
    // the string u8"\xff" wronly in "\xFF", whereas it should translate to "\xC3\xBF".
    // EXPECT_EQ(ssResult, u8"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348");
    EXPECT_EQ(uiByteCnt, 72u);

    // Invalid escaped
    const char szInvalidEscaped1[] = "\\u040xyz";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped1, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped2[] = "\\U0000000";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped2, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped3[] = "\\uD800";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped3, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped4[] = "\\udfff";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped4, nullptr, ssResult, uiByteCnt, false), CCompileException);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCTextUnicodeUTF16)
{
    std::u16string ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\u0040\\u00ff\\U00000040\\U000000FF\\u0024\\u00a3\\u0939\\u20AC\\ud55c\\U00010348";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, u"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348");
    EXPECT_EQ(uiByteCnt, 72u);

    // Invalid escaped
    const char szInvalidEscaped1[] = "\\u040xyz";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped1, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped2[] = "\\U0000000";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped2, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped3[] = "\\uD800";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped3, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped4[] = "\\udfff";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped4, nullptr, ssResult, uiByteCnt, false), CCompileException);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCTextUnicodeUTF32)
{
    std::u32string ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\u0040\\u00ff\\U00000040\\U000000FF\\u0024\\u00a3\\u0939\\u20AC\\ud55c\\U00010348";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, U"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348");
    EXPECT_EQ(uiByteCnt, 72u);

    // Invalid escaped
    const char szInvalidEscaped1[] = "\\u040xyz";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped1, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped2[] = "\\U0000000";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped2, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped3[] = "\\uD800";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped3, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped4[] = "\\udfff";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped4, nullptr, ssResult, uiByteCnt, false), CCompileException);
}

TEST_F(CText2CTextTranslator, InterpretEscapedCTextUnicodeWide)
{
    std::wstring ssResult;
    uint32_t uiByteCnt = 0;

    // Default escaped
    const char szEscapedCode1[] = "\\u0040\\u00ff\\U00000040\\U000000FF\\u0024\\u00a3\\u0939\\u20AC\\ud55c\\U00010348";
    EXPECT_NO_THROW(InterpretCText(szEscapedCode1, nullptr, ssResult, uiByteCnt, false));
    EXPECT_EQ(ssResult, L"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348");
    EXPECT_EQ(uiByteCnt, 72u);

    // Invalid escaped
    const char szInvalidEscaped1[] = "\\u040xyz";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped1, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped2[] = "\\U0000000";   // Invalid code
    EXPECT_THROW(InterpretCText(szInvalidEscaped2, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped3[] = "\\uD800";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped3, nullptr, ssResult, uiByteCnt, false), CCompileException);
    const char szInvalidEscaped4[] = "\\udfff";   // Reserved code
    EXPECT_THROW(InterpretCText(szInvalidEscaped4, nullptr, ssResult, uiByteCnt, false), CCompileException);
}

TEST_F(CText2CTextTranslator, GenerateCTextASCIIString)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText("Hello", 0xFFFFFFFF, true), "Hello");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText("Hello\nYou there", 0xFFFFFFFF, true), "Hello\\nYou there");

    // Limited text with escape sequence
    EXPECT_EQ(GenerateCText("Hello\nYou there", 9, true), "Hello\\nYou");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF8String)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(u8"Hello"), "Hello");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(u8"Hello\nYou there"), "Hello\\nYou there");

    // Limited text with escape sequence
    EXPECT_EQ(GenerateCText(u8"Hello\nYou there", 9), "Hello\\nYou");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF16String)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(u"Hello"), "Hello");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(u"Hello\nYou there"), "Hello\\nYou there");

    // Limited text with escape sequence
    EXPECT_EQ(GenerateCText(u"Hello\nYou there", 9), "Hello\\nYou");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF32String)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(U"Hello"), "Hello");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(U"Hello\nYou there"), "Hello\\nYou there");

    // Limited text with escape sequence
    EXPECT_EQ(GenerateCText(U"Hello\nYou there", 9), "Hello\\nYou");
}

TEST_F(CText2CTextTranslator, GenerateCTextWideString)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(L"Hello"), "Hello");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(L"Hello\nYou there"), "Hello\\nYou there");

    // Limited text with escape sequence
    EXPECT_EQ(GenerateCText(L"Hello\nYou there", 9), "Hello\\nYou");
}

TEST_F(CText2CTextTranslator, GenerateCTextASCIIChar)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText('H', true), "H");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText('\n', true), "\\n");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF8Char)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(u8'H'), "H");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(u8'\n'), "\\n");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF16Char)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(u'H'), "H");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(u'\n'), "\\n");
}

TEST_F(CText2CTextTranslator, GenerateCTextUTF32Char)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(U'H'), "H");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(U'\n'), "\\n");
}

TEST_F(CText2CTextTranslator, GenerateCTextWideChar)
{
    std::string ssResult;

    // Standard text
    EXPECT_EQ(GenerateCText(L'H'), "H");

    // Text with escape sequence
    EXPECT_EQ(GenerateCText(L'\n'), "\\n");
}

TEST_F(CText2CTextTranslator, GenerateEscapedCTextFromString)
{
    // Default escaped
    EXPECT_EQ(GenerateCText("\'\"?\\\a\b\f\n\r\t\v\265 \xB5\xb5", 0xFFFFFFFF, true),
        "\\\'\\\"?\\\\\\a\\b\\f\\n\\r\\t\\v\\265 \\265\\265");
}

TEST_F(CText2CTextTranslator, GenerateUnicodeEscapedCTextFromUTF8String)
{
    // Default escaped
    EXPECT_EQ(GenerateCText(u8"@\u00ff@\u00ff\u0024\u00a3\u0939\u20AC\ud55c\U00010348"),
        "@\\u00ff@\\u00ff$\\u00a3\\u0939\\u20ac\\ud55c\\U00010348");

    // ATTENTION: The following line will run okay with MSVC, but causes an error with GNU-C. This is because GNU-C is translating
    // the string u8"\xff" wronly in "\xFF" and combines it with u8"\u0024" to "\U003c0fe4".
    // EXPECT_EQ(GenerateCText(u8"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348"),
    //     "@\\u00ff@\\u00ff$\\u00a3\\u0939\\u20ac\\ud55c\\U00010348");
}

TEST_F(CText2CTextTranslator, GenerateUnicodeEscapedCTextFromUTF16String)
{
    // Default escaped
    EXPECT_EQ(GenerateCText(u"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348"),
        "@\\u00ff@\\u00ff$\\u00a3\\u0939\\u20ac\\ud55c\\U00010348");
}

TEST_F(CText2CTextTranslator, GenerateUnicodeEscapedCTextFromUTF32String)
{
    // Default escaped
    EXPECT_EQ(GenerateCText(U"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348"),
        "@\\u00ff@\\u00ff$\\u00a3\\u0939\\u20ac\\ud55c\\U00010348");
}

TEST_F(CText2CTextTranslator, GenerateUnicodeEscapedCTextFromWideString)
{
    // Default escaped
    EXPECT_EQ(GenerateCText(L"@\xff@\xff\u0024\u00a3\u0939\u20AC\ud55c\U00010348"),
        "@\\u00ff@\\u00ff$\\u00a3\\u0939\\u20ac\\ud55c\\U00010348");
}

