#include "tokenlist.h"

CTokenList::CTokenList() : m_itCurrent(end())
{}

CTokenList::CTokenList(const CTokenList& rlstTokens) :
    std::list<CToken>(rlstTokens)
{
    // Determine at what token the iterator points to
    CTokenList::const_iterator itTemp = rlstTokens.begin();
    if (rlstTokens.empty())
    {
        m_itCurrent = end();
        return;
    }
    m_itCurrent = begin();
    while (itTemp != rlstTokens.m_itCurrent)
    {
        ++itTemp;
        ++m_itCurrent;
        if (itTemp == rlstTokens.end() || m_itCurrent == end())
            break;
    }
}

CTokenList::CTokenList(CTokenList&& rlstTokens) noexcept :
    std::list<CToken>(std::move(rlstTokens)), m_itCurrent(rlstTokens.m_itCurrent)
{
    rlstTokens.m_itCurrent = rlstTokens.end();
}

CTokenList& CTokenList::operator=(const CTokenList& rlstTokens)
{
    static_cast<std::list<CToken>&>(*this) = rlstTokens;
    if (rlstTokens.empty()) return *this;

    // Determine at what token the iterator points to
    CTokenList::const_iterator itTemp = rlstTokens.begin();
    m_itCurrent = begin();
    while (itTemp != rlstTokens.m_itCurrent)
    {
        ++itTemp;
        ++m_itCurrent;
        if (itTemp == rlstTokens.end() || m_itCurrent == end())
            break;
    }

    return *this;
}

CTokenList& CTokenList::operator=(CTokenList&& rlstTokens) noexcept
{
    static_cast<std::list<CToken>&>(*this) = std::move(rlstTokens);
    m_itCurrent = rlstTokens.m_itCurrent;
    rlstTokens.m_itCurrent = rlstTokens.end();
    return *this;
}

const CToken& CTokenList::Current(size_t nIndex /*= 0*/) const
{
    static const CToken tokenDummy;
    std::list<CToken>::iterator itPos = m_itCurrent;
    for (size_t nCnt = 0; nCnt < nIndex; nCnt++)
    {
        if (itPos == end()) return tokenDummy;
        ++itPos;
    }
    if (itPos != end())
        return *itPos;
    else
        return tokenDummy;
}

const CToken& CTokenList::LastValid() const
{
    static const CToken tokenDummy;
    if (empty()) return tokenDummy;
    if (m_itCurrent != end())
        return *m_itCurrent;
    else
        return *rbegin();
}

CTokenList CTokenList::operator++(int) const
{
    CTokenList tl(*this);
    operator++();
    return tl;
}

const CTokenList& CTokenList::operator++() const
{
    if (!empty() && m_itCurrent != end()) ++m_itCurrent;
    return *this;
}

bool CTokenList::End() const
{
    return empty() || m_itCurrent == end();
}

void CTokenList::push_front(CToken&& rToken)
{
    bool bEmpty = empty();
    bool bEnd = bEmpty || m_itCurrent == end();
    bool bBegin = !bEmpty && m_itCurrent == begin();
    std::list<CToken>::push_front(rToken);
    if (bEmpty) m_itCurrent = begin();
    else if (bEnd) m_itCurrent = end();
    else if (bBegin) m_itCurrent = begin();
}

void CTokenList::push_front(const CToken& rToken)
{
    bool bEmpty = empty();
    bool bEnd = bEmpty || m_itCurrent == end();
    bool bBegin = !bEmpty && m_itCurrent == begin();
    std::list<CToken>::push_front(rToken);
    if (bEmpty) m_itCurrent = begin();
    else if (bEnd) m_itCurrent = end();
    else if (bBegin) m_itCurrent = begin();
}

void CTokenList::pop_front()
{
    if (empty()) return;
    bool bEnd = m_itCurrent == end();
    bool bBegin = m_itCurrent == begin();
    std::list<CToken>::pop_front();
    if (bEnd) m_itCurrent = end();
    else if (bBegin) m_itCurrent = begin();
}

void CTokenList::push_back(const CToken& rToken)
{
    bool bEmpty = empty();
    bool bEnd = bEmpty || m_itCurrent == end();
    std::list<CToken>::push_back(rToken);
    if (bEmpty) m_itCurrent = begin();
    else if (bEnd)
    {
        m_itCurrent = end();
        --m_itCurrent;
    }
}

void CTokenList::push_back(CToken&& rToken)
{
    bool bEmpty = empty();
    bool bEnd = bEmpty || m_itCurrent == end();
    std::list<CToken>::push_back(std::move(rToken));
    if (bEmpty) m_itCurrent = begin();
    else if (bEnd)
    {
        m_itCurrent = end();
        --m_itCurrent;
    }
}

void CTokenList::pop_back()
{
    if (empty()) return;
    bool bEnd = m_itCurrent == end();
    std::list<CToken>::pop_back();
    if (bEnd) m_itCurrent = end();
}

void CTokenList::insert(const CToken& rToken)
{
    if (empty()) m_itCurrent = end();

    // Insert only if the token is not equal to the last token in the list
    if (m_itCurrent != begin())
    {
        iterator itPrev = m_itCurrent;
        --itPrev;
        if (*itPrev == rToken)
        {
            m_itCurrent = itPrev;
            return;
        }
    }

    // New token; insert the token in the list
    m_itCurrent = std::list<CToken>::insert(m_itCurrent, rToken);
}

void CTokenList::insert(CToken&& rToken)
{
    if (empty()) m_itCurrent = end();

    // Insert only if the token is not equal to the last token in the list
    if (m_itCurrent != begin())
    {
        iterator itPrev = m_itCurrent;
        --itPrev;
        if (*itPrev == rToken)
        {
            m_itCurrent = itPrev;
            return;
        }
    }

    // New token; insert the token in the list
    m_itCurrent = std::list<CToken>::insert(m_itCurrent, std::move(rToken));
}
