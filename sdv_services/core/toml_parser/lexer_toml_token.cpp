#include "lexer_toml_token.h"
#include <sstream>

/// The TOML parser namespace
namespace toml_parser
{
    CToken::CToken()
    {}

    CToken::CToken(const TTokenList& rTokenList, EBoundary eBoundary)
    {
        switch (eBoundary)
        {
        case EBoundary::lower_boundary:
            m_optTokenList = rTokenList;
            // NOTE: No iterator assignment.
            break;
        case EBoundary::upper_boundary:
            m_optTokenList = rTokenList;
            m_optLocation  = rTokenList.end();
            break;
        case EBoundary::no_boundary:
        default:
            // NOTE: No tokenlist assignment.
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory) : m_eCategory(eCategory)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
        case ETokenCategory::token_integer:
        case ETokenCategory::token_float:
        case ETokenCategory::token_boolean:
        case ETokenCategory::token_time_local:
        case ETokenCategory::token_date_time_offset:
        case ETokenCategory::token_date_time_local:
        case ETokenCategory::token_date_local:
            {
                std::stringstream sstreamMessage;
                sstreamMessage << "Invalid token category for token without ssContent: " << static_cast<uint32_t>(eCategory);
                m_eCategory = ETokenCategory::token_error;
                new (&m_ssContentString) std::string(sstreamMessage.str());
            }
        default:
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory, const std::string& rssContent,
        ETokenStringType eStringType /*= ETokenStringType::not_specified*/) :
        m_eCategory(eCategory), m_eStringType(eStringType)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            new (&m_ssContentString) std::string(rssContent);
            break;
        default:
            {
                std::stringstream sstreamMessage;
                sstreamMessage << "Invalid token category for string token: " << static_cast<uint32_t>(eCategory);
                m_eCategory = ETokenCategory::token_error;
                new (&m_ssContentString) std::string(sstreamMessage.str());
            }
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory, int64_t iContent) : m_eCategory(eCategory)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_integer:
            m_iContentInteger = iContent;
            break;
        default:
            {
                std::stringstream sstreamMessage;
                sstreamMessage << "Invalid token category for integer token: " << static_cast<uint32_t>(eCategory);
                m_eCategory = ETokenCategory::token_error;
                new (&m_ssContentString) std::string(sstreamMessage.str());
            }
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory, double dContent) : m_eCategory(eCategory)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_float:
            m_dContentFloatingpoint = dContent;
            break;
        default:
            {
                std::stringstream sstreamMessage;
                sstreamMessage << "Invalid token categoryfor floating point token: " << static_cast<uint32_t>(eCategory);
                m_eCategory = ETokenCategory::token_error;
                new (&m_ssContentString) std::string(sstreamMessage.str());
            }
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory, bool bContent) : m_eCategory(eCategory)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_boolean:
            m_bContentBoolean = bContent;
            break;
        default:
            {
                std::stringstream sstreamMessage;
                sstreamMessage << "Invalid token category for boolean token: " << static_cast<uint32_t>(eCategory);
                m_eCategory = ETokenCategory::token_error;
                new (&m_ssContentString) std::string(sstreamMessage.str());
            }
            break;
        }
    }

    CToken::CToken(ETokenCategory eCategory, const sdv::toml::XTOMLParseException& rexcept) :
        CToken(eCategory, std::string(rexcept.what()))
    {}

    CToken::CToken(const CToken& rToken) :
        m_ssRawString(rToken.m_ssRawString), m_optTokenList(rToken.m_optTokenList), m_optLocation(rToken.m_optLocation),
        m_eCategory(rToken.m_eCategory), m_eStringType(rToken.m_eStringType)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            new (&m_ssContentString) std::string(rToken.m_ssContentString);
            break;
        case ETokenCategory::token_integer:
            m_iContentInteger = rToken.m_iContentInteger;
            break;
        case ETokenCategory::token_float:
            m_dContentFloatingpoint = rToken.m_dContentFloatingpoint;
            break;
        case ETokenCategory::token_boolean:
            m_bContentBoolean = rToken.m_bContentBoolean;
            break;
        default:
            break;
        }
    }

    CToken::CToken(CToken&& rToken) :
        m_ssRawString(std::move(rToken.m_ssRawString)), m_optTokenList(std::move(rToken.m_optTokenList)),
        m_optLocation(rToken.m_optLocation), m_eCategory(rToken.m_eCategory), m_eStringType(rToken.m_eStringType)
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            new (&m_ssContentString) std::string(std::move(rToken.m_ssContentString));
            break;
        case ETokenCategory::token_integer:
            m_iContentInteger = rToken.m_iContentInteger;
            break;
        case ETokenCategory::token_float:
            m_dContentFloatingpoint = rToken.m_dContentFloatingpoint;
            break;
        case ETokenCategory::token_boolean:
            m_bContentBoolean = rToken.m_bContentBoolean;
            break;
        default:
            break;
        }

        // Clear the ssContent
        rToken.~CToken();
        rToken.m_eCategory = ETokenCategory::token_none;
        rToken.m_eStringType = ETokenStringType::not_specified;
    }

    CToken::~CToken()
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            m_ssContentString.~basic_string<char>();
            break;
        default:
            break;
        }
    }

    CToken& CToken::operator=(const CToken& rToken)
    {
        // Clear current ssContent.
        this->~CToken();

        // Copy new ssContent
        m_ssRawString = rToken.m_ssRawString;
        m_optTokenList = rToken.m_optTokenList;
        m_optLocation = rToken.m_optLocation;
        m_eCategory = rToken.m_eCategory;
        m_eStringType = rToken.m_eStringType;
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            new (&m_ssContentString) std::string(rToken.m_ssContentString);
            break;
        case ETokenCategory::token_integer:
            m_iContentInteger = rToken.m_iContentInteger;
            break;
        case ETokenCategory::token_float:
            m_dContentFloatingpoint = rToken.m_dContentFloatingpoint;
            break;
        case ETokenCategory::token_boolean:
            m_bContentBoolean = rToken.m_bContentBoolean;
            break;
        default:
            break;
        }

        return *this;
    }

    CToken& CToken::operator=(CToken&& rToken)
    {
        // Clear current ssContent.
        this->~CToken();

        // Copy new ssContent
        switch (rToken.m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            new (&m_ssContentString) std::string(std::move(rToken.m_ssContentString));
            break;
        case ETokenCategory::token_integer:
            m_iContentInteger = rToken.m_iContentInteger;
            break;
        case ETokenCategory::token_float:
            m_dContentFloatingpoint = rToken.m_dContentFloatingpoint;
            break;
        case ETokenCategory::token_boolean:
            m_bContentBoolean = rToken.m_bContentBoolean;
            break;
        default:
            break;
        }
        m_ssRawString  = std::move(rToken.m_ssRawString);
        m_optTokenList = std::move(rToken.m_optTokenList);
        m_optLocation = std::move(rToken.m_optLocation);
        m_eCategory = rToken.m_eCategory;
        m_eStringType = rToken.m_eStringType;

        // Clear origional ssContent
        rToken.~CToken();
        rToken.m_eCategory = ETokenCategory::token_none;
        rToken.m_eStringType = ETokenStringType::not_specified;

        return *this;
    }

    bool CToken::operator==(const CToken& rToken) const
    {
        if (m_optLocation && rToken.m_optLocation)
            return *m_optLocation == *rToken.m_optLocation;

        if (m_eCategory != rToken.m_eCategory)
            return false;
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            return m_ssContentString == rToken.m_ssContentString;
        case ETokenCategory::token_integer:
            return m_iContentInteger == rToken.m_iContentInteger;
        case ETokenCategory::token_float:
            return m_dContentFloatingpoint == rToken.m_dContentFloatingpoint;
        case ETokenCategory::token_boolean:
            return m_bContentBoolean == rToken.m_bContentBoolean;
        default:
            return true; // No value to compare
        }
    }

    bool CToken::operator!=(const CToken& rToken) const
    {
        if (m_optLocation && rToken.m_optLocation)
            return *m_optLocation != *rToken.m_optLocation;

        if (m_eCategory != rToken.m_eCategory)
            return true;
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            return m_ssContentString != rToken.m_ssContentString;
        case ETokenCategory::token_integer:
            return m_iContentInteger != rToken.m_iContentInteger;
        case ETokenCategory::token_float:
            return m_dContentFloatingpoint != rToken.m_dContentFloatingpoint;
        case ETokenCategory::token_boolean:
            return m_bContentBoolean != rToken.m_bContentBoolean;
        default:
            return false; // No value to compare
        }
    }

    CToken::operator bool() const
    {
        return m_eCategory != ETokenCategory::token_none;
    }

    const CToken& CToken::Next(size_t nSkip /*= 0*/) const
    {
        static CToken none;
        if (!m_optLocation || !m_optTokenList) return none;
        const TTokenListIterator& ritToken = *m_optLocation;
        TTokenList::const_iterator ritToken2 = ritToken;
        const TTokenList& rTokenList = *m_optTokenList;
        if (ritToken2 == rTokenList.end()) return m_optTokenList->get().tokenEnd;
        size_t nLocalSkip = nSkip + 1;
        do
        {
            ++ritToken2;
            if (ritToken2 == rTokenList.end()) return m_optTokenList->get().tokenEnd;
        } while (--nLocalSkip);
        return *ritToken2;
    }

    const CToken& CToken::Prev(size_t nSkip /*= 0*/) const
    {
        static CToken none;
        if (!m_optLocation || !m_optTokenList) return none;
        const TTokenListIterator& ritToken = *m_optLocation;
        TTokenList::const_iterator ritToken2 = ritToken;
        const TTokenList& rTokenList = *m_optTokenList;
        size_t nLocalSkip = nSkip + 1;
        do
        {
            if (ritToken2 == rTokenList.begin()) return m_optTokenList->get().tokenReverseEnd;
            --ritToken2;
        } while (--nLocalSkip);
        return *ritToken2;
    }

    const CToken& CToken::JumpToBegin() const
    {
        static CToken none;
        if (!m_optLocation || !m_optTokenList) return none;
        return *(m_optTokenList->get().begin());
    }

    const CToken& CToken::JumpToEnd() const
    {
        static CToken none;
        if (!m_optLocation || !m_optTokenList) return none;
        return m_optTokenList->get().tokenEnd;
    }

    ETokenCategory CToken::Category() const
    {
        return m_eCategory;
    }

    ETokenStringType CToken::StringType() const
    {
        return m_eStringType;
    }

    std::string CToken::StringValue() const
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_key:
        case ETokenCategory::token_string:
        case ETokenCategory::token_error:
        case ETokenCategory::token_terminated:
            return m_ssContentString;
        default:
            return {};
        }
    }

    int64_t CToken::IntegerValue() const
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_integer:
            return m_iContentInteger;
        default:
            return 0;
        }
    }

    double CToken::FloatValue() const
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_float:
            return m_dContentFloatingpoint;
        default:
            return 0.0;
        }
    }

    bool CToken::BooleanValue() const
    {
        switch (m_eCategory)
        {
        case ETokenCategory::token_boolean:
            return m_bContentBoolean;
        default:
            return false;
        }
    }

    const std::string& CToken::RawString() const
    {
        return m_ssRawString;
    }

    uint32_t CToken::TokenIndex() const
    {
        if (m_eCategory == ETokenCategory::token_none)
            return 0xffffffff;
        return m_uiIndex;
    }

    void CToken::RawDataInfo(const std::string& rssString, const TTokenList& rTokenList, const TTokenListIterator& ritLocation)
    {
        m_ssRawString = rssString;
        m_optTokenList = rTokenList;
        m_optLocation = std::cref(ritLocation);
    }

    const std::optional<TTokenListIterator>& CToken::Location() const
    {
        return m_optLocation;
    }

    const std::optional<std::reference_wrapper<const TTokenList>>& CToken::TokenList() const
    {
        return m_optTokenList;
    }

    uint32_t CToken::CreateIndex()
    {
        static uint32_t uiIndexCounter = 0;
        return uiIndexCounter++;
    }

    CTokenList::CTokenList() :
        tokenReverseEnd(*this, CToken::EBoundary::lower_boundary), tokenEnd(*this, CToken::EBoundary::upper_boundary)
    {}

    CTokenRange::CTokenRange(const CToken& rTokenBegin) : m_rTokenBegin(rTokenBegin), m_rTokenEnd(rTokenBegin.JumpToEnd())
    {
        if (!rTokenBegin.TokenList())
            throw XTOMLParseException("The begin token provided to the token range doesn't have a token list assigned.");
    }

    CTokenRange::CTokenRange(const CToken& rTokenBegin, const CToken& rTokenEnd) :
        m_rTokenBegin(rTokenBegin), m_rTokenEnd(rTokenEnd)
    {
        if (!rTokenBegin.TokenList())
            throw XTOMLParseException("The begin token provided to the token range doesn't have a token list assigned.");
        if (!rTokenEnd.TokenList())
            throw XTOMLParseException("The end token provided to the token range doesn't have a token list assigned.");
        if (&(*rTokenBegin.TokenList()).get() != &(*rTokenEnd.TokenList()).get())
            throw XTOMLParseException("The end token of the range is not from the same token list as the begin token.");
    }

    void CTokenRange::ReassignBeginToken(const CToken& rTokenBegin)
    {
        if (!rTokenBegin.TokenList())
            throw XTOMLParseException("The begin token provided to the token range doesn't have a token list assigned.");
        m_rTokenBegin = rTokenBegin;
        m_rTokenEnd = rTokenBegin.JumpToEnd();
    }

    void CTokenRange::ReassignTokenRange(const CToken& rTokenBegin, const CToken& rTokenEnd)
    {
        if (!rTokenBegin.TokenList())
            throw XTOMLParseException("The begin token provided to the token range doesn't have a token list assigned.");
        if (!rTokenEnd.TokenList())
            throw XTOMLParseException("The end token provided to the token range doesn't have a token list assigned.");
        if (&(*rTokenBegin.TokenList()).get() != &(*rTokenEnd.TokenList()).get())
            throw XTOMLParseException("The end token of the range is not from the same token list as the begin token.");
        m_rTokenBegin = rTokenBegin;
        m_rTokenEnd = rTokenEnd;
    }

    void CTokenRange::AssignEndToken(const CToken& rTokenEnd, bool bIncludeEnd /*= true*/)
    {
        // Check whether the end token corresponds to the same list of the begin token.
        if (!rTokenEnd.TokenList())
            throw XTOMLParseException("The end token provided to the token range doesn't have a token list assigned.");
        if (&(*m_rTokenBegin.get().TokenList()).get() != &(*rTokenEnd.TokenList()).get())
            throw XTOMLParseException("The end token of the range is not from the same token list as the begin token.");
        const CToken& rTokenEndCorrected = bIncludeEnd ? rTokenEnd.Next() : rTokenEnd;
        
        m_rTokenEnd = rTokenEndCorrected;
    }

    const CToken& CTokenRange::Begin() const
    {
        return m_rTokenBegin.get();
    }

    const CToken& CTokenRange::End() const
    {
        return m_rTokenEnd.get();
    }

    std::list<CToken> CTokenRange::TokenListSLice() const
    {
        if (!m_rTokenBegin.get().Location() || !m_rTokenEnd.get().Location())
            return {};
        return std::list<CToken>(*m_rTokenBegin.get().Location(), *m_rTokenEnd.get().Location());
    }

    CTokenRange::operator bool() const
    {
        return m_rTokenEnd.get() != m_rTokenBegin.get() && m_rTokenBegin.get().TokenIndex() < m_rTokenEnd.get().TokenIndex();
    }

} // namespace toml_parser