#include <support/serdes.h>

#include "basic_types_test.h"

using CSerdesTest = CBasicTypesTest;

TEST_F(CSerdesTest, PlatformEndianess)
{
    uint16_t uiTest = 0x0102;
    uint8_t* rguiTest = reinterpret_cast<uint8_t*>(&uiTest);
    if (rguiTest[0] == 0x01) // Big endian
        EXPECT_EQ(sdv::GetPlatformEndianess(), sdv::EEndian::big_endian);
    else // Little endian
        EXPECT_EQ(sdv::GetPlatformEndianess(), sdv::EEndian::little_endian);
}

TEST_F(CSerdesTest, Serialize)
{
    uint64_t uiVal = 0x0102030405060708ull;
    sdv::serializer serializer;
    serializer << uiVal;

    ASSERT_EQ(uiVal, *reinterpret_cast<uint64_t*>(serializer.buffer().get()));
}

TEST_F(CSerdesTest, Deserialize)
{
    uint64_t uiVal = 0x0102030405060708ull;
    sdv::serializer serializer;
    serializer << uiVal;

    sdv::deserializer deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    uint64_t uiVal2 = 0;
    deserializer >> uiVal2;
    ASSERT_EQ(uiVal, uiVal2);
}

TEST_F(CSerdesTest, SerializeSimpleEndianSwap)
{
    uint64_t uiVal = 0x0102030405060708ull;
    static constexpr sdv::EEndian eEndian = sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ?
        sdv::EEndian::big_endian :
        sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << uiVal;

    sdv::pointer<uint8_t> ptrBuffer = serializer.buffer();
    union
    {
        uint64_t uiVal;
        uint8_t rguiBytes[8];
    } uVal;
    uVal.rguiBytes[7] = ptrBuffer[0];
    uVal.rguiBytes[6] = ptrBuffer[1];
    uVal.rguiBytes[5] = ptrBuffer[2];
    uVal.rguiBytes[4] = ptrBuffer[3];
    uVal.rguiBytes[3] = ptrBuffer[4];
    uVal.rguiBytes[2] = ptrBuffer[5];
    uVal.rguiBytes[1] = ptrBuffer[6];
    uVal.rguiBytes[0] = ptrBuffer[7];
    ASSERT_EQ(uiVal, uVal.uiVal);
}

TEST_F(CSerdesTest, DeserializeSimpleEndianSwap)
{
    uint64_t uiVal = 0x0102030405060708ull;
    static constexpr sdv::EEndian eEndian = sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ?
        sdv::EEndian::big_endian :
        sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << uiVal;

    sdv::deserializer<eEndian> deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    uint64_t uiVal2 = 0;
    deserializer >> uiVal2;
    ASSERT_EQ(uiVal, uiVal2);
}

TEST_F(CSerdesTest, SerializeArray)
{
    std::srand((unsigned int)time(0));
    int16_t rguiBuffer[4096];
    for (size_t nIndex = 0; nIndex < 4096; nIndex++)
        rguiBuffer[nIndex] = static_cast<int16_t>(std::rand());

    sdv::serializer serializer;
    serializer << rguiBuffer;

    sdv::pointer<uint8_t> ptrBuffer = serializer.buffer();
	for (size_t nIndex = 0; nIndex < 4096; nIndex++)
		ASSERT_EQ(rguiBuffer[nIndex], *reinterpret_cast<int16_t*>(ptrBuffer.get() + nIndex * 2));
}

TEST_F(CSerdesTest, DeserializeArray)
{
    std::srand((unsigned int)time(0));
    int16_t rguiBuffer[4096];
    for (size_t nIndex = 0; nIndex < 4096; nIndex++)
        rguiBuffer[nIndex] = static_cast<int16_t>(std::rand());

    sdv::serializer serializer;
    serializer << rguiBuffer;

    sdv::deserializer deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    int16_t rguiBuffer2[4096]{};
    deserializer >> rguiBuffer2;
    for (size_t n = 0; n < 4096; n++)
        ASSERT_EQ(rguiBuffer[n], rguiBuffer2[n]);
}

TEST_F(CSerdesTest, SerializeArrayEndianSwap)
{
    std::srand((unsigned int)time(0));
    int16_t rguiBuffer[4096];
    for (size_t nIndex = 0; nIndex < 4096; nIndex++)
        rguiBuffer[nIndex] = static_cast<int16_t>(std::rand());

    static constexpr sdv::EEndian eEndian =
        sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ? sdv::EEndian::big_endian : sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << rguiBuffer;

    sdv::pointer<uint8_t> ptrBuffer = serializer.buffer();
    for (size_t nIndex = 0; nIndex < 4096; nIndex++)
    {
        union
        {
            int16_t uiVal;
            uint8_t rguiBytes[2];
        } uVal;
        uVal.rguiBytes[1] = ptrBuffer[nIndex * 2];
        uVal.rguiBytes[0] = ptrBuffer[nIndex * 2 + 1];
        ASSERT_EQ(rguiBuffer[nIndex], uVal.uiVal);
    }
}

TEST_F(CSerdesTest, DeserializeArrayEndianSwap)
{
    std::srand((unsigned int)time(0));
    int16_t rguiBuffer[4096];
    for (size_t nIndex = 0; nIndex < 4096; nIndex++)
        rguiBuffer[nIndex] = static_cast<int16_t>(std::rand());

    static constexpr sdv::EEndian eEndian =
        sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ? sdv::EEndian::big_endian : sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << rguiBuffer;

    sdv::deserializer<eEndian> deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    int16_t rguiBuffer2[4096]{};
    deserializer >> rguiBuffer2;
    for (size_t n = 0; n < 4096; n++)
        ASSERT_EQ(rguiBuffer[n], rguiBuffer2[n]);
}

/**
 * @brief Complex test structure
*/
struct SComplex
{
    SComplex()
    {
        ptrRgbVal.resize(100);
        ptrFixedRgbVal.resize(50);
    }
    uint8_t uiVal8	= 0x08;
    uint16_t uiVal16 = 0x0106;
    uint32_t uiVal32 = 0x00030002;
    uint64_t uiVal64 = 0x0000000600000004;
    float fVal = 1234.1234f;
    double dVal = 5678.5678;
    long double ldVal = 12345678.12345678l;
    bool bVal = true;
    uint32_t rguiVal32[5] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
    char16_t rgcVal[6] = u"Hello";
    sdv::string ssEmptyVal;
    sdv::string ssVal = "Hi I am ";
    sdv::u8string ss8Val = "the big ";
    sdv::u16string ss16Val = u"program that ";
    sdv::u32string ss32Val = U"tests complex ";
    sdv::wstring sswVal = L"structures!";
    sdv::fixed_string<15> ssFixedVal = "Hi I am ";
    sdv::fixed_u8string<15> ss8FixedVal = "the big ";
    sdv::fixed_u16string<15> ss16FixedVal = u"program that ";
    sdv::fixed_u32string<15> ss32FixedVal = U"tests complex ";
    sdv::fixed_wstring<15> sswFixedVal = L"structures!";

    struct SRGB
    {
        sdv::string ssRed = "red";
        sdv::string ssGreen = "green";
        sdv::string ssBlue = "blue";
    };
    sdv::sequence<SRGB> seqEmptyVal;
    sdv::sequence<SRGB> seqRgbVal = sdv::sequence<SRGB>(2);
    sdv::sequence<SRGB, 20> seqFixedRgbVal = sdv::sequence<SRGB>(4);
    sdv::pointer<SRGB> ptrEmptyVal;
    sdv::pointer<SRGB> ptrRgbVal;
    sdv::pointer<SRGB, 200> ptrFixedRgbVal;

    void Reset();
    bool Equal(const SComplex& rsComplex);
};

bool operator==(const SComplex::SRGB& rsLeft, const SComplex::SRGB& rsRight)
{
    return rsLeft.ssRed == rsRight.ssRed && rsLeft.ssGreen == rsRight.ssGreen && rsLeft.ssBlue == rsRight.ssBlue;
}
bool operator!=(const SComplex::SRGB& rsLeft, const SComplex::SRGB& rsRight)
{
    return rsLeft.ssRed != rsRight.ssRed || rsLeft.ssGreen != rsRight.ssGreen || rsLeft.ssBlue != rsRight.ssBlue;
}

void SComplex::Reset()
{
    uiVal8 = 0;
    uiVal16 = 0;
    uiVal32 = 0;
    uiVal64 = 0;
    fVal = 0.0f;
    dVal = 0.0;
    ldVal = 0.0;
    bVal = false;
    rguiVal32[0] = 0;
    rguiVal32[1] = 0;
    rguiVal32[2] = 0;
    rguiVal32[3] = 0;
    rguiVal32[4] = 0;
    rgcVal[0] = '\0';
    rgcVal[1] = '\0';
    rgcVal[2] = '\0';
    rgcVal[3] = '\0';
    rgcVal[4] = '\0';
    rgcVal[5] = '\0';
    ssEmptyVal.clear();
    ssVal.clear();
    ss8Val.clear();
    ss16Val.clear();
    ss32Val.clear();
    sswVal.clear();
    ssFixedVal.clear();
    ss8FixedVal.clear();
    ss16FixedVal.clear();
    ss32FixedVal.clear();
    sswFixedVal.clear();
    seqEmptyVal.clear();
    seqRgbVal.clear();
    seqFixedRgbVal.clear();
    ptrEmptyVal.reset();
    ptrRgbVal.reset();
    ptrFixedRgbVal.reset();
}

bool SComplex::Equal(const SComplex& rsComplex)
{
    if (uiVal8 != rsComplex.uiVal8 ) return false;
    if (uiVal16 != rsComplex.uiVal16) return false;
    if (uiVal32 != rsComplex.uiVal32) return false;
    if (uiVal64 != rsComplex.uiVal64) return false;
    if (fVal != rsComplex.fVal) return false;
    if (dVal != rsComplex.dVal) return false;
    if (ldVal != rsComplex.ldVal) return false;
    if (bVal != rsComplex.bVal) return false;
    if (rguiVal32[0] != rsComplex.rguiVal32[0]) return false;
    if (rguiVal32[1] != rsComplex.rguiVal32[1]) return false;
    if (rguiVal32[2] != rsComplex.rguiVal32[2]) return false;
    if (rguiVal32[3] != rsComplex.rguiVal32[3]) return false;
    if (rguiVal32[4] != rsComplex.rguiVal32[4]) return false;
    if (rgcVal[0] != rsComplex.rgcVal[0]) return false;
    if (rgcVal[1] != rsComplex.rgcVal[1]) return false;
    if (rgcVal[2] != rsComplex.rgcVal[2]) return false;
    if (rgcVal[3] != rsComplex.rgcVal[3]) return false;
    if (rgcVal[4] != rsComplex.rgcVal[4]) return false;
    if (rgcVal[5] != rsComplex.rgcVal[5]) return false;
    if (ssEmptyVal != rsComplex.ssEmptyVal) return false;
    if (ssVal != rsComplex.ssVal) return false;
    if (ss8Val != rsComplex.ss8Val) return false;
    if (ss16Val != rsComplex.ss16Val) return false;
    if (ss32Val != rsComplex.ss32Val) return false;
    if (sswVal != rsComplex.sswVal) return false;
    if (ssFixedVal != rsComplex.ssFixedVal) return false;
    if (ss8FixedVal != rsComplex.ss8FixedVal) return false;
    if (ss16FixedVal != rsComplex.ss16FixedVal) return false;
    if (ss32FixedVal != rsComplex.ss32FixedVal) return false;
    if (sswFixedVal != rsComplex.sswFixedVal) return false;
    if (seqEmptyVal != rsComplex.seqEmptyVal) return false;
    if (seqRgbVal != rsComplex.seqRgbVal) return false;
    if (seqFixedRgbVal != rsComplex.seqFixedRgbVal) return false;
    if (ptrEmptyVal.size() != rsComplex.ptrEmptyVal.size()) return false;
    for (size_t nIndex = 0; nIndex != ptrEmptyVal.size(); nIndex++)
        if (ptrEmptyVal[nIndex] != rsComplex.ptrEmptyVal[nIndex]) return false;
    if (ptrRgbVal.size() != rsComplex.ptrRgbVal.size()) return false;
    for (size_t nIndex = 0; nIndex != ptrRgbVal.size(); nIndex++)
        if (ptrRgbVal[nIndex] != rsComplex.ptrRgbVal[nIndex]) return false;
    if (ptrFixedRgbVal.size() != rsComplex.ptrFixedRgbVal.size()) return false;
    for (size_t nIndex = 0; nIndex != ptrFixedRgbVal.size(); nIndex++)
        if (ptrFixedRgbVal[nIndex] != rsComplex.ptrFixedRgbVal[nIndex]) return false;
    return true;
}

namespace serdes
{
    /**
     * @brief Specialization of serializer/deserializer class for SComplex::SRGB.
     */
    template <>
    class CSerdes<SComplex::SRGB>
    {
    public:
        /**
         * @brief Stream the variable into the serializer.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rsValue Reference to the variable.
         */
        template <sdv::EEndian eTargetEndianess>
        static sdv::serializer<eTargetEndianess>& Serialize(sdv::serializer<eTargetEndianess>& rSerializer, const SComplex::SRGB& rsValue)
        {
            rSerializer << rsValue.ssRed;
            rSerializer << rsValue.ssGreen;
            rSerializer << rsValue.ssBlue;
            return rSerializer;
        }

        /**
         * @brief Stream the variable from the deserializer.
         * @tparam eSourceEndianess The source endianness determines whether to swap the bytes after retrieving them from the buffer.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[in] rsValue Reference to the variable.
         */
        template <sdv::EEndian eSourceEndianess>
        static sdv::deserializer<eSourceEndianess>& Deserialize(sdv::deserializer<eSourceEndianess>& rDeserializer, SComplex::SRGB& rsValue)
        {
            rDeserializer >> rsValue.ssRed;
            rDeserializer >> rsValue.ssGreen;
            rDeserializer >> rsValue.ssBlue;
            return rDeserializer;
        }
    };

    /**
     * @brief Specialization of serializer/deserializer class for SComplex.
     */
    template <>
    class CSerdes<SComplex>
    {
    public:
        /**
         * @brief Stream the variable into the serializer.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rsValue Reference to the variable.
         */
        template <sdv::EEndian eTargetEndianess>
        static sdv::serializer<eTargetEndianess>& Serialize(sdv::serializer<eTargetEndianess>& rSerializer, const SComplex& rsValue)
        {
            rSerializer << rsValue.uiVal8;
            rSerializer << rsValue.uiVal16;
            rSerializer << rsValue.uiVal32;
            rSerializer << rsValue.uiVal64;
            rSerializer << rsValue.fVal;
            rSerializer << rsValue.dVal;
            rSerializer << rsValue.ldVal;
            rSerializer << rsValue.bVal;
            rSerializer << rsValue.rguiVal32;
            rSerializer << rsValue.rgcVal;
            rSerializer << rsValue.ssEmptyVal;
            rSerializer << rsValue.ssVal;
            rSerializer << rsValue.ss8Val;
            rSerializer << rsValue.ss16Val;
            rSerializer << rsValue.ss32Val;
            rSerializer << rsValue.sswVal;
            rSerializer << rsValue.ssFixedVal;
            rSerializer << rsValue.ss8FixedVal;
            rSerializer << rsValue.ss16FixedVal;
            rSerializer << rsValue.ss32FixedVal;
            rSerializer << rsValue.sswFixedVal;
            rSerializer << rsValue.seqEmptyVal;
            rSerializer << rsValue.seqRgbVal;
            rSerializer << rsValue.seqFixedRgbVal;
            rSerializer << rsValue.ptrEmptyVal;
            rSerializer << rsValue.ptrRgbVal;
            rSerializer << rsValue.ptrFixedRgbVal;
            return rSerializer;
        }

        /**
         * @brief Stream the variable from the deserializer.
         * @tparam eSourceEndianess The source endianness determines whether to swap the bytes after retrieving them from the buffer.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[in] rsValue Reference to the variable.
         */
        template <sdv::EEndian eSourceEndianess>
        static sdv::deserializer<eSourceEndianess>& Deserialize(sdv::deserializer<eSourceEndianess>& rDeserializer, SComplex& rsValue)
        {
            rDeserializer >> rsValue.uiVal8;
            rDeserializer >> rsValue.uiVal16;
            rDeserializer >> rsValue.uiVal32;
            rDeserializer >> rsValue.uiVal64;
            rDeserializer >> rsValue.fVal;
            rDeserializer >> rsValue.dVal;
            rDeserializer >> rsValue.ldVal;
            rDeserializer >> rsValue.bVal;
            rDeserializer >> rsValue.rguiVal32;
            rDeserializer >> rsValue.rgcVal;
            rDeserializer >> rsValue.ssEmptyVal;
            rDeserializer >> rsValue.ssVal;
            rDeserializer >> rsValue.ss8Val;
            rDeserializer >> rsValue.ss16Val;
            rDeserializer >> rsValue.ss32Val;
            rDeserializer >> rsValue.sswVal;
            rDeserializer >> rsValue.ssFixedVal;
            rDeserializer >> rsValue.ss8FixedVal;
            rDeserializer >> rsValue.ss16FixedVal;
            rDeserializer >> rsValue.ss32FixedVal;
            rDeserializer >> rsValue.sswFixedVal;
            rDeserializer >> rsValue.seqEmptyVal;
            rDeserializer >> rsValue.seqRgbVal;
            rDeserializer >> rsValue.seqFixedRgbVal;
            rDeserializer >> rsValue.ptrEmptyVal;
            rDeserializer >> rsValue.ptrRgbVal;
            rDeserializer >> rsValue.ptrFixedRgbVal;
            return rDeserializer;
        }
    };
} // namespace serdes

TEST_F(CSerdesTest, SerializeComplex)
{
    SComplex sComplex;
    sdv::serializer serializer;
    serializer << sComplex;

    sdv::pointer<uint8_t> ptrBuffer = serializer.buffer();

    // Check values
    size_t nOffset = 0;
    EXPECT_EQ(sComplex.uiVal8, *ptrBuffer.get());
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(sComplex.uiVal16, *reinterpret_cast<uint16_t*>(ptrBuffer.get() + nOffset));
    nOffset += 2;
    EXPECT_EQ(sComplex.uiVal32, *reinterpret_cast<uint32_t*>(ptrBuffer.get() + nOffset));
    nOffset += 4;
    EXPECT_EQ(sComplex.uiVal64, *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.fVal, *reinterpret_cast<float*>(ptrBuffer.get() + nOffset));
    nOffset += 4;
    EXPECT_EQ(0u, *reinterpret_cast<uint32_t*>(ptrBuffer.get() + nOffset)); // padding
    nOffset += 4;
    EXPECT_EQ(sComplex.dVal, *reinterpret_cast<double*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    if constexpr (sizeof(long double) == 16)
        if (nOffset % 16)
        {
            EXPECT_EQ(0ull, *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset)); // padding
            nOffset += 8;
        }
    EXPECT_EQ(sComplex.ldVal, *reinterpret_cast<long double*>(ptrBuffer.get() + nOffset));
    nOffset += sizeof(long double);
    EXPECT_EQ(sComplex.bVal, *reinterpret_cast<bool*>(ptrBuffer.get() + nOffset));
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    for (size_t nIndex = 0; nIndex < 5; nIndex++)
    {
        EXPECT_EQ(sComplex.rguiVal32[nIndex], *reinterpret_cast<uint32_t*>(ptrBuffer.get() + nOffset));
        nOffset += 4;
    }
    EXPECT_STREQ(static_cast<const char16_t*>(sComplex.rgcVal), reinterpret_cast<const char16_t*>(ptrBuffer.get() + nOffset));
    nOffset += sizeof(sComplex.rgcVal);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ssEmptyVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssVal, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ssVal.size()));
    nOffset += sComplex.ssVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss8Val.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss8Val, sdv::u8string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ss8Val.size()));
    nOffset += sComplex.ss8Val.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss16Val.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss16Val, sdv::u16string(reinterpret_cast<char16_t*>(ptrBuffer.get() + nOffset), sComplex.ss16Val.size()));
    nOffset += sComplex.ss16Val.size() * sizeof(char16_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss32Val.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss32Val, sdv::u32string(reinterpret_cast<char32_t*>(ptrBuffer.get() + nOffset), sComplex.ss32Val.size()));
    nOffset += sComplex.ss32Val.size() * sizeof(char32_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.sswVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.sswVal, sdv::wstring(reinterpret_cast<wchar_t*>(ptrBuffer.get() + nOffset), sComplex.sswVal.size()));
    nOffset += sComplex.sswVal.size() * sizeof(wchar_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ssFixedVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssFixedVal, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ssFixedVal.size()));
    nOffset += sComplex.ssFixedVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss8FixedVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss8FixedVal, sdv::u8string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ss8FixedVal.size()));
    nOffset += sComplex.ss8FixedVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss16FixedVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss16FixedVal, sdv::u16string(reinterpret_cast<char16_t*>(ptrBuffer.get() + nOffset), sComplex.ss16FixedVal.size()));
    nOffset += sComplex.ss16FixedVal.size() * sizeof(char16_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss32FixedVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss32FixedVal, sdv::u32string(reinterpret_cast<char32_t*>(ptrBuffer.get() + nOffset), sComplex.ss32FixedVal.size()));
    nOffset += sComplex.ss32FixedVal.size() * sizeof(char32_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.sswFixedVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.sswFixedVal, sdv::wstring(reinterpret_cast<wchar_t*>(ptrBuffer.get() + nOffset), sComplex.sswFixedVal.size()));
    nOffset += sComplex.sswFixedVal.size() * sizeof(wchar_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.seqEmptyVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.seqRgbVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (const SComplex::SRGB& rsRgb : sComplex.seqRgbVal)
    {
        ASSERT_EQ(rsRgb.ssRed.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.seqFixedRgbVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (const SComplex::SRGB& rsRgb : sComplex.seqFixedRgbVal)
    {
        ASSERT_EQ(rsRgb.ssRed.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.ptrEmptyVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ptrRgbVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (size_t nIndex = 0; nIndex < sComplex.ptrRgbVal.size(); nIndex++)
    {
        const SComplex::SRGB& rsRgb = sComplex.ptrRgbVal[nIndex];
        ASSERT_EQ(rsRgb.ssRed.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.ptrFixedRgbVal.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (size_t nIndex = 0; nIndex < sComplex.ptrFixedRgbVal.size(); nIndex++)
    {
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        const SComplex::SRGB& rsRgb = sComplex.ptrFixedRgbVal[nIndex];
        ASSERT_EQ(rsRgb.ssRed.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
    }
}

TEST_F(CSerdesTest, DeserializeComplex)
{
    SComplex sComplex;
    sdv::serializer serializer;
    serializer << sComplex;

    sdv::deserializer deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    SComplex sComplex2;
    sComplex2.Reset();

    EXPECT_FALSE(sComplex.Equal(sComplex2));
    deserializer >> sComplex2;
    EXPECT_TRUE(sComplex.Equal(sComplex2));
}

TEST_F(CSerdesTest, SerializeComplexEndianSwap)
{
    SComplex sComplex;
    static constexpr sdv::EEndian eEndian =
        sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ? sdv::EEndian::big_endian : sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << sComplex;

    sdv::pointer<uint8_t> ptrBuffer = serializer.buffer();

    auto fnSwap16 = [&](uint8_t* pData) -> uint16_t
    {
		union
		{
            uint8_t		rgui[2];
            uint16_t 	ui;
		} temp{ pData[1], pData[0] };
        return temp.ui;
    };
    auto fnSwap32 = [&](uint8_t* pData) -> uint32_t
    {
		union
		{
            uint8_t		rgui[4];
            uint32_t 	ui;
		} temp{ pData[3], pData[2], pData[1], pData[0] };
        return temp.ui;
    };
    auto fnSwapFloat = [&](uint8_t* pData) -> float
    {
		union
		{
			uint32_t uiTemp;
			float fTemp;
        } temp{ fnSwap32(pData) };
        return temp.fTemp;
    };
    auto fnSwap64 = [&](uint8_t* pData) -> uint64_t
    {
		union
		{
			uint8_t		rgui[8];
			uint64_t 	ui;
		} temp{ pData[7], pData[6], pData[5], pData[4], pData[3], pData[2], pData[1], pData[0] };
        return temp.ui;
    };
    auto fnSwapDouble = [&](uint8_t* pData) -> double
    {
		union
		{
			uint64_t uiTemp;
			double   dTemp;
        } temp{ fnSwap64(pData) };
        return temp.dTemp;
    };
    auto fnSwapLongDouble = [&](uint8_t* pData) -> long double
    {
        if constexpr (sizeof(long double) == 8)
        {
			union
			{
				uint64_t 	uiTemp;
				long double ldTemp;
            } temp{ fnSwap64(pData) };
			return temp.ldTemp;
        }
        else
        {
            union
            {
                uint64_t	rgui[2];
                long double dbl;
            } temp{ fnSwap64(pData + 8), fnSwap64(pData) };
            return temp.dbl;
        }
    };
    auto fnSwapString16 = [&](uint8_t* pData, size_t nSize) -> std::u16string
    {
        std::u16string ss;
        size_t nOffset = 0;
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            ss += static_cast<char16_t>(fnSwap16(pData + nOffset));
            nOffset += sizeof(char16_t);
        }
        return ss;
    };
    auto fnSwapString32 = [&](uint8_t* pData, size_t nSize) -> std::u32string
    {
        std::u32string ss;
        size_t nOffset = 0;
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            ss += static_cast<char32_t>(fnSwap32(pData + nOffset));
            nOffset += sizeof(char32_t);
        }
        return ss;
    };
    auto fnSwapStringW = [&](uint8_t* pData, size_t nSize) -> std::wstring
    {
        std::wstring ss;
        size_t nOffset = 0;
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            if constexpr (sizeof(wchar_t) == 2)
                ss += static_cast<wchar_t>(fnSwap16(pData + nOffset));
            else
                ss += static_cast<wchar_t>(fnSwap32(pData + nOffset));
            nOffset += sizeof(wchar_t);
        }
        return ss;
    };

    // Check values
    size_t nOffset = 0;
    EXPECT_EQ(sComplex.uiVal8, *ptrBuffer.get());
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(sComplex.uiVal16, fnSwap16(ptrBuffer.get() + nOffset));
    nOffset += 2;
    EXPECT_EQ(sComplex.uiVal32, fnSwap32(ptrBuffer.get() + nOffset));
    nOffset += 4;
    EXPECT_EQ(sComplex.uiVal64, fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.fVal, fnSwapFloat(ptrBuffer.get() + nOffset));
    nOffset += 4;
    EXPECT_EQ(0u, *reinterpret_cast<uint32_t*>(ptrBuffer.get() + nOffset)); // padding
    nOffset += 4;
    EXPECT_EQ(sComplex.dVal, fnSwapDouble(ptrBuffer.get() + nOffset));
    nOffset += 8;
    if constexpr (sizeof(long double) == 16)
        if (nOffset % 16)
        {
            EXPECT_EQ(0ull, *reinterpret_cast<uint64_t*>(ptrBuffer.get() + nOffset)); // padding
            nOffset += 8;
        }
    EXPECT_EQ(sComplex.ldVal, fnSwapLongDouble(ptrBuffer.get() + nOffset));
    nOffset += sizeof(long double);
    EXPECT_EQ(sComplex.bVal, *reinterpret_cast<bool*>(ptrBuffer.get() + nOffset));
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
    nOffset++;
    for (size_t nIndex = 0; nIndex < 5; nIndex++)
    {
        EXPECT_EQ(sComplex.rguiVal32[nIndex], fnSwap32(ptrBuffer.get() + nOffset));
        nOffset += 4;
    }
    for (size_t nIndex = 0; nIndex < 6; nIndex++)
    {
        EXPECT_EQ(static_cast<const char16_t*>(sComplex.rgcVal)[nIndex], fnSwap16(ptrBuffer.get() + nOffset));
        nOffset += sizeof(char16_t);
    }
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ssEmptyVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssVal, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ssVal.size()));
    nOffset += sComplex.ssVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss8Val.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss8Val, sdv::u8string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ss8Val.size()));
    nOffset += sComplex.ss8Val.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss16Val.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss16Val, fnSwapString16(ptrBuffer.get() + nOffset, sComplex.ss16Val.size()));
    nOffset += sComplex.ss16Val.size() * sizeof(char16_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss32Val.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss32Val, fnSwapString32(ptrBuffer.get() + nOffset, sComplex.ss32Val.size()));
    nOffset += sComplex.ss32Val.size() * sizeof(char32_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.sswVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.sswVal, fnSwapStringW(ptrBuffer.get() + nOffset, sComplex.sswVal.size()));
    nOffset += sComplex.sswVal.size() * sizeof(wchar_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ssFixedVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ssFixedVal, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ssFixedVal.size()));
    nOffset += sComplex.ssFixedVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss8FixedVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss8FixedVal, sdv::u8string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), sComplex.ss8FixedVal.size()));
    nOffset += sComplex.ss8FixedVal.size();
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss16FixedVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss16FixedVal, fnSwapString16(ptrBuffer.get() + nOffset, sComplex.ss16FixedVal.size()));
    nOffset += sComplex.ss16FixedVal.size() * sizeof(char16_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.ss32FixedVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ss32FixedVal, fnSwapString32(ptrBuffer.get() + nOffset, sComplex.ss32FixedVal.size()));
    nOffset += sComplex.ss32FixedVal.size() * sizeof(char32_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.sswFixedVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.sswFixedVal, fnSwapStringW(ptrBuffer.get() + nOffset, sComplex.sswFixedVal.size()));
    nOffset += sComplex.sswFixedVal.size() * sizeof(wchar_t);
    while (nOffset % 8)
    {
        EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
        nOffset++;
    }
    EXPECT_EQ(sComplex.seqEmptyVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.seqRgbVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (const SComplex::SRGB& rsRgb : sComplex.seqRgbVal)
    {
        ASSERT_EQ(rsRgb.ssRed.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.seqFixedRgbVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (const SComplex::SRGB& rsRgb : sComplex.seqFixedRgbVal)
    {
        ASSERT_EQ(rsRgb.ssRed.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.ptrEmptyVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    EXPECT_EQ(sComplex.ptrRgbVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (size_t nIndex = 0; nIndex < sComplex.ptrRgbVal.size(); nIndex++)
    {
        const SComplex::SRGB& rsRgb = sComplex.ptrRgbVal[nIndex];
        ASSERT_EQ(rsRgb.ssRed.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
    }
    EXPECT_EQ(sComplex.ptrFixedRgbVal.size(), fnSwap64(ptrBuffer.get() + nOffset));
    nOffset += 8;
    for (size_t nIndex = 0; nIndex < sComplex.ptrFixedRgbVal.size(); nIndex++)
    {
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        const SComplex::SRGB& rsRgb = sComplex.ptrFixedRgbVal[nIndex];
        ASSERT_EQ(rsRgb.ssRed.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssRed, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssRed.size()));
        nOffset += rsRgb.ssRed.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssGreen.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssGreen, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssGreen.size()));
        nOffset += rsRgb.ssGreen.size();
        while (nOffset % 8)
        {
            EXPECT_EQ(ptrBuffer[nOffset], 0); // padding
            nOffset++;
        }
        ASSERT_EQ(rsRgb.ssBlue.size(), fnSwap64(ptrBuffer.get() + nOffset));
        nOffset += 8;
        EXPECT_EQ(rsRgb.ssBlue, sdv::string(reinterpret_cast<char*>(ptrBuffer.get() + nOffset), rsRgb.ssBlue.size()));
        nOffset += rsRgb.ssBlue.size();
    }
}

TEST_F(CSerdesTest, DeserializeComplexEndianSwap)
{
    SComplex sComplex;
    static constexpr sdv::EEndian eEndian =
        sdv::GetPlatformEndianess() == sdv::EEndian::little_endian ? sdv::EEndian::big_endian : sdv::EEndian::little_endian;
    sdv::serializer<eEndian> serializer;
    serializer << sComplex;

    sdv::deserializer<eEndian> deserializer;
    deserializer.attach(serializer.buffer(), serializer.checksum());
    SComplex sComplex2;
    sComplex2.Reset();

    EXPECT_FALSE(sComplex.Equal(sComplex2));
    deserializer >> sComplex2;
    EXPECT_TRUE(sComplex.Equal(sComplex2));
}

TEST_F(CSerdesTest, SerializeSize)
{
    int32_t i32 = 10;
    size_t nSize = 0;
    sdv::ser_size(i32,nSize);
    EXPECT_EQ(nSize, sizeof(i32));

    uint16_t ui16 = 99;
    nSize = 0;
    sdv::ser_size(ui16,nSize);
    EXPECT_EQ(nSize, sizeof(ui16));

    char sz[2] = { 'a', 'b' };
    nSize = 0;
    sdv::ser_size(sz,nSize);
    EXPECT_EQ(nSize, sizeof(sz));

    int64_t i64 = -98765432;
    nSize = 0;
    sdv::ser_size(i64,nSize);
    EXPECT_EQ(nSize, sizeof(i64));

    uint64_t ui64 = 123456789;
    nSize = 0;
    sdv::ser_size(ui64,nSize);
    EXPECT_EQ(nSize, sizeof(ui64));

    sdv::string ss = "Hello all!";
    nSize = 0;
    sdv::ser_size(ss,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + ss.size());

    sdv::wstring wss = L"Hello all!";
    nSize = 0;
    sdv::ser_size(wss,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + wss.size() * sizeof(wchar_t));

    sdv::u8string ss8 = "Hello all!";
    nSize = 0;
    sdv::ser_size(ss8,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + ss8.size());

    sdv::u16string ss16 = u"Hello all!";
    nSize = 0;
    sdv::ser_size(ss16,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + ss16.size() * sizeof(char16_t));

    sdv::u32string ss32 = U"Hello all!";
    nSize = 0;
    sdv::ser_size(ss32,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + ss32.size() * sizeof(char32_t));

    sdv::fixed_string<40> fss = "Hello all!";
    nSize = 0;
    sdv::ser_size(fss,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fss.size());

    sdv::fixed_wstring<400> fwss = L"Hello all!";
    nSize = 0;
    sdv::ser_size(fwss,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fwss.size() * sizeof(wchar_t));

    sdv::fixed_u8string<40> fss8 = "Hello all!";
    nSize = 0;
    sdv::ser_size(fss8,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fss8.size());

    sdv::fixed_u16string<40> fss16 = u"Hello all!";
    nSize = 0;
    sdv::ser_size(fss16,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fss16.size() * sizeof(char16_t));

    sdv::fixed_u32string<40> fss32 = U"Hello all!";
    nSize = 0;
    sdv::ser_size(fss32,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fss32.size() * sizeof(char32_t));

    sdv::sequence<uint32_t> fseq = { 10, 20, 30 };
    nSize = 0;
    sdv::ser_size(fseq,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + fseq.size() * sizeof(uint32_t));

    sdv::pointer<uint16_t, 10> ptr;
    ptr.resize(3);
    ptr[0] = 10;
    ptr[1] = 20;
    ptr[2] = 30;
    nSize = 0;
    sdv::ser_size(ptr,nSize);
    EXPECT_EQ(nSize, sizeof(uint64_t) + ptr.size() * sizeof(uint16_t));
}