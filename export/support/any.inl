/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef SDV_ANY_INL
#define SDV_ANY_INL

#ifndef SDV_ANY_H
#error Do not include "any.inl" directly. Include "any.h" instead!
#endif //!defined SDV_ANY_H

#include "string.h"

// Prevent warnings about unitialized union members during static code analysis.
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 26495)
#endif

namespace sdv
{
    /**
     * @brief Range overflow exception.
     */
    except ERange
    {};

    inline any_t::any_t()
    {}

    inline any_t::~any_t()
    {
        clear();
    }

    inline any_t::any_t(bool bValParam) : any_t()
    {
        eValType = EValType::val_type_bool;
        bVal = bValParam;
    }

    inline any_t::any_t(int8_t iVal) : any_t()
    {
        eValType = EValType::val_type_int8;
        i8Val = iVal;
    }

    inline any_t::any_t(int16_t iVal) : any_t()
    {
        eValType = EValType::val_type_int16;
        i16Val = iVal;
    }

    inline any_t::any_t(int32_t iVal) : any_t()
    {
        eValType = EValType::val_type_int32;
        i32Val = iVal;
    }

#ifdef _WIN32
    inline any_t::any_t(long iVal) : any_t()
    {
        eValType = EValType::val_type_int32;
        i32Val = iVal;
    }
#endif

    inline any_t::any_t(int64_t iVal) : any_t()
    {
        eValType = EValType::val_type_int64;
        i64Val = iVal;
    }

#ifdef __linux__
    inline any_t::any_t(long long int iVal) : any_t()
    {
        eValType = EValType::val_type_int64;
        i64Val = iVal;
    }
#endif

    inline any_t::any_t(uint8_t uiVal) : any_t()
    {
        eValType = EValType::val_type_uint8;
        ui8Val = uiVal;
    }

    inline any_t::any_t(uint16_t uiVal) : any_t()
    {
        eValType = EValType::val_type_uint16;
        ui16Val = uiVal;
    }

    inline any_t::any_t(uint32_t uiVal) : any_t()
    {
        eValType = EValType::val_type_uint32;
        ui32Val = uiVal;
    }

#ifdef _WIN32
    inline any_t::any_t(unsigned long uiVal) : any_t()
    {
        eValType = EValType::val_type_uint32;
        ui32Val = uiVal;
    }
#endif

    inline any_t::any_t(uint64_t uiVal) : any_t()
    {
        eValType = EValType::val_type_uint64;
        ui64Val = uiVal;
    }

#ifdef __linux__
    inline any_t::any_t(unsigned long long int uiVal) : any_t()
    {
        eValType = EValType::val_type_uint64;
        ui64Val = uiVal;
    }
#endif

    inline any_t::any_t(char cValParam) : any_t()
    {
        eValType = EValType::val_type_char;
        cVal = cValParam;
    }

    inline any_t::any_t(char16_t cValParam) : any_t()
    {
        eValType = EValType::val_type_char16;
        c16Val = cValParam;
    }

    inline any_t::any_t(char32_t cValParam) : any_t()
    {
        eValType = EValType::val_type_char32;
        c32Val = cValParam;
    }

    inline any_t::any_t(wchar_t cValParam) : any_t()
    {
        eValType = EValType::val_type_wchar;
        cwVal = cValParam;
    }

    inline any_t::any_t(float fValParam) : any_t()
    {
        eValType = EValType::val_type_float;
        fVal = fValParam;
    }

    inline any_t::any_t(double fValParam) : any_t()
    {
        eValType = EValType::val_type_double;
        dVal = fValParam;
    }

    inline any_t::any_t(long double fValParam) : any_t()
    {
        eValType = EValType::val_type_long_double;
        ldVal = fValParam;
    }

    inline any_t::any_t(const string& rssVal) : any_t()
    {
        eValType = EValType::val_type_string;
        new (&ssVal) string(rssVal);
    }

    inline any_t::any_t(const u8string& rssVal) : any_t()
    {
        eValType = EValType::val_type_u8string;
        new (&ss8Val) u8string(rssVal);
    }

    inline any_t::any_t(const u16string& rssVal) : any_t()
    {
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(rssVal);
    }

    inline any_t::any_t(const u32string& rssVal) : any_t()
    {
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(rssVal);
    }

    inline any_t::any_t(const wstring& rssVal) : any_t()
    {
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(rssVal);
    }

    inline any_t::any_t(const char* szVal) : any_t()
    {
        eValType = EValType::val_type_u8string;
        new (&ss8Val) u8string(szVal);
    }
    
    inline any_t::any_t(const char16_t* szVal) : any_t()
    {
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(szVal);
    }

    inline any_t::any_t(const char32_t* szVal) : any_t()
    {
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(szVal);
    }

    inline any_t::any_t(const wchar_t* szVal) : any_t()
    {
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(szVal);
    }

    inline any_t::any_t(const std::string& rssVal) : any_t()
    {
        eValType = EValType::val_type_string;
        new (&ssVal) string(rssVal);
    }

    inline any_t::any_t(const std::u16string& rssVal) : any_t()
    {
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(rssVal);
    }

    inline any_t::any_t(const std::u32string& rssVal) : any_t()
    {
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(rssVal);
    }

    inline any_t::any_t(const std::wstring& rssVal) : any_t()
    {
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(rssVal);
    }

    inline any_t::any_t(const std::filesystem::path& rpathVal) : any_t(rpathVal.generic_u8string())
    {}

    inline any_t::any_t(interface_t ifcValParam) : any_t()
    {
        eValType = EValType::val_type_interface;
        new (&ifcVal) interface_t(ifcValParam);
    }

    // Assignment already covered by any_t(uint64)
    //inline any_t::any_t(interface_id idIfcVal) : any_t()
    //{
    //    eValType = EValType::val_type_interface_id;
    //    new (&idIfcVal) interface_id(idIfcVal);
    //}

    // Assignment already covered by any_t(uint64)
    //inline any_t::any_t(exception_id idExceptVal) : any_t()
    //{
    //    eValType = EValType::val_type_exception_id;
    //    new (&idIfcVal) exception_id(idIfcValParam);
    //}

    template <typename TEnum, typename TEnable>
    inline any_t::any_t(TEnum eVal) : any_t()
    {
        static_assert(std::is_enum_v<TEnum>, "The type is not supported to be converted to sdv::any_t!");
        set(static_cast<std::underlying_type_t<TEnum>>(eVal));
    }

    template <typename TType>
    inline any_t::any_t(TType tVal, EValType eValTypeParam) : any_t()
    {
        set(tVal, eValTypeParam);
    }

    inline any_t::any_t(const any_t& rany) : eValType(rany.eValType)
    {
        switch (eValType)
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        case EValType::val_type_bool:           bVal        = rany.bVal;                break;
        case EValType::val_type_int8:           i8Val       = rany.i8Val;               break;
        case EValType::val_type_uint8:          ui8Val      = rany.ui8Val;              break;
        case EValType::val_type_int16:          i16Val      = rany.i16Val;              break;
        case EValType::val_type_uint16:         ui16Val     = rany.ui16Val;             break;
        case EValType::val_type_int32:          i32Val      = rany.i32Val;              break;
        case EValType::val_type_uint32:         ui32Val     = rany.ui32Val;             break;
        case EValType::val_type_int64:          i64Val      = rany.i64Val;              break;
        case EValType::val_type_uint64:         ui64Val     = rany.ui64Val;             break;
        case EValType::val_type_char:           cVal        = rany.cVal;                break;
        case EValType::val_type_char16:         c16Val      = rany.c16Val;              break;
        case EValType::val_type_char32:         c32Val      = rany.c32Val;              break;
        case EValType::val_type_wchar:          cwVal       = rany.cwVal;               break;
        case EValType::val_type_float:          fVal        = rany.fVal;                break;
        case EValType::val_type_double:         dVal        = rany.dVal;                break;
        case EValType::val_type_long_double:    ldVal       = rany.ldVal;               break;
        //case EValType::val_type_fixed:          new (&fixValue) fixed(rany.fixValue);   break;
        case EValType::val_type_string:         new (&ssVal) string(rany.ssVal);        break;
        case EValType::val_type_u8string:       new (&ss8Val) u8string(rany.ss8Val);    break;
        case EValType::val_type_u16string:      new (&ss16Val) u16string(rany.ss16Val); break;
        case EValType::val_type_u32string:      new (&ss32Val) u32string(rany.ss32Val); break;
        case EValType::val_type_wstring:        new (&sswVal) wstring(rany.sswVal);     break;
        case EValType::val_type_interface:      new (&ifcVal) interface_t(rany.ifcVal); break;
        case EValType::val_type_interface_id:   idIfcVal    = rany.idIfcVal;            break;
        case EValType::val_type_exception_id:   idExceptVal = rany.idExceptVal;         break;
        default:                                                                        break;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }
    }

    inline any_t::any_t(any_t&& rany) noexcept : eValType(rany.eValType)
    {
        switch (eValType)
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        case EValType::val_type_bool:           bVal        = rany.bVal;                            break;
        case EValType::val_type_int8:           i8Val       = rany.i8Val;                           break;
        case EValType::val_type_uint8:          ui8Val      = rany.ui8Val;                          break;
        case EValType::val_type_int16:          i16Val      = rany.i16Val;                          break;
        case EValType::val_type_uint16:         ui16Val     = rany.ui16Val;                         break;
        case EValType::val_type_int32:          i32Val      = rany.i32Val;                          break;
        case EValType::val_type_uint32:         ui32Val     = rany.ui32Val;                         break;
        case EValType::val_type_int64:          i64Val      = rany.i64Val;                          break;
        case EValType::val_type_uint64:         ui64Val     = rany.ui64Val;                         break;
        case EValType::val_type_char:           cVal        = rany.cVal;                            break;
        case EValType::val_type_char16:         c16Val      = rany.c16Val;                          break;
        case EValType::val_type_char32:         c32Val      = rany.c32Val;                          break;
        case EValType::val_type_wchar:          cwVal       = rany.cwVal;                           break;
        case EValType::val_type_float:          fVal        = rany.fVal;                            break;
        case EValType::val_type_double:         dVal        = rany.dVal;                            break;
        case EValType::val_type_long_double:    ldVal       = rany.ldVal;                           break;
        //case EValType::val_type_fixed:          new (&fixValue) fixed(std::move(rany.fixValue));    break;
        case EValType::val_type_string:         new (&ssVal) string(std::move(rany.ssVal));         break;
        case EValType::val_type_u8string:       new (&ss8Val) u8string(std::move(rany.ss8Val));     break;
        case EValType::val_type_u16string:      new (&ss16Val) u16string(std::move(rany.ss16Val));  break;
        case EValType::val_type_u32string:      new (&ss32Val) u32string(std::move(rany.ss32Val));  break;
        case EValType::val_type_wstring:        new (&sswVal) wstring(std::move(rany.sswVal));      break;
        case EValType::val_type_interface:      new (&ifcVal) interface_t(std::move(rany.ifcVal));  break;
        case EValType::val_type_interface_id:   idIfcVal    = rany.idIfcVal;                        break;
        case EValType::val_type_exception_id:   idExceptVal = rany.idExceptVal;                     break;
        default:                                                                                    break;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }
        rany.eValType = EValType::val_type_empty;
    }

    inline any_t& any_t::operator=(bool bValParam)
    {
        set(bValParam);
        return *this;
    }

    inline any_t& any_t::operator=(int8_t iVal)
    {
        set(iVal);
        return *this;
    }

    inline any_t& any_t::operator=(int16_t iVal)
    {
        set(iVal);
        return *this;
    }

    inline any_t& any_t::operator=(int32_t iVal)
    {
        set(iVal);
        return *this;
    }
    
#ifdef _WIN32
    inline any_t& any_t::operator=(long iVal)
    {
        set(iVal);
        return *this;
    }
#endif
    
    inline any_t& any_t::operator=(int64_t iVal)
    {
        set(iVal);
        return *this;
    }

#ifdef __linux__
    inline any_t& any_t::operator=(long long int iVal)
    {
        set(iVal);
        return *this;
    }
#endif

    inline any_t& any_t::operator=(uint8_t uiVal)
    {
        set(uiVal);
        return *this;
    }

    inline any_t& any_t::operator=(uint16_t uiVal)
    {
        set(uiVal);
        return *this;
    }

    inline any_t& any_t::operator=(uint32_t uiVal)
    {
        set(uiVal);
        return *this;
    }

#ifdef _WIN32
    inline any_t& any_t::operator=(unsigned long uiVal)
    {
        set(uiVal);
        return *this;
    }
#endif

    inline any_t& any_t::operator=(uint64_t uiVal)
    {
        set(uiVal);
        return *this;
    }

#ifdef __linux__
    inline any_t& any_t::operator=(unsigned long long int uiVal)
    {
        set(uiVal);
        return *this;
    }
#endif

    inline any_t& any_t::operator=(char cValParam)
    {
        set(cValParam);
        return *this;
    }

    inline any_t& any_t::operator=(char16_t cValParam)
    {
        set(cValParam);
        return *this;
    }

    inline any_t& any_t::operator=(char32_t cValParam)
    {
        set(cValParam);
        return *this;
    }

    inline any_t& any_t::operator=(wchar_t cValParam)
    {
        set(cValParam);
        return *this;
    }

    inline any_t& any_t::operator=(float fValParam)
    {
        set(fValParam);
        return *this;
    }

    inline any_t& any_t::operator=(double fValParam)
    {
        set(fValParam);
        return *this;
    }

    inline any_t& any_t::operator=(long double fValParam)
    {
        set(fValParam);
        return *this;
    }

    inline any_t& any_t::operator=(const string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const u8string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const u16string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const u32string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const wstring& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const char* szVal)
    {
        set(szVal);
        return *this;
    }

    inline any_t& any_t::operator=(const char16_t* szVal)
    {
        set(szVal);
        return *this;
    }

    inline any_t& any_t::operator=(const char32_t* szVal)
    {
        set(szVal);
        return *this;
    }

    inline any_t& any_t::operator=(const wchar_t* szVal)
    {
        set(szVal);
        return *this;
    }

    inline any_t& any_t::operator=(const std::string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const std::u16string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const std::u32string& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const std::wstring& rssVal)
    {
        set(rssVal);
        return *this;
    }

    inline any_t& any_t::operator=(const std::filesystem::path& rpathVal)
    {
        set(rpathVal);
        return *this;
    }

    inline any_t& any_t::operator=(interface_t ifcValParam)
    {
        set(ifcValParam);
        return *this;
    }

    // Assignment already covered by operator=(uint64)
    //inline any_t& any_t::operator=(interface_id idIfcValParam)
    //{
    //    set(idIfcValParam);
    //    return *this;
    //}

    // Assignment already covered by operator=(uint64)
    //inline any_t& any_t::operator=(exception_id idExceptValParam)
    //{
    //    set(idExceptValParam);
    //    return *this;
    //}

    template <typename TEnum, typename TEnable>
    inline any_t& any_t::operator=(TEnum eVal)
    {
        static_assert(std::is_enum_v<TEnum>, "The type is not supported to be converted to sdv::any_t!");
        set(static_cast<std::underlying_type_t<TEnum>>(eVal));
        return *this;
    }

    inline any_t& any_t::operator=(const any_t& rany)
    {
        clear();
        eValType = rany.eValType;
        switch (eValType)
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        case EValType::val_type_bool:           bVal        = rany.bVal;                break;
        case EValType::val_type_int8:           i8Val       = rany.i8Val;               break;
        case EValType::val_type_uint8:          ui8Val      = rany.ui8Val;              break;
        case EValType::val_type_int16:          i16Val      = rany.i16Val;              break;
        case EValType::val_type_uint16:         ui16Val     = rany.ui16Val;             break;
        case EValType::val_type_int32:          i32Val      = rany.i32Val;              break;
        case EValType::val_type_uint32:         ui32Val     = rany.ui32Val;             break;
        case EValType::val_type_int64:          i64Val      = rany.i64Val;              break;
        case EValType::val_type_uint64:         ui64Val     = rany.ui64Val;             break;
        case EValType::val_type_char:           cVal        = rany.cVal;                break;
        case EValType::val_type_char16:         c16Val      = rany.c16Val;              break;
        case EValType::val_type_char32:         c32Val      = rany.c32Val;              break;
        case EValType::val_type_wchar:          cwVal       = rany.cwVal;               break;
        case EValType::val_type_float:          fVal        = rany.fVal;                break;
        case EValType::val_type_double:         dVal        = rany.dVal;                break;
        case EValType::val_type_long_double:    ldVal       = rany.ldVal;               break;
        //case EValType::val_type_fixed:          new (&fixValue) fixed(rany.fixValue);   break;
        case EValType::val_type_string:         new (&ssVal) string(rany.ssVal);        break;
        case EValType::val_type_u8string:       new (&ss8Val) u8string(rany.ss8Val);    break;
        case EValType::val_type_u16string:      new (&ss16Val) u16string(rany.ss16Val); break;
        case EValType::val_type_u32string:      new (&ss32Val) u32string(rany.ss32Val); break;
        case EValType::val_type_wstring:        new (&sswVal) wstring(rany.sswVal);     break;
        case EValType::val_type_interface:      new (&ifcVal) interface_t(rany.ifcVal); break;
        case EValType::val_type_interface_id:   idIfcVal    = rany.idIfcVal;            break;
        case EValType::val_type_exception_id:   idExceptVal = rany.idExceptVal;         break;
        default:                                                                        break;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }
        return *this;
    }

    inline any_t& any_t::operator=(any_t&& rany) noexcept
    {
        clear();
        eValType = rany.eValType;
        switch (eValType)
        {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        case EValType::val_type_bool:           bVal        = rany.bVal;                            break;
        case EValType::val_type_int8:           i8Val       = rany.i8Val;                           break;
        case EValType::val_type_uint8:          ui8Val      = rany.ui8Val;                          break;
        case EValType::val_type_int16:          i16Val      = rany.i16Val;                          break;
        case EValType::val_type_uint16:         ui16Val     = rany.ui16Val;                         break;
        case EValType::val_type_int32:          i32Val      = rany.i32Val;                          break;
        case EValType::val_type_uint32:         ui32Val     = rany.ui32Val;                         break;
        case EValType::val_type_int64:          i64Val      = rany.i64Val;                          break;
        case EValType::val_type_uint64:         ui64Val     = rany.ui64Val;                         break;
        case EValType::val_type_char:           cVal        = rany.cVal;                            break;
        case EValType::val_type_char16:         c16Val      = rany.c16Val;                          break;
        case EValType::val_type_char32:         c32Val      = rany.c32Val;                          break;
        case EValType::val_type_wchar:          cwVal       = rany.cwVal;                           break;
        case EValType::val_type_float:          fVal        = rany.fVal;                            break;
        case EValType::val_type_double:         dVal        = rany.dVal;                            break;
        case EValType::val_type_long_double:    ldVal       = rany.ldVal;                           break;
        //case EValType::val_type_fixed:          new (&fixValue) fixed(std::move(rany.fixValue));    break;
        case EValType::val_type_string:         new (&ssVal) string(std::move(rany.ssVal));         break;
        case EValType::val_type_u8string:       new (&ss8Val) u8string(std::move(rany.ss8Val));     break;
        case EValType::val_type_u16string:      new (&ss16Val) u16string(std::move(rany.ss16Val));  break;
        case EValType::val_type_u32string:      new (&ss32Val) u32string(std::move(rany.ss32Val));  break;
        case EValType::val_type_wstring:        new (&sswVal) wstring(std::move(rany.sswVal));      break;
        case EValType::val_type_interface:      new (&ifcVal) interface_t(std::move(rany.ifcVal));  break;
        case EValType::val_type_interface_id:   idIfcVal    = rany.idIfcVal;                        break;
        case EValType::val_type_exception_id:   idExceptVal = rany.idExceptVal;                     break;
        default:                                                                                    break;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }
        rany.eValType = EValType::val_type_empty;
        return *this;
    }

    inline any_t::operator bool() const
    {
        return get<bool>();
    }

    inline any_t::operator int8_t() const
    {
        return get<int8_t>();
    }

    inline any_t::operator uint8_t() const
    {
        return get<uint8_t>();
    }

    inline any_t::operator int16_t() const
    {
        return get<int16_t>();
    }

    inline any_t::operator uint16_t() const
    {
        return get<uint16_t>();
    }

    inline any_t::operator int32_t() const
    {
        return get<int32_t>();
    }

    inline any_t::operator uint32_t() const
    {
        return get<uint32_t>();
    }

    inline any_t::operator int64_t() const
    {
        return get<int64_t>();
    }

    inline any_t::operator uint64_t() const
    {
        return get<uint64_t>();
    }

    inline any_t::operator char() const
    {
        return get<char>();
    }

    inline any_t::operator char16_t() const
    {
        return get<char16_t>();
    }

    inline any_t::operator char32_t() const
    {
        return get<char32_t>();
    }

    inline any_t::operator wchar_t() const
    {
        return get<wchar_t>();
    }

    inline any_t::operator float() const
    {
        return get<float>();
    }

    inline any_t::operator double() const
    {
        return get<double>();
    }

    inline any_t::operator long double() const
    {
        return get<long double>();
    }

    //inline any_t::operator fixed() const
    //{
    //    return get<fixed>();
    //}

    inline any_t::operator string() const
    {
        return get<string>();
    }

    inline any_t::operator u8string() const
    {
        return get<u8string>();
    }

    inline any_t::operator u16string() const
    {
        return get<u16string>();
    }

    inline any_t::operator u32string() const
    {
        return get<u32string>();
    }

    inline any_t::operator wstring() const
    {
        return get<wstring>();
    }

    inline any_t::operator std::filesystem::path() const
    {
        return get<std::filesystem::path>();
    }

    inline any_t::operator interface_t() const
    {
        return get<interface_t>();
    }

    // Cast already covered by operator uint64()
    //inline any_t::operator interface_id() const
    //{
    //    return get<interface_id>();
    //}

    // Assignment already covered by operator uint64()
    //inline any_t::operator exception_id() const
    //{
    //    return get<exception_id>();
    //}

    inline any_t::operator std::string() const
    {
        return get<std::string>();
    }

    inline any_t::operator std::u16string() const
    {
        return get<std::u16string>();
    }

    inline any_t::operator std::u32string() const
    {
        return get<std::u32string>();
    }

    inline any_t::operator std::wstring() const
    {
        return get<std::wstring>();
    }

    inline bool any_t::empty() const
    {
        return eValType == EValType::val_type_empty;
    }

    inline void any_t::clear()
    {
        switch (eValType)
        {
        case EValType::val_type_string:         ssVal.~string_base();           break;
        case EValType::val_type_u8string:       ss8Val.~string_base();          break;
        case EValType::val_type_u16string:      ss16Val.~string_base();         break;
        case EValType::val_type_u32string:      ss32Val.~string_base();         break;
        case EValType::val_type_wstring:        sswVal.~string_base();          break;
        default:                                                                break;
        }
        eValType = EValType::val_type_empty;
    }

    inline void any_t::set(bool bValParam)
    {
        clear();
        eValType = EValType::val_type_bool;
        bVal     = bValParam;
    }

    inline void any_t::set(int8_t iVal)
    {
        clear();
        eValType = EValType::val_type_int8;
        i8Val    = iVal;
    }

    inline void any_t::set(int16_t iVal)
    {
        clear();
        eValType = EValType::val_type_int16;
        i16Val   = iVal;
    }

    inline void any_t::set(int32_t iVal)
    {
        clear();
        eValType = EValType::val_type_int32;
        i32Val   = iVal;
    }

#ifdef _WIN32
    inline void any_t::set(long iVal)
    {
        clear();
        eValType = EValType::val_type_int32;
        i32Val   = iVal;
    }
#endif

    inline void any_t::set(int64_t iVal)
    {
        clear();
        eValType = EValType::val_type_int64;
        i64Val   = iVal;
    }

#ifdef __linux__
    inline void any_t::set(long long int iVal)
    {
        clear();
        eValType = EValType::val_type_int64;
        i64Val   = iVal;
    }
#endif

    inline void any_t::set(uint8_t uiVal)
    {
        clear();
        eValType = EValType::val_type_uint8;
        ui8Val   = uiVal;
    }

    inline void any_t::set(uint16_t uiVal)
    {
        clear();
        eValType = EValType::val_type_uint16;
        ui16Val  = uiVal;
    }

    inline void any_t::set(uint32_t uiVal)
    {
        clear();
        eValType = EValType::val_type_uint32;
        ui32Val  = uiVal;
    }

#ifdef _WIN32
    inline void any_t::set(unsigned long uiVal)
    {
        clear();
        eValType = EValType::val_type_uint32;
        ui32Val  = uiVal;
    }
#endif

    inline void any_t::set(uint64_t uiVal)
    {
        clear();
        eValType = EValType::val_type_uint64;
        ui64Val  = uiVal;
    }

#ifdef __linux__
    inline void any_t::set(unsigned long long int uiVal)
    {
        clear();
        eValType = EValType::val_type_uint64;
        ui64Val  = uiVal;
    }
#endif

    inline void any_t::set(char cValParam)
    {
        clear();
        eValType = EValType::val_type_char;
        cVal     = cValParam;
    }

    inline void any_t::set(char16_t cValParam)
    {
        clear();
        eValType = EValType::val_type_char16;
        c16Val   = cValParam;
    }

    inline void any_t::set(char32_t cValParam)
    {
        clear();
        eValType = EValType::val_type_char32;
        c32Val   = cValParam;
    }

    inline void any_t::set(wchar_t cValParam)
    {
        clear();
        eValType = EValType::val_type_wchar;
        cwVal    = cValParam;
    }

    inline void any_t::set(float fValParam)
    {
        clear();
        eValType = EValType::val_type_float;
        fVal     = fValParam;
    }

    inline void any_t::set(double fValParam)
    {
        clear();
        eValType = EValType::val_type_double;
        dVal     = fValParam;
    }

    inline void any_t::set(long double fValParam)
    {
        clear();
        eValType = EValType::val_type_long_double;
        ldVal    = fValParam;
    }

    inline void any_t::set(const string& rssVal)
    {
        clear();
        eValType = EValType::val_type_string;
        new (&ssVal) string(rssVal);
    }

    inline void any_t::set(const u8string& rssVal)
    {
        clear();
        eValType = EValType::val_type_u8string;
        new (&ss8Val) u8string(rssVal);
    }

    inline void any_t::set(const u16string& rssVal)
    {
        clear();
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(rssVal);
    }

    inline void any_t::set(const u32string& rssVal)
    {
        clear();
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(rssVal);
    }

    inline void any_t::set(const wstring& rssVal)
    {
        clear();
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(rssVal);
    }

    inline void any_t::set(const char* szVal)
    {
        clear();
        eValType = EValType::val_type_u8string;
        new (&ss8Val) u8string(szVal);
    }

    inline void any_t::set(const char16_t* szVal)
    {
        clear();
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(szVal);
    }

    inline void any_t::set(const char32_t* szVal)
    {
        clear();
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(szVal);
    }

    inline void any_t::set(const wchar_t* szVal)
    {
        clear();
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(szVal);
    }

    inline void any_t::set(const std::string& rssVal)
    {
        clear();
        eValType = EValType::val_type_string;
        new (&ssVal) string(rssVal);
    }

    inline void any_t::set(const std::u16string& rssVal)
    {
        clear();
        eValType = EValType::val_type_u16string;
        new (&ss16Val) u16string(rssVal);
    }

    inline void any_t::set(const std::u32string& rssVal)
    {
        clear();
        eValType = EValType::val_type_u32string;
        new (&ss32Val) u32string(rssVal);
    }

    inline void any_t::set(const std::wstring& rssVal)
    {
        clear();
        eValType = EValType::val_type_wstring;
        new (&sswVal) wstring(rssVal);
    }

    inline void any_t::set(const std::filesystem::path& rpathVal)
    {
        set(rpathVal.generic_u8string());
    }

    inline void any_t::set(interface_t ifcValParam)
    {
        clear();
        eValType = EValType::val_type_interface;
        new (&ifcVal) interface_t(ifcValParam);
    }

    // Assignment already covered by set(uint64)
    //inline void any_t::set(interface_id idIfcValParam)
    //{
    //  clear();
    //  eValType = EValType::val_type_interface_id;
    //  idIfcVal = idIfcValParam;
    //}

    // Assignment already covered by set(uint64)
    //inline void any_t::set(exception_id idExceptValParam)
    //{
    //  clear();
    //  eValType = EValType::val_type_exception;
    //  idExceptVal = idExceptValParam;
    //}

    template <typename TEnum, typename TEnable>
    void any_t::set(TEnum eVal)
    {
        static_assert(std::is_enum_v<TEnum>, "The type is not supported to be converted to sdv::any_t!");
        set(static_cast<std::underlying_type_t<TEnum>>(eVal));
    }

    template <typename TType>
    inline void any_t::set(TType tVal, EValType eValTypeParam)
    {
        clear();
        eValType = eValTypeParam;
        switch (eValType)
        {
        case EValType::val_type_bool:           convert(tVal, bVal);            break;
        case EValType::val_type_int8:           convert(tVal, i8Val);           break;
        case EValType::val_type_uint8:          convert(tVal, ui8Val);          break;
        case EValType::val_type_int16:          convert(tVal, i16Val);          break;
        case EValType::val_type_uint16:         convert(tVal, ui16Val);         break;
        case EValType::val_type_int32:          convert(tVal, i32Val);          break;
        case EValType::val_type_uint32:         convert(tVal, ui32Val);         break;
        case EValType::val_type_int64:          convert(tVal, i64Val);          break;
        case EValType::val_type_uint64:         convert(tVal, ui64Val);         break;
        case EValType::val_type_char:           convert(tVal, cVal);            break;
        case EValType::val_type_char16:         convert(tVal, c16Val);          break;
        case EValType::val_type_char32:         convert(tVal, c32Val);          break;
        case EValType::val_type_wchar:          convert(tVal, cwVal);           break;
        case EValType::val_type_float:          convert(tVal, fVal);            break;
        case EValType::val_type_double:         convert(tVal, dVal);            break;
        case EValType::val_type_long_double:    convert(tVal, ldVal);           break;
        //case EValType::val_type_fixed:          new (&fixValue) fixed();        convert(tVal, fixValue);    break;
        case EValType::val_type_string:         new (&ssVal) string();          convert(tVal, ssVal);       break;
        case EValType::val_type_u8string:       new (&ss8Val) u8string();       convert(tVal, ss8Val);      break;
        case EValType::val_type_u16string:      new (&ss16Val) u16string();     convert(tVal, ss16Val);     break;
        case EValType::val_type_u32string:      new (&ss32Val) u32string();     convert(tVal, ss32Val);     break;
        case EValType::val_type_wstring:        new (&sswVal) wstring();        convert(tVal, sswVal);      break;
        case EValType::val_type_interface:      new (&ifcVal) interface_t();    convert(tVal, ifcVal);      break;
        case EValType::val_type_interface_id:   convert(tVal, idIfcVal);        break;
        case EValType::val_type_exception_id:   convert(tVal, idExceptVal);     break;
        default:                                                                break;
        }
    }

    template <typename TType>
    inline TType any_t::get() const
    {
        if constexpr (std::is_enum_v<TType>)
            return static_cast<TType>(get<std::underlying_type_t<TType>>());
        else
        {
            TType tVal;
            switch (eValType)
            {
            case EValType::val_type_bool:           convert(bVal, tVal);            break;
            case EValType::val_type_int8:           convert(i8Val, tVal);           break;
            case EValType::val_type_uint8:          convert(ui8Val, tVal);          break;
            case EValType::val_type_int16:          convert(i16Val, tVal);          break;
            case EValType::val_type_uint16:         convert(ui16Val, tVal);         break;
            case EValType::val_type_int32:          convert(i32Val, tVal);          break;
            case EValType::val_type_uint32:         convert(ui32Val, tVal);         break;
            case EValType::val_type_int64:          convert(i64Val, tVal);          break;
            case EValType::val_type_uint64:         convert(ui64Val, tVal);         break;
            case EValType::val_type_char:           convert(cVal, tVal);            break;
            case EValType::val_type_char16:         convert(c16Val, tVal);          break;
            case EValType::val_type_char32:         convert(c32Val, tVal);          break;
            case EValType::val_type_wchar:          convert(cwVal, tVal);           break;
            case EValType::val_type_float:          convert(fVal, tVal);            break;
            case EValType::val_type_double:         convert(dVal, tVal);            break;
            case EValType::val_type_long_double:    convert(ldVal, tVal);           break;
            //case EValType::val_type_fixed:          convert(fixValue, tVal);        break;
            case EValType::val_type_string:         convert(ssVal, tVal);           break;
            case EValType::val_type_u8string:       convert(ss8Val, tVal);          break;
            case EValType::val_type_u16string:      convert(ss16Val, tVal);         break;
            case EValType::val_type_u32string:      convert(ss32Val, tVal);         break;
            case EValType::val_type_wstring:        convert(sswVal, tVal);          break;
            case EValType::val_type_interface:      convert(ifcVal, tVal);          break;
            case EValType::val_type_interface_id:   convert(idIfcVal, tVal);        break;
            case EValType::val_type_exception_id:   convert(idExceptVal, tVal);     break;
            default:                                tVal = TType();                 break;
            }
            return tVal;
        }
    }

    namespace internal
    {
        /**
         * @brief Type classification.
        */
        enum class ETypeClass
        {
            arithmetic,     ///< Arihtmetic type classification.
            string,         ///< String type classification.
            other           ///< Non-arithmetic and non-string type .
        };
        /**
         * @brief Classify the type into arithmetic, string and others.
         * @tparam TType The type to classify.
         * @return The classification.
        */
        template <typename TType>
        inline constexpr ETypeClass Classify()
        {
            if constexpr (std::is_arithmetic_v<TType>)
                return ETypeClass::arithmetic;
            else if constexpr (std::is_same_v<TType, string> || std::is_same_v<TType, u8string> ||
                std::is_same_v<TType, u16string> || std::is_same_v<TType, u32string> || std::is_same_v<TType, wstring> ||
                std::is_same_v<TType, std::string> || std::is_same_v<TType, std::u16string> ||
                std::is_same_v<TType, std::u32string> || std::is_same_v<TType, std::wstring> ||
                std::is_same_v<TType, const char*> || std::is_same_v<TType, const char16_t*> ||
                std::is_same_v<TType, const char32_t*> || std::is_same_v<TType, const wchar_t*> ||
                std::is_same_v<TType, std::filesystem::path>)
                return ETypeClass::string;
            else 
                return ETypeClass::other;
        }
        /**
         * @brief Conversion struct from one type to another
         * @tparam TSrcType The source type.
         * @tparam TDstType The destination type.
         * @tparam eSrcClass The source type classification
         * @tparam eDstClass The destination type classification
         */
        template <typename TSrcType, typename TDstType, ETypeClass eSrcClass = Classify<TSrcType>(),
            ETypeClass eDstClass = Classify<TDstType>()>
        struct SConvert
        {
            /**
             * @brief Convert from one type to another (default implementation only assigns when the variables are identical).
             * @param[in] tVal The source value.
             * @return The target value.
             */
            static TDstType convert([[maybe_unused]] TSrcType tVal)
            {
                if constexpr (std::is_same_v<TSrcType, TDstType>)
                    return tVal;
                else
                    return TDstType();
            }
        };
        /**
         * @brief Conversion struct specialization from an arithmetic value to another.
         * @tparam TSrcType The arithmetic value type.
         * @tparam TDstType The arithmetic destrination value type.
        */
        template <typename TSrcType, typename TDstType>
        struct SConvert<TSrcType, TDstType, ETypeClass::arithmetic, ETypeClass::arithmetic>
        {
            /**
            * @brief Convert from an arithmetic value into another arithmetic value.
            * @param[in] tVal The arithmetic value.
            * @return The target value.
            */
            static TDstType convert(TSrcType tVal) { return static_cast<TDstType>(tVal); }
        };
        /**
        * @brief Conversion struct specialization for converting an arithmetic value into a string.
        * @tparam TSrcType The arithmetic value type.
        * @tparam TDstType The destination type string.
        */
        template <typename TSrcType, typename TDstType>
        struct SConvert<TSrcType, TDstType, ETypeClass::arithmetic, ETypeClass::string>
        {
            /**
            * @brief Convert from an arithmetic value to a SDV string.
            * @param[in] tVal The arithmetic value.
            * @return The SDV target string.
            */
            static TDstType convert(TSrcType tVal)
            {
                if constexpr (std::is_same_v<TDstType, std::filesystem::path>)
                    return {};
                else if constexpr (std::is_same_v<TSrcType, char>)
                {
                    char sz[] = {tVal, '\0'};
                    return MakeString<char, typename TDstType::value_type, true, 0>(sz);
                }
                else if constexpr (std::is_same_v<TSrcType, char16_t>)
                {
                    char16_t sz[] = {tVal, u'\0'};
                    return MakeString<char16_t, typename TDstType::value_type, true, 0>(sz);
                }
                else if constexpr (std::is_same_v<TSrcType, char32_t>)
                {
                    char32_t sz[] = {tVal, U'\0'};
                    return MakeString<char32_t, typename TDstType::value_type, true, 0>(sz);
                }
                else if constexpr (std::is_same_v<TSrcType, wchar_t>)
                {
                    wchar_t sz[] = {tVal, L'\0'};
                    return MakeString<wchar_t, typename TDstType::value_type, true, 0>(sz);
                }
                else
                    return MakeString<char, typename TDstType::value_type, true, 0>(std::to_string(tVal));
            }
        };
        /**
         * @brief Conversion struct specialization for converting a string into an arithmetic value.
         * @tparam TSrcType The source type string.
         * @tparam TDstType The arithmetic value type.
        */
        template <typename TSrcType, typename TDstType>
        struct SConvert<TSrcType, TDstType, ETypeClass::string, ETypeClass::arithmetic>
        {
            /**
             * @brief Convert from an arithmetic value to a SDV string.
             * @param[in] tVal The arithmetic value.
             * @return The SDV target string.
            */
            static TDstType convert(TSrcType tVal)
            {
                string ssTemp = MakeUtf8String(tVal);
                if (ssTemp.empty()) return static_cast<TDstType>(0);    // Prevent an exception.
                try
                {
                    if constexpr (std::is_integral_v<TDstType> && std::is_signed_v<TDstType>)
                        return static_cast<TDstType>(std::stoll(ssTemp));
                    else if constexpr (std::is_integral_v<TDstType> && !std::is_signed_v<TDstType>)
                        return static_cast<TDstType>(std::stoull(ssTemp));
                    else if constexpr (std::is_floating_point_v<TDstType>)
                        return static_cast<TDstType>(std::stold(ssTemp));
                }
                catch (std::exception&)
                {}
                return static_cast<TDstType>(0);
            }
        };
        /**
         * @brief Conversion struct specialization for converting one string into another.
         * @tparam TSrcType The source type string.
         * @tparam TDstType The destination type string.
        */
        template <typename TSrcType, typename TDstType>
        struct SConvert<TSrcType, TDstType, ETypeClass::string, ETypeClass::string>
        {
            /**
             * @brief Convert from one string (SDV, C or C++) to another string (SDV).
             * @param[in] rssVal The source string.
             * @return The SDV target string.
            */
            static TDstType convert(const TSrcType& rssVal)
            {
                if constexpr (std::is_pointer_v<TSrcType>)
                {
                    using TSrcCharType = std::remove_const_t<std::remove_pointer_t<TSrcType>>;
                    return MakeString<TSrcCharType, true, 0, typename TDstType::value_type, true, 0>(string_base<TSrcCharType, true, 0>(rssVal));
                }
                else if constexpr (std::is_same_v<TSrcType, std::basic_string<typename TSrcType::value_type>> &&
                        std::is_same_v<TDstType, std::basic_string<typename TDstType::value_type>>)
                    return MakeString<typename TSrcType::value_type, typename TDstType::value_type, true, 0>(rssVal);
                else if constexpr (std::is_same_v<TSrcType, std::basic_string<typename TSrcType::value_type>>)
                    return MakeString<typename TSrcType::value_type, typename TDstType::value_type, typename TDstType::is_unicode, 0>(rssVal);
                else if constexpr (std::is_same_v<TDstType, std::basic_string<typename TDstType::value_type>>)
                    return MakeString<typename TSrcType::value_type, TSrcType::is_unicode, 0, typename TDstType::value_type, true, 0>(rssVal);
                else if constexpr (std::is_same_v<TDstType, std::filesystem::path>)
                    return MakePath(rssVal);
                else
                    return MakeString<typename TSrcType::value_type, TSrcType::is_unicode, 0, typename TDstType::value_type, TDstType::is_unicode, 0>(rssVal);
            }
        };
    }

    template <typename TSourceType, typename TDestType>
    inline void any_t::convert(const TSourceType& rtSrcVal, TDestType& rtDstVal)
    {
        rtDstVal = internal::SConvert<TSourceType, TDestType>::convert(rtSrcVal);
    }

    /**
     * @brief Namespace for internal definitions.
     */
    namespace internal
    {
        /**
         * @brief Structure to help determining the conversion/target type priority during comparison.
         * @tparam TType1 First type
         * @tparam TType2 Second type
        */
        template <typename TType1, typename TType2>
        struct SCompareTypePriority
        {
            /// Are both types equal?
            static constexpr bool bEqualType = std::is_same_v<TType1, TType2>;

            /// Are both types arithmetic and one of them floating?
            static constexpr bool bFloatingPoint = std::is_arithmetic_v<TType1> && std::is_arithmetic_v<TType2> &&
                (std::is_floating_point_v<TType1> || std::is_floating_point_v<TType2>);

            /// Are both types integral types and one of the signed?
            static constexpr bool bSignedInteger = std::is_integral_v<TType1> && std::is_integral_v<TType2> &&
                (std::is_signed_v<TType1> || std::is_signed_v<TType2>);

            /// Are both types integral types and both unsigned?
            static constexpr bool bUnsignedInteger = std::is_integral_v<TType1> && std::is_integral_v<TType2> &&
                !bSignedInteger;

            /// Is the provided type a SDV string?
            /// @tparam The type to check.
            template <typename T>
            static constexpr bool bSdvString = std::is_same_v<T, sdv::string> || std::is_same_v<T, sdv::u8string> ||
                std::is_same_v<T, sdv::u16string> || std::is_same_v<T, sdv::u32string> ||
                std::is_same_v<T, sdv::wstring>;

            /// Is the provided type a STD string?
            /// @tparam The type to check.
            template <typename T>
            static constexpr bool bStdString = std::is_same_v<T, std::string> || std::is_same_v<T, std::u16string> ||
                std::is_same_v<T, std::u32string> || std::is_same_v<T, std::wstring>;

            /// Is the provided type a STD path?
            /// @tparam The type to check.
            template <typename T>
            static constexpr bool bStdPath = std::is_same_v<T, std::filesystem::path>;

            /// Are both types a SDV or STD string or one of the type SDV or STD string and the other C string?
            static constexpr bool bString = (bSdvString<TType1> || bStdString<TType1> || bStdPath<TType1>) &&
                (bSdvString<TType2> || bStdString<TType2> || bStdPath<TType2>);

            /// Type to use when any of these conditions occur.
            using TType = std::conditional_t<bEqualType, TType1,
                std::conditional_t<bFloatingPoint, long double,
                std::conditional_t<bSignedInteger, int64_t,
                std::conditional_t<bUnsignedInteger, uint64_t,
                std::conditional_t<bSdvString<TType1> && bString, TType1,
                std::conditional_t<bSdvString<TType2> && bString, TType2,  
                void>>>>>>;

            /// Is the type combination invalid?
            static constexpr bool bInvalid = std::is_same_v<TType, void>;
        };

        /**
        * @brief The type to use based on the priority extracted from both supplied types.
        * @tparam TType1 The first type.
        * @tparam TType2 The second type.
        */
        template <typename TType1, typename TType2>
        using compare_priority_t = typename SCompareTypePriority<TType1, TType2>::TType;

        /**
        * @brief Is the type combination invalid.
        * @tparam TType1 The first type.
        * @tparam TType2 The second type.
        */
        template <typename TType1, typename TType2>
        static constexpr bool compare_invalid_v = SCompareTypePriority<TType1, TType2>::bInvalid;

        /**
         * @brief Compare the values of two types based on the priority rules.
         * @tparam TType1 The type of the first value.
         * @tparam TType2 The type of the second value
         * @tparam eType Type of comparison to do.
         * @param[in] rtVal1 Reference to the first value.
         * @param[in] rtVal2 Reference to the second value.
         * @return Returns the result of the comparison.
         */
        template <typename TType1, typename TType2, sdv::any_t::ECompareType eType>
        bool Compare(const TType1& rtVal1, const TType2& rtVal2)
        {
            if constexpr (compare_invalid_v<TType1, TType2>)
            {
                if constexpr (eType == any_t::ECompareType::compare_inequal)
                    return true;
                else
                    return false;
            }
            else if constexpr (std::is_same_v<TType1, std::filesystem::path>)
                return Compare<std::string, TType2, eType>(rtVal1.generic_u8string(), rtVal2);
            else if constexpr (std::is_same_v<TType2, std::filesystem::path>)
                return Compare<TType1, std::string, eType>(rtVal1, rtVal2.generic_u8string());
            else if constexpr (eType == any_t::ECompareType::compare_equal)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) ==
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else if constexpr (eType == any_t::ECompareType::compare_inequal)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) !=
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else if constexpr (eType == any_t::ECompareType::compare_smaller)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) <
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else if constexpr (eType == any_t::ECompareType::compare_smaller_equal)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) <=
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else if constexpr (eType == any_t::ECompareType::compare_larger)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) >
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else if constexpr (eType == any_t::ECompareType::compare_larger_equal)
                return static_cast<compare_priority_t<TType1, TType2>>(rtVal1) >=
                    static_cast<compare_priority_t<TType1, TType2>>(rtVal2);
            else
                return false;
        }
    }

    template <typename TType, any_t::ECompareType eType>
    bool any_t::Compare(const TType& rtVal) const
    {
        switch (eValType)
        {
        case EValType::val_type_bool:           return internal::Compare<bool             , TType, eType>(bVal,        rtVal);    break;
        case EValType::val_type_int8:           return internal::Compare<int8_t           , TType, eType>(i8Val,       rtVal);    break;
        case EValType::val_type_uint8:          return internal::Compare<uint8_t          , TType, eType>(ui8Val,      rtVal);    break;
        case EValType::val_type_int16:          return internal::Compare<int16_t          , TType, eType>(i16Val,      rtVal);    break;
        case EValType::val_type_uint16:         return internal::Compare<uint16_t         , TType, eType>(ui16Val,     rtVal);    break;
        case EValType::val_type_int32:          return internal::Compare<int32_t          , TType, eType>(i32Val,      rtVal);    break;
        case EValType::val_type_uint32:         return internal::Compare<uint32_t         , TType, eType>(ui32Val,     rtVal);    break;
        case EValType::val_type_int64:          return internal::Compare<int64_t          , TType, eType>(i64Val,      rtVal);    break;
        case EValType::val_type_uint64:         return internal::Compare<uint64_t         , TType, eType>(ui64Val,     rtVal);    break;
        case EValType::val_type_char:           return internal::Compare<char             , TType, eType>(cVal,        rtVal);    break;
        case EValType::val_type_char16:         return internal::Compare<char16_t         , TType, eType>(c16Val,      rtVal);    break;
        case EValType::val_type_char32:         return internal::Compare<char32_t         , TType, eType>(c32Val,      rtVal);    break;
        case EValType::val_type_wchar:          return internal::Compare<wchar_t          , TType, eType>(cwVal,       rtVal);    break;
        case EValType::val_type_float:          return internal::Compare<float            , TType, eType>(fVal,        rtVal);    break;
        case EValType::val_type_double:         return internal::Compare<double           , TType, eType>(dVal,        rtVal);    break;
        case EValType::val_type_long_double:    return internal::Compare<long double      , TType, eType>(ldVal,       rtVal);    break;
            //case EValType::val_type_fixed:        return internal::Compare<sdv::fixed       , TType, eType>(fixVal,      rtVal);    break;
        case EValType::val_type_string:
            if constexpr (std::is_same_v<TType, const char*>)
                return internal::Compare<sdv::string, sdv::string, eType>(ssVal, sdv::string(rtVal));
            else
                return internal::Compare<sdv::string, TType, eType>(ssVal, rtVal);
            break;
        case EValType::val_type_u8string:
            if constexpr (std::is_same_v<TType, const char*>)
                return internal::Compare<sdv::u8string, sdv::u8string, eType>(ss8Val, sdv::u8string(rtVal));
            else
                return internal::Compare<sdv::u8string, TType, eType>(ss8Val, rtVal);
            break;
        case EValType::val_type_u16string:
            if constexpr (std::is_same_v<TType, const char16_t*>)
                return internal::Compare<sdv::u16string, sdv::u16string, eType>(ss16Val, sdv::u16string(rtVal));
            else
                return internal::Compare<sdv::u16string, TType, eType>(ss16Val, rtVal);
            break;
        case EValType::val_type_u32string:
            if constexpr (std::is_same_v<TType, const char32_t*>)
                return internal::Compare<sdv::u32string, sdv::u32string, eType>(ss32Val, sdv::u32string(rtVal));
            else
                return internal::Compare<sdv::u32string, TType, eType>(ss32Val, rtVal);
            break;
        case EValType::val_type_wstring:
            if constexpr (std::is_same_v<TType, const wchar_t*>)
                return internal::Compare<sdv::wstring, sdv::wstring, eType>(sswVal, sdv::wstring(rtVal));
            else
                return internal::Compare<sdv::wstring, TType, eType>(sswVal, rtVal);
            break;
        case EValType::val_type_interface:      return internal::Compare<sdv::interface_t , TType, eType>(ifcVal,      rtVal);    break;
        case EValType::val_type_interface_id:   return internal::Compare<sdv::interface_id, TType, eType>(idIfcVal,    rtVal);    break;
        case EValType::val_type_exception_id:   return internal::Compare<sdv::exception_id, TType, eType>(idExceptVal, rtVal);    break;
        default:                                return false;                                break;
        }
    }

    template <any_t::ECompareType eType>
    inline bool any_t::Compare(const any_t& ranyVal) const
    {
        switch (ranyVal.eValType)
        {
        case EValType::val_type_empty:          return ranyVal.eValType == EValType::val_type_empty;            break;
        case EValType::val_type_bool:           return Compare<bool             , eType>(ranyVal.bVal);         break;
        case EValType::val_type_int8:           return Compare<int8_t           , eType>(ranyVal.i8Val);        break;
        case EValType::val_type_uint8:          return Compare<uint8_t          , eType>(ranyVal.ui8Val);       break;
        case EValType::val_type_int16:          return Compare<int16_t          , eType>(ranyVal.i16Val);       break;
        case EValType::val_type_uint16:         return Compare<uint16_t         , eType>(ranyVal.ui16Val);      break;
        case EValType::val_type_int32:          return Compare<int32_t          , eType>(ranyVal.i32Val);       break;
        case EValType::val_type_uint32:         return Compare<uint32_t         , eType>(ranyVal.ui32Val);      break;
        case EValType::val_type_int64:          return Compare<int64_t          , eType>(ranyVal.i64Val);       break;
        case EValType::val_type_uint64:         return Compare<uint64_t         , eType>(ranyVal.ui64Val);      break;
        case EValType::val_type_char:           return Compare<char             , eType>(ranyVal.cVal);         break;
        case EValType::val_type_char16:         return Compare<char16_t         , eType>(ranyVal.c16Val);       break;
        case EValType::val_type_char32:         return Compare<char32_t         , eType>(ranyVal.c32Val);       break;
        case EValType::val_type_wchar:          return Compare<wchar_t          , eType>(ranyVal.cwVal);        break;
        case EValType::val_type_float:          return Compare<float            , eType>(ranyVal.fVal);         break;
        case EValType::val_type_double:         return Compare<double           , eType>(ranyVal.dVal);         break;
        case EValType::val_type_long_double:    return Compare<long double      , eType>(ranyVal.ldVal);        break;
        //case EValType::val_type_fixed:        return Compare<sdv::fixed       , eType>(ranyVal.fixVal);       break;
        case EValType::val_type_string:         return Compare<sdv::string      , eType>(ranyVal.ssVal);        break;
        case EValType::val_type_u8string:       return Compare<sdv::u8string    , eType>(ranyVal.ss8Val);       break;
        case EValType::val_type_u16string:      return Compare<sdv::u16string   , eType>(ranyVal.ss16Val);      break;
        case EValType::val_type_u32string:      return Compare<sdv::u32string   , eType>(ranyVal.ss32Val);      break;
        case EValType::val_type_wstring:        return Compare<sdv::wstring     , eType>(ranyVal.sswVal);       break;
        case EValType::val_type_interface:      return Compare<sdv::interface_t , eType>(ranyVal.ifcVal);       break;
        case EValType::val_type_interface_id:   return Compare<sdv::interface_id, eType>(ranyVal.idIfcVal);     break;
        case EValType::val_type_exception_id:   return Compare<sdv::exception_id, eType>(ranyVal.idExceptVal);  break;
        default:                                return false;                                break;
        }
    }

    template <typename TType>
    inline bool operator==(const sdv::any_t& ranyVal1, TType tVal2)
    {
        if constexpr (std::is_enum_v<TType>)
            return ranyVal1.Compare<std::underlying_type_t<TType>, any_t::ECompareType::compare_equal>(
                static_cast<std::underlying_type_t<TType>>(tVal2));
        else
            return ranyVal1.Compare<TType, any_t::ECompareType::compare_equal>(tVal2);
    }

    template <typename TType>
    inline bool operator==(TType tVal1, const sdv::any_t& ranyVal2)
    {
        if constexpr (std::is_enum_v<TType>)
            return ranyVal2.Compare<std::underlying_type_t<TType>, any_t::ECompareType::compare_equal>(
                static_cast<std::underlying_type_t<TType>>(tVal1));
        else
            return ranyVal2.Compare<TType, any_t::ECompareType::compare_equal>(tVal1);
    }

    inline bool operator==(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_equal>(ranyVal2);
    }

    template <typename TType>
    inline bool operator!=(const sdv::any_t& ranyVal1, TType tVal2)
    {
        if constexpr (std::is_enum_v<TType>)
            return ranyVal1.Compare<std::underlying_type_t<TType>, any_t::ECompareType::compare_inequal>(
                static_cast<std::underlying_type_t<TType>>(tVal2));
        else
            return ranyVal1.Compare<TType, any_t::ECompareType::compare_inequal>(tVal2);
    }

    template <typename TType>
    inline bool operator!=(TType tVal1, const sdv::any_t& ranyVal2)
    {
        if constexpr (std::is_enum_v<TType>)
            return ranyVal2.Compare<std::underlying_type_t<TType>, any_t::ECompareType::compare_inequal>(
                static_cast<std::underlying_type_t<TType>>(tVal1));
        else
            return ranyVal2.Compare<TType, any_t::ECompareType::compare_inequal>(tVal1);
    }

    inline bool operator!=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_inequal>(ranyVal2);
    }

    template <typename TType>
    inline bool operator<(const sdv::any_t& ranyVal1, TType tVal2)
    {
        return ranyVal1.Compare<TType, any_t::ECompareType::compare_smaller>(tVal2);
    }

    template <typename TType>
    inline bool operator<(TType tVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal2.Compare<TType, any_t::ECompareType::compare_larger>(tVal1);
    }

    inline bool operator<(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_smaller>(ranyVal2);
    }

    template <typename TType>
    inline bool operator<=(const sdv::any_t& ranyVal1, TType tVal2)
    {
        return ranyVal1.Compare<TType, any_t::ECompareType::compare_smaller_equal>(tVal2);
    }

    template <typename TType>
    inline bool operator<=(TType tVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal2.Compare<TType, any_t::ECompareType::compare_larger_equal>(tVal1);
    }

    inline bool operator<=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_smaller_equal>(ranyVal2);
    }

    template <typename TType>
    inline bool operator>(const sdv::any_t& ranyVal1, TType tVal2)
    {
        return ranyVal1.Compare<TType, any_t::ECompareType::compare_larger>(tVal2);
    }

    template <typename TType>
    inline bool operator>(TType tVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal2.Compare<TType, any_t::ECompareType::compare_smaller>(tVal1);
    }

    inline bool operator>(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_larger>(ranyVal2);
    }

    template <typename TType>
    inline bool operator>=(const sdv::any_t& ranyVal1, TType tVal2)
    {
        return ranyVal1.Compare<TType, any_t::ECompareType::compare_larger_equal>(tVal2);
    }

    template <typename TType>
    inline bool operator>=(TType tVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal2.Compare<TType, any_t::ECompareType::compare_smaller_equal>(tVal1);
    }

    inline bool operator>=(const sdv::any_t& ranyVal1, const sdv::any_t& ranyVal2)
    {
        return ranyVal1.Compare<any_t::ECompareType::compare_larger_equal>(ranyVal2);
    }
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif // !defined SDV_ANY_INL