#include <sstream>

#include <support/sequence.h>

#include "basic_types_test.h"

using CComplexSequenceTypeTest = CBasicTypesTest;

struct SComplexNumber
{
    uint32_t uiIm;
    uint32_t uiRe;
};

inline bool operator==(SComplexNumber s1, SComplexNumber s2)
{
    return s1.uiIm == s2.uiIm && s1.uiRe == s2.uiRe;
}

inline bool operator!=(SComplexNumber s1, SComplexNumber s2)
{
    return !operator==(s1, s2);
}

inline bool operator<(SComplexNumber s1, SComplexNumber s2)
{
    if (s1.uiIm < s2.uiIm)
        return true;
    if (s1.uiIm != s2.uiIm)
        return false;
    return s1.uiRe < s2.uiRe;
}

inline bool operator<=(SComplexNumber s1, SComplexNumber s2)
{
    if (s1.uiIm < s2.uiIm)
        return true;
    if (s1.uiIm != s2.uiIm)
        return false;
    return s1.uiRe <= s2.uiRe;
}

inline bool operator>(SComplexNumber s1, SComplexNumber s2)
{
    if (s1.uiIm > s2.uiIm)
        return true;
    if (s1.uiIm != s2.uiIm)
        return false;
    return s1.uiRe > s2.uiRe;
}

inline bool operator>=(SComplexNumber s1, SComplexNumber s2)
{
    if (s1.uiIm > s2.uiIm)
        return true;
    if (s1.uiIm != s2.uiIm)
        return false;
    return s1.uiRe >= s2.uiRe;
}

TEST_F(CComplexSequenceTypeTest, ConstructorStatic)
{
    // Empty sequence
    sdv::sequence<SComplexNumber> seqEmpty;
    EXPECT_TRUE(seqEmpty.empty());

    // Fill constructor
    SComplexNumber sVal = {100u, 200u};
    sdv::sequence<SComplexNumber, 10> seqFill(5, sVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], sVal);
    EXPECT_EQ(seqFill[1], sVal);
    EXPECT_EQ(seqFill[2], sVal);
    EXPECT_EQ(seqFill[3], sVal);
    EXPECT_EQ(seqFill[4], sVal);

    // Reserve constructor
    sdv::sequence<SComplexNumber, 10> seqReserve(10);
    EXPECT_FALSE(seqReserve.empty());
    EXPECT_EQ(seqReserve.size(), 10);

    // Iterator constructor
    sdv::sequence<SComplexNumber, 15> seqIterator(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], sVal);
    EXPECT_EQ(seqIterator[1], sVal);
    EXPECT_EQ(seqIterator[2], sVal);
    EXPECT_EQ(seqIterator[3], sVal);
    EXPECT_EQ(seqIterator[4], sVal);

    // Copy constructor
    sdv::sequence<SComplexNumber, 12> seqCopy(seqFill);
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 5);
    EXPECT_EQ(seqCopy[0], sVal);
    EXPECT_EQ(seqCopy[1], sVal);
    EXPECT_EQ(seqCopy[2], sVal);
    EXPECT_EQ(seqCopy[3], sVal);
    EXPECT_EQ(seqCopy[4], sVal);

    // Move constructor
    sdv::sequence<SComplexNumber, 10> seqMove(std::move(seqFill));
    EXPECT_TRUE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 5);

    // C++ vector constructor
    std::vector<SComplexNumber> vec = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    sdv::sequence<SComplexNumber, 5> seqVector(vec);
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], sVal10);
    EXPECT_EQ(seqVector[1], sVal20);
    EXPECT_EQ(seqVector[2], sVal30);

    // Initializer list constructor
    sdv::sequence<SComplexNumber, 5> seqIList = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], sVal10);
    EXPECT_EQ(seqIList[1], sVal20);
    EXPECT_EQ(seqIList[2], sVal30);
}

TEST_F(CComplexSequenceTypeTest, ConstructorDynamic)
{
    // Empty sequence
    sdv::sequence<SComplexNumber> seqEmpty;
    EXPECT_TRUE(seqEmpty.empty());

    // Fill constructor
    SComplexNumber sVal = {100u, 200u};
    sdv::sequence<SComplexNumber> seqFill(5, sVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], sVal);
    EXPECT_EQ(seqFill[1], sVal);
    EXPECT_EQ(seqFill[2], sVal);
    EXPECT_EQ(seqFill[3], sVal);
    EXPECT_EQ(seqFill[4], sVal);

    // Reserve constructor
    sdv::sequence<SComplexNumber> seqReserve(10);
    EXPECT_FALSE(seqReserve.empty());
    EXPECT_EQ(seqReserve.size(), 10);

    // Iterator constructor
    sdv::sequence<SComplexNumber> seqIterator(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], sVal);
    EXPECT_EQ(seqIterator[1], sVal);
    EXPECT_EQ(seqIterator[2], sVal);
    EXPECT_EQ(seqIterator[3], sVal);
    EXPECT_EQ(seqIterator[4], sVal);

    // Copy constructor
    sdv::sequence<SComplexNumber> seqCopy(seqFill);
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 5);
    EXPECT_EQ(seqCopy[0], sVal);
    EXPECT_EQ(seqCopy[1], sVal);
    EXPECT_EQ(seqCopy[2], sVal);
    EXPECT_EQ(seqCopy[3], sVal);
    EXPECT_EQ(seqCopy[4], sVal);

    // Move constructor
    sdv::sequence<SComplexNumber> seqMove(std::move(seqFill));
    EXPECT_TRUE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 5);

    // C++ vector constructor
    std::vector<SComplexNumber>	vec = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    sdv::sequence<SComplexNumber> seqVector(vec);
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], sVal10);
    EXPECT_EQ(seqVector[1], sVal20);
    EXPECT_EQ(seqVector[2], sVal30);

    // Initializer list constructor
    sdv::sequence<SComplexNumber> seqIList = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], sVal10);
    EXPECT_EQ(seqIList[1], sVal20);
    EXPECT_EQ(seqIList[2], sVal30);
}

TEST_F(CComplexSequenceTypeTest, AssignmentOperatorStatic)
{
    sdv::sequence<SComplexNumber, 5> seq = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 3);
    EXPECT_EQ(seq[0], sVal10);
    EXPECT_EQ(seq[1], sVal20);
    EXPECT_EQ(seq[2], sVal30);

    // Copy assignment
    sdv::sequence<SComplexNumber, 7> seqCopy;
    EXPECT_TRUE(seqCopy.empty());
    seqCopy = seq;
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 3);
    EXPECT_EQ(seqCopy[0], sVal10);
    EXPECT_EQ(seqCopy[1], sVal20);
    EXPECT_EQ(seqCopy[2], sVal30);

    // Move assignment
    sdv::sequence<SComplexNumber, 3> seqMove;
    EXPECT_TRUE(seqMove.empty());
    seqMove = std::move(seq);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 3);

    // C++ vector assignment
    std::vector<SComplexNumber> vec = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber, 5> seqVector;
    EXPECT_TRUE(seqVector.empty());
    seqVector = vec;
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], sVal10);
    EXPECT_EQ(seqVector[1], sVal20);
    EXPECT_EQ(seqVector[2], sVal30);

    // Initializer list assignment
    sdv::sequence<SComplexNumber, 5> seqIList;
    EXPECT_TRUE(seqIList.empty());
    seqIList = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], sVal10);
    EXPECT_EQ(seqIList[1], sVal20);
    EXPECT_EQ(seqIList[2], sVal30);
}

TEST_F(CComplexSequenceTypeTest, AssignmentOperatorDynamic)
{
    sdv::sequence<SComplexNumber> seq = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 3);
    EXPECT_EQ(seq[0], sVal10);
    EXPECT_EQ(seq[1], sVal20);
    EXPECT_EQ(seq[2], sVal30);

    // Copy assignment
    sdv::sequence<SComplexNumber> seqCopy;
    EXPECT_TRUE(seqCopy.empty());
    seqCopy = seq;
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 3);
    EXPECT_EQ(seqCopy[0], sVal10);
    EXPECT_EQ(seqCopy[1], sVal20);
    EXPECT_EQ(seqCopy[2], sVal30);

    // Move assignment
    sdv::sequence<SComplexNumber> seqMove;
    EXPECT_TRUE(seqMove.empty());
    seqMove = std::move(seq);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 3);

    // C++ vector assignment
    std::vector<SComplexNumber> vec = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber> seqVector;
    EXPECT_TRUE(seqVector.empty());
    seqVector = vec;
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], sVal10);
    EXPECT_EQ(seqVector[1], sVal20);
    EXPECT_EQ(seqVector[2], sVal30);

    // Initializer list assignment
    sdv::sequence<SComplexNumber> seqIList;
    EXPECT_TRUE(seqIList.empty());
    seqIList = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], sVal10);
    EXPECT_EQ(seqIList[1], sVal20);
    EXPECT_EQ(seqIList[2], sVal30);
}

TEST_F(CComplexSequenceTypeTest, AssignmentFunction)
{
    // Fill assignment
    sdv::sequence<SComplexNumber> seqFill;
    SComplexNumber sVal = {100u, 200u};
    seqFill.assign(5, sVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], sVal);
    EXPECT_EQ(seqFill[1], sVal);
    EXPECT_EQ(seqFill[2], sVal);
    EXPECT_EQ(seqFill[3], sVal);
    EXPECT_EQ(seqFill[4], sVal);

    // Iterator assignment
    sdv::sequence<SComplexNumber> seqIterator;
    seqIterator.assign(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], sVal);
    EXPECT_EQ(seqIterator[1], sVal);
    EXPECT_EQ(seqIterator[2], sVal);
    EXPECT_EQ(seqIterator[3], sVal);
    EXPECT_EQ(seqIterator[4], sVal);

    // Initializer list assignment
    sdv::sequence<SComplexNumber> seqIList;
    seqIList.assign({SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], sVal10);
    EXPECT_EQ(seqIList[1], sVal20);
    EXPECT_EQ(seqIList[2], sVal30);
}

TEST_F(CComplexSequenceTypeTest, PositionFunction)
{
    // Position in the sequence
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u};
    sdv::sequence<SComplexNumber> seq1{sVal10, sVal20, sVal30, sVal40, sVal50};
    EXPECT_EQ(seq1.at(0), sVal10);
    EXPECT_EQ(seq1[0], sVal10);
    EXPECT_EQ(seq1.front(), sVal10);
    EXPECT_EQ(seq1.at(4), sVal50);
    EXPECT_EQ(seq1[4], sVal50);
    EXPECT_EQ(seq1.back(), sVal50);
    EXPECT_THROW(seq1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq1[5], sdv::XIndexOutOfRange);

    // Empty sequence
    sdv::sequence<SComplexNumber> seq2;
    EXPECT_THROW(seq2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2.back(), sdv::XIndexOutOfRange);

    // Assignment
    SComplexNumber sVal110{110u, 210u};
    seq1.at(0) = sVal110;
    EXPECT_EQ(seq1[0], sVal110);
    seq1[0] = sVal10;
    EXPECT_EQ(seq1[0], sVal10);
    seq1.front() = sVal110;
    EXPECT_EQ(seq1[0], sVal110);
    SComplexNumber sVal150{150u, 250u};
    seq1.back() = sVal150;
    EXPECT_EQ(seq1[4], sVal150);
}

TEST_F(CComplexSequenceTypeTest, CppVectorAccess)
{
    // C++ cast operator
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u};
    sdv::sequence<SComplexNumber> seq = {sVal10, sVal20, sVal30};
    EXPECT_EQ(seq.size(), 3);
    std::vector<SComplexNumber> vec;
    EXPECT_TRUE(vec.empty());
    vec = seq;
    EXPECT_EQ(vec.size(), 3);

    // Data access
    EXPECT_EQ(memcmp(seq.data(), vec.data(), 3 * sizeof(uint32_t)), 0);
}

TEST_F(CComplexSequenceTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::sequence<SComplexNumber>::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u};
    sdv::sequence seq{sVal10, sVal20, sVal30, sVal40, sVal50};
    EXPECT_NE(itEmpty, seq.begin());
    sdv::sequence<SComplexNumber>::iterator itPos = seq.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, seq.begin());
    sdv::sequence<SComplexNumber>::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::sequence<SComplexNumber>::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, sVal10);
    EXPECT_EQ(itPos[0], sVal10);
    EXPECT_EQ(itPos[4], sVal50);
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    SComplexNumber sVal110{110u, 210u}, sVal150{150u, 250u};
    *itPos = sVal110;
    EXPECT_EQ(seq[0], sVal110);
    itPos[4] = sVal150;
    EXPECT_EQ(seq[4], sVal150);
    seq[0] = sVal10;
    seq[4] = sVal50;

    // Iterator iteration
    itPos++;
    EXPECT_EQ(*itPos, sVal20);
    itPos += 3;
    EXPECT_EQ(*itPos, sVal50);
    EXPECT_NO_THROW(itPos++); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, seq.end());
    EXPECT_NO_THROW(itPos++); // Will be ignored; doesn't increase even more
    itPos--;
    EXPECT_EQ(*itPos, sVal50);
    itPos -= 4;
    EXPECT_EQ(*itPos, sVal10);
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, sVal10);
    EXPECT_EQ(itPos, seq.begin());

    // Const iterator
    sdv::sequence<SComplexNumber>::const_iterator itPosConst = seq.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    itPosConst++;
    EXPECT_EQ(*itPosConst, sVal20);
    EXPECT_NE(itPosConst, itPos);
    itPos++;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(seq.cbegin(), seq.begin());
    EXPECT_EQ(seq.begin(), seq.cbegin());
    EXPECT_EQ(seq.cend(), seq.end());
    EXPECT_EQ(seq.end(), seq.cend());
}

TEST_F(CComplexSequenceTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::sequence<SComplexNumber>::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u};
    sdv::sequence seq{sVal10, sVal20, sVal30, sVal40, sVal50};
    EXPECT_NE(itEmpty, seq.rbegin());
    sdv::sequence<SComplexNumber>::reverse_iterator itPos = seq.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, seq.rbegin());
    sdv::sequence<SComplexNumber>::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::sequence<SComplexNumber>::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    SComplexNumber sVal110{110u, 210u}, sVal150{150u, 250u};
    EXPECT_EQ(*itPos, sVal50);
    EXPECT_EQ(itPos[0], sVal50);
    EXPECT_EQ(itPos[4], sVal10);
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = sVal150;
    EXPECT_EQ(seq[4], sVal150);
    itPos[4] = sVal110;
    EXPECT_EQ(seq[0], sVal110);
    seq[0] = sVal10;
    seq[4] = sVal50;

    // Iterator iteration
    itPos++;
    EXPECT_EQ(*itPos, sVal40);
    itPos += 3;
    EXPECT_EQ(*itPos, sVal10);
    EXPECT_NO_THROW(itPos++); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, seq.rend());
    EXPECT_NO_THROW(itPos++); // Will be ignored; doesn't increase even more
    itPos--;
    EXPECT_EQ(*itPos, sVal10);
    itPos -= 4;
    EXPECT_EQ(*itPos, sVal50);
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, sVal50);
    EXPECT_EQ(itPos, seq.rbegin());

    // Const iterator
    sdv::sequence<SComplexNumber>::const_reverse_iterator itPosConst = seq.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    itPosConst++;
    EXPECT_EQ(*itPosConst, sVal40);
    EXPECT_NE(itPosConst, itPos);
    itPos++;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(seq.crbegin(), seq.rbegin());
    EXPECT_EQ(seq.rbegin(), seq.crbegin());
    EXPECT_EQ(seq.crend(), seq.rend());
    EXPECT_EQ(seq.rend(), seq.crend());
}

TEST_F(CComplexSequenceTypeTest, SequenceCapacityStatic)
{
    // Empty string
    sdv::sequence<SComplexNumber, 20> seq;
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 20);

    // Filled string
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u};
    seq = {sVal10, sVal20, sVal30, sVal40, sVal50};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 5);
    EXPECT_EQ(seq.length(), 5);
    EXPECT_EQ(seq.capacity(), 20);

    // Resize, reserve
    seq.resize(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);
    seq.resize(4);
    EXPECT_EQ(seq.size(), 4);
    EXPECT_EQ(seq.length(), 4);
    EXPECT_EQ(seq.capacity(), 20);
    seq.push_back(sVal50);
    seq.resize(10, sVal60);
    EXPECT_EQ(seq[4], sVal50);
    EXPECT_EQ(seq[5], sVal60);
    EXPECT_EQ(seq[6], sVal60);
    EXPECT_EQ(seq[7], sVal60);
    EXPECT_EQ(seq[8], sVal60);
    EXPECT_EQ(seq[9], sVal60);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);
    seq.resize(5);
    EXPECT_EQ(seq.size(), 5);
    EXPECT_EQ(seq.length(), 5);
    EXPECT_EQ(seq.capacity(), 20);
    seq.reserve(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);
    seq.reserve(4);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);
    seq.resize(5);

    // Shrink to fit
    seq.reserve(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);
    seq.shrink_to_fit();
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 20);

    // Clear
    seq.clear();
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 20);
}

TEST_F(CComplexSequenceTypeTest, SequenceCapacityDynamic)
{
    // Empty string
    sdv::sequence<SComplexNumber> seq;
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 0);

    // Filled string
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u};
    seq = {sVal10, sVal20, sVal30, sVal40, sVal50};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 5);
    EXPECT_EQ(seq.length(), 5);
    EXPECT_EQ(seq.capacity(), 5);

    // Resize, reserve
    seq.resize(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);
    seq.resize(4);
    EXPECT_EQ(seq.size(), 4);
    EXPECT_EQ(seq.length(), 4);
    EXPECT_EQ(seq.capacity(), 4);
    seq.push_back(sVal50);
    seq.resize(10, sVal60);
    EXPECT_EQ(seq[4], sVal50);
    EXPECT_EQ(seq[5], sVal60);
    EXPECT_EQ(seq[6], sVal60);
    EXPECT_EQ(seq[7], sVal60);
    EXPECT_EQ(seq[8], sVal60);
    EXPECT_EQ(seq[9], sVal60);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);
    seq.resize(5);
    EXPECT_EQ(seq.size(), 5);
    EXPECT_EQ(seq.length(), 5);
    EXPECT_EQ(seq.capacity(), 5);
    seq.reserve(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);
    seq.reserve(4);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);
    seq.resize(5);

    // Shrink to fit
    seq.reserve(10);
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);
    seq.shrink_to_fit();
    EXPECT_EQ(seq.size(), 10);
    EXPECT_EQ(seq.length(), 10);
    EXPECT_EQ(seq.capacity(), 10);

    // Clear
    seq.clear();
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 0);
}

TEST_F(CComplexSequenceTypeTest, InsertFunction)
{
    // Value assignment
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u};
    sdv::sequence<SComplexNumber> seq1;
    EXPECT_TRUE(seq1.empty());
    seq1.insert(seq1.begin(), 3, sVal10);
    EXPECT_EQ(seq1[0], sVal10);
    EXPECT_EQ(seq1[1], sVal10);
    EXPECT_EQ(seq1[2], sVal10);
    seq1.insert(seq1.begin(), 2, sVal20);
    EXPECT_EQ(seq1[0], sVal20);
    EXPECT_EQ(seq1[1], sVal20);
    EXPECT_EQ(seq1[2], sVal10);
    seq1.insert(seq1.end(), 2, sVal30);
    EXPECT_EQ(seq1[4], sVal10);
    EXPECT_EQ(seq1[5], sVal30);
    EXPECT_EQ(seq1[6], sVal30);
    seq1.insert(seq1.begin() + 4, 2, sVal40);
    EXPECT_EQ(seq1[0], sVal20);
    EXPECT_EQ(seq1[1], sVal20);
    EXPECT_EQ(seq1[2], sVal10);
    EXPECT_EQ(seq1[3], sVal10);
    EXPECT_EQ(seq1[4], sVal40);
    EXPECT_EQ(seq1[5], sVal40);
    EXPECT_EQ(seq1[6], sVal10);
    EXPECT_EQ(seq1[7], sVal30);
    EXPECT_EQ(seq1[8], sVal30);

    // Iterator assignment
    sdv::sequence<SComplexNumber> seq2;
    sdv::sequence<SComplexNumber> seq({SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    EXPECT_TRUE(seq2.empty());
    seq2.insert(seq2.begin(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);
    seq2.insert(seq2.begin(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);
    EXPECT_EQ(seq2[3], sVal10);
    seq2.insert(seq2.end(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[5], sVal30);
    EXPECT_EQ(seq2[6], sVal10);
    EXPECT_EQ(seq2[7], sVal20);
    EXPECT_EQ(seq2[8], sVal30);
    seq2.insert(seq2.begin() + 4, seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);
    EXPECT_EQ(seq2[3], sVal10);
    EXPECT_EQ(seq2[4], sVal10);
    EXPECT_EQ(seq2[5], sVal20);
    EXPECT_EQ(seq2[6], sVal30);
    EXPECT_EQ(seq2[7], sVal20);
    EXPECT_EQ(seq2[8], sVal30);
    EXPECT_EQ(seq2[9], sVal10);
    EXPECT_EQ(seq2[10], sVal20);
    EXPECT_EQ(seq2[11], sVal30);

    // Initializer list assignment
    sdv::sequence<SComplexNumber> seq3;
    EXPECT_TRUE(seq3.empty());
    seq3.insert(seq3.begin(), {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    EXPECT_EQ(seq3[0], sVal10);
    EXPECT_EQ(seq3[1], sVal20);
    EXPECT_EQ(seq3[2], sVal30);
    seq3.insert(seq3.begin(), {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    EXPECT_EQ(seq3[0], sVal10);
    EXPECT_EQ(seq3[1], sVal20);
    EXPECT_EQ(seq3[2], sVal30);
    EXPECT_EQ(seq3[3], sVal10);
    seq3.insert(seq3.end(), {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    EXPECT_EQ(seq3[5], sVal30);
    EXPECT_EQ(seq3[6], sVal10);
    EXPECT_EQ(seq3[7], sVal20);
    EXPECT_EQ(seq3[8], sVal30);
    seq3.insert(seq3.begin() + 4, {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}});
    EXPECT_EQ(seq3[0], sVal10);
    EXPECT_EQ(seq3[1], sVal20);
    EXPECT_EQ(seq3[2], sVal30);
    EXPECT_EQ(seq3[3], sVal10);
    EXPECT_EQ(seq3[4], sVal10);
    EXPECT_EQ(seq3[5], sVal20);
    EXPECT_EQ(seq3[6], sVal30);
    EXPECT_EQ(seq3[7], sVal20);
    EXPECT_EQ(seq3[8], sVal30);
    EXPECT_EQ(seq3[9], sVal10);
    EXPECT_EQ(seq3[10], sVal20);
    EXPECT_EQ(seq3[11], sVal30);
}

TEST_F(CComplexSequenceTypeTest, EraseFunction)
{
    // Single element erasure
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u},
        sVal70{70u, 170u}, sVal80{80u, 180u}, sVal90{90u, 190u}, sVal100{100u, 200u};
    sdv::sequence<SComplexNumber> seq({sVal10, sVal20, sVal30, sVal40, sVal50, sVal60, sVal70, sVal80, sVal90, sVal100});
    sdv::sequence<SComplexNumber>::iterator it = seq.begin();
    seq.erase(it);
    EXPECT_EQ(seq[0], sVal20);
    it = seq.end() - 1;
    seq.erase(it);
    EXPECT_EQ(seq[seq.size() - 1], sVal90);
    it = seq.begin() + 4;
    seq.erase(it);
    EXPECT_EQ(seq[3], sVal50);
    EXPECT_EQ(seq[4], sVal70);
    seq = {sVal10, sVal20, sVal30, sVal40, sVal50, sVal60, sVal70, sVal80, sVal90, sVal100};
    sdv::sequence<SComplexNumber>::const_iterator cit = seq.cbegin();
    seq.erase(cit);
    EXPECT_EQ(seq[0], sVal20);
    cit = seq.cend() - 1;
    seq.erase(cit);
    EXPECT_EQ(seq[seq.size() - 1], sVal90);
    cit = seq.begin() + 4;
    seq.erase(cit);
    EXPECT_EQ(seq[3], sVal50);
    EXPECT_EQ(seq[4], sVal70);

    // Element range erasure
    seq = {sVal10, sVal20, sVal30, sVal40, sVal50, sVal60, sVal70, sVal80, sVal90, sVal100};
    sdv::sequence<SComplexNumber>::iterator it1 = seq.begin();
    sdv::sequence<SComplexNumber>::iterator it2 = it1 + 1;
    seq.erase(it1, it2);
    EXPECT_EQ(seq[0], sVal20);
    it1 = seq.end() - 1;
    it2 = seq.end();
    seq.erase(it1, it2);
    EXPECT_EQ(seq[seq.size() - 1], sVal90);
    it1 = seq.begin() + 1;
    it2 = it1 + 3;
    seq.erase(it1, it2);
    EXPECT_EQ(seq[0], sVal20);
    EXPECT_EQ(seq[1], sVal60);
    seq = {sVal10, sVal20, sVal30, sVal40, sVal50, sVal60, sVal70, sVal80, sVal90, sVal100};
    sdv::sequence<SComplexNumber>::const_iterator cit1 = seq.cbegin();
    sdv::sequence<SComplexNumber>::const_iterator cit2 = cit1 + 1;
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[0], sVal20);
    cit1 = seq.end() - 1;
    cit2 = seq.end();
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[seq.size() - 1], sVal90);
    cit1 = seq.begin() + 1;
    cit2 = cit1 + 3;
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[0], sVal20);
    EXPECT_EQ(seq[1], sVal60);
}

TEST_F(CComplexSequenceTypeTest, PushPopFunctions)
{
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u};
    sdv::sequence<SComplexNumber> seq;
    seq.push_back(sVal10);
    EXPECT_EQ(seq[0], sVal10);
    EXPECT_EQ(seq.size(), 1);
    seq.push_back(std::move(sVal20));
    EXPECT_EQ(seq[1], sVal20);
    EXPECT_EQ(seq.size(), 2);
    seq.pop_back();
    EXPECT_EQ(seq.size(), 1);
    seq.pop_back();
    EXPECT_TRUE(seq.empty());
    EXPECT_NO_THROW(seq.pop_back());
}

TEST_F(CComplexSequenceTypeTest, SwapFunctionStatic)
{
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u},
        sVal70{70u, 170u};
    sdv::sequence<SComplexNumber, 5> seq1 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber, 10> seq2 = {SComplexNumber{40u, 140u}, SComplexNumber{50u, 150u}, SComplexNumber{60u, 160u}, SComplexNumber{70u, 170u}};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], sVal40);
    EXPECT_EQ(seq1[1], sVal50);
    EXPECT_EQ(seq1[2], sVal60);
    EXPECT_EQ(seq1[3], sVal70);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], sVal10);
    EXPECT_EQ(seq1[1], sVal20);
    EXPECT_EQ(seq1[2], sVal30);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], sVal40);
    EXPECT_EQ(seq2[1], sVal50);
    EXPECT_EQ(seq2[2], sVal60);
    EXPECT_EQ(seq2[3], sVal70);
}

TEST_F(CComplexSequenceTypeTest, SwapFunctionDynamic)
{
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u},
        sVal70{70u, 170u};
    sdv::sequence<SComplexNumber> seq1 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber> seq2 = {SComplexNumber{40u, 140u}, SComplexNumber{50u, 150u}, SComplexNumber{60u, 160u}, SComplexNumber{70u, 170u}};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], sVal40);
    EXPECT_EQ(seq1[1], sVal50);
    EXPECT_EQ(seq1[2], sVal60);
    EXPECT_EQ(seq1[3], sVal70);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], sVal10);
    EXPECT_EQ(seq1[1], sVal20);
    EXPECT_EQ(seq1[2], sVal30);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], sVal40);
    EXPECT_EQ(seq2[1], sVal50);
    EXPECT_EQ(seq2[2], sVal60);
    EXPECT_EQ(seq2[3], sVal70);
}

TEST_F(CComplexSequenceTypeTest, SwapFunctionMixed)
{
    SComplexNumber sVal10{10u, 110u}, sVal20{20u, 120u}, sVal30{30u, 130u}, sVal40{40u, 140u}, sVal50{50u, 150u}, sVal60{60u, 160u},
        sVal70{70u, 170u};
    sdv::sequence<SComplexNumber, 5> seq1 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber> seq2 = {SComplexNumber{40u, 140u}, SComplexNumber{50u, 150u}, SComplexNumber{60u, 160u}, SComplexNumber{70u, 170u}};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], sVal40);
    EXPECT_EQ(seq1[1], sVal50);
    EXPECT_EQ(seq1[2], sVal60);
    EXPECT_EQ(seq1[3], sVal70);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], sVal10);
    EXPECT_EQ(seq2[1], sVal20);
    EXPECT_EQ(seq2[2], sVal30);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], sVal10);
    EXPECT_EQ(seq1[1], sVal20);
    EXPECT_EQ(seq1[2], sVal30);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], sVal40);
    EXPECT_EQ(seq2[1], sVal50);
    EXPECT_EQ(seq2[2], sVal60);
    EXPECT_EQ(seq2[3], sVal70);
}

TEST_F(CComplexSequenceTypeTest, CompareOperator)
{
    sdv::sequence<SComplexNumber> seq1 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    sdv::sequence<SComplexNumber> seq2;
    EXPECT_FALSE(seq1 == seq2);
    EXPECT_TRUE(seq1 != seq2);
    EXPECT_FALSE(seq1 < seq2);
    EXPECT_FALSE(seq1 <= seq2);
    EXPECT_TRUE(seq1 > seq2);
    EXPECT_TRUE(seq1 >= seq2);
    seq2 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_TRUE(seq1 == seq2);
    EXPECT_FALSE(seq1 != seq2);
    EXPECT_FALSE(seq1 < seq2);
    EXPECT_TRUE(seq1 <= seq2);
    EXPECT_FALSE(seq1 > seq2);
    EXPECT_TRUE(seq1 >= seq2);
    SComplexNumber sVal40{40u, 140u};
    seq2.push_back(sVal40);
    EXPECT_FALSE(seq1 == seq2);
    EXPECT_TRUE(seq1 != seq2);
    EXPECT_TRUE(seq1 < seq2);
    EXPECT_TRUE(seq1 <= seq2);
    EXPECT_FALSE(seq1 > seq2);
    EXPECT_FALSE(seq1 >= seq2);
    seq1.clear();
    seq2.clear();
    EXPECT_TRUE(seq1 == seq2);
    EXPECT_FALSE(seq1 != seq2);
    EXPECT_FALSE(seq1 < seq2);
    EXPECT_TRUE(seq1 <= seq2);
    EXPECT_FALSE(seq1 > seq2);
    EXPECT_TRUE(seq1 >= seq2);
}

TEST_F(CComplexSequenceTypeTest, IteratorDistance)
{
    sdv::sequence<SComplexNumber> seq1 = {SComplexNumber{10u, 110u}, SComplexNumber{20u, 120u}, SComplexNumber{30u, 130u}};
    EXPECT_EQ(std::distance(seq1.begin(), seq1.end()), seq1.length());
}