#include "lexer.h"
#include "exception.h"
#include "support.h"

CLexer::CLexer(ILexerCallback* pCallback, bool bCaseSensitive, ELexingMode eLexingMode /*= ELexingMode::lexing_idl*/) :
    m_pLexerCallback(pCallback), m_bCaseSensitive(bCaseSensitive), m_eLexingMode(eLexingMode), 
    m_vecReservedKeywords(g_vecOmgIdlKeywords)
{
    if (!pCallback)
        throw CCompileException("Invalid parameter; no callbback provided.");
}

void CLexer::AddKeyword(const std::string& rssKeyword)
{
    m_vecReservedKeywords.push_back(rssKeyword);
}

CToken CLexer::GetToken(CCodePos& rCode, const CContextPtr& rptrContext) const
{
    if (!rCode.IsValid()) return CToken();

    // Based on the character value decide what to do. For this a tabe is used mapping a character (byte) to a classification.
    // This table uses the UTF-8 coding scheme. Character values under 0x20 are control and space characters. Character values
    // between 0x80 and 0xBF are part of a multi-byte character and therefore have no parsing function when found stand-alone.
    // Character values between 0xC0 and 0xF7 are the initial part of a multi-byte character and therefore are valid participants
    // of an identifier. Character values between 0xF8 and 0xFF are not defined.
    // The following abbreviatios are used:
    //  none    - not classified
    //  eof     - end of file (end of string)
    //  space   - whitespace
    //  ident   - identfier
    //  lit     - literal (number, string)
    //  idlit   - identifier or literal
    //  split   - separator or literal
    //  sep     - separator
    //  oper    - operator
    //  opcom   - operator or comments
    //  pproc   - pre-processor directive or symbol
    // Remarks:
    const enum {none, eof, space, ident, lit, idlit, split, sep, oper, opcom, pproc} rgeClassify[256] =
    {
    //  0x0    0x1    0x2    0x3    0x4    0x5    0x6    0x7    0x8    0x9    0xA    0xB    0xC    0xD    0xE    0xF
        eof,   none,  none,  none,  none,  none,  none,  none,  none,  space, space, space, space, space, none,  none,  // 0x00
        none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  // 0x10
        space, oper,  lit,   pproc, none,  oper,  oper,  lit,   sep,   sep,   oper,  oper,  oper,  oper,  split, opcom, // 0x20
        lit,   lit,   lit,   lit,   lit,   lit,   lit,   lit,   lit,   lit,   sep,   sep,   oper,  oper,  oper,  sep,   // 0x30
        none,  ident, ident, ident, ident, ident, idlit, ident, ident, ident, ident, idlit, idlit, ident, idlit, ident, // 0x40
        ident, ident, idlit, ident, idlit, idlit, ident, ident, ident, ident, ident, sep,   space, sep,   oper,  ident, // 0x50
        none,  ident, ident, ident, ident, ident, idlit, ident, ident, ident, ident, ident, ident, ident, idlit, ident, // 0x60
        ident, ident, ident, ident, idlit, idlit, ident, ident, ident, ident, ident, sep,   oper,  sep,   oper,  none,  // 0x70
        none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  // 0x80
        none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  // 0x90
        none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  // 0xA0
        none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  none,  // 0xB0
        ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, // 0xC0
        ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, // 0xD0
        ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, ident, // 0xE0
        ident, ident, ident, ident, ident, ident, ident, ident, none,  none,  none,  none,  none,  none,  none,  none   // 0xF0
    };

    // Determine the action to do; repeat if needed
    CToken token;
    bool bRetry = false;
    do
    {
        bRetry = false;
        switch (rgeClassify[static_cast<uint32_t>(*rCode)])
        {
        case eof:
            return CToken(); // End of file/end of string.
            break;
        case space:
            token = GetWhitespace(rCode, m_bNewlineOccurred);
            token.SetContext(rptrContext);

            // Callback on the whitespace
            if (token.GetLength())
            {
                if (m_pLexerCallback)
                    m_pLexerCallback->InsertWhitespace(token);

                // Continue processing
                bRetry = true;
                break;
            }
            else
                token = CToken(); // End of string.
            break;
        case ident:
            token = GetIdentifierOrKeyword(rCode);
            m_bNewlineOccurred = false;
            break;
        case lit:
            token = GetLiteral(rCode);
            m_bNewlineOccurred = false;
            break;
        case idlit:
            // Capturing wide, UTF-8, UTF-16, UTF-32 strings and raw strings
            if ((rCode[1] == '\"' || rCode[1] == '\'') ||
                (std::isalnum(rCode[1]) && (rCode[2] == '\"' || rCode[2] == '\'')) ||
                (std::isalnum(rCode[1]) && std::isalpha(rCode[2]) && (rCode[3] == '\"' || rCode[3] == '\'')))
                token = GetLiteral(rCode);
            else
            {
                // Check for the literals true, false and nullptr
                CToken tokenTemp = rCode.GetLocation();
                if (static_cast<std::string>(tokenTemp).substr(0, 4) == "true" && !std::isalnum(rCode[4]))
                    token = GetLiteral(rCode);
                else if (static_cast<std::string>(tokenTemp).substr(0, 4) == "TRUE" && !std::isalnum(rCode[4]))
                    token = GetLiteral(rCode);
                else if (static_cast<std::string>(tokenTemp).substr(0, 5) == "false" && !std::isalnum(rCode[5]))
                    token = GetLiteral(rCode);
                else if (static_cast<std::string>(tokenTemp).substr(0, 5) == "FALSE" && !std::isalnum(rCode[5]))
                    token = GetLiteral(rCode);
                else if (static_cast<std::string>(tokenTemp).substr(0, 7) == "nullptr" && !std::isalnum(rCode[7]))
                    token = GetLiteral(rCode);
                else if (static_cast<std::string>(tokenTemp).substr(0, 4) == "NULL" && !std::isalnum(rCode[4]))
                    token = GetLiteral(rCode);
                else
                    token = GetIdentifierOrKeyword(rCode);
            }
            m_bNewlineOccurred = false;
            break;
        case split:
            // Capturing floating points starting with a dot
            if (std::isdigit(rCode[1]))
                token = GetLiteral(rCode);
            else
                token = GetSeparator(rCode);
            break;
        case sep:
            token = GetSeparator(rCode);
            m_bNewlineOccurred = false;
            break;
        case oper:
            token = GetOperator(rCode);
            m_bNewlineOccurred = false;
            break;
        case opcom:
            if (rCode[1] == '/' || rCode[1] == '*')
            {
                token = GetComments(rCode);
                token.SetContext(rptrContext);

                // Callback on the comment.
                if (m_pLexerCallback)
                    m_pLexerCallback->InsertComment(token);

                // Continue processing
                bRetry = true;
                break;
            }
            else
            {
                token = GetOperator(rCode);
                m_bNewlineOccurred = false;
            }
            break;
        case pproc:
            if (m_eLexingMode != ELexingMode::lexing_preproc)
            {
                // The '#' token is only a valid operator when lexing preprocessor directives
                if (m_bNewlineOccurred)
                {
                    // Callback on the preprocessor.
                    if (m_pLexerCallback)
                        m_pLexerCallback->ProcessPreprocDirective(rCode);

                    // REMARKS: The preprocessor could include another file. This will change the context of the parser. Return an
                    // empty token to indicate the parser to try to get the next token.
                    token = CToken();
                }
                else
                    throw CCompileException(rCode.GetLocation(), "Unexpected token '#'.");
            }
            else
            {
                token = GetOperator(rCode);
                m_bNewlineOccurred = false;
            }
            break;
        case none:
        default:
            throw CCompileException(rCode.GetLocation(), "Unexpected token '", *rCode, "'.");
            break;
        }
    } while (bRetry);   // Retry when some intermediate processing took place.

    if (token)
    {
        m_tokenLastValid = token;
        token.SetContext(rptrContext);
    }
    return token;
}

const CToken& CLexer::GetLastValidToken() const
{
    return m_tokenLastValid;
}

CToken CLexer::GetCustom(CCodePos& rCode, char cSymbol) const
{
    if (!rCode.IsValid()) return CToken();

    // Initialize the token
    CToken token = rCode.GetLocation();

    // Run until done
    bool bDone = false;
    while (!bDone)
    {
        switch (*rCode)
        {
        case '\\':
            if (rCode[1] == '\n')
                rCode += 2;
            else if (rCode[1] == '\r' && rCode[2] == '\n')
                rCode += 3;
            else
                ++rCode;
            break;
        case '\r':
            if (m_eLexingMode == ELexingMode::lexing_preproc && rCode[1] == '\n')
                bDone = true;
            else
                ++rCode;
            break;
        case '\n':
            if (m_eLexingMode == ELexingMode::lexing_preproc)
                bDone = true;
            else
            {
                ++rCode;
                m_bNewlineOccurred = true;
            }
            break;
        case '\0':
            bDone = true;
            break;
        default:
            if (*rCode == cSymbol)
                bDone = true;
            else
                ++rCode;
            break;
        }
    }

    // Update the token
    rCode.UpdateLocation(token);

    if (token) m_tokenLastValid = token;
    return token;
}

void CLexer::SkipLine(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return;

    bool bDone = false;
    while (!bDone)
    {
        switch(*rCode)
        {
        case '\0':
            bDone = true;
            break;
        case '\r':
            if (rCode[1] == '\n')
            {
                if (m_eLexingMode != ELexingMode::lexing_preproc)
                {
                    rCode += 2;
                    m_bNewlineOccurred = true;
                }
                bDone = true;
            }
            else
            {
                m_bNewlineOccurred = false;
                ++rCode;
            }
            break;
        case '\n':
            if (m_eLexingMode != ELexingMode::lexing_preproc)
            {
                ++rCode;
                m_bNewlineOccurred = true;
            }
            bDone = true;
            break;
        case '\\':
            if (rCode[1] == '\n')
                rCode += 2;
            else if (rCode[1] == '\r' && rCode[2] == '\n')
                rCode += 3;
            else
            {
                m_bNewlineOccurred = false;
                ++rCode;
            }
            break;
        default:
            m_bNewlineOccurred = false;
            ++rCode;
            break;
        }
    }
}

void CLexer::EnablePreprocProcessing()
{
    m_bNewlineOccurred = true;
}

CToken CLexer::GetWhitespace(CCodePos& rCode, bool& rbNewline) const
{
    if (!rCode.IsValid()) return CToken();

    CToken token = rCode.GetLocation(ETokenType::token_whitespace);
    bool bDone = false;
    while (!bDone)
    {
        // Check the current position.
        // Space, tab, line feed and carriage return are considered whitespace.
        switch (*rCode)
        {
        case ' ':   // Space
        case '\t':  // Tab
        case '\f':  // Form feed
        case '\v':  // Vertical tab
            ++rCode;
            break;
        case '\r':  // Carriage return
            if (rCode[1] == '\n')
            {
                if (m_eLexingMode != ELexingMode::lexing_preproc)
                {
                    rCode += 2;
                    rbNewline = true;
                } else
                    bDone = true;
            } else
                ++rCode;
            break;
        case '\n':  // Line feed
            if (m_eLexingMode != ELexingMode::lexing_preproc)
            {
                ++rCode;
                rbNewline = true;
            } else
                bDone = true;
            break;
        case '\\':
            if (rCode[1] == '\n')
                rCode += 2;
            else if (rCode[1] == '\r' && rCode[2] == '\n')
                rCode += 3;
            else
                bDone = true;
            break;
        case '\0':
            bDone = true;
            break;
        default:
            bDone = true;
            break;
        }
    }

    // Update the token
    rCode.UpdateLocation(token);

    return token;
}

CToken CLexer::GetComments(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return CToken();

    // Initialize the token
    CToken sToken = rCode.GetLocation(ETokenType::token_comments);

    // C-style comment?
    if (rCode[0] == '/' && rCode[1] == '*')
    {
        // Skip the comment opening
        rCode += 2;

        // Skip characters until comment closing
        bool bDone = false;
        bool bEnd = false;
        while (!bDone)
        {
            switch (*rCode)
            {
            case '*':
                if (rCode[1] == '/')
                {
                    // Skip the comment closing
                    rCode += 2;

                    bEnd = true;
                    bDone = true;
                }
                else
                    ++rCode;
                break;
            case '\r':
                if (rCode[1] == '\n')
                {
                    if (m_eLexingMode != ELexingMode::lexing_preproc)
                        rCode += 2;
                    else
                        bDone = true;
                } else
                    ++rCode;
                break;
            case '\n':
                if (m_eLexingMode != ELexingMode::lexing_preproc)
                    ++rCode;
                else
                    bDone = true;
                break;
            case '\0':
                bDone = true;
                break;
            default:
                ++rCode;
                break;
            }
        }

        // Comment was closed properly?
        if (bEnd)
        {
            // Update the token
            rCode.UpdateLocation(sToken);
        } else
            throw CCompileException(rCode.GetLocation(ETokenType::token_comments), "End of file while looking for a close comment statement.");
    }
    else if (rCode[0] == '/' && rCode[1] == '/') // C++-style comment?
    {
        // Skip the comment opening
        rCode += 2;

        bool bDone = false;
        while (!bDone)
        {
            switch (*rCode)
        {
            case '\r':
                if (rCode[1] == '\n')
                {
                    if (m_eLexingMode != ELexingMode::lexing_preproc)
                        rCode += 2;
                    bDone = true;
                } else
                    ++rCode;
                break;
            case '\n':
                if (m_eLexingMode != ELexingMode::lexing_preproc)
                    ++rCode;
                bDone = true;
                break;
            case '\0':
                bDone = true;
                break;
            default:
                ++rCode;
                break;
            }

            // Update the token
            rCode.UpdateLocation(sToken);
        }
    } else
        throw CCompileException(rCode.GetLocation(ETokenType::token_comments), "Expecting C-sytel or C++ comments.");

    return sToken;
}

CToken CLexer::GetIdentifierOrKeyword(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return CToken();

    // Check for an alphabetical character.
    if (!std::isalpha(*rCode) && *rCode != '_')
        throw CCompileException(rCode.GetLocation(), "Invalid symbol; expected identifier.");

    // Initialize two tokens: identifier and keyword (this is not an expensive action)
    CToken tokenIdentifier = rCode.GetLocation(ETokenType::token_identifier);
    CToken tokenKeyword = rCode.GetLocation(ETokenType::token_keyword);

    // Continue until the current character is not alpha-numerical any more
    while (std::isalnum(*rCode) || *rCode == '_')
    {
        ++rCode;
    }

    // Update the tokens
    rCode.UpdateLocation(tokenIdentifier);
    rCode.UpdateLocation(tokenKeyword);

    // Special treatment if not lexing preprocessor directives
    if (m_eLexingMode != ELexingMode::lexing_preproc)
    {
        // No quote or apostrophe character allowed to follow directly.
        if (*rCode == '\"' || *rCode == '\'')
            throw CCompileException(rCode.GetLocation(), "Invalid characters following identifier.");

        // Check for a keyword or the collision with a keyword
        for (const std::string& rssKeyword : m_vecReservedKeywords)
        {
            // Check for the token to be exactly the keyword.
            if (tokenKeyword == rssKeyword)
                return tokenKeyword;

            // If not case-sensitive, check for the token to be like the keyword with case differences.
            if (!m_bCaseSensitive)
            {
                const std::string& rssIdentifier = static_cast<std::string>(tokenIdentifier);
                if (std::equal(rssIdentifier.begin(), rssIdentifier.end(),
                    rssKeyword.begin(), rssKeyword.end(),
                    [](char a, char b) { return std::tolower(a) == std::tolower(b); }))
                    throw CCompileException(tokenIdentifier, "Identifier collides with reserved keyword (they differ only in case).");
            }
        }
    }

    // The identifier is not a keyword and doesn't collide with any keyword.
    return tokenIdentifier;
}

CToken CLexer::GetSeparator(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return CToken();

    // Check for a separator
    CToken sToken;
    switch (*rCode)
    {
    case '{':
    case '}':
    case '(':
    case ')':
    case '[':
    case ']':
    case ';':
    case '.':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_separator);

        // Skip the separator
        ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    case ':':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_separator);

        // Skip the separator
        ++rCode;

        // Check for another colon - this would be the scope separator
        if (*rCode == ':')
            ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    default:
        throw CCompileException(rCode.GetLocation(), "Invalid symbol; expected separator { } ( ) [ ] : :: ; .");
        break;
    }

    return sToken;
}

CToken CLexer::GetOperator(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return CToken();

    // Check for an operator
    CToken sToken;
    switch (*rCode)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '~':
    case ',':
    case '?':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_operator);

        // Skip the token
        ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    case '#':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_operator);

        // Skip the number sign
        ++rCode;

        // Another number sign might indicate that the operator is a concatinating operator, otherwise it is a stringification
        // operator.
        if (*rCode == '#')
            ++rCode;

        // Another number sign is not allowed
        if (*rCode == '#')
            throw CCompileException(rCode.GetLocation(), "Invalid operator");

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    case '|':
    case '&':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_operator);

        // Deal with bitwise and logical operators (latter having two characters).
        if (rCode[0] == rCode[1])
            rCode += 2;
        else
            ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    case '=':
    case '!':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_identifier);

        // Deal with unary and comparison operators (latter having often two characters).
        if (rCode[1] == '=')
            rCode += 2;
        else
            ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    case '<':
    case '>':
        // Initialize the token
        sToken = rCode.GetLocation(ETokenType::token_identifier);

        // Skip the separator
        // Deal with bitwise shift and comparison operators having one or two characters.
        if (rCode[1] == '=' || rCode[0] == rCode[1])
            rCode += 2;
        else
            ++rCode;

        // Update the token
        rCode.UpdateLocation(sToken);

        break;
    default:
        throw CCompileException(rCode.GetLocation(),
            "Invalid symbol; expected operator = + - * / % ^ ! ~ , << >> | || & && == != < <= > >= # ## ?");
        break;
    }

    return sToken;
}

CToken CLexer::GetLiteral(CCodePos& rCode) const
{
    if (!rCode.IsValid()) return CToken();

    // Check for different literal types
    // Integer examples:
    //  1, 10u, 20U, 010, 0x10L, 0x100LL
    // Floating point examples:
    //  1e10, 1e-5L, 1., 1.e-2, 3.14, .1f, 0.1e-1L, 0x1ffp10, 0X0p-1, 0x1.p0, 0xf.p-1, 0x0.123p-1, 0xa.bp10l
    // String examples:
    //  "...", u8"...", u"...", U"...", L"...", u8R"abc(...)abc"
    // Character examples:
    //  'x', u8't', u'b', U'r', L'b', 'abcd', 'L'abcde'
    CCodePos codeLocal(rCode);
    ETokenLiteralType eTokenLiteralType = ETokenLiteralType::token_undefined;
    if (std::isdigit(*codeLocal) || *codeLocal == '.')
    {
        // Skip prefix (0x for hex and 0b for binary).
        const char* szCollection = "0123456789";    // Decimal collection
        eTokenLiteralType = ETokenLiteralType::token_literal_dec_integer;
        if (codeLocal[0] == '0' && static_cast<char>(std::tolower(codeLocal[1])) == 'x')
        {
            codeLocal += 2;
            szCollection = "0123456789abcdefABCDEF";    // Hexadecimal collection
            eTokenLiteralType = ETokenLiteralType::token_literal_hex_integer;
        } else if (codeLocal[0] == '0' && static_cast<char>(std::tolower(codeLocal[1])) == 'b')
        {
            codeLocal += 2;
            szCollection = "01";    // Binary collection
            eTokenLiteralType = ETokenLiteralType::token_literal_bin_integer;
        }
        else if (codeLocal[0] == '0')
        {
            // Prevent identifying float as octal number
            if (codeLocal[1] != 'e' && codeLocal[1] != 'E' && codeLocal[1] != '.')
            {
                szCollection = "01234567"; // Octal collection
                eTokenLiteralType = ETokenLiteralType::token_literal_oct_integer;
            }
        }

        // Read at least one number (only if not startiung with a dot)
        if (!std::strchr(szCollection, *codeLocal) && *codeLocal != '.')
            throw CCompileException(rCode.GetLocation(), "Invalid number literal");

        // Read number (with separators)
        while ((std::strchr(szCollection, *codeLocal) || *codeLocal == '\'') && *codeLocal != '\0')
        {
            ++codeLocal;
        }

        // Floating or fixed point?
        // Are detected by . p P e E d D or suffix f or l
        char cSuffix = static_cast<char>(std::tolower(*codeLocal));
        if ((eTokenLiteralType == ETokenLiteralType::token_literal_dec_integer &&
                (cSuffix == '.' || cSuffix == 'e' || cSuffix == 'd')) ||
            (eTokenLiteralType == ETokenLiteralType::token_literal_hex_integer && (cSuffix == '.' || cSuffix == 'p')))
        {
            if (*codeLocal == '.')
            {
                // Skip the separator
                ++codeLocal;

                // Read the decimal
                while ((std::strchr(szCollection, *codeLocal) || *codeLocal == '\'') && *codeLocal != '\0')
                {
                    ++codeLocal;
                }
            }

            // Change the type and check for exponent
            bool bExponent = false;
            if (eTokenLiteralType == ETokenLiteralType::token_literal_dec_integer)
            {
                eTokenLiteralType = ETokenLiteralType::token_literal_dec_floating_point;
                if (static_cast<char>(std::tolower(*codeLocal)) == 'e')
                {
                    bExponent = true;
                    ++codeLocal;
                }
                else if (static_cast<char>(std::tolower(*codeLocal)) == 'd')
                {
                    eTokenLiteralType = ETokenLiteralType::token_literal_fixed_point;
                    ++codeLocal;
                }
            }
            if (eTokenLiteralType == ETokenLiteralType::token_literal_hex_integer)
            {
                eTokenLiteralType = ETokenLiteralType::token_literal_hex_floating_point;
                if (static_cast<char>(std::tolower(*codeLocal)) == 'p')
                {
                    bExponent = true;
                    ++codeLocal;
                }
            }

            // Process exponent
            if (bExponent)
            {
                // Check for optional sign
                if (*codeLocal == '+' || *codeLocal == '-')
                {
                    ++codeLocal;
                }

                // A number is compulsory
                if (!std::isdigit(*codeLocal))
                    throw CCompileException(rCode.GetLocation(), "Invalid float literal");

                // Read the decimal number
                while (std::isdigit(*codeLocal))
                {
                    ++codeLocal;
                }
            }

            // Check for a float suffix (only for floating point)
            const char* szSuffix = "fFlL";
            if (eTokenLiteralType != ETokenLiteralType::token_literal_fixed_point &&
                std::strchr(szSuffix, *codeLocal) && *codeLocal != '\0')
            {
                ++codeLocal;
            }
        }
        else
        {
            // Read suffix
            bool bUnsigned = false;
            if (static_cast<char>(std::tolower(*codeLocal)) == 'u')
            {
                ++codeLocal;
                bUnsigned = true;
            }
            if (static_cast<char>(std::tolower(*codeLocal)) == 'l')
            {
                // Skip the suffex (could be l, L, ll or LL)
                if (codeLocal[0] == codeLocal[1])
                    codeLocal += 2;
                else
                    ++codeLocal;

                // Unsigned could also come after the long suffix
                if (!bUnsigned && (static_cast<char>(std::tolower(*codeLocal)) == 'u'))
                    ++codeLocal;
            }
        }
    }
    else     // Character, string, boolean or nullptr
    {
        CToken tokenTemp = codeLocal.GetLocation();
        if (static_cast<std::string>(tokenTemp).substr(0, 4) == "true" && !std::isalnum(codeLocal[4]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_boolean;
            codeLocal += 4;
        }
        else if (static_cast<std::string>(tokenTemp).substr(0, 4) == "TRUE" && !std::isalnum(codeLocal[4]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_boolean;
            codeLocal += 4;
        }
        else if (static_cast<std::string>(tokenTemp).substr(0, 5) == "false" && !std::isalnum(codeLocal[5]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_boolean;
            codeLocal += 5;
        }
        else if (static_cast<std::string>(tokenTemp).substr(0, 5) == "FALSE" && !std::isalnum(codeLocal[5]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_boolean;
            codeLocal += 5;
        }
        else if (static_cast<std::string>(tokenTemp).substr(0, 7) == "nullptr" && !std::isalnum(codeLocal[7]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_nullptr;
            codeLocal += 7;
        }
        else if (static_cast<std::string>(tokenTemp).substr(0, 4) == "NULL" && !std::isalnum(codeLocal[4]))
        {
            eTokenLiteralType = ETokenLiteralType::token_literal_nullptr;
            codeLocal += 4;
        }
        else // check for a string
        {
            // Encoding format of characters.
            enum class ECharEncoding : uint32_t
            {
                encoding_ascii,     //!< Encoding for ASCII ISO Latin-1 (8859-1) character set
                encoding_utf8,      //!< Encoding for UTF-8
                encoding_utf16,     //!< Encoding for UTF-16
                encoding_utf32,     //!< Encoding for UTF-32
                encoding_wide       //!< Encoding for wide character (platform specific)
            };

            // Check for Unicode pre-fix
            ECharEncoding eEncoding = ECharEncoding::encoding_ascii;
            switch (*codeLocal)
            {
            case 'u':
                ++codeLocal;
                if (*codeLocal == '8')
                {
                    eEncoding = ECharEncoding::encoding_utf8;
                    ++codeLocal;
                } else
                    eEncoding = ECharEncoding::encoding_utf16;
                break;
            case 'U':
                ++codeLocal;
                eEncoding = ECharEncoding::encoding_utf32;
                break;
            case 'L':
                eEncoding = ECharEncoding::encoding_wide;
                ++codeLocal;
                break;
            default:
                break;
            }

            // Check for raw prefix
            bool bRaw = false;
            if (*codeLocal == 'R')
            {
                bRaw = true;
                ++codeLocal;
            }

            // Check for quote or apostrophe (the beginning of a string or character).
            bool bIsString    = false;
            bool bIsCharacter = false;
            if (*codeLocal == '\"')
            {
                if (bRaw)
                    eTokenLiteralType = ETokenLiteralType::token_literal_raw_string;
                else
                    eTokenLiteralType = ETokenLiteralType::token_literal_string;
                bIsString = true;
            } else if (*codeLocal == '\'')
            {
                if (bRaw)
                    throw CCompileException(rCode.GetLocation(),"No raw character literal support.");

                if (eEncoding == ECharEncoding::encoding_utf8)
                    throw CCompileException(rCode.GetLocation(),
                        "No UTF-8 character literal support; use ASCII character literal instead.");
                eTokenLiteralType = ETokenLiteralType::token_literal_character;
                bIsCharacter = true;
            }

            // Process string or character
            if (bIsString || bIsCharacter)
            {
                // Skip left quote or apostrophe
                ++codeLocal;

                // If raw check for delimiter pattern (max 16 chars)
                std::string ssDelimiter;
                if (bRaw)
                {
                    // The pattern starts with a right parenthesis.
                    ssDelimiter += ')';

                    // Detect the (optional) raw pattern
                    for (auto i = 0; i < 16; i++)
                    {
                        if (!*codeLocal)
                            throw CCompileException(rCode.GetLocation(), "Invalid string; unexpected end of file detected.");

                        if (*codeLocal != '(')
                        {
                            ssDelimiter += *codeLocal;
                            ++codeLocal;
                        }
                        else
                            break;
                    }

                    // And add for the end quote
                    ssDelimiter += '\"';

                    // Raw strings must start with opening bracket
                    if (*codeLocal != '(')
                        throw CCompileException(rCode.GetLocation(), "Invalid string; expecting '('.");

                    // Skip opening bracket
                    ++codeLocal;
                } else
                    ssDelimiter = bIsCharacter ? "\'" : "\"";

                // Count the amount of real characters (and skip the amount of read characters).
                uint32_t uiByteCnt = 0;
                uint32_t uiCharCnt = 0;
                switch (eEncoding)
                {
                case ECharEncoding::encoding_ascii:
                {
                    std::string ssText;
                    InterpretCText(codeLocal, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw, true);
                    uiCharCnt = static_cast<uint32_t>(ssText.size());
                    break;
                }
                case ECharEncoding::encoding_utf8:
                {
                    std::string ssText;
                    InterpretCText(codeLocal, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
                    uiCharCnt = static_cast<uint32_t>(ssText.size());
                    break;
                }
                case ECharEncoding::encoding_utf16:
                {
                    std::u16string ssText;
                    InterpretCText(codeLocal, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
                    uiCharCnt = static_cast<uint32_t>(ssText.size());
                    break;
                }
                case ECharEncoding::encoding_utf32:
                {
                    std::u32string ssText;
                    InterpretCText(codeLocal, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
                    uiCharCnt = static_cast<uint32_t>(ssText.size());
                    break;
                }
                case ECharEncoding::encoding_wide:
                {
                    std::wstring ssText;
                    InterpretCText(codeLocal, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
                    uiCharCnt = static_cast<uint32_t>(ssText.size());
                    break;
                }
                }
                codeLocal += uiByteCnt;

                // Skip the delimiter
                codeLocal += static_cast<uint32_t>(ssDelimiter.size());

                // Determine whether the character literal is actually a sequence
                if (bIsCharacter && uiCharCnt > 1)
                {
                    switch (eEncoding)
                    {
                    case ECharEncoding::encoding_ascii:
                        if (uiCharCnt == 2 || uiCharCnt == 4 || uiCharCnt == 8)
                            eTokenLiteralType = ETokenLiteralType::token_literal_character_sequence;
                        else
                            throw CCompileException(codeLocal.GetLocation(),
                                                  "Invalid character sequence; only 2, 4 or 8 characters are allowed.");
                        break;
                    case ECharEncoding::encoding_wide:
                        if constexpr (sizeof(wchar_t) == 2)
                        {
                            if (uiCharCnt == 2 || uiCharCnt == 4)
                                eTokenLiteralType = ETokenLiteralType::token_literal_character_sequence;
                            else
                                throw CCompileException(codeLocal.GetLocation(),
                                                      "Invalid character sequence; only 2 or 4 characters are allowed.");
                        }
                        else // sizeof is 4
                        {
                            if (uiCharCnt == 2)
                                eTokenLiteralType = ETokenLiteralType::token_literal_character_sequence;
                            else
                                throw CCompileException(codeLocal.GetLocation(),
                                                      "Invalid character sequence; only 2 characters are allowed.");
                        }
                        break;
                    default:
                        throw CCompileException(codeLocal.GetLocation(),
                                              "Character sequences are only allowed with ASCII or wide characters.");
                    }
                }
            }
            else // Not a string, a character, a boolean or a nullptr
                throw CCompileException(rCode.GetLocation(), "Expecting a literal.");
        }
    }

    // Initialize the token
    CToken token = rCode.GetLocation(ETokenType::token_literal);

    // Update the position
    rCode = codeLocal;

    // No further digit and alpha-digits are allowed to follow the literal
    if (m_eLexingMode != ELexingMode::lexing_preproc && (std::isalnum(*rCode) || *rCode == '_'))
        throw CCompileException(rCode.GetLocation(), "Invalid characters following literal.");

    // Update the token
    rCode.UpdateLocation(token, eTokenLiteralType);

    // Check whether the token is valid
    if (!token)
        throw CCompileException(token, "Internal error: the token is not valid.");

    return token;
}

CTokenList Tokenize(const char* szCode, const CContextPtr& rptrContext)
{
    CCodePos code(szCode);
    SLexerDummyCallback sCallback;
    CLexer lexer(&sCallback, true);
    CToken token;
    CTokenList lstTokens;
    try
    {
        while ((token = lexer.GetToken(code, rptrContext)))
            lstTokens.push_back(std::move(token));
    }
    catch (const sdv::idl::XCompileError &)
    {}
    return lstTokens;
}

CTokenList Tokenize(const std::string& rssCode, const CContextPtr& rptrContext)
{
    return Tokenize(rssCode.c_str(), rptrContext);
}

