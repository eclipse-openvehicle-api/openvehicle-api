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

#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <mutex>

#include <support/pointer.h>
#include <interfaces/mem.h>
#include <support/component_impl.h>
#include <support/mem_access.h>

/**
* @brief Memory Manager service
*/
class CMemoryManager : public sdv::core::IMemoryAlloc, public sdv::IInterfaceAccess, public sdv::internal::IInternalMemAlloc
{
public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::IMemoryAlloc)
    END_SDV_INTERFACE_MAP()

    /**
    * @brief Allocate a memory block of the provided length. Overload of sdv::core::IMemoryAlloc::Allocate.
    * @param[in] uiLength The length of the memory block to allocate.
    * @return Smart pointer to the memory allocation or NULL when allocating was not possible.
    */
    virtual sdv::pointer<uint8_t> Allocate(/*in*/ uint32_t uiLength) override;

private:
    /**
    * @brief Allocate memory. Overload of internal::IInternalMemAlloc::Alloc.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Alloc(size_t nSize) override;

    /**
    * @brief Reallocate memory. Overload of internal::IInternalMemAlloc::Realloc.
    * @param[in] pData Pointer to a previous allocation or NULL when no previous allocation was available.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Realloc(void* pData, size_t nSize) override;

    /**
    * @brief Free a memory allocation. Overload of internal::IInternalMemAlloc::Free.
    * @param[in] pData Pointer to a previous allocation.
    */
    virtual void Free(void* pData) override;

#ifdef MEMORY_TRACKER
    std::mutex m_mtxTracker;                    ///< Synchronize map access
    std::map<void*, size_t> m_mapTracker;       ///< Memory manager allocation tracker
#endif
};

/**
 * @brief Return the memory manager.
 * @return Reference to the memory manager.
 */
CMemoryManager& GetMemoryManager();

#endif // !define MEMORY_H