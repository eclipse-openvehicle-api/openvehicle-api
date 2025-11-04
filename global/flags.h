#ifndef FLAGS_HELPER_H
#define FLAGS_HELPER_H

#include <type_traits>
#include <initializer_list>

/**
 * @brief Helper namespace
*/
namespace hlpr
{
    /**
     * @brief Generic flags type class based on flags defined in an enum.
     * @tparam TEnum The enum type that is used to define flags.
     */
    template <typename TEnum>
    struct flags
    {
        // Only works with enums.
        static_assert(std::is_enum_v<TEnum>);

        /// The enum base type.
        using enum_type = std::underlying_type_t<TEnum>;

        /**
         * @brief Default constructor
         */
        flags() = default;

        /**
         * @brief Copy constructor
         * @param[in] rflags Reference to the flags to copy from.
         */
        flags(const flags& rflags) = default;

        /**
         * @brief Move constructor
         * @param[in] rflags Reference to the flags to move from.
         */
        flags(flags&& rflags) : _tValue(rflags._tValue) { rflags._tValue = 0; }

        /**
         * @brief Constructor with flags value assignment.
         * @param[in] tValue The flags value to assign.
         */
        explicit flags(enum_type tValue) : _tValue(tValue) {}

        /**
         * @brief Constructor with single value assignment.
         * @param[in] eValue The single value to assign.
         */
        flags(TEnum eValue) : _tValue(static_cast<enum_type>(eValue)) {}

        /**
         * @brief Constructor with initializer list
         * @param[in] init The initializer list.
         */
        flags(std::initializer_list<TEnum> init)
        {
            for (TEnum eValue : init)
                _tValue |= static_cast<enum_type>(eValue);
        }

        /**
         * @brief Flags class assignment.
         * @param[in] rflags Reference to the flags to copy from.
         * @return Returns a reference to this class.
         */
        flags& operator=(const flags& rflags) = default;

        /**
         * @brief Flags move assignment.
         * @param[in] rflags Reference to the flags to move from.
         * @return Returns a reference to this class.
         */
        flags& operator=(flags&& rflags) { _tValue = rflags._tValue; rflags._tValue = 0; return *this; }

        /**
         * @brief Flags value assignment.
         * @param[in] tValue The flags value to assign.
         * @return Returns a reference to this class.
         */
        flags& operator=(enum_type tValue) { _tValue = tValue; return *this; }

        /**
         * @brief Single value assignment.
         * @param[in] eValue The single value to assign.
         * @return Returns a reference to this class.
         */
        flags& operator=(TEnum eValue) { _tValue = static_cast<enum_type>(eValue); return *this; }

        /**
         * @brief Cast operator
         * @return The flags value.
         */
        operator enum_type() const { return _tValue; }

        /**
         * @brief Add a flag.
         * @param[in] eValue The flag value to set.
         * @return Reference to this class.
         */
        flags& operator+=(TEnum eValue) { _tValue |= static_cast<enum_type>(eValue); return *this; }

        /**
         * @brief Remove a flag.
         * @param[in] eValue The flag value to remove.
         * @return Reference to this class.
         */
        flags& operator-=(TEnum eValue) { _tValue &= ~static_cast<enum_type>(eValue); return *this; }

        /**
         * @brief Add a flag to a value copy.
         * @param[in] eValue The flag value to set.
         * @return The result flags.
         */
        flags operator+(TEnum eValue) { flags flagsCopy(_tValue); flagsCopy.add(eValue); return flagsCopy; }

        /**
         * @brief Remove a flag from a value copy.
         * @param[in] eValue The flag value to remove.
         * @return The result flags.
         */
        flags operator-(TEnum eValue) { flags flagsCopy(_tValue); flagsCopy.remove(eValue); return flagsCopy; }

        /**
         * @brief Check the flags for the availability of the flag.
         * @param[in] eValue The flag to check for.
         * @return Returns whether the flag was set or not.
         */
        bool operator&(TEnum eValue) const { return (_tValue & static_cast<enum_type>(eValue)) ? true : false; }

        /**
         * @brief Return the flags value.
         * @return The flags value.
         */
        enum_type get() const { return _tValue; }

        /**
         * @brief Add a flag.
         * @param[in] eValue The flag value to set.
         */
        void add(TEnum eValue) { _tValue |= static_cast<enum_type>(eValue); }

        /**
         * @brief Remove a flag.
         * @param[in] eValue The flag value to remove.
         */
        void remove(TEnum eValue) { _tValue &= ~static_cast<enum_type>(eValue); }

        /**
         * @brief Check the flags for the availability of the flag.
         * @param[in] eValue The flag to check for.
         * @return Returns whether the flag was set or not.
         */
        bool check(TEnum eValue) const { return _tValue & static_cast<enum_type>(eValue); }

        /**
         * @brief Check the flags for the availability of at least one flag.
         * @param[in] flagsValue The flags to use for the check.
         * @return Returns whether at least one flag was set.
         */
        bool check_any(flags flagsValue) const { return _tValue & flagsValue._tValue; }

        /**
         * @brief Check the flags for the availability of all supplied flags.
         * @remarks Check for the supplied flags. Additional flags might also be set, but have no influence on the outcome.
         * @param[in] flagsValue The flags to use for the check.
         * @return Returns whether all supplied flags were set.
         */
        bool check_all(flags flagsValue) const { return (_tValue & flagsValue._tValue) == flagsValue._tValue; }

        enum_type _tValue = 0;      ///< The flag value.
    };
}

#endif // !defined FLAGS_HELPER_H
