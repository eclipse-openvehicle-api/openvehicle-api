#ifndef LEXER_TOML_H
#define LEXER_TOML_H

#include <algorithm>
#include <stack>
#include <list>
#include <functional>
#include "character_reader_utf_8.h"
#include "lexer_toml_token.h"

/// The TOML parser namespace
namespace toml_parser
{
    /**
     * @brief Node token range used to regenerate the source from the node entries.
     * @details A node can have several token ranges identifying code that belongs to the node or precedes or succeeds the node. The
     * following ranges can be identified:
     * - code before the node, not belonging to another node
     * - code before the node with comments belonging to the node
     * - the first part of the node
     * - the in between part, which might contain other nodes (in case of an inline node collection)
     * - the second part of the node finishing the node (in case of an inline node collection)
     * - code behind the node with comments belonging to the node
     * - code behind the node, not belonging to another node (this is the code until the end of the node list)
     */
    class CNodeTokenRange
    {
    public:
        /**
         * @brief Initialize the range with the first initial token. Any token before that belong to the token range of the previous
         * node.
         * @param[in] rInitialToken The initial token that defines/starts the range.
         */
        CNodeTokenRange(const CToken& rInitialToken);

        /**
         * @brief Construct a node range with the main node token range.
         * @param[in] rrangeNodeMain Reference to the token range holding the main node tokens.
         */
        CNodeTokenRange(const CTokenRange& rrangeNodeMain);

        /**
         * @brief Set the token identifying the area before the extended node. Will start before or at and end at the extended node
         * range.
         * @param[in] rTokenBegin Reference to the token identifying the begin of the lines before the extended node range.
         * @remarks Setting the begin will not change the extended node range. If starting within the extended node range, the begin
         * of the extended node range determines the begin of the lines before.
         */
        void LinesBeforeNode(const CToken& rTokenBegin);

        /**
         * @brief Get the lines before range, being located before the extended node range.
         * @return The token range with begin and end token of the lines before. If identical, there are no lines before.
         */
        CTokenRange LinesBeforeNode() const;

        /**
         * @brief Set the token range identifying the extended node range. Will start before or at and end behind or at the main
         * node ranges (incl. the finish range).
         * @param[in] rRange Token range holding the begin and end of the extended node range.
         * @remarks If the range starts or ends within the main node range, the extended node range is adapted to the main node
         * range. If the lines before or behind the node fall within the extended node range, the lines will be adapted to the
         * extended node range.
         */
        void ExtendedNode(const CTokenRange& rRange);

        /**
         * @brief Get the extended node range. The range includes the main node ranges (including the node finish range).
         * @return The token range with begin and end token of the extended node range.
         */
        CTokenRange ExtendedNode() const;

        /**
         * @brief Get the node comments part before the node definition, which is the difference between the beginning of the
         * extended node and the main node.
         * @return The token range with begin and end token of the node comments range.
         */
        CTokenRange NodeCommentsBefore() const;

        /**
         * @brief Set the main node range.
         * @param[in] rRange Token range holding the begin and end of the main node range.
         * @remarks If the main node finish range starts within or before the main node range, the main node finish range will be
         * updated. If the extended node range does not include the main range completely (incl. finish node range), the extended
         * node range will be updated.
         */
        void NodeMain(const CTokenRange& rRange);

        /**
         * @brief Get the main node range.
         * @return The token range with begin and end token of the main node range.
         */
        CTokenRange NodeMain() const;

        /**
         * @brief Set the main node finish range (for inline tables and arrays).
         * @param[in] rRange Token range holding the begin and end of the main node finish range.
         * @remarks If the main node finish range starts within or before the main node range, the main node finish range will be
         * updated. If the extended node range does not include the main range completely (incl. finish node range), the extended
         * node range will be updated.
         */
        void NodeMainFinish(const CTokenRange& rRange);

        /**
         * @brief Get the main node finish range.
         * @return The token range with begin and end token of the main node finish range.
         */
        CTokenRange NodeMainFinish() const;

        /**
         * @brief Get the node comments part behind the node definition, which is the difference between the end of the
         * main node and the end of the extended node.
         * @return The token range with begin and end token of the node comments range.
         */
        CTokenRange NodeCommentsBehind() const;

        /**
         * @brief Set the token identifying the area behind the extended node. Will start at and end behind or at the extended node
         * range.
         * @param[in] rTokenEnd Reference to the token identifying the end of the lines behind the extended node range.
         * @remarks Setting the end will not change the extended node range. If ending within the extended node range, the end
         * of the extended node range determines the end of the lines before.
         */
        void LinesBehindNode(const CToken& rTokenEnd);

        /**
         * @brief Get the lines behind range, being located behind the extended node range.
         * @return The token range with begin and end token of the lines behind. If identical, there are no lines behind.
         */
        CTokenRange LinesBehindNode() const;

    private:
        std::reference_wrapper<const CToken>    m_refBeforeNodeBegin;   ///< The start of the lines before the extended node (not
                                                                        ///< belonging to any node).
        CTokenRange                             m_rangeExtendedNode;    ///< The extended node including comments before and behind
                                                                        ///< the node.
        CTokenRange                             m_rangeNodeMain;        ///< The node tokens or for inline tables and arrays the
                                                                        ///< opening part of the node.
        CTokenRange                             m_rangeNodeFinish;      ///< For inline tables and arrays, the node tokens for closing the node.
        std::reference_wrapper<const CToken>    m_refBehindNodeEnd;     ///< The end of the lines behind the extended node (not
                                                                        ///< belonging to any node).
    };

    /**
     * @brief Tokenizes the output of a character reader using TOML v1.0: https://toml.io/en/v1.0.0
     */
    class CLexer
    {
    public:
        /**
         * @brief Default constructor
         */
        CLexer() = default;

        /**
         * @brief Constructs a new LexerTOML object with given input data that will be lexed
         * @param[in] rssString The UTF-8 encoded content of a TOML source
         * @param[in] bValueOnly When set, the lexer should treat the string as a value assignment.
         */
        CLexer(const std::string& rssString, bool bValueOnly = false);

        /**
         * @brief Feed the lexer with the given string. This will replace a previous lexing result.
         * @param[in] rssString UTF-8 input string.
         * @param[in] bValueOnly When set, the lexer should treat the string as a value assignment.
         */
        void Feed(const std::string& rssString, bool bValueOnly = false);

        /**
         * @brief Reset the lexer cursor position.
         */
        void Reset();

        /**
         * @brief Navigation modes supported by the lexer.
         */
        enum class ENavigationMode
        {
            skip_comments_and_whitespace = 1,   ///< Skip comments and whitespace during navigation (default)
            do_not_skip_anything = 2,           ///< Do not skip anything during navigation.
        };

        /**
         * @brief Get the current navigation mode.
         * @return The current naviation mode.
         */
        ENavigationMode NavigationMode() const;

        /**
         * @brief Set the navigation mode.
         * @param[in] eMode The mode to be used for navigation.
         */
        void NavigationMode(ENavigationMode eMode);

        /**
         * @brief Gets the n-th token after the current cursor without advancing the cursor
         * @remarks Whitespace and comments are skipped.
         * @param[in] nSkip Skip the amount of tokens.
         * @return Returns smart pointer to the token in the token list or an empty pointer.
         */
        const CToken& Peek(size_t nSkip = 0) const;

        /**
         * @brief Gets the n-th token after the current cursor and advancing the cursor by n
         * @remarks Whitespace and comments are skipped.
         * @param[in] nSkip Skip the amount of tokens.
         * @return Returns smart pointer to the token in the token list or an empty pointer.
         */
        const CToken& Consume(size_t nSkip = 0);

        /**
         * @brief Checks if the end-token was consumed
         * @return Returns true if the end-token was consumed by Consume() or Consume(n) or if there are no tokens;
         * false otherwise
         */
        bool IsEnd() const;

        /**
         * @brief Check and extend the provided boundaries of a given token range to include white space and comments which
         * obviously belong to the token range and check for additional tokens when reaching the end of the token list.
         * @param[in, out] rTokenRange Reference to the node token range being updated with extended boundaries.
         */
        void SmartExtendNodeRange(CNodeTokenRange& rTokenRange) const;

    private:
        /**
         * @brief Run through the string and generate tokens.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         */
        void GenerateTokens(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read a quoted key. A quoted key accepts basic strings as keys surrounded by double quotes.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a basic quoted key or an empty token if the token hasn't been found.
         */
        CToken ReadBasicQuotedKey(CCharacterReaderUTF8& rReader) const;

        /**
         * @brief Read a literal quoted key. A literal quoted key accepts basic strings as keys surrounded by single quotes.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a literal quoted key or an empty token if the token hasn't been found.
         */
        CToken ReadLiteralQuotedKey(CCharacterReaderUTF8& rReader) const;

        /**
         * @brief Read a bare key. A bare key may only contain ASCII letters, ASCII digits, underscores and dashes (A-Za-z0-9_-).
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a bare key or an empty token if the token hasn't been found.
         */
        CToken ReadBareKey(CCharacterReaderUTF8& rReader) const;

        /**
         * @brief Read a basic string. A basic string may contain any unicode character. Some characters need to be escaped. The
         * basis string is surrounded by double quotes.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a basic string or an empty token if the token hasn't been found.
         */
        CToken ReadBasicString(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read a multi-line basic string. A basic string may contain any unicode character. Some characters need to be
         * escaped. The multi-line basis string is surrounded by three double quotes before and behind the string.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a multi-line basic string or an empty token if the token hasn't been found.
         */
        CToken ReadBasicMultilineString(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read a literal string. A literal string may contain any unicode character, but does not support escaped
         * characters. The literal string is surrounded by single quotes.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a literal string or an empty token if the token hasn't been found.
         */
        CToken ReadLiteralString(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read a multi-line literal string. A literal string may contain any unicode character, but does not support escaped
         * characters. The multi-line literal string is surrounded by three single quotes before and behind the string.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a multi-line literal string or an empty token if the token hasn't been found.
         */
        CToken ReadLiteralMultilineString(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read the integer. An integer is number optionally preceded by a sign and possible defined as hexadecimal
         * number (preceded by 0x), octal number (preceded by 0o) or binary number (preceeeded by 0b). Numbers can be split with
         * underscores.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for an integer number or an empty token if the token hasn't been found.
         */
        CToken ReadInteger(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read the floating point number. A floating point number follows the rules defined in IEEE 754 binary64 and consist
         * of a integer part followed by a fractional part (separated by a dot). An sign can be preceded and an exponential part
         * can be succeeded (separated from the fractional part with the 'e'). The number can be split with underscores. Infinite
         * and not-a-number are also supported (as defined in the IEEE 754).
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a floating point number or an empty token if the token hasn't been found.
         */
        CToken ReadFloat(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read the boolean. A boolean has the value 'true' or 'false'.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a boolean or an empty token if the token hasn't been found.
         */
        CToken ReadBool(CCharacterReaderUTF8& rReader);

        /**
         * @brief Get the whitespace. Whitespace is a tab or space.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for whitespace or an empty token if the token hasn't been found.
         */
        CToken ReadWhitespace(CCharacterReaderUTF8& rReader) const;

        /**
         * @brief Get the syntax element. A syntax element identifies tables, arrays, new-lines, separators and
         * assignments: '[]{},.=\\r\\n'.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for a syntax element or an empty token if the token hasn't been found.
         */
        CToken ReadSyntaxElement(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read the comment. A comment is identified by a hash '#' and includes the rest of the line.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for comment or an empty token if the token hasn't been found.
         */
        static CToken ReadComment(CCharacterReaderUTF8& rReader);

        /**
         * @brief Read the unknown sequence (not represented by defined tokens).
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return Returns the token for an unknown sequence or an empty token if the reader has reached EOF.
         */
        CToken ReadUnknownSequence(CCharacterReaderUTF8& rReader);

        /**
         * @brief In case of an escape, return the interpretation of the character following.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @return The unescaped character.
         */
        static std::string Unescape(CCharacterReaderUTF8& rReader);

        /**
         * @brief Generic implementation of escaped unicode character interpretation.
         * @param[in] rReader Reference to the reader providing the UTF8 characters.
         * @param[in] nDigits The amount of digits the unicode character consist of.
         * @return The interpreted unicode character.
         */
        static std::string EscapedUnicodeCharacterToUTF8(CCharacterReaderUTF8& rReader, size_t nDigits);

        ENavigationMode     m_eNavMode = ENavigationMode::skip_comments_and_whitespace; ///< The current navigation mode.
        TTokenList          m_lstTokens;                                                ///< List of tokens.
        TTokenListIterator  m_itCursor{m_lstTokens.end()};                              ///< Current position within token list.

        /**
         * @brief Enum for differentiating between keys and values that are potentially indifferent like '"value" =
         * "value"'
         */
        enum class EExpectation
        {
            expect_key,        ///< A key is expected over a value
            expect_value,      ///< A value is expected over a key
            expect_value_once, ///< A value is expected over a key once
        };
        std::stack<EExpectation> m_stackExpectations; ///< Tracking of key or value expectations in nested structures

        const std::vector<std::string>
            m_vecKeyDelimiters{"\n", "\t", "\r", " ", "", ".", "=", "]"}; ///< Characters that delimit a key
        const std::vector<std::string>
            m_vecValueDelimiters{"\n", "\t", "\r", " ", ",", "", "]", "}", "#"}; ///< Characters that delimit a value
    };
} // namespace toml_parser
#endif // LEXER_TOML_H
