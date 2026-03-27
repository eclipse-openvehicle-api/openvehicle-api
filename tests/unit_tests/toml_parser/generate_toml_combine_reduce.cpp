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

TEST(GenerateTOML, CombineRoot)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineRootComments)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 30.0     # This is value 3
val1 = 10       # This is again value 1)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineRootwithTable)
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

    std::string ssCombinedTOML = R"code(
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineTablewithRoot)
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

    std::string ssCombinedTOML = R"code([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineTablewithDifferentTable)
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

    std::string ssCombinedTOML = R"code([MyTable1]
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineTablewithIdenticalTable)
{
    toml_parser::CParser parser1(R"toml([MyTable]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineInlineTablewithDifferentStandardTable)
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

    std::string ssCombinedTOML = R"code(MyTable1 = {val1 = 10, val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineInlineTablewithIdenticalStandardTable)
{
    toml_parser::CParser parser1(R"toml(MyTable = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml([MyTable]
val3 = 30.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(MyTable = {val1 = 10, val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineStandardTablewithDifferentInlineTable)
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

    std::string ssCombinedTOML = R"code([MyTable1]
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineStandardTablewithIdenticalInlineTable)
{
    toml_parser::CParser parser1(R"toml([MyTable]
val1 = 10
val2 = "20")toml");
    toml_parser::CParser parser2(R"toml(MyTable = {val3 = 30.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code([MyTable]
val1 = 10
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineInlineTablewithDifferentInlineTable)
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

    std::string ssCombinedTOML = R"code(MyTable1 = {val1 = 10, val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineInlineTablewithIdenticalInlineTable)
{
    toml_parser::CParser parser1(R"toml(MyTable = {val1 = 10, val2 = "20"})toml");
    toml_parser::CParser parser2(R"toml(MyTable = {val3 = 30.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(MyTable = {val1 = 10, val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineDifferentArrays)
{
    toml_parser::CParser parser1(R"toml(
val1 = [10, 20]
val2 = ["30", "40"])toml");
    toml_parser::CParser parser2(R"toml(
val3 = [50.0, 60.0]
val1 = [70, 80])toml");

    EXPECT_TRUE(parser1.Root().Combine(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(
val1 = [70, 80]
val2 = ["30", "40"]
val3 = [50.0, 60.0])code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, CombineDifferentTableArrays)
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

    std::string ssCombinedTOML = R"code([[table_array1]]
val1 = [10, 20]
[[table_array1]]
val2 = ["30", "40"]
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70, 80])code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceRoot)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceRootComments)
{
    toml_parser::CParser parser1(R"toml(
val1 = 10       # This is value 1
val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)toml");
    toml_parser::CParser parser2(R"toml(
val3 = 35.0     # This also is value 3
val1 = 10       # This also is value 1)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(
val2 = "20"     # This is value 2
val3 = 30.0     # This is value 3)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceTable)
{
    toml_parser::CParser parser1(R"toml([my_table]
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml([my_table]
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code([my_table]
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceStandardTableWithInlineTable)
{
    toml_parser::CParser parser1(R"toml([my_table]
val1 = 10
val2 = "20"
val3 = 30.0)toml");
    toml_parser::CParser parser2(R"toml(my_table = {val3 = 35.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code([my_table]
val2 = "20"
val3 = 30.0)code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceInlineTableWithStandardTable)
{
    toml_parser::CParser parser1(R"toml(my_table = {val1 = 10, val2 = "20", val3 = 30.0})toml");
    toml_parser::CParser parser2(R"toml([my_table]
val3 = 35.0
val1 = 10)toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(my_table = { val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceInlineTable)
{
    toml_parser::CParser parser1(R"toml(my_table = {val1 = 10, val2 = "20", val3 = 30.0})toml");
    toml_parser::CParser parser2(R"toml(my_table = {val3 = 35.0, val1 = 10})toml");

    EXPECT_TRUE(parser1.Root().Reduce(parser2.Root().Cast<toml_parser::CNodeCollection>()));

    std::string ssCombinedTOML = R"code(my_table = { val2 = "20", val3 = 30.0})code";

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser1.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssCombinedTOML);
}

TEST(GenerateTOML, ReduceDifferentArrays)
{
    toml_parser::CParser parser1(R"code(
val1 = [10, 20]
val2 = ["30", "40"]
val3 = [50.0, 60.0])code");
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

TEST(GenerateTOML, ReduceDifferentTableArrays)
{
    toml_parser::CParser parser1(R"code([[table_array1]]
val1 = [10, 20]
[[table_array1]]
val2 = ["30", "40"]
[[table_array2]]
val3 = [50.0, 60.0]
[[table_array2]]
val1 = [70, 80])code");
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
