#ifndef LEXER_H
#define LEXER_H

#include "lexerbase.h"
#include "token.h"
#include "tokenlist.h"
#include "codepos.h"
#include <cinttypes>
#include <exception>
#include <string>
#include <cassert>

/**
 * @brief OMG IDL 4.2 keywords
 * @remarks Dependable on the extension settings, the keywords are extended with: char16, char32, u8string, u16string, u32string,
 * null, pointer, interface_id, interface_t and exception_id.
 */
const std::vector<std::string> g_vecOmgIdlKeywords = {
    "abstract", "any", "alias", "attribute", "bitfield", "bitmask", "bitset", "boolean", "case", "char", "component", "connector",
    "const", "consumes", "context", "custom", "default", "double", "exception", "emits", "enum", "eventtype", "factory", "finder",
    "fixed", "float", "getraises", "home", "import", "in", "inout", "interface", "local", "long", "manages", "map", "mirrorport",
    "module", "multiple", "native", "Object", "octet", "oneway", "out", "primarykey", "private", "port", "porttype", "provides",
    "public", "publishes", "raises", "readonly", "setraises", "sequence", "short", "string", "struct", "supports", "switch",
    "truncatable", "typedef", "typeid", "typename", "typeprefix", "unsigned", "union", "uses", "ValueBase", "valuetype", "void",
    "wchar", "wstring", "int8", "uint8", "int16", "int32", "int64", "uint16", "uint32", "uint64"
};

/**
 * @brief SDV IDL lexer class
 */
class CLexer
{
public:
    /**
     * @brief Lexing mode enumerator.
     */
    enum class ELexingMode
    {
        lexing_idl,         ///< Lexing IDL code.
        lexing_preproc,     ///< Lexing preprocessor directives (valid for the current line only).
    };

    /**
     * @brief Constructor
     * @param[in] pCallback Pointer to the lexer callback interface. Must not be NULL.
     * @param[in] bCaseSensitive When set, allow identical names that only differ in case.
     * @param[in] eLexingMode The lexing mode the lexer should run in. This determines the rule-set to use while lexing.
     */
    CLexer(ILexerCallback* pCallback, bool bCaseSensitive, ELexingMode eLexingMode = ELexingMode::lexing_idl);

    /**
     * @brief Destructor
     */
    virtual ~CLexer() = default;

    /**
     * Add keywords to the reserved keyword list (based on the enabled extension).
     * @param[in] rssKeyword Reference to the keyword string to add.
     */
    void AddKeyword(const std::string& rssKeyword);

    /**
     * @brief Get a token from the code.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     * @throws Throws CCompileException on parse error.
     * @remarks Whitespace, comments and preprocessor directions are not provided as a result, but are provided through the
     * callback interface.
     * @post Updates the position to the token following the token.
     * @return Returns the read token or an empty token when OEF has been reached.
     */
    CToken GetToken(CCodePos& rCode, const CContextPtr& rptrContext) const;

    /**
     * @brief Get the last valid token.
     * @return Returns a reference to the member variable containing the last read token or an empty token when no token was read
     * before.
     */
    const CToken& GetLastValidToken() const;

    /**
     * @brief Read until the provided symbol or the end of the text.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @param[in] cSymbol The symbol to mark the end.
     * @post Updates the position to the token of the symbol.
     * @return Returns the token until the symbol or end of text has been reached.
     */
    CToken GetCustom(CCodePos& rCode, char cSymbol) const;

    /**
     * @brief Skip the rest of the line.
     * @remarks End position is at the carriage return or newline for preprocessing directives and past the carriage return or
     * newline for code.
     * @remarks Sets the newline-occurred flag for code.
     * @remarks Back-slash at the end of the line causes the inclusion of the next line.
     * @param[in, out] rCode Reference to the code to be parsed.
     */
    void SkipLine(CCodePos& rCode) const;

    /**
     * @brief Enable preproc processing.
     * @details While parsing preprocessor directive, further preprocessor processing is disabled. If including an additional
     * file, preprocessing needs to be enabled again.
     */
    void EnablePreprocProcessing();

private:
    /**
     * @brief Get whitespace (space, form feed, line feed, carriage return, horizontal tab, vertical tab) until there is no white
     * space any more.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @param[out] rbNewline Set when a newline has been passed.
     * @throws Throws CCompileException on parse error.
     * @post Updates the position to the token following the whitespace.
     */
    CToken GetWhitespace(CCodePos& rCode, bool& rbNewline) const;

    /**
     * @brief Get C-style and C++ comments.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @throws Throws CCompileException on parse error.
     * @post Updates the position to the token following the comment.
     */
    CToken GetComments(CCodePos& rCode) const;

    /**
     * @brief Get the identifier or keyword.
     * @details Get the identifier or keyword. A keyword corresponds to the list of keywords defined by the OMG IDL. All others
     * are identifiers. Identifiers that differ from a keyword through case differences are illegal.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @throws Throws CCompileException on parse error.
     * @post Updates the position to the token following the identifier.
     * @return Returns the identifier token or an empty token when OEF has been reached.
     */
    CToken GetIdentifierOrKeyword(CCodePos& rCode) const;

    /**
     * @brief Get the separator: { } ( ) : :: ; .
     * @param[in, out] rCode Reference to the code to be parsed.
     * @post Updates the position to the token following the separator.
     * @return Returns the separator token or an empty token when no separator is at the requested token.
     */
    CToken GetSeparator(CCodePos& rCode) const;

    /**
     * @brief Get the operator: = + - * / % ^ ! ~ , | # || & && == != < <= > >= ?
     * @remarks The '#' symbol is only an operator when lexing preprocessor directives. This is covered by the GetToken function.
     * @param[in, out] rCode Reference to the code to be parsed.
     * @post Updates the position to the token following the operator.
     * @return Returns the operator token or an empty token when no operator is at the requested token.
     */
    CToken GetOperator(CCodePos& rCode) const;

    /**
     * @brief Get the literal: hex, decimal or octal number, floating/fixed point number, character, strings, boolean and nullptr
     * values.
     * @remarks The following extension on the OMG IDL specification are possible:
     * - binary integer literal as is part of the C++ 14 standard.
     * - integer suffixes as are part of the C++ standard.
     * - unicode UTF-16 and UTF-32 character literals as part of the C++ 11 standard.
     * - ASCII and wide string sequences (2, 4 or 8 characters) as part oft the C++ standard.
     * - Unicode UTF-8, UTF-16, UTF-32 and wide string literals as part of the C++ 11 standard.
     * - raw ASCII, UTF-8, UTF-16, UTF-32 and wide string literals as is part of the C++ 11 standard.
     * - character and string escape sequence using 8 digits to specify a Unicode character as is part of the C++ standard.
     * - hexadecimal floating points as are part of the C++ 17 standard.
     * - floating point suffixes as are part of the C++ standard.
     * - boolean literal as part of the C++ standard (the values 'true' and 'false' as well as 'TRUE' and 'FALSE').
     * - nullptr literal as part of the C++ 11 standard (the values 'nullptr' and 'NULL').
     * @param[in, out] rCode Reference to the code to be parsed.
     * @throws Throws CCompileException on parse error.
     * @post Updates the position to the token following the literal.
     * @return Returns the literal token or an empty token when OEF has been reached.
     */
    CToken GetLiteral(CCodePos& rCode) const;

    ILexerCallback*             m_pLexerCallback = nullptr;                 ///< The lexer callback for inserting comments
    bool                        m_bCaseSensitive = true;                    ///< Case sensitivity during name comparison.
    ELexingMode                 m_eLexingMode = ELexingMode::lexing_idl;    ///< Lexing mode (changes the lexing rules).
    mutable bool                m_bNewlineOccurred = true;                  ///< A newline occurred - a preprocessor directive could take place.
    mutable CToken              m_tokenLastValid;                           ///< The last valid token.
    std::vector<std::string>    m_vecReservedKeywords;                      ///< List of reserved keywords.
};

/**
 * @brief Tokenize a string into a token list.
 * @param[in] szCode The code to tokenize.
 * @param[in] rptrContext Reference to the smart pointer to the source code context.
 * @return The tokenized string.
 */
CTokenList Tokenize(const char* szCode, const CContextPtr& rptrContext);

/**
 * @brief Tokenize a string into a token list.
 * @param[in] rssCode The code to tokenize.
 * @param[in] rptrContext Reference to the smart pointer to the source code context.
 * @return The tokenized string.
 */
CTokenList Tokenize(const std::string& rssCode, const CContextPtr& rptrContext);

/**
 * @brief Dummy callback interface for the lexer ignoring all calls.
 */
struct SLexerDummyCallback : public ILexerCallback
{
    /**
     * @brief Insert whitespace. Dummy implementation. Overload of ILexerCallback::InsertWhitespace.
     */
    virtual void InsertWhitespace(const CToken& /*rtoken*/) override {}

    /**
     * @brief Insert a comment. Dummy implementation. Overload of ILexerCallback::InsertComment.
     */
    virtual void InsertComment(const CToken& /*rtoken*/) override {}

    /**
     * @brief Process a preprocessor directive. Overload of ILexerCallback::ProcessPreprocDirective.
     * @param[in] rCode Reference to the source code to process the preproc directive for.
     */
    virtual void ProcessPreprocDirective(CCodePos& rCode) override
    { SLexerDummyCallback sCallback; CLexer(&sCallback, true).SkipLine(rCode); }
};

/**
 * @brief Callback interface for the lexer storing the provided information.
 */
struct SLexerStoreCallback : public ILexerCallback
{
    /**
     * @brief Clear the value of the callback structure.
     */
    void Clear()
    {
        tokenWhitespace = CToken();
        tokenComment = CToken();
        ssPreprocLine.clear();
    }

    /**
     * @brief Insert whitespace. Overload of ILexerCallback::InsertWhitespace.
     * @param[in] rtoken Reference to token containing the whitespace.
     */
    virtual void InsertWhitespace(const CToken &rtoken) override { tokenWhitespace = rtoken; }

    /**
     * @brief Insert a comment. Overload of ILexerCallback::InsertComment.
     * @param[in] rtoken Reference to the token containing the comment.
     */
    virtual void InsertComment(const CToken &rtoken) override { tokenComment = rtoken; }

    /**
     * @brief Process a preprocessor directive. Overload of ILexerCallback::ProcessPreprocDirective.
     * @param[in] rCode Reference to the source code to process the preproc directive for.
     */
    virtual void ProcessPreprocDirective(CCodePos &rCode) override
    {
        SLexerDummyCallback sCallback;
        CLexer lexer(&sCallback, true);
        CToken token = rCode.GetLocation();
        lexer.SkipLine(rCode);
        rCode.UpdateLocation(token);
        ssPreprocLine = static_cast<std::string>(token);
    }

    CToken tokenWhitespace;     ///< Token holding whitespace.
    CToken tokenComment;        ///< Token holding comment.
    std::string ssPreprocLine;  ///< String holding preprocessing line.
};

#endif // !defined LEXER_H