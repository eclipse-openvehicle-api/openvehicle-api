#include <gtest/gtest.h>
#include "../../../sdv_services/core/toml_parser/parser_toml.h"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.h"

inline void Trim(std::string& rss)
{
    // Remove front whitespace
    size_t nStart = rss.find_first_not_of(" \t\f\r\n\v");
    if (nStart) rss.erase(0, nStart);

    // Remove rear whitespace
    size_t nStop = rss.find_last_not_of(" \t\f\r\n\v");
    rss.erase(nStop + 1);
}

TEST(GenerateTOML, Comment)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")code";
    std::string ssTOMLOutput = R"code(key = "value"
another = "# This is not a comment")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferComment)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# This is a full-line comment
key = "value"  # This is a comment at the end of a line
another = "# This is not a comment")code";
    std::string ssTOMLOutput = R"code([tree.branch]
key = "value"
another = "# This is not a comment")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, Keys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")code";
    std::string ssTOMLOutput = R"code(key = "value"
bare_key = "value"
bare-key = "value"
1234 = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferKeys)
{
    CParserTOML parser;

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
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, QuotedKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = u8R"code("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")code";
    std::string ssTOMLOutput = u8R"code("127.0.0.1" = "value"
"character encoding" = "value"
"ʎǝʞ" = "value"
'key2' = "value"
'quoted "value"' = "value")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeys)
{
    CParserTOML parser;

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
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, BlankKeys)
{
    std::string ssTOMLInput1 = R"code("" = "blank"     # VALID but discouraged)code";
    std::string ssTOMLInput2 = R"code('' = 'blank'     # VALID but discouraged)code";
    std::string ssTOMLOutput1 = R"code("" = "blank")code";
    std::string ssTOMLOutput2 = R"code('' = "blank")code";

    CParserTOML parser1, parser2;
    EXPECT_NO_THROW(parser1.Process(ssTOMLInput1));
    EXPECT_NO_THROW(parser2.Process(ssTOMLInput2));

    std::string ssGenerated1, ssGenerated2;
    EXPECT_NO_THROW(ssGenerated1 = parser1.CreateTOMLText());
    Trim(ssGenerated1);
    EXPECT_NO_THROW(ssGenerated2 = parser2.CreateTOMLText());
    Trim(ssGenerated2);

    EXPECT_EQ(ssGenerated1, ssTOMLOutput1);
    EXPECT_EQ(ssGenerated2, ssTOMLOutput2);
}

TEST(GenerateTOML, TransferBlankKeys)
{
    std::string ssTOMLInput1 = R"code("" = "blank"     # VALID but discouraged)code";
    std::string ssTOMLInput2 = R"code('' = 'blank'     # VALID but discouraged)code";
    std::string ssTOMLOutput1 = R"code([tree.branch]
"" = "blank")code";
    std::string ssTOMLOutput2 = R"code([tree.branch]
'' = "blank")code";

    CParserTOML parser1, parser2;
    EXPECT_NO_THROW(parser1.Process(ssTOMLInput1));
    EXPECT_NO_THROW(parser2.Process(ssTOMLInput2));

    std::string ssGenerated1, ssGenerated2;
    EXPECT_NO_THROW(ssGenerated1 = parser1.CreateTOMLText("tree.branch"));
    Trim(ssGenerated1);
    EXPECT_NO_THROW(ssGenerated2 = parser2.CreateTOMLText("tree.branch"));
    Trim(ssGenerated2);

    EXPECT_EQ(ssGenerated1, ssTOMLOutput1);
    EXPECT_EQ(ssGenerated2, ssTOMLOutput2);
}

TEST(GenerateTOML, DottedKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)code";
    std::string ssTOMLOutput = R"code(name = "Orange"

[physical]
color = "orange"
shape = "round"

[site]
"google.com" = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferDottedKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(name = "Orange"
physical.color = "orange"
physical.shape = "round"
site."google.com" = true)code";
    std::string ssTOMLOutput = R"code([tree.branch]
name = "Orange"

[tree.branch.physical]
color = "orange"
shape = "round"

[tree.branch.site]
"google.com" = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, WhitespaceKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)code";
    std::string ssTOMLOutput = R"code([fruit]
name = "banana"
color = "yellow"
flavor = "banana")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(fruit.name = "banana"     # this is best practice
fruit. color = "yellow"    # same as fruit.color
fruit . flavor = "banana"   # same as fruit.flavor)code";
    std::string ssTOMLOutput = R"code([tree.branch.fruit]
name = "banana"
color = "yellow"
flavor = "banana")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, OutOfOrderKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")code";
    std::string ssTOMLOutput = R"code([apple]
type = "fruit"
skin = "thin"
color = "red"

[orange]
type = "fruit"
skin = "thick"
color = "orange")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOutOfOrderKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(apple.type = "fruit"
orange.type = "fruit"

apple.skin = "thin"
orange.skin = "thick"

apple.color = "red"
orange.color = "orange")code";
    std::string ssTOMLOutput = R"code([tree.branch.apple]
type = "fruit"
skin = "thin"
color = "red"

[tree.branch.orange]
type = "fruit"
skin = "thick"
color = "orange")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, FloatLookingAlikeKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(3.14159 = "pi")code";
    std::string ssTOMLOutput = R"code([3]
14159 = "pi")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatLookingAlikeKeys)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(3.14159 = "pi")code";
    std::string ssTOMLOutput = R"code([tree.branch.3]
14159 = "pi")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, BasicStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";
    std::string ssTOMLOutput = R"code(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBasicStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str = "I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MultiLineStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str1 = """
Roses are red
Violets are blue""")code";
    std::string ssTOMLOutput = R"code(str1 = "Roses are red\nViolets are blue")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str1 = """
Roses are red
Violets are blue""")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str1 = "Roses are red\nViolets are blue")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, LongMultiLineStrings)
{
    CParserTOML parser;

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
    std::string ssTOMLOutput = R"code(str1 = "The quick brown fox jumps over the lazy dog."
str2 = "The quick brown fox jumps over the lazy dog."
str3 = "The quick brown fox jumps over the lazy dog.")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLongMultiLineStrings)
{
    CParserTOML parser;

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
str2 = "The quick brown fox jumps over the lazy dog."
str3 = "The quick brown fox jumps over the lazy dog.")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, QuotingStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")code";
    std::string ssTOMLOutput = R"code(str4 = "Here are two quotation marks: \"\". Simple enough."
str5 = "Here are three quotation marks: \"\"\"."
str6 = "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"."
str7 = "\"This,\" she said, \"is just a pointless statement.\"")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotingStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(str4 = """Here are two quotation marks: "". Simple enough."""
# str5 = """Here are three quotation marks: """."""  # INVALID
str5 = """Here are three quotation marks: ""\"."""
str6 = """Here are fifteen quotation marks: ""\"""\"""\"""\"""\"."""

# "This," she said, "is just a pointless statement."
str7 = """"This," she said, "is just a pointless statement."""")code";
    std::string ssTOMLOutput = R"code([tree.branch]
str4 = "Here are two quotation marks: \"\". Simple enough."
str5 = "Here are three quotation marks: \"\"\"."
str6 = "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"."
str7 = "\"This,\" she said, \"is just a pointless statement.\"")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, LiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')code";
    std::string ssTOMLOutput = R"code(winpath = "C:\\Users\\nodejs\\templates"
winpath2 = "\\\\ServerX\\admin$\\system32\\"
quoted = "Tom \"Dubs\" Preston-Werner"
regex = "<\\i\\c*\\s*>")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferLiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(winpath  = 'C:\Users\nodejs\templates'
winpath2 = '\\ServerX\admin$\system32\'
quoted   = 'Tom "Dubs" Preston-Werner'
regex    = '<\i\c*\s*>')code";
    std::string ssTOMLOutput = R"code([tree.branch]
winpath = "C:\\Users\\nodejs\\templates"
winpath2 = "\\\\ServerX\\admin$\\system32\\"
quoted = "Tom \"Dubs\" Preston-Werner"
regex = "<\\i\\c*\\s*>")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MultiLineLiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')code";
    std::string ssTOMLOutput = R"code(regex2 = "I [dw]on't need \\d{2} apples"
lines = "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineLiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(regex2 = '''I [dw]on't need \d{2} apples'''
lines  = '''
The first newline is
trimmed in raw strings.
   All other whitespace
   is preserved.
''')code";
    std::string ssTOMLOutput = R"code([tree.branch]
regex2 = "I [dw]on't need \\d{2} apples"
lines = "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, QuotedLiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')code";
    std::string ssTOMLOutput = R"code(quot15 = "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
apos15 = "Here are fifteen apostrophes: '''''''''''''''"
str = "'That,' she said, 'is still pointless.'")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedLiteralStrings)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(quot15 = '''Here are fifteen quotation marks: """""""""""""""'''

# apos15 = '''Here are fifteen apostrophes: ''''''''''''''''''  # INVALID
apos15 = "Here are fifteen apostrophes: '''''''''''''''"

# 'That,' she said, 'is still pointless.'
str = ''''That,' she said, 'is still pointless.'''')code";
    std::string ssTOMLOutput = R"code([tree.branch]
quot15 = "Here are fifteen quotation marks: \"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
apos15 = "Here are fifteen apostrophes: '''''''''''''''"
str = "'That,' she said, 'is still pointless.'")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, Integers)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(int1 = +99
int2 = 42
int3 = 0
int4 = -17)code";
    std::string ssTOMLOutput = R"code(int1 = 99
int2 = 42
int3 = 0
int4 = -17)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferIntegers)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(int1 = +99
int2 = 42
int3 = 0
int4 = -17)code";
    std::string ssTOMLOutput = R"code([tree.branch]
int1 = 99
int2 = 42
int3 = 0
int4 = -17)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, ReadibleIntegers)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)code";
    std::string ssTOMLOutput = R"code(int5 = 1000
int6 = 5349221
int7 = 5349221
int8 = 12345)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleIntegers)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(int5 = 1_000
int6 = 5_349_221
int7 = 53_49_221  # Indian number system grouping
int8 = 1_2_3_4_5  # VALID but discouraged)code";
    std::string ssTOMLOutput = R"code([tree.branch]
int5 = 1000
int6 = 5349221
int7 = 5349221
int8 = 12345)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, OtherBaseIntegers)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# hexadecimal with prefix `0x`
hex1 = 0xDEADBEEF
hex2 = 0xdeadbeef
hex3 = 0xdead_beef

# octal with prefix `0o`
oct1 = 0o01234567
oct2 = 0o755 # useful for Unix file permissions

# binary with prefix `0b`
bin1 = 0b11010110)code";
    std::string ssTOMLOutput = R"code(hex1 = 3735928559
hex2 = 3735928559
hex3 = 3735928559
oct1 = 342391
oct2 = 493
bin1 = 214)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferOtherBaseIntegers)
{
    CParserTOML parser;

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
hex1 = 3735928559
hex2 = 3735928559
hex3 = 3735928559
oct1 = 342391
oct2 = 493
bin1 = 214)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, FloatingPoints)
{
    CParserTOML parser;

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
    std::string ssTOMLOutput = R"code(flt1 = 1
flt2 = 3.1415
flt3 = -0.01
flt4 = 5e+22
flt5 = 1000000
flt6 = -0.02
flt7 = 6.626e-34)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferFloatingPoints)
{
    CParserTOML parser;

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
flt1 = 1
flt2 = 3.1415
flt3 = -0.01
flt4 = 5e+22
flt5 = 1000000
flt6 = -0.02
flt7 = 6.626e-34)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, ReadibleFloatingPoints)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(flt8 = 224_617.445_991_228)code";
    std::string ssTOMLOutput = R"code(flt8 = 224617.445991228)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferReadibleFloatingPoints)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(flt8 = 224_617.445_991_228)code";
    std::string ssTOMLOutput = R"code([tree.branch]
flt8 = 224617.445991228)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, SpecialFloatingPoints)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)code";
#if defined __GNUC__ && defined _WIN32
    std::string ssTOMLOutput = R"code(sf1 = inf
sf2 = inf
sf3 = -inf
sf4 = nan
sf5 = nan
sf6 = nan)code";
#else
    std::string ssTOMLOutput = R"code(sf1 = inf
sf2 = inf
sf3 = -inf
sf4 = nan
sf5 = nan
sf6 = -nan)code";
#endif
    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferSpecialFloatingPoints)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# infinity
sf1 = inf  # positive infinity
sf2 = +inf # positive infinity
sf3 = -inf # negative infinity

# not a number
sf4 = nan  # actual sNaN/qNaN encoding is implementation-specific
sf5 = +nan # same as `nan`
sf6 = -nan # valid, actual encoding is implementation-specific)code";
#if defined __GNUC__ && defined _WIN32
    std::string ssTOMLOutput = R"code([tree.branch]
sf1 = inf
sf2 = inf
sf3 = -inf
sf4 = nan
sf5 = nan
sf6 = nan)code";
#else
    std::string ssTOMLOutput = R"code([tree.branch]
sf1 = inf
sf2 = inf
sf3 = -inf
sf4 = nan
sf5 = nan
sf6 = -nan)code";
#endif

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, Booleans)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(bool1 = true
bool2 = false)code";
    std::string ssTOMLOutput = R"code(bool1 = true
bool2 = false)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferBooleans)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(bool1 = true
bool2 = false)code";
    std::string ssTOMLOutput = R"code([tree.branch]
bool1 = true
bool2 = false)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_OffsetDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";
    std::string ssTOMLOutput = R"code(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferOffsetDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";
    std::string ssTOMLOutput = R"code([tree.branch]
odt1 = 1979-05-27T07:32:00Z
odt2 = 1979-05-27T00:32:00-07:00
odt3 = 1979-05-27T00:32:00.999999-07:00)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_ReadibleOffsetDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(odt4 = 1979-05-27 07:32:00Z)code";
    std::string ssTOMLOutput = R"code(odt4 = 1979-05-27T07:32:00Z)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferReadibleOffsetDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(odt4 = 1979-05-27 07:32:00Z)code";
    std::string ssTOMLOutput = R"code([tree.branch]
odt4 = 1979-05-27T07:32:00Z)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_LocalDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";
    std::string ssTOMLOutput = R"code(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDateTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
ldt1 = 1979-05-27T07:32:00
ldt2 = 1979-05-27T00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_LocalDates)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(ld1 = 1979-05-27)code";
    std::string ssTOMLOutput = R"code(ld1 = 1979-05-27)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalDates)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(ld1 = 1979-05-27)code";
    std::string ssTOMLOutput = R"code([tree.branch]
ld1 = 1979-05-27)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_LocalTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(lt1 = 07:32:00
lt2 = 00:32:00.999999)code";
    std::string ssTOMLOutput = R"code(lt1 = 07:32:00
lt2 = 00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, DISABLED_TransferLocalTimes)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(lt1 = 07:32:00
lt2 = 00:32:00.999999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
lt1 = 07:32:00
lt2 = 00:32:00.999999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, Arrays)
{
    CParserTOML parser;

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
    std::string ssTOMLOutput = R"code(integers = [1, 2, 3]
colors = ["red", "yellow", "green"]
nested_arrays_of_ints = [[1, 2], [3, 4, 5]]
nested_mixed_array = [[1, 2], ["a", "b", "c"]]
string_array = ["all", "strings", "are the same", "type"]
numbers = [0.1, 0.2, 0.5, 1, 2, 5]
contributors = ["Foo Bar <foo@example.com>", {name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux"}])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferArrays)
{
    CParserTOML parser;

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
integers = [1, 2, 3]
colors = ["red", "yellow", "green"]
nested_arrays_of_ints = [[1, 2], [3, 4, 5]]
nested_mixed_array = [[1, 2], ["a", "b", "c"]]
string_array = ["all", "strings", "are the same", "type"]
numbers = [0.1, 0.2, 0.5, 1, 2, 5]
contributors = ["Foo Bar <foo@example.com>", {name = "Baz Qux", email = "bazqux@example.com", url = "https://example.com/bazqux"}])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MultiLineArrays)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])code";
    std::string ssTOMLOutput = R"code(integers2 = [1, 2, 3]
integers3 = [1, 2])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMultiLineArrays)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(integers2 = [
  1, 2, 3
]

integers3 = [
  1,
  2, # this is ok
])code";
    std::string ssTOMLOutput = R"code([tree.branch]
integers2 = [1, 2, 3]
integers3 = [1, 2])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, Tables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)code";
    std::string ssTOMLOutput = R"code([table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([table]

[table-1]
key1 = "some string"
key2 = 123

[table-2]
key1 = "another string"
key2 = 456)code";
    std::string ssTOMLOutput = R"code([tree.branch.table-1]
key1 = "some string"
key2 = 123

[tree.branch.table-2]
key1 = "another string"
key2 = 456)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, QuotedKeyTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([dog."tater.man"]
type.name = "pug")code";
    std::string ssTOMLOutput = R"code([dog."tater.man".type]
name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferQuotedKeyTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([dog."tater.man"]
type.name = "pug")code";
    std::string ssTOMLOutput = R"code([tree.branch.dog."tater.man".type]
name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, WhitespaceKeyTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = u8R"code([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)code";
    std::string ssTOMLOutput = u8R"code([a.b.c]
x = 1

[d.e.f]
y = 1

[g.h.i]
z = 1

[j."ʞ".'l']
a = 1)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferWhitespaceKeyTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = u8R"code([a.b.c]            # this is best practice
x = 1
[ d.e.f ]          # same as [d.e.f]
y = 1
[ g .  h  . i ]    # same as [g.h.i]
z = 1
[ j . "ʞ" . 'l' ]  # same as [j."ʞ".'l']
a = 1)code";
    std::string ssTOMLOutput = u8R"code([tree.branch.a.b.c]
x = 1

[tree.branch.d.e.f]
y = 1

[tree.branch.g.h.i]
z = 1

[tree.branch.j."ʞ".'l']
a = 1)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MixedOrderTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)code";
    std::string ssTOMLOutput = R"code([fruit.apple]
a = 1

[fruit.orange]
aa = 11

[animal]
b = 2)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedOrderTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# VALID BUT DISCOURAGED
[fruit.apple]
a = 1
[animal]
b = 2
[fruit.orange]
aa = 11)code";
    std::string ssTOMLOutput = R"code([tree.branch.fruit.apple]
a = 1

[tree.branch.fruit.orange]
aa = 11

[tree.branch.animal]
b = 2)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MixedValueAndTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)code";
    std::string ssTOMLOutput = R"code(name = "Fido"
breed = "pug"

[owner]
name = "Regina Dogman"
member_since = 1999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedValueAndTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(# Top-level table begins.
name = "Fido"
breed = "pug"

# Top-level table ends.
[owner]
name = "Regina Dogman"
member_since = 1999)code";
    std::string ssTOMLOutput = R"code([tree.branch]
name = "Fido"
breed = "pug"

[tree.branch.owner]
name = "Regina Dogman"
member_since = 1999)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, AutomaticTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)code";
    std::string ssTOMLOutput = R"code([fruit.apple]
color = "red"

[fruit.apple.taste]
sweet = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferAutomaticTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(fruit.apple.color = "red"
fruit.apple.taste.sweet = true)code";
    std::string ssTOMLOutput = R"code([tree.branch.fruit.apple]
color = "red"

[tree.branch.fruit.apple.taste]
sweet = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MixedAutomaticTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)code";
    std::string ssTOMLOutput = R"code([fruit.apple]
color = "red"

[fruit.apple.taste]
sweet = true

[fruit.apple.texture]
smooth = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedAutomaticTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([fruit]
apple.color = "red"
apple.taste.sweet = true

# [fruit.apple]  # INVALID
# [fruit.apple.taste]  # INVALID

[fruit.apple.texture]  # you can add sub-tables
smooth = true)code";
    std::string ssTOMLOutput = R"code([tree.branch.fruit.apple]
color = "red"

[tree.branch.fruit.apple.taste]
sweet = true

[tree.branch.fruit.apple.texture]
smooth = true)code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, InlineTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })code";
    std::string ssTOMLOutput = R"code([name]
first = "Tom"
last = "Preston-Werner"

[point]
x = 1
y = 2

[animal.type]
name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTables)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(name = { first = "Tom", last = "Preston-Werner" }
point = { x = 1, y = 2 }
animal = { type.name = "pug" })code";
    std::string ssTOMLOutput = R"code([tree.branch.name]
first = "Tom"
last = "Preston-Werner"

[tree.branch.point]
x = 1
y = 2

[tree.branch.animal.type]
name = "pug")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TableArrays)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code([[products]]
name = "Hammer"
sku = 738594937

[[products]]  # empty table within the array

[[products]]
name = "Nail"
sku = 284758393

color = "gray")code";
    std::string ssTOMLOutput = R"code([[products]]
name = "Hammer"
sku = 738594937

[[products]]

[[products]]
name = "Nail"
sku = 284758393
color = "gray")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferTableArrays)
{
    CParserTOML parser;

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

[[tree.branch.products]]

[[tree.branch.products]]
name = "Nail"
sku = 284758393
color = "gray")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, MixedTableAndTableArrays)
{
    CParserTOML parser;

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
    std::string ssTOMLOutput = R"code([[fruits]]
name = "apple"

[fruits.physical]
color = "red"
shape = "round"

[[fruits.varieties]]
name = "red delicious"

[[fruits.varieties]]
name = "granny smith"

[[fruits]]
name = "banana"

[[fruits.varieties]]
name = "plantain")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferMixedTableAndTableArrays)
{
    CParserTOML parser;

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

[tree.branch.fruits.physical]
color = "red"
shape = "round"

[[tree.branch.fruits.varieties]]
name = "red delicious"

[[tree.branch.fruits.varieties]]
name = "granny smith"

[[tree.branch.fruits]]
name = "banana"

[[tree.branch.fruits.varieties]]
name = "plantain")code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, InlineTableArrays)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])code";
    std::string ssTOMLOutput = R"code(points = [{x = 1, y = 2, z = 3}, {x = 7, y = 8, z = 9}, {x = 2, y = 4, z = 8}])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText());
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

TEST(GenerateTOML, TransferInlineTableArrays)
{
    CParserTOML parser;

    std::string ssTOMLInput = R"code(points = [ { x = 1, y = 2, z = 3 },
           { x = 7, y = 8, z = 9 },
           { x = 2, y = 4, z = 8 } ])code";
    std::string ssTOMLOutput = R"code([tree.branch]
points = [{x = 1, y = 2, z = 3}, {x = 7, y = 8, z = 9}, {x = 2, y = 4, z = 8}])code";

    EXPECT_NO_THROW(parser.Process(ssTOMLInput));

    std::string ssGenerated;
    EXPECT_NO_THROW(ssGenerated = parser.CreateTOMLText("tree.branch"));
    Trim(ssGenerated);

    EXPECT_EQ(ssGenerated, ssTOMLOutput);
}

