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
//  - Inserted and straight away deleted
//  - Inserted with false/deleted reference --error
//  - Inserted values before (okay) and behind (error) tables
//  - Inserted duplicate value -- error
//  - Smart insert (comments/whitespace around)
// Insert as TOML, but only partly correct.

TEST(TOMLInsertNode, TestInsertValuesRoot)
{
    toml_parser::CParser parser;
    EXPECT_NE(parser.Root().InsertValue(sdv::toml::npos, "value_int", 10), nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_int = 10)toml");
    EXPECT_NE(parser.Root().InsertValue(sdv::toml::npos, "value_str", u8"abc"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_int = 10
value_str = "abc")toml");
    EXPECT_NE(parser.Root().InsertValue(0, "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(TOMLInsertNode, TestInsertTableRoot)
{
    // Insert a standard table
    toml_parser::CParser parser;
    EXPECT_NE(parser.Root().InsertTable(sdv::toml::npos, "table1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([table1])toml");

    // Insert an inline table before
    EXPECT_NE(parser.Root().InsertTable(0, "table2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
[table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_NE(parser.Root().InsertTable(sdv::toml::npos, "table3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
table3 = {}
[table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_NE(parser.Root().InsertTable(0, "table4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table2 = {}
table3 = {}
[table4]
[table1])toml");
}

TEST(TOMLInsertNode, TestInsertArrayRoot)
{
    toml_parser::CParser parser;
    EXPECT_NE(parser.Root().InsertArray(sdv::toml::npos, "value_array1"), nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = [])toml");
    EXPECT_NE(parser.Root().InsertArray(sdv::toml::npos, "value_array2"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = []
value_array2 = [])toml");
    EXPECT_NE(parser.Root().InsertArray(0, "value_array3"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(TOMLInsertNode, TestInsertTableArrayRoot)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    EXPECT_NE(parser.Root().InsertTableArray(sdv::toml::npos, "table_array1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array1]])toml");

    // Insert an inline table array before
    EXPECT_NE(parser.Root().InsertTableArray(0, "table_array2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
[[table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_NE(parser.Root().InsertTableArray(sdv::toml::npos, "table_array3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_NE(parser.Root().InsertTableArray(0, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array4]]
[[table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_NE(parser.Root().InsertTableArray(sdv::toml::npos, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{}]
table_array3 = [{}]
[[table_array4]]
[[table_array1]]
[[table_array4]])toml");
}

TEST(TOMLInsertNode, TestInsertValueInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pStandardTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert the values into the table
    EXPECT_NE(pStandardTable->InsertValue(sdv::toml::npos, "value_int", 10), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10)toml");
    EXPECT_NE(pStandardTable->InsertValue(sdv::toml::npos, "value_str", u8"abc"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_int = 10
value_str = "abc")toml");
    EXPECT_NE(pStandardTable->InsertValue(0, "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(TOMLInsertNode, TestInsertTableInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pStandardTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert a standard table
    EXPECT_NE(pStandardTable->InsertTable(sdv::toml::npos, "table1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table.table1])toml");

    // Insert an inline table before
    EXPECT_NE(pStandardTable->InsertTable(0, "table2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
[standard_table.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_NE(pStandardTable->InsertTable(sdv::toml::npos, "table3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_NE(pStandardTable->InsertTable(0, "table4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table2 = {}
table3 = {}
[standard_table.table4]
[standard_table.table1])toml");
}

TEST(TOMLInsertNode, TestInsertArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pStandardTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert arrays
    EXPECT_NE(pStandardTable->InsertArray(sdv::toml::npos, "value_array1"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = [])toml");
    EXPECT_NE(pStandardTable->InsertArray(sdv::toml::npos, "value_array2"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array1 = []
value_array2 = [])toml");
    EXPECT_NE(pStandardTable->InsertArray(0, "value_array3"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(TOMLInsertNode, TestInsertTableArrayInStandardTable)
{
    // Insert a standard table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pStandardTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table])toml");

    // Insert standard table array
    EXPECT_NE(pStandardTable->InsertTableArray(sdv::toml::npos, "table_array1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[standard_table.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_NE(pStandardTable->InsertTableArray(0, "table_array2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
[[standard_table.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_NE(pStandardTable->InsertTableArray(sdv::toml::npos, "table_array3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_NE(pStandardTable->InsertTableArray(0, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_NE(pStandardTable->InsertTableArray(sdv::toml::npos, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([standard_table]
table_array2 = [{}]
table_array3 = [{}]
[[standard_table.table_array4]]
[[standard_table.table_array1]]
[[standard_table.table_array4]])toml");
}

TEST(TOMLInsertNode, TestInsertValueInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "inline_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert the values into the table
    EXPECT_NE(pInlineTable->InsertValue(sdv::toml::npos, "value_int", 10), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10})toml");
    EXPECT_NE(pInlineTable->InsertValue(sdv::toml::npos, "value_str", u8"abc"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_int = 10, value_str = "abc"})toml");
    EXPECT_NE(pInlineTable->InsertValue(0, "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_float = 123.456, value_int = 10, value_str = "abc"})toml");
}

TEST(TOMLInsertNode, TestInsertTableInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "inline_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert a standard table
    EXPECT_NE(pInlineTable->InsertTable(sdv::toml::npos, "table1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table1 = {}})toml");

    // Insert an inline table before
    EXPECT_NE(pInlineTable->InsertTable(0, "table2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}})toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_NE(pInlineTable->InsertTable(sdv::toml::npos, "table3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table2 = {}, table1 = {}, table3 = {}})toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_NE(pInlineTable->InsertTable(0, "table4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table4 = {}, table2 = {}, table1 = {}, table3 = {}})toml");
}

TEST(TOMLInsertNode, TestInsertArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "inline_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert arrays
    EXPECT_NE(pInlineTable->InsertArray(sdv::toml::npos, "value_array1"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = []})toml");
    EXPECT_NE(pInlineTable->InsertArray(sdv::toml::npos, "value_array2"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array1 = [], value_array2 = []})toml");
    EXPECT_NE(pInlineTable->InsertArray(0, "value_array3"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {value_array3 = [], value_array1 = [], value_array2 = []})toml");
}

TEST(TOMLInsertNode, TestInsertTableArrayInInlineTable)
{
    // Insert an inline table
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
        sdv::toml::npos, "inline_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineTable, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {})toml");

    // Insert standard table array
    EXPECT_NE(pInlineTable->InsertTableArray(sdv::toml::npos, "table_array1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array1 = [{}]})toml");

    // Insert an inline table array before
    EXPECT_NE(pInlineTable->InsertTableArray(0, "table_array2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}]})toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_NE(pInlineTable->InsertTableArray(sdv::toml::npos, "table_array3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_NE(pInlineTable->InsertTableArray(0, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array4 = [{}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");

    // Add an additional table array entry for table array #4
    EXPECT_NE(pInlineTable->InsertTableArray(sdv::toml::npos, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_table = {table_array4 = [{}, {}], table_array2 = [{}], table_array1 = [{}], table_array3 = [{}]})toml");
}

TEST(TOMLInsertNode, TestInsertValueInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineArray = sdv::TInterfaceAccessPtr(parser.Root().InsertArray(
        sdv::toml::npos, "inline_array")).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert the values into the table (with or without name)
    EXPECT_NE(pInlineArray->InsertValue(sdv::toml::npos, "", 10), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10])toml");
    EXPECT_NE(pInlineArray->InsertValue(sdv::toml::npos, "", u8"abc"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [10, "abc"])toml");
    EXPECT_NE(pInlineArray->InsertValue(0, "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [123.456, 10, "abc"])toml");
}

TEST(TOMLInsertNode, TestInsertTableInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineArray = sdv::TInterfaceAccessPtr(parser.Root().InsertArray(
        sdv::toml::npos, "inline_array")).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert a standard table
    sdv::toml::INodeCollectionInsert* pTableInsert = sdv::TInterfaceAccessPtr(
        pInlineArray->InsertTable(sdv::toml::npos, "", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableInsert, nullptr);
    pTableInsert->InsertValue(0, "a", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{a = 10}])toml");

    // Insert an inline table before
    pTableInsert = sdv::TInterfaceAccessPtr(
        pInlineArray->InsertTable(0, "", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableInsert, nullptr);
    pTableInsert->InsertValue(0, "b", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}])toml");

    // Insert an inline table behind
    pTableInsert = sdv::TInterfaceAccessPtr(
        pInlineArray->InsertTable(sdv::toml::npos, "", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableInsert, nullptr);
    pTableInsert->InsertValue(0, "c", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{b = 20}, {a = 10}, {c = 30}])toml");

    // Insert a standard table in front
    pTableInsert = sdv::TInterfaceAccessPtr(
        pInlineArray->InsertTable(0, "", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableInsert, nullptr);
    pTableInsert->InsertValue(0, "d", 40);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [{d = 40}, {b = 20}, {a = 10}, {c = 30}])toml");
}

TEST(TOMLInsertNode, TestInsertArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pInlineArray = sdv::TInterfaceAccessPtr(parser.Root().InsertArray(
        sdv::toml::npos, "inline_array")).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

    // Insert arrays
    sdv::toml::INodeCollectionInsert* pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertArray(sdv::toml::npos, "")).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10]])toml");
    pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertArray(sdv::toml::npos, "value_array2")).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[10], [20]])toml");
    pArrayInsert =
        sdv::TInterfaceAccessPtr(pInlineArray->InsertArray(0, "value_array3")).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[30], [10], [20]])toml");
}

TEST(TOMLInsertNode, TestInsertTableArrayInArray)
{
    // Insert an array
    toml_parser::CParser parser;
    auto* pArray = parser.Root().InsertArray(sdv::toml::npos, "inline_array");
    ASSERT_NE(pArray, nullptr);
    sdv::toml::INodeCollectionInsert* pInlineArray =
        sdv::TInterfaceAccessPtr(pArray).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pInlineArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [])toml");

        // Insert standard table array
    sdv::toml::INodeCollectionInsert* pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertTableArray(sdv::toml::npos,
        "table_array1", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "a", 10);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{a = 10}]])toml");

    // Insert an inline table array before
    pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertTableArray(0, "table_array2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "b", 20);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertTableArray(sdv::toml::npos, "table_array3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "c", 30);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    pArrayInsert = sdv::TInterfaceAccessPtr(pInlineArray->InsertTableArray(0, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pArrayInsert, nullptr);
    pArrayInsert->InsertValue(0, "d", 40);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");

    // Add an additional table array entry for table array #4
    sdv::toml::INodeCollection* pCollection = sdv::TInterfaceAccessPtr(pArray).GetInterface<sdv::toml::INodeCollection>();
    ASSERT_NE(pCollection, nullptr);
    sdv::toml::INodeCollectionInsert* pFirstNode =
        sdv::TInterfaceAccessPtr(pCollection->GetNode(0)).GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pFirstNode, nullptr);
    sdv::toml::INodeCollectionInsert* pTableInsert = sdv::TInterfaceAccessPtr(pFirstNode->InsertTable(sdv::toml::npos,
        "table_array4", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableInsert, nullptr);
    pTableInsert->InsertValue(0, "e", 50);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(inline_array = [[{d = 40}, {e = 50}], [{b = 20}], [{a = 10}], [{c = 30}]])toml");
}

TEST(TOMLInsertNode, TestInsertValueInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pTableArray = sdv::TInterfaceAccessPtr(parser.Root().InsertTableArray(
        sdv::toml::npos, "table_array", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert the values into the table
    EXPECT_NE(pTableArray->InsertValue(sdv::toml::npos, "value_int", 10), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10)toml");
    EXPECT_NE(pTableArray->InsertValue(sdv::toml::npos, "value_str", u8"abc"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_int = 10
value_str = "abc")toml");
    EXPECT_NE(pTableArray->InsertValue(0, "value_float", 123.456), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_float = 123.456
value_int = 10
value_str = "abc")toml");
}

TEST(TOMLInsertNode, TestInsertTableInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pTableArray = sdv::TInterfaceAccessPtr(parser.Root().InsertTableArray(
        sdv::toml::npos, "table_array", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert a standard table
    EXPECT_NE(pTableArray->InsertTable(sdv::toml::npos, "table1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[table_array.table1])toml");

    // Insert an inline table before
    EXPECT_NE(pTableArray->InsertTable(0, "table2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
[table_array.table1])toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_NE(pTableArray->InsertTable(sdv::toml::npos, "table3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table1])toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_NE(pTableArray->InsertTable(0, "table4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table2 = {}
table3 = {}
[table_array.table4]
[table_array.table1])toml");
}

TEST(TOMLInsertNode, TestInsertArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pTableArray = sdv::TInterfaceAccessPtr(parser.Root().InsertTableArray(
        sdv::toml::npos, "table_array", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert arrays
    EXPECT_NE(pTableArray->InsertArray(sdv::toml::npos, "value_array1"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = [])toml");
    EXPECT_NE(pTableArray->InsertArray(sdv::toml::npos, "value_array2"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array1 = []
value_array2 = [])toml");
    EXPECT_NE(pTableArray->InsertArray(0, "value_array3"), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
value_array3 = []
value_array1 = []
value_array2 = [])toml");
}

TEST(TOMLInsertNode, TestInsertTableArrayInTableArray)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    sdv::toml::INodeCollectionInsert* pTableArray = sdv::TInterfaceAccessPtr(parser.Root().InsertTableArray(
        sdv::toml::npos, "table_array", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
            GetInterface<sdv::toml::INodeCollectionInsert>();
    ASSERT_NE(pTableArray, nullptr);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]])toml");

    // Insert standard table array
    EXPECT_NE(pTableArray->InsertTableArray(sdv::toml::npos, "table_array1",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
[[table_array.table_array1]])toml");

    // Insert an inline table array before
    EXPECT_NE(pTableArray->InsertTableArray(0, "table_array2",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
[[table_array.table_array1]])toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_NE(pTableArray->InsertTableArray(sdv::toml::npos, "table_array3",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array1]])toml");

    // Insert a standard table array in front -> this will have to be printed behind the inline table array
    EXPECT_NE(pTableArray->InsertTableArray(0, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]])toml");

    // Add an additional table array entry for table array #4
    EXPECT_NE(pTableArray->InsertTableArray(sdv::toml::npos, "table_array4",
        sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard), nullptr);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array]]
table_array2 = [{}]
table_array3 = [{}]
[[table_array.table_array4]]
[[table_array.table_array1]]
[[table_array.table_array4]])toml");
}

TEST(TOMLInsertNode, TestInsertValuesRootAsTOML)
{
    toml_parser::CParser parser;
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(# This is the first value
value_int = 10)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is the first value
value_int = 10)toml");
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(value_str = "abc"         # This is the second value

# Comment in between

# And the third value
value_float = 123.456

# Some
# Final
# Words :-)
)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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

TEST(TOMLInsertNode, TestInsertTableRootAsTOML)
{
    // Insert a standard table
    toml_parser::CParser parser;
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml([table1]     # This is table 1
a = 10
b = 20.30)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert an inline table before
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml(# This is table 2
table2 = {c = 40, d = "50"})toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is table 2
table2 = {c = 40, d = "50"}
[table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert an inline table behind -> this will have to be printed before the standard table
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(table3 = {e = '60'} # This is table 3
)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is table 2
table2 = {c = 40, d = "50"}
table3 = {e = '60'} # This is table 3
[table1]     # This is table 1
a = 10
b = 20.30)toml");

    // Insert a standard table in front -> this will have to be printed behind the inline table
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml(

# And this is table 4
[table4]
f = 70
g = 80.90)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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

TEST(TOMLInsertNode, TestInsertArrayRootAsTOML)
{
    toml_parser::CParser parser;
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(value_array1 = [10, 20, 30] # This is array 1)toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(value_array1 = [10, 20, 30] # This is array 1)toml");
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml(# This is array 2
value_array2 = [
    40,         # This is an integer value
    50.60,      # This is a float value
    "70",       # This is a string value
               ])toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(# This is array 2
value_array2 = [
    40,         # This is an integer value
    50.60,      # This is a float value
    "70",       # This is a string value
               ]
value_array1 = [10, 20, 30] # This is array 1)toml");
    EXPECT_EQ(parser.Root().InsertTOML(1, R"toml(# And finally array 3
value_array3 = [])toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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

TEST(TOMLInsertNode, TestInsertTableArrayRootAsTOML)
{
    // Insert a standard table array
    toml_parser::CParser parser;
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml([[table_array1]]
a = 10
b = 20)toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml([[table_array1]]
a = 10
b = 20)toml");

    // Insert an inline table array before
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml(table_array2 = [{c = 30}, {d = 40}])toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, R"toml(table_array2 = [{c = 30}, {d = 40}]
[[table_array1]]
a = 10
b = 20)toml");

    // Insert an inline table array behind -> this will have to be printed before the standard table array
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(table_array3 =
[
    { e = 50 }
])toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml([[table_array4]]
f = 60
g = 70)toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml(table_array4 = [{h = 80}])toml", true),
        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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
    EXPECT_EQ(parser.Root().InsertTOML(sdv::toml::npos, R"toml([[table_array3]]
i = 90
[[table_array3]]
j = 100
)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
    EXPECT_EQ(parser.Root().InsertTOML(0, R"toml([[table_array3]]
k = 110)toml", true), sdv::toml::INodeCollectionInsert::EInsertResult::insert_success);
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

    parser.Root().AutomaticFormat();
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

TEST(TOMLInsertNode, DISABLED_TestInsertValueInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
    //    sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(TOMLInsertNode, DISABLED_TestInsertTableInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
    //    sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(TOMLInsertNode, DISABLED_TestInsertArrayInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
    //    sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(TOMLInsertNode, DISABLED_TestInsertTableArrayInStandardTableAsTOML)
{
    //// Insert a standard table
    //toml_parser::CParser parser;
    //sdv::toml::INodeCollectionInsert* pStandardTable = sdv::TInterfaceAccessPtr(parser.Root().InsertTable(
    //    sdv::toml::npos, "standard_table", sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_standard)).
    //        GetInterface<sdv::toml::INodeCollectionInsert>();
    //ASSERT_NE(pStandardTable, nullptr);
    //std::string ssTOML = parser.GenerateTOML();
    //EXPECT_EQ(ssTOML, R"toml([standard_table])toml");
}

TEST(TOMLInsertNode, DISABLED_TestInsertValueInInlineTableAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableInInlineTableAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertArrayInInlineTableAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableArrayInInlineTableAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertValueInArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableInArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertArrayInArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableArrayInArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertValueInTableArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableInTableArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertArrayInTableArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertTableArrayInTableArrayAsTOML)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertMixed)
{}

TEST(TOMLInsertNode, DISABLED_TestInsertMixedWithDelete)
{}
