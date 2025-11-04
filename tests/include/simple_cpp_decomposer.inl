#ifndef SIMPLE_CPP_DECOMPOSER_INL
#define SIMPLE_CPP_DECOMPOSER_INL

#ifndef SIMPLE_CPP_DECOMPOSER_H
#error Do not include "simple_cpp_composer.inl" directly. Include "simple_cpp_composer.h" instead!
#endif

#include <cassert>

namespace testing::internal
{
    // Additional helpers for testing C++ code
    inline GTEST_API_ AssertionResult CmpHelperCPPEQ(const char* szLeftExpression, const char* szRightExpression,
        const char* szLeft, const char* szRight)
    {
        if (CSimpleCppDecomposer(szLeft) == CSimpleCppDecomposer(szRight))
        {
            return AssertionSuccess();
        }

        return EqFailure(szLeftExpression, szRightExpression, PrintToString(szLeft), PrintToString(szRight), false);
    }
    inline GTEST_API_ AssertionResult CmpHelperCPPNE(const char* szLeftExpression, const char* szRightExpression,
        const char* szLeft, const char* szRight)
    {
        if (CSimpleCppDecomposer(szLeft) != CSimpleCppDecomposer(szRight))
        {
            return AssertionSuccess();
        }

        return EqFailure(szLeftExpression, szRightExpression, PrintToString(szLeft), PrintToString(szRight), false);
    }
    inline GTEST_API_ AssertionResult CmpHelperCPPEQ(const char* szLeftExpression, const char* szRightExpression,
        const std::string& rssLeft, const std::string& rssRight)
    {
        if (CSimpleCppDecomposer(rssLeft) == CSimpleCppDecomposer(rssRight))
        {
            return AssertionSuccess();
        }

        return EqFailure(szLeftExpression, szRightExpression, PrintToString(rssLeft), PrintToString(rssRight), false);
    }
    inline GTEST_API_ AssertionResult CmpHelperCPPNE(const char* szLeftExpression, const char* szRightExpression,
        const std::string& rssLeft, const std::string& rssRight)
    {
        if (CSimpleCppDecomposer(rssLeft) != CSimpleCppDecomposer(rssRight))
        {
            return AssertionSuccess();
        }

        return EqFailure(szLeftExpression, szRightExpression, PrintToString(rssLeft), PrintToString(rssRight), false);
    }
}

inline CSimpleCppDecomposer::CSimpleCppDecomposer(const std::string& rssCppCode) : m_ssCode(rssCppCode)
{
    Process();
}

inline const std::list<std::string>& CSimpleCppDecomposer::GetDecomposedCode() const
{
    return m_lstCode;
}

inline bool CSimpleCppDecomposer::operator==(const CSimpleCppDecomposer& rdecompChunk) const
{
    return m_lstCode == rdecompChunk.GetDecomposedCode();
}

inline bool CSimpleCppDecomposer::operator!=(const CSimpleCppDecomposer& rdecompChunk) const
{
    return m_lstCode != rdecompChunk.GetDecomposedCode();
}

inline void CSimpleCppDecomposer::Process()
{
    while (m_nPos < m_ssCode.size())
    {
        // First skip whitespace...
        SkipWhitespace();
        if (m_nPos >= m_ssCode.size()) break;

        // Check for preproc directives... skip those
        if (m_ssCode[m_nPos] == '#')
        {
            SkipLine();
            continue;
        }

        // Check for comment
        if (m_ssCode[m_nPos] == '/' && (m_ssCode[m_nPos + 1] == '/' || m_ssCode[m_nPos + 1] == '*'))
        {
            SkipComment();
            continue;
        }

        // If the current character represents an underscore a string or a number, it is a keyword, variable, prefix or a number.
        if (std::isalnum(m_ssCode[m_nPos]) || m_ssCode[m_nPos] == '_')
        {
            ProcessText();
            continue;
        }

        // If the current character represents a quote (single or double), it is a string or a character.
        // NOTE: Cannot deal with raw strings
        if (m_ssCode[m_nPos] == '\"' || m_ssCode[m_nPos] == '\'')
        {
            ProcessString();
            continue;
        }

        // If the current character is anything else, it represents a symbol.
        ProcessSymbol();
    }
}

inline void CSimpleCppDecomposer::SkipWhitespace()
{
    while (m_nPos < m_ssCode.size() && std::isspace(m_ssCode[m_nPos]))
    {
        // Skip concatinating lines (not setting the new line flag).
        if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 1] == '\r' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 3;
            continue;
        }
        else if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 2;
            continue;
        }

        // Remember the new line
        if (m_ssCode[m_nPos] == '\n') m_bNewLine = true;

        // Skip the whitespace
        m_nPos++;
    }
}

inline void CSimpleCppDecomposer::SkipLine()
{
    while (m_nPos < m_ssCode.size())
    {
        // Ignore the line ending with concatinating lines
        if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 1] == '\r' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 3;
            continue;
        }
        else if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 2;
            continue;
        }

        // Finished when a new line has been reached
        if (m_ssCode[m_nPos] == '\n')
        {
            m_nPos++;  // Skip the newline
            m_bNewLine = true;
            break;
        }

        // Skip the character
        m_nPos++;
    }
}

inline void CSimpleCppDecomposer::SkipComment()
{
    if (m_nPos >= m_ssCode.size()) return;
    if (m_ssCode[m_nPos] != '/') return;
    if (m_ssCode[m_nPos + 1] == '/') // C++ comment
    {
        SkipLine();
        return;
    }
    if (m_ssCode[m_nPos + 1] != '*') return;

    // C comment
    m_nPos += 2;
    while (m_nPos < m_ssCode.size())
    {
        // End of comment
        if (m_ssCode[m_nPos] == '*' && m_ssCode[m_nPos + 1] == '/')
        {
            m_nPos += 2;
            break;
        }

        // Ignore the line ending with concatinating lines
        if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 1] == '\r' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 3;
            continue;
        }
        else if (m_ssCode[m_nPos] == '\\' && m_ssCode[m_nPos + 2] == '\n')
        {
            m_nPos += 2;
            continue;
        }

        // Detect a new line
        if (m_ssCode[m_nPos] == '\n') m_bNewLine = true;

        // Skip the character
        m_nPos++;
    }
}

inline void CSimpleCppDecomposer::ProcessText()
{
    m_bNewLine = false;
    size_t nStart = m_nPos;
    while (m_nPos < m_ssCode.size() && (std::isalnum(m_ssCode[m_nPos]) || m_ssCode[m_nPos] == '_'))
        m_nPos++;
    if (m_nPos == nStart) return;
    m_lstCode.push_back(m_ssCode.substr(nStart, m_nPos - nStart));
}

inline void CSimpleCppDecomposer::ProcessString()
{
    m_bNewLine = false;
    size_t nStart = m_nPos;
    m_nPos++; // Skip quote
    while (m_nPos < m_ssCode.size() && m_ssCode[m_nPos] != '\"' && m_ssCode[m_nPos] == '\'') m_nPos++;
    if (m_nPos < m_ssCode.size()) m_nPos++; // Skip quote
    if (m_nPos == nStart) return;
    m_lstCode.push_back(m_ssCode.substr(nStart, m_nPos - nStart));
}

inline void CSimpleCppDecomposer::ProcessSymbol()
{
    m_bNewLine = false;
    if (m_nPos >= m_ssCode.size()) return;
    m_lstCode.push_back(std::string(1, m_ssCode[m_nPos]));
    m_nPos++; // Skip symbol
}

#endif // !defined SIMPLE_CPP_DECOMPOSER_INL