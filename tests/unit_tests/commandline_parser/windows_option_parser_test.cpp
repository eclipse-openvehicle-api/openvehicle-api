#include "../../include/gtest_custom.h"
#include "commandline_parser_test.h"
#include "../../../global/cmdlnparser/cmdlnparser.h"

#ifdef _WIN32

TEST_F(CCommandLineParserTest, WOptionParseBoolean)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/select"};
    CCommandLine cl;
    bool bSelect = false; cl.DefineOption("select", bSelect, "Select it!");
    bool bControlNegative = false; cl.DefineOption("neg_control", bControlNegative, "No change!");
    bool bControlPositive = true; cl.DefineOption("pos_control", bControlPositive, "No change!");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_TRUE(bSelect);
    EXPECT_FALSE(bControlNegative);
    EXPECT_TRUE(bControlPositive);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralIndependent)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/i8var=-1", "/i16var=-2", "/i32var=-3", "/i64var=-4",
        "/ui8var=5", "/ui16var=6", "/ui32var=7", "/ui64var=8"};
    CCommandLine cl;
    int8_t i8Var = 0; cl.DefineOption("i8var", i8Var, "int8_t variable");
    int16_t i16Var = 0; cl.DefineOption("i16var", i16Var, "int16_t variable");
    int32_t i32Var = 0; cl.DefineOption("i32var", i32Var, "int32_t variable");
    int64_t i64Var = 0; cl.DefineOption("i64var", i64Var, "int64_t variable");
    uint8_t ui8Var = 0; cl.DefineOption("ui8var", ui8Var, "uint8_t variable");
    uint16_t ui16Var = 0; cl.DefineOption("ui16var", ui16Var, "uint16_t variable");
    uint32_t ui32Var = 0; cl.DefineOption("ui32var", ui32Var, "uint32_t variable");
    uint64_t ui64Var = 0; cl.DefineOption("ui64var", ui64Var, "uint64_t variable");
    int8_t i8Control = -10; cl.DefineOption("i8control", i8Control, "int8_t no change!");
    int16_t i16Control = -20; cl.DefineOption("i16control", i16Control, "int16_t no change!");
    int32_t i32Control = -30; cl.DefineOption("i32control", i32Control, "int32_t no change!");
    int64_t i64Control = -40; cl.DefineOption("i64control", i64Control, "int64_t no change!");
    uint8_t ui8Control = 50; cl.DefineOption("ui8control", ui8Control, "uint8_t no change!");
    uint16_t ui16Control = 60; cl.DefineOption("ui16control", ui16Control, "uint16_t no change!");
    uint32_t ui32Control = 70; cl.DefineOption("ui32control", ui32Control, "uint32_t no change!");
    uint64_t ui64Control = 80; cl.DefineOption("ui64control", ui64Control, "uint64_t no change!");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(i8Var, -1);
    EXPECT_EQ(i16Var, -2);
    EXPECT_EQ(i32Var, -3);
    EXPECT_EQ(i64Var, -4);
    EXPECT_EQ(ui8Var, 5u);
    EXPECT_EQ(ui16Var, 6u);
    EXPECT_EQ(ui32Var, 7u);
    EXPECT_EQ(ui64Var, 8u);
    EXPECT_EQ(i8Control, -10);
    EXPECT_EQ(i16Control, -20);
    EXPECT_EQ(i32Control, -30);
    EXPECT_EQ(i64Control, -40);
    EXPECT_EQ(ui8Control, 50u);
    EXPECT_EQ(ui16Control, 60u);
    EXPECT_EQ(ui32Control, 70u);
    EXPECT_EQ(ui64Control, 80u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralDedependent)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/cvar=-1", "/svar=-2", "/lvar=-3", "/ivar=-4", "/llvar=-5",
        "/ucvar=6", "/usvar=7", "/ulvar=8", "/uivar=9", "/ullvar=10", "/nvar=11"};
    CCommandLine cl;
    signed char cVar = 0; cl.DefineOption("cvar", cVar, "char variable");
    short sVar = 0; cl.DefineOption("svar", sVar, "short variable");
    long lVar = 0; cl.DefineOption("lvar", lVar, "long variable");
    int iVar = 0; cl.DefineOption("ivar", iVar, "int variable");
    long long llVar = 0; cl.DefineOption("llvar", llVar, "long long variable");
    unsigned char ucVar = 0; cl.DefineOption("ucvar", ucVar, "unsigned char variable");
    unsigned short usVar = 0; cl.DefineOption("usvar", usVar, "unsigned short variable");
    unsigned long ulVar = 0; cl.DefineOption("ulvar", ulVar, "unsigned long variable");
    unsigned int uiVar = 0; cl.DefineOption("uivar", uiVar, "unsigned int variable");
    unsigned long long ullVar = 0; cl.DefineOption("ullvar", ullVar, "unsigned long long variable");
    size_t nVar = 0; cl.DefineOption("nvar", nVar, "size_t variable");
    signed char cControl = -10; cl.DefineOption("ccontrol", cControl, "char no change!");
    short sControl = -20; cl.DefineOption("scontrol", sControl, "short no change!");
    long lControl = -30; cl.DefineOption("lcontrol", lControl, "long no change!");
    int iControl = -40; cl.DefineOption("icontrol", iControl, "int no change!");
    long long llControl = -50; cl.DefineOption("llcontrol", llControl, "long long no change!");
    unsigned char ucControl = 60; cl.DefineOption("uccontrol", ucControl, "unsigned char no change!");
    unsigned short usControl = 70; cl.DefineOption("uscontrol", usControl, "unsigned short no change!");
    unsigned long ulControl = 80; cl.DefineOption("ulcontrol", ulControl, "unsigned long no change!");
    unsigned int uiControl = 90; cl.DefineOption("uicontrol", uiControl, "unsigned int no change!");
    unsigned long long ullControl = 100; cl.DefineOption("ullcontrol", ullControl, "unsigned long long no change!");
    size_t nControl = 110; cl.DefineOption("ncontrol", nControl, "size_t no change!");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(cVar, -1);
    EXPECT_EQ(sVar, -2);
    EXPECT_EQ(lVar, -3);
    EXPECT_EQ(iVar, -4);
    EXPECT_EQ(llVar, -5);
    EXPECT_EQ(ucVar, 6u);
    EXPECT_EQ(usVar, 7u);
    EXPECT_EQ(ulVar, 8u);
    EXPECT_EQ(uiVar, 9u);
    EXPECT_EQ(ullVar, 10u);
    EXPECT_EQ(nVar, 11u);
    EXPECT_EQ(cControl, -10);
    EXPECT_EQ(sControl, -20);
    EXPECT_EQ(lControl, -30);
    EXPECT_EQ(iControl, -40);
    EXPECT_EQ(llControl, -50);
    EXPECT_EQ(ucControl, 60u);
    EXPECT_EQ(usControl, 70u);
    EXPECT_EQ(ulControl, 80u);
    EXPECT_EQ(uiControl, 90u);
    EXPECT_EQ(ullControl, 100u);
    EXPECT_EQ(nControl, 110u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralIndependentVector)
{
    // Attention: std::vector<int8_t> causes compiler problems with MSVC and GCC.
    const char* rgszCommandLine[] = {"this_exe.app", /*"/i8var=-1,2, -3",*/ "/i16var=-2,3,-4", "/i32var=-3,4,-5", "/i64var=-4,5,-6",
        "/ui8var=5,6,7", "/ui16var=6,7,8", "/ui32var=7,8,9", "/ui64var=8,9,10"};
    CCommandLine cl;
    //std::vector<int8_t> veci8Var; cl.DefineOption("i8var", veci8Var, "vector of int8_t variable");
    std::vector<int16_t> veci16Var; cl.DefineOption("i16var", veci16Var, "vector of int16_t variable");
    std::vector<int32_t> veci32Var; cl.DefineOption("i32var", veci32Var, "vector of int32_t variable");
    std::vector<int64_t> veci64Var; cl.DefineOption("i64var", veci64Var, "vector of int64_t variable");
    std::vector<uint8_t> vecui8Var; cl.DefineOption("ui8var", vecui8Var, "vector of uint8_t variable");
    std::vector<uint16_t> vecui16Var; cl.DefineOption("ui16var", vecui16Var, "vector of uint16_t variable");
    std::vector<uint32_t> vecui32Var; cl.DefineOption("ui32var", vecui32Var, "vector of uint32_t variable");
    std::vector<uint64_t> vecui64Var; cl.DefineOption("ui64var", vecui64Var, "vector of uint64_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    //EXPECT_ARREQ(veci8Var, -1, 2, -3);
    EXPECT_ARREQ(veci16Var, -2, 3, -4);
    EXPECT_ARREQ(veci32Var, -3, 4, -5);
    EXPECT_ARREQ(veci64Var, -4, 5, -6);
    EXPECT_ARREQ(vecui8Var, 5u, 6u, 7u);
    EXPECT_ARREQ(vecui16Var, 6u, 7u, 8u);
    EXPECT_ARREQ(vecui32Var, 7u, 8u, 9u);
    EXPECT_ARREQ(vecui64Var, 8u, 9u, 10u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralDedependentVector)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/cvar=-1,2,-3", "/svar=-2,3,-4", "/lvar=-3,4,-5", "/ivar=-4,5,-6", "/llvar=-5,6,-7",
        "/ucvar=6,7,8", "/usvar=7,8,9", "/ulvar=8,9,10", "/uivar=9,10,11", "/ullvar=10,11,12", "/nvar=11,12,13"};
    CCommandLine cl;
    std::vector<signed char> veccVar; cl.DefineOption("cvar", veccVar, "vector of char variable");
    std::vector<short> vecsVar; cl.DefineOption("svar", vecsVar, "vector of short variable");
    std::vector<long> veclVar; cl.DefineOption("lvar", veclVar, "vector of long variable");
    std::vector<int> veciVar; cl.DefineOption("ivar", veciVar, "vector of int variable");
    std::vector<long long> vecllVar; cl.DefineOption("llvar", vecllVar, "vector of long long variable");
    std::vector<unsigned char> vecucVar; cl.DefineOption("ucvar", vecucVar, "vector of unsigned char variable");
    std::vector<unsigned short> vecusVar; cl.DefineOption("usvar", vecusVar, "vector of unsigned short variable");
    std::vector<unsigned long> veculVar; cl.DefineOption("ulvar", veculVar, "vector of unsigned long variable");
    std::vector<unsigned int> vecuiVar; cl.DefineOption("uivar", vecuiVar, "vector of unsigned int variable");
    std::vector<unsigned long long> vecullVar; cl.DefineOption("ullvar", vecullVar, "vector of unsigned long long variable");
    std::vector<size_t> vecnVar; cl.DefineOption("nvar", vecnVar, "list of size_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(veccVar, -1, 2, -3);
    EXPECT_ARREQ(vecsVar, -2, 3, -4);
    EXPECT_ARREQ(veclVar, -3, 4, -5);
    EXPECT_ARREQ(veciVar, -4, 5, -6);
    EXPECT_ARREQ(vecllVar, -5, 6, -7);
    EXPECT_ARREQ(vecucVar, 6u, 7u, 8u);
    EXPECT_ARREQ(vecusVar, 7u, 8u, 9u);
    EXPECT_ARREQ(veculVar, 8u, 9u, 10u);
    EXPECT_ARREQ(vecuiVar, 9u, 10u, 11u);
    EXPECT_ARREQ(vecullVar, 10u, 11u, 12u);
    EXPECT_ARREQ(vecnVar, 11u, 12u, 13u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralIndependentSequence)
{
    // Attention: std::vector<int8_t> causes compiler problems with MSVC and GCC.
    const char* rgszCommandLine[] = {"this_exe.app", /*"/i8var=-1,2, -3",*/ "/i16var=-2,3,-4", "/i32var=-3,4,-5", "/i64var=-4,5,-6",
        "/ui8var=5,6,7", "/ui16var=6,7,8", "/ui32var=7,8,9", "/ui64var=8,9,10"};
    CCommandLine cl;
    //sdv::sequence<int8_t> seqi8Var; cl.DefineOption("i8var", seqi8Var, "sequence of int8_t variable");
    sdv::sequence<int16_t> seqi16Var; cl.DefineOption("i16var", seqi16Var, "sequence of int16_t variable");
    sdv::sequence<int32_t> seqi32Var; cl.DefineOption("i32var", seqi32Var, "sequence of int32_t variable");
    sdv::sequence<int64_t> seqi64Var; cl.DefineOption("i64var", seqi64Var, "sequence of int64_t variable");
    sdv::sequence<uint8_t> sequi8Var; cl.DefineOption("ui8var", sequi8Var, "sequence of uint8_t variable");
    sdv::sequence<uint16_t> sequi16Var; cl.DefineOption("ui16var", sequi16Var, "sequence of uint16_t variable");
    sdv::sequence<uint32_t> sequi32Var; cl.DefineOption("ui32var", sequi32Var, "sequence of uint32_t variable");
    sdv::sequence<uint64_t> sequi64Var; cl.DefineOption("ui64var", sequi64Var, "sequence of uint64_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    //EXPECT_ARREQ(seqi8Var, -1, 2, -3);
    EXPECT_ARREQ(seqi16Var, -2, 3, -4);
    //EXPECT_ARREQ(seqi32Var, -3, 4, -5);
    EXPECT_ARREQ(seqi64Var, -4, 5, -6);
    EXPECT_ARREQ(sequi8Var, 5u, 6u, 7u);
    EXPECT_ARREQ(sequi16Var, 6u, 7u, 8u);
    EXPECT_ARREQ(sequi32Var, 7u, 8u, 9u);
    EXPECT_ARREQ(sequi64Var, 8u, 9u, 10u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralDedependentSequence)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/cvar=-1,2,-3", "/svar=-2,3,-4", "/lvar=-3,4,-5", /*"/ivar=-4,5,-6",*/ "/llvar=-5,6,-7",
        "/ucvar=6,7,8", "/usvar=7,8,9", "/ulvar=8,9,10", "/uivar=9,10,11", "/ullvar=10,11,12", "/nvar=11,12,13"};
    CCommandLine cl;
    sdv::sequence<signed char> seqcVar; cl.DefineOption("cvar", seqcVar, "sequence of char variable");
    sdv::sequence<short> seqsVar; cl.DefineOption("svar", seqsVar, "sequence of short variable");
    sdv::sequence<long> seqlVar; cl.DefineOption("lvar", seqlVar, "sequence of long variable");
    //sdv::sequence<int> seqiVar; cl.DefineOption("ivar", seqiVar, "sequence of int variable");
    sdv::sequence<long long> seqllVar; cl.DefineOption("llvar", seqllVar, "sequence of long long variable");
    sdv::sequence<unsigned char> sequcVar; cl.DefineOption("ucvar", sequcVar, "sequence of unsigned char variable");
    sdv::sequence<unsigned short> sequsVar; cl.DefineOption("usvar", sequsVar, "sequence of unsigned short variable");
    sdv::sequence<unsigned long> sequlVar; cl.DefineOption("ulvar", sequlVar, "sequence of unsigned long variable");
    sdv::sequence<unsigned int> sequiVar; cl.DefineOption("uivar", sequiVar, "sequence of unsigned int variable");
    sdv::sequence<unsigned long long> sequllVar; cl.DefineOption("ullvar", sequllVar, "sequence of unsigned long long variable");
    sdv::sequence<size_t> seqnVar; cl.DefineOption("nvar", seqnVar, "list of size_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(seqcVar, -1, 2, -3);
    EXPECT_ARREQ(seqsVar, -2, 3, -4);
    EXPECT_ARREQ(seqlVar, -3, 4, -5);
    //EXPECT_ARREQ(seqiVar, -4, 5, -6);
    EXPECT_ARREQ(seqllVar, -5, 6, -7);
    EXPECT_ARREQ(sequcVar, 6u, 7u, 8u);
    EXPECT_ARREQ(sequsVar, 7u, 8u, 9u);
    EXPECT_ARREQ(sequlVar, 8u, 9u, 10u);
    EXPECT_ARREQ(sequiVar, 9u, 10u, 11u);
    EXPECT_ARREQ(sequllVar, 10u, 11u, 12u);
    EXPECT_ARREQ(seqnVar, 11u, 12u, 13u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralIndependentList)
{
    // Attention: std::list<int8_t> causes compiler problems with MSVC and GCC.
    const char* rgszCommandLine[] = {"this_exe.app", /*"/i8var=-1,2, -3",*/ "/i16var=-2,3,-4", "/i32var=-3,4,-5", "/i64var=-4,5,-6",
        "/ui8var=5,6,7", "/ui16var=6,7,8", "/ui32var=7,8,9", "/ui64var=8,9,10"};
    CCommandLine cl;
    //std::list<int8_t> lsti8Var; cl.DefineOption("i8var", lsti8Var, "list of int8_t variable");
    std::list<int16_t> lsti16Var; cl.DefineOption("i16var", lsti16Var, "list of int16_t variable");
    std::list<int32_t> lsti32Var; cl.DefineOption("i32var", lsti32Var, "list of int32_t variable");
    std::list<int64_t> lsti64Var; cl.DefineOption("i64var", lsti64Var, "list of int64_t variable");
    std::list<uint8_t> lstui8Var; cl.DefineOption("ui8var", lstui8Var, "list of uint8_t variable");
    std::list<uint16_t> lstui16Var; cl.DefineOption("ui16var", lstui16Var, "list of uint16_t variable");
    std::list<uint32_t> lstui32Var; cl.DefineOption("ui32var", lstui32Var, "list of uint32_t variable");
    std::list<uint64_t> lstui64Var; cl.DefineOption("ui64var", lstui64Var, "list of uint64_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    //EXPECT_ARREQ(lsti8Var, -1, 2, -3);
    EXPECT_ARREQ(lsti16Var, -2, 3, -4);
    EXPECT_ARREQ(lsti32Var, -3, 4, -5);
    EXPECT_ARREQ(lsti64Var, -4, 5, -6);
    EXPECT_ARREQ(lstui8Var, 5u, 6u, 7u);
    EXPECT_ARREQ(lstui16Var, 6u, 7u, 8u);
    EXPECT_ARREQ(lstui32Var, 7u, 8u, 9u);
    EXPECT_ARREQ(lstui64Var, 8u, 9u, 10u);
}

TEST_F(CCommandLineParserTest, WOptionParseIntegralDedependentList)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/cvar=-1,2,-3", "/svar=-2,3,-4", "/lvar=-3,4,-5", "/ivar=-4,5,-6", "/llvar=-5,6,-7",
        "/ucvar=6,7,8", "/usvar=7,8,9", "/ulvar=8,9,10", "/uivar=9,10,11", "/ullvar=10,11,12", "/nvar=11,12,13"};
    CCommandLine cl;
    std::list<signed char> lstcVar; cl.DefineOption("cvar", lstcVar, "list of char variable");
    std::list<short> lstsVar; cl.DefineOption("svar", lstsVar, "list of short variable");
    std::list<long> lstlVar; cl.DefineOption("lvar", lstlVar, "list of long variable");
    std::list<int> lstiVar; cl.DefineOption("ivar", lstiVar, "list of int variable");
    std::list<long long> lstllVar; cl.DefineOption("llvar", lstllVar, "list of long long variable");
    std::list<unsigned char> lstucVar; cl.DefineOption("ucvar", lstucVar, "list of unsigned char variable");
    std::list<unsigned short> lstusVar; cl.DefineOption("usvar", lstusVar, "list of unsigned short variable");
    std::list<unsigned long> lstulVar; cl.DefineOption("ulvar", lstulVar, "list of unsigned long variable");
    std::list<unsigned int> lstuiVar; cl.DefineOption("uivar", lstuiVar, "list of unsigned int variable");
    std::list<unsigned long long> lstullVar; cl.DefineOption("ullvar", lstullVar, "list of unsigned long long variable");
    std::list<size_t> lstnVar; cl.DefineOption("nvar", lstnVar, "list of size_t variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(lstcVar, -1, 2, -3);
    EXPECT_ARREQ(lstsVar, -2, 3, -4);
    EXPECT_ARREQ(lstlVar, -3, 4, -5);
    EXPECT_ARREQ(lstiVar, -4, 5, -6);
    EXPECT_ARREQ(lstllVar, -5, 6, -7);
    EXPECT_ARREQ(lstucVar, 6u, 7u, 8u);
    EXPECT_ARREQ(lstusVar, 7u, 8u, 9u);
    EXPECT_ARREQ(lstulVar, 8u, 9u, 10u);
    EXPECT_ARREQ(lstuiVar, 9u, 10u, 11u);
    EXPECT_ARREQ(lstullVar, 10u, 11u, 12u);
    EXPECT_ARREQ(lstnVar, 11u, 12u, 13u);
}

TEST_F(CCommandLineParserTest, WOptionParseFloatingPoint)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/fvar=-12345.6789", "/dvar=12345678.9E10", "/ldvar=0.12345678E-5"};
    CCommandLine cl;
    float fVar = 0; cl.DefineOption("fvar", fVar, "float variable");
    double dVar = 0; cl.DefineOption("dvar", dVar, "double variable");
    long double ldVar = 0; cl.DefineOption("ldvar", ldVar, "long double variable");
    float fControl = 123.456f; cl.DefineOption("fcontrol", fControl, "float no change!");
    double dControl = 456.789; cl.DefineOption("dcontrol", dControl, "double no change!");
    long double ldControl = 876.543; cl.DefineOption("ldcontrol", ldControl, "long double no change!");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_FPEQ(fVar, -12345.6789f);
    EXPECT_FPEQ(dVar, 12345678.9E10);
    EXPECT_FPEQ(ldVar, 0.12345678E-5);
    EXPECT_FPEQ(fControl, 123.456f);
    EXPECT_FPEQ(dControl, 456.789);
    EXPECT_FPEQ(ldControl, 876.543);
}

TEST_F(CCommandLineParserTest, WOptionParseFloatingPointVector)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/fvar=-12345.6789,9876.54321,134679", "/dvar=12345678.9E10,1098765.43E21", "/ldvar=0.12345678E-5,0.87654321E9"};
    CCommandLine cl;
    std::vector<float> vecfVar; cl.DefineOption("fvar", vecfVar, "vector of float variable");
    std::vector<double> vecdVar; cl.DefineOption("dvar", vecdVar, "vector of double variable");
    std::vector<long double> vecldVar; cl.DefineOption("ldvar", vecldVar, "vector of long double variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(vecfVar, -12345.6789f, 9876.54321f, 134679.f);
    EXPECT_ARREQ(vecdVar, 12345678.9E10, 1098765.43E21);
    // Attention: GCC doesn't support long double very well.
    //EXPECT_ARREQ(vecldVar, 0.12345678E-5, 0.87654321E9);
}

TEST_F(CCommandLineParserTest, WOptionParseFloatingPointSequence)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/fvar=-12345.6789,9876.54321,134679", "/dvar=12345678.9E10,1098765.43E21", "/ldvar=0.12345678E-5,0.87654321E9"};
    CCommandLine cl;
    sdv::sequence<float> seqfVar; cl.DefineOption("fvar", seqfVar, "sequence of float variable");
    sdv::sequence<double> seqdVar; cl.DefineOption("dvar", seqdVar, "sequence of double variable");
    sdv::sequence<long double> seqldVar; cl.DefineOption("ldvar", seqldVar, "sequence of long double variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(seqfVar, -12345.6789f, 9876.54321f, 134679.f);
    EXPECT_ARREQ(seqdVar, 12345678.9E10, 1098765.43E21);
    // Attention: GCC doesn't support long double very well.
    //EXPECT_ARREQ(seqldVar, 0.12345678E-5, 0.87654321E9);
}

TEST_F(CCommandLineParserTest, WOptionParseFloatingPointList)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/fvar=-12345.6789,9876.54321,134679", "/dvar=12345678.9E10,1098765.43E21", "/ldvar=0.12345678E-5,0.87654321E9"};
    CCommandLine cl;
    std::list<float> lstfVar; cl.DefineOption("fvar", lstfVar, "list of float variable");
    std::list<double> lstdVar; cl.DefineOption("dvar", lstdVar, "list of double variable");
    std::list<long double> lstldVar; cl.DefineOption("ldvar", lstldVar, "list of long double variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(lstfVar, -12345.6789f, 9876.54321f, 134679.f);
    EXPECT_ARREQ(lstdVar, 12345678.9E10, 1098765.43E21);
    // Attention: GCC doesn't support long double very well.
    //EXPECT_ARREQ(lstldVar, 0.12345678E-5, 0.87654321E9);
}

TEST_F(CCommandLineParserTest, WOptionParseEnum)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
    const char* rgszCommandLine[] = {"this_exe.app", "/unscoped_enum=test2", "/scoped_enum=test5"};
    CCommandLine cl;
    SEnumArgumentAssoc<EUnscopedTest> rgsUnscopedEnumAssociations[] = {
        {test1, "test1", "Test the #1"},
        {test2, "test2", "Test the #2"},
        {test3, "test3", "Test the #3"}
    };
    SEnumArgumentAssoc<EScopedTest> rgsScopedEnumAssociations[] = {
        {EScopedTest::test4, "test4", "Test the #4"},
        {EScopedTest::test5, "test5", "Test the #5"},
        {EScopedTest::test6, "test6", "Test the #6"}
    };
    EUnscopedTest eUnscopedTest = test1; cl.DefineOption("unscoped_enum", eUnscopedTest, "unscoped enum variable").AddAssociations(rgsUnscopedEnumAssociations);
    EScopedTest eScopedTest = EScopedTest::test4; cl.DefineOption("scoped_enum", eScopedTest, "scoped enum variable").AddAssociations(rgsScopedEnumAssociations);
    EUnscopedTest eUnscopedControl = test1; cl.DefineOption("unscoped_enum_control", eUnscopedControl, "unscoped enum no change!").AddAssociations(rgsUnscopedEnumAssociations);
    EScopedTest eScopedControl = EScopedTest::test4; cl.DefineOption("scoped_enum_control", eScopedControl, "scoped enum no change!").AddAssociations(rgsScopedEnumAssociations);
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(eUnscopedTest, test2);
    EXPECT_EQ(eScopedTest, EScopedTest::test5);
    EXPECT_EQ(eUnscopedControl, test1);
    EXPECT_EQ(eScopedControl, EScopedTest::test4);
}

TEST_F(CCommandLineParserTest, WOptionParseEnumVector)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
    const char* rgszCommandLine[] = {"this_exe.app", "/unscoped_enum=test2,test3", "/scoped_enum=test5,test6"};
    CCommandLine cl;
    SEnumArgumentAssoc<EUnscopedTest> rgsUnscopedEnumAssociations[] = {
        {test1, "test1", "Test the #1"},
        {test2, "test2", "Test the #2"},
        {test3, "test3", "Test the #3"}
    };
    SEnumArgumentAssoc<EScopedTest> rgsScopedEnumAssociations[] = {
        {EScopedTest::test4, "test4", "Test the #4"},
        {EScopedTest::test5, "test5", "Test the #5"},
        {EScopedTest::test6, "test6", "Test the #6"}
    };
    std::vector<EUnscopedTest> vecUnscopedTest; cl.DefineOption("unscoped_enum", vecUnscopedTest, "unscoped enum variable").AddAssociations(rgsUnscopedEnumAssociations);
    std::vector<EScopedTest> vecScopedTest; cl.DefineOption("scoped_enum", vecScopedTest, "scoped enum variable").AddAssociations(rgsScopedEnumAssociations);
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(vecUnscopedTest, test2, test3);
    EXPECT_ARREQ(vecScopedTest, EScopedTest::test5, EScopedTest::test6);
}

TEST_F(CCommandLineParserTest, WOptionParseEnumSequence)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
    const char* rgszCommandLine[] = {"this_exe.app", "/unscoped_enum=test2,test3", "/scoped_enum=test5,test6"};
    CCommandLine cl;
    SEnumArgumentAssoc<EUnscopedTest> rgsUnscopedEnumAssociations[] = {
        {test1, "test1", "Test the #1"},
        {test2, "test2", "Test the #2"},
        {test3, "test3", "Test the #3"}
    };
    SEnumArgumentAssoc<EScopedTest> rgsScopedEnumAssociations[] = {
        {EScopedTest::test4, "test4", "Test the #4"},
        {EScopedTest::test5, "test5", "Test the #5"},
        {EScopedTest::test6, "test6", "Test the #6"}
    };
    sdv::sequence<EUnscopedTest> seqUnscopedTest; cl.DefineOption("unscoped_enum", seqUnscopedTest, "unscoped enum variable").AddAssociations(rgsUnscopedEnumAssociations);
    sdv::sequence<EScopedTest> seqScopedTest; cl.DefineOption("scoped_enum", seqScopedTest, "scoped enum variable").AddAssociations(rgsScopedEnumAssociations);
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(seqUnscopedTest, test2, test3);
    EXPECT_ARREQ(seqScopedTest, EScopedTest::test5, EScopedTest::test6);
}

TEST_F(CCommandLineParserTest, WOptionParseEnumList)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
    const char* rgszCommandLine[] = {"this_exe.app", "/unscoped_enum=test2,test3", "/scoped_enum=test5,test6"};
    CCommandLine cl;
    SEnumArgumentAssoc<EUnscopedTest> rgsUnscopedEnumAssociations[] = {
        {test1, "test1", "Test the #1"},
        {test2, "test2", "Test the #2"},
        {test3, "test3", "Test the #3"}
    };
    SEnumArgumentAssoc<EScopedTest> rgsScopedEnumAssociations[] = {
        {EScopedTest::test4, "test4", "Test the #4"},
        {EScopedTest::test5, "test5", "Test the #5"},
        {EScopedTest::test6, "test6", "Test the #6"}
    };
    std::list<EUnscopedTest> lstUnscopedTest; cl.DefineOption("unscoped_enum", lstUnscopedTest, "unscoped enum variable").AddAssociations(rgsUnscopedEnumAssociations);
    std::list<EScopedTest> lstScopedTest; cl.DefineOption("scoped_enum", lstScopedTest, "scoped enum variable").AddAssociations(rgsScopedEnumAssociations);
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(lstUnscopedTest, test2, test3);
    EXPECT_ARREQ(lstScopedTest, EScopedTest::test5, EScopedTest::test6);
}

TEST_F(CCommandLineParserTest, WOptionParseString)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/ssvar1=test_a", "/ssu8var1=test_b", "/ssvar2=\"test_c\"",
        "/ssu8var2=\"test_d\""};
    CCommandLine cl;
    std::string ssVar1; cl.DefineOption("ssvar1", ssVar1, "std::string variable");
    sdv::u8string ssu8Var1; cl.DefineOption("ssu8var1", ssu8Var1, "sdv::u8string variable");
    std::string ssVar2; cl.DefineOption("ssvar2", ssVar2, "sdv::string variable");
    sdv::u8string ssu8Var2; cl.DefineOption("ssu8var2", ssu8Var2, "sdv::u8string variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(ssVar1, "test_a");
    EXPECT_EQ(ssu8Var1, "test_b");
    EXPECT_EQ(ssVar2, "test_c");
    EXPECT_EQ(ssu8Var2, "test_d");
}

TEST_F(CCommandLineParserTest, WOptionParseStringVector)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/ssvar1=test_a,test_b", "/ssu8var1=test_b,test_c", "/ssvar2=\"test_c\",\"test_d\"",
        "/ssu8var2=\"test_d\",\"test_e\""};
    CCommandLine cl;
    std::vector<std::string> vecssVar1; cl.DefineOption("ssvar1", vecssVar1, "std::vector<std::string> variable");
    std::vector<sdv::u8string> vecssu8Var1; cl.DefineOption("ssu8var1", vecssu8Var1, "std::vector<sdv::u8string> variable");
    std::vector<std::string> vecssVar2; cl.DefineOption("ssvar2", vecssVar2, "std::vector<sdv::string> variable");
    std::vector<sdv::u8string> vecssu8Var2; cl.DefineOption("ssu8var2", vecssu8Var2, "std::vector<sdv::u8string> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(vecssVar1, "test_a", "test_b");
    EXPECT_ARREQ(vecssu8Var1, "test_b", "test_c");
    EXPECT_ARREQ(vecssVar2, "test_c", "test_d");
    EXPECT_ARREQ(vecssu8Var2, "test_d", "test_e");
}

TEST_F(CCommandLineParserTest, WOptionParseStringSequence)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/ssvar1=test_a,test_b", "/ssu8var1=test_b,test_c", "/ssvar2=\"test_c\",\"test_d\"",
        "/ssu8var2=\"test_d\",\"test_e\""};
    CCommandLine cl;
    sdv::sequence<std::string> seqssVar1; cl.DefineOption("ssvar1", seqssVar1, "sdv::sequence<std::string> variable");
    sdv::sequence<sdv::u8string> seqssu8Var1; cl.DefineOption("ssu8var1", seqssu8Var1, "sdv::sequence<sdv::u8string> variable");
    sdv::sequence<std::string> seqssVar2; cl.DefineOption("ssvar2", seqssVar2, "sdv::sequence<sdv::string> variable");
    sdv::sequence<sdv::u8string> seqssu8Var2; cl.DefineOption("ssu8var2", seqssu8Var2, "sdv::sequence<sdv::u8string> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(seqssVar1, "test_a", "test_b");
    EXPECT_ARREQ(seqssu8Var1, "test_b", "test_c");
    EXPECT_ARREQ(seqssVar2, "test_c", "test_d");
    EXPECT_ARREQ(seqssu8Var2, "test_d", "test_e");
}

TEST_F(CCommandLineParserTest, WOptionParseStringList)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/ssvar1=test_a,test_b", "/ssu8var1=test_b,test_c", "/ssvar2=\"test_c\",\"test_d\"",
        "/ssu8var2=\"test_d\",\"test_e\""};
    CCommandLine cl;
    std::list<std::string> lstssVar1; cl.DefineOption("ssvar1", lstssVar1, "std::list<std::string> variable");
    std::list<sdv::u8string> lstssu8Var1; cl.DefineOption("ssu8var1", lstssu8Var1, "std::list<sdv::u8string> variable");
    std::list<std::string> lstssVar2; cl.DefineOption("ssvar2", lstssVar2, "std::list<std::string> variable");
    std::list<sdv::u8string> lstssu8Var2; cl.DefineOption("ssu8var2", lstssu8Var2, "std::list<sdv::u8string> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(lstssVar1, "test_a", "test_b");
    EXPECT_ARREQ(lstssu8Var1, "test_b", "test_c");
    EXPECT_ARREQ(lstssVar2, "test_c", "test_d");
    EXPECT_ARREQ(lstssu8Var2, "test_d", "test_e");
}

TEST_F(CCommandLineParserTest, WOptionParsePath)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/pathvar1=abc.def", "/pathvar2=\"ghi.jkl\""};
    CCommandLine cl;
    std::filesystem::path pathVar1; cl.DefineOption("pathvar1", pathVar1, "std::filesystem::path variable");
    std::filesystem::path pathVar2; cl.DefineOption("pathvar2", pathVar2, "std::filesystem::path variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_EQ(pathVar1, "abc.def");
    EXPECT_EQ(pathVar2, "ghi.jkl");
}

TEST_F(CCommandLineParserTest, WOptionParsePathVector)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/pathvar1=abc.def,ghi.jkl", "/pathvar2=\"ghi.jkl\",\"mno.pqr\""};
    CCommandLine cl;
    std::vector<std::filesystem::path> vecpathVar1; cl.DefineOption("pathvar1", vecpathVar1, "std::vector<std::filesystem::path> variable");
    std::vector<std::filesystem::path> vecpathVar2; cl.DefineOption("pathvar2", vecpathVar2, "std::vector<std::filesystem::path> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(vecpathVar1, "abc.def", "ghi.jkl");
    EXPECT_ARREQ(vecpathVar2, "ghi.jkl", "mno.pqr");
}

TEST_F(CCommandLineParserTest, WOptionParsePathSequence)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/pathvar1=abc.def,ghi.jkl", "/pathvar2=\"ghi.jkl\",\"mno.pqr\""};
    CCommandLine cl;
    sdv::sequence<std::filesystem::path> seqpathVar1; cl.DefineOption("pathvar1", seqpathVar1, "sdv::sequence<std::filesystem::path> variable");
    sdv::sequence<std::filesystem::path> seqpathVar2; cl.DefineOption("pathvar2", seqpathVar2, "sdv::sequence<std::filesystem::path> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(seqpathVar1, "abc.def", "ghi.jkl");
    EXPECT_ARREQ(seqpathVar2, "ghi.jkl", "mno.pqr");
}

TEST_F(CCommandLineParserTest, WOptionParsePathList)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/pathvar1=abc.def,ghi.jkl", "/pathvar2=\"ghi.jkl\",\"mno.pqr\""};
    CCommandLine cl;
    std::list<std::filesystem::path> lstpathVar1; cl.DefineOption("pathvar1", lstpathVar1, "std::list<std::filesystem::path> variable");
    std::list<std::filesystem::path> lstpathVar2; cl.DefineOption("pathvar2", lstpathVar2, "std::list<std::filesystem::path> variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_ARREQ(lstpathVar1, "abc.def", "ghi.jkl");
    EXPECT_ARREQ(lstpathVar2, "ghi.jkl", "mno.pqr");
}

TEST_F(CCommandLineParserTest, WOptionParseFlags)
{
    const char* rgszCommandLine[] = {"this_exe.app", "/flag_neg+", "/flag_pos-"};
    CCommandLine cl;
    bool bFlagNeg = false; cl.DefineFlagOption("flag_neg", bFlagNeg, "negative flag variable");
    bool bFlagPos = true; cl.DefineFlagOption("flag_pos", bFlagPos, "positive flag variable");
    bool bFlagNegControl = false; cl.DefineFlagOption("controlflag_neg", bFlagNegControl, "negative flag variable");
    bool bFlagPosControl = true; cl.DefineFlagOption("control_flag_pos", bFlagPosControl, "positive flag variable");
    EXPECT_NO_THROW(cl.Parse(std::extent<decltype(rgszCommandLine)>::value, rgszCommandLine));
    EXPECT_TRUE(bFlagNeg);
    EXPECT_FALSE(bFlagPos);
    EXPECT_FALSE(bFlagNegControl);
    EXPECT_TRUE(bFlagPosControl);
}

#endif // defined _WIN32