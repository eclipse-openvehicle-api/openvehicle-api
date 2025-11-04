#ifndef LOCATION_H
#define LOCATION_H

#include "constvariant.h"
#include "lexerbase.h"
#include "parsecontext.h"
#include <cstdint>
#include <string>
#include <limits>
#include <cerrno>
#include <cstring>
#include <list>
#include <memory>

// Forward declaration
class CCodePos;

/**
 * @brief Token within the source string.
 */
class CToken
{
    friend CCodePos;

public:
    /**
     * @brief Default constructor
     */
    CToken() = default;

    /**
     * @brief Specific constructor for code insertion.
     * @details During processing, it sometimes is necessary to add created tokens to token lists.
     * @attention The constructor doesn't check for validity of the token text and types.
     * @param[in] rssText The text the token represents.
     * @param[in] eType The token type (default identifier).
     */
    CToken(const std::string& rssText, ETokenType eType = ETokenType::token_identifier);

    /**
     * @brief Specific constructor for code insertion.
     * @details During processing, it sometimes is necessary to add created tokens to token lists.
     * @attention The constructor doesn't check for validity of the token text and types.
     * @param[in] rssText The text the token represents.
     * @param[in] eLiteralType The token literal type (default undefined).
     */
    CToken(const std::string& rssText, ETokenLiteralType eLiteralType);

    /**
     * @brief Specific constructor for code insertion.
     * @details During processing, it sometimes is necessary to add created tokens to token lists.
     * @attention The constructor doesn't check for validity of the token text and types.
     * @param[in] rssText The text the token represents.
     * @param[in] eMetaType The token meta type (default undefined).
     */
    CToken(const std::string& rssText, ETokenMetaType eMetaType);

    /**
     * @brief Move start; adjusts the start position of the token. Does not adjust the end.
     * @param[in] iOffset The amount of characters to adjust. A negative offset moves to the left.
     * @attention Does move before the beginning of the line.
     * @attention Does not move past the end of the token.
     */
    void MoveStart(int32_t iOffset);

    /**
     * @brief Move end; adjusts the end position of the token. Does not adjust the start.
     * @param[in] iOffset The amount of characters to adjust. A negative offset moves to the left.
     * @attention Does move before the start of the token.
     */
    void MoveEnd(int32_t iOffset);

    /**
     * @brief Equality operator.
     * @param[in] rtoken The token to compare with.
     * @return Returns 'true' if the provided token and this token are identical. Otherwise returns 'false'.
     */
    bool operator==(const CToken& rtoken) const;

    /**
     * @brief Inequality operator.
     * @param[in] rtoken The token to compare with.
     * @return Returns 'true' if the provided token and this token are not identical. Otherwise returns 'false'.
     */
    bool operator!=(const CToken& rtoken) const;

    /**
     * @brief Return the line number.
     * @return The line number.
     */
    uint32_t GetLine() const;

    /**
     * @brief Return the column number.
     * @return Get column number.
     */
    uint32_t GetCol() const;

    /**
    * @brief Get the last line this token on.
    * @return The line number.
    */
    uint32_t GetEndLine() const;

    /**
    * @brief Get the last column this token is on.
    * @return The column number.
    */
    uint32_t GetEndCol() const;

    /**
     * @brief Return the length of the current token snippet. Or 0xFFFFFFFF when the rest of the code is part of this snippet.
     * @return Returns the length of the token snippet.
     */
    uint32_t GetLength() const;

    /**
     * @brief Get the token type.
     * @return The token type of this token.
     */
    ETokenType GetType() const;

    /**
     * @brief Get the token's literal type (if the token is a literal token).
     * @return The literal type of this token when the token is a literal token. ETokenLiteralType::token_undefined otherwise.
     */
    ETokenLiteralType GetLiteralType() const;

    /**
     * @brief Get the token's meta type (if the token is a meta token).
     * @return The meta type of this token when the token is a meta token. ETokenMetaType::token_undefined otherwise.
     */
    ETokenMetaType GetMetaType() const;

    /**
     * @brief Validity operator.
     * @return Returns 'true' when the token is valid. Otherwise returns 'false'.
     */
    operator bool() const;

    /**
     * @{
     * @brief Returns the code chunk that the token points to. When a length is part of the token the string is limited to
     * the length. Otherwise the chunk contains the complete string starting at the token.
     * @return Returns the string chunk.
     */
    operator const std::string&() const;
    const char* c_str() const;
    /**
     * @}
     */

    /**
     * @brief Get the left part of the string.
     * @param[in] uiLen The amount of characters to return.
     * @return Returns the left part of the string chunk.
     */
    std::string Left(uint32_t uiLen) const;

    /**
     * @brief Get the right part of the string.
     * @param[in] uiLen The amount of characters to return.
     * @return Returns the right part of the string chunk.
     */
    std::string Right(uint32_t uiLen) const;

    /**
     * @brief String equality operator.
     * @param[in] szString Zero terminated string to use for the comparison.
     * @return Returns 'true' if the string chunk of this token is identical with the provided string. Otherwise returns
     * 'false'.
     */
    bool operator==(const char* szString) const;

    /**
     * @brief String inequality operator.
     * @param[in] szString Zero terminated string to use for the comparison.
     * @return Returns 'true' if the string chunk of this token is not identical with the provided string. Otherwise returns
     * 'false'.
     */
    bool operator!=(const char* szString) const;

    /**
     * @brief String equality operator.
     * @param[in] rssString String object to use for the comparison.
     * @return Returns 'true' if the string chunk of this token is identical with the provided string. Otherwise returns
     * 'false'.
     */
    bool operator==(const std::string& rssString) const;

    /**
     * @brief String inequality operator.
     * @param[in] rssString String object to use for the comparison.
     * @return Returns 'true' if the string chunk of this token is identical with the provided string. Otherwise returns
     * 'false'.
     */
    bool operator!=(const std::string& rssString) const;

    /**
     * @{
     * @brief Information functions.
     * @return Return the information.
     */
    bool IsLiteral() const;
    bool IsInteger() const;
    bool IsSigned() const;
    bool IsUnsigned() const;
    bool IsLong() const;
    bool IsLongLong() const;
    bool IsDecimal() const;
    bool IsHexadecimal() const;
    bool IsOctal() const;
    bool IsBinary() const;
    bool IsFloatingPoint() const;
    bool IsFixedPoint() const;
    bool IsCharacter() const;
    bool IsCharacterSequence() const;
    bool IsString() const;
    bool IsAscii() const;
    bool IsUtf8() const;
    bool IsUtf16() const;
    bool IsUtf32() const;
    bool IsWide() const;
    bool IsRawString() const;
    bool IsBoolean() const;
    bool IsNullptr() const;
    /**
     * @}
     */

    /**
     * @brief Get the value of the token in the smallest fitting data type.
     * @throw The function throws an exception in case the assignment is not possible.
     * @return Returns the value as const variant.
     */
    CConstVariant ValueRef() const;

    /**
     * @brief Set source code context.
     * @details The parser can attach source code context. The context contains all the allocated memory this token is pointing
     * to and holding the context in the token prevents unexpected disappearance of the token during post-processing.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void SetContext(const CContextPtr& rptrContext) { m_ptrContext = rptrContext; }

    /**
     * @brief Get source code context.
     * @return Pointer to the source code context.
    */
    CContextPtr GetContext() const { return m_ptrContext; }

private:
    /**
     * @brief Returns if an occurrence of one or more characters exist in the code chunk.
     * @tparam TChar The type of the character to check (must be 'char' to work).
     * @param[in] c The character to check to exists within the code chunk.
     * @param[in] tChars Optionally zero or more characters to check to exists within the code chunk.
     */
    template <typename... TChar>
    bool Contains(char c, TChar... tChars) const;

    /**
     * @brief Returns if an occurrence of one or more strings exist in the code chunk.
     * @tparam TString The type of the string to check (must be 'const char*' to work).
     * @param[in] sz The string to check to exists within the code chunk.
     * @param[in] tStrings Optionally zero or more strings to check to exists within the code chunk.
     */
    template <typename... TString>
    bool Contains(const char* sz, TString... tStrings) const;

    /**
     * @brief Dummy contains function when there are no more arguments any more.
     */
    bool Contains() const { return false; }

    /**
     * @{
     * @brief Get the value of the token in supplied integer data type. This counts for integers, characters and character
     * sequences.
     * @throw The function throws an exception when the value exceeds or falls below the max or min value of the requested type.
     * @return Returns the value.
     */
    CConstVariant ValueSignedInteger() const;
    CConstVariant ValueUnsignedInteger() const;
    /**
     * @}
     */

    /**
     * @brief Get the value of the token in supplied floating data type.
     * @throw The function throws an exception when the value exceeds or falls below the max or min value of the requested type.
     * @return Returns the value.
     */
    CConstVariant ValueFloat() const;

    /**
     * @brief Get the value of the token for a boolean data type. The boolean data type accepts the C++ style "true" and "false"
     * as well as the IDL style "TRUE" and "FALSE". Integer and character and character sequence types can only be assigned to a
     * boolean if they return the value 1 or 0.
     * @return Returns the boolean value of the token.
     */
    CConstVariant ValueBoolean() const;

    /**
     * @brief Get the value of the token for a nullptr data type. The nullptr data type accepts the C style "NULL" and the C++
     * style "nullptr". Nullptr will be interpreted as the value 0 when assigned to a an integral data type.
     * @return Returns the nullptr value of the token (which is always 0).
     */
    CConstVariant ValueNullptr() const;

    /**
     * @brief Get the value of the token for a string data type.
     * @return Returns the string value of the token.
     */
    CConstVariant ValueString() const;

private:
    CContextPtr                     m_ptrContext;                                           ///< Smart pointer holding a reference
                                                                                            ///< to the source file.
    const char*                     m_szCode = nullptr;                                     ///< Start of the token in IDL code.
    uint32_t                        m_uiLen = 0xffffffff;                                   ///< Length of the token or 0xffffffff
                                                                                            ///< for the whole string.
    uint32_t                        m_uiLine = 0;                                           ///< Current line of the token.
    uint32_t                        m_uiCol = 0;                                            ///< Current column of the token.
    ETokenType                      m_eType = ETokenType::token_none;                       ///< The type of the token.
    ETokenLiteralType               m_eLiteralType = ETokenLiteralType::token_undefined;    ///< The type for literal tokens.
    ETokenMetaType                  m_eMetaType = ETokenMetaType::token_undefined;          ///< The type for meta tokens.
    mutable std::string             m_ssCache;                                              ///< Cached string created when
                                                                                            ///< requesting a string chunk.
};

#endif // !defined LOCATION_H