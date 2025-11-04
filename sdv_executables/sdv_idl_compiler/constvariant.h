#ifndef VARIANT_H
#define VARIANT_H

#include <variant>
#include <string>
#include <type_traits>
#include <limits>
#include <cstdint>

/**
 * @brief Fixed data type is based on the double data type. Highest value is 31 digits positive and lowest 31 digits negative. The
 * best precision is 1/31.
 */
struct fixed
{
    double dValue;      ///< The value
    /**
     * @brief Constructor
     * @tparam TType Type of the value to create a fixed type of
     * @param[in] tValue The value to assign
     */
    template <typename TType>
    constexpr fixed(TType tValue) : dValue(static_cast<double>(tValue)) {}
    /**
     * @brief Assignment operator
     * @tparam TType Type of the value to assign
     * @param[in] tValue The value to assign
     * @return Reference to this class
     */
    template <typename TType>
    constexpr fixed& operator=(TType tValue) { dValue = tValue; return *this; }
    /**
     * @{
     * @brief Contained value as double
     * @return The value
     */
    constexpr operator double() const {return dValue;}
    constexpr operator const double&() const {return dValue;}
    constexpr operator double&() {return dValue;}
    /**
     * @}
     */
    /**
     * @brief Unary operator to return a fixed type.
     * @return the fixed type as a copy from this class.
     */
    fixed constexpr operator+() const {return *this;}
    /**
    * @brief Unary operator to return a negative fixed type.
    * @return the fixed type as a copy from this class.
    */
    fixed constexpr operator-() const {return fixed(-dValue);}
};

/**
 * @brief Add two fixed types
 * @param[in] val1 Value 1
 * @param[in] val2 Value 2
 * @return The result
 */
inline fixed operator+(fixed val1, fixed val2) { return val1.dValue + val2.dValue; }
/**
* @brief Subtract two fixed types
* @param[in] val1 Value 1
* @param[in] val2 Value 2
* @return The result
*/
inline fixed operator-(fixed val1, fixed val2) { return val1.dValue - val2.dValue; }
/**
* @brief Multiply two fixed types
* @param[in] val1 Value 1
* @param[in] val2 Value 2
* @return The result
*/
inline fixed operator*(fixed val1, fixed val2) { return val1.dValue * val2.dValue; }
/**
* @brief Divide two fixed types
* @param[in] val1 Value 1
* @param[in] val2 Value 2
* @return The result
*/
inline fixed operator/(fixed val1, fixed val2) { return val1.dValue / val2.dValue; }
/**
 * @{
 * @brief Compare two types
 * @param[in] val1 Value 1
 * @param[in] val2 Value 2
 * @return The result
 */
inline bool operator<(fixed val1, fixed val2) { return val1.dValue < val2.dValue; }
inline bool operator<(fixed val1, double val2) { return val1.dValue < val2; }
inline bool operator<(double val1, fixed val2) { return val1 < val2.dValue; }
inline bool operator<=(fixed val1, fixed val2) { return val1.dValue <= val2.dValue; }
inline bool operator<=(fixed val1, double val2) { return val1.dValue <= val2; }
inline bool operator<=(double val1, fixed val2) { return val1 <= val2.dValue; }
inline bool operator>(fixed val1, fixed val2) { return val1.dValue > val2.dValue; }
inline bool operator>(fixed val1, double val2) { return val1.dValue > val2; }
inline bool operator>(double val1, fixed val2) { return val1 > val2.dValue; }
inline bool operator>=(fixed val1, fixed val2) { return val1.dValue >= val2.dValue; }
inline bool operator>=(fixed val1, double val2) { return val1.dValue >= val2; }
inline bool operator>=(double val1, fixed val2) { return val1 >= val2.dValue; }
inline bool operator==(fixed val1, fixed val2) { return val1.dValue == val2.dValue; }
inline bool operator==(fixed val1, double val2) { return val1.dValue == val2; }
inline bool operator==(double val1, fixed val2) { return val1 == val2.dValue; }
inline bool operator!=(fixed val1, fixed val2) { return val1.dValue != val2.dValue; }
inline bool operator!=(fixed val1, double val2) { return val1.dValue != val2; }
inline bool operator!=(double val1, fixed val2) { return val1 != val2.dValue; }
/**
 * @}
 */

namespace std
{
#ifndef DOXYGEN_IGNORE
    /**
     * @brief Specialization of floating point variable for fixed data type.
     */
    template <>
    inline constexpr bool is_floating_point_v<::fixed> = true;

    /**
     * @brief Specialization of signed number variable for fixed data type.
     */
    template <>
    inline constexpr bool is_signed_v<::fixed> = true;

    /**
     * @brief Specialization of arithmic data type variable for fixed data type.
     */
    template <>
    inline constexpr bool is_arithmetic_v<::fixed> = true;
#endif

    /**
     * @brief Specialization of numeric limits class for fixed data type.
     */
    template <>
    struct numeric_limits<::fixed> : numeric_limits<double>
    {
        /**
         * @brief Returns the smallest finite value representable by fixed.
         * @return The smallest finite type.
         */
        [[nodiscard]] static constexpr ::fixed(min)() noexcept
        {
            return 1.0 / static_cast<double>(max());
        }

        /**
         * @brief Returns the minimum largest value representable by fixed.
         * @return The largest finite type.
         */
        [[nodiscard]] static constexpr ::fixed(max)() noexcept
        {
            // 31 bits available
            return 1ll << 31ll;
        }

        /**
         * @brief Returns the lowest finite value representable by fixed.
         * @return The lowest finite type.
         */
        [[nodiscard]] static constexpr ::fixed lowest() noexcept
        {
            return -(max)();
        }
    };
}

/**
 * @brief Variant class to store const values.
 * @details This class is used for the storage of all possible data types as well as solving the expressions in const assignments.
 * Only 32-bit, 64-bit integers, largest floating point and several string data type variations are used. The conversion rules for
 * data types follow the C++11 conversion rules:
 * - Assignment of data type:
 *      - Signed integers are assigned to int32_t when smaller than 64-bit; otherwise are assigned to int64_t.
 *      - Unsigned integers are assigned to uint32_t when smaller than 64-bit; otherwise are assigned to uint64_t.
 *      - Integral signed data types are: char, short, long, long long, int8_t, int16_t, int32_t int64_t.
 *      - Integral unsigned data types are: bool, unsigned short, unsigned long, unsigned long long, uint8_t, uint16_t, uint32_t
 *          uint64_t, wchar_t, char16_t, char32_t.
 *      - String objects and string pointers are assigned to their corresponding counterparts.
 * - Arithmic operations with two operands:
 *      - If either operand is long double, the other operand is converted to long double.
 *      - If both integral operands are signed or both are unsigned, the operand with lesser conversion rank is converted to the
 *      - operand with the greater conversion rank.
 *      - Otherwise, if the unsigned integral operand's conversion rank is greater or equal to the conversion rank of the signed
 *          operand, the signed operand is converted to the unsigned operand's type.
 *      - Otherwise, if the signed integral operand's type can represent all values of the unsigned operand, the unsigned operand
 *          is converted to the signed operand's type.
 *      - Otherwise, both integral operands are converted to the unsigned counterpart of the signed operand's type.
 *      - Conversions with and between string based operands are not supported.
 * - Cast to data type:
 *      - If the stored data type and target data type are both signed or both unsigned, the target data type boundaries are
 *          checked with the value before assignment takes place.
 *      - If the destination type is unsigned, the resulting value is the smallest unsigned value equal to the source value modulo
 *           2^n where n is the number of bits used to represent the destination type.
 *      - If the destination type is signed, the value does not change if the source integer can be represented in the destination
 *          type.
 */
class CConstVariant
{
    /**
     * The variant type containing the value in one of the data types supplied. The data type order provides the ranking of the
     * types.
    */
    using TVariant = std::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, fixed, float,
    double, long double, std::string, std::u16string, std::u32string, std::wstring>;
public:
    /**
     * @brief Type to index mapping.
     * @remarks 'char' is represented by 'int8_t'. 'char16_t' is represented by 'uint16_t'. 'char32_t' is represented by
     * 'int32_t'. 'wchar_t' is either represented by 'uint16_t' or by 'uint32_t'. ASCII and UTF-8 strings are represented by
     * 'std::string'.
     */
    enum ETypeMapping
    {
        type_bool = 0, type_int8_t, type_uint8_t, type_int16_t, type_uint16_t, type_int32_t, type_uint32_t, type_int64_t,
        type_uint64_t, type_fixed, type_float, type_double, type_long_double, type_string, type_u16string, type_u32string,
        type_wstring,
    };

    /**
     * @brief Default constructor.
     */
    CConstVariant() = default;

    /**
     * @brief Copy constructor.
     * @param[in] rvar Reference to the const variant to copy from.
     */
    CConstVariant(const CConstVariant& rvar);

    /**
     * @brief Move constructor is not available.
     * @param[in] rvar Reference to the const variant to move from.
     */
    CConstVariant(CConstVariant&& rvar) noexcept;

    /**
     * @brief Assignment constructor.
     * @param[in] bValue Value to assign.
     */
    CConstVariant(bool bValue);

    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
    CConstVariant(int8_t iValue);

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
    CConstVariant(uint8_t uiValue);

    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
    CConstVariant(int16_t iValue);

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
    CConstVariant(uint16_t uiValue);

#ifdef _WIN32
    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
    CConstVariant(long int iValue);

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
    CConstVariant(unsigned long int uiValue);
#endif

    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
    CConstVariant(int32_t iValue);

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
    CConstVariant(uint32_t uiValue);

    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
    CConstVariant(int64_t iValue);

#if defined(__GNUC__) && !defined(_WIN32)
    /**
     * @brief Assignment constructor.
     * @param[in] iValue Value to assign.
     */
     CConstVariant(long long int iValue);

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
     CConstVariant(unsigned long long int uiValue);
 #endif

    /**
     * @brief Assignment constructor.
     * @param[in] uiValue Value to assign.
     */
    CConstVariant(uint64_t uiValue);

    /**
     * @brief Assignment constructor.
     * @param[in] fixValue Value to assign.
     */
    CConstVariant(fixed fixValue);

    /**
     * @brief Assignment constructor.
     * @param[in] fValue Value to assign.
     */
    CConstVariant(float fValue);

    /**
     * @brief Assignment constructor.
     * @param[in] dValue Value to assign.
     */
    CConstVariant(double dValue);

    /**
     * @brief Assignment constructor.
     * @param[in] ldValue Value to assign.
     */
    CConstVariant(long double ldValue);

    /**
     * @brief Assignment constructor.
     * @param[in] rssValue Reference to the string value to assign.
     */
    CConstVariant(const std::string& rssValue);

    /**
     * @brief Assignment constructor.
     * @param[in] rssValue Reference to the string value to assign.
     */
    CConstVariant(const std::u16string& rssValue);

    /**
     * @brief Assignment constructor.
     * @param[in] rssValue Reference to the string value to assign.
     */
    CConstVariant(const std::u32string& rssValue);

    /**
     * @brief Assignment constructor.
     * @param[in] rssValue Reference to the string value to assign.
     */
    CConstVariant(const std::wstring& rssValue);

    /**
     * @brief Copy assignment operator
     * @param[in] rvar Reference to the const variant to copy from.
     * @return Reference to the const variant instance.
     */
    CConstVariant& operator=(const CConstVariant& rvar);

    /**
     * @brief Move operator is not available.
     * @param[in] rvar Reference to the const variant to move from.
     * @return Reference to the const variant instance.
     */
    CConstVariant& operator=(CConstVariant&& rvar) noexcept;

    /**
     * @brief Assignment operator.
     * @tparam TType Type of the value to assign.
     * @param[in] rtValue Reference to the value to assign.
     * @return Reference to the const variant instance.
     */
    template <typename TType>
    CConstVariant& operator=(const TType& rtValue);

    /**
     * @brief Get the value of the variant casted to the provided target type.
     * @throw Throws exception when the value exceeds the boundaries of the target value or the types are incompatible.
     * @tparam TTargetType The target type.
     * @return The value casted to the target type.
     */
    template <typename TTargetType>
    TTargetType Get() const;

    /**
     * @brief Get the value as string.
     * @return Get the value as a string.
     */
    std::string GetAsString() const;

    /**
     * @{
     * @brief Meta information
     * @remarks The ranking function provides a number indicating the ranking of the type.
     * @return Returns 'true' when the type stored in the variant has the specific attribute; 'false' otherwise.
     */
    bool IsArithmetic() const;
    bool IsIntegral() const;
    bool IsFloatingPoint() const;
    bool IsBoolean() const;
    bool IsSigned() const;
    bool IsUnsigned() const;
    size_t Ranking() const;
    template <typename TType> bool IsSame() const;
    /**
     * @}
     */

    /**
     * @brief Convert the variant to the type supplied.
     * @throw Throws exception when the value exceeds the boundaries of the target value or the types are incompatible.
     * @param[in] nRank The rank of the type to convert to.
     */
    void Convert(size_t nRank);

    /**
     * @brief Convert the variant to the type supplied.
     * @throw Throws exception when the value exceeds the boundaries of the target value or the types are incompatible.
     * @tparam TType The type to convert to.
     */
    template <typename TType>
    void Convert();

    /**
     * @{
     * @brief Unary arithmetic operators.
     * @remarks The bit operator '~' and the logical operator '!' work with integral operands only.
     * @result The result of the operation.
     */
    CConstVariant operator!() const;
    CConstVariant operator~() const;
    CConstVariant operator+() const;
    CConstVariant operator-() const;
    /**
     * @}
     */

    /**
     * @{
     * @brief Do unary operation. The first version allows the operation on any arithmetic type. The second version works for
     * integral operation only.
     * @tparam TFunction The function type that should do the operation. The function uses the prototype:
     * @code CConstVariant(auto tOperand); @endcode
     * @param[in] rvarOperand The operand.
     * @param[in] tOperation The function that is used for the operation.
     * @return The const variant with the result.
     */
    template <typename TFunction>
    static CConstVariant UnaryOperation(const CConstVariant& rvarOperand, TFunction tOperation);
    template <typename TFunction>
    static CConstVariant UnaryOperationIntegral(const CConstVariant& rvarOperand, TFunction tOperation);
    /**
     * @}
     */

    /**
     * @{
     * @brief Do binary operation. The first version allows the operation on any arithmetic type. The second version works for
     * integral operation only.
     * @tparam TFunction The function type that should do the operation. The function uses the prototype:
     * @code CConstVariant(auto tOperand1, auto tOperand2); @endcode
     * @param[in] rvarOperand1 The first operand.
     * @param[in] rvarOperand2 The second operand.
     * @param[in] tOperation The function that is used for the operation.
     * @return The const variant with the result.
     */
    template <typename TFunction>
    static CConstVariant BinaryOperation(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2,
        TFunction tOperation);
    template <typename TFunction>
    static CConstVariant BinaryOperationIntegral(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2,
        TFunction tOperation);
    /**
     * @}
     */

private:
    /**
     * @brief Get the value of the variant.
     * @details Get the value of the variant by providing the variant type as well as the target type. Do conversion if necessary.
     * Follow the casting rules as defined at the class description.
     * @throw Throws exception when the value exceeds the boundaries of the target value or the types are incompatible.
     * @tparam TTargetType The target type.
     * @tparam TVariantType The variant type
     * @return The value casted to the target type.
     */
    template <typename TTargetType, typename TVariantType>
    TTargetType InternalGet() const;

    TVariant    m_varValue;      ///< The variant that stores the value.

};

/**
 * @brief Equalize variants.
 * @details Convert the variant of lesser ranking to a higher ranking. This function is used for binary arithmetic operations. The
 * function implements the rules as described at the CConstVariant class description.
 * @param[in] rvar1 The first variant.
 * @param[in] rvar2 The second variant.
 */
void Equalize(CConstVariant& rvar1, CConstVariant& rvar2);

/**
 * @{
 * @brief Arithmetic operators.
 * @remarks The bit operators (~ & | ^ << >>) as well as the remainder (%) operator work with integral operands only.
 * @param[in] rvarOperand1 The first operand of a binary operation.
 * @param[in] rvarOperand2 The second operand of a binary operation.
 * @result The result of the operation.
 */
CConstVariant operator*(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator/(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator+(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator-(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator<(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator<=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator>(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator>=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator+(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator==(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator!=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator%(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator<<(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator>>(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator&(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator^(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator|(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator&&(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
CConstVariant operator||(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2);
/**
 * @}
 */

#endif // !defined(VARIANT_H)