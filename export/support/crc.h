#ifndef SDV_CRC_H
#define SDV_CRC_H

#include <array>
#include <cstdint>
#include <stddef.h>

namespace sdv
{
    /**
     * @brief Reflect the bits within the data type (change the order MSB <--> LSB).
     * @tparam T The type to use for bit reflection.
     * @param[in] tValue The value to reflect. Must be a arithmetic or boolean type.
     * @return The reflected value.
    */
    template <typename T>
    constexpr T reflect(T tValue)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, bool>);

        T tMask = 1;
        T tReflection{0};
        for (size_t nBit = 0; nBit < sizeof(T) * 8; nBit++)
        {
            tReflection <<= 1;
            if (tValue & tMask)
                tReflection |= 0x1;
            tMask <<= 1;
        }
        return tReflection;
    }

    /**
     * @brief Templated CRC calculation class
     * @tparam TCRC The CRC type to use for the calculation.
     * @tparam tPolynomial The polynomial of the CRC function.
     * @tparam tInitVal The initial value to use for the calculation.
     * @tparam tXorOut XOR the result with the provided value.
     * @tparam bReflectIn Reflect the input bits.
     * @tparam bReflectOut Reflect the output bits.
     */
    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    class crc
    {
    public:
        using TCRCType = TCRC;      ///< CRC type

        /**
         * @brief Calculate the CRC checksum value providing a buffer with data of type T.
         * @attention This function doesn't reset the CRC that was calculated before.
         * @remarks The CRC calculation occurs byte-wise regardless of the endianness of the the processor architecture.
         * @tparam T Type of the value buffer to calculate the CRC for.
         * @param[in] pData Pointer to the value buffer.
         * @param[in] nCount Amount of values in the buffer.
         * @return The calculated CRC checksum.
        */
        template <typename T>
        TCRCType calc_checksum(const T* pData, size_t nCount) noexcept;

        /**
         * @brief Add one value to the calculation.
         * @remarks The CRC calculation occurs byte-wise regardless of the endianness of the the processor architecture.
         * @tparam T Type of the value buffer to calculate the CRC for.
         * @param[in] tValue The value to add to the calculation.
        */
        template <typename T>
        void add(T tValue) noexcept;

        /**
         * @brief Get the currently calculated checksum.
         * @return The CRC checksum.
        */
        TCRCType get_checksum() const noexcept;

        /**
         * @brief Set a CRC checksum to continue the calculation with additional data.
         * @param[in] tCrcValue The previously calculated CRC value.
        */
        void set_checksum(TCRCType tCrcValue) noexcept;

        /**
         * @brief Reset the current calculation (resets the CRC value to its initial value).
        */
        void reset() noexcept;

    private:
        /**
         * @brief With of the CRC type in bits.
         */
        static constexpr uint64_t m_nWidth = 8 * sizeof(TCRCType);

        /**
         * @brief MSB for the CRC type.
         */
        static constexpr uint64_t m_nMsb = 1ull << (m_nWidth - 1);

        /**
         * @brief CRC lookup table.
         */
        static constexpr auto m_arrTable = []
        {
            std::array<TCRCType, 256> arrTemp{};
            for (size_t tDividend = 0; tDividend < 256; ++tDividend)
            {
                TCRCType tRemainder = static_cast<TCRCType>(tDividend << (m_nWidth - 8));
                for (uint8_t bit = 8; bit > 0; --bit)
                {
                    if (tRemainder & m_nMsb)
                        tRemainder = (tRemainder << 1) ^ tPolynomial;
                    else
                        tRemainder = (tRemainder << 1);
                }
                arrTemp[tDividend] = tRemainder;
            }
            return arrTemp;
        }();

        TCRCType m_tCrcValue = tInitVal;  ///< Calculated CRC value.
    };

    /// SAE-J1850: polynomial 0x1D, initial = 0xFF, final_xor = 0xFF, reflect_input = false, reflect_output = false
    using crcSAE_J1850 = sdv::crc<uint8_t, 0x1du, 0xff, 0xff, false, false>;

    /// AUTOSAR_8H2F: polynomial 0x2F, initial = 0xFF, final_xor = 0xFF, reflect_input = false, reflect_output = false)
    using crcAUTOSAR_8H2F = sdv::crc<uint8_t, 0x2fu, 0xff, 0xff, false, false>;

    /// CCITT-FALSE: polynomial 0x1021, initial = 0xFFFF, final_xor = 0, reflect_input = false, reflect_output = false
    using crcCCITT_FALSE = sdv::crc<uint16_t, 0x1021u, 0xffffu, 0, false, false> ;

    /// ARC: polynomial 0x8005, initial = 0, final_xor = 0, reflect_input = true, reflect_output = true
    using crcARC = sdv::crc<uint16_t, 0x8005, 0, 0, true, true>;

    /// IEEE_802_3 polynomial 0x04C11DB7, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true, reflect_output = true
    using crcIEEE_802_3 = sdv::crc<uint32_t, 0x04C11DB7u, 0xffffffffu, 0xffffffffu, true, true> ;

    /// AUTOSAR_P4 polynomial 0xF4ACFB13, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true, reflect_output = true
    using crcAUTOSAR_P4 = sdv::crc<uint32_t, 0xF4ACFB13u, 0xffffffffu, 0xffffffffu, true, true> ;

    /// CRC32-C Castagnoli polynomial 0x1EDC6F41, initial = 0xFFFFFFFF, final_xor = 0xFFFFFFFF, reflect_input = true,
    /// reflect_output = true
    using crcCRC32C = sdv::crc<uint32_t, 0x1EDC6F41, 0xffffffffu, 0xffffffffu, true, true> ;

    /// ECMA (polynomial 0x42F0E1EBA9EA3693, initial = 0xFFFFFFFFFFFFFFFF, final_xor = 0xFFFFFFFFFFFFFFFF, reflect_input = true,
    /// reflect_output = true
    using crcECMA = sdv::crc<uint64_t, 0x42F0E1EBA9EA3693, 0xffffffffffffffffu, 0xffffffffffffffffu, true, true>;

    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    template <typename T>
    typename crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::TCRCType
        crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::calc_checksum(const T* pData, size_t nCount) noexcept
    {
        if (!pData || !nCount) return get_checksum();
        for (size_t nIndex = 0; nIndex < nCount; nIndex++)
            add(pData[nIndex]);
        return get_checksum();
    }

    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    template <typename T>
    inline void crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::add(T tValue) noexcept
    {
        if constexpr (bReflectIn)
        {
            for (size_t nIndex = 0; nIndex < sizeof(T); ++nIndex)
            {
                uint8_t data = static_cast<uint8_t>(reflect(reinterpret_cast<uint8_t*>(&tValue)[nIndex]) ^ (m_tCrcValue >> (m_nWidth - 8)));
                m_tCrcValue = m_arrTable[data] ^ (m_tCrcValue << 8);
            }
        }
        else
        {
            for (size_t nIndex = 0; nIndex < sizeof(T); ++nIndex)
            {
                uint8_t data = static_cast<uint8_t>((reinterpret_cast<uint8_t*>(&tValue)[nIndex]) ^ (m_tCrcValue >> (m_nWidth - 8)));
                m_tCrcValue = m_arrTable[data] ^ (m_tCrcValue << 8);
            }
        }
    }

    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    inline typename crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::TCRCType
        crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::get_checksum() const noexcept
    {
        if constexpr (bReflectOut)
            return static_cast<TCRCType>(reflect(m_tCrcValue) ^ tXorOut);
        else
            return static_cast<TCRCType>(m_tCrcValue ^ tXorOut);
    }

    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    inline void crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::set_checksum(TCRCType tCrcValue) noexcept
    {
        if constexpr (bReflectOut)
            m_tCrcValue = static_cast<TCRCType>(reflect(static_cast<TCRCType>(tCrcValue ^ tXorOut)));
        else
            m_tCrcValue = static_cast<TCRCType>(tCrcValue ^ tXorOut);
    }

    template <typename TCRC, TCRC tPolynomial, TCRC tInitVal, TCRC tXorOut, bool bReflectIn, bool bReflectOut>
    inline void crc<TCRC, tPolynomial, tInitVal, tXorOut, bReflectIn, bReflectOut>::reset() noexcept
    {
        m_tCrcValue = tInitVal;
    }

} // namespace sdv

#endif // SDV_CRC_H