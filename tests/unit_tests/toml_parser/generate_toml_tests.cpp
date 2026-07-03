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
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"

TEST(GenerateTOML, Comment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# This is a full-line comment)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, NodeComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, NodeCommentWithSpaces)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(
    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, UnattachedComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayWhitespace)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(
    array = [ 1, 2, 3,  
              4, 5, 6 ]
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayComment)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(
# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ArrayCommentWithSpace)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(

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

)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTableWhitespace)
{
    toml_parser::CParser parser;

    // Note: line-breaks within an inline table are not allowed.
    std::string ssTOML = R"toml(
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTableComment)
{
    toml_parser::CParser parser;

    // Note: line-breaks within an inline table are not allowed.
    std::string ssTOML = R"toml(
# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Keys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = u8R"toml("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, BlankKeys)
{
    std::string ssTOML1 = R"toml("" = "blank"     # VALID but discouraged)toml";
    std::string ssTOML2 = R"toml('' = 'blank'     # VALID but discouraged)toml";

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

    std::string ssTOML = R"toml(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, WhitespaceKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, OutOfOrderKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, FloatLookingAlikeKeys)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(3.14159 = "pi")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, BasicStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(str1 = """
Roses are red
Violets are blue""")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, LongMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotingStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, LiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Integers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(int1 = +99
int2 = 42
int3 = 0
int4 = -17)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ReadibleIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, OtherBaseIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, FloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, ReadibleFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(flt8 = 224_617.445_991_228)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, SpecialFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Booleans)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(bool1 = true
bool2 = false)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_OffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_ReadibleOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(odt4 = 1979-05-27 07:32:00Z)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalDates)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(ld1 = 1979-05-27)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, DISABLED_LocalTimes)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(lt1 = 07:32:00
lt2 = 00:32:00.999999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Arrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(integers = [ 1, 2, 3 ]
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
    ])toml";

    parser.Process(ssTOML);
    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MultiLineArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, Tables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, QuotedKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml([dog."tater.man"]
type.name = "pug")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, WhitespaceKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = u8R"toml([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedOrderTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedValueAndTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, AutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)toml";
    std::string ssTOMLOutput2 = R"toml(apple.color = "red"
apple.taste.sweet = true)toml";

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

    std::string ssTOML = R"toml([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, InlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, EmbeddedInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOML  = R"toml(test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());
    
    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, TableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml([[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray")toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}

TEST(GenerateTOML, MixedTableAndTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOML = R"toml([[fruits]]
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
name = "plantain")toml";
    std::string ssTOMLFruits1Physical = R"toml([physical]  # subtable
color = "red"
shape = "round"

)toml";
    std::string ssTOMLFruits1Varieties = R"toml([[varieties]]  # nested array of tables
name = "red delicious"

[[varieties]]
name = "granny smith"

)toml";
    std::string ssTOMLFruits1Variety1 = R"toml([variety]  # nested array of tables
name = "red delicious"

)toml";
    std::string ssTOMLFruits1Variety2 = R"toml([variety]
name = "granny smith"

)toml";
    std::string ssTOMLFruits2Variety1 = R"toml([variety]
name = "plantain")toml";

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

    std::string ssTOML = R"toml(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])toml";

    EXPECT_NO_THROW(parser.Process(ssTOML));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML());

    EXPECT_EQ(ssGenerated, ssTOML);
}
