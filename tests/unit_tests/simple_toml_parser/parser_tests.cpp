#include <iostream>
#include <gtest/gtest.h>
#include <support/simple_toml.h>

TEST(RecognizeTypes, Root)
{
    sdv::toml::simple_parser::CParser parser("");
    auto sRoot = parser.Root();
    EXPECT_EQ(sRoot.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
}

TEST(RecognizeTypes, Table)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [newTable]
        [secondTable.nestedTable]
        )toml");
    auto sRoot = parser.Root();

    auto table1 = sRoot.GetDirect("newTable");
    EXPECT_EQ(table1.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(table1.GetName(), "newTable");
    EXPECT_EQ(table1.GetValue(), "");

    auto table2 = sRoot.GetDirect("secondTable");
    EXPECT_EQ(table2.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(table2.GetName(), "secondTable");
    EXPECT_EQ(table2.GetValue(), "");

    auto table3 = sRoot.GetDirect("secondTable.nestedTable");
    EXPECT_EQ(table3.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(table3.GetName(), "nestedTable");
    EXPECT_EQ(table3.GetValue(), "");
}

TEST(RecognizeTypes, Key_Value)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        name = "Hammer"
        id = 42
        pi = 3.1415926
        boolean = true
        array = []
        table = {}
        )toml");
    auto sRoot = parser.Root();

    auto value_name = sRoot.GetDirect("name");
    EXPECT_EQ(value_name.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(value_name.GetValue(), "Hammer");

    auto value_id = sRoot.GetDirect("id");
    EXPECT_EQ(value_id.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(value_id.GetValue<int>(), 42);

    auto value_pi = sRoot.GetDirect("pi");
    EXPECT_EQ(value_pi.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(value_pi.GetValue<double>(), 3.1415926);

    auto value_boolean = sRoot.GetDirect("boolean");
    EXPECT_EQ(value_boolean.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(value_boolean.GetValue<bool>(), true);

    auto value_array = sRoot.GetDirect("array");
    EXPECT_EQ(value_array.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
    EXPECT_EQ(value_array.GetValue(), "");

    auto value_table = sRoot.GetDirect("table");
    EXPECT_EQ(value_table.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(value_table.GetValue(), "");
}

TEST(RecognizeTypes, TableArray)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [[newTableArray]]
        [[newTableArray]]
        [[table.nestedTableArray]]
        )toml");
    auto sRoot = parser.Root();

    auto tableArray1 = sRoot.GetDirect("newTableArray");
    EXPECT_EQ(tableArray1.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
    EXPECT_EQ(tableArray1.GetName(), "newTableArray");
    
    auto table1 = sRoot.GetDirect("newTableArray[0]");
    ASSERT_TRUE(table1);
    EXPECT_EQ(table1.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(table1.GetName(), "newTableArray");

    auto table2 = sRoot.GetDirect("newTableArray[1]");
    ASSERT_TRUE(table2);
    EXPECT_EQ(table2.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
    EXPECT_EQ(table2.GetName(), "newTableArray");
}

TEST(NestedContent, Array)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        arr_mixed = [ 1.0, 2, "test string", [ 1, 2 ], { pi = 3.14, e = 2.71828 }, true]
        arr_ints = [ 1, 2, 3, 4]
        arr_ints_trailing_comma = [ 1, 2, 3, 4, ]
        arr_multiline = [
            "first line",
            "second line",
            "third_line",
        ]
        )toml");
    auto sRoot = parser.Root();

    {
        auto array_ints = sRoot.GetDirect("arr_ints");
        EXPECT_EQ(array_ints.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
        auto array_ints_0 = sRoot.GetDirect("arr_ints[0]");
        ASSERT_TRUE(array_ints_0);
        EXPECT_EQ(array_ints_0.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_0.GetValue<int>(), 1);
        auto array_ints_1 = sRoot.GetDirect("arr_ints[1]");
        ASSERT_TRUE(array_ints_1);
        EXPECT_EQ(array_ints_1.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_1.GetValue<int>(), 2);
        auto array_ints_2 = sRoot.GetDirect("arr_ints[2]");
        ASSERT_TRUE(array_ints_2);
        EXPECT_EQ(array_ints_2.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_2.GetValue<int>(), 3);
        auto array_ints_3 = sRoot.GetDirect("arr_ints[3]");
        ASSERT_TRUE(array_ints_3);
        EXPECT_EQ(array_ints_3.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_3.GetValue<int>(), 4);
        auto array_ints_4 = sRoot.GetDirect("arr_ints[4]");
        EXPECT_FALSE(array_ints_4);
    }

    {
        auto array_ints_trailing_comma   = sRoot.GetDirect("arr_ints_trailing_comma");
        auto array_ints_trailing_comma_0 = sRoot.GetDirect("arr_ints_trailing_comma[0]");
        auto array_ints_trailing_comma_1 = sRoot.GetDirect("arr_ints_trailing_comma[1]");
        auto array_ints_trailing_comma_2 = sRoot.GetDirect("arr_ints_trailing_comma[2]");
        auto array_ints_trailing_comma_3 = sRoot.GetDirect("arr_ints_trailing_comma[3]");
        auto array_ints_trailing_comma_4 = sRoot.GetDirect("arr_ints_trailing_comma[4]");

        EXPECT_EQ(array_ints_trailing_comma.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
        ASSERT_TRUE(array_ints_trailing_comma_0);
        EXPECT_EQ(array_ints_trailing_comma_0.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_trailing_comma_0.GetValue<int>(), 1);
        ASSERT_TRUE(array_ints_trailing_comma_1);
        EXPECT_EQ(array_ints_trailing_comma_1.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_trailing_comma_1.GetValue<int>(), 2);
        ASSERT_TRUE(array_ints_trailing_comma_2);
        EXPECT_EQ(array_ints_trailing_comma_2.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_trailing_comma_2.GetValue<int>(), 3);
        ASSERT_TRUE(array_ints_trailing_comma_3);
        EXPECT_EQ(array_ints_trailing_comma_3.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_ints_trailing_comma_3.GetValue<int>(), 4);
        EXPECT_FALSE(array_ints_trailing_comma_4);
    }

    {
        auto array_mixed      = sRoot.GetDirect("arr_mixed");
        auto array_mixed_0    = sRoot.GetDirect("arr_mixed[0]");
        auto array_mixed_1    = sRoot.GetDirect("arr_mixed[1]");
        auto array_mixed_2    = sRoot.GetDirect("arr_mixed[2]");
        auto array_mixed_3    = sRoot.GetDirect("arr_mixed[3]");
        auto array_mixed_3_1  = sRoot.GetDirect("arr_mixed[3][0]");
        auto array_mixed_3_2  = sRoot.GetDirect("arr_mixed[3][1]");
        auto array_mixed_4    = sRoot.GetDirect("arr_mixed[4]");
        auto array_mixed_4_pi = sRoot.GetDirect("arr_mixed[4].pi");
        auto array_mixed_4_e  = sRoot.GetDirect("arr_mixed[4].e");
        auto array_mixed_5    = sRoot.GetDirect("arr_mixed[5]");
        auto array_mixed_6    = sRoot.GetDirect("arr_mixed[6]");

        EXPECT_EQ(array_mixed.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
        ASSERT_TRUE(array_mixed_0);
        EXPECT_EQ(array_mixed_0.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_0.GetValue<double>(), 1.0);
        ASSERT_TRUE(array_mixed_1);
        EXPECT_EQ(array_mixed_1.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_1.GetValue<int>(), 2);
        ASSERT_TRUE(array_mixed_2);
        EXPECT_EQ(array_mixed_2.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_2.GetValue(), "test string");
        ASSERT_TRUE(array_mixed_3);
        EXPECT_EQ(array_mixed_3.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
        EXPECT_EQ(array_mixed_3_1.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_3_1.GetValue<int>(), 1);
        EXPECT_EQ(array_mixed_3_2.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_3_2.GetValue<int>(), 2);
        ASSERT_TRUE(array_mixed_4);
        EXPECT_EQ(array_mixed_4.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
        EXPECT_EQ(array_mixed_4_pi.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_4_pi.GetValue<double>(), 3.14);
        EXPECT_EQ(array_mixed_4_e.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_4_e.GetValue<double>(), 2.71828);
        ASSERT_TRUE(array_mixed_5);
        EXPECT_EQ(array_mixed_5.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_mixed_5.GetValue<bool>(), true);
        EXPECT_FALSE(array_mixed_6);
    }

    {
        auto array_multiline   = sRoot.GetDirect("arr_multiline");
        auto array_multiline_0 = sRoot.GetDirect("arr_multiline[0]");
        auto array_multiline_1 = sRoot.GetDirect("arr_multiline[1]");
        auto array_multiline_2 = sRoot.GetDirect("arr_multiline[2]");
        auto array_multiline_3 = sRoot.GetDirect("arr_multiline[3]");

        EXPECT_EQ(array_multiline.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
        ASSERT_TRUE(array_multiline_0);
        EXPECT_EQ(array_multiline_0.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_multiline_0.GetValue(), "first line");
        ASSERT_TRUE(array_multiline_1);
        EXPECT_EQ(array_multiline_1.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_multiline_1.GetValue(), "second line");
        ASSERT_TRUE(array_multiline_2);
        EXPECT_EQ(array_multiline_2.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(array_multiline_2.GetValue(), "third_line");
        EXPECT_FALSE(array_multiline_3);
    }
}

TEST(NestedContent, Table)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [table]
        a = 2
        b = 1.2
        [anotherTable]
        a = 4
        c = false
        [thirdTable.fourthTable]
        a = "five"
        d = []
        )toml");
    auto sRoot = parser.Root();

    auto table_a        = sRoot.GetDirect("table.a");
    auto table_b        = sRoot.GetDirect("table.b");
    auto anotherTable_a = sRoot.GetDirect("anotherTable.a");
    auto anotherTable_c = sRoot.GetDirect("anotherTable.c");
    auto fourthTable_a  = sRoot.GetDirect("thirdTable.fourthTable.a");
    auto fourthTable_d  = sRoot.GetDirect("thirdTable.fourthTable.d");

    ASSERT_TRUE(table_a);
    EXPECT_EQ(table_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_a.GetValue<int>(), 2);
    ASSERT_TRUE(table_b);
    EXPECT_EQ(table_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_b.GetValue<double>(), 1.2);
    ASSERT_TRUE(anotherTable_a);
    EXPECT_EQ(anotherTable_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(anotherTable_a.GetValue<int>(), 4);
    ASSERT_TRUE(anotherTable_c);
    EXPECT_EQ(anotherTable_c.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(anotherTable_c.GetValue<bool>(), false);
    ASSERT_TRUE(fourthTable_a);
    EXPECT_EQ(fourthTable_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(fourthTable_a.GetValue(), "five");
    ASSERT_TRUE(fourthTable_d);
    EXPECT_EQ(fourthTable_d.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
}

TEST(NestedContent, TableArray)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [[table.test]]
        a = 2
        b = 1.2
        [[table.test]]
        a = 4
        c = false
        [[table.test]]
        a = "five"
        d = []
        )toml");
    auto sRoot = parser.Root();

    auto table_test_1_a = sRoot.GetDirect("table.test[0].a");
    auto table_test_1_b = sRoot.GetDirect("table.test[0].b");
    auto table_test_2_a = sRoot.GetDirect("table.test[1].a");
    auto table_test_2_c = sRoot.GetDirect("table.test[1].c");
    auto table_test_3_a = sRoot.GetDirect("table.test[2].a");
    auto table_test_3_d = sRoot.GetDirect("table.test[2].d");

    ASSERT_TRUE(table_test_1_a);
    EXPECT_EQ(table_test_1_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_test_1_a.GetValue<int>(), 2);
    ASSERT_TRUE(table_test_1_b);
    EXPECT_EQ(table_test_1_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_test_1_b.GetValue<double>(), 1.2);
    ASSERT_TRUE(table_test_2_a);
    EXPECT_EQ(table_test_2_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_test_2_a.GetValue<int>(), 4);
    ASSERT_TRUE(table_test_2_c);
    EXPECT_EQ(table_test_2_c.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_test_2_c.GetValue<bool>(), false);
    ASSERT_TRUE(table_test_3_a);
    EXPECT_EQ(table_test_3_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table_test_3_a.GetValue(), "five");
    ASSERT_TRUE(table_test_3_d);
    EXPECT_EQ(table_test_3_d.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
}

TEST(NestedContent, InlineTable)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        table1 = { a = 0, b = 1.2, c = "string" }
        table2 = { a = [], b = true, e = 2.71828 }
        table3 = { a = { a = "a", b = "A" }, b = {a = "b", b = "B"}, e = {a = "e", b = "E"} }
        )toml");
    auto sRoot = parser.Root();

    auto table1_a   = sRoot.GetDirect("table1.a");
    auto table1_b   = sRoot.GetDirect("table1.b");
    auto table1_c   = sRoot.GetDirect("table1.c");
    auto table2_a   = sRoot.GetDirect("table2.a");
    auto table2_b   = sRoot.GetDirect("table2.b");
    auto table2_e   = sRoot.GetDirect("table2.e");
    auto table3_a_a = sRoot.GetDirect("table3.a.a");
    auto table3_a_b = sRoot.GetDirect("table3.a.b");
    auto table3_b_a = sRoot.GetDirect("table3.b.a");
    auto table3_b_b = sRoot.GetDirect("table3.b.b");
    auto table3_e_a = sRoot.GetDirect("table3.e.a");
    auto table3_e_b = sRoot.GetDirect("table3.e.b");

    ASSERT_TRUE(table1_a);
    EXPECT_EQ(table1_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table1_a.GetValue<int>(), 0);
    ASSERT_TRUE(table1_b);
    EXPECT_EQ(table1_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table1_b.GetValue<double>(), 1.2);
    ASSERT_TRUE(table1_c);
    EXPECT_EQ(table1_c.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table1_c.GetValue(), "string");
    ASSERT_TRUE(table2_a);
    EXPECT_EQ(table2_a.GetType(), sdv::toml::simple_parser::ENodeType::node_array);
    ASSERT_TRUE(table2_b);
    EXPECT_EQ(table2_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table2_b.GetValue<bool>(), true);
    ASSERT_TRUE(table2_e);
    EXPECT_EQ(table2_e.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table2_e.GetValue<double>(), 2.71828);
    ASSERT_TRUE(table3_a_a);
    EXPECT_EQ(table3_a_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_a_a.GetValue(), "a");
    ASSERT_TRUE(table3_a_b);
    EXPECT_EQ(table3_a_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_a_b.GetValue(), "A");
    ASSERT_TRUE(table3_b_a);
    EXPECT_EQ(table3_b_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_b_a.GetValue(), "b");
    ASSERT_TRUE(table3_b_b);
    EXPECT_EQ(table3_b_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_b_b.GetValue(), "B");
    ASSERT_TRUE(table3_e_a);
    EXPECT_EQ(table3_e_a.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_e_a.GetValue(), "e");
    ASSERT_TRUE(table3_e_b);
    EXPECT_EQ(table3_e_b.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
    EXPECT_EQ(table3_e_b.GetValue(), "E");
}

TEST(NestedContent, InlineTableBreakLine)
{
    // The following is not allowed in version 1.0, but is allowed in version 1.1
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
table = { a = 1, b = 2,
    c = 3, d = 4 }
)toml"));
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
table = { a = 1, b = 2
    ,c = 3, d = 4 }
)toml"));

    // Line breaks are allowed when part of an array or have multi-line strings
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
array = [{ a = 1, b = 2},
    {c = 3, d = 4}]
)toml"));
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
table = { a = 1, b = [2, 3,
    4, 5], c = 6, d = 7}
)toml"));
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
table = { x = "abc", y = """def-
ghi""", z = "jkl" }
)toml"));
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
table = { x = 'abc', y = '''def-
ghi''', z = 'jkl' }
)toml"));
}

TEST(SpecialCases, Keys)
{
    std::string ssUTF8String = u8R"toml(
            "127.0.0.1" = "value"
            "character encoding" = "value"
            "ʎǝʞ" = "value"
            'key2' = "value"
            'quoted "value"' = "value"
        )toml";
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(std::string_view(ssUTF8String)));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            key = "value"
            bare_key = "value"
            bare-key = "value"
            1234 = "value"
        )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            "" = "blank"     # VALID but discouraged
        )toml"));
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            '' = 'blank'     # VALID but discouraged
        )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            name = "Orange"
            physical.color = "orange"
            physical.shape = "round"
            site."google.com" = true
        )toml"));


    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            fruit.name = "banana"     # this is best practice
            fruit. color = "yellow"    # same as fruit.color
            fruit . flavor = "banana"   # same as fruit.flavor
        )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            # This makes the key "fruit" into a table.
            fruit.apple.smooth = true
            # So then you can add to the table "fruit" like so:
            fruit.orange = 2
        )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            # VALID BUT DISCOURAGED
            apple.type = "fruit"
            orange.type = "fruit"
            apple.skin = "thin"
            orange.skin = "thick"
            apple.color = "red"
            orange.color = "orange"
        )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            3.1415 = 3.1415
        )toml"));
    {
        sdv::toml::simple_parser::CParser parser(R"toml(
            3.1415 = 3.1415
        )toml");
        auto sRoot = parser.Root();

        auto table = sRoot.GetDirect("3");
        auto pi = sRoot.GetDirect("3.1415");
        ASSERT_TRUE(table);
        EXPECT_EQ(table.GetType(), sdv::toml::simple_parser::ENodeType::node_table);
        ASSERT_TRUE(pi);
        EXPECT_EQ(pi.GetType(), sdv::toml::simple_parser::ENodeType::node_value);
        EXPECT_EQ(pi.GetValue<double>(), 3.1415);
    }
}

TEST(SpecialCases, Arrays)
{
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            integers = [ 1, 2, 3 ]
            colors = [ "red", "yellow", "green" ]
            nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
            nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
            string_array = [ "all", 'strings', """are the same""", '''type''' ]
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
            contributors = [
            "Foo Bar <foo@example.com>",
            { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
            ]
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            integers3 = [
                1,
                2, # this is ok
            ]
            )toml"));
}

TEST(SpecialCases, Tables)
{
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [table-1]
            key1 = "some string"
            key2 = 123

            [table-2]
            key1 = "another string"
            key2 = 456
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [dog."tater.man"]
            type.name = "pug"
            )toml"));

    std::string ssUTF8String = u8R"toml(
            [a.b.c]            # this is best practice
            [ d.e.f ]          # same as [d.e.f]
            [ g .  h  . i ]    # same as [g.h.i]
            [ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
            )toml";
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(std::string_view(ssUTF8String)));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            # [x] you
            # [x.y] don't
            # [x.y.z] need these
            [x.y.z.w] # for this to work
            [x] # defining a super-table afterward is ok
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            # VALID BUT DISCOURAGED
            [fruit.apple]
            [animal]
            [fruit.orange]
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple.texture]  # you can add sub-tables
            )toml"));
}

TEST(SpecialCases, TableArrays)
{
    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [[products]]
            name = "Hammer"
            sku = 738594937
            [[products]]  # empty table within the array
            [[products]]
            name = "Nail"
            sku = 284758393
            color = "gray"
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [[fruits]]
            name = "apple"
            [fruits.physical]  # subtable
            color = "red"
            shape = "round"
            [[fruits.varieties]]  # nested array of tables
            name = "red delicious"
            [[fruits.varieties]]
            name = "granny smith"
            [[fruits]]
            name = "banana"
            [[fruits.varieties]]
            name = "plantain"
            )toml"));

    EXPECT_NO_THROW(sdv::toml::simple_parser::CParser(R"toml(
            points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ]
            )toml"));
}

TEST(ErrorCases, KeyValue)
{
    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(key = # node_invalid)toml"), std::exception);

    EXPECT_THROW(
        sdv::toml::simple_parser::CParser(R"toml(first = "Tom" last = "Preston-Werner" # node_invalid)toml"), std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(= "no key name" # node_invalid)toml"), std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            name = "Tom"
            name = "Pradyun"
        )toml"),
        std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            fruit . flavor = "banana"   # same as fruit.flavor
            fruit.flavor = "banana"
        )toml"),
        std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            spelling = "favorite"
            "spelling" = "favourite"
        )toml"),
        std::exception);


    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            # This defines the value of fruit.apple to be an integer.
            fruit.apple = 1
            # But then this treats fruit.apple like it's a table.
            # You can't turn an integer into a table.
            fruit.apple.smooth = true
        )toml"),
        std::exception);
}

TEST(ErrorCases, Tables)
{
    std::string ssUTF8String(u8R"toml(
            [ j . "ʞ" . 'l' ]
            [j."ʞ".'l']
            )toml");
    EXPECT_THROW(sdv::toml::simple_parser::CParser(std::string_view(ssUTF8String)), std::exception);

    ssUTF8String = u8R"toml(
            [ j . "ʞ" . 'l' ]
            ["j".'ʞ'."l"]
            )toml";
    EXPECT_THROW(sdv::toml::simple_parser::CParser(std::string_view(ssUTF8String)), std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [fruit]
            apple = "red"
            [fruit]
            orange = "orange"
            )toml"),
        std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [fruit]
            apple = "red"
            [fruit.apple]
            texture = "smooth"
            )toml"),
        std::exception);

    // These two tests are not covered with current implementation.
    //EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
    //        [fruit]
    //        apple.color = "red"
    //        apple.taste.sweet = true
    //        [fruit.apple]  # INVALID
    //        )toml"),
    //    std::exception);
    //EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
    //        [fruit]
    //        apple.color = "red"
    //        apple.taste.sweet = true
    //        [fruit.apple.taste]  # INVALID
    //        )toml"),
    //    std::exception);
}

TEST(ErrorCases, InlineTables)
{
    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            type = { name = "Nail" }
            type.edible = false  # INVALID
            )toml"),
        std::exception);
    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [product]
            type.name = "Nail"
            type = { edible = false }  # INVALID
            )toml"),
        std::exception);
}

TEST(ErrorCases, TableArrays)
{
    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [fruit.physical]  # subtable, but to which parent element should it belong?
            color = "red"
            shape = "round"
            [[fruit]]  # parser must throw an error upon discovering that "fruit" is
                       # an array rather than a table
            name = "apple"
            )toml"),
        std::exception);

    // The following test is not covered with the current implementation
    //EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
    //        fruits = []
    //        [[fruits]] # Not allowed
    //        )toml"),
    //    std::exception);

    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [[fruits]]
            name = "apple"
            [[fruits.varieties]]
            name = "red delicious"
            # INVALID: This table conflicts with the previous array of tables
            [fruits.varieties]
            name = "granny smith"
            )toml"),
        std::exception);
    EXPECT_THROW(sdv::toml::simple_parser::CParser(R"toml(
            [[fruits]]
            name = "apple"
            [fruits.physical]
            color = "red"
            shape = "round"
            # INVALID: This array of tables conflicts with the previous table
            [[fruits.physical]]
            color = "green"
            )toml"),
        std::exception);
}

TEST(Ordering, Array)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        array = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    )toml");
    auto sRoot = parser.Root();

    auto two       = sRoot.GetDirect("array[2]");
    auto eleven    = sRoot.GetDirect("array[11]");
    const auto arr = sRoot.GetDirect("array");

    // with direct access
    ASSERT_TRUE(two);
    EXPECT_EQ(two.GetValue<int>(), 2);
    ASSERT_TRUE(eleven);
    EXPECT_EQ(eleven.GetValue<int>(), 11);

    // with indirect access through iterating
    ASSERT_TRUE(arr);
    EXPECT_EQ(arr.GetArray().size(), 12u);
    size_t nIndex = 0;
    for (const auto& rsNode : arr.GetArray())
        EXPECT_EQ(rsNode.GetValue<size_t>(), nIndex++);
}

TEST(Ordering, TableAray)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [[tableArray]]
        a = 0
        [[tableArray]]
        a = 1
        [[tableArray]]
        a = 2
        [[tableArray]]
        a = 3
        [[tableArray]]
        a = 4
        [[tableArray]]
        a = 5
        [[tableArray]]
        a = 6
        [[tableArray]]
        a = 7
        [[tableArray]]
        a = 8
        [[tableArray]]
        a = 9
        [[tableArray]]
        a = 10
        [[tableArray]]
        a = 11
    )toml");
    auto sRoot = parser.Root();

    auto tableArray = sRoot.GetDirect("tableArray");

    ASSERT_TRUE(tableArray);
    EXPECT_EQ(tableArray.GetArray().size(), 12u);
    size_t nIndex = 0;
    for (const auto& rsNode : tableArray.GetArray())
        EXPECT_EQ(rsNode.GetDirect("a").GetValue<size_t>(), nIndex++);
}

TEST(Ordering, TableArayWithTables)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [topTable]
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 0
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 1
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 2
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 3
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 4
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 5
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 6
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 7
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 8
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 9
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 10
        [[topTable.tableArray]]
        [topTable.tableArray.MyTable]
        a = 11
    )toml");
    auto sRoot = parser.Root();

    auto tableArray = sRoot.GetDirect("topTable.tableArray");

    ASSERT_TRUE(tableArray);
    EXPECT_EQ(tableArray.GetArray().size(), 12u);
    size_t nIndex = 0;
    for (const auto& rsNode : tableArray.GetArray())
        EXPECT_EQ(rsNode.GetDirect("MyTable.a").GetValue<size_t>(), nIndex++);
}

TEST(Ordering, NodeGetDirect)
{
    sdv::toml::simple_parser::CParser parser(R"toml(
        [[table.test]]
        a = 2
        b = 1.2
        [[table.test]]
        a = 4
        c = false
        [[table.test]]
        a = "five"
        d = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 }]
        )toml");
    auto sRoot = parser.Root();

    auto table_test_1_a = sRoot.GetDirect("table.test[0].a");
    auto table_test_1_b = sRoot.GetDirect("table.test[0].b");
    auto table_test_2_a = sRoot.GetDirect("table.test[1].a");
    auto table_test_2_c = sRoot.GetDirect("table.test[1].c");
    auto table_test_3_a = sRoot.GetDirect("table.test[2].a");
    auto table_test_3_d = sRoot.GetDirect("table.test[2].d");

    EXPECT_TRUE(table_test_1_a);
    EXPECT_TRUE(table_test_1_b);
    EXPECT_EQ(table_test_3_d.GetType(), sdv::toml::simple_parser::ENodeType::node_array);

    auto table_test_3 = sRoot.GetDirect("table.test[2]");
    auto table_test_3_2nd = table_test_3.GetDirect("d[2].x");
    ASSERT_TRUE(table_test_3_2nd);
}

