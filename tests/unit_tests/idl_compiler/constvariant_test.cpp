#include "includes.h"
#include "lexer_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/constvariant.cpp"

using CConstVariantTest = CLexerTest;

TEST_F(CConstVariantTest, DefaultConstruction)
{
    CConstVariant var;
    EXPECT_EQ(var.Get<int>(), 0);
}

TEST_F(CConstVariantTest, LiteralDirectValueConstruction)
{
    EXPECT_EQ(CConstVariant(5).Get<int>(), 5);
    EXPECT_EQ(CConstVariant(10u).Get<unsigned int>(), 10u);
    EXPECT_EQ(CConstVariant(20l).Get<int32_t>(), static_cast<int32_t>(20l));
    EXPECT_EQ(CConstVariant(static_cast<uint32_t>(30ul)).Get<uint32_t>(), 30ul);
    EXPECT_EQ(CConstVariant(40ll).Get<int64_t>(), 40ll);
    EXPECT_EQ(CConstVariant(50ull).Get<uint64_t>(), 50ull);
    EXPECT_EQ(CConstVariant('A').Get<char>(), 'A');
    EXPECT_EQ(CConstVariant(L'B').Get<wchar_t>(), L'B');
    EXPECT_EQ(CConstVariant(u8'C').Get<char>(), u8'C');
    EXPECT_EQ(CConstVariant(u'D').Get<char16_t>(), u'D');
    EXPECT_EQ(CConstVariant(U'E').Get<char32_t>(), U'E');
    EXPECT_EQ(CConstVariant(true).Get<bool>(), true);
    EXPECT_EQ(CConstVariant(static_cast<std::string>("string")).Get<std::string>(), "string");
    EXPECT_EQ(CConstVariant(static_cast<std::string>(u8"string")).Get<std::string>(), u8"string");
    EXPECT_EQ(CConstVariant(static_cast<std::u16string>(u"string")).Get<std::u16string>(), u"string");
    EXPECT_EQ(CConstVariant(static_cast<std::u32string>(U"string")).Get<std::u32string>(), U"string");
    EXPECT_EQ(CConstVariant(static_cast<std::wstring>(L"string")).Get<std::wstring>(), L"string");
}

TEST_F(CConstVariantTest, LiteralIndirectValueConstruction)
{
    bool b = true;                          EXPECT_EQ(CConstVariant(b).Get<bool>(), true);
    int8_t i8 = 10;                         EXPECT_EQ(CConstVariant(i8).Get<int8_t>(), 10);
    int16_t i16 = 20;                       EXPECT_EQ(CConstVariant(i16).Get<int16_t>(), 20);
    int32_t i32 = 30;                       EXPECT_EQ(CConstVariant(i32).Get<int32_t>(), 30);
    int64_t i64 = 40;                       EXPECT_EQ(CConstVariant(i64).Get<int64_t>(), 40);
    uint8_t ui8 = 50;                       EXPECT_EQ(CConstVariant(ui8).Get<uint8_t>(), 50);
    uint16_t ui16 = 60;                     EXPECT_EQ(CConstVariant(ui16).Get<uint16_t>(), 60u);
    uint32_t ui32 = 70;                     EXPECT_EQ(CConstVariant(ui32).Get<uint32_t>(), 70u);
    uint64_t ui64 = 80;                     EXPECT_EQ(CConstVariant(ui64).Get<uint64_t>(), 80ull);
    float f = 90.0;                         EXPECT_EQ(CConstVariant(f).Get<float>(), 90.0);
    double d = 100.0;                       EXPECT_EQ(CConstVariant(d).Get<double>(), 100.0);
    long double ld = 110.0;                 EXPECT_EQ(CConstVariant(ld).Get<long double>(), 110.0);
    std::string ss = "string";              EXPECT_EQ(CConstVariant(ss).Get<std::string>(), "string");
    std::string ssUtf8 = u8"string";        EXPECT_EQ(CConstVariant(ssUtf8).Get<std::string>(), u8"string");
    std::u16string ssUtf16 = u"string";     EXPECT_EQ(CConstVariant(ssUtf16).Get<std::u16string>(), u"string");
    std::u32string ssUtf32 = U"string";     EXPECT_EQ(CConstVariant(ssUtf32).Get<std::u32string>(), U"string");
    std::wstring ssWide = L"string";        EXPECT_EQ(CConstVariant(ssWide).Get<std::wstring>(), L"string");
}

TEST_F(CConstVariantTest, LiteralDirectValueAssignment)
{
    CConstVariant var;
    var = 5;                                        EXPECT_EQ(var.Get<int>(), 5);
    var = 10u;                                      EXPECT_EQ(var.Get<unsigned int>(), 10u);
    var = 20l;                                      EXPECT_EQ(var.Get<int32_t>(), 20l);
    var = static_cast<uint32_t>(30ul);              EXPECT_EQ(var.Get<uint32_t>(), 30ul);
    var = 40ll;                                     EXPECT_EQ(var.Get<int64_t>(), 40ll);
    var = 50ull;                                    EXPECT_EQ(var.Get<uint64_t>(), 50ull);
    var = 'A';                                      EXPECT_EQ(var.Get<char>(), 'A');
    var = L'B';                                     EXPECT_EQ(var.Get<wchar_t>(), L'B');
    var = u8'C';                                    EXPECT_EQ(var.Get<char>(), u8'C');
    var = u'D';                                     EXPECT_EQ(var.Get<char16_t>(), u'D');
    var = U'E';                                     EXPECT_EQ(var.Get<char32_t>(), U'E');
    var = true;                                     EXPECT_EQ(var.Get<bool>(), true);
    var = static_cast<std::string>("string");       EXPECT_EQ(var.Get<std::string>(), "string");
    var = static_cast<std::string>(u8"string");     EXPECT_EQ(var.Get<std::string>(), u8"string");
    var = static_cast<std::u16string>(u"string");   EXPECT_EQ(var.Get<std::u16string>(), u"string");
    var = static_cast<std::u32string>(U"string");   EXPECT_EQ(var.Get<std::u32string>(), U"string");
    var = static_cast<std::wstring>(L"string");     EXPECT_EQ(var.Get<std::wstring>(), L"string");
}

TEST_F(CConstVariantTest, LiteralIndirectValueAssignment)
{
    CConstVariant var;
    bool b = true;                          var = b;            EXPECT_EQ(var.Get<bool>(), true);
    int8_t i8 = 10;                         var = i8;           EXPECT_EQ(var.Get<int8_t>(), 10);
    int16_t i16 = 20;                       var = i16;          EXPECT_EQ(var.Get<int16_t>(), 20);
    int32_t i32 = 30;                       var = i32;          EXPECT_EQ(var.Get<int32_t>(), 30);
    int64_t i64 = 40;                       var = i64;          EXPECT_EQ(var.Get<int64_t>(), 40);
    uint8_t ui8 = 50;                       var = ui8;          EXPECT_EQ(var.Get<uint8_t>(), 50);
    uint16_t ui16 = 60;                     var = ui16;         EXPECT_EQ(var.Get<uint16_t>(), 60);
    uint32_t ui32 = 70;                     var = ui32;         EXPECT_EQ(var.Get<uint32_t>(), 70u);
    uint64_t ui64 = 80;                     var = ui64;         EXPECT_EQ(var.Get<uint64_t>(), 80ull);
    float f = 90.0;                         var = f;            EXPECT_EQ(var.Get<float>(), 90.0);
    double d = 100.0;                       var = d;            EXPECT_EQ(var.Get<double>(), 100.0);
    long double ld = 110.0;                 var = ld;           EXPECT_EQ(var.Get<long double>(), 110.0);
    std::string ss = "string";              var = ss;           EXPECT_EQ(var.Get<std::string>(), "string");
    std::string ssUtf8 = u8"string";        var = ssUtf8;       EXPECT_EQ(var.Get<std::string>(), u8"string");
    std::u16string ssUtf16 = u"string";     var = ssUtf16;      EXPECT_EQ(var.Get<std::u16string>(), u"string");
    std::u32string ssUtf32 = U"string";     var = ssUtf32;      EXPECT_EQ(var.Get<std::u32string>(), U"string");
    std::wstring ssWide = L"string";        var = ssWide;       EXPECT_EQ(var.Get<std::wstring>(), L"string");
}

TEST_F(CConstVariantTest, TypePromotion)
{
    CConstVariant var;
    bool b = true;
    var = b;
    EXPECT_EQ(var.Get<int8_t>(), 1);
    EXPECT_EQ(var.Get<uint8_t>(), 1);
    EXPECT_EQ(var.Get<int16_t>(), 1);
    EXPECT_EQ(var.Get<uint16_t>(), 1u);
    EXPECT_EQ(var.Get<int32_t>(), 1);
    EXPECT_EQ(var.Get<uint32_t>(), 1u);
    EXPECT_EQ(var.Get<int64_t>(), 1);
    EXPECT_EQ(var.Get<uint64_t>(), 1ull);
    EXPECT_EQ(var.Get<float>(), 1.0f);
    EXPECT_EQ(var.Get<double>(), 1.0);
    EXPECT_EQ(var.Get<long double>(), 1.0l);

    int8_t i8 = -10;
    var = i8;
    EXPECT_EQ(var.Get<uint8_t>(), 0xf6);
    EXPECT_EQ(var.Get<int16_t>(), -10);
    EXPECT_EQ(var.Get<uint16_t>(), 0xfff6);
    EXPECT_EQ(var.Get<int32_t>(), -10);
    EXPECT_EQ(var.Get<uint32_t>(), 0xfffffff6ul);
    EXPECT_EQ(var.Get<int64_t>(), -10);
    EXPECT_EQ(var.Get<uint64_t>(), 0xfffffffffffffff6ull);
    EXPECT_EQ(var.Get<float>(), -10.0f);
    EXPECT_EQ(var.Get<double>(), -10.0);
    EXPECT_EQ(var.Get<long double>(), -10.0l);

    uint8_t ui8 = 50;
    var = ui8;
    EXPECT_EQ(var.Get<int16_t>(), 50);
    EXPECT_EQ(var.Get<uint16_t>(), 50u);
    EXPECT_EQ(var.Get<int32_t>(), 50);
    EXPECT_EQ(var.Get<uint32_t>(), 50u);
    EXPECT_EQ(var.Get<int64_t>(), 50);
    EXPECT_EQ(var.Get<uint64_t>(), 50ull);
    EXPECT_EQ(var.Get<float>(), 50.0f);
    EXPECT_EQ(var.Get<double>(), 50.0);
    EXPECT_EQ(var.Get<long double>(), 50.0l);

    int16_t i16 = -20;
    var = i16;
    EXPECT_EQ(var.Get<uint16_t>(), 0xffec);
    EXPECT_EQ(var.Get<int32_t>(), -20);
    EXPECT_EQ(var.Get<uint32_t>(), 0xffffffecul);
    EXPECT_EQ(var.Get<int64_t>(), -20);
    EXPECT_EQ(var.Get<uint64_t>(), 0xffffffffffffffecull);
    EXPECT_EQ(var.Get<float>(), -20.0f);
    EXPECT_EQ(var.Get<double>(), -20.0);
    EXPECT_EQ(var.Get<long double>(), -20.0l);

    uint16_t ui16 = 60;
    var = ui16;
    EXPECT_EQ(var.Get<int32_t>(), 60);
    EXPECT_EQ(var.Get<uint32_t>(), 60u);
    EXPECT_EQ(var.Get<int64_t>(), 60);
    EXPECT_EQ(var.Get<uint64_t>(), 60ull);
    EXPECT_EQ(var.Get<float>(), 60.0f);
    EXPECT_EQ(var.Get<double>(), 60.0);
    EXPECT_EQ(var.Get<long double>(), 60.0l);

    int32_t i32 = -30;
    var = i32;
    EXPECT_EQ(var.Get<uint32_t>(), 0xffffffe2ul);
    EXPECT_EQ(var.Get<int64_t>(), -30);
    EXPECT_EQ(var.Get<uint64_t>(), 0xffffffffffffffe2ull);
    EXPECT_EQ(var.Get<float>(), -30.0f);
    EXPECT_EQ(var.Get<double>(), -30.0);
    EXPECT_EQ(var.Get<long double>(), -30.0l);

    uint32_t ui32 = 70;
    var = ui32;
    EXPECT_EQ(var.Get<int64_t>(), 70);
    EXPECT_EQ(var.Get<uint64_t>(), 70ull);
    EXPECT_EQ(var.Get<float>(), 70.0f);
    EXPECT_EQ(var.Get<double>(), 70.0);
    EXPECT_EQ(var.Get<long double>(), 70.0l);

    int64_t i64 = -40;
    var = i64;
    EXPECT_EQ(var.Get<uint64_t>(), 0xffffffffffffffd8ull);
    EXPECT_EQ(var.Get<float>(), -40.0f);
    EXPECT_EQ(var.Get<double>(), -40.0);
    EXPECT_EQ(var.Get<long double>(), -40.0l);

    uint64_t ui64 = 80;
    var = ui64;
    EXPECT_EQ(var.Get<float>(), 80.0f);
    EXPECT_EQ(var.Get<double>(), 80.0);
    EXPECT_EQ(var.Get<long double>(), 80.0l);

    float f = -90.0;
    var = f;
    EXPECT_EQ(var.Get<double>(), -90.0);
    EXPECT_EQ(var.Get<long double>(), -90.0l);

    double d = -100.0;
    var = d;
    EXPECT_EQ(var.Get<long double>(), -100.0l);
}

TEST_F(CConstVariantTest, TypeDemotion)
{
    CConstVariant var;

    int8_t i8 = 1;
    var = i8;
    EXPECT_EQ(var.Get<bool>(), true);
    var = std::numeric_limits<int8_t>::min();
    EXPECT_EQ(var.Get<bool>(), true);
    var = std::numeric_limits<int8_t>::max();
    EXPECT_EQ(var.Get<bool>(), true);

    uint8_t ui8 = 50;
    var = ui8;
    EXPECT_EQ(var.Get<int8_t>(), 50);
    EXPECT_EQ(var.Get<bool>(), true);
    var = std::numeric_limits<uint8_t>::max();
    EXPECT_THROW(var.Get<int8_t>(), CCompileException);

    int16_t i16 = -20;
    var = i16;
    EXPECT_EQ(var.Get<uint8_t>(), 0xec);
    EXPECT_EQ(var.Get<int8_t>(), -20);
    var = std::numeric_limits<int16_t>::min();
    EXPECT_THROW(var.Get<uint8_t>(), CCompileException);
    var = std::numeric_limits<int16_t>::max();
    EXPECT_THROW(var.Get<uint8_t>(), CCompileException);

    uint16_t ui16 = 60;
    var = ui16;
    EXPECT_EQ(var.Get<int16_t>(), 60);
    EXPECT_EQ(var.Get<int8_t>(), 60);
    var = std::numeric_limits<uint16_t>::max();
    EXPECT_THROW(var.Get<int16_t>(), CCompileException);

    int32_t i32 = -30;
    var = i32;
    EXPECT_EQ(var.Get<uint16_t>(), 0xffe2);
    EXPECT_EQ(var.Get<int16_t>(), -30);
    var = std::numeric_limits<int32_t>::min();
    EXPECT_THROW(var.Get<uint16_t>(), CCompileException);
    var = std::numeric_limits<int32_t>::max();
    EXPECT_THROW(var.Get<uint16_t>(), CCompileException);

    uint32_t ui32 = 70;
    var = ui32;
    EXPECT_EQ(var.Get<int32_t>(), 70);
    EXPECT_EQ(var.Get<int16_t>(), 70);
    var = std::numeric_limits<uint32_t>::max();
    EXPECT_THROW(var.Get<int32_t>(), CCompileException);

    int64_t i64 = -40;
    var = i64;
    EXPECT_EQ(var.Get<uint32_t>(), 0xffffffd8);
    EXPECT_EQ(var.Get<int32_t>(), -40);
    var = std::numeric_limits<int64_t>::min();
    EXPECT_THROW(var.Get<uint32_t>(), CCompileException);
    var = std::numeric_limits<int64_t>::max();
    EXPECT_THROW(var.Get<uint32_t>(), CCompileException);

    uint64_t ui64 = 80;
    var = ui64;
    EXPECT_EQ(var.Get<int64_t>(), 80);
    EXPECT_EQ(var.Get<int32_t>(), 80);
    var = std::numeric_limits<uint64_t>::max();
    EXPECT_THROW(var.Get<int64_t>(), CCompileException);

    float f = 90.0;
    var = f;
    EXPECT_THROW(var.Get<int64_t>(), CCompileException);
    EXPECT_THROW(var.Get<uint64_t>(), CCompileException);

    double d = 100.0;
    var = d;
    EXPECT_EQ(var.Get<float>(), 100.0f);
    var = std::numeric_limits<double>::max();
    EXPECT_THROW(var.Get<float>(), CCompileException);
    var = std::numeric_limits<double>::min();
    EXPECT_THROW(var.Get<float>(), CCompileException);
    var = std::numeric_limits<double>::lowest();
    EXPECT_THROW(var.Get<float>(), CCompileException);

    if (std::numeric_limits<double>::max() < std::numeric_limits<long double>::max())
    {
        long double ld = 110.0;
        var = ld;
        EXPECT_EQ(var.Get<double>(), 110.0);
        var = std::numeric_limits<long double>::max();
        EXPECT_THROW(var.Get<double>(), CCompileException);
        var = std::numeric_limits<long double>::min();
        EXPECT_THROW(var.Get<double>(), CCompileException);
        var = std::numeric_limits<long double>::lowest();
        EXPECT_THROW(var.Get<double>(), CCompileException);
    }
}

TEST_F(CConstVariantTest, InvalidConversion)
{
    EXPECT_THROW(CConstVariant(90).Get<std::string>(), CCompileException);
    EXPECT_THROW(CConstVariant(90.0f).Get<uint32_t>(), CCompileException);
    EXPECT_THROW(CConstVariant(100.0).Get<char>(), CCompileException);
    EXPECT_THROW(CConstVariant(110.0l).Get<std::wstring>(), CCompileException);
    EXPECT_THROW(CConstVariant("string").Get<std::u16string>(), CCompileException);
    EXPECT_THROW(CConstVariant(u8"string").Get<std::wstring>(), CCompileException);
    EXPECT_THROW(CConstVariant(u"string").Get<std::u32string>(), CCompileException);
    EXPECT_THROW(CConstVariant(U"string").Get<std::string>(), CCompileException);
    EXPECT_THROW(CConstVariant(L"string").Get<std::u16string>(), CCompileException);
}

TEST_F(CConstVariantTest, ArithmicOperatorLogicalNot)
{
    CConstVariant var1(true);
    CConstVariant var2 = !var1;
    EXPECT_EQ(var2.Get<uint32_t>(), 0u);

    var1 = 100.0;
    EXPECT_THROW(var2 = !var1, CCompileException);

    var1 = 100;
    var2 = !var1;
    EXPECT_EQ(var2.Get<double>(), 0);
}

TEST_F(CConstVariantTest, ArithmicOperatorBitwiseNot)
{
    CConstVariant var1(static_cast<uint16_t>(0b1011100010ull));
    CConstVariant var2 = ~var1;
    EXPECT_EQ(var2.Get<uint16_t>(), 0b1111110100011101ull);

    var1 = 100.0;
    EXPECT_THROW(var2 = ~var1, CCompileException);

    var1 = false;
    EXPECT_THROW(var2 = ~var1, CCompileException);

    var1 = static_cast<uint8_t>(0x0f);
    var2 = ~var1;
    EXPECT_EQ(var2.Get<uint16_t>(), 0xfff0);
}

TEST_F(CConstVariantTest, ArithmicOperatorConfirmation)
{
    CConstVariant var1(true);
    CConstVariant var2 = +var1;
    EXPECT_EQ(var2.Get<uint32_t>(), 1u);

    var1 = 100.0;
    var2 = +var1;
    EXPECT_EQ(var2.Get<float>(), 100.0f);

    var1 = 100;
    var2 = +var1;
    EXPECT_EQ(var2.Get<int8_t>(), 100);
}

TEST_F(CConstVariantTest, ArithmicOperatorNegetion)
{
    CConstVariant var1(static_cast<uint8_t>('A'));
    CConstVariant var2 = -var1;
    EXPECT_EQ(var2.Get<int32_t>(), -65);

    var1 = true;
    EXPECT_THROW(var2 = -var1, CCompileException);

    var1 = 100.0;
    var2 = -var1;
    EXPECT_EQ(var2.Get<float>(), -100.0f);

    var1 = 100;
    var2 = -var1;
    EXPECT_EQ(var2.Get<int8_t>(), -100);
}

TEST_F(CConstVariantTest, ArithmicOperatorMultiplication)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 * var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 4050u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var1 * var2;
    EXPECT_EQ(var3.Get<double>(), 4500);

    var1 = 100.0;
    var2 = 'A';
    var3 = var1 * var2;
    EXPECT_EQ(var3.Get<double>(), 6500);
}

TEST_F(CConstVariantTest, ArithmicOperatorDivision)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 / var2;
    EXPECT_EQ(var3.Get<double>(), 2);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var1 / var2;
    EXPECT_EQ(var3.Get<double>(), 100.0/45.0);

    var1 = 130;
    var2 = 'A';
    var3 = var1 / var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 2u);

    EXPECT_THROW(var3 / 0, CCompileException);
}

TEST_F(CConstVariantTest, ArithmicOperatorAddition)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 + var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 135u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var1 + var2;
    EXPECT_EQ(var3.Get<double>(), 145);

    var1 = 100.0;
    var2 = 'A';
    var3 = var1 + var2;
    EXPECT_EQ(var3.Get<double>(), 165);
}

TEST_F(CConstVariantTest, ArithmicOperatorSubtraction)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 - var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 45u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var1 - var2;
    EXPECT_EQ(var3.Get<double>(), 55);

    var1 = 0.0;
    var2 = 'A';
    var3 = var1 - var2;
    EXPECT_EQ(var3.Get<double>(), -65);
}

TEST_F(CConstVariantTest, ArithmicOperatorRemainder)
{
    CConstVariant var1(90);
    CConstVariant var2(40);
    CConstVariant var3 = var1 % var2;
    EXPECT_EQ(var3.Get<double>(), 10);

    var1 = 100.0;
    var2 = 45.0;
    EXPECT_THROW(var1 % var2, CCompileException);

    var1 = 130;
    var2 = 'A';
    var3 = var1 % var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 0u);

    EXPECT_THROW(var3 % 0, CCompileException);
}

TEST_F(CConstVariantTest, ArithmicOperatorShiftLeft)
{
    CConstVariant var1(90);
    CConstVariant var2(3);
    CConstVariant var3 = var1 << var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 720u);

    var1 = 100.0;
    var2 = 2.0;
    EXPECT_THROW(var1 << var2, CCompileException);

    var1 = static_cast<int8_t>('A');
    var2 = static_cast<uint8_t>(2);
    var3 = var1 << var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 260u);
}

TEST_F(CConstVariantTest, ArithmicOperatorShiftRight)
{
    CConstVariant var1(90);
    CConstVariant var2(3);
    CConstVariant var3 = var1 >> var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 11u);

    var1 = 100.0;
    var2 = 2.0;
    EXPECT_THROW(var1 >> var2, CCompileException);

    var1 = 'A';
    var2 = static_cast<uint8_t>(2);
    var3 = var1 >> var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 16u);
}

TEST_F(CConstVariantTest, ArithmicOperatorBitwiseAnd)
{
    CConstVariant var1(0b10101010);
    CConstVariant var2(0b11110000);
    CConstVariant var3 = var1 & var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 0b10100000u);

    var1 = 100.0;
    var2 = 2.0;
    EXPECT_THROW(var1 & var2, CCompileException);

    var1 = 'A';
    var2 = static_cast<uint8_t>(1);
    var3 = var1 & var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);
}

TEST_F(CConstVariantTest, ArithmicOperatorBitwiseXor)
{
    CConstVariant var1(0b10101010);
    CConstVariant var2(0b11110000);
    CConstVariant var3 = var1 ^ var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 0b1011010u);

    var1 = 100.0;
    var2 = 2.0;
    EXPECT_THROW(var1 ^ var2, CCompileException);

    var1 = 'A';
    var2 = static_cast<uint8_t>(1);
    var3 = var1 ^ var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 64u);
}

TEST_F(CConstVariantTest, ArithmicOperatorBitwiseOr)
{
    CConstVariant var1(0b10101010);
    CConstVariant var2(0b11110000);
    CConstVariant var3 = var1 | var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 0b11111010u);

    var1 = 100.0;
    var2 = 2.0;
    EXPECT_THROW(var1 | var2, CCompileException);

    var1 = 'A';
    var2 = static_cast<uint8_t>(2);
    var3 = var1 | var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 67u);
}

TEST_F(CConstVariantTest, ArithmicOperatorLogicalAnd)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 && var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = 100.0;
    CConstVariant var4 = 45.0;
    CConstVariant var5 = var1 == var3 && var2 == var4;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = true;
    var4 = var1 == var2 && var3;
    EXPECT_EQ(var4.Get<bool>(), true);
}

TEST_F(CConstVariantTest, ArithmicOperatorLogicalOr)
{
    CConstVariant var1(90);
    CConstVariant var2(false);
    CConstVariant var3 = var1 || var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = 190.0;
    CConstVariant var4 = 45.0;
    CConstVariant var5 = var1 == var3 || var2 == var4;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = false;
    var4 = var1 == var2 || var3;
    EXPECT_EQ(var4.Get<bool>(), true);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareSmaller)
{
    CConstVariant var1(45);
    CConstVariant var2(90);
    CConstVariant var3 = var1 < var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var2 < var1;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 < var2;
    EXPECT_EQ(var3.Get<bool>(), false);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareSmallerOrEqual)
{
    CConstVariant var1(45);
    CConstVariant var2(90);
    CConstVariant var3 = var1 <= var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 100.0;
    var2 = 45.0;
    var3 = var2 <= var1;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 <= var2;
    EXPECT_EQ(var3.Get<bool>(), true);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareGreater)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 > var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 45.0;
    var2 = 100.0;
    var3 = var2 > var1;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 > var2;
    EXPECT_EQ(var3.Get<bool>(), false);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareGreaterOrEqual)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 >= var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 45.0;
    var2 = 100.0;
    var3 = var2 >= var1;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 >= var2;
    EXPECT_EQ(var3.Get<bool>(), true);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareEqual)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 == var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 0u);

    var1 = 45.0;
    var2 = 100.0;
    var3 = var2 == var1;
    EXPECT_EQ(var3.Get<bool>(), false);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 == var2;
    EXPECT_EQ(var3.Get<bool>(), true);
}

TEST_F(CConstVariantTest, ArithmicOperatorCompareNotEqual)
{
    CConstVariant var1(90);
    CConstVariant var2(45);
    CConstVariant var3 = var1 != var2;
    EXPECT_EQ(var3.Get<uint32_t>(), 1u);

    var1 = 45.0;
    var2 = 100.0;
    var3 = var2 != var1;
    EXPECT_EQ(var3.Get<bool>(), true);

    var1 = 65ull;
    var2 = 'A';
    var3 = var1 != var2;
    EXPECT_EQ(var3.Get<bool>(), false);
}
