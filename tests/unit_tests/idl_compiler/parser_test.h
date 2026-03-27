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