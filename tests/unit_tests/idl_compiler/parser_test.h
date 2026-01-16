#ifndef PARSER_TEST_H
#define PARSER_TEST_H

/**
* @brief Test class for instantiation tests.
*/
class CParserTest : public testing::Test
{
public:
	/**
	* @brief Constructor
	*/
	CParserTest() = default;

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

#endif // define PARSER_TEST_H