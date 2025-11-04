#include "../../include/gtest_custom.h"
#include "commandline_parser_test.h"
#include "../../../global/cmdlnparser/cmdlnparser.cpp"

void CCommandLineParserTest::SetUpTestCase()
{}

void CCommandLineParserTest::TearDownTestCase()
{}

void CCommandLineParserTest::SetUp()
{}

void CCommandLineParserTest::TearDown()
{}

TEST_F(CCommandLineParserTest, Instantiation)
{
    const char* rgszCommandLine[] = {"this_exe.app"};
    CCommandLine cl;
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
}

TEST_F(CCommandLineParserTest, NoOptionAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app"};
    CCommandLine cl;
    size_t nVar1 = 0; cl.DefineOption("nvar1", nVar1, "first variable");
    size_t nVar2 = 0; cl.DefineOption("nvar2", nVar2, "second variable");
    bool bFlag = false; cl.DefineOption("flag", bFlag, "flag variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(nVar1, 0);
    EXPECT_EQ(nVar2, 0);
    EXPECT_FALSE(bFlag);
}

TEST_F(CCommandLineParserTest, NoSubOptionAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app"};
    CCommandLine cl;
    size_t nVar1 = 0; cl.DefineSubOption("nvar1", nVar1, "first variable");
    size_t nVar2 = 0; cl.DefineSubOption("nvar2", nVar2, "second variable");
    bool bFlag = false; cl.DefineSubOption("flag", bFlag, "flag variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(nVar1, 0);
    EXPECT_EQ(nVar2, 0);
    EXPECT_FALSE(bFlag);
}

TEST_F(CCommandLineParserTest, OptionAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-nvar1=1", "-nvar2:2", "-flag"};
    CCommandLine cl;
    size_t nVar1 = 0; cl.DefineOption("nvar1", nVar1, "first variable");
    size_t nVar2 = 0; cl.DefineOption("nvar2", nVar2, "second variable");
    bool bFlag = false; cl.DefineOption("flag", bFlag, "flag variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(nVar1, 1);
    EXPECT_EQ(nVar2, 2);
    EXPECT_TRUE(bFlag);
}

TEST_F(CCommandLineParserTest, SubOptionAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--nvar1=1", "--nvar2:2", "--flag"};
    CCommandLine cl;
    size_t nVar1 = 0; cl.DefineSubOption("nvar1", nVar1, "first variable");
    size_t nVar2 = 0; cl.DefineSubOption("nvar2", nVar2, "second variable");
    bool bFlag = false; cl.DefineSubOption("flag", bFlag, "flag variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(nVar1, 1);
    EXPECT_EQ(nVar2, 2);
    EXPECT_TRUE(bFlag);
}

TEST_F(CCommandLineParserTest, OptionEmptyAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-ssvar1=", "-ssvar2=\"\"", "-nvar3="};
    CCommandLine cl;
    std::string ssVar1; cl.DefineOption("ssvar1", ssVar1, "first variable");
    std::string ssVar2; cl.DefineOption("ssvar2", ssVar2, "second variable");
    size_t nVar3 = 0; cl.DefineOption("nvar3", nVar3, "third variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_TRUE(ssVar1.empty());
    EXPECT_TRUE(ssVar2.empty());
    EXPECT_EQ(nVar3, 0u);
}

TEST_F(CCommandLineParserTest, SubOptionEmptyAssignment)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--ssvar1=", "--ssvar2=\"\"", "--nvar3="};
    CCommandLine cl;
    std::string ssVar1; cl.DefineSubOption("ssvar1", ssVar1, "first variable");
    std::string ssVar2; cl.DefineSubOption("ssvar2", ssVar2, "second variable");
    size_t nVar3 = 0; cl.DefineSubOption("nvar3", nVar3, "third variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_TRUE(ssVar1.empty());
    EXPECT_TRUE(ssVar2.empty());
    EXPECT_EQ(nVar3, 0u);
}

TEST_F(CCommandLineParserTest, InvalidArgOption)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-invalid"};
    CCommandLine cl;
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, InvalidArgSubOption)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--invalid"};
    CCommandLine cl;
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, InvalidDefaultArg)
{
    const char* rgszCommandLine[] = {"this_exe.app", "invalid"};
    CCommandLine cl;
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, InvalidArgTypeOption)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-val=abc"};
    CCommandLine cl;
    size_t nVar = 0; cl.DefineOption("val", nVar, "variable");
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, InvalidArgTypeSubOption)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--val=abc"};
    CCommandLine cl;
    size_t nVar = 0; cl.DefineSubOption("val", nVar, "variable");
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, InvalidDefaultArgType)
{
    const char* rgszCommandLine[] = {"this_exe.app", "abc"};
    CCommandLine cl;
    size_t nVar = 0; cl.DefineDefaultArgument(nVar, "variable");
    EXPECT_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine), SArgumentParseException);
}

TEST_F(CCommandLineParserTest, MixedArguments)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-ssvar1=abc", "--ssvar2=def", "ghi"};
    CCommandLine cl;
    std::string ssVar1; cl.DefineOption("ssvar1", ssVar1, "first variable");
    std::string ssVar2; cl.DefineSubOption("ssvar2", ssVar2, "second variable");
    std::string ssVar3; cl.DefineDefaultArgument(ssVar3, "third variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(ssVar1, "abc");
    EXPECT_EQ(ssVar2, "def");
    EXPECT_EQ(ssVar3, "ghi");
}

TEST_F(CCommandLineParserTest, MixedOptionArgumentsReuse)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-ssvar=abc", "--ssvar=def"};
    CCommandLine cl;
    std::string ssVar1; cl.DefineOption("ssvar", ssVar1, "first variable");
    std::string ssVar2; cl.DefineSubOption("ssvar", ssVar2, "second variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(ssVar1, "abc");
    EXPECT_EQ(ssVar2, "def");
}

TEST_F(CCommandLineParserTest, MixedOptionArgumentsNoAssignmentChar)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-ssvarabc", "-nvar1234"};
    CCommandLine cl(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    std::string ssVar; cl.DefineOption("ssvar", ssVar, "first variable");
    size_t nVar = 0; cl.DefineOption("nvar", nVar, "second variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(ssVar, "abc");
    EXPECT_EQ(nVar, 1234);
}

TEST_F(CCommandLineParserTest, MultipleDefaultArgs)
{
    const char* rgszCommandLine[] = {"this_exe.app", "arg1", "arg2", "arg3"};
    CCommandLine cl;
    std::vector<std::string> vecDefault; cl.DefineDefaultArgument(vecDefault, "default argument");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    ASSERT_EQ(vecDefault.size(), 3);
    EXPECT_EQ(vecDefault[0], "arg1");
    EXPECT_EQ(vecDefault[1], "arg2");
    EXPECT_EQ(vecDefault[2], "arg3");
}

TEST_F(CCommandLineParserTest, MultipleOptionArgs)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-opt=arg1", "-opt=arg2", "-opt=arg3"};
    CCommandLine cl;
    std::vector<std::string> vecOption; cl.DefineOption("opt", vecOption, "default argument");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    ASSERT_EQ(vecOption.size(), 3);
    EXPECT_EQ(vecOption[0], "arg1");
    EXPECT_EQ(vecOption[1], "arg2");
    EXPECT_EQ(vecOption[2], "arg3");
}

TEST_F(CCommandLineParserTest, MultipleOptionArgsNoAssignmentChar)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-optarg1", "-optarg2", "-optarg3"};
    CCommandLine cl(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    std::vector<std::string> vecOption; cl.DefineOption("opt", vecOption, "default argument");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    ASSERT_EQ(vecOption.size(), 3);
    EXPECT_EQ(vecOption[0], "arg1");
    EXPECT_EQ(vecOption[1], "arg2");
    EXPECT_EQ(vecOption[2], "arg3");
}

TEST_F(CCommandLineParserTest, MultipleSubOptionArgs)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--opt=arg1", "--opt=arg2", "--opt=arg3"};
    CCommandLine cl;
    std::vector<std::string> vecOption; cl.DefineSubOption("opt", vecOption, "default argument");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    ASSERT_EQ(vecOption.size(), 3);
    EXPECT_EQ(vecOption[0], "arg1");
    EXPECT_EQ(vecOption[1], "arg2");
    EXPECT_EQ(vecOption[2], "arg3");
}

TEST_F(CCommandLineParserTest, MultipleSubOptionArgsNoAssignmentChar)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--optarg1", "--optarg2", "--optarg3"};
    CCommandLine cl(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    std::vector<std::string> vecOption; cl.DefineSubOption("opt", vecOption, "default argument");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    ASSERT_EQ(vecOption.size(), 3);
    EXPECT_EQ(vecOption[0], "arg1");
    EXPECT_EQ(vecOption[1], "arg2");
    EXPECT_EQ(vecOption[2], "arg3");
}


// Add exceptions (invalid command line args, arg assignments out of scope or too large/small)
// - Wrong default arguments
// - Multiple default arguments, but only one var
// - Wrong option/sub-option
// - Same name for option/sub-option
// - Assignment with semi-colon and assignment character
// - Case sensitivity
// - Multiple assignments with same option adding to vector or list