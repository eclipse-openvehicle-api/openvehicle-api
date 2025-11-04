#ifndef SDV_PTR_INL
#define SDV_PTR_INL

#ifdef min
#undef min
#endif

#ifndef SDV_PTR_H
#error Do not include "pointer.inl" directly. Include "pointer.h" instead!
#endif //!defined SDV_PTR_H

#include "pointer.h"
#include <iostream>
#include <algorithm>

// There are some versions of GCC that produce bogus warnings for -Wstringop-overflow (e.g. version 9.4 warns, 11.4 not - changing
// the compile order without changing the logical behavior, will produce different results).
// See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100477
// And https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115074
// Suppress this warning for the string class.
// NOTE 03.08.2025: Additional bogus warnigs/errors are suppressed for newer versions of the compiler.
#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #pragma GCC diagnostic ignored "-Warray-bounds"
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#ifdef _MSC_VER
    // Prevent static code analysis warnings about noexcept for mode constructors and operators. Due to a constexpr if statement,
    // the constructor might or might not use exceptions and therefore noexcept might or might not be applied.
    #pragma warning(push)
    #pragma warning(disable : 26439)
#endif

/// @cond DOXYGEN_IGNORE
namespace sdv
{
    template <typename T, size_t nFixedSize>
    inline pointer<T, nFixedSize>::pointer() noexcept
    {
        // Ensure the layout is the same on all platforms.
        static_assert(offsetof(pointer, m_uiSize) == 0);
        static_assert(offsetof(pointer, m_rgtData) == 8);
        static_assert(sizeof(pointer) == 8 + aligned_size_bytes);
    }

    template <typename T, size_t nFixedSize>
    inline pointer<T, nFixedSize>::pointer(const pointer& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
    }

    template <typename T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline pointer<T, nFixedSize>::pointer(const pointer<T, nFixedSize2>& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
    }

    template <typename T, size_t nFixedSize>
    inline pointer<T, nFixedSize>::pointer(pointer&& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
    }

    template <typename T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline pointer<T, nFixedSize>::pointer(pointer<T, nFixedSize2>&& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
    }

    template <typename T, size_t nFixedSize>
    inline pointer<T, nFixedSize>::~pointer()
    {
        if constexpr (!std::is_scalar_v<T>)
        {
            for (size_t nIndex = 0; nIndex < nFixedSize; nIndex++)
                m_rgtData[nIndex].~T();
        }
    }

    template <typename T, size_t nFixedSize>
    // Member m_uiReserved is not copied. Suppress static code analysis warning.
    // cppcheck-suppress operatorEqVarError
    inline pointer<T, nFixedSize>& pointer<T, nFixedSize>::operator=(const pointer& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        return *this;
    }

    template <typename T, size_t nFixedSize>
    template <size_t nFixedSize2>
    // Member m_uiReserved is not copied. Suppress static code analysis warning.
    // cppcheck-suppress operatorEqVarError
    inline pointer<T, nFixedSize>& pointer<T, nFixedSize>::operator=(const pointer<T, nFixedSize2>& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        return *this;
    }

    template <typename T, size_t nFixedSize>
    // Member m_uiReserved is not copied. Suppress static code analysis warning.
    // cppcheck-suppress operatorEqVarError
    inline pointer<T, nFixedSize>& pointer<T, nFixedSize>::operator=(pointer&& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
        return *this;
    }

    template <typename T, size_t nFixedSize>
    template <size_t nFixedSize2>
    // Member m_uiReserved is not copied. Suppress static code analysis warning.
    // cppcheck-suppress operatorEqVarError
    inline pointer<T, nFixedSize>& pointer<T, nFixedSize>::operator=(pointer<T, nFixedSize2>&& rptr)
    {
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
        return *this;
    }

    template <typename T, size_t nFixedSize>
    inline void pointer<T, nFixedSize>::reset()
    {
        resize(0);
    }
    
    template <typename T, size_t nFixedSize>
    template <size_t nFixedSizeRight>
    inline void pointer<T, nFixedSize>::swap(pointer<T, nFixedSizeRight>& rptr)
    {
        // Copy the provided content into a temporary buffer.
        T rgtTemp[nFixedSizeRight];
        size_t nSizeRight = rptr.size();
        std::copy_n(rptr.get(), nSizeRight, rgtTemp);

        // Copy the member buffer into the buffer of the provided pointer
        rptr.resize(size());
        std::copy_n(get(), size(), rptr.get());

        // Copy the temporary buffer into this class' buffer.
        resize(nSizeRight);
        std::copy_n(rgtTemp, nSizeRight, get());
    }

    template <typename T, size_t nFixedSize>
    inline typename pointer<T, nFixedSize>::element_type* pointer<T, nFixedSize>::get() const noexcept
    {
        return const_cast<element_type*>(m_rgtData);
    }

    template <typename T, size_t nFixedSize>
    inline T& pointer<T, nFixedSize>::operator*() const
    {
        return *get();
    }

    template <typename T, size_t nFixedSize>
    inline T* pointer<T, nFixedSize>::operator->() const
    {
        return get();
    }

    template <typename T, size_t nFixedSize>
    inline typename pointer<T, nFixedSize>::element_type& pointer<T, nFixedSize>::operator[](size_t nIndex) const
    {
        if (nIndex >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nIndex);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return get()[nIndex];
    }

    template <typename T, size_t nFixedSize>
    inline pointer<T, nFixedSize>::operator bool() const noexcept
    {
        // Always true
        return true;
    }

    template <typename T, size_t nFixedSize>
    inline size_t pointer<T, nFixedSize>::size() const noexcept
    {
        return m_uiSize;
    }

    template <typename T, size_t nFixedSize>
    inline void pointer<T, nFixedSize>::resize(size_t nSize)
    {
        // Replace all deleted elements with the default constructed elements
        for (size_t nIndex = nSize; nIndex < std::min(nFixedSize, static_cast<size_t>(m_uiSize)); nIndex++)
        {
            // Destruct the existing and construct the new
            if constexpr (!std::is_scalar_v<T>)
            {
                m_rgtData[nIndex].~T();
                new (m_rgtData + nIndex) T();
            }
            else
                if (nIndex < nFixedSize)
                    m_rgtData[nIndex] = static_cast<T>(0);

        }

        if (nSize > nFixedSize)
        {
            XBufferTooSmall exception;
            exception.uiSize = static_cast<uint64_t>(nSize);
            exception.uiCapacity = static_cast<uint64_t>(nFixedSize);
            throw exception;
        }

        m_uiSize = static_cast<uint32_t>(nSize);
    }

    template <typename T, size_t nFixedSize>
    inline size_t pointer<T, nFixedSize>::capacity() const noexcept
    {
        return nFixedSize;
    }

    template <typename T>
    inline pointer<T, 0>::pointer() noexcept
    {
        // The size of the pointer class should not be larger than the size of a pointer.
        static_assert(sizeof(pointer<T>) == sizeof(T*));

        // The size of the allocation class should be the size of two pointers and two ints. This is important to ensure that the
        // location of each element is exactly the same for every process regardless of compiler and library version.
        static_assert(sizeof(SAllocation) == sizeof(internal::IInternalMemAlloc*) + sizeof(T*) + sizeof(uint32_t) * 2);
    }

    namespace core
    {
        // Forward declarations
        pointer<uint8_t> AllocMemBytes(size_t nAmount);
    } // namespace core

    namespace internal
    {
        /**
         * @brief Internal memory allocation interface doing the actual allocation. This interface is not exposed publicly and
         * should only be used by the memory manager.
         */
        interface IInternalMemAlloc
        {
            /**
             * @brief Allocate memory.
             * @param[in] nSize The size of the memory to allocate (in bytes).
             * @return Pointer to the memory allocation or NULL when memory allocation failed.
             */
            virtual void* Alloc(size_t nSize) = 0;

            /**
             * @brief Reallocate memory.
             * @param[in] pData Pointer to a previous allocation or NULL when no previous allocation was available.
             * @param[in] nSize The size of the memory to allocate (in bytes).
             * @return Pointer to the memory allocation or NULL when memory allocation failed.
             */
            virtual void* Realloc(void* pData, size_t nSize) = 0;

            /**
             * @brief Free a memory allocation.
             * @param[in] pData Pointer to a previous allocation.
             */
            virtual void Free(void* pData) = 0;
        };
    } // namespace internal

    template <typename T>
    inline pointer<T, 0>::pointer(const pointer& rptr)
    {
        m_psAllocation = rptr.m_psAllocation;
        if (m_psAllocation)
            m_psAllocation->uiRefCnt++;
    }

    template <typename T>
    template <size_t nFixedSize2>
    inline pointer<T, 0>::pointer(const pointer<T, nFixedSize2>& rptr)
    {
        // Fixed sized buffer; copy the content
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
    }

    template <typename T>
    inline pointer<T, 0>::pointer(pointer&& rptr) noexcept
    {
        m_psAllocation = rptr.m_psAllocation;
        rptr.m_psAllocation = nullptr;
    }

    template <typename T>
    template <size_t nFixedSize2>
    inline pointer<T, 0>::pointer(const pointer<T, nFixedSize2>&& rptr)
    {
        // Fixed sized buffer; copy the content and reset supplied pointer
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
    }

    template <typename T>
    inline pointer<T, 0>::~pointer()
    {
        try
        {
            reset();
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
    }

    template <typename T>
    inline pointer<T>& pointer<T, 0>::operator=(const pointer& rptr)
    {
        // Dynamic buffer, share the allocation
        reset();
        m_psAllocation = rptr.m_psAllocation;
        if (m_psAllocation)
            m_psAllocation->uiRefCnt++;
        return *this;
    }

    template <typename T>
    template <size_t nFixedSize2>
    inline pointer<T>& pointer<T, 0>::operator=(const pointer<T, nFixedSize2>& rptr)
    {
        // Fixed sized buffer; copy the content
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        return *this;
    }

    template <typename T>
    inline pointer<T>& pointer<T, 0>::operator=(pointer<T>&& rptr) noexcept
    {
        // Dynamic buffer, take over the allocation.
        try
        {
            reset();
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        m_psAllocation = rptr.m_psAllocation;
        rptr.m_psAllocation = nullptr;
        return *this;
    }

    template <typename T>
    template <size_t nFixedSize2>
    inline pointer<T>& pointer<T, 0>::operator=(pointer<T, nFixedSize2>&& rptr)
    {
        // Fixed sized buffer; copy the content and reset supplied pointer
        resize(rptr.size());
        std::copy_n(rptr.get(), rptr.size(), get());
        rptr.reset();
        return *this;
    }

    template <typename T>
    inline void pointer<T, 0>::attach(pointer<uint8_t>&& rptrBuffer)
    {
        // Move buffer pointer (structures are identical).
        operator=(std::move(*reinterpret_cast<pointer<T>*>(&rptrBuffer)));
    }

    template <typename T>
    inline pointer<uint8_t>&& pointer<T, 0>::detach()
    {
        // Move this pointer (structures are identical).
        return std::move(*reinterpret_cast<pointer<uint8_t>*>(this));
    }

    template <typename T>
    inline void pointer<T, 0>::reset()
    {
        // Reduce allocation sharing reference counter. If 0, no more share; delete allocation.
        if (!m_psAllocation) return;
        if (!m_psAllocation->pAllocator) throw core::XNoMemMgr();
        if (!m_psAllocation->uiRefCnt)
        {
            XInvalidRefCount exception;
            exception.uiCount = 0;
            throw exception;
        }
        if (!--m_psAllocation->uiRefCnt)
        {
            // Call the destructor on all elements
            if constexpr (!std::is_scalar_v<T>)
            {
                for (size_t nIndex = 0; m_psAllocation->pData && nIndex < m_psAllocation->uiSize / sizeof(T); nIndex++)
                    m_psAllocation->pData[nIndex].~T();
            }

            // Delete the data
            if (m_psAllocation->pData)
                m_psAllocation->pAllocator->Free(m_psAllocation->pData);

            // Delete the allocation structure
            delete m_psAllocation;
        }
        m_psAllocation = nullptr;
    }

    template <typename T>
    template <size_t nFixedSizeRight>
    inline void pointer<T, 0>::swap(pointer<T, nFixedSizeRight>& rptr)
    {
        if constexpr (nFixedSizeRight != 0)
        {
            // Fixed sized buffer; copy the content
            // Copy the provided content into a temporary buffer.
            T rgtTemp[nFixedSizeRight];
            size_t nSizeRight = rptr.size();
            std::copy_n(rptr.get(), nSizeRight, rgtTemp);

            // Copy the member buffer into the buffer of the provided pointer
            rptr.resize(size());
            std::copy_n(get(), size(), rptr.get());

            // Copy the temporary buffer into this class' buffer.
            resize(nSizeRight);
            std::copy_n(rgtTemp, nSizeRight, get());
        }
        else
        {
            // Dynamic sized buffer; exchange the pointer
            SAllocation* pTemp	= m_psAllocation;
            m_psAllocation = rptr.m_psAllocation;
            rptr.m_psAllocation = pTemp;
        }
    }

    template <typename T>
    inline typename pointer<T, 0>::element_type* pointer<T, 0>::get() const noexcept
    {
        return m_psAllocation ? m_psAllocation->pData : nullptr;
    }

    template <typename T>
    inline T& pointer<T, 0>::operator*() const
    {
        if (!m_psAllocation) throw XNullPointer();
        return *get();
    }

    template <typename T>
    inline T* pointer<T, 0>::operator->() const
    {
        if (!m_psAllocation) throw XNullPointer();
        return get();
    }

    template <typename T>
    inline typename pointer<T, 0>::element_type& pointer<T, 0>::operator[](size_t nIndex) const
    {
        if (!m_psAllocation) throw XNullPointer();
        if (nIndex >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nIndex);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return get()[nIndex];
    }

    template <typename T>
    inline pointer<T, 0>::operator bool() const noexcept
    {
        return m_psAllocation && m_psAllocation->pData;
    }
    
    template <typename T>
    inline size_t pointer<T, 0>::size() const noexcept
    {
        return m_psAllocation ? m_psAllocation->uiSize / sizeof(T) : 0;
    }

    template <typename T>
    inline void pointer<T, 0>::resize(size_t nSize)
    {
        if (!m_psAllocation || !m_psAllocation->pAllocator || !m_psAllocation->pData)
        {
            operator=(std::move(make_ptr<T>(nSize)));
            return;
        }
        size_t nTotalSize = nSize * sizeof(T);

        // Is the new size smaller than the old size, call the destructors for the deleted
        if (nSize < m_psAllocation->uiSize / sizeof(T))
        {
            if constexpr (!std::is_scalar_v<T>)
            {
                for (size_t nIndex = nSize; nIndex < m_psAllocation->uiSize / sizeof(T); nIndex++)
                    m_psAllocation->pData[nIndex].~T();
            }

            // Since the destructors are called; update the size info already.
            m_psAllocation->uiSize = static_cast<uint32_t>(nSize * sizeof(T));
        }

        // Treat reallocation differently between objects and scalars. Objects might have pointers and therefore need to be
        // copied. Scalars can be reallocated without reinitialization.
        T* ptAllocation = nullptr;
        if constexpr (std::is_scalar_v<T>)
        {
            ptAllocation = reinterpret_cast<T*>(m_psAllocation->pAllocator->Realloc(m_psAllocation->pData, nTotalSize));
            if (nTotalSize && !ptAllocation)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(nTotalSize);
                throw exception;
            }

            // Is the new size larger than the old size, call the constructors for the new elements
            for (size_t nIndex = m_psAllocation->uiSize / sizeof(T); nIndex < nSize; nIndex++)
                ptAllocation[nIndex] = static_cast<T>(0);
        }
        else
        {
            // Allocate new size
            ptAllocation = reinterpret_cast<T*>(m_psAllocation->pAllocator->Alloc(nTotalSize));
            if (nTotalSize && !ptAllocation)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(nTotalSize);
                throw exception;
            }

            // Move the elements from the existing allocation to the new allocation.
            for (size_t nIndex = 0; nIndex < std::min(static_cast<size_t>(m_psAllocation->uiSize / sizeof(T)), nSize); nIndex++)
                new (ptAllocation + nIndex) T(std::move(m_psAllocation->pData[nIndex]));

            // Is the new size larger than the old size, call the constructors for the new elements
            for (size_t nIndex = m_psAllocation->uiSize / sizeof(T); nIndex < nSize; nIndex++)
                new (ptAllocation + nIndex) T();

            // Free the old data.
            if (m_psAllocation->pData) m_psAllocation->pAllocator->Free(m_psAllocation->pData);
        }

        // Set new information
        m_psAllocation->pData = ptAllocation;
        m_psAllocation->uiSize = static_cast<uint32_t>(nSize * sizeof(T));
    }

    template <typename T>
    inline size_t pointer<T, 0>::capacity() const noexcept
    {
        return m_psAllocation ? m_psAllocation->uiSize / sizeof(T) : 0;
    }

    template <typename T>
    inline size_t pointer<T, 0>::ref_count() const noexcept
    {
        return m_psAllocation ? static_cast<size_t>(m_psAllocation->uiRefCnt) : 0;
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline void swap(pointer<T, nFixedSizeLeft>& rptrLeft, pointer<T, nFixedSizeRight>& rptrRight)
    {
        rptrLeft.swap(rptrRight);
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator==(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        if constexpr (nFixedSizeLeft || nFixedSizeRight) // Compare content
        {
            // Prevent comparing NULL-pointer.
            if (rptrLeft.get() == rptrRight.get()) return true;         // Pointers are identical; comparing same object.
            if (!rptrLeft.size() && !rptrRight.size()) return true;     // Both pointers are empty.
            if (rptrLeft.get() && !rptrRight.get()) return false;       // Right is NULL; left not.
            if (!rptrLeft.get() && rptrRight.get()) return false;       // Left is NULL; right not.
            return std::equal(rptrLeft.get(), rptrLeft.get() + rptrLeft.size(),
                rptrRight.get(), rptrRight.get() + rptrRight.size());
        } else // Compare pointers
            return rptrLeft.get() == rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator!=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        return !operator==(rptrLeft, rptrRight);
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator<(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        if constexpr (nFixedSizeLeft || nFixedSizeRight) // Compare content
            return std::lexicographical_compare(rptrLeft.get(), rptrLeft.get() + rptrLeft.size(),
                rptrRight.get(), rptrRight.get() + rptrRight.size());
        else // Compare pointers
            return rptrLeft.get() < rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator<=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        if constexpr (nFixedSizeLeft || nFixedSizeRight) // Compare content
            return !std::lexicographical_compare(rptrRight.get(), rptrRight.get() + rptrRight.size(),
                rptrLeft.get(), rptrLeft.get() + rptrLeft.size());
        else // Compare pointers
            return rptrLeft.get() <= rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator>(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        if constexpr (nFixedSizeLeft || nFixedSizeRight) // Compare content
            return std::lexicographical_compare(rptrRight.get(), rptrRight.get() + rptrRight.size(),
                rptrLeft.get(), rptrLeft.get() + rptrLeft.size());
        else // Compare pointers
            return rptrLeft.get() > rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator>=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        if constexpr (nFixedSizeLeft || nFixedSizeRight) // Compare content
            return !std::lexicographical_compare(rptrLeft.get(), rptrLeft.get() + rptrLeft.size(),
                rptrRight.get(), rptrRight.get() + rptrRight.size());
        else // Compare pointers
            return rptrLeft.get() >= rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator==(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t) noexcept
    {
        return rptrLeft.get() == nullptr;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator==(std::nullptr_t, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        return nullptr == rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator!=(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t) noexcept
    {
        return rptrLeft.get() != nullptr;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator!=(std::nullptr_t, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        return nullptr != rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator<(const pointer<T, nFixedSizeLeft>& /*rptrLeft*/, std::nullptr_t) noexcept
    {
        return false;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator<(std::nullptr_t, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        return nullptr != rptrRight.get();
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator<=(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t) noexcept
    {
        return rptrLeft.get() == nullptr;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator<=(std::nullptr_t, const pointer<T, nFixedSizeRight>& /*rptrRight*/) noexcept
    {
        return true;
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator>(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t) noexcept
    {
        return rptrLeft.get() != nullptr;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator>(std::nullptr_t, const pointer<T, nFixedSizeRight>& /*rptrRight*/) noexcept
    {
        return false;
    }

    template <class T, size_t nFixedSizeLeft>
    inline bool operator>=(const pointer<T, nFixedSizeLeft>& /*rptrLeft*/, std::nullptr_t) noexcept
    {
        return true;
    }

    template <class T, size_t nFixedSizeRight>
    inline bool operator>=(std::nullptr_t, const pointer<T, nFixedSizeRight>& rptrRight) noexcept
    {
        return nullptr == rptrRight.get();
    }

    template <class T, class U, class V, size_t nFixedSize>
    inline std::basic_ostream<U, V>& operator<<(std::basic_ostream<U, V>& rstream, const pointer<T, nFixedSize>& rptr)
    {
        for (size_t nIndex = 0; nIndex < rptr.size(); nIndex++)
            rstream << rptr[nIndex];
        return rstream;
    }

    template <typename T>
    inline pointer<T> make_ptr(size_t nSize /*= 1*/)
    {
        pointer<uint8_t> ptrBuffer = core::AllocMemBytes(nSize * sizeof(T));
        T* pT = reinterpret_cast<T*>(ptrBuffer.get());
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
            new (pT + nIndex) T();
        pointer<T> ptr;
        ptr.attach(std::move(ptrBuffer));
        return ptr;
    }

    namespace internal
    {
        /**
         * @brief Create a pointer class using the allocator and allocating the memory with the given size.
         * @tparam T Type to use for the allocation.
         * @param[in] pAllocator Pointer to the allocator. Must not be NULL.
         * @param[in] nSize Size to allocate (in elements).
         * @return The pointer class.
         */
        template <typename T>
        inline pointer<T> make_ptr(internal::IInternalMemAlloc* pAllocator, size_t nSize /*= 1*/)
        {
            pointer<T> pointer;
            if (!pAllocator) throw core::XNoMemMgr();
            size_t nTotalSize  = nSize * sizeof(T);

            // Create a new allocation structure
            using SAllocation = std::remove_pointer_t<decltype(pointer.m_psAllocation)>;
            pointer.m_psAllocation = new SAllocation;
            if (!pointer.m_psAllocation)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(sizeof(SAllocation));
                throw exception;
            }
            pointer.m_psAllocation->pAllocator = pAllocator;

            // Allocate the amount of elements
            pointer.m_psAllocation->pData = reinterpret_cast<T*>(pAllocator->Alloc(nTotalSize));
            if (!pointer.m_psAllocation->pData)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(nTotalSize);
                throw exception;
            }

            // Call the constructors for each memory allocation
            if constexpr (std::is_scalar_v<T>)
                std::fill(pointer.m_psAllocation->pData, pointer.m_psAllocation->pData + nSize, static_cast<T>(0));
            else
            {
                for (size_t nIndex = 0; nIndex < nSize; nIndex++)
                    new (pointer.m_psAllocation->pData + nIndex) T();
            }

            // Fill in allocation details
            pointer.m_psAllocation->uiSize = static_cast<uint32_t>(nSize * sizeof(T));
            pointer.m_psAllocation->uiRefCnt = 1;
            return pointer;
        }
    } // namespace internal

    template <typename TTraits /*= std::char_traits<char>*/>
    inline pointer<uint8_t> from_stream(std::basic_streambuf<char, TTraits>* pStreamBuf)
    {
        if (!pStreamBuf) throw sdv::XNullPointer();

        // Determine the size of the buffer.
        std::streampos current_pos = pStreamBuf->pubseekoff(0, std::ios::cur, std::ios::in);
        std::streampos end_pos = pStreamBuf->pubseekoff(0, std::ios::end, std::ios::in);
        pStreamBuf->pubseekpos(current_pos, std::ios::in);

        // Allocate the buffer
        pointer<uint8_t, 0> ptr;
        ptr.resize(end_pos);

        // Fill the buffer
        size_t nPos = 0;
        while (true)
        {
            auto nAvail = pStreamBuf->in_avail();
            if (nAvail)
            {
                if (ptr.size() < nPos + nAvail)
                    ptr.resize(nPos + nAvail);
                pStreamBuf->sgetn(reinterpret_cast<char*>(ptr.get() + nPos), nAvail);
                nPos += nAvail;
            }
            else
            {
                auto nVal = pStreamBuf->sbumpc();
                if (nVal == TTraits::eof()) break;
                else
                {
                    if (ptr.size() < nPos + 1)
                        ptr.resize(nPos + 1);
                    ptr[nPos] = static_cast<uint8_t>(nVal);
                    nPos++;
                }
            }
        }

        // Resize needed?
        if (nPos != ptr.size())
            ptr.resize(nPos);

        return ptr;
    }

    template <typename TTraits /*= std::char_traits<char>*/>
    inline void to_stream(const pointer<uint8_t>& rptr, std::basic_streambuf<char, TTraits>* pStreamBuf)
    {
        if (!pStreamBuf) throw sdv::XNullPointer();
        if (rptr.size())
            pStreamBuf->sputn(reinterpret_cast<const char*>(rptr.get()), rptr.size());
    }

    template <typename T>
    inline T* cast(pointer<uint8_t>& rptr, size_t nOffset /*= 0*/)
    {
        if (nOffset + sizeof(T) > rptr.size()) return nullptr;
        return reinterpret_cast<T*>(rptr.get() + nOffset);
    }

    template <typename T>
    inline const T* cast(const pointer<uint8_t>& rptr, size_t nOffset /*= 0*/)
    {
        if (nOffset + sizeof(T) > rptr.size()) return nullptr;
        return reinterpret_cast<const T*>(rptr.get() + nOffset);
    }

} // namespace sdv
/// @endcond

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

#endif // !defined SDV_PTR_INL