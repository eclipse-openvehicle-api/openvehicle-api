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
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"

TEST(GenerateTOML, Comment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# This is a full-line comment)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, NodeComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, NodeCommentWithSpaces)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(
    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, UnattachedComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayWhitespace)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(
    array = [ 1, 2, 3,  
              4, 5, 6 ]
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(
# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayCommentWithSpace)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(

# unattached comment

# Pre-array
array = [   1,              # Value #1

# unattached comment

            2,              # Value #2  

# unattached comment

            3,              # Value #3  

# unattached comment

            4,              # Value #4  

# unattached comment

            5,              # Value #5  

# unattached comment

            6,              # Value #6

# unattached comment

        ] # Post-array

# unattached comment

)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTableWhitespace)
{
    toml_parser::CParser parser;

    // Note: line-breaks within an inline table are not allowed.
    std::string ssTOML = R"code(
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTableComment)
{
    toml_parser::CParser parser;

    // Note: line-breaks within an inline table are not allowed.
    std::string ssTOML = R"code(
# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Keys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = u8R"code("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, BlankKeys)
{
    std::string ssTOML1 = R"code("" = "blank"     # VALID but discouraged)code";
    std::string ssTOML2 = R"code('' = 'blank'     # VALID but discouraged)code";

    toml_parser::CParser parser1, parser2;
    EXPECT_NO_THROW(parser1.Process(ssTOML1));
    EXPECT_NO_THROW(parser2.Process(ssTOML2));

    std::string ssGenerated1, ssGenerated2;
    EXPECT_NO_THROW(ssGenerated1 = parser1.GenerateTOML());
    EXPECT_NO_THROW(ssGenerated2 = parser2.GenerateTOML());
    EXPECT_EQ(ssGenerated1, ssTOML1);
    EXPECT_EQ(ssGenerated2, ssTOML2);
}

TEST(GenerateTOML, DottedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, WhitespaceKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, OutOfOrderKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, FloatLookingAlikeKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(3.14159 = "pi")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, BasicStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(str1 = """
Roses are red
Violets are blue""")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, LongMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotingStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, LiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Integers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(int1 = +99
int2 = 42
int3 = 0
int4 = -17)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ReadibleIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, OtherBaseIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, FloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ReadibleFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(flt8 = 224_617.445_991_228)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, SpecialFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Booleans)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(bool1 = true
bool2 = false)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_OffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_ReadibleOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(odt4 = 1979-05-27 07:32:00Z)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalDates)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(ld1 = 1979-05-27)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(lt1 = 07:32:00
lt2 = 00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Arrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(integers = [ 1, 2, 3 ]
colors = [ "red", "yellow", "green" ]
nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
string_array = [ "all", 'strings', """are the same""", '''type''' ]

# Mixed-type arrays are allowed
numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
contributors =
    [
        "Foo Bar <foo@example.com>", 
        { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
    ])code";

    parser.Process(ssTOML);
    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Tables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code([dog."tater.man"]
type.name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, WhitespaceKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = u8R"code([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedOrderTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedValueAndTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, AutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)code";
    std::string ssTOMLOutput2 = R"code(apple.color = "red"
apple.taste.sweet = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
        EXPECT_EQ(ssGenerated, ssTOML);

    EXPECT_NO_THROW(ssGenerated = parser.Root().Direct("fruit")->GenerateTOML());
    EXPECT_EQ(ssGenerated, ssTOMLOutput2);
}

TEST(GenerateTOML, MixedAutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, EmbeddedInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOML  = R"code(test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, TableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code([[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedTableAndTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code([[fruits]]
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
name = "plantain")code";
    std::string ssTOMLFruits1Physical = R"code([physical]  # subtable
color = "red"
shape = "round"

)code";
    std::string ssTOMLFruits1Varieties = R"code([[varieties]]  # nested array of tables
name = "red delicious"

[[varieties]]
name = "granny smith"

)code";
    std::string ssTOMLFruits1Variety1 = R"code([variety]  # nested array of tables
name = "red delicious"

)code";
    std::string ssTOMLFruits1Variety2 = R"code([variety]
name = "granny smith"

)code";
    std::string ssTOMLFruits2Variety1 = R"code([variety]
name = "plantain")code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    // Identical output
    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    EXPECT_EQ(ssGenerated, ssTOML);

    // Physical from fruits[0]
    auto ptrFruits1Physical = parser.Root().Direct("fruits[0].physical");
    ASSERT_TRUE(ptrFruits1Physical);
    EXPECT_NO_THROW(ssGenerated = ptrFruits1Physical->GenerateTOML(toml_parser::CGenContext("physical")));
    EXPECT_EQ(ssGenerated, ssTOMLFruits1Physical);

    // Varieties from fruits[0]
    auto ptrTOMLFruits1Varieties = parser.Root().Direct("fruits[0].varieties");
    ASSERT_TRUE(ptrTOMLFruits1Varieties);
    EXPECT_NO_THROW(ssGenerated = ptrTOMLFruits1Varieties->GenerateTOML());
    EXPECT_EQ(ssGenerated, ssTOMLFruits1Varieties);

    // Varieties[0] from fruits[0]
    auto ptrTOMLFruits1Variety1 = parser.Root().Direct("fruits[0].varieties[0]");
    ASSERT_TRUE(ptrTOMLFruits1Variety1);
    EXPECT_NO_THROW(ssGenerated = ptrTOMLFruits1Variety1->GenerateTOML(toml_parser::CGenContext("variety")));
    EXPECT_EQ(ssGenerated, ssTOMLFruits1Variety1);

    // Varieties[1] from fruits[0]
    auto ptrTOMLFruits1Variety2 = parser.Root().Direct("fruits[0].varieties[1]");
    ASSERT_TRUE(ptrTOMLFruits1Variety2);
    EXPECT_NO_THROW(ssGenerated = ptrTOMLFruits1Variety2->GenerateTOML(toml_parser::CGenContext("variety")));
    EXPECT_EQ(ssGenerated, ssTOMLFruits1Variety2);

    // Varieties[0] from fruits[1]
    auto ptrTOMLFruits2Variety1 = parser.Root().Direct("fruits[1].varieties[0]");
    ASSERT_TRUE(ptrTOMLFruits2Variety1);
    EXPECT_NO_THROW(ssGenerated = ptrTOMLFruits2Variety1->GenerateTOML(toml_parser::CGenContext("variety")));
    EXPECT_EQ(ssGenerated, ssTOMLFruits2Variety1);

}

TEST(GenerateTOML, InlineTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"code(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])code";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}
