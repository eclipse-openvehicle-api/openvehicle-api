#ifndef CODEPOS_H
#define CODEPOS_H

#include "lexerbase.h"
#include <cstdint>
#include <list>
#include <string>

// Forward declaration
class CToken;

/**
 * @brief Code string with position information.
 */
class CCodePos
{
public:
    /**
     * @brief Default constructor
     * @remarks Use the assignment operator to set the code to be covered by the code position class.
     */
    CCodePos();

    /**
     * @brief Constructor
     * @param[in] szCode Pointer to zero terminated string containing the IDL code. Must not be NULL.
     */
    CCodePos(const char* szCode);

    /**
     * @brief Copy constructor
     * @param[in] rCode Reference to the code to copy.
     */
    CCodePos(const CCodePos& rCode);

    /**
     * @brief Move constructor
     * @param[in] rCode Reference to the code to move.
     */
    CCodePos(CCodePos&& rCode) noexcept;

    /**
     * @{
     * @brief Is the code position class initialized correctly?
     * @return Returns whether the class is initialized.
     */
    bool IsValid() const;
    operator bool() const;
    /**
     * @}
     */

    /**
     * @brief Reset the current navigation.
     * @attention Any prepended code will be removed.
     */
    void Reset();

    /**
     * @brief Return the current line.
     * @return The current line.
     */
    uint32_t GetLine() const;

    /**
     * @brief Return the current column.
     * @return The current column.
     */
    uint32_t GetCol() const;

    /**
     * @brief Check for EOF.
     * @return Returns 'true' when EOF has been reached. Otherwise returns 'false'.
     */
    bool HasEOF() const;

    /**
     * @brief Check for EOL.
     * @return Returns 'true' when EOL has been reached. Otherwise returns 'false'.
     */
    bool HasEOL() const;

    /**
     * @brief Get the token at the current location.
     * @attention The returned position is volatile and might be invalid as soon as code is prepended. Once the UpdateLocation
     * has been called, the code is persistent.
     * @param[in] eTokenType The type of token this token is referring to.
     * @return The token at the current location.
     */
    CToken GetLocation(ETokenType eTokenType = ETokenType::token_none) const;

    /**
     * @{
     * @brief Update the length of the token with current position. Persists the code if composed from (part of) prepended code.
     * @param[in, out] rtoken Reference to the token to be updated.
     */
    void UpdateLocation(CToken& rtoken) const;

    /**
    * @brief Update the length of the token with current position. Persists the code if composed from (part of) prepended code.
    * @param[in, out] rtoken Reference to the token to be updated.
    * @param[in] eLiteralType The literal type to be assigned to the token (if any).
    */
    void UpdateLocation(CToken& rtoken, ETokenLiteralType eLiteralType) const;

    /**
    * @brief Update the length of the token with current position. Persists the code if composed from (part of) prepended code.
    * @param[in, out] rtoken Reference to the token to be updated.
    * @param[in] eMetaType The meta type to be assigned to the token (if any).
    */
    void UpdateLocation(CToken& rtoken, ETokenMetaType eMetaType) const;

    /**
     * @brief Insert code at the current position allowing to parse the prepended code.
     * @details This function prepends code at the current position. The code could be a macro expansion or precedence parsing.
     * @attention This function might invalidate tokens that have a start location, but no fixed location.
     * @param[in] rssCode Reference to a string object representing the code to prepend
     */
    void PrependCode(const std::string& rssCode);

    /**
     * @brief Return whether the current position is part of the prepended code.
     * @return Returns 'true' when the current position is within the prepended code; otherwise returns 'false'.
     */
    bool CurrentPositionInMacroExpansion() const;

    /**
     * @brief Assignment operator.
     * @param[in] rCode Reference to the code to copy.
     * @return Returns reference to this class.
     */
    CCodePos& operator=(const CCodePos& rCode);

    /**
     * @brief Move operator.
     * @param[in] rCode Reference to the code to move.
     * @return Returns reference to this class.
     */
    CCodePos& operator=(CCodePos&& rCode) noexcept;

    /**
     * @brief Get current character (with offset)
     * @param[in] uiOffset The offset of the character to read relative to the current character.
     * @attention The caller must make certain, that a valid character is available at the provided offset by interpreting the
     * characters before.
     * @return The current character or '\0' when there is no current character.
     */
    char operator[](uint32_t uiOffset /*= 0*/) const;

    /**
     * @brief Get current character
     * @attention The caller must make certain, that a valid character is available at the provided offset by interpreting the
     * characters before.
     * @return The current character or '\0' when there is no current character.
     */
    char operator*() const;

    /**
     * @brief Get the code at the current position.
     * @attention The caller must make certain, that a valid character is available at the provided offset by interpreting the
     * characters before.
     * @return Pointer to the code at the current location or nullptr when there is no code.
     */
    operator const char*() const;

    /**
     * @brief Increment the current position
     * @return Copy of the code position before incrementation.
     */
    CCodePos operator++(int);

    /**
     * @brief Increment the current position
     * @return Reference to this class.
     */
    CCodePos& operator++();

    /**
     * @brief Increment the current position
     * @param[in] uiOffset The offset of the character to increment to.
     * @return Reference to this class.
     */
    CCodePos& operator+=(uint32_t uiOffset);

 private:
    const char*                     m_szCode = nullptr;             ///< The code
    const char*                     m_szCurrent = nullptr;          ///< The current position in the code
    const char*                     m_szPrependedPos = nullptr;     ///< The position in the code where the prepended code is
                                                                    ///< inserted into.
    const char*                     m_szPrependedCode = nullptr;    ///< The start position of the prepended code.
    const char*                     m_szPrependedCurrent = nullptr; ///< The current position in the prepended code;
    std::string                     m_ssPrependedCode;              ///< The prepended code being returned before parsing of the
                                                                    ///< original code continues.
    mutable std::list<std::string>  m_lstCodeChunks;                ///< List of code chunks existing of part or complete
                                                                    ///< prepended code.
    uint32_t                        m_uiLine = 0;                   ///< The current line
    uint32_t                        m_uiCol = 0;                    ///< The current column
};

#endif // !defined CODEPOS_H