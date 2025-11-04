#ifndef SDV_SEQUENCE_H
#define SDV_SEQUENCE_H

#include <vector>
#include <algorithm>
#include <limits>
#include "iterator.h"
#include "pointer.h"

namespace sdv
{
    /**
     * @brief Managed sequence class.
     * @details Sequence management class. A sequence provides a dynamic vector based on a buffer implementation of the ptr-class.
     * There are two versions of buffer management, fixed buffer management (nFixedSize template parameter larger than 0) and
     * dynamic buffer management (nFixedSize template parameter is 0). The functions of this class are similar to the functions of
     * the std::vector class.
     * @tparam T Type to use for the buffer allocation.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     */

    template <class T, size_t nFixedSize = 0>
    class sequence
    {
    public:
		/**
         * @brief Value type for this sequence class.
         */
        using value_type = T;

        /**
         * @brief Forward iterator class used by this sequence class.
         */
        using iterator = internal::index_iterator<sequence<T, nFixedSize>, false, false>;

        /**
         * @brief Backward iterator class used by this sequence class.
         */
        using reverse_iterator = internal::index_iterator<sequence<T, nFixedSize>, false, true>;

        /**
         * @brief Const forward iterator class used by this sequence class.
         */
        using const_iterator = internal::index_iterator<sequence<T, nFixedSize>, true, false>;

        /**
         * @brief Const backward iterator class used by this sequence class.
         */
        using const_reverse_iterator = internal::index_iterator<sequence<T, nFixedSize>, true, true>;

        /**
         * @brief Reference type of the element.
         */
        using reference = T&;

        /**
         * @brief Const reference type of the element.
         */
        using const_reference = const T&;

        /**
         * @brief Default constructor
         */
        sequence() noexcept;

        /**
         * @brief Destructor
         */
        ~sequence();

        /**
         * @brief Construct a sequence with a certain amount of values.
         * @param[in] nCount The amount of values to create.
         * @param[in] rtValue Reference to the value to use for the initialization.
         */
        sequence(size_t nCount, const T& rtValue);

        /**
         * @brief Construct a sequence with a certain amount of values.
         * @param[in] nCount The amount of values to create.
         */
        explicit sequence(size_t nCount);

        /**
         * @brief Construct a sequence from a range of values accessible through iterators.
         * @tparam TIterator Type of iterator.
         * @param[in] itFirst The iterator pointing to the itFirst value.
         * @param[in] itLast The iterator pointing to the value past the itLast value.
         */
        template <class TIterator>
        sequence(TIterator itFirst, TIterator itLast);

        /**
         * @brief Copy constructor of same sequence type.
         * @param[in] rseq Reference to the sequence containing the values to copy.
         */
        sequence(const sequence& rseq);

        /**
         * @brief Copy constructor of other sequence types.
         * @tparam nFixedSize2 The fixed size of the provided sequence.
         * @param[in] rseq Reference to the sequence containing the values to copy.
         */
        template <size_t nFixedSize2>
        sequence(const sequence<T, nFixedSize2>& rseq);

        /**
         * @brief Move constructor of same sequence type.
         * @param[in] rseq Reference to the sequence to move the values from.
         */
        sequence(sequence&& rseq) noexcept;

        /**
         * @brief Move constructor of other sequence types.
         * @tparam nFixedSize2 The fixed size of the provided sequence.
         * @param[in] rseq Reference to the sequence to move the values from.
         */
        template <size_t nFixedSize2>
        sequence(sequence<T, nFixedSize2>&& rseq) noexcept;

        /**
         * @brief Construct a sequence from a vector.
         * @param[in] rvec Reference to the sequence.
         */
        sequence(std::vector<T>& rvec);

        /**
         * @brief Construct a sequence from an initializer list.
         * @param[in] ilist Initializer list.
         */
        sequence(std::initializer_list<T> ilist);

        /**
         * @brief Assignment operator of same sequence type.
         * @param[in] rseq Reference to the sequence.
         * @return Reference to this sequence.
         */
        sequence& operator=(const sequence& rseq);

        /**
         * @brief Assignment operator of other sequence types.
         * @tparam nFixedSize2 The fixed size of the provided sequence.
         * @param[in] rseq Reference to the sequence.
         * @return Reference to this sequence.
         */
        template <size_t nFixedSize2>
        sequence& operator=(const sequence<T, nFixedSize2>& rseq);

        /**
         * @brief Move operator of same sequence type.
         * @param[in] rseq Reference to the sequence.
         * @return Reference to this sequence.
         */
        sequence& operator=(sequence&& rseq) noexcept;

        /**
         * @brief Move operator of other sequence types.
         * @tparam nFixedSize2 The fixed size of the provided sequence.
         * @param[in] rseq Reference to the sequence.
         * @return Reference to this sequence.
         */
        template <size_t nFixedSize2>
        sequence& operator=(sequence<T, nFixedSize2>&& rseq) noexcept;

        /**
         * @brief Assignment operator for vector.
         * @param[in] rvec Reference to the vector.
         * @return Reference to this sequence.
         */
        sequence& operator=(const std::vector<T>& rvec);

        /**
         * @brief Assignment operator for initializer list.
         * @param[in] ilist Initializer list.
         * @return Reference to this sequence.
         */
        sequence& operator=(std::initializer_list<T> ilist);

        /**
         * @brief Assign an amount of values to the sequence replacing the values existing before.
         * @param[in] nCount Amount of values to assign.
         * @param[in] rtValue Reference to the value to assign.
        */
        void assign(size_t nCount, const T& rtValue);

        /**
         * @brief Assign an amount of values pointed to by itFirst and itLast iterators replacing the values existing before.
         * @tparam TIterator Type of iterator.
         * @param[in] itFirst The iterator pointing to the itFirst value.
         * @param[in] itLast The iterator pointing to the value past the itLast value.
         */
        template <class TIterator>
        void assign(TIterator itFirst, TIterator itLast);

        /**
         * @brief Assign the values from an initializer list.
         * @param[in] ilist Initializer list.
         */
        void assign(std::initializer_list<T> ilist);

        /**
         * @brief Return a reference to the value at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the value.
         */
        reference at(size_t nPos);

        /**
         * @brief Return a reference to the value at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the value.
         */
        const_reference at(size_t nPos) const;

        /**
         * @brief Return a reference to the value at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the value.
         */
        reference operator[](size_t nPos);

        /**
         * @brief Return a reference to the value at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the value.
         */
        const_reference operator[](size_t nPos) const;

        /**
         * @brief Return a reference to the itFirst value.
         * @return Reference to the value.
         */
        T& front();

        /**
         * @brief Return a reference to the itFirst value.
         * @return Reference to the value.
         */
        const T& front() const;

        /**
         * @brief Return a reference to the itLast value.
         * @return Reference to the value.
         */
        T& back();

        /**
         * @brief Return a reference to the itLast value.
         * @return Reference to the value.
         */
        const T& back() const;

        /**
         * @brief Cast operator for C++ vector
         * @return Returns a C++ vector object containing a copy of the sequence.
         */
        operator std::vector<T>() const;

        /**
         * @brief Access to the underlying data.
         * @return Pointer to the data.
         */
        const T* data() const noexcept;

        /**
         * @brief Access to the buffer.
         * @return Return a reference to the internal buffer.
         */
        pointer<T, nFixedSize>& buffer() noexcept;

        /**
         * @brief Return an iterator to the itFirst value of the sequence.
         * @return Iterator to the itFirst value of te sequence.
         */
        iterator begin() noexcept;

        /**
         * @brief Return an iterator to the itFirst value of the sequence.
         * @return Iterator to the itFirst value of te sequence.
         */
        const_iterator begin() const noexcept;

        /**
         * @brief Return a const iterator to the itFirst value of the sequence.
         * @return Const iterator to the itFirst value of te sequence.
         */
        const_iterator cbegin() const noexcept;

        /**
         * @brief Return a reverse-iterator to the itLast value of the sequence.
         * @return Reverse iterator to the itLast value of te sequence.
         */
        reverse_iterator rbegin() noexcept;

        /**
         * @brief Return a reverse-iterator to the itLast value of the sequence.
         * @return Reverse iterator to the itLast value of te sequence.
         */
        const_reverse_iterator rbegin() const noexcept;

        /**
         * @brief Return a const reverse iterator to the itLast value of the sequence.
         * @return Const reverse iterator to the itLast value of te sequence.
         */
        const_reverse_iterator crbegin() const noexcept;

        /**
         * @brief Return an iterator beyond the itLast value of the sequence.
         * @return Iterator beyond the itLast value of te sequence.
         */
        iterator end() noexcept;

        /**
         * @brief Return an iterator beyond the itLast value of the sequence.
         * @return Iterator beyond the itLast value of te sequence.
         */
        const_iterator end() const noexcept;

        /**
         * @brief Return a const beyond the itLast value of the sequence.
         * @return Const iterator beyond the itLast value of te sequence.
         */
        const_iterator cend() const noexcept;

        /**
         * @brief Return a reverse iterator before the itFirst value of the sequence.
         * @return Reverse iterator before the itLast value of te sequence.
         */
        reverse_iterator rend() noexcept;

        /**
         * @brief Return a reverse iterator before the itFirst value of the sequence.
         * @return Reverse iterator before the itLast value of te sequence.
         */
        const_reverse_iterator rend() const noexcept;

        /**
         * @brief Return a const reverse iterator before the itFirst value of the sequence.
         * @return Const reverse iterator before the itLast value of te sequence.
         */
        const_reverse_iterator crend() const noexcept;

        /**
         * @brief Is the sequence empty?
         * @return Returns 'true' when the sequence is empty; 'false' when not.
         */
        bool empty() const;

        /**
         * @brief Get the size of the sequence.
         * @remarks The length and the size for the sequence are equal.
         * @return The size of the sequence buffer.
         */
        size_t size() const;

        /**
         * @brief Get the length of the sequence.
         * @remarks The length and the size for the sequence are equal.
         * @return The length of the sequence.
         */
        size_t length() const;

        /**
         * @brief Reserve capacity for the sequence buffer. Additional buffer will be filled with zeros.
         * @remarks Reducing the capacity will have no effect.
         * @param[in] nNewCap The new capacity.
         */
        void reserve(size_t nNewCap = 0);

        /**
         * @brief Get the current sequence capacity.
         * @remarks This will be the same as the length and size of the sequence.
         * @return The capacity of the current sequence.
         */
        size_t capacity() const noexcept;

        /**
         * @brief Reduce the buffer to fit the sequence.
         * @remarks This function will have no effect.
         */
        void shrink_to_fit();

        /**
         * @brief Clear the sequence.
         */
        void clear();

        /**
         * @brief Insert a value rtValue at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the value.
         * @param[in] rtValue Reference to the value to insert.
         * @return The iterator pointing to the inserted value.
         */
        iterator insert(const_iterator itPos, const T& rtValue);

        /**
         * @brief Insert a value rtValue at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the value.
         * @param[in] rtValue Reference to the value to insert.
         * @return The iterator pointing to the inserted value.
         */
        iterator insert(const_iterator itPos, T&& rtValue);

        /**
         * @brief Insert an amount of values rtValue at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the value.
         * @param[in] nCount The amount of values to insert.
         * @param[in] rtValue Reference to the value to insert.
         * @return The iterator pointing to the itFirst inserted value.
         */
        iterator insert(const_iterator itPos, size_t nCount, const T& rtValue);

        /**
         * @brief Insert a range of vvalues identified by the supplied iterators.
         * @tparam TIterator Iterator type to use.
         * @param[in] itPos Iterator pointing to the position to insert the value.
         * @param[in] itFirst The iterator pointing to the itFirst value.
         * @param[in] itLast The iterator pointing to the value past the itLast value.
         * @return The iterator pointing to the itFirst inserted value.
         */
        template <class TIterator>
        iterator insert(const_iterator itPos, TIterator itFirst, TIterator itLast);

        /**
         * @brief Insert an initializer list at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the value.
         * @param[in] ilist Initializer list.
         * @return The iterator pointing to the itFirst inserted value.
         */
        iterator insert(const_iterator itPos, std::initializer_list<T> ilist);

        /**
         * @brief Remove a value at the provided position.
         * @remarks The iterator must point to this sequence.
         * @param[in] itPos Iterator pointing to the position to erase the value from.
         * @return Iterator to the value following the erase value or end() when no more values are available.
         */
        iterator erase(iterator itPos);

        /**
         * @brief Remove a value at the provided position.
         * @remarks The iterator must point to this sequence.
         * @param[in] itPos Iterator pointing to the position to erase the value from.
         * @return Iterator to the value following the erase value or end() when no more values are available.
         */
        iterator erase(const_iterator itPos);

        /**
         * @brief Remove al values starting at itFirst until but not including itLast.
         * @remarks Both iterators must point to this sequence.
         * @param[in] itFirst The iterator pointing to the itFirst value.
         * @param[in] itLast The iterator pointing to the value past the itLast value.
         * @return Iterator to the value following the erased values or end() when no more values are available.
         */
        iterator erase(iterator itFirst, iterator itLast);

        /**
         * @brief Remove al values starting at itFirst until but not including itLast.
         * @remarks Both iterators must point to this sequence.
         * @param[in] itFirst The iterator pointing to the itFirst value.
         * @param[in] itLast The iterator pointing to the value past the itLast value.
         * @return Iterator to the value following the erased values or end() when no more values are available.
         */
        iterator erase(const_iterator itFirst, const_iterator itLast);

        /**
         * @brief Appends the given value to the end of the sequence.
         * @param[in] rtValue Reference to the value to append.
         */
        void push_back(const T& rtValue);

        /**
         * @brief Appends the given value to the end of the sequence.
         * @param[in] rtValue Reference to the value to append.
         */
        void push_back(T&& rtValue);

        /**
         * @brief Removes the itLast value from the sequence.
         */
        void pop_back();

        /**
         * @brief Set the new size of the sequence.
         * @param[in] nCount The size of the sequence buffer.
         */
        void resize(size_t nCount);

        /**
         * @brief Set the new size of the sequence. Additional values will be filled with rtValue.
         * @param[in] nCount The size of the sequence buffer.
         * @param[in] rtValue Reference to the value to use for initialization.
         */
        void resize(size_t nCount, const value_type& rtValue);

        /**
         * @brief Exchange the content of provided sequence with this sequence.
         * @tparam nFixedSize2 The fixed size of the provided sequence.
         * @param[in] rseq Reference to the sequence to swap with.
         */
        template <size_t nFixedSize2>
        void swap(sequence<T, nFixedSize2>& rseq) noexcept;

    private:
        pointer<T, nFixedSize>      m_ptrData;        ///< Smart pointer to the data.
    };

    /**
     * @brief Swap two sequences.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the first sequence.
     * @param[in] rseqRight Reference to the second sequence.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    void swap(sequence<T, nFixedSizeLeft>& rseqLeft, sequence<T, nFixedSizeRight>& rseqRight) noexcept;

    /**
     * @brief Compare whether the content of both sequences is identical.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the first sequence.
     * @param[in] rseqRight Reference to the second sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator==(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);

    /**
     * @brief Compare whether the content of both sequences is not identical.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the first sequence.
     * @param[in] rseqRight Reference to the second sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator!=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);

    /**
     * @brief Compare whether the left sequence has a lower content than the right sequence or if equal, has a smaller size.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the left sequence.
     * @param[in] rseqRight Reference to the right sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator<(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);

    /**
     * @brief Compare whether the left sequence has a lower or equal content than the right sequence or if equal, has a smaller
     * or equal size.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the left sequence.
     * @param[in] rseqRight Reference to the right sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator<=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);

    /**
     * @brief Compare whether the left sequence has a higher content than the right sequence or if equal, has a larger size.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the left sequence.
     * @param[in] rseqRight Reference to the right sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator>(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);

    /**
     * @brief Compare whether the left sequence has a higher or equal content than the right sequence or if equal, has a larger
     * or equal size.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left sequence.
     * @tparam nFixedSizeRight The fixed size of the right sequence.
     * @param[in] rseqLeft Reference to the left sequence.
     * @param[in] rseqRight Reference to the right sequence.
     * @return Returns whether the comparison was successful.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator>=(const sequence<T, nFixedSizeLeft>& rseqLeft, const sequence<T, nFixedSizeRight>& rseqRight);
} // namespace sdv

#include "sequence.inl"

#endif // !defined SDV_SEQUENCE_H