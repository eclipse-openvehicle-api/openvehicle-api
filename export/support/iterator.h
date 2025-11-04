#ifndef SDV_ITERATOR_H
#define SDV_ITERATOR_H

#include <iterator>
#ifndef DONT_LOAD_CORE_TYPES
#include "../interfaces/core_types.h"
#endif

namespace sdv
{
    namespace internal
    {
        /**
         * @brief Index based iterators used for linear array based containers.
         * @tparam TContainer Container type to use for this iterator.
         * @tparam bConstIterator When set, the iterator is a const iterator not allowing to change values within the container.
         * @tparam bReserseIterator When set, the iterator starts at the end instead of at the beginning and decreases the location.
        */
        template <class TContainer, bool bConstIterator, bool bReverseIterator>
        class index_iterator
        {
            friend index_iterator<TContainer, !bConstIterator, bReverseIterator>;

        public:
            /**
             * @brief Container type reference
             */
            using container_reference = std::conditional_t<bConstIterator, const TContainer&, TContainer&>;

            /**
             * @brief Contaioner type pointer
             */
            using container_pointer = std::conditional_t<bConstIterator, const TContainer*, TContainer*>;

            /**
             * @brief Iterator category
             */
            using iterator_category = std::random_access_iterator_tag;

            /**
             * @brief Value type
             */
            using value_type = typename TContainer::value_type;

            /**
             * @brief Indexing type
             */
            using difference_type = size_t;

            /**
             * @brief Value type pointer
             */
            using pointer = std::conditional_t<bConstIterator, const value_type*, value_type*>;

            /**
             * @brief Value type reference
             */
            using reference = std::conditional_t<bConstIterator, const value_type&, value_type&>;

            /**
             * @brief Default constructor
             */
            index_iterator() {}

            /**
             * @brief Iterator initialization
             * @param[in] pContainer Pointer to the container.
             */
            index_iterator(container_pointer pContainer) : m_pContainer(pContainer)
            {}

            /**
             * @brief Copy constructor
             * @param[in] rit Reference to the iterator to copy from.
             */
            index_iterator(const index_iterator& rit) : m_pContainer(rit.m_pContainer), m_nIndex(rit.m_nIndex)
            {}

            /**
             * @brief Copy constructor
             * @param[in] rit Reference to the iterator to copy from.
             */
            index_iterator(const index_iterator<TContainer, !bConstIterator, bReverseIterator>& rit) :
                m_pContainer(const_cast<container_pointer>(rit.m_pContainer)), m_nIndex(rit.m_nIndex)
            {}

            /**
             * @brief Move constructor
             * @param[in] rit Reference to the iterator to move from.
             */
            index_iterator(index_iterator&& rit) noexcept : m_pContainer(rit.m_pContainer), m_nIndex(rit.m_nIndex)
            {
                rit.m_pContainer = nullptr;
                rit.m_nIndex = 0;
            }

            /**
             * @brief Assignment operator
             * @param[in] rit Reference to the iterator to copy from.
             * @return Returns a reference to the iterator.
             */
            index_iterator& operator=(const index_iterator& rit)
            {
                m_pContainer = rit.m_pContainer;
                m_nIndex = rit.m_nIndex;
                return *this;
            }

            /**
             * @brief Assignment operator
             * @param[in] rit Reference to the iterator to copy from.
             * @return Returns a reference to the iterator.
             */
            index_iterator& operator=(const index_iterator<TContainer, !bConstIterator, bReverseIterator>& rit)
            {
                m_pContainer = rit.m_pContainer;
                m_nIndex = rit.m_nIndex;
                return *this;
            }

            /**
             * @brief Move operator
             * @param[in] rit Reference to the iterator to move from.
             * @return Returns a reference to the iterator.
             */
            index_iterator& operator=(index_iterator&& rit) noexcept
            {
                m_pContainer = rit.m_pContainer;
                m_nIndex = rit.m_nIndex;
                rit.m_pContainer = nullptr;
                rit.m_nIndex = 0;
                return *this;
            }

            /**
             * @brief Does the iterator iterate the same container.
             * @param[in] rContainer Reference to the container to check whether the iterators are iterating the same container.
             * @return Returns 'true' when the iterator iterates the same container; otherwise returns 'false'.
            */
            bool is_valid(container_reference rContainer) const
            {
                return m_pContainer == &rContainer;
            }

            /**
             * @brief Compare two iterators for iterating the same container and pointing to the same position.
             * @param[in] rit Reference to the iterator to use for the comparison.
             * @return Returns 'true' when the iterators point to the same container and have the same position.
            */
            bool operator==(const index_iterator& rit) const
            {
                return m_pContainer == rit.m_pContainer && m_nIndex == rit.m_nIndex;
            }

            /**
             * @brief Compare two iterators for iterating the same container and pointing to the same position.
             * @param[in] rit Reference to the iterator to use for the comparison.
             * @return Returns 'true' when the iterators point to the same container and have the same position.
             */
            bool operator==(const index_iterator<TContainer, !bConstIterator, bReverseIterator>& rit) const
            {
                return m_pContainer == rit.m_pContainer && m_nIndex == rit.m_nIndex;
            }

            /**
             * @brief Compare two iterators for iterating the different containers or pointing to the another position.
             * @param[in] rit Reference to the iterator to use for the comparison.
             * @return Returns 'true' when the iterators point to different containers or have different positions.
             */
            bool operator!=(const index_iterator& rit) const
            {
                return !(*this == rit);
            }

            /**
             * @brief Compare two iterators for iterating the different containers or pointing to the another position.
             * @param[in] rit Reference to the iterator to use for the comparison.
             * @return Returns 'true' when the iterators point to different containers or have different positions.
             */
            bool operator!=(const index_iterator<TContainer, !bConstIterator, bReverseIterator>& rit) const
            {
                return !(*this == rit);
            }

            /**
             * @brief Get the value from the container at the location the iterator is pointing to.
             * @return Reference to the value.
             */
            reference operator*() const
            {
                size_t nIndex = m_nIndex;
                if (!m_pContainer || m_nIndex >= m_pContainer->size())
                {
                    XIndexOutOfRange exception;
                    exception.uiIndex = static_cast<uint32_t>(nIndex);
                    exception.uiSize = static_cast<uint32_t>(m_pContainer ? m_pContainer->size() : 0);
                    throw exception;
                }
                if constexpr (bReverseIterator)
                    nIndex = m_pContainer->size() - m_nIndex - 1;
                return (*m_pContainer)[nIndex];
            }

            /**
             * @brief Pointer operation the value from the container at the location the iterator is pointing to.
             * @remarks Only valid for container storing pointers.
             * @return Reference to the value.
             */
            pointer operator->() const
            {
                return std::pointer_traits<pointer>::pointer_to(**this);
            }

            /**
             * @brief Increase the iterator with one position. Doesn't go past the one position behind the back.
             * @return Reference to the iterator.
             */
            index_iterator& operator++()
            {
                if (m_pContainer && m_nIndex < m_pContainer->size())
                    m_nIndex++;
                return *this;
            }

            /**
             * @brief Copy the current iterator and increase the copied iterator with one position. Doesn't go past the one
             * position behind the back.
             * @return Copied and increased iterator.
             */
            index_iterator operator++(int)
            {
                index_iterator itCopy = *this;
                operator++();
                return itCopy;
            }

            /**
             * @brief Decrease the iterator with one position. Doesn't go before the front position.
             * @return Reference to the iterator.
             */
            index_iterator& operator--() noexcept
            {
                if (m_pContainer && m_nIndex > 0)
                    m_nIndex--;
                return *this;
            }

            /**
             * @brief Copy the current iterator and decrease the copied iterator with one position. Doesn't go before the front
             * position.
             * @return Copied and decreased iterator.
             */
            index_iterator operator--(int) noexcept
            {
                index_iterator itCopy = *this;
                operator--();
                return itCopy;
            }

            /**
             * @brief Increase the iterator with nOsset positions. Doesn't go past the one position behind the back.
             * @param[in] nOffset The amount of positions to increase.
             * @return Reference to the iterator.
             */
            index_iterator& operator+=(size_t nOffset) noexcept
            {
                if (!m_pContainer) return *this;
                if (nOffset > m_pContainer->size() - m_nIndex)
                    m_nIndex = m_pContainer->size();
                else
                    m_nIndex += nOffset;
                return *this;
            }

            /**
             * @brief Copy the current iterator and increase the copied iterator with nOffset positions. Doesn't go past the one
             * position behind the back.
             * @param[in] nOffset The amount of positions to increase.
             * @return Copied and increased iterator.
             */
            index_iterator operator+(size_t nOffset) const noexcept
            {
                index_iterator itCopy = *this;
                itCopy += nOffset;
                return itCopy;
            }

            /**
             * @brief Decrease the iterator with nOffset positions. Doesn't go before the front position.
             * @param[in] nOffset The amount of positions to decrease.
             * @return Reference to the iterator.
             */
            index_iterator& operator-=(size_t nOffset) noexcept
            {
                if (!m_pContainer) return *this;
                if (m_nIndex > nOffset)
                    m_nIndex -= nOffset;
                else if (m_nIndex)
                    m_nIndex = 0;

                return *this;
            }

            /**
             * @brief Copy the current iterator and decrease the copied iterator with nOffset positions. Doesn't go before the
             * front position.
             * @param[in] nOffset The amount of positions to increase.
             * @return Copied and decreased iterator.
             */
            index_iterator operator-(size_t nOffset) const noexcept
            {
                index_iterator itCopy = *this;
                itCopy -= nOffset;
                return itCopy;
            }

            /**
             * @brief Subtract the indices of two iterators of the same type.
             * @param[in] it The iterator to use for subtraction.
             * @return The difference between the contained index and the index of the supplied iterator.
            */
            difference_type operator-(index_iterator it) const
            {
                if (m_pContainer != it.m_pContainer) return 0;
                if (m_nIndex > it.m_nIndex)
                    return m_nIndex - it.m_nIndex;
                else
                    return it.m_nIndex - m_nIndex;
            }

            /**
             * @brief Get the value from the container at the location the iterator is pointing to added with an offset.
             * @param[in] nOffset The offset to increase the location.
             * @return Reference to the value at the location increased with an offset.
            */
            reference operator[](size_t nOffset) const
            {
                if (!m_pContainer || m_pContainer->empty())
                {
                    XIndexOutOfRange exception;
                    exception.uiIndex = static_cast<uint32_t>(nOffset + m_nIndex);
                    exception.uiSize = static_cast<uint32_t>(0);
                    throw exception;
                }
                return *(operator+(nOffset));
            }

        private:
            container_pointer m_pContainer = nullptr; ///< Pointer to the container.
            size_t            m_nIndex	   = 0;		  ///< Current index within the container.
        };
    } // namespace internal
} // namespace sdv

#endif // !defined SDV_ITERATOR_H