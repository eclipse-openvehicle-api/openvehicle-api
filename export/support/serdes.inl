#ifndef SDV_SERDES_INL
#define SDV_SERDES_INL

#ifndef SDV_SERDES_H
#error Do not include "serdes.inl" directly. Include "serdes.h" instead!
#endif //!defined SDV_SERDES_H

#include <type_traits>

namespace sdv
{
    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline serializer<eTargetEndianess, TCRC>::serializer() noexcept
    {}

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    template <typename T>
    inline void serializer<eTargetEndianess, TCRC>::push_back(T tValue)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, bool> || std::is_enum_v<T>);

        // Make certain the data fits and align to the proper address.
        extend_and_align<T>();

        // Without buffer there is no serialization.
        if (!m_ptrBuffer) return;

        if constexpr (eTargetEndianess == GetPlatformEndianess()) // No swapping
            *reinterpret_cast<T*>(m_ptrBuffer.get() + m_nOffset) = tValue;
        else // Swap bytes
        {
            uint8_t* pData = m_ptrBuffer.get() + m_nOffset;
            for (size_t nIndex = 0; nIndex < sizeof(T); nIndex++)
                pData[nIndex] = reinterpret_cast<uint8_t*>(&tValue)[sizeof(T) - nIndex - 1];
        }

        // Calculate CRC
        for (size_t nIndex = m_nOffset; nIndex < m_nOffset + sizeof(T); nIndex++)
            m_crcChecksum.add(m_ptrBuffer[nIndex]);

        // Increase offset
        m_nOffset += sizeof(T);
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline void serializer<eTargetEndianess, TCRC>::attach(pointer<uint8_t>&& rptrBuffer, size_t nOffset /*= 0*/, typename TCRC::TCRCType uiChecksum /*= 0u*/)
    {
        if (nOffset > rptrBuffer.size())
        {
            XOffsetPastBufferSize exception;
            exception.uiOffset = static_cast<uint32_t>(nOffset);
            exception.uiSize = static_cast<uint32_t>(rptrBuffer.size());
            throw exception;
        }

        m_ptrBuffer = std::move(rptrBuffer);
        m_nOffset = nOffset;
        m_crcChecksum.set_checksum(uiChecksum);
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline void serializer<eTargetEndianess, TCRC>::detach(pointer<uint8_t>& rptrBuffer)
    {
        // Reduce the buffer to the currently calculated offset.
        if (m_ptrBuffer)
            m_ptrBuffer.resize(m_nOffset);

        // Assign the buffer
        rptrBuffer = std::move(m_ptrBuffer);

        // Clear the offset
        m_nOffset = 0;
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline void serializer<eTargetEndianess, TCRC>::reserve(size_t nSize)
    {
        // Check whether reservation is required.
        if (m_ptrBuffer.size() < m_nOffset + nSize)
            m_ptrBuffer.resize(m_ptrBuffer.size() + nSize);
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline pointer<uint8_t> serializer<eTargetEndianess, TCRC>::buffer() const
    {
        pointer<uint8_t> ptrLocal = m_ptrBuffer;

        // Reduce the buffer to the currently calculated offset.
        if (ptrLocal)
            ptrLocal.resize(m_nOffset);

        // Return a copy of the smart pointer to the internal buffer.
        return ptrLocal;
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline typename TCRC::TCRCType serializer<eTargetEndianess, TCRC>::checksum() const noexcept
    {
        return m_crcChecksum.get_checksum();
    }

    template <EEndian eTargetEndianess, typename TCRC>
    inline size_t serializer<eTargetEndianess, TCRC>::offset() const
    {
        return m_nOffset;
    }

    template <sdv::EEndian eTargetEndianess, typename TCRC>
    template <typename T>
    inline void serializer<eTargetEndianess, TCRC>::extend_and_align()
    {
        // Align the offset position dependable on the size of the variable to add.
        size_t nOffsetNew = m_nOffset;
        if ((nOffsetNew & (sizeof(T) - 1)) != 0)
            nOffsetNew = (nOffsetNew | (sizeof(T) - 1)) + 1;

        // Check if the buffer size is large enough to hold the alignment and the type.
        // If not, extend with 1024 bytes.
        if ((nOffsetNew + sizeof(T)) > m_ptrBuffer.size())
            m_ptrBuffer.resize(m_ptrBuffer.size() + 1024);

        // Fill the buffer with padding and add the padding to the checksum value calculation.
        for (size_t nIndex = m_nOffset; nIndex != nOffsetNew; nIndex++)
        {
            m_ptrBuffer[m_nOffset] = 0x00;
            m_crcChecksum.add(static_cast<uint8_t>(0x00));
        }

        // Set the new offset
        m_nOffset = nOffsetNew;
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline deserializer<eSourceEndianess, TCRC>::deserializer()
    {}

    template <EEndian eSourceEndianess, typename TCRC>
    template <typename T>
    inline void deserializer<eSourceEndianess, TCRC>::pop_front(T& rtValue)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, bool> || std::is_enum_v<T>);

        // Without buffer there is no deserialization.
        if (!m_ptrBuffer) return;

        // Align to the proper address.
        align<T>();

        // Check whether the buffer contains the data requested
        if (m_ptrBuffer.size() < m_nOffset + sizeof(T))
        {
            sdv::XBufferTooSmall exception;
            exception.uiSize = m_nOffset + sizeof(T);
            exception.uiCapacity = m_ptrBuffer.size();
            throw exception;
        }

        // Calculate CRC
        for (size_t nIndex = m_nOffset; nIndex < m_nOffset + sizeof(T); nIndex++)
            m_crcChecksum.add(m_ptrBuffer[nIndex]);

        // Copy data
        if constexpr (eSourceEndianess == GetPlatformEndianess()) // No swapping
            rtValue = *reinterpret_cast<T*>(m_ptrBuffer.get() + m_nOffset);
        else // Swap bytes
        {
            const uint8_t* pData = m_ptrBuffer.get() + m_nOffset;
            for (size_t nIndex = 0; nIndex < sizeof(T); nIndex++)
                reinterpret_cast<uint8_t*>(&rtValue)[nIndex] = pData[sizeof(T) - nIndex - 1];
        }

        // Increase offset
        m_nOffset += sizeof(T);
    }

    template <EEndian eSourceEndianess, typename TCRC>
    template <typename T>
    inline void deserializer<eSourceEndianess, TCRC>::peek_front(T& rtValue)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, bool> || std::is_enum_v<T>);

        // Without buffer there is no deserialization.
        if (!m_ptrBuffer) return;

        // Store the offset and checksum
        size_t nOffsetTemp = m_nOffset;
        auto nChecksum = m_crcChecksum.get_checksum();

        // Make certain the data fits and align to the proper address.
        align<T>();

        // Check whether the buffer contains the data requested
        if (m_ptrBuffer.size() < m_nOffset + sizeof(T))
        {
            sdv::XBufferTooSmall exception;
            exception.uiSize = m_nOffset + sizeof(T);
            exception.uiCapacity = m_ptrBuffer.size();
            throw exception;
        }

        // Copy data
        if constexpr (eSourceEndianess == GetPlatformEndianess()) // No swapping
            rtValue = *reinterpret_cast<T*>(m_ptrBuffer.get() + m_nOffset);
        else // Swap bytes
        {
            const uint8_t* pData = m_ptrBuffer.get() + m_nOffset;
            for (size_t nIndex = 0; nIndex < sizeof(T); nIndex++)
                reinterpret_cast<uint8_t*>(&rtValue)[nIndex] = pData[sizeof(T) - nIndex - 1];
        }

        // Reset the offset and checksum
        m_nOffset = nOffsetTemp;
        m_crcChecksum.set_checksum(nChecksum);
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline void deserializer<eSourceEndianess, TCRC>::assign(const uint8_t* pData, size_t nSize, typename TCRC::TCRCType uiChecksum /*= 0*/)
    {
        if (!pData) throw XNullPointer();

        // Check the checksum value
        if (uiChecksum)
        {
            crcCCITT_FALSE crcChecksum;
            for (size_t nIndex = 0; nIndex < nSize; nIndex++)
                crcChecksum.add(pData[nIndex]);
            if (crcChecksum.get_checksum() != uiChecksum)
            {
                XHashNotMatching exception;
                exception.uiCalculated = crcChecksum.get_checksum();
                exception.uiProvided = uiChecksum;
                throw exception;
            }
        }

        m_pData = pData;
        m_nSize = nSize;
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline void deserializer<eSourceEndianess, TCRC>::attach(const pointer<uint8_t>& rptrData, typename TCRC::TCRCType uiChecksum /*= 0*/)
    {
        m_ptrBuffer = rptrData;
        assign(rptrData.get(), rptrData.size(), uiChecksum);
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline typename TCRC::TCRCType deserializer<eSourceEndianess, TCRC>::checksum() const noexcept
    {
        return m_crcChecksum.get_checksum();
    }

    template <EEndian eSourceEndianess, typename TCRC>
    template <typename T>
    inline void deserializer<eSourceEndianess, TCRC>::align()
    {
        // Align the offset position dependable on the size of the variable to get.
        if ((m_nOffset & (sizeof(T) - 1)) != 0)
        {
            size_t nOffsetNew = (m_nOffset | (sizeof(T) - 1)) + 1;

            // Add the padding to the checksum value calculation.
            for (size_t nIndex = m_nOffset; nIndex != nOffsetNew; nIndex++)
                m_crcChecksum.add(m_ptrBuffer[nIndex]);

            m_nOffset = nOffsetNew;
        }
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline size_t deserializer<eSourceEndianess, TCRC>::size() const
    {
        return m_nSize;
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline size_t deserializer<eSourceEndianess, TCRC>::offset() const
    {
        return m_nOffset;
    }

    template <EEndian eSourceEndianess, typename TCRC>
    inline size_t deserializer<eSourceEndianess, TCRC>::remaining() const
    {
        return m_nSize - std::min(m_nOffset, m_nSize);
    }

    template <EEndian eSourceEndianess, typename TCRC>
    void deserializer<eSourceEndianess, TCRC>::jump(size_t nOffset, typename TCRC::TCRCType uiChecksum /*= 0*/)
    {
        if (nOffset > m_ptrBuffer.size())
        {
            XOffsetPastBufferSize exception;
            exception.uiOffset = static_cast<uint32_t>(nOffset);
            exception.uiSize = static_cast<uint32_t>(m_ptrBuffer.size());
            throw exception;
        }

        m_nOffset = nOffset;
        m_crcChecksum.set_checksum(uiChecksum);
    }

    template <typename T>
    inline void ser_size(const T& rtValue, size_t& rnSize)
    {
        serdes::CSerdes<T>::CalcSize(rtValue, rnSize);
    }
} // namespace sdv

namespace serdes
{
    template <typename T>
    inline void CSerdes<T>::CalcSize([[maybe_unused]] const T& rtValue, size_t& rnSize)
    {
        static_assert(std::is_fundamental_v<T> || std::is_enum_v<T>);

        // Add alignment if necessary
        size_t nBytes = rnSize % sizeof(T);
        if (nBytes) rnSize += sizeof(T) - nBytes;

        // Increase the size
        rnSize += sizeof(T);
    }

    template <typename T>
    template <sdv::EEndian eTargetEndianess, typename TCRC>
    inline sdv::serializer<eTargetEndianess, TCRC>& CSerdes<T>::Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const T& rtValue)
    {
        rSerializer.push_back(rtValue);
        return rSerializer;
    }

    template <typename T>
    template <sdv::EEndian eSourceEndianess, typename TCRC>
    inline sdv::deserializer<eSourceEndianess, TCRC>& CSerdes<T>::Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, T& rtValue)
    {
        rDeserializer.pop_front(rtValue);
        return rDeserializer;
    }

    /**
     * @brief Specialization of serializer/deserializer class.
     * @tparam T Type of the variable.
     * @tparam nSize The size of the provided array.
     */
    template <typename T, size_t nSize>
    class CSerdes<T[nSize]>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rrgtValue Reference to an array with variables.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize([[maybe_unused]] const T(&rrgtValue)[nSize], size_t& rnSize)
        {
            for (const T& rtValue : rrgtValue)
                sdv::ser_size(rtValue, rnSize);
        }

        /**
        * @brief Stream the variable into the serializer.
        * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
        * @tparam TCRC The CRC type to use for the checksum calculation.
        * @param[in] rSerializer Reference to the serializer.
        * @param[in] rrgtValue Reference to an array with variables.
        * @return Reference to the serializer.
        */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const T(&rrgtValue)[nSize])
        {
            // Reserve the space in the serializer (this speeds up the serialization process).
            rSerializer.reserve(nSize * sizeof(T));

            // Serialize all entries
            for (const T& rtValue : rrgtValue)
                CSerdes<T>::Serialize(rSerializer, rtValue);
            return rSerializer;
        }

        /**
        * @brief Stream a variable from the deserializer.
        * @tparam eSourceEndianess The source endianess detemines whether to swap the bytes after retrieving them from the buffer.
        * @tparam TCRC The CRC type to use for the checksum calculation.
        * @param[in] rDeserializer Reference to the deserializer.
        * @param[out] rrgtValue Reference to an array with variables to be filled.
        * @return Reference to the deserializer.
        */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, T(&rrgtValue)[nSize])
        {
            for (T& rtValue : rrgtValue)
                CSerdes<T>::Deserialize(rDeserializer, rtValue);
            return rDeserializer;
        }
    };

    /**
     * @brief Specialization of serializer/deserializer class.
     * @tparam T Type of the variable.
     * @tparam nFixedSize The fixed size of the pointer or 0 when the pointer is dynamic.
     */
    template <typename T, size_t nFixedSize>
    class CSerdes<sdv::pointer<T, nFixedSize>>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rptrValue Reference to the pointer.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::pointer<T, nFixedSize>& rptrValue, size_t& rnSize)
        {
            sdv::ser_size(static_cast<uint64_t>(rptrValue.size()), rnSize);
            if constexpr (std::is_fundamental_v<T>)
                rnSize += rptrValue.size() * sizeof(T);
            else
            {
                for (size_t n = 0; n < rptrValue.size(); n++)
                    sdv::ser_size(rptrValue.get()[n], rnSize);
            }
        }

        /**
         * @brief Stream the pointer variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rptrValue Reference to the pointer.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const sdv::pointer<T, nFixedSize>& rptrValue)
        {
            rSerializer << static_cast<uint64_t>(rptrValue.size());
            if (rptrValue)
            {
                // Reserve the space in the serializer (this speeds up the serialization process).
                rSerializer.reserve(rptrValue.size() * sizeof(T));

                // Serialize all entries
                const T* ptValue = rptrValue.get();
                for (size_t nIndex = 0; nIndex < rptrValue.size(); nIndex++)
                    CSerdes<T>::Serialize(rSerializer, ptValue[nIndex]);
            }
            return rSerializer;
        }

        /**
         * @brief Stream the pointer variable from the deserializer.
         * @tparam eSourceEndianess The source endianess detemines whether to swap the bytes after retrieving them from the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[in] rptrValue Reference to the pointer.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, sdv::pointer<T, nFixedSize>& rptrValue)
        {
            uint64_t nSize = 0;
            rDeserializer >> nSize;
            if (nSize * sizeof(T) > rDeserializer.remaining())
            {
                sdv::XOffsetPastBufferSize exception;
                exception.uiSize = rDeserializer.size();
                exception.uiOffset = rDeserializer.offset() + nSize * sizeof(T);
                throw exception;
            }
            rptrValue.resize(nSize);
            if (nSize && rptrValue)
            {
                T* ptValue = rptrValue.get();
                for (size_t nIndex = 0; nIndex < nSize; nIndex++)
                    CSerdes<T>::Deserialize(rDeserializer, ptValue[nIndex]);
            }
            return rDeserializer;
        }
    };
    /**
     * @brief Specialization of serializer/deserializer class.
     * @tparam T Type of the variable.
     * @tparam nFixedSize The fixed size of the sequence or 0 when the sequence is dynamic.
     */
    template <typename T, size_t nFixedSize>
    class CSerdes<sdv::sequence<T, nFixedSize>>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rseqValue Reference to the sequence.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::sequence<T, nFixedSize>& rseqValue, size_t& rnSize)
        {
            sdv::ser_size(static_cast<uint64_t>(rseqValue.size()), rnSize);
            if constexpr (std::is_fundamental_v<T>)
                rnSize += rseqValue.size() * sizeof(T);
            else
            {
                for (const T& rValue: rseqValue)
                    sdv::ser_size(rValue, rnSize);
            }
        }

        /**
         * @brief Stream the sequence variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rseqValue Reference to the sequence.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const sdv::sequence<T, nFixedSize>& rseqValue)
        {
            rSerializer << static_cast<uint64_t>(rseqValue.size());

            // Reserve the space in the serializer (this speeds up the serialization process).
            rSerializer.reserve(rseqValue.size() * sizeof(T));

            // Serialize all entries
            for (const T& rtValue : rseqValue)
                CSerdes<T>::Serialize(rSerializer, rtValue);
            return rSerializer;
        }

        /**
         * @brief Stream the sequence variable from the deserializer.
         * @tparam eSourceEndianess The source endianess detemines whether to swap the bytes after retrieving them from the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[in] rseqValue Reference to the sequence.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, sdv::sequence<T, nFixedSize>& rseqValue)
        {
            uint64_t nSize = 0;
            rDeserializer >> nSize;
            if (nSize * sizeof(T) > rDeserializer.remaining())
            {
                sdv::XOffsetPastBufferSize exception;
                exception.uiSize = rDeserializer.size();
                exception.uiOffset = rDeserializer.offset() + nSize * sizeof(T);
                throw exception;
            }
            rseqValue.resize(nSize);
            if (nSize)
            {
                for (T& rtValue : rseqValue)
                    CSerdes<T>::Deserialize(rDeserializer, rtValue);
            }
            return rDeserializer;
        }
    };

    /**
     * @brief Specialization of serializer/deserializer class.
     * @tparam TCharType Type of the string.
     * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
     * @tparam TCRC The CRC type to use for the checksum calculation.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize The fixed size of the string or 0 when the string is dynamic.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    class CSerdes<sdv::string_base<TCharType, bUnicode, nFixedSize>>
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rssValue Reference to the string.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const sdv::string_base<TCharType, bUnicode, nFixedSize>& rssValue, size_t& rnSize)
        {
            sdv::ser_size(static_cast<uint64_t>(rssValue.size()), rnSize);
            rnSize += rssValue.size() * sizeof(TCharType);
        }

        /**
         * @brief Stream the string variable into the serializer.
         * @tparam eTargetEndianess The target endianess detemines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rssValue Reference to the string.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const sdv::string_base<TCharType, bUnicode, nFixedSize>& rssValue)
        {
            rSerializer << static_cast<uint64_t>(rssValue.size());

            // Reserve the space in the serializer (this speeds up the serialization process).
            rSerializer.reserve(rssValue.size() * sizeof(TCharType));

            // Serialize all entries
            for (TCharType tValue : rssValue)
                CSerdes<TCharType>::Serialize(rSerializer, tValue);
            return rSerializer;
        }

        /**
         * @brief Stream the string variable from the deserializer.
         * @tparam eSourceEndianess The source endianess detemines whether to swap the bytes after retrieving them from the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[in] rssValue Reference to the string.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, sdv::string_base<TCharType, bUnicode, nFixedSize>& rssValue)
        {
            uint64_t nSize = 0;
            rDeserializer >> nSize;
            if (nSize * sizeof(TCharType) > rDeserializer.remaining())
            {
                sdv::XOffsetPastBufferSize exception;
                exception.uiSize = rDeserializer.size();
                exception.uiOffset = rDeserializer.offset() + nSize * sizeof(TCharType);
                throw exception;
            }
            rssValue.resize(nSize);
            if (nSize)
            {
                for (TCharType& rtValue : rssValue)
                    CSerdes<TCharType>::Deserialize(rDeserializer, rtValue);
            }
            return rDeserializer;
        }
    };
} // namespace serdes

template <typename T, sdv::EEndian eTargetEndianess, typename TCRC>
inline sdv::serializer<eTargetEndianess, TCRC>& operator<<(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const T& rtValue)
{
    serdes::CSerdes<T>::Serialize(rSerializer, rtValue);
    return rSerializer;
}

template <typename T, sdv::EEndian eSourceEndianess, typename TCRC>
inline sdv::deserializer<eSourceEndianess, TCRC>& operator>>(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, T& rtValue)
{
    serdes::CSerdes<T>::Deserialize(rDeserializer, rtValue);
    return rDeserializer;
}

#endif // !defined SDV_SERDES_INL