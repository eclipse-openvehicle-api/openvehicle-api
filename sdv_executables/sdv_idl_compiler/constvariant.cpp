#include "constvariant.h"
#include "constvariant.inl"

CConstVariant::CConstVariant(const CConstVariant& rvar) : m_varValue(rvar.m_varValue)
{}

CConstVariant::CConstVariant(CConstVariant&& rvar) noexcept : m_varValue(std::move(rvar.m_varValue))
{}

CConstVariant::CConstVariant(bool bValue) : m_varValue(bValue) {}
CConstVariant::CConstVariant(int8_t iValue) : m_varValue(iValue) {}
CConstVariant::CConstVariant(uint8_t uiValue) : m_varValue(uiValue) {}
CConstVariant::CConstVariant(int16_t iValue) : m_varValue(iValue) {}
CConstVariant::CConstVariant(uint16_t uiValue) : m_varValue(uiValue) {}
#ifdef _WIN32
CConstVariant::CConstVariant(long int iValue) : m_varValue(static_cast<int32_t>(iValue)) {}
CConstVariant::CConstVariant(unsigned long int uiValue) : m_varValue(static_cast<uint32_t>(uiValue)) {}
#endif
CConstVariant::CConstVariant(int32_t iValue) : m_varValue(iValue) {}
CConstVariant::CConstVariant(uint32_t uiValue) : m_varValue(uiValue) {}
CConstVariant::CConstVariant(int64_t iValue) : m_varValue(iValue) {}
CConstVariant::CConstVariant(uint64_t uiValue) : m_varValue(uiValue) {}
#if defined(__GNUC__) && !defined(_WIN32)
 CConstVariant::CConstVariant(long long int iValue) : CConstVariant(static_cast<int64_t>(iValue)) {}
 CConstVariant::CConstVariant(unsigned long long int uiValue) : CConstVariant(static_cast<uint64_t>(uiValue)) {}
 #endif
CConstVariant::CConstVariant(fixed fixValue) : m_varValue(fixValue) {}
CConstVariant::CConstVariant(float fValue) : m_varValue(fValue) {}
CConstVariant::CConstVariant(double dValue) : m_varValue(dValue) {}
CConstVariant::CConstVariant(long double ldValue) : m_varValue(ldValue) {}
CConstVariant::CConstVariant(const std::string& rssValue) : m_varValue(rssValue) {}
CConstVariant::CConstVariant(const std::u16string& rssValue) : m_varValue(rssValue) {}
CConstVariant::CConstVariant(const std::u32string& rssValue) : m_varValue(rssValue) {}
CConstVariant::CConstVariant(const std::wstring& rssValue) : m_varValue(rssValue) {}

CConstVariant& CConstVariant::operator=(const CConstVariant& rvar)
{
    m_varValue = rvar.m_varValue;
    return *this;
}

CConstVariant& CConstVariant::operator=(CConstVariant&& rvar) noexcept
{
    m_varValue = std::move(rvar.m_varValue);
    return *this;
}

bool CConstVariant::IsArithmetic() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::is_arithmetic_v<bool>;
    case ETypeMapping::type_uint8_t:        return std::is_arithmetic_v<uint8_t>;
    case ETypeMapping::type_uint16_t:       return std::is_arithmetic_v<uint16_t>;
    case ETypeMapping::type_uint32_t:       return std::is_arithmetic_v<uint32_t>;
    case ETypeMapping::type_uint64_t:       return std::is_arithmetic_v<uint64_t>;
    case ETypeMapping::type_int8_t:         return std::is_arithmetic_v<int8_t>;
    case ETypeMapping::type_int16_t:        return std::is_arithmetic_v<int16_t>;
    case ETypeMapping::type_int32_t:        return std::is_arithmetic_v<int32_t>;
    case ETypeMapping::type_int64_t:        return std::is_arithmetic_v<int64_t>;
    case ETypeMapping::type_fixed:          return std::is_arithmetic_v<fixed>;
    case ETypeMapping::type_float:          return std::is_arithmetic_v<float>;
    case ETypeMapping::type_double:         return std::is_arithmetic_v<double>;
    case ETypeMapping::type_long_double:    return std::is_arithmetic_v<long double>;
    case ETypeMapping::type_string:         return std::is_arithmetic_v<std::string>;
    case ETypeMapping::type_u16string:      return std::is_arithmetic_v<std::u16string>;
    case ETypeMapping::type_u32string:      return std::is_arithmetic_v<std::u32string>;
    case ETypeMapping::type_wstring:        return std::is_arithmetic_v<std::wstring>;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

bool CConstVariant::IsIntegral() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::is_integral_v<bool>;
    case ETypeMapping::type_uint8_t:        return std::is_integral_v<uint8_t>;
    case ETypeMapping::type_uint16_t:       return std::is_integral_v<uint16_t>;
    case ETypeMapping::type_uint32_t:       return std::is_integral_v<uint32_t>;
    case ETypeMapping::type_uint64_t:       return std::is_integral_v<uint64_t>;
    case ETypeMapping::type_int8_t:         return std::is_integral_v<int8_t>;
    case ETypeMapping::type_int16_t:        return std::is_integral_v<int16_t>;
    case ETypeMapping::type_int32_t:        return std::is_integral_v<int32_t>;
    case ETypeMapping::type_int64_t:        return std::is_integral_v<int64_t>;
    case ETypeMapping::type_fixed:          return std::is_integral_v<fixed>;
    case ETypeMapping::type_float:          return std::is_integral_v<float>;
    case ETypeMapping::type_double:         return std::is_integral_v<double>;
    case ETypeMapping::type_long_double:    return std::is_integral_v<long double>;
    case ETypeMapping::type_string:         return std::is_integral_v<std::string>;
    case ETypeMapping::type_u16string:      return std::is_integral_v<std::u16string>;
    case ETypeMapping::type_u32string:      return std::is_integral_v<std::u32string>;
    case ETypeMapping::type_wstring:        return std::is_integral_v<std::wstring>;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

bool CConstVariant::IsFloatingPoint() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::is_floating_point_v<bool>;
    case ETypeMapping::type_uint8_t:        return std::is_floating_point_v<uint8_t>;
    case ETypeMapping::type_uint16_t:       return std::is_floating_point_v<uint16_t>;
    case ETypeMapping::type_uint32_t:       return std::is_floating_point_v<uint32_t>;
    case ETypeMapping::type_uint64_t:       return std::is_floating_point_v<uint64_t>;
    case ETypeMapping::type_int8_t:         return std::is_floating_point_v<int8_t>;
    case ETypeMapping::type_int16_t:        return std::is_floating_point_v<int16_t>;
    case ETypeMapping::type_int32_t:        return std::is_floating_point_v<int32_t>;
    case ETypeMapping::type_int64_t:        return std::is_floating_point_v<int64_t>;
    case ETypeMapping::type_fixed:          return std::is_floating_point_v<fixed>;
    case ETypeMapping::type_float:          return std::is_floating_point_v<float>;
    case ETypeMapping::type_double:         return std::is_floating_point_v<double>;
    case ETypeMapping::type_long_double:    return std::is_floating_point_v<long double>;
    case ETypeMapping::type_string:         return std::is_floating_point_v<std::string>;
    case ETypeMapping::type_u16string:      return std::is_floating_point_v<std::u16string>;
    case ETypeMapping::type_u32string:      return std::is_floating_point_v<std::u32string>;
    case ETypeMapping::type_wstring:        return std::is_floating_point_v<std::wstring>;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

bool CConstVariant::IsBoolean() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return true;
    case ETypeMapping::type_uint8_t:        return false;
    case ETypeMapping::type_uint16_t:       return false;
    case ETypeMapping::type_uint32_t:       return false;
    case ETypeMapping::type_uint64_t:       return false;
    case ETypeMapping::type_int8_t:         return false;
    case ETypeMapping::type_int16_t:        return false;
    case ETypeMapping::type_int32_t:        return false;
    case ETypeMapping::type_int64_t:        return false;
    case ETypeMapping::type_fixed:          return false;
    case ETypeMapping::type_float:          return false;
    case ETypeMapping::type_double:         return false;
    case ETypeMapping::type_long_double:    return false;
    case ETypeMapping::type_string:         return false;
    case ETypeMapping::type_u16string:      return false;
    case ETypeMapping::type_u32string:      return false;
    case ETypeMapping::type_wstring:        return false;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

bool CConstVariant::IsSigned() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::is_signed_v<bool>;
    case ETypeMapping::type_uint8_t:        return std::is_signed_v<uint8_t>;
    case ETypeMapping::type_uint16_t:       return std::is_signed_v<uint16_t>;
    case ETypeMapping::type_uint32_t:       return std::is_signed_v<uint32_t>;
    case ETypeMapping::type_uint64_t:       return std::is_signed_v<uint64_t>;
    case ETypeMapping::type_int8_t:         return std::is_signed_v<int8_t>;
    case ETypeMapping::type_int16_t:        return std::is_signed_v<int16_t>;
    case ETypeMapping::type_int32_t:        return std::is_signed_v<int32_t>;
    case ETypeMapping::type_int64_t:        return std::is_signed_v<int64_t>;
    case ETypeMapping::type_fixed:          return std::is_signed_v<fixed>;
    case ETypeMapping::type_float:          return std::is_signed_v<float>;
    case ETypeMapping::type_double:         return std::is_signed_v<double>;
    case ETypeMapping::type_long_double:    return std::is_signed_v<long double>;
    case ETypeMapping::type_string:         return std::is_signed_v<std::string>;
    case ETypeMapping::type_u16string:      return std::is_signed_v<std::u16string>;
    case ETypeMapping::type_u32string:      return std::is_signed_v<std::u32string>;
    case ETypeMapping::type_wstring:        return std::is_signed_v<std::wstring>;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

bool CConstVariant::IsUnsigned() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::is_unsigned_v<bool>;
    case ETypeMapping::type_uint8_t:        return std::is_unsigned_v<uint8_t>;
    case ETypeMapping::type_uint16_t:       return std::is_unsigned_v<uint16_t>;
    case ETypeMapping::type_uint32_t:       return std::is_unsigned_v<uint32_t>;
    case ETypeMapping::type_uint64_t:       return std::is_unsigned_v<uint64_t>;
    case ETypeMapping::type_int8_t:         return std::is_unsigned_v<int8_t>;
    case ETypeMapping::type_int16_t:        return std::is_unsigned_v<int16_t>;
    case ETypeMapping::type_int32_t:        return std::is_unsigned_v<int32_t>;
    case ETypeMapping::type_int64_t:        return std::is_unsigned_v<int64_t>;
    case ETypeMapping::type_fixed:          return std::is_unsigned_v<fixed>;
    case ETypeMapping::type_float:          return std::is_unsigned_v<float>;
    case ETypeMapping::type_double:         return std::is_unsigned_v<double>;
    case ETypeMapping::type_long_double:    return std::is_unsigned_v<long double>;
    case ETypeMapping::type_string:         return std::is_unsigned_v<std::string>;
    case ETypeMapping::type_u16string:      return std::is_unsigned_v<std::u16string>;
    case ETypeMapping::type_u32string:      return std::is_unsigned_v<std::u32string>;
    case ETypeMapping::type_wstring:        return std::is_unsigned_v<std::wstring>;
    default:
        throw CCompileException("Internal error: the variant doesn't contain any valid data type.");
    }
}

size_t CConstVariant::Ranking() const
{
    // The ranking is provided through the index.
    return m_varValue.index();
}

void CConstVariant::Convert(size_t nRank)
{
    if (nRank == Ranking()) return;

    switch (static_cast<ETypeMapping>(nRank))
    {
    case ETypeMapping::type_bool:           m_varValue = Get<bool>();               break;
    case ETypeMapping::type_uint8_t:        m_varValue = Get<uint8_t>();            break;
    case ETypeMapping::type_uint16_t:       m_varValue = Get<uint16_t>();           break;
    case ETypeMapping::type_uint32_t:       m_varValue = Get<uint32_t>();           break;
    case ETypeMapping::type_uint64_t:       m_varValue = Get<uint64_t>();           break;
    case ETypeMapping::type_int8_t:         m_varValue = Get<int8_t>();             break;
    case ETypeMapping::type_int16_t:        m_varValue = Get<int16_t>();            break;
    case ETypeMapping::type_int32_t:        m_varValue = Get<int32_t>();            break;
    case ETypeMapping::type_int64_t:        m_varValue = Get<int64_t>();            break;
    case ETypeMapping::type_fixed:          m_varValue = Get<fixed>();              break;
    case ETypeMapping::type_float:          m_varValue = Get<float>();              break;
    case ETypeMapping::type_double:         m_varValue = Get<double>();             break;
    case ETypeMapping::type_long_double:    m_varValue = Get<long double>();        break;
    default:
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

CConstVariant CConstVariant::operator!() const
{
    return UnaryOperationIntegral(*this, [](auto tOperand) {return !tOperand;});
}

CConstVariant CConstVariant::operator~() const
{
    return UnaryOperationIntegral(*this, [](auto tOperand) -> CConstVariant {
            if constexpr (!std::is_same_v<decltype(tOperand), bool>)
                return ~tOperand;
            else
                throw CCompileException("Cannot execute bitwise operations on a boolean."); });
}

CConstVariant CConstVariant::operator+() const
{
    return UnaryOperation(*this, [](auto tOperand) {return tOperand;});
}

CConstVariant CConstVariant::operator-() const
{
    return CConstVariant::UnaryOperation(*this, [](auto tOperand) -> CConstVariant {
            if constexpr (!std::is_same_v<decltype(tOperand), bool>)
            {
                if constexpr (std::is_signed_v<decltype(tOperand)>)
                    return -tOperand;
                else if constexpr (std::is_integral_v<decltype(tOperand)>)
                {
                    // Two's complement plus 1.
                    return ~tOperand + 1;
                } else
                    throw CCompileException("Internal error: cannot execute unary arithmic operation on the type.");
            }
            else
                throw CCompileException("Cannot execute unary arithmic operations on a boolean."); });
}

void Equalize(CConstVariant& rvar1, CConstVariant& rvar2)
{
    // Are the types of both variant the same?
    if (rvar1.Ranking() == rvar2.Ranking()) return;

    // Equalization only works if both types are arithmetic.
    if (!rvar1.IsArithmetic() || !rvar2.IsArithmetic())
        throw CCompileException("The types of both operands are not compatible.");

    // Check for highest ranking and adapt the lowest.
    if (rvar1.Ranking() > rvar2.Ranking())
        rvar2.Convert(rvar1.Ranking());
    else
        rvar1.Convert(rvar2.Ranking());
}

CConstVariant operator*(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 * tOperand2;});
}

CConstVariant operator/(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) -> CConstVariant
        {
            if constexpr (!std::is_same_v<decltype(tOperand1), bool>)
            {
                if (!tOperand2) throw CCompileException("Division by zero.");
                return tOperand1 / tOperand2;
            }
            else
                throw CCompileException("Cannot divide a boolean.");
        });
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

CConstVariant operator+(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 + tOperand2;});
}

CConstVariant operator-(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 - tOperand2;});
}

CConstVariant operator<(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 < tOperand2;});
}

CConstVariant operator<=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 <= tOperand2;});
}

CConstVariant operator>(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 > tOperand2;});
}

CConstVariant operator>=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 >= tOperand2;});
}

CConstVariant operator==(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 == tOperand2;});
}

CConstVariant operator!=(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 != tOperand2;});
}

CConstVariant operator%(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) -> CConstVariant
        {
            if constexpr (std::is_integral_v<decltype(tOperand1)> && !std::is_same_v<decltype(tOperand1), bool>)
            {
                if (!tOperand2) throw CCompileException("Division by zero.");
                return tOperand1 % tOperand2;
            }
            else
                throw CCompileException("Cannot divide a boolean.");
        });
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

CConstVariant operator<<(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperationIntegral(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 << tOperand2;});
}

CConstVariant operator>>(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperationIntegral(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 >> tOperand2;});
}

CConstVariant operator&(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperationIntegral(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 & tOperand2;});
}

CConstVariant operator^(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperationIntegral(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 ^ tOperand2;});
}

CConstVariant operator|(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperationIntegral(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2) {return tOperand1 | tOperand2;});
}

CConstVariant operator&&(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2)
        {
            if constexpr (std::is_integral_v<decltype(tOperand1)> &&
                          std::is_integral_v<decltype(tOperand2)>)
                return tOperand1 && tOperand2;
            else
                return false;
        });
}

CConstVariant operator||(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2)
{
    return CConstVariant::BinaryOperation(rvarOperand1, rvarOperand2,
        [](auto tOperand1, auto tOperand2)
        {
            if constexpr (std::is_integral_v<decltype(tOperand1)> &&
                std::is_integral_v<decltype(tOperand2)>)
                return tOperand1 || tOperand2;
            else
                return false;
        });
}
