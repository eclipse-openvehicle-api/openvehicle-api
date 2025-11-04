#include "token.h"
#include "support.h"
#include "constvariant.inl"
#include "parsecontext.h"

CToken::CToken(const std::string& rssText, ETokenType eType /*= ETokenType::token_identifier*/) :
    m_eType(eType), m_ssCache(rssText)
{}

CToken::CToken(const std::string& rssText, ETokenLiteralType eLiteralType) :
    m_eType(ETokenType::token_literal), m_eLiteralType(eLiteralType), m_ssCache(rssText)
{}

CToken::CToken(const std::string& rssText, ETokenMetaType eMetaType) :
    m_eType(ETokenType::token_meta), m_eMetaType(eMetaType), m_ssCache(rssText)
{}

void CToken::MoveStart(int32_t iOffset)
{
    // Correct boundaries and set the new position
    int32_t iLocalOffset = iOffset;
    if (iOffset < 0)
    {
        // Not before the beginning
        if (m_uiCol < static_cast<uint32_t>(-iOffset))
            iLocalOffset = -static_cast<int32_t>(m_uiCol);

        // Adjust the position and length
        m_uiCol -= static_cast<uint32_t>(-iLocalOffset);
        if (m_uiLen != 0xffffffff)
            m_uiLen += static_cast<uint32_t>(-iLocalOffset);
    } else
    {
        // Not past the end
        if (static_cast<uint32_t>(iOffset) > m_uiLen)
            iLocalOffset = static_cast<int32_t>(m_uiLen);

        // Check for any new lines occurring and adjust the position
        for (int32_t iIdx = 0; iIdx < iLocalOffset; iIdx++)
        {
            if (m_szCode[iIdx] == '\n')
            {
                m_uiLine++;
                m_uiCol = 1;
            }
            else
                m_uiCol++;
        }

        // Adjust the length
        if (m_uiLen != 0xffffffff)
            m_uiLen -= static_cast<uint32_t>(iLocalOffset);
    }

    // Set the new code token
    m_szCode += iLocalOffset;

    // Clear the cache
    m_ssCache.clear();
}

void CToken::MoveEnd(int32_t iOffset)
{
    // Only works when there is a fixed length
    if (m_uiLen == 0xffffffff) return;

    // Correct boundaries and set new length
    if (iOffset < 0)
    {
        if (static_cast<uint32_t>(-iOffset) > m_uiLen)
            m_uiLen = 0;
        else
            m_uiLen -= static_cast<uint32_t>(-iOffset);
    } else
        m_uiLen += static_cast<uint32_t>(iOffset);

    // Clear the cache
    m_ssCache.clear();
}

bool CToken::operator==(const CToken& rtoken) const
{
    return rtoken.m_szCode == m_szCode;
}

bool CToken::operator!=(const CToken& rtoken) const
{
    return rtoken.m_szCode != m_szCode;
}

uint32_t CToken::GetLine() const
{
    return m_uiLine;
}

uint32_t CToken::GetCol() const
{
    return m_uiCol;
}

uint32_t CToken::GetEndLine() const
{
    uint32_t uiEndLine = m_uiLine;
    for (uint32_t uiIndex = 0; m_uiLen != 0xffffffff && uiIndex < m_uiLen; uiIndex++)
    {
        if (m_szCode[uiIndex] == '\n')
        {
            // Only increase the line count when this is not the last character of the snippet.
            if (uiIndex < m_uiLen - 1) uiEndLine++;
        }
    }
    return uiEndLine;
}

uint32_t CToken::GetEndCol() const
{
    uint32_t uiEndCol = m_uiCol;
    for (uint32_t uiIndex = 0; m_uiLen != 0xffffffff && uiIndex < m_uiLen; uiIndex++)
    {
        if (uiIndex) uiEndCol++;    // Do not count the first column
        if (m_szCode[uiIndex] == '\n')
        {
            // Only increase the line count when this is not the last character of the snippet.
            if (uiIndex < m_uiLen - 1) uiEndCol = 0;
        }
    }
    return uiEndCol;
}

uint32_t CToken::GetLength() const
{
    return m_uiLen;
}

ETokenType CToken::GetType() const
{
    return m_eType;
}

ETokenLiteralType CToken::GetLiteralType() const
{
    return m_eLiteralType;
}

ETokenMetaType CToken::GetMetaType() const
{
    return m_eMetaType;
}

CToken::operator bool() const
{
    // Is there any cache?
    if (!m_ssCache.empty()) return true;

    // Validity checks on the token
    if (!m_szCode) return false;

    // Sanity check by getting the value
    if (m_eType == ETokenType::token_literal && m_eLiteralType != ETokenLiteralType::token_undefined)
        ValueRef();

    // No invalidity detected.
    return true;
}

CToken::operator const std::string&() const
{
    if (m_ssCache.empty())
    {
        if (!m_szCode)
            return m_ssCache;
        if (!m_uiCol)
            return m_ssCache;
        if (m_uiLen != 0xffffffff)
            m_ssCache = std::string(m_szCode, m_uiLen);
        else
            m_ssCache = std::string(m_szCode);
    }
    return m_ssCache;
}

const char* CToken::c_str() const
{
    if (m_ssCache.empty())
    {
        if (!m_szCode)
            return m_ssCache.c_str();
        if (!m_uiCol)
            return m_ssCache.c_str();
        if (m_uiLen != 0xffffffff)
            m_ssCache = std::string(m_szCode, m_uiLen);
        else
            m_ssCache = std::string(m_szCode);
    }
    return m_ssCache.c_str();
}

std::string CToken::Left(uint32_t uiLen) const
{
    if (!m_ssCache.empty()) return m_ssCache.substr(0, uiLen);
    if (!m_szCode) return std::string();
    if (!m_uiCol) return std::string();
    uint32_t uiLocalLen = m_uiLen != 0xffffffff ? m_uiLen : static_cast<uint32_t>(std::strlen(m_szCode));
    return std::string(m_szCode, std::min(uiLen, uiLocalLen));
}

std::string CToken::Right(uint32_t uiLen) const
{
    if (!m_ssCache.empty())
    {
        if (uiLen > m_ssCache.size()) return m_ssCache;
        return m_ssCache.substr(m_ssCache.size() - uiLen);
    }
    if (!m_szCode)
        return std::string();
    if (!m_uiCol) return std::string();
    uint32_t uiLocalLen = m_uiLen != 0xffffffff ? m_uiLen : static_cast<uint32_t>(std::strlen(m_szCode));
    uint32_t uiOffset = uiLocalLen < uiLen ? 0 : uiLocalLen - uiLen;
    return std::string(m_szCode + uiOffset, std::min(uiLen, uiLocalLen));
}

bool CToken::operator==(const char* szString) const
{
    if (!m_ssCache.empty()) return m_ssCache == szString;
    if (!m_szCode && szString) return false;
    if (!szString) return false;
    return static_cast<std::string>(*this) == szString;
}

bool CToken::operator!=(const char* szString) const
{
    if (!m_ssCache.empty()) return m_ssCache != szString;
    if (!m_szCode && szString) return true;
    if (!szString) return true;
    return static_cast<std::string>(*this) != szString;
}

bool CToken::operator==(const std::string& rssString) const
{
    if (!m_ssCache.empty()) return m_ssCache == rssString;
    if (!m_szCode && !rssString.empty()) return false;
    if (rssString.empty()) return false;
    return rssString == static_cast<std::string>(*this);
}

bool CToken::operator!=(const std::string& rssString) const
{
    if (!m_ssCache.empty()) return m_ssCache != rssString;
    if (!m_szCode && !rssString.empty()) return true;
    if (rssString.empty()) return true;
    return rssString != static_cast<std::string>(*this);
}

bool CToken::IsLiteral() const
{
    return m_eType == ETokenType::token_literal &&
        m_eLiteralType != ETokenLiteralType::token_undefined;
}

bool CToken::IsInteger() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_oct_integer:
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_bin_integer:
        return true;
    default:
        return false;
    }
}

bool CToken::IsSigned() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_oct_integer:
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_bin_integer:
        // Check the string for not having the suffix U or u
        return !Contains('U', 'u');
    case ETokenLiteralType::token_literal_dec_floating_point:
    case ETokenLiteralType::token_literal_hex_floating_point:
    case ETokenLiteralType::token_literal_fixed_point:
        // Always signed
        return true;
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_character_sequence:
        // True for 'char' type; false for all others
        return IsAscii();
    default:
        return false;
    }
}

bool CToken::IsUnsigned() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_oct_integer:
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_bin_integer:
        // Check the string for having the suffix U or u
        return Contains('U', 'u');
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_character_sequence:
        // True for all but 'char' type
        return !IsAscii();
    default:
        return false;
    }
}

bool CToken::IsLong() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_oct_integer:
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_bin_integer:
    case ETokenLiteralType::token_literal_dec_floating_point:
    case ETokenLiteralType::token_literal_hex_floating_point:
        // Check the string for having the suffix L or l
        return Contains('L', 'l') && !Contains("LL", "ll");
    case ETokenLiteralType::token_literal_character_sequence:
        // The length of all characters in the sequence should fit the long
        if (IsWide())
        {
            std::wstring ssText;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode, "\'", ssText, uiByteCnt);
            return ssText.size() * sizeof(wchar_t) == sizeof(long);
        }
        else
        {
            std::string ssText;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode, "\'", ssText, uiByteCnt, false, IsAscii());
            return ssText.size() == sizeof(long);
        }
    default:
        return false;
    }
}

bool CToken::IsLongLong() const
{
    uint32_t uiByteCnt = 0;
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_oct_integer:
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_bin_integer:
        // Check the string for having the suffix LL or l
        return Contains("LL", "ll");
    case ETokenLiteralType::token_literal_character_sequence:
        // The length of all characters in the sequence should fit the long
        if (IsWide())
        {
            std::wstring ssText;
            uiByteCnt = 0;
            InterpretCText(m_szCode, "\'", ssText, uiByteCnt);
            return ssText.size() * sizeof(wchar_t) == sizeof(long long);
        }
        else
        {
            std::string ssText;
            uiByteCnt = 0;
            InterpretCText(m_szCode, "\'", ssText, uiByteCnt, false, IsAscii());
            return ssText.size() == sizeof(long long);
        }
    default:
        return false;
    }
}

bool CToken::IsDecimal() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_integer:
    case ETokenLiteralType::token_literal_dec_floating_point:
        return true;
    default:
        return false;
    }
}

bool CToken::IsHexadecimal() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_hex_integer:
    case ETokenLiteralType::token_literal_hex_floating_point:
        return true;
    default:
        return false;
    }
}

bool CToken::IsOctal() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_oct_integer:
        return true;
    default:
        return false;
    }
}

bool CToken::IsBinary() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_bin_integer:
        return true;
    default:
        return false;
    }
}

bool CToken::IsFloatingPoint() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_dec_floating_point:
    case ETokenLiteralType::token_literal_hex_floating_point:
        return true;
    default:
        return false;
    }
}

bool CToken::IsFixedPoint() const
{    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_fixed_point:
        return true;
    default:
        return false;
    }
}

bool CToken::IsCharacter() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
        return true;
    default:
        return false;
    }
}

bool CToken::IsCharacterSequence() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character_sequence:
        return true;
    default:
        return false;
    }
}

bool CToken::IsString() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_string:
    case ETokenLiteralType::token_literal_raw_string:
        return true;
    default:
        return false;
    }
}

bool CToken::IsAscii() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_character_sequence:
        return m_szCode[0] == '\'';
    case ETokenLiteralType::token_literal_string:
        return m_szCode[0] == '\"';
    case ETokenLiteralType::token_literal_raw_string:
        return m_szCode[1] == '\"';     // String starting with R"(....)
    default:
        return false;
    }
}

bool CToken::IsUtf8() const
{
    // NOTE: UTF-8 character is not defined for C++17.
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_string:
    case ETokenLiteralType::token_literal_raw_string:
        return m_szCode[0] == 'u' && m_szCode[1] == '8';     // String starting with u8" or u8R"(....)
    default:
        return false;
    }
}

bool CToken::IsUtf16() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_string:
    case ETokenLiteralType::token_literal_raw_string:
        return m_szCode[0] == 'u' && m_szCode[1] != '8';     // String starting with u" or uR"(....)
    default:
        return false;
    }
}

bool CToken::IsUtf32() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_string:
    case ETokenLiteralType::token_literal_raw_string:
        return m_szCode[0] == 'U';     // String starting with U" or UR"(....)
    default:
        return false;
    }
}

bool CToken::IsWide() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_character:
    case ETokenLiteralType::token_literal_character_sequence:
    case ETokenLiteralType::token_literal_string:
    case ETokenLiteralType::token_literal_raw_string:
        return m_szCode[0] == 'L';     // String starting with L" or LR"(....)
    default:
        return false;
    }
}

bool CToken::IsRawString() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_raw_string:
        return true;
    default:
        return false;
    }
}

bool CToken::IsBoolean() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_boolean:
        return true;
    default:
        return false;
    }
}

bool CToken::IsNullptr() const
{
    switch (m_eLiteralType)
    {
    case ETokenLiteralType::token_literal_nullptr:
        return true;
    default:
        return false;
    }
}

CConstVariant CToken::ValueRef() const
{
    if (IsInteger() || IsCharacter() || IsCharacterSequence())
    {
        if (IsSigned())
            return ValueSignedInteger();
        else
            return ValueUnsignedInteger();
    } else if (IsFloatingPoint() || IsFixedPoint())
        return ValueFloat();
    else if (IsString())
        return ValueString();
    else if (IsBoolean())
        return ValueBoolean();
    else if (IsNullptr())
        return ValueNullptr();
    else
        throw CCompileException(*this, "Internal error: invalid value type.");
}

template <typename... TChar>
bool CToken::Contains(char c, TChar... tChars) const
{
    // Dependable on the existence of a length use the memchr or the strchr function.
    if (m_uiLen == 0xFFFFFFFF)
    {
        if (std::strchr(m_szCode, c))
            return true;
    }
    else
    {
        if (std::memchr(m_szCode, c, static_cast<size_t>(m_uiLen)))
            return true;
    }
    return Contains(tChars...);
}

template <typename... TString>
bool CToken::Contains(const char* sz, TString... tStrings) const
{
    // Implementation of strnstr (not part of the C/C++ library).
    auto fnstrnstr = [](const char *haystack, const char *needle, size_t len) -> char*
    {
            int i;
            size_t needle_len;

            if (0 == (needle_len = strnlen(needle, len)))
                    return (char *)haystack;

            for (i=0; i<=(int)(len-needle_len); i++)
            {
                    if ((haystack[0] == needle[0]) &&
                            (0 == strncmp(haystack, needle, needle_len)))
                            return (char *)haystack;

                    haystack++;
            }
            return NULL;
    };

    // Dependable on the existence of a length use the strnstr or the strstr function.
    if (m_uiLen == 0xFFFFFFFF)
    {
        if (std::strstr(m_szCode, sz))
            return true;
    }
    else
    {
        if (fnstrnstr(m_szCode, sz, static_cast<size_t>(m_uiLen)))
            return true;
    }
    return Contains(tStrings...);
}

CConstVariant CToken::ValueSignedInteger() const
{
    int64_t iValue = 0ll;
    if (IsInteger())
    {
        // Reset errno... will not be cleared otherwise.
        errno = 0;
        if (IsOctal())
            iValue = static_cast<int64_t>(std::strtoll(m_szCode + 1, nullptr, 8));
        else if (IsHexadecimal())
            iValue = static_cast<int64_t>(std::strtoll(m_szCode + 2, nullptr, 16));
        else if (IsBinary())
            iValue = static_cast<int64_t>(std::strtoll(m_szCode + 2, nullptr, 2));
        else
            iValue = static_cast<int64_t>(std::strtoll(m_szCode, nullptr, 10));
        if (errno == ERANGE)
            throw CCompileException(*this, "The provided value exceeds the maximum possible value of a long long integer.");
    }
    if (IsCharacter())
    {
        if (IsAscii())
        {
            std::string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 1, "\'", ssValue, uiByteCnt, false, true);
            if (!ssValue.empty()) iValue = ssValue[0];
        }
        if (IsUtf16())
        {
            std::u16string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty()) iValue = ssValue[0];
        }
        if (IsUtf32())
        {
            std::u32string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty()) iValue = ssValue[0];
        }
        if (IsWide())
        {
            std::wstring ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty()) iValue = ssValue[0];
        }
    }
    if (IsCharacterSequence())
    {
        if (IsAscii())
        {
            std::string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 1, "\'", ssValue, uiByteCnt, false, true);
            for (char c : ssValue)
                iValue = iValue << 8 | static_cast<uint8_t>(c);
        }
        if (IsWide())
        {
            std::wstring ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            for (wchar_t c : ssValue)
            {
                if constexpr (sizeof(wchar_t) == sizeof(int16_t))
                    iValue = iValue << 16 | static_cast<uint16_t>(c);
                else
                    iValue = iValue << 32 | static_cast<uint32_t>(c);
            }
        }
    }

    CConstVariant varValue;
    if (iValue > std::numeric_limits<int8_t>::min() && iValue < std::numeric_limits<int8_t>::max())
        varValue = static_cast<int8_t>(iValue);
    else if (iValue > std::numeric_limits<int16_t>::min() && iValue < std::numeric_limits<int16_t>::max())
        varValue = static_cast<int16_t>(iValue);
    else if (iValue > std::numeric_limits<int32_t>::min() && iValue < std::numeric_limits<int32_t>::max())
        varValue = static_cast<int32_t>(iValue);
    else
        varValue = iValue;
    return varValue;
}

CConstVariant CToken::ValueUnsignedInteger() const
{
    uint64_t uiValue = 0ull;
    if (IsInteger())
    {
        // Reset errno... will not be cleared otherwise.
        errno = 0;
        if (IsOctal())
            uiValue = static_cast<uint64_t>(std::strtoull(m_szCode + 1, nullptr, 8));
        else if (IsHexadecimal())
            uiValue = static_cast<uint64_t>(std::strtoull(m_szCode + 2, nullptr, 16));
        else if (IsBinary())
            uiValue = static_cast<uint64_t>(std::strtoull(m_szCode + 2, nullptr, 2));
        else
            uiValue = static_cast<uint64_t>(std::strtoull(m_szCode, nullptr, 10));
        if (errno == ERANGE)
            throw CCompileException(*this,
                "The provided value exceeds the maximum possible value of an unsigned long long integer.");
    }
    if (IsCharacter())
    {
        if (IsAscii())
        {
            std::string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 1, "\'", ssValue, uiByteCnt, false, true);
            if (!ssValue.empty()) uiValue = static_cast<uint8_t>(ssValue[0]);
        }
        if (IsUtf16())
        {
            std::u16string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty()) uiValue = static_cast<uint16_t>(ssValue[0]);
        }
        if (IsUtf32())
        {
            std::u32string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty()) uiValue = static_cast<uint32_t>(ssValue[0]);
        }
        if (IsWide())
        {
            std::wstring ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            if (!ssValue.empty())
                uiValue = sizeof(wchar_t) == 2 ? static_cast<uint16_t>(ssValue[0]) : static_cast<uint32_t>(ssValue[0]);
        }
    }
    if (IsCharacterSequence())
    {
        uiValue = 0;
        if (IsAscii())
        {
            std::string ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 1, "\'", ssValue, uiByteCnt, false, true);
            for (char c : ssValue)
                uiValue = uiValue << 8 | static_cast<uint8_t>(c);
        }
        if (IsWide())
        {
            std::wstring ssValue;
            uint32_t uiByteCnt = 0;
            InterpretCText(m_szCode + 2, "\'", ssValue, uiByteCnt);
            for (wchar_t c : ssValue)
            {
                if constexpr (sizeof(wchar_t) == sizeof(int16_t))
                    uiValue = uiValue << 16 | static_cast<uint16_t>(c);
                else
                    uiValue = uiValue << 32 | static_cast<uint32_t>(c);
            }
        }
    }

    CConstVariant varValue;
    if (uiValue < static_cast<uint64_t>(std::numeric_limits<uint8_t>::max()))
        varValue = static_cast<uint8_t>(uiValue);
    else if (uiValue < static_cast<uint64_t>(std::numeric_limits<int16_t>::max()))
        varValue = static_cast<int16_t>(uiValue);
    else if (uiValue < static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
        varValue = static_cast<int32_t>(uiValue);
    else
        varValue = uiValue;
    return varValue;
}

CConstVariant CToken::ValueFloat() const
{
    long double ldValue = 0.0;

    // Only integer, character, character sequence and boolean can be assigned.
    if (IsInteger() || IsCharacter() || IsCharacterSequence())
    {
        if (IsSigned())
            ldValue = ValueSignedInteger().Get<long double>();
        else  // unsigned
            ldValue = ValueUnsignedInteger().Get<long double>();
    }
    if (IsFloatingPoint())
    {
        errno = 0;
        ldValue = std::strtold(m_szCode, nullptr);
        if (errno == ERANGE)
            throw CCompileException(*this, "Floating point doesn't fit into the data type.");
    }
    if (IsFixedPoint())
    {
        // A fixed point data type is not supported by C++ at the moment. Use the double data type to calculate and limit the
        // result to 31 bits for the value. The definition is not clear how these 31 bits divide between digits and fraction.
        errno = 0;
        ldValue = std::strtod(m_szCode, nullptr);
        if (errno == ERANGE)
            throw CCompileException(*this, "Floating point doesn't fit into the data type.");
        if (ldValue < -2147483647)  // 31 bits
            throw CCompileException(*this, "The value falls below the smallest possible value of the type.");
        if (ldValue > 2147483648)  // 31 bits
            throw CCompileException(*this, "The value exceeds the highest possible value of the type.");
        int iExpValue = 0;
        static int iExpMin = 0;
        long double ldDigitsValue = std::fabs(std::frexp(ldValue, &iExpValue));
        static long double ldDigitsMin = std::frexp(1.0 / static_cast<double>(1ll << 31), &iExpMin);
        if ((iExpValue < iExpMin) || ((iExpValue == iExpMin) && ldDigitsValue < ldDigitsMin))
            throw CCompileException(*this, "The value precision falls below the smallest possible precision of the type.");
    }
    if (IsString())
        throw CCompileException(*this, "Cannot assign a string to a floating/fixed point value.");
    if (IsBoolean())
        ldValue = ValueBoolean().Get<long double>();
    if (IsNullptr())
        ldValue = ValueNullptr().Get<long double>();

    CConstVariant varValue;
    static int iExpValue = 0, iExpFloatMin = 0, iExpDoubleMin = 0/*, iExpLongDoubleMin = 0*/;
    static float ldDigitsFloatMin = std::frexp(std::numeric_limits<float>::min(), &iExpFloatMin);
    static double ldDigitsDoubleMin = std::frexp(std::numeric_limits<double>::min(), &iExpDoubleMin);
    // static long double ldDigitsLongDoubleMin = std::frexp(std::numeric_limits<long double>::min(), &iExpLongDoubleMin);
    long double ldDigitsValue = std::fabs(std::frexp(ldValue, &iExpValue));
    if (ldValue < std::numeric_limits<float>::max() && ldValue > std::numeric_limits<float>::lowest() &&
        ((iExpValue > iExpFloatMin) || ((iExpValue == iExpFloatMin) && ldDigitsValue > ldDigitsFloatMin)))
        varValue = static_cast<float>(ldValue);
    else if (ldValue < std::numeric_limits<double>::max() && ldValue > std::numeric_limits<double>::lowest() &&
        ((iExpValue > iExpDoubleMin) || ((iExpValue == iExpDoubleMin) && ldDigitsValue > ldDigitsDoubleMin)))
        varValue = static_cast<double>(ldValue);
    else
        varValue = ldValue;

    return varValue;
}

CConstVariant CToken::ValueBoolean() const
{
    if (IsBoolean())
    {
        if (*this == "true") return CConstVariant(true);
        if (*this == "TRUE") return CConstVariant(true);
        if (*this == "false") return CConstVariant(false);
        if (*this == "FALSE") return CConstVariant(false);
        throw CCompileException(*this, "Expecting a boolean value.");
    }
    if (IsInteger() || IsCharacter() || IsCharacterSequence())
    {
        int64_t iValue = ValueSignedInteger().Get<int64_t>();
        if (iValue == 1) return CConstVariant(true);
        if (iValue == 0) return CConstVariant(false);
        throw CCompileException(*this, "Expecting a boolean value or an integer value of 1 or 0.");
    }
    if (IsNullptr())
    {
        if (ValueNullptr().Get<uint64_t>() == 0) return CConstVariant(false);
        throw CCompileException(*this, "Expecting a nullptr value.");
    }
    throw CCompileException(*this, "Expecting a boolean value.");
}

CConstVariant CToken::ValueNullptr() const
{
    if (IsNullptr()) return CConstVariant(0);
    if (IsInteger() || IsCharacter() || IsCharacterSequence())
    {
        int64_t iValue = ValueSignedInteger().Get<int64_t>();
        if (iValue == 0) return CConstVariant(0);
        throw CCompileException(*this, "Expecting a nullptr value or the integer value of 0.");
    }
    throw CCompileException(*this, "Expecting a nullptr value.");
}

CConstVariant CToken::ValueString() const
{

    if (IsString())
    {
        if (IsAscii())
        {
            uint32_t uiByteCnt = 0;
            std::string ssValue;
            InterpretCText(m_szCode + 1, "\"", ssValue, uiByteCnt, false, true);
            return CConstVariant(ssValue);
        }
        if (IsUtf8())
        {
            uint32_t uiByteCnt = 0;
            std::string ssValue;
            InterpretCText(m_szCode + 3, "\"", ssValue, uiByteCnt);
            return CConstVariant(ssValue);
        }
        if (IsUtf16())
        {
            uint32_t uiByteCnt = 0;
            std::u16string ssValue;
            InterpretCText(m_szCode + 2, "\"", ssValue, uiByteCnt);
            return CConstVariant(ssValue);
        }
        if (IsUtf32())
        {
            uint32_t uiByteCnt = 0;
            std::u32string ssValue;
            InterpretCText(m_szCode + 2, "\"", ssValue, uiByteCnt);
            return CConstVariant(ssValue);
        }
        if (IsWide())
        {
            uint32_t uiByteCnt = 0;
            std::wstring ssValue;
            InterpretCText(m_szCode + 2, "\"", ssValue, uiByteCnt);
            return CConstVariant(ssValue);
        }
    }

    throw CCompileException(*this, "Cannot convert type to string.");
}
