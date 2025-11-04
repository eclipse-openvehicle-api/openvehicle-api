#include <support/crc.h>

#include "basic_types_test.h"

using CCrcTest = CBasicTypesTest;

TEST_F(CCrcTest, BitReflection)
{
    uint8_t ui8 = 0b11001010;
    EXPECT_EQ(sdv::reflect(ui8), 0b01010011);
    uint16_t ui16 = 0b0111001111001010;
    EXPECT_EQ(sdv::reflect(ui16), 0b0101001111001110);
    uint32_t ui32 = 0b01110011110010100111001111001010u;
    EXPECT_EQ(sdv::reflect(ui32), 0b01010011110011100101001111001110u);
    uint64_t ui64 = 0b0111001111001010011100111100101001110011110010100111001111001010ull;
    EXPECT_EQ(sdv::reflect(ui64), 0b0101001111001110010100111100111001010011110011100101001111001110ull);
}

TEST_F(CCrcTest, CalcCRC)
{
    // Create data table
    constexpr auto arrTable = []
    {
        std::array<uint8_t, 1024> arrTemp{};
        for (size_t n = 0; n < 1024; n++)
            arrTemp[n] = static_cast<uint8_t>(n & 0xff);
        return arrTemp;
    }();

    // Use http://www.sunshine2k.de/coding/javascript/crc/crc_js.html to check the CRC values.
    // for (size_t nRow = 0; nRow < 64; nRow++)
    //{
    //	for (size_t nCol = 0; nCol < 16; nCol++)
    //		std::cout << " 0x" << std::setfill('0') << std::setw(2) << std::hex << (int)arrTable[nRow * 16 + nCol];
    //	std::cout << std::endl;
    //}

    // SAE-J1850: polynomial 0x1D, initial = 0xFF, final_xor = 0xFF, reflect_input = false, reflect_output = false
    sdv::crcSAE_J1850 crcSAE_J1850;
    EXPECT_EQ(crcSAE_J1850.calc_checksum(arrTable.data(), 1024), 0x84);

    // AUTOSAR_8H2F: polynomial 0x2F, initial = 0xFF, final_xor = 0xFF, reflect_input = false, reflect_output = false)
    sdv::crcAUTOSAR_8H2F crcAUTOSAR_8H2F;
    EXPECT_EQ(crcAUTOSAR_8H2F.calc_checksum(arrTable.data(), 1024), 0x36);

    // CCITT-FALSE: polynomial 0x1021, initial = 0xFFFF, final_xor = 0, reflect_input = false, reflect_output = false
    sdv::crcCCITT_FALSE crcCCITT_FALSE;
    EXPECT_EQ(crcCCITT_FALSE.calc_checksum(arrTable.data(), 1024), 0x758f);

    // ARC: polynomial 0x8005, initial = 0, final_xor = 0, reflect_input = true, reflect_output = true
    sdv::crcARC crcARC;
    EXPECT_EQ(crcARC.calc_checksum(arrTable.data(), 1024), 0x3840);

    // IEEE_802_3 polynomial 0x04C11DB7, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true, reflect_output = true
    sdv::crcIEEE_802_3 crcIEEE_802_3;
    EXPECT_EQ(crcIEEE_802_3.calc_checksum(arrTable.data(), 1024), 0xB70B4C26);

    // AUTOSAR_P4 polynomial 0xF4ACFB13, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true, reflect_output = true
    sdv::crcAUTOSAR_P4 crcAUTOSAR_P4;
    EXPECT_EQ(crcAUTOSAR_P4.calc_checksum(arrTable.data(), 1024), 0xCF0881B6);

    // CRC32-C Castagnoli polynomial 0x1EDC6F41, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true, reflect_output
    // = true
    sdv::crcCRC32C crcCRC32C;
    EXPECT_EQ(crcCRC32C.calc_checksum(arrTable.data(), 1024), 0x2CDF6E8Fu);

    // ECMA (polynomial 0x42F0E1EBA9EA3693, initial = 0xFFFFFFFFFFFFFFFF, final_xor = 0xFFFFFFFFFFFFFFFF, reflect_input = true,
    // reflect_output = true)
    sdv::crcECMA crcECMA;
    EXPECT_EQ(crcECMA.calc_checksum(arrTable.data(), 1024), 0xD51FB58DC789C400);
}

TEST_F(CCrcTest, CalcPartialCRC)
{
    // Create data table
    constexpr auto arrTable = []
    {
        std::array<uint8_t, 1024> arrTemp{};
        for (size_t n = 0; n < 1024; n++)
            arrTemp[n] = static_cast<uint8_t>(n & 0xff);
        return arrTemp;
    }();

    sdv::crcCRC32C crcCRC32C;

    // Full calculation
    EXPECT_EQ(crcCRC32C.get_checksum(), 0x00000000u);
    crcCRC32C.calc_checksum(arrTable.data(), 1024);
    EXPECT_EQ(crcCRC32C.get_checksum(), 0x2CDF6E8Fu);

    // Partial calulcation
    crcCRC32C.reset();
    EXPECT_EQ(crcCRC32C.get_checksum(), 0x00000000u);
    crcCRC32C.calc_checksum(arrTable.data(), 512);
    EXPECT_EQ(crcCRC32C.get_checksum(), 0xAE10EE5Au);
    crcCRC32C.calc_checksum(arrTable.data() + 512, 512);
    EXPECT_EQ(crcCRC32C.get_checksum(), 0x2CDF6E8Fu);

    crcCRC32C.set_checksum(0xAE10EE5Au);
    crcCRC32C.calc_checksum(arrTable.data() + 512, 512);
    EXPECT_EQ(crcCRC32C.get_checksum(), 0x2CDF6E8Fu);
}
