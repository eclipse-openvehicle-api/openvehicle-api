#include "includes.h"
#include "lexer_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/lexer.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/exception.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/codepos.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/token.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/tokenlist.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/constvariant.inl"

void CLexerTest::SetUpTestCase()
{
    ASSERT_TRUE(true);
}

void CLexerTest::TearDownTestCase()
{}

void CLexerTest::SetUp()
{}

void CLexerTest::TearDown()
{}

TEST_F(CLexerTest, Instantiate)
{
    // Instantiate a code object without code (should throw exception):
    EXPECT_THROW(CCodePos(nullptr), CCompileException);
    EXPECT_NO_THROW(CCodePos(""));
    EXPECT_TRUE(CCodePos("").IsValid());

    // Instantiate lexer without callback (should throw exception)
    EXPECT_THROW(CLexer(nullptr, true), CCompileException);
    SLexerDummyCallback sCallback;
    EXPECT_NO_THROW(CLexer lexer(&sCallback, true));
}

TEST_F(CLexerTest, ParsingLocation)
{
    const char szCode[] = R"code(identifier1
identifier2
  identifier3
    identifier4
    R"(abc
def)"
    identifier5
      0x1234
)code";

    // Check locations
    CCodePos code(szCode);
    SLexerDummyCallback sCallback;
    CLexer lexer(&sCallback, true);
    CToken sLocIdentifier1 = lexer.GetToken(code, nullptr);
    CToken sLocIdentifier2 = lexer.GetToken(code, nullptr);
    CToken sLocIdentifier3 = lexer.GetToken(code, nullptr);
    CToken sLocIdentifier4 = lexer.GetToken(code, nullptr);
    CToken sLocLiteral1 = lexer.GetToken(code, nullptr);
    CToken sLocIdentifier5 = lexer.GetToken(code, nullptr);
    CToken sLocLiteral2 = lexer.GetToken(code, nullptr);
    EXPECT_EQ(sLocIdentifier1.GetLine(), 1u);
    EXPECT_EQ(sLocIdentifier1.GetCol(), 1u);
    EXPECT_EQ(sLocIdentifier2.GetLine(), 2u);
    EXPECT_EQ(sLocIdentifier2.GetCol(), 1u);
    EXPECT_EQ(sLocIdentifier3.GetLine(), 3u);
    EXPECT_EQ(sLocIdentifier3.GetCol(), 3u);
    EXPECT_EQ(sLocIdentifier4.GetLine(), 4u);
    EXPECT_EQ(sLocIdentifier4.GetCol(), 5u);
    EXPECT_EQ(sLocLiteral1.GetLine(), 5u);
    EXPECT_EQ(sLocLiteral1.GetCol(), 5u);
    EXPECT_EQ(sLocIdentifier5.GetLine(), 7u);
    EXPECT_EQ(sLocIdentifier5.GetCol(), 5u);
    EXPECT_EQ(sLocLiteral2.GetLine(), 8u);
    EXPECT_EQ(sLocLiteral2.GetCol(), 7u);
}

TEST_F(CLexerTest, EmptyString)
{
    // Parse empty string
    CCodePos codeEmptyString("");
    EXPECT_TRUE(codeEmptyString.HasEOF());
    SLexerDummyCallback sCallback;
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeEmptyString, nullptr), CToken());
    EXPECT_TRUE(codeEmptyString.HasEOF());
}

TEST_F(CLexerTest, Whitespace)
{
    // Parse whitespace string
    SLexerStoreCallback sCallback;

    // Parse code with only whitespace.
    CCodePos codeWhitespaceString1(" \t\r\n ");
    EXPECT_FALSE(codeWhitespaceString1.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString1, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t\r\n ");
    EXPECT_TRUE(codeWhitespaceString1.HasEOF());
    EXPECT_EQ(codeWhitespaceString1.GetLine(), 2u);
    EXPECT_EQ(codeWhitespaceString1.GetCol(), 2u);

    // Parse code with only whitespace and limit parsing to one line only.
    CCodePos codeWhitespaceString2(" \t\r\n ");
    EXPECT_FALSE(codeWhitespaceString2.HasEOF());
    CLexer lexerWhitespaceString2(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    EXPECT_EQ(lexerWhitespaceString2.GetToken(codeWhitespaceString2, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t");
    EXPECT_FALSE(codeWhitespaceString2.HasEOF());
    EXPECT_EQ(codeWhitespaceString2.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString2.GetCol(), 5u);
    CCodePos codeWhitespaceString3(" \t\n ");
    EXPECT_FALSE(codeWhitespaceString3.HasEOF());
    CLexer lexerWhitespaceString3(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    EXPECT_EQ(lexerWhitespaceString3.GetToken(codeWhitespaceString3, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t");
    EXPECT_FALSE(codeWhitespaceString3.HasEOF());
    EXPECT_EQ(codeWhitespaceString3.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString3.GetCol(), 5u);

    // Parse code with only whitespace with concatenating lines and limit parsing to one line only.
    CCodePos codeWhitespaceString5(" \t\\\r\n ");
    EXPECT_FALSE(codeWhitespaceString5.HasEOF());
    CLexer lexerWhitespaceString5(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    EXPECT_EQ(lexerWhitespaceString5.GetToken(codeWhitespaceString5, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t\\\r\n ");
    EXPECT_TRUE(codeWhitespaceString5.HasEOF());
    EXPECT_EQ(codeWhitespaceString5.GetLine(), 2u);
    EXPECT_EQ(codeWhitespaceString5.GetCol(), 2u);
    CCodePos codeWhitespaceString4(" \t\\\n ");
    EXPECT_FALSE(codeWhitespaceString4.HasEOF());
    CLexer lexerWhitespaceString4(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    EXPECT_EQ(lexerWhitespaceString4.GetToken(codeWhitespaceString4, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t\\\n ");
    EXPECT_TRUE(codeWhitespaceString4.HasEOF());
    EXPECT_EQ(codeWhitespaceString4.GetLine(), 2u);
    EXPECT_EQ(codeWhitespaceString4.GetCol(), 2u);

    // TODO....
    //EXPECT_EQ(sCallback.ssComment, "//Multi line comment");
    //CCodePos codeCommentStr10("/*Multi line comment\r\nmore comment*/");
    //CToken token = lexerCommentStr9.GetToken(codeCommentStr10, nullptr);
    //EXPECT_EQ(token.GetLine(), 1u);
    //EXPECT_EQ(token.GetEndLine(), 2u);
    //EXPECT_EQ(token.GetCol(), 1);
    //EXPECT_EQ(token.GetEndCol(), 14);

}

TEST_F(CLexerTest, WhitespaceTabAlignment)
{
    // Parse whitespace string
    SLexerStoreCallback sCallback;

    // Parse code with only whitespace.
    CCodePos codeWhitespaceString1("\t");
    EXPECT_FALSE(codeWhitespaceString1.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString1, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "\t");
    EXPECT_TRUE(codeWhitespaceString1.HasEOF());
    EXPECT_EQ(codeWhitespaceString1.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString1.GetCol(), 5u);

    CCodePos codeWhitespaceString2(" \t");
    EXPECT_FALSE(codeWhitespaceString2.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString2, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), " \t");
    EXPECT_TRUE(codeWhitespaceString2.HasEOF());
    EXPECT_EQ(codeWhitespaceString2.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString2.GetCol(), 5u);

    CCodePos codeWhitespaceString3("   \t");
    EXPECT_FALSE(codeWhitespaceString3.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString3, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "   \t");
    EXPECT_TRUE(codeWhitespaceString3.HasEOF());
    EXPECT_EQ(codeWhitespaceString3.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString3.GetCol(), 5u);

    CCodePos codeWhitespaceString4("    \t");
    EXPECT_FALSE(codeWhitespaceString4.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString4, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "    \t");
    EXPECT_TRUE(codeWhitespaceString4.HasEOF());
    EXPECT_EQ(codeWhitespaceString4.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString4.GetCol(), 9u);

    CCodePos codeWhitespaceString5("\t\t");
    EXPECT_FALSE(codeWhitespaceString5.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString5, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "\t\t");
    EXPECT_TRUE(codeWhitespaceString5.HasEOF());
    EXPECT_EQ(codeWhitespaceString5.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString5.GetCol(), 9u);

    CCodePos codeWhitespaceString6("\t\t ");
    EXPECT_FALSE(codeWhitespaceString6.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString6, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "\t\t ");
    EXPECT_TRUE(codeWhitespaceString6.HasEOF());
    EXPECT_EQ(codeWhitespaceString6.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString6.GetCol(), 10u);

    CCodePos codeWhitespaceString7("\t \t ");
    EXPECT_FALSE(codeWhitespaceString7.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString7, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "\t \t ");
    EXPECT_TRUE(codeWhitespaceString7.HasEOF());
    EXPECT_EQ(codeWhitespaceString7.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString7.GetCol(), 10u);

    CCodePos codeWhitespaceString8("\t    \t ");
    EXPECT_FALSE(codeWhitespaceString8.HasEOF());
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeWhitespaceString8, nullptr), CToken());
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenWhitespace), "\t    \t ");
    EXPECT_TRUE(codeWhitespaceString8.HasEOF());
    EXPECT_EQ(codeWhitespaceString8.GetLine(), 1u);
    EXPECT_EQ(codeWhitespaceString8.GetCol(), 14u);

}

TEST_F(CLexerTest, PreprocessorLines)
{
    // Parse preprocessor command
    SLexerStoreCallback sCallback;
    CCodePos code1("#define abc");
    CLexer(&sCallback, true).GetToken(code1, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc");
    CCodePos code2("  #define abc");
    CLexer(&sCallback, true).GetToken(code2, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc");
    CCodePos code3("/* abc preproc */ #define abc");
    CLexer(&sCallback, true).GetToken(code3, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc");
    CCodePos code4("#define abc    // Preproc");
    CLexer(&sCallback, true).GetToken(code4, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc    // Preproc");
    CCodePos code5("#define abc \\\r\ndef\r\nghi");
    CLexer(&sCallback, true).GetToken(code5, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc \\\r\ndef\r\n");
    CCodePos code6("#define abc \\\ndef\nghi");
    CLexer(&sCallback, true).GetToken(code6, nullptr);
    EXPECT_EQ(sCallback.ssPreprocLine, "#define abc \\\ndef\n");
    CCodePos codeNotPreprocessorStr("identifier #define abc");
    CLexer lexer(&sCallback, true);
    lexer.GetToken(codeNotPreprocessorStr, nullptr);
    EXPECT_THROW(lexer.GetToken(codeNotPreprocessorStr, nullptr), CCompileException);
}

TEST_F(CLexerTest, Identifier)
{
    // Get itdentifier
    SLexerDummyCallback sCallback;
    CCodePos code1("identifier");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "identifier");
    CCodePos code2("identifier");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_identifier);
    CCodePos code3("__identifier");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr), "__identifier");
    CCodePos code4("attribute");
    EXPECT_NE(CLexer(&sCallback, true).GetToken(code4, nullptr).GetType(), ETokenType::token_identifier);
    CCodePos code5("AttriBUTE");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "AttriBUTE");
    code5.Reset();
    EXPECT_THROW(CLexer(&sCallback, false).GetToken(code5, nullptr), CCompileException);
    CCodePos code6("_attribute");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "_attribute");
    CCodePos code7("iden_tifier");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "iden_tifier");
    CCodePos code8("__identifier__");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "__identifier__");
    CCodePos code9("  identifier");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "identifier");
    CCodePos code10("  {identifier");
    EXPECT_NE(CLexer(&sCallback, true).GetToken(code10, nullptr).GetType(), ETokenType::token_identifier);
}

TEST_F(CLexerTest, InvalidIdentifier)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("  1identifier");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code1, nullptr), CCompileException);
    CCodePos code2("  identifier\"");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code2, nullptr), CCompileException);
    CCodePos code3("  identifier\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code3, nullptr), CCompileException);
}

TEST_F(CLexerTest, Keyword)
{
    SLexerDummyCallback sCallback;
    // Compare each keyword
    for (const std::string& rssKeyword : g_vecOmgIdlKeywords)
    {
        // Check for keyword.
        CCodePos codeKeyword(rssKeyword.c_str());
        EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeKeyword, nullptr), rssKeyword.c_str());
        codeKeyword.Reset();
        EXPECT_EQ(CLexer(&sCallback, true).GetToken(codeKeyword, nullptr).GetType(),
            ETokenType::token_keyword);

        // Keyword prepended with '_' is an identifier.
        std::string ssIdentifier = std::string("_") + rssKeyword;
        CCodePos codeIdentifier(ssIdentifier.c_str());
        EXPECT_NE(CLexer(&sCallback, true).GetToken(codeIdentifier, nullptr).GetType(),
            ETokenType::token_keyword);

        // Keyword with different case writing is illegal if not case sensitive (is not an identifier).
        std::string ssCaseDiffKeyword = rssKeyword;
        bool bLower = false;
        for (char& rc : ssCaseDiffKeyword)
        {
            rc = bLower ? static_cast<char>(std::tolower(rc)) : static_cast<char>(std::toupper(rc));
            bLower = !bLower;
        }
        CCodePos codeCaseDiffKeyword(ssCaseDiffKeyword.c_str());
        EXPECT_NO_THROW(CLexer(&sCallback, true).GetToken(codeCaseDiffKeyword, nullptr));
        codeCaseDiffKeyword.Reset();
        EXPECT_THROW(CLexer(&sCallback, false).GetToken(codeCaseDiffKeyword, nullptr), CCompileException);
    }
}

TEST_F(CLexerTest, Separator)
{
    // Has separator
    SLexerDummyCallback sCallback;
    CCodePos code1("{");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "{");
    CCodePos code2("{");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_separator);
    CCodePos code3("(");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr), "(");
    CCodePos code4("[");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "[");
    CCodePos code5("]");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "]");
    CCodePos code6(")");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), ")");
    CCodePos code7("}");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "}");
    CCodePos code8(":");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), ":");
    CCodePos code9(";");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), ";");
    CCodePos code10(".");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), ".");
    CCodePos code11("  {");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "{");
    CCodePos code12("  text{");
    EXPECT_NE(CLexer(&sCallback, true).GetToken(code12, nullptr).GetType(), ETokenType::token_separator);
}

TEST_F(CLexerTest, Operator)
{
    // Has operator
    SLexerDummyCallback sCallback;
    CCodePos code1("+");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "+");
    CCodePos code2("+");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_operator);
    CCodePos code3("-");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr), "-");
    CCodePos code4("*");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "*");
    CCodePos code5("/");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "/");
    CCodePos code6("%");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "%");
    CCodePos code7("^");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "^");
    CCodePos code8("~");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "~");
    CCodePos code9(",");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), ",");
    CCodePos code10("|");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "|");
    CCodePos code11("||");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "||");
    CCodePos code12("&");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "&");
    CCodePos code13("&&");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code13, nullptr), "&&");
    CCodePos code14("=");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code14, nullptr), "=");
    CCodePos code15("==");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code15, nullptr), "==");
    CCodePos code16("!");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code16, nullptr), "!");
    CCodePos code17("!=");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code17, nullptr), "!=");
    CCodePos code18("<");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code18, nullptr), "<");
    CCodePos code19("<=");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code19, nullptr), "<=");
    CCodePos code20(">");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code20, nullptr), ">");
    CCodePos code21(">=");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code21, nullptr), ">=");
    CCodePos code22("  +");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code22, nullptr), "+");
    CCodePos code23("  text+");
    EXPECT_NE(CLexer(&sCallback, true).GetToken(code23, nullptr).GetType(), ETokenType::token_operator);

    // This will be detected as preprocessor directive
    CCodePos code24("#");
    EXPECT_NE(CLexer(&sCallback, true).GetToken(code24, nullptr), "#");

    // This will be detected as operator directive
    code24.Reset();
    EXPECT_EQ(CLexer(&sCallback, true, CLexer::ELexingMode::lexing_preproc).GetToken(code24, nullptr), "#");
}

TEST_F(CLexerTest, DecimalIntegerLiteral)
{
    // Has literal
    SLexerDummyCallback sCallback;
    CCodePos code1("1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "1234");
    CCodePos code2("1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_dec_integer);
    CCodePos code4("1234u");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "1234u");
    CCodePos code5("1234U");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "1234U");
    CCodePos code6("1234l");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "1234l");
    CCodePos code7("1234L");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "1234L");
    CCodePos code8("1234ul");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "1234ul");
    CCodePos code9("1234UL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "1234UL");
    CCodePos code10("1234ll");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "1234ll");
    CCodePos code11("1234LL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "1234LL");
    CCodePos code12("1234ull");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "1234ull");
    CCodePos code13("1234ULL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code13, nullptr), "1234ULL");
}

TEST_F(CLexerTest, OctalIntegerLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("01234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "01234");
    CCodePos code2("01234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("01234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_oct_integer);
    CCodePos code4("01234l");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "01234l");
    CCodePos code5("01234L");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "01234L");
    CCodePos code6("01234ll");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "01234ll");
    CCodePos code7("01234LL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "01234LL");
    CCodePos code8("01234u");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "01234u");
    CCodePos code9("01234U");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "01234U");
    CCodePos code10("01234ul");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "01234ul");
    CCodePos code11("01234UL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "01234UL");
    CCodePos code12("01234ull");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "01234ull");
    CCodePos code13("01234ULL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code13, nullptr), "01234ULL");
}

TEST_F(CLexerTest, HexIntegerLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("0xaBcDu");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "0xaBcDu");
    CCodePos code2("0xaBcDu");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("0xaBcDu");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_hex_integer);
    CCodePos code4("0xaBcDU");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "0xaBcDU");
    CCodePos code5("0xaBcDl");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "0xaBcDl");
    CCodePos code6("0xaBcDL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "0xaBcDL");
    CCodePos code7("0xaBcDul");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "0xaBcDul");
    CCodePos code8("0xaBcDUL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "0xaBcDUL");
    CCodePos code9("0xaBcDll");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "0xaBcDll");
    CCodePos code10("0xaBcDLL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "0xaBcDLL");
    CCodePos code11("0xaBcDull");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "0xaBcDull");
    CCodePos code12("0xaBcDULL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "0xaBcDULL");
}

TEST_F(CLexerTest, BinaryIntegerLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("0b1010");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "0b1010");
    CCodePos code2("0b1010");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("0b1010");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_bin_integer);
    CCodePos code4("0b0010l");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "0b0010l");
    CCodePos code5("0b1010L");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "0b1010L");
    CCodePos code6("0b0010ll");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "0b0010ll");
    CCodePos code7("0b1010LL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "0b1010LL");
    CCodePos code8("0b1010u");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "0b1010u");
    CCodePos code9("0b1010U");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "0b1010U");
    CCodePos code10("0b1010ul");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "0b1010ul");
    CCodePos code11("0b1010UL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "0b1010UL");
    CCodePos code12("0b1010ull");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "0b1010ull");
    CCodePos code13("0b1010ULL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code13, nullptr), "0b1010ULL");
}

TEST_F(CLexerTest, DecimalFloatingPointLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("1234.");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "1234.");
    CCodePos code2("1234.");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("1234.");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_dec_floating_point);
    CCodePos code4("1234.5");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "1234.5");
    CCodePos code5(".5");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), ".5");
    CCodePos code6("1234e5");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "1234e5");
    CCodePos code7("1234e-5f");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "1234e-5f");
    CCodePos code8("1234e+5F");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "1234e+5F");
    CCodePos code9("1234e-5L");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "1234e-5L");
    CCodePos code10("1234e+5l");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "1234e+5l");
}

TEST_F(CLexerTest, HexadecimalFloatingPointLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("0x1ffp10");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "0x1ffp10");
    CCodePos code2("0x1ffp10");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("0x1ffp10");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_hex_floating_point);
    CCodePos code4("0x0p-1");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "0x0p-1");
    CCodePos code5("0x1.p0");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "0x1.p0");
    CCodePos code6("0xf.p-1");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "0xf.p-1");
    CCodePos code7("0x0.123p-1");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "0x0.123p-1");
    CCodePos code8("0xa.bp10l");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "0xa.bp10l");
}

TEST_F(CLexerTest, FixedPointLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("1234.d");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "1234.d");
    CCodePos code2("1234.d");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("1234.d");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_fixed_point);
    CCodePos code4("1234.5D");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "1234.5D");
    CCodePos code5(".5D");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), ".5D");
}

TEST_F(CLexerTest, ASCIIStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("\"...\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "\"...\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_string);
}

TEST_F(CLexerTest, ASCIIRawStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("R\"abc(...\")abc..\")abc\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "R\"abc(...\")abc..\")abc\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_raw_string);
}

TEST_F(CLexerTest, UTF8StringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("u8\"...\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "u8\"...\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_string);
}

TEST_F(CLexerTest, UTF8RawStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("u8R\"abc(...\")abc..\")abc\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "u8R\"abc(...\")abc..\")abc\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_raw_string);
}

TEST_F(CLexerTest, UTF16StringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("u\"...\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "u\"...\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_string);
}

TEST_F(CLexerTest, UTF16RawStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("uR\"abc(...\")abc..\")abc\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "uR\"abc(...\")abc..\")abc\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_raw_string);
}

TEST_F(CLexerTest, UTF32StringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("U\"...\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "U\"...\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_string);
}

TEST_F(CLexerTest, UTF32RawStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("UR\"abc(...\")abc..\")abc\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "UR\"abc(...\")abc..\")abc\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_raw_string);
}

TEST_F(CLexerTest, WideStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("L\"...\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "L\"...\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_string);
}

TEST_F(CLexerTest, WideRawStringLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("LR\"abc(...\")abc..\")abc\"");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "LR\"abc(...\")abc..\")abc\"");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_raw_string);
}

TEST_F(CLexerTest, ASCIICharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("\'x\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "\'x\'");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character);
}

TEST_F(CLexerTest, UTF16CharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("u\'x\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "u\'x\'");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character);
}

TEST_F(CLexerTest, UTF32CharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("U\'x\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "U\'x\'");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character);
}

TEST_F(CLexerTest, WideCharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("L\'x\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr), "L\'x\'");
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetType(), ETokenType::token_literal);
    code.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character);
}

TEST_F(CLexerTest, InvalidCharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code("u8\'x\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code, nullptr), CCompileException);
}

TEST_F(CLexerTest, ASCIICharacterSequenceLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("\'ab\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "\'ab\'");
    CCodePos code2("\'ab\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("\'ab\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character_sequence);
    CCodePos code4("\'abcd\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "\'abcd\'");
    CCodePos code5("\'abc\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code5, nullptr), CCompileException);
    CCodePos code6("\'abcdefgh\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "\'abcdefgh\'");
}

TEST_F(CLexerTest, WideCharacterSequenceLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("L\'ab\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "L\'ab\'");
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetType(), ETokenType::token_literal);
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character_sequence);

    // Dependable on the size of wchar_t two or four byte wide character sequences are possible
    CCodePos code2("L\'abcd\'");
    if constexpr (sizeof(wchar_t) == 4)   // size if four bytes
        EXPECT_THROW(CLexer(&sCallback, true).GetToken(code2, nullptr), CCompileException);
    else    // size if two bytes
        EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr), "L\'abcd\'");
    CCodePos code3("L\'abc\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code3, nullptr), CCompileException);
    CCodePos code4("L\'abcdefgh\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code4, nullptr), CCompileException);
}

TEST_F(CLexerTest, InvalidCharacterSequences)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("u\'ab\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code1, nullptr), CCompileException);
    CCodePos code2("u8\'ab\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code2, nullptr), CCompileException);
    CCodePos code3("U\'ab\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code3, nullptr), CCompileException);
}

TEST_F(CLexerTest, EscapeCharacterLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("\'\\\'\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "\'\\\'\'");
    CCodePos code2("\'\\'\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr).GetType(), ETokenType::token_literal);
    CCodePos code3("\'\\'\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_character);
    CCodePos code4("\'\\\"\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "\'\\\"\'");
    CCodePos code5("\'\\?\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "\'\\?\'");
    CCodePos code6("\'\\\\\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code6, nullptr), "\'\\\\\'");
    CCodePos code7("\'\\b\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code7, nullptr), "\'\\b\'");
    CCodePos code8("\'\\f\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code8, nullptr), "\'\\f\'");
    CCodePos code9("\'\\n\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code9, nullptr), "\'\\n\'");
    CCodePos code10("\'\\r\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code10, nullptr), "\'\\r\'");
    CCodePos code11("\'\\t\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code11, nullptr), "\'\\t\'");
    CCodePos code12("\'\\v\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code12, nullptr), "\'\\v\'");
    CCodePos code13("\'\\265\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code13, nullptr), "\'\\265\'");            // µ character
    CCodePos code14("\'\\xb5\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code14, nullptr), "\'\\xb5\'");            // µ character
    CCodePos code15("\'\\xB5\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code15, nullptr), "\'\\xB5\'");            // µ character
    CCodePos code16("\'\\u00B5\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code16, nullptr), "\'\\u00B5\'");          // µ character
    CCodePos code17("\'\\U000000b5\'");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code17, nullptr), "\'\\U000000b5\'");      // µ character
    CCodePos code18("\'\\u0B5\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code18, nullptr), CCompileException);
    CCodePos code19("\'\\U00b5\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code19, nullptr), CCompileException);
    CCodePos code20("\'\\q\'");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code20, nullptr), CCompileException);
}

TEST_F(CLexerTest, BooleanLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("true");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "true");
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetType(), ETokenType::token_literal);
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_boolean);
    CCodePos code2("TRUE");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr), "TRUE");
    CCodePos code3("false");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr), "false");
    CCodePos code4("FALSE");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "FALSE");
}

TEST_F(CLexerTest, NullptrLiteral)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("nullptr");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "nullptr");
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetType(), ETokenType::token_literal);
    code1.Reset();
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr).GetLiteralType(), ETokenLiteralType::token_literal_nullptr);
    CCodePos code2("NULL");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr), "NULL");
}

TEST_F(CLexerTest, LiteralFollowingWhitespace)
{
    SLexerDummyCallback sCallback;
    CCodePos code1("  1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code1, nullptr), "1234");
    CCodePos code2("  +1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code2, nullptr), "+");
    CCodePos code3("  -1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code3, nullptr), "-");
    CCodePos code4("  {-}1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code4, nullptr), "{");
    CCodePos code5("  abc1234");
    EXPECT_EQ(CLexer(&sCallback, true).GetToken(code5, nullptr), "abc1234");
    CCodePos code6("  1234a");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code6, nullptr), CCompileException);
    CCodePos code7("  012348");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code7, nullptr), CCompileException);
    CCodePos code8("  0xabcdg1234");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code8, nullptr), CCompileException);
    CCodePos code9("  0b111112");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code9, nullptr), CCompileException);
    CCodePos code10("  \"...");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code10, nullptr), CCompileException);
    CCodePos code11("  u8\"...");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code11, nullptr), CCompileException);
    CCodePos code12("  u\"...");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code12, nullptr), CCompileException);
    CCodePos code13("  U\"...");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code13, nullptr), CCompileException);
    CCodePos code14("  L\"...");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code14, nullptr), CCompileException);
    CCodePos code15("  uR\"abc(...)abcd\"");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code15, nullptr), CCompileException);
    CCodePos code16("  uR\"abc(...)ab\"");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code16, nullptr), CCompileException);
    CCodePos code17("  uR\"abc(...)abc");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(code17, nullptr), CCompileException);
}

TEST_F(CLexerTest, Comments)
{
    // Parse comment string
    SLexerStoreCallback sCallback;
    CCodePos codeCommentStr1(" /*comment*/");
    CLexer(&sCallback, true).GetToken(codeCommentStr1, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "/*comment*/");
    CCodePos codeCommentStr2(" //comment\r\n");
    CLexer(&sCallback, true).GetToken(codeCommentStr2, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "//comment\r\n");
    CCodePos codeCommentStr3(" /*comment*//*next comment*/");
    CLexer(&sCallback, true).GetToken(codeCommentStr3, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "/*next comment*/");
    CCodePos codeCommentStr4(" //comment\r\n//next comment");
    CLexer(&sCallback, true).GetToken(codeCommentStr4, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "//next comment");
    CCodePos codeCommentStr5(" //comment\n//next comment");
    CLexer(&sCallback, true).GetToken(codeCommentStr5, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "//next comment");
    CCodePos codeCommentStr6(" /*comment\r\nmore comment*/");
    CLexer(&sCallback, true).GetToken(codeCommentStr6, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "/*comment\r\nmore comment*/");
    CCodePos codeCommentStr7("/*unfinished comment");
    EXPECT_THROW(CLexer(&sCallback, true).GetToken(codeCommentStr7, nullptr), CCompileException);
    CCodePos codeCommentStr8("/*Multi line comment\nmore comment*/");
    CLexer lexerCommentStr8(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    EXPECT_THROW(lexerCommentStr8.GetToken(codeCommentStr8, nullptr), CCompileException);
    CCodePos codeCommentStr9("//Multi line comment\r\n//more comment");
    CLexer lexerCommentStr9(&sCallback, true);
    lexerCommentStr9.GetToken(codeCommentStr9, nullptr);
    EXPECT_EQ(static_cast<std::string>(sCallback.tokenComment), "//more comment");

    CLexer lexerCommentStr10(&sCallback, true);
    CCodePos codeCommentStr10("//Single line comment\r\n    struct");
    lexerCommentStr10.GetToken(codeCommentStr10, nullptr);
    EXPECT_EQ(sCallback.tokenComment.GetLine(), 1u);
    EXPECT_EQ(sCallback.tokenComment.GetEndLine(), 1u);
    EXPECT_EQ(sCallback.tokenComment.GetCol(), 1u);
    EXPECT_EQ(sCallback.tokenComment.GetEndCol(), 23u);

    CLexer lexerCommentStr11(&sCallback, true);
    CCodePos codeCommentStr11("/*Multi line comment\r\nmore comment*/");
    lexerCommentStr11.GetToken(codeCommentStr11, nullptr);
    EXPECT_EQ(sCallback.tokenComment.GetLine(), 1u);
    EXPECT_EQ(sCallback.tokenComment.GetEndLine(), 2u);
    EXPECT_EQ(sCallback.tokenComment.GetCol(), 1u);
    EXPECT_EQ(sCallback.tokenComment.GetEndCol(), 14u);
}

TEST_F(CLexerTest, ComplexParsing)
{
    const char szCode[] = R"code(identifier1
identifier2
  /*comment*/ identifier3
    identifier4 // comment
    /*comment*/ /*more comment*/R"(abc
def)"
    identifier5
      0x1234
)code";

    // Check locations
    CCodePos codeStr(szCode);
    SLexerDummyCallback sCallback;
    CLexer lexer(&sCallback, true);
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "identifier1");
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "identifier2");
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "identifier3");
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "identifier4");
    CToken sLiteralLoc = lexer.GetToken(codeStr, nullptr);
    EXPECT_TRUE(sLiteralLoc == "R\"(abc\r\ndef)\"" ||       // Windows
        sLiteralLoc == "R\"(abc\ndef)\"");                  // Linux
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "identifier5");
    EXPECT_EQ(lexer.GetToken(codeStr, nullptr), "0x1234");
}

TEST_F(CLexerTest, ParsingCustom)
{
    // Parse string
    SLexerDummyCallback sCallback;
    CLexer lexer(&sCallback, true);
    CCodePos code1("This is a text\"");
    EXPECT_EQ(lexer.GetCustom(code1, '\"'), "This is a text");
    CCodePos code2("This is a text\"abc");
    EXPECT_EQ(lexer.GetCustom(code2, '\"'), "This is a text");
    CCodePos code3("This is a text");
    EXPECT_EQ(lexer.GetCustom(code3, '\"'), "This is a text");
    CCodePos code4("\"This is a text\"abc");
    EXPECT_EQ(lexer.GetCustom(code4, '\"'), "");
    CCodePos code5("#include <abc.h> // ABC");
    EXPECT_EQ(lexer.GetCustom(code5, '>'), "#include <abc.h");
    CCodePos code6("This is a text\r\nabc\"def");
    EXPECT_EQ(lexer.GetCustom(code6, '\"'), "This is a text\r\nabc");
    CCodePos code7("This is a text\nabc\"def");
    EXPECT_EQ(lexer.GetCustom(code7, '\"'), "This is a text\nabc");
    CLexer lexerLineOnly(&sCallback, true, CLexer::ELexingMode::lexing_preproc);
    CCodePos code8("This is a text\r\nabc\"def");
    EXPECT_EQ(lexerLineOnly.GetCustom(code8, '\"'), "This is a text");
    CCodePos code9("This is a text\nabc\"def");
    EXPECT_EQ(lexerLineOnly.GetCustom(code9, '\"'), "This is a text");
    CCodePos code10("This is a text\\\r\nabc\"def");
    EXPECT_EQ(lexerLineOnly.GetCustom(code10, '\"'), "This is a text\\\r\nabc");
    CCodePos code11("This is a text\\\nabc\"def");
    EXPECT_EQ(lexerLineOnly.GetCustom(code11, '\"'), "This is a text\\\nabc");
}

    const char szCode[] = R"(
module BankDemo
{
    typedef float CashAmount;  // Type for representing cash
    typedef string AccountId;  // Type for representing account ids
    //...
    interface Account
    {
    readonly attribute AccountId  account_id;
    readonly attribute CashAmount balance;

    void
    withdraw(in CashAmount amount)
    raises (InsufficientFunds);

    void
    deposit(in CashAmount amount);
    }
}
)";
