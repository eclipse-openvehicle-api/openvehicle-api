#include "../../include/gtest_custom.h"
#include "dbc_parser_test.h"
#include "../../../global/dbcparser/dbcparser.cpp"
#include <iostream>
#include <fstream>

void CDbcParserTest::SetUpTestCase()
{}

void CDbcParserTest::TearDownTestCase()
{}

void CDbcParserTest::SetUp()
{}

void CDbcParserTest::TearDown()
{}

TEST_F(CDbcParserTest, EmptySource)
{
    EXPECT_NO_THROW(dbc::CDbcSource());

    dbc::CDbcSource src;
    EXPECT_NO_THROW(dbc::CDbcSource dst(src));

    EXPECT_NO_THROW(dbc::CDbcSource dst(std::move(src)));
}

TEST_F(CDbcParserTest, SourceString)
{
    std::string ssEmpty;
    EXPECT_NO_THROW(dbc::CDbcSource srcEmpty(ssEmpty));

    std::string ssSourceSpace = " ";
    EXPECT_NO_THROW(dbc::CDbcSource srcSpace(ssSourceSpace));

    std::string ssSource = "VERSION \"123\"";
    EXPECT_NO_THROW(dbc::CDbcSource src(ssSource));
}

TEST_F(CDbcParserTest, SourceFile)
{
    dbc::CDbcSource src;
    EXPECT_EQ(src.Path(), std::filesystem::path(""));

    std::ofstream fstreamFileEmpty("empty.dbc", std::ios::trunc);
    fstreamFileEmpty.close();
    EXPECT_NO_THROW(dbc::CDbcSource(std::filesystem::path("empty.dbc")));
    dbc::CDbcSource srcEmpty((std::filesystem::path("empty.dbc")));
    EXPECT_EQ(srcEmpty.Path(), std::filesystem::path("empty.dbc"));
    try
    {
        std::filesystem::remove("empty.dbc");
    } catch (const std::filesystem::filesystem_error&)
    {}

    std::ofstream fstreamFile("test.dbc", std::ios::trunc);
    fstreamFile << " ";
    fstreamFile.close();
    EXPECT_NO_THROW(dbc::CDbcSource(std::filesystem::path("test.dbc")));
    try
    {
        std::filesystem::remove("test.dbc");
    } catch (const std::filesystem::filesystem_error&)
    {}

    EXPECT_THROW(dbc::CDbcSource(std::filesystem::path("not_exist")), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, SourcePosition)
{
    std::string ssEmpty;
    dbc::CDbcSource srcEmpty(ssEmpty);
    EXPECT_EQ(srcEmpty.Pos(), 0);
    EXPECT_TRUE(srcEmpty.IsEOF());

    std::string ssSource = "VERSION \"123\"";
    dbc::CDbcSource src(ssSource);
    EXPECT_EQ(src.Pos(), 0);
    EXPECT_FALSE(src.IsEOF());
    src.Pos() += 5;
    EXPECT_EQ(src.Pos(), 5);
    EXPECT_FALSE(src.IsEOF());
    src.Pos() += 8;
    EXPECT_EQ(src.Pos(), 13);
    EXPECT_TRUE(src.IsEOF());
    src.Pos() = 100;
    EXPECT_EQ(src.Pos(), 100);
    EXPECT_TRUE(src.IsEOF());
    src.ResetPos();
    EXPECT_EQ(src.Pos(), 0);
    EXPECT_FALSE(src.IsEOF());
}

TEST_F(CDbcParserTest, SourcePosLock)
{
    std::string ssSource = "VERSION \"123\"";
    dbc::CDbcSource src(ssSource);
    src.Pos() += 5;
    EXPECT_EQ(src.Pos(), 5);

    // Automatic rollback
    {
        auto sPosLock = src.CreatePosLock();
        src.Pos() += 8;
        EXPECT_EQ(src.Pos(), 13);
    }
    EXPECT_EQ(src.Pos(), 5);

    // Rollback; automatic rollback disabled
    {
        auto sPosLock = src.CreatePosLock();
        src.Pos() += 8;
        EXPECT_EQ(src.Pos(), 13);
        sPosLock.Rollback();
        EXPECT_EQ(src.Pos(), 5);
        src.Pos()++;
    }
    EXPECT_EQ(src.Pos(), 6);

    // Promotion; automatic rollback disabled
    src.Pos() = 5;
    {
        auto sPosLock = src.CreatePosLock();
        src.Pos() += 8;
        EXPECT_EQ(src.Pos(), 13);
        sPosLock.Promote();
    }
    EXPECT_EQ(src.Pos(), 13);
}

TEST_F(CDbcParserTest, SourceContent)
{
    std::string ssSource = "VERSION \"123\"";
    dbc::CDbcSource src(ssSource);

    EXPECT_EQ(ssSource, src.Content());
    EXPECT_EQ(ssSource, src.ContentPtr());
    src.Pos() += 2;
    EXPECT_EQ(ssSource.substr(2), src.ContentPtr());
    EXPECT_EQ('R', src.CurrentChar());
}

TEST_F(CDbcParserTest, SourceLineColumn)
{
    std::string ssSource = R"code(VERSION ""


NS_ :
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_)code";

    dbc::CDbcSource src(ssSource);
    EXPECT_EQ(src.CalcLine(), 1);
    EXPECT_EQ(src.CalcColumn(), 1);
    src.Pos() += 30;
    EXPECT_EQ(src.CalcLine(), 5);
    EXPECT_EQ(src.CalcColumn(), 12);
    src.Pos() += 99999;
    EXPECT_EQ(src.CalcLine(), 14);
    EXPECT_EQ(src.CalcColumn(), 13);

    std::string ssTabs = "\t\na\t\nabcd\t\nabcde\td\t";
    dbc::CDbcSource srcTabs(ssTabs);
    EXPECT_EQ(srcTabs.CalcLine(), 1);
    EXPECT_EQ(srcTabs.CalcColumn(), 1);
    srcTabs.Pos()++;
    EXPECT_EQ(srcTabs.CalcLine(), 1);
    EXPECT_EQ(srcTabs.CalcColumn(), 5);
    srcTabs.Pos()++;
    EXPECT_EQ(srcTabs.CalcLine(), 2);
    EXPECT_EQ(srcTabs.CalcColumn(), 1);
    srcTabs.Pos() += 2;
    EXPECT_EQ(srcTabs.CalcLine(), 2);
    EXPECT_EQ(srcTabs.CalcColumn(), 5);
    srcTabs.Pos() += 6;
    EXPECT_EQ(srcTabs.CalcLine(), 3);
    EXPECT_EQ(srcTabs.CalcColumn(), 9);
    srcTabs.Pos() += 7;
    EXPECT_EQ(srcTabs.CalcLine(), 4);
    EXPECT_EQ(srcTabs.CalcColumn(), 9);
    srcTabs.Pos() += 2;
    EXPECT_EQ(srcTabs.CalcLine(), 4);
    EXPECT_EQ(srcTabs.CalcColumn(), 13);
    srcTabs.Pos()++;
    EXPECT_EQ(srcTabs.CalcLine(), 4);
    EXPECT_EQ(srcTabs.CalcColumn(), 13);
}

TEST_F(CDbcParserTest, ExceptionStructTest)
{
    dbc::SDbcParserException sExceptReason("This is an exception");
    EXPECT_STREQ(sExceptReason.what(), "This is an exception");

    dbc::SDbcParserException sExceptParamMiddle("This is %1 exception", "an");
    EXPECT_STREQ(sExceptParamMiddle.what(), "This is an exception");
    dbc::SDbcParserException sExceptParamMiddleNumber("This is %1 exception", 1);
    EXPECT_STREQ(sExceptParamMiddleNumber.what(), "This is 1 exception");
    int i = 1;
    dbc::SDbcParserException sExceptParamMiddleNumber2("This is %1 exception", i);
    EXPECT_STREQ(sExceptParamMiddleNumber2.what(), "This is 1 exception");

    dbc::SDbcParserException sExceptParamBegin("%1 is an exception", "This");
    EXPECT_STREQ(sExceptParamBegin.what(), "This is an exception");

    dbc::SDbcParserException sExceptParamEnd("This is an %1", "exception");
    EXPECT_STREQ(sExceptParamEnd.what(), "This is an exception");

    dbc::SDbcParserException sExceptParamMultiple("%1 %2 %3 %4", "This", "is", "an", "exception");
    EXPECT_STREQ(sExceptParamMultiple.what(), "This is an exception");
    dbc::SDbcParserException sExceptParamMultipleMixed("%3 %4 %1 %2", "an", "exception", "This", "is");
    EXPECT_STREQ(sExceptParamMultipleMixed.what(), "This is an exception");
    dbc::SDbcParserException sExceptParamMultipleUnused("This is %3 exception", "This", "is", "an", "exception");
    EXPECT_STREQ(sExceptParamMultipleUnused.what(), "This is an exception");

    dbc::SDbcParserException sExceptParamNoEscape("This is an %% exception");
    EXPECT_STREQ(sExceptParamNoEscape.what(), "This is an %% exception");

    dbc::SDbcParserException sExceptParamEscape("This is an %% %1", "exception");
    EXPECT_STREQ(sExceptParamEscape.what(), "This is an % exception");

    dbc::SDbcParserException sExceptParamInvalidIndex("%0 This %2 is an %1 %999", "exception");
    EXPECT_STREQ(sExceptParamInvalidIndex.what(), "<unknown> This <unknown> is an exception <unknown>");
}

TEST_F(CDbcParserTest, ExceptionStructTestWithSource)
{
    std::string ssSource = "\nVERSION \"123\"\n";
    dbc::CDbcSource src(ssSource);
    src.Pos() += 9;
    EXPECT_EQ(src.CalcLine(), 2);
    EXPECT_EQ(src.CalcColumn(), 9);

    dbc::SDbcParserException sExceptReason(src, "This is an exception");
    EXPECT_STREQ(sExceptReason.what(), "[2, 9]: This is an exception");

    dbc::SDbcParserException sExceptParamMiddle(src, "This is %1 exception", "an");
    EXPECT_STREQ(sExceptParamMiddle.what(), "[2, 9]: This is an exception");
    dbc::SDbcParserException sExceptParamMiddleNumber(src, "This is %1 exception", 1);
    EXPECT_STREQ(sExceptParamMiddleNumber.what(), "[2, 9]: This is 1 exception");
    int i = 1;
    dbc::SDbcParserException sExceptParamMiddleNumber2(src, "This is %1 exception", i);
    EXPECT_STREQ(sExceptParamMiddleNumber2.what(), "[2, 9]: This is 1 exception");

    dbc::SDbcParserException sExceptParamBegin(src, "%1 is an exception", "This");
    EXPECT_STREQ(sExceptParamBegin.what(), "[2, 9]: This is an exception");

    dbc::SDbcParserException sExceptParamEnd(src, "This is an %1", "exception");
    EXPECT_STREQ(sExceptParamEnd.what(), "[2, 9]: This is an exception");

    dbc::SDbcParserException sExceptParamMultiple(src, "%1 %2 %3 %4", "This", "is", "an", "exception");
    EXPECT_STREQ(sExceptParamMultiple.what(), "[2, 9]: This is an exception");
    dbc::SDbcParserException sExceptParamMultipleMixed(src, "%3 %4 %1 %2", "an", "exception", "This", "is");
    EXPECT_STREQ(sExceptParamMultipleMixed.what(), "[2, 9]: This is an exception");
    dbc::SDbcParserException sExceptParamMultipleUnused(src, "This is %3 exception", "This", "is", "an", "exception");
    EXPECT_STREQ(sExceptParamMultipleUnused.what(), "[2, 9]: This is an exception");

    dbc::SDbcParserException sExceptParamNoEscape(src, "This is an %% exception");
    EXPECT_STREQ(sExceptParamNoEscape.what(), "[2, 9]: This is an %% exception");

    dbc::SDbcParserException sExceptParamEscape(src, "This is an %% %1", "exception");
    EXPECT_STREQ(sExceptParamEscape.what(), "[2, 9]: This is an % exception");

    dbc::SDbcParserException sExceptParamInvalidIndex(src, "%0 This %2 is an %1 %999", "exception");
    EXPECT_STREQ(sExceptParamInvalidIndex.what(), "[2, 9]: <unknown> This <unknown> is an exception <unknown>");

    dbc::SDbcParserException sExceptReasonStablePosition(src, "This is an exception");
    src.Pos() += 999;
    EXPECT_EQ(src.CalcLine(), 3);
    EXPECT_EQ(src.CalcColumn(), 1);
    EXPECT_EQ(sExceptReasonStablePosition.Source().CalcLine(), 2);
    EXPECT_EQ(sExceptReasonStablePosition.Source().CalcColumn(), 9);

    src.Pos() = 9;
    dbc::SDbcParserException sExceptParamMiddleStablePosition(src, "This is %1 exception", "an");
    src.Pos() += 999;
    EXPECT_EQ(src.CalcLine(), 3);
    EXPECT_EQ(src.CalcColumn(), 1);
    EXPECT_EQ(sExceptParamMiddleStablePosition.Source().CalcLine(), 2);
    EXPECT_EQ(sExceptParamMiddleStablePosition.Source().CalcColumn(), 9);
}

TEST_F(CDbcParserTest, Instantiation)
{
    EXPECT_NO_THROW(dbc::CDbcParser());
}

TEST_F(CDbcParserTest, Version)
{
    std::string ssVersion1 = "VERSION \"123\"";
    dbc::CDbcSource srcVersion1(ssVersion1);
    std::string ssVersion2 = "VERSION \"\"";
    dbc::CDbcSource srcVersion2(ssVersion2);
    std::string ssVersion3 = "\n  VERSION \"123\"\n";
    dbc::CDbcSource srcVersion3(ssVersion3);
    std::string ssMaliciousVersion = "VERSION 123\n";
    dbc::CDbcSource srcMaliciousVersion(ssMaliciousVersion);

    dbc::CDbcParser parser1;
    EXPECT_NO_THROW(parser1.Parse(srcVersion1));
    EXPECT_EQ(parser1.GetVersions(), std::vector<std::string>({ "123" }));

    dbc::CDbcParser parser2;
    EXPECT_NO_THROW(parser2.Parse(srcVersion2));
    EXPECT_EQ(parser2.GetVersions(), std::vector<std::string>({ "" }));

    dbc::CDbcParser parser3;
    EXPECT_NO_THROW(parser3.Parse(srcVersion3));
    EXPECT_EQ(parser3.GetVersions(), std::vector<std::string>({ "123" }));

    dbc::CDbcParser parserMaliciousVersion;
    EXPECT_THROW(parserMaliciousVersion.Parse(srcMaliciousVersion), dbc::SDbcParserException);
    EXPECT_EQ(parserMaliciousVersion.GetVersions(), std::vector<std::string>());

    dbc::CDbcParser parserMulti;
    srcVersion1.ResetPos();
    srcVersion2.ResetPos();
    srcVersion3.ResetPos();
    EXPECT_NO_THROW(parserMulti.Parse(srcVersion1));
    EXPECT_NO_THROW(parserMulti.Parse(srcVersion2));
    EXPECT_NO_THROW(parserMulti.Parse(srcVersion3));
    EXPECT_EQ(parserMulti.GetVersions(), std::vector<std::string>({ "123", "", "123" }));
}

TEST_F(CDbcParserTest, Comments)
{
    // NOTE: Comments are not part of the official standard.
    std::string ssComment1 = "// test 123";
    dbc::CDbcSource srcComment1(ssComment1);
    dbc::CDbcParser parser;
    EXPECT_NO_THROW(parser.Parse(srcComment1));
    std::string ssComment2 = "VERSION \"\" // test 123";
    dbc::CDbcSource srcComment2(ssComment2);
    EXPECT_NO_THROW(parser.Parse(srcComment2));
}

TEST_F(CDbcParserTest, NewSymbols)
{
    dbc::CDbcParser parser;

    // No symbols.
    std::string ssEmptyNS = "NS_:";
    dbc::CDbcSource srcEmptyNS(ssEmptyNS);
    EXPECT_NO_THROW(parser.Parse(srcEmptyNS));

    // One Symbol
    std::string ssOneNS = "NS_: VAL_TABLE_";
    dbc::CDbcSource srcOneNS(ssOneNS);
    EXPECT_NO_THROW(parser.Parse(srcOneNS));

    // All symbols
    std::string ssAllNS = R"code(NS_ : NS_DESC_ CM_ BA_DEF_ BA_ VAL_ CAT_DEF_ CAT_ FILTER BA_DEF_DEF_ EV_DATA_
        ENVVAR_DATA_ SGTYPE_ SGTYPE_VAL_ BA_DEF_SGTYPE_ BA_SGTYPE_ SIG_TYPE_REF_ VAL_TABLE_ SIG_GROUP_
        SIG_VALTYPE_ SIGTYPE_VALTYPE_ BO_TX_BU_ BA_DEF_REL_ BA_REL_ BA_DEF_DEF_REL_ BU_SG_REL_
        BU_EV_REL_ BU_BO_REL_ SG_MUL_VAL_)code";
    dbc::CDbcSource srcAllNS(ssAllNS);
    EXPECT_NO_THROW(parser.Parse(srcAllNS));

    // Duplicate symbols
    std::string ssDuplicateNS = "NS_: VAL_TABLE_ VAL_TABLE_";
    dbc::CDbcSource srcDuplicateNS(ssDuplicateNS);
    EXPECT_NO_THROW(parser.Parse(srcDuplicateNS));

    // Invalid symbols
    std::string ssInvalidNS = "NS_: INVALID_";
    dbc::CDbcSource srcInvalidNS(ssInvalidNS);
    EXPECT_THROW(parser.Parse(srcInvalidNS), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, BitTiming)
{
    dbc::CDbcParser parser;

    // Valid bit timing
    std::string ssValidBitTiming = "BS_: 500000 : 4, 3";
    dbc::CDbcSource srcValidBitTiming(ssValidBitTiming);
    EXPECT_NO_THROW(parser.Parse(srcValidBitTiming));

    // No bit timing
    std::string ssNoBitTiming = "BS_:";
    dbc::CDbcSource srcNoBitTiming(ssNoBitTiming);
    EXPECT_NO_THROW(parser.Parse(srcNoBitTiming));

    // Invalid bit timing
    std::string ssInvalidBitTiming1 = "BS_: 500000 : 4";
    std::string ssInvalidBitTiming2 = "BS_: 500000 : ";
    std::string ssInvalidBitTiming3 = "BS_: 500000";
    std::string ssInvalidBitTiming4 = "BS_: 4, 3";
    std::string ssInvalidBitTiming5 = "BS_: 500000 : a, b";
    dbc::CDbcSource srcInvalidBitTiming1(ssInvalidBitTiming1);
    dbc::CDbcSource srcInvalidBitTiming2(ssInvalidBitTiming2);
    dbc::CDbcSource srcInvalidBitTiming3(ssInvalidBitTiming3);
    dbc::CDbcSource srcInvalidBitTiming4(ssInvalidBitTiming4);
    dbc::CDbcSource srcInvalidBitTiming5(ssInvalidBitTiming5);
    EXPECT_THROW(parser.Parse(srcInvalidBitTiming1), dbc::SDbcParserException);
    EXPECT_THROW(parser.Parse(srcInvalidBitTiming2), dbc::SDbcParserException);
    EXPECT_THROW(parser.Parse(srcInvalidBitTiming3), dbc::SDbcParserException);
    EXPECT_THROW(parser.Parse(srcInvalidBitTiming4), dbc::SDbcParserException);
    EXPECT_THROW(parser.Parse(srcInvalidBitTiming5), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, NodeDef)
{
    dbc::CDbcParser parser;

    // Valid node def
    std::string ssValidNodeDef = "BU_: MAB SAS RASM BBM";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("MAB"));
    EXPECT_TRUE(parser.HasNodeDef("SAS"));
    EXPECT_TRUE(parser.HasNodeDef("RASM"));
    EXPECT_TRUE(parser.HasNodeDef("BBM"));
    EXPECT_FALSE(parser.HasNodeDef("MSRB"));
    auto vecNodes = parser.GetNodeDefNames();
    EXPECT_EQ(vecNodes.size(), 4);
    EXPECT_NE(std::find(vecNodes.begin(), vecNodes.end(), "MAB"), vecNodes.end());
    EXPECT_NE(std::find(vecNodes.begin(), vecNodes.end(), "SAS"), vecNodes.end());
    EXPECT_NE(std::find(vecNodes.begin(), vecNodes.end(), "RASM"), vecNodes.end());
    EXPECT_NE(std::find(vecNodes.begin(), vecNodes.end(), "BBM"), vecNodes.end());

    // Duplicate node def (in one call, invliad, in separate calls valid).
    std::string ssDuplicateNodeDef1 = "BU_: XYZ SAS";
    dbc::CDbcSource srcDuplicateNodeDef1(ssDuplicateNodeDef1);
    EXPECT_NO_THROW(parser.Parse(srcDuplicateNodeDef1));
    EXPECT_TRUE(parser.HasNodeDef("XYZ"));
    std::string ssDuplicateNodeDef2 = "BU_: ABC DEF ABC";
    dbc::CDbcSource srcDuplicateNodeDef2(ssDuplicateNodeDef2);
    EXPECT_THROW(parser.Parse(srcDuplicateNodeDef2), dbc::SDbcParserException);
    EXPECT_FALSE(parser.HasNodeDef("ABC"));
    EXPECT_FALSE(parser.HasNodeDef("DEF"));

    // No node def
    std::string ssNoNodeDef = "BU_:";
    dbc::CDbcSource srcNoNodeDef(ssNoNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcNoNodeDef));

    // Invalid node defs
    std::string ssInvalidNodeDef1 = "BU_: 123";
    dbc::CDbcSource srcInvalidNodeDef1(ssInvalidNodeDef1);
    EXPECT_THROW(parser.Parse(srcInvalidNodeDef1), dbc::SDbcParserException);
    std::string ssInvalidNodeDef2 = "BU_: BS_";
    dbc::CDbcSource srcInvalidNodeDef2(ssInvalidNodeDef2);
    EXPECT_THROW(parser.Parse(srcInvalidNodeDef2), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, ValueTable)
{
    dbc::CDbcParser parser;

    // Value table
    std::string ssValidValueTable1 = "VAL_TABLE_ table1 10 \"ten\" 20 \"twenty\";";
    dbc::CDbcSource srcValidValueTable1(ssValidValueTable1);
    EXPECT_NO_THROW(parser.Parse(srcValidValueTable1));
    EXPECT_TRUE(parser.HasValueTable("table1"));
    EXPECT_TRUE(parser.HasValue("table1", 10));
    EXPECT_TRUE(parser.HasValue("table1", 20));
    EXPECT_EQ(parser.GetValue("table1", 10), "ten");
    EXPECT_EQ(parser.GetValue("table1", 20), "twenty");
    std::string ssValidValueTable2 = "VAL_TABLE_ table2 20 \"twenty\" 30 \"thirty\";"; // Redefinition of value names allowed
    dbc::CDbcSource srcValidValueTable2(ssValidValueTable2);
    EXPECT_NO_THROW(parser.Parse(srcValidValueTable2));
    auto vecTables = parser.GetValueTableNames();
    ASSERT_EQ(vecTables.size(), 2);
    EXPECT_EQ(vecTables[0], "table1");
    EXPECT_EQ(vecTables[1], "table2");
    auto prEntries = parser.GetValues("table1");
    EXPECT_TRUE(prEntries.second);
    ASSERT_EQ(prEntries.first.size(), 2);
    EXPECT_EQ(prEntries.first[0], 10u);
    EXPECT_EQ(prEntries.first[1], 20u);
    prEntries = parser.GetValues("table2");
    EXPECT_TRUE(prEntries.second);
    ASSERT_EQ(prEntries.first.size(), 2);
    EXPECT_EQ(prEntries.first[0], 20u);
    EXPECT_EQ(prEntries.first[1], 30u);

    // Empty table
    std::string ssEmptyValueTable = "VAL_TABLE_ empty;";
    dbc::CDbcSource srcEmptyValueTable(ssEmptyValueTable);
    EXPECT_NO_THROW(parser.Parse(srcEmptyValueTable));
    EXPECT_TRUE(parser.HasValueTable("empty"));

    // Invalid table
    std::string ssNoValueTable = "VAL_TABLE_;";
    dbc::CDbcSource srcNoValueTable(ssNoValueTable);
    EXPECT_THROW(parser.Parse(srcNoValueTable), dbc::SDbcParserException);
    std::string ssDuplicateValueTable1 = "VAL_TABLE_ table1;"; // Redefinition of "table1"
    dbc::CDbcSource srcDuplicateValueTable1(ssDuplicateValueTable1);
    EXPECT_THROW(parser.Parse(srcDuplicateValueTable1), dbc::SDbcParserException);
    std::string ssDuplicateValueTable2 = "VAL_TABLE_ duplicate_val_table 10 \"ten\" 20 \"twenty\" 10 \"ten\";"; // Duplicate values
    dbc::CDbcSource srcDuplicateValueTable2(ssDuplicateValueTable2);
    EXPECT_THROW(parser.Parse(srcDuplicateValueTable2), dbc::SDbcParserException);
    std::string ssInvalidValueTable1 = "VAL_TABLE_ invalid_table \"ten\" 10 \"twenty\" 20;";   // Wrong order of values
    dbc::CDbcSource srcInvalidValueTable1(ssInvalidValueTable1);
    EXPECT_THROW(parser.Parse(srcInvalidValueTable1), dbc::SDbcParserException);
    std::string ssInvalidValueTable2 = "VAL_TABLE_ invalid_table 10 \"ten\" 20 \"twenty\"";    // Semi-colon missing
    dbc::CDbcSource srcInvalidValueTable2(ssInvalidValueTable2);
    EXPECT_THROW(parser.Parse(srcInvalidValueTable2), dbc::SDbcParserException);
    std::string ssDuplicateValueTable3 = "VAL_TABLE_: table3;"; // Invalid colon
    dbc::CDbcSource srcDuplicateValueTable3(ssDuplicateValueTable3);
    EXPECT_THROW(parser.Parse(srcDuplicateValueTable3), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, MessageDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));

    // Message definition
    std::string ssValidMsgDefStdId = "BO_ 1 msg1: 8 nodeTx";
    dbc::CDbcSource srcValidMsgDefStdId(ssValidMsgDefStdId);
    EXPECT_NO_THROW(parser.Parse(srcValidMsgDefStdId));
    EXPECT_TRUE(parser.HasMsgDef("msg1"));
    EXPECT_TRUE(parser.HasMsgDefStdId(1u));
    EXPECT_FALSE(parser.HasMsgDefExtId(1u));
    EXPECT_TRUE(parser.HasMsgDef(1u));
    auto prMsgDef = parser.GetMsgDef("msg1");
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg1");
    prMsgDef = parser.GetMsgDef(1u);
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg1");
    prMsgDef = parser.GetMsgDefStdId(1u);
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg1");
    prMsgDef = parser.GetMsgDefExtId(1u);
    EXPECT_FALSE(prMsgDef.second);
    EXPECT_TRUE(prMsgDef.first.ssName.empty());
    std::string ssValidMsgDefExtId = "BO_ 2147483650 msg2: 8 nodeTx";
    dbc::CDbcSource srcValidMsgDefExtId(ssValidMsgDefExtId);
    EXPECT_NO_THROW(parser.Parse(srcValidMsgDefExtId));
    EXPECT_TRUE(parser.HasMsgDef("msg2"));
    EXPECT_FALSE(parser.HasMsgDefStdId(2));
    EXPECT_TRUE(parser.HasMsgDefExtId(2));
    EXPECT_TRUE(parser.HasMsgDef(2u | 0x80000000u));
    prMsgDef = parser.GetMsgDef("msg2");
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg2");
    prMsgDef = parser.GetMsgDef(2u | 0x80000000u);
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg2");
    prMsgDef = parser.GetMsgDefStdId(2u);
    EXPECT_FALSE(prMsgDef.second);
    EXPECT_TRUE(prMsgDef.first.ssName.empty());
    prMsgDef = parser.GetMsgDefExtId(2u);
    EXPECT_TRUE(prMsgDef.second);
    EXPECT_EQ(prMsgDef.first.ssName, "msg2");
    auto vecMessagIDs = parser.GetMessageIDs();
    ASSERT_EQ(vecMessagIDs.size(), 2);
    EXPECT_EQ(vecMessagIDs[0], 1u);
    EXPECT_EQ(vecMessagIDs[1], 2u | 0x80000000u);

    // Default node
    std::string ssDefaultNode = "BO_ 3 msg3: 8 Vector__XXX";
    dbc::CDbcSource srcDefaultNode(ssDefaultNode);
    EXPECT_NO_THROW(parser.Parse(srcDefaultNode));
    EXPECT_TRUE(parser.HasMsgDef("msg3"));
    EXPECT_TRUE(parser.HasMsgDefStdId(3));
    EXPECT_FALSE(parser.HasMsgDefExtId(3));

    // Default message
    std::string ssDefaultMsg = "BO_ 1 VECTOR__INDEPENDENT_SIG_MSG: 8 nodeTx BO_ 2 VECTOR__INDEPENDENT_SIG_MSG: 8 Vector__XXX";
    dbc::CDbcSource srcDefaultMsg(ssDefaultMsg);
    EXPECT_NO_THROW(parser.Parse(srcDefaultMsg));
    EXPECT_TRUE(parser.HasMsgDef("VECTOR__INDEPENDENT_SIG_MSG"));

    // Invalid message
    std::string ssDuplicateMsg = "BO_ 1 msg1: 7 nodeTx";    // Duplicate through name; defined before already
    dbc::CDbcSource srcDuplicateMsg(ssDuplicateMsg);
    EXPECT_THROW(parser.Parse(srcDuplicateMsg), dbc::SDbcParserException);
    std::string ssDuplicateMsgId = "BO_ 1 msg11: 8 nodeTx"; // Duplicate through Id.
    dbc::CDbcSource srcDuplicateMsgId(ssDuplicateMsgId);
    EXPECT_THROW(parser.Parse(srcDuplicateMsgId), dbc::SDbcParserException);
    std::string ssInvalidMsg1 = "BO_ 123 msg123: 8";    // Missing transmitter node
    dbc::CDbcSource srcInvalidMsg1(ssInvalidMsg1);
    EXPECT_THROW(parser.Parse(srcInvalidMsg1), dbc::SDbcParserException);
    std::string ssInvalidMsg2 = "BO_ 123 msg123: nodeTx";    // Missing message size
    dbc::CDbcSource srcInvalidMsg2(ssInvalidMsg2);
    EXPECT_THROW(parser.Parse(srcInvalidMsg2), dbc::SDbcParserException);
    std::string ssInvalidMsg3 = "BO_ 123 msg123 8 nodeTx";    // Missing colon
    dbc::CDbcSource srcInvalidMsg3(ssInvalidMsg3);
    EXPECT_THROW(parser.Parse(srcInvalidMsg3), dbc::SDbcParserException);
    std::string ssInvalidMsg4 = "BO_ 123 : 8 nodeTx";    // Missing name
    dbc::CDbcSource srcInvalidMsg4(ssInvalidMsg4);
    EXPECT_THROW(parser.Parse(srcInvalidMsg4), dbc::SDbcParserException);
    std::string ssInvalidMsg5 = "BO_ msg123: 8 nodeTx";    // Missing ID
    dbc::CDbcSource srcInvalidMsg5(ssInvalidMsg5);
    EXPECT_THROW(parser.Parse(srcInvalidMsg5), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, SignalDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx1 nodeRx2";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx1"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx2"));

    // Message definition
    std::string ssValidMsgDef1 = R"code(
        BO_ 1 msg_big_endian: 8 nodeTx
            SG_ sig1 : 7|4@0+ (1,0) [0|8191] "Nm" nodeRx1
            SG_ sig2 : 3|8@0+ (1,0) [0|8191] "Nm" nodeRx1, nodeRx2
            SG_ sig3 : 11|6@0+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig4 : 21|6@0+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig5 : 31|8@0+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig6 : 39|32@0+ (1,0) [0|8191] "Nm" Vector__XXX
        BO_ 2 msg_little_endian: 8 nodeTx
            SG_ sig1 : 0|4@1+ (1,0) [0|8191] "Nm" nodeRx1
            SG_ sig2 : 4|8@1+ (1,0) [0|8191] "Nm" nodeRx1, nodeRx2
            SG_ sig3 : 12|6@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig4 : 18|6@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig5 : 24|8@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig6 : 32|32@1+ (1,0) [0|8191] "Nm" Vector__XXX
        BO_ 2147483651 msg_multiplexer: 8 nodeTx
            SG_ sig1 M : 0|4@1+ (1,0) [0|0] "" nodeRx1
            SG_ sig2a m 1: 4|8@1+ (1,0) [0|8191] "Nm" nodeRx1, nodeRx2
            SG_ sig3a m 1: 12|6@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig4a m 1: 18|6@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig5a m 1: 24|8@1+ (1,0) [0|8191] "Nm" Vector__XXX
            SG_ sig2b m 0: 4|32@1+ (1,0) [0|8191] "Nm" nodeRx1, nodeRx2
            SG_ sig6 : 32|32@1+ (1,0) [0|8191] "Nm" Vector__XXX
)code";
    dbc::CDbcSource srcValidMsgDef1(ssValidMsgDef1);
    EXPECT_NO_THROW(parser.Parse(srcValidMsgDef1));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig1"));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig2"));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig3"));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig4"));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig5"));
    EXPECT_TRUE(parser.HasSignalDef("msg_big_endian", "sig6"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig1"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig2"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig3"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig4"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig5"));
    EXPECT_TRUE(parser.HasSignalDef("msg_little_endian", "sig6"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig1"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig2a"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig3a"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig4a"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig5a"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig2b"));
    EXPECT_TRUE(parser.HasSignalDef("msg_multiplexer", "sig6"));
    EXPECT_TRUE(parser.HasSignalDef(1, "sig1"));
    EXPECT_TRUE(parser.HasSignalDef(1, "sig6"));
    EXPECT_TRUE(parser.HasSignalDef(2, "sig1"));
    EXPECT_TRUE(parser.HasSignalDef(2, "sig6"));
    EXPECT_TRUE(parser.HasSignalDef(2147483651, "sig1"));
    EXPECT_TRUE(parser.HasSignalDef(2147483651, "sig6"));
    EXPECT_TRUE(parser.HasSignalDefStdId(1, "sig1"));
    EXPECT_TRUE(parser.HasSignalDefStdId(1, "sig6"));
    EXPECT_TRUE(parser.HasSignalDefStdId(2, "sig1"));
    EXPECT_TRUE(parser.HasSignalDefStdId(2, "sig6"));
    EXPECT_FALSE(parser.HasSignalDefStdId(3, "sig1"));
    EXPECT_FALSE(parser.HasSignalDefStdId(3, "sig6"));
    EXPECT_FALSE(parser.HasSignalDefExtId(1, "sig1"));
    EXPECT_FALSE(parser.HasSignalDefExtId(1, "sig6"));
    EXPECT_FALSE(parser.HasSignalDefExtId(2, "sig1"));
    EXPECT_FALSE(parser.HasSignalDefExtId(2, "sig6"));
    EXPECT_TRUE(parser.HasSignalDefExtId(3, "sig1"));
    EXPECT_TRUE(parser.HasSignalDefExtId(3, "sig6"));
    std::vector<std::string> vecSignals = parser.GetSignalNames(1);
    ASSERT_EQ(vecSignals.size(), 6u);
    EXPECT_EQ(vecSignals[0], "sig1");
    EXPECT_EQ(vecSignals[1], "sig2");
    EXPECT_EQ(vecSignals[2], "sig3");
    EXPECT_EQ(vecSignals[3], "sig4");
    EXPECT_EQ(vecSignals[4], "sig5");
    EXPECT_EQ(vecSignals[5], "sig6");
    EXPECT_TRUE(parser.GetSignalDef("msg_big_endian", "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef("msg_big_endian", "sig6").second);
    EXPECT_TRUE(parser.GetSignalDef("msg_little_endian", "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef("msg_little_endian", "sig6").second);
    EXPECT_TRUE(parser.GetSignalDef("msg_multiplexer", "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef("msg_multiplexer", "sig6").second);
    EXPECT_TRUE(parser.GetSignalDef(1, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef(1, "sig6").second);
    EXPECT_TRUE(parser.GetSignalDef(2, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef(2, "sig6").second);
    EXPECT_TRUE(parser.GetSignalDef(2147483651, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDef(2147483651, "sig6").second);
    EXPECT_TRUE(parser.GetSignalDefStdId(1, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDefStdId(1, "sig6").second);
    EXPECT_TRUE(parser.GetSignalDefStdId(2, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDefStdId(2, "sig6").second);
    EXPECT_FALSE(parser.GetSignalDefStdId(3, "sig1").second);
    EXPECT_FALSE(parser.GetSignalDefStdId(3, "sig6").second);
    EXPECT_FALSE(parser.GetSignalDefExtId(1, "sig1").second);
    EXPECT_FALSE(parser.GetSignalDefExtId(1, "sig6").second);
    EXPECT_FALSE(parser.GetSignalDefExtId(2, "sig1").second);
    EXPECT_FALSE(parser.GetSignalDefExtId(2, "sig6").second);
    EXPECT_TRUE(parser.GetSignalDefExtId(3, "sig1").second);
    EXPECT_TRUE(parser.GetSignalDefExtId(3, "sig6").second);

    // Duplicate signals
    std::string ssDuplicateSigDef1 = R"code(
        BO_ 4 msg4: 8 nodeTx
            SG_ sig1 : 52|13@0+ (1,0) [0|8191] "Nm" nodeRx1
            SG_ sig2 : 36|13@0+ (1,0) [0|8191] "Nm" nodeRx1, nodeRx2
            SG_ sig1 : 18|13@0+ (1,0) [0|8191] "Nm" Vector__XXX
        )code";
    dbc::CDbcSource srcDuplicateSigDef1(ssDuplicateSigDef1);
    EXPECT_THROW(parser.Parse(srcDuplicateSigDef1), dbc::SDbcParserException);

    // Invalid signals
    std::string ssInvalidStartBit = R"code(
        BO_ 5 msg5: 8 nodeTx
            SG_ sig1 : 64|13@0+ (1,0) [0|8191] "Nm" nodeRx1
        )code";
    dbc::CDbcSource srcInvalidStartBit(ssInvalidStartBit);
    EXPECT_THROW(parser.Parse(srcInvalidStartBit), dbc::SDbcParserException);
    std::string ssInvalidLengthLittleEndian = R"code(
        BO_ 6 msg6: 8 nodeTx
            SG_ sig1 : 33|32@1+ (1,0) [0|8191] "Nm" nodeRx1
        )code";
    dbc::CDbcSource srcInvalidLengthLittleEndian(ssInvalidLengthLittleEndian);
    EXPECT_THROW(parser.Parse(srcInvalidLengthLittleEndian), dbc::SDbcParserException);
    std::string ssInvalidLengthBigEndian = R"code(
        BO_ 7 msg7: 8 nodeTx
            SG_ sig1 : 38|32@0+ (1,0) [0|8191] "Nm" nodeRx1
        )code";
    dbc::CDbcSource srcInvalidLengthBigEndian(ssInvalidLengthBigEndian);
    EXPECT_THROW(parser.Parse(srcInvalidLengthBigEndian), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, SignalTypeDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg_float: 8 nodeTx
            SG_ sig_float : 0|32@1- (1,0) [0|8191] "Nm" nodeRx
            SG_ sig_int : 32|32@1+ (1,0) [0|8191] "Nm" nodeRx
        BO_ 2 msg_double: 8 nodeTx
            SG_ sig_double : 0|64@1- (1,0) [0|8191] "Nm" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
    EXPECT_TRUE(parser.HasMsgDef("msg_float"));
    EXPECT_TRUE(parser.HasMsgDef("msg_double"));
    auto prSigFloat = parser.GetSignalDef("msg_float", "sig_float");
    EXPECT_TRUE(prSigFloat.second);
    EXPECT_EQ(prSigFloat.first.eValType, dbc::SSignalDef::EValueType::signed_integer);
    auto prSigInt = parser.GetSignalDef("msg_float", "sig_int");
    EXPECT_TRUE(prSigInt.second);
    EXPECT_EQ(prSigInt.first.eValType, dbc::SSignalDef::EValueType::unsigned_integer);
    auto prSigDouble = parser.GetSignalDef("msg_double", "sig_double");
    EXPECT_TRUE(prSigDouble.second);
    EXPECT_EQ(prSigDouble.first.eValType, dbc::SSignalDef::EValueType::signed_integer);

    // Set the signal value types.
    std::string ssSigValType = R"code(
        SIG_VALTYPE_ 1 sig_float 1;
        SIG_VALTYPE_ 1 sig_int 0;
        SIG_VALTYPE_ 2 sig_double 2;
        )code";
    dbc::CDbcSource srcSigValType(ssSigValType);
    EXPECT_NO_THROW(parser.Parse(srcSigValType));

    // Test for int, IEEE float and double.
    prSigFloat = parser.GetSignalDef("msg_float", "sig_float");
    EXPECT_TRUE(prSigFloat.second);
    EXPECT_EQ(prSigFloat.first.eValType, dbc::SSignalDef::EValueType::ieee_float);
    prSigInt = parser.GetSignalDef("msg_float", "sig_int");
    EXPECT_TRUE(prSigInt.second);
    EXPECT_EQ(prSigInt.first.eValType, dbc::SSignalDef::EValueType::unsigned_integer);
    prSigDouble = parser.GetSignalDef("msg_double", "sig_double");
    EXPECT_TRUE(prSigDouble.second);
    EXPECT_EQ(prSigDouble.first.eValType, dbc::SSignalDef::EValueType::ieee_double);
}

TEST_F(CDbcParserTest, TransmitterMsgDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeTx2 nodeTx3";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx2"));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx3"));

    // Message definition
    std::string ssMsgDef = "BO_ 1 msg1: 8 nodeTx";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
    auto prMsgDef = parser.GetMsgDef(1);
    EXPECT_TRUE(prMsgDef.second);
    ASSERT_EQ(prMsgDef.first.vecTransmitters.size(), 1);
    EXPECT_EQ(prMsgDef.first.vecTransmitters[0], "nodeTx");

    // Add additional transmitters
    std::string ssMsgTxDef = "BO_TX_BU_ 1: nodeTx2 nodeTx3 Vector__XXX;";
    dbc::CDbcSource srcMsgTxDef(ssMsgTxDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgTxDef));
    prMsgDef = parser.GetMsgDef(1);
    EXPECT_TRUE(prMsgDef.second);
    ASSERT_EQ(prMsgDef.first.vecTransmitters.size(), 4);
    EXPECT_EQ(prMsgDef.first.vecTransmitters[0], "nodeTx");
    EXPECT_EQ(prMsgDef.first.vecTransmitters[1], "nodeTx2");
    EXPECT_EQ(prMsgDef.first.vecTransmitters[2], "nodeTx3");
    EXPECT_EQ(prMsgDef.first.vecTransmitters[3], "Vector__XXX");

    // Empty transmitter definition
    std::string ssMsgTxDefEmpty = "BO_TX_BU_ 1:;";
    dbc::CDbcSource srcMsgTxDefEmpty(ssMsgTxDefEmpty);
    EXPECT_NO_THROW(parser.Parse(srcMsgTxDefEmpty));

    // Invalid transmitter definitions
    std::string ssMsgTxDefDuplicate1 = "BO_TX_BU_ 1: nodeTx2;";
    dbc::CDbcSource srcMsgTxDefDuplicate1(ssMsgTxDefDuplicate1);
    EXPECT_THROW(parser.Parse(srcMsgTxDefDuplicate1), dbc::SDbcParserException);
    std::string ssMsgTxDefDuplicate2 = "BO_TX_BU_ 1: Vector__XXX;";
    dbc::CDbcSource srcMsgTxDefDuplicate2(ssMsgTxDefDuplicate2);
    EXPECT_THROW(parser.Parse(srcMsgTxDefDuplicate2), dbc::SDbcParserException);
    std::string ssMsgTxDefMissingSemiColon = "BO_TX_BU_ 1:";
    dbc::CDbcSource srcMsgTxDefMissingSemiColon(ssMsgTxDefMissingSemiColon);
    EXPECT_THROW(parser.Parse(srcMsgTxDefMissingSemiColon), dbc::SDbcParserException);
    std::string ssMsgTxDefInvalidNode = "BO_TX_BU_ 1: abc;";
    dbc::CDbcSource srcMsgTxDefInvalidNode(ssMsgTxDefInvalidNode);
    EXPECT_THROW(parser.Parse(srcMsgTxDefInvalidNode), dbc::SDbcParserException);
    std::string ssMsgTxDefInvalidMsg = "BO_TX_BU_ 2: ;";
    dbc::CDbcSource srcMsgTxDefInvalidMsg(ssMsgTxDefInvalidMsg);
    EXPECT_THROW(parser.Parse(srcMsgTxDefInvalidMsg), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, SignalValueDescriptions)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg_enum: 8 nodeTx
            SG_ sig_enum : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig_enum_empty : 0|32@1+ (1,0) [0|0] "" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
    EXPECT_TRUE(parser.HasMsgDef("msg_enum"));
    EXPECT_TRUE(parser.HasSignalDef("msg_enum", "sig_enum"));
    auto prSigEnum = parser.GetSignalDef("msg_enum", "sig_enum");
    EXPECT_TRUE(prSigEnum.second);
    EXPECT_TRUE(prSigEnum.first.mapValueDescriptions.empty());

    // Signal value descriptions
    std::string ssSigValDesc = "VAL_ 1 sig_enum 10 \"ten\" 20 \"twenty\" 30 \"thirty\";";
    dbc::CDbcSource srcSigValDesc(ssSigValDesc);
    EXPECT_NO_THROW(parser.Parse(srcSigValDesc));
    prSigEnum = parser.GetSignalDef("msg_enum", "sig_enum");
    EXPECT_TRUE(prSigEnum.second);
    ASSERT_EQ(prSigEnum.first.mapValueDescriptions.size(), 3);
    EXPECT_EQ(prSigEnum.first.mapValueDescriptions[10], "ten");
    EXPECT_EQ(prSigEnum.first.mapValueDescriptions[20], "twenty");
    EXPECT_EQ(prSigEnum.first.mapValueDescriptions[30], "thirty");

    // Empty signal value description
    std::string ssSigValDescEmpty = "VAL_ 1 sig_enum_empty; ";
    dbc::CDbcSource srcSigValDescEmpty(ssSigValDescEmpty);
    EXPECT_NO_THROW(parser.Parse(srcSigValDescEmpty));
    prSigEnum = parser.GetSignalDef("msg_enum", "sig_enum_empty");
    EXPECT_TRUE(prSigEnum.second);
    EXPECT_TRUE(prSigEnum.first.mapValueDescriptions.empty());

    // Invalid value descriptions
    std::string ssSigValDescDuplicate1 = "VAL_ 1 sig_enum;";
    dbc::CDbcSource srcSigValDescDuplicate1(ssSigValDescDuplicate1);
    EXPECT_THROW(parser.Parse(srcSigValDescDuplicate1), dbc::SDbcParserException);
    std::string ssSigValDescDuplicate2 = "VAL_ 1 sig_enum_empty 10 \"ten\" 20 \"twenty\" 10 \"ten\";";
    dbc::CDbcSource srcSigValDescDuplicate2(ssSigValDescDuplicate2);
    EXPECT_THROW(parser.Parse(srcSigValDescDuplicate2), dbc::SDbcParserException);
    std::string ssSigValDescMissingSemiColon = "VAL_ 1 sig_enum_empty 10 \"ten\"";
    dbc::CDbcSource srcSigValDescMissingSemiColon(ssSigValDescMissingSemiColon);
    EXPECT_THROW(parser.Parse(srcSigValDescMissingSemiColon), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, EnvironmentVariableDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Environment variable definition
    std::string ssEnvVarDef = R"code(
    EV_ var1 : 0 [10|20] "steps" 15 100 DUMMY_NODE_VECTOR3 nodeTx, nodeRx;
    EV_ var2 : 1 [0.1|0.2] "tenth steps" 0.5 101 DUMMY_NODE_VECTOR2 nodeTx;
    EV_ var3 : 2 [0.0|0.0] "string val" 0.0 102 DUMMY_NODE_VECTOR8001 nodeRx;
)code";
    dbc::CDbcSource srcEnvVarDef(ssEnvVarDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDef));
    auto vecEnvVars = parser.GetEnvVarNames();
    ASSERT_EQ(vecEnvVars.size(), 3u);
    EXPECT_EQ(vecEnvVars[0], "var1");
    EXPECT_EQ(vecEnvVars[1], "var2");
    EXPECT_EQ(vecEnvVars[2], "var3");
    auto prEnvVar = parser.GetEnvVarDef("var1");
    EXPECT_TRUE(prEnvVar.second);
    EXPECT_EQ(prEnvVar.first.ssName, "var1");
    EXPECT_EQ(prEnvVar.first.eType, dbc::SEnvVarDef::EType::integer);
    EXPECT_EQ(prEnvVar.first.dMinimum, 10.0);
    EXPECT_EQ(prEnvVar.first.dMaximum, 20.0);
    EXPECT_EQ(prEnvVar.first.ssUnit, "steps");
    EXPECT_EQ(prEnvVar.first.dInitVal, 15.0);
    EXPECT_EQ(prEnvVar.first.uiId, 100u);
    EXPECT_EQ(prEnvVar.first.eAccess, dbc::SEnvVarDef::EAccessType::readwrite);
    ASSERT_EQ(prEnvVar.first.vecNodes.size(), 2);
    EXPECT_EQ(prEnvVar.first.vecNodes[0], "nodeTx");
    EXPECT_EQ(prEnvVar.first.vecNodes[1], "nodeRx");
    prEnvVar = parser.GetEnvVarDef("var2");
    EXPECT_TRUE(prEnvVar.second);
    EXPECT_EQ(prEnvVar.first.ssName, "var2");
    EXPECT_EQ(prEnvVar.first.eType, dbc::SEnvVarDef::EType::floating_point);
    EXPECT_EQ(prEnvVar.first.dMinimum, 0.1);
    EXPECT_EQ(prEnvVar.first.dMaximum, 0.2);
    EXPECT_EQ(prEnvVar.first.ssUnit, "tenth steps");
    EXPECT_EQ(prEnvVar.first.dInitVal, 0.5);
    EXPECT_EQ(prEnvVar.first.uiId, 101u);
    EXPECT_EQ(prEnvVar.first.eAccess, dbc::SEnvVarDef::EAccessType::write);
    ASSERT_EQ(prEnvVar.first.vecNodes.size(), 1);
    EXPECT_EQ(prEnvVar.first.vecNodes[0], "nodeTx");
    prEnvVar = parser.GetEnvVarDef("var3");
    EXPECT_TRUE(prEnvVar.second);
    EXPECT_EQ(prEnvVar.first.ssName, "var3");
    EXPECT_EQ(prEnvVar.first.eType, dbc::SEnvVarDef::EType::string);
    EXPECT_EQ(prEnvVar.first.dMinimum, 0.0);
    EXPECT_EQ(prEnvVar.first.dMaximum, 0.0);
    EXPECT_EQ(prEnvVar.first.ssUnit, "string val");
    EXPECT_EQ(prEnvVar.first.dInitVal, 0.0);
    EXPECT_EQ(prEnvVar.first.uiId, 102u);
    EXPECT_EQ(prEnvVar.first.eAccess, dbc::SEnvVarDef::EAccessType::read);
    ASSERT_EQ(prEnvVar.first.vecNodes.size(), 1);
    EXPECT_EQ(prEnvVar.first.vecNodes[0], "nodeRx");
}

TEST_F(CDbcParserTest, EnvVarData)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Environment variable definition
    std::string ssEnvVarDef = R"code(
    EV_ var1 : 0 [10|20] "steps" 15 100 DUMMY_NODE_VECTOR3 nodeTx, nodeRx;
    EV_ var2 : 1 [0.1|0.2] "tenth steps" 0.5 101 DUMMY_NODE_VECTOR2 nodeTx;
    EV_ var3 : 2 [0.0|0.0] "string val" 0.0 102 DUMMY_NODE_VECTOR8001 nodeRx;
)code";
    dbc::CDbcSource srcEnvVarDef(ssEnvVarDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDef));

    // Environment variable data definition
    std::string ssEnvVarDataDef = R"code(
    ENVVAR_DATA_ var1 : 10;
)code";
    dbc::CDbcSource srcEnvVarDataDef(ssEnvVarDataDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDataDef));
    auto prEnvVar = parser.GetEnvVarDef("var1");
    EXPECT_TRUE(prEnvVar.second);
    EXPECT_EQ(prEnvVar.first.ssName, "var1");
    EXPECT_EQ(prEnvVar.first.eType, dbc::SEnvVarDef::EType::data);
    EXPECT_EQ(prEnvVar.first.uiDataSize, 10u);
    EXPECT_EQ(prEnvVar.first.dMinimum, 10.0);
    EXPECT_EQ(prEnvVar.first.dMaximum, 20.0);
    EXPECT_EQ(prEnvVar.first.ssUnit, "steps");
    EXPECT_EQ(prEnvVar.first.dInitVal, 15.0);
    EXPECT_EQ(prEnvVar.first.uiId, 100u);
    EXPECT_EQ(prEnvVar.first.eAccess, dbc::SEnvVarDef::EAccessType::readwrite);
    ASSERT_EQ(prEnvVar.first.vecNodes.size(), 2);
    EXPECT_EQ(prEnvVar.first.vecNodes[0], "nodeTx");
    EXPECT_EQ(prEnvVar.first.vecNodes[1], "nodeRx");
}

TEST_F(CDbcParserTest, EnvVarValueDescriptions)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Environment variable definition
    std::string ssEnvVarDef = R"code(
    EV_ var1 : 0 [10|20] "steps" 15 100 DUMMY_NODE_VECTOR3 nodeTx, nodeRx;
    EV_ var2 : 1 [0.1|0.2] "tenth steps" 0.5 101 DUMMY_NODE_VECTOR2 nodeTx;
    EV_ var3 : 2 [0.0|0.0] "string val" 0.0 102 DUMMY_NODE_VECTOR8001 nodeRx;
)code";
    dbc::CDbcSource srcEnvVarDef(ssEnvVarDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDef));

    // Signal value descriptions
    std::string ssSigValDesc = "VAL_ var1 10 \"ten\" 20 \"twenty\" 30 \"thirty\";";
    dbc::CDbcSource srcSigValDesc(ssSigValDesc);
    EXPECT_NO_THROW(parser.Parse(srcSigValDesc));
    auto prEnvVar = parser.GetEnvVarDef("var1");
    EXPECT_TRUE(prEnvVar.second);
    ASSERT_EQ(prEnvVar.first.mapValueDescriptions.size(), 3);
    EXPECT_EQ(prEnvVar.first.mapValueDescriptions[10], "ten");
    EXPECT_EQ(prEnvVar.first.mapValueDescriptions[20], "twenty");
    EXPECT_EQ(prEnvVar.first.mapValueDescriptions[30], "thirty");

    // Empty signal value description
    std::string ssSigValDescEmpty = "VAL_ var2; ";
    dbc::CDbcSource srcSigValDescEmpty(ssSigValDescEmpty);
    EXPECT_NO_THROW(parser.Parse(srcSigValDescEmpty));
    prEnvVar = parser.GetEnvVarDef("var2");
    EXPECT_TRUE(prEnvVar.second);
    EXPECT_TRUE(prEnvVar.first.mapValueDescriptions.empty());

    // Invalid value descriptions
    std::string ssEnvVarValDescDuplicate2 = "VAL_ var3 10 \"ten\" 20 \"twenty\" 10 \"ten\";";
    dbc::CDbcSource srcEnvVarValDescDuplicate2(ssEnvVarValDescDuplicate2);
    EXPECT_THROW(parser.Parse(srcEnvVarValDescDuplicate2), dbc::SDbcParserException);
    std::string ssEnvVarValDescMissingSemiColon = "VAL_ var3 10 \"ten\"";
    dbc::CDbcSource srcEnvVarValDescMissingSemiColon(ssEnvVarValDescMissingSemiColon);
    EXPECT_THROW(parser.Parse(srcEnvVarValDescMissingSemiColon), dbc::SDbcParserException);
}

TEST_F(CDbcParserTest, ExtSignalTypeDef)
{
    dbc::CDbcParser parser;

    // Value table
    std::string ssValueTable = R"code(
        VAL_TABLE_ table1 10 "ten" 20 "twenty";
)code";
    dbc::CDbcSource srcValueTable(ssValueTable);
    EXPECT_NO_THROW(parser.Parse(srcValueTable));

    // Value table type definition
    std::string ssValueTableType = R"code(
        SGTYPE_ sgtype1 : 32 @ 1+ (1.0, 0) [0|100] "signal unit" 50, table1;
        SGTYPE_ sgtype2 : 16 @ 0- (-10.5, 10) [-100|200] "signal unit2" 13, Vector__XXX;
)code";
    dbc::CDbcSource srcValueTableType(ssValueTableType);
    EXPECT_NO_THROW(parser.Parse(srcValueTableType));
    auto vecSignalTypes = parser.GetSignalTypeDefNames();
    ASSERT_EQ(vecSignalTypes.size(), 2);
    EXPECT_EQ(vecSignalTypes[0], "sgtype1");
    EXPECT_EQ(vecSignalTypes[1], "sgtype2");
    auto prSignalType = parser.GetSignalTypeDef("sgtype1");
    EXPECT_TRUE(prSignalType.second);
    EXPECT_EQ(prSignalType.first.ssName, "sgtype1");
    EXPECT_EQ(prSignalType.first.dDefaultValue, 50.0);
    EXPECT_EQ(prSignalType.first.ssValueTable, "table1");
    EXPECT_EQ(prSignalType.first.uiSize, 32u);
    EXPECT_EQ(prSignalType.first.eByteOrder, dbc::SSignalTypeBase::EByteOrder::little_endian);
    EXPECT_EQ(prSignalType.first.eValType, dbc::SSignalTypeBase::EValueType::unsigned_integer);
    EXPECT_EQ(prSignalType.first.dFactor, 1.0);
    EXPECT_EQ(prSignalType.first.dOffset, 0.0);
    EXPECT_EQ(prSignalType.first.dMinimum, 0.0);
    EXPECT_EQ(prSignalType.first.dMaximum, 100.0);
    EXPECT_EQ(prSignalType.first.ssUnit, "signal unit");
    prSignalType = parser.GetSignalTypeDef("sgtype2");
    EXPECT_TRUE(prSignalType.second);
    EXPECT_EQ(prSignalType.first.ssName, "sgtype2");
    EXPECT_EQ(prSignalType.first.dDefaultValue, 13.0);
    EXPECT_EQ(prSignalType.first.ssValueTable, "Vector__XXX");
    EXPECT_EQ(prSignalType.first.uiSize, 16u);
    EXPECT_EQ(prSignalType.first.eByteOrder, dbc::SSignalTypeBase::EByteOrder::big_endian);
    EXPECT_EQ(prSignalType.first.eValType, dbc::SSignalTypeBase::EValueType::signed_integer);
    EXPECT_EQ(prSignalType.first.dFactor, -10.5);
    EXPECT_EQ(prSignalType.first.dOffset, 10.0);
    EXPECT_EQ(prSignalType.first.dMinimum, -100.0);
    EXPECT_EQ(prSignalType.first.dMaximum, 200.0);
    EXPECT_EQ(prSignalType.first.ssUnit, "signal unit2");
}

TEST_F(CDbcParserTest, SignalTypeRef)
{
    dbc::CDbcParser parser;

    // Value table
    std::string ssValueTable = R"code(
        VAL_TABLE_ table1 10 "ten" 20 "twenty";
        VAL_TABLE_ table2 20 "twenty" 30 "thirty";
)code";
    dbc::CDbcSource srcValueTable(ssValueTable);
    EXPECT_NO_THROW(parser.Parse(srcValueTable));

    // Signal type definition
    std::string ssSignalType = R"code(
        SGTYPE_ sgtype1 : 32 @ 1+ (1.0, 0) [0|100] "signal unit" 50, table1;
)code";
    dbc::CDbcSource srcSignalType(ssSignalType);
    EXPECT_NO_THROW(parser.Parse(srcSignalType));

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Signal type reference
    std::string ssSignalTypeRef = "SGTYPE_ 1 sig1 : sgtype1;";
    dbc::CDbcSource srcSignalTypeRef(ssSignalTypeRef);
    EXPECT_NO_THROW(parser.Parse(srcSignalTypeRef));
    auto prSignal = parser.GetSignalDef(1, "sig1");
    ASSERT_TRUE(prSignal.second);
    EXPECT_EQ(prSignal.first.ssSignalTypeDef, "sgtype1");
}

TEST_F(CDbcParserTest, SignalGroupDef)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig3 : 0|32@1+ (1,0) [0|0] "" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Signal group definition
    std::string ssSignalGroup = R"code(
        SIG_GROUP_ 1 group1 10 : sig1 sig2;
        SIG_GROUP_ 1 group2 20 : sig3;
    )code";
    dbc::CDbcSource srcSignalGroup(ssSignalGroup);
    EXPECT_NO_THROW(parser.Parse(srcSignalGroup));
    auto vecGroupNames = parser.GetSignalGroupDefNames(1);
    ASSERT_EQ(vecGroupNames.size(), 2u);
    EXPECT_EQ(vecGroupNames[0], "group1");
    EXPECT_EQ(vecGroupNames[1], "group2");
    auto prGroupDef = parser.GetSignalGroupDef(1, "group1");
    EXPECT_TRUE(prGroupDef.second);
    EXPECT_EQ(prGroupDef.first.ssName, "group1");
    EXPECT_EQ(prGroupDef.first.uiRepetitions, 10u);
    ASSERT_EQ(prGroupDef.first.vecSignals.size(), 2u);
    EXPECT_EQ(prGroupDef.first.vecSignals[0], "sig1");
    EXPECT_EQ(prGroupDef.first.vecSignals[1], "sig2");
    prGroupDef = parser.GetSignalGroupDef(1, "group2");
    EXPECT_TRUE(prGroupDef.second);
    EXPECT_EQ(prGroupDef.first.ssName, "group2");
    EXPECT_EQ(prGroupDef.first.uiRepetitions, 20u);
    ASSERT_EQ(prGroupDef.first.vecSignals.size(), 1u);
    EXPECT_EQ(prGroupDef.first.vecSignals[0], "sig3");
}

TEST_F(CDbcParserTest, GlobalComments)
{
    dbc::CDbcParser parser;

    // Global comments
    std::string ssComments = R"code(
        CM_ "first comment";
        CM_ "second comment";
        CM_ "third comment";
        CM_ "fourth comment";
    )code";
    dbc::CDbcSource srcComments(ssComments);
    EXPECT_NO_THROW(parser.Parse(srcComments));
    auto vecComments = parser.GetComments();
    ASSERT_EQ(vecComments.size(), 4u);
    EXPECT_EQ(vecComments[0], "first comment");
    EXPECT_EQ(vecComments[1], "second comment");
    EXPECT_EQ(vecComments[2], "third comment");
    EXPECT_EQ(vecComments[3], "fourth comment");
}

TEST_F(CDbcParserTest, NodeComments)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Node comments
    std::string ssComments = R"code(
        CM_ BU_ nodeTx "first comment";
        CM_ BU_ nodeTx "second comment \"with quotes\"";
        CM_ BU_ nodeRx "third comment 'with single quotes'";
        CM_ BU_ nodeRx "fourth comment";
    )code";
    dbc::CDbcSource srcComments(ssComments);
    EXPECT_NO_THROW(parser.Parse(srcComments));
    auto prNode = parser.GetNodeDef("nodeTx");
    EXPECT_TRUE(prNode.second);
    ASSERT_EQ(prNode.first.vecComments.size(), 2u);
    EXPECT_EQ(prNode.first.vecComments[0], "first comment");
    EXPECT_EQ(prNode.first.vecComments[1], "second comment \"with quotes\"");
    prNode = parser.GetNodeDef("nodeRx");
    EXPECT_TRUE(prNode.second);
    ASSERT_EQ(prNode.first.vecComments.size(), 2u);
    EXPECT_EQ(prNode.first.vecComments[0], "third comment 'with single quotes'");
    EXPECT_EQ(prNode.first.vecComments[1], "fourth comment");
}

TEST_F(CDbcParserTest, MessageComments)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig3 : 0|32@1+ (1,0) [0|0] "" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Message comments
    std::string ssComments = R"code(
        CM_ BO_ 1 "first comment";
        CM_ BO_ 1 "second comment";
        CM_ BO_ 1 "third comment";
        CM_ BO_ 1 "fourth comment";
    )code";
    dbc::CDbcSource srcComments(ssComments);
    EXPECT_NO_THROW(parser.Parse(srcComments));
    auto prMsgDef = parser.GetMsgDef(1);
    EXPECT_TRUE(prMsgDef.second);
    ASSERT_EQ(prMsgDef.first.vecComments.size(), 4u);
    EXPECT_EQ(prMsgDef.first.vecComments[0], "first comment");
    EXPECT_EQ(prMsgDef.first.vecComments[1], "second comment");
    EXPECT_EQ(prMsgDef.first.vecComments[2], "third comment");
    EXPECT_EQ(prMsgDef.first.vecComments[3], "fourth comment");
}

TEST_F(CDbcParserTest, SignalComments)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Signal definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig3 : 0|32@1+ (1,0) [0|0] "" nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Node comments
    std::string ssComments = R"code(
        CM_ SG_ 1 sig1 "first comment";
        CM_ SG_ 1 sig1 "second comment";
        CM_ SG_ 1 sig2 "third comment";
        CM_ SG_ 1 sig3 "fourth comment";
    )code";
    dbc::CDbcSource srcComments(ssComments);
    EXPECT_NO_THROW(parser.Parse(srcComments));
    auto prSignal = parser.GetSignalDef(1, "sig1");
    EXPECT_TRUE(prSignal.second);
    ASSERT_EQ(prSignal.first.vecComments.size(), 2u);
    EXPECT_EQ(prSignal.first.vecComments[0], "first comment");
    EXPECT_EQ(prSignal.first.vecComments[1], "second comment");
    prSignal = parser.GetSignalDef(1, "sig2");
    EXPECT_TRUE(prSignal.second);
    ASSERT_EQ(prSignal.first.vecComments.size(), 1u);
    EXPECT_EQ(prSignal.first.vecComments[0], "third comment");
    prSignal = parser.GetSignalDef(1, "sig3");
    EXPECT_TRUE(prSignal.second);
    ASSERT_EQ(prSignal.first.vecComments.size(), 1u);
    EXPECT_EQ(prSignal.first.vecComments[0], "fourth comment");
}

TEST_F(CDbcParserTest, EnvVarComments)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Environment variable definition
    std::string ssEnvVarDef = R"code(
    EV_ var1 : 0 [10|20] "steps" 15 100 DUMMY_NODE_VECTOR3 nodeTx, nodeRx;
    EV_ var2 : 1 [0.1|0.2] "tenth steps" 0.5 101 DUMMY_NODE_VECTOR2 nodeTx;
    EV_ var3 : 2 [0.0|0.0] "string val" 0.0 102 DUMMY_NODE_VECTOR8001 nodeRx;
)code";
    dbc::CDbcSource srcEnvVarDef(ssEnvVarDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDef));

    // Env var comments
    std::string ssComments = R"code(
        CM_ EV_ var1 "first comment";
        CM_ EV_ var1 "second comment";
        CM_ EV_ var2 "third comment";
        CM_ EV_ var3 "fourth comment";
    )code";
    dbc::CDbcSource srcComments(ssComments);
    EXPECT_NO_THROW(parser.Parse(srcComments));
    auto prEnvVar = parser.GetEnvVarDef("var1");
    EXPECT_TRUE(prEnvVar.second);
    ASSERT_EQ(prEnvVar.first.vecComments.size(), 2u);
    EXPECT_EQ(prEnvVar.first.vecComments[0], "first comment");
    EXPECT_EQ(prEnvVar.first.vecComments[1], "second comment");
    prEnvVar = parser.GetEnvVarDef("var2");
    EXPECT_TRUE(prEnvVar.second);
    ASSERT_EQ(prEnvVar.first.vecComments.size(), 1u);
    EXPECT_EQ(prEnvVar.first.vecComments[0], "third comment");
    prEnvVar = parser.GetEnvVarDef("var3");
    EXPECT_TRUE(prEnvVar.second);
    ASSERT_EQ(prEnvVar.first.vecComments.size(), 1u);
    EXPECT_EQ(prEnvVar.first.vecComments[0], "fourth comment");
}

TEST_F(CDbcParserTest, AttributeDef)
{
    dbc::CDbcParser parser(true);

    // Global attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ "attr1" INT 10 20;
        BA_DEF_ "attr2" HEX 10 20;
        BA_DEF_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ "attr4" STRING;
        BA_DEF_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));
    auto vecAttr = parser.GetAttributeDefNames();
    ASSERT_EQ(vecAttr.size(), 6u);
    EXPECT_EQ(vecAttr[0], "attr1");
    EXPECT_EQ(vecAttr[1], "attr2");
    EXPECT_EQ(vecAttr[2], "attr3");
    EXPECT_EQ(vecAttr[3], "attr4");
    EXPECT_EQ(vecAttr[4], "attr5");
    EXPECT_EQ(vecAttr[5], "attr6");
    auto prAttr = parser.GetAttributeDef("attr1");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr1");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::integer);
    EXPECT_EQ(prAttr.first.sIntValues.iMinimum, 10);
    EXPECT_EQ(prAttr.first.sIntValues.iMaximum, 20);
    prAttr = parser.GetAttributeDef("attr2");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr2");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::hex_integer);
    EXPECT_EQ(prAttr.first.sHexValues.uiMinimum, 10u);
    EXPECT_EQ(prAttr.first.sHexValues.uiMaximum, 20u);
    prAttr = parser.GetAttributeDef("attr3");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr3");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::floating_point);
    EXPECT_EQ(prAttr.first.sFltValues.dMinimum, 1.0);
    EXPECT_EQ(prAttr.first.sFltValues.dMaximum, 10.9);
    prAttr = parser.GetAttributeDef("attr4");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr4");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::string);
    prAttr = parser.GetAttributeDef("attr5");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr5");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::enumerator);
    ASSERT_EQ(prAttr.first.sEnumValues.vecEnumValues.size(), 3u);
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[0], "abc");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[1], "def");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[2], "ghi");
    prAttr = parser.GetAttributeDef("attr6");
    EXPECT_TRUE(prAttr.second);
    EXPECT_EQ(prAttr.first.ssName, "attr6");
    EXPECT_EQ(prAttr.first.eType, dbc::SAttributeDef::EType::enumerator);
    ASSERT_EQ(prAttr.first.sEnumValues.vecEnumValues.size(), 7u);
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[0], "abc");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[1], "n/a");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[2], "not-used");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[3], "jkl");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[4], "n/a");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[5], "not-used");
    EXPECT_EQ(prAttr.first.sEnumValues.vecEnumValues[6], "stu");
}

TEST_F(CDbcParserTest, AttributeDefaultValDef)
{
    dbc::CDbcParser parser;

    // Global attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ "attr1" INT 10 20;
        BA_DEF_ "attr2" HEX 10 20;
        BA_DEF_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ "attr4" STRING;
        BA_DEF_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Attribute def default value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrDefDefaultVal = R"code(
        BA_DEF_DEF_ "attr1" 15;
        BA_DEF_DEF_ "attr2" 15;
        BA_DEF_DEF_ "attr3" 5.5;
        BA_DEF_DEF_ "attr4" "hello";
        BA_DEF_DEF_ "attr5" "def";
        BA_DEF_DEF_ "attr6" 6;
    )code";
    dbc::CDbcSource srcAttrDefDefaultVal(ssAttrDefDefaultVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrDefDefaultVal));

    auto prAttr = parser.GetAttributeDef("attr1");
    EXPECT_EQ(prAttr.first.sIntValues.iDefault, 15);
    prAttr = parser.GetAttributeDef("attr2");
    EXPECT_EQ(prAttr.first.sHexValues.uiDefault, 15u);
    prAttr = parser.GetAttributeDef("attr3");
    EXPECT_EQ(prAttr.first.sFltValues.dDefault, 5.5);
    prAttr = parser.GetAttributeDef("attr4");
    EXPECT_EQ(prAttr.first.sStringValues.ssDefault, "hello");
    prAttr = parser.GetAttributeDef("attr5");
    EXPECT_EQ(prAttr.first.sEnumValues.ssDefault, "def");
    prAttr = parser.GetAttributeDef("attr6");
    EXPECT_EQ(prAttr.first.sEnumValues.ssDefault, "stu");
}

TEST_F(CDbcParserTest, GlobalAttributeVal)
{
    dbc::CDbcParser parser;

    // Global attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ "attr1" INT 10 20;
        BA_DEF_ "attr2" HEX 10 20;
        BA_DEF_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ "attr4" STRING;
        BA_DEF_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Attribute value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrVal = R"code(
        BA_ "attr1" 11;
        BA_ "attr2" 12;
        BA_ "attr2" 13;
        BA_ "attr3" 3.5;
        BA_ "attr4" "hi";
        BA_ "attr5" "ghi";
        BA_ "attr6" 6;
    )code";
    dbc::CDbcSource srcAttrVal(ssAttrVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrVal));
    auto vecAttr = parser.GetAttributes();
    ASSERT_EQ(vecAttr.size(), 7u);
    EXPECT_EQ(vecAttr[0].iValue, 11);
    EXPECT_EQ(vecAttr[1].uiValue, 12u);
    EXPECT_EQ(vecAttr[2].uiValue, 13u);
    EXPECT_EQ(vecAttr[3].dValue, 3.5);
    EXPECT_EQ(vecAttr[4].ssValue, "hi");
    EXPECT_EQ(vecAttr[5].ssValue, "ghi");
    EXPECT_EQ(vecAttr[6].ssValue, "stu");
}

TEST_F(CDbcParserTest, NodeAttributeVal)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Node attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ BU_ "attr1" INT 10 20;
        BA_DEF_ BU_ "attr2" HEX 10 20;
        BA_DEF_ BU_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ BU_ "attr4" STRING;
        BA_DEF_ BU_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ BU_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Attribute value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrVal = R"code(
        BA_ "attr1" BU_ nodeTx 11;
        BA_ "attr2" BU_ nodeTx 12;
        BA_ "attr2" BU_ nodeTx 13;
        BA_ "attr3" BU_ nodeRx 3.5;
        BA_ "attr4" BU_ nodeRx "hi";
        BA_ "attr5" BU_ nodeRx "ghi";
        BA_ "attr6" BU_ nodeRx 6;
    )code";
    dbc::CDbcSource srcAttrVal(ssAttrVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrVal));
    auto prNodeDef = parser.GetNodeDef("nodeTx");
    ASSERT_EQ(prNodeDef.first.vecAttributes.size(), 3u);
    EXPECT_EQ(prNodeDef.first.vecAttributes[0].iValue, 11);
    EXPECT_EQ(prNodeDef.first.vecAttributes[1].uiValue, 12u);
    EXPECT_EQ(prNodeDef.first.vecAttributes[2].uiValue, 13u);
    prNodeDef = parser.GetNodeDef("nodeRx");
    ASSERT_EQ(prNodeDef.first.vecAttributes.size(), 4u);
    EXPECT_EQ(prNodeDef.first.vecAttributes[0].dValue, 3.5);
    EXPECT_EQ(prNodeDef.first.vecAttributes[1].ssValue, "hi");
    EXPECT_EQ(prNodeDef.first.vecAttributes[2].ssValue, "ghi");
    EXPECT_EQ(prNodeDef.first.vecAttributes[3].ssValue, "stu");
}

TEST_F(CDbcParserTest, MessageAttributeVal)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Message attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ BO_ "attr1" INT 10 20;
        BA_DEF_ BO_ "attr2" HEX 10 20;
        BA_DEF_ BO_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ BO_ "attr4" STRING;
        BA_DEF_ BO_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ BO_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig3 : 0|32@1+ (1,0) [0|0] "" nodeRx
        BO_ 2 msg2: 8 nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Attribute value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrVal = R"code(
        BA_ "attr1" BO_ 1 11;
        BA_ "attr2" BO_ 1 12;
        BA_ "attr2" BO_ 1 13;
        BA_ "attr3" BO_ 2 3.5;
        BA_ "attr4" BO_ 2 "hi";
        BA_ "attr5" BO_ 2 "ghi";
        BA_ "attr6" BO_ 2 6;
    )code";
    dbc::CDbcSource srcAttrVal(ssAttrVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrVal));
    auto prMsgDef = parser.GetMsgDef(1);
    ASSERT_EQ(prMsgDef.first.vecAttributes.size(), 3u);
    EXPECT_EQ(prMsgDef.first.vecAttributes[0].iValue, 11);
    EXPECT_EQ(prMsgDef.first.vecAttributes[1].uiValue, 12u);
    EXPECT_EQ(prMsgDef.first.vecAttributes[2].uiValue, 13u);
    prMsgDef = parser.GetMsgDef(2);
    ASSERT_EQ(prMsgDef.first.vecAttributes.size(), 4u);
    EXPECT_EQ(prMsgDef.first.vecAttributes[0].dValue, 3.5);
    EXPECT_EQ(prMsgDef.first.vecAttributes[1].ssValue, "hi");
    EXPECT_EQ(prMsgDef.first.vecAttributes[2].ssValue, "ghi");
    EXPECT_EQ(prMsgDef.first.vecAttributes[3].ssValue, "stu");
}

TEST_F(CDbcParserTest, SignalAttributeVal)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Signal attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ SG_ "attr1" INT 10 20;
        BA_DEF_ SG_ "attr2" HEX 10 20;
        BA_DEF_ SG_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ SG_ "attr4" STRING;
        BA_DEF_ SG_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ SG_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 1 msg: 8 nodeTx
            SG_ sig1 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig2 : 0|32@1+ (1,0) [0|0] "" nodeRx
            SG_ sig3 : 0|32@1+ (1,0) [0|0] "" nodeRx
        BO_ 2 msg2: 8 nodeRx
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    // Attribute value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrVal = R"code(
        BA_ "attr1" SG_ 1 sig1 11;
        BA_ "attr2" SG_ 1 sig1 12;
        BA_ "attr2" SG_ 1 sig2 13;
        BA_ "attr3" SG_ 1 sig2 3.5;
        BA_ "attr4" SG_ 1 sig3 "hi";
        BA_ "attr5" SG_ 1 sig3 "ghi";
        BA_ "attr6" SG_ 1 sig3 6;
    )code";
    dbc::CDbcSource srcAttrVal(ssAttrVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrVal));
    auto prSigDef = parser.GetSignalDef(1, "sig1");
    ASSERT_EQ(prSigDef.first.vecAttributes.size(), 2u);
    EXPECT_EQ(prSigDef.first.vecAttributes[0].iValue, 11);
    EXPECT_EQ(prSigDef.first.vecAttributes[1].uiValue, 12u);
    prSigDef = parser.GetSignalDef(1, "sig2");
    ASSERT_EQ(prSigDef.first.vecAttributes.size(), 2u);
    EXPECT_EQ(prSigDef.first.vecAttributes[0].uiValue, 13u);
    EXPECT_EQ(prSigDef.first.vecAttributes[1].dValue, 3.5);
    prSigDef = parser.GetSignalDef(1, "sig3");
    ASSERT_EQ(prSigDef.first.vecAttributes.size(), 3u);
    EXPECT_EQ(prSigDef.first.vecAttributes[0].ssValue, "hi");
    EXPECT_EQ(prSigDef.first.vecAttributes[1].ssValue, "ghi");
    EXPECT_EQ(prSigDef.first.vecAttributes[2].ssValue, "stu");
}

TEST_F(CDbcParserTest, EnvVarAttributeVal)
{
    dbc::CDbcParser parser;

    // Node definition
    std::string ssValidNodeDef = "BU_: nodeTx nodeRx";
    dbc::CDbcSource srcValidNodeDef(ssValidNodeDef);
    EXPECT_NO_THROW(parser.Parse(srcValidNodeDef));
    EXPECT_TRUE(parser.HasNodeDef("nodeTx"));
    EXPECT_TRUE(parser.HasNodeDef("nodeRx"));

    // Environment variable attribute definition
    // The use of "n/a" and "not-used" is not part of the official standard.
    std::string ssAttribute = R"code(
        BA_DEF_ EV_ "attr1" INT 10 20;
        BA_DEF_ EV_ "attr2" HEX 10 20;
        BA_DEF_ EV_ "attr3" FLOAT 1.0 10.9;
        BA_DEF_ EV_ "attr4" STRING;
        BA_DEF_ EV_ "attr5" ENUM "abc", "def", "ghi";
        BA_DEF_ EV_ "attr6" ENUM "abc", "n/a", "not-used", "jkl", "n/a", "not-used", "stu";
    )code";
    dbc::CDbcSource srcAttribute(ssAttribute);
    EXPECT_NO_THROW(parser.Parse(srcAttribute));

    // Environment variable definition
    std::string ssEnvVarDef = R"code(
    EV_ var1 : 0 [10|20] "steps" 15 100 DUMMY_NODE_VECTOR3 nodeTx, nodeRx;
    EV_ var2 : 1 [0.1|0.2] "tenth steps" 0.5 101 DUMMY_NODE_VECTOR2 nodeTx;
    EV_ var3 : 2 [0.0|0.0] "string val" 0.0 102 DUMMY_NODE_VECTOR8001 nodeRx;
)code";
    dbc::CDbcSource srcEnvVarDef(ssEnvVarDef);
    EXPECT_NO_THROW(parser.Parse(srcEnvVarDef));

    // Attribute value
    // The use of an index instead of a value for the enum value is not part of the official standard.
    std::string ssAttrVal = R"code(
        BA_ "attr1" EV_ var1 11;
        BA_ "attr2" EV_ var1 12;
        BA_ "attr2" EV_ var2 13;
        BA_ "attr3" EV_ var2 3.5;
        BA_ "attr4" EV_ var3 "hi";
        BA_ "attr5" EV_ var3 "ghi";
        BA_ "attr6" EV_ var3 6;
    )code";
    dbc::CDbcSource srcAttrVal(ssAttrVal);
    EXPECT_NO_THROW(parser.Parse(srcAttrVal));
    auto prEnvVarDef = parser.GetEnvVarDef("var1");
    ASSERT_EQ(prEnvVarDef.first.vecAttributes.size(), 2u);
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[0].iValue, 11);
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[1].uiValue, 12u);
    prEnvVarDef = parser.GetEnvVarDef("var2");
    ASSERT_EQ(prEnvVarDef.first.vecAttributes.size(), 2u);
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[0].uiValue, 13u);
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[1].dValue, 3.5);
    prEnvVarDef = parser.GetEnvVarDef("var3");
    ASSERT_EQ(prEnvVarDef.first.vecAttributes.size(), 3u);
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[0].ssValue, "hi");
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[1].ssValue, "ghi");
    EXPECT_EQ(prEnvVarDef.first.vecAttributes[2].ssValue, "stu");
}

TEST_F(CDbcParserTest, SignalExtendedMultiplexing)
{
    dbc::CDbcParser parser;

    // Message definition
    std::string ssMsgDef = R"code(
        BO_ 100 MuxMsg: 1 Vector__XXX
            SG_ Mux_4 m2 : 6|2@1+ (1,0) [0|0] "" Vector__XXX
            SG_ Mux_3 m3M : 4|2@1+ (1,0) [0|0] "" Vector__XXX
            SG_ Mux_2 m3M : 2|2@1+ (1,0) [0|0] "" Vector__XXX
            SG_ Mux_1 M : 0|2@1+ (1,0) [0|0] "" Vector__XXX

        SG_MUL_VAL_ 100 Mux_2 Mux_1 3-3, 5-10;
        SG_MUL_VAL_ 100 Mux_3 Mux_2 3-3;
        SG_MUL_VAL_ 100 Mux_4 Mux_3 2-2;
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));

    auto prSigDef = parser.GetSignalDef(100, "Mux_4");
    EXPECT_TRUE(prSigDef.second);
    EXPECT_EQ(prSigDef.first.uiMultiplexBitmask, static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_val));
    ASSERT_EQ(prSigDef.first.vecExtMultiplex.size(), 1u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].rsMultiplexor.ssName, "Mux_3");
    ASSERT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges.size(), 1u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].first, 2u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].second, 2u);
    prSigDef = parser.GetSignalDef(100, "Mux_3");
    EXPECT_TRUE(prSigDef.second);
    EXPECT_EQ(prSigDef.first.uiMultiplexBitmask, static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_val) |
        static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_switch));
    ASSERT_EQ(prSigDef.first.vecExtMultiplex.size(), 1u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].rsMultiplexor.ssName, "Mux_2");
    ASSERT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges.size(), 1u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].first, 3u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].second, 3u);
    prSigDef = parser.GetSignalDef(100, "Mux_2");
    EXPECT_TRUE(prSigDef.second);
    EXPECT_EQ(prSigDef.first.uiMultiplexBitmask, static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_val) |
        static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_switch));
    ASSERT_EQ(prSigDef.first.vecExtMultiplex.size(), 1u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].rsMultiplexor.ssName, "Mux_1");
    ASSERT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges.size(), 2u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].first, 3u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[0].second, 3u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[1].first, 5u);
    EXPECT_EQ(prSigDef.first.vecExtMultiplex[0].vecRanges[1].second, 10u);
    prSigDef = parser.GetSignalDef(100, "Mux_1");
    EXPECT_TRUE(prSigDef.second);
    EXPECT_EQ(prSigDef.first.uiMultiplexBitmask, static_cast<uint32_t>(dbc::SSignalDef::EMultiplexBitmask::mltplx_switch));
    ASSERT_EQ(prSigDef.first.vecExtMultiplex.size(), 0u);
}

TEST_F(CDbcParserTest, ExampleDBC1)
{
    dbc::CDbcParser parser;

    // Message definition
    std::string ssMsgDef = R"code(
VERSION ""

NS_ :
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SGTYPE_
    SGTYPE_VAL_
    BA_DEF_SGTYPE_
    BA_SGTYPE_
    SIG_TYPE_REF_
    VAL_TABLE_
    SIG_GROUP_
    SIG_VALTYPE_
    SIGTYPE_VALTYPE_
    BO_TX_BU_
    BA_DEF_REL_
    BA_REL_
    BA_DEF_DEF_REL_
    BU_SG_REL_
    BU_EV_REL_
    BU_BO_REL_

BS_:

BU_: Engine Gateway

BO_ 100 EngineData: 8 Engine
    SG_ PetrolLevel : 24|8@1+ (1,0) [0|255] "l" Gateway
    SG_ EngPower : 48|16@1+ (0.01,0) [0|150] "kW" Gateway
    SG_ EngForce : 32|16@1+ (1,0) [0|0] "N" Gateway
    SG_ IdleRunning : 23|1@1+ (1,0) [0|0] "" Gateway
    SG_ EngTemp : 16|7@1+ (2,-50) [-50|150] "degC" Gateway
    SG_ EngSpeed : 0|16@1+ (1,0) [0|8000] "rpm" Gateway

CM_ "CAN communication matrix for power train electronics
*******************************************************
implemented: turn lights, warning lights, windows";

VAL_ 100 IdleRunning 0 "Running" 1 "Idle" ;
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
}
TEST_F(CDbcParserTest, ExampleDBC2)
{
    dbc::CDbcParser parser;

    // Message definition
    std::string ssMsgDef = R"code(
VERSION ""


NS_ :
	NS_DESC_
	CM_
	BA_DEF_
	BA_
	VAL_
	CAT_DEF_
	CAT_
	FILTER
	BA_DEF_DEF_
	EV_DATA_
	ENVVAR_DATA_
	SGTYPE_
	SGTYPE_VAL_
	BA_DEF_SGTYPE_
	BA_SGTYPE_
	SIG_TYPE_REF_
	VAL_TABLE_
	SIG_GROUP_
	SIG_VALTYPE_
	SIGTYPE_VALTYPE_
	BO_TX_BU_
	BA_DEF_REL_
	BA_REL_
	BA_DEF_DEF_REL_
	BU_SG_REL_
	BU_EV_REL_
	BU_BO_REL_
	SG_MUL_VAL_

BS_:

BU_: ProConnect Goldbox DataLynx HMI MAB
VAL_TABLE_ HMI_Fct_Req 8 "FCT_DEACTVN_REQ" 7 "FCT_ACTVN_OTA_REQ" 6 "FCT_ACTVN_SAVETHESPOILER_CFMD" 5 "FCT_ACTVN_SAVETHESPOILER_REQ" 4 "FCT_ACTVN_CRABWALK_CFMD" 3 "FCT_ACTVN_CRABWALK_REQ" 2 "FCT_ACTVN_AI4MTN_CFMD" 1 "FCT_ACTVN_AI4MTN_REQ" 0 "FCT_ACTVN_NONE" ;
VAL_TABLE_ Lateral_Longitudinal_AI4Motion 3 "LONGITUDINAL_CTRL_REQ" 2 "LONGITUDINAL_AND_LATERAL" 1 "LATERAL_CTRL_REQ" 0 "NONE" ;
VAL_TABLE_ OpenLoop_ClosedLoop_AI4Motion 2 "OPEN_LOOP_REQ" 1 "CLOSED_LOOP_REQ" 0 "NONE" ;
VAL_TABLE_ SOVD_states 2 "SOVD_SHOWCASE_ACTIVE" 1 "SOVD_SHOWCASE_DEACTIVE" 0 "SOVD_NONE" ;
VAL_TABLE_ OTA_states 7 "OTA_DOWNLOAD_FAILED" 6 "OTA_INSTALL_FAILED" 5 "OTA_INSTALL_FINISHED" 4 "OTA_INSTALL_START" 3 "OTA_DOWNLOAD_FINISHED" 2 "OTA_DOWNLOAD_START" 1 "OTA_AVAILABLE" 0 "OTA_NONE" ;
VAL_TABLE_ AI4Motion_Variant 4 "NONE" 3 "VARIANT_4" 2 "VARIANT_3" 1 "VARIANT_2" 0 "VARIANT_1" ;


BO_ 3221225472 VECTOR__INDEPENDENT_SIG_MSG: 0 Vector__XXX
 SG_ New_Signal_25 : 0|64@1- (1,0) [0|0] "" Vector__XXX

BO_ 6 Test: 8 Vector__XXX
 SG_ Signal_1 : 7|4@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Signal_2 : 3|8@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Signal_3 : 11|6@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Signal_4 : 21|6@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Signal_5 : 31|8@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Signal_6 : 39|32@0+ (1,0) [0|0] "" Vector__XXX

BO_ 5 ProConnect_OTA_Status: 8 ProConnect
 SG_ ProConnect_OTA_Sts : 7|8@0+ (1,0) [0|0] "enum"  HMI,MAB

BO_ 4 MAB_HMI_Feedback: 6 MAB
 SG_ MAB_Fault_codes : 23|32@0+ (1,0) [0|0] "bit coded"  HMI
 SG_ MAB_Global_activation_status : 7|16@0+ (1,0) [0|0] "bit coded"  HMI

BO_ 3 HMI_OTA_SOVD: 2 HMI

BO_ 2 DataLynx_AI4Motion: 7 DataLynx
 SG_ DataLynx_Brake_Deceleration_Req : 55|8@0+ (1,0) [0|0] "m/s^2"  MAB
 SG_ DataLynx_Brake_Torque_Req : 47|8@0+ (1,0) [0|0] "Nm"  MAB
 SG_ DataLynx_Camera_Rolling_Counter : 39|8@0+ (1,0) [0|14] "unitless integer"  MAB
 SG_ DataLynx_Rolling_Counter : 31|8@0+ (1,0) [0|14] "unitless integer"  MAB
 SG_ DataLynx_Steering_Angle_Request : 23|8@0- (1,0) [0|0] "radians"  MAB
 SG_ DataLynx_Brake_Perc_Request : 15|8@0+ (1,0) [0|100] "percent"  MAB
 SG_ DataLynx_Acceleration_Request : 7|8@0+ (1,0) [0|100] "percent"  MAB

BO_ 1 HMI_Requests: 4 HMI
 SG_ HMI_Drvr_Req : 31|8@0+ (1,0) [0|0] "enum"  ProConnect,MAB
 SG_ HMI_open_closed_loop_Req : 23|8@0+ (1,0) [0|0] ""  MAB
 SG_ HMI_Lateral_Long_Ctrl_Req : 15|8@0+ (1,0) [0|0] ""  MAB
 SG_ HMI_Algo_Variant_Request : 7|8@0- (1,0) [0|0] "enum"  MAB

BO_ 0 MAB_AI4Motion_Actvn: 4 MAB
 SG_ MAB_AI4Motion_Algo_Var_Selected : 31|8@0+ (1,0) [0|0] "enum"  DataLynx
 SG_ MAB_Actual_Steering_angle : 23|8@0- (1,0) [0|0] "radian"  DataLynx
 SG_ MAB_Actual_vehicle_speed : 15|8@0+ (1,0) [0|0] "m/sec"  DataLynx
 SG_ MAB_Activation_AI4Motion : 7|8@0+ (1,0) [0|0] "enum"  DataLynx



CM_ BO_ 3221225472 "This is a message for not used signals, created by Vector CANdb++ DBC OLE DB Provider.";
CM_ SG_ 4 MAB_Fault_codes "Bit coded signal

";
CM_ SG_ 4 MAB_Global_activation_status "Bit coded signal";
CM_ SG_ 2 DataLynx_Camera_Rolling_Counter "Replace this signal with an error signal (flag) which is detected by VAPI if camera image is frozen";
CM_ SG_ 2 DataLynx_Steering_Angle_Request "Clarify with Daniel if Radlenkwinkel oder Lenkradwinkel";
BA_DEF_ BO_  "GenMsgStartDelayTime" INT 0 0;
BA_DEF_ BO_  "GenMsgSendType" INT 0 0;
BA_DEF_ BO_  "GenMsgRequestable" INT 0 0;
BA_DEF_ BO_  "GenMsgNrOfRepetition" INT 0 0;
BA_DEF_ BO_  "GenMsgLSupport" INT 0 0;
BA_DEF_ BO_  "GenMsgFastOnStart" INT 0 0;
BA_DEF_ BO_  "GenMsgDelayTime" INT 0 0;
BA_DEF_ BO_  "GenMsgCycleTimeFast" INT 0 1000;
BA_DEF_ BO_  "GenMsgCycleTime" INT 0 1000;
BA_DEF_  "BusType" STRING ;
BA_DEF_DEF_  "GenMsgStartDelayTime" 0;
BA_DEF_DEF_  "GenMsgSendType" 0;
BA_DEF_DEF_  "GenMsgRequestable" 0;
BA_DEF_DEF_  "GenMsgNrOfRepetition" 0;
BA_DEF_DEF_  "GenMsgLSupport" 0;
BA_DEF_DEF_  "GenMsgFastOnStart" 0;
BA_DEF_DEF_  "GenMsgDelayTime" 0;
BA_DEF_DEF_  "GenMsgCycleTimeFast" 100;
BA_DEF_DEF_  "GenMsgCycleTime" 100;
BA_DEF_DEF_  "BusType" "";
BA_ "BusType" "CAN";
VAL_ 5 ProConnect_OTA_Sts 7 "OTA_DOWNLOAD_FAILED" 6 "OTA_INSTALL_FAILED" 5 "OTA_INSTALL_FINISHED" 4 "OTA_INSTALL_START" 3 "OTA_DOWNLOAD_FINISHED" 2 "OTA_DOWNLOAD_START" 1 "OTA_AVAILABLE" 0 "OTA_NONE" ;
VAL_ 1 HMI_Drvr_Req 8 "FCT_DEACTVN_REQ" 7 "FCT_ACTVN_OTA_REQ" 6 "FCT_ACTVN_SAVETHESPOILER_CFMD" 5 "FCT_ACTVN_SAVETHESPOILER_REQ" 4 "FCT_ACTVN_CRABWALK_CFMD" 3 "FCT_ACTVN_CRABWALK_REQ" 2 "FCT_ACTVN_AI4MTN_CFMD" 1 "FCT_ACTVN_AI4MTN_REQ" 0 "FCT_ACTVN_NONE" ;
VAL_ 1 HMI_open_closed_loop_Req 2 "OPEN_LOOP_REQ" 1 "CLOSED_LOOP_REQ" 0 "NONE" ;
VAL_ 1 HMI_Lateral_Long_Ctrl_Req 3 "LONGITUDINAL_CTRL_REQ" 2 "LONGITUDINAL_AND_LATERAL" 1 "LATERAL_CTRL_REQ" 0 "NONE" ;
VAL_ 1 HMI_Algo_Variant_Request 4 "NONE" 3 "VARIANT_4" 2 "VARIANT_3" 1 "VARIANT_2" 0 "VARIANT_1" ;
VAL_ 0 MAB_AI4Motion_Algo_Var_Selected 4 "NONE" 3 "VARIANT_4" 2 "VARIANT_3" 1 "VARIANT_2" 0 "VARIANT_1" ;
VAL_ 0 MAB_Activation_AI4Motion 3 "LONGITUDINAL_CTRL_REQ" 2 "LONGITUDINAL_AND_LATERAL" 1 "LATERAL_CTRL_REQ" 0 "NONE" ;
SIG_VALTYPE_ 3221225472 New_Signal_25 : 2;    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
}
TEST_F(CDbcParserTest, ExampleDBC3)
{
    dbc::CDbcParser parser;

    // Message definition
    std::string ssMsgDef = R"code(
VERSION ""


                        NS_ :
                        BA_
                        BA_DEF_
                        BA_DEF_DEF_
                        BA_DEF_DEF_REL_
                        BA_DEF_REL_
                        BA_DEF_SGTYPE_
                        BA_REL_
                        BA_SGTYPE_
                        BO_TX_BU_
                        BU_BO_REL_
                        BU_EV_REL_
                        BU_SG_REL_
                        CAT_
                        CAT_DEF_
                        CM_
                        ENVVAR_DATA_
                        EV_DATA_
                        FILTER
                        NS_DESC_
                        SGTYPE_
                        SGTYPE_VAL_
                        SG_MUL_VAL_
                        SIGTYPE_VALTYPE_
                        SIG_GROUP_
                        SIG_TYPE_REF_
                        SIG_VALTYPE_
                        VAL_
                        VAL_TABLE_


                        BS_:


                        BU_: DBG DRIVER IO MOTOR SENSOR


                        BO_ 100 DRIVER_HEARTBEAT: 1 DRIVER
                        SG_ DRIVER_HEARTBEAT_cmd : 0|8@1+ (1,0) [0|0] "" SENSOR,MOTOR


                        BO_ 500 IO_DEBUG: 4 IO
                        SG_ IO_DEBUG_test_unsigned : 0|8@1+ (1,0) [0|0] "" DBG
                        SG_ IO_DEBUG_test_enum : 8|8@1+ (1,0) [0|0] "" DBG
                        SG_ IO_DEBUG_test_signed : 16|8@1- (1,0) [0|0] "" DBG
                        SG_ IO_DEBUG_test_float : 24|8@1+ (0.5,0) [0|0] "" DBG


                        BO_ 101 MOTOR_CMD: 1 DRIVER
                        SG_ MOTOR_CMD_steer : 0|4@1- (1,-5) [-5|5] "" MOTOR
                        SG_ MOTOR_CMD_drive : 4|4@1+ (1,0) [0|9] "" MOTOR


                        BO_ 400 MOTOR_STATUS: 3 MOTOR
                        SG_ MOTOR_STATUS_wheel_error : 0|1@1+ (1,0) [0|0] "" DRIVER,IO
                        SG_ MOTOR_STATUS_speed_kph : 8|16@1+ (0.001,0) [0|0] "kph" DRIVER,IO


                        BO_ 200 SENSOR_SONARS: 8 SENSOR
                        SG_ SENSOR_SONARS_mux M : 0|4@1+ (1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_err_count : 4|12@1+ (1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_left m0 : 16|12@1+ (0.1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_middle m0 : 28|12@1+ (0.1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_right m0 : 40|12@1+ (0.1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_rear m0 : 52|12@1+ (0.1,0) [0|0] "" DRIVER,IO
                        SG_ SENSOR_SONARS_no_filt_left m1 : 16|12@1+ (0.1,0) [0|0] "" DBG
                        SG_ SENSOR_SONARS_no_filt_middle m1 : 28|12@1+ (0.1,0) [0|0] "" DBG
                        SG_ SENSOR_SONARS_no_filt_right m1 : 40|12@1+ (0.1,0) [0|0] "" DBG
                        SG_ SENSOR_SONARS_no_filt_rear m1 : 52|12@1+ (0.1,0) [0|0] "" DBG


                        CM_ BU_ DRIVER "The driver controller driving the car";
                        CM_ BU_ MOTOR "The motor controller of the car";
                        CM_ BU_ SENSOR "The sensor controller of the car";
                        CM_ BO_ 100 "Sync message used to synchronize the controllers";


                        BA_DEF_ "BusType" STRING ;
                        BA_DEF_ BO_ "GenMsgCycleTime" INT 0 0;
                        BA_DEF_ SG_ "FieldType" STRING ;


                        BA_DEF_DEF_ "BusType" "CAN";
                        BA_DEF_DEF_ "FieldType" "";
                        BA_DEF_DEF_ "GenMsgCycleTime" 0;


                        BA_ "GenMsgCycleTime" BO_ 100 1000;
                        BA_ "GenMsgCycleTime" BO_ 500 100;
                        BA_ "GenMsgCycleTime" BO_ 101 100;
                        BA_ "GenMsgCycleTime" BO_ 400 100;
                        BA_ "GenMsgCycleTime" BO_ 200 100;
                        BA_ "FieldType" SG_ 100 DRIVER_HEARTBEAT_cmd "DRIVER_HEARTBEAT_cmd";
                        BA_ "FieldType" SG_ 500 IO_DEBUG_test_enum "IO_DEBUG_test_enum";


                        VAL_ 100 DRIVER_HEARTBEAT_cmd 2 "DRIVER_HEARTBEAT_cmd_REBOOT" 1 "DRIVER_HEARTBEAT_cmd_SYNC" 0
                        "DRIVER_HEARTBEAT_cmd_NOOP" ;
                        VAL_ 500 IO_DEBUG_test_enum 2 "IO_DEBUG_test2_enum_two" 1 "IO_DEBUG_test2_enum_one" ;
    )code";
    dbc::CDbcSource srcMsgDef(ssMsgDef);
    EXPECT_NO_THROW(parser.Parse(srcMsgDef));
}
