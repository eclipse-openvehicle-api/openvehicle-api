#include "miscellaneous.h"
#include "exception.h"
#include <sstream>
#include <limits>

namespace toml_parser
{
    std::string EscapedUnicodeCharacterToUTF8(const std::string& rss)
    {
        // Read the characters
        uint32_t uiUTFVal = HexadecimalToDecimal(rss);

        // One byte UTF-8 character
        if (uiUTFVal < 0x80)
            return std::string() + static_cast<char>(uiUTFVal);

        // Two byte UTF-8 character
        if (uiUTFVal < 0x800)
            return std::string() + static_cast<char>(uiUTFVal >> 6 | 0xc0) + static_cast<char>((uiUTFVal & 0b111111) | 0x80);

        // Three byte UTF-8 character
        if (uiUTFVal < 0x10000)
            return std::string() + static_cast<char>(uiUTFVal >> 12 | 0xe0) + static_cast<char>(((uiUTFVal >> 6) & 0b111111) | 0x80)
                   + static_cast<char>((uiUTFVal & 0b111111) | 0x80);

        // Four byte UTF-8 character
        if (uiUTFVal < 0x110000)
        {
            return std::string() + static_cast<char>(uiUTFVal >> 18 | 0xf0)
                   + static_cast<char>(((uiUTFVal >> 12) & 0b111111) | 0x80)
                   + static_cast<char>(((uiUTFVal >> 6) & 0b111111) | 0x80) + static_cast<char>((uiUTFVal & 0b111111) | 0x80);
        }

        std::stringstream sstream;
        sstream << "Invalid unicode conversion for hexadecimal number: " << rss;
        throw XTOMLParseException(sstream.str());
    }

    uint32_t HexadecimalToDecimal(const std::string& rss)
    {
        if (rss.empty()) throw XTOMLParseException("Invalid hexadecimal number interpretation for string");
        uint32_t uiVal = 0;
        bool bInitial = true;
        for (char c : rss)
        {
            if (c >= '0' && c <= '9')
                uiVal = uiVal * 16u + static_cast<uint32_t>(c - '0');
            else if (c >= 'A' && c <= 'F')
                uiVal = uiVal * 16u + static_cast<uint32_t>(c - 'A' + 10u);
            else if (c >= 'a' && c <= 'f')
                uiVal = uiVal * 16u + static_cast<uint32_t>(c - 'a' + 10u);
            else
            {
                // Not a correct value... only an error if it is the first digit
                if (bInitial)
                    throw XTOMLParseException(std::string("Invalid hexadecimal number interpretation for string: ") + rss);
                break;
            }
            bInitial = false;
            if (static_cast<uint64_t>(uiVal) * 16u > std::numeric_limits<uint32_t>().max()) break;
        }
        return uiVal;
    }

    uint32_t DecimalToDecimal(const std::string& rss)
    {
        if (rss.empty()) throw XTOMLParseException("Invalid decimal number interpretation for string");
        uint32_t uiVal = 0;
        bool bInitial = true;
        for (char c : rss)
        {
            if (c >= '0' && c <= '9')
                uiVal = uiVal * 10u + static_cast<uint32_t>(c - '0');
            else
            {
                // Not a correct value... only an error if it is the first digit
                if (bInitial)
                    throw XTOMLParseException(std::string("Invalid decimal number interpretation for string: ") + rss);
                break;
            }
            bInitial = false;
            if (static_cast<uint64_t>(uiVal) * 10u > std::numeric_limits<uint32_t>().max()) break;
        }
        return uiVal;
    }

    uint32_t OctalToDecimal(const std::string& rss)
    {
        if (rss.empty()) throw XTOMLParseException("Invalid octal number interpretation for string");
        uint32_t uiVal = 0;
        bool bInitial  = true;
        for (char c : rss)
        {
            if (c >= '0' && c <= '7')
                uiVal = uiVal * 8u + static_cast<uint32_t>(c - '0');
            else
            {
                // Not a correct value... only an error if it is the first digit
                if (bInitial)
                    throw XTOMLParseException(std::string("Invalid octal number interpretation for string: ") + rss);
                break;
            }
            bInitial = false;
            if (static_cast<uint64_t>(uiVal) * 8u > std::numeric_limits<uint32_t>().max()) break;
        }
        return uiVal;
    }

    uint32_t BinaryToDecimal(const std::string& rss)
    {
        if (rss.empty()) throw XTOMLParseException("Invalid binary number interpretation for string");
        uint32_t uiVal = 0;
        bool bInitial  = true;
        for (char c : rss)
        {
            if (c == '0')
                uiVal = uiVal * 2u;
            else if (c == '1')
                uiVal = uiVal * 2u + static_cast<uint32_t>(c - '0');
            else
            {
                // Not a correct value... only an error if it is the first digit
                if (bInitial)
                    throw XTOMLParseException(std::string("Invalid binary number interpretation for string: ") + rss);
                break;
            }
            bInitial = false;
            if (static_cast<uint64_t>(uiVal) * 2u > std::numeric_limits<uint32_t>().max()) break;
        }
        return uiVal;
    }

    std::pair<std::string, std::string> SplitNodeKey(const std::string& rssKeyPath)
    {
        if (rssKeyPath.empty())
            return {};
        enum class EType
        {
            normal,
            expect_separator,
            single_quoted_string,
            double_quoted_string,
            expect_index,
            expect_index_or_end_bracket
        } eType = EType::normal;
        size_t nPos = 0;
        std::string ssFirst;
        while (nPos < rssKeyPath.size())
        {
            char c = rssKeyPath[nPos];
            switch (c)
            {
            case ' ':
            case '\t':
                // When inside a string, add the whitespace to the key.
                if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
                {
                    ssFirst += c;
                    break;
                }
                if (eType == EType::normal && !ssFirst.empty())   // This is only allowed at the end of a key
                    eType = EType::expect_separator;
                break;
            case '\r':
            case '\n':
                if (eType == EType::normal && !ssFirst.empty()) // This is only allowed at the end of a key
                    eType = EType::expect_separator;
                break;
            case '\'':
                if (eType == EType::normal)
                {
                    if (!ssFirst.empty())
                        return {}; // Cannot be placed in the middle of the key
                    eType = EType::single_quoted_string;
                }
                else if (eType == EType::single_quoted_string)
                    eType = EType::expect_separator;
                else if (eType == EType::double_quoted_string)
                    ssFirst += c;
                else
                    return {}; // Invalid usage
                break;
            case '\"':
                if (eType == EType::normal)
                {
                    if (!ssFirst.empty())
                        return {}; // Cannot be placed in the middle of the key
                    eType = EType::double_quoted_string;
                }
                else if (eType == EType::double_quoted_string)
                    eType = EType::expect_separator;
                else if (eType == EType::single_quoted_string)
                    ssFirst += c;
                else
                    return {}; // Invalid usage
                break;
            case '\\':
                if (eType == EType::single_quoted_string)
                {
                    ssFirst += c;
                    break;
                }
                if (eType != EType::double_quoted_string) return {};    // Only allowed for double quoted strings.
                nPos++;
                if (nPos >= rssKeyPath.size()) return {};   // Ended with escape...
                try
                {
                    switch (rssKeyPath[nPos])
                    {
                    case 'b':       ssFirst += '\b';     break;
                    case 't':       ssFirst += '\t';     break;
                    case 'n':       ssFirst += '\n';     break;
                    case 'f':       ssFirst += '\f';     break;
                    case 'r':       ssFirst += '\r';     break;
                    case '"':       ssFirst += '\"';     break;
                    case '\\':      ssFirst += '\\';     break;
                    case 'u':       ssFirst += EscapedUnicodeCharacterToUTF8(rssKeyPath.substr(nPos + 1, 4));   nPos += 4;  break;
                    case 'U':       ssFirst += EscapedUnicodeCharacterToUTF8(rssKeyPath.substr(nPos + 1, 8));   nPos += 8;  break;
                    default:
                        return {};  // Invalid escape sequence
                    }
                }
                catch (const sdv::toml::XTOMLParseException&)
                {
                    return {};
                }
                break;
            case '.':
                if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
                {
                    ssFirst += c;
                    break;
                }
                if (ssFirst.empty()) break; // Ignore an initial dot.
                if (eType != EType::normal && eType != EType::expect_separator)
                    return {};  // Unexpected separator
                return std::make_pair(ssFirst, rssKeyPath.substr(nPos + 1));
            case '[':
                if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
                {
                    ssFirst += c;
                    break;
                }
                if (eType != EType::normal && eType != EType::expect_separator)
                    return {}; // Unexpected separator
                if (!ssFirst.empty()) // Belongs to second part?
                    return std::make_pair(ssFirst, rssKeyPath.substr(nPos));
                eType  = EType::expect_index;
                break;
            case ']':
                if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
                {
                    ssFirst += c;
                    break;
                }
                if (eType != EType::expect_index_or_end_bracket) // Allowed?
                    return {};
                eType = EType::expect_separator;   // Expected a separator to follow
                break;
            default:
                if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
                    ssFirst += c;
                else if ((eType == EType::expect_index || eType == EType::expect_index_or_end_bracket) && std::isdigit(c))
                {
                    ssFirst += c;
                    eType = EType::expect_index_or_end_bracket;
                }
                // Protect against multi-byte characters (UTF-8)
                else if (eType == EType::normal && static_cast<uint8_t>(c) < 127u && (std::isalnum(c) || c == '-' || c == '_'))
                    ssFirst += c;
                else
                    return {}; // Invalid key character
                break;
            }
            nPos++;
        }

        // When within quotes, this is an error
        if (eType == EType::single_quoted_string || eType == EType::double_quoted_string)
            return {};

        // When expecting index and/or end bracket, this is an error
        if (eType == EType::expect_index || eType == EType::expect_index_or_end_bracket)
            return {};

        // When coming here, there is no more character in the string.
        return std::make_pair(ssFirst, std::string());
    }

    std::pair<std::reference_wrapper<const CToken>, CTokenRange> SplitNodeKey(const CTokenRange& rrangeKeyPath)
    {
        std::reference_wrapper<const CToken> refToken = rrangeKeyPath.Begin();
        std::reference_wrapper<const CToken> refTokenFirst = refToken;
        enum class EState
        {
            initial_key_or_index_marker,
            dot_or_index_marker_or_end,
            key,
            index,
            index_marker_close,
        } eState = EState::initial_key_or_index_marker;
        while (refToken.get() != rrangeKeyPath.End())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_comment:
            case ETokenCategory::token_syntax_new_line:
                break;
            case ETokenCategory::token_key:
                if (eState == EState::initial_key_or_index_marker)
                    refTokenFirst = refToken;
                else if (eState == EState::key)
                    return std::make_pair(refTokenFirst, CTokenRange(refToken.get(), rrangeKeyPath.End()));
                else
                    throw XTOMLParseException("Internal error: invalid token range, expecting key.");
                eState = EState::dot_or_index_marker_or_end;
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::dot_or_index_marker_or_end)
                    throw XTOMLParseException("Internal error: invalid token range, expecting dot.");
                eState = EState::key;
                break;
            case ETokenCategory::token_syntax_array_open:
                if (eState == EState::initial_key_or_index_marker)
                    eState = EState::index;
                else if (eState == EState::dot_or_index_marker_or_end)
                    return std::make_pair(refTokenFirst, CTokenRange(refToken, rrangeKeyPath.End()));
                else
                    throw XTOMLParseException("Internal error: invalid token range, unexpected array open.");
                break;
            case ETokenCategory::token_integer:
                if (eState != EState::index)
                    throw XTOMLParseException("Internal error: invalid token range, unexpected index.");
                refTokenFirst = refToken;
                eState = EState::index_marker_close;
                break;
            case ETokenCategory::token_syntax_array_close:
                if (eState != EState::index_marker_close)
                    throw XTOMLParseException("Internal error: invalid token range, unexpected array close.");
                eState = EState::dot_or_index_marker_or_end;
                break;
            default:
                throw XTOMLParseException("Internal error: invalid token range, unexpected token.");
            }
            refToken = refToken.get().Next();
        }

        // Coming here would mean that there was one key, but nothing more
        if (eState != EState::dot_or_index_marker_or_end)
            throw XTOMLParseException("Internal error: invalid token range, unexpected end.");

        // Return a pair with the first token and an empty token range.
        return std::make_pair(refTokenFirst, CTokenRange(refToken.get(), refToken.get()));
    }

    std::string ExtractKeyName(const std::string& rssKeyPath)
    {
        // Split the key parh until there is no second part any more.
        auto prSplittedKey = SplitNodeKey(rssKeyPath);
        while (!prSplittedKey.second.empty())
            prSplittedKey = SplitNodeKey(prSplittedKey.second);
        return prSplittedKey.first;
    }

    std::string QuoteText(const std::string& rssText, EQuoteRequest eQuoteRequest /*= EQuoteRequest::smart_key*/)
    {
        EQuoteRequest eQuoteNeeded = eQuoteRequest;
        if (rssText.empty() && eQuoteNeeded == EQuoteRequest::smart_key)
            eQuoteNeeded = EQuoteRequest::quoted_text;
        std::stringstream sstreamQuotedText;
        sstreamQuotedText << "\"";
        for (size_t nPos = 0; nPos < rssText.size(); nPos++)
        {
            uint8_t uiChar     = static_cast<uint8_t>(rssText[nPos]);
            uint32_t uiUTFChar = 0;
            switch (uiChar)
            {
            case '\'':
                if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text &&
                    eQuoteRequest != EQuoteRequest::multi_line_literal_text)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                sstreamQuotedText << '\'';
                break; // Single quote character
            case '\b':
                if (eQuoteRequest == EQuoteRequest::multi_line_literal_text)
                    eQuoteNeeded = EQuoteRequest::multi_line_quoted_text;
                else if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                sstreamQuotedText << "\\b";
                break; // Escape backspace
            case '\t':
                if (eQuoteRequest == EQuoteRequest::multi_line_literal_text)
                    eQuoteNeeded = EQuoteRequest::multi_line_quoted_text;
                else if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                sstreamQuotedText << "\\t";
                break; // Escape tab
            case '\n':
                if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text &&
                    eQuoteRequest != EQuoteRequest::multi_line_literal_text)
                {
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                    sstreamQuotedText << "\\n";
                }
                else
                    sstreamQuotedText << "\n";
                break; // Escape linefeed
            case '\f':
                if (eQuoteRequest == EQuoteRequest::multi_line_literal_text)
                    eQuoteNeeded = EQuoteRequest::multi_line_quoted_text;
                else if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                sstreamQuotedText << "\\f";
                break; // Escape form feed
            case '\r':
                if (eQuoteRequest != EQuoteRequest::multi_line_quoted_text &&
                    eQuoteRequest != EQuoteRequest::multi_line_literal_text)
                {
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                    sstreamQuotedText << "\\r";
                } else
                    sstreamQuotedText << "\r";
                break; // Escape carriage return
            case '\"':
                if (eQuoteNeeded == EQuoteRequest::smart_key)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                else if (eQuoteNeeded == EQuoteRequest::smart_text)
                    eQuoteNeeded = EQuoteRequest::literal_text;
                sstreamQuotedText << "\\\"";
                break; // Escape quote
            case '\\':
                if (eQuoteNeeded == EQuoteRequest::smart_key)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                else if (eQuoteNeeded == EQuoteRequest::smart_text)
                    eQuoteNeeded = EQuoteRequest::literal_text;
                sstreamQuotedText << "\\\\";
                break; // Escape backslash
            default:
                // Check for ASCII character
                if (uiChar >= 0x20 && uiChar < 0x7f)
                {
                    // Standard ASCII
                    sstreamQuotedText << static_cast<char>(uiChar);
                    if (!std::isalnum(static_cast<char>(uiChar)) && uiChar != '-' && uiChar != '_')
                    {
                        if (eQuoteNeeded == EQuoteRequest::smart_key || eQuoteNeeded == EQuoteRequest::smart_text)
                            eQuoteNeeded = EQuoteRequest::quoted_text;
                    }
                    break;
                }

                // Not a standard ASCII character
                if (eQuoteNeeded == EQuoteRequest::smart_key || eQuoteNeeded == EQuoteRequest::smart_text)
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                else if (eQuoteNeeded == EQuoteRequest::literal_text && (uiChar < 0x20 || uiChar == 0x7f))
                {
                    // If the character is one of the control characters (< 0x20 || == 0x7f) then must be quoted
                    eQuoteNeeded = EQuoteRequest::quoted_text;
                }
                else if (eQuoteNeeded == EQuoteRequest::multi_line_literal_text && (uiChar < 0x20 || uiChar == 0x7f))
                {
                    // If the character is one of the control characters (< 0x20 || == 0x7f) then must be quoted
                    eQuoteNeeded = EQuoteRequest::multi_line_quoted_text;
                }

                // Use UNICODE escape character for the quoted text
                if (uiChar <= 0x80) // One byte UTF-8
                    uiUTFChar = static_cast<uint32_t>(uiChar);
                else if (uiChar <= 0xDF) // Two bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00011111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }
                else if (uiChar <= 0xEF) // Three bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00001111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }
                else if (uiChar <= 0xF7) // Four bytes UTF-8
                {
                    uiUTFChar = static_cast<size_t>(uiChar & 0b00000111) << 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                    uiUTFChar <<= 6;

                    // Expecting the next character to be between 0x80 and 0xBF
                    nPos++;
                    if (nPos >= rssText.size())
                        break;
                    uiUTFChar |= static_cast<size_t>(rssText[nPos] & 0b00111111);
                }

                // Stream the UTF character
                if (uiUTFChar <= 0xFFFF)
                    sstreamQuotedText << "\\u" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << uiUTFChar;
                else
                    sstreamQuotedText << "\\U" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << uiUTFChar;
                break;
            }
        }
        sstreamQuotedText << "\"";

        // Return depedent of the needed quotation.
        switch (eQuoteNeeded)
        {
        case EQuoteRequest::smart_key:
            return rssText; // No need to change
        case EQuoteRequest::literal_text:
            return "'" + rssText + "'";
        case EQuoteRequest::multi_line_literal_text:
            return "'''" + rssText + "'''";
        case EQuoteRequest::multi_line_quoted_text:
            return "\"\"" + sstreamQuotedText.str() + "\"\"";
        case EQuoteRequest::smart_text:
        case EQuoteRequest::quoted_text:
        default:
            return sstreamQuotedText.str();
        }
    }
} // namespace toml_parser