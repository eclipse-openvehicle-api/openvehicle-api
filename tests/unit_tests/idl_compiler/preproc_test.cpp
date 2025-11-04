#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/preproc.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/macro.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/exception.h"
#include <fstream>

using CPreprocTest = CParserTest;

TEST_F(CPreprocTest, PreprocessorUnknownDirective)
{
    // Unknown preproc error
    EXPECT_THROW(CParser(nullptr).LexicalCheck(), CCompileException);
    EXPECT_THROW(CParser("#abc").LexicalCheck(), CCompileException);
}

TEST_F(CPreprocTest, PreprocessorWhitespace)
{
    // Parse preprocessor with additional whitespace before

    // Single preprocessing directive
    EXPECT_TRUE(CParser("  #define TEST").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("  # define TEST").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("\t#\tdefine TEST").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("  \n#define TEST").LexicalCheck().GetEnvironment().Defined("TEST"));

    // Multiple preprocessing directives
    EXPECT_TRUE(CParser(R"code(
 # if defined HELLO
 # define NO_TEST
 # else
 # define TEST
 # endif)code").LexicalCheck().GetEnvironment().Defined("TEST"));

    // Tests bug fix: #381248
    // Verbatim block test
    EXPECT_NO_THROW(CParser(R"code(#verbatim_begin
    // ...
    #verbatim_end)code").Parse());
    EXPECT_NO_THROW(CParser(R"code( # verbatim_begin
    // ...
     # not valid text...
     # verbatim_end)code").Parse());
}

TEST_F(CPreprocTest, PreprocessorMacroDefined)
{
    // Parse preprocessor
    EXPECT_TRUE(CParser("#define TEST").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST()").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST(a)").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST(a, b,c)").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST value").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST(a) value a").LexicalCheck().GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(CParser("#define TEST(a, b,c) value a*b*c").LexicalCheck().GetEnvironment().Defined("TEST"));
}

TEST_F(CPreprocTest, PreprocessorDefineRedefine)
{
    // Redefinition
    EXPECT_NO_THROW(CParser("#define TEST(a, b,c) value a*b*c\n#define TEST(a, b,c) value a*b*c").LexicalCheck());
    EXPECT_THROW(CParser("#define TEST(a, b,c) value a*b*c\n#define TEST").LexicalCheck(), CCompileException);
}

TEST_F(CPreprocTest, PreprocessorDefineAndSingleExpand)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define ABBA 2\n"
        "#define ABBABA 3\n"
        "#define AB(c) AB##c\n"
        "#define BA(c) AB ## c\n"
        "#define TEXT(c) # c"
        );
    parser.LexicalCheck();

    // Code will be prepended with an empty tring
    CCodePos codeTest("");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("TEST", codeTest));
    EXPECT_TRUE(static_cast<std::string>(codeTest.GetLocation()).empty());

    // Code will be prepended with the value "2"
    CCodePos codeAbba("");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("ABBA", codeAbba));
    EXPECT_EQ(static_cast<std::string>(codeAbba.GetLocation()), "2");

    // Code will be prepended with "ABBA" and expanded to "2"
    CCodePos codeAbba2("(BA)");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("AB", codeAbba2));
    EXPECT_EQ(static_cast<std::string>(codeAbba2.GetLocation()), "2");

    // Code will be prepended with "ABBA" and expanded to "2"
    CCodePos codeAbba3("(BA)");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("BA", codeAbba3));
    EXPECT_EQ(static_cast<std::string>(codeAbba3.GetLocation()), "2");

    // Code will be prepende with "\"hello\""
    CCodePos codeText("(hello)");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("TEXT", codeText));
    EXPECT_EQ(static_cast<std::string>(codeText.GetLocation()), "\"hello\"");

    // Code will be prepended with "ABBA"
    CCodePos codeAbba4("(B)A");
    EXPECT_TRUE(parser.GetEnvironment().TestAndExpand("AB", codeAbba4));
    CToken tokenAbba4 = codeAbba4.GetLocation();
    ++codeAbba4;    // Skip prepended 'A'
    ++codeAbba4;    // Skip prepended 'B'
    ++codeAbba4;    // Skip prepended 'C'
    ++codeAbba4;    // Skip 'A'
    codeAbba4.UpdateLocation(tokenAbba4);
    EXPECT_EQ(static_cast<std::string>(tokenAbba4), "ABBA");
}

TEST_F(CPreprocTest, PreprocessorDefineAndExpand)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define ABBA 2\n"
        "#define ABBABA 3\n"
        "#define AB(c) AB##c\n"
        "#define BA(c) AB ## c\n"
        "#define DE AB(BA) ## BA\n"
        "#define EF ABBA ## BA\n"
        "#define CIRCULAR CIRCULAR\n"
        "#define MULTI(a, b) a ## b\n"
        "#define MULTI_SPACE(a, b)      a \\\n   \\\r\n\t   b\n"
        "#define MULTI_COMMENT1(a, b) a // ## b\n"
        "#define MULTI_COMMENT2(a, b) a /* ## */ b\n"
        "#define TEXT(c) # c\n"
        "#define Fx abc\n"
        "#define Bx def\n"
        "#define FB(arg) #arg\n"
        "#define FB1(arg) FB(arg)\n"
        "#define DOx(x) x\n"
        "#define CIRCULAR2 DOx(CIRCULAR2)\n"
        "#define CIRCULAR3 CIRCULAR"
        );
    parser.LexicalCheck();

    // Simple parsing function
    auto fnGetLastProcessedToken = [&](CCodePos& rCode) -> CToken
    {
        CToken token;
        while (!rCode.HasEOF())
        {
            // If the current position is not part of the macro expansion, reset the set of macros used in a expansion.
            bool bInMacroExpansion = rCode.CurrentPositionInMacroExpansion();

            // Get a token
            token = parser.GetLexer().GetToken(rCode, nullptr);

            // Check whether the token is an identifier, if so, check for any macro
            if (token.GetType() == ETokenType::token_identifier)
            {
                if (parser.GetEnvironment().TestAndExpand(static_cast<std::string>(token).c_str(), rCode, bInMacroExpansion))
                    continue;   // macro was replaced, get a token again.
            }
        }
        return token;
    };

    // Multiple arguments.
    CCodePos codeMulti("FB1(MULTI(abc, def))");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeMulti)), "\"abcdef\"");

    // Comments are ignored in expansion.
    CCodePos codeComment1("FB1(MULTI_COMMENT1(abc, def))");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeComment1)), "\"abc\"");
    CCodePos codeComment2("FB1(MULTI_COMMENT2(abc, def))");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeComment2)), "\"abc def\"");
    CCodePos codeComment3("FB1(MULTI(abc /*the first*/, /*the second*/def))");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeComment3)), "\"abcdef\"");

    // Macro is formed using another macro and expanded itself to "2".
    CCodePos codeAbba5("AB(B)A");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeAbba5)), "2");

    // Strings from arguments - argument expansion only when not stringificating.
    // Version 1 stringificates the argument without expansion
    // Version 2 expands the argument first and then stringificates
    CCodePos codeFB0("FB(Fx Bx)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeFB0)), "\"Fx Bx\"");
    CCodePos codeFB1("FB1(Fx Bx)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeFB1)), "\"abc def\"");

    // Reduce large spaces to one space
    // Version 1 has space in the arguments, which will be reduced to one space
    // Version 2 has space in the definition, which will be reduced to one space
    CCodePos codeSpace1("FB1(Fx    \t\\\n  \t\t\\\r\n    \tBx)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeSpace1)), "\"abc def\"");
    CCodePos codeSpace2("FB1(MULTI_SPACE(abc, def))");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeSpace2)), "\"abc def\"");

    // Expand results before added to the code
    // Version 1 expands ABBA into 2 and then adds A
    // VERSION 2 glues ABBA to BA and expands ABABA into 3
    CCodePos codeRes1("FB1(DE)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeRes1)), "\"2BA\"");
    CCodePos codeRes2("FB1(EF)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeRes2)), "\"3\"");

    // Circular referencing
    // Version 1 calls itself
    // Version 2 calls another macro calling itsel
    // Version 3 has multiple circular macros in one parameter
    CCodePos codeCircular1("FB1(CIRCULAR)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeCircular1)), "\"CIRCULAR\"");
    CCodePos codeCircular2("FB1(CIRCULAR2)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeCircular2)), "\"CIRCULAR2\"");
    CCodePos codeCircular3("FB1(CIRCULAR3 CIRCULAR CIRCULAR3)");
    EXPECT_EQ(static_cast<std::string>(fnGetLastProcessedToken(codeCircular3)), "\"CIRCULAR CIRCULAR CIRCULAR\"");
}

TEST_F(CPreprocTest, PreprocessorUndef)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define ABBA 2\n"
        "#define ABBABA 3\n"
        "#define AB(c) AB##c\n"
        "#define BA(c) AB ## c\n"
        "#undef TEST\n"
        "#undef ABRACADABRA\n"
        "#undef ABBABA with dummy text... is ignored\n"
        "#undef BA // with comments"
        );
    parser.LexicalCheck();

    EXPECT_FALSE(parser.GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("ABBA"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("ABBABA"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("AB"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("BA"));
}

TEST_F(CPreprocTest, PreprocessorIncludeLocal)
{
    // Create a dummy files
    std::ofstream fstreamNormal("dummy_source_test_preprocessor_include.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamNormal.is_open());
    fstreamNormal << "#define HELLO2";
    fstreamNormal.close();
    std::ofstream fstreamCircular("dummy_source_test_preprocessor_include_circular.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamCircular.is_open());
    fstreamCircular << "#include \"dummy_source_test_preprocessor_include_circular.tmp\"";
    fstreamCircular.close();
    std::filesystem::create_directory("dummy1");
    std::filesystem::path pathDummy1File = "dummy1/dummy1_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy1Subdir(pathDummy1File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy1Subdir.is_open());
    fstreamDummy1Subdir << "#define DUMMY1";
    fstreamDummy1Subdir.close();
    std::filesystem::create_directory("dummy2");
    std::filesystem::path pathDummy2File = "dummy2/dummy2_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy2Subdir(pathDummy2File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy2Subdir.is_open());
    fstreamDummy2Subdir << "#include \"../dummy3/dummy3_subdir_source_test_preprocessor_include.tmp\"";
    fstreamDummy2Subdir.close();
    std::filesystem::create_directory("dummy3");
    std::filesystem::path pathDummy3File = "dummy3/dummy3_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy3Subdir(pathDummy3File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy3Subdir.is_open());
    fstreamDummy3Subdir << "#define DUMMY3\n#include \"dummy4_subdir_source_test_preprocessor_include.tmp\"";
    fstreamDummy3Subdir.close();
    std::filesystem::path pathDummy4File = "dummy3/dummy4_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy4Subdir(pathDummy4File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy4Subdir.is_open());
    fstreamDummy4Subdir << "#define DUMMY4";
    fstreamDummy4Subdir.close();

    // Make include dirs (1st argument is the executable file path).
    std::vector<std::string> rvecArgs = {"idl_compiler_test", "-Idummy1", "-Idummy2"};
    CIdlCompilerEnvironment environment(rvecArgs);

    // Check non-existing
    EXPECT_THROW(CParser("#include \"non_existing_file.tmp\"", environment).LexicalCheck(), CCompileException);

    // Check local
    EXPECT_TRUE(CParser("#include \"dummy_source_test_preprocessor_include.tmp\"", environment).LexicalCheck().
        GetEnvironment().Defined("HELLO2"));

    // Check local with relative path
    EXPECT_TRUE(CParser("#include \"dummy1/dummy1_subdir_source_test_preprocessor_include.tmp\"").LexicalCheck().
        GetEnvironment().Defined("DUMMY1"));

    // Check circular (re-inclusion is prevented)
    EXPECT_NO_THROW(CParser("#include \"dummy_source_test_preprocessor_include_circular.tmp\"", environment).LexicalCheck());

    // Check through include path
    EXPECT_TRUE(CParser("#include \"dummy1_subdir_source_test_preprocessor_include.tmp\"", environment).LexicalCheck().
        GetEnvironment().Defined("DUMMY1"));

    // Check local includes from an included file not in the search path
    EXPECT_TRUE(CParser("#include \"dummy2_subdir_source_test_preprocessor_include.tmp\"", environment).LexicalCheck().
        GetEnvironment().Defined("DUMMY4"));

    try
    {
        std::filesystem::remove("dummy_source_test_preprocessor_include.tmp");
        std::filesystem::remove("dummy_source_test_preprocessor_include_circular.tmp");
        std::filesystem::remove_all("dummy1");
        std::filesystem::remove_all("dummy2");
        std::filesystem::remove_all("dummy3");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CPreprocTest, PreprocessorIncludeLocalAbsolute)
{
    // Create a dummy files
    std::filesystem::path pathCurrent = std::filesystem::current_path();
    std::ofstream fstreamNormal("dummy_source_test_preprocessor_include.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamNormal.is_open());
    fstreamNormal << "#include \"" << (pathCurrent / "dummy" / "dummy_subdir_source_test_preprocessor_include.tmp").generic_u8string() << "\"";
    fstreamNormal.close();
    std::filesystem::create_directory("dummy");
    std::filesystem::path pathDummy1File = "dummy/dummy_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummySubdir(pathDummy1File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummySubdir.is_open());
    fstreamDummySubdir << "#define HELLO";
    fstreamDummySubdir.close();

    // Check access through path (should not be possible)
    EXPECT_THROW(CParser("#include \"dummy_subdir_source_test_preprocessor_include.tmp\"").LexicalCheck(), CCompileException);

    // Check relative path
    EXPECT_TRUE(CParser("#include \"dummy/dummy_subdir_source_test_preprocessor_include.tmp\"").LexicalCheck().
        GetEnvironment().Defined("HELLO"));

    // Check direct absolute path
    std::string ssCode = "#include \"";
    ssCode += (pathCurrent / "dummy" / "dummy_subdir_source_test_preprocessor_include.tmp").generic_u8string();
    ssCode += "\"";
    EXPECT_TRUE(CParser(ssCode.c_str()).LexicalCheck().
        GetEnvironment().Defined("HELLO"));

    // Check indirect absolute path
    EXPECT_TRUE(CParser("#include \"dummy_source_test_preprocessor_include.tmp\"").LexicalCheck().
        GetEnvironment().Defined("HELLO"));

    try
    {
        std::filesystem::remove("dummy_source_test_preprocessor_include.tmp");
        std::filesystem::remove_all("dummy");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CPreprocTest, PreprocessorIncludeGlobal)
{
    // Create a dummy files
    std::ofstream fstreamNormal("dummy_source_test_preprocessor_include.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamNormal.is_open());
    fstreamNormal << "#define HELLO2";
    fstreamNormal.close();
    std::ofstream fstreamCircular("dummy_source_test_preprocessor_include_circular.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamCircular.is_open());
    fstreamCircular << "#include \"dummy_source_test_preprocessor_include_circular.tmp\"";
    fstreamCircular.close();
    std::filesystem::create_directory("dummy1");
    std::filesystem::path pathDummy1File = "dummy1/dummy1_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy1Subdir(pathDummy1File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy1Subdir.is_open());
    fstreamDummy1Subdir << "#define DUMMY1";
    fstreamDummy1Subdir.close();
    std::filesystem::create_directory("dummy2");
    std::filesystem::path pathDummy2File = "dummy2/dummy2_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy2Subdir(pathDummy2File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy2Subdir.is_open());
    fstreamDummy2Subdir << "#include \"../dummy3/dummy3_subdir_source_test_preprocessor_include.tmp\"";
    fstreamDummy2Subdir.close();
    std::filesystem::create_directory("dummy3");
    std::filesystem::path pathDummy3File = "dummy3/dummy3_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy3Subdir(pathDummy3File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy3Subdir.is_open());
    fstreamDummy3Subdir << "#define DUMMY3\n#include \"dummy4_subdir_source_test_preprocessor_include.tmp\"";
    fstreamDummy3Subdir.close();
    std::filesystem::path pathDummy4File = "dummy3/dummy4_subdir_source_test_preprocessor_include.tmp";
    std::ofstream fstreamDummy4Subdir(pathDummy4File.string(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamDummy4Subdir.is_open());
    fstreamDummy4Subdir << "#define DUMMY4";
    fstreamDummy4Subdir.close();

    // Make include dirs (1st argument is the executable file path).
	std::vector<std::string> rvecArgs = {"idl_compiler_test.exe", "-Idummy1", "-Idummy2"};
    CIdlCompilerEnvironment environment(rvecArgs);

    // Check non-existing
    EXPECT_THROW(CParser("#include <non_existing_file.tmp>", environment).LexicalCheck(), CCompileException);

    // Check local
    EXPECT_THROW(CParser("#include <dummy_source_test_preprocessor_include.tmp>", environment).LexicalCheck(), CCompileException);

    // Check local with relative path
    EXPECT_THROW(CParser("#include <dummy1/dummy1_subdir_source_test_preprocessor_include.tmp>").LexicalCheck(), CCompileException);

    // Check through include path
    EXPECT_TRUE(CParser("#include <dummy1_subdir_source_test_preprocessor_include.tmp>", environment).LexicalCheck().
        GetEnvironment().Defined("DUMMY1"));

    // Check local includes from an included file not in the search path
    EXPECT_TRUE(CParser("#include <dummy2_subdir_source_test_preprocessor_include.tmp>", environment).LexicalCheck().
        GetEnvironment().Defined("DUMMY4"));

    try
    {
        std::filesystem::remove("dummy_source_test_preprocessor_include.tmp");
        std::filesystem::remove("dummy_source_test_preprocessor_include_circular.tmp");
        std::filesystem::remove_all("dummy1");
        std::filesystem::remove_all("dummy2");
        std::filesystem::remove_all("dummy3");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CPreprocTest, PreprocessorIf)
{
    // Direct if with integer
    EXPECT_TRUE(CParser(
        "#if 1\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Direct if with boolean
    EXPECT_TRUE(CParser(
        "#if true\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Direct if with character
    EXPECT_TRUE(CParser(
        "#if 'a'\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Direct if with macro
    EXPECT_TRUE(CParser(
        "#define a 10\n"
        "#if a\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Direct if with zerod macro
    EXPECT_TRUE(CParser(
        "#define a 0\n"
        "#if a\n"
        "#define INCORRECT\n"
        "#else\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Direct if with defined macro without value
    EXPECT_THROW(CParser(
        "#define a\n"
        "#if a\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // Direct if with non-existent macro (equals to 0)
    EXPECT_TRUE(CParser(
        "#if a\n"
        "#define INCORRECT\n"
        "#else\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Incomplete if statement
    EXPECT_THROW(CParser(
        "#if\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // Correct if section using macros
    EXPECT_TRUE(CParser(
        "#define a 10\n"
        "#define b(c, d) c*d\n"
        "#define e 10\n"
        "#if a + b(5, 5) + e == 45\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Incorrect if section using macros
    EXPECT_FALSE(CParser(
        "#define a 10\n"
        "#define b(c, d) c*d\n"
        "#define e 10\n"
        "#if a + b(5, 5) + e == 45\n"
        "#define CORRECT\n"
        "#else\n"
        "#define INCORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("INCORRECT"));

    // Parenthesis while calculating
    EXPECT_TRUE(CParser(
        "#define a 10\n"
        "#define b(c, d) c*d\n"
        "#define e 10\n"
        "#if a * (b(5, 5) + e) == 350\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Operators + - / * %
    EXPECT_TRUE(CParser(
        "#if 10 + 11 * 35 % 10 - 2 == 13\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Operators << >> & | ~
    // NOTE: '!=' comes before '&'
    EXPECT_TRUE(CParser(
        "#if (2 << 2 | 7 >> 1 & (~ 120 & 7)) != 11\n"
        "#define INCORRECT\n"
        "#else\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Operators && || < > <= >= == !=
    EXPECT_TRUE(CParser(
        "#if (20 || 10 && 1) < 2\n"
        "#define CORRECT1 1\n"
        "#endif\n"
        "#if 50 + 7 >= 57\n"
        "#define CORRECT2 1\n"
        "#endif\n"
        "#if 50 + 8 > 57\n"
        "#define CORRECT3 1\n"
        "#endif\n"
        "#if 50 + 7 <= 57\n"
        "#define CORRECT4 1\n"
        "#endif\n"
        "#if 10 != TEST\n"
        "#define CORRECT5 1\n"
        "#endif\n"
        "#if CORRECT1 + CORRECT2 + CORRECT3 + CORRECT4 + CORRECT5 == 5\n"
        "#define CORRECT\n"
        "#endif\n"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));

    // Div by zero /
    EXPECT_THROW(CParser(
        "#if 10 / TEST\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // Div by zero %
    EXPECT_THROW(CParser(
        "#if 10 % TEST\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // Incomplete
    EXPECT_THROW(CParser(
        "#define TEST\n"
        "#if 10 % TEST\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // Exclude code
    EXPECT_NO_THROW(CParser(
        "#if 0\n"
        "bla bla\n"
        "#endif"
    ).LexicalCheck());

    // Exclude code incomplete
    EXPECT_THROW(CParser(
        "#if 0\n"
        "bla bla\n"
    ).LexicalCheck(), CCompileException);
}

TEST_F(CPreprocTest, PreprocessorDefinedOperator)
{
    // defined(macro)
    EXPECT_TRUE(CParser(
        "#if !defined(TEST)\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));
    EXPECT_TRUE(CParser(
        "#define TEST\n"
        "#if defined(TEST)\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));
    EXPECT_THROW(CParser(
        "#if defined()\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);

    // defined macro
    EXPECT_TRUE(CParser(
        "#if !defined TEST\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));
    EXPECT_TRUE(CParser(
        "#define TEST\n"
        "#if defined TEST\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck().GetEnvironment().Defined("CORRECT"));
    EXPECT_THROW(CParser(
        "#if defined\n"
        "#define CORRECT\n"
        "#endif"
        ).LexicalCheck(), CCompileException);
}

TEST_F(CPreprocTest, PreprocessorElif)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define AB(c) AB##c\n"
        "#ifdef TOAST\n"
        "#define XY\n"
        "#elif defined(TEST)\n"
        "#define VW\n"
        "#endif"
        );
    parser.LexicalCheck();

    EXPECT_TRUE(parser.GetEnvironment().Defined("TEST"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("XY"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("VW"));
}

TEST_F(CPreprocTest, PreprocessorIfDef)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define AB(c) AB##c\n"
        "#ifdef TEST\n"
        "#define XY\n"
        "#ifdef AB\n"
        "#define VW\n"
        "#endif\n"
        "#endif\n"
        "#ifdef TEST2\n"
        "#define MN\n"
        "#endif"
        );
    parser.LexicalCheck();

    EXPECT_TRUE(parser.GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("AB"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("XY"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("VW"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("MD"));
}

TEST_F(CPreprocTest, PreprocessorIfnDef)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#define AB(c) AB##c\n"
        "#ifndef TEST\n"
        "#define XY\n"
        "#ifndef AB\n"
        "#define VW\n"
        "#endif\n"
        "#endif\n"
        "#ifndef TEST2\n"
        "#define MN\n"
        "#endif"
        );
    parser.LexicalCheck();

    EXPECT_TRUE(parser.GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("AB"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("XY"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("VW"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("MN"));
}

TEST_F(CPreprocTest, PreprocessorElse)
{
    // Parse preprocessor
    CParser parser(
        "#define TEST\n"
        "#ifdef TEST\n"
        "#define XY\n"
        "#else\n"
        "#define VW\n"
        "#endif\n"
        "#ifdef TEST2\n"
        "#define ZA\n"
        "#else\n"
        "#define TR\n"
        "#endif"
        );
    parser.LexicalCheck();

    EXPECT_TRUE(parser.GetEnvironment().Defined("TEST"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("XY"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("VW"));
    EXPECT_FALSE(parser.GetEnvironment().Defined("ZA"));
    EXPECT_TRUE(parser.GetEnvironment().Defined("TR"));
}

TEST_F(CPreprocTest, ProcessVerbatim)
{
    // Parse preprocessor
    CParser parser(
        "#verbatim const int i = 10\n"
        "#verbatim const int j = 10\\n"
        "#verbatim const int k = 10\n"
        "#verbatim #define L\n"
        "#verbatim #include <string>"
    );
    EXPECT_NO_THROW(parser.LexicalCheck());
}

TEST_F(CPreprocTest, ProcessVerbatimBlock)
{
    // Parse preprocessor
    CParser parser(
        "#verbatim_begin ignored text\n"
        "const int j = 10\\n"
        "const int k = 10\n"
        "#define L\n"
        "#include <string>\n"
        "#verbatim_end ignored text"
    );
    EXPECT_NO_THROW(parser.LexicalCheck());

    CParser parser2(
        "#verbatim_end ignored text"
    );
    EXPECT_THROW(parser2.LexicalCheck(), CCompileException);

    CParser parser3(
        "#verbatim_begin ignored text\n"
        "const int j = 10\\n"
        "const int k = 10\n"
        "#define L\n"
        "#include <string>\n"
    );
    EXPECT_THROW(parser3.LexicalCheck(), CCompileException);
}

