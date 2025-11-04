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

using CUtf8StringTypeTest = CBasicTypesTest;

TEST_F(CUtf8StringTypeTest, ConstructorStatic)
{
    // Empty string
    sdv::fixed_u8string<10> ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::fixed_u8string<10> ssCString("Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");
    std::string ss("Hello");
    sdv::fixed_u8string<10> ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, "Hello");

    // Copy constructor
    sdv::fixed_u8string<8> ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");

    // Move constructor
    sdv::fixed_u8string<12> ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");

    // Fill constructor
    sdv::fixed_u8string<20> ssFillString(10, '*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, "**********");

    // Substring constructor
    sdv::fixed_u8string<20> ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, "llo");
    sdv::fixed_u8string<21> ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, "ll");
    sdv::fixed_u8string<22> ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, "llo");
    sdv::fixed_u8string<23> ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, "ll");
    sdv::fixed_u8string<24> ssSubstring5("He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, "He\0l", 4), 0);

    // Iterator based construction
    sdv::fixed_u8string<10> ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, "Hello");
    sdv::fixed_u8string<10> ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, "Hello");

    // Construct using initializer list
    sdv::fixed_u8string<10> ssInitListString{'H', 'e', 'l', 'l', 'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, "Hello");
}

TEST_F(CUtf8StringTypeTest, ConstructorDynamic)
{
    // Empty string
    sdv::u8string ssEmpty;
    EXPECT_TRUE(ssEmpty.empty());

    // Assignment constructors
    sdv::u8string ssCString("Hello");
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");
    std::string ss("Hello");
    sdv::u8string ssCppString(ss);
    EXPECT_FALSE(ssCppString.empty());
    EXPECT_EQ(ssCppString.size(), 5);
    EXPECT_EQ(ssCppString, "Hello");

    // Copy constructor
    sdv::u8string ssCopyString(ssCString);
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");

    // Move constructor
    sdv::u8string ssMoveString(std::move(ssCopyString));
    EXPECT_TRUE(ssCopyString.empty());
    EXPECT_FALSE(ssCString.empty());
    EXPECT_EQ(ssCString.size(), 5);
    EXPECT_EQ(ssCString, "Hello");

    // Fill constructor
    sdv::u8string ssFillString(10, '*');
    EXPECT_FALSE(ssFillString.empty());
    EXPECT_EQ(ssFillString.size(), 10);
    EXPECT_EQ(ssFillString, "**********");

    // Substring constructor
    sdv::u8string ssSubstring1(ssCString, 2);
    EXPECT_FALSE(ssSubstring1.empty());
    EXPECT_EQ(ssSubstring1.size(), 3);
    EXPECT_EQ(ssSubstring1, "llo");
    sdv::u8string ssSubstring2(ssCString, 2, 2);
    EXPECT_FALSE(ssSubstring2.empty());
    EXPECT_EQ(ssSubstring2.size(), 2);
    EXPECT_EQ(ssSubstring2, "ll");
    sdv::u8string ssSubstring3(ss, 2);
    EXPECT_FALSE(ssSubstring3.empty());
    EXPECT_EQ(ssSubstring3.size(), 3);
    EXPECT_EQ(ssSubstring3, "llo");
    sdv::u8string ssSubstring4(ss, 2, 2);
    EXPECT_FALSE(ssSubstring4.empty());
    EXPECT_EQ(ssSubstring4.size(), 2);
    EXPECT_EQ(ssSubstring4, "ll");
    sdv::u8string ssSubstring5("He\0llo", 4);
    EXPECT_FALSE(ssSubstring5.empty());
    EXPECT_EQ(ssSubstring5.size(), 4);
    EXPECT_EQ(ssSubstring5.compare(0, 4, "He\0l", 4), 0);

    // Iterator based construction
    sdv::u8string ssIteratorString1(ssCString.begin(), ssCString.end());
    EXPECT_FALSE(ssIteratorString1.empty());
    EXPECT_EQ(ssIteratorString1.size(), 5);
    EXPECT_EQ(ssIteratorString1, "Hello");
    sdv::u8string ssIteratorString2(ss.begin(), ss.end());
    EXPECT_FALSE(ssIteratorString2.empty());
    EXPECT_EQ(ssIteratorString2.size(), 5);
    EXPECT_EQ(ssIteratorString2, "Hello");

    // Construct using initializer list
    sdv::u8string ssInitListString{'H', 'e', 'l', 'l', 'o'};
    EXPECT_FALSE(ssInitListString.empty());
    EXPECT_EQ(ssInitListString.size(), 5);
    EXPECT_EQ(ssInitListString, "Hello");
}

TEST_F(CUtf8StringTypeTest, ConstructorMixed)
{
    // Copy constructor
    sdv::u8string ssDynamic = u8"dynamic";
    sdv::fixed_u8string<8> ssStaticCopy(ssDynamic);
    EXPECT_EQ(ssStaticCopy, u8"dynamic");
    sdv::fixed_u8string<8> ssStatic = u8"static";
    sdv::u8string ssDynamicCopy(ssStatic);
    EXPECT_EQ(ssDynamicCopy, u8"static");

    // Move constructor
    sdv::fixed_u8string<8> ssStaticMove(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u8"dynamic");
    sdv::u8string ssDynamicMove(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u8"static");
}

TEST_F(CUtf8StringTypeTest, AssignmentOperatorStatic)
{
    // SDV-String assignment
    sdv::fixed_u8string<10> ssSdvString1("Hello");
    sdv::fixed_u8string<15> ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, "Hello");

    // Move assignment
    sdv::fixed_u8string<10> ssSdvString3("Hello");
    sdv::fixed_u8string<15> ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, "Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::string ss1("Hello");
    sdv::fixed_u8string<10> ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, "Hello");

    // C string assignment
    const char sz1[] = "Hello";
    sdv::fixed_u8string<10> ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, "Hello");
    const char* sz2 = "Hello";
    sdv::fixed_u8string<10> ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, "Hello");

    // Initializer list
    sdv::fixed_u8string<10> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = "Hello";
    EXPECT_EQ(ssSdvString8, "Hello");
    sdv::fixed_u8string<10> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {'H', 'e', 'l', 'l', 'o'};
    EXPECT_EQ(ssSdvString9, "Hello");
}

TEST_F(CUtf8StringTypeTest, AssignmentOperatorDynamic)
{
    // SDV-String assignment
    sdv::u8string ssSdvString1("Hello");
    sdv::u8string ssSdvString2;
    EXPECT_NE(ssSdvString1, ssSdvString2);
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 = ssSdvString1;
    EXPECT_EQ(ssSdvString2, ssSdvString1);
    EXPECT_EQ(ssSdvString2, "Hello");

    // Move assignment
    sdv::u8string ssSdvString3("Hello");
    sdv::u8string ssSdvString4;
    EXPECT_NE(ssSdvString3, ssSdvString4);
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 = std::move(ssSdvString3);
    EXPECT_NE(ssSdvString4, ssSdvString3);
    EXPECT_EQ(ssSdvString4, "Hello");
    EXPECT_TRUE(ssSdvString3.empty());

    // C++ string assignment
    std::string ss1("Hello");
    sdv::u8string ssSdvString5;
    EXPECT_NE(ss1, ssSdvString5);
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 = ss1;
    EXPECT_EQ(ssSdvString5, ss1);
    EXPECT_EQ(ssSdvString5, "Hello");

    // C string assignment
    const char	sz1[] = "Hello";
    sdv::u8string ssSdvString6;
    EXPECT_NE(sz1, ssSdvString6);
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6 = sz1;
    EXPECT_EQ(ssSdvString6, sz1);
    EXPECT_EQ(ssSdvString6, "Hello");
    const char* sz2 = "Hello";
    sdv::u8string ssSdvString7;
    EXPECT_NE(sz2, ssSdvString7);
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7 = sz2;
    EXPECT_EQ(ssSdvString7, sz2);
    EXPECT_EQ(ssSdvString7, "Hello");

    // Initializer list
    sdv::u8string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8 = "Hello";
    EXPECT_EQ(ssSdvString8, "Hello");
    sdv::u8string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9 = {'H', 'e', 'l', 'l', 'o'};
    EXPECT_EQ(ssSdvString9, "Hello");
}

TEST_F(CUtf8StringTypeTest, AssignmentOperatorMixed)
{
    // Copy assignment
    sdv::u8string ssDynamic = u8"dynamic";
    sdv::fixed_u8string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy = ssDynamic;
    EXPECT_EQ(ssStaticCopy, u8"dynamic");
    sdv::fixed_u8string<8> ssStatic = u8"static";
    sdv::u8string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy = ssStatic;
    EXPECT_EQ(ssDynamicCopy, u8"static");

    // Move assignment
    sdv::fixed_u8string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove = std::move(ssDynamic);
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u8"dynamic");
    sdv::u8string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove = std::move(ssStatic);
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u8"static");
}

TEST_F(CUtf8StringTypeTest, AssignmentFunctionStatic)
{
    // Character assignment
    sdv::fixed_u8string<10> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, '*');
    EXPECT_EQ(ssSdvString1, "**********");

    // String copy assignment
    sdv::fixed_u8string<10> ssSdvString2;
    sdv::fixed_u8string<15> ssSdvString3("Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");

    // C++ string assignment
    sdv::fixed_u8string<10> ssSdvString4;
    std::string	  ss("Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");

    // C string assignment
    sdv::fixed_u8string<10> ssSdvString5;
    const char	  sz1[] = "Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    sdv::fixed_u8string<10> ssSdvString6;
    const char*	  sz2 = "Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, "Hello");

    // Substring assignment
    sdv::fixed_u8string<10> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    sdv::fixed_u8string<11> ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, "ell");
    sdv::fixed_u8string<12> ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, "ell");

    // Move assignment
    sdv::fixed_u8string<10> ssSdvString10;
    sdv::fixed_u8string<15> ssSdvString11("Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, "Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::fixed_u8string<10> ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, "Hello");

    // Initializer list
    sdv::fixed_u8string<10> ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({'H', 'e', 'l', 'l', 'o'});
    EXPECT_EQ(ssSdvString13, "Hello");
}

TEST_F(CUtf8StringTypeTest, AssignmentFunctionDynamic)
{
    // Character assignment
    sdv::u8string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.assign(10, '*');
    EXPECT_EQ(ssSdvString1, "**********");

    // String copy assignment
    sdv::u8string ssSdvString2;
    sdv::u8string ssSdvString3("Hello");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.assign(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");

    // C++ string assignment
    sdv::u8string ssSdvString4;
    std::string ss("Hello");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.assign(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");

    // C string assignment
    sdv::u8string ssSdvString5;
    const char	sz1[] = "Hello";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.assign(sz1);
    EXPECT_EQ(ssSdvString5, sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    sdv::u8string ssSdvString6;
    const char* sz2 = "Hello";
    EXPECT_TRUE(ssSdvString6.empty());
    ssSdvString6.assign(sz2);
    EXPECT_EQ(ssSdvString6, sz2);
    EXPECT_EQ(ssSdvString6, "Hello");

    // Substring assignment
    sdv::u8string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.assign(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    sdv::u8string ssSdvString8;
    EXPECT_TRUE(ssSdvString8.empty());
    ssSdvString8.assign(ss, 1, 3);
    EXPECT_EQ(ssSdvString8, "ell");
    sdv::u8string ssSdvString9;
    EXPECT_TRUE(ssSdvString9.empty());
    ssSdvString9.assign(sz1 + 1, 3);
    EXPECT_EQ(ssSdvString9, "ell");

    // Move assignment
    sdv::u8string ssSdvString10;
    sdv::u8string ssSdvString11("Hello");
    EXPECT_TRUE(ssSdvString10.empty());
    ssSdvString10.assign(std::move(ssSdvString11));
    EXPECT_NE(ssSdvString10, ssSdvString11);
    EXPECT_EQ(ssSdvString10, "Hello");
    EXPECT_TRUE(ssSdvString11.empty());

    // Iterator assignment
    sdv::u8string ssSdvString12;
    EXPECT_TRUE(ssSdvString12.empty());
    ssSdvString12.assign(ssSdvString3.begin(), ssSdvString3.end());
    EXPECT_EQ(ssSdvString12, ssSdvString3);
    EXPECT_EQ(ssSdvString12, "Hello");

    // Initializer list
    sdv::u8string ssSdvString13;
    EXPECT_TRUE(ssSdvString13.empty());
    ssSdvString13.assign({'H', 'e', 'l', 'l', 'o'});
    EXPECT_EQ(ssSdvString13, "Hello");
}

TEST_F(CUtf8StringTypeTest, AssignmentFunctionMixed)
{
    // Copy assignment
    sdv::u8string ssDynamic = u8"dynamic";
    sdv::fixed_u8string<8> ssStaticCopy;
    EXPECT_TRUE(ssStaticCopy.empty());
    ssStaticCopy.assign(ssDynamic);
    EXPECT_EQ(ssStaticCopy, u8"dynamic");
    sdv::fixed_u8string<8> ssStatic = u8"static";
    sdv::u8string ssDynamicCopy;
    EXPECT_TRUE(ssDynamicCopy.empty());
    ssDynamicCopy.assign(ssStatic);
    EXPECT_EQ(ssDynamicCopy, u8"static");

    // Move assignment
    sdv::fixed_u8string<8> ssStaticMove;
    EXPECT_TRUE(ssStaticMove.empty());
    ssStaticMove.assign(std::move(ssDynamic));
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_EQ(ssStaticMove, u8"dynamic");
    sdv::u8string ssDynamicMove;
    EXPECT_TRUE(ssDynamicMove.empty());
    ssDynamicMove.assign(std::move(ssStatic));
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_EQ(ssDynamicMove, u8"static");

    // Substring assignment
    sdv::fixed_u8string<10> ssStaticSubstring;
    EXPECT_TRUE(ssStaticSubstring.empty());
    ssStaticSubstring.assign(ssDynamicMove, 1, 3);
    EXPECT_EQ(ssStaticSubstring, u8"tat");
    sdv::u8string ssDynamicSubstring;
    EXPECT_TRUE(ssDynamicSubstring.empty());
    ssDynamicSubstring.assign(ssStaticMove, 1, 3);
    EXPECT_EQ(ssDynamicSubstring, u8"yna");
}

TEST_F(CUtf8StringTypeTest, PositionFunction)
{
    // Position in the string
    sdv::u8string ssSdvString1("Hello");
    EXPECT_EQ(ssSdvString1.at(0), 'H');
    EXPECT_EQ(ssSdvString1[0], 'H');
    EXPECT_EQ(ssSdvString1.front(), 'H');
    EXPECT_EQ(ssSdvString1.at(4), 'o');
    EXPECT_EQ(ssSdvString1[4], 'o');
    EXPECT_EQ(ssSdvString1.back(), 'o');
    EXPECT_THROW(ssSdvString1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString1[5], sdv::XIndexOutOfRange);

    // Empty string
    sdv::u8string ssSdvString2;
    EXPECT_THROW(ssSdvString2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(ssSdvString2.back(), sdv::XIndexOutOfRange);

    // Assignment
    ssSdvString1.at(0) = 'B';
    EXPECT_EQ(ssSdvString1, "Bello");
    ssSdvString1[0] = 'D';
    EXPECT_EQ(ssSdvString1, "Dello");
    ssSdvString1.front() = 'M';
    EXPECT_EQ(ssSdvString1, "Mello");
    ssSdvString1.back() = 'k';
    EXPECT_EQ(ssSdvString1, "Mellk");
}

TEST_F(CUtf8StringTypeTest, CAndCppStringAccess)
{
    // C++ cast operator
    sdv::u8string ssSdvString("Hello");
    std::string ss1(ssSdvString);
    EXPECT_EQ(ss1, "Hello");
    std::string ss2;
    EXPECT_TRUE(ss2.empty());
    ss2 = ssSdvString;
    EXPECT_EQ(ss2, "Hello");

    // Data access
    sdv::u8string ssSdvString2("He\0lo", 5);
    EXPECT_EQ(ssSdvString2.size(), 5);
    const char* sz1 = ssSdvString2.data();
    EXPECT_EQ(memcmp(sz1, "He\0lo", 5), 0);

    // C string access
    const char* sz2 = ssSdvString.c_str();
    EXPECT_STREQ(sz2, "Hello");
}

TEST_F(CUtf8StringTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::u8string::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u8string ssSdvString("Hello");
    EXPECT_NE(itEmpty, ssSdvString.begin());
    sdv::u8string::iterator itPos = ssSdvString.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.begin());
    sdv::u8string::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u8string::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, 'H');
    EXPECT_EQ(itPos[0], 'H');
    EXPECT_EQ(itPos[4], 'o');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'B';
    EXPECT_EQ(ssSdvString, "Bello");
    itPos[4] = 'k';
    EXPECT_EQ(ssSdvString, "Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, 'e');
    itPos += 3;
    EXPECT_EQ(*itPos, 'o');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.end());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, 'o');
    itPos -= 4;
    EXPECT_EQ(*itPos, 'H');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, 'H');
    EXPECT_EQ(itPos, ssSdvString.begin());

    // Const iterator
    sdv::u8string::const_iterator itPosConst = ssSdvString.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, 'e');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.cbegin(), ssSdvString.begin());
    EXPECT_EQ(ssSdvString.begin(), ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString.cend(), ssSdvString.end());
    EXPECT_EQ(ssSdvString.end(), ssSdvString.cend());
}

TEST_F(CUtf8StringTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::u8string::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::u8string ssSdvString("Hello");
    EXPECT_NE(itEmpty, ssSdvString.rbegin());
    sdv::u8string::reverse_iterator itPos = ssSdvString.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, ssSdvString.rbegin());
    sdv::u8string::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::u8string::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, 'o');
    EXPECT_EQ(itPos[0], 'o');
    EXPECT_EQ(itPos[4], 'H');
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 'k';
    EXPECT_EQ(ssSdvString, "Hellk");
    itPos[4] = 'B';
    EXPECT_EQ(ssSdvString, "Bellk");
    ssSdvString[0] = 'H';
    ssSdvString[4] = 'o';

    // Iterator iteration
    ++itPos;
    EXPECT_EQ(*itPos, 'l');
    itPos += 3;
    EXPECT_EQ(*itPos, 'H');
    EXPECT_NO_THROW(++itPos); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, ssSdvString.rend());
    EXPECT_NO_THROW(++itPos); // Will be ignored; doesn't increase even more
    --itPos;
    EXPECT_EQ(*itPos, 'H');
    itPos -= 4;
    EXPECT_EQ(*itPos, 'o');
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, 'o');
    EXPECT_EQ(itPos, ssSdvString.rbegin());

    // Const iterator
    sdv::u8string::const_reverse_iterator itPosConst = ssSdvString.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    ++itPosConst;
    EXPECT_EQ(*itPosConst, 'l');
    EXPECT_NE(itPosConst, itPos);
    ++itPos;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(ssSdvString.crbegin(), ssSdvString.rbegin());
    EXPECT_EQ(ssSdvString.rbegin(), ssSdvString.crbegin());
    EXPECT_EQ(ssSdvString.crend(), ssSdvString.rend());
    EXPECT_EQ(ssSdvString.rend(), ssSdvString.crend());
}

TEST_F(CUtf8StringTypeTest, StringCapacityStatic)
{
    // Empty string
    sdv::fixed_u8string<10> ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Filled string
    ssSdvString = "Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, "Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString += 'o';
    ssSdvString.resize(10, '*');
    EXPECT_EQ(ssSdvString, "Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, "Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
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

TEST_F(CUtf8StringTypeTest, StringCapacityDynamic)
{
    // Empty string
    sdv::u8string ssSdvString;
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 0);
    EXPECT_EQ(ssSdvString.length(), 0);
    EXPECT_EQ(ssSdvString.capacity(), 0);

    // Filled string
    ssSdvString = "Hello";
    EXPECT_FALSE(ssSdvString.empty());
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);

    // Resize, reserve
    ssSdvString.resize(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(4);
    EXPECT_EQ(ssSdvString, "Hell");
    EXPECT_EQ(ssSdvString.size(), 4);
    EXPECT_EQ(ssSdvString.length(), 4);
    EXPECT_EQ(ssSdvString.capacity(), 5);
    ssSdvString += 'o';
    ssSdvString.resize(10, '*');
    EXPECT_EQ(ssSdvString, "Hello*****");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);
    EXPECT_EQ(ssSdvString, "Hello");
    EXPECT_EQ(ssSdvString.size(), 5);
    EXPECT_EQ(ssSdvString.length(), 5);
    EXPECT_EQ(ssSdvString.capacity(), 6);
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.reserve(4);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.resize(5);

    // Shrink to fit
    ssSdvString.reserve(10);
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
    EXPECT_EQ(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString.length(), 10);
    EXPECT_EQ(ssSdvString.capacity(), 11);
    ssSdvString.shrink_to_fit();
    EXPECT_STREQ(ssSdvString.c_str(), "Hello");
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

TEST_F(CUtf8StringTypeTest, InsertFunctionStatic)
{
    // Character assignment
    sdv::fixed_u8string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, '*');
    EXPECT_EQ(ssSdvString1, "**********");
    ssSdvString1.insert(0, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********??");
    ssSdvString1.insert(std::string::npos, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********????");
    ssSdvString1.insert(4, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**??********????");

    // C string assignment
    sdv::fixed_u8string<20> ssSdvString5;
    const char	sz1[] = "Hello";
    const char* sz2	  = "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::string::npos, sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, "**Hello");
    ssSdvString5.insert(std::string::npos, sz2);
    EXPECT_EQ(ssSdvString5, "**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, "**He**llo**");

    // String copy assignment
    sdv::fixed_u8string<20> ssSdvString2;
    sdv::fixed_u8string<20> ssSdvString3("Hello");
    sdv::fixed_u8string<20> ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**Hello");
    ssSdvString2.insert(std::string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**He**llo**");

    // C++ string assignment
    sdv::fixed_u8string<20> ssSdvString4;
    std::string ss("Hello");
    std::string ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, "**Hello");
    ssSdvString4.insert(std::string::npos, ss2);
    EXPECT_EQ(ssSdvString4, "**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, "**He**llo**");

    // Substring assignment
    sdv::fixed_u8string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, "eellll");
    ssSdvString7.insert(std::string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, "eellllell");

    // Iterator assignment
    sdv::fixed_u8string<20> ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), '+');
    EXPECT_EQ(ssSdvString12, "+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), '-');
    EXPECT_EQ(ssSdvString12, "-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), '+');
    EXPECT_EQ(ssSdvString12, "-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), '-');
    EXPECT_EQ(ssSdvString12, "-+Hello+-");
    sdv::fixed_u8string<20>::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, '#');
    EXPECT_EQ(ssSdvString12, "-#+Hello+-");
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, '+');
    EXPECT_EQ(ssSdvString12, "++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, '+');
    EXPECT_EQ(ssSdvString12, "--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, '#');
    EXPECT_EQ(ssSdvString12, "-##-++Hello++--");
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, '+');
    EXPECT_EQ(ssSdvString12, "++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, '+');
    EXPECT_EQ(ssSdvString12, "--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, '#');
    EXPECT_EQ(ssSdvString12, "-##-++Hello++--");
    ssSdvString12			 = "Hello";
    sdv::fixed_u8string<20> ssSdvString8 = "12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "112212Hello1212");

    // Initializer list
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {'1', '2'});
    EXPECT_EQ(ssSdvString12, "112212Hello1212");
}

TEST_F(CUtf8StringTypeTest, InsertFunctionDynamic)
{
    // Character assignment
    sdv::u8string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.insert(0, 10, '*');
    EXPECT_EQ(ssSdvString1, "**********");
    ssSdvString1.insert(0, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********");
    ssSdvString1.insert(ssSdvString1.size(), 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********??");
    ssSdvString1.insert(std::string::npos, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**********????");
    ssSdvString1.insert(4, 2, '?');
    EXPECT_EQ(ssSdvString1, "??**??********????");

    // C string assignment
    sdv::u8string ssSdvString5;
    const char	  sz1[] = "Hello";
    const char*	  sz2	= "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.insert(std::string::npos, sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5.insert(0, sz2);
    EXPECT_EQ(ssSdvString5, "**Hello");
    ssSdvString5.insert(std::string::npos, sz2);
    EXPECT_EQ(ssSdvString5, "**Hello**");
    ssSdvString5.insert(4, sz2);
    EXPECT_EQ(ssSdvString5, "**He**llo**");

    // String copy assignment
    sdv::u8string ssSdvString2;
    sdv::u8string ssSdvString3("Hello");
    sdv::u8string ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.insert(3, ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2.insert(0, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**Hello");
    ssSdvString2.insert(std::string::npos, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**Hello**");
    ssSdvString2.insert(4, ssSdvString6);
    EXPECT_EQ(ssSdvString2, "**He**llo**");

    // C++ string assignment
    sdv::u8string ssSdvString4;
    std::string	  ss("Hello");
    std::string	  ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.insert(0, ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4.insert(0, ss2);
    EXPECT_EQ(ssSdvString4, "**Hello");
    ssSdvString4.insert(std::string::npos, ss2);
    EXPECT_EQ(ssSdvString4, "**Hello**");
    ssSdvString4.insert(4, ss2);
    EXPECT_EQ(ssSdvString4, "**He**llo**");

    // Substring assignment
    sdv::u8string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.insert(0, ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    ssSdvString7.insert(1, ss, 1, 3);
    EXPECT_EQ(ssSdvString7, "eellll");
    ssSdvString7.insert(std::string::npos, sz1 + 1, 3);
    EXPECT_EQ(ssSdvString7, "eellllell");

    // Iterator assignment
    sdv::u8string ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), '+');
    EXPECT_EQ(ssSdvString12, "+Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), '-');
    EXPECT_EQ(ssSdvString12, "-+Hello");
    ssSdvString12.insert(ssSdvString12.end(), '+');
    EXPECT_EQ(ssSdvString12, "-+Hello+");
    ssSdvString12.insert(ssSdvString12.cend(), '-');
    EXPECT_EQ(ssSdvString12, "-+Hello+-");
    sdv::u8string::iterator itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, '#');
    EXPECT_EQ(ssSdvString12, "-#+Hello+-");
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, '+');
    EXPECT_EQ(ssSdvString12, "++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, '+');
    EXPECT_EQ(ssSdvString12, "--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, '#');
    EXPECT_EQ(ssSdvString12, "-##-++Hello++--");
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), 2, '+');
    EXPECT_EQ(ssSdvString12, "++Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello");
    ssSdvString12.insert(ssSdvString12.end(), 2, '+');
    EXPECT_EQ(ssSdvString12, "--++Hello++");
    ssSdvString12.insert(ssSdvString12.cend(), 2, '-');
    EXPECT_EQ(ssSdvString12, "--++Hello++--");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, 2, '#');
    EXPECT_EQ(ssSdvString12, "-##-++Hello++--");
    ssSdvString12			   = "Hello";
    sdv::u8string ssSdvString8 = "12";
    ssSdvString12.insert(ssSdvString12.begin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, ssSdvString8.begin(), ssSdvString8.end());
    EXPECT_EQ(ssSdvString12, "112212Hello1212");

    // Initializer list
    ssSdvString12 = "Hello";
    ssSdvString12.insert(ssSdvString12.begin(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "12Hello");
    ssSdvString12.insert(ssSdvString12.cbegin(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello");
    ssSdvString12.insert(ssSdvString12.end(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello12");
    ssSdvString12.insert(ssSdvString12.cend(), {'1', '2'});
    EXPECT_EQ(ssSdvString12, "1212Hello1212");
    itPos = ssSdvString12.begin();
    ++itPos;
    ssSdvString12.insert(itPos, {'1', '2'});
    EXPECT_EQ(ssSdvString12, "112212Hello1212");
}

TEST_F(CUtf8StringTypeTest, InsertFunctionMixed)
{
    // String copy assignment - static into dynamic
    sdv::u8string ssDynamic;
    sdv::fixed_u8string<20> ssStatic1(u8"Hello");
    sdv::fixed_u8string<20> ssStatic2(u8"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(3, ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"Hello");
    ssDynamic.insert(0, ssStatic2);
    EXPECT_EQ(ssDynamic, u8"**Hello");
    ssDynamic.insert(sdv::u8string::npos, ssStatic2);
    EXPECT_EQ(ssDynamic, u8"**Hello**");
    ssDynamic.insert(4, ssStatic2);
    EXPECT_EQ(ssDynamic, u8"**He**llo**");

    // String copy assignment - dynamic into static
    sdv::fixed_u8string<20> ssStatic;
    sdv::fixed_u8string<20> ssDynamic1(u8"Hello");
    sdv::fixed_u8string<20> ssDynamic2(u8"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(3, ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"Hello");
    ssStatic.insert(0, ssDynamic2);
    EXPECT_EQ(ssStatic, u8"**Hello");
    ssStatic.insert(sdv::u8string::npos, ssDynamic2);
    EXPECT_EQ(ssStatic, u8"**Hello**");
    ssStatic.insert(4, ssDynamic2);
    EXPECT_EQ(ssStatic, u8"**He**llo**");

    // Substring assignment - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.insert(0, ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, u8"ell");

    // Substring assignment - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.insert(0, ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, u8"ell");
}

TEST_F(CUtf8StringTypeTest, EraseFunction)
{
    // Erase index based
    sdv::u8string ssSdvString = "12345678901234567890";
    ssSdvString.erase(8, 5);
    EXPECT_EQ(ssSdvString, "123456784567890");
    ssSdvString.erase(0, 2);
    EXPECT_EQ(ssSdvString, "3456784567890");
    ssSdvString.erase(ssSdvString.size(), 10);
    EXPECT_EQ(ssSdvString, "3456784567890");
    ssSdvString.erase(sdv::u8string::npos);
    EXPECT_EQ(ssSdvString, "3456784567890");
    ssSdvString.erase(8);
    EXPECT_EQ(ssSdvString, "34567845");

    // Erase iterator
    ssSdvString = "12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin());
    EXPECT_EQ(ssSdvString, "2345678901234567890");
    ssSdvString.erase(ssSdvString.cend());
    EXPECT_EQ(ssSdvString, "2345678901234567890");
    ssSdvString.erase(ssSdvString.cbegin() + 5);
    EXPECT_EQ(ssSdvString, "234568901234567890");
    sdv::u8string::iterator itPos = ssSdvString.erase(ssSdvString.end() - 2);
    EXPECT_EQ(ssSdvString, "23456890123456780");
    EXPECT_EQ(*itPos, '0');
    ++itPos;
    EXPECT_EQ(itPos, ssSdvString.cend());

    // Erase iterator range
    ssSdvString = "12345678901234567890";
    ssSdvString.erase(ssSdvString.cbegin(), ssSdvString.cbegin() + 4);
    EXPECT_EQ(ssSdvString, "5678901234567890");
    itPos = ssSdvString.erase(ssSdvString.cbegin() + 4, ssSdvString.cbegin() + 6);
    EXPECT_EQ(ssSdvString, "56781234567890");
    EXPECT_EQ(*itPos, '1');
    itPos += 10;
    EXPECT_EQ(itPos, ssSdvString.cend());
}

TEST_F(CUtf8StringTypeTest, PushPopFunctions)
{
    sdv::u8string ssSdvString;
    ssSdvString.push_back('1');
    EXPECT_EQ(ssSdvString, "1");
    ssSdvString.push_back('2');
    EXPECT_EQ(ssSdvString, "12");
    ssSdvString.pop_back();
    EXPECT_EQ(ssSdvString, "1");
    ssSdvString.pop_back();
    EXPECT_TRUE(ssSdvString.empty());
    EXPECT_NO_THROW(ssSdvString.pop_back());
}

TEST_F(CUtf8StringTypeTest, AppendFunctionStatic)
{
    // Append characters
    sdv::fixed_u8string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, '*');
    EXPECT_EQ(ssSdvString1, "**********");
    ssSdvString1.append(2, '?');
    EXPECT_EQ(ssSdvString1, "**********??");

    // Append string
    sdv::fixed_u8string<20> ssSdvString2;
    sdv::fixed_u8string<10> ssSdvString3("Hello");
    sdv::fixed_u8string<15> ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, "Hello**");

    // Append C++ string
    sdv::fixed_u8string<20> ssSdvString4;
    std::string ss("Hello");
    std::string ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, "Hello**");

    // Append substring
    sdv::fixed_u8string<20> ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, "ellell");

    // C string assignment
    sdv::fixed_u8string<20> ssSdvString5;
    const char	sz1[] = "Hello";
    const char* sz2	  = "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, "Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, "Hello**Hello");

    // Iterator based
    sdv::fixed_u8string<20> ssSdvString12 = "Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, "Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = "Hello";
    ssSdvString12.append({'1', '2'});
    EXPECT_EQ(ssSdvString12, "Hello12");
}

TEST_F(CUtf8StringTypeTest, AppendFunctionDynamic)
{
    // Append characters
    sdv::u8string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1.append(10, '*');
    EXPECT_EQ(ssSdvString1, "**********");
    ssSdvString1.append(2, '?');
    EXPECT_EQ(ssSdvString1, "**********??");

    // Append string
    sdv::u8string ssSdvString2;
    sdv::u8string ssSdvString3("Hello");
    sdv::u8string ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2.append(ssSdvString3);
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2.append(ssSdvString6);
    EXPECT_EQ(ssSdvString2, "Hello**");

    // Append C++ string
    sdv::u8string ssSdvString4;
    std::string ss("Hello");
    std::string ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4.append(ss);
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4.append(ss2);
    EXPECT_EQ(ssSdvString4, "Hello**");

    // Append substring
    sdv::u8string ssSdvString7;
    EXPECT_TRUE(ssSdvString7.empty());
    ssSdvString7.append(ssSdvString3, 1, 3);
    EXPECT_EQ(ssSdvString7, "ell");
    ssSdvString7.append(ss, 1, 3);
    EXPECT_EQ(ssSdvString7, "ellell");

    // C string assignment
    sdv::u8string ssSdvString5;
    const char	sz1[] = "Hello";
    const char* sz2	  = "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5.append(sz2);
    EXPECT_EQ(ssSdvString5, "Hello**");
    ssSdvString5.append(sz1);
    EXPECT_EQ(ssSdvString5, "Hello**Hello");

    // Iterator based
    sdv::u8string ssSdvString12 = "Hello";
    ssSdvString12.append(ssSdvString7.begin(), ssSdvString7.end());
    EXPECT_EQ(ssSdvString12, "Helloellell");
    EXPECT_THROW(ssSdvString12.append(ssSdvString7.begin(), ssSdvString12.end()), sdv::XIndexOutOfRange);

    // Initializer list
    ssSdvString12 = "Hello";
    ssSdvString12.append({'1', '2'});
    EXPECT_EQ(ssSdvString12, "Hello12");
}

TEST_F(CUtf8StringTypeTest, AppendFunctionMixed)
{
    // Append string - static into dynamic
    sdv::u8string ssDynamic;
    sdv::fixed_u8string<10> ssStatic1(u8"Hello");
    sdv::fixed_u8string<15> ssStatic2(u8"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1);
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"Hello");
    ssDynamic.append(ssStatic2);
    EXPECT_EQ(ssDynamic, u8"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u8string<20> ssStatic;
    sdv::u8string ssDynamic1(u8"Hello");
    sdv::u8string ssDynamic2(u8"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1);
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"Hello");
    ssStatic.append(ssDynamic2);
    EXPECT_EQ(ssStatic, u8"Hello**");

    // Append substring - static into dynamic
    ssDynamic.clear();
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic.append(ssStatic1, 1, 3);
    EXPECT_EQ(ssDynamic, u8"ell");

    // Append substring - dynamic into static
    ssStatic.clear();
    EXPECT_TRUE(ssStatic.empty());
    ssStatic.append(ssDynamic1, 1, 3);
    EXPECT_EQ(ssStatic, u8"ell");
}

TEST_F(CUtf8StringTypeTest, AppendOperatorStatic)
{
    // Append string
    sdv::fixed_u8string<20> ssSdvString2;
    sdv::fixed_u8string<15> ssSdvString3("Hello");
    sdv::fixed_u8string<10> ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, "Hello**");

    // Append C++ string
    sdv::fixed_u8string<20> ssSdvString4;
    std::string	  ss("Hello");
    std::string	  ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, "Hello**");

    // Append character
    sdv::fixed_u8string<20> ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, "*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, "*?");

    // C string assignment
    sdv::fixed_u8string<20> ssSdvString5;
    const char	  sz1[] = "Hello";
    const char*	  sz2	= "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, "Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, "Hello**Hello");

    // Initializer list
    sdv::fixed_u8string<20> ssSdvString12 = "Hello";
    ssSdvString12 += {'1', '2'};
    EXPECT_EQ(ssSdvString12, "Hello12");
}

TEST_F(CUtf8StringTypeTest, AppendOperatorDynamic)
{
    // Append string
    sdv::u8string ssSdvString2;
    sdv::u8string ssSdvString3("Hello");
    sdv::u8string ssSdvString6("**");
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString2 += ssSdvString3;
    EXPECT_EQ(ssSdvString2, ssSdvString3);
    EXPECT_EQ(ssSdvString2, "Hello");
    ssSdvString2 += ssSdvString6;
    EXPECT_EQ(ssSdvString2, "Hello**");

    // Append C++ string
    sdv::u8string ssSdvString4;
    std::string ss("Hello");
    std::string ss2("**");
    EXPECT_TRUE(ssSdvString4.empty());
    ssSdvString4 += ss;
    EXPECT_EQ(ssSdvString4, ss);
    EXPECT_EQ(ssSdvString4, "Hello");
    ssSdvString4 += ss2;
    EXPECT_EQ(ssSdvString4, "Hello**");

    // Append character
    sdv::u8string ssSdvString1;
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 += '*';
    EXPECT_EQ(ssSdvString1, "*");
    ssSdvString1 += '?';
    EXPECT_EQ(ssSdvString1, "*?");

    // C string assignment
    sdv::u8string ssSdvString5;
    const char	sz1[] = "Hello";
    const char* sz2	  = "**";
    EXPECT_TRUE(ssSdvString5.empty());
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, "Hello");
    ssSdvString5 += sz2;
    EXPECT_EQ(ssSdvString5, "Hello**");
    ssSdvString5 += sz1;
    EXPECT_EQ(ssSdvString5, "Hello**Hello");

    // Initializer list
    sdv::u8string ssSdvString12 = "Hello";
    ssSdvString12 += {'1', '2'};
    EXPECT_EQ(ssSdvString12, "Hello12");
}

TEST_F(CUtf8StringTypeTest, AppendOperatorMixed)
{
    // Append string - static into dynamic
    sdv::u8string ssDynamic;
    sdv::fixed_u8string<15> ssStatic1(u8"Hello");
    sdv::fixed_u8string<10> ssStatic2(u8"**");
    EXPECT_TRUE(ssDynamic.empty());
    ssDynamic += ssStatic1;
    EXPECT_EQ(ssDynamic, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"Hello");
    ssDynamic += ssStatic2;
    EXPECT_EQ(ssDynamic, u8"Hello**");

    // Append string - dynamic into static
    sdv::fixed_u8string<20> ssStatic;
    sdv::u8string ssDynamic1(u8"Hello");
    sdv::u8string ssDynamic2(u8"**");
    EXPECT_TRUE(ssStatic.empty());
    ssStatic += ssDynamic1;
    EXPECT_EQ(ssStatic, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"Hello");
    ssStatic += ssStatic2;
    EXPECT_EQ(ssStatic, u8"Hello**");
}

TEST_F(CUtf8StringTypeTest, CompareFunctionStatic)
{
    // Compare strings
    sdv::fixed_u8string<20> ssSdvString1;
    sdv::fixed_u8string<10> ssSdvString2("Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::string ssString("Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char* sz1 = "Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char sz2[] = "Hello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = "Kello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssSdvString2, 1, sdv::fixed_u8string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = "Kello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString, 1, sdv::fixed_u8string<20>::npos), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, ssString, 1, sdv::fixed_u8string<20>::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = "Kellogg";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char sz3[] = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz3), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz3), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz3), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz3), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char sz4[] = "Kellogg";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::fixed_u8string<20>::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf8StringTypeTest, CompareFunctionDynamic)
{
    // Compare strings
    sdv::u8string ssSdvString1;
    sdv::u8string ssSdvString2("Hello");
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(ssSdvString2), 0);

    // Compare with C++ string
    ssSdvString1.clear();
    std::string ssString("Hello");
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(ssString), 0);

    // Compare with C string
    ssSdvString1.clear();
    const char* sz1 = "Hello";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(sz1), 0);
    const char sz2[] = "Hello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(sz2), 0);

    // Compare substring
    ssSdvString2 = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2), 0);
    ssSdvString2 = "Kello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssSdvString2, 1, sdv::u8string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssSdvString2, 1, 4), 0);

    // Compare C++ substring
    ssString = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssString), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssString), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, ssString), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssString), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssString), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString), 0);
    ssString	 = "Kello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssString, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, ssString, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, ssString, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssString, 1, sdv::u8string::npos), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, ssString, 1, sdv::u8string::npos), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, ssString, 1, 4), 0);

    // Compare C substring
    sz1	 = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz1), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz1), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, sz1), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz1), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz1), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1), 0);
    sz1	 = "Kellogg";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz1 + 1, 4), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz1 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz1 + 1, 4), 0);
    const char sz3[] = "ello";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz3), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz3), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, sz3), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz3), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz3), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz3), 0);
    const char sz4[] = "Kellogg";
    ssSdvString1 = "Helln";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "HellnO";
    EXPECT_LT(ssSdvString1.compare(1, sdv::u8string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Hello";
    EXPECT_EQ(ssSdvString1.compare(1, sdv::u8string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Hellp";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz4 + 1, 4), 0);
    ssSdvString1 = "Helloa";
    EXPECT_GT(ssSdvString1.compare(1, sdv::u8string::npos, sz4 + 1, 4), 0);
    EXPECT_EQ(ssSdvString1.compare(1, 4, sz4 + 1, 4), 0);
}

TEST_F(CUtf8StringTypeTest, CompareFunctionMixed)
{
    // Compare strings - compare dynamic with static
    sdv::u8string ssDynamic;
    sdv::fixed_u8string<10> ssStatic1("Hello");
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = "Helln";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = "HellnO";
    EXPECT_LT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = "Hello";
    EXPECT_EQ(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = "Hellp";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);
    ssDynamic = "Helloa";
    EXPECT_GT(ssDynamic.compare(ssStatic1), 0);

    // Compare substring
    ssStatic1 = "ello";
    ssDynamic = "Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1), 0);
    ssDynamic = "HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1), 0);
    ssDynamic = "Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1), 0);
    ssDynamic = "Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1), 0);
    ssDynamic = "Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1), 0);
    ssStatic1 = "Kello";
    ssDynamic = "Helln";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssDynamic = "HellnO";
    EXPECT_LT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssDynamic = "Hello";
    EXPECT_EQ(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssDynamic = "Hellp";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssDynamic = "Helloa";
    EXPECT_GT(ssDynamic.compare(1, sdv::fixed_u8string<20>::npos, ssStatic1, 1, sdv::fixed_u8string<20>::npos), 0);
    EXPECT_EQ(ssDynamic.compare(1, 4, ssStatic1, 1, 4), 0);

    // Compare strings - compare static with dynamic
    sdv::fixed_u8string<10> ssStatic;
    sdv::u8string ssDynamic1("Hello");
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = "Helln";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = "HellnO";
    EXPECT_LT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = "Hello";
    EXPECT_EQ(ssStatic.compare(ssDynamic1), 0);
    ssStatic = "Hellp";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);
    ssStatic = "Helloa";
    EXPECT_GT(ssStatic.compare(ssDynamic1), 0);

    // Compare substring
    ssDynamic1 = "ello";
    ssStatic = "Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1), 0);
    ssStatic = "HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1), 0);
    ssStatic = "Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1), 0);
    ssStatic = "Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1), 0);
    ssStatic = "Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1), 0);
    ssDynamic1 = "Kello";
    ssStatic = "Helln";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssStatic = "HellnO";
    EXPECT_LT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssStatic = "Hello";
    EXPECT_EQ(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssStatic = "Hellp";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1, 1, sdv::fixed_u8string<20>::npos), 0);
    ssStatic = "Helloa";
    EXPECT_GT(ssStatic.compare(1, sdv::fixed_u8string<20>::npos, ssDynamic1, 1, sdv::fixed_u8string<20>::npos), 0);
    EXPECT_EQ(ssStatic.compare(1, 4, ssDynamic1, 1, 4), 0);
}

TEST_F(CUtf8StringTypeTest, CompareOperatorStatic)
{
    // Compare strings
    sdv::fixed_u8string<20> ssSdvString1;
    sdv::fixed_u8string<10> ssSdvString2("Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::string ssString("Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char* sz1 = "Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char sz2[] = "Hello";
    ssSdvString1	 = "Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf8StringTypeTest, CompareOperatorDynamic)
{
    // Compare strings
    sdv::u8string ssSdvString1;
    sdv::u8string ssSdvString2("Hello");
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < ssSdvString2);
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 <= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 == ssSdvString2);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > ssSdvString2);
    EXPECT_TRUE(ssSdvString1 >= ssSdvString2);
    EXPECT_TRUE(ssSdvString1 != ssSdvString2);

    // Compare with C++ string
    ssSdvString1.clear();
    std::string ssString("Hello");
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < ssString);
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString > ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 <= ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 == ssString);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString >= ssSdvString1);
    EXPECT_TRUE(ssString == ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > ssString);
    EXPECT_TRUE(ssSdvString1 >= ssString);
    EXPECT_TRUE(ssSdvString1 != ssString);
    EXPECT_TRUE(ssString < ssSdvString1);
    EXPECT_TRUE(ssString <= ssSdvString1);
    EXPECT_TRUE(ssString != ssSdvString1);

    // Compare with C string
    ssSdvString1.clear();
    const char* sz1 = "Hello";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Helln";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 > ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 == sz1);
    EXPECT_TRUE(ssSdvString1 <= sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(sz1 == ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 >= ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > sz1);
    EXPECT_TRUE(ssSdvString1 >= sz1);
    EXPECT_TRUE(ssSdvString1 != sz1);
    EXPECT_TRUE(sz1 < ssSdvString1);
    EXPECT_TRUE(sz1 <= ssSdvString1);
    EXPECT_TRUE(sz1 != ssSdvString1);
    const char sz2[] = "Hello";
    ssSdvString1	 = "Helln";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "HellnO";
    EXPECT_TRUE(ssSdvString1 < sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 > ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "Hello";
    EXPECT_TRUE(ssSdvString1 == sz2);
    EXPECT_TRUE(ssSdvString1 <= sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(sz2 == ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 >= ssSdvString1);
    ssSdvString1 = "Hellp";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
    ssSdvString1 = "Helloa";
    EXPECT_TRUE(ssSdvString1 > sz2);
    EXPECT_TRUE(ssSdvString1 >= sz2);
    EXPECT_TRUE(ssSdvString1 != sz2);
    EXPECT_TRUE(sz2 < ssSdvString1);
    EXPECT_TRUE(sz2 <= ssSdvString1);
    EXPECT_TRUE(sz2 != ssSdvString1);
}

TEST_F(CUtf8StringTypeTest, CompareOperatorMixed)
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

TEST_F(CUtf8StringTypeTest, ReplaceFunctionStatic)
{
    // Replace string
    sdv::fixed_u8string<20> ssSdvString = "1234567890";
    sdv::fixed_u8string<10> ssSdvString2 = "abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString	 = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace C++ string
    ssSdvString	 = "1234567890";
    std::string ss = "abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ss);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace C string
    ssSdvString	 = "1234567890";
    const char sz1[] = "abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz1);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString		 = "1234567890";
    const char* sz2 = "abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz2);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace substring
    ssSdvString	 = "1234567890";
    ssSdvString2 = "xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = "1234567890";
    ss = "xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = "1234567890";
    const char sz3[] = "abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    sz2 = "abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace with characters
    ssSdvString = "1234567890";
    ssSdvString.replace(1, 2, 4, '+');
    EXPECT_EQ(ssSdvString, "1++++4567890");
    ssSdvString.replace(2, 6, 4, '-');
    EXPECT_EQ(ssSdvString, "1+----7890");
    ssSdvString.replace(3, 4, 4, '*');
    EXPECT_EQ(ssSdvString, "1+-****890");
    ssSdvString.replace(0, 0, 4, '#');
    EXPECT_EQ(ssSdvString, "####1+-****890");
    ssSdvString.replace(std::string::npos, 100, 4, '~');
    EXPECT_EQ(ssSdvString, "####1+-****890~~~~");
    ssSdvString.replace(0, std::string::npos, 4, '=');
    EXPECT_EQ(ssSdvString, "====");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, '+');
    EXPECT_EQ(ssSdvString, "1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, '-');
    EXPECT_EQ(ssSdvString, "1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, '*');
    EXPECT_EQ(ssSdvString, "1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, '#');
    EXPECT_EQ(ssSdvString, "####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, '~');
    EXPECT_EQ(ssSdvString, "####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, '=');
    EXPECT_EQ(ssSdvString, "====");

    // Replace with iterators
    ssSdvString = "1234567890";
    ss = "abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace with initialization list
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd");
}

TEST_F(CUtf8StringTypeTest, ReplaceFunctionDynamic)
{
    // Replace string
    sdv::u8string ssSdvString = "1234567890";
    sdv::u8string ssSdvString2 = "abcd";
    ssSdvString.replace(1, 2, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString	 = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ssSdvString2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ssSdvString2);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace C++ string
    ssSdvString	 = "1234567890";
    std::string ss = "abcd";
    ssSdvString.replace(1, 2, ss);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ss);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ss);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ss);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace C string
    ssSdvString	 = "1234567890";
    const char sz1[] = "abcd";
    ssSdvString.replace(1, 2, sz1);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz1);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz1);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz1);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz1);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz1);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz1);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz1);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString		 = "1234567890";
    const char* sz2 = "abcd";
    ssSdvString.replace(1, 2, sz2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz2);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz2);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz2);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz2);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz2);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace substring
    ssSdvString	 = "1234567890";
    ssSdvString2 = "xxabcdxx";
    ssSdvString.replace(1, 2, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ssSdvString2, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ssSdvString2, 2, 4);

    // Replace C++ substring
    ssSdvString	 = "1234567890";
    ss = "xxabcdxx";
    ssSdvString.replace(1, 2, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, ss, 2, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, ss, 2, 4);

    // Replace C substring
    ssSdvString		 = "1234567890";
    const char sz3[] = "abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    sz2 = "abcd**";
    ssSdvString.replace(1, 2, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(2, 6, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(3, 4, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(0, 0, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(std::string::npos, 100, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(0, std::string::npos, sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, sz3, 4);
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, sz3, 4);
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), sz3, 4);
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace with characters
    ssSdvString = "1234567890";
    ssSdvString.replace(1, 2, 4, '+');
    EXPECT_EQ(ssSdvString, "1++++4567890");
    ssSdvString.replace(2, 6, 4, '-');
    EXPECT_EQ(ssSdvString, "1+----7890");
    ssSdvString.replace(3, 4, 4, '*');
    EXPECT_EQ(ssSdvString, "1+-****890");
    ssSdvString.replace(0, 0, 4, '#');
    EXPECT_EQ(ssSdvString, "####1+-****890");
    ssSdvString.replace(std::string::npos, 100, 4, '~');
    EXPECT_EQ(ssSdvString, "####1+-****890~~~~");
    ssSdvString.replace(0, std::string::npos, 4, '=');
    EXPECT_EQ(ssSdvString, "====");
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, 4, '+');
    EXPECT_EQ(ssSdvString, "1++++4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, 4, '-');
    EXPECT_EQ(ssSdvString, "1+----7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, 4, '*');
    EXPECT_EQ(ssSdvString, "1+-****890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), 4, '#');
    EXPECT_EQ(ssSdvString, "####1+-****890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), 4, '~');
    EXPECT_EQ(ssSdvString, "####1+-****890~~~~");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), 4, '=');
    EXPECT_EQ(ssSdvString, "====");

    // Replace with iterators
    ssSdvString = "1234567890";
    ss = "abcd";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), ss.begin(), ss.end());
    EXPECT_EQ(ssSdvString, "abcd");

    // Replace with initialization list
    ssSdvString = "1234567890";
    ssSdvString.replace(ssSdvString.begin() + 1, ssSdvString.begin() + 3, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1abcd4567890");
    ssSdvString.replace(ssSdvString.begin() + 2, ssSdvString.begin() + 8, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1aabcd7890");
    ssSdvString.replace(ssSdvString.begin() + 3, ssSdvString.begin() + 7, {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "1aaabcd890");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.begin(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890");
    ssSdvString.replace(ssSdvString.end(), ssSdvString.end(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd1aaabcd890abcd");
    ssSdvString.replace(ssSdvString.begin(), ssSdvString.end(), {'a', 'b', 'c', 'd'});
    EXPECT_EQ(ssSdvString, "abcd");
}

TEST_F(CUtf8StringTypeTest, ReplaceFunctionMixed)
{
    // Replace string - replace dynamic by static
    sdv::u8string ssDynamic = u8"1234567890";
    sdv::fixed_u8string<10> ssStatic1 = u8"abcd";
    ssDynamic.replace(1, 2, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890");
    ssDynamic.replace(std::string::npos, 100, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::string::npos, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd");
    ssDynamic = u8"1234567890";
    ssDynamic.replace(ssDynamic.begin() + 1, ssDynamic.begin() + 3, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1abcd4567890");
    ssDynamic.replace(ssDynamic.begin() + 2, ssDynamic.begin() + 8, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1aabcd7890");
    ssDynamic.replace(ssDynamic.begin() + 3, ssDynamic.begin() + 7, ssStatic1);
    EXPECT_EQ(ssDynamic, u8"1aaabcd890");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.begin(), ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890");
    ssDynamic.replace(ssDynamic.end(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890abcd");
    ssDynamic.replace(ssDynamic.begin(), ssDynamic.end(), ssStatic1);
    EXPECT_EQ(ssDynamic, u8"abcd");

    // Replace substring
    ssDynamic = u8"1234567890";
    ssStatic1 = u8"xxabcdxx";
    ssDynamic.replace(1, 2, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u8"1abcd4567890");
    ssDynamic.replace(2, 6, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u8"1aabcd7890");
    ssDynamic.replace(3, 4, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u8"1aaabcd890");
    ssDynamic.replace(0, 0, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890");
    ssDynamic.replace(std::string::npos, 100, ssStatic1, 2, 4);
    EXPECT_EQ(ssDynamic, u8"abcd1aaabcd890abcd");
    ssDynamic.replace(0, std::string::npos, ssStatic1, 2, 4);

    // Replace string - replace static by dynamic
    sdv::fixed_u8string<20> ssStatic = u8"1234567890";
    sdv::u8string ssDynamic1 = u8"abcd";
    ssStatic.replace(1, 2, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890");
    ssStatic.replace(std::string::npos, 100, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::string::npos, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd");
    ssStatic = u8"1234567890";
    ssStatic.replace(ssStatic.begin() + 1, ssStatic.begin() + 3, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1abcd4567890");
    ssStatic.replace(ssStatic.begin() + 2, ssStatic.begin() + 8, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1aabcd7890");
    ssStatic.replace(ssStatic.begin() + 3, ssStatic.begin() + 7, ssDynamic1);
    EXPECT_EQ(ssStatic, u8"1aaabcd890");
    ssStatic.replace(ssStatic.begin(), ssStatic.begin(), ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890");
    ssStatic.replace(ssStatic.end(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890abcd");
    ssStatic.replace(ssStatic.begin(), ssStatic.end(), ssDynamic1);
    EXPECT_EQ(ssStatic, u8"abcd");

    // Replace substring
    ssStatic = u8"1234567890";
    ssDynamic1 = u8"xxabcdxx";
    ssStatic.replace(1, 2, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u8"1abcd4567890");
    ssStatic.replace(2, 6, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u8"1aabcd7890");
    ssStatic.replace(3, 4, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u8"1aaabcd890");
    ssStatic.replace(0, 0, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890");
    ssStatic.replace(std::string::npos, 100, ssDynamic1, 2, 4);
    EXPECT_EQ(ssStatic, u8"abcd1aaabcd890abcd");
    ssStatic.replace(0, std::string::npos, ssDynamic1, 2, 4);
}

TEST_F(CUtf8StringTypeTest, SubstringFunctionStatic)
{
    sdv::fixed_u8string<20> ssSdvString = "1234567890";
    EXPECT_EQ(ssSdvString.substr(), "1234567890");
    EXPECT_EQ(ssSdvString.substr(2), "34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), "34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf8StringTypeTest, SubstringFunctionDynamic)
{
    sdv::u8string ssSdvString = "1234567890";
    EXPECT_EQ(ssSdvString.substr(), "1234567890");
    EXPECT_EQ(ssSdvString.substr(2), "34567890");
    EXPECT_EQ(ssSdvString.substr(2, 2), "34");
    EXPECT_TRUE(ssSdvString.substr(ssSdvString.size()).empty());
}

TEST_F(CUtf8StringTypeTest, CopyFunction)
{
    sdv::u8string ssSdvString = "1234567890";
    char sz[32] = {};
    EXPECT_EQ(ssSdvString.copy(sz, 2, 5), 2);
    EXPECT_STREQ(sz, "67");
    std::fill_n(sz, 32, static_cast<char>('\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 20, 5), 5);
    EXPECT_STREQ(sz, "67890");
    std::fill_n(sz, 32, static_cast<char>('\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0, 5), 0);
    EXPECT_STREQ(sz, "");
    std::fill_n(sz, 32, static_cast<char>('\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 2), 2);
    EXPECT_STREQ(sz, "12");
    std::fill_n(sz, 32, static_cast<char>('\0'));
    EXPECT_EQ(ssSdvString.copy(sz, 0), 0);
    EXPECT_STREQ(sz, "");
    std::fill_n(sz, 32, static_cast<char>('\0'));
    EXPECT_EQ(ssSdvString.copy(sz, std::string::npos), 10);
    EXPECT_STREQ(sz, "1234567890");
}

TEST_F(CUtf8StringTypeTest, SwapFunctionStatic)
{
    sdv::fixed_u8string<20> ssSdvString1 = "12345";
    sdv::fixed_u8string<10> ssSdvString2 = "67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, "67890");
    EXPECT_EQ(ssSdvString2, "12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, "12345");
    EXPECT_EQ(ssSdvString2, "67890");
}

TEST_F(CUtf8StringTypeTest, SwapFunctionDynamic)
{
    sdv::u8string ssSdvString1 = "12345";
    sdv::u8string ssSdvString2 = "67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, "67890");
    EXPECT_EQ(ssSdvString2, "12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, "12345");
    EXPECT_EQ(ssSdvString2, "67890");
}

TEST_F(CUtf8StringTypeTest, SwapFunctionMixed)
{
    sdv::fixed_u8string<20> ssSdvString1 = "12345";
    sdv::u8string ssSdvString2 = "67890";
    ssSdvString1.swap(ssSdvString2);
    EXPECT_EQ(ssSdvString1, "67890");
    EXPECT_EQ(ssSdvString2, "12345");

    sdv::swap(ssSdvString1, ssSdvString2);
    EXPECT_EQ(ssSdvString1, "12345");
    EXPECT_EQ(ssSdvString2, "67890");
}

TEST_F(CUtf8StringTypeTest, FindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "aabbaaccbbcc";
    sdv::fixed_u8string<10> ssSdvString2 = "aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::fixed_u8string<20>::npos);

    // Find C++ string in string
    std::string ss = "aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::fixed_u8string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 3, 4), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 9), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find('\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find('\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find('\0', 11), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "aabbaaccbbcc";
    sdv::u8string ssSdvString2 = "aa";
    EXPECT_EQ(ssSdvString1.find(ssSdvString2), 0);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ssSdvString2, 5), sdv::u8string::npos);

    // Find C++ string in string
    std::string ss = "aa";
    EXPECT_EQ(ssSdvString1.find(ss), 0);
    EXPECT_EQ(ssSdvString1.find(ss, 1), 4);
    EXPECT_EQ(ssSdvString1.find(ss, 5), sdv::u8string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u8string("aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 0, 4), 2);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 3, 4), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 0), 2);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 3), 8);
    EXPECT_EQ(ssSdvString1.find("bb\0a", 9), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find('\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find('\0', 5), 10);
    EXPECT_EQ(ssSdvString1.find('\0', 11), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, FindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u8string ssDynamic = u8"aabbaaccbbcc";
    sdv::fixed_u8string<10> ssStatic1 = u8"aa";
    EXPECT_EQ(ssDynamic.find(ssStatic1), 0);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 1), 4);
    EXPECT_EQ(ssDynamic.find(ssStatic1, 5), sdv::fixed_string<20>::npos);

    // Find string in string - find static in dynamic
    sdv::fixed_u8string<20> ssStatic = u8"aabbaaccbbcc";
    sdv::u8string ssDynamic1 = u8"aa";
    EXPECT_EQ(ssStatic.find(ssDynamic1), 0);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 1), 4);
    EXPECT_EQ(ssStatic.find(ssDynamic1, 5), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, ReverseFindFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "aabbaaccbbcc";
    sdv::fixed_u8string<10> ssSdvString2 = "aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::string ss = "aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", sdv::fixed_u8string<20>::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 1, 4), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", sdv::fixed_u8string<20>::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 1), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind('\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind('\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind('\0', 3), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, ReverseFindFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "aabbaaccbbcc";
    sdv::u8string ssSdvString2 = "aa";
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2), 4);
    EXPECT_EQ(ssSdvString1.rfind(ssSdvString2, 3), 0);

    // Find C++ string in string
    std::string ss = "aa";
    EXPECT_EQ(ssSdvString1.rfind(ss), 4);
    EXPECT_EQ(ssSdvString1.rfind(ss, 3), 0);

    // Find C string in string
    ssSdvString1 = sdv::u8string("aabb\0accbb\0c", 12);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", sdv::u8string::npos, 4), 2);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 1, 4), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", sdv::u8string::npos), 8);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 7), 2);
    EXPECT_EQ(ssSdvString1.rfind("bb\0a", 1), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.rfind('\0'), 10);
    EXPECT_EQ(ssSdvString1.rfind('\0', 9), 4);
    EXPECT_EQ(ssSdvString1.rfind('\0', 3), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, ReverseFindFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u8string ssDynamic = u8"aabbaaccbbcc";
    sdv::fixed_u8string<10> ssStatic1 = u8"aa";
    EXPECT_EQ(ssDynamic.rfind(ssStatic1), 4);
    EXPECT_EQ(ssDynamic.rfind(ssStatic1, 3), 0);

    // Find string in string - find static in dynamic
    sdv::fixed_u8string<20> ssStatic = u8"aabbaaccbbcc";
    sdv::u8string ssDynamic1 = u8"aa";
    EXPECT_EQ(ssStatic.rfind(ssDynamic1), 4);
    EXPECT_EQ(ssStatic.rfind(ssDynamic1, 3), 0);
}

TEST_F(CUtf8StringTypeTest, FindFirstOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "12341234";
    sdv::fixed_u8string<10> ssSdvString2 = "34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::fixed_u8string<20>::npos);

    // Find C++ string in string
    std::string ss = "34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::fixed_u8string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 10, 2), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 8), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 11), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindFirstOfFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "12341234";
    sdv::u8string ssSdvString2 = "34";
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ssSdvString2, 8), sdv::u8string::npos);

    // Find C++ string in string
    std::string ss = "34";
    EXPECT_EQ(ssSdvString1.find_first_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_of(ss, 8), sdv::u8string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u8string("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 0, 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 3, 2), 4);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 5, 2), 7);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 8, 2), 9);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 10, 2), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_of("c\0", 8), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 0), 4);
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 5), 9);
    EXPECT_EQ(ssSdvString1.find_first_of('\0', 11), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, FindFirstOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u8string ssDynamic = u8"12341234";
    sdv::fixed_u8string<10> ssStatic1 = u8"34";
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in sttaic
    sdv::fixed_u8string<10> ssStatic = u8"12341234";
    sdv::u8string ssDynamic1 = u8"34";
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindFirstNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "12341234";
    sdv::fixed_u8string<10> ssSdvString2 = "12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::fixed_u8string<20>::npos);

    // Find C++ string in string
    std::string ss = "12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::fixed_u8string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 9, 3), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 10), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 12), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindFirstNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "12341234";
    sdv::u8string ssSdvString2 = "12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ssSdvString2, 8), sdv::u8string::npos);

    // Find C++ string in string
    std::string ss = "12";
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 4), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of(ss, 8), sdv::u8string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u8string("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 0, 3), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 3, 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 4, 3), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 8, 3), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 9, 3), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 0), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 4), 4);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 5), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 9), 9);
    EXPECT_EQ(ssSdvString1.find_first_not_of("ab\0", 10), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 0), 0);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 1), 1);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 2), 2);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 3), 3);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 4), 5);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 6), 6);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 7), 7);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 8), 8);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 9), 10);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 11), 11);
    EXPECT_EQ(ssSdvString1.find_first_not_of('\0', 12), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, FindFirstNotOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u8string ssDynamic = u8"12341234";
    sdv::fixed_u8string<10> ssStatic1 = u8"12";
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1), 2);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 3), 3);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 4), 6);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 7), 7);
    EXPECT_EQ(ssDynamic.find_first_not_of(ssStatic1, 8), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u8string<10> ssStatic = u8"12341234";
    sdv::u8string ssDynamic1 = u8"12";
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1), 2);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 3), 3);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 4), 6);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 7), 7);
    EXPECT_EQ(ssStatic.find_first_not_of(ssDynamic1, 8), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "12341234";
    sdv::fixed_u8string<10> ssSdvString2 = "34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::fixed_u8string<20>::npos);

    // Find C++ string in string
    std::string ss = "34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::fixed_u8string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", std::string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 2, 2), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 2), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of('\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of('\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of('\0', 4), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastOfFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "12341234";
    sdv::u8string ssSdvString2 = "34";
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ssSdvString2, 2), sdv::u8string::npos);

    // Find C++ string in string
    std::string ss = "34";
    EXPECT_EQ(ssSdvString1.find_last_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_of(ss, 2), sdv::u8string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u8string("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", std::string::npos, 2), 9);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 9, 2), 7);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 7, 2), 4);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 4, 2), 2);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 2, 2), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0"), 7);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 7), 2);
    EXPECT_EQ(ssSdvString1.find_last_of("c\0", 2), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_of('\0'), 9);
    EXPECT_EQ(ssSdvString1.find_last_of('\0', 9), 4);
    EXPECT_EQ(ssSdvString1.find_last_of('\0', 4), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastOfFunctionMixed)
{
    // Find string in string - find static in dynamic
    sdv::u8string ssDynamic = u8"12341234";
    sdv::fixed_u8string<10> ssStatic1 = u8"34";
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u8string<10> ssStatic = u8"12341234";
    sdv::u8string ssDynamic1 = u8"34";
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastNotOfFunctionStatic)
{
    // Find string in string
    sdv::fixed_u8string<20> ssSdvString1 = "12341234";
    sdv::fixed_u8string<10> ssSdvString2 = "12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::fixed_u8string<20>::npos);

    // Find C++ string in string
    std::string ss = "12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::fixed_u8string<20>::npos);

    // Find C string in string
    ssSdvString1 = sdv::fixed_u8string<20>("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", std::string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 2, 3), sdv::fixed_u8string<20>::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 2), sdv::fixed_u8string<20>::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 0), sdv::fixed_u8string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastNotOfFunctionDynamic)
{
    // Find string in string
    sdv::u8string ssSdvString1 = "12341234";
    sdv::u8string ssSdvString2 = "12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ssSdvString2, 2), sdv::u8string::npos);

    // Find C++ string in string
    std::string ss = "12";
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 6), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of(ss, 2), sdv::u8string::npos);

    // Find C string in string
    ssSdvString1 = sdv::u8string("abcd\0abcd\0ab", 12);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", std::string::npos, 3), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 8, 3), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 7, 3), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 3, 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 2, 3), sdv::u8string::npos);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0"), 9);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 9), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 7), 4);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 4), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of("ab\0", 2), sdv::u8string::npos);

    // Find character in string
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0'), 11);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 11), 10);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 10), 8);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 8), 7);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 7), 6);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 6), 5);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 5), 3);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 3), 2);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 2), 1);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 1), 0);
    EXPECT_EQ(ssSdvString1.find_last_not_of('\0', 0), sdv::u8string::npos);
}

TEST_F(CUtf8StringTypeTest, FindLastNotOfFunctionMixed)
{
    // Find string in string - find static in dyanmic
    sdv::u8string ssDynamic = u8"12341234";
    sdv::fixed_u8string<10> ssStatic1 = u8"12";
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1), 7);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 7), 6);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 6), 3);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 3), 2);
    EXPECT_EQ(ssDynamic.find_last_not_of(ssStatic1, 2), sdv::fixed_string<20>::npos);

    // Find string in string - find dynamic in static
    sdv::fixed_u8string<10> ssStatic = u8"12341234";
    sdv::u8string ssDynamic1 = u8"12";
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1), 7);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 7), 6);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 6), 3);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 3), 2);
    EXPECT_EQ(ssStatic.find_last_not_of(ssDynamic1, 2), sdv::fixed_string<20>::npos);
}

TEST_F(CUtf8StringTypeTest, SumOperatorStatic)
{
    sdv::fixed_u8string<20> ssSdvString = "1234";
    std::string ss = "5678";

    EXPECT_EQ(ssSdvString + ssSdvString, "12341234");
    EXPECT_EQ(ss + ssSdvString, "56781234");
    EXPECT_EQ(ssSdvString + ss, "12345678");
    EXPECT_EQ("abcd" + ssSdvString, "abcd1234");
    EXPECT_EQ(ssSdvString + "abcd", "1234abcd");
    EXPECT_EQ('a' + ssSdvString, "a1234");
    EXPECT_EQ(ssSdvString + 'a', "1234a");
    sdv::fixed_u8string<20> ssSdvString1 = "1234";
    sdv::fixed_u8string<10> ssSdvString2 = "5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    ssSdvString2 = "5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + "abcd", "1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + 'a', "1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), "56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), "56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ("abcd" + std::move(ssSdvString1), "abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ('a' + std::move(ssSdvString1), "a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf8StringTypeTest, SumOperatorDynamic)
{
    sdv::u8string ssSdvString = "1234";
    std::string ss = "5678";

    EXPECT_EQ(ssSdvString + ssSdvString, "12341234");
    EXPECT_EQ(ss + ssSdvString, "56781234");
    EXPECT_EQ(ssSdvString + ss, "12345678");
    EXPECT_EQ("abcd" + ssSdvString, "abcd1234");
    EXPECT_EQ(ssSdvString + "abcd", "1234abcd");
    EXPECT_EQ('a' + ssSdvString, "a1234");
    EXPECT_EQ(ssSdvString + 'a', "1234a");
    sdv::u8string ssSdvString1 = "1234", ssSdvString2 = "5678";
    EXPECT_EQ(std::move(ssSdvString1) + std::move(ssSdvString2), "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_TRUE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    ssSdvString2 = "5678";
    EXPECT_EQ(std::move(ssSdvString1) + ssSdvString2, "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + ss, "12345678");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + "abcd", "1234abcd");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(std::move(ssSdvString1) + 'a', "1234a");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(ssSdvString2 + std::move(ssSdvString1), "56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    EXPECT_FALSE(ssSdvString2.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ(ss + std::move(ssSdvString1), "56781234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ("abcd" + std::move(ssSdvString1), "abcd1234");
    EXPECT_TRUE(ssSdvString1.empty());
    ssSdvString1 = "1234";
    EXPECT_EQ('a' + std::move(ssSdvString1), "a1234");
    EXPECT_TRUE(ssSdvString1.empty());
}

TEST_F(CUtf8StringTypeTest, SumOperatorMixed)
{
    // Add strings - add static onto dynamic
    sdv::u8string ssDynamic = u8"1234";
    sdv::fixed_u8string<20> ssStatic1 = u8"5678";
    EXPECT_EQ(ssDynamic + ssStatic1, u8"12345678");
    EXPECT_EQ(std::move(ssDynamic) + std::move(ssStatic1), u8"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_TRUE(ssStatic1.empty());
    ssDynamic = u8"1234";
    ssStatic1 = u8"5678";
    EXPECT_EQ(std::move(ssDynamic) + ssStatic1, u8"12345678");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());
    ssDynamic = u8"1234";
    EXPECT_EQ(ssStatic1 + std::move(ssDynamic), u8"56781234");
    EXPECT_TRUE(ssDynamic.empty());
    EXPECT_FALSE(ssStatic1.empty());

    // Add strings - add dynamic onto static
    sdv::fixed_u8string<20> ssStatic = u8"1234";
    sdv::u8string ssDynamic1 = u8"5678";
    EXPECT_EQ(ssStatic + ssDynamic1, u8"12345678");
    EXPECT_EQ(std::move(ssStatic) + std::move(ssDynamic1), u8"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_TRUE(ssDynamic1.empty());
    ssStatic = u8"1234";
    ssDynamic1 = u8"5678";
    EXPECT_EQ(std::move(ssStatic) + ssDynamic1, u8"12345678");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
    ssStatic = u8"1234";
    EXPECT_EQ(ssDynamic1 + std::move(ssStatic), u8"56781234");
    EXPECT_TRUE(ssStatic.empty());
    EXPECT_FALSE(ssDynamic1.empty());
}

TEST_F(CUtf8StringTypeTest, StreamString)
{
    std::stringstream sstream;
    sdv::u8string ss = "1234";
    sstream << ss;
    EXPECT_EQ(sstream.str(), "1234");
    ss.clear();
    EXPECT_TRUE(ss.empty());
    sstream >> ss;
    EXPECT_EQ(ss, "1234");
}

TEST_F(CUtf8StringTypeTest, StringGetLine)
{
    std::istringstream sstream;
    sstream.str("0;1;2;3");
    int nIndex = 0;
    for (sdv::u8string ss; sdv::getline(sstream, ss, ';');)
        EXPECT_EQ(ss, std::to_string(nIndex++));
    EXPECT_EQ(sstream.str(), "0;1;2;3");

    sstream.str("0\n1\n2\n3");
    nIndex = 0;
    for (sdv::u8string ss; sdv::getline(std::move(sstream), ss);)
        EXPECT_EQ(ss, std::to_string(nIndex++));
}

TEST_F(CUtf8StringTypeTest, MakeString)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf8String(ssAnsi), u8"Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssWide), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf8), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf16), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf32), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssAnsi.c_str()), u8"Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssWide.c_str()), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf8.c_str()), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf16.c_str()), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssUtf32.c_str()), u8"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    EXPECT_EQ(sdv::MakeUtf8String(ssCppAnsi), u8"Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssCppWide), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssCppUtf8), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssCppUtf16), u8"Hello\U00024B62Hello");
    EXPECT_EQ(sdv::MakeUtf8String(ssCppUtf32), u8"Hello\U00024B62Hello");
}

TEST_F(CUtf8StringTypeTest, MakeStringGeneric)
{
    sdv::string	   ssAnsi  = "Hello";
    sdv::wstring   ssWide  = L"Hello\U00024B62Hello";
    sdv::u8string  ssUtf8  = u8"Hello\U00024B62Hello";
    sdv::u16string ssUtf16 = u"Hello\U00024B62Hello";
    sdv::u32string ssUtf32 = U"Hello\U00024B62Hello";
    sdv::u8string ssDst;
    ssDst = sdv::MakeString<char, false, 0, char, true, 0>(ssAnsi);
    EXPECT_EQ(ssDst, u8"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char, true, 0>(ssWide);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char, true, 0>(ssUtf8);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char, true, 0>(ssUtf16);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char, true, 0>(ssUtf32);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, false, 0, char, true, 0>(ssAnsi.c_str());
    EXPECT_EQ(ssDst, u8"Hello");
    ssDst = sdv::MakeString<wchar_t, true, 0, char, true, 0>(ssWide.c_str());
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, true, 0, char, true, 0>(ssUtf8.c_str());
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, true, 0, char, true, 0>(ssUtf16.c_str());
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, true, 0, char, true, 0>(ssUtf32.c_str());
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");

    std::string	   ssCppAnsi  = "Hello";
    std::wstring   ssCppWide  = L"Hello\U00024B62Hello";
    std::string	   ssCppUtf8  = u8"Hello\U00024B62Hello";
    std::u16string ssCppUtf16 = u"Hello\U00024B62Hello";
    std::u32string ssCppUtf32 = U"Hello\U00024B62Hello";
    ssDst = sdv::MakeString<char, char, true, 0>(ssCppAnsi);
    EXPECT_EQ(ssDst, u8"Hello");
    ssDst = sdv::MakeString<wchar_t, char, true, 0>(ssCppWide);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char, char, true, 0>(ssCppUtf8);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char16_t, char, true, 0>(ssCppUtf16);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
    ssDst = sdv::MakeString<char32_t, char, true, 0>(ssCppUtf32);
    EXPECT_EQ(ssDst, u8"Hello\U00024B62Hello");
}

TEST_F(CUtf8StringTypeTest, IteratorDistance)
{
    sdv::string ss = u8"This is a text";
    EXPECT_EQ(std::distance(ss.begin(), ss.end()), ss.length());
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
