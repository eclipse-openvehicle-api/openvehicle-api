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

TEST(GenerateTOML, TransferNodeComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")code";
    std::string ssTOMLOutput = R"code([tree.branch]
# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferNodeCommentWithSpaces)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(
    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)code";
    std::string ssTOMLOutput = R"code([tree.branch]

    # This is a full-line comment
    key   =   "value"  # This is a comment at the end of a line
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferUnattachedComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)code";
    std::string ssTOMLOutput = R"code([tree.branch]
# Comment not belonging to node

# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment"

# Comment not belonging to node)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayWhitespace)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(
    array = [ 1, 2, 3,  
              4, 5, 6 ]
)code";
    std::string ssTOMLOutput = R"code([tree.branch]

    array = [ 1, 2, 3,  
              4, 5, 6 ]
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayComment)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(
# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)code";
    std::string ssTOMLOutput = R"code([tree.branch]

# Pre-array
array = [   1,              # Value #1
            2,              # Value #2  
            3,              # Value #3  
            4,              # Value #4  
            5,              # Value #5  
            6,              # Value #6
        ] # Post-array
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrayCommentWithSpace)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(

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
    std::string ssTOMLOutput = R"code([tree.branch]


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

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableWhitespace)
{
    toml_parser::CParser parser;

    // NOTE: Line-breaks within inline tables are not allowed.
    std::string ssTOMLInput = R"code(
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)code";
    std::string ssTOMLOutput = R"code([tree.branch]

    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 }
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableComment)
{
    toml_parser::CParser parser;

    // NOTE: Line-breaks within inline tables are not allowed.
    std::string ssTOMLInput  = R"code(
# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)code";
    std::string ssTOMLOutput = R"code([tree.branch]

# Pre-table
    table = { a = 1, b = 2, d = 3, e = 4, f = 5, g = 6 } # Post-table
)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")code";
    std::string ssTOMLOutput = R"code([tree.branch]
key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = u8R"code("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")code";
    std::string ssTOMLOutput = u8R"code([tree.branch]
"127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBlankKeys)
{
    std::string ssTOMLInput1 = R"code("" = "blank"     # VALID but discouraged)code";
    std::string ssTOMLInput2 = R"code('' = 'blank'     # VALID but discouraged)code";
    std::string ssTOMLOutput1 = R"code([tree.branch]
"" = "blank"     # VALID but discouraged)code";
    std::string ssTOMLOutput2 = R"code([tree.branch]
'' = 'blank'     # VALID but discouraged)code";

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

    std::string ssTOMLInput = R"code(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)code";
    std::string ssTOMLOutput = R"code([tree.branch]
name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)code";
    std::string ssTOMLOutput = R"code([tree.branch]
fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOutOfOrderKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")code";
    std::string ssTOMLOutput = R"code([tree.branch]
apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatLookingAlikeKeys)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(3.14159 = "pi")code";
    std::string ssTOMLOutput = R"code([tree.branch]
3.14159 = "pi")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBasicStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(str1 = """
Roses are red
Violets are blue""")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str1 = """
Roses are red
Violets are blue""")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLongMultiLineStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)code";
    std::string ssTOMLOutput = R"code([tree.branch]
str1 = "The quick brown fox jumps over the lazy dog."

str2 = """
The quick brown \


  fox jumps over \
    the lazy dog."""

str3 = """\
       The quick brown \
       fox jumps over \
       the lazy dog.\
       """)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotingStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')code";
    std::string ssTOMLOutput = R"code([tree.branch]
winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')code";
    std::string ssTOMLOutput = R"code([tree.branch]
regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedLiteralStrings)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')code";
    std::string ssTOMLOutput = R"code([tree.branch]
quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(int1 = +99
int2 = 42
int3 = 0
int4 = -17)code";
    std::string ssTOMLOutput = R"code([tree.branch]
int1 = +99
int2 = 42
int3 = 0
int4 = -17)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)code";
    std::string ssTOMLOutput = R"code([tree.branch]
int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOtherBaseIntegers)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)code";
    std::string ssTOMLOutput = R"code([tree.branch]
# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)code";
    std::string ssTOMLOutput = R"code([tree.branch]
# fractional
flt1 = +1.0
flt2 = 3.1415
flt3 = -0.01

# exponent
flt4 = 5e+22
flt5 = 1e06
flt6 = -2E-2

# both
flt7 = 6.626e-34)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(flt8 = 224_617.445_991_228)code";
    std::string ssTOMLOutput = R"code([tree.branch]
flt8 = 224_617.445_991_228)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferSpecialFloatingPoints)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)code";
    std::string ssTOMLOutput = R"code([tree.branch]
# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBooleans)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(bool1 = true
bool2 = false)code";
    std::string ssTOMLOutput = R"code([tree.branch]
bool1 = true
bool2 = false)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";
    std::string ssTOMLOutput = R"code([tree.branch]
odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferReadibleOffsetDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(odt4 = 1979-05-27 07:32:00Z)code";
    std::string ssTOMLOutput = R"code([tree.branch]
odt4 = 1979-05-27 07:32:00Z)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDateTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDates)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(ld1 = 1979-05-27)code";
    std::string ssTOMLOutput = R"code([tree.branch]
ld1 = 1979-05-27)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalTimes)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(lt1 = 07:32:00
lt2 = 00:32:00.999999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
lt1 = 07:32:00
lt2 = 00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(integers = [ 1, 2, 3 ]
colors = [ "red", "yellow", "green" ]
nested_arrays_of_ints = [ [ 1, 2 ], [3, 4, 5] ]
nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
string_array = [ "all", 'strings', """are the same""", '''type''' ]

# Mixed-type arrays are allowed
numbers = [ 0.1, 0.2, 0.5, 1, 2, 5 ]
contributors = [
  "Foo Bar <foo@example.com>",
  { name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux" }
])code";
    std::string ssTOMLOutput = R"code([tree.branch]
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
])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])code";
    std::string ssTOMLOutput = R"code([tree.branch]
integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)code";
    std::string ssTOMLOutput = R"code([tree.branch.table]

[tree.branch.table-1]
key1 = "some string"
key2 = 123

[tree.branch.table-2]
key1 = "another string"
key2 = 456)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code([dog."tater.man"]
type.name = "pug")code";
    std::string ssTOMLOutput = R"code([tree.branch.dog."tater.man"]
type.name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeyTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = u8R"code([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)code";
    std::string ssTOMLOutput = u8R"code([tree.branch.a.b.c]            # this is best practice
x = 1
[tree.branch. d.e.f ]          # same as [d.e.f]
y = 1
[tree.branch. g .  h  . i ]    # same as [g.h.i]
z = 1
[tree.branch. j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedOrderTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)code";
    std::string ssTOMLOutput = R"code(# VALID BUT DISCOURAGED
[tree.branch.fruit.apple]
a = 1
[tree.branch.animal]
b = 2
[tree.branch.fruit.orange]
aa = 11)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedValueAndTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[tree.branch.owner]
name = "Regina Dogman"
member_since = 1999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferAutomaticTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)code";
    std::string ssTOMLOutput = R"code([tree.branch]
fruit.apple.color = "red"
fruit.apple.taste.sweet = true)code";
    std::string ssTOMLOutput2 = R"code([tree.branch]
apple.color = "red"
apple.taste.sweet = true)code";

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

    std::string ssTOMLInput = R"code([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)code";
    std::string ssTOMLOutput = R"code([tree.branch.fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[tree.branch.fruit.apple.texture]  # you can add sub-tables
smooth = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })code";
    std::string ssTOMLOutput = R"code([tree.branch]
name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })code";
    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferEmbeddedInlineTables)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput  = R"code(test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])code";
    std::string ssTOMLOutput = R"code([tree.branch]
test=[{ first = "Tom", last = "Preston-Werner" },
{ x = 1, y = 2 },
{ type.name = "pug" }])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));
    
    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code([[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray")code";
    std::string ssTOMLOutput = R"code([[tree.branch.products]]
name = "Hammer"
sku = 738594937

[[tree.branch.products]]  # empty table within the array

[[tree.branch.products]]
name = "Nail"
sku = 284758393

color = "gray")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedTableAndTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code([[fruits]]
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
    std::string ssTOMLOutput = R"code([[tree.branch.fruits]]
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
name = "plantain")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableArrays)
{
    toml_parser::CParser parser;

    std::string ssTOMLInput = R"code(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])code";
    std::string ssTOMLOutput = R"code([tree.branch]
points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])code";
    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.GenerateTOML("tree.branch"));

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}
