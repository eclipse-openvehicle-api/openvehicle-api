#include "codepos.h"
#include "lexer.h"
#include "exception.h"
#include "token.h"

CCodePos::CCodePos()
{}

CCodePos::CCodePos(const char* szCode) : m_szCode(szCode)
{
    if (!szCode)
        throw CCompileException("No code supplied.");
    m_szCurrent = szCode;
    m_uiLine = 1;
    m_uiCol = 1;
}

CCodePos::CCodePos(const CCodePos& rCode) :
    m_szCode(rCode.m_szCode), m_szCurrent(rCode.m_szCurrent), m_szPrependedPos(rCode.m_szPrependedPos),
    m_szPrependedCode(nullptr), m_szPrependedCurrent(nullptr), m_ssPrependedCode(rCode.m_ssPrependedCode),
    m_uiLine(rCode.m_uiLine), m_uiCol(rCode.m_uiCol)
{
    if (rCode.m_szPrependedCurrent && rCode.m_szPrependedCode)
    {
        m_szPrependedCode = m_ssPrependedCode.c_str();
        m_szPrependedCurrent = m_szPrependedCode + (rCode.m_szPrependedCurrent - rCode.m_szPrependedCode);
    }
}

CCodePos::CCodePos(CCodePos&& rCode) noexcept :
    m_szCode(rCode.m_szCode), m_szCurrent(rCode.m_szCurrent), m_szPrependedPos(rCode.m_szPrependedPos),
    m_szPrependedCode(rCode.m_szPrependedCode), m_szPrependedCurrent(rCode.m_szPrependedCurrent),
    m_ssPrependedCode(std::move(rCode.m_ssPrependedCode)), m_lstCodeChunks(std::move(rCode.m_lstCodeChunks)),
    m_uiLine(rCode.m_uiLine), m_uiCol(rCode.m_uiCol)
{
    rCode.m_szCode = nullptr;
    rCode.m_szCurrent = nullptr;
    rCode.m_szPrependedPos = nullptr;
    rCode.m_szPrependedCode = nullptr;
    rCode.m_szPrependedCurrent = nullptr;
    rCode.m_uiLine = 0;
    rCode.m_uiCol = 0;
}

bool CCodePos::IsValid() const
{
    return m_szCode != nullptr &&
        m_szCurrent != nullptr &&
        m_uiLine > 0 &&
        m_uiCol > 0;
}

CCodePos::operator bool() const
{
    return IsValid();
}

void CCodePos::Reset()
{
    m_szCurrent = m_szCode;
    m_szPrependedPos = nullptr;
    m_szPrependedCode = nullptr;
    m_szPrependedCurrent = nullptr;
    m_uiLine = 1;
    m_uiCol = 1;
    m_ssPrependedCode.clear();
}

uint32_t CCodePos::GetLine() const
{
    return m_uiLine;
}

uint32_t CCodePos::GetCol() const
{
    return m_uiCol;
}

bool CCodePos::HasEOF() const
{
    return !m_szCurrent || ((!m_szPrependedCurrent || *m_szPrependedCurrent == '\0') && *m_szCurrent == '\0');
}

bool CCodePos::HasEOL() const
{
    if (HasEOF()) return true;
    if (m_szPrependedCurrent && *m_szPrependedCurrent != '\0')
        return *m_szPrependedCurrent == '\n' || (m_szPrependedCurrent[0] == '\r' && m_szPrependedCurrent[1] == '\n');
    return *m_szCurrent == '\n' || (m_szCurrent[0] == '\r' && m_szCurrent[1] == '\n');
}

CToken CCodePos::GetLocation(ETokenType eTokenType /*= ETokenType::token_none*/) const
{
    CToken token;
    token.m_szCode = (m_szPrependedCurrent && *m_szPrependedCurrent) ? m_szPrependedCurrent : m_szCurrent;
    token.m_uiLine = m_uiLine;
    token.m_uiCol = m_uiCol;
    token.m_eType = eTokenType;
    return token;
}

void CCodePos::UpdateLocation(CToken& rtoken) const
{
    // Check for a valid token.
    if (!rtoken) return;

    // If the token start position is within the prepended start code and the prepended current code, update the location by
    // creating a code chunk.
    if (m_szPrependedCode && rtoken.m_szCode >= m_szPrependedCode && rtoken.m_szCode <= m_szPrependedCurrent)
    {
        // Create a chunk based on the prepended code
        std::string ssChunk = std::string(rtoken.m_szCode, m_szPrependedCurrent - rtoken.m_szCode);

        // If the current position of the prepended code is at the end, add a part of the code.
        if (!*m_szPrependedCurrent)
            ssChunk += std::string(m_szPrependedPos, m_szCurrent - m_szPrependedPos);

        // Add the chunk to the list
        m_lstCodeChunks.emplace_back(std::move(ssChunk));

        // Assign the new start and length infos
        rtoken.m_szCode = m_lstCodeChunks.back().c_str();
        rtoken.m_uiLen = static_cast<uint32_t>(m_lstCodeChunks.back().size());

        return;
    }

    // If the token start position is within the code, update the length
    if (rtoken.m_szCode >= m_szCode && rtoken.m_szCode <= m_szCurrent)
    {
        rtoken.m_uiLen = static_cast<uint32_t>(m_szCurrent - rtoken.m_szCode);
        return;
    }

    // The token start position is invalid. Set the code to nullptr and the length to 0.
    rtoken.m_szCode = nullptr;
    rtoken.m_uiLen = 0;
}

void CCodePos::UpdateLocation(CToken& rtoken, ETokenLiteralType eLiteralType) const
{
    UpdateLocation(rtoken);
    switch (rtoken.GetType())
    {
    case ETokenType::token_literal:
    case ETokenType::token_none:
        rtoken.m_eType = ETokenType::token_literal;
        rtoken.m_eLiteralType = eLiteralType;
        break;
    default:
        throw CCompileException("Internal error: invalid token type during literal type assignment.");
        break;
    }
}

void CCodePos::UpdateLocation(CToken& rtoken, ETokenMetaType eMetaType) const
{
    UpdateLocation(rtoken);
	switch (rtoken.GetType())
	{
	case ETokenType::token_meta:
	case ETokenType::token_none:
		rtoken.m_eType = ETokenType::token_meta;
		rtoken.m_eMetaType = eMetaType;
		break;
	default:
		throw CCompileException("Internal error: invalid token type during meta type assignment.");
		break;
	}
}

void CCodePos::PrependCode(const std::string& rssCode)
{
    if (rssCode.empty()) return;
    std::string ssPrependedCode = rssCode;
    if (m_szPrependedCurrent)
        ssPrependedCode += m_szPrependedCurrent;
    m_ssPrependedCode = std::move(ssPrependedCode);
    m_szPrependedCode = m_ssPrependedCode.c_str();
    m_szPrependedCurrent = m_ssPrependedCode.c_str();
    m_szPrependedPos = m_szCurrent;
}

bool CCodePos::CurrentPositionInMacroExpansion() const
{
    return m_szPrependedCurrent && *m_szPrependedCurrent;
}

CCodePos& CCodePos::operator=(const CCodePos& rCode)
{
    m_szCode = rCode.m_szCode;
    m_szCurrent = rCode.m_szCurrent;
    m_szPrependedPos = rCode.m_szPrependedPos;
    m_ssPrependedCode = rCode.m_ssPrependedCode;
    if (rCode.m_szPrependedCurrent && rCode.m_szPrependedCode)
    {
        m_szPrependedCode = m_ssPrependedCode.c_str();
        m_szPrependedCurrent = m_szPrependedCode + (rCode.m_szPrependedCurrent - rCode.m_szPrependedCode);
    } else
    {
        m_szPrependedCode = nullptr;
        m_szPrependedCurrent = nullptr;
    }
    m_uiLine = rCode.m_uiLine;
    m_uiCol = rCode.m_uiCol;
    return *this;
}

CCodePos& CCodePos::operator=(CCodePos&& rCode) noexcept
{
    m_szCode = rCode.m_szCode;
    m_szCurrent = rCode.m_szCurrent;
    m_szPrependedPos = rCode.m_szPrependedPos;
    m_szPrependedCode = rCode.m_szPrependedCode;
    m_szPrependedCurrent = rCode.m_szPrependedCurrent;
    m_ssPrependedCode = std::move(rCode.m_ssPrependedCode);
    m_lstCodeChunks = std::move(rCode.m_lstCodeChunks);
    m_uiLine = rCode.m_uiLine;
    m_uiCol = rCode.m_uiCol;
    rCode.m_szCode = nullptr;
    rCode.m_szCurrent = nullptr;
    rCode.m_szPrependedPos = nullptr;
    rCode.m_szPrependedCode = nullptr;
    rCode.m_szPrependedCurrent = nullptr;
    rCode.m_uiLine = 0;
    rCode.m_uiCol = 0;
    return *this;
}

char CCodePos::operator[](uint32_t uiOffset /*= 0*/) const
{
    // Handle an offset past the prepended string.
    for (uint32_t uiIndex = 0; uiIndex <= uiOffset; uiIndex++)
    {
        if (!m_szPrependedCurrent || m_szPrependedCurrent[uiIndex] == '\0')
            return m_szCurrent ? m_szCurrent[uiOffset - uiIndex] : '\0';
    }

    // Return a character from the prepended string.
    return m_szPrependedCurrent[uiOffset];
}

char CCodePos::operator*() const
{
    if (m_szPrependedCurrent && m_szPrependedCurrent[0]) return m_szPrependedCurrent[0];
    return m_szCurrent ? *m_szCurrent : '\0';
}

CCodePos::operator const char*() const
{
    if (m_szPrependedCurrent) return m_szPrependedCurrent;
    return m_szCurrent;
}

CCodePos CCodePos::operator++(int)
{
    CCodePos posCopy(*this);
    operator++();
    return posCopy;
}

CCodePos& CCodePos::operator++()
{
    // Check for a valid prepended character.
    if (m_szPrependedCurrent && *m_szPrependedCurrent)
    {
        // Increase current position
        m_szPrependedCurrent++;

        // End of prepended code?
        if (!m_szPrependedCurrent)
        {
            m_szPrependedCurrent = nullptr;
            m_ssPrependedCode.clear();
        }

        return *this;
    }

    // Check for a valid character
    if (m_szCurrent && *m_szCurrent != '\0')
    {
        // Currently a newline?
        if (*m_szCurrent == '\n')
        {
            // Set new position at beginning of ext line
            m_uiCol = 1;
            m_uiLine++;
        }
        else if (*m_szCurrent == '\t')
        {
            // Increase the column
            m_uiCol++;

            // Align with a tab of 4 characters
            while ((m_uiCol - 1) % 4) m_uiCol++;
        }
        else
            m_uiCol++;  // Increase the column

                        // Next character
        m_szCurrent++;
    }

    return *this;
}

CCodePos& CCodePos::operator+=(uint32_t uiOffset)
{
    for (uint32_t uiIndex = 0; uiIndex != uiOffset; uiIndex++)
        operator++(0);

    return *this;
}
