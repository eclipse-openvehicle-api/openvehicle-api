#ifndef CONSTVARIANT_INL
#define CONSTVARIANT_INL

#include "exception.h"
#include "constvariant.h"
#include <cmath>
#include <cstdlib>

template <typename TType>
inline CConstVariant& CConstVariant::operator=(const TType& rtValue)
{
    m_varValue = rtValue;
    return *this;
}

#ifdef _WIN32
template <>
inline CConstVariant& CConstVariant::operator=(const long int& rtValue)
{
	m_varValue = static_cast<int32_t>(rtValue);
	return *this;
}

template <>
inline CConstVariant& CConstVariant::operator=(const unsigned long int& rtValue)
{
	m_varValue = static_cast<uint32_t>(rtValue);
	return *this;
}
#endif

#if defined( __GNUC__) && !defined(_WIN32)

template <>
inline CConstVariant& CConstVariant::operator=(const long long int& rtValue)
{
    m_varValue = static_cast<int64_t>(rtValue);
    return *this;
}

template <>
inline CConstVariant& CConstVariant::operator=(const unsigned long long int& rtValue)
{
    m_varValue = static_cast<uint64_t>(rtValue);
    return *this;
}

#endif

template <typename TTargetType>
inline TTargetType CConstVariant::Get() const
{
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return InternalGet<TTargetType, bool>();
    case ETypeMapping::type_uint8_t:        return InternalGet<TTargetType, uint8_t>();
    case ETypeMapping::type_uint16_t:       return InternalGet<TTargetType, uint16_t>();
    case ETypeMapping::type_uint32_t:       return InternalGet<TTargetType, uint32_t>();
    case ETypeMapping::type_uint64_t:       return InternalGet<TTargetType, uint64_t>();
    case ETypeMapping::type_int8_t:         return InternalGet<TTargetType, int8_t>();
    case ETypeMapping::type_int16_t:        return InternalGet<TTargetType, int16_t>();
    case ETypeMapping::type_int32_t:        return InternalGet<TTargetType, int32_t>();
    case ETypeMapping::type_int64_t:        return InternalGet<TTargetType, int64_t>();
    case ETypeMapping::type_fixed:          return InternalGet<TTargetType, fixed>();
    case ETypeMapping::type_float:          return InternalGet<TTargetType, float>();
    case ETypeMapping::type_double:         return InternalGet<TTargetType, double>();
    case ETypeMapping::type_long_double:    return InternalGet<TTargetType, long double>();
    case ETypeMapping::type_string:         return InternalGet<TTargetType, std::string>();
    case ETypeMapping::type_u16string:      return InternalGet<TTargetType, std::u16string>();
    case ETypeMapping::type_u32string:      return InternalGet<TTargetType, std::u32string>();
    case ETypeMapping::type_wstring:        return InternalGet<TTargetType, std::wstring>();
    default:
        throw CCompileException("Conversion to target data type is not supported.");
    }
}

inline std::string CConstVariant::GetAsString() const
{
    // TODO: Add conversion for the missing types
    switch (static_cast<ETypeMapping>(m_varValue.index()))
    {
    case ETypeMapping::type_bool:           return std::to_string(std::get<bool>(m_varValue));
    case ETypeMapping::type_uint8_t:        return std::to_string(std::get<uint8_t>(m_varValue));
    case ETypeMapping::type_uint16_t:       return std::to_string(std::get<uint16_t>(m_varValue));
    case ETypeMapping::type_uint32_t:       return std::to_string(std::get<uint32_t>(m_varValue));
    case ETypeMapping::type_uint64_t:       return std::to_string(std::get<uint64_t>(m_varValue));
    case ETypeMapping::type_int8_t:         return std::to_string(std::get<int8_t>(m_varValue));
    case ETypeMapping::type_int16_t:        return std::to_string(std::get<int16_t>(m_varValue));
    case ETypeMapping::type_int32_t:        return std::to_string(std::get<int32_t>(m_varValue));
    case ETypeMapping::type_int64_t:        return std::to_string(std::get<int64_t>(m_varValue));
    case ETypeMapping::type_fixed:          return "fixed";
    case ETypeMapping::type_float:          return std::to_string(std::get<float>(m_varValue));
    case ETypeMapping::type_double:         return std::to_string(std::get<double>(m_varValue));
    case ETypeMapping::type_long_double:    return std::to_string(std::get<long double>(m_varValue));
    case ETypeMapping::type_string:         return std::get<std::string>(m_varValue);
    case ETypeMapping::type_u16string:      return "UTF-16 string";
    case ETypeMapping::type_u32string:      return "UTF-32string";
    case ETypeMapping::type_wstring:        return "wstring";
    default:
        throw CCompileException("Creating a string from value is not possible.");
    }
}

template <typename TType>
inline bool CConstVariant::IsSame() const
{
    return std::holds_alternative<TType>(m_varValue);
}

template <typename TType>
inline void CConstVariant::Convert()
{
    // Conversion needed?
    if (IsSame<TType>()) return;

    if constexpr (std::is_integral_v<TType>)
    {
        if constexpr (std::is_signed_v<TType>)
        {
            // Singed integer
            // NOTE: 'if' is used instead of switch to allow the use of constexpr.
            if constexpr (sizeof(TType) == sizeof(int8_t))
                Convert(ETypeMapping::type_int8_t);
            else if constexpr (sizeof(TType) == sizeof(int16_t))
                Convert(ETypeMapping::type_int16_t);
            else if constexpr (sizeof(TType) == sizeof(int32_t))
                Convert(ETypeMapping::type_int32_t);
            else
                Convert(ETypeMapping::type_int64_t);
        } else
        {
            // Unsigned integer
            // NOTE: 'if' is used instead of switch to allow the use of constexpr.
            if constexpr (sizeof(TType) == sizeof(uint8_t))
            {
                if constexpr (std::is_same_v<bool, TType>)
                    Convert(ETypeMapping::type_bool);
                else
                    Convert(ETypeMapping::type_uint8_t);
            } else if constexpr (sizeof(TType) == sizeof(uint16_t))
                Convert(ETypeMapping::type_uint16_t);
            else if constexpr (sizeof(TType) == sizeof(uint32_t))
                Convert(ETypeMapping::type_uint32_t);
            else
                Convert(ETypeMapping::type_uint64_t);
        }

        // Done!
        return;
    }
    else if constexpr (std::is_floating_point_v<TType>)
    {
        // NOTE: 'if' is used instead of switch to allow the use of constexpr.
        if constexpr (std::is_same_v<TType, fixed>)
            Convert(ETypeMapping::type_fixed);
        else if constexpr (sizeof(TType) == sizeof(float))
            Convert(ETypeMapping::type_float);
        else if constexpr (sizeof(TType) == sizeof(double))
            Convert(ETypeMapping::type_double);
        else
            Convert(ETypeMapping::type_long_double);

        // Done!
        return;
    } else
    {
        // Conversion is only possible between arithmetic types.
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

template <typename TFunction>
inline CConstVariant CConstVariant::UnaryOperation(const CConstVariant& rvarOperand, TFunction tOperation)
{
    // Based on the operand type, execute the provided function
    switch (static_cast<ETypeMapping>(rvarOperand.Ranking()))
    {
    case ETypeMapping::type_bool:           return tOperation(rvarOperand.Get<bool>());          break;
    case ETypeMapping::type_uint8_t:        return tOperation(rvarOperand.Get<uint8_t>());       break;
    case ETypeMapping::type_uint16_t:       return tOperation(rvarOperand.Get<uint16_t>());      break;
    case ETypeMapping::type_uint32_t:       return tOperation(rvarOperand.Get<uint32_t>());      break;
    case ETypeMapping::type_uint64_t:       return tOperation(rvarOperand.Get<uint64_t>());      break;
    case ETypeMapping::type_int8_t:         return tOperation(rvarOperand.Get<int8_t>());        break;
    case ETypeMapping::type_int16_t:        return tOperation(rvarOperand.Get<int16_t>());       break;
    case ETypeMapping::type_int32_t:        return tOperation(rvarOperand.Get<int32_t>());       break;
    case ETypeMapping::type_int64_t:        return tOperation(rvarOperand.Get<int64_t>());       break;
    case ETypeMapping::type_fixed:          return tOperation(rvarOperand.Get<fixed>());         break;
    case ETypeMapping::type_float:          return tOperation(rvarOperand.Get<float>());         break;
    case ETypeMapping::type_double:         return tOperation(rvarOperand.Get<double>());        break;
    case ETypeMapping::type_long_double:    return tOperation(rvarOperand.Get<long double>());   break;
    default:
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

template <typename TFunction>
inline CConstVariant CConstVariant::UnaryOperationIntegral(const CConstVariant& rvarOperand, TFunction tOperation)
{
    // Based on the operand type, execute the provided function
    switch (static_cast<ETypeMapping>(rvarOperand.Ranking()))
    {
    case ETypeMapping::type_bool:           return tOperation(rvarOperand.Get<bool>());         break;
    case ETypeMapping::type_uint8_t:        return tOperation(rvarOperand.Get<uint8_t>());      break;
    case ETypeMapping::type_uint16_t:       return tOperation(rvarOperand.Get<uint16_t>());     break;
    case ETypeMapping::type_uint32_t:       return tOperation(rvarOperand.Get<uint32_t>());     break;
    case ETypeMapping::type_uint64_t:       return tOperation(rvarOperand.Get<uint64_t>());     break;
    case ETypeMapping::type_int8_t:         return tOperation(rvarOperand.Get<int8_t>());       break;
    case ETypeMapping::type_int16_t:        return tOperation(rvarOperand.Get<int16_t>());      break;
    case ETypeMapping::type_int32_t:        return tOperation(rvarOperand.Get<int32_t>());      break;
    case ETypeMapping::type_int64_t:        return tOperation(rvarOperand.Get<int64_t>());      break;
    default:
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

template <typename TFunction>
inline CConstVariant CConstVariant::BinaryOperation(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2,
    TFunction tOperation)
{
    // Equalize the operands
    CConstVariant varOperand1 = rvarOperand1;
    CConstVariant varOperand2 = rvarOperand2;
    Equalize(varOperand1, varOperand2);

    // Based on the operand type, execute the provided function
    switch (static_cast<ETypeMapping>(varOperand1.Ranking()))
    {
        // NOTE: Arithmetic operations on "bool" can cause warnings. Use uint8_t instead.
    case ETypeMapping::type_bool:           return tOperation(varOperand1.Get<uint8_t>(), varOperand2.Get<uint8_t>());          break;
    case ETypeMapping::type_uint8_t:        return tOperation(varOperand1.Get<uint8_t>(), varOperand2.Get<uint8_t>());          break;
    case ETypeMapping::type_uint16_t:       return tOperation(varOperand1.Get<uint16_t>(), varOperand2.Get<uint16_t>());        break;
    case ETypeMapping::type_uint32_t:       return tOperation(varOperand1.Get<uint32_t>(), varOperand2.Get<uint32_t>());        break;
    case ETypeMapping::type_uint64_t:       return tOperation(varOperand1.Get<uint64_t>(), varOperand2.Get<uint64_t>());        break;
    case ETypeMapping::type_int8_t:         return tOperation(varOperand1.Get<int8_t>(), varOperand2.Get<int8_t>());            break;
    case ETypeMapping::type_int16_t:        return tOperation(varOperand1.Get<int16_t>(), varOperand2.Get<int16_t>());          break;
    case ETypeMapping::type_int32_t:        return tOperation(varOperand1.Get<int32_t>(), varOperand2.Get<int32_t>());          break;
    case ETypeMapping::type_int64_t:        return tOperation(varOperand1.Get<int64_t>(), varOperand2.Get<int64_t>());          break;
    case ETypeMapping::type_fixed:          return tOperation(varOperand1.Get<fixed>(), varOperand2.Get<fixed>());              break;
    case ETypeMapping::type_float:          return tOperation(varOperand1.Get<float>(), varOperand2.Get<float>());              break;
    case ETypeMapping::type_double:         return tOperation(varOperand1.Get<double>(), varOperand2.Get<double>());            break;
    case ETypeMapping::type_long_double:    return tOperation(varOperand1.Get<long double>(), varOperand2.Get<long double>());  break;
    default:
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

template <typename TFunction>
inline CConstVariant CConstVariant::BinaryOperationIntegral(const CConstVariant& rvarOperand1, const CConstVariant& rvarOperand2,
    TFunction tOperation)
{
    // Equalize the operands
    CConstVariant varOperand1 = rvarOperand1;
    CConstVariant varOperand2 = rvarOperand2;
    Equalize(varOperand1, varOperand2);

    // Based on the operand type, execute the provided function
    switch (static_cast<ETypeMapping>(varOperand1.Ranking()))
    {
    case ETypeMapping::type_uint8_t:        return tOperation(varOperand1.Get<uint8_t>(), varOperand2.Get<uint8_t>());          break;
    case ETypeMapping::type_uint16_t:       return tOperation(varOperand1.Get<uint16_t>(), varOperand2.Get<uint16_t>());        break;
    case ETypeMapping::type_uint32_t:       return tOperation(varOperand1.Get<uint32_t>(), varOperand2.Get<uint32_t>());        break;
    case ETypeMapping::type_uint64_t:       return tOperation(varOperand1.Get<uint64_t>(), varOperand2.Get<uint64_t>());        break;
    case ETypeMapping::type_int8_t:         return tOperation(varOperand1.Get<int8_t>(), varOperand2.Get<int8_t>());            break;
    case ETypeMapping::type_int16_t:        return tOperation(varOperand1.Get<int16_t>(), varOperand2.Get<int16_t>());          break;
    case ETypeMapping::type_int32_t:        return tOperation(varOperand1.Get<int32_t>(), varOperand2.Get<int32_t>());          break;
    case ETypeMapping::type_int64_t:        return tOperation(varOperand1.Get<int64_t>(), varOperand2.Get<int64_t>());          break;
    default:
        throw CCompileException("Internal error: incompatible data type conversion.");
    }
}

template <typename TTargetType, typename TVariantType>
inline TTargetType CConstVariant::InternalGet() const
{
   if constexpr (std::is_same_v<TTargetType, TVariantType>)
        return std::get<TVariantType>(m_varValue);
    else if constexpr (std::is_floating_point_v<TTargetType> && std::is_arithmetic_v<TVariantType>)
    {
        TVariantType tValue = std::get<TVariantType>(m_varValue);
        if (static_cast<long double>(tValue) > static_cast<long double>(std::numeric_limits<TTargetType>::max()))
            throw CCompileException("Cannot cast to type, the value exceeds the maximum possible value of the target type.");
        if (static_cast<long double>(tValue) < static_cast<long double>(std::numeric_limits<TTargetType>::lowest()))
            throw CCompileException("Cannot cast to type, the value is below the minumum possible value of the target type.");
        if constexpr (std::is_floating_point_v<TVariantType>)
        {
            int iExpValue = 0, iExpMin = 0;
            long double ldDigitsValue = std::fabs(std::frexp(static_cast<long double>(tValue), &iExpValue));
            long double ldDigitsMin = std::frexp(static_cast<long double>(std::numeric_limits<TTargetType>::min()), &iExpMin);
            if ((iExpValue < iExpMin) || ((iExpValue == iExpMin) && ldDigitsValue < ldDigitsMin))
                throw CCompileException("Cannot cast to type, the value precision is below the smallest possible"
                                      " precision of the target type.");
        }
        return static_cast<TTargetType>(tValue);
    }
    else if constexpr (std::is_integral_v<TTargetType> && std::is_integral_v<TVariantType> && !std::is_same_v<TTargetType, bool>)
    {
        if constexpr (std::is_signed_v<TTargetType> && std::is_signed_v<TVariantType>)
        {
            TVariantType tValue = std::get<TVariantType>(m_varValue);
            if (static_cast<int64_t>(tValue) > static_cast<int64_t>(std::numeric_limits<TTargetType>::max()))
                throw CCompileException("Cannot cast to type, the value exceeds the maximum possible value of the target type.");
            if (static_cast<int64_t>(tValue) < static_cast<int64_t>(std::numeric_limits<TTargetType>::min()))
                throw CCompileException("Cannot cast to type, the value is below the minumum possible value of the target type.");
            return static_cast<TTargetType>(tValue);
        }
        if constexpr (std::is_unsigned_v<TTargetType> && std::is_unsigned_v<TVariantType>)
        {
            TVariantType tValue = std::get<TVariantType>(m_varValue);
            if (static_cast<uint64_t>(tValue) > static_cast<uint64_t>(std::numeric_limits<TTargetType>::max()))
                throw CCompileException("Cannot cast to type, the value exceeds the maximum possible value of the target type.");
            if (static_cast<uint64_t>(tValue) < static_cast<uint64_t>(std::numeric_limits<TTargetType>::min()))
                throw CCompileException("Cannot cast to type, the value is below the minumum possible value of the target type.");
            return static_cast<TTargetType>(tValue);
        }
        if constexpr (std::is_signed_v<TTargetType> && std::is_unsigned_v<TVariantType>)
        {
            TVariantType tValue = std::get<TVariantType>(m_varValue);
            if (static_cast<uint64_t>(tValue) > static_cast<uint64_t>(std::numeric_limits<TTargetType>::max()))
                throw CCompileException("Cannot cast to type, the value exceeds the maximum possible value of the target type.");
            return static_cast<TTargetType>(tValue);
        }
        if constexpr (std::is_unsigned_v<TTargetType> && std::is_signed_v<TVariantType>)
        {
            TVariantType tValue = std::get<TVariantType>(m_varValue);
            if (tValue > 0 && static_cast<uint64_t>(tValue) > static_cast<uint64_t>(std::numeric_limits<TTargetType>::max()))
                throw CCompileException("Cannot cast to type, the value exceeds the maximum possible value of the target type.");
            else if (tValue < 0 && static_cast<int64_t>(tValue) <
                static_cast<int64_t>(std::numeric_limits<std::make_signed_t<TTargetType>>::min()))
                throw CCompileException("Cannot cast to type, the value is below the minumum possible value of the signed version"
                                        " of the target type.");
            return static_cast<TTargetType>(tValue);
        }
    }
    else if constexpr (std::is_same_v<TTargetType, bool> && std::is_arithmetic_v<TVariantType>)
        return std::get<TVariantType>(m_varValue) != static_cast<TVariantType>(0) ? true : false;
    else
    {
        // Conversion not possible
        throw CCompileException("Cannot cast to target type, the types are incompatible.");
    }
}

#endif // !defined(CONSTVARIANT_INL)