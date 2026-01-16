#ifndef LEXER_TOML_TOKEN_H
#define LEXER_TOML_TOKEN_H

#include <string>
#include <cstdint>
#include <list>
#include <optional>
#include <memory>
#include "exception.h"

/// The TOML parser namespace
namespace toml_parser
{
    /**
     * @brief Enum for all possible token categories
     */
    enum class ETokenCategory : uint32_t
    {
        token_none,                      ///< Default - not having read anything
        token_syntax_assignment,         ///< '='
        token_syntax_array_open,         ///< '[' after '='
        token_syntax_array_close,        ///< ']' after an array open
        token_syntax_table_open,         ///< '['
        token_syntax_table_close,        ///< ']'
        token_syntax_table_array_open,   ///< '[['
        token_syntax_table_array_close,  ///< ']]'
        token_syntax_inline_table_open,  ///< '{'
        token_syntax_inline_table_close, ///< '}'
        token_syntax_comma,              ///< ','
        token_syntax_dot,                ///< '.'
        token_syntax_new_line,           ///< Line break
        token_key,                       ///< Key of a Key-Value-Pair
        token_string,                    ///< A for a Value of a Key-Value-Pair or Array
        token_integer,                   ///< An integer for a Value of a Key-Value-Pair or Array
        token_float,                     ///< A floating point number for a Value of a Key-Value-Pair or Array
        token_boolean,                   ///< A bool for a Value of a Key-Value-Pair or Array
        token_time_local,                ///< Unused for now
        token_date_time_offset,          ///< Unused for now
        token_date_time_local,           ///< Unused for now
        token_date_local,                ///< Unused for now
        token_whitespace,                ///< Whitespace token
        token_comment,                   ///< Comment token
        token_error,                     ///< Error token containing an error message; further lexing is not affected
        token_empty,                     ///< Empty token for trying to read out of bounds
        token_terminated,                ///< Terminated token containing an error message; further lexing is terminated
    };

    /**
     * @brief The string type for key and string tokens.
     */
    enum ETokenStringType
    {
        not_specified,      ///< Not specified or not applicable
        quoted_string,      ///< Quoted string
        literal_string,     ///< Literal string
        multi_line_quoted,  ///< Multiple lines as a quoted string (only for string tokens)
        multi_line_literal, ///< Multiple lines as a literal string (only for string tokens)
    };

    // Forward declaration
    class CLexer;
    class CToken;
    class CTokenList;
    class CTokenRange;

    /// Token list type definition
    using TTokenList = CTokenList;

    /// Token list iterator definition
    using TTokenListIterator = std::list<CToken>::const_iterator;

    /**
     * @brief Contains lexed information for the parser
     */
    class CToken
    {
        // Access to private functions is allowed for the following classes:
        friend class CLexer;
        friend class CTokenRange;

    public:
        /**
         * @brief Default constructor
         */
        CToken();

        /**
         * @brief Boundary identification.
         */
        enum EBoundary
        {
            no_boundary,        ///< The token is not defined a boundary token.
            lower_boundary,     ///< One token before the first token in the token list.
            upper_boundary,     ///< One token behind the last token in the token list.
        };

        /**
         * @brief Constructor to construct a boundary token (beginning or end of the token list).
         * @param[in] rTokenList Reference to the token list this token resides in.
         * @param[in] eBoundary The boundary type to define.
         */
        explicit CToken(const TTokenList& rTokenList, EBoundary eBoundary);

        /**
         * @brief Constructs a new Token object with a given category (can be anything, except key, string, integer, boolean, time
         * and error).
         * @param[in] eCategory The initial token category value for the token to be constructed.
         */
        explicit CToken(ETokenCategory eCategory);

        /**
         * @brief Constructs a new Token object with a given category (can only be a key, a string or an error).
         * @param[in] eCategory The token category value for the token to be constructed.
         * @param[in] rssContent Reference to the string value.
         * @param[in] eStringType The type of string that should be used.
         */
        CToken(ETokenCategory eCategory, const std::string& rssContent,
            ETokenStringType eStringType = ETokenStringType::not_specified);

        /**
         * @brief Constructs a new Token object with a given category (can only be an integer value).
         * @param[in] eCategory The token category value for the token to be constructed.
         * @param[in] iContent The integer value.
         */
        explicit CToken(ETokenCategory eCategory, int64_t iContent);

        /**
         * @brief Constructs a new Token object with a given category (can only be a floating point value).
         * @param[in] eCategory The token category value for the token to be constructed.
         * @param[in] dContent The double precision floating point value.
         */
        explicit CToken(ETokenCategory eCategory, double dContent);

        /**
         * @brief Constructs a new Token object with a given category (can only be a boolean value).
         * @param[in] eCategory The token category value for the token to be constructed
         * @param[in] bContent The boolean value.
         */
        explicit CToken(ETokenCategory eCategory, bool bContent);

        /**
         * @brief Error token.
         * @param[in] eCategory The token category value for the token to be constructed (category error or terminated).
         * @param[in] rexcept Reference to the exception that was triggered.
         */
        CToken(ETokenCategory eCategory, const sdv::toml::XTOMLParseException& rexcept);

        /**
         * @brief Copy constructor.
         * @param[in] rToken Reference to the token to copy from.
         */
        CToken(const CToken& rToken);

        /**
         * @brief Move constructor.
         * @param[in] rToken Reference to the token to move from.
         */
        CToken(CToken&& rToken);

        /**
         * @brief Destructor
         */
        ~CToken();

        /**
         * @brief Copy assignment operator.
         * @param[in] rToken Reference to the token to copy from.
         * @return Reference to this token.
         */
        CToken& operator=(const CToken& rToken);

        /**
         * @brief Move constructor.
         * @param[in] rToken Reference to the token to move from.
         * @return Reference to this token.
         */
        CToken& operator=(CToken&& rToken);

        /**
         * @brief Check whether the supplied token is identical to this token. The tokens are identical if the token iterators of
         * the token list are identical, or if they don't have a token iterator, have identical category and value.
         * @param[in] rToken Reference to the token to use for the comparison.
         * @return Returns whether the supplied token is identical.
         */
        bool operator==(const CToken& rToken) const;

        /**
         * @brief Check whether the supplied token is not identical to this token. The tokens are identical if the token iterators
         * of the token list are identical, or if they don't have a token iterator, have identical category and value.
         * @param[in] rToken Reference to the token to use for the comparison.
         * @return Returns whether the supplied token is not identical.
         */
        bool operator!=(const CToken& rToken) const;

        /**
         * @brief Checks whether the token is initialized with any token other then ETokenCategory::token_none.
         * @return Returns whether the token is initialized.
         */
        operator bool() const;

        /**
         * @brief Quick navigation through the token list. Get the next token in the token list.
         * @param[in] nSkip The amount of tokens to skip while getting the next token.
         * @return Returns the next token or an none-token when the end of the token list has been reached or no tokenlist
         * assignment has been made for this token.
         */
        const CToken& Next(size_t nSkip = 0) const;

        /**
         * @brief Quick navigation through the token list. Get the previous token in the token list.
         * @param[in] nSkip The amount of tokens to skip while getting the previous token.
         * @return Returns the previous token or an none-token when the begin of the token list has been reached or no tokenlist
         * assignment has been made for this token.
         */
        const CToken& Prev(size_t nSkip = 0) const;

        /**
         * @brief Quick navigation through the token list to the begin of the token list.
         * @return Returns the first token of the token list.
         */
        const CToken& JumpToBegin() const;

        /**
         * @brief Quick navigation through the token list to the end of the token list.
         * @remarks The last token of the token list will return "false" when tested. It can be used to iterate backwards through
         * the token list.
         * @return Returns the one beyond the last token of the token list.
         */
        const CToken& JumpToEnd() const;

        /**
         * @brief Return the current category.
         * @return The token category.
         */
        ETokenCategory Category() const;

        /**
         * @brief Get the token string type.
         * @return Returns the string type for the key or string token. Returns ETokenStringType::not_specified when there is no
         * string type defined or the token isn't string based.
         */
        ETokenStringType StringType() const;

        /**
         * @brief Get the string value. Will be filled for keys, strings and errors.
         * @return The string value or empty string when the category type is invalid.
         */
        std::string StringValue() const;

        /**
         * @brief Get the integer value. Will be filled for a integer number.
         * @return The integer value or zero when the category type is invalid.
         */
        int64_t IntegerValue() const;

        /**
         * @brief Get the floating point number value. Will be filled for a floating point number.
         * @return The floating point number value value or 0.0 when the category type is invalid.
         */
        double FloatValue() const;

        /**
         * @brief Get the boolean value. Will be filled for a boolean.
         * @return The boolean value or false when the category type is invalid.
         */
        bool BooleanValue() const;

        /**
         * @brief Get the associated raw string.
         * @return Reference to the raw string.
         */
        const std::string& RawString() const;

        /**
         * @brief The token index identifies the token order.
         * @return The unique index of this token.
         */
        uint32_t TokenIndex() const;

    private:
        /**
         * @brief Set the string chunk from the original TOML belonging to this token and the location within the lexer token list.
         * @param[in] rssString Reference to the string containing the "raw" value of the token.
         * @param[in] rTokenList Reference to the token list the iterator is referring to.
         * @param[in] ritLocation The location in the lexer list.
         */
        void RawDataInfo(const std::string& rssString, const TTokenList& rTokenList, const TTokenListIterator& ritLocation);

        /**
         * @brief Return the location in the lexer token list.
         * @return Reference to the iterator location option.
         */
        const std::optional<TTokenListIterator>& Location() const;

        /**
         * @brief Return a reference to the token list this token is located in.
         * @return Reference to the token list reference option.
         */
        const std::optional<std::reference_wrapper<const TTokenList>>& TokenList() const;

        /**
         * @brief Create a new index. Indices are used to guarantee correct token order.
         * @return The next index number.
         */
        static uint32_t CreateIndex();

        std::string m_ssRawString;                                              ///< The raw string without interpretation.
        std::optional<std::reference_wrapper<const TTokenList>> m_optTokenList; ///< The token list this token is referring to.
        std::optional<TTokenListIterator> m_optLocation;                        ///< Location in the lexer token list.

        union
        {
            std::string m_ssContentString;  ///< Token string content (used with keys, strings and errors).
            int64_t m_iContentInteger;      ///< Token integer content
            double m_dContentFloatingpoint; ///< Token floatingpoint content
            bool m_bContentBoolean;         ///< Token boolean content
        };
        ETokenCategory      m_eCategory = ETokenCategory::token_none;           ///< Token category
        ETokenStringType    m_eStringType = ETokenStringType::not_specified;    ///< Token string type for key and string tokens.
        uint32_t            m_uiIndex = CreateIndex();                          ///< Current token index.
    };

    /**
     * @brief Token list with boundary tokens used in navigation.
     */
    class CTokenList : public std::list<CToken>
    {
    public:
        /**
         * @brief Default constructor.
         */
        CTokenList();

        CToken tokenReverseEnd; ///< One token before first token in token list. Used to detect backwards iteration boundary.
        CToken tokenEnd;        ///< One token behind last token in token list. Used to detect forwards iteration boundary.
    };

    /**
     * @brief The token range is used to identify the initial and the final token belonging to a collection. All intermediate
     * tokens are then part of the collection.
     */
    class CTokenRange
    {
    public:
        /**
         * @brief The default constructor is deleted.
         */
        CTokenRange() = delete;

        /**
         * @brief Start a token range using the begin token.
         * @remarks The end token is automatically set to one token beyond the last token in the list. This can be changed by
         * assigning the end token.
         * @param[in] rTokenBegin Reference to the first token in this range.
         */
        CTokenRange(const CToken& rTokenBegin);

        /**
         * @brief Directly assign a token range.
         * @remarks If the begin token and the end token are identical, the range doesn't contain any tokens.
         * @param[in] rTokenBegin Reference to the first token in this range.
         * @param[in] rTokenEnd Reference to one token beyond the last token in this range.
         */
        CTokenRange(const CToken& rTokenBegin, const CToken& rTokenEnd);

        /**
         * @brief Reassign the first token in the range. This will set the end token to the one beyond the last token in the list.
         * @param[in] rTokenBegin Reference to the first token in this range.
         */
        void ReassignBeginToken(const CToken& rTokenBegin);

        /**
         * @brief Reassign the token range.
         * @remarks If the begin token and the end token are identical, the range doesn't contain any tokens.
         * @param[in] rTokenBegin Reference to the first token in the range.
         * @param[in] rTokenEnd Reference to one token beyond the last token in the range.
         */
        void ReassignTokenRange(const CToken& rTokenBegin, const CToken& rTokenEnd);

        /**
         * @brief Assign or overwrite an end token.
         * @param[in] rTokenEnd Reference to the last token in this range.
         * @param[in] bIncludeEnd When set, the token belongs to the range; if not, the token points past the last range token.
         */
        void AssignEndToken(const CToken& rTokenEnd, bool bIncludeEnd = true);

        /**
         * @brief Get the initial token in the range.
         * @return Reference to the smart pointer containing the first token in this range.
         */
        const CToken& Begin() const;

        /**
         * @brief Get one token beyond the final token in the range.
         * @remarks The end token can be one token beyond the last token in the list. This token will return 'false' when tested and
         * can be used for navigation.
         * @return Reference to the smart pointer containing one token beyond the final token in this range.
         */
        const CToken& End() const;

        /**
         * @brief Create a copy of the token list part identified by the start and end token (not including the end token).
         * @return The token list containing the slice.
         */
        std::list<CToken> TokenListSLice() const;

        /**
         * @brief Does the token range contain a valid range of at least one token.
         * @return Returns whether the token range is valid.
         */
        operator bool() const;

    private:
        std::reference_wrapper<const CToken> m_rTokenBegin; ///< Smart pointer to the first token in the range.
        std::reference_wrapper<const CToken> m_rTokenEnd;   ///< Smart pointer to one token beyond the last token in the range.
    };


} // namespace toml_parser

#endif // !defined LEXER_TOML_TOKEN_H
