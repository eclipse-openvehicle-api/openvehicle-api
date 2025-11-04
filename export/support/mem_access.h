#ifndef MEM_SUPPORT_H
#define MEM_SUPPORT_H

#include "../interfaces/mem.h"
#include <map>
#include <thread>
#include <iostream>
#include "interface_ptr.h"
#include "sdv_core.h"

namespace sdv
{
    namespace core
    {
        // Forward declarations
        TInterfaceAccessPtr GetCore();
        template <typename TInterface>
        TInterface* GetCore();

        /**
         * @brief Get access to the memory manager.
         * @return Pointer to the memory allocation interface of the memory manager.
         */
        inline sdv::core::IMemoryAlloc* GetMemMgr()
        {
            static sdv::core::IMemoryAlloc* pAlloc = nullptr;
            static const sdv::IInterfaceAccess* pLocalServices = nullptr;

            // cppcheck warns that GetCore is always returning a pointer. This mechanism is preparing for the situation where the
            // core might be shut down and therefore there is no memory management any more. Suppress the warning.
            // cppcheck-suppress knownConditionTrueFalse
            if (GetCore())
            {
                if (GetCore() != pLocalServices || !pAlloc)
                    pAlloc = GetCore<IMemoryAlloc>();
            }
            else
                pAlloc = nullptr;
            pLocalServices = GetCore();
            return pAlloc;
        }

        /**
         * @brief Allocate memory as byte pointer.
         * @param[in] nAmount The amount of bytes to allocate.
         * @return Returns a pointer to the allocated memory or NULL when the allocation failed.
         */
        inline pointer<uint8_t> AllocMemBytes(size_t nAmount)
        {
            sdv::core::IMemoryAlloc* pMemMgr = GetMemMgr();
            if (!pMemMgr)
                throw sdv::core::XNoMemMgr{};
            pointer<uint8_t> ptrBuffer = pMemMgr->Allocate(static_cast<uint32_t>(nAmount));
            if (!ptrBuffer)
            {
                sdv::core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(nAmount);
                throw exception;
            }
            return ptrBuffer;
        }

        /**
         * @brief Allocate memory
         * @tparam T The type to use for the allocation. Default is uint8_t.
         * @param[in] nAmount The amount of elements to allocate memory for.
         * @return Returns a pointer to the allocated memory of type T or NULL when the allocation failed.
         */
        template <typename T /*= uint8_t*/>
        inline pointer<T> AllocMem(size_t nAmount)
        {
            return make_ptr<T>(nAmount);
            //pointer<uint8_t> ptrBuffer = AllocMemBytes(nAmount * sizeof(T));
            //T* pT = reinterpret_cast<T*>(ptrBuffer.get());
            //for (size_t nIndex = 0; nIndex < nAmount; nIndex++)
            //    new (pT + nIndex) T();
            //pointer<T> ptr;
            //ptr.attach(std::move(ptrBuffer));
            //return ptr;
        }
    } // namespace core
} // namespace sdv


#endif // !defined(MEM_SUPPORT_H)