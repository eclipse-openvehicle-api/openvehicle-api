#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../include/sdv_test_macro.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(SDVTestMacros, TestExpectEq)
{
    int val1 = 5;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_EQ(val1, val2, sdv_test::WARNING_REDUCED);
	else
        SDV_EXPECT_EQ(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertEq)
{
    int val1 = 5;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_EQ(val1, val2, sdv_test::WARNING_REDUCED);
	else
        SDV_ASSERT_EQ(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectNe)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_NE(val1, val2, sdv_test::WARNING_REDUCED);
	else
		SDV_EXPECT_NE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertNe)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
        SDV_ASSERT_NE(val1, val2, sdv_test::WARNING_REDUCED);
    else
		SDV_ASSERT_NE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectTrue)
{
    bool condition = true;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_TRUE(condition, sdv_test::WARNING_REDUCED);
	else
		SDV_EXPECT_TRUE(condition, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertTrue)
{
    bool condition = true;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_TRUE(condition, sdv_test::WARNING_REDUCED);
	else
		SDV_ASSERT_TRUE(condition, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectFalse)
{
    bool condition = false;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_FALSE(condition, sdv_test::WARNING_REDUCED);
    else
		SDV_EXPECT_FALSE(condition, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertFalse)
{
    bool condition = false;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
        SDV_ASSERT_FALSE(condition, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_FALSE(condition, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectLt)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_LT(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_LT(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertLt)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_LT(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_LT(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectLe)
{
    int val1 = 5;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_LE(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_LE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertLe)
{
    int val1 = 5;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_LE(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_LE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectGt)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_GT(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_GT(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertGt)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_GT(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_GT(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectGe)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_GE(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_GE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertGe)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_GE(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_GE(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectStreq)
{
    std::string str1 = "test";
    std::string str2 = "test";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_STREQ(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_STREQ(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertStreq)
{
    std::string str1 = "test";
    std::string str2 = "test";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_STREQ(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_STREQ(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectStrne)
{
    std::string str1 = "test";
    std::string str2 = "test1";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_STRNE(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_STRNE(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertStrne)
{
    std::string str1 = "test";
    std::string str2 = "test1";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_STRNE(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_STRNE(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectStrcaseeq)
{
    std::string str1 = "test";
    std::string str2 = "TEST";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_STRCASEEQ(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_STRCASEEQ(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertStrcaseeq)
{
    std::string str1 = "test";
    std::string str2 = "TEST";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_STRCASEEQ(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_STRCASEEQ(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestExpectStrcasene)
{
    std::string str1 = "test";
    std::string str2 = "TEST1";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_STRCASENE(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_STRCASENE(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestAssertStrcasene)
{
    std::string str1 = "test";
    std::string str2 = "TEST1";
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_ASSERT_STRCASENE(str1, str2, sdv_test::WARNING_REDUCED);
    else
        SDV_ASSERT_STRCASENE(str1, str2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectEq)
{
    int val1 = 5;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_EQ_WARN(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_EQ_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectNe)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_NE_WARN(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_NE_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectGt)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_GT_WARN(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_GT_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectLt)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_LT_WARN(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_LT_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectGe)
{
    int val1 = 6;
    int val2 = 5;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_GE_WARN(val1, val2, sdv_test::WARNING_REDUCED);
	else
		SDV_EXPECT_GE_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}

TEST(SDVTestMacros, TestTimingExpectLe)
{
    int val1 = 5;
    int val2 = 6;
	if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
		SDV_EXPECT_LE_WARN(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_LE_WARN(val1, val2, sdv_test::WARNING_ENABLED);
}
