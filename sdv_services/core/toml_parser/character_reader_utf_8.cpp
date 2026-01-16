#include "character_reader_utf_8.h"
#include "exception.h"

/// The TOML parser namespace
namespace toml_parser
{
    CCharacterReaderUTF8::CCharacterReaderUTF8(const std::string& rssString) : m_ssString{rssString}
    {
        CheckForInvalidUTF8Bytes();
        CheckForInvalidUTF8Sequences();
    }

    void CCharacterReaderUTF8::Feed(const std::string& rssString)
    {
        m_ssString = rssString;
        m_nCursor  = 0;

        CheckForInvalidUTF8Bytes();
        CheckForInvalidUTF8Sequences();
    }

    void CCharacterReaderUTF8::Reset()
    {
        m_ssString.clear();
        m_nCursor = 0;
    }

    std::string CCharacterReaderUTF8::Peek(size_t nSkip /*= 0*/, size_t nAmount /*= 1*/) const
    {
        if (IsEOF())
            return {};
        size_t nOffset = 0;
        for (size_t n = 0; n < nSkip; ++n)
        {
            nOffset += GetLengthOfNextCharacter(nOffset);
            if (m_ssString.size() <= m_nCursor + nOffset)
                return {};
        }
        std::string ssCharacters;
        for (size_t n = 0; n < nAmount; n++)
        {
            ssCharacters += GetNextCharacter(nOffset);
            nOffset += GetLengthOfNextCharacter(nOffset);
        }
        return ssCharacters;
    }

    std::string CCharacterReaderUTF8::PeekUntil(const std::vector<std::string>& lstCollection) const
    {
        size_t nOffset = 0;
        bool bFound    = false;
        std::string accumulation;
        while (!bFound && m_ssString.size() > m_nCursor + nOffset)
        {
            std::string ssCharacter = GetNextCharacter(nOffset);
            nOffset += GetLengthOfNextCharacter(nOffset);
            for (const auto& delimiter : lstCollection)
            {
                if (delimiter == ssCharacter)
                    bFound = true;
            }
            if (!bFound)
                accumulation += ssCharacter;
        }
        return accumulation;
    }

    std::string CCharacterReaderUTF8::Consume(size_t nSkip /*= 0*/, size_t nAmount /*= 1*/)
    {
        if (IsEOF())
            return {};
        for (size_t n = 0; n < nSkip; ++n)
        {
            size_t nLength = GetLengthOfNextCharacter();
            m_nCursor += nLength;
            if (!nLength || m_ssString.size() < m_nCursor)
                return {};
        }
        std::string ssCharacters;
        for (size_t n = 0; n < nAmount; n++)
        {
            ssCharacters += GetNextCharacter();
            m_nCursor += GetLengthOfNextCharacter();
        }
        return ssCharacters;
    }

    std::string CCharacterReaderUTF8::ConsumeUntil(const std::vector<std::string>& lstCollection)
    {
        size_t nOffset = 0;
        bool bFound    = false;
        std::string accumulation;
        while (!bFound && m_ssString.size() > m_nCursor + nOffset)
        {
            std::string ssCharacter = GetNextCharacter(nOffset);
            nOffset += GetLengthOfNextCharacter(nOffset);
            for (const auto& delimiter : lstCollection)
            {
                if (delimiter == ssCharacter)
                {
                    bFound = true;
                }
            }
            if (!bFound)
            {
                accumulation += ssCharacter;
            }
            else
            {
                nOffset -= ssCharacter.size();
            }
        }
        m_nCursor += nOffset;
        return accumulation;
    }

    bool CCharacterReaderUTF8::IsEOF() const
    {
        return m_ssString.size() < m_nCursor + 1;
    }

    void CCharacterReaderUTF8::SetBookmark()
    {
        m_nBookmark = m_nCursor;
    }

    std::string CCharacterReaderUTF8::StringFromBookmark() const
    {
        return m_ssString.substr(m_nBookmark, (m_nCursor > m_nBookmark) ? m_nCursor - m_nBookmark : 0);
    }

    void CCharacterReaderUTF8::CheckForInvalidUTF8Bytes() const
    {
        const unsigned char invalidByteC0{0xC0};
        const unsigned char invalidByteC1{0xC1};
        const unsigned char lowerBoundInvalidRegion{0xF5};
        for (size_t i = 0; i < m_ssString.size(); ++i)
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
        EState eCurrentState = EState::state_neutral;
        uint8_t uiIndex      = 0;

        auto fnCheckByteInNeutralState = [&uiIndex, &eCurrentState](unsigned char uc)
        {
            if ((uc & m_uiOneByteCheckMask) == m_OneByteCheckValue)
            {
                eCurrentState = EState::state_neutral;
                uiIndex       = 0;
            }
            else if ((uc & m_uiFourByteCheckMask) == m_uiFourByteCheckValue)
            {
                eCurrentState = EState::state_four_byte;
                uiIndex       = 1;
            }
            else if ((uc & m_uiThreeByteCheckMask) == m_uiThreeByteCheckValue)
            {
                eCurrentState = EState::state_three_byte;
                uiIndex       = 1;
            }
            else if ((uc & m_uiTwoByteCheckMask) == m_uiTwoByteCheckValue)
            {
                eCurrentState = EState::state_two_byte;
                uiIndex       = 1;
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
                uiIndex       = 0;
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
                uiIndex       = 0;
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
                uiIndex       = 0;
                eCurrentState = EState::state_neutral;
            }
            else
            {
                eCurrentState = EState::state_error;
            }
        };
        for (size_t i = 0; i < m_ssString.size(); ++i)
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
            }
        }
        if (eCurrentState != EState::state_neutral)
        {
            std::stringstream sstreamMessage;
            sstreamMessage << "Unfinished character at the end of file\n";
            throw XTOMLParseException(sstreamMessage.str());
        }
    }

    std::string CCharacterReaderUTF8::GetNextCharacter(size_t nOffset /*= 0*/) const
    {
        if (IsEOF())
            return {};
        return {m_ssString, m_nCursor + nOffset, GetLengthOfNextCharacter(nOffset)};
    }

    size_t CCharacterReaderUTF8::GetLengthOfNextCharacter(size_t nOffset) const
    {
        if (IsEOF())
            return 0;
        uint8_t ui    = static_cast<uint8_t>(m_ssString[m_nCursor + nOffset]);
        int32_t uiRet = 0;
        if ((ui & m_uiOneByteCheckMask) == m_OneByteCheckValue)
        {
            uiRet = 1;
        }
        else if ((ui & m_uiFourByteCheckMask) == m_uiFourByteCheckValue)
        {
            uiRet = 4;
        }
        else if ((ui & m_uiThreeByteCheckMask) == m_uiThreeByteCheckValue)
        {
            uiRet = 3;
        }
        else if ((ui & m_uiTwoByteCheckMask) == m_uiTwoByteCheckValue)
        {
            uiRet = 2;
        }
        else
        {
            std::stringstream sstreamMessage;
            sstreamMessage << "Invalid character sequence with byte " << std::hex << ui << std::dec << " as start byte\n";
            throw XTOMLParseException(sstreamMessage.str());
        }
        return uiRet;
    }
} // namespace toml_parser