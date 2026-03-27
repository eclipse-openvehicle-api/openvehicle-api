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

#include <gtest/gtest.h>

#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"

// Test TODO:
// Switch to inline and vice versa

TEST(GenerateTOML, SwitchTableToInline)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code([table]
key1 = "some string"
key2 = 123)code";
    std::string ssTOMLOutput = R"code(table = {key1 = "some string", key2 = 123})code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    EXPECT_EQ(ssGenerated, ssTOMLInput);

    sdv::TInterfaceAccessPtr ptrTable = parser.Root().GetNodeDirect("table");
    EXPECT_TRUE(ptrTable);
    sdv::toml::INodeInfo* pInfo = ptrTable.GetInterface<sdv::toml::INodeInfo>();
    sdv::toml::INodeCollectionConvert* pConvert = ptrTable.GetInterface<sdv::toml::INodeCollectionConvert>();
    ASSERT_NE(pInfo, nullptr);
    ASSERT_NE(pConvert, nullptr);
    EXPECT_FALSE(pInfo->IsInline());
    EXPECT_TRUE(pConvert->CanMakeInline());
    EXPECT_TRUE(pConvert->MakeInline());
    EXPECT_TRUE(pInfo->IsInline());

    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}
