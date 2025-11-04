#include "../../include/gtest_custom.h"
#include "commandline_parser_test.h"
#include "../../../global/cmdlnparser/cmdlnparser.h"

using CCommandLineParserTestArgumentSelection = CCommandLineParserTest;

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseBoolean)
{
    CCommandLine cl;
    bool bSelect = false;
    auto& rSelect = cl.DefineSubOption("select", bSelect, "Select it!", true, 0, 1, 2);
    EXPECT_TRUE(rSelect.PartOfArgumentGroup(0));
    EXPECT_TRUE(rSelect.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSelect.PartOfArgumentGroup(2));
    EXPECT_TRUE(rSelect.PartOfArgumentGroup(3));    // Options of group #0 are always included.
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralIndependent)
{
    CCommandLine cl;
    int8_t i8Var = 0;
    auto& rI8Var = cl.DefineSubOption("i8var", i8Var, "int8_t variable", true, 0, 1);
    int16_t i16Var = 0;
    auto& rI16Var = cl.DefineSubOption("i16var", i16Var, "int16_t variable", true, 1, 2);
    int32_t i32Var = 0;
    auto& rI32Var = cl.DefineSubOption("i32var", i32Var, "int32_t variable", true, 2, 3);
    int64_t i64Var = 0;
    auto& rI64Var = cl.DefineSubOption("i64var", i64Var, "int64_t variable", true, 3, 4);
    uint8_t ui8Var = 0;
    auto& rUI8Var = cl.DefineSubOption("ui8var", ui8Var, "uint8_t variable", true, 4, 5);
    uint16_t ui16Var = 0;
    auto& rUI16Var = cl.DefineSubOption("ui16var", ui16Var, "uint16_t variable", true, 5, 6);
    uint32_t ui32Var = 0;
    auto& rUI32Var = cl.DefineSubOption("ui32var", ui32Var, "uint32_t variable", true, 6, 7);
    uint64_t ui64Var = 0;
    auto& rUI64Var = cl.DefineSubOption("ui64var", ui64Var, "uint64_t variable", true, 7, 8);

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(0));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(1));
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(1));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(2));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(2));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(3));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(4));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(5));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(6));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(7));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(8));

    EXPECT_TRUE(rI8Var.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(9));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralDedependent)
{
    CCommandLine cl;
    signed char cVar = 0;
    auto& rCVar = cl.DefineSubOption("cvar", cVar, "char variable", true, 0, 1);
    short sVar = 0;
    auto& rSVar = cl.DefineSubOption("svar", sVar, "short variable", true, 1, 2);
    long lVar = 0;
    auto& rLVar = cl.DefineSubOption("lvar", lVar, "long variable", true, 2, 3);
    int iVar = 0;
    auto& rIVar = cl.DefineSubOption("ivar", iVar, "int variable", true, 3, 4);
    long long llVar = 0;
    auto& rLLVar = cl.DefineSubOption("llvar", llVar, "long long variable", true, 4, 5);
    unsigned char ucVar = 0;
    auto& rUCVar = cl.DefineSubOption("ucvar", ucVar, "unsigned char variable", true, 5, 6);
    unsigned short usVar = 0;
    auto& rUSVar = cl.DefineSubOption("usvar", usVar, "unsigned short variable", true, 6, 7);
    unsigned long ulVar = 0;
    auto& rULVar = cl.DefineSubOption("ulvar", ulVar, "unsigned long variable", true, 7, 8);
    unsigned int uiVar = 0;
    auto& rUIVar = cl.DefineSubOption("uivar", uiVar, "unsigned int variable", true, 8, 9);
    unsigned long long ullVar = 0;
    auto& rULLVar = cl.DefineSubOption("ullvar", ullVar, "unsigned long long variable", true, 9, 10);
    size_t nVar = 0;
    auto& rNVar = cl.DefineSubOption("nvar", nVar, "size_t variable", true, 10, 11);

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(0));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(1));    // Options of group #0 are always included.
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(4));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(5));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(6));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(7));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(8));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(9));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(10));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(10));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(11));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(11));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(11));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(12));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(12));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralIndependentVector)
{
    // Attention: std::vector<int8_t> causes compiler problems with MSVC and GCC.
    CCommandLine cl;
    //std::vector<int8_t> veci8Var;
    //auto& rI8Var = cl.DefineSubOption("i8var", veci8Var, "vector of int8_t variable", true, 0, 1);
    std::vector<int16_t> veci16Var;
    auto& rI16Var = cl.DefineSubOption("i16var", veci16Var, "vector of int16_t variable", true, 1, 2);
    std::vector<int32_t> veci32Var;
    auto& rI32Var = cl.DefineSubOption("i32var", veci32Var, "vector of int32_t variable", true, 2, 3);
    std::vector<int64_t> veci64Var;
    auto& rI64Var = cl.DefineSubOption("i64var", veci64Var, "vector of int64_t variable", true, 3, 4);
    std::vector<uint8_t> vecui8Var;
    auto& rUI8Var = cl.DefineSubOption("ui8var", vecui8Var, "vector of uint8_t variable", true, 4, 5);
    std::vector<uint16_t> vecui16Var;
    auto& rUI16Var = cl.DefineSubOption("ui16var", vecui16Var, "vector of uint16_t variable", true, 5, 6);
    std::vector<uint32_t> vecui32Var;
    auto& rUI32Var = cl.DefineSubOption("ui32var", vecui32Var, "vector of uint32_t variable", true, 6, 7);
    std::vector<uint64_t> vecui64Var;
    auto& rUI64Var = cl.DefineSubOption("ui64var", vecui64Var, "vector of uint64_t variable", true, 7, 8);

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(0));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(1));
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(1));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(2));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(2));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(3));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(4));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(5));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(6));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(7));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(8));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(9));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralDedependentVector)
{
    CCommandLine cl;
    std::vector<signed char> veccVar;
    auto& rCVar = cl.DefineSubOption("cvar", veccVar, "vector of char variable", true, 0, 1);
    std::vector<short> vecsVar;
    auto& rSVar = cl.DefineSubOption("svar", vecsVar, "vector of short variable", true, 1, 2);
    std::vector<long> veclVar;
    auto& rLVar = cl.DefineSubOption("lvar", veclVar, "vector of long variable", true, 2, 3);
    std::vector<int> veciVar;
    auto& rIVar = cl.DefineSubOption("ivar", veciVar, "vector of int variable", true, 3, 4);
    std::vector<long long> vecllVar;
    auto& rLLVar = cl.DefineSubOption("llvar", vecllVar, "vector of long long variable", true, 4, 5);
    std::vector<unsigned char> vecucVar;
    auto& rUCVar = cl.DefineSubOption("ucvar", vecucVar, "vector of unsigned char variable", true, 5, 6);
    std::vector<unsigned short> vecusVar;
    auto& rUSVar = cl.DefineSubOption("usvar", vecusVar, "vector of unsigned short variable", true, 6, 7);
    std::vector<unsigned long> veculVar;
    auto& rULVar = cl.DefineSubOption("ulvar", veculVar, "vector of unsigned long variable", true, 7, 8);
    std::vector<unsigned int> vecuiVar;
    auto& rUIVar = cl.DefineSubOption("uivar", vecuiVar, "vector of unsigned int variable", true, 8, 9);
    std::vector<unsigned long long> vecullVar;
    auto& rULLVar = cl.DefineSubOption("ullvar", vecullVar, "vector of unsigned long long variable", true, 9, 10);
    std::vector<size_t> vecnVar;
    auto& rNVar = cl.DefineSubOption("nvar", vecnVar, "list of size_t variable", true, 10, 11);

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(4));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(5));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(6));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(7));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(8));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(9));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(10));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(10));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(11));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(11));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(11));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(12));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(12));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralIndependentSequence)
{
    // Attention: sdv::sequence<int8_t> causes compiler problems with MSVC and GCC.
    CCommandLine cl;
    //sdv::sequence<int8_t> seqi8Var;
    //auto& rI8Var = cl.DefineSubOption("i8var", seqi8Var, "sequence of int8_t variable", true, 0, 1);
    sdv::sequence<int16_t> seqi16Var;
    auto& rI16Var = cl.DefineSubOption("i16var", seqi16Var, "sequence of int16_t variable", true, 1, 2);
    sdv::sequence<int32_t> seqi32Var;
    auto& rI32Var = cl.DefineSubOption("i32var", seqi32Var, "sequence of int32_t variable", true, 2, 3);
    sdv::sequence<int64_t> seqi64Var;
    auto& rI64Var = cl.DefineSubOption("i64var", seqi64Var, "sequence of int64_t variable", true, 3, 4);
    sdv::sequence<uint8_t> sequi8Var;
    auto& rUI8Var = cl.DefineSubOption("ui8var", sequi8Var, "sequence of uint8_t variable", true, 4, 5);
    sdv::sequence<uint16_t> sequi16Var;
    auto& rUI16Var = cl.DefineSubOption("ui16var", sequi16Var, "sequence of uint16_t variable", true, 5, 6);
    sdv::sequence<uint32_t> sequi32Var;
    auto& rUI32Var = cl.DefineSubOption("ui32var", sequi32Var, "sequence of uint32_t variable", true, 6, 7);
    sdv::sequence<uint64_t> sequi64Var;
    auto& rUI64Var = cl.DefineSubOption("ui64var", sequi64Var, "sequence of uint64_t variable", true, 7, 8);

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(0));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(1));
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(1));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(2));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(2));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(3));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(4));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(5));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(6));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(7));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(8));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(9));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralDedependentSequence)
{
    CCommandLine cl;
    sdv::sequence<signed char> seqcVar;
    auto& rCVar = cl.DefineSubOption("cvar", seqcVar, "sequence of char variable", true, 0, 1);
    sdv::sequence<short> seqsVar;
    auto& rSVar = cl.DefineSubOption("svar", seqsVar, "sequence of short variable", true, 1, 2);
    sdv::sequence<long> seqlVar;
    auto& rLVar = cl.DefineSubOption("lvar", seqlVar, "sequence of long variable", true, 2, 3);
    sdv::sequence<int> seqiVar;
    auto& rIVar = cl.DefineSubOption("ivar", seqiVar, "sequence of int variable", true, 3, 4);
    sdv::sequence<long long> seqllVar;
    auto& rLLVar = cl.DefineSubOption("llvar", seqllVar, "sequence of long long variable", true, 4, 5);
    sdv::sequence<unsigned char> sequcVar;
    auto& rUCVar = cl.DefineSubOption("ucvar", sequcVar, "sequence of unsigned char variable", true, 5, 6);
    sdv::sequence<unsigned short> sequsVar;
    auto& rUSVar = cl.DefineSubOption("usvar", sequsVar, "sequence of unsigned short variable", true, 6, 7);
    sdv::sequence<unsigned long> sequlVar;
    auto& rULVar = cl.DefineSubOption("ulvar", sequlVar, "sequence of unsigned long variable", true, 7, 8);
    sdv::sequence<unsigned int> sequiVar;
    auto& rUIVar = cl.DefineSubOption("uivar", sequiVar, "sequence of unsigned int variable", true, 8, 9);
    sdv::sequence<unsigned long long> sequllVar;
    auto& rULLVar = cl.DefineSubOption("ullvar", sequllVar, "sequence of unsigned long long variable", true, 9, 10);
    sdv::sequence<size_t> seqnVar;
    auto& rNVar = cl.DefineSubOption("nvar", seqnVar, "list of size_t variable", true, 10, 11);

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(4));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(5));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(6));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(7));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(8));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(9));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(10));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(10));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(11));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(11));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(11));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(12));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(12));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralIndependentList)
{
    // Attention: std::list<int8_t> causes compiler problems with MSVC and GCC.
    CCommandLine cl;
    //std::list<int8_t> lsti8Var;
    //auto& rI8Var = cl.DefineSubOption("i8var", lsti8Var, "list of int8_t variable", true, 0, 1);
    std::list<int16_t> lsti16Var;
    auto& rI16Var = cl.DefineSubOption("i16var", lsti16Var, "list of int16_t variable", true, 1, 2);
    std::list<int32_t> lsti32Var;
    auto& rI32Var = cl.DefineSubOption("i32var", lsti32Var, "list of int32_t variable", true, 2, 3);
    std::list<int64_t> lsti64Var;
    auto& rI64Var = cl.DefineSubOption("i64var", lsti64Var, "list of int64_t variable", true, 3, 4);
    std::list<uint8_t> lstui8Var;
    auto& rUI8Var = cl.DefineSubOption("ui8var", lstui8Var, "list of uint8_t variable", true, 4, 5);
    std::list<uint16_t> lstui16Var;
    auto& rUI16Var = cl.DefineSubOption("ui16var", lstui16Var, "list of uint16_t variable", true, 5, 6);
    std::list<uint32_t> lstui32Var;
    auto& rUI32Var = cl.DefineSubOption("ui32var", lstui32Var, "list of uint32_t variable", true, 6, 7);
    std::list<uint64_t> lstui64Var;
    auto& rUI64Var = cl.DefineSubOption("ui64var", lstui64Var, "list of uint64_t variable", true, 7, 8);

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(0));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(1));
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(1));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rI16Var.PartOfArgumentGroup(2));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(2));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI32Var.PartOfArgumentGroup(3));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(3));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rI64Var.PartOfArgumentGroup(4));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(4));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI8Var.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(5));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI16Var.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(6));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI32Var.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(7));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUI64Var.PartOfArgumentGroup(8));

    //EXPECT_TRUE(rI8Var.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rI64Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI8Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI16Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI32Var.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUI64Var.PartOfArgumentGroup(9));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseIntegralDedependentList)
{
    CCommandLine cl;
    std::list<signed char> lstcVar;
    auto& rCVar = cl.DefineSubOption("cvar", lstcVar, "list of char variable", true, 0, 1);
    std::list<short> lstsVar;
    auto& rSVar = cl.DefineSubOption("svar", lstsVar, "list of short variable", true, 1, 2);
    std::list<long> lstlVar;
    auto& rLVar = cl.DefineSubOption("lvar", lstlVar, "list of long variable", true, 2, 3);
    std::list<int> lstiVar;
    auto& rIVar = cl.DefineSubOption("ivar", lstiVar, "list of int variable", true, 3, 4);
    std::list<long long> lstllVar;
    auto& rLLVar = cl.DefineSubOption("llvar", lstllVar, "list of long long variable", true, 4, 5);
    std::list<unsigned char> lstucVar;
    auto& rUCVar = cl.DefineSubOption("ucvar", lstucVar, "list of unsigned char variable", true, 5, 6);
    std::list<unsigned short> lstusVar;
    auto& rUSVar = cl.DefineSubOption("usvar", lstusVar, "list of unsigned short variable", true, 6, 7);
    std::list<unsigned long> lstulVar;
    auto& rULVar = cl.DefineSubOption("ulvar", lstulVar, "list of unsigned long variable", true, 7, 8);
    std::list<unsigned int> lstuiVar;
    auto& rUIVar = cl.DefineSubOption("uivar", lstuiVar, "list of unsigned int variable", true, 8, 9);
    std::list<unsigned long long> lstullVar;
    auto& rULLVar = cl.DefineSubOption("ullvar", lstullVar, "list of unsigned long long variable", true, 9, 10);
    std::list<size_t> lstnVar;
    auto& rNVar = cl.DefineSubOption("nvar", lstnVar, "list of size_t variable", true, 10, 11);

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(2));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(3));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rIVar.PartOfArgumentGroup(4));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(4));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(5));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rLLVar.PartOfArgumentGroup(5));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(5));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(5));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(6));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUCVar.PartOfArgumentGroup(6));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(6));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(6));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(7));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rUSVar.PartOfArgumentGroup(7));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(7));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(7));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(8));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rULVar.PartOfArgumentGroup(8));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(8));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(8));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(9));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rUIVar.PartOfArgumentGroup(9));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(9));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(9));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(10));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(10));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rULLVar.PartOfArgumentGroup(10));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(10));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(11));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(11));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(11));
    EXPECT_TRUE(rNVar.PartOfArgumentGroup(11));

    EXPECT_TRUE(rCVar.PartOfArgumentGroup(12));    // Options of group #0 are always included.
    EXPECT_FALSE(rSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rLLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUCVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUSVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rUIVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rULLVar.PartOfArgumentGroup(12));
    EXPECT_FALSE(rNVar.PartOfArgumentGroup(12));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseFloatingPoint)
{
    CCommandLine cl;
    float fVar = 0;
    auto& rFVar = cl.DefineSubOption("fvar", fVar, "float variabl1e", true, 0, 1);
    double dVar = 0;
    auto& rDVar = cl.DefineSubOption("dvar", dVar, "double variable", true, 1, 2);
    long double ldVar = 0;
    auto& rLDVar = cl.DefineSubOption("ldvar", ldVar, "long double variable", true, 2, 3);

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(4));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseFloatingPointVector)
{
    CCommandLine cl;
    std::vector<float> vecfVar;
    auto& rFVar = cl.DefineSubOption("fvar", vecfVar, "vector of float variable", true, 0, 1);
    std::vector<double> vecdVar;
    auto& rDVar = cl.DefineSubOption("dvar", vecdVar, "vector of double variable", true, 1, 2);
    std::vector<long double> vecldVar;
    auto& rLDVar = cl.DefineSubOption("ldvar", vecldVar, "vector of long double variable", true, 2, 3);

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(4));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseFloatingPointSequence)
{
    CCommandLine cl;
    sdv::sequence<float> seqfVar;
    auto& rFVar = cl.DefineSubOption("fvar", seqfVar, "sequence of float variable", true, 0, 1);
    sdv::sequence<double> seqdVar;
    auto& rDVar = cl.DefineSubOption("dvar", seqdVar, "sequence of double variable", true, 1, 2);
    sdv::sequence<long double> seqldVar;
    auto& rLDVar = cl.DefineSubOption("ldvar", seqldVar, "sequence of long double variable", true, 2, 3);

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(4));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseFloatingPointList)
{
    CCommandLine cl;
    std::list<float> lstfVar;
    auto& rFVar = cl.DefineSubOption("fvar", lstfVar, "list of float variable", true, 0, 1);
    std::list<double> lstdVar;
    auto& rDVar = cl.DefineSubOption("dvar", lstdVar, "list of double variable", true, 1, 2);
    std::list<long double> lstldVar;
    auto& rLDVar = cl.DefineSubOption("ldvar", lstldVar, "list of long double variable", true, 2, 3);

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(1));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rDVar.PartOfArgumentGroup(2));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(3));
    EXPECT_TRUE(rLDVar.PartOfArgumentGroup(3));

    EXPECT_TRUE(rFVar.PartOfArgumentGroup(4));    // Options of group #0 are always included.
    EXPECT_FALSE(rDVar.PartOfArgumentGroup(4));
    EXPECT_FALSE(rLDVar.PartOfArgumentGroup(4));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseEnum)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
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
    EUnscopedTest eUnscopedTest = test1;
    auto& rEUVar = cl.DefineSubOption("unscoped_enum", eUnscopedTest, "unscoped enum variable", true, 0, 1);
    rEUVar.AddAssociations(rgsUnscopedEnumAssociations);
    EScopedTest eScopedTest = EScopedTest::test4;
    auto& rESVar = cl.DefineSubOption("scoped_enum", eScopedTest, "scoped enum variable", true, 1, 2);
    rESVar.AddAssociations(rgsScopedEnumAssociations);

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseEnumVector)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
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
    std::vector<EUnscopedTest> vecUnscopedTest;
    auto& rEUVar = cl.DefineSubOption("unscoped_enum", vecUnscopedTest, "unscoped enum variable", true, 0, 1);
    rEUVar.AddAssociations(rgsUnscopedEnumAssociations);
    std::vector<EScopedTest> vecScopedTest;
    auto& rESVar = cl.DefineSubOption("scoped_enum", vecScopedTest, "scoped enum variable", true, 1, 2);
    rESVar.AddAssociations(rgsScopedEnumAssociations);

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseEnumSequence)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
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
    sdv::sequence<EUnscopedTest> seqUnscopedTest;
    auto& rEUVar = cl.DefineSubOption("unscoped_enum", seqUnscopedTest, "unscoped enum variable", true, 0, 1);
    rEUVar.AddAssociations(rgsUnscopedEnumAssociations);
    sdv::sequence<EScopedTest> seqScopedTest;
    auto& rESVar = cl.DefineSubOption("scoped_enum", seqScopedTest, "scoped enum variable", true, 1, 2);
    rESVar.AddAssociations(rgsScopedEnumAssociations);

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseEnumList)
{
    enum EUnscopedTest { test1, test2, test3 };
    enum class EScopedTest : size_t {test4, test5, test6};
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
    std::list<EUnscopedTest> lstUnscopedTest;
    auto& rEUVar = cl.DefineSubOption("unscoped_enum", lstUnscopedTest, "unscoped enum variable", true, 0, 1);
    rEUVar.AddAssociations(rgsUnscopedEnumAssociations);
    std::list<EScopedTest> lstScopedTest;
    auto& rESVar = cl.DefineSubOption("scoped_enum", lstScopedTest, "scoped enum variable", true, 1, 2);
    rESVar.AddAssociations(rgsScopedEnumAssociations);

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rESVar.PartOfArgumentGroup(2));

    EXPECT_TRUE(rEUVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rESVar.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseString)
{
    CCommandLine cl;
    std::string ssVar1;
    auto& rSSVar = cl.DefineSubOption("ssvar1", ssVar1, "std::string variable", true, 0, 1);
    sdv::u8string ssu8Var1;
    auto& rSSU8Var = cl.DefineSubOption("ssu8var1", ssu8Var1, "sdv::u8string variable", true, 1, 2);

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(0));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(1));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(2));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseStringVector)
{
    CCommandLine cl;
    std::vector<std::string> vecssVar1;
    auto& rSSVar = cl.DefineSubOption("ssvar1", vecssVar1, "std::vector<std::string> variable", true, 0, 1);
    std::vector<sdv::u8string> vecssu8Var1;
    auto& rSSU8Var = cl.DefineSubOption("ssu8var1", vecssu8Var1, "std::vector<sdv::u8string> variable", true, 1, 2);

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(0));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(1));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(2));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseStringSequence)
{
    CCommandLine cl;
    sdv::sequence<std::string> seqssVar1;
    auto& rSSVar = cl.DefineSubOption("ssvar1", seqssVar1, "sdv::sequence<std::string> variable", true, 0, 1);
    sdv::sequence<sdv::u8string> seqssu8Var1;
    auto& rSSU8Var = cl.DefineSubOption("ssu8var1", seqssu8Var1, "sdv::sequence<sdv::u8string> variable", true, 1, 2);

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(0));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(1));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(2));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseStringList)
{
    CCommandLine cl;
    std::list<std::string> lstssVar1;
    auto& rSSVar = cl.DefineSubOption("ssvar1", lstssVar1, "std::list<std::string> variable", true, 0, 1);
    std::list<sdv::u8string> lstssu8Var1;
    auto& rSSU8Var = cl.DefineSubOption("ssu8var1", lstssu8Var1, "std::list<sdv::u8string> variable", true, 1, 2);

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(0));
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(0));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(1));
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(1));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
    EXPECT_TRUE(rSSU8Var.PartOfArgumentGroup(2));

    EXPECT_TRUE(rSSVar.PartOfArgumentGroup(3));    // Options of group #0 are always included.
    EXPECT_FALSE(rSSU8Var.PartOfArgumentGroup(3));
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParsePath)
{
    CCommandLine cl;
    std::filesystem::path pathVar1;
    auto& rPATHVar = cl.DefineSubOption("pathvar1", pathVar1, "std::filesystem::path variable", true, 0, 1);

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParsePathVector)
{
    CCommandLine cl;
    std::vector<std::filesystem::path> vecpathVar1;
    auto& rPATHVar = cl.DefineSubOption("pathvar1", vecpathVar1, "std::vector<std::filesystem::path> variable", true, 0, 1);

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParsePathSequence)
{
    CCommandLine cl;
    sdv::sequence<std::filesystem::path> seqpathVar1;
    auto& rPATHVar = cl.DefineSubOption("pathvar1", seqpathVar1, "sdv::sequence<std::filesystem::path> variable", true, 0, 1);

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParsePathList)
{
    CCommandLine cl;
    std::list<std::filesystem::path> lstpathVar1;
    auto& rPATHVar = cl.DefineSubOption("pathvar1", lstpathVar1, "std::list<std::filesystem::path> variable", true, 0, 1);

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rPATHVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
}

TEST_F(CCommandLineParserTestArgumentSelection, SubOptionParseFlags)
{
    CCommandLine cl;
    bool bFlag = false;
    auto& rFLAGVar = cl.DefineFlagSubOption("flag", bFlag, "flag variable", true, 0, 1);

    EXPECT_TRUE(rFLAGVar.PartOfArgumentGroup(0));

    EXPECT_TRUE(rFLAGVar.PartOfArgumentGroup(1));

    EXPECT_TRUE(rFLAGVar.PartOfArgumentGroup(2));    // Options of group #0 are always included.
}
