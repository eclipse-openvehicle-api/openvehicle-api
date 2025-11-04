#ifndef SDV_TEST_MACRO_H
#define SDV_TEST_MACRO_H

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
	/**
	 * @brief Namespace TEST
	 */
    namespace TEST
    {
        /**
         * @brief Enum for warning levels.
         */
        enum WarningLevel
        {
            WARNING_ENABLED,	//<! With this level Failed tests are reported as error as it is in GTEST normally.
            WARNING_REDUCED,	//<! With this level Failed tests are reported as warning.
			WARNING_DISABLED	//<! With this level no action is implemented at this moment.
        };

        /**
         * @brief Function to report a warning when the expected condition is not met.
         * @param[in] condition The condition that is being checked (should be true if valid).
         * @param[in] warningLevel The level of warning to display.
         * @param[in] message The warning message to display.
         * @param[in] file The name of the file where the warning occurred.
         * @param[in] line The line number where the warning occurred.
         */
        inline void ReportWarning(bool condition, WarningLevel warningLevel, const std::string& message, const char* file, int line)
        {
            if (!condition)
            {
                switch (warningLevel)
                {
                    case WARNING_ENABLED:
                        FAIL() << "[  FAILED  ]: " << message << " in file " << file << " on line " << line << std::endl;
                        break;
                    case WARNING_DISABLED:
                        // No action
                        break;
                    case WARNING_REDUCED:
                        std::clog << "[[ WARNING ]] TEST FAILURE NOT EVALUATED: " << message << " in file " << file << " on line " << line << std::endl;
                        break;
                    default:
                        break;
                }
            }
        }

        /**
         * @brief Function to detect whether tests are running with CMAKE build or locally manually.
         * @return Returns true if it is running with CMAKE build, otherwise false.
         */
        inline bool IsRunningTestsWithCmakeBuild() 
        {
            auto envVar = std::getenv("TEST_EXECUTION_MODE");

            if (envVar && std::string(envVar) == "CMake") return true;
            else return false;
        }
    } // namespace TEST
} // namespace sdv

/**
 * @brief Macro for checking whether tests are running with CMAKE build or locally manually.
 * Returns true if it is running with CMAKE build, otherwise false.
 */
#define SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD sdv::TEST::IsRunningTestsWithCmakeBuild()

/**
 * @brief Helper macro to handle warning levels.
 * @param[in] level The warning level.
 * @param[in] statement The statement to execute.
 * @param[in] val1 The first value for comparison.
 * @param[in] val2 The second value for comparison.
 * @param[in] condition The condition to check.
 */
#define HANDLE_WARNING_LEVEL(level, statement, val1, val2, condition)																\
	do																																\
	{																																\
		switch (level)																												\
		{																															\
		case sdv::TEST::WarningLevel::WARNING_ENABLED:																				\
			statement;																												\
			break;																													\
		case sdv::TEST::WarningLevel::WARNING_REDUCED:																				\
			if (val1 condition val2) statement;																						\
			else																													\
			{																														\
				std::ostringstream oss;																								\
				oss << "[[ WARNING ]] TEST FAILURE NOT EVALUATED: Condition did not match for [" 									\
					<< #val1 "] and [" #val2 << "] in file " << __FILE__ << " on line " << __LINE__;								\
				std::clog << oss.str() << std::endl;																				\
			}																														\
			break;																													\
			case sdv::TEST::WarningLevel::WARNING_DISABLED: /* No action */															\
			break;																													\
		default:																													\
			break;																													\
		}																															\
	} while (0)

/**
 * @brief Redefine GTEST macros with warning level.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] level The warning level.
 */
#define SDV_EXPECT_EQ(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_EQ(val1, val2), val1, val2, ==)
		
#define SDV_ASSERT_EQ(val1, val2, level)																							\
	 HANDLE_WARNING_LEVEL(level, ASSERT_EQ(val1, val2), val1, val2, ==)
		
#define SDV_EXPECT_NE(val1, val2, level)																							\
	 HANDLE_WARNING_LEVEL(level, EXPECT_NE(val1, val2), val1, val2, !=)

#define SDV_ASSERT_NE(val1, val2, level)																							\
	 HANDLE_WARNING_LEVEL(level, ASSERT_NE(val1, val2), val1, val2, !=)

#define SDV_EXPECT_TRUE(condition, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_TRUE(condition), condition, true, ==)
	
#define SDV_ASSERT_TRUE(condition, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_TRUE(condition), condition, true, ==)
	
#define SDV_EXPECT_FALSE(condition, level)																							\
	 HANDLE_WARNING_LEVEL(level, EXPECT_FALSE(condition), condition, false, ==)
		
#define SDV_ASSERT_FALSE(condition, level)																							\
	 HANDLE_WARNING_LEVEL(level, ASSERT_FALSE(condition), condition, false, ==)

#define SDV_EXPECT_LT(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_LT(val1, val2), val1, val2, <)
																									
#define SDV_ASSERT_LT(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_LT(val1, val2), val1, val2, <)
																									
#define SDV_EXPECT_LE(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_LE(val1, val2), val1, val2, <=)
																								
#define SDV_ASSERT_LE(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_LE(val1, val2), val1, val2, <=)
																								
#define SDV_EXPECT_GT(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_GT(val1, val2), val1, val2, >)
																								
#define SDV_ASSERT_GT(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_GT(val1, val2), val1, val2, >)

#define SDV_EXPECT_GE(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_GE(val1, val2), val1, val2, >=)

#define SDV_ASSERT_GE(val1, val2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_GE(val1, val2), val1, val2, >=)

#define SDV_EXPECT_STREQ(str1, str2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_STREQ((str1).c_str(), (str2).c_str()), str1, str2, ==)

#define SDV_ASSERT_STREQ(str1, str2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_STREQ((str1).c_str(), (str2).c_str()), str1, str2, ==)

#define SDV_EXPECT_STRNE(str1, str2, level)																							\
	HANDLE_WARNING_LEVEL(level, EXPECT_STRNE(str1.c_str(), str2.c_str()), str1, str2, !=)

#define SDV_ASSERT_STRNE(str1, str2, level)																							\
	HANDLE_WARNING_LEVEL(level, ASSERT_STRNE(str1.c_str(), str2.c_str()), str1, str2, !=)

#define SDV_EXPECT_STRCASEEQ(str1, str2, level)																						\
	HANDLE_WARNING_LEVEL(level, EXPECT_STRCASEEQ((str1).c_str(), (str2).c_str()), str1, str2, ==)

#define SDV_ASSERT_STRCASEEQ(str1, str2, level)																						\
	HANDLE_WARNING_LEVEL(level, ASSERT_STRCASEEQ((str1).c_str(), (str2).c_str()), str1, str2, ==)

#define SDV_EXPECT_STRCASENE(str1, str2, level)																						\
	HANDLE_WARNING_LEVEL(level, EXPECT_STRCASENE((str1).c_str(), (str2).c_str()), str1, str2, !=)

#define SDV_ASSERT_STRCASENE(str1, str2, level)																						\
	HANDLE_WARNING_LEVEL(level, ASSERT_STRCASENE((str1).c_str(), (str2).c_str()), str1, str2, !=)

/**
 * @brief Macro for equality check (==) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_EQ(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) == (val2), warningLevel,																		\
		"Expected " #val1 " == " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__,  __LINE__)

/**
 * @brief Macro for inequality check (!=) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_NE(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) != (val2), warningLevel,																		\
		"Expected " #val1 " != " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__,  __LINE__)

/**
 * @brief Macro for greater-than check (>) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_GT(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) > (val2), warningLevel,																			\
		"Expected " #val1 " > " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__, __LINE__)

/**
 * @brief Macro for less-than check (<) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_LT(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) < (val2), warningLevel,																			\
		"Expected " #val1 " < " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__,  __LINE__)

/**
 * @brief Macro for greater-than-or-equal-to check (>=) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_GE(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) >= (val2),  warningLevel,																		\
		"Expected " #val1 " >= " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__, __LINE__)

/**
 * @brief Macro for less-than-or-equal-to check (<=) with warning reporting for time critical tests.
 * @param[in] val1 The first value.
 * @param[in] val2 The second value.
 * @param[in] warningLevel The level of warning to display.
 */
#define SDV_TIMING_EXPECT_LE(val1, val2, warningLevel)																				\
	sdv::TEST::ReportWarning((val1) <= (val2),  warningLevel,																		\
		"Expected " #val1 " <= " #val2 " (" #val1 "=" + std::to_string(val1) + ", " #val2 "=" + std::to_string(val2) + ")",			\
		__FILE__, __LINE__)

#endif // SDV_TEST_MACRO_H
