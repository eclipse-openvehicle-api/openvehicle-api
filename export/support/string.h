#ifndef SDV_STRING_H
#define SDV_STRING_H

#include <string>
#include <algorithm>
#include <ostream>
#include <istream>
#include <filesystem>
#include "iterator.h"
#include "pointer.h"

namespace sdv
{
    /**
     * @brief Templated string class.
     * @tparam TCharType The character type that the class uses.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     */
    template <typename TCharType, bool bUnicode = true, size_t nFixedSize = 0>
    class string_base
    {
    public:
		/**
         * @brief Value type for this string class.
         */
        using value_type = TCharType;

        /**
         * @brief Forward iterator class used by this string class.
         */
        using iterator = internal::index_iterator<string_base<TCharType, bUnicode, nFixedSize>, false, false>;

        /**
         * @brief Backward iterator class used by this string class.
         */
        using reverse_iterator = internal::index_iterator<string_base<TCharType, bUnicode, nFixedSize>, false, true>;

        /**
         * @brief Const forward iterator class used by this string class.
         */
        using const_iterator = internal::index_iterator<string_base<TCharType, bUnicode, nFixedSize>, true, false>;

        /**
         * @brief Const backward iterator class used by this string class.
         */
        using const_reverse_iterator = internal::index_iterator<string_base<TCharType, bUnicode, nFixedSize>, true, true>;

        /**
         * @brief Reference type of the element.
         */
        using reference = TCharType&;

        /**
         * @brief Const reference type of the element.
         */
        using const_reference = const TCharType&;

        /**
         * @brief Set when the type is a unicode type.
        */
        static constexpr bool is_unicode = bUnicode;

        /**
         * @brief Declaration of npos.
        */
        static constexpr size_t npos = static_cast<size_t>(-1);

        /**
         * @brief Default constructor
         */
        string_base() noexcept;

        /**
         * @brief Destructor
         */
        ~string_base();

        /**
         * @brief Copy constructor of same string type.
         * @param[in] rss Reference to the string to copy from.
         */
        string_base(const string_base& rss);

        /**
         * @brief Copy constructor of other string types.
         * @tparam TCharType2 The character type that the provided string class uses.
         * @tparam bUnicode2 When set, the provided string is a unicode string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to copy from.
         */
        template <typename TCharType2, bool bUnicode2, size_t nFixedSize2>
        explicit string_base(const string_base<TCharType2, bUnicode2, nFixedSize2>& rss);

        /**
         * @brief Move constructor of same string type.
         * @param[in] rss Reference to the string to move the data from.
         */
        string_base(string_base&& rss) noexcept;

        /**
         * @brief Move constructor of other string types.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to move the data from.
         */
        template <size_t nFixedSize2>
        string_base(string_base<TCharType, bUnicode, nFixedSize2>&& rss);

        /**
         * @brief Constructor with C++ string assignment
         * @tparam TCharType2 The character type that the provided string class uses.
         * @param[in] rss Reference to the string to copy from.
         */
        template <typename TCharType2>
        string_base(const std::basic_string<TCharType2>& rss);

        /**
         * @brief Constructor with zero terminated C string assignment
         * @param[in] szStr Pointer to zero terminated string.
         */
        string_base(const TCharType* szStr);

        /**
         * @brief Construct a string consisting of nCount copies of character c.
         * @param[in] nCount Number of characters to insert.
         * @param[in] c The character to insert nCount times.
         */
        string_base(size_t nCount, TCharType c);

        /**
         * @brief Construct a string from a substring starting at nPos.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
        */
        template <size_t nFixedSize2>
        string_base(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos);

        /**
         * @brief Construct a string from a substring starting at nPos.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
         */
        string_base(const std::basic_string<TCharType>& rss, size_t nPos);

        /**
         * @brief Construct a string from a substring starting at nPos.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
         * @param[in] nCount The amount of characters of the substring to include.
         */
        template <size_t nFixedSize2>
        string_base(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount);

        /**
         * @brief Construct a string from a substring starting at nPos.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
         * @param[in] nCount The amount of characters of the substring to include.
         */
        string_base(const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount);

        /**
         * @brief Construct a string with nCount characters.
         * @param[in] sz C-Style string. Can contain null characters.
         * @param[in] nCount The amount of characters to copy.
         */
        string_base(const TCharType* sz, size_t nCount);

        /**
         * @brief Construct a string with the content of a range defined by two iterators.
         * @remarks Both iterators must point to the same string.
         * @tparam TIterator Iterator type to use.
         * @param[in] itFirst The iterator pointing to the first character.
         * @param[in] itLast The iterator pointing to the value past the last character.
         */
        template <class TIterator>
        string_base(TIterator itFirst, TIterator itLast);

        /**
         * @brief Construct a string from an initializer list.
         * @param[in] ilist Initializer list.
        */
        string_base(std::initializer_list<TCharType> ilist);

        /**
         * @brief Assignment operator of same string type.
         * @param[in] rss Reference to the string to assign from.
         * @return Returns a reference to this object.
         */
        string_base& operator=(const string_base& rss);

        /**
         * @brief Assignment operator of other string types.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to assign from.
         * @return Returns a reference to this object.
         */
        template <size_t nFixedSize2>
        string_base& operator=(const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Move operator of same string type.
         * @param[in] rss Reference to the string to assign from.
         * @return Returns a reference to this object.
         */
        string_base& operator=(string_base&& rss) noexcept;

        /**
         * @brief Move operator of other string types.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to assign from.
         * @return Returns a reference to this object.
         */
        template <size_t nFixedSize2>
        string_base& operator=(string_base<TCharType, bUnicode, nFixedSize2>&& rss);

        /**
         * @brief Assignment operator with C++ string assignment
         * @param[in] rss Reference to the string to assign from.
         * @return Returns a reference to this object.
         */
        string_base& operator=(const std::basic_string<TCharType>& rss);

        /**
         * @brief Assignment operator with zero terminated C string assignment
         * @param[in] szStr Pointer to zero terminated string.
         * @return Returns a reference to this object.
         */
        string_base& operator=(const TCharType* szStr);

        /**
         * @brief Assignment operator with initializer list.
         * @param[in] ilist Initializer list
         * @return Returns a reference to this object.
         */
        string_base& operator=(std::initializer_list<TCharType> ilist);

        /**
         * @brief Assign a string consisting of nCount copies of character c.
         * @param[in] nCount Number of characters to insert.
         * @param[in] c The character to insert nCount times.
         * @return Returns a reference to this object.
         */
        string_base& assign(size_t nCount, TCharType c);

        /**
         * @brief Assign a copy of a string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to copy from.
         * @return Returns a reference to this object.
         */
        template <size_t nFixedSize2>
        string_base& assign(const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Assign a copy of a C++ string.
         * @param[in] rss Reference to the string to copy from.
         * @return Returns a reference to this object.
         */
        string_base& assign(const std::basic_string<TCharType>& rss);

        /**
         * @brief Assign a string from a substring starting at nPos.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
         * @param[in] nCount The amount of characters of the substring to include.
         * @return Returns a reference to this object.
         */
        template <size_t nFixedSize2>
        string_base& assign(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Assign a string from a substring starting at nPos.
         * @param[in] rss Reference of the string to extract the substring from.
         * @param[in] nPos The position to start the substring.
         * @param[in] nCount The amount of characters of the substring to include.
         * @return Returns a reference to this object.
         */
        string_base& assign(const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Move the string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to move the data from.
         * @return Returns a reference to this object.
         */
        template <size_t nFixedSize2>
        string_base& assign(string_base<TCharType, bUnicode, nFixedSize2>&& rss);

        /**
         * @brief Assign a string with nCount characters.
         * @param[in] sz C-Style string. Can contain null characters.
         * @param[in] nCount The amount of characters to copy.
         * @return Returns a reference to this object.
         */
        string_base& assign(const TCharType* sz, size_t nCount);

        /**
         * @brief Assign with zero terminated C string assignment
         * @param[in] sz Pointer to zero terminated string.
         * @return Returns a reference to this object.
         */
        string_base& assign(const TCharType* sz);

        /**
         * @brief Assign a string with the content of a range defined by two iterators.
         * @remarks Both iterators must point to the same string.
         * @tparam TIterator Iterator type to use.
         * @param[in] itFirst The iterator pointing to the first character.
         * @param[in] itLast The iterator pointing to the value past the last character.
         * @return Returns a reference to this object.
         */
        template <class TIterator>
        string_base& assign(TIterator itFirst, TIterator itLast);

        /**
         * @brief Assign a string from an initializer list.
         * @param[in] ilist Initializer list.
         * @return Returns a reference to this object.
         */
        string_base& assign(std::initializer_list<TCharType> ilist);

        /**
         * @brief Return a reference to the character at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the character.
        */
        reference at(size_t nPos);

        /**
         * @brief Return a reference to the character at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the character.
         */
        const_reference at(size_t nPos) const;

        /**
         * @brief Return a reference to the character at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the character.
         */
        reference operator[](size_t nPos);

        /**
         * @brief Return a reference to the character at the specified position.
         * @param[in] nPos The specified position.
         * @return Reference to the character.
         */
        const_reference operator[](size_t nPos) const;

        /**
         * @brief Return a reference to the first character.
         * @return Reference to the character.
         */
        TCharType& front();

        /**
         * @brief Return a reference to the first character.
         * @return Reference to the character.
         */
        const TCharType& front() const;

        /**
         * @brief Return a reference to the last character.
         * @return Reference to the character.
         */
        TCharType& back();

        /**
         * @brief Return a reference to the last character.
         * @return Reference to the character.
         */
        const TCharType& back() const;

        /**
         * @brief Cast operator for SDV strings.
         * @tparam TCharType2 The character type of the string to convert to.
         * @tparam bUnicode2 When set, the string to convert to is a unicode string.
         * @tparam nFixedSize2 The fixed size of the string to convert to.
         * @return Returns a SDV string object containing a copy of the string.
         */
        template <typename TCharType2, bool bUnicode2, size_t nFixedSize2>
        operator string_base<TCharType2, bUnicode2, nFixedSize2>() const;

        /**
         * @brief Cast operator for C++ strings.
         * @return Returns a C++ string object containing a copy of the string.
         */
        operator std::basic_string<TCharType>() const;

        /**
         * @brief Access to the underlying data.
         * @remarks The underlying data might not be zero terminated.
         * @return Pointer to the string.
        */
        const TCharType* data() const noexcept;

        /**
         * @brief Access to the buffer.
         * @return Returns reference to the internal buffer.
         */
        pointer<TCharType, nFixedSize ? nFixedSize + 1 : 0>& buffer() noexcept;

        /**
         * @brief Return a pointer to a zero terminate string.
         * @return Pointer to the string.
        */
        const TCharType* c_str() const noexcept;

        /**
         * @brief Return an iterator to the first character of the string.
         * @return Iterator to the first character of te string.
        */
        iterator begin() noexcept;

        /**
		 * @brief Return an iterator to the first character of the string.
		 * @return Iterator to the first character of te string.
		 */
		const_iterator begin() const noexcept;

		/**
         * @brief Return a const iterator to the first character of the string.
         * @return Const iterator to the first character of te string.
         */
        const_iterator cbegin() const noexcept;

        /**
         * @brief Return a reverse-iterator to the last character of the string.
         * @return Reverse iterator to the last character of te string.
        */
        reverse_iterator rbegin() noexcept;

        /**
		 * @brief Return a reverse-iterator to the last character of the string.
		 * @return Reverse iterator to the last character of te string.
		 */
		const_reverse_iterator rbegin() const noexcept;

		/**
         * @brief Return a const reverse iterator to the last character of the string.
         * @return Const reverse iterator to the last character of te string.
         */
        const_reverse_iterator crbegin() const noexcept;

        /**
         * @brief Return an iterator beyond the last character of the string.
         * @return Iterator beyond the last character of te string.
        */
        iterator end() noexcept;

        /**
		 * @brief Return an iterator beyond the last character of the string.
		 * @return Iterator beyond the last character of te string.
		 */
		const_iterator end() const noexcept;

		/**
         * @brief Return a const beyond the last character of the string.
         * @return Const iterator beyond the last character of te string.
        */
        const_iterator cend() const noexcept;

        /**
         * @brief Return a reverse iterator before the first character of the string.
         * @return Reverse iterator before the last character of te string.
        */
        reverse_iterator rend() noexcept;

        /**
		 * @brief Return a reverse iterator before the first character of the string.
		 * @return Reverse iterator before the last character of te string.
		 */
		const_reverse_iterator rend() const noexcept;

		/**
         * @brief Return a const reverse iterator before the first character of the string.
         * @return Const reverse iterator before the last character of te string.
         */
        const_reverse_iterator crend() const noexcept;

        /**
         * @brief Is the string empty?
         * @return Returns 'true' when the string is empty; 'false' when not.
         */
        bool empty() const;

        /**
         * @brief Get the size of the string.
         * @remarks The length and the size for the string are equal.
         * @return The size of the string buffer.
         */
        size_t size() const;

        /**
         * @brief Get the length of the string.
         * @remarks The length and the size for the string are equal.
         * @return The length of the string.
         */
        size_t length() const;

        /**
         * @brief Reserve capacity for the string buffer. Additional buffer will be filled with zeros.
         * @remarks Reducing the capacity will have no effect.
         * @param[in] nNewCap The new capacity.
        */
        void reserve(size_t nNewCap = 0);

        /**
         * @brief Get the current string capacity.
         * @remarks This will be the same as the length and size of the string.
         * @return The capacity of the current string.
         */
        size_t capacity() const noexcept;

        /**
         * @brief Reduce the buffer to fit the string.
         * @remarks This function will have no effect.
         */
        void shrink_to_fit();

        /**
         * @brief Clear the string.
         */
        void clear();

        /**
         * @brief Insert nCount copies of character c at the position nIndex.
         * @param[in] nIndex The index at which to insert the characters.
         * @param[in] nCount Number of characters to insert.
         * @param[in] c The character to insert nCount times.
         * @return Reference to this string.
         */
        string_base& insert(size_t nIndex, size_t nCount, TCharType c);

        /**
         * @brief Inserts zero-terminated character string pointed to by sz at the position nIndex.
         * @param[in] nIndex The index at which to insert the characters.
         * @param[in] sz Zero terminated string.
         * @return Reference to this string.
         */
        string_base& insert(size_t nIndex, const TCharType* sz);

        /**
         * @brief Inserts nCount characters from the string at the position nIndex.
         * @param[in] nIndex The index at which to insert the characters.
         * @param[in] sz Zero terminated string.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        string_base& insert(size_t nIndex, const TCharType* sz, size_t nCount);

        /**
         * @brief Inserts string rss at the position nIndex.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nIndex The index at which to insert the string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& insert(size_t nIndex, const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Inserts a sub-string starting at index nPos and with nCount character at the position nIndex
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nIndex The index at which to insert the string.
         * @param[in] rss Reference to the string to insert.
         * @param[in] nPos Position of the first character in ss to insert.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& insert(size_t nIndex, const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Inserts C++ string_base<TCharType> ss at the position nIndex.
         * @param[in] nIndex The index at which to insert the string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        string_base& insert(size_t nIndex, const std::basic_string<TCharType>& rss);

        /**
         * @brief Inserts a sub-string starting at index nPos and with nCount character at the position nIndex
         * @param[in] nIndex The index at which to insert the string.
         * @param[in] rss Reference to the string to insert.
         * @param[in] nPos Position of the first character in ss to insert.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        string_base& insert(size_t nIndex, const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Insert a character c at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the characters.
         * @param[in] c The character to insert.
         * @return The iterator pointing to the inserted character.
         */
        iterator insert(const_iterator itPos, TCharType c);

        /**
         * @brief Insert nCount copies of character c at the position itPos.
         * @param[in] itPos Iterator pointing to the position to insert the characters.
         * @param[in] nCount Number of characters to insert.
         * @param[in] c The character to insert nCount times.
         * @return The iterator pointing to the inserted character.
         */
        iterator insert(const_iterator itPos, size_t nCount, TCharType c);

        /**
         * @brief Insert a string with the content of a range defined by two iterators.
         * @remarks Both iterators must point to the same string.
         * @tparam TIterator Iterator type to use.
         * @param[in] itPos Iterator pointing to the position to insert the characters.
         * @param[in] itFirst The iterator pointing to the first character.
         * @param[in] itLast The iterator pointing to the value past the last character.
         * @return The iterator pointing to the inserted string.
         */
        template <class TIterator>
        iterator insert(const_iterator itPos, TIterator itFirst, TIterator itLast);

        /**
         * @brief Insert a string from an initializer list.
         * @param[in] itPos Iterator pointing to the position to insert the characters.
         * @param[in] ilist Initializer list.
         * @return The iterator pointing to the inserted character.
         */
        iterator insert(const_iterator itPos, std::initializer_list<TCharType> ilist);

        /**
         * @brief Remove characters at the provided position.
         * @param[in] nIndex The index at which to erase the characters.
         * @param[in] nCount Number of characters to erase.
         * @return Returns a reference to this object.
         */
        string_base& erase(size_t nIndex = 0, size_t nCount = npos);

        /**
         * @brief Remove a character at the provided position.
         * @remarks The iterator must point to this string.
         * @param[in] itPos Iterator pointing to the position to erase the character from.
         * @return Iterator to the character following the erase character or end() when no more characters are available.
         */
        iterator erase(const_iterator itPos);

        /**
         * @brief Remove al characters starting at itFirst until but not including itLast.
         * @remarks Both iterators must point to this string.
         * @param[in] itFirst The iterator pointing to the first character.
         * @param[in] itLast The iterator pointing to the value past the last character.
         * @return Iterator to the character following the erased characters or end() when no more characters are available.
         */
        iterator erase(const_iterator itFirst, const_iterator itLast);

        /**
         * @brief Appends the given character to the end of the string.
         * @param[in] c The character to append.
        */
        void push_back(TCharType c);

        /**
         * @brief Removes the last character from the string.
        */
        void pop_back();

        /**
         * @brief Append nCount copies of character c at the end of the string.
         * @param[in] nCount Number of characters to insert.
         * @param[in] c The character to append nCount times.
         * @return Reference to this string.
         */
        string_base& append(size_t nCount, TCharType c);

        /**
         * @brief Append string rss at the end of the string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& append(const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Append string rss at the end of the string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        string_base& append(const std::basic_string<TCharType>& rss);

        /**
         * @brief Append a sub-string starting at index nPos and with nCount character at the end of the string
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to insert.
         * @param[in] nPos Position of the first character in ss to insert.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& append(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Append a sub-string starting at index nPos and with nCount character at the end of the string
         * @param[in] rss Reference to the string to insert.
         * @param[in] nPos Position of the first character in ss to insert.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        string_base& append(const std::basic_string<TCharType>& rss, size_t nPos, size_t nCount = npos);

        /**
         * @brief Append nCount characters from the string at the end of the string.
         * @param[in] sz Zero terminated string.
         * @param[in] nCount Number of characters to insert.
         * @return Reference to this string.
         */
        string_base& append(const TCharType* sz, size_t nCount);

        /**
         * @brief Append the provided string at the end of the string.
         * @param[in] sz Zero terminated string.
         * @return Reference to this string.
         */
        string_base& append(const TCharType* sz);

        /**
         * @brief Append a string with the content of a range defined by two iterators.
         * @remarks Both iterators must point to the same string.
         * @tparam TIterator Iterator type to use.
         * @param[in] itFirst The iterator pointing to the first character.
         * @param[in] itLast The iterator pointing to the value past the last character.
         * @return Reference to this string.
         */
        template <class TIterator>
        string_base& append(TIterator itFirst, TIterator itLast);

        /**
         * @brief Append a string from an initializer list.
         * @param[in] ilist Initializer list.
         * @return Reference to this string.
         */
        string_base& append(std::initializer_list<TCharType> ilist);

        /**
         * @brief Append string rss at the end of the string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& operator+=(const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Append string rss at the end of the string.
         * @param[in] rss Reference to the string to insert.
         * @return Reference to this string.
         */
        string_base& operator+=(const std::basic_string<TCharType>& rss);

        /**
         * @brief Append the character c at the end of the string.
         * @param[in] c The character to append.
         * @return Reference to this string.
         */
        string_base& operator+=(TCharType c);

        /**
         * @brief Append the provided string at the end of the string.
         * @param[in] sz Zero terminated string.
         * @return Reference to this string.
         */
        string_base& operator+=(const TCharType* sz);

        /**
         * @brief Append a string from an initializer list.
         * @param[in] ilist Initializer list.
         * @return Reference to this string.
         */
        string_base& operator+=(std::initializer_list<TCharType> ilist);

        /**
         * @brief Compare two character sequences.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to compare this string with.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        template <size_t nFixedSize2>
        int compare(const string_base<TCharType, bUnicode, nFixedSize2>& rss) const noexcept;

        /**
         * @brief Compare two character sequences.
         * @param[in] rss Reference to the string to compare this string with.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        int compare(const std::basic_string<TCharType>& rss) const noexcept;

        /**
         * @brief Compare two character sequences.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nPos1 Position of the first character in the string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] rss Reference to the string to compare this string with.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        template <size_t nFixedSize2>
        int compare(size_t nPos1, size_t nCount1, const string_base<TCharType, bUnicode, nFixedSize2>& rss) const;

        /**
         * @brief Compare two character sequences.
         * @param[in] nPos1 Position of the first character in the string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] rss Reference to the string to compare this string with.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        int compare(size_t nPos1, size_t nCount1, const std::basic_string<TCharType>& rss) const;

        /**
         * @brief Compare two character sequences.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nPos1 Position of the first character in this string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] rss Reference to the string to compare this string with.
         * @param[in] nPos2 Position of the first character in the provided string to compare.
         * @param[in] nCount2 Number if characters in the provided string to compare.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        template <size_t nFixedSize2>
        int compare(size_t nPos1, size_t nCount1, const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos2, size_t nCount2 = npos) const;

        /**
         * @brief Compare two character sequences.
         * @param[in] nPos1 Position of the first character in this string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] rss Reference to the string to compare this string with.
         * @param[in] nPos2 Position of the first character in the provided string to compare.
         * @param[in] nCount2 Number if characters in the provided string to compare.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        int compare(size_t nPos1, size_t nCount1, const std::basic_string<TCharType>& rss, size_t nPos2, size_t nCount2 = npos) const;

        /**
         * @brief Compare two character sequences.
         * @param[in] sz Pointer to the character string to compare.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        int compare(const TCharType* sz) const;

        /**
         * @brief Compare two character sequences.
         * @param[in] nPos1 Position of the first character in this string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] sz Pointer to the character string to compare.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
         */
        int compare(size_t nPos1, size_t nCount1, const TCharType* sz) const;

        /**
         * @brief Compare two character sequences.
         * @param[in] nPos1 Position of the first character in this string to compare.
         * @param[in] nCount1 Number if characters in this string to compare.
         * @param[in] sz Pointer to the character string to compare.
         * @param[in] nCount2 Number if characters in the provided string to compare.
         * @return Is negative when the string contains characters of lesser lexicographical order; returns prositive when the
         * string contains characters of higher lexcicographical order; otherwise returns 0.
        */
        int compare(size_t nPos1, size_t nCount1, const TCharType* sz, size_t nCount2) const;

        /**
         * @brief Replace characters with provided string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& replace(size_t nPos, size_t nCount, const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Replace characters with provided string.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(size_t nPos, size_t nCount, const std::basic_string<TCharType>& rss);

        /**
         * @brief Replace characters with provided string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& replace(const_iterator itFirst, const_iterator itLast, const string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Replace characters with provided string.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(const_iterator itFirst, const_iterator itLast, const std::basic_string<TCharType>& rss);

        /**
         * @brief Replace characters with provided string substring.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @param[in] nPos2 Position of the characters within the provided string.
         * @param[in] nCount2 Amount of characters within the provided string.
         * @return Reference to this string.
         */
        template <size_t nFixedSize2>
        string_base& replace(
            size_t nPos, size_t nCount, const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos2, size_t nCount2 = npos);

        /**
         * @brief Replace characters with provided string substring.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] rss Reference to the string to use for recplacement.
         * @param[in] nPos2 Position of the characters within the provided string.
         * @param[in] nCount2 Amount of characters within the provided string.
         * @return Reference to this string.
         */
        string_base& replace(size_t nPos, size_t nCount, const std::basic_string<TCharType>& rss, size_t nPos2, size_t nCount2 = npos);

        /**
         * @brief Replace characters with provided string substring.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] sz Pointer to the C string to use for recplacement. Can contain zeros.
         * @param[in] nCount2 Amount of characters within the provided string.
         * @return Reference to this string.
         */
        string_base& replace(size_t nPos, size_t nCount, const TCharType* sz, size_t nCount2);

        /**
         * @brief Replace characters with provided string substring.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] sz Pointer to the C string to use for recplacement. Can contain zeros.
         * @param[in] nCount2 Amount of characters within the provided string.
         * @return Reference to this string.
         */
        string_base& replace(const_iterator itFirst, const_iterator itLast, const TCharType* sz, size_t nCount2);

        /**
         * @brief Replace characters with provided zero terminated string.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] sz Pointer to the zero terminated string to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(size_t nPos, size_t nCount, const TCharType* sz);

        /**
         * @brief Replace characters with provided zero terminated string.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] sz Pointer to the zero terminated string to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(const_iterator itFirst, const_iterator itLast, const TCharType* sz);

        /**
         * @brief Replace characters with nCount2 characters.
         * @param[in] nPos Position to replace characters.
         * @param[in] nCount Amount of characters to replace.
         * @param[in] nCount2 Amount of characters to use for replacement.
         * @param[in] c Character to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(size_t nPos, size_t nCount, size_t nCount2, TCharType c);

        /**
         * @brief Replace characters with nCount2 characters.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
		 * @param[in] nCount2 Amount of characters to use for replacement.
		 * @param[in] c Character to use for recplacement.
         * @return Reference to this string.
         */
        string_base& replace(const_iterator itFirst, const_iterator itLast, size_t nCount2, TCharType c);

        /**
         * @brief Replace characters with a container of TCharType identified by beginning and ending iterators.
         * @remarks itFirst and itLast must point to a position of this string.
         * @tparam TIterator Type of iterator to use for iterating through the container.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] itFirst2 Beginning iterator of the container.
         * @param[in] itLast2 Ending iterator of the container
         * @return Reference to this string.
         */
        template <class TIterator>
        string_base& replace(const_iterator itFirst, const_iterator itLast, TIterator itFirst2, TIterator itLast2);

        /**
         * @brief Replace characters with an initializer list.
         * @remarks itFirst and itLast must point to a position of this string.
         * @param[in] itFirst Iterator position of the first character to replace.
         * @param[in] itLast Iterator position of the character beyond the last character to replace.
         * @param[in] ilist Initializer list.
         * @return Reference to this string.
         */
        string_base& replace(const_iterator itFirst, const_iterator itLast, std::initializer_list<TCharType> ilist);

        /**
         * @brief Return a substring.
         * @param[in] nPos The substring start position.
         * @param[in] nCount The amount of characters in the substring (if more than the available, all the characters right of the
         * position).
         * @return Returns a string with the substring characters.
        */
        string_base substr(size_t nPos = 0, size_t nCount = npos) const;

        /**
         * @brief Copy a substring into the szDestination.
         * @remarks Does not copy a zero terminating character.
         * @param[in] szDest Destination string.
         * @param[in] nCount Amount of characters to copy (or the maximum amount is determined by the size).
         * @param[in] nPos The position from where to start copying.
         * @return Amount of characters copied.
        */
        size_t copy(TCharType* szDest, size_t nCount, size_t nPos = 0) const;

        /**
         * @brief Set the new size of the string. Additional characters will be filled with '\0'.
         * @param[in] nCount The size of the string buffer.
         */
        void resize(size_t nCount);

        /**
         * @brief Set the new size of the string. Additional characters will be filled with c.
         * @param[in] nCount The size of the string buffer.
         * @param[in] c Character to use to fill additional buffer space with.
         */
        void resize(size_t nCount, TCharType c);

        /**
         * @brief Exchange the content of provided string with this string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss String to swap with.
         */
        template <size_t nFixedSize2>
        void swap(string_base<TCharType, bUnicode, nFixedSize2>& rss);

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t find(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find(const std::basic_string<TCharType>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Position to start searching.
         * @param[in] nCount Length of the string to search for.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find(const TCharType* sz, size_t nPos = 0) const;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] c Character to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the character hasn't been found.
         */
        size_t find(TCharType c, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching in reverse order (nPos - 1).
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t rfind(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching in reverse order (nPos - 1).
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t rfind(const std::basic_string<TCharType>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Position to start searching in reverse order (nPos - 1).
         * @param[in] nCount Length of the string to search for.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t rfind(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Position to start searching in reverse order (nPos - 1).
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t rfind(const TCharType* sz, size_t nPos = npos) const;

        /**
         * @brief Find the position of the occurrence of the character sequence in the string.
         * @param[in] c Character to find to find.
         * @param[in] nPos Position to start searching in reverse order (nPos - 1).
         * @return Position of the first occurrence or npos if the character hasn't been found.
         */
        size_t rfind(TCharType c, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the position of the occurrence of one of the characters being in the provided character sequence.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t find_first_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of one of the characters being in the provided character sequence.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_of(const std::basic_string<TCharType>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of one of the characters being in the provided character sequence.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Position to start searching.
         * @param[in] nCount Length of the string to search for.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_of(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the position of the occurrence of one of the characters being in the provided character sequence.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_of(const TCharType* sz, size_t nPos = 0) const;

        /**
         * @brief Find the position of the occurrence of the provided character.
         * @param[in] c Character to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the character hasn't been found.
         */
        size_t find_first_of(TCharType c, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of any characters not being in the provided character.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t find_first_not_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of any characters not being in the provided character.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_not_of(const std::basic_string<TCharType>& rss, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the occurrence of any characters not being in the provided character.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Position to start searching.
         * @param[in] nCount Length of the string to search for.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_not_of(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the position of the occurrence of any characters not being in the provided character.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the string hasn't been found.
         */
        size_t find_first_not_of(const TCharType* sz, size_t nPos = 0) const;

        /**
         * @brief Find the position of the occurrence of any characters not being the provided character.
         * @param[in] c Character to find.
         * @param[in] nPos Position to start searching.
         * @return Position of the first occurrence or npos if the character hasn't been found.
         */
        size_t find_first_not_of(TCharType c, size_t nPos = 0) const noexcept;

        /**
         * @brief Find the position of the last occurrence of one of the characters being in the provided character sequence.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t find_last_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the position of the last occurrence of one of the characters being in the provided character sequence.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_of(const std::basic_string<TCharType>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the position of the last occurrence of one of the characters being in the provided character sequence.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @param[in] nCount Length of the string to search for.
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_of(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the position of the last occurrence of one of the characters being in the provided character sequence.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_of(const TCharType* sz, size_t nPos = npos) const;

        /**
         * @brief Find the position of the last occurrence of the provided character.
         * @param[in] c Character to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the character hasn't been found.
         */
        size_t find_last_of(TCharType c, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the last position of the occurrence of any characters not being in the provided character.
         * @tparam nFixedSize2 The fixed size of the provided string.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        template <size_t nFixedSize2>
        size_t find_last_not_of(const string_base<TCharType, bUnicode, nFixedSize2>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the last position of the occurrence of any characters not being in the provided character.
         * @param[in] rss Reference to the string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_not_of(const std::basic_string<TCharType>& rss, size_t nPos = npos) const noexcept;

        /**
         * @brief Find the last position of the occurrence of any characters not being in the provided character.
         * @param[in] sz Pointer to the string to find. May contain zeros.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @param[in] nCount Length of the string to search for.
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_not_of(const TCharType* sz, size_t nPos, size_t nCount) const;

        /**
         * @brief Find the last position of the occurrence of any characters not being in the provided character.
         * @param[in] sz Pointer to zero terminated string to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the string hasn't been found.
         */
        size_t find_last_not_of(const TCharType* sz, size_t nPos = npos) const;

        /**
         * @brief Find the last position of the occurrence of any characters not being the provided character.
         * @param[in] c Character to find.
         * @param[in] nPos Ending position to start searching (last character ist nPos - 1).
         * @return Position of the last occurrence or npos if the character hasn't been found.
         */
        size_t find_last_not_of(TCharType c, size_t nPos = npos) const noexcept;

    private:
        pointer<TCharType, nFixedSize ? nFixedSize + 1 : 0> m_ptrData; ///< Smart pointer to the data.
    };

    /**
     * @brief String based on 'char' data type covering ANSI and UTF-8 character set.
     */
    using string = string_base<char, false, 0>;

    /**
     * @brief String based on 'char' data type covering ANSI and UTF-8 character set.
     * @tparam nFixedSize The fixed size of the string.
     */
    template <size_t nFixedSize>
    using fixed_string = string_base<char, false, nFixedSize>;

    /**
     * @brief String based on 'wchar_t' data type covering UTF-16 (on Windows) and UTF-32 (on Linux) character set.
     */
    using wstring = string_base<wchar_t, true, 0>;

    /**
     * @brief String based on 'wchar_t' data type covering UTF-16 (on Windows) and UTF-32 (on Linux) character set.
     * @tparam nFixedSize The fixed size of the string.
     */
    template <size_t nFixedSize>
    using fixed_wstring = string_base<wchar_t, true, nFixedSize>;

    /**
     * @brief String based on 'char' data type covering UTF-8 character set.
     */
    using u8string = string_base<char, true, 0>;

    /**
     * @brief String based on 'char' data type covering UTF-8 character set.
     * @tparam nFixedSize The fixed size of the string.
     */
    template <size_t nFixedSize>
    using fixed_u8string = string_base<char, true, nFixedSize>;

    /**
     * @brief String based on 'char16_t' data type covering UTF-16 character set.
     */
    using u16string = string_base<char16_t, true, 0>;

    /**
     * @brief String based on 'char16_t' data type covering UTF-16 character set.
     * @tparam nFixedSize The fixed size of the string.
     */
    template <size_t nFixedSize>
    using fixed_u16string = string_base<char16_t, true, nFixedSize>;

    /**
     * @brief String based on 'char32_t' data type covering UTF-32 character set.
     */
    using u32string = string_base<char32_t, true, 0>;

    /**
     * @brief String based on 'char32_t' data type covering UTF-32 character set.
     * @tparam nFixedSize The fixed size of the string.
     */
    template <size_t nFixedSize>
    using fixed_u32string = string_base<char32_t, true, nFixedSize>;

    /**
     * @brief Returns a string composed from left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&  rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Returns a string composed from left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>&rssRight);

    /**
     * @brief Returns a string composed from left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Returns a string composed from left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to zero terminate right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const TCharType* szRight);

    /**
     * @brief Returns a string composed from left characters followed by right character.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] cRight Right character.
     * @return String with left characters concatenated with right character.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        TCharType cRight);

    /**
     * @brief Returns a string composed from left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to zero terminated left string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        const TCharType* szLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Returns a string composed from left character followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] cLeft Left character.
     * @param[in] rssRight Reference to the right string.
     * @return String with left character concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        TCharType cLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Returns a string by moving left characters followed by moving right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string. Content will be moved to the new string.
     * @param[in] rssRight Reference to the right string. Content will be moved to the new string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight);

    /**
     * @brief Returns a string by moving left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string. Content will be moved to the new string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Returns a string by moving left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string. Content will be moved to the new string.
     * @param[in] rssRight Reference to the right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const std::basic_string<TCharType>& rssRight);

    /**
     * @brief Returns a string by moving left characters followed by right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string. Content will be moved to the new string.
     * @param[in] szRight Pointer to zero terminated right string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        const TCharType* szRight);

    /**
     * @brief Returns a string by moving left characters followed by right character.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string. Content will be moved to the new string.
     * @param[in] cRight Right character.
     * @return String with left characters concatenated with right character.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    string_base<TCharType, bUnicodeLeft, nFixedSizeLeft> operator+(
        string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>&& rssLeft,
        TCharType cRight);

    /**
     * @brief Returns a string filled with left characters followed by moving right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string. Content will be moved to the new string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight);

    /**
     * @brief Returns a string filled with left characters followed by moving right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string. Content will be moved to the new string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        const std::basic_string<TCharType>& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight);

    /**
     * @brief Returns a string filled with left characters followed by moving right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to zero terminated left string.
     * @param[in] rssRight Reference to the right string. Content will be moved to the new string.
     * @return String with left characters concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        const TCharType* szLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight);

    /**
     * @brief Returns a string filled with left character followed by moving right characters.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] cLeft Left character.
     * @param[in] rssRight Reference to the right string. Content will be moved to the new string.
     * @return String with left character concatenated with right characters.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    string_base<TCharType, bUnicodeRight, nFixedSizeRight> operator+(
        TCharType cLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>&& rssRight);

    /**
     * @brief Swap the content of the left string with the content of the right string.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    void swap(string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Stream the content of a string into a std::basic_ostream object.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_ostream<TCharType, std::char_traits<TCharType>>& operator<<(
        std::basic_ostream<TCharType, std::char_traits<TCharType>>& rstream,
        const string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Stream the content of a std::basic_istream into a string.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_istream<TCharType, std::char_traits<TCharType>>& operator>>(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Read a line from the stream.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @param[in] cDelim Delimiter to separate lines.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss,
        TCharType cDelim);

    /**
     * @brief Read a line from the stream.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @param[in] cDelim Delimiter to separate lines.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>&& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss,
        TCharType cDelim);

    /**
     * @brief Read a line separated by 'newline' from the stream.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Read a line separated by 'newline' from the stream.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Fixed size of the string.
     * @param[in] rstream Reference to the stream.
     * @param[in] rss Reference to the string.
     * @return Reference to the stream.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::basic_istream<TCharType, std::char_traits<TCharType>>& getline(
        std::basic_istream<TCharType, std::char_traits<TCharType>>&& rstream, string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Compare two character sequences for equality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are equal; 'false' otherwise.
    */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for equality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for equality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator==(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for inequality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are not equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for inequality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are not equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for inequality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when the strings are not equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator!=(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<(const std::basic_string<TCharType>& rssLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is smaller than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<=(const std::basic_string<TCharType>& rssLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @param[in] rssLeft Reference to the left string.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>(const std::basic_string<TCharType>& rssLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft,
        const std::basic_string<TCharType>& rssRight) noexcept;

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] rssRight Reference to the right string.
     * @return Returns 'true' when left is larger than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>=(const std::basic_string<TCharType>& rssLeft,
        const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight) noexcept;

    /**
     * @brief Compare two character sequences for equality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when the strings are equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator==(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare two character sequences for equality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when the strings are equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator==(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Compare two character sequences for inequality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when the strings are not equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator!=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare two character sequences for inequality.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when the strings are not equal; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator!=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when left is smaller than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator<(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when left is smaller than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when left is smaller than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator<=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when left is smaller than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator<=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when left is larger than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator>(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically larger than the right character sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when left is larger than right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeLeft When set, the left string is a unicode string.
     * @tparam nFixedSizeLeft Fixed size of the left string.
     * @param[in] rssLeft Reference to the left string.
     * @param[in] szRight Pointer to the tero terminated right string.
     * @return Returns 'true' when left is larger than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeLeft, size_t nFixedSizeLeft>
    bool operator>=(const string_base<TCharType, bUnicodeLeft, nFixedSizeLeft>& rssLeft, const TCharType* szRight);

    /**
     * @brief Compare whether the left character sequence is lexicograpgically smaller than or equal to the right character
     * sequences.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicodeRight When set, the right string is a unicode string.
     * @tparam nFixedSizeRight Fixed size of the right string.
     * @param[in] szLeft Pointer to the tero terminated left string.
     * @param[in] rssRight Reference to the left string.
     * @return Returns 'true' when left is larger than or equal to right; 'false' otherwise.
     */
    template <typename TCharType, bool bUnicodeRight, size_t nFixedSizeRight>
    bool operator>=(const TCharType* szLeft, const string_base<TCharType, bUnicodeRight, nFixedSizeRight>& rssRight);

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @tparam TCharType The character type to use for the strings.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
    */
    template <typename TCharType>
    string MakeAnsiString(const TCharType* sz, size_t nCount = string::npos, char cFill = '_');

    /**
     * @brief Create Utf-8 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u8string MakeUtf8String(const TCharType* sz, size_t nCount = u8string::npos);

    /**
     * @brief Create Utf-16 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u16string MakeUtf16String(const TCharType* sz, size_t nCount = u16string::npos);

    /**
     * @brief Create Utf-32 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u32string MakeUtf32String(const TCharType* sz, size_t nCount = u32string::npos);

    /**
     * @brief Create wide string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @param[in] sz Pointer to a string.
     * @param[in] nCount The amount of characters in the string or 'npos' when the string is zero terminated.
     * @return Returns the created string.
     */
    template <typename TCharType>
    wstring MakeWString(const TCharType* sz, size_t nCount = wstring::npos);

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    string MakeAnsiString(const string_base<TCharType, bUnicode, nFixedSize>& rss, char cFill = '_');

    /**
     * @brief Create wide string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    wstring MakeWString(const string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Create Utf-8 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    u8string MakeUtf8String(const string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Create Utf-16 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    u16string MakeUtf16String(const string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Create Utf-32 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    u32string MakeUtf32String(const string_base<TCharType, bUnicode, nFixedSize>& rss);

    /**
     * @brief Create an ANSI (ISO/IEC 8859-1) string by providing strings of other types.
     * @remarks An automatic conversion from UTF-8 to ANSI is not possible (since both use the "char" data type).
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @param[in] cFill The filling character to add when a Unicode character was detected that was not fitting the ANSI character set.
     * @return Returns the created string.
     */
    template <typename TCharType>
    string MakeAnsiString(const std::basic_string<TCharType>& rss, char cFill = '_');

    /**
     * @brief Create wide string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType>
    wstring MakeWString(const std::basic_string<TCharType>& rss);

    /**
     * @brief Create Utf-8 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u8string MakeUtf8String(const std::basic_string<TCharType>& rss);

    /**
     * @brief Create Utf-16 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u16string MakeUtf16String(const std::basic_string<TCharType>& rss);

    /**
     * @brief Create Utf-32 string by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rss Reference to the string.
     * @return Returns the created string.
     */
    template <typename TCharType>
    u32string MakeUtf32String(const std::basic_string<TCharType>& rss);

    /**
    * @brief Generic string conversion.
    * @tparam TCharTypeSrc The character type to use for the source string.
    * @tparam bUnicodeSrc When set, the source string is a unicode string.
    * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the source string.
    * @tparam TCharTypeDst The character type to use for the destination string.
    * @tparam bUnicodeDst When set, the destination string is a unicode string.
    * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the destrination string.
    * @param[in] rss Reference to the string.
    * @return Returns the created string.
    */
    template <typename TCharTypeSrc, bool bUnicodeSrc, size_t nFixedSizeSrc, typename TCharTypeDst, bool bUnicodeDst, size_t nFixedSizeDst>
    string_base<TCharTypeDst, bUnicodeDst, nFixedSizeDst> MakeString(const string_base<TCharTypeSrc, bUnicodeSrc, nFixedSizeSrc>& rss);

    /**
    * @brief Generic string conversion.
    * @tparam TCharTypeSrc The character type to use for the source string.
    * @tparam TCharTypeDst The character type to use for the destination string.
    * @tparam bUnicodeDst When set, the destination string is a unicode string.
    * @tparam nFixedSizeSrc Size of the fixed size buffer or 0 for a dynamic sized buffer of the destrination string.
    * @param[in] rss Reference to the string.
    * @return Returns the created string.
    */
    template <typename TCharTypeSrc, typename TCharTypeDst, bool bUnicodeDst, size_t nFixedSizeDst>
    string_base<TCharTypeDst, bUnicodeDst, nFixedSizeDst> MakeString(const std::basic_string<TCharTypeSrc>& rss);

    /**
     * @brief Create path by providing strings of other types.
     * @tparam TCharType The character type to use for the strings.
     * @tparam bUnicode When set, the string is a unicode string.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     * @param[in] rssPath Reference to the string.
     * @return Returns the created path.
     */
    template <typename TCharType, bool bUnicode, size_t nFixedSize>
    std::filesystem::path MakePath(const string_base<TCharType, bUnicode, nFixedSize>& rssPath);

} // namespace sdv

#include "string.inl"

#endif // !defined SDV_STRING_H