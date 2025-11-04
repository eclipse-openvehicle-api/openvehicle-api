#include "../../include/gtest_custom.h"
#include "commandline_parser_test.h"
#include "../../../global/cmdlnparser/cmdlnparser.h"

using CCommandLineParserTestIncompatibleArguments = CCommandLineParserTest;

TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseSensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-i8var=-1", "-i16var=-2", "-i32var=-3", "-i64var=-4",
        "-ui8var=5", "-ui16var=6", "-ui32var=7", "-ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "-i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}

#ifdef _WIN32
TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseSensitiveFullArgumentWin)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8var=-1", "/i16var=-2", "/i32var=-3", "/i64var=-4",
        "/ui8var=5", "/ui16var=6", "/ui32var=7", "/ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "/i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}
#endif

TEST_F(CCommandLineParserTestIncompatibleArguments, SubOptionParseCaseSensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--i8var=-1", "--i16var=-2", "--i32var=-3", "--i64var=-4",
        "--ui8var=5", "--ui16var=6", "--ui32var=7", "--ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineSubOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineSubOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineSubOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineSubOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineSubOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineSubOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineSubOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineSubOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineSubOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineSubOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineSubOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineSubOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineSubOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineSubOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineSubOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineSubOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "--i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}

TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseInsensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-i8VAR=-1", "-i16VAR=-2", "-i32VAR=-3", "-i64VAR=-4",
        "-ui8VAR=5", "-ui16VAR=6", "-ui32VAR=7", "-ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "-i16VAR=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}

#ifdef _WIN32
TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseInsensitiveFullArgumentWin)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8VAR=-1", "/i16VAR=-2", "/i32VAR=-3", "/i64VAR=-4",
        "/ui8VAR=5", "/ui16VAR=6", "/ui32VAR=7", "/ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "/i16VAR=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}
#endif

TEST_F(CCommandLineParserTestIncompatibleArguments, SubOptionParseCaseInsensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--i8VAR=-1", "--i16VAR=-2", "--i32VAR=-3", "--i64VAR=-4",
        "--ui8VAR=5", "--ui16VAR=6", "--ui32VAR=7", "--ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineSubOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineSubOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineSubOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineSubOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineSubOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineSubOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineSubOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineSubOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineSubOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineSubOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineSubOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineSubOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineSubOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineSubOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineSubOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineSubOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "--i16VAR=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}

TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseSensitiveNameArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-i8var=-1", "-i16var=-2", "-i32var=-3", "-i64var=-4",
        "-ui8var=5", "-ui16var=6", "-ui32var=7", "-ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "-i16var"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
}

#ifdef _WIN32
TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseSensitiveNameArgumentWin)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8var=-1", "/i16var=-2", "/i32var=-3", "/i64var=-4",
        "/ui8var=5", "/ui16var=6", "/ui32var=7", "/ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "/i16var"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
}
#endif

TEST_F(CCommandLineParserTestIncompatibleArguments, SubOptionParseCaseSensitiveNameArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--i8var=-1", "--i16var=-2", "--i32var=-3", "--i64var=-4",
        "--ui8var=5", "--ui16var=6", "--ui32var=7", "--ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineSubOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0; cl.DefineSubOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0; cl.DefineSubOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0; cl.DefineSubOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0; cl.DefineSubOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0; cl.DefineSubOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0; cl.DefineSubOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0; cl.DefineSubOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);
    int8_t i8Control = -10; cl.DefineSubOption("i8control", i8Control, "int8_t no change!", true, 0, 1);
    int16_t i16Control = -20; cl.DefineSubOption("i16control", i16Control, "int16_t no change!", true, 1, 2);
    int32_t i32Control = -30; cl.DefineSubOption("i32control", i32Control, "int32_t no change!", true, 2, 3);
    int64_t i64Control = -40; cl.DefineSubOption("i64control", i64Control, "int64_t no change!", true, 3, 4);
    uint8_t ui8Control = 50; cl.DefineSubOption("ui8control", ui8Control, "uint8_t no change!", true, 4, 5);
    uint16_t ui16Control = 60; cl.DefineSubOption("ui16control", ui16Control, "uint16_t no change!", true, 5, 6);
    uint32_t ui32Control = 70; cl.DefineSubOption("ui32control", ui32Control, "uint32_t no change!", true, 6, 7);
    uint64_t ui64Control = 80; cl.DefineSubOption("ui64control", ui64Control, "uint64_t no change!", true, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "--i16var"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
}

TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseInsensitiveNameArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-i8VAR=-1", "-i16VAR=-2", "-i32VAR=-3", "-i64VAR=-4",
        "-ui8VAR=5", "-ui16VAR=6", "-ui32VAR=7", "-ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "-i16VAR"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
    }

#ifdef _WIN32
TEST_F(CCommandLineParserTestIncompatibleArguments, OptionParseCaseInsensitiveNameArgumentWin)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8VAR=-1", "/i16VAR=-2", "/i32VAR=-3", "/i64VAR=-4",
        "/ui8VAR=5", "/ui16VAR=6", "/ui32VAR=7", "/ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "/i16VAR"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
}
#endif

TEST_F(CCommandLineParserTestIncompatibleArguments, SubOptionParseCaseInsensitiveNameArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--i8VAR=-1", "--i16VAR=-2", "--i32VAR=-3", "--i64VAR=-4",
        "--ui8VAR=5", "--ui16VAR=6", "--ui32VAR=7", "--ui64VAR=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineSubOption("i8var", i8Var, "int8_t variable", false, 0, 1);
    int16_t i16Var = 0; cl.DefineSubOption("i16var", i16Var, "int16_t variable", false, 1, 2);
    int32_t i32Var = 0; cl.DefineSubOption("i32var", i32Var, "int32_t variable", false, 2, 3);
    int64_t i64Var = 0; cl.DefineSubOption("i64var", i64Var, "int64_t variable", false, 3, 4);
    uint8_t ui8Var = 0; cl.DefineSubOption("ui8var", ui8Var, "uint8_t variable", false, 4, 5);
    uint16_t ui16Var = 0; cl.DefineSubOption("ui16var", ui16Var, "uint16_t variable", false, 5, 6);
    uint32_t ui32Var = 0; cl.DefineSubOption("ui32var", ui32Var, "uint32_t variable", false, 6, 7);
    uint64_t ui64Var = 0; cl.DefineSubOption("ui64var", ui64Var, "uint64_t variable", false, 7, 8);
    int8_t i8Control = -10; cl.DefineSubOption("i8control", i8Control, "int8_t no change!", false, 0, 1);
    int16_t i16Control = -20; cl.DefineSubOption("i16control", i16Control, "int16_t no change!", false, 1, 2);
    int32_t i32Control = -30; cl.DefineSubOption("i32control", i32Control, "int32_t no change!", false, 2, 3);
    int64_t i64Control = -40; cl.DefineSubOption("i64control", i64Control, "int64_t no change!", false, 3, 4);
    uint8_t ui8Control = 50; cl.DefineSubOption("ui8control", ui8Control, "uint8_t no change!", false, 4, 5);
    uint16_t ui16Control = 60; cl.DefineSubOption("ui16control", ui16Control, "uint16_t no change!", false, 5, 6);
    uint32_t ui32Control = 70; cl.DefineSubOption("ui32control", ui32Control, "uint32_t no change!", false, 6, 7);
    uint64_t ui64Control = 80; cl.DefineSubOption("ui64control", ui64Control, "uint64_t no change!", false, 7, 8);

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0, false).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0, false)[0], "--i16VAR"); // Argument without assignment
    EXPECT_EQ(cl.IncompatibleArguments(1, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7, false).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8, false).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9, false).size(), 7);    // Arguments of group #0 are always included.
}

TEST_F(CCommandLineParserTestIncompatibleArguments, MultiOptionParseCaseSensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "-i8var=-1", "-i16var=-2", "-i32var=-3", "-i64var=-4",
        "-ui8var=5", "-ui16var=6", "-ui32var=7", "-ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0;
    auto& rI8Var = cl.DefineOption("i8VAR", i8Var, "int8_t variable", true, 0, 1);
    rI8Var.AddOptionName("i8var");
    int16_t i16Var = 0;
    auto& rI16Var = cl.DefineOption("i16VAR", i16Var, "int16_t variable", true, 1, 2);
    rI16Var.AddOptionName("i16var");
    int32_t i32Var = 0;
    auto& rI32Var = cl.DefineOption("i32VAR", i32Var, "int32_t variable", true, 2, 3);
    rI32Var.AddOptionName("i32var");
    int64_t i64Var = 0;
    auto& rI64Var = cl.DefineOption("i64VAR", i64Var, "int64_t variable", true, 3, 4);
    rI64Var.AddOptionName("i64var");
    uint8_t ui8Var = 0;
    auto& rUI8Var = cl.DefineOption("ui8VAR", ui8Var, "uint8_t variable", true, 4, 5);
    rUI8Var.AddOptionName("ui8var");
    uint16_t ui16Var = 0;
    auto& rUI16Var = cl.DefineOption("ui16VAR", ui16Var, "uint16_t variable", true, 5, 6);
    rUI16Var.AddOptionName("ui16var");
    uint32_t ui32Var = 0;
    auto& rUI32Var = cl.DefineOption("ui32VAR", ui32Var, "uint32_t variable", true, 6, 7);
    rUI32Var.AddOptionName("ui32var");
    uint64_t ui64Var = 0;
    auto& rUI64Var = cl.DefineOption("ui64VAR", ui64Var, "uint64_t variable", true, 7, 8);
    rUI64Var.AddOptionName("ui64var");

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "-i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}

#ifdef _WIN32
TEST_F(CCommandLineParserTestIncompatibleArguments, MultiOptionParseCaseSensitiveFullArgumentWin)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8var=-1", "/i16var=-2", "/i32var=-3", "/i64var=-4",
        "/ui8var=5", "/ui16var=6", "/ui32var=7", "/ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0;
    auto& rI8Var = cl.DefineOption("i8VAR", i8Var, "int8_t variable", true, 0, 1);
    rI8Var.AddOptionName("i8var");
    int16_t i16Var = 0;
    auto& rI16Var = cl.DefineOption("i16VAR", i16Var, "int16_t variable", true, 1, 2);
    rI16Var.AddOptionName("i16var");
    int32_t i32Var = 0;
    auto& rI32Var = cl.DefineOption("i32VAR", i32Var, "int32_t variable", true, 2, 3);
    rI32Var.AddOptionName("i32var");
    int64_t i64Var = 0;
    auto& rI64Var = cl.DefineOption("i64VAR", i64Var, "int64_t variable", true, 3, 4);
    rI64Var.AddOptionName("i64var");
    uint8_t ui8Var = 0;
    auto& rUI8Var = cl.DefineOption("ui8VAR", ui8Var, "uint8_t variable", true, 4, 5);
    rUI8Var.AddOptionName("ui8var");
    uint16_t ui16Var = 0;
    auto& rUI16Var = cl.DefineOption("ui16VAR", ui16Var, "uint16_t variable", true, 5, 6);
    rUI16Var.AddOptionName("ui16var");
    uint32_t ui32Var = 0;
    auto& rUI32Var = cl.DefineOption("ui32VAR", ui32Var, "uint32_t variable", true, 6, 7);
    rUI32Var.AddOptionName("ui32var");
    uint64_t ui64Var = 0;
    auto& rUI64Var = cl.DefineOption("ui64VAR", ui64Var, "uint64_t variable", true, 7, 8);
    rUI64Var.AddOptionName("ui64var");

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "/i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);    // Arguments of group #0 are always included.
}
#endif

TEST_F(CCommandLineParserTestIncompatibleArguments, MultiSubOptionParseCaseSensitiveFullArgument)
{
    const char* rgszCommandLine[] = {"this_exe.app", "--i8var=-1", "--i16var=-2", "--i32var=-3", "--i64var=-4",
        "--ui8var=5", "--ui16var=6", "--ui32var=7", "--ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0;
    auto& rI8Var = cl.DefineSubOption("i8VAR", i8Var, "int8_t variable", true, 0, 1);
    rI8Var.AddSubOptionName("i8var");
    int16_t i16Var = 0;
    auto& rI16Var = cl.DefineSubOption("i16VAR", i16Var, "int16_t variable", true, 1, 2);
    rI16Var.AddSubOptionName("i16var");
    int32_t i32Var = 0;
    auto& rI32Var = cl.DefineSubOption("i32VAR", i32Var, "int32_t variable", true, 2, 3);
    rI32Var.AddSubOptionName("i32var");
    int64_t i64Var = 0;
    auto& rI64Var = cl.DefineSubOption("i64VAR", i64Var, "int64_t variable", true, 3, 4);
    rI64Var.AddSubOptionName("i64var");
    uint8_t ui8Var = 0;
    auto& rUI8Var = cl.DefineSubOption("ui8VAR", ui8Var, "uint8_t variable", true, 4, 5);
    rUI8Var.AddSubOptionName("ui8var");
    uint16_t ui16Var = 0;
    auto& rUI16Var = cl.DefineSubOption("ui16VAR", ui16Var, "uint16_t variable", true, 5, 6);
    rUI16Var.AddSubOptionName("ui16var");
    uint32_t ui32Var = 0;
    auto& rUI32Var = cl.DefineSubOption("ui32VAR", ui32Var, "uint32_t variable", true, 6, 7);
    rUI32Var.AddSubOptionName("ui32var");
    uint64_t ui64Var = 0;
    auto& rUI64Var = cl.DefineSubOption("ui64VAR", ui64Var, "uint64_t variable", true, 7, 8);
    rUI64Var.AddSubOptionName("ui64var");

    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));

    // For each group check the supplied arguments. The arguments belonging to group 0 are compatible with all groups. The other
    // arguments are compatible with the group they have been assigned to only.
    ASSERT_EQ(cl.IncompatibleArguments(0).size(), 7);
    EXPECT_EQ(cl.IncompatibleArguments(0)[0], "--i16var=-2"); // Full argument
    EXPECT_EQ(cl.IncompatibleArguments(1).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(2).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(3).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(4).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(5).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(6).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(7).size(), 5);
    EXPECT_EQ(cl.IncompatibleArguments(8).size(), 6);
    EXPECT_EQ(cl.IncompatibleArguments(9).size(), 7);     // Arguments of group #0 are always included.
}
