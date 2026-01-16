#include <gtest/gtest.h>
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"

/* Requirements TOML toml_parser::CParser
 * - The output after parsing is a tree structure only if parsing is successful
 * - No recovery will be attempted when syntax errors occur
 * - all errors and features (except Date-Time) as defined at https://toml.io/en/v1.0.0 have to result in an invalid or valid
 *   outcome respectively
 */

TEST(RecognizeTypes, Root)
{
    toml_parser::CParser parser("");
    auto& rroot = parser.Root();
    EXPECT_EQ(rroot.GetParent(), nullptr);
    EXPECT_EQ(rroot.GetIndex(), sdv::toml::npos);
}

TEST(RecognizeTypes, Table)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        [newTable]
        [secondTable.nestedTable]
        )"s);

    auto table1 = parser.Root().Direct("newTable");
    EXPECT_EQ(table1->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table1->GetName(), "newTable");
    EXPECT_EQ(table1->GetValue(), sdv::any_t());
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(table1.get())->GetInterface<sdv::toml::INodeCollection>(), nullptr);

    auto table2 = parser.Root().Direct("secondTable");
    EXPECT_EQ(table2->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table2->GetName(), "secondTable");
    EXPECT_EQ(table2->GetValue(), sdv::any_t());
    EXPECT_NE(((sdv::IInterfaceAccess*) table2.get())->GetInterface<sdv::toml::INodeCollection>(), nullptr);

    auto table3 = parser.Root().Direct("secondTable.nestedTable");
    EXPECT_EQ(table3->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table3->GetName(), "nestedTable");
    EXPECT_EQ(table3->GetValue(), sdv::any_t());
    EXPECT_NE(((sdv::IInterfaceAccess*) table3.get())->GetInterface<sdv::toml::INodeCollection>(), nullptr);
}

TEST(RecognizeTypes, Key_Value)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        name = "Hammer"
        id = 42
        pi = 3.1415926
        boolean = true
        array = []
        table = {}
        )"s);


    auto value_name = parser.Root().Direct("name");
    EXPECT_EQ(value_name->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(value_name->GetValue(), sdv::any_t("Hammer"));
    EXPECT_EQ(value_name->GetIndex(), 0u);

    auto value_id = parser.Root().Direct("id");
    EXPECT_EQ(value_id->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(value_id->GetValue(), 42);
    EXPECT_EQ(value_id->GetIndex(), 1u);

    auto value_pi = parser.Root().Direct("pi");
    EXPECT_EQ(value_pi->GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(value_pi->GetValue(), 3.1415926);
    EXPECT_EQ(value_pi->GetIndex(), 2u);

    auto value_boolean = parser.Root().Direct("boolean");
    EXPECT_EQ(value_boolean->GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(value_boolean->GetValue(), true);
    EXPECT_EQ(value_boolean->GetIndex(), 3u);

    auto value_array = parser.Root().Direct("array");
    EXPECT_EQ(value_array->GetType(), sdv::toml::ENodeType::node_array);
    EXPECT_EQ(value_array->GetValue(), sdv::any_t());
    EXPECT_EQ(value_array->GetIndex(), 4u);

    auto value_table = parser.Root().Direct("table");
    EXPECT_EQ(value_table->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(value_table->GetValue(), sdv::any_t());
    EXPECT_EQ(value_table->GetIndex(), 5u);
}

TEST(RecognizeTypes, TableArray)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        [[newTableArray]]
        [[newTableArray]]
        [[table.nestedTableArray]]
        )"s);

    auto tableArray1 = parser.Root().Direct("newTableArray");
    EXPECT_EQ(tableArray1->GetType(), sdv::toml::ENodeType::node_array);
    EXPECT_NE(static_cast<sdv::IInterfaceAccess*>(tableArray1.get())->GetInterface<sdv::toml::INodeCollection>(), nullptr);
    EXPECT_EQ(tableArray1->GetName(), "newTableArray");
    sdv::toml::INodeCollection* pArrayCollection =
        static_cast<sdv::IInterfaceAccess*>(tableArray1.get())->GetInterface<sdv::toml::INodeCollection>();
    ASSERT_NE(pArrayCollection, nullptr);
    EXPECT_EQ(pArrayCollection->GetCount(), 2u);
    sdv::IInterfaceAccess* pTableNode0 = pArrayCollection->GetNode(0);
    EXPECT_NE(pTableNode0, nullptr);
    sdv::IInterfaceAccess* pTableNode1 = pArrayCollection->GetNode(1);
    EXPECT_NE(pTableNode1, nullptr);
    EXPECT_EQ(pArrayCollection->GetNode(2), nullptr);

    auto table1 = parser.Root().Direct("newTableArray[0]");
    ASSERT_TRUE(table1);
    EXPECT_EQ(table1->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(static_cast<sdv::IInterfaceAccess*>(table1.get()), pTableNode0);
    EXPECT_EQ(table1->GetName(), "newTableArray");

    auto table2 = parser.Root().Direct("newTableArray[1]");
    ASSERT_TRUE(table2);
    EXPECT_EQ(static_cast<sdv::IInterfaceAccess*>(table2.get()), pTableNode1);
    EXPECT_EQ(table2->GetType(), sdv::toml::ENodeType::node_table);
    EXPECT_EQ(table2->GetName(), "newTableArray");
}

TEST(NestedContent, Array)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        arr_mixed = [ 1.0, 2, "test string", [ 1, 2 ], { pi = 3.14, e = 2.71828 }, true]
        arr_ints = [ 1, 2, 3, 4]
        arr_ints_trailing_comma = [ 1, 2, 3, 4, ]
        arr_multiline = [
            "first line",
            "second line",
            "third_line",
        ]
        )"s);

    {

        auto array_ints	  = parser.Root().Direct("arr_ints");
        EXPECT_EQ(array_ints->GetType(), sdv::toml::ENodeType::node_array);
        sdv::toml::INodeCollection* pIntArrayCollection =
            static_cast<sdv::IInterfaceAccess*>(array_ints.get())->GetInterface<sdv::toml::INodeCollection>();
        ASSERT_NE(pIntArrayCollection, nullptr);
        EXPECT_EQ(pIntArrayCollection->GetCount(), 4u);
        EXPECT_NE(pIntArrayCollection->GetNode(0), nullptr);
        EXPECT_NE(pIntArrayCollection->GetNode(1), nullptr);
        EXPECT_NE(pIntArrayCollection->GetNode(2), nullptr);
        EXPECT_NE(pIntArrayCollection->GetNode(3), nullptr);
        EXPECT_EQ(pIntArrayCollection->GetNode(4), nullptr);
        auto array_ints_0 = parser.Root().Direct("arr_ints[0]");
        ASSERT_NE(array_ints_0, nullptr);
        EXPECT_EQ(array_ints_0->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_0->GetValue(), 1);
        EXPECT_EQ(array_ints_0->GetIndex(), 0u);
        auto array_ints_1 = parser.Root().Direct("arr_ints[1]");
        ASSERT_NE(array_ints_1, nullptr);
        EXPECT_EQ(array_ints_1->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_1->GetValue(), 2);
        EXPECT_EQ(array_ints_1->GetIndex(), 1u);
        auto array_ints_2 = parser.Root().Direct("arr_ints[2]");
        ASSERT_NE(array_ints_2, nullptr);
        EXPECT_EQ(array_ints_2->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_2->GetValue(), 3);
        EXPECT_EQ(array_ints_2->GetIndex(), 2u);
        auto array_ints_3 = parser.Root().Direct("arr_ints[3]");
        ASSERT_NE(array_ints_3, nullptr);
        EXPECT_EQ(array_ints_3->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_3->GetValue(), 4);
        EXPECT_EQ(array_ints_3->GetIndex(), 3u);
        auto array_ints_4 = parser.Root().Direct("arr_ints[4]");
        EXPECT_EQ(array_ints_4, nullptr);
    }

    {
        auto array_ints_trailing_comma	 = parser.Root().Direct("arr_ints_trailing_comma");
        auto array_ints_trailing_comma_0 = parser.Root().Direct("arr_ints_trailing_comma[0]");
        auto array_ints_trailing_comma_1 = parser.Root().Direct("arr_ints_trailing_comma[1]");
        auto array_ints_trailing_comma_2 = parser.Root().Direct("arr_ints_trailing_comma[2]");
        auto array_ints_trailing_comma_3 = parser.Root().Direct("arr_ints_trailing_comma[3]");
        auto array_ints_trailing_comma_4 = parser.Root().Direct("arr_ints_trailing_comma[4]");

        EXPECT_EQ(array_ints_trailing_comma->GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_NE(array_ints_trailing_comma_0, nullptr);
        EXPECT_EQ(array_ints_trailing_comma_0->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_0->GetValue(), 1);
        EXPECT_EQ(array_ints_trailing_comma_0->GetIndex(), 0u);
        ASSERT_NE(array_ints_trailing_comma_1, nullptr);
        EXPECT_EQ(array_ints_trailing_comma_1->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_1->GetValue(), 2);
        EXPECT_EQ(array_ints_trailing_comma_1->GetIndex(), 1u);
        ASSERT_NE(array_ints_trailing_comma_2, nullptr);
        EXPECT_EQ(array_ints_trailing_comma_2->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_2->GetValue(), 3);
        EXPECT_EQ(array_ints_trailing_comma_2->GetIndex(), 2u);
        ASSERT_NE(array_ints_trailing_comma_3, nullptr);
        EXPECT_EQ(array_ints_trailing_comma_3->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_ints_trailing_comma_3->GetValue(), 4);
        EXPECT_EQ(array_ints_trailing_comma_3->GetIndex(), 3u);
        EXPECT_EQ(array_ints_trailing_comma_4, nullptr);
    }

    {
        auto array_mixed	  = parser.Root().Direct("arr_mixed");
        auto array_mixed_0	  = parser.Root().Direct("arr_mixed[0]");
        auto array_mixed_1	  = parser.Root().Direct("arr_mixed[1]");
        auto array_mixed_2	  = parser.Root().Direct("arr_mixed[2]");
        auto array_mixed_3	  = parser.Root().Direct("arr_mixed[3]");
        auto array_mixed_3_1  = parser.Root().Direct("arr_mixed[3][0]");
        auto array_mixed_3_2  = parser.Root().Direct("arr_mixed[3][1]");
        auto array_mixed_4	  = parser.Root().Direct("arr_mixed[4]");
        auto array_mixed_4_pi = parser.Root().Direct("arr_mixed[4].pi");
        auto array_mixed_4_e = parser.Root().Direct("arr_mixed[4].e");
        auto array_mixed_5	  = parser.Root().Direct("arr_mixed[5]");
        auto array_mixed_6	  = parser.Root().Direct("arr_mixed[6]");

        EXPECT_EQ(array_mixed->GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_NE(array_mixed_0, nullptr);
        EXPECT_EQ(array_mixed_0->GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(array_mixed_0->GetValue(), 1.0);
        EXPECT_EQ(array_mixed_0->GetIndex(), 0u);
        ASSERT_NE(array_mixed_1, nullptr);
        EXPECT_EQ(array_mixed_1->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_mixed_1->GetValue(), 2);
        EXPECT_EQ(array_mixed_1->GetIndex(), 1u);
        ASSERT_NE(array_mixed_2, nullptr);
        EXPECT_EQ(array_mixed_2->GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_mixed_2->GetValue()), "test string");
        EXPECT_EQ(array_mixed_2->GetIndex(), 2u);
        ASSERT_NE(array_mixed_3, nullptr);
        EXPECT_EQ(array_mixed_3->GetType(), sdv::toml::ENodeType::node_array);
        EXPECT_EQ(array_mixed_3->GetIndex(), 3u);
        EXPECT_EQ(array_mixed_3_1->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_mixed_3_1->GetValue(), 1);
        EXPECT_EQ(array_mixed_3_1->GetIndex(), 0u);
        EXPECT_EQ(array_mixed_3_2->GetType(), sdv::toml::ENodeType::node_integer);
        EXPECT_EQ(array_mixed_3_2->GetValue(), 2);
        EXPECT_EQ(array_mixed_3_2->GetIndex(), 1u);
        ASSERT_NE(array_mixed_4, nullptr);
        EXPECT_EQ(array_mixed_4->GetType(), sdv::toml::ENodeType::node_table);
        EXPECT_EQ(array_mixed_4->GetIndex(), 4u);
        EXPECT_EQ(array_mixed_4_pi->GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(array_mixed_4_pi->GetValue(), 3.14);
        EXPECT_EQ(array_mixed_4_pi->GetIndex(), 0u);
        EXPECT_EQ(array_mixed_4_e->GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(array_mixed_4_e->GetValue(), 2.71828);
        EXPECT_EQ(array_mixed_4_e->GetIndex(), 1u);
        ASSERT_NE(array_mixed_5, nullptr);
        EXPECT_EQ(array_mixed_5->GetType(), sdv::toml::ENodeType::node_boolean);
        EXPECT_EQ(array_mixed_5->GetValue(), true);
        EXPECT_EQ(array_mixed_5->GetValue(), sdv::any_t());
        EXPECT_EQ(array_mixed_5->GetIndex(), 5u);
        EXPECT_EQ(array_mixed_6, nullptr);
    }

    {
        auto array_multiline   = parser.Root().Direct("arr_multiline");
        auto array_multiline_0 = parser.Root().Direct("arr_multiline[0]");
        auto array_multiline_1 = parser.Root().Direct("arr_multiline[1]");
        auto array_multiline_2 = parser.Root().Direct("arr_multiline[2]");
        auto array_multiline_3 = parser.Root().Direct("arr_multiline[3]");

        EXPECT_EQ(array_multiline->GetType(), sdv::toml::ENodeType::node_array);
        ASSERT_NE(array_multiline_0, nullptr);
        EXPECT_EQ(array_multiline_0->GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_0->GetValue()), "first line");
        EXPECT_EQ(array_multiline_0->GetIndex(), 0u);
        ASSERT_NE(array_multiline_1, nullptr);
        EXPECT_EQ(array_multiline_1->GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_1->GetValue()), "second line");
        EXPECT_EQ(array_multiline_1->GetIndex(), 1u);
        ASSERT_NE(array_multiline_2, nullptr);
        EXPECT_EQ(array_multiline_2->GetType(), sdv::toml::ENodeType::node_string);
        EXPECT_EQ(static_cast<std::string>(array_multiline_2->GetValue()), "third_line");
        EXPECT_EQ(array_multiline_2->GetIndex(), 2u);
        EXPECT_EQ(array_multiline_3, nullptr);
    }
}

TEST(NestedContent, Table)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        [table]
        a = 2
        b = 1.2
        [anotherTable]
        a = 4
        c = false
        [thirdTable.fourthTable]
        a = "five"
        d = []
        )"s);

    auto table_a		= parser.Root().Direct("table.a");
    auto table_b		= parser.Root().Direct("table.b");
    auto anotherTable_a = parser.Root().Direct("anotherTable.a");
    auto anotherTable_c = parser.Root().Direct("anotherTable.c");
    auto fourthTable_a	= parser.Root().Direct("thirdTable.fourthTable.a");
    auto fourthTable_d	= parser.Root().Direct("thirdTable.fourthTable.d");

    ASSERT_NE(table_a, nullptr);
    EXPECT_EQ(table_a->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_a->GetValue(), 2);
    ASSERT_NE(table_b, nullptr);
    EXPECT_EQ(table_b->GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table_b->GetValue(), 1.2);
    ASSERT_NE(anotherTable_a, nullptr);
    EXPECT_EQ(anotherTable_a->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(anotherTable_a->GetValue(), 4);
    ASSERT_NE(anotherTable_c, nullptr);
    EXPECT_EQ(anotherTable_c->GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(anotherTable_c->GetValue(), false);
    ASSERT_NE(fourthTable_a, nullptr);
    EXPECT_EQ(fourthTable_a->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(fourthTable_a->GetValue()), "five");
    ASSERT_NE(fourthTable_d, nullptr);
    EXPECT_EQ(fourthTable_d->GetType(), sdv::toml::ENodeType::node_array);
}

TEST(NestedContent, TableArray)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        [[table.test]]
        a = 2
        b = 1.2
        [[table.test]]
        a = 4
        c = false
        [[table.test]]
        a = "five"
        d = []
        )"s);

    auto table_test_1_a = parser.Root().Direct("table.test[0].a");
    auto table_test_1_b = parser.Root().Direct("table.test[0].b");
    auto table_test_2_a = parser.Root().Direct("table.test[1].a");
    auto table_test_2_c = parser.Root().Direct("table.test[1].c");
    auto table_test_3_a = parser.Root().Direct("table.test[2].a");
    auto table_test_3_d = parser.Root().Direct("table.test[2].d");

    ASSERT_NE(table_test_1_a, nullptr);
    EXPECT_EQ(table_test_1_a->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_test_1_a->GetValue(), 2);
    EXPECT_EQ(table_test_1_a->GetIndex(), 0u);
    ASSERT_NE(table_test_1_b, nullptr);
    EXPECT_EQ(table_test_1_b->GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table_test_1_b->GetValue(), 1.2);
    EXPECT_EQ(table_test_1_b->GetIndex(), 1u);
    ASSERT_NE(table_test_2_a, nullptr);
    EXPECT_EQ(table_test_2_a->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table_test_2_a->GetValue(), 4);
    EXPECT_EQ(table_test_2_a->GetIndex(), 0u);
    ASSERT_NE(table_test_2_c, nullptr);
    EXPECT_EQ(table_test_2_c->GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(table_test_2_c->GetValue(), false);
    EXPECT_EQ(table_test_2_c->GetIndex(), 1u);
    ASSERT_NE(table_test_3_a, nullptr);
    EXPECT_EQ(table_test_3_a->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table_test_3_a->GetValue()), "five");
    EXPECT_EQ(table_test_3_a->GetIndex(), 0u);
    ASSERT_NE(table_test_3_d, nullptr);
    EXPECT_EQ(table_test_3_d->GetType(), sdv::toml::ENodeType::node_array);
    EXPECT_EQ(table_test_3_d->GetIndex(), 1u);
}

TEST(NestedContent, InlineTable)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        table1 = { a = 0, b = 1.2, c = "string" }
        table2 = { a = [], b = true, e = 2.71828 }
        table3 = { a = { a = "a", b = "A" }, b = {a = "b", b = "B"}, e = {a = "e", b = "E"} }
        )"s);

    auto table1_a	= parser.Root().Direct("table1.a");
    auto table1_b	= parser.Root().Direct("table1.b");
    auto table1_c	= parser.Root().Direct("table1.c");
    auto table2_a	= parser.Root().Direct("table2.a");
    auto table2_b	= parser.Root().Direct("table2.b");
    auto table2_e	= parser.Root().Direct("table2.e");
    auto table3_a_a = parser.Root().Direct("table3.a.a");
    auto table3_a_b = parser.Root().Direct("table3.a.b");
    auto table3_b_a = parser.Root().Direct("table3.b.a");
    auto table3_b_b = parser.Root().Direct("table3.b.b");
    auto table3_e_a = parser.Root().Direct("table3.e.a");
    auto table3_e_b = parser.Root().Direct("table3.e.b");

    ASSERT_NE(table1_a, nullptr);
    EXPECT_EQ(table1_a->GetType(), sdv::toml::ENodeType::node_integer);
    EXPECT_EQ(table1_a->GetValue(), 0);
    ASSERT_NE(table1_b, nullptr);
    EXPECT_EQ(table1_b->GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table1_b->GetValue(), 1.2);
    ASSERT_NE(table1_c, nullptr);
    EXPECT_EQ(table1_c->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table1_c->GetValue()), "string");
    ASSERT_NE(table2_a, nullptr);
    EXPECT_EQ(table2_a->GetType(), sdv::toml::ENodeType::node_array);
    ASSERT_NE(table2_b, nullptr);
    EXPECT_EQ(table2_b->GetType(), sdv::toml::ENodeType::node_boolean);
    EXPECT_EQ(table2_b->GetValue(), true);
    ASSERT_NE(table2_e, nullptr);
    EXPECT_EQ(table2_e->GetType(), sdv::toml::ENodeType::node_floating_point);
    EXPECT_EQ(table2_e->GetValue(), 2.71828);
    ASSERT_NE(table3_a_a, nullptr);
    EXPECT_EQ(table3_a_a->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_a_a->GetValue()), "a");
    ASSERT_NE(table3_a_b, nullptr);
    EXPECT_EQ(table3_a_b->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_a_b->GetValue()), "A");
    ASSERT_NE(table3_b_a, nullptr);
    EXPECT_EQ(table3_b_a->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_b_a->GetValue()), "b");
    ASSERT_NE(table3_b_b, nullptr);
    EXPECT_EQ(table3_b_b->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_b_b->GetValue()), "B");
    ASSERT_NE(table3_e_a, nullptr);
    EXPECT_EQ(table3_e_a->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_e_a->GetValue()), "e");
    ASSERT_NE(table3_e_b, nullptr);
    EXPECT_EQ(table3_e_b->GetType(), sdv::toml::ENodeType::node_string);
    EXPECT_EQ(static_cast<std::string>(table3_e_b->GetValue()), "E");
}

TEST(SpecialCases, Keys)
{
    using namespace std::string_literals;
    EXPECT_NO_THROW(toml_parser::CParser(R"(
            "127.0.0.1" = "value"
            "character encoding" = "value"
            "ʎǝʞ" = "value"
            'key2' = "value"
            'quoted "value"' = "value"
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            key = "value"
            bare_key = "value"
            bare-key = "value"
            1234 = "value"
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            "" = "blank"     # VALID but discouraged
        )"s));
    EXPECT_NO_THROW(toml_parser::CParser(R"(
            '' = 'blank'     # VALID but discouraged
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            name = "Orange"
            physical.color = "orange"
            physical.shape = "round"
            site."google.com" = true
        )"s));


    EXPECT_NO_THROW(toml_parser::CParser(R"(
            fruit.name = "banana"     # this is best practice
            fruit. color = "yellow"    # same as fruit.color
            fruit . flavor = "banana"   # same as fruit.flavor
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            # This makes the key "fruit" into a table.
            fruit.apple.smooth = true
            # So then you can add to the table "fruit" like so:
            fruit.orange = 2
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            # VALID BUT DISCOURAGED
            apple.type = "fruit"
            orange.type = "fruit"
            apple.skin = "thin"
            orange.skin = "thick"
            apple.color = "red"
            orange.color = "orange"
        )"s));

    EXPECT_NO_THROW(toml_parser::CParser(R"(
            3.1415 = 3.1415
        )"s));
    {
        toml_parser::CParser parser(R"(
            3.1415 = 3.1415
        )"s);
        auto   table = parser.Root().Direct("3");
        auto   pi	 = parser.Root().Direct("3.1415");
        ASSERT_NE(table, nullptr);
        EXPECT_EQ(table->GetType(), sdv::toml::ENodeType::node_table);
        ASSERT_NE(pi, nullptr);
        EXPECT_EQ(pi->GetType(), sdv::toml::ENodeType::node_floating_point);
        EXPECT_EQ(pi->GetValue(), 3.1415);
    }
}

TEST(SpecialCases, Arrays)
{
    using namespace std::string_literals;
    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            integers = [ 1, 2, 3 ]
            colors = [ "red", "yellow", "green" ]
            nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
            nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
            string_array = [ "all", 'strings', """are the same""", '''type''' ]
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
            contributors = [
            "Foo Bar <foo@example.com>",
            { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
            ]
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            integers3 = [
                1,
                2, # this is ok
            ]
            )"s));
}

TEST(SpecialCases, Tables)
{
    using namespace std::string_literals;
    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            [table-1]
            key1 = "some string"
            key2 = 123

            [table-2]
            key1 = "another string"
            key2 = 456
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            [dog."tater.man"]
            type.name = "pug"
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            [a.b.c]            # this is best practice
            [ d.e.f ]          # same as [d.e.f]
            [ g .  h  . i ]    # same as [g.h.i]
            [ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            # [x] you
            # [x.y] don't
            # [x.y.z] need these
            [x.y.z.w] # for this to work
            [x] # defining a super-table afterward is ok
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            # VALID BUT DISCOURAGED
            [fruit.apple]
            [animal]
            [fruit.orange]
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple.texture]  # you can add sub-tables
            )"s));
}

TEST(SpecialCases, TableArrays)
{
    using namespace std::string_literals;
    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            [[products]]
            name = "Hammer"
            sku = 738594937
            [[products]]  # empty table within the array
            [[products]]
            name = "Nail"
            sku = 284758393
            color = "gray"
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
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
            )"s));

    EXPECT_NO_THROW(toml_parser::CParser parser(R"(
            points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ]
            )"s));
}

TEST(ErrorCases, KeyValue)
{
    using namespace std::string_literals;
    EXPECT_THROW(toml_parser::CParser parser(R"(key = # node_invalid)"s), sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(first = "Tom" last = "Preston-Werner" # node_invalid)"s), sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(= "no key name" # node_invalid)"s), sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            name = "Tom"
            name = "Pradyun"
        )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser(R"(
            fruit . flavor = "banana"   # same as fruit.flavor
            fruit.flavor = "banana"
        )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser(R"(
            spelling = "favorite"
            "spelling" = "favourite"
        )"s),
                 sdv::toml::XTOMLParseException);


    EXPECT_THROW(toml_parser::CParser(R"(
            # This defines the value of fruit.apple to be an integer.
            fruit.apple = 1
            # But then this treats fruit.apple like it's a table.
            # You can't turn an integer into a table.
            fruit.apple.smooth = true
        )"s),
                 sdv::toml::XTOMLParseException);
}

TEST(ErrorCases, Tables)
{
    using namespace std::string_literals;
    EXPECT_THROW(toml_parser::CParser parser(R"(
            [ j . "ʞ" . 'l' ]
            [j."ʞ".'l']
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            [ j . "ʞ" . 'l' ]
            ["j".'ʞ'."l"]
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            [fruit]
            apple = "red"
            [fruit]
            orange = "orange"
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            [fruit]
            apple = "red"
            [fruit.apple]
            texture = "smooth"
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple]  # INVALID
            )"s),
                 sdv::toml::XTOMLParseException);
    EXPECT_THROW(toml_parser::CParser parser(R"(
            [fruit]
            apple.color = "red"
            apple.taste.sweet = true
            [fruit.apple.taste]  # INVALID
            )"s),
                 sdv::toml::XTOMLParseException);
}

TEST(ErrorCases, InlineTables)
{
    using namespace std::string_literals;

    EXPECT_THROW(toml_parser::CParser parser(R"(
            type = { name = "Nail" }
            type.edible = false  # INVALID
            )"s),
                 sdv::toml::XTOMLParseException);
    EXPECT_THROW(toml_parser::CParser parser(R"(
            [product]
            type.name = "Nail"
            type = { edible = false }  # INVALID
            )"s),
                 sdv::toml::XTOMLParseException);
}

TEST(ErrorCases, TableArrays)
{
    using namespace std::string_literals;
    EXPECT_THROW(toml_parser::CParser parser(R"(
            [fruit.physical]  # subtable, but to which parent element should it belong?
            color = "red"
            shape = "round"
            [[fruit]]  # parser must throw an error upon discovering that "fruit" is
                       # an array rather than a table
            name = "apple"
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            fruits = []
            [[fruits]] # Not allowed
            )"s),
                 sdv::toml::XTOMLParseException);

    EXPECT_THROW(toml_parser::CParser parser(R"(
            [[fruits]]
            name = "apple"
            [[fruits.varieties]]
            name = "red delicious"
            # INVALID: This table conflicts with the previous array of tables
            [fruits.varieties]
            name = "granny smith"
            )"s),
                 sdv::toml::XTOMLParseException);
    EXPECT_THROW(toml_parser::CParser parser(R"(
            [[fruits]]
            name = "apple"
            [fruits.physical]
            color = "red"
            shape = "round"
            # INVALID: This array of tables conflicts with the previous table
            [[fruits.physical]]
            color = "green"
            )"s),
                 sdv::toml::XTOMLParseException);
}

TEST(Ordering, Array)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
        array = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    )"s);

    auto	   two	  = parser.Root().Direct("array[2]");
    auto	   eleven = parser.Root().Direct("array[11]");
    const auto arr	  = parser.Root().Direct("array");

    // with direct access
    ASSERT_NE(two, nullptr);
    EXPECT_EQ(two->GetValue(), 2);
    ASSERT_NE(eleven, nullptr);
    EXPECT_EQ(eleven->GetValue(), 11);

    // with indirect access through iterating
    ASSERT_NE(arr, nullptr);
    auto ptrArray = arr->Cast<toml_parser::CArray>();
    EXPECT_EQ(ptrArray->GetCount(), 12u);
    for (uint32_t uiIndex = 0; uiIndex < ptrArray->GetCount(); uiIndex++)
        EXPECT_EQ(ptrArray->Get(uiIndex)->GetValue(), (int64_t)uiIndex);
}

TEST(Ordering, TableAray)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
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
    )"s);

    auto tableArray = parser.Root().Direct("tableArray");

    ASSERT_NE(tableArray, nullptr);
    auto ptrArray = tableArray->Cast<toml_parser::CArray>();
    EXPECT_EQ(ptrArray->GetCount(), 12u);
    for (uint32_t uiIndex = 0; uiIndex < ptrArray->GetCount(); uiIndex++)
        EXPECT_EQ(ptrArray->Get(uiIndex)->Cast<toml_parser::CTable>()->Direct("a")->GetValue(), (int64_t) uiIndex);
}

TEST(Ordering, NodeGetDirect)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
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
        )"s);

    auto table_test_1_a = parser.Root().Direct("table.test[0].a");
    auto table_test_1_b = parser.Root().Direct("table.test[0].b");
    auto table_test_2_a = parser.Root().Direct("table.test[1].a");
    auto table_test_2_c = parser.Root().Direct("table.test[1].c");
    auto table_test_3_a = parser.Root().Direct("table.test[2].a");
    auto table_test_3_d = parser.Root().Direct("table.test[2].d");

    EXPECT_TRUE(table_test_1_a);
    EXPECT_TRUE(table_test_1_b);
    EXPECT_EQ(table_test_3_d->GetType(), sdv::toml::ENodeType::node_array);

    auto table_test_3 = parser.Root().Direct("table.test[2]");
    auto table_test_3_2nd = table_test_3->Cast<toml_parser::CTable>()->GetNodeDirect("d[2].x");
    EXPECT_NE(table_test_3_2nd, nullptr);
}

TEST(NodeAccess, Parent)
{
    using namespace std::string_literals;
    toml_parser::CParser parser(R"(
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
        )"s);

    auto fnGetName = [](sdv::IInterfaceAccess* pNode) -> std::string
    {
        sdv::toml::INodeInfo* pNodeInfo = sdv::TInterfaceAccessPtr(pNode).GetInterface<sdv::toml::INodeInfo>();
        if (!pNodeInfo) return {};
        return pNodeInfo->GetPath(true);
    };

    auto root = &parser.Root();
    auto table = parser.Root().Direct("table");
    ASSERT_TRUE(table);
    EXPECT_EQ(table->GetParent(), root);
    EXPECT_EQ(fnGetName(table->GetParent()), "");
    auto table_test = parser.Root().Direct("table.test");
    ASSERT_TRUE(table_test);
    EXPECT_EQ(table_test->GetParent(), table.get());
    EXPECT_EQ(fnGetName(table_test->GetParent()), "table");
    auto table_test_1 = parser.Root().Direct("table.test[0]");
    ASSERT_TRUE(table_test_1);
    EXPECT_EQ(table_test_1->GetParent(), table_test.get());
    EXPECT_EQ(fnGetName(table_test_1->GetParent()), "table.test");
    auto table_test_1_a = parser.Root().Direct("table.test[0].a");
    ASSERT_TRUE(table_test_1_a);
    EXPECT_EQ(table_test_1_a->GetParent(), table_test_1.get());
    EXPECT_EQ(fnGetName(table_test_1_a->GetParent()), "table.test[0]");
    auto table_test_1_b = parser.Root().Direct("table.test[0].b");
    ASSERT_TRUE(table_test_1_b);
    EXPECT_EQ(table_test_1_b->GetParent(), table_test_1.get());
    EXPECT_EQ(fnGetName(table_test_1_b->GetParent()), "table.test[0]");
    auto table_test_2 = parser.Root().Direct("table.test[1]");
    ASSERT_TRUE(table_test_2);
    EXPECT_EQ(table_test_2->GetParent(), table_test.get());
    EXPECT_EQ(fnGetName(table_test_2->GetParent()), "table.test");
    auto table_test_2_a = parser.Root().Direct("table.test[1].a");
    ASSERT_TRUE(table_test_2_a);
    EXPECT_EQ(table_test_2_a->GetParent(), table_test_2.get());
    EXPECT_EQ(fnGetName(table_test_2_a->GetParent()), "table.test[1]");
    auto table_test_2_c = parser.Root().Direct("table.test[1].c");
    ASSERT_TRUE(table_test_2_c);
    EXPECT_EQ(table_test_2_c->GetParent(), table_test_2.get());
    EXPECT_EQ(fnGetName(table_test_2_c->GetParent()), "table.test[1]");
    auto table_test_3 = parser.Root().Direct("table.test[2]");
    ASSERT_TRUE(table_test_3);
    EXPECT_EQ(table_test_3->GetParent(), table_test.get());
    EXPECT_EQ(fnGetName(table_test_3->GetParent()), "table.test");
    auto table_test_3_a = parser.Root().Direct("table.test[2].a");
    ASSERT_TRUE(table_test_3_a);
    EXPECT_EQ(table_test_3_a->GetParent(), table_test_3.get());
    EXPECT_EQ(fnGetName(table_test_3_a->GetParent()), "table.test[2]");
    auto table_test_3_d = parser.Root().Direct("table.test[2].d");
    ASSERT_TRUE(table_test_3_d);
    EXPECT_EQ(table_test_3_d->GetParent(), table_test_3.get());
    EXPECT_EQ(fnGetName(table_test_3_d->GetParent()), "table.test[2]");
}
