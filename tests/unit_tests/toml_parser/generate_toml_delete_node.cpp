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

// Delete nodes (for all types of nodes, between all types of nodes)
//  - beginning
//  - middle
//  - end
//  - Using deleted node info   -- error
//  - Last node (empty after that)
//  - Smart delete (comments/whitespace around)

// Insert nodes (for all types of nodes, between all types of nodes)
// Before and after:
//  - beginning
//  - middle
//  - end
//  - Being the first item in a TOML file
//  - Inserted and straight away deleted
//  - Inserted with false/deleted reference --error
//  - Inserted values before (okay) and behind (error) tables
//  - Inserted duplicate value -- error
//  - Smart insert (comments/whitespace around)

// Shift nodes (for all types of nodes, between root, tables and arrays)

/*
 * @brief Delete a key from the TOML string.
 * @param[in] rssTOMLInput Reference to the TOML string.
 * @param[in] rssKey Reference to the key to delete.
 * @param[in] rssOuput Reference to the expected ouput.
 * @return Returns 'true' on success.
 */ 
bool TestDelete(const std::string& rssTOMLInput, const std::string& rssKey, const std::string& rssOutput)
{
    toml_parser::CParser parser;
    bool bRes = true;
    EXPECT_NO_THROW(bRes = parser.Process(rssTOMLInput));
    EXPECT_TRUE(bRes);
    if (!bRes) return bRes;
    auto ptrNode = parser.Root().Direct(rssKey);
    EXPECT_TRUE(ptrNode);
    if (!ptrNode) return false;
    EXPECT_TRUE(bRes = ptrNode->DeleteNode());
    if (!bRes) return bRes;
    std::string ssTOML = parser.GenerateTOML();
    EXPECT_EQ(ssTOML, rssOutput);
    if (ssTOML != rssOutput) return false;
    return true;
};

TEST(TOMLDeleteNode, DeleteValues)
{
    // Delete a key from the begin (whitespace reduced)
    EXPECT_TRUE(TestDelete(R"toml(key = 10            # value key
bare_key = "value"  # value bare_key
bare-key = false    # value bare-key)toml",
        "key",
        R"toml(bare_key = "value"  # value bare_key
bare-key = false    # value bare-key)toml"));

    // Delete a key from the begin
    EXPECT_TRUE(TestDelete(R"toml(
key = 10            # value key
bare_key = "value"  # value bare_key
bare-key = false    # value bare-key
)toml",
        "key",
        R"toml(
bare_key = "value"  # value bare_key
bare-key = false    # value bare-key
)toml"));

    // Delete a key from the middle
    EXPECT_TRUE(TestDelete(R"toml(
key = 10            # value key
bare_key = "value"  # value bare_key
bare-key = false    # value bare-key
)toml",
        "bare_key",
        R"toml(
key = 10            # value key
bare-key = false    # value bare-key
)toml"));

    // Delete a key from the end
    EXPECT_TRUE(TestDelete(R"toml(
key = 10            # value key
bare_key = "value"  # value bare_key
bare-key = false    # value bare-key
)toml",
        "bare-key",
        R"toml(
key = 10            # value key
bare_key = "value"  # value bare_key
)toml"));
}

TEST(TOMLDeleteNode, DeleteInlineTableValues)
{
    // Delete key from the inline table
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "1234.y",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "1234.x",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = { y = 1, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "1234.str",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2}
)toml"));

}

TEST(TOMLDeleteNode, DeleteInlineSubTableValues)
{
    // Delete key from the inline sub-table
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "1234.tbl.b",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, c=3}}
)toml"));

}

TEST(TOMLDeleteNode, DeleteInlineTables)
{
    // Delete table
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "1234.tbl",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "1234",
        R"toml(
key = 10
bare_key = "value"
bare-key = false
)toml"));
}

TEST(TOMLDeleteNode, DeleteTableValues)
{
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
)toml",
        "my_table.key",
        R"toml(
[my_table]
bare_key = "value"
bare-key = false
)toml"));

    // Delete a key from the middle
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
)toml",
        "my_table.bare_key",
        R"toml(
[my_table]
key = 10
bare-key = false
)toml"));

    // Delete a key from the end
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
)toml",
        "my_table.bare-key",
        R"toml(
[my_table]
key = 10
bare_key = "value"
)toml"));

}

TEST(TOMLDeleteNode, DeleteInlineTableValueInTable)
{
    // Delete key from the inline table in a table
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "my_table.1234.y",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "my_table.1234.x",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = { y = 1, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml",
        "my_table.1234.str",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2}
)toml"));

}

TEST(TOMLDeleteNode, DeleteInlineSubTableValueInTable)
{
    // Delete key from the inline sub-table
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "my_table.1234.tbl.b",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, c=3}}
)toml"));

}

TEST(TOMLDeleteNode, DeleteInlineTableInTable)
{
    // Delete table
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "my_table.1234.tbl",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc"}
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
1234 = {x = 0, y = 1, z = 2, str = "abc", tbl={a =1, b=2, c=3}}
)toml",
        "my_table.1234",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
)toml"));

}

TEST(TOMLDeleteNode, DeleteValuesInChildTable)
{
    // Delete key from the child-table
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
)toml",
        "my_table.1234.y",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
z = 2
str = "abc"
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
)toml",
        "my_table.1234.x",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
y = 1
z = 2
str = "abc"
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
)toml",
        "my_table.1234.str",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
)toml"));

}

TEST(TOMLDeleteNode, DeleteChildTableInTable)
{
    // Delete table
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
[my_table.1234.tbl]
a =1
b=2
c=3
)toml",
        "my_table.1234.tbl",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
[my_table.1234]
x = 0
y = 1
z = 2
str = "abc"
[my_table.1234.tbl]
a =1
b=2
c=3
)toml",
        "my_table.1234",
        R"toml(
[my_table]
key = 10
bare_key = "value"
bare-key = false
)toml"));
}

TEST(TOMLDeleteNode, DeleteArrayValues)
{
    // Delete array values
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "key[0]",
        R"toml(
key = [ 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "key[1]",
        R"toml(
key = [10, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "key[2]",
        R"toml(
key = [10, 20]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));

}

TEST(TOMLDeleteNode, DeleteArrayValuesWithSucceedingComma)
{
    // Delete array values with succeeding comma
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare_key[0]",
        R"toml(
key = [10, 20, 30]
bare_key = [ "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare_key[1]",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare_key[2]",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));

}

TEST(TOMLDeleteNode, DeleteValuesWithComments)
{
    // Delete array values with comments
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare-key[0]",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare-key[0].a",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{ b = true},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare-key[0].b",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false},  # value 0
            2020,                   # value 1
           ]
)toml"));
    EXPECT_TRUE(TestDelete(R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
            2020,                   # value 1
           ]
)toml",
        "bare-key[1]",
        R"toml(
key = [10, 20, 30]
bare_key = ["value1", "value2", 3030, ]
bare-key = [{a = false, b = true},  # value 0
           ]
)toml"));
}