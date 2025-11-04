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

using CUtf16StringTypeTest = CBasicTypesTest;

TEST_F(CUtf16StringTypeTest, ConstructorStatic)
{
    // Empty string
    sdv::fixed_u16string<10> ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::fixed_u16string<10> ssCString(u"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");
    std::u16string ss(u"Hello");
    sdv::fixed_u16string<10> ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, u"Hello");

    // Copy constructor
    sdv::fixed_u16string<8> ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");

    // Move constructor
    sdv::fixed_u16string<12> ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");

    // Fill constructor
    sdv::fixed_u16string<20> ssFillString(10, u'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, u"**********");

    // Substring constructor
    sdv::fixed_u16string<20> ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, u"llo");
    sdv::fixed_u16string<21> ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, u"ll");
    sdv::fixed_u16string<22> ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, u"llo");
    sdv::fixed_u16string<23> ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, u"ll");
    sdv::fixed_u16string<24> ssSubstring5(u"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, u"He\0l", 4), 0);

    // Iterator based construction
    sdv::fixed_u16string<10> ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, u"Hello");
    sdv::fixed_u16string<10> ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, u"Hello");

    // Construct using initializer list
    sdv::fixed_u16string<10> ssInitListString{u'H', u'e', u'l', u'l', u'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, u"Hello");
}

TEST_F(CUtf16StringTypeTest, ConstructorDynamic)
{
    // Empty string
    sdv::u16string ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::u16string ssCString(u"Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");
    std::u16string ss(u"Hello");
    sdv::u16string ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, u"Hello");

    // Copy constructor
    sdv::u16string ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");

    // Move constructor
    sdv::u16string ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, u"Hello");

    // Fill constructor
    sdv::u16string ssFillString(10, u'*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, u"**********");

    // Substring constructor
    sdv::u16string ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, u"llo");
    sdv::u16string ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, u"ll");
    sdv::u16string ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, u"llo");
    sdv::u16string ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, u"ll");
    sdv::u16string ssSubstring5(u"He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, u"He\0l", 4), 0);

    // Iterator based construction
    sdv::u16string ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, u"Hello");
    sdv::u16string ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, u"Hello");

    // Construct using initializer list
    sdv::u16string ssInitListString{u'H', u'e', u'l', u'l', u'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, u"Hello");
}

TEST_F(CUtf16StringTypeTest, ConstructorMixed)
{
    // Copy constructor
    sdv::u16string ssDynamic = u"dynamic";
    sdv::fixed_u16string<8> ssStaticCopy(ssDynamic);
    EXPECT_EQ(ssStaticCopy, u"dynamic");
    sdv::fixed_u16string<8> ssStatic = u"static";
    sdv::u16string ssDynamicCopy(ssStatic);
    EXPECT_EQ(ssDynamicCopy, u"static");

    // Move constructor
    sdv::fixed_u16string<8> ssStaticMove(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u"dynamic");
    sdv::u16string ssDynamicMove(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u"static");
}

TEST_F(CUtf16StringTypeTest, AssignmentOperatorStatic)
{
    // SDV-String assignment
    sdv::fixed_u16string<10> ssSdvString1(u"Hello");
    sdv::fixed_u16string<15> ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, u"Hello");

    // Move assignment
    sdv::fixed_u16string<10> ssSdvString3(u"Hello");
    sdv::fixed_u16string<15> ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, u"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::u16string ss1(u"Hello");
    sdv::fixed_u16string<10> ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, u"Hello");

    // C string assignment
    const char16_t sz1[] = u"Hello";
    sdv::fixed_u16string<10> ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, u"Hello");
    const char16_t* sz2 = u"Hello";
    sdv::fixed_u16string<10>	ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, u"Hello");

    // Initializer list
    sdv::fixed_u16string<10> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = u"Hello";
    EXPECT_EQ(ssSdvString8, u"Hello");
    sdv::fixed_u16string<10> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {u'H', u'e', u'l', u'l', u'o'};
    EXPECT_EQ(ssSdvString9, u"Hello");
}

TEST_F(CUtf16StringTypeTest, AssignmentOperatorDynamic)
{
    // SDV-String assignment
    sdv::u16string ssSdvString1(u"Hello");
    sdv::u16string ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, u"Hello");

    // Move assignment
    sdv::u16string ssSdvString3(u"Hello");
    sdv::u16string ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, u"Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::u16string ss1(u"Hello");
    sdv::u16string ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, u"Hello");

    // C string assignment
    const char16_t sz1[] = u"Hello";
    sdv::u16string ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, u"Hello");
    const char16_t* sz2 = u"Hello";
    sdv::u16string ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, u"Hello");

    // Initializer list
    sdv::u16string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = u"Hello";
    EXPECT_EQ(ssSdvString8, u"Hello");
    sdv::u16string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {u'H', u'e', u'l', u'l', u'o'};
    EXPECT_EQ(ssSdvString9, u"Hello");
}

TEST_F(CUtf16StringTypeTest, AssignmentOperatorMixed)
{
    // Copy assignment
    sdv::u16string ssDynamic = u"dynamic";
    sdv::fixed_u16string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy = ssDynamic;
    EXPECT_EQ(ssStaticCopy, u"dynamic");
    sdv::fixed_u16string<8> ssStatic = u"static";
    sdv::u16string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy = ssStatic;
    EXPECT_EQ(ssDynamicCopy, u"static");

    // Move assignment
    sdv::fixed_u16string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove = std::move(ssDynamic);
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u"dynamic");
    sdv::u16string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove = std::move(ssStatic);
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u"static");
}

TEST_F(CUtf16StringTypeTest, AssignmentFunctionStatic)
{
    // Character assignment
    sdv::fixed_u16string<10> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");

    // String copy assignment
    sdv::fixed_u16string<10> ssSdvString2;
    sdv::fixed_u16string<15> ssSdvString3(u"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");

    // C++ string assignment
    sdv::fixed_u16string<10> ssSdvString4;
    std::u16string ss(u"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");

    // C string assignment
    sdv::fixed_u16string<10> ssSdvString5;
    const char16_t sz1[] = u"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    sdv::fixed_u16string<10>	ssSdvString6;
    const char16_t* sz2 = u"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, u"Hello");

    // Substring assignment
    sdv::fixed_u16string<10> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    sdv::fixed_u16string<11> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, u"ell");
    sdv::fixed_u16string<12> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, u"ell");

    // Move assignment
    sdv::fixed_u16string<10> ssSdvString10;
    sdv::fixed_u16string<15> ssSdvString11(u"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, u"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::fixed_u16string<10> ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, u"Hello");

    // Initializer list
    sdv::fixed_u16string<10> ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({u'H', u'e', u'l', u'l', u'o'});
    EXPECT_EQ(ssSdvString13, u"Hello");
}

TEST_F(CUtf16StringTypeTest, AssignmentFunctionDynamic)
{
    // Character assignment
    sdv::u16string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");

    // String copy assignment
    sdv::u16string ssSdvString2;
    sdv::u16string ssSdvString3(u"Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");

    // C++ string assignment
    sdv::u16string ssSdvString4;
    std::u16string ss(u"Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");

    // C string assignment
    sdv::u16string ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    sdv::u16string ssSdvString6;
    const char16_t* sz2 = u"Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, u"Hello");

    // Substring assignment
    sdv::u16string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    sdv::u16string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, u"ell");
    sdv::u16string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, u"ell");

    // Move assignment
    sdv::u16string ssSdvString10;
    sdv::u16string ssSdvString11(u"Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, u"Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::u16string ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, u"Hello");

    // Initializer list
    sdv::u16string ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({u'H', u'e', u'l', u'l', u'o'});
    EXPECT_EQ(ssSdvString13, u"Hello");
}

TEST_F(CUtf16StringTypeTest, AssignmentFunctionMixed)
{
    // Copy assignment
    sdv::u16string ssDynamic = u"dynamic";
    sdv::fixed_u16string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy.assign(ssDynamic);
    EXPECT_EQ(ssStaticCopy, u"dynamic");
    sdv::fixed_u16string<8> ssStatic = u"static";
    sdv::u16string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy.assign(ssStatic);
    EXPECT_EQ(ssDynamicCopy, u"static");

    // Move assignment
    sdv::fixed_u16string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove.assign(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u"dynamic");
    sdv::u16string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove.assign(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u"static");

    // Substring assignment
    sdv::fixed_u16string<10> ssStaticSubstring;
    EXPECT_TRUE(ssStaticSubstring.empty());
    ssStaticSubstring.assign(ssDynamicMove, 1, 3);
    EXPECT_EQ(ssStaticSubstring, u"tat");
    sdv::u16string ssDynamicSubstring;
    EXPECT_TRUE(ssDynamicSubstring.empty());
    ssDynamicSubstring.assign(ssStaticMove, 1, 3);
    EXPECT_EQ(ssDynamicSubstring, u"yna");
}

TEST_F(CUtf16StringTypeTest, PositionFunction)
{
    // Position in the string
    sdv::u16string ssSdvString1(u"Hello");
    EXPECT_EQ(ssSdvString1.at(0), u'H');
    EXPECT_EQ(ssSdvString1[0], u'H');
    EXPECT_EQ(ssSdvString1.front(), u'H');
    EXPECT_EQ(ssSdvString1.at(4), u'o');
    EXPECT_EQ(ssSdvString1[4], u'o');
    EXPECT_EQ(ssSdvString1.back(), u'o');
    EXPECT_THROW(ssSdvString1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString1[5], sdv::XIndexOutOfRange);

    // Empty string
    sdv::u16string ssSdvString2;
    EXPECT_THROW(ssSdvString2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.back(), sdv::XIndexOutOfRange);

    // Assignment
    ssSdvString1.at(0) = 'B';
    EXPECT_EQ(ssSdvString1, u"Bello");
    ssSdvString1[0] = 'D';
    EXPECT_EQ(ssSdvString1, u"Dello");
    ssSdvString1.front() = 'M';
    EXPECT_EQ(ssSdvString1, u"Mello");
    ssSdvString1.back() = 'k';
    EXPECT_EQ(ssSdvString1, u"Mellk");
}

TEST_F(CUtf16StringTypeTest, CAndCppStringAccess)
{
    // C++ cast operator
    sdv::u16string ssSdvString(u"Hello");
    std::u16string ss1(ssSdvString);
    EXPECT_EQ(ss1, u"Hello");
    std::u16string ss2;
    EXPECT_TRUE(ss2.empty());
    ss2 = ssSdvString;
    EXPECT_EQ(ss2, u"Hello");

    // Data access
    sdv::u16string ssSdvString2(u"He\0lo", 5);
    EXPECT_EQ(ssSdvString2.size(), 5);
    const char16_t* sz1 = ssSdvString2.data();
    EXPECT_EQ(memcmp(sz1, u"He\0lo", 5 * sizeof(char16_t)), 0);

    // C string access
    const char16_t* sz2 = ssSdvString.c_str();
    EXPECT_STREQ(sz2, u"Hello");
}

TEST_F(CUtf16StringTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::u16string::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u16string ssSdvString(u"Hello");
    EXPECT_NE(itEmpty, ssSdvString.begin());
    sdv::u16string::iterator itPos = ssSdvString.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.begin());
    sdv::u16string::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u16string::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, u'H');
    EXPECT_EQ(itPos[0], u'H');
    EXPECT_EQ(itPos[4], u'o');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'B';
    EXPECT_EQ(ssSdvString, u"Bello");
    itPos[4] = 'k';
    EXPECT_EQ(ssSdvString, u"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, u'e');
    itPos += 3;
    EXPECT_EQ(*itPos, u'o');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.end());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, u'o');
    itPos -= 4;
    EXPECT_EQ(*itPos, u'H');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, u'H');
    EXPECT_EQ(itPos, ssSdvString.begin());

    // Const iterator
    sdv::u16string::const_iterator itPosConst = ssSdvString.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, u'e');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.cbegin(), ssSdvString.begin());
    EXPECT_EQ(ssSdvString.begin(), ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString.cend(), ssSdvString.end());
    EXPECT_EQ(ssSdvString.end(), ssSdvString.cend());
}

TEST_F(CUtf16StringTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::u16string::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u16string ssSdvString(u"Hello");
    EXPECT_NE(itEmpty, ssSdvString.rbegin());
    sdv::u16string::reverse_iterator itPos = ssSdvString.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.rbegin());
    sdv::u16string::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u16string::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, u'o');
    EXPECT_EQ(itPos[0], u'o');
    EXPECT_EQ(itPos[4], u'H');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'k';
    EXPECT_EQ(ssSdvString, u"Hellk");
    itPos[4] = 'B';
    EXPECT_EQ(ssSdvString, u"Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, u'l');
    itPos += 3;
    EXPECT_EQ(*itPos, u'H');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.rend());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, u'H');
    itPos -= 4;
    EXPECT_EQ(*itPos, u'o');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, u'o');
    EXPECT_EQ(itPos, ssSdvString.rbegin());

    // Const iterator
    sdv::u16string::const_reverse_iterator itPosConst = ssSdvString.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, u'l');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.crbegin(), ssSdvString.rbegin());
    EXPECT_EQ(ssSdvString.rbegin(), ssSdvString.crbegin());
    EXPECT_EQ(ssSdvString.crend(), ssSdvString.rend());
    EXPECT_EQ(ssSdvString.rend(), ssSdvString.crend());
}

TEST_F(CUtf16StringTypeTest, StringCapacityStatic)
{
    // Empty string
    sdv::fixed_u16string<10> ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Filled string
    ssSdvString = u"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, u"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString += 'o';
    ssSdvString.resize(10, u'*');
    EXPECT_EQ(ssSdvString, u"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, u"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
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

TEST_F(CUtf16StringTypeTest, StringCapacityDynamic)
{
    // Empty string
    sdv::u16string ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 0);

    // Filled string
    ssSdvString = u"Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, u"Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 5);
    ssSdvString += 'o';
    ssSdvString.resize(10, u'*');
    EXPECT_EQ(ssSdvString, u"Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, u"Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), u"Hello");
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

TEST_F(CUtf16StringTypeTest, InsertFunctionStatic)
{
    // Character assignment
    sdv::fixed_u16string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");
    ssSdvString1.insert(0, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********??");
    ssSdvString1.insert(std::u16string::npos, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********????");
    ssSdvString1.insert(4, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**??********????");

    // C string assignment
    sdv::fixed_u16string<20> ssSdvString5;
    const char16_t sz1[] = u"Hello";
    const char16_t* sz2 = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::u16string::npos, sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, u"**Hello");
    ssSdvString5.insert(std::u16string::npos, sz2);
    EXPECT_EQ(ssSdvString5, u"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, u"**He**llo**");

    // String copy assignment
    sdv::fixed_u16string<20> ssSdvString2;
    sdv::fixed_u16string<20> ssSdvString3(u"Hello");
    sdv::fixed_u16string<20> ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**Hello");
    ssSdvString2.insert(std::u16string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**He**llo**");

    // C++ string assignment
    sdv::fixed_u16string<20> ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, u"**Hello");
    ssSdvString4.insert(std::u16string::npos, ss2);
    EXPECT_EQ(ssSdvString4, u"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, u"**He**llo**");

    // Substring assignment
    sdv::fixed_u16string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, u"eellll");
    ssSdvString7.insert(std::u16string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, u"eellllell");

    // Iterator assignment
    sdv::fixed_u16string<20> ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), u'+');
    EXPECT_EQ(ssSdvString12, u"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), u'-');
    EXPECT_EQ(ssSdvString12, u"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), u'+');
    EXPECT_EQ(ssSdvString12, u"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), u'-');
    EXPECT_EQ(ssSdvString12, u"-+Hello+-");
    sdv::fixed_u16string<20>::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, u'#');
    EXPECT_EQ(ssSdvString12, u"-#+Hello+-");
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, u'#');
    EXPECT_EQ(ssSdvString12, u"-##-++Hello++--");
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, u'#');
    EXPECT_EQ(ssSdvString12, u"-##-++Hello++--");
    ssSdvString12 = u"Hello";
    sdv::fixed_u16string<20> ssSdvString8 = u"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"112212Hello1212");

    // Initializer list
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"112212Hello1212");
}

TEST_F(CUtf16StringTypeTest, InsertFunctionDynamic)
{
    // Character assignment
    sdv::u16string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");
    ssSdvString1.insert(0, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********??");
    ssSdvString1.insert(std::u16string::npos, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**********????");
    ssSdvString1.insert(4, 2, u'?');
    EXPECT_EQ(ssSdvString1, u"??**??********????");

    // C string assignment
    sdv::u16string ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    const char16_t* sz2	  = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::u16string::npos, sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, u"**Hello");
    ssSdvString5.insert(std::u16string::npos, sz2);
    EXPECT_EQ(ssSdvString5, u"**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, u"**He**llo**");

    // String copy assignment
    sdv::u16string ssSdvString2;
    sdv::u16string ssSdvString3(u"Hello");
    sdv::u16string ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**Hello");
    ssSdvString2.insert(std::u16string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"**He**llo**");

    // C++ string assignment
    sdv::u16string ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, u"**Hello");
    ssSdvString4.insert(std::u16string::npos, ss2);
    EXPECT_EQ(ssSdvString4, u"**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, u"**He**llo**");

    // Substring assignment
    sdv::u16string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, u"eellll");
    ssSdvString7.insert(std::u16string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, u"eellllell");

    // Iterator assignment
    sdv::u16string ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), u'+');
    EXPECT_EQ(ssSdvString12, u"+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), u'-');
    EXPECT_EQ(ssSdvString12, u"-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), u'+');
    EXPECT_EQ(ssSdvString12, u"-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), u'-');
    EXPECT_EQ(ssSdvString12, u"-+Hello+-");
    sdv::u16string::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, u'#');
    EXPECT_EQ(ssSdvString12, u"-#+Hello+-");
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, u'#');
    EXPECT_EQ(ssSdvString12, u"-##-++Hello++--");
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, u'+');
    EXPECT_EQ(ssSdvString12, u"--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, u'-');
    EXPECT_EQ(ssSdvString12, u"--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, u'#');
    EXPECT_EQ(ssSdvString12, u"-##-++Hello++--");
    ssSdvString12			 = u"Hello";
    sdv::u16string ssSdvString8 = u"12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, u"112212Hello1212");

    // Initializer list
    ssSdvString12 = u"Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"112212Hello1212");
}

TEST_F(CUtf16StringTypeTest, InsertFunctionMixed)
{
    // String copy assignment - static into dynamic
    sdv::u16string ssDynamic;
    sdv::fixed_u16string<20> ssStatic1(u"Hello");
    sdv::fixed_u16string<20> ssStatic2(u"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(3, ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u"Hello");
    ssDynamic.insert(0, ssStatic2);
    EXPECT_EQ(ssDynamic, u"**Hello");
    ssDynamic.insert(sdv::u16string::npos, ssStatic2);
    EXPECT_EQ(ssDynamic, u"**Hello**");
    ssDynamic.insert(4, ssStatic2);
    EXPECT_EQ(ssDynamic, u"**He**llo**");

    // String copy assignment - dynamic into static
    sdv::fixed_u16string<20> ssStatic;
    sdv::fixed_u16string<20> ssDynamic1(u"Hello");
    sdv::fixed_u16string<20> ssDynamic2(u"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(3, ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u"Hello");
    ssStatic.insert(0, ssDynamic2);
    EXPECT_EQ(ssStatic, u"**Hello");
    ssStatic.insert(sdv::u16string::npos, ssDynamic2);
    EXPECT_EQ(ssStatic, u"**Hello**");
    ssStatic.insert(4, ssDynamic2);
    EXPECT_EQ(ssStatic, u"**He**llo**");

    // Substring assignment - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(0, ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, u"ell");

    // Substring assignment - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(0, ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, u"ell");
}

TEST_F(CUtf16StringTypeTest, EraseFunction)
{
    // Erase index based
    sdv::u16string ssSdvString = u"12345678901234567890";
    ssSdvString.erase(8, 5);
    EXPECT_EQ(ssSdvString, u"123456784567890");
    ssSdvString.erase(0, 2);
    EXPECT_EQ(ssSdvString, u"3456784567890");
    ssSdvString.erase(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString, u"3456784567890");
    ssSdvString.erase(sdv::u16string::npos);
    EXPECT_EQ(ssSdvString, u"3456784567890");
    ssSdvString.erase(8);
    EXPECT_EQ(ssSdvString, u"34567845");

    // Erase iterator
    ssSdvString = u"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString, u"2345678901234567890");
    ssSdvString.erase(ssSdvString.cend());
    EXPECT_EQ(ssSdvString, u"2345678901234567890");
    ssSdvString.erase(ssSdvString.cbegin() + 5);
    EXPECT_EQ(ssSdvString, u"234568901234567890");
    sdv::u16string::iterator itPos = ssSdvString.erase(ssSdvString.end() - 2);
    EXPECT_EQ(ssSdvString, u"23456890123456780");
    EXPECT_EQ(*itPos, u'0');
    ++itPos;
    EXPECT_EQ(itPos, ssSdvString.cend());

    // Erase iterator range
    ssSdvString = u"12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin(), ssSdvString.cbegin() + 4);
    EXPECT_EQ(ssSdvString, u"5678901234567890");
    itPos = ssSdvString.erase(ssSdvString.cbegin() + 4, ssSdvString.cbegin() + 6);
    EXPECT_EQ(ssSdvString, u"56781234567890");
    EXPECT_EQ(*itPos, u'1');
    itPos += 10;
    EXPECT_EQ(itPos, ssSdvString.cend());
}

TEST_F(CUtf16StringTypeTest, PushPopFunctions)
{
    sdv::u16string ssSdvString;
    ssSdvString.push_back(u'1');
    EXPECT_EQ(ssSdvString, u"1");
    ssSdvString.push_back(u'2');
    EXPECT_EQ(ssSdvString, u"12");
    ssSdvString.pop_back();
    EXPECT_EQ(ssSdvString, u"1");
    ssSdvString.pop_back();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_NO_THROW(ssSdvString.pop_back());
}

TEST_F(CUtf16StringTypeTest, AppendFunctionStatic)
{
    // Append characters
    sdv::fixed_u16string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");
    ssSdvString1.append(2, u'?');
    EXPECT_EQ(ssSdvString1, u"**********??");

    // Append string
    sdv::fixed_u16string<20> ssSdvString2;
    sdv::fixed_u16string<10> ssSdvString3(u"Hello");
    sdv::fixed_u16string<15> ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"Hello**");

    // Append C++ string
    sdv::fixed_u16string<20> ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, u"Hello**");

    // Append substring
    sdv::fixed_u16string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ellell");

    // C string assignment
    sdv::fixed_u16string<20> ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    const char16_t* sz2	  = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, u"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, u"Hello**Hello");

    // Iterator based
    sdv::fixed_u16string<20> ssSdvString12 = u"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, u"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = u"Hello";
    ssSdvString12.append({u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"Hello12");
}

TEST_F(CUtf16StringTypeTest, AppendFunctionDynamic)
{
    // Append characters
    sdv::u16string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, u'*');
    EXPECT_EQ(ssSdvString1, u"**********");
    ssSdvString1.append(2, u'?');
    EXPECT_EQ(ssSdvString1, u"**********??");

    // Append string
    sdv::u16string ssSdvString2;
    sdv::u16string ssSdvString3(u"Hello");
    sdv::u16string ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, u"Hello**");

    // Append C++ string
    sdv::u16string ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, u"Hello**");

    // Append substring
    sdv::u16string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, u"ellell");

    // C string assignment
    sdv::u16string ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    const char16_t* sz2	  = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, u"Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, u"Hello**Hello");

    // Iterator based
    sdv::u16string ssSdvString12 = u"Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, u"Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = u"Hello";
    ssSdvString12.append({u'1', u'2'});
    EXPECT_EQ(ssSdvString12, u"Hello12");
}

TEST_F(CUtf16StringTypeTest, AppendFunctionMixed)
{
    // Append string - static into dynamic
    sdv::u16string ssDynamic;
    sdv::fixed_u16string<10> ssStatic1(u"Hello");
    sdv::fixed_u16string<15> ssStatic2(u"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u"Hello");
    ssDynamic.append(ssStatic2);
    EXPECT_EQ(ssDynamic, u"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u16string<20> ssStatic;
    sdv::u16string ssDynamic1(u"Hello");
    sdv::u16string ssDynamic2(u"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u"Hello");
    ssStatic.append(ssDynamic2);
    EXPECT_EQ(ssStatic, u"Hello**");

    // Append substring - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, u"ell");

    // Append substring - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, u"ell");
}

TEST_F(CUtf16StringTypeTest, AppendOperatorStatic)
{
    // Append string
    sdv::fixed_u16string<20> ssSdvString2;
    sdv::fixed_u16string<15> ssSdvString3(u"Hello");
    sdv::fixed_u16string<10> ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, u"Hello**");

    // Append C++ string
    sdv::fixed_u16string<20> ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, u"Hello**");

    // Append character
    sdv::fixed_u16string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, u"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, u"*?");

    // C string assignment
    sdv::fixed_u16string<20>	ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    const char16_t* sz2	  = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, u"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, u"Hello**Hello");

    // Initializer list
    sdv::fixed_u16string<20> ssSdvString12 = u"Hello";
    ssSdvString12 += {u'1', u'2'};
    EXPECT_EQ(ssSdvString12, u"Hello12");
}

TEST_F(CUtf16StringTypeTest, AppendOperatorDynamic)
{
    // Append string
    sdv::u16string ssSdvString2;
    sdv::u16string ssSdvString3(u"Hello");
    sdv::u16string ssSdvString6(u"**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, u"Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, u"Hello**");

    // Append C++ string
    sdv::u16string ssSdvString4;
    std::u16string ss(u"Hello");
    std::u16string ss2(u"**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, u"Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, u"Hello**");

    // Append character
    sdv::u16string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, u"*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, u"*?");

    // C string assignment
    sdv::u16string ssSdvString5;
    const char16_t	sz1[] = u"Hello";
    const char16_t* sz2	  = u"**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, u"Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, u"Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, u"Hello**Hello");

    // Initializer list
    sdv::u16string ssSdvString12 = u"Hello";
    ssSdvString12 += {u'1', u'2'};
    EXPECT_EQ(ssSdvString12, u"Hello12");
}

TEST_F(CUtf16StringTypeTest, AppendOperatorMixed)
{
    // Append string - static into dynamic
    sdv::u16string ssDynamic;
    sdv::fixed_u16string<15> ssStatic1(u"Hello");
    sdv::fixed_u16string<10> ssStatic2(u"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic += ssStatic1;
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u"Hello");
    ssDynamic += ssStatic2;
    EXPECT_EQ(ssDynamic, u"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u16string<20> ssStatic;
    sdv::u16string ssDynamic1(u"Hello");
    sdv::u16string ssDynamic2(u"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic += ssDynamic1;
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u"Hello");
    ssStatic += ssStatic2;
    EXPECT_EQ(ssStatic, u"Hello**");
}

TEST_F(CUtf16StringTypeTest, CompareFunctionStatic)
{
    // Compare strings
    sdv::fixed_u16string<20> ssSdvString1;
    sdv::fixed_u16string<10> ssSdvString2(u"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u16string ssString(u"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char16_t* sz1 = u"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char16_t sz2[] = u"Hello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = u"Kello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssSdvString2, 1, sdv::fixed_u16string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = u"Kello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString, 1, sdv::fixed_u16string<20>::npos), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, ssString, 1, sdv::fixed_u16string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = u"Kellogg";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char16_t sz3[] = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz3), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz3), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz3), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz3), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char16_t sz4[] = u"Kellogg";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u16string<20>::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf16StringTypeTest, CompareFunctionDynamic)
{
    // Compare strings
    sdv::u16string ssSdvString1;
    sdv::u16string ssSdvString2(u"Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u16string ssString(u"Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char16_t* sz1 = u"Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char16_t sz2[] = u"Hello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = u"Kello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssSdvString2, 1, sdv::u16string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssString), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssString), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, ssString), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssString), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = u"Kello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssString, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, ssString, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, ssString, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssString, 1, sdv::u16string::npos), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, ssString, 1, sdv::u16string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz1), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz1), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, sz1), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz1), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = u"Kellogg";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char16_t sz3[] = u"ello";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz3), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz3), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, sz3), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz3), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char16_t sz4[] = u"Kellogg";
    ssSdvString1 = u"Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u16string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u16string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = u"Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u16string::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf16StringTypeTest, CompareFunctionMixed)
{
    // Compare strings - compare dynamic with static
    sdv::u16string ssDynamic;
    sdv::fixed_u16string<10> ssStatic1(u"Hello");
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = u"Helln";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = u"HellnO";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = u"Hello";
    EXPECT_EQ(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = u"Hellp";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = u"Helloa";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);

    // Compare substring
    ssStatic1 = u"ello";
    ssDynamic = u"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1), 0);
    ssDynamic = u"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1), 0);
    ssDynamic = u"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1), 0);
    ssDynamic = u"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1), 0);
    ssDynamic = u"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1), 0);
    ssStatic1 = u"Kello";
    ssDynamic = u"Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssDynamic = u"HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssDynamic = u"Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssDynamic = u"Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssDynamic = u"Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u16string<20>::npos, ssStatic1, 1, sdv::fixed_u16string<20>::npos), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1, 1, 4), 0);

    // Compare strings - compare static with dynamic
    sdv::fixed_u16string<10> ssStatic;
    sdv::u16string ssDynamic1(u"Hello");
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = u"Helln";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = u"HellnO";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = u"Hello";
    EXPECT_EQ(ssStatic.compare(ssDynamic1), 0);
    ssStatic = u"Hellp";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = u"Helloa";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);

    // Compare substring
    ssDynamic1 = u"ello";
    ssStatic = u"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1), 0);
    ssStatic = u"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1), 0);
    ssStatic = u"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1), 0);
    ssStatic = u"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1), 0);
    ssStatic = u"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1), 0);
    ssDynamic1 = u"Kello";
    ssStatic = u"Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssStatic = u"HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssStatic = u"Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssStatic = u"Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1, 1, sdv::fixed_u16string<20>::npos), 0);
    ssStatic = u"Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u16string<20>::npos, ssDynamic1, 1, sdv::fixed_u16string<20>::npos), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1, 1, 4), 0);
}

TEST_F(CUtf16StringTypeTest, CompareOperatorStatic)
{
    // Compare strings
    sdv::fixed_u16string<20> ssSdvString1;
    sdv::fixed_u16string<10> ssSdvString2(u"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u16string ssString(u"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char16_t* sz1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char16_t sz2[] = u"Hello";
    ssSdvString1		 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf16StringTypeTest, CompareOperatorDynamic)
{
    // Compare strings
    sdv::u16string ssSdvString1;
    sdv::u16string ssSdvString2(u"Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::u16string ssString(u"Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char16_t* sz1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char16_t sz2[] = u"Hello";
    ssSdvString1	 = u"Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = u"Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = u"Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf16StringTypeTest, CompareOperatorMixed)
{
    // Compare strings - compare dynamic with static
    sdv::u16string ssDynamic;
    sdv::fixed_u16string<10> ssStatic1(u"Hello");
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = u"Helln";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = u"HellnO";
    EXPECT_TRUE(ssDynamic < ssStatic1);
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = u"Hello";
    EXPECT_TRUE(ssDynamic <= ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic == ssStatic1);
    ssDynamic = u"Hellp";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);
    ssDynamic = u"Helloa";
    EXPECT_TRUE(ssDynamic > ssStatic1);
    EXPECT_TRUE(ssDynamic >= ssStatic1);
    EXPECT_TRUE(ssDynamic != ssStatic1);

    // Compare strings - compare static with dynamic
    sdv::fixed_u16string<10> ssStatic;
    sdv::u16string ssDynamic1(u"Hello");
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = u"Helln";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = u"HellnO";
    EXPECT_TRUE(ssStatic < ssDynamic1);
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = u"Hello";
    EXPECT_TRUE(ssStatic <= ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic == ssDynamic1);
    ssStatic = u"Hellp";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
    ssStatic = u"Helloa";
    EXPECT_TRUE(ssStatic > ssDynamic1);
    EXPECT_TRUE(ssStatic >= ssDynamic1);
    EXPECT_TRUE(ssStatic != ssDynamic1);
}

TEST_F(CUtf16StringTypeTest, ReplaceFunctionStatic)
{
    // Replace string
    sdv::fixed_u16string<20> ssSdvString = u"1234567890";
    sdv::fixed_u16string<10> ssSdvString2 = u"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString	 = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace C++ string
    ssSdvString	 = u"1234567890";
    std::u16string ss = u"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ss);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace C string
    ssSdvString	 = u"1234567890";
    const char16_t sz1[] = u"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz1);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString		 = u"1234567890";
    const char16_t* sz2 = u"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz2);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace substring
    ssSdvString	 = u"1234567890";
    ssSdvString2 = u"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = u"1234567890";
    ss = u"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = u"1234567890";
    const char16_t sz3[] = u"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    sz2 = u"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace with characters
    ssSdvString = u"1234567890";
    ssSdvString.replace(1, 2, 4, u'+');
    EXPECT_EQ(ssSdvString, u"1++++4567890");
    ssSdvString.replace(2, 6, 4, u'-');
    EXPECT_EQ(ssSdvString, u"1+----7890");
    ssSdvString.replace(3, 4, 4, u'*');
    EXPECT_EQ(ssSdvString, u"1+-****890");
    ssSdvString.replace(0, 0, 4, u'#');
    EXPECT_EQ(ssSdvString, u"####1+-****890");
    ssSdvString.replace(std::u16string::npos, 100, 4, u'~');
    EXPECT_EQ(ssSdvString, u"####1+-****890~~~~");
    ssSdvString.replace(0, std::u16string::npos, 4, u'=');
    EXPECT_EQ(ssSdvString, u"====");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, u'+');
    EXPECT_EQ(ssSdvString, u"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, u'-');
    EXPECT_EQ(ssSdvString, u"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, u'*');
    EXPECT_EQ(ssSdvString, u"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, u'#');
    EXPECT_EQ(ssSdvString, u"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, u'~');
    EXPECT_EQ(ssSdvString, u"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, u'=');
    EXPECT_EQ(ssSdvString, u"====");

    // Replace with iterators
    ssSdvString = u"1234567890";
    ss = u"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace with initialization list
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd");
}

TEST_F(CUtf16StringTypeTest, ReplaceFunctionDynamic)
{
    // Replace string
    sdv::u16string ssSdvString = u"1234567890";
    sdv::u16string ssSdvString2 = u"abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString	 = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace C++ string
    ssSdvString	 = u"1234567890";
    std::u16string ss = u"abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ss);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace C string
    ssSdvString	 = u"1234567890";
    const char16_t sz1[] = u"abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz1);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString		 = u"1234567890";
    const char16_t* sz2 = u"abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz2);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace substring
    ssSdvString	 = u"1234567890";
    ssSdvString2 = u"xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = u"1234567890";
    ss = u"xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = u"1234567890";
    const char16_t sz3[] = u"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    sz2 = u"abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(std::u16string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::u16string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace with characters
    ssSdvString = u"1234567890";
    ssSdvString.replace(1, 2, 4, u'+');
    EXPECT_EQ(ssSdvString, u"1++++4567890");
    ssSdvString.replace(2, 6, 4, u'-');
    EXPECT_EQ(ssSdvString, u"1+----7890");
    ssSdvString.replace(3, 4, 4, u'*');
    EXPECT_EQ(ssSdvString, u"1+-****890");
    ssSdvString.replace(0, 0, 4, u'#');
    EXPECT_EQ(ssSdvString, u"####1+-****890");
    ssSdvString.replace(std::u16string::npos, 100, 4, u'~');
    EXPECT_EQ(ssSdvString, u"####1+-****890~~~~");
    ssSdvString.replace(0, std::u16string::npos, 4, u'=');
    EXPECT_EQ(ssSdvString, u"====");
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, u'+');
    EXPECT_EQ(ssSdvString, u"1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, u'-');
    EXPECT_EQ(ssSdvString, u"1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, u'*');
    EXPECT_EQ(ssSdvString, u"1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, u'#');
    EXPECT_EQ(ssSdvString, u"####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, u'~');
    EXPECT_EQ(ssSdvString, u"####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, u'=');
    EXPECT_EQ(ssSdvString, u"====");

    // Replace with iterators
    ssSdvString = u"1234567890";
    ss = u"abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, u"abcd");

    // Replace with initialization list
    ssSdvString = u"1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {u'a', u'b', u'c', u'd'});
    EXPECT_EQ(ssSdvString, u"abcd");
}

TEST_F(CUtf16StringTypeTest, ReplaceFunctionMixed)
{
    // Replace string - replace dynamic by static
    sdv::u16string ssDynamic = u"1234567890";
    sdv::fixed_u16string<10> ssStatic1 = u"abcd";
    ssDynamic.replace(1, 2, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890");
    ssDynamic.replace(std::u16string::npos, 100, ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::u16string::npos, ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd");
    ssDynamic = u"1234567890";
    ssDynamic.replace(ssDynamic.begin() + 1, ssDynamic.begin() + 3, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1abcd4567890");
    ssDynamic.replace(ssDynamic.begin() + 2, ssDynamic.begin() + 8, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1aabcd7890");
    ssDynamic.replace(ssDynamic.begin() + 3, ssDynamic.begin() + 7, ssStatic1);
    EXPECT_EQ(ssDynamic, u"1aaabcd890");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.begin(), ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890");
    ssDynamic.replace(ssDynamic.end(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890abcd");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, u"abcd");

    // Replace substring
    ssDynamic = u"1234567890";
    ssStatic1 = u"xxabcdxx";
    ssDynamic.replace(1, 2, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890");
    ssDynamic.replace(std::u16string::npos, 100, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::u16string::npos, ssStatic1, 2, 4);

    // Replace string - replace static by dynamic
    sdv::fixed_u16string<20> ssStatic = u"1234567890";
    sdv::u16string ssDynamic1 = u"abcd";
    ssStatic.replace(1, 2, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890");
    ssStatic.replace(std::u16string::npos, 100, ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::u16string::npos, ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd");
    ssStatic = u"1234567890";
    ssStatic.replace(ssStatic.begin() + 1, ssStatic.begin() + 3, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1abcd4567890");
    ssStatic.replace(ssStatic.begin() + 2, ssStatic.begin() + 8, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1aabcd7890");
    ssStatic.replace(ssStatic.begin() + 3, ssStatic.begin() + 7, ssDynamic1);
    EXPECT_EQ(ssStatic, u"1aaabcd890");
    ssStatic.replace(ssStatic.begin(), ssStatic.begin(), ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890");
    ssStatic.replace(ssStatic.end(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890abcd");
    ssStatic.replace(ssStatic.begin(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, u"abcd");

    // Replace substring
    ssStatic = u"1234567890";
    ssDynamic1 = u"xxabcdxx";
    ssStatic.replace(1, 2, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890");
    ssStatic.replace(std::u16string::npos, 100, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::u16string::npos, ssDynamic1, 2, 4);
}

TEST_F(CUtf16StringTypeTest, SubstringFunctionStatic)
{
    sdv::fixed_u16string<20> ssSdvString = u"1234567890";
    EXPECT_EQ(ssSdvString.substr(), u"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), u"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), u"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf16StringTypeTest, SubstringFunctionDynamic)
{
    sdv::u16string ssSdvString = u"1234567890";
    EXPECT_EQ(ssSdvString.substr(), u"1234567890");
    EXPECT_EQ(ssSdvString.substr(2), u"34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), u"34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf16StringTypeTest, CopyFunction)
{
    sdv::u16string ssSdvString = u"1234567890";
    char16_t sz[32] = {};
    EXPECT_EQ(ssSdvString.copy(sz, 2, 5), 2);
    EXPECT_STREQ(sz, u"67");
    std::fill_n(sz, 32, static_cast<char16_t>(u'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 20, 5), 5);
    EXPECT_STREQ(sz, u"67890");
    std::fill_n(sz, 32, static_cast<char16_t>(u'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0, 5), 0);
    EXPECT_STREQ(sz, u"");
    std::fill_n(sz, 32, static_cast<char16_t>(u'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 2), 2);
    EXPECT_STREQ(sz, u"12");
    std::fill_n(sz, 32, static_cast<char16_t>(u'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0), 0);
    EXPECT_STREQ(sz, u"");
    std::fill_n(sz, 32, static_cast<char16_t>(u'\0'));
    EXPECT_EQ(ssSdvString.copy(sz, std::u16string::npos), 10);
    EXPECT_STREQ(sz, u"1234567890");
}

TEST_F(CUtf16StringTypeTest, SwapFunctionStatic)
{
    sdv::fixed_u16string<20> ssSdvString1 = u"12345";
    sdv::fixed_u16string<10> ssSdvString2 = u"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"67890");
    EXPECT_EQ(ssSdvString2, u"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"12345");
    EXPECT_EQ(ssSdvString2, u"67890");
}

TEST_F(CUtf16StringTypeTest, SwapFunctionDynamic)
{
    sdv::u16string ssSdvString1 = u"12345";
    sdv::u16string ssSdvString2 = u"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"67890");
    EXPECT_EQ(ssSdvString2, u"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"12345");
    EXPECT_EQ(ssSdvString2, u"67890");
}

TEST_F(CUtf16StringTypeTest, SwapFunctionMixed)
{
    sdv::fixed_u16string<20> ssSdvString1 = u"12345";
    sdv::u16string ssSdvString2 = u"67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"67890");
    EXPECT_EQ(ssSdvString2, u"12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, u"12345");
    EXPECT_EQ(ssSdvString2, u"67890");
}

TEST_F(CUtf16StringTypeTest, FindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"aabbaaccbbcc";
    sdv::fixed_u16string<10> ssSdvString2 = u"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::fixed_u16string<20>::npos);

    // Find C++ string in string
    std::u16string ss = u"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::fixed_u16string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 3, 4), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 9), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(u'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(u'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(u'\0', 11), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"aabbaaccbbcc";
    sdv::u16string ssSdvString2 = u"aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::u16string::npos);

    // Find C++ string in string
    std::u16string ss = u"aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::u16string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 3, 4), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find(u"bb\0a", 9), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find(u'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find(u'\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find(u'\0', 11), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, FindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u16string ssDynamic = u"aabbaaccbbcc";
    sdv::fixed_u16string<10> ssStatic1 = u"aa";
    EXPECT_EQ(ssDynamic.find(ssStatic1), 0);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 1), 4);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 5), sdv::fixed_string<20>::npos);

    // Find string in string - find static in dynamic
    sdv::fixed_u16string<20> ssStatic = u"aabbaaccbbcc";
    sdv::u16string ssDynamic1 = u"aa";
    EXPECT_EQ(ssStatic.find(ssDynamic1), 0);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 1), 4);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 5), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, ReverseFindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"aabbaaccbbcc";
    sdv::fixed_u16string<20> ssSdvString2 = u"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::u16string ss = u"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", sdv::fixed_u16string<20>::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 1, 4), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", sdv::fixed_u16string<20>::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 1), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(u'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(u'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(u'\0', 3), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, ReverseFindFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"aabbaaccbbcc";
    sdv::u16string ssSdvString2 = u"aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::u16string ss = u"aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", sdv::u16string::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 1, 4), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", sdv::u16string::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind(u"bb\0a", 1), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind(u'\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind(u'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind(u'\0', 3), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, ReverseFindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u16string ssDynamic = u"aabbaaccbbcc";
    sdv::fixed_u16string<10> ssStatic1 = u"aa";
    EXPECT_EQ(ssDynamic.rfind(ssStatic1), 4);
    EXPECT_EQ(ssDynamic.rfind(ssStatic1, 3), 0);

    // Find string in string - find static in dynamic
    sdv::fixed_u16string<20> ssStatic = u"aabbaaccbbcc";
    sdv::u16string ssDynamic1 = u"aa";
    EXPECT_EQ(ssStatic.rfind(ssDynamic1), 4);
    EXPECT_EQ(ssStatic.rfind(ssDynamic1, 3), 0);
}

TEST_F(CUtf16StringTypeTest, FindFirstOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"12341234";
    sdv::fixed_u16string<10> ssSdvString2 = u"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::fixed_u16string<20>::npos);

    // Find C++ string in string
    std::u16string ss = u"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::fixed_u16string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 10, 2), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 8), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 11), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindFirstOfFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"12341234";
    sdv::u16string ssSdvString2 = u"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::u16string::npos);

    // Find C++ string in string
    std::u16string ss = u"34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::u16string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 10, 2), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(u"c\0", 8), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of(u'\0', 11), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, FindFirstOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u16string ssDynamic = u"12341234";
    sdv::fixed_u16string<10> ssStatic1 = u"34";
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in sttaic
    sdv::fixed_u16string<10> ssStatic = u"12341234";
    sdv::u16string ssDynamic1 = u"34";
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindFirstNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"12341234";
    sdv::fixed_u16string<10> ssSdvString2 = u"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::fixed_u16string<20>::npos);

    // Find C++ string in string
    std::u16string ss = u"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::fixed_u16string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 9, 3), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 10), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 12), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindFirstNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"12341234";
    sdv::u16string ssSdvString2 = u"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::u16string::npos);

    // Find C++ string in string
    std::u16string ss = u"12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::u16string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 9, 3), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u"ab\0", 10), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of(u'\0', 12), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, FindFirstNotOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u16string ssDynamic = u"12341234";
    sdv::fixed_u16string<10> ssStatic1 = u"12";
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u16string<10> ssStatic = u"12341234";
    sdv::u16string ssDynamic1 = u"12";
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"12341234";
    sdv::fixed_u16string<10> ssSdvString2 = u"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::fixed_u16string<20>::npos);

    // Find C++ string in string
    std::u16string ss = u"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::fixed_u16string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", std::u16string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 2, 2), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 2), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0', 4), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastOfFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"12341234";
    sdv::u16string ssSdvString2 = u"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::u16string::npos);

    // Find C++ string in string
    std::u16string ss = u"34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::u16string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", std::u16string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 2, 2), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(u"c\0", 2), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of(u'\0', 4), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u16string ssDynamic = u"12341234";
    sdv::fixed_u16string<10> ssStatic1 = u"34";
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u16string<10> ssStatic = u"12341234";
    sdv::u16string ssDynamic1 = u"34";
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u16string<20> ssSdvString1 = u"12341234";
    sdv::fixed_u16string<10> ssSdvString2 = u"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::fixed_u16string<20>::npos);

    // Find C++ string in string
    std::u16string ss = u"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::fixed_u16string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u16string<20>(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", std::u16string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 2, 3), sdv::fixed_u16string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 2), sdv::fixed_u16string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 0), sdv::fixed_u16string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u16string ssSdvString1 = u"12341234";
    sdv::u16string ssSdvString2 = u"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::u16string::npos);

    // Find C++ string in string
    std::u16string ss = u"12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::u16string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u16string(u"abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", std::u16string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 2, 3), sdv::u16string::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u"ab\0", 2), sdv::u16string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of(u'\0', 0), sdv::u16string::npos);
}

TEST_F(CUtf16StringTypeTest, FindLastNotOfFunctionMixed)
{
    // Find string in string - find static in dyanmic
    sdv::u16string ssDynamic = u"12341234";
    sdv::fixed_u16string<10> ssStatic1 = u"12";
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u16string<10> ssStatic = u"12341234";
    sdv::u16string ssDynamic1 = u"12";
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf16StringTypeTest, SumOperatorStatic)
{
    sdv::fixed_u16string<20> ssSdvString = u"1234";
    std::u16string ss = u"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, u"12341234");
    EXPECT_EQ(ss + ssSdvString, u"56781234");
    EXPECT_EQ(ssSdvString + ss, u"12345678");
    EXPECT_EQ(u"abcd" + ssSdvString, u"abcd1234");
    EXPECT_EQ(ssSdvString + u"abcd", u"1234abcd");
    EXPECT_EQ(u'a' + ssSdvString, u"a1234");
    EXPECT_EQ(ssSdvString + u'a', u"1234a");
    sdv::fixed_u16string<20> ssSdvString1 = u"1234";
    sdv::fixed_u16string<10> ssSdvString2 = u"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    ssSdvString2 = u"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + u"abcd", u"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + u'a', u"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), u"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), u"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(u"abcd" + std::move(ssSdvString1), u"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(u'a' + std::move(ssSdvString1), u"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf16StringTypeTest, SumOperatorDynamic)
{
    sdv::u16string ssSdvString = u"1234";
    std::u16string ss = u"5678";

    EXPECT_EQ(ssSdvString + ssSdvString, u"12341234");
    EXPECT_EQ(ss + ssSdvString, u"56781234");
    EXPECT_EQ(ssSdvString + ss, u"12345678");
    EXPECT_EQ(u"abcd" + ssSdvString, u"abcd1234");
    EXPECT_EQ(ssSdvString + u"abcd", u"1234abcd");
    EXPECT_EQ(u'a' + ssSdvString, u"a1234");
    EXPECT_EQ(ssSdvString + u'a', u"1234a");
    sdv::u16string ssSdvString1 = u"1234", ssSdvString2 = u"5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    ssSdvString2 = u"5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, u"12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + u"abcd", u"1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(std::move(ssSdvString1) + u'a', u"1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), u"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), u"56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(u"abcd" + std::move(ssSdvString1), u"abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = u"1234";
    EXPECT_EQ(u'a' + std::move(ssSdvString1), u"a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf16StringTypeTest, SumOperatorMixed)
{
    // Add strings - add static onto dynamic
    sdv::u16string ssDynamic = u"1234";
    sdv::fixed_u16string<20> ssStatic1 = u"5678";
    EXPECT_EQ(ssDynamic + ssStatic1, u"12345678");
    EXPECT_EQ(std::move(ssDynamic) + std::move(ssStatic1), u"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_TRUE(ssStatic1.empty());
    ssDynamic = u"1234";
    ssStatic1 = u"5678";
    EXPECT_EQ(std::move(ssDynamic) + ssStatic1, u"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());
    ssDynamic = u"1234";
    EXPECT_EQ(ssStatic1 + std::move(ssDynamic), u"56781234");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());

    // Add strings - add dynamic onto static
    sdv::fixed_u16string<20> ssStatic = u"1234";
    sdv::u16string ssDynamic1 = u"5678";
    EXPECT_EQ(ssStatic + ssDynamic1, u"12345678");
    EXPECT_EQ(std::move(ssStatic) + std::move(ssDynamic1), u"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_TRUE(ssDynamic1.empty());
    ssStatic = u"1234";
    ssDynamic1 = u"5678";
    EXPECT_EQ(std::move(ssStatic) + ssDynamic1, u"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
    ssStatic = u"1234";
    EXPECT_EQ(ssDynamic1 + std::move(ssStatic), u"56781234");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
}

#ifdef __GNUC__
// Disabling this test, since it is not supported by the GCC library.
TEST_F(CUtf16StringTypeTest, DISABLED_StreamString)
#else
TEST_F(CUtf16StringTypeTest, StreamString)
#endif
{
    std::basic_stringstream<char16_t> sstream;
    sdv::u16string ss = u"1234";
    sstream << ss;
    EXPECT_EQ(sstream.str(), u"1234");
    ss.clear();
    EXPECT_TRUE(ss.empty());
    sstream >> ss;
    EXPECT_EQ(ss, u"1234");
}

TEST_F(CUtf16StringTypeTest, StringGetLine)
{
    std::basic_istringstream<char16_t> sstream;
    sstream.str(u"0;1;2;3");
    int nIndex = 0;
    for (sdv::u16string ss; sdv::getline(sstream, ss, u';');)
        EXPECT_EQ(*ss.c_str(), u'0' + static_cast<char16_t>(nIndex++));
    EXPECT_EQ(sstream.str(), u"0;1;2;3");

    sstream.str(u"0\n1\n2\n3");
    nIndex = 0;
    for (sdv::u16string ss; sdv::getline(std::move(sstream), ss);)
        EXPECT_EQ(*ss.c_str(), u'0' + static_cast<char16_t>(nIndex++));
}

TEST_F(CUtf16StringTypeTest, MakeString)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf16String(ssAnsi), u"Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssWide), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf8), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf16), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf32), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssAnsi.c_str()), u"Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssWide.c_str()), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf8.c_str()), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf16.c_str()), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssUtf32.c_str()), u"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf16String(ssCppAnsi), u"Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssCppWide), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssCppUtf8), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssCppUtf16), u"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf16String(ssCppUtf32), u"Hello\U00024B62Hello");
}

TEST_F(CUtf16StringTypeTest, MakeStringGeneric)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    sdv::u16string ssDst;
    ssDst = sdv::MakeString<char, false, 0, char16_t, true, 0>(ssAnsi);
    EXPECT_EQ(ssDst, u"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char16_t, true, 0>(ssWide);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char16_t, true, 0>(ssUtf8);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char16_t, true, 0>(ssUtf16);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char16_t, true, 0>(ssUtf32);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, false, 0, char16_t, true, 0>(ssAnsi.c_str());
    EXPECT_EQ(ssDst, u"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char16_t, true, 0>(ssWide.c_str());
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char16_t, true, 0>(ssUtf8.c_str());
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char16_t, true, 0>(ssUtf16.c_str());
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char16_t, true, 0>(ssUtf32.c_str());
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    ssDst = sdv::MakeString<char, char16_t, true, 0>(ssCppAnsi);
    EXPECT_EQ(ssDst, u"Hello");
    ssDst = sdv::MakeString<wchar_t, char16_t, true, 0>(ssCppWide);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, char16_t, true, 0>(ssCppUtf8);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, char16_t, true, 0>(ssCppUtf16);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, char16_t, true, 0>(ssCppUtf32);
    EXPECT_EQ(ssDst, u"Hello\U00024B62Hello");
}

TEST_F(CUtf16StringTypeTest, IteratorDistance)
{
    sdv::u16string ss = u"This is a text";
    EXPECT_EQ(std::distance(ss.begin(), ss.end()), ss.length());
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
