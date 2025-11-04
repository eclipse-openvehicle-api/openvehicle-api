#ifndef COMPOSER_TEST_SUITE_H
#define COMPOSER_TEST_SUITE_H

#include <gtest/gtest.h>
#include <filesystem>
#include <regex>
#include <string>

/**
 * @brief Install package composer test suite class.
 */
class CPathMatchTest : public ::testing::Test
{
public:
    /**
     * @brief Setup test test
     */
    virtual void SetUp();

    /**
     * @brief Tear down test
     */
    virtual void TearDown();

    /**
     * @brief Setup test case
     */
    static void SetUpTestCase();

    /**
     * @brief Tear down test case
     */
    static void TearDownTestSuite();
};

#endif // !defined COMPOSER_TEST_SUITE_H