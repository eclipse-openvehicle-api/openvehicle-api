#ifndef CHARACTER_READER_UTF_8_H
#define CHARACTER_READER_UTF_8_H

#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdint>

/// The TOML parser namespace
namespace toml_parser
{
    /**
     * @brief Reads a given input string, interprets bytes as UTF-8 and returns UTF-8 characters or strings in order of demand.
     */
    class CCharacterReaderUTF8
    {
    public:
        /**
         * @brief Standard constructor for empty character reader.
         */
        CCharacterReaderUTF8() = default;

        /**
         * @brief Constructs a character reader from a given string.
         * @param[in] rssString UTF-8 input string.
         * @throw XTOMLParseException Throws an InvalidCharacterException if the input contains invalid UTF-8 characters.
         * @throw XTOMLParseException Throws an InvalidByteException if the input contains for UTF-8 invalid bytes.
         */
        CCharacterReaderUTF8(const std::string& rssString);

        /**
         * @brief Feed the character reader from the given string.
         * @param[in] rssString UTF-8 input string.
         */
        void Feed(const std::string& rssString);

        /**
         * @brief Reset the character reader content.
         */
        void Reset();

        /**
         * @brief Get the next n-th UTF-8 character without advancing the cursor.
         * @param[in] nSkip Amount of characters to skip.
         * @param[in] nAmount The amount of characters to read.
         * @return Returns one or more characters starting at the requested position after the current cursor or an empty string
         * if the requested position is behind the last character.
         */
        std::string Peek(size_t nSkip = 0, size_t nAmount = 1) const;

        /**
         * @brief Get all upcoming UTF-8 characters until a terminating character without advancing the cursor.
         * @param[in] lstCollection A collection of terminating characters.
         * @return Returns a string of UTF-8 characters until (excluding) the first occurrence of one of the given characters.
         */
        std::string PeekUntil(const std::vector<std::string>& lstCollection) const;

        /**
         * @brief Get the next n-th UTF-8 character and advancing the cursor by n.
         * @param[in] nSkip Amount of characters to skip.
         * @param[in] nAmount The amount of characters to read.
         * @return Returns one or more characters starting at the requested position after the current cursor or an empty string
         * if the requested position is behind the last character.
         */
        std::string Consume(size_t nSkip = 0, size_t nAmount = 1);

        /**
         * @brief Get all upcoming UTF-8 characters until a terminating character and advancing the cursor by the number of
         * characters in the returned string.
         * @param[in] lstCollection A collection of terminating characters.
         * @return Returns a string of UTF-8 characters until (excluding) the first occurrence of one of the given characters.
         */
        std::string ConsumeUntil(const std::vector<std::string>& lstCollection);

        /**
         * @brief Checks if the cursor is at the end of the data to read.
         * @return Returns true if the cursor is at the end of the readable data, false otherwise.
         */
        bool IsEOF() const;

        /**
         * @brief Set the bookmark at the current sursor position. This can be used to read the raw string.
         */
        void SetBookmark();

        /**
         * @brief Get string from bookmark.
         * @return Returns the raw string from the last bookmark position (if set), or from the beginning of the string (if not
         * set).
         */
        std::string StringFromBookmark() const;

    private:
        /**
         * @brief Check for an out-of-bound UTF8 byte.
         * @throw XTOMLParseException Throws an InvalidByteException if the input contains for UTF-8 invalid bytes.
         */
        void CheckForInvalidUTF8Bytes() const;

        /**
         * @brief Check for an invalid UTF8 sequence.
         * @throw XTOMLParseException Throws an InvalidCharacterException if the input contains invalid UTF-8 characters.
         */
        void CheckForInvalidUTF8Sequences() const;

        /**
         * @brief Get the next character using an nOffset of nOffset bytes.
         * @attention This function doesn't protect for an invalid nOffset when reading multi-byte characters.
         * @remarks In UTF8 one character could contain up to 4 bytes.
         * @param[in] nOffset The nOffset in bytes to the next character to read.
         * @return The character from the string or an empty string if there are no more characters in the string.
         */
        std::string GetNextCharacter(size_t nOffset = 0) const;

        /**
         * @brief Get the length of the next character in bytes.
         * @remarks In UTF8 one character could contain up to 4 bytes.
         * @param[in] nOffset The nOffset in bytes to the next character to read.
         * @return the length of the character in bytes or zero when there are no more characters in the string.
         * @throw XTOMLParseException Throws an InvalidCharacterException if the input contains invalid UTF-8 characters.
         */
        size_t GetLengthOfNextCharacter(size_t nOffset = 0) const;

        static const uint8_t m_uiOneByteCheckMask{0b10000000};    ///< Checkmask for 1-Byte UTF-8 characters
        static const uint8_t m_OneByteCheckValue{0b00000000};     ///< Value of a 1-Byte UTF-8 character
                                                                  ///< after being or-ed with the checkmask
        static const uint8_t m_uiFollowByteCheckMask{0b11000000}; ///< Checkmask for followbyte of a multi-Byte UTF-8 character
        static const uint8_t m_uiFollowByteValue{0b10000000};     ///< Value of a followbyte of a multi-Byte UTF-8 character
                                                                  ///< after being or-ed with the checkmask
        static const uint8_t m_uiTwoByteCheckMask{0b11100000};    ///< Checkmask for startbyte of 2-Byte UTF-8 characters
        static const uint8_t m_uiTwoByteCheckValue{0b11000000};   ///< Value of a startbyte of a 2-Byte UTF-8 character
                                                                  ///< after being or-ed with the checkmask
        static const uint8_t m_uiThreeByteCheckMask{0b11110000};  ///< Checkmask for startbyte of 3-Byte UTF-8 characters
        static const uint8_t m_uiThreeByteCheckValue{0b11100000}; ///< Value of a startbyte of a 3-Byte UTF-8 character
                                                                  ///< after being or-ed with the checkmask
        static const uint8_t m_uiFourByteCheckMask{0b11111000};   ///< Checkmask for startbyte of 4-Byte UTF-8 characters
        static const uint8_t m_uiFourByteCheckValue{0b11110000};  ///< Value of a startbyte of a 4-Byte UTF-8 character
                                                                  ///< after being or-ed with the checkmask

        std::string m_ssString; ///< String containing the characters to acquire.
        size_t m_nCursor   = 0; ///< Current position pointing to the next character.
        size_t m_nBookmark = 0; ///< Bookmark cursor position to use to get raw string chunks.
    };
} // namespace toml_parser

#endif // CHARACTER_READER_UTF_8_H
