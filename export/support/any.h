#ifndef SDV_ANY_H
#define SDV_ANY_H

#include <string>
#include <algorithm>
#include <ostream>
#include <istream>
#include <filesystem>
#include "string.h"
#include "interface.h"
#include "except.h"

namespace sdv
{
    /**
     * @brief The 'any' class implementation.
     */
    class any_t
    {
    public:
        /**
         * @brief Value type enumeration
         */
        enum class EValType : uint32_t
        {
            val_type_empty = 0,             ///< Empty value
            val_type_bool = 1,              ///< Boolean value; bVal is used.
            val_type_int8 = 8,              ///< 8-Bit signed value; i8Val is used.
            val_type_uint8 = 9,             ///< 8-Bit unsigned value; ui8Val is used.
            val_type_int16 = 16,            ///< 16-Bit signed value; i16Val is used.
            val_type_uint16 = 17,           ///< 16-Bit unsigned value; ui16Val is used.
            val_type_int32 = 32,            ///< 32-Bit signed value; i32Val is used.
            val_type_uint32 = 33,           ///< 32-Bit unsigned value; ui32Val is used.
            val_type_int64 = 64,            ///< 64-Bit signed value; i64Val is used.
            val_type_uint64 = 65,           ///< 64-Bit unsigned value; ui64Val is used.
            val_type_char = 100,            ///< ANSI Character value; cVal is used.
            val_type_char16 = 116,          ///< UTF-16 Character value; c16Val is used.
            val_type_char32 = 132,          ///< UTF-32 Character value; c32Val is used.
            val_type_wchar = 101,           ///< Wide character value; cwVal is used.
            val_type_float = 232,           ///< Singled precision floating point value; fVal is used.
            val_type_double = 264,          ///< Double precision floating point value; dVal is used.
            val_type_long_double = 265,     ///< Long double precision floating point value; ldVal is used.
            val_type_fixed = 300,           ///< Fixed point value; fixValue is used.
            val_type_string = 1000,         ///< ANSI string value; ssVal is used.
            val_type_u8string = 1008,       ///< UTF-8 string value; ss8Val is used.
            val_type_u16string = 1016,      ///< UTF-16 string value; ss16Val is used.
            val_type_u32string = 1032,      ///< UTF-32 string value; ss32Val is used.
            val_type_wstring = 1001,        ///< Wide string value; sswVal is used.
            val_type_interface = 2000,      ///< Interface type value; ifcVal is used.
            val_type_interface_id = 2001,   ///< Interface id value; idIfcVal is used.
            val_type_exception_id = 3000,   ///< Exception id value; idExceptVal is used.
        } eValType = EValType::val_type_empty;     ///< The value type

        /// Anonymous union
        union
        {
            bool            bVal;           ///< Boolean value
            int8_t          i8Val;          ///< 8-Bit signed value
            uint8_t         ui8Val;         ///< 8-Bit unsigned value
            int16_t         i16Val;         ///< 16-Bit signed value
            uint16_t        ui16Val;        ///< 16-Bit unsigned value
            int32_t         i32Val;         ///< 32-Bit signed value
            uint32_t        ui32Val;        ///< 32-Bit unsigned value
            int64_t         i64Val;         ///< 64-Bit signed value
            uint64_t        ui64Val;        ///< 64-Bit unsigned value
            char            cVal;           ///< ANSI Character value
            char16_t        c16Val;         ///< UTF-16 Character value
            char32_t        c32Val;         ///< UTF-32 Character value
            wchar_t         cwVal;          ///< Wide character value
            float           fVal;           ///< Singled precision floating point value
            double          dVal;           ///< Double precision floating point value
            long double     ldVal;          ///< Long double precision floating point value
            //fixed           fixValue;       ///< Fixed point value
            string          ssVal;          ///< ANSI string value
            u8string        ss8Val;         ///< UTF-8 string value
            u16string       ss16Val;        ///< UTF-16 string value
            u32string       ss32Val;        ///< UTF-32 string value
            wstring         sswVal;         ///< Wide string value
            interface_t     ifcVal;         ///< Interface type value
            interface_id    idIfcVal;       ///< Interface id value
            exception_id    idExceptVal;    ///< Exception id value
        };

        /**
         * @brief Default constructor; construct an empty any
         */
        any_t();

        /**
         * @brief destructor
         */
        ~any_t();

        /**
         * @brief Assignment constructor.
         * @param[in] tVal The value to assign.
         */
        template <typename TType>
        explicit any_t(TType tVal);

        /**
        * @{
        * @brief SDV string constructors.
        * @param[in] rssVal Reference to the string object.
        */
        explicit any_t(const string& rssVal);
        explicit any_t(const u8string& rssVal);
        explicit any_t(const u16string& rssVal);
        explicit any_t(const u32string& rssVal);
        explicit any_t(const wstring& rssVal);
        /**
        * @}
        */

        /**
         * @brief C-style string constructors.
         * @param[in] sz Zero terminated string.
         */
        any_t(const char* sz);

        /**
         * @brief C-style string constructors.
         * @param[in] sz Zero terminated string.
         */
        any_t(const char16_t* sz);

        /**
         * @brief C-style string constructors.
         * @param[in] sz Zero terminated string.
         */
        any_t(const char32_t* sz);

        /**
         * @brief C-style string constructors.
         * @param[in] sz Zero terminated string.
         */
        any_t(const wchar_t* sz);

        /**
        * @{
        * @brief STD string constructors.
        * @param[in] rssVal Reference to the string object.
        */
        explicit any_t(const std::string& rssVal);
        explicit any_t(const std::u16string& rssVal);
        explicit any_t(const std::u32string& rssVal);
        explicit any_t(const std::wstring& rssVal);
        /**
        * @}
        */

        /**
        * @brief Assignment constructor.
        * @param[in] tVal The value to assign.
        * @param[in] eValTypeParam The target value type.
        */
        template <typename TType>
        any_t(TType tVal, EValType eValTypeParam);

        /**
        * @brief Copy constructor
        * @param[in] rany Reference to any class to copy from.
        */
        any_t(const any_t& rany);

        /**
        * @brief Move constructor.
        * @param[in] rany Reference to any class to move from.
        */
        any_t(any_t&& rany) noexcept;

        /**
         * @brief Assignment operator.
         * @param[in] tVal The value to assign.
         * @return Reference to this class.
         */
        template <typename TType>
        any_t& operator=(TType tVal);

        /**
         * @brief Copy assignment operator.
         * @param[in] rany Reference to any class to copy from.
         * @return Reference to this class.
         */
        any_t& operator=(const any_t& rany);

        /**
        * @brief Move operator.
        * @param[in] rany Reference to any class to move from.
        * @return Reference to this class.
        */
        any_t& operator=(any_t&& rany) noexcept;

        /**
         * @{
         * @brief Cast operators (provides conversion if not identical to the original type).
         * @remarks The operators for ANSI string, interface id and exception id are rerpresented by u8string and uint64_t
         * respectively and cannot be used to cast to automatically.
         * @return The value stored.
         */
        operator bool() const;
        operator int8_t() const;
        operator uint8_t() const;
        operator int16_t() const;
        operator uint16_t() const;
        operator int32_t() const;
        operator uint32_t() const;
        operator int64_t() const;
        operator uint64_t() const;
#ifdef __linux__
        operator long long int() const { return static_cast<long long int>(operator int64_t()); }
        operator unsigned long long int() const { return static_cast<long long int>(operator uint64_t()); }
#endif
        operator char() const;
        operator char16_t() const;
        operator char32_t() const;
        operator wchar_t() const;
        operator float() const;
        operator double() const;
        operator long double() const;
        //operator fixed() const;
        operator string() const;
        operator u8string() const;
        operator u16string() const;
        operator u32string() const;
        operator wstring() const;
        operator interface_t() const;
        //operator interface_id() const;
        //operator exception_id() const;
        operator std::string() const;
        operator std::u16string() const;
        operator std::u32string() const;
        operator std::wstring() const;
        /**
         * @}
         */

        /**
         * @brief Check for an empty any.
         * @return Retrurns whether the any is empty.
        */
        bool empty() const;

        /**
         * @brief Empty the any.
         */
        void clear();

        /**
         * @brief Assign the value to the any. The any takes the value type based on the value.
         * @tparam TType The type of the value to set.
         * @param[in] tVal The value to set.
         */
        template <typename TType>
        void set(TType tVal);

        /**
         * @brief Assign the value to the any. The value will be converted to the provided value type.
         * @tparam TType The type of the value to set.
         * @param[in] tVal The valiue to set.
         * @param[in] eValTypeParam The value type of the any.
        */
        template <typename TType>
        void set(TType tVal, EValType eValTypeParam);

        /**
         * @brief Get the value converted to the provided type.
         * @tparam TType The type of the value to get,
         * @return The value to get.
        */
        template <typename TType>
        TType get() const;

        /**
         * @brief Comparison type.
        */
        enum class ECompareType
        {
            compare_equal,
            compare_inequal,
            compare_smaller,
            compare_smaller_equal,
            compare_larger,
            compare_larger_equal,
        };

        /**
         * @brief Compare the provided value with the contained value.
         * @tparam TType Type of the supplied value.
         * @tparam eType Type of comparison to do.
         * @param[in] rtVal Reference to the value to use for comparison.
         * @return The result of the comparison.
         */
        template <typename TType, ECompareType eType>
        bool Compare(const TType& rtVal) const;

        /**
         * @brief Compare the provided value with the contained value.
         * @tparam eType Type of comparison to do.
         * @param[in] ranyVal Reference to the value to use for comparison.
         * @return The result of the comparison.
         */
        template <ECompareType eType>
        bool Compare(const any_t& ranyVal) const;

    private:
        /**
         * @brief Convert the value from one type to the other.
         * @remarks This function doesn't check for correct sizes. Invalid conversions lead to an empty destination.
         * @tparam TSourceType The source type to convert from.
         * @tparam TDestTyoe The destination type to convert to.
         * @param[in] rtSrcVal Reference to the source value.
         * @param[in] rtDstVal Reference to the destination value.
         */
        template <typename TSourceType, typename TDestType>
        static void convert(const TSourceType& rtSrcVal, TDestType& rtDstVal);
    };

    /**
     * @brief Equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator==(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator==(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Equality comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator==(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

    /**
     * @brief Inequality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator!=(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Inequality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator!=(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Inequality comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator!=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

    /**
     * @brief Smaller than comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator<(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Smaller than comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator<(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Smaller than comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator<(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

    /**
     * @brief Smaller than or equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator<=(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Smaller than or equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator<=(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Smaller than or equality comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator<=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

    /**
     * @brief Greater than comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator>(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Greater than comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator>(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Greater than comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator>(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

    /**
     * @brief Greater than or equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] ranyVal1 Reference to the any value to compare with.
     * @param[in] tVal2 Value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator>=(const sdv::any_t& ranyVal1, TType tVal2);

    /**
     * @brief Greater than or equality comparison operator.
     * @tparam TType The type to use for the comparison.
     * @param[in] tVal1 Value to compare with.
     * @param[in] ranyVal2 Reference to the any value to compare with.
     * @return The result of the comparison.
     */
    template <typename TType>
    bool operator>=(TType tVal1, const sdv::any_t& ranyVal2);

    /**
    * @brief Greater than or equality comparison operator.
    * @param[in] ranyVal1 Reference to the any value to compare with.
    * @param[in] ranyVal2 Reference to the any value to compare with.
    * @return The result of the comparison.
    */
    bool operator>=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2);

}

#include "any.inl"

#endif // !defined SDV_ANY_H