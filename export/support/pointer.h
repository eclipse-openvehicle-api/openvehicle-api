#ifndef SDV_PTR_H
#define SDV_PTR_H

#include <ostream>
#include <atomic>
#include <algorithm>
#include "iterator.h"
#ifndef DONT_LOAD_CORE_TYPES
#include "../interfaces/core_types.h"
#endif

namespace sdv
{
    // Forward declaration
    template <typename, size_t>
    class pointer;

    /**
     * @brief Managed pointer class.
     * @details Buffer management class. There are two versions of buffer management, fixed buffer management (nFixedSize template
     * parameter larger than 0) and dynamic buffer management (nFixedSize template parameter is 0). The fixed buffer management
     * contains a fixed memory allocation of the amount of elements (capacity) and manages the size within this buffer. Copies of
     * this pointer class will copy the buffer as well. The dynamic buffer behaves like a shared pointer. Copies of the pointer class will
     * copy the pointer, but not the content. The lifetime of the content is managed through reference counter. The functions of
     * this class are similar to the functions of the std::shared_ptr class.
     * @tparam T Type to use for the buffer allocation.
     * @tparam nFixedSize Size of the fixed size buffer or 0 for a dynamic sized buffer.
     */
    template <typename T, size_t nFixedSize = 0>
    class pointer
    {
    public:
        /**
         * @brief The element type of this pointer.
         */
        using element_type = T;

        /**
         * @brief The size of this pointer allocation element in bytes.
         */
        static const size_t size_bytes = nFixedSize * sizeof(T);

        /**
         * @brief The size of this pointer allocation in bytes aligned to 64-bits.
         */
        static const size_t aligned_size_bytes = nFixedSize * sizeof(T) + ((nFixedSize * sizeof(T) % 8) ? (8 - nFixedSize * sizeof(T) % 8) : 0);

        /**
         * @brief Default constructor
        */
        pointer() noexcept;

        /**
         * @brief Copy constructor of same pointer type.
         * @param[in] rptr Reference to the pointer to copy.
         */
        pointer(const pointer& rptr);

        /**
         * @brief Copy constructor of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to copy.
         */
        template <size_t nFixedSize2>
        pointer(const pointer<T, nFixedSize2>& rptr);

        /**
         * @brief Move constructor of same pointer type.
         * @param[in] rptr Reference to the pointer to copy.
         */
        pointer(pointer&& rptr);

        /**
         * @brief Move constructor of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to copy.
         */
        template <size_t nFixedSize2>
        pointer(pointer<T, nFixedSize2>&& rptr);

        /**
         * @brief Destructor
         */
        ~pointer();

        /**
         * @brief Assignment operator of same pointer type.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        pointer& operator=(const pointer& rptr);

        /**
         * @brief Assignment operator of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        template <size_t nFixedSize2>
        pointer& operator=(const pointer<T, nFixedSize2>& rptr);

        /**
         * @brief Move operator of same pointer type.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        pointer& operator=(pointer&& rptr);

        /**
         * @brief Move operator of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        template <size_t nFixedSize2>
        pointer& operator=(pointer<T, nFixedSize2>&& rptr);

        /**
         * @brief Reduces the reference count and if zero deletes the allocation. Clears the pointer.
         */
        void reset();

        /**
         * @brief Swaps this pointer with the provided pointer.
         * tparam nFixedSizeRight The fixed size of the provided pointer.
         * @param[in] rptr Reference to the provided pointer to swap with.
         */
        template <size_t nFixedSizeRight>
        void swap(pointer<T, nFixedSizeRight>& rptr);

        /**
         * @brief Get access to the underlying buffer.
         * @return Pointer to the buffer.
        */
        element_type* get() const noexcept;

        /**
         * @brief Get a reference to the pointer.
         * @return Reference to the value.
         */
        T& operator*() const;

        /**
         * @brief Get a reference to the pointer.
         * @return Pointer to the value.
        */
        T* operator->() const;

        /**
         * @brief Get the indexed value of the pointer.
         * @param[in] nIndex Index to request the value of.
         * @return Reference to the indexed value.
        */
        element_type& operator[](size_t nIndex) const;

        /**
         * @brief Return whether a pointer is assigned.
         * @return Returns 'true' when a pointer is assigned; 'false' otherwise.
        */
        operator bool() const noexcept;

        /**
         * @brief Return the amount of elements allocated for this pointer.
         * @return The size of the pointer.
        */
        size_t size() const noexcept;

        /**
         * @brief Resize the pointer.
         * @param[in] nSize Size of the new pointer.
        */
        void resize(size_t nSize);

        /**
         * @brief Return the capacity (in amount of elements) this buffer can hold.
         * @return The capacity of the buffer pointed to by this pointer.
         */
        size_t capacity() const noexcept;

    private:
        uint32_t m_uiSize = 0;                  ///< Amount of elements allocated.
        uint32_t m_uiReserved = 0;              ///< Reserved for alignment purposes.
        union
        {
            T m_rgtData[nFixedSize] = {};               ///< The size of the buffer.
            uint8_t m_rguiBuffer[aligned_size_bytes];   ///< The buffer aligned to 64 bits.
        };
    };

    namespace internal
    {
        /// @cond DOXYGEN_IGNORE
        // Forward declaration
        interface IInternalMemAlloc;
        template <typename T>
        pointer<T, 0> make_ptr(IInternalMemAlloc* pAllocator, size_t nSize = 1);
        /// @endcond
    }

    /**
     * @brief Create a pointer class using the allocator and allocating the memory with the given size.
     * @tparam T Type to use for the allocation.
     * @param[in] nSize Size to allocate (in elements).
     * @return The pointer class.
     */
    template <typename T>
    pointer<T> make_ptr(size_t nSize = 1);

    /**
     * @brief Specialization for dynamic sized buffer management.
     * @tparam T Type to use for the buffer allocation.
     */
    template <typename T>
    class pointer<T, 0>
    {
        friend pointer<T> internal::make_ptr<T>(internal::IInternalMemAlloc* pAllocator, size_t nSize /*= 1*/);
        friend pointer<T> make_ptr<T>(size_t);

    public:
        /**
         * @brief The element type of this pointer.
         */
        using element_type = T;

        /**
         * @brief Default constructor
         */
        pointer() noexcept;

        /**
         * @brief Copy constructor of same pointer type.
         * @param[in] rptr Reference to the pointer to copy.
         */
        pointer(const pointer& rptr);

        /**
         * @brief Copy constructor of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to copy.
         */
        template <size_t nFixedSize2>
        pointer(const pointer<T, nFixedSize2>& rptr);

        /**
         * @brief Move constructor
         * @param[in] rptr Reference to the pointer to move.
         */
        pointer(pointer&& rptr) noexcept;

        /**
         * @brief Move constructor of other pointer types.
         * @tparam nFixedSize2 The fixed size of the provided pointer.
         * @param[in] rptr Reference to the pointer to copy.
         */
        template <size_t nFixedSize2>
        pointer(const pointer<T, nFixedSize2>&& rptr);

        /**
         * @brief Destructor
         */
        ~pointer();

        /**
         * @brief Assignment operator of same pointer type.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        pointer& operator=(const pointer& rptr);

        /**
         * @brief Assignment operator of other pointer types.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        template <size_t nFixedSize2>
        pointer& operator=(const pointer<T, nFixedSize2>& rptr);

        /**
         * @brief Move operator of same pointer type.
         * @param[in] rptr Reference to the pointer to move.
         * @return Reference to this pointer.
         */
        pointer& operator=(pointer&& rptr) noexcept;

        /**
         * @brief Move operator of other pointer types.
         * @param[in] rptr Reference to the pointer to assign.
         * @return Reference to this pointer.
         */
        template <size_t nFixedSize2>
        pointer& operator=(pointer<T, nFixedSize2>&& rptr);

    protected:
        /**
         * @brief Attach an uint8_t buffer.
         * @param[in] rptrBuffer Reference to the pointer to attach.
         */
        void attach(pointer<uint8_t>&& rptrBuffer);

        /**
         * @brief Detach the uint8_t buffer.
         * @return Reference to the pointer to detach.
         */
        pointer<uint8_t>&& detach();

    public:
        /**
         * @brief Reduces the reference count and if zero deletes the allocation. Clears the pointer.
         */
        void reset();

        /**
         * @brief Swaps this pointer with the provided pointer.
         * tparam nFixedSizeRight The fixed size of the provided pointer.
         * @param[in] rptr Reference to the provided pointer to swap with.
         */
        template <size_t nFixedSizeRight>
        void swap(pointer<T, nFixedSizeRight>& rptr);

        /**
         * @brief Get access to the underlying buffer.
         * @return Pointer to the buffer.
         */
        element_type* get() const noexcept;

        /**
         * @brief Get a reference to the pointer.
         * @return Reference to the value.
         */
        T& operator*() const;

        /**
         * @brief Get a reference to the pointer.
         * @return Pointer to the value.
         */
        T* operator->() const;

        /**
         * @brief Get the indexed value of the pointer.
         * @param[in] nIndex Index to request the value of.
         * @return Reference to the indexed value.
         */
        element_type& operator[](size_t nIndex) const;

        /**
         * @brief Return whether a pointer is assigned.
         * @return Returns 'true' when a pointer is assigned; 'false' otherwise.
         */
        operator bool() const noexcept;

        /**
         * @brief Return the amount of elements allocated for this pointer.
         * @return The size of the pointer.
         */
        size_t size() const noexcept;

        /**
         * @brief Resize the pointer.
         * @attention A call to this function is not synchronized with other functions.
         * @param[in] nSize Size of the new pointer.
         */
        void resize(size_t nSize);

        /**
         * @brief Return the capacity (in amount of elements) this buffer can hold.
         * @return The capacity of the buffer pointed to by this pointer.
         */
        size_t capacity() const noexcept;

        /**
         * @brief Get reference count on current shares.
         * @return Reference count.
         */
        size_t ref_count() const noexcept;

    private:
        /**
         * @brief The internal data structure holding the pointer information.
         */
        struct SAllocation
        {
            internal::IInternalMemAlloc* pAllocator = nullptr;  ///< Pointer to the memory allocator.
            T*                           pData      = nullptr;  ///< Pointer to the data.
            uint32_t                     uiSize     = 0;        ///< Amount of bytes allocated.
            std::atomic_uint32_t         uiRefCnt   = 0;        ///< Reference count.
        };

        SAllocation* m_psAllocation = nullptr; ///< Actual pointer to the allocation
    };

    /**
     * @brief Swap two pointers.
     * @tparam T Type to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the first pointer.
     * @param[in] rptrRight Reference to the second pointer.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    void swap(pointer<T, nFixedSizeLeft>& rptrLeft, pointer<T, nFixedSizeRight>& rptrRight);

    /**
     * @brief Compare two pointers for equality.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when both pointers are equal; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator==(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare two pointers for inequality.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when both pointers are not equal; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator!=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether one pointer is smaller than another pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when the left pointer is smaller than the right pointer; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator<(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether one pointer is smaller than or equal to another pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when the left pointer is smaller than or equal to the right pointer; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator<=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether one pointer is larger than another pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when the left pointer is larger than the right pointer; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator>(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether one pointer is larger than or equal to another pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @return Returns 'true' when the left pointer is larger than or equal to the right pointer; false when not.
     */
    template <class T, size_t nFixedSizeLeft, size_t nFixedSizeRight>
    bool operator>=(const pointer<T, nFixedSizeLeft>& rptrLeft, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is equal too NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer equals NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator==(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the right pointer is equal too NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the right pointer equals NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator==(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is not equal too NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer does not equal to NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator!=(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the right pointer is not equal too NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the right pointer does not equal to NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator!=(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is smaller than NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer is smaller than NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator<(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the NULL is smaller than the left pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if NULL is smaller than the left pointer; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator<(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is smaller than or equal to NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer is smaller than or equal to NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator<=(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the NULL is smaller than or equal to the left pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if NULL is smaller than or equal to the left pointer; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator<=(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is larger than NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer is larger than NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator>(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the NULL is larger than the left pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if NULL is larger than the left pointer; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator>(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Compare whether the left pointer is smaller than or equal to NULL.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeLeft The fixed size of the left pointer.
     * @param[in] rptrLeft Reference to the left pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if the left pointer is smaller than or equal to NULL; 'false' when not.
     */
    template <class T, size_t nFixedSizeLeft>
    bool operator>=(const pointer<T, nFixedSizeLeft>& rptrLeft, std::nullptr_t pNull) noexcept;

    /**
     * @brief Compare whether the NULL is smaller than or equal to the left pointer.
     * @tparam T Type of the pointer to use as a base.
     * @tparam nFixedSizeRight The fixed size of the right pointer.
     * @param[in] rptrRight Reference to the right pointer.
     * @param[in] pNull Pointer to a nullptr.
     * @return Returns 'true' if NULL is smaller than or equal to the left pointer; 'false' when not.
     */
    template <class T, size_t nFixedSizeRight>
    bool operator>=(std::nullptr_t pNull, const pointer<T, nFixedSizeRight>& rptrRight) noexcept;

    /**
     * @brief Stream the value of a the pointer to an output stream.
     * @tparam T Type of pointer.
     * @tparam U Character type of output stream.
     * @tparam V Character traits of output stream.
     * @tparam nFixedSize The fixed size of the pointer.
     * @param[in] rstream Reference to the stream.
     * @param[in] rptr Reference to the pointer.
     * @return Reference to the stream.
    */
    template <class T, class U, class V, size_t nFixedSize>
    std::basic_ostream<U, V>& operator<<(std::basic_ostream<U, V>& rstream, const pointer<T, nFixedSize>& rptr);

    /**
     * @brief C++ library stream support - reading from stream.
     * @tparam TTraits Character traits of the stream buffer.
     * @param[in, out] pStreamBuf Pointer to the stream buffer to read from.
     * @return Pointer object created from the stream buffer.
     */
    template <typename TTraits = std::char_traits<char>>
    pointer<uint8_t> from_stream(std::basic_streambuf<char, TTraits>* pStreamBuf);

    /**
     * @brief C++ library stream support - writing to stream.
     * @tparam TTraits Character traits of the stream buffer.
     * @param[in] rptr Reference to the pointer to write to the stream.
     * @param[in, out] pStreamBuf Pointer to the stream buffer to write to.
     */
    template <typename TTraits = std::char_traits<char>>
    void to_stream(const pointer<uint8_t>& rptr, std::basic_streambuf<char, TTraits>* pStreamBuf);

    /**
     * @brief For the uint8_t pointer a specific cast operation exists.
     * @attention Use with care. There is no guarantee that the pointer corresponds to the layout of the requested type. If
     * possible use serialization instead.
     * @tparam T Type to cast to. T must be a standard-layout-type.
     * @param[in] rptr Reference to the pointer containing the data buffer.
     * @param[in] nOffset Offset the type starts.
     * @return Returns a casted pointer if the buffer is large enough (offset + sizeof(T)); NULL otherwise.
     */
    template <typename T>
    T* cast(pointer<uint8_t>& rptr, size_t nOffset = 0);

    /**
    * @brief For the uint8_t pointer a specific cast operation exists.
    * @attention Use with care. There is no guarantee that the pointer corresponds to the layout of the requested type. If
    * possible use serialization instead.
    * @tparam T Type to cast to. T must be a standard-layout-type.
    * @param[in] rptr Reference to the pointer containing the data buffer.
    * @param[in] nOffset Offset the type starts.
    * @return Returns a casted pointer if the buffer is large enough (offset + sizeof(T)); NULL otherwise.
    */
    template <typename T>
    const T* cast(const pointer<uint8_t>& rptr, size_t nOffset = 0);

} // namespace sdv

#include "pointer.inl"

#endif // !defined SDV_PTR_H