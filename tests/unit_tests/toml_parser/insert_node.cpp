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
#include <support/toml.h>

// Test TODO:
//  - Inserted and straight away deleted
//  - Inserted with false/deleted reference --error
//  - Inserted values before (okay) and behind (error) tables
//  - Inserted duplicate value -- error
//  - Smart insert (comments/whitespace around)
// Insert as TOML, but only partly correct.

TEST(InsertNode, InsertValuesRoot)
{
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertValue("", "value_int", 10));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_int = 10)toml");
    EXPECT_TRUE(root.InsertValue("", "value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_int = 10
value_str = "abc")toml");
    EXPECT_NE(parser.Root().InsertValue(root.GetNodeNameByIndex(0), "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(InsertNode, InsertTableRoot)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "table1", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([table1])toml");

    // Insert an inline table before
    EXPECT_TRUE(root.InsertTable(root.GetNodeNameByIndex(0), "table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
[table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(root.InsertTable("", "table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
table3 = {}
[table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_TRUE(root.InsertTable(root.GetNodeNameByIndex(0), "table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
table3 = {}
[table4]
[table1])toml");
}

TEST(InsertNode, InsertArrayRoot)
{
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertArray("", "value_array1"));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = [])toml");
    EXPECT_TRUE(root.InsertArray("", "value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = []
value_array2 = [])toml");
    EXPECT_TRUE(root.InsertArray(root.GetNodeNameByIndex(0), "value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(InsertNode, InsertTableArrayRoot)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTableArray("", "table_array1", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array1]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(root.InsertTableArray(root.GetNodeNameByIndex(0), "table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
[[table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(root.InsertTableArray("", "table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(root.InsertTableArray(root.GetNodeNameByIndex(0), "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array4]]
[[table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(root.InsertTableArray("", "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array4]]
[[table_array1]]
[[table_array4]])toml");
}

TEST(InsertNode, InsertValueInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableStandard = root.InsertTable("", "standard_table", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert the values into the table
    EXPECT_TRUE(tableStandard.InsertValue("", "value_int", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10)toml");
    EXPECT_TRUE(tableStandard.InsertValue("", "value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10
value_str = "abc")toml");
    EXPECT_TRUE(tableStandard.InsertValue(tableStandard.GetNodeNameByIndex(0), "value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(InsertNode, InsertTableInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableStandard = root.InsertTable("", "standard_table", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert a standard table
    EXPECT_TRUE(tableStandard.InsertTable("", "table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table.table1])toml");

    // Insert an inline table before
    EXPECT_TRUE(tableStandard.InsertTable(tableStandard.GetNodeNameByIndex(0), "table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
[standard_table.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(tableStandard.InsertTable("", "table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table relative to the elements of the
    // standard_table.
    EXPECT_TRUE(tableStandard.InsertTable(tableStandard.GetNodeNameByIndex(0), "table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table4]
[standard_table.table1])toml");
}

TEST(InsertNode, InsertArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableStandard = root.InsertTable("", "standard_table", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert arrays
    EXPECT_TRUE(tableStandard.InsertArray("", "value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = [])toml");
    EXPECT_TRUE(tableStandard.InsertArray("", "value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = []
value_array2 = [])toml");
    EXPECT_TRUE(tableStandard.InsertArray(tableStandard.GetNodeNameByIndex(0), "value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(InsertNode, InsertTableArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableStandard = root.InsertTable("", "standard_table", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert standard table array
    EXPECT_TRUE(tableStandard.InsertTableArray("", "table_array1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[standard_table.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(tableStandard.InsertTableArray(tableStandard.GetNodeNameByIndex(0), "table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
[[standard_table.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(tableStandard.InsertTableArray("", "table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array relative to the elements
    // of the standard_table.
    EXPECT_TRUE(tableStandard.InsertTableArray(tableStandard.GetNodeNameByIndex(0), "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(tableStandard.InsertTableArray("", "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]]
[[standard_table.table_array4]])toml");
}

TEST(InsertNode, InsertValueInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableInline = root.InsertTable("", "inline_table", true);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert the values into the table
    EXPECT_TRUE(tableInline.InsertValue("", "value_int", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10})toml");
    EXPECT_TRUE(tableInline.InsertValue("", "value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10, value_str = "abc"})toml");
    EXPECT_TRUE(tableInline.InsertValue(tableInline.GetNodeNameByIndex(0), "value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_float = 123.456, value_int = 10, value_str = "abc"})toml");
}

TEST(InsertNode, InsertTableInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableInline = root.InsertTable("", "inline_table", true);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert a standard table
    EXPECT_TRUE(tableInline.InsertTable("", "table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table1 = {}})toml");

    // Insert an inline table before
    EXPECT_TRUE(tableInline.InsertTable(tableInline.GetNodeNameByIndex(0), "table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}})toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(tableInline.InsertTable("", "table3",
        true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}, table3 = {}})toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_TRUE(tableInline.InsertTable(tableInline.GetNodeNameByIndex(0), "table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table4 = {}, table2 = {}, table1 = {}, table3 = {}})toml");
}

TEST(InsertNode, InsertArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableInline = root.InsertTable("", "inline_table", true);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert arrays
    EXPECT_TRUE(tableInline.InsertArray("", "value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = []})toml");
    EXPECT_TRUE(tableInline.InsertArray("", "value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = [], value_array2 = []})toml");
    EXPECT_TRUE(tableInline.InsertArray(tableInline.GetNodeNameByIndex(0), "value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array3 = [], value_array1 = [], value_array2 = []})toml");
}

TEST(InsertNode, InsertTableArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection tableInline = root.InsertTable("", "inline_table", true);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert standard table array
    EXPECT_TRUE(tableInline.InsertTableArray("", "table_array1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array1 = [{}]})toml");

    // Insert an inline table array before
    EXPECT_TRUE(tableInline.InsertTableArray(tableInline.GetNodeNameByIndex(0), "table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}]})toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(tableInline.InsertTableArray("", "table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(tableInline.InsertTableArray(tableInline.GetNodeNameByIndex(0), "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML,
        R"toml(inline_table = {table_array4 = [{}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(tableInline.InsertTableArray("", "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML,
        R"toml(inline_table = {table_array4 = [{}, {}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");
}

TEST(InsertNode, InsertValueInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection arrayInline = root.InsertArray("", "inline_array");
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert the values into the table (with or without name)
    EXPECT_TRUE(arrayInline.InsertValue("", "", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10])toml");
    EXPECT_TRUE(arrayInline.InsertValue("", "", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10, "abc"])toml");
    EXPECT_TRUE(arrayInline.InsertValue(arrayInline.GetNodeNameByIndex(0), "", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [123.456, 10, "abc"])toml");
}

TEST(InsertNode, InsertTableInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection arrayInline = root.InsertArray("", "inline_array");
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert a standard table
    sdv::toml::CNodeCollection table = arrayInline.InsertTable("", "", false);
    EXPECT_TRUE(table);
    table.InsertValue("", "a", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{a = 10}])toml");

    // Insert an inline table before
    table = arrayInline.InsertTable(arrayInline.GetNodeNameByIndex(0), "", true);
    EXPECT_TRUE(table);
    table.InsertValue("", "b", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}])toml");

    // Insert an inline table behind
    table = arrayInline.InsertTable("", "", true);
    EXPECT_TRUE(table);
    table.InsertValue("", "c", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}, {c = 30}])toml");

    // Insert a standard table in front
    table = arrayInline.InsertTable(arrayInline.GetNodeNameByIndex(0), "", false);
    EXPECT_TRUE(table);
    table.InsertValue("", "d", 40);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{d = 40}, {b = 20}, {a = 10}, {c = 30}])toml");
}

TEST(InsertNode, InsertArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection arrayInline = root.InsertArray("", "inline_array");
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert arrays
    sdv::toml::CNodeCollection array = arrayInline.InsertArray("", "");
    EXPECT_TRUE(array);
    array.InsertValue("", "", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10]])toml");
    array = arrayInline.InsertArray("", "");
    EXPECT_TRUE(array);
    array.InsertValue("", "", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10], [20]])toml");
    array = arrayInline.InsertArray(arrayInline.GetNodeNameByIndex(0), "");
    EXPECT_TRUE(array);
    array.InsertValue("", "", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[30], [10], [20]])toml");
}

TEST(InsertNode, InsertTableArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection arrayInline = root.InsertArray("", "inline_array");
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert standard table array
    sdv::toml::CNodeCollection table = arrayInline.InsertTableArray("", "", false);
    EXPECT_TRUE(table);
    table.InsertValue("", "a", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{a = 10}]])toml");

    // Insert an inline table array before
    table = arrayInline.InsertTableArray(arrayInline.GetNodeNameByIndex(0), "", true);
    EXPECT_TRUE(table);
    table.InsertValue("", "b", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    table = arrayInline.InsertTableArray("", "", true);
    EXPECT_TRUE(table);
    table.InsertValue("", "c", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    table = arrayInline.InsertTableArray(arrayInline.GetNodeNameByIndex(0), "", false);
    EXPECT_TRUE(table);
    table.InsertValue("", "d", 40);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Add an additional table array entry for table array #4
    sdv::toml::CNodeCollection array = arrayInline.Get(0);
    EXPECT_TRUE(array);
    table = array.InsertTable("", "", false);
    EXPECT_TRUE(table);
    table.InsertValue("", "e", 50);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40}, {e = 50}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");
}

TEST(InsertNode, InsertValueInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection table = root.InsertTableArray("", "table_array", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert the values into the table
    EXPECT_TRUE(table.InsertValue("", "value_int", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10)toml");
    EXPECT_TRUE(table.InsertValue("", "value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10
value_str = "abc")toml");
    EXPECT_TRUE(table.InsertValue(table.GetNodeNameByIndex(0), "value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(InsertNode, InsertTableInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection table = root.InsertTableArray("", "table_array", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert a standard table
    EXPECT_TRUE(table.InsertTable("", "table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[table_array.table1])toml");

    // Insert an inline table before
    EXPECT_TRUE(table.InsertTable(table.GetNodeNameByIndex(0), "table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
[table_array.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(table.InsertTable("", "table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_TRUE(table.InsertTable(table.GetNodeNameByIndex(0), "table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table4]
[table_array.table1])toml");
}

TEST(InsertNode, InsertArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection table = root.InsertTableArray("", "table_array", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert arrays
    EXPECT_TRUE(table.InsertArray("", "value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = [])toml");
    EXPECT_TRUE(table.InsertArray("", "value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = []
value_array2 = [])toml");
    EXPECT_TRUE(table.InsertArray(table.GetNodeNameByIndex(0), "value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(InsertNode, InsertTableArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    sdv::toml::CNodeCollection table = root.InsertTableArray("", "table_array", false);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert standard table array
    EXPECT_TRUE(table.InsertTableArray("", "table_array1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[[table_array.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(table.InsertTableArray(table.GetNodeNameByIndex(0), "table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
[[table_array.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(table.InsertTableArray("", "table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(table.InsertTableArray(table.GetNodeNameByIndex(0), "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(table.InsertTableArray("", "table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]]
[[table_array.table_array4]])toml");
}

TEST(InsertNode, InsertValuesRootAsTOML)
{
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_EQ(root.InsertTOML("", R"toml(# This is the first value
value_int = 10)toml", true), 1);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is the first value
value_int = 10)toml");
    EXPECT_EQ(root.InsertTOML("", R"toml(value_str = "abc"         # This is the second value

# Comment in between

# And the third value
value_float = 123.456

# Some
# Final
# Words :-)
)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is the first value
value_int = 10
value_str = "abc"         # This is the second value

# Comment in between

# And the third value
value_float = 123.456

# Some
# Final
# Words :-)
)toml");
}

TEST(InsertNode, InsertTableRootAsTOML)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_EQ(root.InsertTOML("", R"toml([table1]     # This is table 1
a = 10
b = 20.30)toml", true), 1);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert an inline table before
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml(# This is table 2
table2 = {c = 40, d = "50"})toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is table 2
table2 = {c = 40, d = "50"}
[table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_EQ(root.InsertTOML("", R"toml(table3 = {e = '60'} # This is table 3
)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is table 2
table2 = {c = 40, d = "50"}
table3 = {e = '60'} # This is table 3
[table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml(

# And this is table 4
[table4]
f = 70
g = 80.90)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is table 2
table2 = {c = 40, d = "50"}
table3 = {e = '60'} # This is table 3


# And this is table 4
[table4]
f = 70
g = 80.90
[table1]     # This is table 1
a = 10
b = 20.30)toml");
}

TEST(InsertNode, InsertArrayRootAsTOML)
{
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_EQ(root.InsertTOML("", R"toml(value_array1 = [10, 20, 30] # This is array 1)toml", true),
        1);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = [10, 20, 30] # This is array 1)toml");
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml(# This is array 2
value_array2 = [
    40,         # This is an integer value
    50.60,      # This is a float value
    "70",       # This is a string value
               ])toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is array 2
value_array2 = [
    40,         # This is an integer value
    50.60,      # This is a float value
    "70",       # This is a string value
               ]
value_array1 = [10, 20, 30] # This is array 1)toml");
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(1), R"toml(# And finally array 3
value_array3 = [])toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is array 2
value_array2 = [
    40,         # This is an integer value
    50.60,      # This is a float value
    "70",       # This is a string value
               ]
# And finally array 3
value_array3 = []
value_array1 = [10, 20, 30] # This is array 1)toml");
}

TEST(InsertNode, InsertTableArrayRootAsTOML)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_EQ(root.InsertTOML("", R"toml([[table_array1]]
a = 10
b = 20)toml", true), 1);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array1]]
a = 10
b = 20)toml");

    // Insert an inline table array before
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml(table_array2 = [{c = 30}, {d = 40}])toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
[[table_array1]]
a = 10
b = 20)toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_EQ(root.InsertTOML("", R"toml(table_array3 =
[
    { e = 50 }
])toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
table_array3 =
[
    { e = 50 }
]
[[table_array1]]
a = 10
b = 20)toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml([[table_array4]]
f = 60
g = 70)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
table_array3 =
[
    { e = 50 }
]
[[table_array4]]
f = 60
g = 70
[[table_array1]]
a = 10
b = 20)toml");

    // Add an additional table array entry for table array #4 (even provided as inline, must be added as standard).
    EXPECT_EQ(root.InsertTOML("", R"toml(table_array4 = [{h = 80}])toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
table_array3 =
[
    { e = 50 }
]
[[table_array4]]
f = 60
g = 70
[[table_array1]]
a = 10
b = 20
[[table_array4]]
h = 80)toml");

    // Add an additional table array entry for table array #4 (even provided as inline, must be added as standard).
    EXPECT_EQ(root.InsertTOML("", R"toml([[table_array3]]
i = 90
[[table_array3]]
j = 100
)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(root.InsertTOML(root.GetNodeNameByIndex(0), R"toml([[table_array3]]
k = 110)toml", true), 1);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
table_array3 =
[{k = 110}, 
    { e = 50 },
{i = 90}, {j = 100}]
[[table_array4]]
f = 60
g = 70
[[table_array1]]
a = 10
b = 20
[[table_array4]]
h = 80)toml");

    root.AutomaticFormat(true);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
table_array3 = [{k = 110}, {e = 50}, {i = 90}, {j = 100}]
[[table_array4]]
f = 60
g = 70
[[table_array1]]
a = 10
b = 20
[[table_array4]]
h = 80)toml");
}

TEST(InsertNode, DISABLED_TestInsertValueInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(root.InsertTable(
    //    sdv::toml::npos, "standard_table", false)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(InsertNode, DISABLED_TestInsertTableInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(root.InsertTable(
    //    sdv::toml::npos, "standard_table", false)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(InsertNode, DISABLED_TestInsertArrayInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(root.InsertTable(
    //    sdv::toml::npos, "standard_table", false)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(InsertNode, DISABLED_TestInsertTableArrayInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(root.InsertTable(
    //    sdv::toml::npos, "standard_table", false)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(InsertNode, DISABLED_TestInsertValueInInlineTableAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableInInlineTableAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertArrayInInlineTableAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableArrayInInlineTableAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertValueInArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableInArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertArrayInArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableArrayInArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertValueInTableArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableInTableArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertArrayInTableArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertTableArrayInTableArrayAsTOML)
{}

TEST(InsertNode, DISABLED_TestInsertMixed)
{}

TEST(InsertNode, DISABLED_TestInsertMixedWithDelete)
{}
