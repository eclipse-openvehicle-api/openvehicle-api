#ifndef SDV_SERDES_H
#define SDV_SERDES_H

#include <cstdint>
#include "pointer.h"
#include "sequence.h"
#include "string.h"
#include "any.h"
#include "crc.h"

namespace sdv
{

    /**
     * @brief Return the endianness for this platform.
     * @return The platform endianness enum value.
     */
    inline constexpr EEndian GetPlatformEndianess()
    {
        // Since C++11 there is no programmatic way to test for endianness in a constexpr function. During runtime this is possible
        // using a union or a casting pointers. C++20 adds platform endian support and the implementation is likely done using
        // compiler constants.
#if defined _MSC_VER
        return EEndian::little_endian;
#elif defined __GNUC__
        return __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ ? EEndian::big_endian : EEndian::little_endian;
#endif
    }

    /**
     * @brief Serializer class implementing the serialization of basic types and memory management.
     * @details The serialization into the buffer is aligned to the size of the value to store. For example, bytes can be stored at
     * any position. 16-bit words can be stored at 2 bytes boundary. 32-bits can be stored at 4 bytes boundary and 64-bits can be
     * stored at 8 bytes boundary.
     * The buffer allocation occurs in 1024 bytes at the time. The buffer is readjusted to the correct size just before detaching
     * the buffer.
     * @tparam eTargetEndianess The targeendiannessss determines whether to swap the bytes before storing them into the buffer.
     * @tparam TCRC The CRC type to use for the checksum calculation.
     */
    template <sdv::EEndian eTargetEndianess = GetPlatformEndianess(), typename TCRC = crcCCITT_FALSE>
    class serializer
    {
    public:
        /**
         * @brief Constructor
         */
        serializer() noexcept;

        /**
         * @brief Push value into the serializer.
         * @tparam T Type of the value. Only arithmic and boolean types can be added. All other types need to be decomposed before
         * they can be added.
         * @param[in] tValue The value to add.
         */
        template <typename T>
        void push_back(T tValue);

        /**
         * @brief Attach a buffer to serialize into.
         * @param[in] rptrBuffer Reference to the buffer.
         * @param[in] nOffset Offset to start serializing.
         * @param[in] uiChecksum Current checksum value to continue with in the CRC calculation.
         */
        void attach(pointer<uint8_t>&& rptrBuffer, size_t nOffset = 0, typename TCRC::TCRCType uiChecksum = 0u);

        /**
         * @brief Detach the internal buffer and assign this buffer to the supplied pointer.
         * @param[out] rptrBuffer Reference to the pointer to assign this buffer to.
         */
        void detach(pointer<uint8_t>& rptrBuffer);

        /**
         * @brief Reserve space for a large amount of data.
         * @details Reserving space for data is done automatically, but only in small chunks. If large data is serialized, it is
         * more efficient to reserve the space at once before the serialization process takes place. This will prevent many smaller
         * reallocations taking place.
         * @param[in] nSize The size of new data to reserve space for.
         */
        void reserve(size_t nSize);

        /**
        * @brief Get a copy of the buffer pointer.
        * @return Smart pointer to the contained buffer.
        */
        pointer<uint8_t> buffer() const;

        /**
         * @brief Return the calculated checksum value of the serialized data.
         * @return Checksum value.
         */
        typename TCRC::TCRCType checksum() const noexcept;

        /**
         * @brief Return the current offset.
         * @return The offset from the start of the stream.
         */
        size_t offset() const;

    private:
        /**
         * @brief Extend the buffer and align to the size of the variable.
         * @tparam T The type of the variable to use the size from.
        */
        template <typename T>
        void extend_and_align();

        pointer<uint8_t> m_ptrBuffer;   ///< Buffer smart pointer.
        size_t m_nOffset = 0;           ///< Current offset in the buffer.
        TCRC m_crcChecksum;             ///< Calculated checksum value.
    };

    /**
     * @brief Deserializer class implementing the deserialization of basic types.
     * @details The deserialization from the buffer is aligned to the size of the value that was stored. For example, bytes are
     * stored at any position. 16-bit words are stored at 2 bytes aligned. 32-bits can be stored at 4 bytes aligned and 64-bits are
     * stored at 8 bytes aligned.
     * @tparam eSourceEndianess The source endianness determines whether to swap the bytes after retrieving them from the buffer.
     * @tparam TCRC The CRC type to use for the checksum calculation.
     */
    template <sdv::EEndian eSourceEndianess = GetPlatformEndianess(), typename TCRC = crcCCITT_FALSE>
    class deserializer
    {
    public:
        /**
         * @brief Constructor
         */
        deserializer();

        /**
         * @brief Pull the value from the deserializer.
         * @tparam T Type of the value. Only arithmic and boolean types can be deserialized. All other types need to be composed
         * by deserialized arithmic and boolean types.
         * @param[out] rtValue Reference to the value to get.
         */
        template <typename T>
        void pop_front(T& rtValue);

        /**
         * @brief Peek for the value from the deserializer without popping the value from the deserializer.
         * @tparam T Type of the value. Only arithmic and boolean types can be deserialized.
         * @param[out] rtValue Reference to the value to get.
         */
        template <typename T>
        void peek_front(T& rtValue);

        /**
         * @brief Assign a buffer.
         * @param[in] pData Pointer to the data.
         * @param[in] nSize Data size;
         * @param[in] uiChecksum The checksum value that was calculated or 0 when no checksum checking should occur before
         * deserialization.
         */
        void assign(const uint8_t* pData, size_t nSize, typename TCRC::TCRCType uiChecksum = 0);

        /**
         * @brief Attach a buffer.
         * @remarks Providing the checksum will result a check through the data in the supplied buffer for the fitting checksum.
         * @param[in] rptrData Reference to the data pointer.
         * @param[in] uiChecksum The checksum value that was calculated or 0 when no checksum checking should occur.
         */
        void attach(const pointer<uint8_t>& rptrData, typename TCRC::TCRCType uiChecksum = 0);

        /**
         * @brief Return the calculated checksum value of the deserialized data.
         * @return Checksum value.
         */
        typename TCRC::TCRCType checksum() const noexcept;

        /**
         * @brief Return the size of the contained buffer.
         * @return The size of the buffer.
         */
        size_t size() const;

        /**
         * @brief Return the current offset.
         * @return The offset from the start of the stream.
         */
        size_t offset() const;

        /**
         * @brief Return the leftover space within the buffer (size - offset).
         * @return The leftover space of the buffer.
         */
        size_t remaining() const;

        /**
         * @brief Skip to the supplied offset and start calculating the rest of the checksum with the supplied checksum.
         * @details This function allows navigating through the buffer or chunkwise deserialization. By supplying an offset, the
         * deserialization can start from the supplied offset using the supplied checksum to check any further. It is also possible
         * to do chunkwise deserialization, providing a small buffer covering only part of the large serialized package. Jumping to
         * the beginning of the buffer (offset 0), but explicitly setting the checksum, allows processing the package as if part of
         * a larger buffer.
         * @param[in] nOffset New offset to start processing from. Must be smaller than the buffer size.
         * @param[in] uiChecksum Checksum to start calculating the checksum value over the following data with.
         */
        void jump(size_t nOffset, typename TCRC::TCRCType uiChecksum = 0);

    private:
        /**
         * @brief Align the offset to the size of the variable.
         * @tparam T The type of the variable to use the size from.
         */
        template <typename T>
        void align();

        pointer<uint8_t> m_ptrBuffer;       ///< Buffer smart pointer (might not be used).
        const uint8_t* m_pData = nullptr;   ///< Pointer to the data.
        size_t m_nSize = 0;                 ///< Current buffer length.
        size_t m_nOffset = 0;               ///< Current offset in the buffer.
        TCRC m_crcChecksum;                 ///< Calculated checksum value.

    };

    /**
     * @brief Return the size of type T in serialized form.
     * @remarks Dependable on the size value, padding is added to align data.
     * @tparam T Type of the variable.
     * @param[in] rtValue Reference to the variable.
     * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
     */
    template <typename T>
    void ser_size(const T& rtValue, size_t& rnSize);
} // namespace sdv

/**
 * @brief Serializer/deserializer namespace.
 */
namespace serdes
{
    /**
     * @brief Serializer/deserializer class.
     * @tparam T Type of the variable.
     */
    template <typename T>
    class CSerdes
    {
    public:
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rtValue Reference to the variable.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(const T& rtValue, size_t& rnSize);

        /**
         * @brief Stream the variable into the serializer.
         * @tparam eTargetEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rtValue Reference to the variable.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const T& rtValue);

        /**
         * @brief Stream a variable from the deserializer.
         * @tparam eSourceEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rtValue Reference to the variable to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, T& rtValue);
    };
} // namespace serdes

/**
 * @brief Stream the variable into the serializer.
 * @tparam T Type of the variable.
 * @tparam eTargetEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
 * @tparam TCRC The CRC type to use for the checksum calculation.
 * @param[in] rSerializer Reference to the serializer.
 * @param[in] rtValue Reference to the variable.
 * @return Reference to the serializer.
 */
template <typename T, sdv::EEndian eTargetEndianess, typename TCRC>
sdv::serializer<eTargetEndianess, TCRC>& operator<<(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, const T& rtValue);

/**
 * @brief Stream a variable from the deserializer.
 * @tparam T Type of the variable.
 * @tparam eSourceEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
 * @tparam TCRC The CRC type to use for the checksum calculation.
 * @param[in] rDeserializer Reference to the deserializer.
 * @param[out] rtValue Reference to the variable to be filled.
 * @return Reference to the deserializer.
 */
template <typename T, sdv::EEndian eSourceEndianess, typename TCRC>
sdv::deserializer<eSourceEndianess, TCRC>& operator>>(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, T& rtValue);

#include "serdes.inl"

#endif // !defined SDV_SERDES_H
