#ifndef SUPPORT_H
#define SUPPORT_H

#include "exception.h"
#include "codepos.h"
#include <utility>
#include <sstream>

/**
 * @brief Get the characters within a string or character (sequence) literal.
 * @details Get the characters for a string or a character sequence (a single character is a character sequence of one
 * character). The characters in the string that are represented through escape sequences are interpreted. This includes C-style
 * string standard characters used for whitespace and quotation markers as well as special Unicode characters only presented by
 * Unicode points within the string. Unicode codepoints could be needing more than one physical character to be represented.
 * @tparam TLiteral The type of the variable holding the literal (either const char* or CCodePos).
 * @tparam TCharType The character type to use for the text generation.
 * @param[in, out] rtLiteral The literal text to get the characters for (not including the prefix and quote symbol).
 * @param[in] szDelim Delimiter to use for detection of the end of the string (will not be included in the result). If nullptr
 * interprets the complete string.
 * @param[out] rssText Reference to the string receiving the characters.
 * @param[out] ruiByteCnt Reference to the variable receiving the byte count.
 * @param[in] bIgnoreEscape When set, do not interpret escape characters.
 * @param[in] bNotUnicode When set, the encoding is ASCII. Only valid with 'char' data type.
 * @throws Can throw a CCompileException exception if invalid combinations where detected.
 * @returns Returns a vector with the characters that have been read and interpreted.
 */
template <typename TLiteral, typename TCharType>
inline void InterpretCText(TLiteral rtLiteral, const char* szDelim, std::basic_string<TCharType>& rssText, uint32_t& ruiByteCnt,
    bool bIgnoreEscape = false, bool bNotUnicode = false)
{
    if (!rtLiteral)
        throw CCompileException("Internal error: invalid parameter.");
    TLiteral rtLiteralLocal = rtLiteral;    // Prevent changing the local variable (some compilers don't like this).
    rssText.clear();
    ruiByteCnt = 0;

    // Check to see an incomptiblity with the parameter 'bNotUnicode'.
    if (bNotUnicode && sizeof(TCharType) != 1)
        throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Internal error: invalid parameter.");

    // Get the octal number for correct presentation.
    auto fnGetOctalChar = [&]()
    {
        // Octal number of three digits not exceeding the character capacity
        std::string ssNumber;
        size_t n = 0;
        do
        {
            if (*rtLiteralLocal < '0' || *rtLiteralLocal > '7')
                throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(),
                                      "Invalid escape sequence in string; expecting a valid octal number.");
            ssNumber += *rtLiteralLocal;
            ruiByteCnt++;
            rtLiteralLocal++;
        } while (++n < 3 && *rtLiteralLocal >= '0' && *rtLiteralLocal <= '7');
        uint32_t uiResult = static_cast<uint32_t>(std::stoul(ssNumber, nullptr, 8));

        // Byte character literals cannot have a value higher than 0377 (0xff).
        if (uiResult > 0377)
            throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(),
                                  "Invalid escape sequence in string; expecting a valid octal number of at the most 0377.");

        return uiResult;
    };

    // Get the hexadecimal number for correct presentation.
    auto fnGetHexChar = [&](size_t nFixedDigits = 0)
    {
        // Determine the amount of digits
        size_t nFixedDigitsLocal = nFixedDigits ? nFixedDigits : sizeof(TCharType) * 2;

        // Check the amount of digits
        std::string ssNumber;
        size_t n = 0;
        do
        {
            if (!std::isxdigit(*rtLiteralLocal))
                throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; expecting a "
                                                           "valid hexadecimal number of ", nFixedDigitsLocal, " digits.");
            ssNumber += *rtLiteralLocal;
            rtLiteralLocal++;
            ruiByteCnt++;
        } while (++n < nFixedDigitsLocal && std::isxdigit(*rtLiteralLocal));
        if (nFixedDigits && n != nFixedDigits)
                throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; expecting a "
                                                           "valid hexadecimal number of ", nFixedDigitsLocal, " digits.");

        return static_cast<uint32_t>(std::stoul(ssNumber, nullptr, 16));
    };

    // Check for the correct unicode presentation and add the characters to the vector.
    auto fnAddUnicodeCharacter = [&](bool b32BitCodePoint = false)
    {
        uint32_t uiCodePoint = fnGetHexChar(b32BitCodePoint ? 8 : 4);

        if (uiCodePoint >= 0xD800 && uiCodePoint < 0xE000) // Range U+D800...U+DFFF are reserved and not assigned any character.
        throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; Unicode range"
            " U+D800...U+DFFF are not valid code point.");

        // Check whether the number fits (wide characters are considered to correspond to a UTF-16 or UTF-32
        // character).
        if constexpr (sizeof(TCharType) == 4) // UTF-32
            rssText.push_back(uiCodePoint);
        else if constexpr (sizeof(TCharType) == 2) // UTF-16
        {
            if (uiCodePoint < 0xD800)   // Range U+0000...U+D7FF fit in 16 bits.
                rssText.push_back(static_cast<TCharType>(uiCodePoint));
            else if (uiCodePoint >= 0xE000 && uiCodePoint < 0x10000)    // Range U+E000...U+FFFF fit in 16 bits.
                rssText.push_back(static_cast<TCharType>(uiCodePoint));
            else if (uiCodePoint >= 0x10000 && uiCodePoint < 0x110000)   // Range U+10000...U+10FFFF fit in 2X 16 bits.
            {
                // Subtract 0x10000 from the code point
                uiCodePoint -= 0x10000;
                // High ten bits form the first character
                rssText.push_back(static_cast<TCharType>(0xD800 | ((uiCodePoint >> 10) & 0x3ff)));
                // Low ten bits form the second character
                rssText.push_back(static_cast<TCharType>(0xDC00 | (uiCodePoint & 0x3FF)));
            }
            else if (uiCodePoint >= 0x110000)    // Range above U+10FFFF are invalid.
                throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; expecting a"
                    " valid Unicode code point.");
        }
        else if constexpr (sizeof(TCharType) == 1) // ASCII and UTF-8
        {
            if (uiCodePoint < 0x0080)   // Range between U+0000 and U+007F fit in 8 bits.
                rssText.push_back(static_cast<TCharType>(uiCodePoint));
            else if (/*uiCodePoint >= 0x0080 &&*/ uiCodePoint < 0x009F)  // Range U+0080...U+009F fit in 2X 8 bits (only UTF-8).
            {
                if (bNotUnicode)
                    throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; Unicode"
                    " code point doesn't fit into ASCII character.");
                // High 5 bits form the first character
                rssText.push_back(static_cast<TCharType>(0xC0 | ((uiCodePoint >> 6) & 0x1f)));
                // Low 6 bits form the second character
                rssText.push_back(static_cast<TCharType>(0x80 | (uiCodePoint & 0x3F)));
            }
            else if (/*uiCodePoint >= 0x00A0 &&*/ uiCodePoint < 0x0100)  // Range U+00A0...U+00FF fit in 2X 8 bits or 1X 8 bits.
            {
                if (bNotUnicode)    // ASCII character
                    rssText.push_back(static_cast<TCharType>(uiCodePoint));
                else
                {
                    // High 5 bits form the first character
                    rssText.push_back(static_cast<TCharType>(0xC0 | ((uiCodePoint >> 6) & 0x1f)));
                    // Low 6 bits form the second character
                    rssText.push_back(static_cast<TCharType>(0x80 | (uiCodePoint & 0x3F)));
                }
            }
            else if (/*uiCodePoint >= 0x0100 &&*/ uiCodePoint < 0x0800)  // Range U+0100...U+07FF fit in 2X 8 bits (only UTF-8).
            {
                if (bNotUnicode)
                    throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; Unicode"
                    " code point doesn't fit into ASCII character.");
                // High 5 bits form the first character
                rssText.push_back(static_cast<TCharType>(0xC0 | ((uiCodePoint >> 6) & 0x1f)));
                // Low 6 bits form the second character
                rssText.push_back(static_cast<TCharType>(0x80 | (uiCodePoint & 0x3F)));
            }
            else if (/*uiCodePoint >= 0x0800 &&*/ uiCodePoint < 0xFFFF)  // Range U+0800...U+FFFF fit in 3X 8 bits (only UTF-8).
            {
                if (bNotUnicode)
                    throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; Unicode"
                    " code point doesn't fit into ASCII character.");
                // High 4 bits form the first character
                rssText.push_back(static_cast<TCharType>(0xE0 | ((uiCodePoint >> 12) & 0x0f)));
                // Next 6 bits form the second character
                rssText.push_back(static_cast<TCharType>(0x80 | ((uiCodePoint >> 6) & 0x3f)));
                // Low 6 bits form the third character
                rssText.push_back(static_cast<TCharType>(0x80 | (uiCodePoint & 0x3F)));
            }
            else if (/*uiCodePoint >= 0x01000 &&*/ uiCodePoint < 0x10FFFF)  // Range U+10000...U+10FFFF fit in 4x 8 bits (only UTF-8).
            {
                if (bNotUnicode)
                    throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; Unicode"
                    " code point doesn't fit into ASCII character.");
                // High 3 bits form the first character
                rssText.push_back(static_cast<TCharType>(0xF0 | ((uiCodePoint >> 18) & 0x07)));
                // Next 6 bits form the second character
                rssText.push_back(static_cast<TCharType>(0x80 | ((uiCodePoint >> 12) & 0x3f)));
                // Next 6 bits form the second character
                rssText.push_back(static_cast<TCharType>(0x80 | ((uiCodePoint >> 6) & 0x3f)));
                // Low 6 bits form the third character
                rssText.push_back(static_cast<TCharType>(0x80 | (uiCodePoint & 0x3F)));
            }
            else if (uiCodePoint >= 0x110000) // Range above U+10FFF are invalid.
                throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string; expecting a"
                    " valid Unicode code point.");
        }
    };

    // Check whether to continue processing based on the delimiter.
    auto fnContinueProcessing = [&]() -> bool
    {
        // In case there is no delimiter, end processing when the literal is '\0'.
        if (!szDelim) return *rtLiteralLocal ? true : false;

        // There is a delimiter. Continue processing when the delimiter pattern hasn't been detected.
        uint32_t uiIndex = 0;
        do
        {
            if (szDelim[uiIndex] != rtLiteralLocal[uiIndex])
                return true;
        } while (szDelim[++uiIndex] != '\0');
        return false;
    };

    while (fnContinueProcessing())
    {
        switch (*rtLiteralLocal)
        {
        case '\0':
            throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid string; unexpected end of file detected.");
            break;
        case '\\':
            // Skip backslash
            rtLiteralLocal++;
            ruiByteCnt++;

            // Handle as escape or as character
            if (bIgnoreEscape)
            {
                rssText.push_back('\\');
                break;
            }

            // Handle escape
            if (std::isdigit(*rtLiteralLocal))
            {
                // Octal character value
                rssText.push_back(static_cast<TCharType>(fnGetOctalChar()));
            }
            else
            {
                char cEscapeChar = *rtLiteralLocal;
                rtLiteralLocal++;
                ruiByteCnt++;
                switch (cEscapeChar)
                {
                case '\'': // Single quote
                case '\"': // Double quote
                case '?':  // Question mark
                case '\\': // Backslash
                    rssText.push_back(cEscapeChar);
                    break;
                case 'a':  // Bell
                    rssText.push_back('\a');
                    break;
                case 'b':  // Backspace
                    rssText.push_back('\b');
                    break;
                case 'f':  // Form feed
                    rssText.push_back('\f');
                    break;
                case 'n':  // Line feed
                    rssText.push_back('\n');
                    break;
                case 'r':  // Carriage return
                    rssText.push_back('\r');
                    break;
                case 't':  // Horizontal tab
                    rssText.push_back('\t');
                    break;
                case 'v':  // Vertical tab
                    rssText.push_back('\v');
                    break;
                case 'x': // Hex character value
                    rssText.push_back(static_cast<TCharType>(fnGetHexChar()));
                    break;
                case 'u': // 4 digit Unicode character
                    fnAddUnicodeCharacter();
                    break;
                case 'U': // 8 digit Unicode character
                    fnAddUnicodeCharacter(true);
                    break;
                default:
                    throw CCompileException(CCodePos(rtLiteralLocal).GetLocation(), "Invalid escape sequence in string");
                }
            }
            break;
        default:
            // Skip char
            rssText.push_back(*rtLiteralLocal);
            rtLiteralLocal++;
            ruiByteCnt++;
            break;
        }
    }
}

/**
 * @brief Generate an ASCII C/C++ character string from a text with zero or more characters.
 * @tparam TCharType The character type to use for the text interpretation.
 * @param[in] szText The text to translate into C/C++ style text (inserting escape characters if necessary).
 * @param[in] uiLen The length of the text or 0xFFFFFFFF when to use a zero terminated string.
 * @param[in] bNotUnicode When set, the encoding is ASCII. Only valid with 'char' data type.
 * @return std::string
 */
template <typename TCharType>
inline std::string GenerateCText(const TCharType* szText, uint32_t uiLen = 0xFFFFFFFF, bool bNotUnicode = false)
{
    std::stringstream sstreamResult;
    if (!szText) return sstreamResult.str();

    // Check to see an incomptiblity with the parameter 'bNotUnicode'.
    if (bNotUnicode && sizeof(TCharType) != 1)
        throw CCompileException("Internal error: invalid parameter.");

    // Standard ASCII code generation (until the value of 128)
    auto fnGenerateASCII = [&](uint32_t c) -> bool
    {
        // Check for low level...
        switch (c)
        {
        case '\'':          sstreamResult << "\\\'";     break;
        case '\"':          sstreamResult << "\\\"";     break;
        case '\\':          sstreamResult << "\\\\";     break;
        case '\a':          sstreamResult << "\\a";      break;
        case '\b':          sstreamResult << "\\b";      break;
        case '\f':          sstreamResult << "\\f";      break;
        case '\n':          sstreamResult << "\\n";      break;
        case '\r':          sstreamResult << "\\r";      break;
        case '\t':          sstreamResult << "\\t";      break;
        case '\v':          sstreamResult << "\\v";      break;
        default:
            if (c < 0x20)
                sstreamResult << "\\" << std::oct << c;
            else if (c < 0x80)
                sstreamResult << static_cast<char>(c);
            else return false;
        }
        return true;
    };

    const TCharType* szTextPos = szText;
    size_t n = 0;
    if constexpr (sizeof(TCharType) == 4) // UTF-32 and possibly wide character
    {
        while (*szTextPos && n++ < uiLen)
        {
            uint32_t uiChar = static_cast<uint32_t>(*szTextPos);
            if (!fnGenerateASCII(uiChar))
            {
                // Generate an UNICODE character
                if (uiChar < 0x10000)
                    sstreamResult << "\\u" << std::hex << std::setfill('0') << std::setw(4) << uiChar;
                else
                    sstreamResult << "\\U" << std::hex << std::setfill('0') << std::setw(8) << uiChar;
            }
            szTextPos++;
        }
    } else if constexpr (sizeof(TCharType) == 2) // UTF-16 and possibly wide character
    {
        while (*szTextPos && n++ < uiLen)
        {
            uint16_t uiChar = static_cast<uint16_t>(*szTextPos);
            if (!fnGenerateASCII(uiChar))
            {
                // Generate an UNICODE character
                if (uiChar < 0xD800 || uiChar >= 0xE000) // One character
                    sstreamResult << "\\u" << std::hex << std::setfill('0') << std::setw(4) << static_cast<uint32_t>(uiChar);
                else // Two characters
                {
                    uint32_t uiUnicodeCodePoint = 0x10000 + ((static_cast<uint32_t>(uiChar) & 0x3ff) << 10);
                    szTextPos++;
                    uiChar = static_cast<uint16_t>(*szTextPos);
                    uiUnicodeCodePoint |= static_cast<uint32_t>(uiChar) & 0x3ff;
                    sstreamResult << "\\U" << std::hex << std::setfill('0') << std::setw(8) << uiUnicodeCodePoint;
                }
            }
            szTextPos++;
        }
    }
    else // UTF-8 or ASCII
    {
        while (*szTextPos && n++ < uiLen)
        {
            uint8_t uiChar = static_cast<uint8_t>(*szTextPos);
            if (!fnGenerateASCII(static_cast<uint8_t>(uiChar)))
            {
                // Generate ASCII character
                if (bNotUnicode)
                {
                    sstreamResult << "\\" << std::oct << static_cast<uint32_t>(uiChar);
                } else // Generate an UNICODE character
                {
                    if (uiChar < 0x80)  // One character
                        sstreamResult << "\\u" << std::hex << std::setfill('0') << std::setw(4) << static_cast<uint32_t>(uiChar);
                    else if (uiChar < 0xE0) // Two characters
                    {
                        uint32_t uiUnicodeCodePoint = ((static_cast<uint32_t>(uiChar) & ~0xE0) << 6);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= static_cast<uint32_t>(uiChar) & ~0xC0;
                        sstreamResult << "\\u" << std::hex << std::setfill('0') << std::setw(4) << uiUnicodeCodePoint;
                    }
                    else if (/*uiChar >= 0xE0 &&*/ uiChar < 0xF0) // Three characters
                    {
                        uint32_t uiUnicodeCodePoint = ((static_cast<uint32_t>(uiChar) & ~0xF0) << 12);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= ((static_cast<uint32_t>(uiChar) & ~0xC0) << 6);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= static_cast<uint32_t>(uiChar) & ~0xC0;
                        sstreamResult << "\\u" << std::hex << std::setfill('0') << std::setw(4) << uiUnicodeCodePoint;
                    }
                    else /*if (uiChar >= 0xF0)*/ // Four characters
                    {
                        uint32_t uiUnicodeCodePoint = ((static_cast<uint32_t>(uiChar) & ~0xF0) << 18);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= ((static_cast<uint32_t>(uiChar) & ~0xC0) << 12);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= ((static_cast<uint32_t>(uiChar) & ~0xC0) << 6);
                        szTextPos++;
                        uiChar = static_cast<uint8_t>(*szTextPos);
                        uiUnicodeCodePoint |= static_cast<uint32_t>(uiChar) & ~0xC0;
                        sstreamResult << "\\U" << std::hex << std::setfill('0') << std::setw(8) << uiUnicodeCodePoint;
                    }
                }
            }
            szTextPos++;
        }
    }
    return sstreamResult.str();
}

/**
 * @{
 * @brief Generate an ASCII C/C++ character string from a text with zero or more characters.
 * @tparam TCharType The character type to use for the text interpretation.
 * @param[in] cChar The character to translate into C/C++ style text (inserting escape characters if necessary).
 * @param[in] bNotUnicode When set, the encoding is ASCII. Only valid with 'char' data type.
 * @return std::string
 */
inline std::string GenerateCText(char cChar, bool bNotUnicode = false)
{
    return GenerateCText(&cChar, 1, bNotUnicode);
}
inline std::string GenerateCText(char16_t cChar, bool bNotUnicode = false)
{
    return GenerateCText(&cChar, 1, bNotUnicode);
}
inline std::string GenerateCText(char32_t cChar, bool bNotUnicode = false)
{
    return GenerateCText(&cChar, 1, bNotUnicode);
}
inline std::string GenerateCText(wchar_t cChar, bool bNotUnicode = false)
{
    return GenerateCText(&cChar, 1, bNotUnicode);
}
/**
 * @}
 */

#endif // !defined(SUPPORT_H)