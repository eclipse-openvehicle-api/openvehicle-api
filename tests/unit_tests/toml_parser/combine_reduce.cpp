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
#include "../../../global/localmemmgr.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"

TEST(CombineReduse, CombineRoot)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineRootComments)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 30.0     # This is value 3
val1 = 10       # This is again value 1)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineRootwithTableValues)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    auto ptrTable = parser2.Root().Direct("MyTable");
    ASSERT_TRUE(ptrTable);
    EXPECT_TRUE(parser1.Root().Combine(ptrTable->Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineRootwithStandardTable)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    auto ptrTable = &parser2.Root();
    ASSERT_TRUE(ptrTable);
    EXPECT_TRUE(parser1.Root().Combine(ptrTable->Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = 10
val2 = "20"
[MyTable]
val3 = 30.0
val1 = 10)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineRootwithInlineTable)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(MyTable = {val3 = 30.0, val1 = 10})toml");

    auto ptrTable = &parser2.Root();
    ASSERT_TRUE(ptrTable);
    EXPECT_TRUE(parser1.Root().Combine(ptrTable->Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = 10
val2 = "20"
MyTable = {val3 = 30.0, val1 = 10})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineTablewithRoot)
{
    toml_parser::CParser parser1(R"toml([MyTable]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(
val3 = 30.0
val1 = 10)toml");

    auto ptrNode = parser1.Root().Direct("MyTable");
    ASSERT_TRUE(ptrNode);
    auto ptrTable = ptrNode->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable);
    EXPECT_TRUE(ptrTable->Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineTablewithDifferentTable)
{
    toml_parser::CParser parser1(R"toml([MyTable1]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml([MyTable2]
val3 = 30.0
val1 = 10)toml");

    auto ptrNode1 = parser1.Root().Direct("MyTable1");
    ASSERT_TRUE(ptrNode1);
    auto ptrTable1 = ptrNode1->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable1);
    auto ptrNode2 = parser2.Root().Direct("MyTable2");
    ASSERT_TRUE(ptrNode2);
    auto ptrTable2 = ptrNode2->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable2);
    EXPECT_TRUE(ptrTable1->Combine(ptrTable2));

    std::string ssCombinedTOML = R"toml([MyTable1]
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineTablewithIdenticalTable)
{
    toml_parser::CParser parser1(R"toml([MyTable]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineInlineTablewithDifferentStandardTable)
{
    toml_parser::CParser parser1(R"toml(MyTable1 = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml([MyTable2]
val3 = 30.0
val1 = 10)toml");

    auto ptrNode1 = parser1.Root().Direct("MyTable1");
    ASSERT_TRUE(ptrNode1);
    auto ptrTable1 = ptrNode1->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable1);
    auto ptrNode2 = parser2.Root().Direct("MyTable2");
    ASSERT_TRUE(ptrNode2);
    auto ptrTable2 = ptrNode2->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable2);
    EXPECT_TRUE(ptrTable1->Combine(ptrTable2));

    std::string ssCombinedTOML = R"toml(MyTable1 = {val1 = 10, val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineInlineTablewithIdenticalStandardTable)
{
    toml_parser::CParser parser1(R"toml(MyTable = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(MyTable = {val1 = 10, val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineStandardTablewithDifferentInlineTable)
{
    toml_parser::CParser parser1(R"toml([MyTable1]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(MyTable2 = {val3 = 30.0, val1 = 10})toml");

    auto ptrNode1 = parser1.Root().Direct("MyTable1");
    ASSERT_TRUE(ptrNode1);
    auto ptrTable1 = ptrNode1->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable1);
    auto ptrNode2 = parser2.Root().Direct("MyTable2");
    ASSERT_TRUE(ptrNode2);
    auto ptrTable2 = ptrNode2->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable2);
    EXPECT_TRUE(ptrTable1->Combine(ptrTable2));

    std::string ssCombinedTOML = R"toml([MyTable1]
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineStandardTablewithIdenticalInlineTable)
{
    toml_parser::CParser parser1(R"toml([MyTable]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(MyTable = {val3 = 30.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineInlineTablewithDifferentInlineTable)
{
    toml_parser::CParser parser1(R"toml(MyTable1 = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml(MyTable2 = {val3 = 30.0, val1 = 10})toml");

    auto ptrNode1 = parser1.Root().Direct("MyTable1");
    ASSERT_TRUE(ptrNode1);
    auto ptrTable1 = ptrNode1->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable1);
    auto ptrNode2 = parser2.Root().Direct("MyTable2");
    ASSERT_TRUE(ptrNode2);
    auto ptrTable2 = ptrNode2->Cast<toml_parser::CNodeCollection>();
    ASSERT_TRUE(ptrTable2);
    EXPECT_TRUE(ptrTable1->Combine(ptrTable2));

    std::string ssCombinedTOML = R"toml(MyTable1 = {val1 = 10, val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineInlineTablewithIdenticalInlineTable)
{
    toml_parser::CParser parser1(R"toml(MyTable = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml(MyTable = {val3 = 30.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(MyTable = {val1 = 10, val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineDifferentArrays)
{
    toml_parser::CParser parser1(R"toml(
val1 = [10, 20]
val2 = ["30", "40"])toml");
    toml_parser::CParser parser2(R"toml(
val3 = [50.0, 60.0]
val1 = [70, 80])toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = [70, 80]
val2 = ["30", "40"]
val3 = [50.0, 60.0])toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, CombineDifferentTableArrays)
{
    toml_parser::CParser parser1(R"toml([[table_array1]]
val1 = [10, 20]
[[table_array1]]
val2 = ["30", "40"])toml");

    toml_parser::CParser parser2(R"toml(
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70, 80])toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([[table_array1]]
val1 = [10, 20]
[[table_array1]]
val2 = ["30", "40"]
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70, 80])toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceRoot)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceRootComments)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 35.0     # This also is value 3
val1 = 10       # This also is value 1)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceTable)
{
    toml_parser::CParser parser1(R"toml([my_table]
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml([my_table]
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([my_table]
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceStandardTableWithInlineTable)
{
    toml_parser::CParser parser1(R"toml([my_table]
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml(my_table = {val3 = 35.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([my_table]
val2 = "20"
val3 = 30.0)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceInlineTableWithStandardTable)
{
    toml_parser::CParser parser1(R"toml(my_table = {val1 = 10, val2 = "20", val3 = 30.0})toml");
    toml_parser::CParser parser2(R"toml([my_table]
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(my_table = { val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceInlineTable)
{
    toml_parser::CParser parser1(R"toml(my_table = {val1 = 10, val2 = "20", val3 = 30.0})toml");
    toml_parser::CParser parser2(R"toml(my_table = {val3 = 35.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(my_table = { val2 = "20", val3 = 30.0})toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceDifferentArrays)
{
    toml_parser::CParser parser1(R"toml(
val1 = [10, 20]
val2 = ["30", "40"]
val3 = [50.0, 60.0])toml");
    toml_parser::CParser parser2(R"toml(
val3 = [50.0, 60.0]
val1 = [70, 80])toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml(
val1 = [10, 20]
val2 = ["30", "40"]
)toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(CombineReduse, ReduceDifferentTableArrays)
{
    toml_parser::CParser parser1(R"toml([[table_array1]]
val1 = [10, 20]
[[table_array1]]
val2 = ["30", "40"]
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70, 80])toml");
    toml_parser::CParser parser2(R"toml(
[[table_array1]]
val2 = ["30", "40"]
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70]
)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"toml([[table_array1]]
val1 = [10, 20]
[[table_array2]]
val1 = [70, 80])toml";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}
