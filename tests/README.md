# Running tests

When the project is build the tests are executed with warning level. 
To run the test without warning level aou have to execute the test locally by executing the script files:

**On Windows PowerShell:** run_tests_on_windows.ps1
or
**On Linux:** run_tests_on_linux.sh



# SDV TEST MACRO
## Overview
There is set of macros in "tests/include/sdv_test_macro.h" which are designed to extend the Google Test (GTEST) framework by incorporating a warning level mechanism. These macros allow developers to specify a warning level for each test assertion, providing more control over test outcomes and enabling better handling of test result conditions based on their warning level.

## Purpose
The primary purpose of these macros is to enhance the flexibility and robustness of test assertions in the GTEST framework. By integrating a warning level, developers can decide whether to report failed tests as warning or error and handle them accordingly. This is particularly useful in where different test failures may have varying impacts on the overall build system.

## Macros
### Equality Assertions:
These macros check for equality or inequality between two values. The level parameter specifies the warning level to be handled by the HANDLE_WARNING_LEVEL function.
- SDV_EXPECT_EQ(val1, val2, level)
- SDV_ASSERT_EQ(val1, val2, level)
- SDV_EXPECT_NE(val1, val2, level)
- SDV_ASSERT_NE(val1, val2, level)

### Boolean Assertions:
These macros check if a condition is true or false. The level parameter specifies the warning level to be handled by the HANDLE_WARNING_LEVEL function.
- SDV_EXPECT_TRUE(condition, level)
- SDV_ASSERT_TRUE(condition, level)
- SDV_EXPECT_FALSE(condition, level)
- SDV_ASSERT_FALSE(condition, level)

### Relational Assertions:
These macros check for relational conditions (less than, less than or equal to, greater than, greater than or equal to) between two values. The level parameter specifies the warning level to be handled by the HANDLE_WARNING_LEVEL function.
- SDV_EXPECT_LT(val1, val2, level)
- SDV_ASSERT_LT(val1, val2, level)
- SDV_EXPECT_LE(val1, val2, level)
- SDV_ASSERT_LE(val1, val2, level)
- SDV_EXPECT_GT(val1, val2, level)
- SDV_ASSERT_GT(val1, val2, level)
- SDV_EXPECT_GE(val1, val2, level)
- SDV_ASSERT_GE(val1, val2, level)

### String Assertions:
These macros check for string equality or inequality, both case-sensitive and case-insensitive. The level parameter specifies the warning level to be handled by the HANDLE_WARNING_LEVEL function.
- SDV_EXPECT_STREQ(str1, str2, level)
- SDV_ASSERT_STREQ(str1, str2, level)
- SDV_EXPECT_STRNE(str1, str2, level)
- SDV_ASSERT_STRNE(str1, str2, level)
- SDV_EXPECT_STRCASEEQ(str1, str2, level)
- SDV_ASSERT_STRCASEEQ(str1, str2, level)
- SDV_EXPECT_STRCASENE(str1, str2, level)
- SDV_ASSERT_STRCASENE(str1, str2, level)

## Warning Levels
The WarningLevel enum defines three levels of warnings:
- WARNING_ENABLED: With this warning level, failed tests are reported as errors, as is standard in GTEST.
- WARNING_REDUCED: With warning this level, failed tests are reported as warnings.
- WARNING_DISABLED: With this warning level, no action is implemented at this moment.

Additionally SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD is implemented to check if tests are running with CMake build or any other way. It returns true if they are running with CMake build.

## Usage
To use these macros, include the header file '#include "tests/include/sdv_test_macro.h"' in your test files. When writing test cases, use the SDV macros instead of the standard GTEST macros, and specify the appropriate warning level for each assertion.

```cpp
#include "tests/include/sdv_test_macro.h"

TEST(SDVTestMacros, TestExpectEq)
{
    int val1 = 5;
    int val2 = 5;
    if (SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
        SDV_EXPECT_EQ(val1, val2, sdv_test::WARNING_REDUCED);
    else
        SDV_EXPECT_EQ(val1, val2, sdv_test::WARNING_ENABLED);
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
```
In this example, the SDV_EXPECT_EQ macro checks if val1 and val2 are equal, while the SDV_EXPECT_STREQ macro checks if str1 and str2 are equal. In both test cases, it is checked whether it is running
with the Cmake build. If it is running with the CMake build then report test failures as warning, otherwise report normally as GTEST does.