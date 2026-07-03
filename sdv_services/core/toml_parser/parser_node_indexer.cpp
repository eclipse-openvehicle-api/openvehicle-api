/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include "parser_node_indexer.h"
#include <interfaces/toml.h>
#include <iostream>

namespace toml_parser
{
    CNodeIndex::CNodeIndex(CIndexList& rIndexList, CIndexListIterator itPos) : m_ptrPos(std::make_shared<CIteratorWrapper>(rIndexList, itPos))
    {}

    CNodeIndex::~CNodeIndex()
    {
        m_ptrPos.reset();
    }

    CNodeIndex::CNodeIndex(const CNodeIndex& rIndex) : m_ptrPos(rIndex.m_ptrPos)
    {}

    CNodeIndex::CNodeIndex(CNodeIndex&& rIndex) : m_ptrPos(std::move(rIndex.m_ptrPos))
    {}

    CNodeIndex& CNodeIndex::operator=(const CNodeIndex& rIndex)
    {
        m_ptrPos = rIndex.m_ptrPos;
        return *this;
    }

    CNodeIndex& CNodeIndex::operator=(CNodeIndex&& rIndex)
    {
        m_ptrPos = std::move(rIndex.m_ptrPos);
        return *this;
    }

    bool CNodeIndex::operator==(const CNodeIndex& rIndex) const
    {
        return rIndex.m_ptrPos == m_ptrPos;
    }

    bool CNodeIndex::operator!=(const CNodeIndex& rIndex) const
    {
        return !operator==(rIndex);
    }

    bool CNodeIndex::operator<(const CNodeIndex& rIndex) const
    {
        if (!m_ptrPos) return false;
        if (!rIndex.m_ptrPos) return true;
        return m_ptrPos->Index() < rIndex.m_ptrPos->Index();
    }

    bool CNodeIndex::operator<=(const CNodeIndex& rIndex) const
    {
        if (!m_ptrPos) return false;
        if (!rIndex.m_ptrPos) return true;
        return rIndex.m_ptrPos == m_ptrPos || m_ptrPos->Index() < rIndex.m_ptrPos->Index();
    }

    bool CNodeIndex::operator>(const CNodeIndex& rIndex) const
    {
        if (!m_ptrPos) return true;
        if (!rIndex.m_ptrPos) return false;
        return m_ptrPos->Index() > rIndex.m_ptrPos->Index();
    }

    bool CNodeIndex::operator>=(const CNodeIndex& rIndex) const
    {
        if (!m_ptrPos) return true;
        if (!rIndex.m_ptrPos) return false;
        return rIndex.m_ptrPos == m_ptrPos || m_ptrPos->Index() > rIndex.m_ptrPos->Index();
    }

    CNodeIndex::operator bool() const
    {
        return m_ptrPos ? true : false;
    }

    void CNodeIndex::MoveBefore(const CNodeIndex& rIndex)
    {
        if (m_ptrPos && rIndex.m_ptrPos)
            m_ptrPos->MoveBeforeIndex(*rIndex.m_ptrPos);
    }

    uint32_t CNodeIndex::Index() const
    {
        if (!m_ptrPos) return sdv::toml::npos;
        return m_ptrPos->Index();
    }

    CNodeIndex::CIteratorWrapper::CIteratorWrapper(CIndexList& rIndexList, CIndexListIterator itPos) :
        m_rIndexList(rIndexList), m_itPos(itPos)
    {}

    CNodeIndex::CIteratorWrapper ::~CIteratorWrapper()
    {
        m_rIndexList.erase(m_itPos);
    }

    uint32_t CNodeIndex::CIteratorWrapper::Index() const
    {
        return static_cast<uint32_t>(std::distance(m_rIndexList.cbegin(), m_itPos));
    }

    void CNodeIndex::CIteratorWrapper::MoveBeforeIndex(const CIteratorWrapper& ritTarget)
    {
        m_rIndexList.splice(ritTarget.m_itPos, m_rIndexList, m_itPos);
    }

    // Global index list
    CIndexList CNodeIndexer::m_lstIndexList;

    CNodeIndexer::CNodeIndexer()
    {}

    CNodeIndex CNodeIndexer::CreateIndex()
    {
        auto itPos = m_lstIndexList.insert(m_lstIndexList.end(), SNodeIndexElement());
        CNodeIndex index(m_lstIndexList, itPos);
        return index;
    }

    CNodeIndex CNodeIndexer::CreateIndex(const CNodeIndex& rInsertBefore)
    {
        CNodeIndex node_index = CreateIndex();
        node_index.MoveBefore(rInsertBefore);
        return node_index;
    }

    size_t CNodeIndexer::Count()
    {
        return m_lstIndexList.size();
    }

} // namespace toml_parser