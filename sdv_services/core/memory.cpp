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

#include "memory.h"

CMemoryManager& GetMemoryManager()
{
    static CMemoryManager memory_mgr;
    return memory_mgr;
}

sdv::pointer<uint8_t> CMemoryManager::Allocate(uint32_t uiLength)
{
    return sdv::internal::make_ptr<uint8_t>(this, uiLength);
}

void* CMemoryManager::Alloc(size_t nSize)
{
    void* pAlloc = malloc(nSize);
#ifdef MEMORY_TRACKER
    std::unique_lock<std::mutex> lock(m_mtxTracker);
    if (pAlloc)
        m_mapTracker.insert(std::make_pair(pAlloc, nSize));
#endif
    return pAlloc;
}

void* CMemoryManager::Realloc(void* pData, size_t nSize)
{
#ifdef MEMORY_TRACKER
    std::unique_lock<std::mutex> lock(m_mtxTracker);
    auto itAlloc = m_mapTracker.find(pData);
    if (pData && itAlloc == m_mapTracker.end())
    {
        std::cout << "Illegal request for resizing memory at location 0x" << (void*)pData << std::endl;
        return nullptr;
    }
    lock.unlock();
#endif

    void* pAlloc = realloc(pData, nSize);

#ifdef MEMORY_TRACKER
    if (pAlloc)
    {
        lock.lock();
        m_mapTracker.erase(pData);
        m_mapTracker.insert(std::make_pair(pAlloc, nSize));
    }
#endif

    return pAlloc;
}

void CMemoryManager::Free(void* pData)
{
#ifdef MEMORY_TRACKER
    std::unique_lock<std::mutex> lock(m_mtxTracker);
    auto itAlloc = m_mapTracker.find(pData);
    if (itAlloc == m_mapTracker.end())
    {
        std::cout << "Illegal request for freeing memory at location 0x" << (void*)pData << std::endl;
        return;
    }
    else
        m_mapTracker.erase(itAlloc);
    lock.unlock();
#endif

    free(pData);
}
