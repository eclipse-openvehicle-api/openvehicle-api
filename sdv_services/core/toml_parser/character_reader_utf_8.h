#ifndef CHARACTER_READER_UTF_8_H
#define CHARACTER_READER_UTF_8_H

#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdint>

/**
 * @brief Reads a given input string, interprets bytes as UTF-8 and returns UTF-8 characters or strings in order on
 * demand
 */
class CCharacterReaderUTF8
{
public:
    /**
     * @brief Standard constructor for empty character reader
     */
    CCharacterReaderUTF8();

    /**
     * @brief Constructs a character reader from a given string
     * @param[in] rssString UTF-8 input string.
     * @throw InvalidCharacterException Throws an InvalidCharacterException if the input contains invalid UTF-8 characters
     * @throw InvalidByteException Throws an InvalidByteException if the input contains for UTF-8 invalid bytes
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
     * @brief Gets the next UTF-8 character without advancing the cursor
     * @return Returns the next UTF-8 character after the current cursor or an empty string if the cursor is at the
     * end
     */
    std::string Peek();

    /**
     * @brief Gets the next n-th UTF-8 character without advancing the cursor
     * @param[in] n Step size
     * @return Returns the n-th UTF-8 character after the current cursor or an empty string if n<1 or it would read
     * after the last character
     */
    std::string Peek(std::size_t n);

    /**
     * @brief Gets all upcoming UTF-8 characters until a terminating character without advancing the cursor
     * @param[in] lstCollection A collection of terminating characters
     * @return Returns a string of UTF-8 characters until (excluding) the first occurrence of one of the given
     * characters
     */
    std::string PeekUntil(const std::vector<std::string>& lstCollection);

    /**
     * @brief Gets the next UTF-8 character and advancing the cursor by one
     * @return Returns the next UTF-8 character after the current cursor or an empty string if the cursor is at the
     * end
     */
    std::string Consume();

    /**
     * @brief Gets the next n-th UTF-8 character and advancing the cursor by n
     * @param[in] n Step size
     * @return Returns the n-th UTF-8 character after the current cursor or an empty string if n<1 or it would read
     * after the last character
     */
    std::string Consume(std::size_t n);

    /**
     * @brief Gets all upcoming UTF-8 characters until a terminating character and advancing the cursor by the number
     * of characters in the returned string
     * @param[in] lstCollection A collection of terminating characters
     * @return Returns a string of UTF-8 characters until excludingg) the first occurrence of one of the given
     * characters
     */
    std::string ConsumeUntil(const std::vector<std::string>& lstCollection);

    /**
     * @brief Checks if the cursor is at the end of the data to read
     * @return Returns true if the cursor is at the end of the readable data, false otherwise
     */
    bool IsEOF() const;

private:
    void CheckForInvalidUTF8Bytes() const;

    void CheckForInvalidUTF8Sequences() const;

    std::string GetNextCharacter();

    std::string GetNextCharacter(std::size_t offset);

    size_t GetLengthOfNextCharacter() const;

    size_t GetLengthOfNextCharacter(std::size_t offset) const;

    static const uint8_t m_uiOneByteCheckMask{0b10000000};  //!< Checkmask for 1-Byte UTF-8 characters
    static const uint8_t m_OneByteCheckValue{0b00000000}; //!< Value of a 1-Byte UTF-8 character
                                                          //!< after being or-ed with the checkmask
    static const uint8_t m_uiFollowByteCheckMask{0b11000000}; //!< Checkmask for followbyte of a multi-Byte UTF-8 character
    static const uint8_t m_uiFollowByteValue{0b10000000};	  //!< Value of a followbyte of a multi-Byte UTF-8 character
                                                          //!< after being or-ed with the checkmask
    static const uint8_t m_uiTwoByteCheckMask{0b11100000};  //!< Checkmask for startbyte of 2-Byte UTF-8 characters
    static const uint8_t m_uiTwoByteCheckValue{0b11000000}; //!< Value of a startbyte of a 2-Byte UTF-8 character
                                                          //!< after being or-ed with the checkmask
    static const uint8_t m_uiThreeByteCheckMask{0b11110000};	//!< Checkmask for startbyte of 3-Byte UTF-8 characters
    static const uint8_t m_uiThreeByteCheckValue{0b11100000}; //!< Value of a startbyte of a 3-Byte UTF-8 character
                                                            //!< after being or-ed with the checkmask
    static const uint8_t m_uiFourByteCheckMask{0b11111000};	//!< Checkmask for startbyte of 4-Byte UTF-8 characters
    static const uint8_t m_uiFourByteCheckValue{0b11110000};	//!< Value of a startbyte of a 4-Byte UTF-8 character
                                                            //!< after being or-ed with the checkmask

    std::string m_ssString;
    std::size_t m_nDataLength;
    std::size_t m_nCursor;
};

#endif // CHARACTER_READER_UTF_8_H
