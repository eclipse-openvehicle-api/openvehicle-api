#include <support/any.h>
#include <sstream>
#include "basic_types_test.h"

using CAnyTypeTest = CBasicTypesTest;

TEST_F(CAnyTypeTest, EmptyAny)
{
    sdv::any_t any;
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_TRUE(any.empty());
}

TEST_F(CAnyTypeTest, ValueConstructor)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64.ui64Val, 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anychar.cVal, 'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16char.c16Val, u'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32char.c32Val, U'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywchar.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_float);
    float f = 10.1234f;
    EXPECT_EQ(anyfloat.fVal, f);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_double);
    double d = 10.1234;
    EXPECT_EQ(anydouble.dVal, d);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_long_double);
    long double ld = 10.1234l;
    EXPECT_EQ(anylongdouble.ldVal, ld);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(anyfixed.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyfixed.fixVal, ???);

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    //EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystring.ssVal, "hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8string.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16string.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32string.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstring.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterface.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    //EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceid.idIfcVal, static_cast<sdv::interface_id>(1234u));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    //EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionid.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, CopyConstructor)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    sdv::any_t anyint8Copy(anyint8);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8.i8Val, 66);
    EXPECT_EQ(anyint8Copy.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8Copy.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    sdv::any_t anyuint8Copy(anyuint8);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8.ui8Val, 66u);
    EXPECT_EQ(anyuint8Copy.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8Copy.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    sdv::any_t anyint16Copy(anyint16);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16.i16Val, 66);
    EXPECT_EQ(anyint16Copy.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16Copy.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    sdv::any_t anyuint16Copy(anyuint16);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16.ui16Val, 66u);
    EXPECT_EQ(anyuint16Copy.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16Copy.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    sdv::any_t anyint32Copy(anyint32);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32.i32Val, 66);
    EXPECT_EQ(anyint32Copy.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32Copy.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    sdv::any_t anyuint32Copy(anyuint32);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32.ui32Val, 66u);
    EXPECT_EQ(anyuint32Copy.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32Copy.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    sdv::any_t anyint64Copy(anyint64);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64.i64Val, 66);
    EXPECT_EQ(anyint64Copy.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64Copy.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    sdv::any_t anyuint64Copy(anyuint64);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64.ui64Val, 66u);
    EXPECT_EQ(anyuint64Copy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64Copy.ui64Val, 66u);

    sdv::any_t anychar('A');
    sdv::any_t anycharCopy(anychar);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anychar.cVal, 'A');
    EXPECT_EQ(anycharCopy.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anycharCopy.cVal, 'A');

    sdv::any_t anyu16char(u'A');
    sdv::any_t anyu16charCopy(anyu16char);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16char.c16Val, u'A');
    EXPECT_EQ(anyu16charCopy.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16charCopy.c16Val, u'A');

    sdv::any_t anyu32char(U'A');
    sdv::any_t anyu32charCopy(anyu32char);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32char.c32Val, U'A');
    EXPECT_EQ(anyu32charCopy.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32charCopy.c32Val, U'A');

    sdv::any_t anywchar(L'A');
    sdv::any_t anywcharCopy(anywchar);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywchar.cwVal, L'A');
    EXPECT_EQ(anywcharCopy.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywcharCopy.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f);
    sdv::any_t anyfloatCopy(anyfloat);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_float);
    float f = 10.1234f;
    EXPECT_EQ(anyfloat.fVal, f);
    EXPECT_EQ(anyfloatCopy.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyfloatCopy.fVal, f);

    sdv::any_t anydouble(10.1234);
    sdv::any_t anydoubleCopy(anydouble);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_double);
    double d = 10.1234;
    EXPECT_EQ(anydouble.dVal, d);
    EXPECT_EQ(anydoubleCopy.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anydoubleCopy.dVal, d);

    sdv::any_t anylongdouble(10.1234l);
    sdv::any_t anylongdoubleCopy(anylongdouble);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_long_double);
    long double ld = 10.1234l;
    EXPECT_EQ(anylongdouble.ldVal, ld);
    EXPECT_EQ(anylongdoubleCopy.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anylongdoubleCopy.ldVal, ld);

    //sdv::any_t anyfixed(???);
    //sdv::any_t anyfixedCopy(anyfixed);
    //EXPECT_EQ(anyfixed.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyfixed.fixVal, ???);
    //EXPECT_EQ(anyfixedCopy.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyfixedCopy.fixVal, ???);

    sdv::any_t anystring("hello");
    sdv::any_t anystringCopy(anystring);
    // ANSI string is detected as UTF-8 string
    //EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystring.ssVal, "hello");
    EXPECT_EQ(anystringCopy.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystringCopy.ssVal, "hello");

    sdv::any_t anyu8string("hello");
    sdv::any_t anyu8stringCopy(anyu8string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8string.ss8Val, "hello");
    EXPECT_EQ(anyu8stringCopy.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8stringCopy.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello");
    sdv::any_t anyu16stringCopy(anyu16string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16string.ss16Val, u"hello");
    EXPECT_EQ(anyu16stringCopy.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16stringCopy.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello");
    sdv::any_t anyu32stringCopy(anyu32string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32string.ss32Val, U"hello");
    EXPECT_EQ(anyu32stringCopy.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32stringCopy.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello");
    sdv::any_t anywstringCopy(anywstring);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstring.sswVal, L"hello");
    EXPECT_EQ(anywstringCopy.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstringCopy.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    sdv::any_t anyinterfaceCopy(anyinterface);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterface.ifcVal);
    EXPECT_EQ(anyinterfaceCopy.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterfaceCopy.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    sdv::any_t anyinterfaceidCopy(anyinterfaceid);
    // interface_id is detected as uin64_t
    //EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceid.idIfcVal, static_cast<sdv::interface_id>(1234u));
    EXPECT_EQ(anyinterfaceidCopy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceidCopy.idIfcVal, static_cast<sdv::interface_id>(1234u));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    sdv::any_t anyexceptionidCopy(anyexceptionid);
    // exception_id is detected as uin64_t
    //EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionid.idExceptVal, static_cast<sdv::exception_id>(1234u));
    EXPECT_EQ(anyexceptionidCopy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionidCopy.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, ImplicitCStringConstructor)
{
    sdv::any_t anyString(u8"Hello");
    EXPECT_EQ(static_cast<sdv::u8string>(anyString), u8"Hello");
    sdv::any_t anyu16String(u"Hello");
    EXPECT_EQ(static_cast<sdv::u16string>(anyu16String), u"Hello");
    sdv::any_t anyu32String(U"Hello");
    EXPECT_EQ(static_cast<sdv::u32string>(anyu32String), U"Hello");
    sdv::any_t anywString(L"Hello");
    EXPECT_EQ(static_cast<sdv::wstring>(anywString), L"Hello");

    const char* szu8 = u8"Hello";
    sdv::any_t anyString2(szu8);
    EXPECT_EQ(static_cast<sdv::u8string>(anyString2), u8"Hello");
    const char16_t* szu16 = u"Hello";
    sdv::any_t anyu16String2(szu16);
    EXPECT_EQ(static_cast<sdv::u16string>(anyu16String2), u"Hello");
    const char32_t* szu32 = U"Hello";
    sdv::any_t anyu32String2(szu32);
    EXPECT_EQ(static_cast<sdv::u32string>(anyu32String2), U"Hello");
    const wchar_t* szw = L"Hello";
    sdv::any_t anywString2(szw);
    EXPECT_EQ(static_cast<sdv::wstring>(anywString2), L"Hello");

    szu8 = nullptr;
    sdv::any_t anyString3(szu8);
    EXPECT_TRUE(static_cast<sdv::u8string>(anyString3).empty());
    szu16 = nullptr;
    sdv::any_t anyu16String3(szu16);
    EXPECT_TRUE(static_cast<sdv::u16string>(anyu16String3).empty());
    szu32 = nullptr;
    sdv::any_t anyu32String3(szu32);
    EXPECT_TRUE(static_cast<sdv::u32string>(anyu32String3).empty());
    szw = nullptr;
    sdv::any_t anywString3(szw);
    EXPECT_TRUE(static_cast<sdv::wstring>(anywString3).empty());
}

TEST_F(CAnyTypeTest, MoveConstructor)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    sdv::any_t anyint8Move(std::move(anyint8));
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyint8Move.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8Move.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    sdv::any_t anyuint8Move(std::move(anyuint8));
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyuint8Move.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8Move.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    sdv::any_t anyint16Move(std::move(anyint16));
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyint16Move.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16Move.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    sdv::any_t anyuint16Move(std::move(anyuint16));
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyuint16Move.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16Move.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    sdv::any_t anyint32Move(std::move(anyint32));
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyint32Move.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32Move.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    sdv::any_t anyuint32Move(std::move(anyuint32));
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyuint32Move.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32Move.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    sdv::any_t anyint64Move(std::move(anyint64));
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyint64Move.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64Move.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    sdv::any_t anyuint64Move(std::move(anyuint64));
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyuint64Move.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64Move.ui64Val, 66u);

    sdv::any_t anychar('A');
    sdv::any_t anycharMove(std::move(anychar));
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anycharMove.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anycharMove.cVal, 'A');

    sdv::any_t anyu16char(u'A');
    sdv::any_t anyu16charMove(std::move(anyu16char));
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyu16charMove.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16charMove.c16Val, u'A');

    sdv::any_t anyu32char(U'A');
    sdv::any_t anyu32charMove(std::move(anyu32char));
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyu32charMove.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32charMove.c32Val, U'A');

    sdv::any_t anywchar(L'A');
    sdv::any_t anywcharMove(std::move(anywchar));
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anywcharMove.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywcharMove.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f);
    sdv::any_t anyfloatMove(std::move(anyfloat));
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_empty);
    float f = 10.1234f;
    EXPECT_EQ(anyfloatMove.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyfloatMove.fVal, f);

    sdv::any_t anydouble(10.1234);
    sdv::any_t anydoubleMove(std::move(anydouble));
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_empty);
    double d = 10.1234;
    EXPECT_EQ(anydoubleMove.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anydoubleMove.dVal, d);

    sdv::any_t anylongdouble(10.1234l);
    sdv::any_t anylongdoubleMove(std::move(anylongdouble));
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_empty);
    long double ld = 10.1234l;
    EXPECT_EQ(anylongdoubleMove.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anylongdoubleMove.ldVal, ld);

    //sdv::any_t anyfixed(???);
    //sdv::any_t anyfixedMove(std::move(anyfixed));
    //EXPECT_EQ(anyfixed.eValType, sdv::any_t::EValType::val_type_empty);
    //EXPECT_EQ(anyfixedMove.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyfixedMove.fixVal, ???);

    sdv::any_t anystring("hello");
    sdv::any_t anystringMove(std::move(anystring));
    // ANSI string is detected as UTF-8 string
    //EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anystringMove.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystringMove.ssVal, "hello");

    sdv::any_t anyu8string("hello");
    sdv::any_t anyu8stringMove(std::move(anyu8string));
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyu8stringMove.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8stringMove.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello");
    sdv::any_t anyu16stringMove(std::move(anyu16string));
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyu16stringMove.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16stringMove.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello");
    sdv::any_t anyu32stringMove(std::move(anyu32string));
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyu32stringMove.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32stringMove.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello");
    sdv::any_t anywstringMove(std::move(anywstring));
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anywstringMove.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstringMove.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    sdv::any_t anyinterfaceMove(std::move(anyinterface));
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyinterfaceMove.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterfaceMove.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    sdv::any_t anyinterfaceidMove(std::move(anyinterfaceid));
    // interface_id is detected as uin64_t
    //EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyinterfaceidMove.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceidMove.idIfcVal, static_cast<sdv::interface_id>(1234u));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    sdv::any_t anyexceptionidMove(std::move(anyexceptionid));
    // exception_id is detected as uin64_t
    //EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyexceptionidMove.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionidMove.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, ValueAssignment)
{
    sdv::any_t any;

    any = static_cast<int8_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(any.i8Val, 66);

    any = static_cast<uint8_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(any.ui8Val, 66u);

    any = static_cast<int16_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(any.i16Val, 66);

    any = static_cast<uint16_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(any.ui16Val, 66u);

    any = static_cast<int32_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(any.i32Val, 66);

    any = static_cast<uint32_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(any.ui32Val, 66u);

    any = static_cast<int64_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(any.i64Val, 66);

    any = static_cast<uint64_t>(66);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(any.ui64Val, 66u);

    any = 'A';
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(any.cVal, 'A');

    any = u'A';
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(any.c16Val, u'A');

    any = U'A';
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(any.c32Val, U'A');

    any = L'A';
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(any.cwVal, L'A');

    any = 10.1234f;
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_float);
    float f = 10.1234f;
    EXPECT_EQ(any.fVal, f);

    any = 10.1234;
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_double);
    double d = 10.1234;
    EXPECT_EQ(any.dVal, d);

    any = static_cast<long double>(10.1234l);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_long_double);
    long double ld = 10.1234l;
    EXPECT_EQ(any.ldVal, ld);

    //any = ???;
    //EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(any.fixVal, ???);

    any = "hello";
    // ANSI string is detected as UTF-8 string
    //EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(any.ssVal, "hello");

    any = "hello";
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(any.ss8Val, "hello");

    any = u"hello";
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(any.ss16Val, u"hello");

    any = U"hello";
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(any.ss32Val, U"hello");

    any = L"hello";
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(any.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    any = ifc;
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(any.ifcVal);

    any = static_cast<sdv::interface_id>(1234u);
    // interface_id is detected as uin64_t
    //EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(any.idIfcVal, static_cast<sdv::interface_id>(1234u));

    any = static_cast<sdv::exception_id>(1234u);
    // exception_id is detected as uin64_t
    //EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(any.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(any.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, CopyAssignment)
{
    sdv::any_t anyCopy;

    sdv::any_t anyint8(static_cast<int8_t>(66));
    anyCopy = anyint8;
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8.i8Val, 66);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyCopy.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    anyCopy = anyuint8;
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8.ui8Val, 66u);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyCopy.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    anyCopy = anyint16;
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16.i16Val, 66);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyCopy.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    anyCopy = anyuint16;
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16.ui16Val, 66u);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyCopy.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    anyCopy = anyint32;
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32.i32Val, 66);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyCopy.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    anyCopy = anyuint32;
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32.ui32Val, 66u);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyCopy.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    anyCopy = anyint64;
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64.i64Val, 66);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyCopy.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    anyCopy = anyuint64;
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64.ui64Val, 66u);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyCopy.ui64Val, 66u);

    sdv::any_t anychar('A');
    anyCopy = anychar;
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anychar.cVal, 'A');
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyCopy.cVal, 'A');

    sdv::any_t anyu16char(u'A');
    anyCopy = anyu16char;
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16char.c16Val, u'A');
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyCopy.c16Val, u'A');

    sdv::any_t anyu32char(U'A');
    anyCopy = anyu32char;
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32char.c32Val, U'A');
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyCopy.c32Val, U'A');

    sdv::any_t anywchar(L'A');
    anyCopy = anywchar;
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywchar.cwVal, L'A');
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyCopy.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f);
    anyCopy = anyfloat;
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_float);
    float f = 10.1234f;
    EXPECT_EQ(anyfloat.fVal, f);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyCopy.fVal, f);

    sdv::any_t anydouble(10.1234);
    anyCopy = anydouble;
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_double);
    double d = 10.1234;
    EXPECT_EQ(anydouble.dVal, d);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyCopy.dVal, d);

    sdv::any_t anylongdouble(10.1234l);
    anyCopy = anylongdouble;
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_long_double);
    long double ld = 10.1234l;
    EXPECT_EQ(anylongdouble.ldVal, ld);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyCopy.ldVal, ld);

    //sdv::any_t anyfixed(???);
    //anyCopy = anyfixed
    //EXPECT_EQ(anyfixed.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyfixed.fixVal, ???);
    //EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyCopy.fixVal, ???);

    sdv::any_t anystring("hello");
    anyCopy = anystring;
    // ANSI string is detected as UTF-8 string
    //EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystring.ssVal, "hello");
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyCopy.ssVal, "hello");

    sdv::any_t anyu8string("hello");
    anyCopy = anyu8string;
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8string.ss8Val, "hello");
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyCopy.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello");
    anyCopy = anyu16string;
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16string.ss16Val, u"hello");
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyCopy.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello");
    anyCopy = anyu32string;
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32string.ss32Val, U"hello");
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyCopy.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello");
    anyCopy = anywstring;
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstring.sswVal, L"hello");
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyCopy.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    anyCopy = anyinterface;
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterface.ifcVal);
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyCopy.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    anyCopy = anyinterfaceid;
    // interface_id is detected as uin64_t
    //EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceid.idIfcVal, static_cast<sdv::interface_id>(1234u));
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyCopy.idIfcVal, static_cast<sdv::interface_id>(1234u));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    anyCopy = anyexceptionid;
    // exception_id is detected as uin64_t
    //EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionid.idExceptVal, static_cast<sdv::exception_id>(1234u));
    EXPECT_EQ(anyCopy.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyCopy.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, MoveAssignment)
{
    sdv::any_t anyMove;

    sdv::any_t anyint8(static_cast<int8_t>(66));
    anyMove = std::move(anyint8);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyMove.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    anyMove = std::move(anyuint8);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyMove.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    anyMove = std::move(anyint16);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyMove.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    anyMove = std::move(anyuint16);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyMove.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    anyMove = std::move(anyint32);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyMove.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    anyMove = std::move(anyuint32);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyMove.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    anyMove = std::move(anyint64);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyMove.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    anyMove = std::move(anyuint64);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyMove.ui64Val, 66u);

    sdv::any_t anychar('A');
    anyMove = std::move(anychar);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyMove.cVal, 'A');

    sdv::any_t anyu16char(u'A');
    anyMove = std::move(anyu16char);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyMove.c16Val, u'A');

    sdv::any_t anyu32char(U'A');
    anyMove = std::move(anyu32char);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyMove.c32Val, U'A');

    sdv::any_t anywchar(L'A');
    anyMove = std::move(anywchar);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyMove.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f);
    anyMove = std::move(anyfloat);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_empty);
    float f = 10.1234f;
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyMove.fVal, f);

    sdv::any_t anydouble(10.1234);
    anyMove = std::move(anydouble);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_empty);
    double d = 10.1234;
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyMove.dVal, d);

    sdv::any_t anylongdouble(10.1234l);
    anyMove = std::move(anylongdouble);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_empty);
    long double ld = 10.1234l;
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyMove.ldVal, ld);

    //sdv::any_t anyfixed(???);
    //anyMove = std::move(anyfixed)
    //EXPECT_EQ(anyfixed.eValType, sdv::any_t::EValType::val_type_empty);
    //EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_fixed);
    //EXPECT_EQ(anyMove.fixVal, ???);

    sdv::any_t anystring("hello");
    anyMove = std::move(anystring);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyMove.ssVal, "hello");

    sdv::any_t anyu8string("hello");
    anyMove = std::move(anyu8string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyMove.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello");
    anyMove = std::move(anyu16string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyMove.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello");
    anyMove = std::move(anyu32string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyMove.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello");
    anyMove = std::move(anywstring);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyMove.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    anyMove = std::move(anyinterface);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyMove.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    anyMove = std::move(anyinterfaceid);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyMove.idIfcVal, static_cast<sdv::interface_id>(1234u));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    anyMove = std::move(anyexceptionid);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_empty);
    EXPECT_EQ(anyMove.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyMove.idExceptVal, static_cast<sdv::exception_id>(1234u));
}

TEST_F(CAnyTypeTest, CastOperatorInt8)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyint8), 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyuint8), 66);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyint16), 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyuint16), 66);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyint32), 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyuint32), 66);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyint64), 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<int8_t>(anyuint64), 66);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<int8_t>(anychar), 65);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<int8_t>(anyu16char), 65);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<int8_t>(anyu32char), 65);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<int8_t>(anywchar), 65);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<int8_t>(anyfloat), 10);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<int8_t>(anydouble), 10);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<int8_t>(anylongdouble), 10);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<int8_t>(anyfixed), ???);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<int8_t>(anystring), 12);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<int8_t>(anyu8string), 12);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<int8_t>(anyu16string), 12);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<int8_t>(anyu32string), 12);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<int8_t>(anywstring), 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<int8_t>(anyinterface), 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<int8_t>(anyinterfaceid), static_cast<int8_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<int8_t>(anyexceptionid), static_cast<int8_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorUInt8)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<uint8_t>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<uint8_t>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<uint8_t>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<uint8_t>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<uint8_t>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<uint8_t>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<uint8_t>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<uint8_t>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<uint8_t>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<uint8_t>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<uint8_t>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<uint8_t>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<uint8_t>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<uint8_t>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<uint8_t>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint8_t>(anyinterfaceid), static_cast<uint8_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint8_t>(anyexceptionid), static_cast<uint8_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorInt16)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyint8), 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyuint8), 66);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyint16), 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyuint16), 66);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyint32), 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyuint32), 66);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyint64), 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<int16_t>(anyuint64), 66);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<int16_t>(anychar), 65);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<int16_t>(anyu16char), 65);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<int16_t>(anyu32char), 65);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<int16_t>(anywchar), 65);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<int16_t>(anyfloat), 10);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<int16_t>(anydouble), 10);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<int16_t>(anylongdouble), 10);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<int16_t>(anyfixed), ???);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<int16_t>(anystring), 12);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<int16_t>(anyu8string), 12);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<int16_t>(anyu16string), 12);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<int16_t>(anyu32string), 12);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<int16_t>(anywstring), 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<int16_t>(anyinterface), 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<int16_t>(anyinterfaceid), static_cast<int16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<int16_t>(anyexceptionid), static_cast<int16_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorUInt16)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<uint16_t>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<uint16_t>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<uint16_t>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<uint16_t>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<uint16_t>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<uint16_t>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<uint16_t>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<uint16_t>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<uint16_t>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<uint16_t>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<uint16_t>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<uint16_t>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<uint16_t>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<uint16_t>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<uint16_t>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint16_t>(anyinterfaceid), static_cast<uint16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint16_t>(anyexceptionid), static_cast<uint16_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorInt32)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyint8), 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyuint8), 66);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyint16), 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyuint16), 66);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyint32), 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyuint32), 66);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyint64), 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<int32_t>(anyuint64), 66);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<int32_t>(anychar), 65);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<int32_t>(anyu16char), 65);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<int32_t>(anyu32char), 65);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<int32_t>(anywchar), 65);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<int32_t>(anyfloat), 10);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<int32_t>(anydouble), 10);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<int32_t>(anylongdouble), 10);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<int32_t>(anyfixed), ???);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<int32_t>(anystring), 12);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<int32_t>(anyu8string), 12);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<int32_t>(anyu16string), 12);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<int32_t>(anyu32string), 12);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<int32_t>(anywstring), 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<int32_t>(anyinterface), 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<int32_t>(anyinterfaceid), 1234);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<int32_t>(anyexceptionid), 1234);
}

TEST_F(CAnyTypeTest, CastOperatorUInt32)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<uint32_t>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<uint32_t>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<uint32_t>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<uint32_t>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<uint32_t>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<uint32_t>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<uint32_t>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<uint32_t>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<uint32_t>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<uint32_t>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<uint32_t>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<uint32_t>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<uint32_t>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<uint32_t>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<uint32_t>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint32_t>(anyinterfaceid), 1234u);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint32_t>(anyexceptionid), 1234u);
}

TEST_F(CAnyTypeTest, CastOperatorInt64)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyint8), 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyuint8), 66);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyint16), 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyuint16), 66);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyint32), 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyuint32), 66);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyint64), 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<int64_t>(anyuint64), 66);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<int64_t>(anychar), 65);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<int64_t>(anyu16char), 65);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<int64_t>(anyu32char), 65);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<int64_t>(anywchar), 65);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<int64_t>(anyfloat), 10);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<int64_t>(anydouble), 10);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<int64_t>(anylongdouble), 10);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<int64_t>(anyfixed), ???);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<int64_t>(anystring), 12);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<int64_t>(anyu8string), 12);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<int64_t>(anyu16string), 12);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<int64_t>(anyu32string), 12);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<int64_t>(anywstring), 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<int64_t>(anyinterface), 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<int64_t>(anyinterfaceid), 1234);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<int64_t>(anyexceptionid), 1234);
}

TEST_F(CAnyTypeTest, CastOperatorUInt64)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<uint64_t>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<uint64_t>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<uint64_t>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<uint64_t>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<uint64_t>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<uint64_t>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<uint64_t>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<uint64_t>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<uint64_t>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<uint64_t>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<uint64_t>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<uint64_t>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<uint64_t>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<uint64_t>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<uint64_t>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint64_t>(anyinterfaceid), 1234u);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<uint64_t>(anyexceptionid), 1234u);
}

TEST_F(CAnyTypeTest, CastOperatorChar)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<char>(anyint8), 'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint8), 'B');

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<char>(anyint16), 'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint16), 'B');

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<char>(anyint32), 'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint32), 'B');

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<char>(anyint64), 'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint64), 'B');

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<char>(anychar), 'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<char>(anyu16char), 'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<char>(anyu32char), 'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<char>(anywchar), 'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<char>(anyfloat), '\n');

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<char>(anydouble), '\n');

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<char>(anylongdouble), '\n');

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<char>(anyfixed), '?');

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<char>(anystring), '\f');

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<char>(anyu8string), '\f');

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<char>(anyu16string), '\f');

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<char>(anyu32string), '\f');

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<char>(anywstring), '\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<char>(anyinterface), '\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<char>(anyinterfaceid), static_cast<char>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<char>(anyexceptionid), static_cast<char>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorU8Char)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<char>(anyint8), 'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint8), 'B');

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<char>(anyint16), 'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint16), 'B');

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<char>(anyint32), 'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint32), 'B');

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<char>(anyint64), 'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<char>(anyuint64), 'B');

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<char>(anychar), 'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<char>(anyu16char), 'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<char>(anyu32char), 'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<char>(anywchar), 'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<char>(anyfloat), '\n');

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<char>(anydouble), '\n');

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<char>(anylongdouble), '\n');

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<char>(anyfixed), '?');

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<char>(anystring), '\f');

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<char>(anyu8string), '\f');

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<char>(anyu16string), '\f');

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<char>(anyu32string), '\f');

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<char>(anywstring), '\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<char>(anyinterface), '\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<char>(anyinterfaceid), static_cast<char>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<char>(anyexceptionid), static_cast<char>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorU16Char)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyint8), u'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyuint8), u'B');

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyint16), u'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyuint16), u'B');

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyint32), u'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyuint32), u'B');

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyint64), u'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<char16_t>(anyuint64), u'B');

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<char16_t>(anychar), u'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<char16_t>(anyu16char), u'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<char16_t>(anyu32char), u'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<char16_t>(anywchar), u'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<char16_t>(anyfloat), u'\n');

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<char16_t>(anydouble), u'\n');

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<char16_t>(anylongdouble), u'\n');

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<char16_t>(anyfixed), u'?');

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<char16_t>(anystring), u'\f');

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<char16_t>(anyu8string), u'\f');

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<char16_t>(anyu16string), u'\f');

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<char16_t>(anyu32string), u'\f');

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<char16_t>(anywstring), u'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<char16_t>(anyinterface), u'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<char16_t>(anyinterfaceid), static_cast<char16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<char16_t>(anyexceptionid), static_cast<char16_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorU32Char)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyint8), U'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyuint8), U'B');

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyint16), U'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyuint16), U'B');

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyint32), U'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyuint32), U'B');

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyint64), U'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<char32_t>(anyuint64), U'B');

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<char32_t>(anychar), U'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<char32_t>(anyu16char), U'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<char32_t>(anyu32char), U'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<char32_t>(anywchar), U'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<char32_t>(anyfloat), U'\n');

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<char32_t>(anydouble), U'\n');

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<char32_t>(anylongdouble), U'\n');

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<char32_t>(anyfixed), U'?');

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<char32_t>(anystring), U'\f');

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<char32_t>(anyu8string), U'\f');

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<char32_t>(anyu16string), U'\f');

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<char32_t>(anyu32string), U'\f');

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<char32_t>(anywstring), U'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<char32_t>(anyinterface), U'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<char32_t>(anyinterfaceid), static_cast<char32_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<char32_t>(anyexceptionid), static_cast<char32_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorWChar)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyint8), L'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyuint8), L'B');

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyint16), L'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyuint16), L'B');

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyint32), L'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyuint32), L'B');

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyint64), L'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<wchar_t>(anyuint64), L'B');

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<wchar_t>(anychar), L'A');

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<wchar_t>(anyu16char), L'A');

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<wchar_t>(anyu32char), L'A');

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<wchar_t>(anywchar), L'A');

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<wchar_t>(anyfloat), L'\n');

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<wchar_t>(anydouble), L'\n');

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<wchar_t>(anylongdouble), L'\n');

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<wchar_t>(anyfixed), L'?');

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<wchar_t>(anystring), L'\f');

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<wchar_t>(anyu8string), L'\f');

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<wchar_t>(anyu16string), L'\f');

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<wchar_t>(anyu32string), L'\f');

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<wchar_t>(anywstring), L'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<wchar_t>(anyinterface), L'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<wchar_t>(anyinterfaceid), static_cast<wchar_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<wchar_t>(anyexceptionid), static_cast<wchar_t>(1234));
}

TEST_F(CAnyTypeTest, CastOperatorFloat)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<float>(anyint8), 66.0f);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<float>(anyuint8), 66.0f);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<float>(anyint16), 66.0f);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<float>(anyuint16), 66.0f);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<float>(anyint32), 66.0f);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<float>(anyuint32), 66.0f);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<float>(anyint64), 66.0f);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<float>(anyuint64), 66.0f);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<float>(anychar), 65.0f);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<float>(anyu16char), 65.0f);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<float>(anyu32char), 65.0f);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<float>(anywchar), 65.0f);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<float>(anyfloat), 10.1234f);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<float>(anydouble), 10.1234f);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<float>(anylongdouble), 10.1234f);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<float>(anyfixed), ???.1234f);

    sdv::any_t anystring("12.34");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<float>(anystring), 12.34f);

    sdv::any_t anyu8string("12.34");
    EXPECT_EQ(static_cast<float>(anyu8string), 12.34f);

    sdv::any_t anyu16string(u"12.34");
    EXPECT_EQ(static_cast<float>(anyu16string), 12.34f);

    sdv::any_t anyu32string(U"12.34");
    EXPECT_EQ(static_cast<float>(anyu32string), 12.34f);

    sdv::any_t anywstring(L"12.34");
    EXPECT_EQ(static_cast<float>(anywstring), 12.34f);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<float>(anyinterface), 0.0f);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<float>(anyinterfaceid), 1234.0f);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<float>(anyexceptionid), 1234.0f);
}

TEST_F(CAnyTypeTest, CastOperatorDouble)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<double>(anyint8), 66.0);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<double>(anyuint8), 66.0);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<double>(anyint16), 66.0);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<double>(anyuint16), 66.0);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<double>(anyint32), 66.0);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<double>(anyuint32), 66.0);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<double>(anyint64), 66.0);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<double>(anyuint64), 66.0);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<double>(anychar), 65.0);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<double>(anyu16char), 65.0);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<double>(anyu32char), 65.0);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<double>(anywchar), 65.0);

    sdv::any_t anyfloat(10.1234);
    EXPECT_EQ(static_cast<double>(anyfloat), 10.1234);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<double>(anydouble), 10.1234);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<double>(anylongdouble), 10.1234);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<double>(anyfixed), ???.1234);

    sdv::any_t anystring("12.34");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<double>(anystring), 12.34);

    sdv::any_t anyu8string("12.34");
    EXPECT_EQ(static_cast<double>(anyu8string), 12.34);

    sdv::any_t anyu16string(u"12.34");
    EXPECT_EQ(static_cast<double>(anyu16string), 12.34);

    sdv::any_t anyu32string(U"12.34");
    EXPECT_EQ(static_cast<double>(anyu32string), 12.34);

    sdv::any_t anywstring(L"12.34");
    EXPECT_EQ(static_cast<double>(anywstring), 12.34);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<double>(anyinterface), 0.0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<double>(anyinterfaceid), 1234.0);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<double>(anyexceptionid), 1234.0);
}

TEST_F(CAnyTypeTest, CastOperatorLongDouble)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<long double>(anyint8), 66.0l);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<long double>(anyuint8), 66.0l);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<long double>(anyint16), 66.0l);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<long double>(anyuint16), 66.0l);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<long double>(anyint32), 66.0l);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<long double>(anyuint32), 66.0l);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<long double>(anyint64), 66.0l);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<long double>(anyuint64), 66.0l);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<long double>(anychar), 65.0l);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<long double>(anyu16char), 65.0l);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<long double>(anyu32char), 65.0l);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<long double>(anywchar), 65.0l);

    sdv::any_t anyfloat(10.1234l);
    EXPECT_EQ(static_cast<long double>(anyfloat), 10.1234l);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<long double>(anydouble), static_cast<long double>(10.1234));

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<long double>(anylongdouble), 10.1234l);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<long double>(anyfixed), ???.1234l);

    sdv::any_t anystring("12.34");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<long double>(anystring), 12.34l);

    sdv::any_t anyu8string("12.34");
    EXPECT_EQ(static_cast<long double>(anyu8string), 12.34l);

    sdv::any_t anyu16string(u"12.34");
    EXPECT_EQ(static_cast<long double>(anyu16string), 12.34l);

    sdv::any_t anyu32string(U"12.34");
    EXPECT_EQ(static_cast<long double>(anyu32string), 12.34l);

    sdv::any_t anywstring(L"12.34");
    EXPECT_EQ(static_cast<long double>(anywstring), 12.34l);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<long double>(anyinterface), 0.0l);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<long double>(anyinterfaceid), 1234.0l);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<long double>(anyexceptionid), 1234.0l);
}

//TEST_F(CAnyTypeTest, CastOperatorFixed)
//{
//    sdv::any_t anyint8(static_cast<uint8_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyint8), 66.0l);
//
//    sdv::any_t anyuint8(static_cast<uint8_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyuint8), 66.0l);
//
//    sdv::any_t anyint16(static_cast<int16_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyint16), 66.0l);
//
//    sdv::any_t anyuint16(static_cast<uint16_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyuint16), 66.0l);
//
//    sdv::any_t anyint32(static_cast<int32_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyint32), 66.0l);
//
//    sdv::any_t anyuint32(static_cast<uint32_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyuint32), 66.0l);
//
//    sdv::any_t anyint64(static_cast<int64_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyint64), 66.0l);
//
//    sdv::any_t anyuint64(static_cast<uint64_t>(66));
//    EXPECT_EQ(static_cast<sdv::fixed>(anyuint64), 66.0l);
//
//    sdv::any_t anychar('A');
//    EXPECT_EQ(static_cast<sdv::fixed>(anychar), 65.0l);
//
//    sdv::any_t anyu16char(u'A');
//    EXPECT_EQ(static_cast<sdv::fixed>(anyu16char), 65.0l);
//
//    sdv::any_t anyu32char(U'A');
//    EXPECT_EQ(static_cast<sdv::fixed>(anyu32char), 65.0l);
//
//    sdv::any_t anywchar(L'A');
//    EXPECT_EQ(static_cast<sdv::fixed>(anywchar), 65.0l);
//
//    sdv::any_t anyfloat(10.1234l);
//    EXPECT_EQ(static_cast<sdv::fixed>(anyfloat), 10.1234l);
//
//    sdv::any_t anydouble(10.1234);
//    EXPECT_EQ(static_cast<sdv::fixed>(anydouble), 10.1234l);
//
//    sdv::any_t anylongdouble(10.1234l);
//    EXPECT_EQ(static_cast<sdv::fixed>(anylongdouble), 10.1234l);
//
//    //sdv::any_t anyfixed(???);
//    //EXPECT_EQ(static_cast<sdv::fixed>(anyfixed), ???.1234l);
//
//    sdv::any_t anystring("12.34");
//    // ANSI string is detected as UTF-8 string
//    EXPECT_EQ(static_cast<sdv::fixed>(anystring), 12.34l);
//
//    sdv::any_t anyu8string("12.34");
//    EXPECT_EQ(static_cast<sdv::fixed>(anyu8string), 12.34l);
//
//    sdv::any_t anyu16string(u"12.34");
//    EXPECT_EQ(static_cast<sdv::fixed>(anyu16string), 12.34l);
//
//    sdv::any_t anyu32string(U"12.34");
//    EXPECT_EQ(static_cast<sdv::fixed>(anyu32string), 12.34l);
//
//    sdv::any_t anywstring(L"12.34");
//    EXPECT_EQ(static_cast<sdv::fixed>(anywstring), 12.34l);
//
//    struct STest : public sdv::IInterfaceAccess
//    {
//        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
//    } sTest;
//    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
//    sdv::any_t anyinterface(ifc);
//    EXPECT_EQ(static_cast<sdv::fixed>(anyinterface), 0.0l);
//
//    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
//    // interface_id is detected as uin64_t
//    EXPECT_EQ(static_cast<sdv::fixed>(anyinterfaceid), 1234.0l);
//
//    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
//    // exception_id is detected as uin64_t
//    EXPECT_EQ(static_cast<sdv::fixed>(anyexceptionid), 1234.0l);
//}

TEST_F(CAnyTypeTest, CastOperatorString)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyint8), "66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyuint8), "66");

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyint16), "66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyuint16), "66");

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyint32), "66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyuint32), "66");

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyint64), "66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::string>(anyuint64), "66");

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::string>(anychar), "A");

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::string>(anyu16char), "A");

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::string>(anyu32char), "A");

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::string>(anywchar), "A");

    // Ignore trailing zeros
    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::string>(anyfloat).substr(0, 7), "10.1234");

    // Ignore trailing zeros
    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::string>(anydouble).substr(0, 7), "10.1234");

    // Ignore trailing zeros
    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::string>(anylongdouble).substr(0, 7), "10.1234");

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::string>(anyfixed), "???");

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::string>(anystring), "hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(static_cast<sdv::string>(anyu8string), "hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(static_cast<sdv::string>(anyu16string), "hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(static_cast<sdv::string>(anyu32string), "hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(static_cast<sdv::string>(anywstring), "hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::string>(anyinterface).empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::string>(anyinterfaceid), "1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::string>(anyexceptionid), "1234");
}

TEST_F(CAnyTypeTest, CastOperatorU8String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyint8), "66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyuint8), "66");

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyint16), "66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyuint16), "66");

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyint32), "66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyuint32), "66");

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyint64), "66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::u8string>(anyuint64), "66");

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::u8string>(anychar), "A");

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::u8string>(anyu16char), "A");

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::u8string>(anyu32char), "A");

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::u8string>(anywchar), "A");

    // Ignore trailing zeros
    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::u8string>(anyfloat).substr(0, 7), "10.1234");

    // Ignore trailing zeros
    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::u8string>(anydouble).substr(0, 7), "10.1234");

    // Ignore trailing zeros
    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::u8string>(anylongdouble).substr(0, 7), "10.1234");

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::u8string>(anyfixed), "???");

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::u8string>(anystring), "hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(static_cast<sdv::u8string>(anyu8string), "hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(static_cast<sdv::u8string>(anyu16string), "hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(static_cast<sdv::u8string>(anyu32string), "hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(static_cast<sdv::u8string>(anywstring), "hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::u8string>(anyinterface).empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u8string>(anyinterfaceid), "1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u8string>(anyexceptionid), "1234");
}

TEST_F(CAnyTypeTest, CastOperatorU16String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyint8), u"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyuint8), u"66");

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyint16), u"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyuint16), u"66");

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyint32), u"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyuint32), u"66");

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyint64), u"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::u16string>(anyuint64), u"66");

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::u16string>(anychar), u"A");

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::u16string>(anyu16char), u"A");

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::u16string>(anyu32char), u"A");

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::u16string>(anywchar), u"A");

    // Ignore trailing zeros
    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::u16string>(anyfloat).substr(0, 7), u"10.1234");

    // Ignore trailing zeros
    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::u16string>(anydouble).substr(0, 7), u"10.1234");

    // Ignore trailing zeros
    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::u16string>(anylongdouble).substr(0, 7), u"10.1234");

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::u16string>(anyfixed), u"???");

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::u16string>(anystring), u"hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(static_cast<sdv::u16string>(anyu8string), u"hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(static_cast<sdv::u16string>(anyu16string), u"hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(static_cast<sdv::u16string>(anyu32string), u"hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(static_cast<sdv::u16string>(anywstring), u"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::u16string>(anyinterface).empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u16string>(anyinterfaceid), u"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u16string>(anyexceptionid), u"1234");
}

TEST_F(CAnyTypeTest, CastOperatorU32String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyint8), U"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyuint8), U"66");

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyint16), U"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyuint16), U"66");

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyint32), U"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyuint32), U"66");

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyint64), U"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::u32string>(anyuint64), U"66");

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::u32string>(anychar), U"A");

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::u32string>(anyu16char), U"A");

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::u32string>(anyu32char), U"A");

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::u32string>(anywchar), U"A");

    // Ignore trailing zeros
    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::u32string>(anyfloat).substr(0, 7), U"10.1234");

    // Ignore trailing zeros
    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::u32string>(anydouble).substr(0, 7), U"10.1234");

    // Ignore trailing zeros
    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::u32string>(anylongdouble).substr(0, 7), U"10.1234");

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::u32string>(anyfixed), U"???");

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::u32string>(anystring), U"hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(static_cast<sdv::u32string>(anyu8string), U"hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(static_cast<sdv::u32string>(anyu16string), U"hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(static_cast<sdv::u32string>(anyu32string), U"hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(static_cast<sdv::u32string>(anywstring), U"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::u32string>(anyinterface).empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u32string>(anyinterfaceid), U"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::u32string>(anyexceptionid), U"1234");
}

TEST_F(CAnyTypeTest, CastOperatorWString)
{
    sdv::any_t anyint8(static_cast<int8_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyint8), L"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyuint8), L"66");

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyint16), L"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyuint16), L"66");

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyint32), L"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyuint32), L"66");

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyint64), L"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::wstring>(anyuint64), L"66");

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::wstring>(anychar), L"A");

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::wstring>(anyu16char), L"A");

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::wstring>(anyu32char), L"A");

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::wstring>(anywchar), L"A");

    // Ignore trailing zeros
    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::wstring>(anyfloat).substr(0, 7), L"10.1234");

    // Ignore trailing zeros
    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::wstring>(anydouble).substr(0, 7), L"10.1234");

    // Ignore trailing zeros
    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::wstring>(anylongdouble).substr(0, 7), L"10.1234");

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::wstring>(anyfixed), L"???");

    sdv::any_t anystring("hello");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::wstring>(anystring), L"hello");

    sdv::any_t anyu8string("hello");
    EXPECT_EQ(static_cast<sdv::wstring>(anyu8string), L"hello");

    sdv::any_t anyu16string(u"hello");
    EXPECT_EQ(static_cast<sdv::wstring>(anyu16string), L"hello");

    sdv::any_t anyu32string(U"hello");
    EXPECT_EQ(static_cast<sdv::wstring>(anyu32string), L"hello");

    sdv::any_t anywstring(L"hello");
    EXPECT_EQ(static_cast<sdv::wstring>(anywstring), L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::wstring>(anyinterface).empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::wstring>(anyinterfaceid), L"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::wstring>(anyexceptionid), L"1234");
}

TEST_F(CAnyTypeTest, CastOperatorInterface)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyint8));

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyuint8));

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyint16));

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyuint16));

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyint32));

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyuint32));

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyint64));

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyuint64));

    sdv::any_t anychar('A');
    EXPECT_FALSE(static_cast<sdv::interface_t>(anychar));

    sdv::any_t anyu16char(u'A');
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyu16char));

    sdv::any_t anyu32char(U'A');
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyu32char));

    sdv::any_t anywchar(L'A');
    EXPECT_FALSE(static_cast<sdv::interface_t>(anywchar));

    sdv::any_t anyfloat(10.1234);
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyfloat));

    sdv::any_t anydouble(10.1234);
    EXPECT_FALSE(static_cast<sdv::interface_t>(anydouble));

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_FALSE(static_cast<sdv::interface_t>(anylongdouble));

    //sdv::any_t anyfixed(???);
    //EXPECT_FALSE(static_cast<sdv::interface_t>(anyfixed));

    sdv::any_t anystring("12.34");
    // ANSI string is detected as UTF-8 string
    EXPECT_FALSE(static_cast<sdv::interface_t>(anystring));

    sdv::any_t anyu8string("12.34");
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyu8string));

    sdv::any_t anyu16string(u"12.34");
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyu16string));

    sdv::any_t anyu32string(U"12.34");
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyu32string));

    sdv::any_t anywstring(L"12.34");
    EXPECT_FALSE(static_cast<sdv::interface_t>(anywstring));

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_TRUE(static_cast<sdv::interface_t>(anyinterface));

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyinterfaceid));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_FALSE(static_cast<sdv::interface_t>(anyexceptionid));
}

TEST_F(CAnyTypeTest, CastOperatorInterfaceID)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::interface_id>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::interface_id>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::interface_id>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::interface_id>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::interface_id>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::interface_id>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::interface_id>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::interface_id>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::interface_id>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::interface_id>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<sdv::interface_id>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<sdv::interface_id>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<sdv::interface_id>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<sdv::interface_id>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<sdv::interface_id>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::interface_id>(anyinterfaceid), 1234u);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::interface_id>(anyexceptionid), 1234u);
}

TEST_F(CAnyTypeTest, CastOperatorExceptionID)
{
    sdv::any_t anyint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyint8), 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyuint8), 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyint16), 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyuint16), 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyint32), 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyuint32), 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyint64), 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66));
    EXPECT_EQ(static_cast<sdv::exception_id>(anyuint64), 66u);

    sdv::any_t anychar('A');
    EXPECT_EQ(static_cast<sdv::exception_id>(anychar), 65u);

    sdv::any_t anyu16char(u'A');
    EXPECT_EQ(static_cast<sdv::exception_id>(anyu16char), 65u);

    sdv::any_t anyu32char(U'A');
    EXPECT_EQ(static_cast<sdv::exception_id>(anyu32char), 65u);

    sdv::any_t anywchar(L'A');
    EXPECT_EQ(static_cast<sdv::exception_id>(anywchar), 65u);

    sdv::any_t anyfloat(10.1234f);
    EXPECT_EQ(static_cast<sdv::exception_id>(anyfloat), 10u);

    sdv::any_t anydouble(10.1234);
    EXPECT_EQ(static_cast<sdv::exception_id>(anydouble), 10u);

    sdv::any_t anylongdouble(10.1234l);
    EXPECT_EQ(static_cast<sdv::exception_id>(anylongdouble), 10u);

    //sdv::any_t anyfixed(???);
    //EXPECT_EQ(static_cast<sdv::exception_id>(anyfixed), ???u);

    sdv::any_t anystring("12");
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(static_cast<sdv::exception_id>(anystring), 12u);

    sdv::any_t anyu8string("12");
    EXPECT_EQ(static_cast<sdv::exception_id>(anyu8string), 12u);

    sdv::any_t anyu16string(u"12");
    EXPECT_EQ(static_cast<sdv::exception_id>(anyu16string), 12u);

    sdv::any_t anyu32string(U"12");
    EXPECT_EQ(static_cast<sdv::exception_id>(anyu32string), 12u);

    sdv::any_t anywstring(L"12");
    EXPECT_EQ(static_cast<sdv::exception_id>(anywstring), 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc);
    EXPECT_EQ(static_cast<sdv::exception_id>(anyinterface), 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::exception_id>(1234u));
    // interface_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::interface_id>(anyinterfaceid), 1234u);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u));
    // exception_id is detected as uin64_t
    EXPECT_EQ(static_cast<sdv::exception_id>(anyexceptionid), 1234u);
}

TEST_F(CAnyTypeTest, SetInt8)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint8.i8Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint8.i8Val, 66);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint16.i8Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint16.i8Val, 66);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint32.i8Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint32.i8Val, 66);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyint64.i8Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyuint64.i8Val, 66);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anychar.i8Val, 65);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu16char.i8Val, 65);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu32char.i8Val, 65);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anywchar.i8Val, 65);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyfloat.i8Val, 10);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anydouble.i8Val, 10);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anylongdouble.i8Val, 10);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_int8);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_int8);
    //EXPECT_EQ(anyfixedi8Val, 10);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_int8);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anystring.i8Val, 12);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu8string.i8Val, 12);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu16string.i8Val, 12);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyu32string.i8Val, 12);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anywstring.i8Val, 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyinterface.i8Val, 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_int8);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyinterfaceid.i8Val, static_cast<int8_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_int8);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_int8);
    EXPECT_EQ(anyexceptionid.i8Val, static_cast<int8_t>(1234));
}

TEST_F(CAnyTypeTest, SetUInt8)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint8.ui8Val, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint8.ui8Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint16.ui8Val, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint16.ui8Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint32.ui8Val, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint32.ui8Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyint64.ui8Val, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyuint64.ui8Val, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anychar.ui8Val, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu16char.ui8Val, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu32char.ui8Val, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anywchar.ui8Val, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyfloat.ui8Val, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anydouble.ui8Val, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anylongdouble.ui8Val, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_uint8);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_uint8);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_uint8);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anystring.ui8Val, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu8string.ui8Val, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu16string.ui8Val, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyu32string.ui8Val, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anywstring.ui8Val, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyinterface.ui8Val, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_uint8);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyinterfaceid.ui8Val, static_cast<uint8_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_uint8);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint8);
    EXPECT_EQ(anyexceptionid.ui8Val, static_cast<uint8_t>(1234));
}

TEST_F(CAnyTypeTest, SetInt16)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint8.i16Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint8.i16Val, 66);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint16.i16Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint16.i16Val, 66);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint32.i16Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint32.i16Val, 66);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyint64.i16Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyuint64.i16Val, 66);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anychar.i16Val, 65);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu16char.i16Val, 65);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu32char.i16Val, 65);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anywchar.i16Val, 65);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyfloat.i16Val, 10);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anydouble.i16Val, 10);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anylongdouble.i16Val, 10);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_int16);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_int16);
    //EXPECT_EQ(anyfixedi8Val, 10);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_int16);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anystring.i16Val, 12);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu8string.i16Val, 12);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu16string.i16Val, 12);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyu32string.i16Val, 12);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anywstring.i16Val, 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyinterface.i16Val, 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_int16);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyinterfaceid.i16Val, static_cast<int16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_int16);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_int16);
    EXPECT_EQ(anyexceptionid.i16Val, static_cast<int16_t>(1234));
}

TEST_F(CAnyTypeTest, SetUInt16)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint8.ui16Val, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint8.ui16Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint16.ui16Val, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint16.ui16Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint32.ui16Val, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint32.ui16Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyint64.ui16Val, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyuint64.ui16Val, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anychar.ui16Val, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu16char.ui16Val, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu32char.ui16Val, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anywchar.ui16Val, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyfloat.ui16Val, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anydouble.ui16Val, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anylongdouble.ui16Val, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_uint16);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_uint16);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_uint16);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anystring.ui16Val, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu8string.ui16Val, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu16string.ui16Val, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyu32string.ui16Val, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anywstring.ui16Val, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyinterface.ui16Val, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_uint16);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyinterfaceid.ui16Val, static_cast<uint16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_uint16);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint16);
    EXPECT_EQ(anyexceptionid.ui16Val, static_cast<uint16_t>(1234));
}

TEST_F(CAnyTypeTest, SetInt32)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint8.i32Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint8.i32Val, 66);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint16.i32Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint16.i32Val, 66);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint32.i32Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint32.i32Val, 66);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyint64.i32Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyuint64.i32Val, 66);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anychar.i32Val, 65);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu16char.i32Val, 65);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu32char.i32Val, 65);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anywchar.i32Val, 65);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyfloat.i32Val, 10);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anydouble.i32Val, 10);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anylongdouble.i32Val, 10);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_int32);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_int32);
    //EXPECT_EQ(anyfixedi8Val, 10);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_int32);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anystring.i32Val, 12);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu8string.i32Val, 12);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu16string.i32Val, 12);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyu32string.i32Val, 12);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anywstring.i32Val, 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyinterface.i32Val, 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_int32);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyinterfaceid.i32Val, static_cast<int32_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_int32);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_int32);
    EXPECT_EQ(anyexceptionid.i32Val, static_cast<int32_t>(1234));
}

TEST_F(CAnyTypeTest, SetUInt32)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint8.ui32Val, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint8.ui32Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint16.ui32Val, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint16.ui32Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint32.ui32Val, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint32.ui32Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyint64.ui32Val, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyuint64.ui32Val, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anychar.ui32Val, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu16char.ui32Val, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu32char.ui32Val, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anywchar.ui32Val, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyfloat.ui32Val, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anydouble.ui32Val, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anylongdouble.ui32Val, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_uint32);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_uint32);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_uint32);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anystring.ui32Val, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu8string.ui32Val, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu16string.ui32Val, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyu32string.ui32Val, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anywstring.ui32Val, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyinterface.ui32Val, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_uint32);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyinterfaceid.ui32Val, static_cast<uint32_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_uint32);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint32);
    EXPECT_EQ(anyexceptionid.ui32Val, static_cast<uint32_t>(1234));
}

TEST_F(CAnyTypeTest, SetInt64)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint8.i64Val, 66);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint8.i64Val, 66);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint16.i64Val, 66);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint16.i64Val, 66);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint32.i64Val, 66);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint32.i64Val, 66);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyint64.i64Val, 66);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyuint64.i64Val, 66);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anychar.i64Val, 65);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu16char.i64Val, 65);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu32char.i64Val, 65);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anywchar.i64Val, 65);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyfloat.i64Val, 10);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anydouble.i64Val, 10);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anylongdouble.i64Val, 10);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_int64);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_int64);
    //EXPECT_EQ(anyfixedi8Val, 10);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_int64);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anystring.i64Val, 12);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu8string.i64Val, 12);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu16string.i64Val, 12);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyu32string.i64Val, 12);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anywstring.i64Val, 12);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyinterface.i64Val, 0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_int64);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyinterfaceid.i64Val, static_cast<int64_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_int64);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_int64);
    EXPECT_EQ(anyexceptionid.i64Val, static_cast<int64_t>(1234));
}

TEST_F(CAnyTypeTest, SetUInt64)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint8.ui64Val, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint8.ui64Val, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint16.ui64Val, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint16.ui64Val, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint32.ui64Val, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint32.ui64Val, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyint64.ui64Val, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyuint64.ui64Val, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anychar.ui64Val, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu16char.ui64Val, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu32char.ui64Val, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anywchar.ui64Val, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyfloat.ui64Val, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anydouble.ui64Val, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anylongdouble.ui64Val, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_uint64);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_uint64);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_uint64);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anystring.ui64Val, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu8string.ui64Val, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu16string.ui64Val, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyu32string.ui64Val, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anywstring.ui64Val, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterface.ui64Val, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_uint64);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyinterfaceid.ui64Val, static_cast<uint64_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_uint64);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_uint64);
    EXPECT_EQ(anyexceptionid.ui64Val, static_cast<uint64_t>(1234));
}

TEST_F(CAnyTypeTest, SetChar)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint8.cVal, 'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint8.cVal, 'B');

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint16.cVal, 'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint16.cVal, 'B');

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint32.cVal, 'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint32.cVal, 'B');

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyint64.cVal, 'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyuint64.cVal, 'B');

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anychar.cVal, 'A');

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu16char.cVal, 'A');

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu32char.cVal, 'A');

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anywchar.cVal, 'A');

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyfloat.cVal, '\n');

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anydouble.cVal, '\n');

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anylongdouble.cVal, '\n');

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_char);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_char);
    //EXPECT_EQ(anyfixedi8Val, '\n');

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_char);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anystring.cVal, '\f');

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu8string.cVal, '\f');

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu16string.cVal, '\f');

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyu32string.cVal, '\f');

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anywstring.cVal, '\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyinterface.cVal, '\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_char);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyinterfaceid.cVal, static_cast<char>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_char);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_char);
    EXPECT_EQ(anyexceptionid.cVal, static_cast<char>(1234));
}

TEST_F(CAnyTypeTest, SetChar16)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint8.c16Val, u'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint8.c16Val, u'B');

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint16.c16Val, u'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint16.c16Val, u'B');

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint32.c16Val, u'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint32.c16Val, u'B');

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyint64.c16Val, u'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyuint64.c16Val, u'B');

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anychar.c16Val, u'A');

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16char.c16Val, u'A');

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu32char.c16Val, u'A');

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anywchar.c16Val, u'A');

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyfloat.c16Val, u'\n');

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anydouble.c16Val, u'\n');

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anylongdouble.c16Val, u'\n');

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_char16);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_char16);
    //EXPECT_EQ(anyfixedi8Val, '\n');

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_char16);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anystring.c16Val, u'\f');

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu8string.c16Val, u'\f');

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu16string.c16Val, u'\f');

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyu32string.c16Val, u'\f');

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anywstring.c16Val, u'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyinterface.c16Val, u'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_char16);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyinterfaceid.c16Val, static_cast<char16_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_char16);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_char16);
    EXPECT_EQ(anyexceptionid.c16Val, static_cast<char16_t>(1234));
}

TEST_F(CAnyTypeTest, SetChar32)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint8.c32Val, U'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint8.c32Val, U'B');

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint16.c32Val, U'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint16.c32Val, U'B');

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint32.c32Val, U'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint32.c32Val, U'B');

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyint64.c32Val, U'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyuint64.c32Val, U'B');

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anychar.c32Val, U'A');

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu16char.c32Val, U'A');

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32char.c32Val, U'A');

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anywchar.c32Val, U'A');

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyfloat.c32Val, U'\n');

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anydouble.c32Val, U'\n');

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anylongdouble.c32Val, U'\n');

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_char32);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_char32);
    //EXPECT_EQ(anyfixedi8Val, '\n');

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_char32);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anystring.c32Val, U'\f');

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu8string.c32Val, U'\f');

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu16string.c32Val, U'\f');

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyu32string.c32Val, U'\f');

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anywstring.c32Val, U'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyinterface.c32Val, U'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_char32);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyinterfaceid.c32Val, static_cast<char32_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_char32);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_char32);
    EXPECT_EQ(anyexceptionid.c32Val, static_cast<char32_t>(1234));
}

TEST_F(CAnyTypeTest, SetWChar)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint8.cwVal, L'B');

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint8.cwVal, L'B');

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint16.cwVal, L'B');

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint16.cwVal, L'B');

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint32.cwVal, L'B');

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint32.cwVal, L'B');

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyint64.cwVal, L'B');

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyuint64.cwVal, L'B');

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anychar.cwVal, L'A');

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu16char.cwVal, L'A');

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu32char.cwVal, L'A');

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywchar.cwVal, L'A');

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyfloat.cwVal, L'\n');

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anydouble.cwVal, L'\n');

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anylongdouble.cwVal, L'\n');

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_wchar);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_wchar);
    //EXPECT_EQ(anyfixedi8Val, '\n');

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_wchar);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anystring.cwVal, L'\f');

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu8string.cwVal, L'\f');

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu16string.cwVal, L'\f');

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyu32string.cwVal, L'\f');

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anywstring.cwVal, L'\f');

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyinterface.cwVal, L'\0');

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_wchar);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyinterfaceid.cwVal, static_cast<wchar_t>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_wchar);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_wchar);
    EXPECT_EQ(anyexceptionid.cwVal, static_cast<wchar_t>(1234));
}

TEST_F(CAnyTypeTest, SetFloat)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint8.fVal, 66.0f);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint8.fVal, 66.0f);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint16.fVal, 66.0f);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint16.fVal, 66.0f);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint32.fVal, 66.0f);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint32.fVal, 66.0f);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyint64.fVal, 66.0f);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyuint64.fVal, 66.0f);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anychar.fVal, 65.0f);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu16char.fVal, 65.0f);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu32char.fVal, 65.0f);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anywchar.fVal, 65.0f);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyfloat.fVal, 10.1234f);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anydouble.fVal, 10.1234f);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anylongdouble.fVal, 10.1234f);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_float);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_float);
    //EXPECT_EQ(anyfixedi8Val, 10.1234f);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_float);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anystring.fVal, 12.0f);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu8string.fVal, 12.0f);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu16string.fVal, 12.0f);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyu32string.fVal, 12.0f);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anywstring.fVal, 12.0f);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyinterface.fVal, 0.0f);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_float);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyinterfaceid.fVal, static_cast<float>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_float);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_float);
    EXPECT_EQ(anyexceptionid.fVal, static_cast<float>(1234));
}

TEST_F(CAnyTypeTest, SetDouble)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint8.dVal, 66.0);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint8.dVal, 66.0);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint16.dVal, 66.0);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint16.dVal, 66.0);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint32.dVal, 66.0);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint32.dVal, 66.0);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyint64.dVal, 66.0);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyuint64.dVal, 66.0);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anychar.dVal, 65.0);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu16char.dVal, 65.0);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu32char.dVal, 65.0);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anywchar.dVal, 65.0);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyfloat.dVal, static_cast<double>(10.1234f));

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anydouble.dVal, 10.1234);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anylongdouble.dVal, 10.1234);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_double);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_double);
    //EXPECT_EQ(anyfixedi8Val, 10.1234);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_double);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anystring.dVal, 12.0);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu8string.dVal, 12.0);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu16string.dVal, 12.0);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyu32string.dVal, 12.0);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anywstring.dVal, 12.0);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyinterface.dVal, 0.0);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_double);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyinterfaceid.dVal, static_cast<double>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_double);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_double);
    EXPECT_EQ(anyexceptionid.dVal, static_cast<double>(1234));
}

TEST_F(CAnyTypeTest, SetLongDouble)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint8.ldVal, 66.0l);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint8.ldVal, 66.0l);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint16.ldVal, 66.0l);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint16.ldVal, 66.0l);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint32.ldVal, 66.0l);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint32.ldVal, 66.0l);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyint64.ldVal, 66.0l);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyuint64.ldVal, 66.0l);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anychar.ldVal, 65.0l);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu16char.ldVal, 65.0l);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu32char.ldVal, 65.0l);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anywchar.ldVal, 65.0l);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyfloat.ldVal, static_cast<long double>(10.1234f));

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anydouble.ldVal, static_cast<long double>(10.1234));

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anylongdouble.ldVal, 10.1234l);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_long_double);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_long_double);
    //EXPECT_EQ(anyfixedi8Val, 10.1234l);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_long_double);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anystring.ldVal, 12.0l);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu8string.ldVal, 12.0l);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu16string.ldVal, 12.0l);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyu32string.ldVal, 12.0l);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anywstring.ldVal, 12.0l);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyinterface.ldVal, 0.0l);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_long_double);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyinterfaceid.ldVal, static_cast<long double>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_long_double);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_long_double);
    EXPECT_EQ(anyexceptionid.ldVal, static_cast<long double>(1234));
}

//TEST_F(CAnyTypeTest, SetFixed)
//{
//    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint8.fixVal, 66.0);
//
//    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint8.fixVal, 66.0);
//
//    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint16.fixVal, 66.0);
//
//    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint16.fixVal, 66.0);
//
//    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint32.fixVal, 66.0);
//
//    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint32.fixVal, 66.0);
//
//    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyint64.fixVal, 66.0);
//
//    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyuint64.fixVal, 66.0);
//
//    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anychar.fixVal, 65.0);
//
//    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu16char.fixVal, 65.0);
//
//    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu32char.fixVal, 65.0);
//
//    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anywchar.fixVal, 65.0);
//
//    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyfloat.fixVal, static_cast<double>(10.1234f));
//
//    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anydouble.fixVal, 10.1234);
//
//    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anylongdouble.fixVal, 10.1234);
//
//    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_fixed);
//    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_fixed);
//    //EXPECT_EQ(anyfixedi8Val, 10.1234);
//
//    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_fixed);
//    // ANSI string is detected as UTF-8 string
//    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anystring.fixVal, 12.0);
//
//    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu8string.fixVal, 12.0);
//
//    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu16string.fixVal, 12.0);
//
//    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyu32string.fixVal, 12.0);
//
//    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anywstring.fixVal, 12.0);
//
//    struct STest : public sdv::IInterfaceAccess
//    {
//        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
//    } sTest;
//    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
//    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyinterface.fixVal, 0.0);
//
//    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_fixed);
//    // interface_id is detected as uin64_t
//    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyinterfaceid.fixVal, static_cast<sdv::fixed>(1234));
//
//    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_fixed);
//    // exception_id is detected as uin64_t
//    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_fixed);
//    EXPECT_EQ(anyexceptionid.fixVal, static_cast<sdv::fixed>(1234));
//}

TEST_F(CAnyTypeTest, SetString)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint8.ssVal, "66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint8.ssVal, "66");

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint16.ssVal, "66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint16.ssVal, "66");

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint32.ssVal, "66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint32.ssVal, "66");

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyint64.ssVal, "66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyuint64.ssVal, "66");

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anychar.ssVal, "A");

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu16char.ssVal, "A");

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu32char.ssVal, "A");

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anywchar.ssVal, "A");

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyfloat.ssVal.substr(0, 7), "10.1234");

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anydouble.ssVal.substr(0, 7), "10.1234");

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anylongdouble.ssVal.substr(0, 7), "10.1234");

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_string);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_string);
    //EXPECT_EQ(anyfixed.ssVal.substr(0, 7), 10.1234");

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_string);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anystring.ssVal, "hello");

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu8string.ssVal, "hello");

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu16string.ssVal, "hello");

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyu32string.ssVal, "hello");

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anywstring.ssVal, "hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_TRUE(anyinterface.ssVal.empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_string);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyinterfaceid.ssVal, "1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_string);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_string);
    EXPECT_EQ(anyexceptionid.ssVal, "1234");
}

TEST_F(CAnyTypeTest, SetU8String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint8.ss8Val, "66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint8.ss8Val, "66");

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint16.ss8Val, "66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint16.ss8Val, "66");

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint32.ss8Val, "66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint32.ss8Val, "66");

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyint64.ss8Val, "66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyuint64.ss8Val, "66");

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anychar.ss8Val, "A");

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu16char.ss8Val, "A");

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu32char.ss8Val, "A");

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anywchar.ss8Val, "A");

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyfloat.ss8Val.substr(0, 7), "10.1234");

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anydouble.ss8Val.substr(0, 7), "10.1234");

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anylongdouble.ss8Val.substr(0, 7), "10.1234");

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_u8string);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_u8string);
    //EXPECT_EQ(anyfixed.ss8Val.substr(0, 7), 10.1234");

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_u8string);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anystring.ss8Val, "hello");

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu8string.ss8Val, "hello");

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu16string.ss8Val, "hello");

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyu32string.ss8Val, "hello");

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anywstring.ss8Val, "hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_TRUE(anyinterface.ss8Val.empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_u8string);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyinterfaceid.ss8Val, "1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_u8string);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_u8string);
    EXPECT_EQ(anyexceptionid.ss8Val, "1234");
}

TEST_F(CAnyTypeTest, SetU16String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint8.ss16Val, u"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint8.ss16Val, u"66");

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint16.ss16Val, u"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint16.ss16Val, u"66");

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint32.ss16Val, u"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint32.ss16Val, u"66");

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyint64.ss16Val, u"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyuint64.ss16Val, u"66");

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anychar.ss16Val, u"A");

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16char.ss16Val, u"A");

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu32char.ss16Val, u"A");

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anywchar.ss16Val, u"A");

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyfloat.ss16Val.substr(0, 7), u"10.1234");

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anydouble.ss16Val.substr(0, 7), u"10.1234");

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anylongdouble.ss16Val.substr(0, 7), u"10.1234");

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_u16string);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_u16string);
    //EXPECT_EQ(anyfixed.ss16Val.substr(0, 7), 10.1234");

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_u16string);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anystring.ss16Val, u"hello");

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu8string.ss16Val, u"hello");

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu16string.ss16Val, u"hello");

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyu32string.ss16Val, u"hello");

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anywstring.ss16Val, u"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_TRUE(anyinterface.ss16Val.empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_u16string);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyinterfaceid.ss16Val, u"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_u16string);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_u16string);
    EXPECT_EQ(anyexceptionid.ss16Val, u"1234");
}

TEST_F(CAnyTypeTest, SetU32String)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint8.ss32Val, U"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint8.ss32Val, U"66");

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint16.ss32Val, U"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint16.ss32Val, U"66");

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint32.ss32Val, U"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint32.ss32Val, U"66");

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyint64.ss32Val, U"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyuint64.ss32Val, U"66");

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anychar.ss32Val, U"A");

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu16char.ss32Val, U"A");

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32char.ss32Val, U"A");

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anywchar.ss32Val, U"A");

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyfloat.ss32Val.substr(0, 7), U"10.1234");

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anydouble.ss32Val.substr(0, 7), U"10.1234");

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anylongdouble.ss32Val.substr(0, 7), U"10.1234");

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_u32string);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_u32string);
    //EXPECT_EQ(anyfixed.ss32Val.substr(0, 7), 10.1234");

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_u32string);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anystring.ss32Val, U"hello");

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu8string.ss32Val, U"hello");

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu16string.ss32Val, U"hello");

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyu32string.ss32Val, U"hello");

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anywstring.ss32Val, U"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_TRUE(anyinterface.ss32Val.empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_u32string);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyinterfaceid.ss32Val, U"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_u32string);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_u32string);
    EXPECT_EQ(anyexceptionid.ss32Val, U"1234");
}

TEST_F(CAnyTypeTest, SetWString)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint8.sswVal, L"66");

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint8.sswVal, L"66");

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint16.sswVal, L"66");

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint16.sswVal, L"66");

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint32.sswVal, L"66");

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint32.sswVal, L"66");

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyint64.sswVal, L"66");

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyuint64.sswVal, L"66");

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anychar.sswVal, L"A");

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu16char.sswVal, L"A");

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu32char.sswVal, L"A");

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywchar.sswVal, L"A");

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyfloat.sswVal.substr(0, 7), L"10.1234");

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anydouble.sswVal.substr(0, 7), L"10.1234");

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anylongdouble.sswVal.substr(0, 7), L"10.1234");

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_wstring);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_wstring);
    //EXPECT_EQ(anyfixed.sswVal.substr(0, 7), 10.1234");

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_wstring);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anystring.sswVal, L"hello");

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu8string.sswVal, L"hello");

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu16string.sswVal, L"hello");

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyu32string.sswVal, L"hello");

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anywstring.sswVal, L"hello");

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_TRUE(anyinterface.sswVal.empty());

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_wstring);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyinterfaceid.sswVal, L"1234");

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_wstring);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_wstring);
    EXPECT_EQ(anyexceptionid.sswVal, L"1234");
}

TEST_F(CAnyTypeTest, SetInterface)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyint8.ifcVal);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyuint8.ifcVal);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyint16.ifcVal);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyuint16.ifcVal);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyint32.ifcVal);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyuint32.ifcVal);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyint64.ifcVal);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyuint64.ifcVal);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anychar.ifcVal);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyu16char.ifcVal);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyu32char.ifcVal);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anywchar.ifcVal);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyfloat.ifcVal);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anydouble.ifcVal);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anylongdouble.ifcVal);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_interface);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_interface);
    //EXPECT_FALSE(anyfixed.ifcVal);

    sdv::any_t anystring("hello", sdv::any_t::EValType::val_type_interface);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anystring.ifcVal);

    sdv::any_t anyu8string("hello", sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyu8string.ifcVal);

    sdv::any_t anyu16string(u"hello", sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyu16string.ifcVal);

    sdv::any_t anyu32string(U"hello", sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyu32string.ifcVal);

    sdv::any_t anywstring(L"hello", sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anywstring.ifcVal);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_interface);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_TRUE(anyinterface.ifcVal);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_interface);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyinterfaceid.ifcVal);

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_interface);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_interface);
    EXPECT_FALSE(anyexceptionid.ifcVal);
}

TEST_F(CAnyTypeTest, SetInterfaceID)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint8.idIfcVal, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint8.idIfcVal, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint16.idIfcVal, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint16.idIfcVal, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint32.idIfcVal, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint32.idIfcVal, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyint64.idIfcVal, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyuint64.idIfcVal, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anychar.idIfcVal, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu16char.idIfcVal, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu32char.idIfcVal, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anywchar.idIfcVal, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyfloat.idIfcVal, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anydouble.idIfcVal, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anylongdouble.idIfcVal, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_interface_id);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_interface_id);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_interface_id);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anystring.idIfcVal, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu8string.idIfcVal, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu16string.idIfcVal, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyu32string.idIfcVal, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anywstring.idIfcVal, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterface.idIfcVal, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_interface_id);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyinterfaceid.idIfcVal, static_cast<sdv::interface_id>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_interface_id);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_interface_id);
    EXPECT_EQ(anyexceptionid.idIfcVal, static_cast<sdv::interface_id>(1234));
}

TEST_F(CAnyTypeTest, SetExceptionID)
{
    sdv::any_t anyint8(static_cast<int8_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint8.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint8.idExceptVal, 66u);

    sdv::any_t anyuint8(static_cast<uint8_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint8.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint8.idExceptVal, 66u);

    sdv::any_t anyint16(static_cast<int16_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint16.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint16.idExceptVal, 66u);

    sdv::any_t anyuint16(static_cast<uint16_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint16.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint16.idExceptVal, 66u);

    sdv::any_t anyint32(static_cast<int32_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint32.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint32.idExceptVal, 66u);

    sdv::any_t anyuint32(static_cast<uint32_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint32.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint32.idExceptVal, 66u);

    sdv::any_t anyint64(static_cast<int64_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint64.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyint64.idExceptVal, 66u);

    sdv::any_t anyuint64(static_cast<uint64_t>(66), sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint64.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyuint64.idExceptVal, 66u);

    sdv::any_t anychar('A', sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anychar.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anychar.idExceptVal, 65u);

    sdv::any_t anyu16char(u'A', sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu16char.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu16char.idExceptVal, 65u);

    sdv::any_t anyu32char(U'A', sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu32char.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu32char.idExceptVal, 65u);

    sdv::any_t anywchar(L'A', sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anywchar.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anywchar.idExceptVal, 65u);

    sdv::any_t anyfloat(10.1234f, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyfloat.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyfloat.idExceptVal, 10u);

    sdv::any_t anydouble(10.1234, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anydouble.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anydouble.idExceptVal, 10u);

    sdv::any_t anylongdouble(10.1234l, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anylongdouble.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anylongdouble.idExceptVal, 10u);

    //sdv::any_t anyfixed(???, sdv::any_t::EValType::val_type_exception_id);
    //EXPECT_EQ(anyfixedeValType, sdv::any_t::EValType::val_type_exception_id);
    //EXPECT_EQ(anyfixedi8Val, 10u);

    sdv::any_t anystring("12", sdv::any_t::EValType::val_type_exception_id);
    // ANSI string is detected as UTF-8 string
    EXPECT_EQ(anystring.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anystring.idExceptVal, 12u);

    sdv::any_t anyu8string("12", sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu8string.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu8string.idExceptVal, 12u);

    sdv::any_t anyu16string(u"12", sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu16string.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu16string.idExceptVal, 12u);

    sdv::any_t anyu32string(U"12", sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu32string.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyu32string.idExceptVal, 12u);

    sdv::any_t anywstring(L"12", sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anywstring.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anywstring.idExceptVal, 12u);

    struct STest : public sdv::IInterfaceAccess
    {
        virtual sdv::interface_t GetInterface(/*in*/ sdv::interface_id /*idInterface*/) { return nullptr; };
    } sTest;
    sdv::interface_t ifc = static_cast<sdv::IInterfaceAccess*>(&sTest);
    sdv::any_t anyinterface(ifc, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyinterface.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyinterface.idExceptVal, 0u);

    sdv::any_t anyinterfaceid(static_cast<sdv::interface_id>(1234u), sdv::any_t::EValType::val_type_exception_id);
    // interface_id is detected as uin64_t
    EXPECT_EQ(anyinterfaceid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyinterfaceid.idExceptVal, static_cast<sdv::exception_id>(1234));

    sdv::any_t anyexceptionid(static_cast<sdv::exception_id>(1234u), sdv::any_t::EValType::val_type_exception_id);
    // exception_id is detected as uin64_t
    EXPECT_EQ(anyexceptionid.eValType, sdv::any_t::EValType::val_type_exception_id);
    EXPECT_EQ(anyexceptionid.idExceptVal, static_cast<sdv::exception_id>(1234));
}

TEST_F(CAnyTypeTest, CompareEqualArithmetic)
{
    uint8_t ui8Val = 10;
    EXPECT_EQ(sdv::any_t(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), ui8Val);
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<float>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<double>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<char>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val));
    EXPECT_EQ(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val));
    EXPECT_EQ(ui8Val                       , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<uint16_t>(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<uint32_t>(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<uint64_t>(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<int8_t>(ui8Val)  , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<int16_t>(ui8Val) , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<int32_t>(ui8Val) , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<int64_t>(ui8Val) , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<float>(ui8Val)   , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<double>(ui8Val)  , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<char>(ui8Val)    , sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<char16_t>(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<char32_t>(ui8Val), sdv::any_t(ui8Val));
    EXPECT_EQ(static_cast<wchar_t>(ui8Val) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 20;
    EXPECT_EQ(sdv::any_t(ui16Val), sdv::any_t(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), ui16Val);
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<float>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<double>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<char>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val));
    EXPECT_EQ(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val));
    EXPECT_EQ(ui16Val                       , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<uint32_t>(ui16Val), sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<uint64_t>(ui16Val), sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<int8_t>(ui16Val)  , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<int16_t>(ui16Val) , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<int32_t>(ui16Val) , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<int64_t>(ui16Val) , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<float>(ui16Val)   , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<double>(ui16Val)  , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<char>(ui16Val)    , sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<char16_t>(ui16Val), sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<char32_t>(ui16Val), sdv::any_t(ui16Val));
    EXPECT_EQ(static_cast<wchar_t>(ui16Val) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 30;
    EXPECT_EQ(sdv::any_t(ui32Val), sdv::any_t(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), ui32Val);
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<float>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<double>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<char>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val));
    EXPECT_EQ(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val));
    EXPECT_EQ(ui32Val                       , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<uint64_t>(ui32Val), sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<int8_t>(ui32Val)  , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<int16_t>(ui32Val) , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<int32_t>(ui32Val) , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<int64_t>(ui32Val) , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<float>(ui32Val)   , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<double>(ui32Val)  , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<char>(ui32Val)    , sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<char16_t>(ui32Val), sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<char32_t>(ui32Val), sdv::any_t(ui32Val));
    EXPECT_EQ(static_cast<wchar_t>(ui32Val) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 40;
    EXPECT_EQ(sdv::any_t(ui64Val), sdv::any_t(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), ui64Val);
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<float>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<double>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<char>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val));
    EXPECT_EQ(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val));
    EXPECT_EQ(ui64Val                       , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<int8_t>(ui64Val)  , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<int16_t>(ui64Val) , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<int32_t>(ui64Val) , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<int64_t>(ui64Val) , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<float>(ui64Val)   , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<double>(ui64Val)  , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<char>(ui64Val)    , sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<char16_t>(ui64Val), sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<char32_t>(ui64Val), sdv::any_t(ui64Val));
    EXPECT_EQ(static_cast<wchar_t>(ui64Val) , sdv::any_t(ui64Val));

    int8_t i8Val = 50;
    EXPECT_EQ(sdv::any_t(i8Val), sdv::any_t(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), i8Val);
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<int16_t>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<int32_t>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<int64_t>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<float>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<double>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<char>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<char16_t>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<char32_t>(i8Val));
    EXPECT_EQ(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val));
    EXPECT_EQ(i8Val                       , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<int16_t>(i8Val) , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<int32_t>(i8Val) , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<int64_t>(i8Val) , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<float>(i8Val)   , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<double>(i8Val)  , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<char>(i8Val)    , sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<char16_t>(i8Val), sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<char32_t>(i8Val), sdv::any_t(i8Val));
    EXPECT_EQ(static_cast<wchar_t>(i8Val) , sdv::any_t(i8Val));

    int16_t i16Val = 60;
    EXPECT_EQ(sdv::any_t(i16Val), sdv::any_t(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), i16Val);
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<int32_t>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<int64_t>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<float>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<double>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<char>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<char16_t>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<char32_t>(i16Val));
    EXPECT_EQ(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val));
    EXPECT_EQ(i16Val                       , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<int32_t>(i16Val) , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<int64_t>(i16Val) , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<float>(i16Val)   , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<double>(i16Val)  , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<char>(i16Val)    , sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<char16_t>(i16Val), sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<char32_t>(i16Val), sdv::any_t(i16Val));
    EXPECT_EQ(static_cast<wchar_t>(i16Val) , sdv::any_t(i16Val));

    int32_t i32Val = 70;
    EXPECT_EQ(sdv::any_t(i32Val), sdv::any_t(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), i32Val);
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<int64_t>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<float>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<double>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<char>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<char16_t>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<char32_t>(i32Val));
    EXPECT_EQ(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val));
    EXPECT_EQ(i32Val                       , sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<int64_t>(i32Val) , sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<float>(i32Val)   , sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<double>(i32Val)  , sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<char>(i32Val)    , sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<char16_t>(i32Val), sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<char32_t>(i32Val), sdv::any_t(i32Val));
    EXPECT_EQ(static_cast<wchar_t>(i32Val) , sdv::any_t(i32Val));

    int64_t i64Val = 80;
    EXPECT_EQ(sdv::any_t(i64Val), sdv::any_t(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), i64Val);
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<float>(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<double>(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<char>(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<char16_t>(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<char32_t>(i64Val));
    EXPECT_EQ(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val));
    EXPECT_EQ(i64Val                       , sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<float>(i64Val)   , sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<double>(i64Val)  , sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<char>(i64Val)    , sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<char16_t>(i64Val), sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<char32_t>(i64Val), sdv::any_t(i64Val));
    EXPECT_EQ(static_cast<wchar_t>(i64Val) , sdv::any_t(i64Val));

    float fVal = 123.456f;
    EXPECT_EQ(sdv::any_t(fVal), sdv::any_t(fVal));
    EXPECT_EQ(sdv::any_t(fVal), fVal);
    EXPECT_EQ(sdv::any_t(fVal), static_cast<double>(fVal));
    EXPECT_EQ(sdv::any_t(123.f), static_cast<char>(fVal));
    EXPECT_EQ(sdv::any_t(123.f), static_cast<char16_t>(fVal));
    EXPECT_EQ(sdv::any_t(123.f), static_cast<char32_t>(fVal));
    EXPECT_EQ(sdv::any_t(123.f), static_cast<wchar_t>(fVal));
    EXPECT_EQ(fVal                       , sdv::any_t(fVal));
    EXPECT_EQ(static_cast<double>(fVal)  , sdv::any_t(fVal));
    EXPECT_EQ(static_cast<char>(fVal)    , sdv::any_t(123.f));
    EXPECT_EQ(static_cast<char16_t>(fVal), sdv::any_t(123.f));
    EXPECT_EQ(static_cast<char32_t>(fVal), sdv::any_t(123.f));
    EXPECT_EQ(static_cast<wchar_t>(fVal) , sdv::any_t(123.f));

    double dVal = 456.123;
    EXPECT_EQ(sdv::any_t(dVal), sdv::any_t(dVal));
    EXPECT_EQ(sdv::any_t(dVal), dVal);
    EXPECT_EQ(sdv::any_t(456.0), static_cast<char16_t>(dVal));
    EXPECT_EQ(sdv::any_t(456.0), static_cast<char32_t>(dVal));
    EXPECT_EQ(sdv::any_t(456.0), static_cast<wchar_t>(dVal));
    EXPECT_EQ(dVal                       , sdv::any_t(dVal));
    EXPECT_EQ(static_cast<char16_t>(dVal), sdv::any_t(456.0));
    EXPECT_EQ(static_cast<char32_t>(dVal), sdv::any_t(456.0));
    EXPECT_EQ(static_cast<wchar_t>(dVal) , sdv::any_t(456.0));

    long double ldVal = 654.321;
    EXPECT_EQ(sdv::any_t(ldVal), sdv::any_t(ldVal));
    EXPECT_EQ(sdv::any_t(ldVal), ldVal);
    EXPECT_EQ(sdv::any_t(654.0L), static_cast<char16_t>(ldVal));
    EXPECT_EQ(sdv::any_t(654.0L), static_cast<char32_t>(ldVal));
    EXPECT_EQ(sdv::any_t(654.0L), static_cast<wchar_t>(ldVal));
    EXPECT_EQ(ldVal                       , sdv::any_t(ldVal));
    EXPECT_EQ(static_cast<char16_t>(ldVal), sdv::any_t(654.0L));
    EXPECT_EQ(static_cast<char32_t>(ldVal), sdv::any_t(654.0L));
    EXPECT_EQ(static_cast<wchar_t>(ldVal) , sdv::any_t(654.0L));
}

TEST_F(CAnyTypeTest, CompareEqualString)
{
    sdv::string ssVal = "Hello";
    EXPECT_EQ(sdv::any_t(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), ssVal);
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<std::string>(ssVal));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_EQ(sdv::any_t(ssVal), static_cast<std::wstring>(L"Hello"));
    EXPECT_EQ(ssVal, sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<sdv::u8string>(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<sdv::u16string>(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<sdv::u32string>(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<sdv::wstring>(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<std::string>(ssVal), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<std::u16string>(u"Hello"), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<std::u32string>(U"Hello"), sdv::any_t(ssVal));
    EXPECT_EQ(static_cast<std::wstring>(L"Hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"Hello";
    EXPECT_EQ(sdv::any_t(ss8Val), sdv::any_t(ss8Val));
    EXPECT_EQ(sdv::any_t(ss8Val), ss8Val);
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<std::string>("Hello"));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_EQ(sdv::any_t(ss8Val), static_cast<std::wstring>(L"Hello"));
    EXPECT_EQ(ss8Val, sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<sdv::u16string>(ss8Val), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<sdv::u32string>(ss8Val), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<sdv::wstring>(ss8Val), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<std::string>("Hello"), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss8Val));
    EXPECT_EQ(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"Hello";
    EXPECT_EQ(sdv::any_t(ss16Val), sdv::any_t(ss16Val));
    EXPECT_EQ(sdv::any_t(ss16Val), ss16Val);
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val));
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val));
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<std::string>("Hello"));
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_EQ(sdv::any_t(ss16Val), static_cast<std::wstring>(L"Hello"));
    EXPECT_EQ(ss16Val, sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<sdv::u32string>(ss16Val), sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<sdv::wstring>(ss16Val), sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<std::string>("Hello"), sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss16Val));
    EXPECT_EQ(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"Hello";
    EXPECT_EQ(sdv::any_t(ss32Val), sdv::any_t(ss32Val));
    EXPECT_EQ(sdv::any_t(ss32Val), ss32Val);
    EXPECT_EQ(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val));
    EXPECT_EQ(sdv::any_t(ss32Val), static_cast<std::string>("Hello"));
    EXPECT_EQ(sdv::any_t(ss32Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_EQ(sdv::any_t(ss32Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_EQ(sdv::any_t(ss32Val), static_cast<std::wstring>(L"Hello"));
    EXPECT_EQ(ss32Val, sdv::any_t(ss32Val));
    EXPECT_EQ(static_cast<sdv::wstring>(ss32Val), sdv::any_t(ss32Val));
    EXPECT_EQ(static_cast<std::string>("Hello"), sdv::any_t(ss32Val));
    EXPECT_EQ(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss32Val));
    EXPECT_EQ(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss32Val));
    EXPECT_EQ(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"Hello";
    EXPECT_EQ(sdv::any_t(sswVal), sdv::any_t(sswVal));
    EXPECT_EQ(sdv::any_t(sswVal), sswVal);
    EXPECT_EQ(sdv::any_t(sswVal), static_cast<std::string>("Hello"));
    EXPECT_EQ(sdv::any_t(sswVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_EQ(sdv::any_t(sswVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_EQ(sdv::any_t(sswVal), static_cast<std::wstring>(L"Hello"));
    EXPECT_EQ(sswVal, sdv::any_t(sswVal));
    EXPECT_EQ(static_cast<std::string>("Hello"), sdv::any_t(sswVal));
    EXPECT_EQ(static_cast<std::u16string>(u"Hello"), sdv::any_t(sswVal));
    EXPECT_EQ(static_cast<std::u32string>(U"Hello"), sdv::any_t(sswVal));
    EXPECT_EQ(static_cast<std::wstring>(L"Hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareEqualSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_EQ(sdv::any_t(ifcVal), sdv::any_t(ifcVal));
    EXPECT_EQ(sdv::any_t(ifcVal), ifcVal);
    EXPECT_EQ(ifcVal, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id;
    EXPECT_EQ(sdv::any_t(idifcVal), sdv::any_t(idifcVal));
    EXPECT_EQ(sdv::any_t(idifcVal), idifcVal);
    EXPECT_EQ(idifcVal, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id;
    EXPECT_EQ(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal));
    EXPECT_EQ(sdv::any_t(idexceptVal), idexceptVal);
    EXPECT_EQ(idexceptVal, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareEqualInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_FALSE(sdv::any_t(dVal) == sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_FALSE(sdv::any_t(ifcVal) == sdv::any_t("123.456"));
}


TEST_F(CAnyTypeTest, CompareNotEqualArithmetic)
{
    uint8_t ui8Val = 10;
    uint8_t ui8Val2 = 20;
    EXPECT_NE(sdv::any_t(ui8Val), sdv::any_t(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), ui8Val2);
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<float>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<double>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<char>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val2));
    EXPECT_NE(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val2));
    EXPECT_NE(ui8Val2                       , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<uint16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<uint32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<uint64_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<int8_t>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<int16_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<int32_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<int64_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<float>(ui8Val2)   , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<double>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<char>(ui8Val2)    , sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<char16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<char32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_NE(static_cast<wchar_t>(ui8Val2) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 20;
    uint16_t ui16Val2 = 30;
    EXPECT_NE(sdv::any_t(ui16Val), sdv::any_t(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), ui16Val2);
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<float>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<double>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<char>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val2));
    EXPECT_NE(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val2));
    EXPECT_NE(ui16Val2                       , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<uint32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<uint64_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<int8_t>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<int16_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<int32_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<int64_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<float>(ui16Val2)   , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<double>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<char>(ui16Val2)    , sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<char16_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<char32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_NE(static_cast<wchar_t>(ui16Val2) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 30;
    uint32_t ui32Val2 = 40;
    EXPECT_NE(sdv::any_t(ui32Val), sdv::any_t(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), ui32Val2);
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<float>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<double>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<char>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val2));
    EXPECT_NE(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val2));
    EXPECT_NE(ui32Val2                       , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<uint64_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<int8_t>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<int16_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<int32_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<int64_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<float>(ui32Val2)   , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<double>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<char>(ui32Val2)    , sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<char16_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<char32_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_NE(static_cast<wchar_t>(ui32Val2) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 40;
    uint64_t ui64Val2 = 50;
    EXPECT_NE(sdv::any_t(ui64Val), sdv::any_t(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), ui64Val2);
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<float>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<double>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<char>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val2));
    EXPECT_NE(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val2));
    EXPECT_NE(ui64Val2                       , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<int8_t>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<int16_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<int32_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<int64_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<float>(ui64Val2)   , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<double>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<char>(ui64Val2)    , sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<char16_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<char32_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_NE(static_cast<wchar_t>(ui64Val2) , sdv::any_t(ui64Val));

    int8_t i8Val = 50;
    int8_t i8Val2 = 60;
    EXPECT_NE(sdv::any_t(i8Val), sdv::any_t(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), i8Val2);
    EXPECT_NE(sdv::any_t(i8Val), static_cast<int16_t>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<int32_t>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<int64_t>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<float>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<double>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<char>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<char16_t>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<char32_t>(i8Val2));
    EXPECT_NE(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val2));
    EXPECT_NE(i8Val2                       , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<int16_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<int32_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<int64_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<float>(i8Val2)   , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<double>(i8Val2)  , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<char>(i8Val2)    , sdv::any_t(i8Val));
    EXPECT_NE(static_cast<char16_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_NE(static_cast<char32_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_NE(static_cast<wchar_t>(i8Val2) , sdv::any_t(i8Val));

    int16_t i16Val = 60;
    int16_t i16Val2 = 70;
    EXPECT_NE(sdv::any_t(i16Val), sdv::any_t(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), i16Val2);
    EXPECT_NE(sdv::any_t(i16Val), static_cast<int32_t>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<int64_t>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<float>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<double>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<char>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<char16_t>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<char32_t>(i16Val2));
    EXPECT_NE(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val2));
    EXPECT_NE(i16Val2                       , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<int32_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<int64_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<float>(i16Val2)   , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<double>(i16Val2)  , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<char>(i16Val2)    , sdv::any_t(i16Val));
    EXPECT_NE(static_cast<char16_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_NE(static_cast<char32_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_NE(static_cast<wchar_t>(i16Val2) , sdv::any_t(i16Val));

    int32_t i32Val = 70;
    int32_t i32Val2 = 80;
    EXPECT_NE(sdv::any_t(i32Val), sdv::any_t(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), i32Val2);
    EXPECT_NE(sdv::any_t(i32Val), static_cast<int64_t>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<float>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<double>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<char>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<char16_t>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<char32_t>(i32Val2));
    EXPECT_NE(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val2));
    EXPECT_NE(i32Val2                       , sdv::any_t(i32Val));
    EXPECT_NE(static_cast<int64_t>(i32Val2) , sdv::any_t(i32Val));
    EXPECT_NE(static_cast<float>(i32Val2)   , sdv::any_t(i32Val));
    EXPECT_NE(static_cast<double>(i32Val2)  , sdv::any_t(i32Val));
    EXPECT_NE(static_cast<char>(i32Val2)    , sdv::any_t(i32Val));
    EXPECT_NE(static_cast<char16_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_NE(static_cast<char32_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_NE(static_cast<wchar_t>(i32Val2) , sdv::any_t(i32Val));

    int64_t i64Val = 80;
    int64_t i64Val2 = 90;
    EXPECT_NE(sdv::any_t(i64Val), sdv::any_t(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), i64Val2);
    EXPECT_NE(sdv::any_t(i64Val), static_cast<float>(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), static_cast<double>(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), static_cast<char>(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), static_cast<char16_t>(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), static_cast<char32_t>(i64Val2));
    EXPECT_NE(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val2));
    EXPECT_NE(i64Val2                       , sdv::any_t(i64Val));
    EXPECT_NE(static_cast<float>(i64Val2)   , sdv::any_t(i64Val));
    EXPECT_NE(static_cast<double>(i64Val2)  , sdv::any_t(i64Val));
    EXPECT_NE(static_cast<char>(i64Val2)    , sdv::any_t(i64Val));
    EXPECT_NE(static_cast<char16_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_NE(static_cast<char32_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_NE(static_cast<wchar_t>(i64Val2) , sdv::any_t(i64Val));

    float fVal = 123.456f;
    float fVal2 = 231.456f;
    EXPECT_NE(sdv::any_t(fVal), sdv::any_t(fVal2));
    EXPECT_NE(sdv::any_t(fVal), fVal2);
    EXPECT_NE(sdv::any_t(fVal), static_cast<double>(fVal2));
    EXPECT_NE(sdv::any_t(123.f), static_cast<char>(fVal2));
    EXPECT_NE(sdv::any_t(123.f), static_cast<char16_t>(fVal2));
    EXPECT_NE(sdv::any_t(123.f), static_cast<char32_t>(fVal2));
    EXPECT_NE(sdv::any_t(123.f), static_cast<wchar_t>(fVal2));
    EXPECT_NE(fVal2                       , sdv::any_t(fVal));
    EXPECT_NE(static_cast<double>(fVal2)  , sdv::any_t(fVal));
    EXPECT_NE(static_cast<char>(fVal2)    , sdv::any_t(123.f));
    EXPECT_NE(static_cast<char16_t>(fVal2), sdv::any_t(123.f));
    EXPECT_NE(static_cast<char32_t>(fVal2), sdv::any_t(123.f));
    EXPECT_NE(static_cast<wchar_t>(fVal2) , sdv::any_t(123.f));

    double dVal = 456.123;
    double dVal2 = 562.123;
    EXPECT_NE(sdv::any_t(dVal), sdv::any_t(dVal2));
    EXPECT_NE(sdv::any_t(dVal), dVal2);
    EXPECT_NE(sdv::any_t(456.0), static_cast<char16_t>(dVal2));
    EXPECT_NE(sdv::any_t(456.0), static_cast<char32_t>(dVal2));
    EXPECT_NE(sdv::any_t(456.0), static_cast<wchar_t>(dVal2));
    EXPECT_NE(dVal2                       , sdv::any_t(dVal));
    EXPECT_NE(static_cast<char16_t>(dVal2), sdv::any_t(456.0));
    EXPECT_NE(static_cast<char32_t>(dVal2), sdv::any_t(456.0));
    EXPECT_NE(static_cast<wchar_t>(dVal2) , sdv::any_t(456.0));

    long double ldVal = 654.321;
    long double ldVal2 = 546.321;
    EXPECT_NE(sdv::any_t(ldVal), sdv::any_t(ldVal2));
    EXPECT_NE(sdv::any_t(ldVal), ldVal2);
    EXPECT_NE(sdv::any_t(654.0L), static_cast<char16_t>(ldVal2));
    EXPECT_NE(sdv::any_t(654.0L), static_cast<char32_t>(ldVal2));
    EXPECT_NE(sdv::any_t(654.0L), static_cast<wchar_t>(ldVal2));
    EXPECT_NE(ldVal2                       , sdv::any_t(ldVal));
    EXPECT_NE(static_cast<char16_t>(ldVal2), sdv::any_t(654.0L));
    EXPECT_NE(static_cast<char32_t>(ldVal2), sdv::any_t(654.0L));
    EXPECT_NE(static_cast<wchar_t>(ldVal2) , sdv::any_t(654.0L));
}

TEST_F(CAnyTypeTest, CompareNotEqualString)
{
    sdv::string ssVal = "Hello";
    sdv::string ssVal2 = "hello";
    EXPECT_NE(sdv::any_t(ssVal), sdv::any_t(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), ssVal2);
    EXPECT_NE(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<std::string>(ssVal2));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<std::u16string>(u"hello"));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<std::u32string>(U"hello"));
    EXPECT_NE(sdv::any_t(ssVal), static_cast<std::wstring>(L"hello"));
    EXPECT_NE(ssVal2, sdv::any_t(ssVal));
    EXPECT_NE(static_cast<sdv::u8string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<sdv::u16string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<sdv::u32string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<sdv::wstring>(ssVal2), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<std::string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<std::u16string>(u"hello"), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<std::u32string>(U"hello"), sdv::any_t(ssVal));
    EXPECT_NE(static_cast<std::wstring>(L"hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"Hello";
    sdv::u8string ss8Val2 = u8"hello";
    EXPECT_NE(sdv::any_t(ss8Val), sdv::any_t(ss8Val2));
    EXPECT_NE(sdv::any_t(ss8Val), ss8Val2);
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val2));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val2));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val2));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<std::string>("hello"));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<std::u16string>(u"hello"));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<std::u32string>(U"hello"));
    EXPECT_NE(sdv::any_t(ss8Val), static_cast<std::wstring>(L"hello"));
    EXPECT_NE(ss8Val2, sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<sdv::u16string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<sdv::u32string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<sdv::wstring>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<std::string>("hello"), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss8Val));
    EXPECT_NE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"Hello";
    sdv::u16string ss16Val2 = u"hello";
    EXPECT_NE(sdv::any_t(ss16Val), sdv::any_t(ss16Val2));
    EXPECT_NE(sdv::any_t(ss16Val), ss16Val2);
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val2));
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val2));
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<std::string>("hello"));
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<std::u16string>(u"hello"));
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<std::u32string>(U"hello"));
    EXPECT_NE(sdv::any_t(ss16Val), static_cast<std::wstring>(L"hello"));
    EXPECT_NE(ss16Val2, sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<sdv::u32string>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<sdv::wstring>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<std::string>("hello"), sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss16Val));
    EXPECT_NE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"Hello";
    sdv::u32string ss32Val2 = U"hello";
    EXPECT_NE(sdv::any_t(ss32Val), sdv::any_t(ss32Val2));
    EXPECT_NE(sdv::any_t(ss32Val), ss32Val2);
    EXPECT_NE(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val2));
    EXPECT_NE(sdv::any_t(ss32Val), static_cast<std::string>("hello"));
    EXPECT_NE(sdv::any_t(ss32Val), static_cast<std::u16string>(u"hello"));
    EXPECT_NE(sdv::any_t(ss32Val), static_cast<std::u32string>(U"hello"));
    EXPECT_NE(sdv::any_t(ss32Val), static_cast<std::wstring>(L"hello"));
    EXPECT_NE(ss32Val2, sdv::any_t(ss32Val));
    EXPECT_NE(static_cast<sdv::wstring>(ss32Val2), sdv::any_t(ss32Val));
    EXPECT_NE(static_cast<std::string>("hello"), sdv::any_t(ss32Val));
    EXPECT_NE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss32Val));
    EXPECT_NE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss32Val));
    EXPECT_NE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"Hello";
    sdv::wstring sswVal2 = L"hello";
    EXPECT_NE(sdv::any_t(sswVal), sdv::any_t(sswVal2));
    EXPECT_NE(sdv::any_t(sswVal), sswVal2);
    EXPECT_NE(sdv::any_t(sswVal), static_cast<std::string>("hello"));
    EXPECT_NE(sdv::any_t(sswVal), static_cast<std::u16string>(u"hello"));
    EXPECT_NE(sdv::any_t(sswVal), static_cast<std::u32string>(U"hello"));
    EXPECT_NE(sdv::any_t(sswVal), static_cast<std::wstring>(L"hello"));
    EXPECT_NE(sswVal2, sdv::any_t(sswVal));
    EXPECT_NE(static_cast<std::string>("hello"), sdv::any_t(sswVal));
    EXPECT_NE(static_cast<std::u16string>(u"hello"), sdv::any_t(sswVal));
    EXPECT_NE(static_cast<std::u32string>(U"hello"), sdv::any_t(sswVal));
    EXPECT_NE(static_cast<std::wstring>(L"hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareNotEqualSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test, test2;
    sdv::interface_t ifcVal = &test;
    sdv::interface_t ifcVal2 = &test2;
    EXPECT_NE(sdv::any_t(ifcVal), sdv::any_t(ifcVal2));
    EXPECT_NE(sdv::any_t(ifcVal), ifcVal2);
    EXPECT_NE(ifcVal2, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id;
    sdv::interface_id idifcVal2 = sdv::IObjectDestroy::_id;
    EXPECT_NE(sdv::any_t(idifcVal), sdv::any_t(idifcVal2));
    EXPECT_NE(sdv::any_t(idifcVal), idifcVal2);
    EXPECT_NE(idifcVal2, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id;
    sdv::exception_id idexceptVal2 = sdv::XNullPointer::_id;
    EXPECT_NE(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal2));
    EXPECT_NE(sdv::any_t(idexceptVal), idexceptVal2);
    EXPECT_NE(idexceptVal2, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareNotEqualInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_TRUE(sdv::any_t(dVal) != sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_TRUE(sdv::any_t(ifcVal) != sdv::any_t("123.456"));
}

TEST_F(CAnyTypeTest, CompareSmallerThanArithmetic)
{
    uint8_t ui8Val = 10;
    uint8_t ui8Val2 = 20;
    EXPECT_LT(sdv::any_t(ui8Val), sdv::any_t(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), ui8Val2);
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<float>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<double>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<char>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val2));
    EXPECT_LT(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val2));
    ui8Val2 = 5;
    EXPECT_LT(ui8Val2                       , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<uint16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<uint32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<uint64_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<int8_t>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<int16_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<int32_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<int64_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<float>(ui8Val2)   , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<double>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<char>(ui8Val2)    , sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<char16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<char32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LT(static_cast<wchar_t>(ui8Val2) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 20;
    uint16_t ui16Val2 = 30;
    EXPECT_LT(sdv::any_t(ui16Val), sdv::any_t(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), ui16Val2);
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<float>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<double>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<char>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val2));
    EXPECT_LT(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val2));
    ui16Val2 = 10;
    EXPECT_LT(ui16Val2                       , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<uint32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<uint64_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<int8_t>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<int16_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<int32_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<int64_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<float>(ui16Val2)   , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<double>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<char>(ui16Val2)    , sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<char16_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<char32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LT(static_cast<wchar_t>(ui16Val2) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 30;
    uint32_t ui32Val2 = 40;
    EXPECT_LT(sdv::any_t(ui32Val), sdv::any_t(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), ui32Val2);
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<float>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<double>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<char>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val2));
    EXPECT_LT(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val2));
    ui32Val2 = 20;
    EXPECT_LT(ui32Val2                       , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<uint64_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<int8_t>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<int16_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<int32_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<int64_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<float>(ui32Val2)   , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<double>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<char>(ui32Val2)    , sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<char16_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<char32_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LT(static_cast<wchar_t>(ui32Val2) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 40;
    uint64_t ui64Val2 = 50;
    EXPECT_LT(sdv::any_t(ui64Val), sdv::any_t(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), ui64Val2);
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<float>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<double>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<char>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val2));
    EXPECT_LT(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val2));
    ui64Val2 = 30;
    EXPECT_LT(ui64Val2                       , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<int8_t>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<int16_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<int32_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<int64_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<float>(ui64Val2)   , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<double>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<char>(ui64Val2)    , sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<char16_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<char32_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_LT(static_cast<wchar_t>(ui64Val2) , sdv::any_t(ui64Val));

    int8_t i8Val = 50;
    int8_t i8Val2 = 60;
    EXPECT_LT(sdv::any_t(i8Val), sdv::any_t(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), i8Val2);
    EXPECT_LT(sdv::any_t(i8Val), static_cast<int16_t>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<int32_t>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<int64_t>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<float>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<double>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<char>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<char16_t>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<char32_t>(i8Val2));
    EXPECT_LT(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val2));
    i8Val2 = 40;
    EXPECT_LT(i8Val2                       , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<int16_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<int32_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<int64_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<float>(i8Val2)   , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<double>(i8Val2)  , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<char>(i8Val2)    , sdv::any_t(i8Val));
    EXPECT_LT(static_cast<char16_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_LT(static_cast<char32_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_LT(static_cast<wchar_t>(i8Val2) , sdv::any_t(i8Val));

    int16_t i16Val = 60;
    int16_t i16Val2 = 70;
    EXPECT_LT(sdv::any_t(i16Val), sdv::any_t(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), i16Val2);
    EXPECT_LT(sdv::any_t(i16Val), static_cast<int32_t>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<int64_t>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<float>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<double>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<char>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<char16_t>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<char32_t>(i16Val2));
    EXPECT_LT(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val2));
    i16Val2 = 50;
    EXPECT_LT(i16Val2                       , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<int32_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<int64_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<float>(i16Val2)   , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<double>(i16Val2)  , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<char>(i16Val2)    , sdv::any_t(i16Val));
    EXPECT_LT(static_cast<char16_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_LT(static_cast<char32_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_LT(static_cast<wchar_t>(i16Val2) , sdv::any_t(i16Val));

    int32_t i32Val = 70;
    int32_t i32Val2 = 80;
    EXPECT_LT(sdv::any_t(i32Val), sdv::any_t(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), i32Val2);
    EXPECT_LT(sdv::any_t(i32Val), static_cast<int64_t>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<float>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<double>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<char>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<char16_t>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<char32_t>(i32Val2));
    EXPECT_LT(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val2));
    i32Val2 = 60;
    EXPECT_LT(i32Val2                       , sdv::any_t(i32Val));
    EXPECT_LT(static_cast<int64_t>(i32Val2) , sdv::any_t(i32Val));
    EXPECT_LT(static_cast<float>(i32Val2)   , sdv::any_t(i32Val));
    EXPECT_LT(static_cast<double>(i32Val2)  , sdv::any_t(i32Val));
    EXPECT_LT(static_cast<char>(i32Val2)    , sdv::any_t(i32Val));
    EXPECT_LT(static_cast<char16_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_LT(static_cast<char32_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_LT(static_cast<wchar_t>(i32Val2) , sdv::any_t(i32Val));

    int64_t i64Val = 80;
    int64_t i64Val2 = 90;
    EXPECT_LT(sdv::any_t(i64Val), sdv::any_t(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), i64Val2);
    EXPECT_LT(sdv::any_t(i64Val), static_cast<float>(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), static_cast<double>(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), static_cast<char>(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), static_cast<char16_t>(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), static_cast<char32_t>(i64Val2));
    EXPECT_LT(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val2));
    i64Val2 = 70;
    EXPECT_LT(i64Val2                       , sdv::any_t(i64Val));
    EXPECT_LT(static_cast<float>(i64Val2)   , sdv::any_t(i64Val));
    EXPECT_LT(static_cast<double>(i64Val2)  , sdv::any_t(i64Val));
    EXPECT_LT(static_cast<char>(i64Val2)    , sdv::any_t(i64Val));
    EXPECT_LT(static_cast<char16_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_LT(static_cast<char32_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_LT(static_cast<wchar_t>(i64Val2) , sdv::any_t(i64Val));

    float fVal = 63.456f;
    float fVal2 = 95.456f;
    EXPECT_LT(sdv::any_t(fVal), sdv::any_t(fVal2));
    EXPECT_LT(sdv::any_t(fVal), fVal2);
    EXPECT_LT(sdv::any_t(fVal), static_cast<double>(fVal2));
    EXPECT_LT(sdv::any_t(63.f), static_cast<char>(fVal2));
    EXPECT_LT(sdv::any_t(63.f), static_cast<char16_t>(fVal2));
    EXPECT_LT(sdv::any_t(63.f), static_cast<char32_t>(fVal2));
    EXPECT_LT(sdv::any_t(63.f), static_cast<wchar_t>(fVal2));
    fVal2 = 31.456f;
    EXPECT_LT(fVal2                       , sdv::any_t(fVal));
    EXPECT_LT(static_cast<double>(fVal2)  , sdv::any_t(fVal));
    EXPECT_LT(static_cast<char>(fVal2)    , sdv::any_t(63.f));
    EXPECT_LT(static_cast<char16_t>(fVal2), sdv::any_t(63.f));
    EXPECT_LT(static_cast<char32_t>(fVal2), sdv::any_t(63.f));
    EXPECT_LT(static_cast<wchar_t>(fVal2) , sdv::any_t(63.f));

    double dVal = 456.123;
    double dVal2 = 562.123;
    EXPECT_LT(sdv::any_t(dVal), sdv::any_t(dVal2));
    EXPECT_LT(sdv::any_t(dVal), dVal2);
    EXPECT_LT(sdv::any_t(456.0), static_cast<char16_t>(dVal2));
    EXPECT_LT(sdv::any_t(456.0), static_cast<char32_t>(dVal2));
    EXPECT_LT(sdv::any_t(456.0), static_cast<wchar_t>(dVal2));
    dVal2 = 62.123;
    EXPECT_LT(dVal2                       , sdv::any_t(dVal));
    EXPECT_LT(static_cast<char16_t>(dVal2), sdv::any_t(456.0));
    EXPECT_LT(static_cast<char32_t>(dVal2), sdv::any_t(456.0));
    EXPECT_LT(static_cast<wchar_t>(dVal2) , sdv::any_t(456.0));

    long double ldVal = 454.321;
    long double ldVal2 = 546.321;
    EXPECT_LT(sdv::any_t(ldVal), sdv::any_t(ldVal2));
    EXPECT_LT(sdv::any_t(ldVal), ldVal2);
    EXPECT_LT(sdv::any_t(454.0L), static_cast<char16_t>(ldVal2));
    EXPECT_LT(sdv::any_t(454.0L), static_cast<char32_t>(ldVal2));
    EXPECT_LT(sdv::any_t(454.0L), static_cast<wchar_t>(ldVal2));
    ldVal2 = 46.321;
    EXPECT_LT(ldVal2                       , sdv::any_t(ldVal));
    EXPECT_LT(static_cast<char16_t>(ldVal2), sdv::any_t(454.0L));
    EXPECT_LT(static_cast<char32_t>(ldVal2), sdv::any_t(454.0L));
    EXPECT_LT(static_cast<wchar_t>(ldVal2) , sdv::any_t(454.0L));
}

TEST_F(CAnyTypeTest, CompareSmallerThanString)
{
    sdv::string ssVal = "Hello";
    sdv::string ssVal2 = "hello";
    EXPECT_LT(sdv::any_t(ssVal), sdv::any_t(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), ssVal2);
    EXPECT_LT(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<std::string>(ssVal2));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<std::u16string>(u"hello"));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<std::u32string>(U"hello"));
    EXPECT_LT(sdv::any_t(ssVal), static_cast<std::wstring>(L"hello"));
    ssVal = "hello";
    ssVal2 = "Hello";
    EXPECT_LT(ssVal2, sdv::any_t(ssVal));
    EXPECT_LT(static_cast<sdv::u8string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<sdv::u16string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<sdv::u32string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<sdv::wstring>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<std::string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<std::u16string>(u"Hello"), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<std::u32string>(U"Hello"), sdv::any_t(ssVal));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"Hello";
    sdv::u8string ss8Val2 = u8"hello";
    EXPECT_LT(sdv::any_t(ss8Val), sdv::any_t(ss8Val2));
    EXPECT_LT(sdv::any_t(ss8Val), ss8Val2);
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val2));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val2));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val2));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<std::string>("hello"));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LT(sdv::any_t(ss8Val), static_cast<std::wstring>(L"hello"));
    ss8Val = u8"hello";
    ss8Val2 = u8"Hello";
    EXPECT_LT(ss8Val2, sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<sdv::u16string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<sdv::u32string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<sdv::wstring>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<std::string>("Hello"), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss8Val));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"Hello";
    sdv::u16string ss16Val2 = u"hello";
    EXPECT_LT(sdv::any_t(ss16Val), sdv::any_t(ss16Val2));
    EXPECT_LT(sdv::any_t(ss16Val), ss16Val2);
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val2));
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val2));
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<std::string>("hello"));
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LT(sdv::any_t(ss16Val), static_cast<std::wstring>(L"hello"));
    ss16Val = u"hello";
    ss16Val2 = u"Hello";
    EXPECT_LT(ss16Val2, sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<sdv::u32string>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<sdv::wstring>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<std::string>("Hello"), sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss16Val));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"Hello";
    sdv::u32string ss32Val2 = U"hello";
    EXPECT_LT(sdv::any_t(ss32Val), sdv::any_t(ss32Val2));
    EXPECT_LT(sdv::any_t(ss32Val), ss32Val2);
    EXPECT_LT(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val2));
    EXPECT_LT(sdv::any_t(ss32Val), static_cast<std::string>("hello"));
    EXPECT_LT(sdv::any_t(ss32Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LT(sdv::any_t(ss32Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LT(sdv::any_t(ss32Val), static_cast<std::wstring>(L"hello"));
    ss32Val = U"hello";
    ss32Val2 = U"Hello";
    EXPECT_LT(ss32Val2, sdv::any_t(ss32Val));
    EXPECT_LT(static_cast<sdv::wstring>(ss32Val2), sdv::any_t(ss32Val));
    EXPECT_LT(static_cast<std::string>("Hello"), sdv::any_t(ss32Val));
    EXPECT_LT(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss32Val));
    EXPECT_LT(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss32Val));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"Hello";
    sdv::wstring sswVal2 = L"hello";
    EXPECT_LT(sdv::any_t(sswVal), sdv::any_t(sswVal2));
    EXPECT_LT(sdv::any_t(sswVal), sswVal2);
    EXPECT_LT(sdv::any_t(sswVal), static_cast<std::string>("hello"));
    EXPECT_LT(sdv::any_t(sswVal), static_cast<std::u16string>(u"hello"));
    EXPECT_LT(sdv::any_t(sswVal), static_cast<std::u32string>(U"hello"));
    EXPECT_LT(sdv::any_t(sswVal), static_cast<std::wstring>(L"hello"));
    sswVal = L"hello";
    sswVal2 = L"Hello";
    EXPECT_LT(sswVal2, sdv::any_t(sswVal));
    EXPECT_LT(static_cast<std::string>("Hello"), sdv::any_t(sswVal));
    EXPECT_LT(static_cast<std::u16string>(u"Hello"), sdv::any_t(sswVal));
    EXPECT_LT(static_cast<std::u32string>(U"Hello"), sdv::any_t(sswVal));
    EXPECT_LT(static_cast<std::wstring>(L"Hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareSmallerThanSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test, test2;
    sdv::interface_t ifcVal = reinterpret_cast<uint64_t>(&test) < reinterpret_cast<uint64_t>(&test2) ? &test : &test2;
    sdv::interface_t ifcVal2 = reinterpret_cast<uint64_t>(&test) < reinterpret_cast<uint64_t>(&test2) ? &test2 : &test;
    EXPECT_LT(sdv::any_t(ifcVal), sdv::any_t(ifcVal2));
    EXPECT_LT(sdv::any_t(ifcVal), ifcVal2);
    std::swap(ifcVal, ifcVal2);
    EXPECT_LT(ifcVal2, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id < sdv::IObjectDestroy::_id ? sdv::IInterfaceAccess::_id : sdv::IObjectDestroy::_id;
    sdv::interface_id idifcVal2 = sdv::IInterfaceAccess::_id < sdv::IObjectDestroy::_id ? sdv::IObjectDestroy::_id : sdv::IInterfaceAccess::_id;
    EXPECT_LT(sdv::any_t(idifcVal), sdv::any_t(idifcVal2));
    EXPECT_LT(sdv::any_t(idifcVal), idifcVal2);
    std::swap(idifcVal, idifcVal2);
    EXPECT_LT(idifcVal2, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id < sdv::XNullPointer::_id ? sdv::XNoInterface::_id : sdv::XNullPointer::_id;
    sdv::exception_id idexceptVal2 = sdv::XNoInterface::_id < sdv::XNullPointer::_id ? sdv::XNullPointer::_id : sdv::XNoInterface::_id;
    EXPECT_LT(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal2));
    EXPECT_LT(sdv::any_t(idexceptVal), idexceptVal2);
    std::swap(idexceptVal, idexceptVal2);
    EXPECT_LT(idexceptVal2, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareSmallerThanInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_FALSE(sdv::any_t(dVal) < sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_FALSE(sdv::any_t(ifcVal) < sdv::any_t("123.456"));
}

TEST_F(CAnyTypeTest, CompareSmallerThanOrEqualArithmetic)
{
    uint8_t ui8Val = 10;
    uint8_t ui8Val2 = 20;
    EXPECT_LE(sdv::any_t(ui8Val), sdv::any_t(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), sdv::any_t(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), ui8Val2);
    EXPECT_LE(sdv::any_t(ui8Val), ui8Val);
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<float>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<float>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<double>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<double>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<char>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<char>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val2));
    EXPECT_LE(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val));
    ui8Val2 = 5;
    EXPECT_LE(ui8Val2                       , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<uint16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<uint32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<uint64_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<int8_t>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<int16_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<int32_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<int64_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<float>(ui8Val2)   , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<double>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<char>(ui8Val2)    , sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<char16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<char32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_LE(static_cast<wchar_t>(ui8Val2) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 20;
    uint16_t ui16Val2 = 30;
    EXPECT_LE(sdv::any_t(ui16Val), sdv::any_t(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), ui16Val2);
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<float>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<double>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<char>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val2));
    EXPECT_LE(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val2));
    ui16Val2 = 10;
    EXPECT_LE(ui16Val2                       , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<uint32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<uint64_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<int8_t>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<int16_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<int32_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<int64_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<float>(ui16Val2)   , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<double>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<char>(ui16Val2)    , sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<char16_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<char32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_LE(static_cast<wchar_t>(ui16Val2) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 30;
    uint32_t ui32Val2 = 40;
    EXPECT_LE(sdv::any_t(ui32Val), sdv::any_t(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), ui32Val2);
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<float>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<double>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<char>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val2));
    EXPECT_LE(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val2));
    ui32Val2 = 20;
    EXPECT_LE(ui32Val2                       , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<uint64_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<int8_t>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<int16_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<int32_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<int64_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<float>(ui32Val2)   , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<double>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<char>(ui32Val2)    , sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<char16_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<char32_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_LE(static_cast<wchar_t>(ui32Val2) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 40;
    uint64_t ui64Val2 = 50;
    EXPECT_LE(sdv::any_t(ui64Val), sdv::any_t(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), ui64Val2);
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<float>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<double>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<char>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val2));
    EXPECT_LE(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val2));
    ui64Val2 = 30;
    EXPECT_LE(ui64Val2                       , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<int8_t>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<int16_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<int32_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<int64_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<float>(ui64Val2)   , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<double>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<char>(ui64Val2)    , sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<char16_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<char32_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_LE(static_cast<wchar_t>(ui64Val2) , sdv::any_t(ui64Val));

    int8_t i8Val = 50;
    int8_t i8Val2 = 60;
    EXPECT_LE(sdv::any_t(i8Val), sdv::any_t(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), i8Val2);
    EXPECT_LE(sdv::any_t(i8Val), static_cast<int16_t>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<int32_t>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<int64_t>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<float>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<double>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<char>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<char16_t>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<char32_t>(i8Val2));
    EXPECT_LE(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val2));
    i8Val2 = 40;
    EXPECT_LE(i8Val2                       , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<int16_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<int32_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<int64_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<float>(i8Val2)   , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<double>(i8Val2)  , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<char>(i8Val2)    , sdv::any_t(i8Val));
    EXPECT_LE(static_cast<char16_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_LE(static_cast<char32_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_LE(static_cast<wchar_t>(i8Val2) , sdv::any_t(i8Val));

    int16_t i16Val = 60;
    int16_t i16Val2 = 70;
    EXPECT_LE(sdv::any_t(i16Val), sdv::any_t(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), i16Val2);
    EXPECT_LE(sdv::any_t(i16Val), static_cast<int32_t>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<int64_t>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<float>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<double>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<char>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<char16_t>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<char32_t>(i16Val2));
    EXPECT_LE(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val2));
    i16Val2 = 50;
    EXPECT_LE(i16Val2                       , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<int32_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<int64_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<float>(i16Val2)   , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<double>(i16Val2)  , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<char>(i16Val2)    , sdv::any_t(i16Val));
    EXPECT_LE(static_cast<char16_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_LE(static_cast<char32_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_LE(static_cast<wchar_t>(i16Val2) , sdv::any_t(i16Val));

    int32_t i32Val = 70;
    int32_t i32Val2 = 80;
    EXPECT_LE(sdv::any_t(i32Val), sdv::any_t(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), i32Val2);
    EXPECT_LE(sdv::any_t(i32Val), static_cast<int64_t>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<float>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<double>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<char>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<char16_t>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<char32_t>(i32Val2));
    EXPECT_LE(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val2));
    i32Val2 = 60;
    EXPECT_LE(i32Val2                       , sdv::any_t(i32Val));
    EXPECT_LE(static_cast<int64_t>(i32Val2) , sdv::any_t(i32Val));
    EXPECT_LE(static_cast<float>(i32Val2)   , sdv::any_t(i32Val));
    EXPECT_LE(static_cast<double>(i32Val2)  , sdv::any_t(i32Val));
    EXPECT_LE(static_cast<char>(i32Val2)    , sdv::any_t(i32Val));
    EXPECT_LE(static_cast<char16_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_LE(static_cast<char32_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_LE(static_cast<wchar_t>(i32Val2) , sdv::any_t(i32Val));

    int64_t i64Val = 80;
    int64_t i64Val2 = 90;
    EXPECT_LE(sdv::any_t(i64Val), sdv::any_t(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), i64Val2);
    EXPECT_LE(sdv::any_t(i64Val), static_cast<float>(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), static_cast<double>(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), static_cast<char>(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), static_cast<char16_t>(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), static_cast<char32_t>(i64Val2));
    EXPECT_LE(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val2));
    i64Val2 = 70;
    EXPECT_LE(i64Val2                       , sdv::any_t(i64Val));
    EXPECT_LE(static_cast<float>(i64Val2)   , sdv::any_t(i64Val));
    EXPECT_LE(static_cast<double>(i64Val2)  , sdv::any_t(i64Val));
    EXPECT_LE(static_cast<char>(i64Val2)    , sdv::any_t(i64Val));
    EXPECT_LE(static_cast<char16_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_LE(static_cast<char32_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_LE(static_cast<wchar_t>(i64Val2) , sdv::any_t(i64Val));

    float fVal = 63.456f;
    float fVal2 = 95.456f;
    EXPECT_LE(sdv::any_t(fVal), sdv::any_t(fVal2));
    EXPECT_LE(sdv::any_t(fVal), fVal2);
    EXPECT_LE(sdv::any_t(fVal), static_cast<double>(fVal2));
    EXPECT_LE(sdv::any_t(63.f), static_cast<char>(fVal2));
    EXPECT_LE(sdv::any_t(63.f), static_cast<char16_t>(fVal2));
    EXPECT_LE(sdv::any_t(63.f), static_cast<char32_t>(fVal2));
    EXPECT_LE(sdv::any_t(63.f), static_cast<wchar_t>(fVal2));
    fVal2 = 31.456f;
    EXPECT_LE(fVal2                       , sdv::any_t(fVal));
    EXPECT_LE(static_cast<double>(fVal2)  , sdv::any_t(fVal));
    EXPECT_LE(static_cast<char>(fVal2)    , sdv::any_t(63.f));
    EXPECT_LE(static_cast<char16_t>(fVal2), sdv::any_t(63.f));
    EXPECT_LE(static_cast<char32_t>(fVal2), sdv::any_t(63.f));
    EXPECT_LE(static_cast<wchar_t>(fVal2) , sdv::any_t(63.f));

    double dVal = 456.123;
    double dVal2 = 562.123;
    EXPECT_LE(sdv::any_t(dVal), sdv::any_t(dVal2));
    EXPECT_LE(sdv::any_t(dVal), dVal2);
    EXPECT_LE(sdv::any_t(456.0), static_cast<char16_t>(dVal2));
    EXPECT_LE(sdv::any_t(456.0), static_cast<char32_t>(dVal2));
    EXPECT_LE(sdv::any_t(456.0), static_cast<wchar_t>(dVal2));
    dVal2 = 62.123;
    EXPECT_LE(dVal2                       , sdv::any_t(dVal));
    EXPECT_LE(static_cast<char16_t>(dVal2), sdv::any_t(456.0));
    EXPECT_LE(static_cast<char32_t>(dVal2), sdv::any_t(456.0));
    EXPECT_LE(static_cast<wchar_t>(dVal2) , sdv::any_t(456.0));

    long double ldVal = 454.321;
    long double ldVal2 = 546.321;
    EXPECT_LE(sdv::any_t(ldVal), sdv::any_t(ldVal2));
    EXPECT_LE(sdv::any_t(ldVal), ldVal2);
    EXPECT_LE(sdv::any_t(454.0L), static_cast<char16_t>(ldVal2));
    EXPECT_LE(sdv::any_t(454.0L), static_cast<char32_t>(ldVal2));
    EXPECT_LE(sdv::any_t(454.0L), static_cast<wchar_t>(ldVal2));
    ldVal2 = 46.321;
    EXPECT_LE(ldVal2                       , sdv::any_t(ldVal));
    EXPECT_LE(static_cast<char16_t>(ldVal2), sdv::any_t(454.0L));
    EXPECT_LE(static_cast<char32_t>(ldVal2), sdv::any_t(454.0L));
    EXPECT_LE(static_cast<wchar_t>(ldVal2) , sdv::any_t(454.0L));
}

TEST_F(CAnyTypeTest, CompareSmallerThanOrEqualString)
{
    sdv::string ssVal = "Hello";
    sdv::string ssVal2 = "hello";
    EXPECT_LE(sdv::any_t(ssVal), sdv::any_t(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), sdv::any_t(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), ssVal2);
    EXPECT_LE(sdv::any_t(ssVal), ssVal);
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::string>(ssVal2));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::string>(ssVal));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::u16string>(u"hello"));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::u32string>(U"hello"));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::wstring>(L"hello"));
    EXPECT_LE(sdv::any_t(ssVal), static_cast<std::wstring>(L"Hello"));
    ssVal = "hello";
    ssVal2 = "Hello";
    EXPECT_LE(ssVal2, sdv::any_t(ssVal));
    EXPECT_LE(static_cast<sdv::u8string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<sdv::u16string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<sdv::u32string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<sdv::wstring>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<std::string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<std::u16string>(u"Hello"), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<std::u32string>(U"Hello"), sdv::any_t(ssVal));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"Hello";
    sdv::u8string ss8Val2 = u8"hello";
    EXPECT_LE(sdv::any_t(ss8Val), sdv::any_t(ss8Val2));
    EXPECT_LE(sdv::any_t(ss8Val), ss8Val2);
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val2));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val2));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val2));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<std::string>("hello"));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LE(sdv::any_t(ss8Val), static_cast<std::wstring>(L"hello"));
    ss8Val = u8"hello";
    ss8Val2 = u8"Hello";
    EXPECT_LE(ss8Val2, sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<sdv::u16string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<sdv::u32string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<sdv::wstring>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<std::string>("Hello"), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss8Val));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"Hello";
    sdv::u16string ss16Val2 = u"hello";
    EXPECT_LE(sdv::any_t(ss16Val), sdv::any_t(ss16Val2));
    EXPECT_LE(sdv::any_t(ss16Val), ss16Val2);
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val2));
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val2));
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<std::string>("hello"));
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LE(sdv::any_t(ss16Val), static_cast<std::wstring>(L"hello"));
    ss16Val = u"hello";
    ss16Val2 = u"Hello";
    EXPECT_LE(ss16Val2, sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<sdv::u32string>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<sdv::wstring>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<std::string>("Hello"), sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss16Val));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"Hello";
    sdv::u32string ss32Val2 = U"hello";
    EXPECT_LE(sdv::any_t(ss32Val), sdv::any_t(ss32Val2));
    EXPECT_LE(sdv::any_t(ss32Val), ss32Val2);
    EXPECT_LE(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val2));
    EXPECT_LE(sdv::any_t(ss32Val), static_cast<std::string>("hello"));
    EXPECT_LE(sdv::any_t(ss32Val), static_cast<std::u16string>(u"hello"));
    EXPECT_LE(sdv::any_t(ss32Val), static_cast<std::u32string>(U"hello"));
    EXPECT_LE(sdv::any_t(ss32Val), static_cast<std::wstring>(L"hello"));
    ss32Val = U"hello";
    ss32Val2 = U"Hello";
    EXPECT_LE(ss32Val2, sdv::any_t(ss32Val));
    EXPECT_LE(static_cast<sdv::wstring>(ss32Val2), sdv::any_t(ss32Val));
    EXPECT_LE(static_cast<std::string>("Hello"), sdv::any_t(ss32Val));
    EXPECT_LE(static_cast<std::u16string>(u"Hello"), sdv::any_t(ss32Val));
    EXPECT_LE(static_cast<std::u32string>(U"Hello"), sdv::any_t(ss32Val));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"Hello";
    sdv::wstring sswVal2 = L"hello";
    EXPECT_LE(sdv::any_t(sswVal), sdv::any_t(sswVal2));
    EXPECT_LE(sdv::any_t(sswVal), sswVal2);
    EXPECT_LE(sdv::any_t(sswVal), static_cast<std::string>("hello"));
    EXPECT_LE(sdv::any_t(sswVal), static_cast<std::u16string>(u"hello"));
    EXPECT_LE(sdv::any_t(sswVal), static_cast<std::u32string>(U"hello"));
    EXPECT_LE(sdv::any_t(sswVal), static_cast<std::wstring>(L"hello"));
    sswVal = L"hello";
    sswVal2 = L"Hello";
    EXPECT_LE(sswVal2, sdv::any_t(sswVal));
    EXPECT_LE(static_cast<std::string>("Hello"), sdv::any_t(sswVal));
    EXPECT_LE(static_cast<std::u16string>(u"Hello"), sdv::any_t(sswVal));
    EXPECT_LE(static_cast<std::u32string>(U"Hello"), sdv::any_t(sswVal));
    EXPECT_LE(static_cast<std::wstring>(L"Hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareSmallerThanOrEqualSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test, test2;
    sdv::interface_t ifcVal = reinterpret_cast<uint64_t>(&test) < reinterpret_cast<uint64_t>(&test2) ? &test : &test2;
    sdv::interface_t ifcVal2 = reinterpret_cast<uint64_t>(&test) < reinterpret_cast<uint64_t>(&test2) ? &test2 : &test;
    EXPECT_LE(sdv::any_t(ifcVal), sdv::any_t(ifcVal2));
    EXPECT_LE(sdv::any_t(ifcVal), sdv::any_t(ifcVal));
    EXPECT_LE(sdv::any_t(ifcVal), ifcVal2);
    EXPECT_LE(sdv::any_t(ifcVal), ifcVal);
    std::swap(ifcVal, ifcVal2);
    EXPECT_LE(ifcVal2, sdv::any_t(ifcVal));
    EXPECT_LE(ifcVal, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id < sdv::IObjectDestroy::_id ? sdv::IInterfaceAccess::_id : sdv::IObjectDestroy::_id;
    sdv::interface_id idifcVal2 = sdv::IInterfaceAccess::_id < sdv::IObjectDestroy::_id ? sdv::IObjectDestroy::_id : sdv::IInterfaceAccess::_id;
    EXPECT_LE(sdv::any_t(idifcVal), sdv::any_t(idifcVal2));
    EXPECT_LE(sdv::any_t(idifcVal), sdv::any_t(idifcVal));
    EXPECT_LE(sdv::any_t(idifcVal), idifcVal2);
    EXPECT_LE(sdv::any_t(idifcVal), idifcVal);
    std::swap(idifcVal, idifcVal2);
    EXPECT_LE(idifcVal2, sdv::any_t(idifcVal));
    EXPECT_LE(idifcVal, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id < sdv::XNullPointer::_id ? sdv::XNoInterface::_id : sdv::XNullPointer::_id;
    sdv::exception_id idexceptVal2 = sdv::XNoInterface::_id < sdv::XNullPointer::_id ? sdv::XNullPointer::_id : sdv::XNoInterface::_id;
    EXPECT_LE(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal2));
    EXPECT_LE(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal));
    EXPECT_LE(sdv::any_t(idexceptVal), idexceptVal2);
    EXPECT_LE(sdv::any_t(idexceptVal), idexceptVal);
    std::swap(idexceptVal, idexceptVal2);
    EXPECT_LE(idexceptVal2, sdv::any_t(idexceptVal));
    EXPECT_LE(idexceptVal, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareSmallerThanOrEqualInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_FALSE(sdv::any_t(dVal) <= sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_FALSE(sdv::any_t(ifcVal) <= sdv::any_t("123.456"));
}

TEST_F(CAnyTypeTest, CompareLargerThanArithmetic)
{
    uint8_t ui8Val = 20;
    uint8_t ui8Val2 = 10;
    EXPECT_GT(sdv::any_t(ui8Val), sdv::any_t(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), ui8Val2);
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<float>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<double>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<char>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val2));
    EXPECT_GT(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val2));
    ui8Val2 = 30;
    EXPECT_GT(ui8Val2                       , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<uint16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<uint32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<uint64_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<int8_t>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<int16_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<int32_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<int64_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<float>(ui8Val2)   , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<double>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<char>(ui8Val2)    , sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<char16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<char32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GT(static_cast<wchar_t>(ui8Val2) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 30;
    uint16_t ui16Val2 = 20;
    EXPECT_GT(sdv::any_t(ui16Val), sdv::any_t(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), ui16Val2);
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<float>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<double>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<char>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val2));
    EXPECT_GT(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val2));
    ui16Val2 = 40;
    EXPECT_GT(ui16Val2                       , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<uint32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<uint64_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<int8_t>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<int16_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<int32_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<int64_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<float>(ui16Val2)   , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<double>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<char>(ui16Val2)    , sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<char16_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<char32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GT(static_cast<wchar_t>(ui16Val2) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 40;
    uint32_t ui32Val2 = 30;
    EXPECT_GT(sdv::any_t(ui32Val), sdv::any_t(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), ui32Val2);
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<float>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<double>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<char>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val2));
    EXPECT_GT(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val2));
    ui32Val2 = 50;
    EXPECT_GT(ui32Val2                       , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<uint64_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<int8_t>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<int16_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<int32_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<int64_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<float>(ui32Val2)   , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<double>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<char>(ui32Val2)    , sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<char16_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<char32_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GT(static_cast<wchar_t>(ui32Val2) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 50;
    uint64_t ui64Val2 = 40;
    EXPECT_GT(sdv::any_t(ui64Val), sdv::any_t(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), ui64Val2);
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<float>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<double>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<char>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val2));
    EXPECT_GT(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val2));
    ui64Val2 = 60;
    EXPECT_GT(ui64Val2                       , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<int8_t>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<int16_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<int32_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<int64_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<float>(ui64Val2)   , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<double>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<char>(ui64Val2)    , sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<char16_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<char32_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_GT(static_cast<wchar_t>(ui64Val2) , sdv::any_t(ui64Val));

    int8_t i8Val = 60;
    int8_t i8Val2 = 50;
    EXPECT_GT(sdv::any_t(i8Val), sdv::any_t(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), i8Val2);
    EXPECT_GT(sdv::any_t(i8Val), static_cast<int16_t>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<int32_t>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<int64_t>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<float>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<double>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<char>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<char16_t>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<char32_t>(i8Val2));
    EXPECT_GT(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val2));
    i8Val2 = 70;
    EXPECT_GT(i8Val2                       , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<int16_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<int32_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<int64_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<float>(i8Val2)   , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<double>(i8Val2)  , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<char>(i8Val2)    , sdv::any_t(i8Val));
    EXPECT_GT(static_cast<char16_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_GT(static_cast<char32_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_GT(static_cast<wchar_t>(i8Val2) , sdv::any_t(i8Val));

    int16_t i16Val = 70;
    int16_t i16Val2 = 60;
    EXPECT_GT(sdv::any_t(i16Val), sdv::any_t(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), i16Val2);
    EXPECT_GT(sdv::any_t(i16Val), static_cast<int32_t>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<int64_t>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<float>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<double>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<char>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<char16_t>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<char32_t>(i16Val2));
    EXPECT_GT(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val2));
    i16Val2 = 80;
    EXPECT_GT(i16Val2                       , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<int32_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<int64_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<float>(i16Val2)   , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<double>(i16Val2)  , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<char>(i16Val2)    , sdv::any_t(i16Val));
    EXPECT_GT(static_cast<char16_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_GT(static_cast<char32_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_GT(static_cast<wchar_t>(i16Val2) , sdv::any_t(i16Val));

    int32_t i32Val = 80;
    int32_t i32Val2 = 70;
    EXPECT_GT(sdv::any_t(i32Val), sdv::any_t(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), i32Val2);
    EXPECT_GT(sdv::any_t(i32Val), static_cast<int64_t>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<float>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<double>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<char>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<char16_t>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<char32_t>(i32Val2));
    EXPECT_GT(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val2));
    i32Val2 = 90;
    EXPECT_GT(i32Val2                       , sdv::any_t(i32Val));
    EXPECT_GT(static_cast<int64_t>(i32Val2) , sdv::any_t(i32Val));
    EXPECT_GT(static_cast<float>(i32Val2)   , sdv::any_t(i32Val));
    EXPECT_GT(static_cast<double>(i32Val2)  , sdv::any_t(i32Val));
    EXPECT_GT(static_cast<char>(i32Val2)    , sdv::any_t(i32Val));
    EXPECT_GT(static_cast<char16_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_GT(static_cast<char32_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_GT(static_cast<wchar_t>(i32Val2) , sdv::any_t(i32Val));

    int64_t i64Val = 90;
    int64_t i64Val2 = 80;
    EXPECT_GT(sdv::any_t(i64Val), sdv::any_t(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), i64Val2);
    EXPECT_GT(sdv::any_t(i64Val), static_cast<float>(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), static_cast<double>(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), static_cast<char>(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), static_cast<char16_t>(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), static_cast<char32_t>(i64Val2));
    EXPECT_GT(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val2));
    i64Val2 = 100;
    EXPECT_GT(i64Val2                       , sdv::any_t(i64Val));
    EXPECT_GT(static_cast<float>(i64Val2)   , sdv::any_t(i64Val));
    EXPECT_GT(static_cast<double>(i64Val2)  , sdv::any_t(i64Val));
    EXPECT_GT(static_cast<char>(i64Val2)    , sdv::any_t(i64Val));
    EXPECT_GT(static_cast<char16_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_GT(static_cast<char32_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_GT(static_cast<wchar_t>(i64Val2) , sdv::any_t(i64Val));

    float fVal = 95.456f;
    float fVal2 = 63.456f;
    EXPECT_GT(sdv::any_t(fVal), sdv::any_t(fVal2));
    EXPECT_GT(sdv::any_t(fVal), fVal2);
    EXPECT_GT(sdv::any_t(fVal), static_cast<double>(fVal2));
    EXPECT_GT(sdv::any_t(95.f), static_cast<char>(fVal2));
    EXPECT_GT(sdv::any_t(95.f), static_cast<char16_t>(fVal2));
    EXPECT_GT(sdv::any_t(95.f), static_cast<char32_t>(fVal2));
    EXPECT_GT(sdv::any_t(95.f), static_cast<wchar_t>(fVal2));
    fVal2 = 111.456f;
    EXPECT_GT(fVal2                       , sdv::any_t(fVal));
    EXPECT_GT(static_cast<double>(fVal2)  , sdv::any_t(fVal));
    EXPECT_GT(static_cast<char>(fVal2)    , sdv::any_t(95.f));
    EXPECT_GT(static_cast<char16_t>(fVal2), sdv::any_t(95.f));
    EXPECT_GT(static_cast<char32_t>(fVal2), sdv::any_t(95.f));
    EXPECT_GT(static_cast<wchar_t>(fVal2) , sdv::any_t(95.f));

    double dVal = 562.123;
    double dVal2 = 456.123;
    EXPECT_GT(sdv::any_t(dVal), sdv::any_t(dVal2));
    EXPECT_GT(sdv::any_t(dVal), dVal2);
    EXPECT_GT(sdv::any_t(562.0), static_cast<char16_t>(dVal2));
    EXPECT_GT(sdv::any_t(562.0), static_cast<char32_t>(dVal2));
    EXPECT_GT(sdv::any_t(562.0), static_cast<wchar_t>(dVal2));
    dVal2 = 1062.123;
    EXPECT_GT(dVal2                       , sdv::any_t(dVal));
    EXPECT_GT(static_cast<char16_t>(dVal2), sdv::any_t(562.0));
    EXPECT_GT(static_cast<char32_t>(dVal2), sdv::any_t(562.0));
    EXPECT_GT(static_cast<wchar_t>(dVal2) , sdv::any_t(562.0));

    long double ldVal = 546.321;
    long double ldVal2 = 454.321;
    EXPECT_GT(sdv::any_t(ldVal), sdv::any_t(ldVal2));
    EXPECT_GT(sdv::any_t(ldVal), ldVal2);
    EXPECT_GT(sdv::any_t(546.0L), static_cast<char16_t>(ldVal2));
    EXPECT_GT(sdv::any_t(546.0L), static_cast<char32_t>(ldVal2));
    EXPECT_GT(sdv::any_t(546.0L), static_cast<wchar_t>(ldVal2));
    ldVal2 = 1046.321;
    EXPECT_GT(ldVal2                       , sdv::any_t(ldVal));
    EXPECT_GT(static_cast<char16_t>(ldVal2), sdv::any_t(546.0L));
    EXPECT_GT(static_cast<char32_t>(ldVal2), sdv::any_t(546.0L));
    EXPECT_GT(static_cast<wchar_t>(ldVal2) , sdv::any_t(546.0L));
}

TEST_F(CAnyTypeTest, CompareLargerThanString)
{
    sdv::string ssVal = "hello";
    sdv::string ssVal2 = "Hello";
    EXPECT_GT(sdv::any_t(ssVal), sdv::any_t(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), ssVal2);
    EXPECT_GT(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<std::string>(ssVal2));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_GT(sdv::any_t(ssVal), static_cast<std::wstring>(L"Hello"));
    ssVal = "Hello";
    ssVal2 = "hello";
    EXPECT_GT(ssVal2, sdv::any_t(ssVal));
    EXPECT_GT(static_cast<sdv::u8string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<sdv::u16string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<sdv::u32string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<sdv::wstring>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<std::string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<std::u16string>(u"hello"), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<std::u32string>(U"hello"), sdv::any_t(ssVal));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"hello";
    sdv::u8string ss8Val2 = u8"Hello";
    EXPECT_GT(sdv::any_t(ss8Val), sdv::any_t(ss8Val2));
    EXPECT_GT(sdv::any_t(ss8Val), ss8Val2);
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val2));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val2));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val2));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<std::string>("Hello"));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GT(sdv::any_t(ss8Val), static_cast<std::wstring>(L"Hello"));
    ss8Val = u8"Hello";
    ss8Val2 = u8"hello";
    EXPECT_GT(ss8Val2, sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<sdv::u16string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<sdv::u32string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<sdv::wstring>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<std::string>("hello"), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<std::u16string>(u"hello"), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<std::u32string>(U"hello"), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(ss8Val));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"hello";
    sdv::u16string ss16Val2 = u"Hello";
    EXPECT_GT(sdv::any_t(ss16Val), sdv::any_t(ss16Val2));
    EXPECT_GT(sdv::any_t(ss16Val), ss16Val2);
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val2));
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val2));
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<std::string>("Hello"));
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GT(sdv::any_t(ss16Val), static_cast<std::wstring>(L"Hello"));
    ss16Val = u"Hello";
    ss16Val2 = u"hello";
    EXPECT_GT(ss16Val2, sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<sdv::u32string>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<sdv::wstring>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<std::string>("hello"), sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<std::u16string>(u"hello"), sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<std::u32string>(U"hello"), sdv::any_t(ss16Val));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"hello";
    sdv::u32string ss32Val2 = U"Hello";
    EXPECT_GT(sdv::any_t(ss32Val), sdv::any_t(ss32Val2));
    EXPECT_GT(sdv::any_t(ss32Val), ss32Val2);
    EXPECT_GT(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val2));
    EXPECT_GT(sdv::any_t(ss32Val), static_cast<std::string>("Hello"));
    EXPECT_GT(sdv::any_t(ss32Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GT(sdv::any_t(ss32Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GT(sdv::any_t(ss32Val), static_cast<std::wstring>(L"Hello"));
    ss32Val = U"Hello";
    ss32Val2 = U"hello";
    EXPECT_GT(ss32Val2, sdv::any_t(ss32Val));
    EXPECT_GT(static_cast<sdv::wstring>(ss32Val2), sdv::any_t(ss32Val));
    EXPECT_GT(static_cast<std::string>("hello"), sdv::any_t(ss32Val));
    EXPECT_GT(static_cast<std::u16string>(u"hello"), sdv::any_t(ss32Val));
    EXPECT_GT(static_cast<std::u32string>(U"hello"), sdv::any_t(ss32Val));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"hello";
    sdv::wstring sswVal2 = L"Hello";
    EXPECT_GT(sdv::any_t(sswVal), sdv::any_t(sswVal2));
    EXPECT_GT(sdv::any_t(sswVal), sswVal2);
    EXPECT_GT(sdv::any_t(sswVal), static_cast<std::string>("Hello"));
    EXPECT_GT(sdv::any_t(sswVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_GT(sdv::any_t(sswVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_GT(sdv::any_t(sswVal), static_cast<std::wstring>(L"Hello"));
    sswVal = L"Hello";
    sswVal2 = L"hello";
    EXPECT_GT(sswVal2, sdv::any_t(sswVal));
    EXPECT_GT(static_cast<std::string>("hello"), sdv::any_t(sswVal));
    EXPECT_GT(static_cast<std::u16string>(u"hello"), sdv::any_t(sswVal));
    EXPECT_GT(static_cast<std::u32string>(U"hello"), sdv::any_t(sswVal));
    EXPECT_GT(static_cast<std::wstring>(L"hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareLargerThanSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test, test2;
    sdv::interface_t ifcVal = reinterpret_cast<uint64_t>(&test) > reinterpret_cast<uint64_t>(&test2) ? &test : &test2;
    sdv::interface_t ifcVal2 = reinterpret_cast<uint64_t>(&test) > reinterpret_cast<uint64_t>(&test2) ? &test2 : &test;
    EXPECT_GT(sdv::any_t(ifcVal), sdv::any_t(ifcVal2));
    EXPECT_GT(sdv::any_t(ifcVal), ifcVal2);
    std::swap(ifcVal, ifcVal2);
    EXPECT_GT(ifcVal2, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id > sdv::IObjectDestroy::_id ? sdv::IInterfaceAccess::_id : sdv::IObjectDestroy::_id;
    sdv::interface_id idifcVal2 = sdv::IInterfaceAccess::_id > sdv::IObjectDestroy::_id ? sdv::IObjectDestroy::_id : sdv::IInterfaceAccess::_id;
    EXPECT_GT(sdv::any_t(idifcVal), sdv::any_t(idifcVal2));
    EXPECT_GT(sdv::any_t(idifcVal), idifcVal2);
    std::swap(idifcVal, idifcVal2);
    EXPECT_GT(idifcVal2, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id > sdv::XNullPointer::_id ? sdv::XNoInterface::_id : sdv::XNullPointer::_id;
    sdv::exception_id idexceptVal2 = sdv::XNoInterface::_id > sdv::XNullPointer::_id ? sdv::XNullPointer::_id : sdv::XNoInterface::_id;
    EXPECT_GT(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal2));
    EXPECT_GT(sdv::any_t(idexceptVal), idexceptVal2);
    std::swap(idexceptVal, idexceptVal2);
    EXPECT_GT(idexceptVal2, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareLargerThanInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_FALSE(sdv::any_t(dVal) > sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_FALSE(sdv::any_t(ifcVal) > sdv::any_t("123.456"));
}

TEST_F(CAnyTypeTest, CompareLargerThanOrEqualArithmetic)
{
    uint8_t ui8Val = 20;
    uint8_t ui8Val2 = 10;
    EXPECT_GE(sdv::any_t(ui8Val), sdv::any_t(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), sdv::any_t(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), ui8Val2);
    EXPECT_GE(sdv::any_t(ui8Val), ui8Val);
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint16_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint32_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<uint64_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int8_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int16_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int32_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<int64_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<float>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<float>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<double>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<double>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<char>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<char>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<char16_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<char32_t>(ui8Val));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val2));
    EXPECT_GE(sdv::any_t(ui8Val), static_cast<wchar_t>(ui8Val));
    ui8Val2 = 30;
    EXPECT_GE(ui8Val2                       , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<uint16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<uint32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<uint64_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<int8_t>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<int16_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<int32_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<int64_t>(ui8Val2) , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<float>(ui8Val2)   , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<double>(ui8Val2)  , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<char>(ui8Val2)    , sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<char16_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<char32_t>(ui8Val2), sdv::any_t(ui8Val));
    EXPECT_GE(static_cast<wchar_t>(ui8Val2) , sdv::any_t(ui8Val));

    uint16_t ui16Val = 30;
    uint16_t ui16Val2 = 20;
    EXPECT_GE(sdv::any_t(ui16Val), sdv::any_t(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), ui16Val2);
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<uint32_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<uint64_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<int8_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<int16_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<int32_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<int64_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<float>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<double>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<char>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<char16_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<char32_t>(ui16Val2));
    EXPECT_GE(sdv::any_t(ui16Val), static_cast<wchar_t>(ui16Val2));
    ui16Val2 = 40;
    EXPECT_GE(ui16Val2                       , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<uint32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<uint64_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<int8_t>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<int16_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<int32_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<int64_t>(ui16Val2) , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<float>(ui16Val2)   , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<double>(ui16Val2)  , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<char>(ui16Val2)    , sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<char16_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<char32_t>(ui16Val2), sdv::any_t(ui16Val));
    EXPECT_GE(static_cast<wchar_t>(ui16Val2) , sdv::any_t(ui16Val));

    uint32_t ui32Val = 40;
    uint32_t ui32Val2 = 30;
    EXPECT_GE(sdv::any_t(ui32Val), sdv::any_t(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), ui32Val2);
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<uint64_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<int8_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<int16_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<int32_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<int64_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<float>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<double>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<char>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<char16_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<char32_t>(ui32Val2));
    EXPECT_GE(sdv::any_t(ui32Val), static_cast<wchar_t>(ui32Val2));
    ui32Val2 = 50;
    EXPECT_GE(ui32Val2                       , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<uint64_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<int8_t>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<int16_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<int32_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<int64_t>(ui32Val2) , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<float>(ui32Val2)   , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<double>(ui32Val2)  , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<char>(ui32Val2)    , sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<char16_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<char32_t>(ui32Val2), sdv::any_t(ui32Val));
    EXPECT_GE(static_cast<wchar_t>(ui32Val2) , sdv::any_t(ui32Val));

    uint64_t ui64Val = 50;
    uint64_t ui64Val2 = 40;
    EXPECT_GE(sdv::any_t(ui64Val), sdv::any_t(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), ui64Val2);
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<int8_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<int16_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<int32_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<int64_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<float>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<double>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<char>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<char16_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<char32_t>(ui64Val2));
    EXPECT_GE(sdv::any_t(ui64Val), static_cast<wchar_t>(ui64Val2));
    ui64Val2 = 60;
    EXPECT_GE(ui64Val2                       , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<int8_t>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<int16_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<int32_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<int64_t>(ui64Val2) , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<float>(ui64Val2)   , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<double>(ui64Val2)  , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<char>(ui64Val2)    , sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<char16_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<char32_t>(ui64Val2), sdv::any_t(ui64Val));
    EXPECT_GE(static_cast<wchar_t>(ui64Val2) , sdv::any_t(ui64Val));

    int8_t i8Val = 60;
    int8_t i8Val2 = 50;
    EXPECT_GE(sdv::any_t(i8Val), sdv::any_t(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), i8Val2);
    EXPECT_GE(sdv::any_t(i8Val), static_cast<int16_t>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<int32_t>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<int64_t>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<float>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<double>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<char>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<char16_t>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<char32_t>(i8Val2));
    EXPECT_GE(sdv::any_t(i8Val), static_cast<wchar_t>(i8Val2));
    i8Val2 = 70;
    EXPECT_GE(i8Val2                       , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<int16_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<int32_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<int64_t>(i8Val2) , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<float>(i8Val2)   , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<double>(i8Val2)  , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<char>(i8Val2)    , sdv::any_t(i8Val));
    EXPECT_GE(static_cast<char16_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_GE(static_cast<char32_t>(i8Val2), sdv::any_t(i8Val));
    EXPECT_GE(static_cast<wchar_t>(i8Val2) , sdv::any_t(i8Val));

    int16_t i16Val = 70;
    int16_t i16Val2 = 60;
    EXPECT_GE(sdv::any_t(i16Val), sdv::any_t(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), i16Val2);
    EXPECT_GE(sdv::any_t(i16Val), static_cast<int32_t>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<int64_t>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<float>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<double>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<char>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<char16_t>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<char32_t>(i16Val2));
    EXPECT_GE(sdv::any_t(i16Val), static_cast<wchar_t>(i16Val2));
    i16Val2 = 80;
    EXPECT_GE(i16Val2                       , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<int32_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<int64_t>(i16Val2) , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<float>(i16Val2)   , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<double>(i16Val2)  , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<char>(i16Val2)    , sdv::any_t(i16Val));
    EXPECT_GE(static_cast<char16_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_GE(static_cast<char32_t>(i16Val2), sdv::any_t(i16Val));
    EXPECT_GE(static_cast<wchar_t>(i16Val2) , sdv::any_t(i16Val));

    int32_t i32Val = 80;
    int32_t i32Val2 = 70;
    EXPECT_GE(sdv::any_t(i32Val), sdv::any_t(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), i32Val2);
    EXPECT_GE(sdv::any_t(i32Val), static_cast<int64_t>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<float>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<double>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<char>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<char16_t>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<char32_t>(i32Val2));
    EXPECT_GE(sdv::any_t(i32Val), static_cast<wchar_t>(i32Val2));
    i32Val2 = 90;
    EXPECT_GE(i32Val2                       , sdv::any_t(i32Val));
    EXPECT_GE(static_cast<int64_t>(i32Val2) , sdv::any_t(i32Val));
    EXPECT_GE(static_cast<float>(i32Val2)   , sdv::any_t(i32Val));
    EXPECT_GE(static_cast<double>(i32Val2)  , sdv::any_t(i32Val));
    EXPECT_GE(static_cast<char>(i32Val2)    , sdv::any_t(i32Val));
    EXPECT_GE(static_cast<char16_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_GE(static_cast<char32_t>(i32Val2), sdv::any_t(i32Val));
    EXPECT_GE(static_cast<wchar_t>(i32Val2) , sdv::any_t(i32Val));

    int64_t i64Val = 90;
    int64_t i64Val2 = 80;
    EXPECT_GE(sdv::any_t(i64Val), sdv::any_t(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), i64Val2);
    EXPECT_GE(sdv::any_t(i64Val), static_cast<float>(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), static_cast<double>(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), static_cast<char>(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), static_cast<char16_t>(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), static_cast<char32_t>(i64Val2));
    EXPECT_GE(sdv::any_t(i64Val), static_cast<wchar_t>(i64Val2));
    i64Val2 = 100;
    EXPECT_GE(i64Val2                       , sdv::any_t(i64Val));
    EXPECT_GE(static_cast<float>(i64Val2)   , sdv::any_t(i64Val));
    EXPECT_GE(static_cast<double>(i64Val2)  , sdv::any_t(i64Val));
    EXPECT_GE(static_cast<char>(i64Val2)    , sdv::any_t(i64Val));
    EXPECT_GE(static_cast<char16_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_GE(static_cast<char32_t>(i64Val2), sdv::any_t(i64Val));
    EXPECT_GE(static_cast<wchar_t>(i64Val2) , sdv::any_t(i64Val));

    float fVal = 95.456f;
    float fVal2 = 63.456f;
    EXPECT_GE(sdv::any_t(fVal), sdv::any_t(fVal2));
    EXPECT_GE(sdv::any_t(fVal), fVal2);
    EXPECT_GE(sdv::any_t(fVal), static_cast<double>(fVal2));
    EXPECT_GE(sdv::any_t(95.f), static_cast<char>(fVal2));
    EXPECT_GE(sdv::any_t(95.f), static_cast<char16_t>(fVal2));
    EXPECT_GE(sdv::any_t(95.f), static_cast<char32_t>(fVal2));
    EXPECT_GE(sdv::any_t(95.f), static_cast<wchar_t>(fVal2));
    fVal2 = 111.456f;
    EXPECT_GE(fVal2                       , sdv::any_t(fVal));
    EXPECT_GE(static_cast<double>(fVal2)  , sdv::any_t(fVal));
    EXPECT_GE(static_cast<char>(fVal2)    , sdv::any_t(95.f));
    EXPECT_GE(static_cast<char16_t>(fVal2), sdv::any_t(95.f));
    EXPECT_GE(static_cast<char32_t>(fVal2), sdv::any_t(95.f));
    EXPECT_GE(static_cast<wchar_t>(fVal2) , sdv::any_t(95.f));

    double dVal = 562.123;
    double dVal2 = 456.123;
    EXPECT_GE(sdv::any_t(dVal), sdv::any_t(dVal2));
    EXPECT_GE(sdv::any_t(dVal), dVal2);
    EXPECT_GE(sdv::any_t(456.0), static_cast<char16_t>(dVal2));
    EXPECT_GE(sdv::any_t(456.0), static_cast<char32_t>(dVal2));
    EXPECT_GE(sdv::any_t(456.0), static_cast<wchar_t>(dVal2));
    dVal2 = 662.123;
    EXPECT_GE(dVal2                       , sdv::any_t(dVal));
    EXPECT_GE(static_cast<char16_t>(dVal2), sdv::any_t(562.0));
    EXPECT_GE(static_cast<char32_t>(dVal2), sdv::any_t(562.0));
    EXPECT_GE(static_cast<wchar_t>(dVal2) , sdv::any_t(562.0));

    long double ldVal = 546.321;
    long double ldVal2 = 454.321;
    EXPECT_GE(sdv::any_t(ldVal), sdv::any_t(ldVal2));
    EXPECT_GE(sdv::any_t(ldVal), ldVal2);
    EXPECT_GE(sdv::any_t(546.0L), static_cast<char16_t>(ldVal2));
    EXPECT_GE(sdv::any_t(546.0L), static_cast<char32_t>(ldVal2));
    EXPECT_GE(sdv::any_t(546.0L), static_cast<wchar_t>(ldVal2));
    ldVal2 = 1046.321;
    EXPECT_GE(ldVal2                       , sdv::any_t(ldVal));
    EXPECT_GE(static_cast<char16_t>(ldVal2), sdv::any_t(546.0L));
    EXPECT_GE(static_cast<char32_t>(ldVal2), sdv::any_t(546.0L));
    EXPECT_GE(static_cast<wchar_t>(ldVal2) , sdv::any_t(546.0L));
}

TEST_F(CAnyTypeTest, CompareLargerThanOrEqualString)
{
    sdv::string ssVal = "hello";
    sdv::string ssVal2 = "Hello";
    EXPECT_GE(sdv::any_t(ssVal), sdv::any_t(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), sdv::any_t(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), ssVal2);
    EXPECT_GE(sdv::any_t(ssVal), ssVal);
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u8string>(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u16string>(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::u32string>(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<sdv::wstring>(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::string>(ssVal2));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::string>(ssVal));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::u16string>(u"hello"));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::u32string>(U"hello"));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::wstring>(L"Hello"));
    EXPECT_GE(sdv::any_t(ssVal), static_cast<std::wstring>(L"hello"));
    ssVal = "Hello";
    ssVal2 = "hello";
    EXPECT_GE(ssVal2, sdv::any_t(ssVal));
    EXPECT_GE(static_cast<sdv::u8string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<sdv::u16string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<sdv::u32string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<sdv::wstring>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<std::string>(ssVal2), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<std::u16string>(u"hello"), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<std::u32string>(U"hello"), sdv::any_t(ssVal));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(ssVal));

    sdv::u8string ss8Val = u8"hello";
    sdv::u8string ss8Val2 = u8"Hello";
    EXPECT_GE(sdv::any_t(ss8Val), sdv::any_t(ss8Val2));
    EXPECT_GE(sdv::any_t(ss8Val), ss8Val2);
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<sdv::u16string>(ss8Val2));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<sdv::u32string>(ss8Val2));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<sdv::wstring>(ss8Val2));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<std::string>("Hello"));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GE(sdv::any_t(ss8Val), static_cast<std::wstring>(L"Hello"));
    ss8Val = u8"Hello";
    ss8Val2 = u8"hello";
    EXPECT_GE(ss8Val2, sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<sdv::u16string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<sdv::u32string>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<sdv::wstring>(ss8Val2), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<std::string>("hello"), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss8Val));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss8Val));

    sdv::u16string ss16Val = u"hello";
    sdv::u16string ss16Val2 = u"Hello";
    EXPECT_GE(sdv::any_t(ss16Val), sdv::any_t(ss16Val2));
    EXPECT_GE(sdv::any_t(ss16Val), ss16Val2);
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<sdv::u32string>(ss16Val2));
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<sdv::wstring>(ss16Val2));
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<std::string>("Hello"));
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GE(sdv::any_t(ss16Val), static_cast<std::wstring>(L"Hello"));
    ss16Val = u"Hello";
    ss16Val2 = u"hello";
    EXPECT_GE(ss16Val2, sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<sdv::u32string>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<sdv::wstring>(ss16Val2), sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<std::string>("hello"), sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss16Val));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss16Val));

    sdv::u32string ss32Val = U"hello";
    sdv::u32string ss32Val2 = U"Hello";
    EXPECT_GE(sdv::any_t(ss32Val), sdv::any_t(ss32Val2));
    EXPECT_GE(sdv::any_t(ss32Val), ss32Val2);
    EXPECT_GE(sdv::any_t(ss32Val), static_cast<sdv::wstring>(ss32Val2));
    EXPECT_GE(sdv::any_t(ss32Val), static_cast<std::string>("Hello"));
    EXPECT_GE(sdv::any_t(ss32Val), static_cast<std::u16string>(u"Hello"));
    EXPECT_GE(sdv::any_t(ss32Val), static_cast<std::u32string>(U"Hello"));
    EXPECT_GE(sdv::any_t(ss32Val), static_cast<std::wstring>(L"hello"));
    ss32Val = U"Hello";
    ss32Val2 = U"hello";
    EXPECT_GE(ss32Val2, sdv::any_t(ss32Val));
    EXPECT_GE(static_cast<sdv::wstring>(ss32Val2), sdv::any_t(ss32Val));
    EXPECT_GE(static_cast<std::string>("hello"), sdv::any_t(ss32Val));
    EXPECT_GE(static_cast<std::u16string>(u"hello"), sdv::any_t(ss32Val));
    EXPECT_GE(static_cast<std::u32string>(U"hello"), sdv::any_t(ss32Val));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(ss32Val));

    sdv::wstring sswVal = L"hello";
    sdv::wstring sswVal2 = L"Hello";
    EXPECT_GE(sdv::any_t(sswVal), sdv::any_t(sswVal2));
    EXPECT_GE(sdv::any_t(sswVal), sswVal2);
    EXPECT_GE(sdv::any_t(sswVal), static_cast<std::string>("Hello"));
    EXPECT_GE(sdv::any_t(sswVal), static_cast<std::u16string>(u"Hello"));
    EXPECT_GE(sdv::any_t(sswVal), static_cast<std::u32string>(U"Hello"));
    EXPECT_GE(sdv::any_t(sswVal), static_cast<std::wstring>(L"Hello"));
    sswVal = L"Hello";
    sswVal2 = L"hello";
    EXPECT_GE(sswVal2, sdv::any_t(sswVal));
    EXPECT_GE(static_cast<std::string>("hello"), sdv::any_t(sswVal));
    EXPECT_GE(static_cast<std::u16string>(u"hello"), sdv::any_t(sswVal));
    EXPECT_GE(static_cast<std::u32string>(U"hello"), sdv::any_t(sswVal));
    EXPECT_GE(static_cast<std::wstring>(L"hello"), sdv::any_t(sswVal));
}

TEST_F(CAnyTypeTest, CompareLargerThanOrEqualSpecialTypes)
{
    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test, test2;
    sdv::interface_t ifcVal = reinterpret_cast<uint64_t>(&test) > reinterpret_cast<uint64_t>(&test2) ? &test : &test2;
    sdv::interface_t ifcVal2 = reinterpret_cast<uint64_t>(&test) > reinterpret_cast<uint64_t>(&test2) ? &test2 : &test;
    EXPECT_GE(sdv::any_t(ifcVal), sdv::any_t(ifcVal2));
    EXPECT_GE(sdv::any_t(ifcVal), sdv::any_t(ifcVal));
    EXPECT_GE(sdv::any_t(ifcVal), ifcVal2);
    EXPECT_GE(sdv::any_t(ifcVal), ifcVal);
    std::swap(ifcVal, ifcVal2);
    EXPECT_GE(ifcVal2, sdv::any_t(ifcVal));
    EXPECT_GE(ifcVal, sdv::any_t(ifcVal));

    sdv::interface_id idifcVal = sdv::IInterfaceAccess::_id > sdv::IObjectDestroy::_id ? sdv::IInterfaceAccess::_id : sdv::IObjectDestroy::_id;
    sdv::interface_id idifcVal2 = sdv::IInterfaceAccess::_id > sdv::IObjectDestroy::_id ? sdv::IObjectDestroy::_id : sdv::IInterfaceAccess::_id;
    EXPECT_GE(sdv::any_t(idifcVal), sdv::any_t(idifcVal2));
    EXPECT_GE(sdv::any_t(idifcVal), sdv::any_t(idifcVal));
    EXPECT_GE(sdv::any_t(idifcVal), idifcVal2);
    EXPECT_GE(sdv::any_t(idifcVal), idifcVal);
    std::swap(idifcVal, idifcVal2);
    EXPECT_GE(idifcVal2, sdv::any_t(idifcVal));
    EXPECT_GE(idifcVal, sdv::any_t(idifcVal));

    sdv::exception_id idexceptVal = sdv::XNoInterface::_id > sdv::XNullPointer::_id ? sdv::XNoInterface::_id : sdv::XNullPointer::_id;
    sdv::exception_id idexceptVal2 = sdv::XNoInterface::_id > sdv::XNullPointer::_id ? sdv::XNullPointer::_id : sdv::XNoInterface::_id;
    EXPECT_GE(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal2));
    EXPECT_GE(sdv::any_t(idexceptVal), sdv::any_t(idexceptVal));
    EXPECT_GE(sdv::any_t(idexceptVal), idexceptVal2);
    EXPECT_GE(sdv::any_t(idexceptVal), idexceptVal);
    std::swap(idexceptVal, idexceptVal2);
    EXPECT_GE(idexceptVal2, sdv::any_t(idexceptVal));
    EXPECT_GE(idexceptVal, sdv::any_t(idexceptVal));
}

TEST_F(CAnyTypeTest, CompareLargerThanOrEqualInvalidTypes)
{
    double dVal = 123.456;
    EXPECT_FALSE(sdv::any_t(dVal) >= sdv::any_t("123.456"));

    struct STest : public sdv::IInterfaceAccess
    {
        BEGIN_SDV_INTERFACE_MAP()
        END_SDV_INTERFACE_MAP()
    } test;
    sdv::interface_t ifcVal = &test;
    EXPECT_FALSE(sdv::any_t(ifcVal) >= sdv::any_t("123.456"));
}