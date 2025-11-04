#include <support/string.h>
#include <sstream>
#include "basic_types_test.h"

#ifdef __GNUC__
    // There are some versions of GCC that produce bogus warnings for -Wstringop-overflow (e.g. version 9.4 warns, 11.4 not -
    // changing the compile order without changing the logical behavior, will produce different results). See also:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100477 And https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115074 Suppress this
    // warning for the string class. NOTE 03.08.2025: Additional bogus warnigs/errors are suppressed for newer versions of the
    // compiler.
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
    #pragma GCC diagnostic ignored "-Warray-bounds"
    #pragma GCC diagnostic ignored "-Wrestrict"
#endif

using CWStringTypeTest = CBasicTypesTest;

TEST_F(CWStringTypeTest, ConstructorStatic)
{
    // Empty string
    sdv::fixed_wstring<10> ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::fixed_wstring<10> ssCString(L"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");
    std::wstring ss(L"Hello");
    sdv::fixed_wstring<10> ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, L"Hello");

    // Copy constructor
    sdv::fixed_wstring<8> ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");

    // Move constructor
    sdv::fixed_wstring<12> ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");

    // Fill constructor
    sdv::fixed_wstring<20> ssFillString(10, L'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, L"**********");

    // Substring constructor
    sdv::fixed_wstring<20> ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, L"llo");
    sdv::fixed_wstring<21> ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, L"ll");
    sdv::fixed_wstring<22> ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, L"llo");
    sdv::fixed_wstring<23> ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, L"ll");
    sdv::fixed_wstring<24> ssSubstring5(L"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, L"He\0l", 4), 0);

    // Iterator based construction
    sdv::fixed_wstring<10> ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, L"Hello");
    sdv::fixed_wstring<10> ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, L"Hello");

    // Construct using initializer list
    sdv::fixed_wstring<10> ssInitListString{L'H', L'e', L'l', L'l', L'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, L"Hello");
}

TEST_F(CWStringTypeTest, ConstructorDynamic)
{
    // Empty string
    sdv::wstring ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::wstring ssCString(L"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");
    std::wstring ss(L"Hello");
    sdv::wstring ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, L"Hello");

    // Copy constructor
    sdv::wstring ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");

    // Move constructor
    sdv::wstring ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, L"Hello");

    // Fill constructor
    sdv::wstring ssFillString(10, L'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, L"**********");

    // Substring constructor
    sdv::wstring ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, L"llo");
    sdv::wstring ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, L"ll");
    sdv::wstring ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, L"llo");
    sdv::wstring ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, L"ll");
    sdv::wstring ssSubstring5(L"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, L"He\0l", 4), 0);

    // Iterator based construction
    sdv::wstring ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, L"Hello");
    sdv::wstring ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, L"Hello");

    // Construct using initializer list
    sdv::wstring ssInitListString{L'H', L'e', L'l', L'l', L'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, L"Hello");
}

TEST_F(CWStringTypeTest, ConstructorMixed)
{
    // Copy constructor
    sdv::wstring ssDynamic = L"dynamic";
    sdv::fixed_wstring<8> ssStaticCopy(ssDynamic);
    EXPECT_EQ(ssStaticCopy, L"dynamic");
    sdv::fixed_wstring<8> ssStatic = L"static";
    sdv::wstring ssDynamicCopy(ssStatic);
    EXPECT_EQ(ssDynamicCopy, L"static");

    // Move constructor
    sdv::fixed_wstring<8> ssStaticMove(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, L"dynamic");
    sdv::wstring ssDynamicMove(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, L"static");
}

TEST_F(CWStringTypeTest, AssignmentOperatorStatic)
{
    // SDV-String assignment
    sdv::fixed_wstring<10> ssSdvString1(L"Hello");
    sdv::fixed_wstring<15> ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, L"Hello");

    // Move assignment
    sdv::fixed_wstring<10> ssSdvString3(L"Hello");
    sdv::fixed_wstring<15> ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, L"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::wstring ss1(L"Hello");
    sdv::fixed_wstring<10> ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, L"Hello");

    // C string assignment
    const wchar_t sz1[] = L"Hello";
    sdv::fixed_wstring<10>  ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, L"Hello");
    const wchar_t* sz2 = L"Hello";
    sdv::fixed_wstring<10>   ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, L"Hello");

    // Initializer list
    sdv::fixed_wstring<10> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = L"Hello";
    EXPECT_EQ(ssSdvString8, L"Hello");
    sdv::fixed_wstring<10> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {L'H', L'e', L'l', L'l', L'o'};
    EXPECT_EQ(ssSdvString9, L"Hello");
}

TEST_F(CWStringTypeTest, AssignmentOperatorDynamic)
{
    // SDV-String assignment
    sdv::wstring ssSdvString1(L"Hello");
    sdv::wstring ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, L"Hello");

    // Move assignment
    sdv::wstring ssSdvString3(L"Hello");
    sdv::wstring ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, L"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::wstring ss1(L"Hello");
    sdv::wstring ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, L"Hello");

    // C string assignment
    const wchar_t sz1[] = L"Hello";
    sdv::wstring ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, L"Hello");
    const wchar_t* sz2 = L"Hello";
    sdv::wstring ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, L"Hello");

    // Initializer list
    sdv::wstring ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = L"Hello";
    EXPECT_EQ(ssSdvString8, L"Hello");
    sdv::wstring ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {L'H', L'e', L'l', L'l', L'o'};
    EXPECT_EQ(ssSdvString9, L"Hello");
}

TEST_F(CWStringTypeTest, AssignmentOperatorMixed)
{
    // Copy assignment
    sdv::wstring ssDynamic = L"dynamic";
    sdv::fixed_wstring<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy = ssDynamic;
    EXPECT_EQ(ssStaticCopy, L"dynamic");
    sdv::fixed_wstring<8> ssStatic = L"static";
    sdv::wstring ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy = ssStatic;
    EXPECT_EQ(ssDynamicCopy, L"static");

    // Move assignment
    sdv::fixed_wstring<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove = std::move(ssDynamic);
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, L"dynamic");
    sdv::wstring ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove = std::move(ssStatic);
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, L"static");
}

TEST_F(CWStringTypeTest, AssignmentFunctionStatic)
{
    // Character assignment
    sdv::fixed_wstring<10> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");

    // String copy assignment
    sdv::fixed_wstring<10> ssSdvString2;
    sdv::fixed_wstring<15> ssSdvString3(L"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");

    // C++ string assignment
    sdv::fixed_wstring<10> ssSdvString4;
    std::wstring ss(L"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");

    // C string assignment
    sdv::fixed_wstring<10>  ssSdvString5;
    const wchar_t sz1[] = L"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    sdv::fixed_wstring<10>   ssSdvString6;
    const wchar_t* sz2 = L"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, L"Hello");

    // Substring assignment
    sdv::fixed_wstring<10> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    sdv::fixed_wstring<11> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, L"ell");
    sdv::fixed_wstring<12> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, L"ell");

    // Move assignment
    sdv::fixed_wstring<10> ssSdvString10;
    sdv::fixed_wstring<15> ssSdvString11(L"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, L"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::fixed_wstring<10> ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, L"Hello");

    // Initializer list
    sdv::fixed_wstring<10> ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({L'H', L'e', L'l', L'l', L'o'});
    EXPECT_EQ(ssSdvString13, L"Hello");
}

TEST_F(CWStringTypeTest, AssignmentFunctionDynamic)
{
    // Character assignment
    sdv::wstring ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");

    // String copy assignment
    sdv::wstring ssSdvString2;
    sdv::wstring ssSdvString3(L"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");

    // C++ string assignment
    sdv::wstring ssSdvString4;
    std::wstring ss(L"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");

    // C string assignment
    sdv::wstring ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    sdv::wstring ssSdvString6;
    const wchar_t* sz2 = L"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, L"Hello");

    // Substring assignment
    sdv::wstring ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    sdv::wstring ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, L"ell");
    sdv::wstring ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, L"ell");

    // Move assignment
    sdv::wstring ssSdvString10;
    sdv::wstring ssSdvString11(L"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, L"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::wstring ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, L"Hello");

    // Initializer list
    sdv::wstring ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({L'H', L'e', L'l', L'l', L'o'});
    EXPECT_EQ(ssSdvString13, L"Hello");
}

TEST_F(CWStringTypeTest, AssignmentFunctionMixed)
{
    // Copy assignment
    sdv::wstring ssDynamic = L"dynamic";
    sdv::fixed_wstring<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy.assign(ssDynamic);
    EXPECT_EQ(ssStaticCopy, L"dynamic");
    sdv::fixed_wstring<8> ssStatic = L"static";
    sdv::wstring ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy.assign(ssStatic);
    EXPECT_EQ(ssDynamicCopy, L"static");

    // Move assignment
    sdv::fixed_wstring<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove.assign(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, L"dynamic");
    sdv::wstring ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove.assign(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, L"static");

    // Substring assignment
    sdv::fixed_wstring<10> ssStaticSubstring;
    EXPECT_TRUE(ssStaticSubstring.empty());
    ssStaticSubstring.assign(ssDynamicMove, 1, 3);
    EXPECT_EQ(ssStaticSubstring, L"tat");
    sdv::wstring ssDynamicSubstring;
    EXPECT_TRUE(ssDynamicSubstring.empty());
    ssDynamicSubstring.assign(ssStaticMove, 1, 3);
    EXPECT_EQ(ssDynamicSubstring, L"yna");
}

TEST_F(CWStringTypeTest, PositionFunction)
{
    // Position in the string
    sdv::wstring ssSdvString1(L"Hello");
    EXPECT_EQ(ssSdvString1.at(0), L'H');
    EXPECT_EQ(ssSdvString1[0], L'H');
    EXPECT_EQ(ssSdvString1.front(), L'H');
    EXPECT_EQ(ssSdvString1.at(4), L'o');
    EXPECT_EQ(ssSdvString1[4], L'o');
    EXPECT_EQ(ssSdvString1.back(), L'o');
    EXPECT_THROW(ssSdvString1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString1[5], sdv::XIndexOutOfRange);

    // Empty string
    sdv::wstring ssSdvString2;
    EXPECT_THROW(ssSdvString2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.back(), sdv::XIndexOutOfRange);

    // Assignment
    ssSdvString1.at(0) = 'B';
    EXPECT_EQ(ssSdvString1, L"Bello");
    ssSdvString1[0] = 'D';
    EXPECT_EQ(ssSdvString1, L"Dello");
    ssSdvString1.front() = 'M';
    EXPECT_EQ(ssSdvString1, L"Mello");
    ssSdvString1.back() = 'k';
    EXPECT_EQ(ssSdvString1, L"Mellk");
}

TEST_F(CWStringTypeTest, CAndCppStringAccess)
{
    // C++ cast operator
    sdv::wstring ssSdvString(L"Hello");
    std::wstring ss1(ssSdvString);
    EXPECT_EQ(ss1, L"Hello");
    std::wstring ss2;
    EXPECT_TRUE(ss2.empty());
    ss2 = ssSdvString;
    EXPECT_EQ(ss2, L"Hello");

    // Data access
    sdv::wstring ssSdvString2(L"He\0lo", 5);
    EXPECT_EQ(ssSdvString2.size(), 5);
    const wchar_t* sz1 = ssSdvString2.data();
    EXPECT_EQ(memcmp(sz1, L"He\0lo", 5 * sizeof(wchar_t)), 0);

    // C string access
    const wchar_t* sz2 = ssSdvString.c_str();
    EXPECT_STREQ(sz2, L"Hello");
}

TEST_F(CWStringTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::wstring::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::wstring ssSdvString(L"Hello");
    EXPECT_NE(itEmpty, ssSdvString.begin());
    sdv::wstring::iterator itPos = ssSdvString.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.begin());
    sdv::wstring::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::wstring::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, L'H');
    EXPECT_EQ(itPos[0], L'H');
    EXPECT_EQ(itPos[4], L'o');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'B';
    EXPECT_EQ(ssSdvString, L"Bello");
    itPos[4] = 'k';
    EXPECT_EQ(ssSdvString, L"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, L'e');
    itPos += 3;
    EXPECT_EQ(*itPos, L'o');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.end());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, L'o');
    itPos -= 4;
    EXPECT_EQ(*itPos, L'H');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, L'H');
    EXPECT_EQ(itPos, ssSdvString.begin());

    // Const iterator
    sdv::wstring::const_iterator itPosConst = ssSdvString.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, L'e');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.cbegin(), ssSdvString.begin());
    EXPECT_EQ(ssSdvString.begin(), ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString.cend(), ssSdvString.end());
    EXPECT_EQ(ssSdvString.end(), ssSdvString.cend());
}

TEST_F(CWStringTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::wstring::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::wstring ssSdvString(L"Hello");
    EXPECT_NE(itEmpty, ssSdvString.rbegin());
    sdv::wstring::reverse_iterator itPos = ssSdvString.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.rbegin());
    sdv::wstring::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::wstring::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, L'o');
    EXPECT_EQ(itPos[0], L'o');
    EXPECT_EQ(itPos[4], L'H');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'k';
    EXPECT_EQ(ssSdvString, L"Hellk");
    itPos[4] = 'B';
    EXPECT_EQ(ssSdvString, L"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, L'l');
    itPos += 3;
    EXPECT_EQ(*itPos, L'H');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.rend());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, L'H');
    itPos -= 4;
    EXPECT_EQ(*itPos, L'o');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, L'o');
    EXPECT_EQ(itPos, ssSdvString.rbegin());

    // Const iterator
    sdv::wstring::const_reverse_iterator itPosConst = ssSdvString.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, L'l');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.crbegin(), ssSdvString.rbegin());
    EXPECT_EQ(ssSdvString.rbegin(), ssSdvString.crbegin());
    EXPECT_EQ(ssSdvString.crend(), ssSdvString.rend());
    EXPECT_EQ(ssSdvString.rend(), ssSdvString.crend());
}

TEST_F(CWStringTypeTest, StringCapacityStatic)
{
    // Empty string
    sdv::fixed_wstring<10> ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Filled string
    ssSdvString = L"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, L"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString += 'o';
    ssSdvString.resize(10, L'*');
    EXPECT_EQ(ssSdvString, L"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, L"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Clear
    ssSdvString.clear();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 11);
}

TEST_F(CWStringTypeTest, StringCapacityDynamic)
{
    // Empty string
    sdv::wstring ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 0);

    // Filled string
    ssSdvString = L"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, L"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 5);
    ssSdvString += 'o';
    ssSdvString.resize(10, L'*');
    EXPECT_EQ(ssSdvString, L"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, L"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), L"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Clear
    ssSdvString.clear();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 0);
}

TEST_F(CWStringTypeTest, InsertFunctionStatic)
{
    // Character assignment
    sdv::fixed_wstring<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");
    ssSdvString1.insert(0, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********??");
    ssSdvString1.insert(std::wstring::npos, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********????");
    ssSdvString1.insert(4, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**??********????");

    // C string assignment
    sdv::fixed_wstring<20>   ssSdvString5;
    const wchar_t  sz1[] = L"Hello";
    const wchar_t* sz2	 = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::wstring::npos, sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, L"**Hello");
    ssSdvString5.insert(std::wstring::npos, sz2);
    EXPECT_EQ(ssSdvString5, L"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, L"**He**llo**");

    // String copy assignment
    sdv::fixed_wstring<20> ssSdvString2;
    sdv::fixed_wstring<20> ssSdvString3(L"Hello");
    sdv::fixed_wstring<20> ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**Hello");
    ssSdvString2.insert(std::wstring::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**He**llo**");

    // C++ string assignment
    sdv::fixed_wstring<20> ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, L"**Hello");
    ssSdvString4.insert(std::wstring::npos, ss2);
    EXPECT_EQ(ssSdvString4, L"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, L"**He**llo**");

    // Substring assignment
    sdv::fixed_wstring<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, L"eellll");
    ssSdvString7.insert(std::wstring::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, L"eellllell");

    // Iterator assignment
    sdv::fixed_wstring<20> ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), L'+');
    EXPECT_EQ(ssSdvString12, L"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), L'-');
    EXPECT_EQ(ssSdvString12, L"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), L'+');
    EXPECT_EQ(ssSdvString12, L"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), L'-');
    EXPECT_EQ(ssSdvString12, L"-+Hello+-");
    sdv::fixed_wstring<20>::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, L'#');
    EXPECT_EQ(ssSdvString12, L"-#+Hello+-");
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, L'#');
    EXPECT_EQ(ssSdvString12, L"-##-++Hello++--");
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, L'#');
    EXPECT_EQ(ssSdvString12, L"-##-++Hello++--");
    ssSdvString12			  = L"Hello";
    sdv::fixed_wstring<20> ssSdvString8 = L"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"112212Hello1212");

    // Initializer list
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"112212Hello1212");
}

TEST_F(CWStringTypeTest, InsertFunctionDynamic)
{
    // Character assignment
    sdv::wstring ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");
    ssSdvString1.insert(0, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********??");
    ssSdvString1.insert(std::wstring::npos, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**********????");
    ssSdvString1.insert(4, 2, L'?');
    EXPECT_EQ(ssSdvString1, L"??**??********????");

    // C string assignment
    sdv::wstring ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    const wchar_t* sz2	  = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::wstring::npos, sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, L"**Hello");
    ssSdvString5.insert(std::wstring::npos, sz2);
    EXPECT_EQ(ssSdvString5, L"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, L"**He**llo**");

    // String copy assignment
    sdv::wstring ssSdvString2;
    sdv::wstring ssSdvString3(L"Hello");
    sdv::wstring ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**Hello");
    ssSdvString2.insert(std::wstring::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"**He**llo**");

    // C++ string assignment
    sdv::wstring ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, L"**Hello");
    ssSdvString4.insert(std::wstring::npos, ss2);
    EXPECT_EQ(ssSdvString4, L"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, L"**He**llo**");

    // Substring assignment
    sdv::wstring ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, L"eellll");
    ssSdvString7.insert(std::wstring::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, L"eellllell");

    // Iterator assignment
    sdv::wstring ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), L'+');
    EXPECT_EQ(ssSdvString12, L"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), L'-');
    EXPECT_EQ(ssSdvString12, L"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), L'+');
    EXPECT_EQ(ssSdvString12, L"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), L'-');
    EXPECT_EQ(ssSdvString12, L"-+Hello+-");
    sdv::wstring::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, L'#');
    EXPECT_EQ(ssSdvString12, L"-#+Hello+-");
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, L'#');
    EXPECT_EQ(ssSdvString12, L"-##-++Hello++--");
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, L'+');
    EXPECT_EQ(ssSdvString12, L"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, L'-');
    EXPECT_EQ(ssSdvString12, L"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, L'#');
    EXPECT_EQ(ssSdvString12, L"-##-++Hello++--");
    ssSdvString12			 = L"Hello";
    sdv::wstring ssSdvString8 = L"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, L"112212Hello1212");

    // Initializer list
    ssSdvString12 = L"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"112212Hello1212");
}

TEST_F(CWStringTypeTest, InsertFunctionMixed)
{
    // String copy assignment - static into dynamic
    sdv::wstring ssDynamic;
    sdv::fixed_wstring<20> ssStatic1(L"Hello");
    sdv::fixed_wstring<20> ssStatic2(L"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(3, ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, L"Hello");
    ssDynamic.insert(0, ssStatic2);
    EXPECT_EQ(ssDynamic, L"**Hello");
    ssDynamic.insert(std::wstring::npos, ssStatic2);
    EXPECT_EQ(ssDynamic, L"**Hello**");
    ssDynamic.insert(4, ssStatic2);
    EXPECT_EQ(ssDynamic, L"**He**llo**");

    // String copy assignment - dynamic into static
    sdv::fixed_wstring<20> ssStatic;
    sdv::fixed_wstring<20> ssDynamic1(L"Hello");
    sdv::fixed_wstring<20> ssDynamic2(L"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(3, ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, L"Hello");
    ssStatic.insert(0, ssDynamic2);
    EXPECT_EQ(ssStatic, L"**Hello");
    ssStatic.insert(std::wstring::npos, ssDynamic2);
    EXPECT_EQ(ssStatic, L"**Hello**");
    ssStatic.insert(4, ssDynamic2);
    EXPECT_EQ(ssStatic, L"**He**llo**");

    // Substring assignment - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(0, ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, L"ell");

    // Substring assignment - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(0, ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, L"ell");
}

TEST_F(CWStringTypeTest, EraseFunction)
{
    // Erase index based
    sdv::wstring ssSdvString = L"12345678901234567890";
    ssSdvString.erase(8, 5);
    EXPECT_EQ(ssSdvString, L"123456784567890");
    ssSdvString.erase(0, 2);
    EXPECT_EQ(ssSdvString, L"3456784567890");
    ssSdvString.erase(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString, L"3456784567890");
    ssSdvString.erase(sdv::wstring::npos);
    EXPECT_EQ(ssSdvString, L"3456784567890");
    ssSdvString.erase(8);
    EXPECT_EQ(ssSdvString, L"34567845");

    // Erase iterator
    ssSdvString = L"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString, L"2345678901234567890");
    ssSdvString.erase(ssSdvString.cend());
    EXPECT_EQ(ssSdvString, L"2345678901234567890");
    ssSdvString.erase(ssSdvString.cbegin() + 5);
    EXPECT_EQ(ssSdvString, L"234568901234567890");
    sdv::wstring::iterator itPos = ssSdvString.erase(ssSdvString.end() - 2);
    EXPECT_EQ(ssSdvString, L"23456890123456780");
    EXPECT_EQ(*itPos, L'0');
    ++itPos;
    EXPECT_EQ(itPos, ssSdvString.cend());

    // Erase iterator range
    ssSdvString = L"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin(), ssSdvString.cbegin() + 4);
    EXPECT_EQ(ssSdvString, L"5678901234567890");
    itPos = ssSdvString.erase(ssSdvString.cbegin() + 4, ssSdvString.cbegin() + 6);
    EXPECT_EQ(ssSdvString, L"56781234567890");
    EXPECT_EQ(*itPos, L'1');
    itPos += 10;
    EXPECT_EQ(itPos, ssSdvString.cend());
}

TEST_F(CWStringTypeTest, PushPopFunctions)
{
    sdv::wstring ssSdvString;
    ssSdvString.push_back(L'1');
    EXPECT_EQ(ssSdvString, L"1");
    ssSdvString.push_back(L'2');
    EXPECT_EQ(ssSdvString, L"12");
    ssSdvString.pop_back();
    EXPECT_EQ(ssSdvString, L"1");
    ssSdvString.pop_back();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_NO_THROW(ssSdvString.pop_back());
}

TEST_F(CWStringTypeTest, AppendFunctionStatic)
{
    // Append characters
    sdv::fixed_wstring<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");
    ssSdvString1.append(2, L'?');
    EXPECT_EQ(ssSdvString1, L"**********??");

    // Append string
    sdv::fixed_wstring<20> ssSdvString2;
    sdv::fixed_wstring<10> ssSdvString3(L"Hello");
    sdv::fixed_wstring<15> ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"Hello**");

    // Append C++ string
    sdv::fixed_wstring<20> ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, L"Hello**");

    // Append substring
    sdv::fixed_wstring<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ellell");

    // C string assignment
    sdv::fixed_wstring<20> ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    const wchar_t* sz2	  = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, L"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, L"Hello**Hello");

    // Iterator based
    sdv::fixed_wstring<20> ssSdvString12 = L"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, L"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = L"Hello";
    ssSdvString12.append({L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"Hello12");
}

TEST_F(CWStringTypeTest, AppendFunctionDynamic)
{
    // Append characters
    sdv::wstring ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, L'*');
    EXPECT_EQ(ssSdvString1, L"**********");
    ssSdvString1.append(2, L'?');
    EXPECT_EQ(ssSdvString1, L"**********??");

    // Append string
    sdv::wstring ssSdvString2;
    sdv::wstring ssSdvString3(L"Hello");
    sdv::wstring ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, L"Hello**");

    // Append C++ string
    sdv::wstring ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, L"Hello**");

    // Append substring
    sdv::wstring ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, L"ellell");

    // C string assignment
    sdv::wstring ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    const wchar_t* sz2	  = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, L"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, L"Hello**Hello");

    // Iterator based
    sdv::wstring ssSdvString12 = L"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, L"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = L"Hello";
    ssSdvString12.append({L'1', L'2'});
    EXPECT_EQ(ssSdvString12, L"Hello12");
}

TEST_F(CWStringTypeTest, AppendFunctionMixed)
{
    // Append string - static into dynamic
    sdv::wstring ssDynamic;
    sdv::fixed_wstring<10> ssStatic1(L"Hello");
    sdv::fixed_wstring<15> ssStatic2(L"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, L"Hello");
    ssDynamic.append(ssStatic2);
    EXPECT_EQ(ssDynamic, L"Hello**");

    // Append string - dynamic into static
    sdv::fixed_wstring<20> ssStatic;
    sdv::wstring ssDynamic1(L"Hello");
    sdv::wstring ssDynamic2(L"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, L"Hello");
    ssStatic.append(ssDynamic2);
    EXPECT_EQ(ssStatic, L"Hello**");

    // Append substring - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, L"ell");

    // Append substring - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, L"ell");
}

TEST_F(CWStringTypeTest, AppendOperatorStatic)
{
    // Append string
    sdv::fixed_wstring<20> ssSdvString2;
    sdv::fixed_wstring<15> ssSdvString3(L"Hello");
    sdv::fixed_wstring<10> ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, L"Hello**");

    // Append C++ string
    sdv::fixed_wstring<20> ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, L"Hello**");

    // Append character
    sdv::fixed_wstring<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, L"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, L"*?");

    // C string assignment
    sdv::fixed_wstring<20> ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    const wchar_t* sz2	  = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, L"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, L"Hello**Hello");

    // Initializer list
    sdv::fixed_wstring<20> ssSdvString12 = L"Hello";
    ssSdvString12 += {L'1', L'2'};
    EXPECT_EQ(ssSdvString12, L"Hello12");
}

TEST_F(CWStringTypeTest, AppendOperatorDynamic)
{
    // Append string
    sdv::wstring ssSdvString2;
    sdv::wstring ssSdvString3(L"Hello");
    sdv::wstring ssSdvString6(L"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, L"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, L"Hello**");

    // Append C++ string
    sdv::wstring ssSdvString4;
    std::wstring ss(L"Hello");
    std::wstring ss2(L"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, L"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, L"Hello**");

    // Append character
    sdv::wstring ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, L"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, L"*?");

    // C string assignment
    sdv::wstring ssSdvString5;
    const wchar_t	sz1[] = L"Hello";
    const wchar_t* sz2	  = L"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, L"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, L"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, L"Hello**Hello");

    // Initializer list
    sdv::wstring ssSdvString12 = L"Hello";
    ssSdvString12 += {L'1', L'2'};
    EXPECT_EQ(ssSdvString12, L"Hello12");
}

TEST_F(CWStringTypeTest, AppendOperatorMixed)
{
    // Append string - static into dynamic
    sdv::wstring ssDynamic;
    sdv::fixed_wstring<15> ssStatic1(L"Hello");
    sdv::fixed_wstring<10> ssStatic2(L"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic += ssStatic1;
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, L"Hello");
    ssDynamic += ssStatic2;
    EXPECT_EQ(ssDynamic, L"Hello**");

    // Append string - dynamic into static
    sdv::fixed_wstring<20> ssStatic;
    sdv::wstring ssDynamic1(L"Hello");
    sdv::wstring ssDynamic2(L"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic += ssDynamic1;
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, L"Hello");
    ssStatic += ssStatic2;
    EXPECT_EQ(ssStatic, L"Hello**");
}

TEST_F(CWStringTypeTest, CompareFunctionStatic)
{
    // Compare strings
    sdv::fixed_wstring<20> ssSdvString1;
    sdv::fixed_wstring<10> ssSdvString2(L"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::wstring ssString(L"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const wchar_t* sz1 = L"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const wchar_t sz2[] = L"Hello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = L"Kello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssSdvString2, 1, sdv::fixed_wstring<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = L"Kello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString, 1, sdv::fixed_wstring<20>::npos), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, ssString, 1, sdv::fixed_wstring<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = L"Kellogg";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const wchar_t sz3[] = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz3), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz3), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz3), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz3), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const wchar_t sz4[] = L"Kellogg";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_wstring<20>::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CWStringTypeTest, CompareFunctionDynamic)
{
    // Compare strings
    sdv::wstring ssSdvString1;
    sdv::wstring ssSdvString2(L"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::wstring ssString(L"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const wchar_t* sz1 = L"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const wchar_t sz2[] = L"Hello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = L"Kello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssSdvString2, 1, sdv::wstring::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssString), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssString), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, ssString), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssString), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = L"Kello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssString, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, ssString, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, ssString, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssString, 1, sdv::wstring::npos), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, ssString, 1, sdv::wstring::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz1), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz1), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, sz1), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz1), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = L"Kellogg";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz1 + 1, 4), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const wchar_t sz3[] = L"ello";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz3), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz3), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, sz3), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz3), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const wchar_t sz4[] = L"Kellogg";
    ssSdvString1 = L"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::wstring::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::wstring::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz4 + 1, 4), 0);
    ssSdvString1 = L"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::wstring::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CWStringTypeTest, CompareFunctionMixed)
{
    // Compare strings - compare dynamic with static
    sdv::wstring ssDynamic;
    sdv::fixed_wstring<10> ssStatic1(L"Hello");
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = L"Helln";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = L"HellnO";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = L"Hello";
    EXPECT_EQ(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = L"Hellp";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = L"Helloa";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);

    // Compare substring
    ssStatic1 = L"ello";
    ssDynamic = L"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1), 0);
    ssDynamic = L"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1), 0);
    ssDynamic = L"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1), 0);
    ssDynamic = L"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1), 0);
    ssDynamic = L"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1), 0);
    ssStatic1 = L"Kello";
    ssDynamic = L"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssDynamic = L"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssDynamic = L"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssDynamic = L"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssDynamic = L"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_wstring<20>::npos, ssStatic1, 1, sdv::fixed_wstring<20>::npos), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1, 1, 4), 0);

    // Compare strings - compare static with dynamic
    sdv::fixed_wstring<10> ssStatic;
    sdv::wstring ssDynamic1(L"Hello");
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = L"Helln";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = L"HellnO";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = L"Hello";
    EXPECT_EQ(ssStatic.compare(ssDynamic1), 0);
    ssStatic = L"Hellp";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = L"Helloa";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);

    // Compare substring
    ssDynamic1 = L"ello";
    ssStatic = L"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1), 0);
    ssStatic = L"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1), 0);
    ssStatic = L"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1), 0);
    ssStatic = L"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1), 0);
    ssStatic = L"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1), 0);
    ssDynamic1 = L"Kello";
    ssStatic = L"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssStatic = L"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssStatic = L"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssStatic = L"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1, 1, sdv::fixed_wstring<20>::npos), 0);
    ssStatic = L"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_wstring<20>::npos, ssDynamic1, 1, sdv::fixed_wstring<20>::npos), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1, 1, 4), 0);
}

TEST_F(CWStringTypeTest, CompareOperatorStatic)
{
    // Compare strings
    sdv::fixed_wstring<20> ssSdvString1;
    sdv::fixed_wstring<10> ssSdvString2(L"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::wstring ssString(L"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const wchar_t* sz1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const wchar_t sz2[] = L"Hello";
    ssSdvString1	 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CWStringTypeTest, CompareOperatorDynamic)
{
    // Compare strings
    sdv::wstring ssSdvString1;
    sdv::wstring ssSdvString2(L"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::wstring ssString(L"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const wchar_t* sz1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const wchar_t sz2[] = L"Hello";
    ssSdvString1	 = L"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = L"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = L"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CWStringTypeTest, CompareOperatorMixed)
{
    // Compare strings - compare dynamic with static
    sdv::wstring ssDynamic;
    sdv::fixed_wstring<10> ssStatic1(L"Hello");
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = L"Helln";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = L"HellnO";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = L"Hello";
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic == ssStatic1);
    ssDynamic = L"Hellp";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = L"Helloa";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);

    // Compare strings - compare static with dynamic
    sdv::fixed_wstring<10> ssStatic;
    sdv::wstring ssDynamic1(L"Hello");
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = L"Helln";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = L"HellnO";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = L"Hello";
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic == ssDynamic1);
    ssStatic = L"Hellp";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = L"Helloa";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
}

TEST_F(CWStringTypeTest, ReplaceFunctionStatic)
{
    // Replace string
    sdv::fixed_wstring<20> ssSdvString = L"1234567890";
    sdv::fixed_wstring<10> ssSdvString2 = L"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString	 = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace C++ string
    ssSdvString	 = L"1234567890";
    std::wstring ss = L"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ss);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace C string
    ssSdvString	 = L"1234567890";
    const wchar_t sz1[] = L"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz1);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString		 = L"1234567890";
    const wchar_t* sz2 = L"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz2);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace substring
    ssSdvString	 = L"1234567890";
    ssSdvString2 = L"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = L"1234567890";
    ss = L"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = L"1234567890";
    const wchar_t sz3[] = L"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    sz2 = L"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace with characters
    ssSdvString = L"1234567890";
    ssSdvString.replace(1, 2, 4, L'+');
    EXPECT_EQ(ssSdvString, L"1++++4567890");
    ssSdvString.replace(2, 6, 4, L'-');
    EXPECT_EQ(ssSdvString, L"1+----7890");
    ssSdvString.replace(3, 4, 4, L'*');
    EXPECT_EQ(ssSdvString, L"1+-****890");
    ssSdvString.replace(0, 0, 4, L'#');
    EXPECT_EQ(ssSdvString, L"####1+-****890");
    ssSdvString.replace(std::wstring::npos, 100, 4, L'~');
    EXPECT_EQ(ssSdvString, L"####1+-****890~~~~");
    ssSdvString.replace(0, std::wstring::npos, 4, L'=');
    EXPECT_EQ(ssSdvString, L"====");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, L'+');
    EXPECT_EQ(ssSdvString, L"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, L'-');
    EXPECT_EQ(ssSdvString, L"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, L'*');
    EXPECT_EQ(ssSdvString, L"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, L'#');
    EXPECT_EQ(ssSdvString, L"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, L'~');
    EXPECT_EQ(ssSdvString, L"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, L'=');
    EXPECT_EQ(ssSdvString, L"====");

    // Replace with iterators
    ssSdvString = L"1234567890";
    ss = L"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace with initialization list
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd");
}

TEST_F(CWStringTypeTest, ReplaceFunctionDynamic)
{
    // Replace string
    sdv::wstring ssSdvString = L"1234567890";
    sdv::wstring ssSdvString2 = L"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString	 = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace C++ string
    ssSdvString	 = L"1234567890";
    std::wstring ss = L"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ss);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace C string
    ssSdvString	 = L"1234567890";
    const wchar_t sz1[] = L"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz1);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString		 = L"1234567890";
    const wchar_t* sz2 = L"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz2);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace substring
    ssSdvString	 = L"1234567890";
    ssSdvString2 = L"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = L"1234567890";
    ss = L"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = L"1234567890";
    const wchar_t sz3[] = L"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    sz2 = L"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(std::wstring::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::wstring::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace with characters
    ssSdvString = L"1234567890";
    ssSdvString.replace(1, 2, 4, L'+');
    EXPECT_EQ(ssSdvString, L"1++++4567890");
    ssSdvString.replace(2, 6, 4, L'-');
    EXPECT_EQ(ssSdvString, L"1+----7890");
    ssSdvString.replace(3, 4, 4, L'*');
    EXPECT_EQ(ssSdvString, L"1+-****890");
    ssSdvString.replace(0, 0, 4, L'#');
    EXPECT_EQ(ssSdvString, L"####1+-****890");
    ssSdvString.replace(std::wstring::npos, 100, 4, L'~');
    EXPECT_EQ(ssSdvString, L"####1+-****890~~~~");
    ssSdvString.replace(0, std::wstring::npos, 4, L'=');
    EXPECT_EQ(ssSdvString, L"====");
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, L'+');
    EXPECT_EQ(ssSdvString, L"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, L'-');
    EXPECT_EQ(ssSdvString, L"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, L'*');
    EXPECT_EQ(ssSdvString, L"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, L'#');
    EXPECT_EQ(ssSdvString, L"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, L'~');
    EXPECT_EQ(ssSdvString, L"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, L'=');
    EXPECT_EQ(ssSdvString, L"====");

    // Replace with iterators
    ssSdvString = L"1234567890";
    ss = L"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, L"abcd");

    // Replace with initialization list
    ssSdvString = L"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {L'a', L'b', L'c', L'd'});
    EXPECT_EQ(ssSdvString, L"abcd");
}

TEST_F(CWStringTypeTest, ReplaceFunctionMixed)
{
    // Replace string - replace dynamic by static
    sdv::wstring ssDynamic = L"1234567890";
    sdv::fixed_wstring<10> ssStatic1 = L"abcd";
    ssDynamic.replace(1, 2, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890");
    ssDynamic.replace(std::wstring::npos, 100, ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::wstring::npos, ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd");
    ssDynamic = L"1234567890";
    ssDynamic.replace(ssDynamic.begin() + 1, ssDynamic.begin() + 3, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1abcd4567890");
    ssDynamic.replace(ssDynamic.begin() + 2, ssDynamic.begin() + 8, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1aabcd7890");
    ssDynamic.replace(ssDynamic.begin() + 3, ssDynamic.begin() + 7, ssStatic1);
    EXPECT_EQ(ssDynamic, L"1aaabcd890");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.begin(), ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890");
    ssDynamic.replace(ssDynamic.end(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890abcd");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, L"abcd");

    // Replace substring
    ssDynamic = L"1234567890";
    ssStatic1 = L"xxabcdxx";
    ssDynamic.replace(1, 2, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, L"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, L"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, L"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890");
    ssDynamic.replace(std::wstring::npos, 100, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, L"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::wstring::npos, ssStatic1, 2, 4);

    // Replace string - replace static by dynamic
    sdv::fixed_wstring<20> ssStatic = L"1234567890";
    sdv::wstring ssDynamic1 = L"abcd";
    ssStatic.replace(1, 2, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890");
    ssStatic.replace(std::wstring::npos, 100, ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::wstring::npos, ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd");
    ssStatic = L"1234567890";
    ssStatic.replace(ssStatic.begin() + 1, ssStatic.begin() + 3, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1abcd4567890");
    ssStatic.replace(ssStatic.begin() + 2, ssStatic.begin() + 8, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1aabcd7890");
    ssStatic.replace(ssStatic.begin() + 3, ssStatic.begin() + 7, ssDynamic1);
    EXPECT_EQ(ssStatic, L"1aaabcd890");
    ssStatic.replace(ssStatic.begin(), ssStatic.begin(), ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890");
    ssStatic.replace(ssStatic.end(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890abcd");
    ssStatic.replace(ssStatic.begin(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, L"abcd");

    // Replace substring
    ssStatic = L"1234567890";
    ssDynamic1 = L"xxabcdxx";
    ssStatic.replace(1, 2, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, L"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, L"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, L"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890");
    ssStatic.replace(std::wstring::npos, 100, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, L"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::wstring::npos, ssDynamic1, 2, 4);
}

TEST_F(CWStringTypeTest, SubstringFunctionStatic)
{
    sdv::fixed_wstring<20> ssSdvString = L"1234567890";
    EXPECT_EQ(ssSdvString.substr(), L"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), L"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), L"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CWStringTypeTest, SubstringFunctionDynamic)
{
    sdv::wstring ssSdvString = L"1234567890";
    EXPECT_EQ(ssSdvString.substr(), L"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), L"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), L"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CWStringTypeTest, CopyFunction)
{
    sdv::wstring ssSdvString = L"1234567890";
    wchar_t sz[32] = {};
    EXPECT_EQ(ssSdvString.copy(sz, 2, 5), 2);
    EXPECT_STREQ(sz, L"67");
    std::fill_n(sz, 32, static_cast<wchar_t>(L'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 20, 5), 5);
    EXPECT_STREQ(sz, L"67890");
    std::fill_n(sz, 32, static_cast<wchar_t>(L'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0, 5), 0);
    EXPECT_STREQ(sz, L"");
    std::fill_n(sz, 32, static_cast<wchar_t>(L'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 2), 2);
    EXPECT_STREQ(sz, L"12");
    std::fill_n(sz, 32, static_cast<wchar_t>(L'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0), 0);
    EXPECT_STREQ(sz, L"");
    std::fill_n(sz, 32, static_cast<wchar_t>(L'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, std::wstring::npos), 10);
    EXPECT_STREQ(sz, L"1234567890");
}

TEST_F(CWStringTypeTest, SwapFunctionStatic)
{
    sdv::fixed_wstring<20> ssSdvString1 = L"12345";
    sdv::fixed_wstring<10> ssSdvString2 = L"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"67890");
    EXPECT_EQ(ssSdvString2, L"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"12345");
    EXPECT_EQ(ssSdvString2, L"67890");
}

TEST_F(CWStringTypeTest, SwapFunctionDynamic)
{
    sdv::wstring ssSdvString1 = L"12345";
    sdv::wstring ssSdvString2 = L"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"67890");
    EXPECT_EQ(ssSdvString2, L"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"12345");
    EXPECT_EQ(ssSdvString2, L"67890");
}

TEST_F(CWStringTypeTest, SwapFunctionMixed)
{
    sdv::fixed_wstring<20> ssSdvString1 = L"12345";
    sdv::wstring ssSdvString2 = L"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"67890");
    EXPECT_EQ(ssSdvString2, L"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, L"12345");
    EXPECT_EQ(ssSdvString2, L"67890");
}

TEST_F(CWStringTypeTest, FindFunctionStatic)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"aabbaaccbbcc";
    sdv::fixed_wstring<10> ssSdvString2 = L"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::fixed_wstring<20>::npos);

    // Find C++ string in string
    std::wstring ss = L"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::fixed_wstring<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 3, 4), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 9), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(L'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(L'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(L'\0', 11), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, FindFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"aabbaaccbbcc";
    sdv::wstring ssSdvString2 = L"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::wstring::npos);

    // Find C++ string in string
    std::wstring ss = L"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::wstring::npos);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 3, 4), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(L"bb\0a", 9), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(L'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(L'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(L'\0', 11), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, FindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::wstring ssDynamic = L"aabbaaccbbcc";
    sdv::fixed_wstring<10> ssStatic1 = L"aa";
    EXPECT_EQ(ssDynamic.find(ssStatic1), 0);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 1), 4);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 5), sdv::fixed_string<20>::npos);

    // Find string in string - find static in dynamic
    sdv::fixed_wstring<20> ssStatic = L"aabbaaccbbcc";
    sdv::wstring ssDynamic1 = L"aa";
    EXPECT_EQ(ssStatic.find(ssDynamic1), 0);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 1), 4);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 5), sdv::fixed_string<20>::npos);
}

TEST_F(CWStringTypeTest, ReverseFindFunctionStatic)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"aabbaaccbbcc";
    sdv::fixed_wstring<10> ssSdvString2 = L"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::wstring ss = L"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", sdv::fixed_wstring<20>::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 1, 4), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", sdv::fixed_wstring<20>::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 1), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(L'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(L'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(L'\0', 3), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, ReverseFindFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"aabbaaccbbcc";
    sdv::wstring ssSdvString2 = L"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::wstring ss = L"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", sdv::wstring::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 1, 4), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", sdv::wstring::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(L"bb\0a", 1), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(L'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(L'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(L'\0', 3), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, ReverseFindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::wstring ssDynamic = L"aabbaaccbbcc";
    sdv::fixed_wstring<10> ssStatic1 = L"aa";
    EXPECT_EQ(ssDynamic.rfind(ssStatic1), 4);
    EXPECT_EQ(ssDynamic.rfind(ssStatic1, 3), 0);

    // Find string in string - find static in dynamic
    sdv::fixed_wstring<20> ssStatic = L"aabbaaccbbcc";
    sdv::wstring ssDynamic1 = L"aa";
    EXPECT_EQ(ssStatic.rfind(ssDynamic1), 4);
    EXPECT_EQ(ssStatic.rfind(ssDynamic1, 3), 0);
}

TEST_F(CWStringTypeTest, FindFirstOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"12341234";
    sdv::fixed_wstring<10> ssSdvString2 = L"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::fixed_wstring<20>::npos);

    // Find C++ string in string
    std::wstring ss = L"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::fixed_wstring<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 10, 2), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 8), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 11), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, FindFirstOfFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"12341234";
    sdv::wstring ssSdvString2 = L"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::wstring::npos);

    // Find C++ string in string
    std::wstring ss = L"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::wstring::npos);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 10, 2), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(L"c\0", 8), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(L'\0', 11), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, FindFirstOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::wstring ssDynamic = L"12341234";
    sdv::fixed_wstring<10> ssStatic1 = L"34";
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in sttaic
    sdv::fixed_wstring<10> ssStatic = L"12341234";
    sdv::wstring ssDynamic1 = L"34";
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CWStringTypeTest, FindFirstNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"12341234";
    sdv::fixed_wstring<10> ssSdvString2 = L"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::fixed_wstring<20>::npos);

    // Find C++ string in string
    std::wstring ss = L"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::fixed_wstring<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 9, 3), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 10), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 12), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, FindFirstNotOfFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"12341234";
    sdv::wstring ssSdvString2 = L"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::wstring::npos);

    // Find C++ string in string
    std::wstring ss = L"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::wstring::npos);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 9, 3), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L"ab\0", 10), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(L'\0', 12), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, FindFirstNotOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::wstring ssDynamic = L"12341234";
    sdv::fixed_wstring<10> ssStatic1 = L"12";
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_wstring<10> ssStatic = L"12341234";
    sdv::wstring ssDynamic1 = L"12";
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CWStringTypeTest, FindLastOfFunction)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"12341234";
    sdv::fixed_wstring<10> ssSdvString2 = L"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::fixed_wstring<20>::npos);

    // Find C++ string in string
    std::wstring ss = L"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::fixed_wstring<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", std::wstring::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 2, 2), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 2), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0', 4), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, FindLastOfFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"12341234";
    sdv::wstring ssSdvString2 = L"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::wstring::npos);

    // Find C++ string in string
    std::wstring ss = L"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::wstring::npos);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", std::wstring::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 2, 2), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(L"c\0", 2), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(L'\0', 4), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, FindLastOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::wstring ssDynamic = L"12341234";
    sdv::fixed_wstring<10> ssStatic1 = L"34";
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_wstring<10> ssStatic = L"12341234";
    sdv::wstring ssDynamic1 = L"34";
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CWStringTypeTest, FindLastNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_wstring<20> ssSdvString1 = L"12341234";
    sdv::fixed_wstring<10> ssSdvString2 = L"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::fixed_wstring<20>::npos);

    // Find C++ string in string
    std::wstring ss = L"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::fixed_wstring<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_wstring<20>(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", std::wstring::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 2, 3), sdv::fixed_wstring<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 2), sdv::fixed_wstring<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 0), sdv::fixed_wstring<20>::npos);
}

TEST_F(CWStringTypeTest, FindLastNotOfFunctionDynamic)
{
    // Find string in string
    sdv::wstring ssSdvString1 = L"12341234";
    sdv::wstring ssSdvString2 = L"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::wstring::npos);

    // Find C++ string in string
    std::wstring ss = L"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::wstring::npos);

    // Find C string in string
    ssSdvString1 = sdv::wstring(L"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", std::wstring::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 2, 3), sdv::wstring::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L"ab\0", 2), sdv::wstring::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(L'\0', 0), sdv::wstring::npos);
}

TEST_F(CWStringTypeTest, FindLastNotOfFunctionMixed)
{
    // Find string in string - find static in dyanmic
    sdv::wstring ssDynamic = L"12341234";
    sdv::fixed_wstring<10> ssStatic1 = L"12";
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_wstring<10> ssStatic = L"12341234";
    sdv::wstring ssDynamic1 = L"12";
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CWStringTypeTest, SumOperatorStatic)
{
    sdv::fixed_wstring<20> ssSdvString = L"1234";
    std::wstring ss = L"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, L"12341234");
    EXPECT_EQ(ss + ssSdvString, L"56781234");
    EXPECT_EQ(ssSdvString + ss, L"12345678");
    EXPECT_EQ(L"abcd" + ssSdvString, L"abcd1234");
    EXPECT_EQ(ssSdvString + L"abcd", L"1234abcd");
    EXPECT_EQ(L'a' + ssSdvString, L"a1234");
    EXPECT_EQ(ssSdvString + L'a', L"1234a");
    sdv::fixed_wstring<20> ssSdvString1 = L"1234";
    sdv::fixed_wstring<10> ssSdvString2 = L"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    ssSdvString2 = L"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + L"abcd", L"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + L'a', L"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), L"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), L"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(L"abcd" + std::move(ssSdvString1), L"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(L'a' + std::move(ssSdvString1), L"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CWStringTypeTest, SumOperatorDynamic)
{
    sdv::wstring ssSdvString = L"1234";
    std::wstring ss = L"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, L"12341234");
    EXPECT_EQ(ss + ssSdvString, L"56781234");
    EXPECT_EQ(ssSdvString + ss, L"12345678");
    EXPECT_EQ(L"abcd" + ssSdvString, L"abcd1234");
    EXPECT_EQ(ssSdvString + L"abcd", L"1234abcd");
    EXPECT_EQ(L'a' + ssSdvString, L"a1234");
    EXPECT_EQ(ssSdvString + L'a', L"1234a");
    sdv::wstring ssSdvString1 = L"1234", ssSdvString2 = L"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    ssSdvString2 = L"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, L"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + L"abcd", L"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(std::move(ssSdvString1) + L'a', L"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), L"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), L"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(L"abcd" + std::move(ssSdvString1), L"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = L"1234";
    EXPECT_EQ(L'a' + std::move(ssSdvString1), L"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CWStringTypeTest, SumOperatorMixed)
{
    // Add strings - add static onto dynamic
    sdv::wstring ssDynamic = L"1234";
    sdv::fixed_wstring<20> ssStatic1 = L"5678";
    EXPECT_EQ(ssDynamic + ssStatic1, L"12345678");
    EXPECT_EQ(std::move(ssDynamic) + std::move(ssStatic1), L"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_TRUE(ssStatic1.empty());
    ssDynamic = L"1234";
    ssStatic1 = L"5678";
    EXPECT_EQ(std::move(ssDynamic) + ssStatic1, L"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());
    ssDynamic = L"1234";
    EXPECT_EQ(ssStatic1 + std::move(ssDynamic), L"56781234");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());

    // Add strings - add dynamic onto static
    sdv::fixed_wstring<20> ssStatic = L"1234";
    sdv::wstring ssDynamic1 = L"5678";
    EXPECT_EQ(ssStatic + ssDynamic1, L"12345678");
    EXPECT_EQ(std::move(ssStatic) + std::move(ssDynamic1), L"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_TRUE(ssDynamic1.empty());
    ssStatic = L"1234";
    ssDynamic1 = L"5678";
    EXPECT_EQ(std::move(ssStatic) + ssDynamic1, L"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
    ssStatic = L"1234";
    EXPECT_EQ(ssDynamic1 + std::move(ssStatic), L"56781234");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
}

TEST_F(CWStringTypeTest, StreamString)
{
    std::wstringstream sstream;
    sdv::wstring ss = L"1234";
    sstream << ss;
    EXPECT_EQ(sstream.str(), L"1234");
    ss.clear();
    EXPECT_TRUE(ss.empty());
    sstream >> ss;
    EXPECT_EQ(ss, L"1234");
}

TEST_F(CWStringTypeTest, StringGetLine)
{
    std::wistringstream sstream;
    sstream.str(L"0;1;2;3");
    int nIndex = 0;
    for (sdv::wstring ss; sdv::getline(sstream, ss, L';');)
        EXPECT_EQ(ss, std::to_wstring(nIndex++));
    EXPECT_EQ(sstream.str(), L"0;1;2;3");

    sstream.str(L"0\n1\n2\n3");
    nIndex = 0;
    for (sdv::wstring ss; sdv::getline(std::move(sstream), ss);)
        EXPECT_EQ(ss, std::to_wstring(nIndex++));
}

TEST_F(CWStringTypeTest, MakeString)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeWString(ssAnsi), L"Hello");
    EXPECT_EQ(sdv::MakeWString(ssWide), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf8), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf16), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf32), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssAnsi.c_str()), L"Hello");
    EXPECT_EQ(sdv::MakeWString(ssWide.c_str()), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf8.c_str()), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf16.c_str()), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssUtf32.c_str()), L"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeWString(ssCppAnsi), L"Hello");
    EXPECT_EQ(sdv::MakeWString(ssCppWide), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssCppUtf8), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssCppUtf16), L"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeWString(ssCppUtf32), L"Hello\U00024B62Hello");
}

TEST_F(CWStringTypeTest, MakeStringGeneric)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    sdv::wstring ssDst;
    ssDst = sdv::MakeString<char, false, 0, wchar_t, true, 0>(ssAnsi);
    EXPECT_EQ(ssDst, L"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, wchar_t, true, 0>(ssWide);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, wchar_t, true, 0>(ssUtf8);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, wchar_t, true, 0>(ssUtf16);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, wchar_t, true, 0>(ssUtf32);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, false, 0, wchar_t, true, 0>(ssAnsi.c_str());
    EXPECT_EQ(ssDst, L"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, wchar_t, true, 0>(ssWide.c_str());
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, wchar_t, true, 0>(ssUtf8.c_str());
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, wchar_t, true, 0>(ssUtf16.c_str());
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, wchar_t, true, 0>(ssUtf32.c_str());
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    ssDst = sdv::MakeString<char, wchar_t, true, 0>(ssCppAnsi);
    EXPECT_EQ(ssDst, L"Hello");
    ssDst = sdv::MakeString<wchar_t, wchar_t, true, 0>(ssCppWide);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, wchar_t, true, 0>(ssCppUtf8);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, wchar_t, true, 0>(ssCppUtf16);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, wchar_t, true, 0>(ssCppUtf32);
    EXPECT_EQ(ssDst, L"Hello\U00024B62Hello");
}

TEST_F(CWStringTypeTest, IteratorDistance)
{
    sdv::wstring ss = L"This is a text";
    EXPECT_EQ(std::distance(ss.begin(), ss.end()), ss.length());
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
