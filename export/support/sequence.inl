#ifndef SDV_SEQUENCE_INL
#define SDV_SEQUENCE_INL

#ifndef SDV_SEQUENCE_H
#error Do not include "sequence.inl" directly. Include "sequence.h" instead!
#endif //!defined SDV_SEQUENCE_H

namespace sdv
{
    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence() noexcept
    {}

    template <class T, size_t nFixedSize>
    sequence<T, nFixedSize>::~sequence()
    {
        clear();
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(size_t nCount, const T& rtValue) : sequence()
    {
        insert(begin(), nCount, rtValue);
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(size_t nCount) : sequence()
    {
        T t{};
        insert(begin(), nCount, t);
    }

    template <class T, size_t nFixedSize>
    template <class TIterator>
    inline sequence<T, nFixedSize>::sequence(TIterator itFirst, TIterator itLast) : sequence()
    {
        insert(begin(), itFirst, itLast);
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(const sequence& rseq) : sequence()
    {
        if (!rseq.empty())
        {
            resize(rseq.size());
            if (m_ptrData)
                std::copy_n(rseq.data(), std::min(rseq.size(), size()), m_ptrData.get());
        }
    }

    /// @cond DOXYGEN_IGNORE
    template <class T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline sequence<T, nFixedSize>::sequence(const sequence<T, nFixedSize2>& rseq) : sequence()
    {
        if (!rseq.empty())
        {
            resize(rseq.size());
            if (m_ptrData)
                std::copy_n(rseq.data(), std::min(rseq.size(), size()), m_ptrData.get());
        }
    }
	/// @endcond

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(sequence&& rseq) noexcept : sequence()
    {
        m_ptrData = std::move(rseq.m_ptrData);
    }

    /// @cond DOXYGEN_IGNORE
    template <class T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline sequence<T, nFixedSize>::sequence(sequence<T, nFixedSize2>&& rseq) noexcept : sequence()
    {
        m_ptrData = std::move(rseq.buffer());
    }
	/// @endcond

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(std::vector<T>& rvec) : sequence()
    {
        if (!rvec.empty())
        {
            resize(rvec.size());
            if (m_ptrData)
                std::copy_n(rvec.data(), std::min(rvec.size(), size()), m_ptrData.get());
        }
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::sequence(std::initializer_list<T> ilist) : sequence()
    {
        insert(begin(), ilist);
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(const sequence& rseq)
    {
        clear();
        if (!rseq.empty())
        {
            resize(rseq.size());
            if (m_ptrData)
                std::copy_n(rseq.data(), std::min(rseq.size(), size()), m_ptrData.get());
        }
        return *this;
    }

    /// @cond DOXYGEN_IGNORE
    template <class T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(const sequence<T, nFixedSize2>& rseq)
    {
        clear();
        if (!rseq.empty())
        {
            resize(rseq.size());
            if (m_ptrData)
                std::copy_n(rseq.data(), std::min(rseq.size(), size()), m_ptrData.get());
        }
        return *this;
    }
	/// @endcond

    /// @cond DOXYGEN_IGNORE
    template <class T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(sequence<T, nFixedSize2>&& rseq) noexcept
    {
        clear();
        m_ptrData = std::move(rseq.buffer());
        return *this;
    }
	/// @endcond

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(sequence&& rseq) noexcept
    {
        clear();
        m_ptrData = std::move(rseq.m_ptrData);
        return *this;
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(const std::vector<T>& rvec)
    {
        clear();
        if (!rvec.empty())
        {
            resize(rvec.size());
            if (m_ptrData)
                std::copy_n(rvec.data(), std::min(rvec.size(), size()), m_ptrData.get());
        }
        return *this;
    }

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>& sequence<T, nFixedSize>::operator=(std::initializer_list<T> ilist)
    {
        clear();
        insert(begin(), ilist);
        return *this;
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::assign(size_t nCount, const T& rtValue)
    {
        clear();
        insert(begin(), nCount, rtValue);
    }

    template <class T, size_t nFixedSize>
    template <class TIterator>
    inline void sequence<T, nFixedSize>::assign(TIterator itFirst, TIterator itLast)
    {
        clear();
        insert(begin(), itFirst, itLast);
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::assign(std::initializer_list<T> ilist)
    {
        clear();
        insert(begin(), ilist);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::reference sequence<T, nFixedSize>::at(size_t nPos)
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

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reference sequence<T, nFixedSize>::at(size_t nPos) const
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

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::reference sequence<T, nFixedSize>::operator[](size_t nPos)
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

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reference sequence<T, nFixedSize>::operator[](size_t nPos) const
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

    template <class T, size_t nFixedSize>
    inline T& sequence<T, nFixedSize>::front()
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

    template <class T, size_t nFixedSize>
    inline const T& sequence<T, nFixedSize>::front() const
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

    template <class T, size_t nFixedSize>
    inline T& sequence<T, nFixedSize>::back()
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

    template <class T, size_t nFixedSize>
    inline const T& sequence<T, nFixedSize>::back() const
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

    template <class T, size_t nFixedSize>
    inline sequence<T, nFixedSize>::operator std::vector<T>() const
    {
        std::vector<T> vec;
        if (size())
        {
            try
            {
                vec.resize(size());
            }
            catch (const std::bad_alloc&)
            {
                core::XAllocFailed exception;
                exception.uiSize = static_cast<uint32_t>(size() * sizeof(T));
                throw exception;
            }
            std::copy_n(data(), std::min(size(), vec.size()), &vec.front());
        }
        return vec;
    }

    template <class T, size_t nFixedSize>
    inline const T* sequence<T, nFixedSize>::data() const noexcept
    {
        return m_ptrData.get();
    }

    template <class T, size_t nFixedSize>
    inline pointer<T, nFixedSize>& sequence<T, nFixedSize>::buffer() noexcept
    {
        return m_ptrData;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::begin() noexcept
    {
        return iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_iterator sequence<T, nFixedSize>::begin() const noexcept
    {
        return iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_iterator sequence<T, nFixedSize>::cbegin() const noexcept
    {
        return const_iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::reverse_iterator sequence<T, nFixedSize>::rbegin() noexcept
    {
        return reverse_iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reverse_iterator sequence<T, nFixedSize>::rbegin() const noexcept
    {
        return reverse_iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reverse_iterator sequence<T, nFixedSize>::crbegin() const noexcept
    {
        return const_reverse_iterator(this);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::end() noexcept
    {
        iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_iterator sequence<T, nFixedSize>::end() const noexcept
    {
        iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_iterator sequence<T, nFixedSize>::cend() const noexcept
    {
        const_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::reverse_iterator sequence<T, nFixedSize>::rend() noexcept
    {
        reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reverse_iterator sequence<T, nFixedSize>::rend() const noexcept
    {
        reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::const_reverse_iterator sequence<T, nFixedSize>::crend() const noexcept
    {
        const_reverse_iterator it(this);
        it += std::numeric_limits<size_t>::max();
        return it;
    }

    template <class T, size_t nFixedSize>
    inline bool sequence<T, nFixedSize>::empty() const
    {
        return !size();
    }

    template <class T, size_t nFixedSize>
    inline size_t sequence<T, nFixedSize>::size() const
    {
        return m_ptrData.size();
    }

    template <class T, size_t nFixedSize>
    inline size_t sequence<T, nFixedSize>::length() const
    {
        return m_ptrData.size();
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::reserve(size_t nNewCap /*= 0*/)
    {
        if (nNewCap > size())
            resize(nNewCap);
    }

    template <class T, size_t nFixedSize>
    inline size_t sequence<T, nFixedSize>::capacity() const noexcept
    {
        return m_ptrData.capacity();
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::shrink_to_fit()
    {
        // Do nothing...
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::clear()
    {
        m_ptrData.reset();
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::insert(const_iterator itPos, const T& rtValue)
    {
        return insert(itPos, static_cast<size_t>(1), rtValue);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::insert(const_iterator itPos, T&& rtValue)
    {
        T t{};
        insert(itPos, static_cast<size_t>(1), t);
        iterator it = itPos;
        *it = std::move(rtValue);
        return it;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::insert(const_iterator itPos, size_t nCount, const T& rtValue)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();

        // Determine location
        size_t nIndex = 0;
        if (itPos == cend())
            nIndex = size();
        else if (!empty())
            nIndex = &itPos[0] - data();

        // Calculate count
        size_t nCountTemp = nCount;
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;
        if (!nCountTemp) return iterator(itPos);;

        // Allocate space for a new sequence.
        m_ptrData.resize(nCurrentLen + nCountTemp);

        // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Insert the values
        if (nCountTemp) std::fill_n(m_ptrData.get() + std::min(nIndex, nCurrentLen), nCountTemp, rtValue);

        return iterator(itPos);
    }

    template <class T, size_t nFixedSize>
    template <class TIterator>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::insert(const_iterator itPos, TIterator itFirst, TIterator itLast)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();

        // Count the amount of entries to insert
        size_t nCount = 0;
        for (TIterator itIndex = itFirst; itIndex != itLast; itIndex++)
            nCount++;

        // Determine location
        size_t nIndex = 0;
        if (itPos == cend())
            nIndex = size();
        else if (!empty())
            nIndex = &itPos[0] - data();

        // Calculate count
        size_t nCountTemp = nCount;
        size_t nCurrentLen = size();
        if (nFixedSize && nCurrentLen + nCountTemp > nFixedSize)
            nCountTemp = nFixedSize - nCurrentLen;
        if (!nCountTemp) return iterator(itPos);;

        // Allocate space for a new sequence.
        m_ptrData.resize(nCurrentLen + nCountTemp);

        // Copy the part following the index.
        if (nIndex < nCurrentLen)
            std::copy_backward(data() + nIndex, data() + nCurrentLen, m_ptrData.get() + nCurrentLen + nCountTemp);

        // Copy the data
        for (TIterator itIndex = itFirst; itIndex != itLast; itIndex++)
            at(nIndex++) = *itIndex;

        return iterator(itPos);
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::insert(const_iterator itPos, std::initializer_list<T> ilist)
    {
        if (!itPos.is_valid(*this)) throw XInvalidIterator();
        return insert(itPos, ilist.begin(), ilist.end());
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::erase(iterator itPos)
    {
        erase(itPos, itPos + 1);
        return itPos;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::erase(const_iterator itPos)
    {
        erase(itPos, itPos + 1);
        return itPos;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::erase(iterator itFirst, iterator itLast)
    {
        erase(const_iterator(itFirst), const_iterator(itLast));
        return itFirst;
    }

    template <class T, size_t nFixedSize>
    inline typename sequence<T, nFixedSize>::iterator sequence<T, nFixedSize>::erase(const_iterator itFirst, const_iterator itLast)
    {
        if (!itFirst.is_valid(*this)) throw XInvalidIterator();
        if (!itLast.is_valid(*this)) throw XInvalidIterator();
        if (empty()) return cbegin(); // Nothing to do
        if (itFirst == cend()) return cbegin(); // Nothing to do
        if (itFirst == itLast) return itFirst; // Nothing to do

        // Determine location
        size_t nIndex = &itFirst[0] - data();
        size_t nCount = itLast == cend() ? (size() - nIndex) : (&itLast[0] - &itFirst[0]);

        // Copy any leftover characters
        if (nCount < size() && nIndex + nCount < size())
        {
            std::copy_n(m_ptrData.get() + nIndex + nCount, size() - nIndex - nCount, m_ptrData.get() + nIndex);
            resize(size() - nCount);
        }
        else
            resize(nIndex);

        return itFirst;
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::push_back(const T& rtValue)
    {
        insert(end(), rtValue);
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::push_back(T&& rtValue)
    {
        insert(end(), std::move(rtValue));
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::pop_back()
    {
        if (empty()) return; // Nothing to do
        erase(cend() - 1);
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::resize(size_t nCount)
    {
        T t{};
        resize(nCount, t);
    }

    template <class T, size_t nFixedSize>
    inline void sequence<T, nFixedSize>::resize(size_t nCount, const value_type& rtValue)
    {
        if (!nCount)
            clear();
        else
        {
            size_t nCurrentLen = size();
            m_ptrData.resize(nCount);
            // False positive of CppCheck - condition is not always true. Suppress warning.
            // cppcheck-suppress knownConditionTrueFalse
            if (size() > nCurrentLen)
                std::fill_n(m_ptrData.get() + nCurrentLen, size() - nCurrentLen, rtValue);
        }
    }

    /// @cond DOXYGEN_IGNORE
    template <class T, size_t nFixedSize>
    template <size_t nFixedSize2>
    inline void sequence<T, nFixedSize>::swap(sequence<T, nFixedSize2>& rseq) noexcept
    {
        sequence seqTemp = std::move(rseq);
        rseq = std::move(*this);
        operator=(std::move(seqTemp));
    }
	/// @endcond

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline void swap(sequence<T, nFixedSizeLeft>& rseqLeft, sequence<T, nFixedSizeRight>& rseqRight) noexcept
    {
        rseqLeft.swap(rseqRight);
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator==(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        if (rseqLeft.size() != rseqRight.size()) return false;
        for (size_t nIndex = 0; nIndex < rseqLeft.size(); nIndex++)
            if (rseqLeft[nIndex] != rseqRight[nIndex]) return false;
        return true;
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator!=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        return !operator==(rseqLeft, rseqRight);
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator<(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        for (size_t nIndex = 0; nIndex < std::min(rseqLeft.size(), rseqRight.size()); nIndex++)
        {
            if (rseqLeft[nIndex] < rseqRight[nIndex]) return true;
            if (rseqLeft[nIndex] > rseqRight[nIndex]) return false;
        }
        return rseqLeft.size() < rseqRight.size();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator<=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        for (size_t nIndex = 0; nIndex < std::min(rseqLeft.size(), rseqRight.size()); nIndex++)
        {
            if (rseqLeft[nIndex] < rseqRight[nIndex]) return true;
            if (rseqLeft[nIndex] > rseqRight[nIndex]) return false;
        }
        return rseqLeft.size() <= rseqRight.size();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator>(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        for (size_t nIndex = 0; nIndex < std::min(rseqLeft.size(), rseqRight.size()); nIndex++)
        {
            if (rseqLeft[nIndex] < rseqRight[nIndex]) return false;
            if (rseqLeft[nIndex] > rseqRight[nIndex]) return true;
        }
        return rseqLeft.size() > rseqRight.size();
    }

    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    inline bool operator>=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight)
    {
        for (size_t nIndex = 0; nIndex < std::min(rseqLeft.size(), rseqRight.size()); nIndex++)
        {
            if (rseqLeft[nIndex] < rseqRight[nIndex]) return false;
            if (rseqLeft[nIndex] > rseqRight[nIndex]) return true;
        }
        return rseqLeft.size() >= rseqRight.size();
    }

} // namespace sdv


#endif // !defined SDV_SEQUENCE_INL