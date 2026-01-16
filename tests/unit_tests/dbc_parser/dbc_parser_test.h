#ifndef COMMANDLINE_PARSER_TEST_H
#define COMMANDLINE_PARSER_TEST_H

#include <support/mem_access.h>
#include <support/interface_ptr.h>

/**
* @brief Test class for instantiation tests.
*/
class CDbcParserTest : public testing::Test
{
public:
    /**
    * @brief Constructor
    */
    CDbcParserTest() = default;

    /**
    * @brief Set up the test suite.
    */
    static void SetUpTestCase();

    /**
    * @brief Tear down the test suite.
    */
    static void TearDownTestCase();

    /**
    * @brief Test setup.
    */
    void SetUp() override;

    /**
    * @brief Test teardown.
    */
    void TearDown() override;
};

#endif // !defined COMMANDLINE_PARSER_TEST_H