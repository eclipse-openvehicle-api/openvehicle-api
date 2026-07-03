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

TEST(CascadeInsertNode, InsertValueInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());

    // Insert the value into the new table
    EXPECT_TRUE(root.InsertValue("", "standard_table.value_int", 10));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10)toml");

    // Insert the values into the existing table
    EXPECT_TRUE(root.InsertValue("", "standard_table.value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10
value_str = "abc")toml");
    EXPECT_TRUE(root.InsertValue("standard_table.value_int", "standard_table.value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(CascadeInsertNode, InsertTableInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "standard_table", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert a standard table
    EXPECT_TRUE(root.InsertTable("", "standard_table.table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table.table1])toml");

    // Insert an inline table before
    EXPECT_TRUE(root.InsertTable("standard_table.table1", "standard_table.table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
[standard_table.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(root.InsertTable("", "standard_table.table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table relative to the elements of the
    // standard_table.
    EXPECT_TRUE(root.InsertTable("standard_table.table2", "standard_table.table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table4]
[standard_table.table1])toml");
}

TEST(CascadeInsertNode, InsertArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "standard_table", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert arrays
    EXPECT_TRUE(root.InsertArray("", "standard_table.value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = [])toml");
    EXPECT_TRUE(root.InsertArray("", "standard_table.value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = []
value_array2 = [])toml");
    EXPECT_TRUE(root.InsertArray("standard_table.value_array1", "standard_table.value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(CascadeInsertNode, InsertTableArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "standard_table", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert standard table array
    EXPECT_TRUE(root.InsertTableArray("", "standard_table.table_array1",
        false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[standard_table.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(root.InsertTableArray("standard_table.table_array1", "standard_table.table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
[[standard_table.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(root.InsertTableArray("", "standard_table.table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array relative to the elements
    // of the standard_table.
    EXPECT_TRUE(root.InsertTableArray("standard_table.table_array2", "standard_table.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(root.InsertTableArray("", "standard_table.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]]
[[standard_table.table_array4]])toml");
}

TEST(CascadeInsertNode, InsertValueInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "inline_table", true));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert the values into the table
    EXPECT_TRUE(root.InsertValue("", "inline_table.value_int", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10})toml");
    EXPECT_TRUE(root.InsertValue("", "inline_table.value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10, value_str = "abc"})toml");
    EXPECT_TRUE(root.InsertValue("inline_table.value_int", "inline_table.value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_float = 123.456, value_int = 10, value_str = "abc"})toml");
}

TEST(CascadeInsertNode, InsertTableInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "inline_table", true));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert a standard table
    EXPECT_TRUE(root.InsertTable("", "inline_table.table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table1 = {}})toml");

    // Insert an inline table before
    EXPECT_TRUE(root.InsertTable("inline_table.table1", "inline_table.table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}})toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(root.InsertTable("", "inline_table.table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}, table3 = {}})toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_TRUE(root.InsertTable("inline_table.table2", "inline_table.table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table4 = {}, table2 = {}, table1 = {}, table3 = {}})toml");
}

TEST(CascadeInsertNode, InsertArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "inline_table",
        true));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert arrays
    EXPECT_TRUE(root.InsertArray("", "inline_table.value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = []})toml");
    EXPECT_TRUE(root.InsertArray("", "inline_table.value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = [], value_array2 = []})toml");
    EXPECT_TRUE(root.InsertArray("inline_table.value_array1", "inline_table.value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array3 = [], value_array1 = [], value_array2 = []})toml");
}

TEST(CascadeInsertNode, InsertTableArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTable("", "inline_table", true));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert standard table array
    EXPECT_TRUE(root.InsertTableArray("", "inline_table.table_array1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array1 = [{}]})toml");

    // Insert an inline table array before
    EXPECT_TRUE(root.InsertTableArray("inline_table.table_array1", "inline_table.table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}]})toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(root.InsertTableArray("", "inline_table.table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(root.InsertTableArray("inline_table.table_array2", "inline_table.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML,
        R"toml(inline_table = {table_array4 = [{}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(root.InsertTableArray("", "inline_table.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML,
        R"toml(inline_table = {table_array4 = [{}, {}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");
}

TEST(CascadeInsertNode, InsertValueInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert the values into the table (with or without name)
    EXPECT_TRUE(root.InsertValue("", "inline_array", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10])toml");
    EXPECT_TRUE(root.InsertValue("", "inline_array", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10, "abc"])toml");
    EXPECT_TRUE(root.InsertValue("inline_array[0]", "inline_array", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [123.456, 10, "abc"])toml");
}

TEST(CascadeInsertNode, InsertTableInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert a standard table
    EXPECT_TRUE(root.InsertTable("", "inline_array", false));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].a", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{a = 10}])toml");

    // Insert an inline table before
    EXPECT_TRUE(root.InsertTable("inline_array[0]", "inline_array", true));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].b", 20));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}])toml");

    // Insert an inline table behind
    EXPECT_TRUE(root.InsertTable("", "inline_array", true));
    EXPECT_TRUE(root.InsertValue("", "inline_array.c", 30));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}, {c = 30}])toml");

    // Insert a standard table in front
    EXPECT_TRUE(root.InsertTable("inline_array[0]", "inline_array", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].d", 40));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{d = 40}, {b = 20}, {a = 10}, {c = 30}])toml");
}

TEST(CascadeInsertNode, InsertArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert arrays
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0]", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10]])toml");
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    EXPECT_TRUE(root.InsertValue("", "inline_array[1]", 20));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10], [20]])toml");
    EXPECT_TRUE(root.InsertArray("inline_array[0]", "inline_array"));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0]", 30));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[30], [10], [20]])toml");
}

TEST(CascadeInsertNode, InsertTableArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertArray("", "inline_array"));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert standard table array
    EXPECT_TRUE(root.InsertTableArray("", "inline_array", false));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].a", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{a = 10}]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(root.InsertTableArray("inline_array[0]", "inline_array", true));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].b", 20));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(root.InsertTableArray("", "inline_array", true));
    EXPECT_TRUE(root.InsertValue("", "inline_array[999].c", 30));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(root.InsertTableArray("inline_array[0]", "inline_array", false));
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].d", 40));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(root.InsertValue("", "inline_array[0].e", 50));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40, e = 50}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");
}

TEST(CascadeInsertNode, InsertValueInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTableArray("", "table_array", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert the values into the table
    EXPECT_TRUE(root.InsertValue("", "table_array.value_int", 10));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10)toml");
    EXPECT_TRUE(root.InsertValue("", "table_array.value_str", u8"abc"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10
value_str = "abc")toml");
    EXPECT_TRUE(root.InsertValue("table_array[0]", "table_array.value_float", 123.456));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(CascadeInsertNode, InsertTableInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTableArray("", "table_array", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert a standard table
    EXPECT_TRUE(root.InsertTable("", "table_array.table1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[table_array.table1])toml");

    // Insert an inline table before
    EXPECT_TRUE(root.InsertTable("table_array[0]", "table_array.table2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
[table_array.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_TRUE(root.InsertTable("", "table_array.table3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_TRUE(root.InsertTable("table_array[0]", "table_array.table4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table4]
[table_array.table1])toml");
}

TEST(CascadeInsertNode, InsertArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTableArray("", "table_array", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert arrays
    EXPECT_TRUE(root.InsertArray("", "table_array.value_array1"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = [])toml");
    EXPECT_TRUE(root.InsertArray("", "table_array.value_array2"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = []
value_array2 = [])toml");
    EXPECT_TRUE(root.InsertArray("table_array[0]", "table_array.value_array3"));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(CascadeInsertNode, InsertTableArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::CNodeCollection root(&parser.Root());
    EXPECT_TRUE(root.InsertTableArray("", "table_array", false));
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert standard table array
    EXPECT_TRUE(root.InsertTableArray("", "table_array.table_array1", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[[table_array.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_TRUE(root.InsertTableArray("table_array[0]", "table_array.table_array2", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
[[table_array.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_TRUE(root.InsertTableArray("", "table_array.table_array3", true));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_TRUE(root.InsertTableArray("table_array[0]", "table_array.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_TRUE(root.InsertTableArray("", "table_array.table_array4", false));
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]]
[[table_array.table_array4]])toml");
}
