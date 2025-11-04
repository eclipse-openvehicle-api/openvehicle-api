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

using CUtf32StringTypeTest = CBasicTypesTest;

TEST_F(CUtf32StringTypeTest, ConstructorStatic)
{
    // Empty string
    sdv::fixed_u32string<10> ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::fixed_u32string<10> ssCString(U"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");
    std::u32string ss(U"Hello");
    sdv::fixed_u32string<10> ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, U"Hello");

    // Copy constructor
    sdv::fixed_u32string<8> ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");

    // Move constructor
    sdv::fixed_u32string<12> ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");

    // Fill constructor
    sdv::fixed_u32string<20> ssFillString(10, U'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, U"**********");

    // Substring constructor
    sdv::fixed_u32string<20> ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, U"llo");
    sdv::fixed_u32string<21> ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, U"ll");
    sdv::fixed_u32string<22> ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, U"llo");
    sdv::fixed_u32string<23> ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, U"ll");
    sdv::fixed_u32string<24> ssSubstring5(U"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, U"He\0l", 4), 0);

    // Iterator based construction
    sdv::fixed_u32string<10> ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, U"Hello");
    sdv::fixed_u32string<10> ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, U"Hello");

    // Construct using initializer list
    sdv::fixed_u32string<10> ssInitListString{U'H', U'e', U'l', U'l', U'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, U"Hello");
}

TEST_F(CUtf32StringTypeTest, ConstructorDynamic)
{
    // Empty string
    sdv::u32string ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::u32string ssCString(U"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");
    std::u32string ss(U"Hello");
    sdv::u32string ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, U"Hello");

    // Copy constructor
    sdv::u32string ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");

    // Move constructor
    sdv::u32string ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, U"Hello");

    // Fill constructor
    sdv::u32string ssFillString(10, U'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, U"**********");

    // Substring constructor
    sdv::u32string ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, U"llo");
    sdv::u32string ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, U"ll");
    sdv::u32string ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, U"llo");
    sdv::u32string ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, U"ll");
    sdv::u32string ssSubstring5(U"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, U"He\0l", 4), 0);

    // Iterator based construction
    sdv::u32string ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, U"Hello");
    sdv::u32string ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, U"Hello");

    // Construct using initializer list
    sdv::u32string ssInitListString{U'H', U'e', U'l', U'l', U'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, U"Hello");
}

TEST_F(CUtf32StringTypeTest, ConstructorMixed)
{
    // Copy constructor
    sdv::u32string ssDynamic = U"dynamic";
    sdv::fixed_u32string<8> ssStaticCopy(ssDynamic);
    EXPECT_EQ(ssStaticCopy, U"dynamic");
    sdv::fixed_u32string<8> ssStatic = U"static";
    sdv::u32string ssDynamicCopy(ssStatic);
    EXPECT_EQ(ssDynamicCopy, U"static");

    // Move constructor
    sdv::fixed_u32string<8> ssStaticMove(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, U"dynamic");
    sdv::u32string ssDynamicMove(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, U"static");
}

TEST_F(CUtf32StringTypeTest, AssignmentOperatorStatic)
{
    // SDV-String assignment
    sdv::fixed_u32string<10> ssSdvString1(U"Hello");
    sdv::fixed_u32string<15> ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, U"Hello");

    // Move assignment
    sdv::fixed_u32string<10> ssSdvString3(U"Hello");
    sdv::fixed_u32string<15> ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, U"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::u32string ss1(U"Hello");
    sdv::fixed_u32string<10> ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, U"Hello");

    // C string assignment
    const char32_t sz1[] = U"Hello";
    sdv::fixed_u32string<10> ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, U"Hello");
    const char32_t* sz2 = U"Hello";
    sdv::fixed_u32string<10>	ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, U"Hello");

    // Initializer list
    sdv::fixed_u32string<10> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = U"Hello";
    EXPECT_EQ(ssSdvString8, U"Hello");
    sdv::fixed_u32string<10> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {U'H', U'e', U'l', U'l', U'o'};
    EXPECT_EQ(ssSdvString9, U"Hello");
}

TEST_F(CUtf32StringTypeTest, AssignmentOperatorDynamic)
{
    // SDV-String assignment
    sdv::u32string ssSdvString1(U"Hello");
    sdv::u32string ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, U"Hello");

    // Move assignment
    sdv::u32string ssSdvString3(U"Hello");
    sdv::u32string ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, U"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::u32string ss1(U"Hello");
    sdv::u32string ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, U"Hello");

    // C string assignment
    const char32_t sz1[] = U"Hello";
    sdv::u32string ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, U"Hello");
    const char32_t* sz2 = U"Hello";
    sdv::u32string ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, U"Hello");

    // Initializer list
    sdv::u32string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = U"Hello";
    EXPECT_EQ(ssSdvString8, U"Hello");
    sdv::u32string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {U'H', U'e', U'l', U'l', U'o'};
    EXPECT_EQ(ssSdvString9, U"Hello");
}

TEST_F(CUtf32StringTypeTest, AssignmentOperatorMixed)
{
    // Copy assignment
    sdv::u32string ssDynamic = U"dynamic";
    sdv::fixed_u32string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy = ssDynamic;
    EXPECT_EQ(ssStaticCopy, U"dynamic");
    sdv::fixed_u32string<8> ssStatic = U"static";
    sdv::u32string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy = ssStatic;
    EXPECT_EQ(ssDynamicCopy, U"static");

    // Move assignment
    sdv::fixed_u32string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove = std::move(ssDynamic);
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, U"dynamic");
    sdv::u32string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove = std::move(ssStatic);
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, U"static");
}

TEST_F(CUtf32StringTypeTest, AssignmentFunctionStatic)
{
    // Character assignment
    sdv::fixed_u32string<10> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");

    // String copy assignment
    sdv::fixed_u32string<10> ssSdvString2;
    sdv::fixed_u32string<15> ssSdvString3(U"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");

    // C++ string assignment
    sdv::fixed_u32string<10> ssSdvString4;
    std::u32string ss(U"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");

    // C string assignment
    sdv::fixed_u32string<10> ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    sdv::fixed_u32string<10> ssSdvString6;
    const char32_t* sz2 = U"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, U"Hello");

    // Substring assignment
    sdv::fixed_u32string<10> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    sdv::fixed_u32string<11> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, U"ell");
    sdv::fixed_u32string<12> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, U"ell");

    // Move assignment
    sdv::fixed_u32string<10> ssSdvString10;
    sdv::fixed_u32string<15> ssSdvString11(U"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, U"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::fixed_u32string<10> ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, U"Hello");

    // Initializer list
    sdv::fixed_u32string<10> ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({U'H', U'e', U'l', U'l', U'o'});
    EXPECT_EQ(ssSdvString13, U"Hello");
}

TEST_F(CUtf32StringTypeTest, AssignmentFunctionDynamic)
{
    // Character assignment
    sdv::u32string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");

    // String copy assignment
    sdv::u32string ssSdvString2;
    sdv::u32string ssSdvString3(U"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");

    // C++ string assignment
    sdv::u32string ssSdvString4;
    std::u32string ss(U"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");

    // C string assignment
    sdv::u32string ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    sdv::u32string ssSdvString6;
    const char32_t* sz2 = U"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, U"Hello");

    // Substring assignment
    sdv::u32string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    sdv::u32string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, U"ell");
    sdv::u32string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, U"ell");

    // Move assignment
    sdv::u32string ssSdvString10;
    sdv::u32string ssSdvString11(U"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, U"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::u32string ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, U"Hello");

    // Initializer list
    sdv::u32string ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({U'H', U'e', U'l', U'l', U'o'});
    EXPECT_EQ(ssSdvString13, U"Hello");
}

TEST_F(CUtf32StringTypeTest, AssignmentFunctionMixed)
{
    // Copy assignment
    sdv::u32string ssDynamic = U"dynamic";
    sdv::fixed_u32string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy.assign(ssDynamic);
    EXPECT_EQ(ssStaticCopy, U"dynamic");
    sdv::fixed_u32string<8> ssStatic = U"static";
    sdv::u32string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy.assign(ssStatic);
    EXPECT_EQ(ssDynamicCopy, U"static");

    // Move assignment
    sdv::fixed_u32string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove.assign(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, U"dynamic");
    sdv::u32string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove.assign(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, U"static");

    // Substring assignment
    sdv::fixed_u32string<10> ssStaticSubstring;
    EXPECT_TRUE(ssStaticSubstring.empty());
    ssStaticSubstring.assign(ssDynamicMove, 1, 3);
    EXPECT_EQ(ssStaticSubstring, U"tat");
    sdv::u32string ssDynamicSubstring;
    EXPECT_TRUE(ssDynamicSubstring.empty());
    ssDynamicSubstring.assign(ssStaticMove, 1, 3);
    EXPECT_EQ(ssDynamicSubstring, U"yna");
}

TEST_F(CUtf32StringTypeTest, PositionFunction)
{
    // Position in the string
    sdv::u32string ssSdvString1(U"Hello");
    EXPECT_EQ(ssSdvString1.at(0), U'H');
    EXPECT_EQ(ssSdvString1[0], U'H');
    EXPECT_EQ(ssSdvString1.front(), U'H');
    EXPECT_EQ(ssSdvString1.at(4), U'o');
    EXPECT_EQ(ssSdvString1[4], U'o');
    EXPECT_EQ(ssSdvString1.back(), U'o');
    EXPECT_THROW(ssSdvString1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString1[5], sdv::XIndexOutOfRange);

    // Empty string
    sdv::u32string ssSdvString2;
    EXPECT_THROW(ssSdvString2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.back(), sdv::XIndexOutOfRange);

    // Assignment
    ssSdvString1.at(0) = 'B';
    EXPECT_EQ(ssSdvString1, U"Bello");
    ssSdvString1[0] = 'D';
    EXPECT_EQ(ssSdvString1, U"Dello");
    ssSdvString1.front() = 'M';
    EXPECT_EQ(ssSdvString1, U"Mello");
    ssSdvString1.back() = 'k';
    EXPECT_EQ(ssSdvString1, U"Mellk");
}

TEST_F(CUtf32StringTypeTest, CAndCppStringAccess)
{
    // C++ cast operator
    sdv::u32string ssSdvString(U"Hello");
    std::u32string ss1(ssSdvString);
    EXPECT_EQ(ss1, U"Hello");
    std::u32string ss2;
    EXPECT_TRUE(ss2.empty());
    ss2 = ssSdvString;
    EXPECT_EQ(ss2, U"Hello");

    // Data access
    sdv::u32string ssSdvString2(U"He\0lo", 5);
    EXPECT_EQ(ssSdvString2.size(), 5);
    const char32_t* sz1 = ssSdvString2.data();
    EXPECT_EQ(memcmp(sz1, U"He\0lo", 5 * sizeof(char32_t)), 0);

    // C string access
    const char32_t* sz2 = ssSdvString.c_str();
    EXPECT_STREQ(sz2, U"Hello");
}

TEST_F(CUtf32StringTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::u32string::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u32string ssSdvString(U"Hello");
    EXPECT_NE(itEmpty, ssSdvString.begin());
    sdv::u32string::iterator itPos = ssSdvString.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.begin());
    sdv::u32string::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u32string::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, U'H');
    EXPECT_EQ(itPos[0], U'H');
    EXPECT_EQ(itPos[4], U'o');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'B';
    EXPECT_EQ(ssSdvString, U"Bello");
    itPos[4] = 'k';
    EXPECT_EQ(ssSdvString, U"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, U'e');
    itPos += 3;
    EXPECT_EQ(*itPos, U'o');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.end());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, U'o');
    itPos -= 4;
    EXPECT_EQ(*itPos, U'H');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, U'H');
    EXPECT_EQ(itPos, ssSdvString.begin());

    // Const iterator
    sdv::u32string::const_iterator itPosConst = ssSdvString.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, U'e');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.cbegin(), ssSdvString.begin());
    EXPECT_EQ(ssSdvString.begin(), ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString.cend(), ssSdvString.end());
    EXPECT_EQ(ssSdvString.end(), ssSdvString.cend());
}

TEST_F(CUtf32StringTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::u32string::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u32string ssSdvString(U"Hello");
    EXPECT_NE(itEmpty, ssSdvString.rbegin());
    sdv::u32string::reverse_iterator itPos = ssSdvString.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.rbegin());
    sdv::u32string::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u32string::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, U'o');
    EXPECT_EQ(itPos[0], U'o');
    EXPECT_EQ(itPos[4], U'H');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'k';
    EXPECT_EQ(ssSdvString, U"Hellk");
    itPos[4] = 'B';
    EXPECT_EQ(ssSdvString, U"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, U'l');
    itPos += 3;
    EXPECT_EQ(*itPos, U'H');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.rend());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, U'H');
    itPos -= 4;
    EXPECT_EQ(*itPos, U'o');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, U'o');
    EXPECT_EQ(itPos, ssSdvString.rbegin());

    // Const iterator
    sdv::u32string::const_reverse_iterator itPosConst = ssSdvString.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, U'l');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.crbegin(), ssSdvString.rbegin());
    EXPECT_EQ(ssSdvString.rbegin(), ssSdvString.crbegin());
    EXPECT_EQ(ssSdvString.crend(), ssSdvString.rend());
    EXPECT_EQ(ssSdvString.rend(), ssSdvString.crend());
}

TEST_F(CUtf32StringTypeTest, StringCapacityStatic)
{
    // Empty string
    sdv::fixed_u32string<10> ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Filled string
    ssSdvString = U"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, U"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString += 'o';
    ssSdvString.resize(10, U'*');
    EXPECT_EQ(ssSdvString, U"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, U"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
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

TEST_F(CUtf32StringTypeTest, StringCapacityDynamic)
{
    // Empty string
    sdv::u32string ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 0);

    // Filled string
    ssSdvString = U"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, U"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 5);
    ssSdvString += 'o';
    ssSdvString.resize(10, U'*');
    EXPECT_EQ(ssSdvString, U"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, U"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), U"Hello");
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

TEST_F(CUtf32StringTypeTest, InsertFunctionStatic)
{
    // Character assignment
    sdv::fixed_u32string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");
    ssSdvString1.insert(0, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********??");
    ssSdvString1.insert(std::u32string::npos, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********????");
    ssSdvString1.insert(4, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**??********????");

    // C string assignment
    sdv::fixed_u32string<20> ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::u32string::npos, sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, U"**Hello");
    ssSdvString5.insert(std::u32string::npos, sz2);
    EXPECT_EQ(ssSdvString5, U"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, U"**He**llo**");

    // String copy assignment
    sdv::fixed_u32string<20> ssSdvString2;
    sdv::fixed_u32string<20> ssSdvString3(U"Hello");
    sdv::fixed_u32string<20> ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**Hello");
    ssSdvString2.insert(std::u32string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**He**llo**");

    // C++ string assignment
    sdv::fixed_u32string<20> ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, U"**Hello");
    ssSdvString4.insert(std::u32string::npos, ss2);
    EXPECT_EQ(ssSdvString4, U"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, U"**He**llo**");

    // Substring assignment
    sdv::fixed_u32string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, U"eellll");
    ssSdvString7.insert(std::u32string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, U"eellllell");

    // Iterator assignment
    sdv::fixed_u32string<20> ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), U'+');
    EXPECT_EQ(ssSdvString12, U"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), U'-');
    EXPECT_EQ(ssSdvString12, U"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), U'+');
    EXPECT_EQ(ssSdvString12, U"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), U'-');
    EXPECT_EQ(ssSdvString12, U"-+Hello+-");
    sdv::fixed_u32string<20>::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, U'#');
    EXPECT_EQ(ssSdvString12, U"-#+Hello+-");
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, U'#');
    EXPECT_EQ(ssSdvString12, U"-##-++Hello++--");
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, U'#');
    EXPECT_EQ(ssSdvString12, U"-##-++Hello++--");
    ssSdvString12			 = U"Hello";
    sdv::fixed_u32string<20> ssSdvString8 = U"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"112212Hello1212");

    // Initializer list
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"112212Hello1212");
}

TEST_F(CUtf32StringTypeTest, InsertFunctionDynamic)
{
    // Character assignment
    sdv::u32string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");
    ssSdvString1.insert(0, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********??");
    ssSdvString1.insert(std::u32string::npos, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**********????");
    ssSdvString1.insert(4, 2, U'?');
    EXPECT_EQ(ssSdvString1, U"??**??********????");

    // C string assignment
    sdv::u32string ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::u32string::npos, sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, U"**Hello");
    ssSdvString5.insert(std::u32string::npos, sz2);
    EXPECT_EQ(ssSdvString5, U"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, U"**He**llo**");

    // String copy assignment
    sdv::u32string ssSdvString2;
    sdv::u32string ssSdvString3(U"Hello");
    sdv::u32string ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**Hello");
    ssSdvString2.insert(std::u32string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"**He**llo**");

    // C++ string assignment
    sdv::u32string ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, U"**Hello");
    ssSdvString4.insert(std::u32string::npos, ss2);
    EXPECT_EQ(ssSdvString4, U"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, U"**He**llo**");

    // Substring assignment
    sdv::u32string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, U"eellll");
    ssSdvString7.insert(std::u32string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, U"eellllell");

    // Iterator assignment
    sdv::u32string ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), U'+');
    EXPECT_EQ(ssSdvString12, U"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), U'-');
    EXPECT_EQ(ssSdvString12, U"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), U'+');
    EXPECT_EQ(ssSdvString12, U"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), U'-');
    EXPECT_EQ(ssSdvString12, U"-+Hello+-");
    sdv::u32string::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, U'#');
    EXPECT_EQ(ssSdvString12, U"-#+Hello+-");
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, U'#');
    EXPECT_EQ(ssSdvString12, U"-##-++Hello++--");
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, U'+');
    EXPECT_EQ(ssSdvString12, U"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, U'-');
    EXPECT_EQ(ssSdvString12, U"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, U'#');
    EXPECT_EQ(ssSdvString12, U"-##-++Hello++--");
    ssSdvString12 = U"Hello";
    sdv::u32string ssSdvString8 = U"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, U"112212Hello1212");

    // Initializer list
    ssSdvString12 = U"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"112212Hello1212");
}

TEST_F(CUtf32StringTypeTest, InsertFunctionMixed)
{
    // String copy assignment - static into dynamic
    sdv::u32string ssDynamic;
    sdv::fixed_u32string<20> ssStatic1(U"Hello");
    sdv::fixed_u32string<20> ssStatic2(U"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(3, ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, U"Hello");
    ssDynamic.insert(0, ssStatic2);
    EXPECT_EQ(ssDynamic, U"**Hello");
    ssDynamic.insert(sdv::u32string::npos, ssStatic2);
    EXPECT_EQ(ssDynamic, U"**Hello**");
    ssDynamic.insert(4, ssStatic2);
    EXPECT_EQ(ssDynamic, U"**He**llo**");

    // String copy assignment - dynamic into static
    sdv::fixed_u32string<20> ssStatic;
    sdv::fixed_u32string<20> ssDynamic1(U"Hello");
    sdv::fixed_u32string<20> ssDynamic2(U"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(3, ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, U"Hello");
    ssStatic.insert(0, ssDynamic2);
    EXPECT_EQ(ssStatic, U"**Hello");
    ssStatic.insert(sdv::u32string::npos, ssDynamic2);
    EXPECT_EQ(ssStatic, U"**Hello**");
    ssStatic.insert(4, ssDynamic2);
    EXPECT_EQ(ssStatic, U"**He**llo**");

    // Substring assignment - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(0, ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, U"ell");

    // Substring assignment - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(0, ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, U"ell");
}

TEST_F(CUtf32StringTypeTest, EraseFunction)
{
    // Erase index based
    sdv::u32string ssSdvString = U"12345678901234567890";
    ssSdvString.erase(8, 5);
    EXPECT_EQ(ssSdvString, U"123456784567890");
    ssSdvString.erase(0, 2);
    EXPECT_EQ(ssSdvString, U"3456784567890");
    ssSdvString.erase(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString, U"3456784567890");
    ssSdvString.erase(sdv::u32string::npos);
    EXPECT_EQ(ssSdvString, U"3456784567890");
    ssSdvString.erase(8);
    EXPECT_EQ(ssSdvString, U"34567845");

    // Erase iterator
    ssSdvString = U"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString, U"2345678901234567890");
    ssSdvString.erase(ssSdvString.cend());
    EXPECT_EQ(ssSdvString, U"2345678901234567890");
    ssSdvString.erase(ssSdvString.cbegin() + 5);
    EXPECT_EQ(ssSdvString, U"234568901234567890");
    sdv::u32string::iterator itPos = ssSdvString.erase(ssSdvString.end() - 2);
    EXPECT_EQ(ssSdvString, U"23456890123456780");
    EXPECT_EQ(*itPos, U'0');
    ++itPos;
    EXPECT_EQ(itPos, ssSdvString.cend());

    // Erase iterator range
    ssSdvString = U"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin(), ssSdvString.cbegin() + 4);
    EXPECT_EQ(ssSdvString, U"5678901234567890");
    itPos = ssSdvString.erase(ssSdvString.cbegin() + 4, ssSdvString.cbegin() + 6);
    EXPECT_EQ(ssSdvString, U"56781234567890");
    EXPECT_EQ(*itPos, U'1');
    itPos += 10;
    EXPECT_EQ(itPos, ssSdvString.cend());
}

TEST_F(CUtf32StringTypeTest, PushPopFunctions)
{
    sdv::u32string ssSdvString;
    ssSdvString.push_back(U'1');
    EXPECT_EQ(ssSdvString, U"1");
    ssSdvString.push_back(U'2');
    EXPECT_EQ(ssSdvString, U"12");
    ssSdvString.pop_back();
    EXPECT_EQ(ssSdvString, U"1");
    ssSdvString.pop_back();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_NO_THROW(ssSdvString.pop_back());
}

TEST_F(CUtf32StringTypeTest, AppendFunctionStatic)
{
    // Append characters
    sdv::fixed_u32string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");
    ssSdvString1.append(2, U'?');
    EXPECT_EQ(ssSdvString1, U"**********??");

    // Append string
    sdv::fixed_u32string<20> ssSdvString2;
    sdv::fixed_u32string<10> ssSdvString3(U"Hello");
    sdv::fixed_u32string<15> ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"Hello**");

    // Append C++ string
    sdv::fixed_u32string<20> ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, U"Hello**");

    // Append substring
    sdv::fixed_u32string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ellell");

    // C string assignment
    sdv::fixed_u32string<20> ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, U"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, U"Hello**Hello");

    // Iterator based
    sdv::fixed_u32string<20> ssSdvString12 = U"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, U"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = U"Hello";
    ssSdvString12.append({U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"Hello12");
}

TEST_F(CUtf32StringTypeTest, AppendFunctionDynamic)
{
    // Append characters
    sdv::u32string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, U'*');
    EXPECT_EQ(ssSdvString1, U"**********");
    ssSdvString1.append(2, U'?');
    EXPECT_EQ(ssSdvString1, U"**********??");

    // Append string
    sdv::u32string ssSdvString2;
    sdv::u32string ssSdvString3(U"Hello");
    sdv::u32string ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, U"Hello**");

    // Append C++ string
    sdv::u32string ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, U"Hello**");

    // Append substring
    sdv::u32string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, U"ellell");

    // C string assignment
    sdv::u32string ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, U"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, U"Hello**Hello");

    // Iterator based
    sdv::u32string ssSdvString12 = U"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, U"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = U"Hello";
    ssSdvString12.append({U'1', U'2'});
    EXPECT_EQ(ssSdvString12, U"Hello12");
}

TEST_F(CUtf32StringTypeTest, AppendFunctionMixed)
{
    // Append string - static into dynamic
    sdv::u32string ssDynamic;
    sdv::fixed_u32string<10> ssStatic1(U"Hello");
    sdv::fixed_u32string<15> ssStatic2(U"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, U"Hello");
    ssDynamic.append(ssStatic2);
    EXPECT_EQ(ssDynamic, U"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u32string<20> ssStatic;
    sdv::u32string ssDynamic1(U"Hello");
    sdv::u32string ssDynamic2(U"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, U"Hello");
    ssStatic.append(ssDynamic2);
    EXPECT_EQ(ssStatic, U"Hello**");

    // Append substring - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, U"ell");

    // Append substring - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, U"ell");
}

TEST_F(CUtf32StringTypeTest, AppendOperatorStatic)
{
    // Append string
    sdv::fixed_u32string<20> ssSdvString2;
    sdv::fixed_u32string<15> ssSdvString3(U"Hello");
    sdv::fixed_u32string<10> ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, U"Hello**");

    // Append C++ string
    sdv::fixed_u32string<20> ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, U"Hello**");

    // Append character
    sdv::fixed_u32string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, U"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, U"*?");

    // C string assignment
    sdv::fixed_u32string<20>	ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, U"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, U"Hello**Hello");

    // Initializer list
    sdv::fixed_u32string<20> ssSdvString12 = U"Hello";
    ssSdvString12 += {U'1', U'2'};
    EXPECT_EQ(ssSdvString12, U"Hello12");
}

TEST_F(CUtf32StringTypeTest, AppendOperatorDynamic)
{
    // Append string
    sdv::u32string ssSdvString2;
    sdv::u32string ssSdvString3(U"Hello");
    sdv::u32string ssSdvString6(U"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, U"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, U"Hello**");

    // Append C++ string
    sdv::u32string ssSdvString4;
    std::u32string ss(U"Hello");
    std::u32string ss2(U"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, U"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, U"Hello**");

    // Append character
    sdv::u32string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, U"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, U"*?");

    // C string assignment
    sdv::u32string ssSdvString5;
    const char32_t	sz1[] = U"Hello";
    const char32_t* sz2	  = U"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, U"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, U"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, U"Hello**Hello");

    // Initializer list
    sdv::u32string ssSdvString12 = U"Hello";
    ssSdvString12 += {U'1', U'2'};
    EXPECT_EQ(ssSdvString12, U"Hello12");
}

TEST_F(CUtf32StringTypeTest, AppendOperatorMixed)
{
    // Append string - static into dynamic
    sdv::u32string ssDynamic;
    sdv::fixed_u32string<15> ssStatic1(U"Hello");
    sdv::fixed_u32string<10> ssStatic2(U"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic += ssStatic1;
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, U"Hello");
    ssDynamic += ssStatic2;
    EXPECT_EQ(ssDynamic, U"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u32string<20> ssStatic;
    sdv::u32string ssDynamic1(U"Hello");
    sdv::u32string ssDynamic2(U"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic += ssDynamic1;
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, U"Hello");
    ssStatic += ssStatic2;
    EXPECT_EQ(ssStatic, U"Hello**");
}

TEST_F(CUtf32StringTypeTest, CompareFunctionStatic)
{
    // Compare strings
    sdv::fixed_u32string<20> ssSdvString1;
    sdv::fixed_u32string<10> ssSdvString2(U"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u32string ssString(U"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char32_t* sz1 = U"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char32_t sz2[] = U"Hello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = U"Kello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssSdvString2, 1, sdv::fixed_u32string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = U"Kello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString, 1, sdv::fixed_u32string<20>::npos), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, ssString, 1, sdv::fixed_u32string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = U"Kellogg";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char32_t sz3[] = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz3), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz3), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz3), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz3), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char32_t sz4[] = U"Kellogg";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u32string<20>::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf32StringTypeTest, CompareFunctionDynamic)
{
    // Compare strings
    sdv::u32string ssSdvString1;
    sdv::u32string ssSdvString2(U"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u32string ssString(U"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char32_t* sz1 = U"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char32_t sz2[] = U"Hello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = U"Kello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssSdvString2, 1, sdv::u32string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssString), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssString), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, ssString), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssString), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = U"Kello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssString, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, ssString, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, ssString, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssString, 1, sdv::u32string::npos), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, ssString, 1, sdv::u32string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz1), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz1), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, sz1), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz1), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = U"Kellogg";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char32_t sz3[] = U"ello";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz3), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz3), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, sz3), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz3), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char32_t sz4[] = U"Kellogg";
    ssSdvString1 = U"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u32string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u32string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = U"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u32string::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf32StringTypeTest, CompareFunctionMixed)
{
    // Compare strings - compare dynamic with static
    sdv::u32string ssDynamic;
    sdv::fixed_u32string<10> ssStatic1(U"Hello");
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = U"Helln";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = U"HellnO";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = U"Hello";
    EXPECT_EQ(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = U"Hellp";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = U"Helloa";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);

    // Compare substring
    ssStatic1 = U"ello";
    ssDynamic = U"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1), 0);
    ssDynamic = U"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1), 0);
    ssDynamic = U"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1), 0);
    ssDynamic = U"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1), 0);
    ssDynamic = U"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1), 0);
    ssStatic1 = U"Kello";
    ssDynamic = U"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssDynamic = U"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssDynamic = U"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssDynamic = U"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssDynamic = U"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u32string<20>::npos, ssStatic1, 1, sdv::fixed_u32string<20>::npos), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1, 1, 4), 0);

    // Compare strings - compare static with dynamic
    sdv::fixed_u32string<10> ssStatic;
    sdv::u32string ssDynamic1(U"Hello");
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = U"Helln";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = U"HellnO";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = U"Hello";
    EXPECT_EQ(ssStatic.compare(ssDynamic1), 0);
    ssStatic = U"Hellp";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = U"Helloa";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);

    // Compare substring
    ssDynamic1 = U"ello";
    ssStatic = U"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1), 0);
    ssStatic = U"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1), 0);
    ssStatic = U"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1), 0);
    ssStatic = U"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1), 0);
    ssStatic = U"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1), 0);
    ssDynamic1 = U"Kello";
    ssStatic = U"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssStatic = U"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssStatic = U"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssStatic = U"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1, 1, sdv::fixed_u32string<20>::npos), 0);
    ssStatic = U"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u32string<20>::npos, ssDynamic1, 1, sdv::fixed_u32string<20>::npos), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1, 1, 4), 0);
}

TEST_F(CUtf32StringTypeTest, CompareOperatorStatic)
{
    // Compare strings
    sdv::fixed_u32string<20> ssSdvString1;
    sdv::fixed_u32string<10> ssSdvString2(U"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u32string ssString(U"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char32_t* sz1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char32_t sz2[] = U"Hello";
    ssSdvString1	 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf32StringTypeTest, CompareOperatorDynamic)
{
    // Compare strings
    sdv::u32string ssSdvString1;
    sdv::u32string ssSdvString2(U"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u32string ssString(U"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char32_t* sz1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char32_t sz2[] = U"Hello";
    ssSdvString1	 = U"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = U"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = U"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf32StringTypeTest, CompareOperatorMixed)
{
    // Compare strings - compare dynamic with static
    sdv::u32string ssDynamic;
    sdv::fixed_u32string<10> ssStatic1(U"Hello");
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = U"Helln";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = U"HellnO";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = U"Hello";
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic == ssStatic1);
    ssDynamic = U"Hellp";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = U"Helloa";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);

    // Compare strings - compare static with dynamic
    sdv::fixed_u32string<10> ssStatic;
    sdv::u32string ssDynamic1(U"Hello");
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = U"Helln";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = U"HellnO";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = U"Hello";
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic == ssDynamic1);
    ssStatic = U"Hellp";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = U"Helloa";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
}

TEST_F(CUtf32StringTypeTest, ReplaceFunctionStatic)
{
    // Replace string
    sdv::fixed_u32string<20> ssSdvString = U"1234567890";
    sdv::fixed_u32string<10> ssSdvString2 = U"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString	 = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace C++ string
    ssSdvString	 = U"1234567890";
    std::u32string ss = U"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ss);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace C string
    ssSdvString	 = U"1234567890";
    const char32_t sz1[] = U"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz1);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString		 = U"1234567890";
    const char32_t* sz2 = U"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz2);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace substring
    ssSdvString	 = U"1234567890";
    ssSdvString2 = U"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = U"1234567890";
    ss = U"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = U"1234567890";
    const char32_t sz3[] = U"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    sz2 = U"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace with characters
    ssSdvString = U"1234567890";
    ssSdvString.replace(1, 2, 4, U'+');
    EXPECT_EQ(ssSdvString, U"1++++4567890");
    ssSdvString.replace(2, 6, 4, U'-');
    EXPECT_EQ(ssSdvString, U"1+----7890");
    ssSdvString.replace(3, 4, 4, U'*');
    EXPECT_EQ(ssSdvString, U"1+-****890");
    ssSdvString.replace(0, 0, 4, U'#');
    EXPECT_EQ(ssSdvString, U"####1+-****890");
    ssSdvString.replace(std::u32string::npos, 100, 4, U'~');
    EXPECT_EQ(ssSdvString, U"####1+-****890~~~~");
    ssSdvString.replace(0, std::u32string::npos, 4, U'=');
    EXPECT_EQ(ssSdvString, U"====");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, U'+');
    EXPECT_EQ(ssSdvString, U"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, U'-');
    EXPECT_EQ(ssSdvString, U"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, U'*');
    EXPECT_EQ(ssSdvString, U"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, U'#');
    EXPECT_EQ(ssSdvString, U"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, U'~');
    EXPECT_EQ(ssSdvString, U"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, U'=');
    EXPECT_EQ(ssSdvString, U"====");

    // Replace with iterators
    ssSdvString = U"1234567890";
    ss = U"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace with initialization list
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd");
}

TEST_F(CUtf32StringTypeTest, ReplaceFunctionDynamic)
{
    // Replace string
    sdv::u32string ssSdvString = U"1234567890";
    sdv::u32string ssSdvString2 = U"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString	 = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace C++ string
    ssSdvString	 = U"1234567890";
    std::u32string ss = U"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ss);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace C string
    ssSdvString	 = U"1234567890";
    const char32_t sz1[] = U"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz1);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString		 = U"1234567890";
    const char32_t* sz2 = U"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz2);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace substring
    ssSdvString	 = U"1234567890";
    ssSdvString2 = U"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = U"1234567890";
    ss = U"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = U"1234567890";
    const char32_t sz3[] = U"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    sz2 = U"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(std::u32string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u32string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace with characters
    ssSdvString = U"1234567890";
    ssSdvString.replace(1, 2, 4, U'+');
    EXPECT_EQ(ssSdvString, U"1++++4567890");
    ssSdvString.replace(2, 6, 4, U'-');
    EXPECT_EQ(ssSdvString, U"1+----7890");
    ssSdvString.replace(3, 4, 4, U'*');
    EXPECT_EQ(ssSdvString, U"1+-****890");
    ssSdvString.replace(0, 0, 4, U'#');
    EXPECT_EQ(ssSdvString, U"####1+-****890");
    ssSdvString.replace(std::u32string::npos, 100, 4, U'~');
    EXPECT_EQ(ssSdvString, U"####1+-****890~~~~");
    ssSdvString.replace(0, std::u32string::npos, 4, U'=');
    EXPECT_EQ(ssSdvString, U"====");
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, U'+');
    EXPECT_EQ(ssSdvString, U"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, U'-');
    EXPECT_EQ(ssSdvString, U"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, U'*');
    EXPECT_EQ(ssSdvString, U"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, U'#');
    EXPECT_EQ(ssSdvString, U"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, U'~');
    EXPECT_EQ(ssSdvString, U"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, U'=');
    EXPECT_EQ(ssSdvString, U"====");

    // Replace with iterators
    ssSdvString = U"1234567890";
    ss = U"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, U"abcd");

    // Replace with initialization list
    ssSdvString = U"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {U'a', U'b', U'c', U'd'});
    EXPECT_EQ(ssSdvString, U"abcd");
}

TEST_F(CUtf32StringTypeTest, ReplaceFunctionMixed)
{
    // Replace string - replace dynamic by static
    sdv::u32string ssDynamic = U"1234567890";
    sdv::fixed_u32string<10> ssStatic1 = U"abcd";
    ssDynamic.replace(1, 2, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890");
    ssDynamic.replace(std::u32string::npos, 100, ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::u32string::npos, ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd");
    ssDynamic = U"1234567890";
    ssDynamic.replace(ssDynamic.begin() + 1, ssDynamic.begin() + 3, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1abcd4567890");
    ssDynamic.replace(ssDynamic.begin() + 2, ssDynamic.begin() + 8, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1aabcd7890");
    ssDynamic.replace(ssDynamic.begin() + 3, ssDynamic.begin() + 7, ssStatic1);
    EXPECT_EQ(ssDynamic, U"1aaabcd890");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.begin(), ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890");
    ssDynamic.replace(ssDynamic.end(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890abcd");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, U"abcd");

    // Replace substring
    ssDynamic = U"1234567890";
    ssStatic1 = U"xxabcdxx";
    ssDynamic.replace(1, 2, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, U"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, U"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, U"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890");
    ssDynamic.replace(std::u32string::npos, 100, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, U"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::u32string::npos, ssStatic1, 2, 4);

    // Replace string - replace static by dynamic
    sdv::fixed_u32string<20> ssStatic = U"1234567890";
    sdv::u32string ssDynamic1 = U"abcd";
    ssStatic.replace(1, 2, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890");
    ssStatic.replace(std::u32string::npos, 100, ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::u32string::npos, ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd");
    ssStatic = U"1234567890";
    ssStatic.replace(ssStatic.begin() + 1, ssStatic.begin() + 3, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1abcd4567890");
    ssStatic.replace(ssStatic.begin() + 2, ssStatic.begin() + 8, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1aabcd7890");
    ssStatic.replace(ssStatic.begin() + 3, ssStatic.begin() + 7, ssDynamic1);
    EXPECT_EQ(ssStatic, U"1aaabcd890");
    ssStatic.replace(ssStatic.begin(), ssStatic.begin(), ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890");
    ssStatic.replace(ssStatic.end(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890abcd");
    ssStatic.replace(ssStatic.begin(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, U"abcd");

    // Replace substring
    ssStatic = U"1234567890";
    ssDynamic1 = U"xxabcdxx";
    ssStatic.replace(1, 2, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, U"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, U"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, U"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890");
    ssStatic.replace(std::u32string::npos, 100, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, U"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::u32string::npos, ssDynamic1, 2, 4);
}

TEST_F(CUtf32StringTypeTest, SubstringFunctionStatic)
{
    sdv::fixed_u32string<20> ssSdvString = U"1234567890";
    EXPECT_EQ(ssSdvString.substr(), U"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), U"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), U"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf32StringTypeTest, SubstringFunctionDynamic)
{
    sdv::u32string ssSdvString = U"1234567890";
    EXPECT_EQ(ssSdvString.substr(), U"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), U"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), U"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf32StringTypeTest, CopyFunction)
{
    sdv::u32string ssSdvString = U"1234567890";
    char32_t sz[32] = {};
    EXPECT_EQ(ssSdvString.copy(sz, 2, 5), 2);
    EXPECT_STREQ(sz, U"67");
    std::fill_n(sz, 32, static_cast<char32_t>(U'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 20, 5), 5);
    EXPECT_STREQ(sz, U"67890");
    std::fill_n(sz, 32, static_cast<char32_t>(U'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0, 5), 0);
    EXPECT_STREQ(sz, U"");
    std::fill_n(sz, 32, static_cast<char32_t>(U'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 2), 2);
    EXPECT_STREQ(sz, U"12");
    std::fill_n(sz, 32, static_cast<char32_t>(U'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0), 0);
    EXPECT_STREQ(sz, U"");
    std::fill_n(sz, 32, static_cast<char32_t>(U'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, std::u32string::npos), 10);
    EXPECT_STREQ(sz, U"1234567890");
}

TEST_F(CUtf32StringTypeTest, SwapFunctionStatic)
{
    sdv::fixed_u32string<20> ssSdvString1 = U"12345";
    sdv::fixed_u32string<10> ssSdvString2 = U"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"67890");
    EXPECT_EQ(ssSdvString2, U"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"12345");
    EXPECT_EQ(ssSdvString2, U"67890");
}

TEST_F(CUtf32StringTypeTest, SwapFunctionDynamic)
{
    sdv::u32string ssSdvString1 = U"12345";
    sdv::u32string ssSdvString2 = U"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"67890");
    EXPECT_EQ(ssSdvString2, U"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"12345");
    EXPECT_EQ(ssSdvString2, U"67890");
}

TEST_F(CUtf32StringTypeTest, SwapFunctionMixed)
{
    sdv::fixed_u32string<20> ssSdvString1 = U"12345";
    sdv::u32string ssSdvString2 = U"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"67890");
    EXPECT_EQ(ssSdvString2, U"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, U"12345");
    EXPECT_EQ(ssSdvString2, U"67890");
}

TEST_F(CUtf32StringTypeTest, FindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"aabbaaccbbcc";
    sdv::fixed_u32string<10> ssSdvString2 = U"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::fixed_u32string<20>::npos);

    // Find C++ string in string
    std::u32string ss = U"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::fixed_u32string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 3, 4), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 9), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(U'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(U'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(U'\0', 11), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"aabbaaccbbcc";
    sdv::u32string ssSdvString2 = U"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::u32string::npos);

    // Find C++ string in string
    std::u32string ss = U"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::u32string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 3, 4), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(U"bb\0a", 9), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(U'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(U'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(U'\0', 11), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, FindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u32string ssDynamic = U"aabbaaccbbcc";
    sdv::fixed_u32string<10> ssStatic1 = U"aa";
    EXPECT_EQ(ssDynamic.find(ssStatic1), 0);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 1), 4);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 5), sdv::fixed_string<20>::npos);

    // Find string in string - find static in dynamic
    sdv::fixed_u32string<20> ssStatic = U"aabbaaccbbcc";
    sdv::u32string ssDynamic1 = U"aa";
    EXPECT_EQ(ssStatic.find(ssDynamic1), 0);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 1), 4);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 5), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, ReverseFindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"aabbaaccbbcc";
    sdv::fixed_u32string<10> ssSdvString2 = U"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::u32string ss = U"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", sdv::fixed_u32string<20>::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 1, 4), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", sdv::fixed_u32string<20>::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 1), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(U'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(U'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(U'\0', 3), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, ReverseFindFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"aabbaaccbbcc";
    sdv::u32string ssSdvString2 = U"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::u32string ss = U"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", sdv::u32string::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 1, 4), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", sdv::u32string::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(U"bb\0a", 1), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(U'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(U'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(U'\0', 3), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, ReverseFindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u32string ssDynamic = U"aabbaaccbbcc";
    sdv::fixed_u32string<10> ssStatic1 = U"aa";
    EXPECT_EQ(ssDynamic.rfind(ssStatic1), 4);
    EXPECT_EQ(ssDynamic.rfind(ssStatic1, 3), 0);

    // Find string in string - find static in dynamic
    sdv::fixed_u32string<20> ssStatic = U"aabbaaccbbcc";
    sdv::u32string ssDynamic1 = U"aa";
    EXPECT_EQ(ssStatic.rfind(ssDynamic1), 4);
    EXPECT_EQ(ssStatic.rfind(ssDynamic1, 3), 0);
}

TEST_F(CUtf32StringTypeTest, FindFirstOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"12341234";
    sdv::fixed_u32string<10> ssSdvString2 = U"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::fixed_u32string<20>::npos);

    // Find C++ string in string
    std::u32string ss = U"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::fixed_u32string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 10, 2), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 8), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 11), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindFirstOfFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"12341234";
    sdv::u32string ssSdvString2 = U"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::u32string::npos);

    // Find C++ string in string
    std::u32string ss = U"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::u32string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 10, 2), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(U"c\0", 8), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(U'\0', 11), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, FindFirstOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u32string ssDynamic = U"12341234";
    sdv::fixed_u32string<10> ssStatic1 = U"34";
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in sttaic
    sdv::fixed_u32string<10> ssStatic = U"12341234";
    sdv::u32string ssDynamic1 = U"34";
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindFirstNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"12341234";
    sdv::fixed_u32string<10> ssSdvString2 = U"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::fixed_u32string<20>::npos);

    // Find C++ string in string
    std::u32string ss = U"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::fixed_u32string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 9, 3), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 10), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 12), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindFirstNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"12341234";
    sdv::u32string ssSdvString2 = U"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::u32string::npos);

    // Find C++ string in string
    std::u32string ss = U"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::u32string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 9, 3), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U"ab\0", 10), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(U'\0', 12), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, FindFirstNotOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u32string ssDynamic = U"12341234";
    sdv::fixed_u32string<10> ssStatic1 = U"12";
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u32string<10> ssStatic = U"12341234";
    sdv::u32string ssDynamic1 = U"12";
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"12341234";
    sdv::fixed_u32string<10> ssSdvString2 = U"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::fixed_u32string<20>::npos);

    // Find C++ string in string
    std::u32string ss = U"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::fixed_u32string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", std::u32string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 2, 2), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 2), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0', 4), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastOfFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"12341234";
    sdv::u32string ssSdvString2 = U"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::u32string::npos);

    // Find C++ string in string
    std::u32string ss = U"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::u32string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", std::u32string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 2, 2), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(U"c\0", 2), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(U'\0', 4), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u32string ssDynamic = U"12341234";
    sdv::fixed_u32string<10> ssStatic1 = U"34";
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u32string<10> ssStatic = U"12341234";
    sdv::u32string ssDynamic1 = U"34";
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u32string<20> ssSdvString1 = U"12341234";
    sdv::fixed_u32string<10> ssSdvString2 = U"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::fixed_u32string<20>::npos);

    // Find C++ string in string
    std::u32string ss = U"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::fixed_u32string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u32string<20>(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", std::u32string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 2, 3), sdv::fixed_u32string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 2), sdv::fixed_u32string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 0), sdv::fixed_u32string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u32string ssSdvString1 = U"12341234";
    sdv::u32string ssSdvString2 = U"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::u32string::npos);

    // Find C++ string in string
    std::u32string ss = U"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::u32string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u32string(U"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", std::u32string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 2, 3), sdv::u32string::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U"ab\0", 2), sdv::u32string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(U'\0', 0), sdv::u32string::npos);
}

TEST_F(CUtf32StringTypeTest, FindLastNotOfFunctionMixed)
{
    // Find string in string - find static in dyanmic
    sdv::u32string ssDynamic = U"12341234";
    sdv::fixed_u32string<10> ssStatic1 = U"12";
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u32string<10> ssStatic = U"12341234";
    sdv::u32string ssDynamic1 = U"12";
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf32StringTypeTest, SumOperatorStatic)
{
    sdv::fixed_u32string<20>ssSdvString = U"1234";
    std::u32string ss = U"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, U"12341234");
    EXPECT_EQ(ss + ssSdvString, U"56781234");
    EXPECT_EQ(ssSdvString + ss, U"12345678");
    EXPECT_EQ(U"abcd" + ssSdvString, U"abcd1234");
    EXPECT_EQ(ssSdvString + U"abcd", U"1234abcd");
    EXPECT_EQ(U'a' + ssSdvString, U"a1234");
    EXPECT_EQ(ssSdvString + U'a', U"1234a");
    sdv::fixed_u32string<20>ssSdvString1 = U"1234";
    sdv::fixed_u32string<10>ssSdvString2 = U"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    ssSdvString2 = U"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + U"abcd", U"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + U'a', U"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), U"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), U"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(U"abcd" + std::move(ssSdvString1), U"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(U'a' + std::move(ssSdvString1), U"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf32StringTypeTest, SumOperatorDynamic)
{
    sdv::u32string ssSdvString = U"1234";
    std::u32string ss = U"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, U"12341234");
    EXPECT_EQ(ss + ssSdvString, U"56781234");
    EXPECT_EQ(ssSdvString + ss, U"12345678");
    EXPECT_EQ(U"abcd" + ssSdvString, U"abcd1234");
    EXPECT_EQ(ssSdvString + U"abcd", U"1234abcd");
    EXPECT_EQ(U'a' + ssSdvString, U"a1234");
    EXPECT_EQ(ssSdvString + U'a', U"1234a");
    sdv::u32string ssSdvString1 = U"1234", ssSdvString2 = U"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    ssSdvString2 = U"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, U"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + U"abcd", U"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(std::move(ssSdvString1) + U'a', U"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), U"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), U"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(U"abcd" + std::move(ssSdvString1), U"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = U"1234";
    EXPECT_EQ(U'a' + std::move(ssSdvString1), U"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf32StringTypeTest, SumOperatorMixed)
{
    // Add strings - add static onto dynamic
    sdv::u32string ssDynamic = U"1234";
    sdv::fixed_u32string<20> ssStatic1 = U"5678";
    EXPECT_EQ(ssDynamic + ssStatic1, U"12345678");
    EXPECT_EQ(std::move(ssDynamic) + std::move(ssStatic1), U"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_TRUE(ssStatic1.empty());
    ssDynamic = U"1234";
    ssStatic1 = U"5678";
    EXPECT_EQ(std::move(ssDynamic) + ssStatic1, U"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());
    ssDynamic = U"1234";
    EXPECT_EQ(ssStatic1 + std::move(ssDynamic), U"56781234");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());

    // Add strings - add dynamic onto static
    sdv::fixed_u32string<20> ssStatic = U"1234";
    sdv::u32string ssDynamic1 = U"5678";
    EXPECT_EQ(ssStatic + ssDynamic1, U"12345678");
    EXPECT_EQ(std::move(ssStatic) + std::move(ssDynamic1), U"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_TRUE(ssDynamic1.empty());
    ssStatic = U"1234";
    ssDynamic1 = U"5678";
    EXPECT_EQ(std::move(ssStatic) + ssDynamic1, U"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
    ssStatic = U"1234";
    EXPECT_EQ(ssDynamic1 + std::move(ssStatic), U"56781234");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
}

#ifdef __GNUC__
// Disabling this test, since it is not supported by the GCC library.
TEST_F(CUtf32StringTypeTest, DISABLED_StreamString)
#else
TEST_F(CUtf32StringTypeTest, StreamString)
#endif
{
    std::basic_stringstream<char32_t> sstream;
    sdv::u32string ss = U"1234";
    sstream << ss;
    EXPECT_EQ(sstream.str(), U"1234");
    ss.clear();
    EXPECT_TRUE(ss.empty());
    sstream >> ss;
    EXPECT_EQ(ss, U"1234");
}

TEST_F(CUtf32StringTypeTest, StringGetLine)
{
    std::basic_istringstream<char32_t> sstream;
    sstream.str(U"0;1;2;3");
    int nIndex = 0;
    for (sdv::u32string ss; sdv::getline(sstream, ss, U';');)
        EXPECT_EQ(*ss.c_str(), U'0' + static_cast<char32_t>(nIndex++));
    EXPECT_EQ(sstream.str(), U"0;1;2;3");

    sstream.str(U"0\n1\n2\n3");
    nIndex = 0;
    for (sdv::u32string ss; sdv::getline(std::move(sstream), ss);)
        EXPECT_EQ(*ss.c_str(), U'0' + static_cast<char32_t>(nIndex++));
}

TEST_F(CUtf32StringTypeTest, MakeString)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf32String(ssAnsi), U"Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssWide), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf8), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf16), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf32), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssAnsi.c_str()), U"Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssWide.c_str()), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf8.c_str()), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf16.c_str()), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssUtf32.c_str()), U"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf32String(ssCppAnsi), U"Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssCppWide), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssCppUtf8), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssCppUtf16), U"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf32String(ssCppUtf32), U"Hello\U00024B62Hello");
}

TEST_F(CUtf32StringTypeTest, MakeStringGeneric)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    std::u32string ssDst;
    ssDst = sdv::MakeString<char, false, 0, char32_t, true, 0>(ssAnsi);
    EXPECT_EQ(ssDst, U"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char32_t, true, 0>(ssWide);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char32_t, true, 0>(ssUtf8);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char32_t, true, 0>(ssUtf16);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char32_t, true, 0>(ssUtf32);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, false, 0, char32_t, true, 0>(ssAnsi.c_str());
    EXPECT_EQ(ssDst, U"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char32_t, true, 0>(ssWide.c_str());
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char32_t, true, 0>(ssUtf8.c_str());
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char32_t, true, 0>(ssUtf16.c_str());
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char32_t, true, 0>(ssUtf32.c_str());
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    ssDst = sdv::MakeString<char, char32_t, true, 0>(ssCppAnsi);
    EXPECT_EQ(ssDst, U"Hello");
    ssDst = sdv::MakeString<wchar_t, char32_t, true, 0>(ssCppWide);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, char32_t, true, 0>(ssCppUtf8);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, char32_t, true, 0>(ssCppUtf16);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, char32_t, true, 0>(ssCppUtf32);
    EXPECT_EQ(ssDst, U"Hello\U00024B62Hello");
}

TEST_F(CUtf32StringTypeTest, IteratorDistance)
{
    sdv::u32string ss = U"This is a text";
    EXPECT_EQ(std::distance(ss.begin(), ss.end()), ss.length());
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
