#ifndef GTEST_CUSTOM_H
#define GTEST_CUSTOM_H

#ifdef _MSC_VER
#pragma warning(disable: 4102)
#endif

#define GTEST_HAS_EXCEPTIONS 1
#include <gtest/gtest.h>

#include <iostream>
#include <algorithm>
#include <math.h>

#ifndef countof
/**
 * @brief Count the amount of elements in the array.
 * @tparam T Array base type.
 * @tparam N The amount of elements.
 * @return The amount of elements in the array.
 */
template <typename T, int N>
constexpr int countof(T const (&)[N]) noexcept
{
    return N;
}
#endif

/**
 * @brief Equality test macro for container functions.
 */
#define EXPECT_ARREQ(val1, ...) { bool b = val1 == decltype(val1)({__VA_ARGS__}); EXPECT_TRUE(b); }

/**
 * @brief Unequality test macro for container functions.
 */
#define EXPECT_ARRNE(val1, ...) { bool b = val1 == decltype(val1)({__VA_ARGS__}); EXPECT_FALSE(b); }

/**
 * @brief Check for equality fo floating point values.
 */
#define EXPECT_FPEQ(val1, val2) EXPECT_TRUE(std::fabs(val1 - val2) < std::numeric_limits<decltype(val1)>::epsilon());

#include "simple_cpp_decomposer.h"

/**
 * @brief Compare CPP code for equality.
 */
#define EXPECT_CPPEQ(s1, s2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperCPPEQ, s1, s2)

/**
 * @brief Compare CPP code for inequality.
 */
#define EXPECT_CPPNE(s1, s2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperCPPNE, s1, s2)


#endif // ! defined GTEST_CUSTOM_H