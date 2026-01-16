#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <filesystem>

/**
 * @brief This class covers the source file reading
 */
class CSource
{
public:
    /**
     * @brief Default constructor
     */
    CSource();

    /**
     * @brief Constructor reading the source file.
     * @param[in] rpath Path to the source file.
     */
    CSource(const std::filesystem::path& rpath);

    /**
     * @brief Construct using the code, bypassing the reading of the file.
     * @param[in] szCode Zero terminated string containing the code.
     */
    CSource(const char* szCode);

    /**
     * @brief Copy constructor.
     * @param[in] rSource Reference to the source object.
     */
    CSource(const CSource& rSource) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rSource Reference to the source object.
     */
    CSource(CSource&& rSource) noexcept;

    /**
     * @brief Assignment operator
     * @param[in] rSource Reference to the source object.
     * @return Reference to the source object.
     */
    CSource& operator=(const CSource& rSource) = delete;

    /**
     * @brief Move operator
     * @param[in] rSource Reference to the source object.
     * @return Reference to the source object.
     */
    CSource& operator=(CSource&& rSource) noexcept;

    /**
     * @brief Get the source file path (as reference).
     * @return The source file path.
     */
    const std::filesystem::path& GetPathRef() const;

    /**
     * @brief Get the source code (as reference).
     * @return The source code.
     */
    const std::string& GetCodeRef() const;

    /**
     * @brief Read a source file.
     * @param[in] rpath Path to the source file.
     */
    void ReadFile(const std::filesystem::path& rpath);

private:
    /**
     * @brief Determine whether the system endianness is big-endian.
     * @return Returns 'true' when the system is a big-endian system. Otherwise returns 'false'.
     */
    constexpr static bool IsSystemBigEndian();

    /**
     * @brief Determine whether the system endianness is little-endian.
     * @return Returns 'true' when the system is a little-endian system. Otherwise returns 'false'.
     */
    constexpr static bool IsSystemLittleEndian();

    /**
     * @brief Swap the endianness of the character.
     * @param[in, out] rc16 Reference to the UTF16 character.
     */
    static void EndianSwap(char16_t& rc16);

    /**
     * @brief Swap the endianness of the character.
     * @param[in, out] rc32 Reference to the UTF32 character.
     */
    static void EndianSwap(char32_t& rc32);

    /**
     * @brief Potentially in-place swap the data in a buffer if necessary.
     * @tparam TChar the character type to use for swapping.
     * @param[in, out] szBuffer The buffer to swap the data for.
     * @param[in] nSize The size of the buffer in characters.
     * @param[in] bIsSourceBigEndian When set, the source data in the buffer is organized big-endian.
     */
    template <typename TChar>
    void PotentialSwapBuffer(TChar* szBuffer, size_t nSize, bool bIsSourceBigEndian);

    /**
     * @brief Convert a UTF16 string to UTF-8.
     * @param[in] szBuffer The source string.
     * @param[in] nSize The length of the string (or zero terminating string when supplied as 0).
     * @return Returns the string as UTF8 std::string object.
     */
    static std::string ConvertToUTF8(const char16_t* szBuffer, size_t nSize);

    /**
     * @brief Convert a UTF32 string to UTF-8.
     * @param[in] szBuffer The source string.
     * @param[in] nSize The length of the string (or zero terminating string when supplied as 0).
     * @return Returns the string as UTF8 std::string object.
     */
    static std::string ConvertToUTF8(const char32_t* szBuffer, size_t nSize);

    std::filesystem::path   m_path;         ///< Path to the file.
    std::string             m_ssSource;     ///< The source file content.
};

inline void CSource::EndianSwap(char16_t& rc16)
{
    uint8_t uiTemp = reinterpret_cast<uint8_t*>(&rc16)[0];
    reinterpret_cast<uint8_t*>(&rc16)[0] = reinterpret_cast<uint8_t*>(&rc16)[1];
    reinterpret_cast<uint8_t*>(&rc16)[1] = uiTemp;
}

inline void CSource::EndianSwap(char32_t& rc32)
{
    uint8_t uiTemp = reinterpret_cast<uint8_t*>(&rc32)[0];
    reinterpret_cast<uint8_t*>(&rc32)[0] = reinterpret_cast<uint8_t*>(&rc32)[3];
    reinterpret_cast<uint8_t*>(&rc32)[3] = uiTemp;
    reinterpret_cast<uint8_t*>(&rc32)[1] = reinterpret_cast<uint8_t*>(&rc32)[2];
    reinterpret_cast<uint8_t*>(&rc32)[2] = uiTemp;
}

template <typename TChar>
inline void CSource::PotentialSwapBuffer(TChar* szBuffer, size_t nSize, bool bIsSourceBigEndian)
{
    // Only works for two or four byte characters
    static_assert(sizeof(TChar) == 2 || sizeof(TChar) == 4);

    // Check for a valid buffer
    if (!szBuffer) return;

    // Only swap when the endianness of the data is not corresponding to the character endianness of the system.
    if (bIsSourceBigEndian == IsSystemBigEndian()) return;

    // Swap all bytes
    for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        EndianSwap(szBuffer[nIndex]);
}


#endif // !defined SOURCE_H