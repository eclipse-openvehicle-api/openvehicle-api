#ifndef LEXER_TEST_H
#define LEXER_TEST_H

/**
* \brief Test class for instantiation tests.
*/
class CLexerTest : public testing::Test
{
public:
	/**
	* \brief Constructor
	*/
	CLexerTest() = default;

	/**
	* \brief Set up the test suite.
	*/
	static void SetUpTestCase();

	/**
	* \brief Tear down the test suite.
	*/
	static void TearDownTestCase();

	/**
	* \brief Test setup.
	*/
	void SetUp() override;

	/**
	* \brief Test teardown.
	*/
	void TearDown() override;

};

#endif // !defined LEXER_TEST_H