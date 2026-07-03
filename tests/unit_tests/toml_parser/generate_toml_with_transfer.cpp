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

TEST(GenerateTOML, TransferNodeComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"toml(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferNodeCommentWithSpaces)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(
    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]

    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferUnattachedComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayWhitespace)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(
    array = [ 1, 2, 3,  
              4, 5, 6 ]
)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]

    array = [ 1, 2, 3,  
              4, 5, 6 ]
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(
# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]

# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayCommentWithSpace)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(

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
    std::string ssTOMLOutput = R"toml([tree.branch]


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

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableWhitespace)
{
    toml_parser::CParser parser;

    // NOTE: Line-breaks within inline tables are not allowed.
    std::string ssTOMLInput = R"toml(
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]

    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableComment)
{
    toml_parser::CParser parser;

    // NOTE: Line-breaks within inline tables are not allowed.
    std::string ssTOMLInput  = R"toml(
# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]

# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = u8R"toml("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")toml";
    std::string ssTOMLOutput = u8R"toml([tree.branch]
"127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBlankKeys)
{
    std::string ssTOMLInput1 = R"toml("" = "blank"     # VALID but discouraged)toml";
    std::string ssTOMLInput2 = R"toml('' = 'blank'     # VALID but discouraged)toml";
    std::string ssTOMLOutput1 = R"toml([tree.branch]
"" = "blank"     # VALID but discouraged)toml";
    std::string ssTOMLOutput2 = R"toml([tree.branch]
'' = 'blank'     # VALID but discouraged)toml";

    toml_parser::CParser parser1, parser2;
    EXPECT_NO_THROW(parser1.Process(ssTOMLInput1));
    EXPECT_NO_THROW(parser2.Process(ssTOMLInput2));

    std::string ssGenerated1, ssGenerated2;
    EXPECT_NO_THROW(ssGenerated1 = parser1.GenerateTOML("tree.branch"));
    EXPECT_NO_THROW(ssGenerated2 = parser2.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated1, ssTOMLOutput1);
    EXPECT_EQ(ssGenerated2, ssTOMLOutput2);
}

TEST(GenerateTOML, TransferDottedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOutOfOrderKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatLookingAlikeKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(3.14159 = "pi")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
3.14159 = "pi")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBasicStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(str1 = """
Roses are red
Violets are blue""")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
str1 = """
Roses are red
Violets are blue""")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLongMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotingStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(int1 = +99
int2 = 42
int3 = 0
int4 = -17)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
int1 = +99
int2 = 42
int3 = 0
int4 = -17)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOtherBaseIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(flt8 = 224_617.445_991_228)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
flt8 = 224_617.445_991_228)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferSpecialFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBooleans)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(bool1 = true
bool2 = false)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
bool1 = true
bool2 = false)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferReadibleOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(odt4 = 1979-05-27 07:32:00Z)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
odt4 = 1979-05-27 07:32:00Z)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDates)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(ld1 = 1979-05-27)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
ld1 = 1979-05-27)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(lt1 = 07:32:00
lt2 = 00:32:00.999999)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
lt1 = 07:32:00
lt2 = 00:32:00.999999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(integers = [ 1, 2, 3 ]
colors = [ "red", "yellow", "green" ]
nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
string_array = [ "all", 'strings', """are the same""", '''type''' ]

# Mixed-type arrays are allowed
numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
contributors = [
  "Foo Bar <foo@example.com>",
  { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
])toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
integers = [ 1, 2, 3 ]
colors = [ "red", "yellow", "green" ]
nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
string_array = [ "all", 'strings', """are the same""", '''type''' ]

# Mixed-type arrays are allowed
numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
contributors = [
  "Foo Bar <foo@example.com>",
  { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
])toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)toml";
    std::string ssTOMLOutput = R"toml([tree.branch.table]

[tree.branch.table-1]
key1 = "some string"
key2 = 123

[tree.branch.table-2]
key1 = "another string"
key2 = 456)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml([dog."tater.man"]
type.name = "pug")toml";
    std::string ssTOMLOutput = R"toml([tree.branch.dog."tater.man"]
type.name = "pug")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = u8R"toml([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)toml";
    std::string ssTOMLOutput = u8R"toml([tree.branch.a.b.c]            # this is best practice
x = 1
[tree.branch. d.e.f ]          # same as [d.e.f]
y = 1
[tree.branch. g .  h  . i ]    # same as [g.h.i]
z = 1
[tree.branch. j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedOrderTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)toml";
    std::string ssTOMLOutput = R"toml(# VALID BUT DISCOURAGED
[tree.branch.fruit.apple]
a = 1
[tree.branch.animal]
b = 2
[tree.branch.fruit.orange]
aa = 11)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedValueAndTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[tree.branch.owner]
name = "Regina Dogman"
member_since = 1999)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferAutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
fruit.apple.color = "red"
fruit.apple.taste.sweet = true)toml";
    std::string ssTOMLOutput2 = R"toml([tree.branch]
apple.color = "red"
apple.taste.sweet = true)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    EXPECT_EQ(ssGenerated, ssTOMLOutput);

    EXPECT_NO_THROW(ssGenerated = parser.Root().Direct("fruit")->GenerateTOML(toml_parser::CGenContext("tree.branch")));
    EXPECT_EQ(ssGenerated, ssTOMLOutput2);
}

TEST(GenerateTOML, TransferMixedAutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)toml";
    std::string ssTOMLOutput = R"toml([tree.branch.fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[tree.branch.fruit.apple.texture]  # you can add sub-tables
smooth = true)toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })toml";
    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferEmbeddedInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"toml(test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml([[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray")toml";
    std::string ssTOMLOutput = R"toml([[tree.branch.products]]
name = "Hammer"
sku = 738594937

[[tree.branch.products]]  # empty table within the array

[[tree.branch.products]]
name = "Nail"
sku = 284758393

color = "gray")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedTableAndTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml([[fruits]]
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
    std::string ssTOMLOutput = R"toml([[tree.branch.fruits]]
name = "apple"

[tree.branch.fruits.physical]  # subtable
color = "red"
shape = "round"

[[tree.branch.fruits.varieties]]  # nested array of tables
name = "red delicious"

[[tree.branch.fruits.varieties]]
name = "granny smith"


[[tree.branch.fruits]]
name = "banana"

[[tree.branch.fruits.varieties]]
name = "plantain")toml";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"toml(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])toml";
    std::string ssTOMLOutput = R"toml([tree.branch]
points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])toml";
    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}
