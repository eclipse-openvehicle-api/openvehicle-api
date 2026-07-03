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
 *   Erik Verhoeven - initial implementation
 ********************************************************************************/

#include <functional>
#include <gtest/gtest.h>
#include <limits>
#include "../../../global/localmemmgr.h"
#include "../../../sdv_services/core/toml_parser/exception.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include <support/toml.h>

TEST(MakeInline, Value)
{
    std::string ssTomlIn = R"toml(value1 = 1)toml";
    std::string ssTomlOut = R"toml(value1 = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, Value)
{
    std::string ssTomlIn = R"toml(value1 = 1)toml";
    std::string ssTomlOut = R"toml(value1 = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_FALSE(parser.Root().CanMakeStandard());
    EXPECT_FALSE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, Array)
{
    std::string ssTomlIn = R"toml(value1 = ["abc", "def", "ghi"])toml";
    std::string ssTomlOut = R"toml(value1 = ["abc", "def", "ghi"])toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, Array)
{
    std::string ssTomlIn  = R"toml(value1 = ["abc", "def", "ghi"])toml";
    std::string ssTomlOut = R"toml(value1 = ["abc", "def", "ghi"])toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_FALSE(parser.Root().CanMakeStandard());
    EXPECT_FALSE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, StandardTable)
{
    std::string ssTomlIn = R"toml([table1]
value = 1)toml";
    std::string ssTomlOut = R"toml(table1 = {value = 1})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, StandardTable)
{
    std::string ssTomlIn  = R"toml([table1]
value = 1)toml";
    std::string ssTomlOut = R"toml([table1]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, InlineTable)
{
    std::string ssTomlIn = R"toml(table1 = {value = 1})toml";
    std::string ssTomlOut = R"toml(table1 = {value = 1})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, InlineTable)
{
    std::string ssTomlIn  = R"toml(table1 = {value = 1})toml";
    std::string ssTomlOut = R"toml([table1]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, StandardTableArray)
{
    std::string ssTomlIn = R"toml([[table_array1]]
value = 1)toml";
    std::string ssTomlOut = R"toml(table_array1 = [{value = 1}])toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, StandardTableArray)
{
    std::string ssTomlIn  = R"toml([[table_array1]]
value = 1)toml";
    std::string ssTomlOut = R"toml([[table_array1]]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, InlineTableArray)
{
    std::string ssTomlIn = R"toml(table_array1 = [{value = 1}])toml";
    std::string ssTomlOut = R"toml(table_array1 = [{value = 1}])toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, InlineTableArray)
{
    std::string ssTomlIn  = R"toml(table_array1 = [{value = 1}])toml";
    std::string ssTomlOut = R"toml([[table_array1]]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, StandardTableImplicit)
{
    std::string ssTomlIn  = R"toml([root_table.table1]
value = 1)toml";
    std::string ssTomlOut = R"toml(root_table.table1 = {value = 1})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, StandardTableImplicit)
{
    std::string ssTomlIn  = R"toml([root_table.table1]
value = 1)toml";
    std::string ssTomlOut = R"toml([root_table.table1]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, InlineTableImplicit)
{
    std::string ssTomlIn  = R"toml(root_table.table1 = {value = 1})toml";
    std::string ssTomlOut = R"toml(root_table.table1 = {value = 1})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, InlineTableImplicit)
{
    std::string ssTomlIn  = R"toml(root_table.table1 = {value = 1})toml";
    std::string ssTomlOut = R"toml([root_table.table1]
value = 1)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, StandardTableNested)
{
    std::string ssTomlIn  = R"toml([table1]
value1 = 1
[table1.table2]
value2 = 2
[table1.table3.table4]
value3 = 3
value4 = 4)toml";
    std::string ssTomlOut = R"toml(table1 = {value1 = 1, table2 = {value2 = 2}, table3.table4 = {value3 = 3, value4 = 4}})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, StandardTableNested)
{
    std::string ssTomlIn  = R"toml([table1]
value = 1
[table1.table2]
value = 2)toml";
    std::string ssTomlOut = R"toml([table1]
value = 1
[table1.table2]
value = 2)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeInline, InlineTableNested)
{
    std::string ssTomlIn  = R"toml(table1 = {value = 1, table2 = {value = 2}})toml";
    std::string ssTomlOut = R"toml(table1 = {value = 1, table2 = {value = 2}})toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeInline());
    EXPECT_TRUE(parser.Root().MakeInline());
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

TEST(MakeStandard, InlineTableNested)
{
    std::string ssTomlIn  = R"toml(table1 = {value = 1, table2 = {value = 2}})toml";
    std::string ssTomlOut = R"toml([table1]
value = 1
[table1.table2]
value = 2)toml";

    toml_parser::CParser parser(ssTomlIn);
    EXPECT_TRUE(parser.Root().CanMakeStandard());
    EXPECT_TRUE(parser.Root().MakeStandard(true));
    std::string ssOut = parser.Root().GetTOML();
    EXPECT_EQ(ssTomlOut, ssOut);
}

