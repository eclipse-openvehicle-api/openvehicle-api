#include "includes.h"
#include "lexer_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/core_idl_backup.h"
#include "../../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/environment.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/logger.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/exception.h"

using CCommandLineTest = CLexerTest;

TEST_F(CCommandLineTest, UnknownOption)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLOption1 = { "idl_compiler_test", "-xyz"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLOption1), CCompileException);
#ifdef _WIN32
    std::vector<std::string> vecCLOption2 = { "idl_compiler_test", "/xyz"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLOption2), CCompileException);
#endif
    std::vector<std::string> vecCLOption3 = { "idl_compiler_test", "--xyz"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLOption3), CCompileException);
    std::vector<std::string> vecCLOption4 = { "idl_compiler_test", "--help", "-xyz"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLOption4), CCompileException);
#ifdef _WIN32
    std::vector<std::string> vecCLOption5 = { "idl_compiler_test", "--help", "/xyz"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLOption5), CCompileException);
#endif
}

TEST_F(CCommandLineTest, CommandLineHelp)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLHelp1 = { "idl_compiler_test", "--help" };
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLHelp1)).Help());
#ifdef _WIN32
    std::vector<std::string> vecCLHelp2 = { "idl_compiler_test", "/?" };
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLHelp2)).Help());
#endif
    std::vector<std::string> vecCLHelp3 = { "idl_compiler_test", "-?" };
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLHelp3)).Help());
    std::vector<std::string> vecCLHelp4 = { "idl_compiler_test", "--version", "--help" };
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLHelp4)).Help());
}

TEST_F(CCommandLineTest, CommandLineVersion)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLVersion1 = { "idl_compiler_test", "--version"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLVersion1)).Version());
    std::vector<std::string> vecCLVersion2 = { "idl_compiler_test", "--help", "--version"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLVersion2)).Version());
}

TEST_F(CCommandLineTest, CommandLineIncludeDirs)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLIncludeDirs1 = { "idl_compiler_test", "-IHello", "-I../Hoho"};
    std::vector<std::filesystem::path> vecIncludeDirs1 =
        CIdlCompilerEnvironment(vecCLIncludeDirs1).GetIncludeDirs();
    ASSERT_EQ(vecIncludeDirs1.size(), 2);
    EXPECT_EQ(vecIncludeDirs1[0], "Hello");
    EXPECT_EQ(vecIncludeDirs1[1], "../Hoho");
#ifdef _WIN32
    std::vector<std::string> vecCLIncludeDirs2 = { "idl_compiler_test", "/IHello", "/I..\\Hoho"};
    std::vector<std::filesystem::path> vecIncludeDirs2 =
        CIdlCompilerEnvironment(vecCLIncludeDirs2).GetIncludeDirs();
    ASSERT_EQ(vecIncludeDirs2.size(), 2);
    EXPECT_EQ(vecIncludeDirs2[0], "Hello");
    EXPECT_EQ(vecIncludeDirs2[1], "..\\Hoho");
#endif
}

TEST_F(CCommandLineTest, CommandLineOutputDir)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLOutputDir1 = { "idl_compiler_test", "-OHello" };
    std::filesystem::path pathOutputDir1 = CIdlCompilerEnvironment(vecCLOutputDir1).GetOutputDir();
    EXPECT_EQ(pathOutputDir1, "Hello");
#ifdef _WIN32
    std::vector<std::string> vecCLOutputDir2 = { "idl_compiler_test", "/OHello" };
    std::filesystem::path pathOutputDir2 = CIdlCompilerEnvironment(vecCLOutputDir2).GetOutputDir();
    EXPECT_EQ(pathOutputDir2, "Hello");
#endif
    std::vector<std::string> vecCLOutputDir3 = { "idl_compiler_test", "-OHello", "-OHello2" };
    EXPECT_THROW((env = CIdlCompilerEnvironment(vecCLOutputDir3)).GetOutputDir(), CCompileException);
}

TEST_F(CCommandLineTest, CommandLineDefines)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecCLDefine1 = { "idl_compiler_test", "-DTEST"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLDefine1)).Defined("TEST"));
#ifdef _WIN32
    std::vector<std::string> vecCLDefine2 = { "idl_compiler_test", "/DTEST"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLDefine2)).Defined("TEST"));
#endif
    std::vector<std::string> vecCLDefine3 = { "idl_compiler_test", "-DTEST", "-DTEST"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLDefine3)).Defined("TEST"));
    std::vector<std::string> vecCLDefine4 = { "idl_compiler_test", "-DTEST=2"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLDefine4)).Defined("TEST"));
    std::vector<std::string> vecCLDefine5 = { "idl_compiler_test", "-DTEST(a,b,c)=a+b+c"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecCLDefine5)).Defined("TEST"));
    std::vector<std::string> vecCLDefine6 = { "idl_compiler_test", "-DTEST", "-DTEST=2"};
    EXPECT_THROW(env = CIdlCompilerEnvironment(vecCLDefine6), CCompileException);
}

TEST_F(CCommandLineTest, CommandLineResolveConst)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecResolveConst = { "idl_compiler_test", "--resolve_const"};
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecResolveConst)).ResolveConst());
}

TEST_F(CCommandLineTest, CommandLineNoPS)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecNoPS = { "idl_compiler_test", "--no_ps" };
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vecNoPS)).NoProxyStub());
}

TEST_F(CCommandLineTest, CommandLineProxyStubLibName)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    std::vector<std::string> vecProxyStub = { "idl_compiler_test" };
    auto ssLibName = CIdlCompilerEnvironment(vecProxyStub).GetProxStubCMakeTarget();
    EXPECT_EQ(ssLibName, "proxystub");
    vecProxyStub.push_back("--ps_lib_nameExampleString");
    ssLibName = CIdlCompilerEnvironment(vecProxyStub).GetProxStubCMakeTarget();
    EXPECT_EQ(ssLibName, "ExampleString");
}

TEST_F(CCommandLineTest, CommandLineExtensions)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
    EXPECT_TRUE(env.InterfaceTypeExtension());
    std::vector<std::string> vec = { "idl_compiler_test", "--interface_type-" };
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).InterfaceTypeExtension());
    vec[1] = "--interface_type+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).InterfaceTypeExtension());

    EXPECT_TRUE(env.ExceptionTypeExtension());
    vec[1] = "--exception_type-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).ExceptionTypeExtension());
    vec[1] = "--exception_type+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).ExceptionTypeExtension());

    EXPECT_TRUE(env.PointerTypeExtension());
    vec[1] = "--pointer_type-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).PointerTypeExtension());
    vec[1] = "--pointer_type+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).PointerTypeExtension());

    EXPECT_TRUE(env.UnicodeExtension());
    vec[1] = "--unicode_char-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).UnicodeExtension());
    vec[1] = "--unicode_char+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).UnicodeExtension());

    EXPECT_TRUE(env.CaseSensitiveTypeExtension());
    vec[1] = "--case_sensitive-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).CaseSensitiveTypeExtension());
    vec[1] = "--case_sensitive+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).CaseSensitiveTypeExtension());

    EXPECT_TRUE(env.ContextDependentNamesExtension());
    vec[1] = "--context_names-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).ContextDependentNamesExtension());
    vec[1] = "--context_names+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).ContextDependentNamesExtension());

    EXPECT_TRUE(env.MultiDimArrayExtension());
    vec[1] = "--multi_dimensional_array-";
    EXPECT_FALSE((env = CIdlCompilerEnvironment(vec)).MultiDimArrayExtension());
    vec[1] = "--multi_dimensional_array+";
    EXPECT_TRUE((env = CIdlCompilerEnvironment(vec)).MultiDimArrayExtension());
}

TEST_F(CCommandLineTest, CommandLineStrict)
{
    // Note: 1st argument is the executable file path.
    CIdlCompilerEnvironment env;
#ifdef _WIN32
    std::vector<std::string> vec = { "idl_compiler_test", "--strict" };
#else
    std::vector<std::string> vec = { "idl_compiler_test", "--strict" };
#endif
    EXPECT_TRUE(env.InterfaceTypeExtension());
    EXPECT_TRUE(env.ExceptionTypeExtension());
    EXPECT_TRUE(env.UnicodeExtension());
    EXPECT_TRUE(env.CaseSensitiveTypeExtension());
    EXPECT_TRUE(env.ContextDependentNamesExtension());
    EXPECT_TRUE(env.MultiDimArrayExtension());
    env = CIdlCompilerEnvironment(vec);
    EXPECT_FALSE(env.InterfaceTypeExtension());
    EXPECT_FALSE(env.ExceptionTypeExtension());
    EXPECT_FALSE(env.UnicodeExtension());
    EXPECT_FALSE(env.CaseSensitiveTypeExtension());
    EXPECT_FALSE(env.ContextDependentNamesExtension());
    EXPECT_FALSE(env.MultiDimArrayExtension());
}

TEST_F(CCommandLineTest, CommandLineDefaultArg)
{
    // Note: 1st argument is the executable file path.
    std::vector<std::string> vec = { "idl_compiler_test", "test1.idl", "test2.idl"};
    CIdlCompilerEnvironment env(vec);
    std::filesystem::path pathFile = env.GetNextFile();
    EXPECT_EQ(pathFile, "test1.idl");
    pathFile = env.GetNextFile();
    EXPECT_EQ(pathFile, "test2.idl");
    pathFile = env.GetNextFile();
    EXPECT_TRUE(pathFile.empty());
}

