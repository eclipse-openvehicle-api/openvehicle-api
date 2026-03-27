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