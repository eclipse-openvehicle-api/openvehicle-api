#ifndef LEXERBASE_H
#define LEXERBASE_H

/**
 * @brief Token type
 */
enum class ETokenType
{
    token_none,             ///< No token
    token_whitespace,       ///< Token is whitespace
    token_comments,         ///< Token is C-style or C++ comments
    token_identifier,       ///< Token is an identifier
    token_keyword,          ///< Token is a keyword
    token_literal,          ///< Token is a literal
    token_separator,        ///< Token is a separator
    token_operator,         ///< Token is a operator
    token_meta,             ///< Token contains meta data
};

/**
 * @brief Token literal type (valid when token is a literal token).
 */
enum class ETokenLiteralType
{
    token_undefined,                        ///< No literal type defined
    token_literal_dec_integer,              ///< Decimal integer literal
    token_literal_oct_integer,              ///< Octal integer literal
    token_literal_hex_integer,              ///< IsHexadecimal() integer literal
    token_literal_bin_integer,              ///< Binary integer literal
    token_literal_dec_floating_point,       ///< Decimal floating point literal
    token_literal_hex_floating_point,       ///< IsHexadecimal() floating point literal
    token_literal_fixed_point,              ///< Fixed point literal
    token_literal_string,                   ///< String literal (ASCII ISO Latin-1 (8859-1) character set, UTF-8, UTF-16,
                                            ///< UTF-32, wide - platform specific)
    token_literal_raw_string,               ///< Raw string literal (ASCII ISO Latin-1 (8859-1) character set, UTF-8, UTF-16,
                                            ///< UTF-32, wide - platform specific)
    token_literal_character,                ///< Character literal (ASCII ISO Latin-1 (8859-1) character set, UTF-16, UTF-32,
                                            ///< wide - platform specific)
    token_literal_character_sequence,       ///< Character sequence literal (ASCII or wide - both platform cpecific)
    token_literal_boolean,                  ///< Boolean literal
    token_literal_nullptr,                  ///< Nullptr literal
};

/**
 * @brief Token meta type (valid when token is a metza token).
 */
enum class ETokenMetaType
{
    token_undefined,                        ///< No meta type defined
    token_meta_include_local,               ///< Local include file
    token_meta_include_global,              ///< Global include file
    token_meta_define,                      ///< Definition
    token_meta_undef,                       ///< Remove definition
    token_meta_verbatim,                    ///< Verbatim text to be inserted by the generator
};

// Forward declarations
class CToken;
class CCodePos;

/**
 * @brief Lexer callback interface
 */
struct ILexerCallback
{
    /**
     * @brief Insert whitespace - used to preserve whitespace.
     * @param[in] rtoken Reference to the token structure pointing to the whitespace.
     */
    virtual void InsertWhitespace(const CToken &rtoken) = 0;

    /**
     * @brief Insert a comment, either standalone, belonging to the last statement or belonging to the next statement.
     * @param[in] rtoken Reference to the token structure pointing to the comment.
     */
    virtual void InsertComment(const CToken& rtoken) = 0;

    /**
     * @brief Process a preprocessor directive. Preprocessor directives occur at the beginning of a line and can be preceded by
     * whitespace. The directive starts with the '#' character.
     * @param[in] rCode Reference to the current code position starting the directive.
     */
    virtual void ProcessPreprocDirective(CCodePos& rCode) = 0;
};




#endif // !defined LEXERBASE_H