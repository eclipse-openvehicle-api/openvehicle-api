#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

#include <string>
#include <cstdint>
#include <cctype>
#include <utility>
#include "lexer_toml_token.h"

/// The TOML parser namespace
namespace toml_parser
{
    /**
     * @brief Implementation of hexadecimal number to UTF8 unicode character.
     * @param[in] rss Reference to the string providing the hexadecimal unicode number.
     * @return The interpreted unicode character.
     */
    std::string EscapedUnicodeCharacterToUTF8(const std::string& rss);

    /**
     * @brief Calculate the interpretation value of a hexadecimal number in a string (characters 0-9a-fA-F).
     * @param[in] rss Reference to the string containing the number.
     * @return The calculated value.
     */
    uint32_t HexadecimalToDecimal(const std::string& rss);

    /**
     * @brief Calculate the interpretation value of a decimal number in a string (characters 0-9).
     * @param[in] rss Reference to the string containing the number.
     * @return The calculated value.
     */
    uint32_t DecimalToDecimal(const std::string& rss);

    /**
     * @brief Calculate the interpretation value of an octal number in a string (characters 0-7).
     * @param[in] rss Reference to the string containing the number.
     * @return The calculated value.
     */
    uint32_t OctalToDecimal(const std::string& rss);

    /**
     * @brief Calculate the interpretation value of a binary number in a string (characters 0-1).
     * @param[in] rss Reference to the string containing the number.
     * @return The calculated value.
     */
    uint32_t BinaryToDecimal(const std::string& rss);

    /**
     * @brief Split the node key in a first node key and the rest of the node key (if available). Separators are the dot, for table
     * separation and [] index for array separation.
     * @param[in] rssKeyPath Reference to the string containing the key path.
     * @return Pair of string containing the first key (unquoted) and the rest of the key string (still quoted) if applicable. If
     * no key is available, will be a reference to two empty strings. The dot and the indexing markers will not be part of the
     * returned first string.
     */
    std::pair<std::string, std::string> SplitNodeKey(const std::string& rssKeyPath);

    /**
     * @brief Split the node key contained in the token range in a first node key and the rest of the node key (if available).
     * Separators are the dot, for table separation and [] index for array separation.
     * @param[in] rrangeKeyPath Reference to the token range ccontaining the tokens representing the key path.
     * @return Pair of string containing the first key (unquoted) and the rest of the key string (still quoted) if applicable. If
     * no key is available, will be a reference to two empty strings. The dot and the indexing markers will not be part of the
     * returned token.
     */
    std::pair<std::reference_wrapper<const CToken>, CTokenRange> SplitNodeKey(const CTokenRange& rrangeKeyPath);

    /**
     * @brief Extract the unquoted key name from a key path. The key name is the last node on the key path.
     * @remarks Removes quotes, escape characters and index markers from the key name before returning.
     * @param[in] rssKeyPath Reference to the string containing the key path.
     * @return The unquoted key name.
     */
    std::string ExtractKeyName(const std::string& rssKeyPath);

    /**
     * @brief Quotation request enum for quoting text (or keys).
     */
    enum class EQuoteRequest
    {
        smart_key,                  ///< Uses quotation markers and escape characters if needed.
        smart_text,                 ///< Use literal markers if possible, use quotation markers if needed.
        quoted_text,                ///< Will always use quotation markers.
        literal_text,               ///< Will use literal markers if possible; otherwise uses quotation markers.
        multi_line_quoted_text,     ///< Multiple lines quoted text (carriage return and newline are not escaped).
        multi_line_literal_text,    ///< Multiple lines literal text (carriage return and newline are not escaped).
    };

    /**
     * @brief If the provided key contains characters which are not allowed to be used in a bare-key, add quotes to the key and
     * use escape characters if applicable.
     * @param[in] rssText The text to (optionally) quote.
     * @param[in] eQuoteRequest The quotation method requested.
     * @return The quoted key (if applicable); otherwise the key without quotes.
     */
    std::string QuoteText(const std::string& rssText, EQuoteRequest eQuoteRequest = EQuoteRequest::smart_text);
} // namespace toml_parser

#endif // !defined MISCELLANEOUS_H