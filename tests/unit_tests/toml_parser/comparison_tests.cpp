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
#include <support/toml.h>
#include "../../../sdv_services/core/toml_parser/exception.h"
#include "../../../sdv_services/core/toml_parser/miscellaneous.h"
#include "../../../sdv_services/core/toml_parser/parser_toml.h"

bool CompareTest(const std::string& rssToml1, const std::string& rssToml2,
    uint32_t uiCompareFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all))
{
    try
    {
        toml_parser::CParser parser1(rssToml1);
        toml_parser::CParser parser2(rssToml2);
        sdv::toml::CNodeCollection collection1(&parser1.Root());
        sdv::toml::CNodeCollection collection2(&parser2.Root());
        return sdv::toml::internal::CompareNodes(collection1, collection2, uiCompareFlags) ==
            sdv::toml::ECompareResult::compare_identical;
    }
    catch (const toml_parser::XTOMLParseException&)
    {
        return false;
    }
}

TEST(Comparison, CompareInvalid)
{
    EXPECT_FALSE(CompareTest("var = A", ""));
    EXPECT_FALSE(CompareTest("[[test]", ""));
    EXPECT_FALSE(CompareTest(R"toml([test]
test1 = "this is invalid')toml", ""));
}

TEST(Comparison, CompareEmpty)
{
    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_FALSE(CompareTest("var = 1", "", uiFlags));
    EXPECT_FALSE(CompareTest("", "var = 2", uiFlags));
    EXPECT_TRUE(CompareTest("", "", uiFlags));
    EXPECT_FALSE(CompareTest("", R"toml( 
)toml", uiFlags));
    EXPECT_FALSE(CompareTest("", R"toml(# This is a comment)toml", uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_FALSE(CompareTest("var = 1", "", uiFlags));
    EXPECT_FALSE(CompareTest("", "var = 2", uiFlags));
    EXPECT_TRUE(CompareTest("", "", uiFlags));
    EXPECT_TRUE(CompareTest("", R"toml( 
)toml", uiFlags));
    EXPECT_FALSE(CompareTest("", R"toml(# This is a comment)toml", uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_FALSE(CompareTest("var = 1", "", uiFlags));
    EXPECT_FALSE(CompareTest("", "var = 2", uiFlags));
    EXPECT_TRUE(CompareTest("", "", uiFlags));
    EXPECT_TRUE(CompareTest("", R"toml( 
)toml", uiFlags));
    EXPECT_TRUE(CompareTest("", R"toml(# This is a comment)toml", uiFlags));
}

TEST(Comparison, CompareValues)
{
    std::string ssVal = "val = 1";
    std::string ssVal2 = "val = 2";
    std::string ssValSpace = " val   =  1";
    std::string ssValCommentsBefore = R"toml(# comment

# more comment
 val   =  1)toml";
    std::string ssValCommentsBehind = R"toml( val   =  1 # comment
# more comment

# lots more comment)toml";

    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_TRUE(CompareTest(ssVal, ssVal, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssVal2, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssValSpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssValCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssValCommentsBehind, uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_TRUE(CompareTest(ssVal, ssVal, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssVal2, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValSpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssValCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssValCommentsBehind, uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_TRUE(CompareTest(ssVal, ssVal, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssVal2, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValSpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValCommentsBehind, uiFlags));

    // Ignore all
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all);
    EXPECT_TRUE(CompareTest(ssVal, ssVal, uiFlags));
    EXPECT_FALSE(CompareTest(ssVal, ssVal2, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValSpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssVal, ssValCommentsBehind, uiFlags));
}

TEST(Comparison, CompareTables)
{
    std::string ssStandardTable = R"toml([table1]
val = 1)toml";
    std::string ssStandardTable2 = R"toml([table2]
val = 1)toml";
    std::string ssStandardTable1_3 = R"toml([table1]
val = 3)toml";
    std::string ssInlineTable = R"toml(table1 = {val = 1})toml";
    std::string ssTableSpace = R"toml( [table1]

  val  =     1)toml";
    std::string ssTableCommentsBefore = R"toml(# comment

# more comment
 [table1]
val = 1)toml";
    std::string ssTableCommentsBehind = R"toml( [table1] # comment
val = 1 # comment
# more comment

# lots more comment)toml";

    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_TRUE(CompareTest(ssStandardTable, ssStandardTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssInlineTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableSpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBehind, uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_TRUE(CompareTest(ssStandardTable, ssStandardTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssInlineTable, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableSpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBehind, uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_TRUE(CompareTest(ssStandardTable, ssStandardTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssInlineTable, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableSpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableCommentsBehind, uiFlags));

    // Ignore inline
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_inline);
    EXPECT_TRUE(CompareTest(ssStandardTable, ssStandardTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable1_3, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssInlineTable, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableSpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssTableCommentsBehind, uiFlags));

    // Ignore all
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all);
    EXPECT_TRUE(CompareTest(ssStandardTable, ssStandardTable, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTable, ssStandardTable1_3, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssInlineTable, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableSpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTable, ssTableCommentsBehind, uiFlags));
}

TEST(Comparison, CompareArrays)
{
    std::string ssStandardArray = R"toml(array1 = [123, 456])toml";
    std::string ssStandardArray2 = R"toml(array2 = [789, 543])toml";
    std::string ssArraySpace = R"toml( array1 =   [
                                                    123,
                                                    456 ])toml";
    std::string ssArrayCommentsBefore = R"toml(# comment
array1 = 
# more comment
 [ 123, # xyz
456])toml";
    std::string ssArrayCommentsBehind = R"toml( array1 = [123, 456] # comment
# more comment

# lots more comment)toml";

    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_TRUE(CompareTest(ssStandardArray, ssStandardArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssStandardArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssArraySpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssArrayCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssArrayCommentsBehind, uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_TRUE(CompareTest(ssStandardArray, ssStandardArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssStandardArray2, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArraySpace, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssArrayCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssArrayCommentsBehind, uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_TRUE(CompareTest(ssStandardArray, ssStandardArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssStandardArray2, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArraySpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArrayCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArrayCommentsBehind, uiFlags));

    // Ignore all
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all);
    EXPECT_TRUE(CompareTest(ssStandardArray, ssStandardArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardArray, ssStandardArray2, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArraySpace, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArrayCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardArray, ssArrayCommentsBehind, uiFlags));
}

TEST(Comparison, CompareTableArrays)
{
    std::string ssStandardTableArray = R"toml([[tableArray1]]
val = 1)toml";
    std::string ssStandardTableArray2 = R"toml([[tableArray2]]
val = 1)toml";
    std::string ssStandardTableArray1_3 = R"toml([[tableArray1]]
val = 3)toml";
    std::string ssInlineTableArray = R"toml(tableArray1 = [{val = 1}])toml";
    std::string ssTableSpaceArray = R"toml( [[tableArray1]]

  val  =     1)toml";
    std::string ssTableArrayCommentsBefore = R"toml(# comment

# more comment
 [[tableArray1]]
val = 1)toml";
    std::string ssTableArrayCommentsBehind = R"toml( [[tableArray1]] # comment
val = 1 # comment
# more comment

# lots more comment)toml";

    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssStandardTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssInlineTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableSpaceArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBehind, uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssStandardTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssInlineTableArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableSpaceArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBehind, uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssStandardTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray1_3, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssInlineTableArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableSpaceArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBehind, uiFlags));

    // Ignore inline
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_inline);
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssStandardTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray1_3, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssInlineTableArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableSpaceArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBefore, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBehind, uiFlags));

    // Ignore all
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all);
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssStandardTableArray, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray2, uiFlags));
    EXPECT_FALSE(CompareTest(ssStandardTableArray, ssStandardTableArray1_3, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssInlineTableArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableSpaceArray, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBefore, uiFlags));
    EXPECT_TRUE(CompareTest(ssStandardTableArray, ssTableArrayCommentsBehind, uiFlags));
}

TEST(Comparison, CompareComplex)
{
    std::string ssToml1 = R"toml(# This is a complex TOML file

# Top value description
[Top]
value1 = 1                      # Value 1
valueA = "A"                    # Value A
arrayBCD = [ "B",               # Array element B
             "C",               # Array element C
             "D" ]              # Array element D

# Table 1
[Top.table1]
value2 = 2                      # Value 2

# Table array in table 1
[[Top.table1.table_array]]      # Table array 
valueE = "E"

[[Top.table1.table_array]]      # Table array 
valueF = "F"

[Top.table1.other_table]
inline_table = { x = "X", y = "Y", z = "Z" }    # Inline table

[[Top.table1.table_array]]      # Table array 
valueG = "G"
)toml";

    std::string ssToml2 = R"toml(
[Top]
    value1 = 1
    valueA = "A"
    arrayBCD = ["B", "C", "D" ]
    table1 = { value2 = 2, table_array = [{valueE = "E"}, {valueF = "F"}, {valueG = "G"}], other_table = { inline_table = {x = "X", y = "Y", z = "Z"}}} # Mega table
)toml";

    // Identical
    uint32_t uiFlags = 0u;
    EXPECT_TRUE(CompareTest(ssToml1, ssToml1, uiFlags));
    EXPECT_FALSE(CompareTest(ssToml1, ssToml2, uiFlags));

    // Ignore whitespace
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_whitespace);
    EXPECT_TRUE(CompareTest(ssToml1, ssToml1, uiFlags));
    EXPECT_FALSE(CompareTest(ssToml1, ssToml2, uiFlags));

    // Ignore comments
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_comments);
    EXPECT_TRUE(CompareTest(ssToml1, ssToml1, uiFlags));
    EXPECT_FALSE(CompareTest(ssToml1, ssToml2, uiFlags));

    // Ignore inline
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_inline);
    EXPECT_TRUE(CompareTest(ssToml1, ssToml1, uiFlags));
    EXPECT_FALSE(CompareTest(ssToml1, ssToml2, uiFlags));

    // Ignore all
    uiFlags = static_cast<uint32_t>(sdv::toml::ECompareFlags::compare_ignore_all);
    EXPECT_TRUE(CompareTest(ssToml1, ssToml1, uiFlags));
    EXPECT_TRUE(CompareTest(ssToml1, ssToml2, uiFlags));
}