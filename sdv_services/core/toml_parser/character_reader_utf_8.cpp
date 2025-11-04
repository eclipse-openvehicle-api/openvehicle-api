#include "character_reader_utf_8.h"
#include "exception.h"

CCharacterReaderUTF8::CCharacterReaderUTF8() : m_nDataLength{0}, m_nCursor{0}
{}

CCharacterReaderUTF8::CCharacterReaderUTF8(const std::string& rssString) :
    m_ssString{rssString}, m_nDataLength{rssString.size()}, m_nCursor{0}
{
    CheckForInvalidUTF8Bytes();
    CheckForInvalidUTF8Sequences();
}

void CCharacterReaderUTF8::Feed(const std::string& rssString)
{
    m_ssString = rssString;
    m_nDataLength = rssString.size();
    m_nCursor = 0;

    CheckForInvalidUTF8Bytes();
    CheckForInvalidUTF8Sequences();
}

void CCharacterReaderUTF8::Reset()
{
    m_ssString.clear();
    m_nDataLength = 0;
    m_nCursor = 0;
}

std::string CCharacterReaderUTF8::Peek()
{
    return GetNextCharacter();
}

std::string CCharacterReaderUTF8::Peek(std::size_t n)
{
    if (n < 1)
    {
        return "";
    }
    size_t offset{0};
    for (std::size_t i = 0; i < n - 1; ++i)
    {
        offset += GetLengthOfNextCharacter(offset);
        if (m_nDataLength <= m_nCursor + offset)
        {
            return "";
        }
    }
    return GetNextCharacter(offset);
}

std::string CCharacterReaderUTF8::PeekUntil(const std::vector<std::string>& lstCollection)
{
    size_t offset{0};
    bool found{false};
    std::string accumulation;
    while (!found && m_nDataLength > m_nCursor + offset)
    {
        std::string character = GetNextCharacter(offset);
        offset += GetLengthOfNextCharacter(offset);
        for (const auto& delimiter : lstCollection)
        {
            if (delimiter == character)
            {
                found = true;
            }
        }
        if (!found)
        {
            accumulation += character;
        }
    }
    return accumulation;
}

std::string CCharacterReaderUTF8::Consume()
{
    if (IsEOF())
    {
        return "";
    }
    std::string character{GetNextCharacter()};
    m_nCursor += GetLengthOfNextCharacter();
    return character;
}

std::string CCharacterReaderUTF8::Consume(std::size_t n)
{
    if (n < 1)
    {
        return "";
    }
    size_t offset{0};
    for (uint32_t i = 0; i < n - 1; ++i)
    {
        offset += GetLengthOfNextCharacter(offset);
        if (m_nDataLength < m_nCursor + offset)
        {
            return "";
        }
    }
    std::string character{GetNextCharacter(offset)};
    m_nCursor += offset + GetLengthOfNextCharacter(offset);
    return character;
}

std::string CCharacterReaderUTF8::ConsumeUntil(const std::vector<std::string>& lstCollection)
{
    std::size_t offset{0};
    bool		found{false};
    std::string accumulation;
    while (!found && m_nDataLength > m_nCursor + offset)
    {
        std::string character = GetNextCharacter(offset);
        offset += GetLengthOfNextCharacter(offset);
        for (const auto& delimiter : lstCollection)
        {
            if (delimiter == character)
            {
                found = true;
            }
        }
        if (!found)
        {
            accumulation += character;
        }
        else
        {
            offset -= character.size();
        }
    }
    m_nCursor += offset;
    return accumulation;
}

bool CCharacterReaderUTF8::IsEOF() const
{
    return m_nDataLength < m_nCursor + 1;
}

void CCharacterReaderUTF8::CheckForInvalidUTF8Bytes() const
{
    const unsigned char invalidByteC0{0xC0};
    const unsigned char invalidByteC1{0xC1};
    const unsigned char lowerBoundInvalidRegion{0xF5};
    for (std::size_t i = 0; i < m_ssString.size(); ++i)
    {
        unsigned char uc = m_ssString[i];
        if (uc == invalidByteC0 || uc == invalidByteC1 || uc >= lowerBoundInvalidRegion)
        {
            std::stringstream message;
            message << "Invalid byte " << std::hex << uc << std::dec << " at position " << i << "\n";
            throw XTOMLParseException(message.str());
        }
    }
}

void CCharacterReaderUTF8::CheckForInvalidUTF8Sequences() const
{
    enum class EState
    {
        state_neutral,
        state_two_byte,
        state_three_byte,
        state_four_byte,
        state_error
    };
    EState eCurrentState{EState::state_neutral};
    uint8_t		  uiIndex{0};
    auto		  fnCheckByteInNeutralState = [&uiIndex, &eCurrentState](unsigned char uc)
    {
        if ((uc & m_uiOneByteCheckMask) == m_OneByteCheckValue)
        {
            eCurrentState = EState::state_neutral;
            uiIndex		 = 0;
        }
        else if ((uc & m_uiFourByteCheckMask) == m_uiFourByteCheckValue)
        {
            eCurrentState = EState::state_four_byte;
            uiIndex		 = 1;
        }
        else if ((uc & m_uiThreeByteCheckMask) == m_uiThreeByteCheckValue)
        {
            eCurrentState = EState::state_three_byte;
            uiIndex		 = 1;
        }
        else if ((uc & m_uiTwoByteCheckMask) == m_uiTwoByteCheckValue)
        {
            eCurrentState = EState::state_two_byte;
            uiIndex		 = 1;
        }
        else
        {
            eCurrentState = EState::state_error;
        }
    };
    auto fnCheckByteInTwoByteState = [&uiIndex, &eCurrentState](unsigned char uc)
    {
        if ((uc & m_uiFollowByteCheckMask) == m_uiFollowByteValue)
        {
            uiIndex		 = 0;
            eCurrentState = EState::state_neutral;
        }
        else
        {
            eCurrentState = EState::state_error;
        }
    };
    auto fnCheckByteInThreeByteState = [&uiIndex, &eCurrentState](unsigned char uc)
    {
        if (uiIndex == 1 && (uc & m_uiFollowByteCheckMask) == m_uiFollowByteValue)
        {
            uiIndex = 2;
        }
        else if (uiIndex == 2 && (uc & m_uiFollowByteCheckMask) == m_uiFollowByteValue)
        {
            uiIndex		 = 0;
            eCurrentState = EState::state_neutral;
        }
        else
        {
            eCurrentState = EState::state_error;
        }
    };
    auto fnCheckByteInFourByteState = [&uiIndex, &eCurrentState](unsigned char uc)
    {
        if (uiIndex <= 2 && (uc & m_uiFollowByteCheckMask) == m_uiFollowByteValue)
        {
            ++uiIndex;
        }
        else if (uiIndex == 3 && (uc & m_uiFollowByteCheckMask) == m_uiFollowByteValue)
        {
            uiIndex		 = 0;
            eCurrentState = EState::state_neutral;
        }
        else
        {
            eCurrentState = EState::state_error;
        }
    };
    for (std::size_t i = 0; i < m_ssString.size(); ++i)
    {
        uint8_t uiCurrentByte = m_ssString[i];
        switch (eCurrentState)
        {
        case EState::state_neutral:
            fnCheckByteInNeutralState(uiCurrentByte);
            break;
        case EState::state_two_byte:
            fnCheckByteInTwoByteState(uiCurrentByte);
            break;
        case EState::state_three_byte:
            fnCheckByteInThreeByteState(uiCurrentByte);
            break;
        case EState::state_four_byte:
            fnCheckByteInFourByteState(uiCurrentByte);
            break;
        default:
            std::stringstream sstreamMessage;
            sstreamMessage << "Invalid character with byte " << std::hex << m_ssString[i - 1] << std::dec << "("
                << static_cast<int32_t>(m_ssString[i - 1]) << ") at index " << i - 1 << "\n";
            throw XTOMLParseException(sstreamMessage.str());
            break;
        }
    }
    if (eCurrentState != EState::state_neutral)
    {
        std::stringstream sstreamMessage;
        sstreamMessage << "Unfinished character at the end of file\n";
        throw XTOMLParseException(sstreamMessage.str());
    }
}

std::string CCharacterReaderUTF8::GetNextCharacter()
{
    if (IsEOF())
    {
        return "";
    }
    return {m_ssString, m_nCursor, GetLengthOfNextCharacter()};
}

std::string CCharacterReaderUTF8::GetNextCharacter(std::size_t offset)
{
    return {m_ssString, m_nCursor + offset, GetLengthOfNextCharacter(offset)};
}

size_t CCharacterReaderUTF8::GetLengthOfNextCharacter() const
{
    return GetLengthOfNextCharacter(0);
}

size_t CCharacterReaderUTF8::GetLengthOfNextCharacter(std::size_t offset) const
{
    uint8_t ui = m_ssString[m_nCursor + offset];
    int32_t ret;
    if ((ui & m_uiOneByteCheckMask) == m_OneByteCheckValue)
    {
        ret = 1;
    }
    else if ((ui & m_uiFourByteCheckMask) == m_uiFourByteCheckValue)
    {
        ret = 4;
    }
    else if ((ui & m_uiThreeByteCheckMask) == m_uiThreeByteCheckValue)
    {
        ret = 3;
    }
    else if ((ui & m_uiTwoByteCheckMask) == m_uiTwoByteCheckValue)
    {
        ret = 2;
    }
    else
    {
        std::stringstream sstreamMessage;
        sstreamMessage << "Invalid character sequence with byte " << std::hex << ui << std::dec
            << " as start byte\n";
        throw XTOMLParseException(sstreamMessage.str());
    }
    return ret;
}
