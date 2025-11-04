#ifndef SDV_INTERFACE_H
#define SDV_INTERFACE_H

#include <cstdint>

#ifndef interface
/** Define the interface keyword */
#define interface struct
#endif

namespace sdv
{
    /**
     * @brief Interface ID type
     */
    using interface_id = uint64_t;

    /**
     * @brief Get the interface ID.
     * @tparam TIfc The interface type
     * @return The ID of the interface
     */
    template <typename TIfc>
    constexpr inline interface_id GetInterfaceId() noexcept
    {
        // Return the interface ID.
        return TIfc::_id;
    }

    /**
     * @brief Interface type class
     */
    class interface_t
    {
    public:
        /**
         * @brief Default constructor
         */
        interface_t() noexcept;

        /**
         * @brief Copy constructor
         * @param[in] rifc Reference to the interface to copy the pointer from.
         */
        interface_t(const interface_t& rifc) noexcept;

        /**
         * @brief Move constructor
         * @param[in] rifc Reference to the interface to move the pointer from.
         */
        interface_t(interface_t&& rifc) noexcept;

        /**
         * @brief Null-pointer constructor
         */
        interface_t(std::nullptr_t) noexcept;

        /**
         * @brief Assignment constructor
         * @tparam TInterface Interface type
         * @param[in] pInterface Interface pointer
         */
        template <typename TInterface>
        interface_t(TInterface* pInterface) noexcept;

        /**
         * @brief Copy assignment
         * @param[in] rifc Reference to the interface to copy the pointer from.
         * @return Reference to this interface type.
         */
        interface_t& operator=(const interface_t& rifc) noexcept;

        /**
         * @brief Move assignment
         * @param[in] rifc Reference to the interface to move the pointer from.
         * @return Reference to this interface type.
         */
        interface_t& operator=(interface_t&& rifc) noexcept;

        /**
         * @brief Null-pointer assignment
         * @return Reference to this interface type.
         */
        interface_t& operator=(std::nullptr_t) noexcept;

        /**
         * @brief Interface assignment operator
         * @tparam TInterface Interface type
         * @param[in] pInterface Interface pointer
         * @return Reference to this interface type.
         */
        template <typename TInterface>
        interface_t& operator=(TInterface* pInterface) noexcept;

        /**
         * @brief Boolean cast operator. Returns whether an interface (!= nullptr) has been assigned.
         * @return Returns 'true' when assigned; 'false' when not.
         */
        operator bool() const noexcept;

        /**
         * @brief Reset the interface pointer.
         */
        void reset() noexcept;

        /**
         * @brief Get the interface ID.
         * @return The ID of the stored interface.
         */
        interface_id id() const noexcept;

        /**
         * @brief Get a pointer to the stored interface.
         * @tparam TInterface The interface to get the pointer for.
         * @return Returns the stored interface pointer if the ID of the requested interface corresponds to the stored interface.
         * Returns a NULL-pointer if not.
         */
        template <typename TInterface>
        TInterface* get() noexcept;

        /**
         * @brief Get a pointer to the stored interface.
         * @tparam TInterface The interface to get the pointer for.
         * @return Returns the stored interface pointer if the ID of the requested interface corresponds to the stored interface.
         * Returns a NULL-pointer if not.
         */
        template <typename TInterface>
        const TInterface* get() const noexcept;

        /**
         * @brief Compare the interface with another interface.
         * @param[in] rifc Reference to the interface to compare with.
         * @return Returns negative when the stored interface is smaller, 0 when the interfaces are equal and positive when the
         * stored interface is larger.
         */
        int compare(const interface_t& rifc) const noexcept;

    private:
        interface_id    m_id = 0ull;            ///< The interface ID of the stored interface.
        void*           m_pInterface = nullptr; ///< Interface pointer.
    };

    /**
     * @brief Compare two interfaces for equality.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether both interfaces are equal.
     */
    bool operator==(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare the interface for equality to NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the interface is a NULL-pointer.
     */
    bool operator==(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare the interface for equality to NULL-pointer.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the interface is a NULL-pointer.
     */
    bool operator==(std::nullptr_t, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare two interfaces for in-equality.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether both interfaces are not equal.
     */
    bool operator!=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare the interfaces for in-equality to NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the interface is not a NULL-pointer.
     */
    bool operator!=(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare the interfaces for in-equality to NULL-pointer.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the interface is not a NULL-pointer.
     */
    bool operator!=(std::nullptr_t, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is smaller than the right interface.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the left interface is smaller.
     */
    bool operator<(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is smaller than a NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the left interface is smaller.
     */
    bool operator<(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare whether a NULL-pointer is smaller than the right interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the NULL-pointer is smaller.
     */
    bool operator<(std::nullptr_t, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is smaller than or equal to the right interface.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the left interface is smaller or equal.
     */
    bool operator<=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is smaller than or equal to a NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the left interface is smaller.
     */
    bool operator<=(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare whether a NULL-pointer is smaller than or equal to the right interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the NULL-pointer is smaller or equal.
     */
    bool operator<=(std::nullptr_t, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is larger than the right interface.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the left interface is larger.
     */
    bool operator>(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is larger than a NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the left interface is larger.
     */
    bool operator>(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare whether a NULL-pointer is larger than the right interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the NULL-pointer is larger.
     */
    bool operator>(std::nullptr_t, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is larger than or equal to the right interface.
     * @param[in] rifcLeft Reference to the left interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the left interface is larger or equal.
     */
    bool operator>=(const interface_t& rifcLeft, const interface_t& rifcRight) noexcept;

    /**
     * @brief Compare whether the left interface is larger than or equal to a NULL-pointer.
     * @param[in] rifcLeft Reference to the left interface.
     * @return Returns whether the left interface is larger or equal.
     */
    bool operator>=(const interface_t& rifcLeft, std::nullptr_t) noexcept;

    /**
     * @brief Compare whether a NULL-pointer is larger than or equal to the right interface.
     * @param[in] rifcRight Reference to the right interface.
     * @return Returns whether the NULL-pointer larger or equal.
     */
    bool operator>=(std::nullptr_t, const interface_t& rifcRight) noexcept;

} // namespace idl

#include "interface.inl"

#endif // !defined SDV_INTERFACE_H