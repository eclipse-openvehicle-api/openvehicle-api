#include <gtest/gtest.h>
#include <support/toml.h>
#include <support/sdv_core.h>
#include <support/app_control.h>
#include "../../../global/process_watchdog.h"

#ifdef _MSC_VER
#pragma warning(disable : 4566)
#endif

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(Config, Instantiate)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config;
    EXPECT_TRUE(config.Process(""));

    config.Clear();
    appcontrol.Shutdown();
}

TEST(RecognizeTypes, Table)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        [newTable]
        [secondTable.nestedTable]
        )");
    EXPECT_TRUE(config.IsValid());

    auto table1 = config.GetDirect("newTable");
    EXPECT_TRUE(table1);
    EXPECT_EQ(table1.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table1.GetName(), "newTable");
    EXPECT_EQ(table1.GetValue(), sdv::any_t());
    sdv::toml::CNodeCollection collection = table1;
    EXPECT_TRUE(collection);

    sdv::toml::CNodeCollection table2 = config.GetDirect("secondTable");
    EXPECT_TRUE(table2);
    EXPECT_EQ(table2.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table2.GetName(), "secondTable");
    EXPECT_EQ(table2.GetValue(), sdv::any_t());

    sdv::toml::CNodeCollection table3 = config.GetDirect("secondTable.nestedTable");
    EXPECT_TRUE(table3);
    EXPECT_EQ(table3.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table3.GetName(), "nestedTable");
    EXPECT_EQ(table3.GetValue(), sdv::any_t());

    config.Clear();
    appcontrol.Shutdown();
}

TEST(RecognizeTypes, Key_Value)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        name = "Hammer"
        id = 42
        pi = 3.1415926
        boolean = true
        array = []
        table = {}
        )");


    auto value_name	   = config.GetDirect("name");
    EXPECT_EQ(value_name.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(value_name.GetValue(), sdv::any_t("Hammer"));

    auto value_id	   = config.GetDirect("id");
    EXPECT_EQ(value_id.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(value_id.GetValue(), 42);

    auto value_pi	   = config.GetDirect("pi");
    EXPECT_EQ(value_pi.GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(value_pi.GetValue(), 3.1415926);

    auto value_boolean = config.GetDirect("boolean");
    EXPECT_EQ(value_boolean.GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(value_boolean.GetValue(), true);

    auto value_array   = config.GetDirect("array");
    EXPECT_EQ(value_array.GetType(), sdv::toml::ENodeType::node_array);
    EXPECT_EQ(value_array.GetValue(), sdv::any_t());

    auto value_table   = config.GetDirect("table");
    EXPECT_EQ(value_table.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(value_table.GetValue(), sdv::any_t());

    config.Clear();
    appcontrol.Shutdown();
}

TEST(RecognizeTypes, TableArray)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        [[newTableArray]]
        [[newTableArray]]
        [[table.nestedTableArray]]
        )");

    sdv::toml::CNodeCollection tableArray1 = config.GetDirect("newTableArray");
    EXPECT_EQ(tableArray1.GetType(), sdv::toml::ENodeType::node_array);
    EXPECT_TRUE(tableArray1);
    EXPECT_EQ(tableArray1.GetName(), "newTableArray");
    EXPECT_EQ(tableArray1.GetCount(), 2u);
    auto tableNode0 = tableArray1[0];
    EXPECT_TRUE(tableNode0);
    auto tableNode0b = tableArray1.Get(0);
    EXPECT_TRUE(tableNode0b);
    auto tableNode1 = tableArray1[1];
    EXPECT_TRUE(tableNode1);
    auto tableNode1b = tableArray1.Get(1);
    EXPECT_TRUE(tableNode1b);
    EXPECT_FALSE(tableArray1[2]);
    EXPECT_FALSE(tableArray1.Get(2));

    auto table1 = config.GetDirect("newTableArray[0]");
    EXPECT_EQ(table1.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_TRUE(table1.GetName().empty());

    auto table2 = config.GetDirect("newTableArray[1]");
    EXPECT_EQ(table2.GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_TRUE(table2.GetName().empty());

    config.Clear();
    appcontrol.Shutdown();
}

TEST(NestedContent, Array)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        arr_mixed = [ 1.0, 2, "test string", [ 1, 2, ], { pi = 3.14, e = 2.71828 }, true]
        arr_ints = [ 1, 2, 3, 4]
        arr_ints_trailing_comma = [ 1, 2, 3, 4, ]
        arr_multiline = [
            "first line",
            "second line",
            "third_line",
        ]
        )");

    {

        sdv::toml::CNodeCollection array_ints	  = config.GetDirect("arr_ints");
        EXPECT_EQ(array_ints.GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_TRUE(array_ints);
        EXPECT_EQ(array_ints.GetCount(), 4u);
        EXPECT_TRUE(array_ints[0]);
        EXPECT_TRUE(array_ints[1]);
        EXPECT_TRUE(array_ints[2]);
        EXPECT_TRUE(array_ints[3]);
        EXPECT_FALSE(array_ints[4]);
        auto array_ints_0 = config.GetDirect("arr_ints[0]");
        ASSERT_TRUE(array_ints_0);
        EXPECT_EQ(array_ints_0.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_0.GetValue(), 1);
        auto array_ints_1 = config.GetDirect("arr_ints[1]");
        ASSERT_TRUE(array_ints_1);
        EXPECT_EQ(array_ints_1.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_1.GetValue(), 2);
        auto array_ints_2 = config.GetDirect("arr_ints[2]");
        ASSERT_TRUE(array_ints_2);
        EXPECT_EQ(array_ints_2.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_2.GetValue(), 3);
        auto array_ints_3 = config.GetDirect("arr_ints[3]");
        ASSERT_TRUE(array_ints_3);
        EXPECT_EQ(array_ints_3.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_3.GetValue(), 4);
        auto array_ints_4 = config.GetDirect("arr_ints[4]");
        EXPECT_FALSE(array_ints_4);
    }

    {
        auto array_ints_trailing_comma	 = config.GetDirect("arr_ints_trailing_comma");
        auto array_ints_trailing_comma_0 = config.GetDirect("arr_ints_trailing_comma[0]");
        auto array_ints_trailing_comma_1 = config.GetDirect("arr_ints_trailing_comma[1]");
        auto array_ints_trailing_comma_2 = config.GetDirect("arr_ints_trailing_comma[2]");
        auto array_ints_trailing_comma_3 = config.GetDirect("arr_ints_trailing_comma[3]");
        auto array_ints_trailing_comma_4 = config.GetDirect("arr_ints_trailing_comma[4]");

        EXPECT_EQ(array_ints_trailing_comma.GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_TRUE(array_ints_trailing_comma_0);
        EXPECT_EQ(array_ints_trailing_comma_0.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_0.GetValue(), 1);
        ASSERT_TRUE(array_ints_trailing_comma_1);
        EXPECT_EQ(array_ints_trailing_comma_1.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_1.GetValue(), 2);
        ASSERT_TRUE(array_ints_trailing_comma_2);
        EXPECT_EQ(array_ints_trailing_comma_2.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_2.GetValue(), 3);
        ASSERT_TRUE(array_ints_trailing_comma_3);
        EXPECT_EQ(array_ints_trailing_comma_3.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_3.GetValue(), 4);
        EXPECT_FALSE(array_ints_trailing_comma_4);
    }

    {
        auto array_mixed	  = config.GetDirect("arr_mixed");
        auto array_mixed_0	  = config.GetDirect("arr_mixed[0]");
        auto array_mixed_1	  = config.GetDirect("arr_mixed[1]");
        auto array_mixed_2	  = config.GetDirect("arr_mixed[2]");
        auto array_mixed_3	  = config.GetDirect("arr_mixed[3]");
        auto array_mixed_3_2  = config.GetDirect("arr_mixed[3][1]");
        auto array_mixed_4	  = config.GetDirect("arr_mixed[4]");
        auto array_mixed_4_pi = config.GetDirect("arr_mixed[4].pi");
        auto array_mixed_5	  = config.GetDirect("arr_mixed[5]");
        auto array_mixed_6	  = config.GetDirect("arr_mixed[6]");

        EXPECT_EQ(array_mixed.GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_TRUE(array_mixed_0);
        EXPECT_EQ(array_mixed_0.GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(array_mixed_0.GetValue(), 1.0);
        ASSERT_TRUE(array_mixed_1);
        EXPECT_EQ(array_mixed_1.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_mixed_1.GetValue(), 2);
        ASSERT_TRUE(array_mixed_2);
        EXPECT_EQ(array_mixed_2.GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_mixed_2.GetValue()), "test string");
        ASSERT_TRUE(array_mixed_3);
        EXPECT_EQ(array_mixed_3.GetType(), sdv::toml::ENodeType::node_array);
        EXPECT_EQ(array_mixed_3_2.GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_mixed_3_2.GetValue(), 2);
        ASSERT_TRUE(array_mixed_4);
        EXPECT_EQ(array_mixed_4.GetType(), sdv::toml::ENodeType::node_table);
        EXPECT_EQ(array_mixed_4_pi.GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(array_mixed_4_pi.GetValue(), 3.14);
        ASSERT_TRUE(array_mixed_5);
        EXPECT_EQ(array_mixed_5.GetType(), sdv::toml::ENodeType::node_boolean);
        EXPECT_EQ(array_mixed_5.GetValue(), true);
        EXPECT_EQ(array_mixed_5.GetValue(), sdv::any_t());
        EXPECT_FALSE(array_mixed_6);
    }

    {
        auto array_multiline   = config.GetDirect("arr_multiline");
        auto array_multiline_0 = config.GetDirect("arr_multiline[0]");
        auto array_multiline_1 = config.GetDirect("arr_multiline[1]");
        auto array_multiline_2 = config.GetDirect("arr_multiline[2]");
        auto array_multiline_3 = config.GetDirect("arr_multiline[3]");

        EXPECT_EQ(array_multiline.GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_TRUE(array_multiline_0);
        EXPECT_EQ(array_multiline_0.GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_0.GetValue()), "first line");
        ASSERT_TRUE(array_multiline_1);
        EXPECT_EQ(array_multiline_1.GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_1.GetValue()), "second line");
        ASSERT_TRUE(array_multiline_2);
        EXPECT_EQ(array_multiline_2.GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_2.GetValue()), "third_line");
        EXPECT_FALSE(array_multiline_3);
    }

    config.Clear();
    appcontrol.Shutdown();
}

TEST(NestedContent, Table)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        [table]
        a = 2
        b = 1.2
        [anotherTable]
        a = 4
        c = false
        [thirdTable.fourthTable]
        a = "five"
        d = []
        )");

    auto table_a		= config.GetDirect("table.a");
    auto table_b		= config.GetDirect("table.b");
    auto anotherTable_a = config.GetDirect("anotherTable.a");
    auto anotherTable_c = config.GetDirect("anotherTable.c");
    auto fourthTable_a	= config.GetDirect("thirdTable.fourthTable.a");
    auto fourthTable_d	= config.GetDirect("thirdTable.fourthTable.d");

    ASSERT_TRUE(table_a);
    EXPECT_EQ(table_a.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_a.GetValue(), 2);
    ASSERT_TRUE(table_b);
    EXPECT_EQ(table_b.GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table_b.GetValue(), 1.2);
    ASSERT_TRUE(anotherTable_a);
    EXPECT_EQ(anotherTable_a.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(anotherTable_a.GetValue(), 4);
    ASSERT_TRUE(anotherTable_c);
    EXPECT_EQ(anotherTable_c.GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(anotherTable_c.GetValue(), false);
    ASSERT_TRUE(fourthTable_a);
    EXPECT_EQ(fourthTable_a.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(fourthTable_a.GetValue()), "five");
    ASSERT_TRUE(fourthTable_d);
    EXPECT_EQ(fourthTable_d.GetType(), sdv::toml::ENodeType::node_array);

    config.Clear();
    appcontrol.Shutdown();
}

TEST(NestedContent, TableArray)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        [[table.test]]
        a = 2
        b = 1.2
        [[table.test]]
        a = 4
        c = false
        [[table.test]]
        a = "five"
        d = []
        )");

    auto table_test_1_a = config.GetDirect("table.test[0].a");
    auto table_test_1_b = config.GetDirect("table.test[0].b");
    auto table_test_2_a = config.GetDirect("table.test[1].a");
    auto table_test_2_c = config.GetDirect("table.test[1].c");
    auto table_test_3_a = config.GetDirect("table.test[2].a");
    auto table_test_3_d = config.GetDirect("table.test[2].d");

    ASSERT_TRUE(table_test_1_a);
    EXPECT_EQ(table_test_1_a.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_test_1_a.GetValue(), 2);
    ASSERT_TRUE(table_test_1_b);
    EXPECT_EQ(table_test_1_b.GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table_test_1_b.GetValue(), 1.2);
    ASSERT_TRUE(table_test_2_a);
    EXPECT_EQ(table_test_2_a.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_test_2_a.GetValue(), 4);
    ASSERT_TRUE(table_test_2_c);
    EXPECT_EQ(table_test_2_c.GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(table_test_2_c.GetValue(), false);
    ASSERT_TRUE(table_test_3_a);
    EXPECT_EQ(table_test_3_a.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table_test_3_a.GetValue()), "five");
    ASSERT_TRUE(table_test_3_d);
    EXPECT_EQ(table_test_3_d.GetType(), sdv::toml::ENodeType::node_array);

    config.Clear();
    appcontrol.Shutdown();
}

TEST(NestedContent, InlineTable)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        table1 = { a = 0, b = 1.2, c = "string" }
        table2 = { a = [], b = true, e = 2.71828 }
        table3 = { a = { a = "a", b = "A" }, b = {a = "b", b = "B"}, e = {a = "e", b = "E"} }
        )");

    auto table1_a	= config.GetDirect("table1.a");
    auto table1_b	= config.GetDirect("table1.b");
    auto table1_c	= config.GetDirect("table1.c");
    auto table2_a	= config.GetDirect("table2.a");
    auto table2_b	= config.GetDirect("table2.b");
    auto table2_e	= config.GetDirect("table2.e");
    auto table3_a_a = config.GetDirect("table3.a.a");
    auto table3_a_b = config.GetDirect("table3.a.b");
    auto table3_b_a = config.GetDirect("table3.b.a");
    auto table3_b_b = config.GetDirect("table3.b.b");
    auto table3_e_a = config.GetDirect("table3.e.a");
    auto table3_e_b = config.GetDirect("table3.e.b");

    ASSERT_TRUE(table1_a);
    EXPECT_EQ(table1_a.GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table1_a.GetValue(), 0);
    ASSERT_TRUE(table1_b);
    EXPECT_EQ(table1_b.GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table1_b.GetValue(), 1.2);
    ASSERT_TRUE(table1_c);
    EXPECT_EQ(table1_c.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table1_c.GetValue()), "string");
    ASSERT_TRUE(table2_a);
    EXPECT_EQ(table2_a.GetType(), sdv::toml::ENodeType::node_array);
    ASSERT_TRUE(table2_b);
    EXPECT_EQ(table2_b.GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(table2_b.GetValue(), true);
    ASSERT_TRUE(table2_e);
    EXPECT_EQ(table2_e.GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table2_e.GetValue(), 2.71828);
    ASSERT_TRUE(table3_a_a);
    EXPECT_EQ(table3_a_a.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_a_a.GetValue()), "a");
    ASSERT_TRUE(table3_a_b);
    EXPECT_EQ(table3_a_b.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_a_b.GetValue()), "A");
    ASSERT_TRUE(table3_b_a);
    EXPECT_EQ(table3_b_a.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_b_a.GetValue()), "b");
    ASSERT_TRUE(table3_b_b);
    EXPECT_EQ(table3_b_b.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_b_b.GetValue()), "B");
    ASSERT_TRUE(table3_e_a);
    EXPECT_EQ(table3_e_a.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_e_a.GetValue()), "e");
    ASSERT_TRUE(table3_e_b);
    EXPECT_EQ(table3_e_b.GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_e_b.GetValue()), "E");

    config.Clear();
    appcontrol.Shutdown();
}

TEST(SpecialCases, Keys)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_TRUE(sdv::toml::CTOMLParser(u8R"(
            "127.0.0.1" = "value"
            "character encoding" = "value"
            "ʎǝʞ" = "value"
            'key2' = "value"
            'quoted "value"' = "value"
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            key = "value"
            bare_key = "value"
            bare-key = "value"
            1234 = "value"
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            "" = "blank"     # VALID but discouraged
        )"));
    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            '' = 'blank'     # VALID but discouraged
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            name = "Orange"
            physical.color = "orange"
            physical.shape = "round"
            site."google.com" = true
        )"));


    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            fruit.name = "banana"     # this is best practice
            fruit. color = "yellow"    # same as fruit.color
            fruit . flavor = "banana"   # same as fruit.flavor
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            # This makes the key "fruit" into a table.
            fruit.apple.smooth = true
            # So then you can add to the table "fruit" like so:
            fruit.orange = 2
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            # VALID BUT DISCOURAGED
            apple.type = "fruit"
            orange.type = "fruit"
            apple.skin = "thin"
            orange.skin = "thick"
            apple.color = "red"
            orange.color = "orange"
        )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            3.1415 = 3.1415
        )"));
    {
        sdv::toml::CTOMLParser config(R"(
            3.1415 = 3.1415
        )");
        auto   table = config.GetDirect("3");
        auto   pi	 = config.GetDirect("3.1415");
        ASSERT_TRUE(table);
        EXPECT_EQ(table.GetType(), sdv::toml::ENodeType::node_table);
        ASSERT_TRUE(pi);
        EXPECT_EQ(pi.GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(pi.GetValue(), 3.1415);
    }

    appcontrol.Shutdown();
}

TEST(SpecialCases, Arrays)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            integers = [ 1, 2, 3 ]
            colors = [ "red", "yellow", "green" ]
            nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
            nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
            string_array = [ "all", 'strings', """are the same""", '''type''' ]
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
            contributors = [
            "Foo Bar <foo@example.com>",
            { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
            ]
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            integers3 = [
                1,
                2, # this is ok
            ]
            )"));

    appcontrol.Shutdown();
}

TEST(SpecialCases, Tables)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            [table-1]
            key1 = "some string"
            key2 = 123

            [table-2]
            key1 = "another string"
            key2 = 456
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            [dog."tater.man"]
            type.name = "pug"
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(u8R"(
            [a.b.c]            # this is best practice
            [ d.e.f ]          # same as [d.e.f]
            [ g .  h  . i ]    # same as [g.h.i]
            [ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            # [x] you
            # [x.y] don't
            # [x.y.z] need these
            [x.y.z.w] # for this to work
            [x] # defining a super-table afterward is ok
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            # VALID BUT DISCOURAGED
            [fruit.apple]
            [animal]
            [fruit.orange]
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple.texture]  # you can add sub-tables
            )"));

    appcontrol.Shutdown();
}

TEST(SpecialCases, TableArrays)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            [[products]]
            name = "Hammer"
            sku = 738594937
            [[products]]  # empty table within the array
            [[products]]
            name = "Nail"
            sku = 284758393
            color = "gray"
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
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
            )"));

    EXPECT_TRUE(sdv::toml::CTOMLParser(R"(
            points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ]
            )"));

    appcontrol.Shutdown();
}

TEST(ErrorCases, KeyValue)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(key = # node_invalid)"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(first = "Tom" last = "Preston-Werner" # node_invalid)"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(= "no key name" # node_invalid)"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            name = "Tom"
            name = "Pradyun"
        )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            fruit . flavor = "banana"   # same as fruit.flavor
            fruit.flavor = "banana"
        )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            spelling = "favorite"
            "spelling" = "favourite"
        )"));


    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            # This defines the value of fruit.apple to be an integer.
            fruit.apple = 1
            # But then this treats fruit.apple like it's a table.
            # You can't turn an integer into a table.
            fruit.apple.smooth = true
        )"));

    appcontrol.Shutdown();
}

TEST(ErrorCases, Tables)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_FALSE(sdv::toml::CTOMLParser(u8R"(
            [ j . "ʞ" . 'l' ]
            [j."ʞ".'l']
            )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [fruit]
            apple = "red"
            [fruit]
            orange = "orange"
            )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [fruit]
            apple = "red"
            [fruit.apple]
            texture = "smooth"
            )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple]  # INVALID
            )"));
    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple.taste]  # INVALID
            )"));

    appcontrol.Shutdown();
}

TEST(ErrorCases, InlineTables)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            type = { name = "Nail" }
            type.edible = false  # INVALID
            )"));
    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [product]
            type.name = "Nail"
            type = { edible = false }  # INVALID
            )"));

    appcontrol.Shutdown();
}

TEST(ErrorCases, TableArrays)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [fruit.physical]  # subtable, but to which parent element should it belong?
            color = "red"
            shape = "round"
            [[fruit]]  # parser must throw an error upon discovering that "fruit" is
                       # an array rather than a table
            name = "apple"
            )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            fruits = []
            [[fruits]] # Not allowed
            )"));

    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [[fruits]]
            name = "apple"
            [[fruits.varieties]]
            name = "red delicious"
            # INVALID: This table conflicts with the previous array of tables
            [fruits.varieties]
            name = "granny smith"
            )"));
    EXPECT_FALSE(sdv::toml::CTOMLParser(R"(
            [[fruits]]
            name = "apple"
            [fruits.physical]
            color = "red"
            shape = "round"
            # INVALID: This array of tables conflicts with the previous table
            [[fruits.physical]]
            color = "green"
            )"));

    appcontrol.Shutdown();
}

TEST(Ordering, Array)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
        array = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    )");

    auto	   two	  = config.GetDirect("array[2]");
    auto	   eleven = config.GetDirect("array[11]");
    const sdv::toml::CNodeCollection arr = config.GetDirect("array");

    // with direct access
    ASSERT_TRUE(two);
    EXPECT_EQ(two.GetValue(), 2);
    ASSERT_TRUE(eleven);
    EXPECT_EQ(eleven.GetValue(), 11);

    // with indirect access through iterating
    ASSERT_TRUE(arr);
    for (std::size_t i = 0; i < arr.GetCount(); ++i)
    {
        EXPECT_EQ(arr[i].GetValue(), (int64_t)i);
    }

    config.Clear();
    appcontrol.Shutdown();
}

TEST(Ordering, TableAray)
{
    sdv::app::CAppControl appcontrol(R"config(
[Application]
Mode = "Essential"

[LogHandler]
ViewFilter = "Fatal"
)config");
    ASSERT_TRUE(appcontrol.IsRunning());

    sdv::toml::CTOMLParser config(R"(
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
    )");

    sdv::toml::CNodeCollection tableArray = config.GetDirect("tableArray");

    ASSERT_TRUE(tableArray);
    for (std::size_t i = 0; i < tableArray.GetCount(); ++i)
    {
        EXPECT_EQ(sdv::toml::CNodeCollection(tableArray[i])[0].GetValue(), (int64_t) i);
    }

    config.Clear();
    appcontrol.Shutdown();
}

