#ifndef SDV_STRING_INL
#define SDV_STRING_INL

#ifndef SDV_STRING_H
#error Do not include "string.inl" directly. Include "string.h" instead!
#endif //!defined SDV_STRING_H

#include <string>
#include <algorithm>
#include <ostream>
#include <istream>

#ifdef _MSC_VER
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#ifdef __GNUC__
// There are some versions of GCC that produce bogus warnings for -Wstringop-overflow (e.g. version 9.4 warns, 11.4 not - changing
// the compile order without changing the logical behavior, will produce different results).
// See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100477
// And https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115074
// Suppress this warning for the string class.
// NOTE 03.08.2025: Additional bogus warnigs/errors are suppressed for newer versions of the compiler.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wrestrict"
#endif

namespace sdv
{
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base() noexcept
    {}

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::~string_base()
    {
        clear();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const string_base& rss) : string_base()
    {
        insert(0, rss);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <typename TCharType2, bool bUnicode2, size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const string_base<TCharType2, bUnicode2, nFixedSize2>& rss) : string_base()
    {
        insert(0, MakeString<TCharType2, bUnicode2, nFixedSize2, TCharType, bUnicode, nFixedSize>(rss));
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(string_base&& rss) noexcept : m_ptrData(std::move(rss.m_ptrData))
    {}

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(string_base<TCharType, bUnicode, nFixedSize2>&& rss)
    {
        m_ptrData = std::move(rss.buffer());
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <typename TCharType2>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const std::basic_string<TCharType2>& rss) : string_base()
    {
        insert(0, MakeString<TCharType2, TCharType, bUnicode, nFixedSize>(rss));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const TCharType* szStr) : string_base()
    {
        insert(0, szStr);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(size_t nCount, TCharType c) : string_base()
    {
        insert(0, nCount, c);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos) :
        string_base()
    {
        insert(0, rss, nPos);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const std::basic_string<TCharType>& rss, size_t nPos) : string_base()
    {
        insert(0, rss, nPos);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos,
        size_t nCount) : string_base()
    {
        insert(0, rss, nPos, nCount);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount) : string_base()
    {
        insert(0, rss, nPos, nCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(const TCharType* sz, size_t nCount) : string_base()
    {
        insert(0, sz, nCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <class TIterator>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(TIterator itFirst, TIterator itLast) : string_base()
    {
        insert(cbegin(), itFirst, itLast);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::string_base(std::initializer_list<TCharType> ilist) : string_base()
    {
        insert(cbegin(), ilist.begin(), ilist.end());
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(const string_base& rss)
    {
        clear();
        return insert(0, rss);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        clear();
        return insert(0, rss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(string_base&& rss) noexcept
    {
        clear();
        m_ptrData = std::move(rss.m_ptrData);
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(
        string_base<TCharType, bUnicode, nFixedSize2>&& rss)
    {
        clear();
        m_ptrData = std::move(rss.buffer());
        return *this;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(
        const std::basic_string<TCharType>& rss)
    {
        clear();
        return insert(0, rss);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(const TCharType* szStr)
    {
        clear();
        return insert(0, szStr);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator=(
        std::initializer_list<TCharType> ilist)
    {
        clear();
        insert(cbegin(), ilist);
        return *this;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(size_t nCount, TCharType c)
    {
        clear();
        insert(0, nCount, c);
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        clear();
        return insert(0, rss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(const std::basic_string<TCharType>& rss)
    {
        clear();
        return insert(0, rss);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount /*= npos*/)
    {
        clear();
        return insert(0, rss, nPos, nCount);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(const std::basic_string<TCharType>& rss,
        size_t nPos, size_t nCount /*= npos*/)
    {
        clear();
        return insert(0, rss, nPos, nCount);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(string_base<TCharType, bUnicode, nFixedSize2>&& rss)
    {
        clear();
        m_ptrData = std::move(rss.buffer());
        return *this;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(const TCharType* sz, size_t nCount)
    {
        clear();
        return insert(0, sz, nCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(const TCharType* sz)
    {
        clear();
        return insert(0, sz);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <class TIterator>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(TIterator itFirst, TIterator itLast)
    {
        clear();
        insert(cbegin(), itFirst, itLast);
        return *this;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::assign(std::initializer_list<TCharType> ilist)
    {
        clear();
        insert(cbegin(), ilist);
        return *this;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::reference string_base<TCharType, bUnicode, nFixedSize>::at(size_t nPos)
    {
        if (nPos >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nPos);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[nPos];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reference string_base<TCharType, bUnicode, nFixedSize>::at(size_t nPos) const
    {
        if (nPos >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nPos);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[nPos];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::reference string_base<TCharType, bUnicode, nFixedSize>::operator[](size_t nPos)
    {
        if (nPos >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nPos);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[nPos];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reference string_base<TCharType, bUnicode, nFixedSize>::operator[](size_t nPos) const
    {
        if (nPos >= size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(nPos);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[nPos];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline TCharType& string_base<TCharType, bUnicode, nFixedSize>::front()
    {
        if (!size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(0);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[0];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline const TCharType& string_base<TCharType, bUnicode, nFixedSize>::front() const
    {
        if (!size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(0);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[0];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline TCharType& string_base<TCharType, bUnicode, nFixedSize>::back()
    {
        if (!size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(0);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[size() - 1];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline const TCharType& string_base<TCharType, bUnicode, nFixedSize>::back() const
    {
        if (!size())
        {
            XIndexOutOfRange exception;
            exception.uiIndex = static_cast<uint32_t>(0);
            exception.uiSize = static_cast<uint32_t>(size());
            throw exception;
        }
        return m_ptrData[size() - 1];
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <typename TCharType2, bool bUnicode2, size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>::operator string_base<TCharType2, bUnicode2, nFixedSize2>() const
    {
        return MakeString<TCharType, bUnicode, nFixedSize, TCharType2, bUnicode2, nFixedSize2>(*this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>::operator std::basic_string<TCharType>() const
    {
        std::basic_string<TCharType> ss;
        if (size())
        {
            try
            {
                ss.resize(size());
            } catch (const std::bad_alloc&)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(size() * sizeof(TCharType));
                throw exception;
            }
            std::copy_n(data(), size(), &ss.front());
        }
        return ss;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline const TCharType* string_base<TCharType, bUnicode, nFixedSize>::data() const noexcept
    {
        return m_ptrData.get();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline pointer<TCharType, nFixedSize ? nFixedSize + 1 : 0>& string_base<TCharType, bUnicode, nFixedSize>::buffer() noexcept
    {
        return m_ptrData;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline const TCharType* string_base<TCharType, bUnicode, nFixedSize>::c_str() const noexcept
    {
        return m_ptrData.get();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::begin() noexcept
    {
        return iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_iterator string_base<TCharType, bUnicode, nFixedSize>::begin() const noexcept
    {
        return const_iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_iterator string_base<TCharType, bUnicode, nFixedSize>::cbegin() const noexcept
    {
        return const_iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::rbegin() noexcept
    {
        return reverse_iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::rbegin() const noexcept
    {
        return const_reverse_iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::crbegin() const noexcept
    {
        return const_reverse_iterator(this);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::end() noexcept
    {
        iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_iterator string_base<TCharType, bUnicode, nFixedSize>::end() const noexcept
    {
        const_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_iterator string_base<TCharType, bUnicode, nFixedSize>::cend() const noexcept
    {
        const_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::rend() noexcept
    {
        reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::rend() const noexcept
    {
        const_reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::const_reverse_iterator string_base<TCharType, bUnicode, nFixedSize>::crend() const noexcept
    {
        const_reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline bool string_base<TCharType, bUnicode, nFixedSize>::empty() const
    {
        return !size();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::size() const
    {
        return m_ptrData.size() ? m_ptrData.size() - 1 : 0;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::length() const
    {
        return size();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::reserve(size_t nNewCap /*= 0*/)
    {
        if (nNewCap > size())
            resize(nNewCap);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::capacity() const noexcept
    {
        return m_ptrData.capacity();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::shrink_to_fit()
    {
        // Do nothing...
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::clear()
    {
        m_ptrData.reset();
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex, size_t nCount, TCharType c)
    {
        // Calculate count
        size_t nCountTemp = nCount;
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;
        if (!nCountTemp) return *this;

        // Allocate space for a new string.
        m_ptrData.resize(nCurrentLen + nCount + 1); // Allocate space for a terminating zero

                                                    // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Insert the characters
        if (nCountTemp) std::fill_n(m_ptrData.get() + std::min(nIndex, nCurrentLen), nCountTemp, c);

        // Set the terminating zero
        m_ptrData[size()] = static_cast<TCharType>('\0');

        return *this;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex, const TCharType* sz)
    {
        return insert(nIndex, sz, sz ? std::char_traits<TCharType>::length(sz) : 0);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex, const TCharType* sz, size_t nCount)
    {
        // Check for a valid pointer
        if (!sz && nCount) throw XNullPointer();

        // Calculate count
        size_t nCountTemp = nCount;
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;
        if (!nCountTemp) return *this;

        // Allocate space for a new string.
        m_ptrData.resize(nCurrentLen + nCount + 1); // Allocate space for a terminating zero
        if (nCountTemp + nCurrentLen > size())
        {
            XBufferTooSmall exception;
            exception.uiSize = static_cast<uint64_t>(nCurrentLen + nCount);
            exception.uiCapacity = static_cast<uint64_t>(size());
            throw exception;
        }

        // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Copy the characters
        if (nCountTemp)
            std::copy_n(sz, nCountTemp, m_ptrData.get() + std::min(nIndex, nCurrentLen));

        // Set the terminating zero
        m_ptrData[size()] = static_cast<TCharType>('\0');

        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        return insert(nIndex, rss, 0);
    }
    /// @endcond

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount /*= npos*/)
    {
        // Calculate the substring position and count
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nPosTemp = std::min(nPos, rss.size());
        size_t nSizeTemp = rss.size() > nPosTemp ? rss.size() - nPosTemp : 0;
        if (nFixedSize2 && nPosTemp > nFixedSize2) nPosTemp = nFixedSize2;
        size_t nCountTemp = std::min(nSizeTemp, nCount);
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;
        if (!nCountTemp) return *this;

        // Allocate space for a new string.
        m_ptrData.resize(nCurrentLen + nCountTemp + 1); // Allocate space for a terminating zero
        if (nCountTemp + nCurrentLen > size())
        {
            XBufferTooSmall exception;
            exception.uiSize = static_cast<uint64_t>(nCurrentLen + nCount);
            exception.uiCapacity = static_cast<uint64_t>(size());
            throw exception;
        }

        // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Copy the characters        
#ifdef __GNUC__
        // Use a GCC specific implementation to prevent a bogus -Werror=stringop-overread error - also mentioned in the thread at:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98465
        for (size_t n = 0; n < nCountTemp; n++)
        {
            if ((!nFixedSize || n + std::min(nIndex, nCurrentLen) < nFixedSize) && (!nFixedSize2 || n + nPosTemp < nFixedSize2))
                (m_ptrData.get() + std::min(nIndex, nCurrentLen))[n] = (rss.data() + nPosTemp)[n];
        }
#else
        std::copy_n(rss.data() + nPosTemp, nCountTemp, m_ptrData.get() + std::min(nIndex, nCurrentLen));
#endif

        // Set the terminating zero
        m_ptrData[size()] = static_cast<TCharType>('\0');

        return *this;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex,
        const std::basic_string<TCharType>& rss)
    {
        return insert(nIndex, rss, 0);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::insert(size_t nIndex,
        const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount /*= npos*/)
    {
        // Calculate the substring position and count
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nPosTemp = std::min(nPos, rss.size());
        size_t nSizeTemp = rss.size() > nPosTemp ? rss.size() - nPosTemp : 0;
        size_t nCountTemp = std::min(nSizeTemp, nCount);
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;

        // Allocate space for a new string.
        m_ptrData.resize(nCurrentLen + nCountTemp + 1); // Allocate space for a terminating zero

        // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Copy the characters
        if (nCountTemp) std::copy_n(rss.data() + nPosTemp, nCountTemp, m_ptrData.get() + std::min(nIndex, nCurrentLen));

        // Set the terminating zero
        m_ptrData[size()] = static_cast<TCharType>('\0');

        return *this;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::insert(const_iterator itPos, TCharType c)
    {
        return insert(itPos, 1, c);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::insert(const_iterator itPos, size_t nCount, TCharType c)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();
        size_t nIndex = 0;
        if (itPos == cend())
            nIndex = size();
        else if (!empty())
            nIndex = &itPos[0] - data();
        insert(nIndex, nCount, c);
        return iterator(itPos);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <class TIterator>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::insert(const_iterator itPos, TIterator itFirst, TIterator itLast)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();
        string_base ss;
        for (TIterator itIndex = itFirst; itIndex != itLast; itIndex++)
            ss += *itIndex;
        size_t nIndex = 0;
        if (itPos == cend())
            nIndex = size();
        else if (!empty())
            nIndex = &itPos[0] - data();
        insert(nIndex, ss);
        return iterator(itPos);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::insert(const_iterator itPos, std::initializer_list<TCharType> ilist)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();
        return insert(itPos, ilist.begin(), ilist.end());
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::erase(size_t nIndex /*= 0*/, size_t nCount /*= npos*/)
    {
        if (!nCount) return *this; // Do nothing
        if (nIndex >= size()) return *this; // Do nothing

        // Copy any leftover characters
        if (nCount < size() && ((nIndex + nCount) < size()))
        {
            std::copy_n(m_ptrData.get() + nIndex + nCount, size() - nIndex - nCount, m_ptrData.get() + nIndex);
            resize(size() - nCount);
        }
        else
            resize(nIndex);
        return *this;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::erase(const_iterator itPos)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();
        if (itPos == cend()) return itPos; // Nothing to do
        if (empty()) return cbegin(); // Nothing to do
        size_t nIndex = &itPos[0] - data();
        erase(nIndex, 1);
        return itPos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline typename string_base<TCharType, bUnicode, nFixedSize>::iterator string_base<TCharType, bUnicode, nFixedSize>::erase(const_iterator itFirst, const_iterator itLast)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return itFirst; // Nothing to do
        if (empty()) return cbegin(); // Nothing to do
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst) throw XInvalidIterator();
        erase(nIndexFirst, nIndexLast - nIndexFirst);
        return itFirst;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::push_back(TCharType c)
    {
        append(1, c);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::pop_back()
    {
        if (empty()) return; // Nothing to do
        erase(cend() - 1);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(size_t nCount, TCharType c)
    {
        insert(size(), nCount, c);
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        return insert(size(), rss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(const std::basic_string<TCharType>& rss)
    {
        return insert(size(), rss);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount /*= npos*/)
    {
        return insert(size(), rss, nPos, nCount);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount /*= npos*/)
    {
        return insert(size(), rss, nPos, nCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(const TCharType* sz, size_t nCount)
    {
        return insert(size(), sz, nCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(const TCharType* sz)
    {
        return insert(size(), sz);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <class TIterator>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(TIterator itFirst, TIterator itLast)
    {
        insert(cend(), itFirst, itLast);
        return *this;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::append(std::initializer_list<TCharType> ilist)
    {
        insert(cend(), ilist);
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator+=(
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        return insert(size(), rss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator+=(const std::basic_string<TCharType>& rss)
    {
        return insert(size(), rss);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator+=(TCharType c)
    {
        return insert(size(), 1, c);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator+=(const TCharType* sz)
    {
        return insert(size(), sz);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::operator+=(std::initializer_list<TCharType> ilist)
    {
        insert(cend(), ilist);
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(const string_base<TCharType, bUnicode, nFixedSize2>& rss) const noexcept
    {
        return compare(0, npos, rss, 0);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(const std::basic_string<TCharType>& rss) const noexcept
    {
        return compare(0, npos, rss, 0);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(size_t nPos1, size_t nCount1,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss) const
    {
        return compare(nPos1, nCount1, rss, 0);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(size_t nPos1, size_t nCount1,
        const std::basic_string<TCharType>& rss) const
    {
        return compare(nPos1, nCount1, rss, 0);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(size_t nPos1, size_t nCount1,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos2, size_t nCount2 /*= npos*/) const
    {
        // Deal with NULL pointer
        if (rss.empty())
            return empty() ? 0 : 1;

        // Determine the correct length
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nRightPos = std::min(static_cast<size_t>(size()), nPos2);
        size_t nRightSize = rss.size() > nRightPos ? rss.size() - nRightPos : 0;
        size_t nRightCount = std::min(nCount2, nRightSize);
        const TCharType* szRight = rss.data() + nRightPos;

        return compare(nPos1, nCount1, szRight, nRightCount);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(
        size_t nPos1, size_t nCount1, const std::basic_string<TCharType>& rss, size_t nPos2, size_t nCount2 /*= npos*/) const
    {
        // Deal with NULL pointer
        if (rss.empty())
            return empty() ? 0 : 1;

        // Determine the correct length
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nRightPos = std::min(size(), nPos2);
        size_t nRightSize = rss.size() > nRightPos ? rss.size() - nRightPos : 0;
        size_t nRightCount = std::min(nCount2, nRightSize);
        const TCharType* szRight = rss.data() + nRightPos;

        return compare(nPos1, nCount1, szRight, nRightCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(const TCharType* sz) const
    {
        return compare(0, std::basic_string<TCharType>::npos, sz, sz ? std::char_traits<TCharType>::length(sz) : 0);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(size_t nPos1, size_t nCount1, const TCharType* sz) const
    {
        return compare(nPos1, nCount1, sz, sz ? std::char_traits<TCharType>::length(sz) : 0);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline int string_base<TCharType, bUnicode, nFixedSize>::compare(size_t nPos1, size_t nCount1, const TCharType* sz, size_t nCount2) const
    {
        // Deal with NULL pointer
        if (empty()) return sz ? -1 : 0;

        // Determine the correct length
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nLeftPos = std::min(size(), nPos1);
        size_t nLeftSize = size() > nLeftPos ? size() - nLeftPos : 0;
        size_t nLeftCount = std::min(nCount1, nLeftSize);
        const TCharType* szLeft = data() + nLeftPos;

        // Compare
        int iResult = std::char_traits<TCharType>::compare(szLeft, sz, std::min(nLeftCount, nCount2));
        if (iResult != 0) return iResult;

        // If equal, the comparison is determined by the size
        if (nLeftCount < nCount2) return -1;
        if (nLeftCount > nCount2) return 1;
        return 0;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        return replace(nPos, nCount, rss.data(), rss.size());
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount,
        const std::basic_string<TCharType>& rss)
    {
        return replace(nPos, nCount, rss.data(), rss.size());
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst,
        const_iterator itLast, const string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return append(rss);
        if (empty()) return assign(rss);
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst) throw XInvalidIterator();
        return replace(nIndexFirst, nIndexLast - nIndexFirst, rss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, const std::basic_string<TCharType>& rss)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return append(rss);
        if (empty()) return assign(rss);
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst) throw XInvalidIterator();
        return replace(nIndexFirst, nIndexLast - nIndexFirst, rss);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount,
        const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos2, size_t nCount2 /*= npos*/)
    {
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nRightPos = std::min(nPos2, rss.size());
        size_t nRightSize = rss.size() > nRightPos ? rss.size() - nRightPos : 0;
        size_t nRightCount = nCount2 > nRightSize ? 0 : nCount2;
        return replace(nPos, nCount, rss.data() + nRightPos, nRightCount);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount, const std::basic_string<TCharType>& rss, size_t nPos2, size_t nCount2 /*= npos*/)
    {
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nRightPos = std::min(nPos2, rss.size());
        size_t nRightSize = rss.size() > nRightPos ? rss.size() - nRightPos : 0;
        size_t nRightCount = nCount2 > nRightSize ? 0 : nCount2;
        return replace(nPos, nCount, rss.data() + nRightPos, nRightCount);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount, const TCharType* sz, size_t nCount2)
    {
        if (!sz && nCount) throw XNullPointer();

        // Determine the correct length
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nLeftPos = std::min(size(), nPos);
        size_t nLeftSize = size() > nLeftPos ? size() - nLeftPos : 0;
        size_t nLeftCount = std::min(nCount, nLeftSize);
        TCharType* szLeft = m_ptrData.get() + nLeftPos;

        // Copy (part of) the replacement string over the existing string
        size_t nRep1Count = std::min(nLeftCount, nCount2);
        if (nRep1Count)
            std::copy_n(sz, nRep1Count, szLeft);

        // Is there anything left to copy; then insert this into the string.
        if (nRep1Count < nCount2)
            return insert(nLeftPos + nRep1Count, sz + nRep1Count, nCount2 - nRep1Count);

        // Are there characters too many to replace; erase them from this string.
        if (nRep1Count < nLeftCount)
            return erase(nLeftPos + nRep1Count, nLeftCount - nRep1Count);

        return *this;
    }
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, const TCharType* sz, size_t nCount2)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return append(sz, nCount2);
        if (empty()) return assign(sz, nCount2);
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst) throw XInvalidIterator();
        return replace(nIndexFirst, nIndexLast - nIndexFirst, sz, nCount2);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount, const TCharType* sz)
    {
        if (!sz) throw XNullPointer();
        return replace(nPos, nCount, sz, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, const TCharType* sz)
    {
        if (!sz) throw XNullPointer();
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return append(sz);
        if (empty()) return assign(sz);
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst) throw XInvalidIterator();
        return replace(nIndexFirst, nIndexLast - nIndexFirst, sz, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(size_t nPos, size_t nCount, size_t nCount2, TCharType c)
    {
        string_base<TCharType, bUnicode, nFixedSize> ss(nCount2, c);
        return replace(nPos, nCount, ss);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, size_t nCount2, TCharType c)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (itFirst == cend()) return append(nCount2, c);
        if (empty()) return assign(nCount2, c);
        size_t nIndexFirst = &itFirst[0] - data();
        size_t nIndexLast = itLast == cend() ? size() : &itLast[0] - data();
        if (nIndexLast < nIndexFirst)
            throw XInvalidIterator();
        return replace(nIndexFirst, nIndexLast - nIndexFirst, nCount2, c);
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <class TIterator>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, TIterator itFirst2, TIterator itLast2)
    {
        string_base<TCharType, bUnicode, nFixedSize> ss;
        for (TIterator itPos = itFirst2; itPos != itLast2; itPos++)
            ss += *itPos;
        return replace(itFirst, itLast, ss);
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize>& string_base<TCharType, bUnicode, nFixedSize>::replace(const_iterator itFirst, const_iterator itLast, std::initializer_list<TCharType> ilist)
    {
        string_base<TCharType, bUnicode, nFixedSize> ss(ilist);
        return replace(itFirst, itLast, ss);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string_base<TCharType, bUnicode, nFixedSize> string_base<TCharType, bUnicode, nFixedSize>::substr(size_t nPos /*= 0*/, size_t nCount /*= npos*/) const
    {
        string_base<TCharType, bUnicode, nFixedSize> ss(*this, nPos, nCount);
        return ss;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::copy(TCharType* szDest, size_t nCount, size_t nPos /*= 0*/) const
    {
        if (!szDest) throw XNullPointer();
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        size_t nLeftPos = std::min(nPos, size());
        size_t nLeftSize = size() > nLeftPos ? size() - nLeftPos : 0;
        size_t nLeftCount = std::min(nCount, nLeftSize);
        if (nLeftCount)
            std::copy_n(data() + nLeftPos, nLeftCount, szDest);
        return nLeftCount;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::resize(size_t nCount)
    {
        resize(nCount, static_cast<TCharType>('\0'));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline void string_base<TCharType, bUnicode, nFixedSize>::resize(size_t nCount, TCharType c)
    {
        if (!nCount)
            clear();
        else
        {
            size_t nCurrentLen = size();
            m_ptrData.resize(nCount + 1); // Allocate space for a terminating zero
            // False positive of CppCheck - condition is not always true. Suppress warning.
            // cppcheck-suppress knownConditionTrueFalse
            if (size() > nCurrentLen)
                std::fill_n(m_ptrData.get() + nCurrentLen, size() - nCurrentLen, c);
            if (size() > 0)
                m_ptrData[size()] = static_cast<TCharType>('\0');
        }
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline void string_base<TCharType, bUnicode, nFixedSize>::swap(string_base<TCharType, bUnicode, nFixedSize2>& rss)
    {
        string_base ssTemp = std::move(rss);
        rss = std::move(*this);
        assign(std::move(ssTemp));
    }
    /// @endcond

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find(const std::basic_string<TCharType>& rss, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz) throw XNullPointer();
        if (!nCount) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (nPos + nCount > size()) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        // NOTE: prevent -Werror=stringop-overflow= when subtracting size().
        for (size_t nIndex = nPos; (size() >= nCount) && (nIndex <= size() - nCount); nIndex++)
        {
            if (std::equal(sz, sz + nCount, cbegin() + nIndex))
                return nIndex;
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find(const TCharType* sz, size_t nPos /*= 0*/) const
    {
        if (!sz) throw XNullPointer();
        return find(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find(TCharType c, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::rfind(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return rfind(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::rfind(const std::basic_string<TCharType>& rss, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return rfind(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::rfind(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz) throw XNullPointer();
        if (!nCount) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (nCount > size()) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (empty()) return string_base<TCharType, bUnicode, nFixedSize>::npos;

        for (int64_t nIndex = static_cast<int64_t>(std::min(nPos, size() - nCount)); nIndex >= 0; nIndex--)
        {
            if (std::equal(sz, sz + nCount, cbegin() + nIndex))
                return static_cast<size_t>(nIndex);
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::rfind(const TCharType* sz, size_t nPos /*= npos*/) const
    {
        if (!sz)
            throw XNullPointer();
        return rfind(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::rfind(TCharType c, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return rfind(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_of(const std::basic_string<TCharType>& rss, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_of(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz) throw XNullPointer();
        if (!nCount) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (nPos >= size()) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        for (size_t nIndex = nPos; nIndex < size(); nIndex++)
        {
            for (size_t nIndex2 = 0; nIndex2 < nCount; nIndex2++)
            {
                if (at(nIndex) == sz[nIndex2])
                    return nIndex;
            }
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_of(const TCharType* sz, size_t nPos /*= 0*/) const
    {
        if (!sz)
            throw XNullPointer();
        return find_first_of(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_of(TCharType c, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_of(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_not_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_not_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_not_of(const std::basic_string<TCharType>& rss, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_not_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_not_of(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz)
            throw XNullPointer();
        if (!nCount)
            return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (nPos >= size())
            return string_base<TCharType, bUnicode, nFixedSize>::npos;
        for (size_t nIndex = nPos; nIndex < size(); nIndex++)
        {
            bool bFound = false;
            for (size_t nIndex2 = 0; !bFound && nIndex2 < nCount; nIndex2++)
            {
                if (at(nIndex) == sz[nIndex2])
                    bFound = true;
            }
            if (!bFound)
                return nIndex;
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_not_of(const TCharType* sz, size_t nPos /*= 0*/) const
    {
        if (!sz)
            throw XNullPointer();
        return find_first_not_of(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_first_not_of(TCharType c, size_t nPos /*= 0*/) const noexcept
    {
        try
        {
            return find_first_not_of(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_of(const std::basic_string<TCharType>& rss, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_of(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz) throw XNullPointer();
        if (!nCount) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (empty()) return string_base<TCharType, bUnicode, nFixedSize>::npos;

        for (int64_t nIndex = static_cast<int64_t>(std::min(nPos, size())) - 1; nIndex >= 0; nIndex--)
        {
            for (size_t nIndex2 = 0; nIndex2 < nCount; nIndex2++)
            {
                if (at(nIndex) == sz[nIndex2])
                    return static_cast<size_t>(nIndex);
            }
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_of(const TCharType* sz, size_t nPos /*= npos*/) const
    {
        if (!sz)
            throw XNullPointer();
        return find_last_of(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_of(TCharType c, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_of(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    /// @cond DOXYGEN_IGNORE
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_not_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss,
        size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_not_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }
    /// @endcond

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_not_of(const std::basic_string<TCharType>& rss, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_not_of(rss.data(), nPos, rss.size());
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_not_of(const TCharType* sz, size_t nPos, size_t nCount) const
    {
        if (!sz) throw XNullPointer();
        if (!nCount) return string_base<TCharType, bUnicode, nFixedSize>::npos;
        if (empty()) return string_base<TCharType, bUnicode, nFixedSize>::npos;

        for (int64_t nIndex = static_cast<int64_t>(std::min(nPos, size())) - 1; nIndex >= 0; nIndex--)
        {
            bool bFound = false;
            for (size_t nIndex2 = 0; !bFound && nIndex2 < nCount; nIndex2++)
            {
                if (at(nIndex) == sz[nIndex2])
                    bFound = true;
            }
            if (!bFound)
                return static_cast<size_t>(nIndex);
        }
        return npos;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_not_of(const TCharType* sz, size_t nPos /*= npos*/) const
    {
        if (!sz)
            throw XNullPointer();
        return find_last_not_of(sz, nPos, std::char_traits<TCharType>::length(sz));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline size_t string_base<TCharType, bUnicode, nFixedSize>::find_last_not_of(TCharType c, size_t nPos /*= npos*/) const noexcept
    {
        try
        {
            return find_last_not_of(&c, nPos, 1);
        }
        catch (const sdv::XSysExcept&)
        {
        }
        catch (const std::exception&)
        {
        }
        return npos;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(rssLeft);
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(rssLeft);
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(rssLeft);
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const TCharType* szRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(rssLeft);
        ss.append(szRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        TCharType cRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(rssLeft);
        ss.append(1, cRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(const TCharType* szLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(szLeft);
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(TCharType cLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(1, cLeft);
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(std::move(rssLeft));
        ss.append(rssRight);
        rssRight.clear();
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(std::move(rssLeft));
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const std::basic_string<TCharType>& rssRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(std::move(rssLeft));
        ss.append(rssRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const TCharType* szRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(std::move(rssLeft));
        ss.append(szRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft, TCharType cRight)
    {
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> ss(std::move(rssLeft));
        ss.append(1, cRight);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(std::move(rssRight));
        ss.insert(0, rssLeft);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(const std::basic_string<TCharType>& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(std::move(rssRight));
        ss.insert(0, rssLeft);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(const TCharType* szLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(std::move(rssRight));
        ss.insert(0, szLeft);
        return ss;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(TCharType cLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight)
    {
        string_base<TCharType, bUnicodeRight, nFixedSizeRight> ss(std::move(rssRight));
        ss.insert(0, 1, cLeft);
        return ss;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline void swap(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        rssLeft.swap(rssRight);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_ostream<TCharType, std::char_traits<TCharType>>& operator<<(
        std::basic_ostream<TCharType, std::char_traits<TCharType>>& rstream, const string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        rstream << static_cast<std::basic_string<TCharType>>(rss);
        return rstream;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_istream<TCharType, std::char_traits<TCharType>>& operator>>(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        std::basic_string<TCharType> ss;
        rstream >> ss;
        rss = ss;
        return rstream;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss,
        TCharType cDelim)
    {
        std::basic_string<TCharType> ss;
        std::getline(rstream, ss, cDelim);
        rss = ss;
        return rstream;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>&& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss,
        TCharType cDelim)
    {
        std::basic_string<TCharType> ss;
        // Access to moved variable is intended in this function. Suppress the implementation. 
        // cppcheck-suppress accessMoved
        std::getline(std::move(rstream), ss, cDelim);
        rss = ss;
        // cppcheck-suppress accessMoved
        return rstream;
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return getline(rstream, rss, static_cast<TCharType>('\n'));
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>&& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return getline(std::move(rstream), rss, static_cast<TCharType>('\n'));
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) == 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) == 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator==(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) == 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) != 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) != 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator!=(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) != 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) < 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) < 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) < 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) <= 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) <= 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<=(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) <= 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) > 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) > 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) > 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) >= 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) >= 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>=(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept
    {
        return rssLeft.compare(rssRight) >= 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) == 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator==(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) == 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) != 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator!=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) != 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) < 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) > 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) <= 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator<=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) >= 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) > 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) < 0;
    }

    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    inline bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight)
    {
        return rssLeft.compare(szRight) >= 0;
    }

    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    inline bool operator>=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight)
    {
        return rssRight.compare(szLeft) <= 0;
    }

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types. Specialized version for "char" data type.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline string MakeAnsiString(const char* sz, size_t nCount /*= string::npos*/, char /*cFill = '_'*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        string ss(sz, nCount == string::npos ? std::char_traits<char>::length(sz) : nCount);
        return ss;
    }

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types. Specialized version for "char16_t" data
     * type.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
     */
    template <>
    inline string MakeAnsiString(const char16_t* sz, size_t nCount /*= string::npos*/, char cFill /*= '_'*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char16_t>::length(sz) : nCount;

        string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x00ff
            if (sz[nIndex] < 0x0100)
            {
                ss += static_cast<char>(sz[nIndex]);
                continue;
            }

            // Range: 0x0100 <= c > 0xd800 - non-convertable Unicode character
            if (sz[nIndex] < 0xd800)
            {
                ss += cFill;
                continue;
            }

            // Range: 0xd800 <= c < 0xe000 - two-character encoding (for range c > U+10000)
            if (sz[nIndex] < 0xe000)
            {
                ss += cFill;
                nIndex++;
                continue;
            }

            // Range: 0xe000 <= c <= 0xffff - non-convertable Unicode character
            ss += cFill;
        }
        return ss;
    }

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types. Specialized version for "char32_t" data
     * type.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
     */
    template <>
    inline string MakeAnsiString(const char32_t* sz, size_t nCount /*= string::npos*/, char cFill /*= '_'*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char32_t>::length(sz) : nCount;

        string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x00ff
            if (sz[nIndex] < 0x00000100)
            {
                ss += static_cast<char>(sz[nIndex]);
                continue;
            }

            // Range: c > 0x00000100 - non-convertable Unicode character
            ss += cFill;
        }
        return ss;
    }

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types. Specialized version for "wchar_t" data
     * type.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
     */
    template <>
    inline string MakeAnsiString(const wchar_t* sz, size_t nCount /*= string::npos*/, char cFill /*= '_'*/)
    {
        if constexpr (sizeof(wchar_t) == 2)
            return MakeAnsiString(reinterpret_cast<const char16_t*>(sz), nCount, cFill);
        else
            return MakeAnsiString(reinterpret_cast<const char32_t*>(sz), nCount, cFill);
    }

    /**
     * @brief Create Utf-8 string by providing strings of other types. Specialized version for "char" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u8string MakeUtf8String(const char* sz, size_t nCount /*= string::npos*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        u8string ss(sz, nCount == string::npos ? std::char_traits<char>::length(sz) : nCount);
        return ss;
    }

    /**
     * @brief Create Utf-8 string by providing strings of other types. Specialized version for "char16_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u8string MakeUtf8String(const char16_t* sz, size_t nCount /*= string::npos*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char16_t>::length(sz) : nCount;

        // Convert a Utf-32 value in Utf-8 and add to string.
        u8string ss;
        auto fnAddAndConvertUtf32ToUtf8 = [&](char32_t c32) -> void
        {
            // Range: c < 0x80 - One byte encoding
            if (c32 < 0x80)
            {
                ss += static_cast<char>(c32 & 0x7f);
                return;
            }

            // Range: 0x80 <= c < 0x800 - Two bytes encoding
            if (c32 < 0x800)
            {
                ss += 0xc0 + static_cast<char>((c32 >> 6) & 0x1f);
                ss += 0x80 + static_cast<char>(c32 & 0x3f);
                return;
            }

            // Range: 0x800 <= c < 0x10000 - Three bytes encoding
            if (c32 < 0x10000)
            {
                ss += 0xe0 + static_cast<char>((c32 >> 12) & 0x0f);
                ss += 0x80 + static_cast<char>((c32 >> 6) & 0x3f);
                ss += 0x80 + static_cast<char>(c32 & 0x3f);
                return;
            }

            // Range: c >= 0x10000 - Four bytes encoding
            ss += 0xf0 + static_cast<char>((c32 >> 18) & 0x07);
            ss += 0x80 + static_cast<char>((c32 >> 12) & 0x3f);
            ss += 0x80 + static_cast<char>((c32 >> 6) & 0x3f);
            ss += 0x80 + static_cast<char>(c32 & 0x3f);
        };

        // Iterate through the string; since the coding of Utf-16 needs adding 0x10000, code to Utf-32 first.
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0xD800
            if (sz[nIndex] < 0xd800)
            {
                fnAddAndConvertUtf32ToUtf8(static_cast<char32_t>(sz[nIndex]));
                continue;
            }

            // Range: c < 0xE000 - two-character encoding (for range c > U+10000)
            // Hint: the value will be increased with 0x10000
            if (sz[nIndex] < 0xe000)
            {
                fnAddAndConvertUtf32ToUtf8(
                    ((static_cast<char32_t>(sz[nIndex] & 0x03ff) << 10) | static_cast<char32_t>(sz[nIndex + 1] & 0x03ff))
                    + 0x10000);
                nIndex++;
                continue;
            }

            // Range c > 0xE000
            fnAddAndConvertUtf32ToUtf8(static_cast<char32_t>(sz[nIndex]));
        }
        return ss;
    }

    /**
     * @brief Create Utf-8 string by providing strings of other types. Specialized version for "char32_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u8string MakeUtf8String(const char32_t* sz, size_t nCount /*= string::npos*/)
    {
        // Assume that the string is Utf-8.
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char32_t>::length(sz) : nCount;

        u8string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x80 - One byte encoding
            if (sz[nIndex] < 0x80)
            {
                ss += static_cast<char>(sz[nIndex] & 0x7f);
                continue;
            }

            // Range: 0x80 <= c < 0x800 - Two bytes encoding
            if (sz[nIndex] < 0x800)
            {
                ss += 0xc0 + static_cast<char>((sz[nIndex] >> 6) & 0x1f);
                ss += 0x80 + static_cast<char>(sz[nIndex] & 0x3f);
                continue;
            }

            // Range: 0x800 <= c < 0x10000 - Three bytes encoding
            if (sz[nIndex] < 0x10000)
            {
                ss += 0xe0 + static_cast<char>((sz[nIndex] >> 12) & 0x0f);
                ss += 0x80 + static_cast<char>((sz[nIndex] >> 6) & 0x3f);
                ss += 0x80 + static_cast<char>(sz[nIndex] & 0x3f);
                continue;
            }

            // Range: c >= 0x10000 - Four bytes encoding
            ss += 0xf0 + static_cast<char>((sz[nIndex] >> 18) & 0x07);
            ss += 0x80 + static_cast<char>((sz[nIndex] >> 12) & 0x3f);
            ss += 0x80 + static_cast<char>((sz[nIndex] >> 6) & 0x3f);
            ss += 0x80 + static_cast<char>(sz[nIndex] & 0x3f);
        }
        return ss;
    }

    /**
     * @brief Create Utf-8 string by providing strings of other types. Specialized version for "wchar_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u8string MakeUtf8String(const wchar_t* sz, size_t nCount /*= string::npos*/)
    {
        if constexpr (sizeof(wchar_t) == 2)
            return MakeUtf8String(reinterpret_cast<const char16_t*>(sz), nCount);
        else
            return MakeUtf8String(reinterpret_cast<const char32_t*>(sz), nCount);
    }

    /**
     * @brief Create Utf-16 string by providing strings of other types. Specialized version for "char" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u16string MakeUtf16String(const char* sz, size_t nCount /*= string::npos*/)
    {
        // Assume that the string is Utf-8.
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char>::length(sz) : nCount;

        u16string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x80
            if (static_cast<uint8_t>(sz[nIndex]) < 0x80)
            {
                ss += static_cast<char16_t>(sz[nIndex]);
                continue;
            }

            // Range: 0x80 <= c < 0xc0 - non-convertable Unicode character - could be ANSI
            if (static_cast<uint8_t>(sz[nIndex]) < 0xc0)
            {
                ss += static_cast<char16_t>(sz[nIndex]);
                continue;
            }

            // Range: 0xc0 <= c < 0xe0 - two-character encoding (for range U+0080 <= c < U+0800)
            if (static_cast<uint8_t>(sz[nIndex]) < 0xe0)
            {
                ss += (static_cast<char16_t>(sz[nIndex] & 0xbf) << 6) | (static_cast<char16_t>(sz[nIndex + 1] & 0x3f));
                nIndex++;
                continue;
            }

            // Range: 0xe0 <= c < 0xef - three-character encoding (for range U+0800 <= c < 0x10000)
            if (static_cast<uint8_t>(sz[nIndex]) <= 0xef)
            {
                ss += (static_cast<char16_t>(sz[nIndex] & 0x0f) << 12) | (static_cast<char16_t>(sz[nIndex + 1] & 0x3f) << 6)
                    | static_cast<char16_t>(sz[nIndex + 2] & 0x3f);
                nIndex += 2;
                continue;
            }

            // Range: c >= 0xf0 - four-character encoding (for range c > U+10000)
            // Encode in two Utf-16 characters - hint: the value will be subtracted with U+10000.
            ss += (0xd800 | (static_cast<char16_t>(sz[nIndex] & 0x03) << 8) | (static_cast<char16_t>(sz[nIndex + 1] & 0x3f) << 2)
                | (static_cast<char16_t>(sz[nIndex + 2] & 0x30) >> 4))
                - 0x40;
            nIndex += 2;
            ss += 0xdc00 | (static_cast<char16_t>(sz[nIndex] & 0x0f) << 6) | static_cast<char16_t>(sz[nIndex + 1] & 0x3f);
            nIndex++;
        }
        return ss;
    }

    /**
     * @brief Create Utf-16 string by providing strings of other types. Specialized version for "char16_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u16string MakeUtf16String(const char16_t* sz, size_t nCount /*= string::npos*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        u16string ss(sz, nCount == string::npos ? std::char_traits<char16_t>::length(sz) : nCount);
        return ss;
    }

    /**
     * @brief Create Utf-16 string by providing strings of other types. Specialized version for "char32_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u16string MakeUtf16String(const char32_t* sz, size_t nCount /*= string::npos*/)
    {
        // Assume that the string is Utf-8.
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char32_t>::length(sz) : nCount;

        u16string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x10000
            if (sz[nIndex] < 0x00010000)
            {
                ss += static_cast<char16_t>(sz[nIndex]);
                continue;
            }

            // Range: c >= 0x10000
            // Encode in two Utf-16 characters - hint: the value will be subtracted with 0x10000.
            ss += (0xd800 | static_cast<char16_t>(sz[nIndex] >> 10 & 0x03ff)) - 0x40;
            ss += 0xdc00 | static_cast<char16_t>(sz[nIndex] & 0x03ff);
        }
        return ss;
    }

    /**
     * @brief Create Utf-16 string by providing strings of other types. Specialized version for "wchar_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u16string MakeUtf16String(const wchar_t* sz, size_t nCount /*= string::npos*/)
    {
        if constexpr (sizeof(wchar_t) == 2)
            return MakeUtf16String(reinterpret_cast<const char16_t*>(sz), nCount);
        else
            return MakeUtf16String(reinterpret_cast<const char32_t*>(sz), nCount);
    }

    /**
     * @brief Create Utf-32 string by providing strings of other types. Specialized version for "char" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u32string MakeUtf32String(const char* sz, size_t nCount /*= string::npos*/)
    {
        // Assume that the string is Utf-8.
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char>::length(sz) : nCount;

        u32string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0x80
            if (static_cast<uint8_t>(sz[nIndex]) < 0x80)
            {
                ss += static_cast<char32_t>(sz[nIndex]);
                continue;
            }

            // Range: 0x80 <= c < 0xc0 - non-convertable Unicode character - could be ANSI
            if (static_cast<uint8_t>(sz[nIndex]) < 0xc0)
            {
                ss += static_cast<char32_t>(sz[nIndex]);
                continue;
            }

            // Range: 0xc0 <= c < 0xe0 - two-character encoding (for range U+0080 <= c < U+0800)
            if (static_cast<uint8_t>(sz[nIndex]) < 0xe0)
            {
                ss += (static_cast<char32_t>(sz[nIndex] & 0xbf) << 6) | (static_cast<char32_t>(sz[nIndex + 1] & 0x3f));
                nIndex++;
                continue;
            }

            // Range: 0xe0 <= c < 0xef - three-character encoding (for range U+0800 <= c < 0x10000)
            if (static_cast<uint8_t>(sz[nIndex]) <= 0xef)
            {
                ss += (static_cast<char32_t>(sz[nIndex] & 0x0f) << 12) | (static_cast<char32_t>(sz[nIndex + 1] & 0x3f) << 6)
                    | static_cast<char32_t>(sz[nIndex + 2] & 0x3f);
                nIndex += 2;
                continue;
            }

            // Range: c >= 0xf0 - four-character encoding (for range c > U+10000)
            ss += (static_cast<char32_t>(sz[nIndex] & 0x07) << 18) | (static_cast<char32_t>(sz[nIndex + 1] & 0x3f) << 12)
                | (static_cast<char32_t>(sz[nIndex + 2] & 0x3f) << 6) | static_cast<char32_t>(sz[nIndex + 3] & 0x3f);
            nIndex += 3;
        }
        return ss;
    }

    /**
     * @brief Create Utf-32 string by providing strings of other types. Specialized version for "char16_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u32string MakeUtf32String(const char16_t* sz, size_t nCount /*= string::npos*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        size_t nLen = nCount == string::npos ? std::char_traits<char16_t>::length(sz) : nCount;

        u32string ss;
        for (size_t nIndex = 0; nIndex < nLen; nIndex++)
        {
            // Range: c < 0xD800
            if (sz[nIndex] < 0xd800)
            {
                ss += static_cast<char32_t>(sz[nIndex]);
                continue;
            }

            // Range: c < 0xE000 - two-character encoding (for range c > U+10000)
            // Hint: the value will be increased with 0x10000
            if (sz[nIndex] < 0xe000)
            {
                ss +=
                    ((static_cast<char32_t>(sz[nIndex] & 0x03ff) << 10) | static_cast<char32_t>(sz[nIndex + 1] & 0x03ff)) + 0x10000;
                nIndex++;
                continue;
            }

            // Range c > 0xE000
            ss += static_cast<char32_t>(sz[nIndex]);
        }
        return ss;
    }

    /**
     * @brief Create Utf-32 string by providing strings of other types. Specialized version for "char32_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u32string MakeUtf32String(const char32_t* sz, size_t nCount /*= string::npos*/)
    {
        if (nCount && nCount != string::npos && !sz)
            throw XNullPointer();
        u32string ss(sz, nCount == string::npos ? std::char_traits<char32_t>::length(sz) : nCount);
        return ss;
    }

    /**
     * @brief Create Utf-32 string by providing strings of other types. Specialized version for "wchar_t" data type.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <>
    inline u32string MakeUtf32String(const wchar_t* sz, size_t nCount /*= string::npos*/)
    {
        if constexpr (sizeof(wchar_t) == 2)
            return MakeUtf32String(reinterpret_cast<const char16_t*>(sz), nCount);
        else
            return MakeUtf32String(reinterpret_cast<const char32_t*>(sz), nCount);
    }

    template <typename TCharType>
    inline wstring MakeWString(const TCharType* sz, size_t nCount /*= string::npos*/)
    {
        if constexpr (sizeof(wchar_t) == 2)
        {
            u16string ss16 = MakeUtf16String(sz, nCount);
            wstring   ss(reinterpret_cast<const wchar_t*>(ss16.data()), ss16.size());
            return ss;
        }
        else
        {
            u32string ss32 = MakeUtf32String(sz, nCount);
            wstring   ss(reinterpret_cast<const wchar_t*>(ss32.data()), ss32.size());
            return ss;
        }
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline string MakeAnsiString(const string_base<TCharType, bUnicode, nFixedSize>& rss, char cFill /*= '_'*/)
    {
        return MakeAnsiString(rss.data(), rss.size(), cFill);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline wstring MakeWString(const string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return MakeWString(rss.data(), rss.size());
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline u8string MakeUtf8String(const string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return MakeUtf8String(rss.data(), rss.size());
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline u16string MakeUtf16String(const string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return MakeUtf16String(rss.data(), rss.size());
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline u32string MakeUtf32String(const string_base<TCharType, bUnicode, nFixedSize>& rss)
    {
        return MakeUtf32String(rss.data(), rss.size());
    }

    template <typename TCharType>
    inline string MakeAnsiString(const std::basic_string<TCharType>& rss, char cFill /*= '_'*/)
    {
        return MakeAnsiString(rss.data(), rss.size(), cFill);
    }

    template <typename TCharType>
    inline wstring MakeWString(const std::basic_string<TCharType>& rss)
    {
        return MakeWString(rss.data(), rss.size());
    }

    template <typename TCharType>
    inline u8string MakeUtf8String(const std::basic_string<TCharType>& rss)
    {
        return MakeUtf8String(rss.data(), rss.size());
    }

    template <typename TCharType>
    inline u16string MakeUtf16String(const std::basic_string<TCharType>& rss)
    {
        return MakeUtf16String(rss.data(), rss.size());
    }

    template <typename TCharType>
    inline u32string MakeUtf32String(const std::basic_string<TCharType>& rss)
    {
        return MakeUtf32String(rss.data(), rss.size());
    }

    namespace internal
    {
        /**
         * @brief Helper struct to create the target string.
         * @tparam TCharType The character type to use for the string creation.
         * @tparam bUnicode When set, the string is a unicode string.
         * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
         */
        template <typename TCharTypeDst, bool bUnicode, size_t nFixedSizeDst>
        struct SMakeString;

        /**
        * @brief Secialization for making ANSI strings
        * @tparam nFixedSizeDst The fixed size of the string.
        */
        template <size_t nFixedSizeDst>
        struct SMakeString<char, false, nFixedSizeDst>
        {
            /**
             * @brief Make a SDV string
             * @tparam TCharTypeSrc The source character type
             * @tparam bUnicodeSrc When set, the source string is a unicode string.
             * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc>
            static string_base<char, false, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
            {
                return MakeAnsiString(rss);
            }
            /**
             * @brief Make a SDV string using a C++ string
             * @tparam TCharTypeSrc The source character type
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc>
            static string_base<char, false, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
            {
                return MakeAnsiString(rss);
            }
        };

        /**
        * @brief Secialization for making UTF-8 strings
        * @tparam nFixedSizeDst The fixed size of the string.
        */
        template <size_t nFixedSizeDst>
        struct SMakeString<char, true, nFixedSizeDst>
        {
            /**
             * @brief Make a SDV string
             * @tparam TCharTypeSrc The source character type
             * @tparam bUnicodeSrc When set, the source string is a unicode string.
             * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc>
            static string_base<char, true, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
            {
                return MakeUtf8String(rss);
            }
            /**
             * @brief Make a SDV string using a C++ string
             * @tparam TCharTypeSrc The source character type
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc>
            static string_base<char, true, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
            {
                return MakeUtf8String(rss);
            }
        };

        /**
         * @brief Secialization for making UTF-16 strings
         * @tparam nFixedSizeDst The fixed size of the string.
         */
        template <size_t nFixedSizeDst>
        struct SMakeString<char16_t, true, nFixedSizeDst>
        {
            /**
             * @brief Make a SDV string
             * @tparam TCharTypeSrc The source character type
             * @tparam bUnicodeSrc When set, the source string is a unicode string.
             * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc>
            static string_base<char16_t, true, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
            {
                return MakeUtf16String(rss);
            }
            /**
             * @brief Make a SDV string using a C++ string
             * @tparam TCharTypeSrc The source character type
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc>
            static string_base<char16_t, true, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
            {
                return MakeUtf16String(rss);
            }
        };
        /**
         * @brief Secialization for making UTF-32 strings
         * @tparam nFixedSizeDst The fixed size of the string.
         */
        template <size_t nFixedSizeDst>
        struct SMakeString<char32_t, true, nFixedSizeDst>
        {
            /**
             * @brief Make a SDV string
             * @tparam TCharTypeSrc The source character type
             * @tparam bUnicodeSrc When set, the source string is a unicode string.
             * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc>
            static string_base<char32_t, true, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
            {
                return MakeUtf32String(rss);
            }
            /**
             * @brief Make a SDV string using a C++ string
             * @tparam TCharTypeSrc The source character type
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc>
            static string_base<char32_t, true, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
            {
                return MakeUtf32String(rss);
            }
        };
        /**
         * @brief Secialization for making wide strings
         * @tparam nFixedSizeDst The fixed size of the string.
         */
        template <size_t nFixedSizeDst>
        struct SMakeString<wchar_t, true, nFixedSizeDst>
        {
            /**
             * @brief Make a SDV string
             * @tparam TCharTypeSrc The source character type
             * @tparam bUnicodeSrc When set, the source string is a unicode string.
             * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc>
            static string_base<wchar_t, true, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
            {
                return MakeWString(rss);
            }
            /**
             * @brief Make a SDV string using a C++ string
             * @tparam TCharTypeSrc The source character type
             * @param[in] rss Reference to the source string
             * @return The SDV string
             */
            template <typename TCharTypeSrc>
            static string_base<wchar_t, true, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
            {
                return MakeWString(rss);
            }
        };
    }

    template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc, typename TCharTypeDst, bool bUnicodeDst, size_t nFixedSizeDst>
    string_base<TCharTypeDst, bUnicodeDst, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss)
    {
        // Since partial specialization of template functions is not allowed, use a helper class to implement this function.
        return internal::SMakeString<TCharTypeDst, bUnicodeDst, nFixedSizeDst>::MakeString(rss);
    }

    template <typename TCharTypeSrc, typename TCharTypeDst, bool bUnicodeDst, size_t nFixedSizeDst>
    string_base<TCharTypeDst, bUnicodeDst, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss)
    {
        // Since partial specialization of template functions is not allowed, use a helper class to implement this function.
        return internal::SMakeString<TCharTypeDst, bUnicodeDst, nFixedSizeDst>::MakeString(rss);
    }

    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    inline std::filesystem::path MakePath(const string_base<TCharType, bUnicode, nFixedSize>& rssPath)
    {
        std::filesystem::path path(MakeWString(rssPath).c_str());
        return path;
    }

} // namespace sdv

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // !defined SDV_STRING_INL