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
 *   Martin Stimpfl - initial API and implementation
 *   Erik Verhoeven - writing TOML and whitespace preservation
 ********************************************************************************/

#include <gtest/gtest.h>
#include "../../../global/localmemmgr.h"
#include "../../../sdv_services/core/toml_parser/parser_node_indexer.h"

TEST(IndexerTest, InitializeSingle)
{
    toml_parser::CNodeIndexer indexer;

    EXPECT_NO_THROW(toml_parser::CNodeIndex index = indexer.CreateIndex());
}

TEST(IndexerTest, CompareOrder)
{
    toml_parser::CNodeIndexer indexer;
    auto index1 = indexer.CreateIndex();
    auto index2 = indexer.CreateIndex();
    auto index3 = indexer.CreateIndex();
    EXPECT_GT(index2, index1);
    EXPECT_GT(index3, index1);
    EXPECT_GT(index3, index2);
}

TEST(IndexerTest, SwitchOrder)
{
    toml_parser::CNodeIndexer indexer;
    auto index1 = indexer.CreateIndex();
    auto index2 = indexer.CreateIndex();
    auto index3 = indexer.CreateIndex();
    index3.MoveBefore(index1);
    EXPECT_GT(index2, index1);
    EXPECT_LT(index3, index1);
    EXPECT_LT(index3, index2);
}

TEST(IndexerTest, SwitchOrderMultipleIndexer)
{
    toml_parser::CNodeIndexer indexer1;
    toml_parser::CNodeIndexer indexer2;
    toml_parser::CNodeIndexer indexer3;
    auto index1 = indexer1.CreateIndex();
    auto index2 = indexer2.CreateIndex();
    auto index3 = indexer3.CreateIndex();
    EXPECT_GT(index2, index1);
    EXPECT_GT(index3, index1);
    EXPECT_GT(index3, index2);
    index3.MoveBefore(index1);
    EXPECT_GT(index2, index1);
    EXPECT_LT(index3, index1);
    EXPECT_LT(index3, index2);
    index2.MoveBefore(index3);
    EXPECT_LT(index2, index1);
    EXPECT_LT(index3, index1);
    EXPECT_GT(index3, index2);
}

TEST(IndexerTest, IndexLifetime)
{
    toml_parser::CNodeIndexer indexer;
    
    auto index1 = indexer.CreateIndex();
    size_t nCurrentCnt = indexer.Count();
    {
        auto index2 = indexer.CreateIndex();
        EXPECT_GT(index2, index1);
        EXPECT_EQ(indexer.Count(), nCurrentCnt + 1);
    }
    EXPECT_EQ(indexer.Count(), nCurrentCnt);
    auto index3 = indexer.CreateIndex();
    EXPECT_GT(index3, index1);
}

TEST(IndexerTest, CopyIndex)
{
    toml_parser::CNodeIndexer indexer;
    
    auto index1 = indexer.CreateIndex();
    auto index2 = indexer.CreateIndex();
    auto index2b = index2;
    EXPECT_TRUE(index2b);
    EXPECT_TRUE(index2);
    EXPECT_GT(index2b, index1);
    EXPECT_GT(index2, index1);
    EXPECT_EQ(index2, index2b);
    auto index3 = indexer.CreateIndex();
    EXPECT_GT(index3, index2);
    EXPECT_GT(index3, index1);
}

TEST(IndexerTest, MoveIndex)
{
    toml_parser::CNodeIndexer indexer;

    auto index1  = indexer.CreateIndex();
    auto index2  = indexer.CreateIndex();
    auto index2b = std::move(index2);
    EXPECT_TRUE(index2b);
    EXPECT_FALSE(index2);
    EXPECT_GT(index2b, index1);
    EXPECT_GT(index2, index1);  // index 2 is not initialized any more.
    EXPECT_GT(index2, index2b);
    auto index3 = indexer.CreateIndex();
    EXPECT_GT(index3, index2b);
    EXPECT_GT(index3, index1);
    EXPECT_GT(index2, index3);
}
