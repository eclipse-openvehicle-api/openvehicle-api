/********************************************************************************
 * Copyright (c) 2025-2026 Contributors to the Eclipse Foundation
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
#include <support/param_impl.h>

TEST(ParameterTest, AutoTypeInitializeBoolean)
{
    bool b;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(b, "b", true, "", "", "", 0)->Boolean());
}

TEST(ParameterTest, AutoTypeInitializeNumeric)
{
    // Signed integer
    int8_t i8;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i8, "i8", 1, "", "", "", 0)->Numeric());
    int16_t i16;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i16, "i16", 2, "", "", "", 0)->Numeric());
    int32_t i32;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i32, "i32", 3, "", "", "", 0)->Numeric());
    int64_t i64;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i64, "i64", 4, "", "", "", 0)->Numeric());
    int i;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i, "i", 5, "", "", "", 0)->Numeric());
    long l;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(l, "l", 6, "", "", "", 0)->Numeric());
    long long ll;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ll, "ll", 7, "", "", "", 0)->Numeric());

    // Unsigned integer
    uint8_t ui8;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui8, "ui8", 8, "", "", "", 0)->Numeric());
    uint16_t ui16;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui16, "ui16", 9, "", "", "", 0)->Numeric());
    uint32_t ui32;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui32, "ui32", 10, "", "", "", 0)->Numeric());
    uint32_t ui64;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui64, "ui64", 11, "", "", "", 0)->Numeric());
    unsigned int ui;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui, "ui", 12, "", "", "", 0)->Numeric());
    unsigned long ul;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ul, "ul", 13, "", "", "", 0)->Numeric());
    unsigned long long ull;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ull, "ull", 14, "", "", "", 0)->Numeric());

    // Floating point
    float f;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(f, "f", 15.0, "", "", "", 0)->Numeric());
    double d;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(d, "d", 16.0, "", "", "", 0)->Numeric());
    long double ld;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ld, "ld", 17.0, "", "", "", 0)->Numeric());
}

TEST(ParameterTest, AutoTypeInitializeString)
{
    // STD string object
    std::string ss;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss, "ss", "18", "", "", "", 0)->String());
    std::wstring ssw;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ssw, "ssw", L"19", "", "", "", 0)->String());
    std::u16string ss16;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss16, "ss16", u"20", "", "", "", 0)->String());
    std::u32string ss32;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss32, "ss32", U"21", "", "", "", 0)->String());

    // SDV string object
    sdv::string ss_sdv;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss_sdv, "ss_sdv", "22", "", "", "", 0)->String());
    sdv::u8string ss8_sdv;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss8_sdv, "ss8_sdv", u8"23", "", "", "", 0)->String());
    sdv::wstring ssw_sdv;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ssw_sdv, "ssw_sdv", L"24", "", "", "", 0)->String());
    sdv::u16string ss16_sdv;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss16_sdv, "ss16_sdv", u"25", "", "", "", 0)->String());
    sdv::u32string ss32_sdv;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss32_sdv, "ss32_sdv", U"26", "", "", "", 0)->String());

    // C character array
    char sz[] = "";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(sz, "sz", "27", "", "", "", 0)->String());
    char16_t sz16[] = u"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(sz16, "sz16", u"28", "", "", "", 0)->String());
    char32_t sz32[] = U"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(sz32, "sz32", U"29", "", "", "", 0)->String());
    wchar_t wsz[] = L"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(wsz, "wsz", L"30", "", "", "", 0)->String());

    // Pointer to a string
    char* psz = new char[2];
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(psz, "psz", "31", "", "", "", 0)->String());
    delete[] psz;
    char16_t* psz16 = new char16_t[2];
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(psz16, "psz16", u"32", "", "", "", 0)->String());
    delete[] psz16;
    char32_t* psz32 = new char32_t[2];
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(psz32, "psz32", U"33", "", "", "", 0)->String());
    delete[] psz32;
    wchar_t* pwsz = new wchar_t[2];
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pwsz, "pwsz", L"34", "", "", "", 0)->String());
    delete[] pwsz;

    // Pointer to a const string
    const char* pcsz = "";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pcsz, "pcsz", "35", "", "", "", 0)->String());
    const char16_t* pcsz16 = u"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pcsz16, "pcsz16", u"36", "", "", "", 0)->String());
    const char32_t* pcsz32 = U"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pcsz32, "pcsz32", U"37", "", "", "", 0)->String());
    const wchar_t* pcwsz = L"";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pcwsz, "pcwsz", L"38", "", "", "", 0)->String());
}

TEST(ParameterTest, AutoTypeInitializeEnum)
{
    // C-style enum
    enum ECStyle {one, two, three} eC;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(eC, "ECStyle", ECStyle::one, "", "", "", 0)->Enum());

    // C++-style enum
    enum class ECppStyle : uint8_t {one, two, three} eCpp;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(eCpp, "ECppStyle", ECppStyle::two, "", "", "", 0)->Enum());
}

TEST(ParameterTest, AutoTypeReadonly)
{
    // Numeric read/write
    int32_t i32RW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(i32RW, "i32RW", 10, "", "", "", 0)->ReadOnly());
    uint32_t ui32RW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(ui32RW, "ui32RW", 20, "", "", "", 0)->ReadOnly());
    double dRW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(dRW, "dRW", 30, "", "", "", 0)->ReadOnly());

    // Strings read/write
    // Remarks: the C-style string and the character pointer are both marked as read-only.
    std::string ssRW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(ssRW, "ssRW", "40", "", "", "", 0)->ReadOnly());
    sdv::string ss_sdvRW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(ss_sdvRW, "ss_sdvRW", "50", "", "", "", 0)->ReadOnly());

    // Enums read/write
    enum ECStyle {one, two, three};
    enum class ECppStyle : uint8_t {one, two, three};
    ECStyle eCStyleRW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(eCStyleRW , "eCStyleRW", ECStyle ::one, "", "", "", 0)->ReadOnly());
    ECppStyle eCppStyleRW;
    EXPECT_FALSE(std::make_shared<sdv::CSdvParamInfo>(eCppStyleRW, "eCppStyleRW", ECppStyle::two, "", "", "", 0)->ReadOnly());

    // Numeric read-only
    const int32_t i32R = 0;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(i32R, "i32R", 60, "", "", "", 0)->ReadOnly());
    const uint32_t ui32R = 0;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ui32R, "ui32R", 70, "", "", "", 0)->ReadOnly());
    const double dR = 0.0;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(dR, "dR", 80, "", "", "", 0)->ReadOnly());

    // Strings read/write
    const std::string ssR;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ssR, "ssR", "90", "", "", "", 0)->ReadOnly());
    const sdv::string ss_sdvR;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(ss_sdvR, "ss_sdvR", "100", "", "", "", 0)->ReadOnly());
    char szR[] = "";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(szR, "szR", "101", "", "", "", 0)->ReadOnly());
    char* pszR = new char[2];
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pszR, "pszR", "102", "", "", "", 0)->ReadOnly());
    delete[] pszR;
    const char* pcszR = "";
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(pcszR, "pcszR", "103", "", "", "", 0)->ReadOnly());

    // Enums read/write
    const ECStyle eCStyleR = ECStyle::one;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(eCStyleR, "eCStyleR", ECStyle::one, "", "", "", 0)->ReadOnly());
    const ECppStyle eCppStyleR = ECppStyle::one;
    EXPECT_TRUE(std::make_shared<sdv::CSdvParamInfo>(eCppStyleR, "eCppStyleR", ECppStyle::one, "", "", "", 0)->ReadOnly());
}

