#ifndef LEXER_TOML_H
#define LEXER_TOML_H

#include <algorithm>
#include <stack>
#include "character_reader_utf_8.h"

/**
 * @brief Tokenizes the output of a character reader in regard of the TOML format and returns tokens in order on demand
 */
class CLexerTOML
{
public:
    /**
     * @brief Enum for all possible token categories
     */
    enum class ETokenCategory : uint8_t
    {
        token_none,                         ///< Default
        token_syntax_assignment,            ///< '='
        token_syntax_array_open,            ///< '[' after '='
        token_syntax_array_close,           ///< ']' after an array open
        token_syntax_table_open,            ///< '['
        token_syntax_table_close,           ///< ']'
        token_syntax_table_array_open,      ///< '[['
        token_syntax_table_array_close,     ///< ']]'
        token_syntax_inline_table_open,     ///< '{'
        token_syntax_inline_table_close,    ///< '}'
        token_syntax_comma,                 ///< ','
        token_syntax_dot,                   ///< '.'
        token_syntax_new_line,              ///< Line break
        token_key,                          ///< Key of a Key-Value-Pair
        token_string,                       ///< A string for a Value of a Key-Value-Pair or Array
        token_integer,                      ///< An integer for a Value of a Key-Value-Pair or Array
        token_float,                        ///< A floating point number for a Value of a Key-Value-Pair or Array
        token_boolean,                      ///< A bool for a Value of a Key-Value-Pair or Array
        token_time_local,                   ///< Unused for now
        token_date_time_offset,             ///< Unused for now
        token_date_time_local,              ///< Unused for now
        token_date_local,                   ///< Unused for now
        token_eof,                          ///< End of File Token; may only be at the end of the token array
        token_error,                        ///< Error token containing an error message; further lexing is not affected
        token_empty,                        ///< Empty token for trying to read out of bounds
        token_terminated,                   ///< Terminated token containing an error message; further lexing is terminated
    };

    /**
     * @brief Contains lexed information for the parser
     */
    struct SToken
    {
        /**
         * @brief Default constructor
         */
        SToken() = default;

        /**
         * @brief Constructs a new Token object with a given category
         * @param[in] category The initial token category value for the token to be constructed
         */
        explicit SToken(ETokenCategory category) : eCategory(category)
        {}

        std::string	   ssContentString;                         ///< Token string content
        int64_t		   iContentInteger{};                       ///< Token integer content
        double		   dContentFloatingpoint{};                 ///< Token floatingpoint content
        bool		   bContentBoolean{};                       ///< Token boolean content
        ETokenCategory eCategory{ETokenCategory::token_none};   ///< Token category
    };

    /**
     * @brief Default constructor
     */
    CLexerTOML() = default;

    /**
     * @brief Constructs a new LexerTOML object with given input data that will be lexed
     * @param[in] rssString The UTF-8 encoded content of a TOML source
     */
    CLexerTOML(const std::string& rssString);

    /**
    * @brief Feed the lexer with the given string.
    * @param[in] rssString UTF-8 input string.
    */
    void Feed(const std::string& rssString);

    /**
    * @brief Reset the lexer content.
    */
    void Reset();

    /**
     * @brief Gets the next token after the current cursor position without advancing the cursor
     * @return Returns the next token after the current cursor position or a End-of-File-Token if there are no
     * tokens
     */
    SToken Peek() const;

    /**
     * @brief Gets the next token after the current cursor position and advancing the cursor by one
     * @return Returns the next token after the current cursor position or a End-of-File-Token if there are no
     * tokens
     */
    SToken Consume();

    /**
     * @brief Gets the n-th token after the current cursor without advancing the cursor
     * @param[in] n Step size
     * @return Returns the n-th token after the current cursor position or and empty token if n<1 or the end-token
     * if a step of n would read a position after the end-token
     */
    SToken Peek(int32_t n);

    /**
     * @brief Gets the n-th token after the current cursor and advancing the cursor by n
     * @param[in] n Step size
     * @return Returns the n-th token after the current cursor position or and empty token if n<1 or the end-token
     * if a step of n would read a position after the end-token
     */
    SToken Consume(int32_t n);

    /**
     * @brief Checks if the end-token was consumed
     * @return Returns true if the end-token was consumed by Consume() or Consume(n) or if there are no tokens;
     * false otherwise
     */
    bool IsEnd() const;

private:
    void GenerateTokens();

    bool IsBasicQuotedKey();

    void ReadBasicQuotedKey();

    bool IsLiteralQuotedKey();

    void ReadLiteralQuotedKey();

    bool IsBareKey();

    void ReadBareKey();

    bool IsBasicString();

    void ReadBasicString();

    bool IsBasicMultilineString();

    void ReadBasicMultilineString();

    bool IsLiteralString();

    void ReadLiteralString();

    bool IsLiteralMultilineString();

    void ReadLiteralMultilineString();

    bool IsInteger();

    void ReadInteger();

    bool IsFloat();

    void ReadFloat();

    bool IsBool();

    void ReadBool();

    bool IsWhitespace();

    void ReadWhitespace();

    bool IsSyntaxElement();

    void ReadSyntaxElement();

    bool IsComment();

    void ReadComment();

    void ReadUnknownSequence();

    std::string Unescape();

    std::string Unicode4DigitToUTF8();

    std::string Unicode8DigitToUTF8();

    std::string UnicodeToUTF8(uint8_t numCharacters);

    static uint32_t HexToDecimal(const char character);

    static uint32_t DecimalToDecimal(const char character);

    static uint32_t OctalToDecimal(const char character);

    static uint32_t BinaryToDecimal(const char character);

    CCharacterReaderUTF8 m_reader;
    std::vector<SToken>	m_vecTokens;
    std::size_t			m_nCursor{0};

    /**
     * @brief Enum for differentiating between keys and values that are potentially indifferent like '"value" =
     * "value"'
     */
    enum class EExpectation
    {
        expect_key,         ///< A key is expected over a value
        expect_value,       ///< A value is expected over a key
        expect_value_once,  ///< A value is expected over a key once
    };
    std::stack<EExpectation> m_stackExpectations; ///< Tracking of key or value expectations in nested structures
    // int32_t m_LineCount{0};

    const std::vector<std::string> m_vecKeyDelimiters{
        "\n", "\t", "\r", " ", "", ".", "=", "]"}; ///< Characters that delimit a key
    const std::vector<std::string> m_vecValueDelimiters{
        "\n", "\t", "\r", " ", ",", "", "]", "}"}; ///< Characters that delimit a value
};

#endif // LEXER_TOML_H
