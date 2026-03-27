/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef LEXER_TEST_H
#define LEXER_TEST_H

/**
* @brief Test class for instantiation tests.
*/
class CLexerTest : public testing::Test
{
public:
	/**
	* @brief Constructor
	*/
	CLexerTest() = default;

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

#endif // !defined LEXER_TEST_H