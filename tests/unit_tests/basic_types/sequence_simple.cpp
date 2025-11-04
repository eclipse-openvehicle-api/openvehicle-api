#include <sstream>

#include <support/sequence.h>

#include "basic_types_test.h"

using CSimpleSequenceTypeTest = CBasicTypesTest;

TEST_F(CSimpleSequenceTypeTest, ConstructorStatic)
{
    // Empty sequence
    sdv::sequence<uint32_t, 10> seqEmpty;
    EXPECT_TRUE(seqEmpty.empty());

    // Fill constructor
    uint32_t uiVal = 100u;
    sdv::sequence<uint32_t, 10> seqFill(5, uiVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], 100u);
    EXPECT_EQ(seqFill[1], 100u);
    EXPECT_EQ(seqFill[2], 100u);
    EXPECT_EQ(seqFill[3], 100u);
    EXPECT_EQ(seqFill[4], 100u);

    // Reserve constructor
    sdv::sequence<uint32_t, 10> seqReserve(10);
    EXPECT_FALSE(seqReserve.empty());
    EXPECT_EQ(seqReserve.size(), 10);

    // Iterator constructor
    sdv::sequence<uint32_t, 15> seqIterator(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], 100u);
    EXPECT_EQ(seqIterator[1], 100u);
    EXPECT_EQ(seqIterator[2], 100u);
    EXPECT_EQ(seqIterator[3], 100u);
    EXPECT_EQ(seqIterator[4], 100u);

    // Copy constructor
    sdv::sequence<uint32_t, 12> seqCopy(seqFill);
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 5);
    EXPECT_EQ(seqCopy[0], 100u);
    EXPECT_EQ(seqCopy[1], 100u);
    EXPECT_EQ(seqCopy[2], 100u);
    EXPECT_EQ(seqCopy[3], 100u);
    EXPECT_EQ(seqCopy[4], 100u);

    // Move constructor
    sdv::sequence<uint32_t, 10> seqMove(std::move(seqFill));
    EXPECT_TRUE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 5);

    // C++ vector constructor
    std::vector<uint32_t> vec = {10u, 20u, 30u};
    sdv::sequence<uint32_t, 5> seqVector(vec);
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], 10u);
    EXPECT_EQ(seqVector[1], 20u);
    EXPECT_EQ(seqVector[2], 30u);

    // Initializer list constructor
    sdv::sequence<uint32_t, 5> seqIList = {10u, 20u, 30u};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], 10u);
    EXPECT_EQ(seqIList[1], 20u);
    EXPECT_EQ(seqIList[2], 30u);
}

TEST_F(CSimpleSequenceTypeTest, ConstructorDynamic)
{
    // Empty sequence
    sdv::sequence<uint32_t> seqEmpty;
    EXPECT_TRUE(seqEmpty.empty());

    // Fill constructor
    uint32_t				uiVal = 100u;
    sdv::sequence<uint32_t> seqFill(5, uiVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], 100u);
    EXPECT_EQ(seqFill[1], 100u);
    EXPECT_EQ(seqFill[2], 100u);
    EXPECT_EQ(seqFill[3], 100u);
    EXPECT_EQ(seqFill[4], 100u);

    // Reserve constructor
    sdv::sequence<uint32_t> seqReserve(10);
    EXPECT_FALSE(seqReserve.empty());
    EXPECT_EQ(seqReserve.size(), 10);

    // Iterator constructor
    sdv::sequence<uint32_t> seqIterator(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], 100u);
    EXPECT_EQ(seqIterator[1], 100u);
    EXPECT_EQ(seqIterator[2], 100u);
    EXPECT_EQ(seqIterator[3], 100u);
    EXPECT_EQ(seqIterator[4], 100u);

    // Copy constructor
    sdv::sequence<uint32_t> seqCopy(seqFill);
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 5);
    EXPECT_EQ(seqCopy[0], 100u);
    EXPECT_EQ(seqCopy[1], 100u);
    EXPECT_EQ(seqCopy[2], 100u);
    EXPECT_EQ(seqCopy[3], 100u);
    EXPECT_EQ(seqCopy[4], 100u);

    // Move constructor
    sdv::sequence<uint32_t> seqMove(std::move(seqFill));
    EXPECT_TRUE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 5);

    // C++ vector constructor
    std::vector<uint32_t>	vec = {10u, 20u, 30u};
    sdv::sequence<uint32_t> seqVector(vec);
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], 10u);
    EXPECT_EQ(seqVector[1], 20u);
    EXPECT_EQ(seqVector[2], 30u);

    // Initializer list constructor
    sdv::sequence<uint32_t> seqIList = {10u, 20u, 30u};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], 10u);
    EXPECT_EQ(seqIList[1], 20u);
    EXPECT_EQ(seqIList[2], 30u);
}

TEST_F(CSimpleSequenceTypeTest, AssignmentOperatorStatic)
{
    sdv::sequence<uint32_t, 5> seq = {10u, 20u, 30u};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 3);
    EXPECT_EQ(seq[0], 10u);
    EXPECT_EQ(seq[1], 20u);
    EXPECT_EQ(seq[2], 30u);

    // Copy assignment
    sdv::sequence<uint32_t, 7> seqCopy;
    EXPECT_TRUE(seqCopy.empty());
    seqCopy = seq;
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 3);
    EXPECT_EQ(seqCopy[0], 10u);
    EXPECT_EQ(seqCopy[1], 20u);
    EXPECT_EQ(seqCopy[2], 30u);

    // Move assignment
    sdv::sequence<uint32_t, 3> seqMove;
    EXPECT_TRUE(seqMove.empty());
    seqMove = std::move(seq);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 3);

    // C++ vector assignment
    std::vector<uint32_t> vec = {10u, 20u, 30u};
    sdv::sequence<uint32_t, 5> seqVector;
    EXPECT_TRUE(seqVector.empty());
    seqVector = vec;
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], 10u);
    EXPECT_EQ(seqVector[1], 20u);
    EXPECT_EQ(seqVector[2], 30u);

    // Initializer list assignment
    sdv::sequence<uint32_t, 5> seqIList;
    EXPECT_TRUE(seqIList.empty());
    seqIList = {10u, 20u, 30u};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], 10u);
    EXPECT_EQ(seqIList[1], 20u);
    EXPECT_EQ(seqIList[2], 30u);
}

TEST_F(CSimpleSequenceTypeTest, AssignmentOperatorDynamic)
{
    sdv::sequence<uint32_t> seq = {10u, 20u, 30u};
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(seq.size(), 3);
    EXPECT_EQ(seq[0], 10u);
    EXPECT_EQ(seq[1], 20u);
    EXPECT_EQ(seq[2], 30u);

    // Copy assignment
    sdv::sequence<uint32_t> seqCopy;
    EXPECT_TRUE(seqCopy.empty());
    seqCopy = seq;
    EXPECT_FALSE(seqCopy.empty());
    EXPECT_EQ(seqCopy.size(), 3);
    EXPECT_EQ(seqCopy[0], 10u);
    EXPECT_EQ(seqCopy[1], 20u);
    EXPECT_EQ(seqCopy[2], 30u);

    // Move assignment
    sdv::sequence<uint32_t> seqMove;
    EXPECT_TRUE(seqMove.empty());
    seqMove = std::move(seq);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seqMove.empty());
    EXPECT_EQ(seqMove.size(), 3);

    // C++ vector assignment
    std::vector<uint32_t>	vec = {10u, 20u, 30u};
    sdv::sequence<uint32_t> seqVector;
    EXPECT_TRUE(seqVector.empty());
    seqVector = vec;
    EXPECT_FALSE(seqVector.empty());
    EXPECT_EQ(seqVector.size(), 3);
    EXPECT_EQ(seqVector[0], 10u);
    EXPECT_EQ(seqVector[1], 20u);
    EXPECT_EQ(seqVector[2], 30u);

    // Initializer list assignment
    sdv::sequence<uint32_t> seqIList;
    EXPECT_TRUE(seqIList.empty());
    seqIList = {10u, 20u, 30u};
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], 10u);
    EXPECT_EQ(seqIList[1], 20u);
    EXPECT_EQ(seqIList[2], 30u);
}

TEST_F(CSimpleSequenceTypeTest, AssignmentFunction)
{
    // Fill assignment
    sdv::sequence<uint32_t> seqFill;
    uint32_t uiVal = 100;
    seqFill.assign(5, uiVal);
    EXPECT_FALSE(seqFill.empty());
    EXPECT_EQ(seqFill.size(), 5);
    EXPECT_EQ(seqFill[0], 100u);
    EXPECT_EQ(seqFill[1], 100u);
    EXPECT_EQ(seqFill[2], 100u);
    EXPECT_EQ(seqFill[3], 100u);
    EXPECT_EQ(seqFill[4], 100u);

    // Iterator assignment
    sdv::sequence<uint32_t> seqIterator;
    seqIterator.assign(seqFill.begin(), seqFill.end());
    EXPECT_FALSE(seqIterator.empty());
    EXPECT_EQ(seqIterator.size(), 5);
    EXPECT_EQ(seqIterator[0], 100u);
    EXPECT_EQ(seqIterator[1], 100u);
    EXPECT_EQ(seqIterator[2], 100u);
    EXPECT_EQ(seqIterator[3], 100u);
    EXPECT_EQ(seqIterator[4], 100u);

    // Initializer list assignment
    sdv::sequence<uint32_t> seqIList;
    seqIList.assign({10u, 20u, 30u});
    EXPECT_FALSE(seqIList.empty());
    EXPECT_EQ(seqIList.size(), 3);
    EXPECT_EQ(seqIList[0], 10u);
    EXPECT_EQ(seqIList[1], 20u);
    EXPECT_EQ(seqIList[2], 30u);
}

TEST_F(CSimpleSequenceTypeTest, PositionFunction)
{
    // Position in the sequence
    sdv::sequence<uint32_t> seq1({10u, 20u, 30u, 40u, 50u});
    EXPECT_EQ(seq1.at(0), 10u);
    EXPECT_EQ(seq1[0], 10u);
    EXPECT_EQ(seq1.front(), 10u);
    EXPECT_EQ(seq1.at(4), 50u);
    EXPECT_EQ(seq1[4], 50u);
    EXPECT_EQ(seq1.back(), 50u);
    EXPECT_THROW(seq1.at(5), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq1[5], sdv::XIndexOutOfRange);

    // Empty sequence
    sdv::sequence<uint32_t> seq2;
    EXPECT_THROW(seq2.at(0), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2[0], sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2.front(), sdv::XIndexOutOfRange);
    EXPECT_THROW(seq2.back(), sdv::XIndexOutOfRange);

    // Assignment
    seq1.at(0) = 110u;
    EXPECT_EQ(seq1[0], 110u);
    seq1[0] = 10u;
    EXPECT_EQ(seq1[0], 10u);
    seq1.front() = 110u;
    EXPECT_EQ(seq1[0], 110u);
    seq1.back() = 150u;
    EXPECT_EQ(seq1[4], 150u);
}

TEST_F(CSimpleSequenceTypeTest, CppVectorAccess)
{
    // C++ cast operator
    sdv::sequence<uint32_t>	seq = {10u, 20u, 30u};
    EXPECT_EQ(seq.size(), 3);
    std::vector<uint32_t> vec;
    EXPECT_TRUE(vec.empty());
    vec = seq;
    EXPECT_EQ(vec.size(), 3);

    // Data access
    EXPECT_EQ(memcmp(seq.data(), vec.data(), 3 * sizeof(uint32_t)), 0);
}

TEST_F(CSimpleSequenceTypeTest, ForwardIteratorBasedAccess)
{
    // Empty forward iterator
    sdv::sequence<uint32_t>::iterator itEmpty;
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::sequence seq({10u, 20u, 30u, 40u, 50u});
    EXPECT_NE(itEmpty, seq.begin());
    sdv::sequence<uint32_t>::iterator itPos = seq.begin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, seq.begin());
    sdv::sequence<uint32_t>::iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::sequence<uint32_t>::iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, 10u);
    EXPECT_EQ(itPos[0], 10u);
    EXPECT_EQ(itPos[4], 50u);
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 110u;
    EXPECT_EQ(seq[0], 110u);
    itPos[4] = 150u;
    EXPECT_EQ(seq[4], 150u);
    seq[0] = 10u;
    seq[4] = 50u;

    // Iterator iteration
    itPos++;
    EXPECT_EQ(*itPos, 20u);
    itPos += 3;
    EXPECT_EQ(*itPos, 50u);
    EXPECT_NO_THROW(itPos++); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, seq.end());
    EXPECT_NO_THROW(itPos++); // Will be ignored; doesn't increase even more
    itPos--;
    EXPECT_EQ(*itPos, 50u);
    itPos -= 4;
    EXPECT_EQ(*itPos, 10u);
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, 10u);
    EXPECT_EQ(itPos, seq.begin());

    // Const iterator
    sdv::sequence<uint32_t>::const_iterator itPosConst = seq.cbegin();
    EXPECT_EQ(itPos, itPosConst);
    itPosConst++;
    EXPECT_EQ(*itPosConst, 20u);
    EXPECT_NE(itPosConst, itPos);
    itPos++;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(seq.cbegin(), seq.begin());
    EXPECT_EQ(seq.begin(), seq.cbegin());
    EXPECT_EQ(seq.cend(), seq.end());
    EXPECT_EQ(seq.end(), seq.cend());
}

TEST_F(CSimpleSequenceTypeTest, ReverseIteratorBasedAccess)
{
    // Empty reverse iterator
    sdv::sequence<uint32_t>::reverse_iterator itEmpty;
    EXPECT_NO_THROW(itEmpty--);
    EXPECT_NO_THROW(itEmpty++);
    EXPECT_NO_THROW(itEmpty -= 2);
    EXPECT_NO_THROW(itEmpty += 2);
    EXPECT_THROW(itEmpty[0], sdv::XIndexOutOfRange);

    // Iterator assignment
    sdv::sequence seq({10u, 20u, 30u, 40u, 50u});
    EXPECT_NE(itEmpty, seq.rbegin());
    sdv::sequence<uint32_t>::reverse_iterator itPos = seq.rbegin();
    EXPECT_NE(itEmpty, itPos);
    EXPECT_EQ(itPos, seq.rbegin());
    sdv::sequence<uint32_t>::reverse_iterator itPosCopy;
    EXPECT_NE(itPosCopy, itPos);
    itPosCopy = itPos;
    EXPECT_EQ(itPosCopy, itPos);
    sdv::sequence<uint32_t>::reverse_iterator itPosNew;
    EXPECT_NE(itPosNew, itPos);
    itPosNew = std::move(itPosCopy);
    EXPECT_EQ(itPosNew, itPos);
    EXPECT_NE(itPosCopy, itPos);

    // Iterator element access
    EXPECT_EQ(*itPos, 50u);
    EXPECT_EQ(itPos[0], 50u);
    EXPECT_EQ(itPos[4], 10u);
    EXPECT_THROW(itPos[5], sdv::XIndexOutOfRange);
    *itPos = 150u;
    EXPECT_EQ(seq[4], 150u);
    itPos[4] = 110u;
    EXPECT_EQ(seq[0], 110u);
    seq[0] = 10u;
    seq[4] = 50u;

    // Iterator iteration
    itPos++;
    EXPECT_EQ(*itPos, 40u);
    itPos += 3;
    EXPECT_EQ(*itPos, 10u);
    EXPECT_NO_THROW(itPos++); // Increases at the position following last
    EXPECT_THROW(*itPos, sdv::XIndexOutOfRange);
    EXPECT_EQ(itPos, seq.rend());
    EXPECT_NO_THROW(itPos++); // Will be ignored; doesn't increase even more
    itPos--;
    EXPECT_EQ(*itPos, 10u);
    itPos -= 4;
    EXPECT_EQ(*itPos, 50u);
    EXPECT_NO_THROW(itPos--); // Will be ignored; doesn't decrease even more
    EXPECT_EQ(*itPos, 50u);
    EXPECT_EQ(itPos, seq.rbegin());

    // Const iterator
    sdv::sequence<uint32_t>::const_reverse_iterator itPosConst = seq.crbegin();
    EXPECT_EQ(itPos, itPosConst);
    itPosConst++;
    EXPECT_EQ(*itPosConst, 40u);
    EXPECT_NE(itPosConst, itPos);
    itPos++;
    EXPECT_EQ(itPosConst, itPos);
    EXPECT_EQ(seq.crbegin(), seq.rbegin());
    EXPECT_EQ(seq.rbegin(), seq.crbegin());
    EXPECT_EQ(seq.crend(), seq.rend());
    EXPECT_EQ(seq.rend(), seq.crend());
}

TEST_F(CSimpleSequenceTypeTest, SequenceCapacityStatic)
{
    // Empty string
    sdv::sequence<uint32_t, 20> seq;
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 20);

    // Filled string
    seq = {10u, 20u, 30u, 40u, 50u};
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
    seq.push_back(50u);
    seq.resize(10, 60u);
    EXPECT_EQ(seq[4], 50u);
    EXPECT_EQ(seq[5], 60u);
    EXPECT_EQ(seq[6], 60u);
    EXPECT_EQ(seq[7], 60u);
    EXPECT_EQ(seq[8], 60u);
    EXPECT_EQ(seq[9], 60u);
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

TEST_F(CSimpleSequenceTypeTest, SequenceCapacityDynamic)
{
    // Empty string
    sdv::sequence<uint32_t> seq;
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_EQ(seq.length(), 0);
    EXPECT_EQ(seq.capacity(), 0);

    // Filled string
    seq = {10u, 20u, 30u, 40u, 50u};
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
    seq.push_back(50u);
    seq.resize(10, 60u);
    EXPECT_EQ(seq[4], 50u);
    EXPECT_EQ(seq[5], 60u);
    EXPECT_EQ(seq[6], 60u);
    EXPECT_EQ(seq[7], 60u);
    EXPECT_EQ(seq[8], 60u);
    EXPECT_EQ(seq[9], 60u);
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

TEST_F(CSimpleSequenceTypeTest, InsertFunction)
{
    // Value assignment
    sdv::sequence<uint32_t> seq1;
    EXPECT_TRUE(seq1.empty());
    seq1.insert(seq1.begin(), 3, 10u);
    EXPECT_EQ(seq1[0], 10u);
    EXPECT_EQ(seq1[1], 10u);
    EXPECT_EQ(seq1[2], 10u);
    seq1.insert(seq1.begin(), 2, 20u);
    EXPECT_EQ(seq1[0], 20u);
    EXPECT_EQ(seq1[1], 20u);
    EXPECT_EQ(seq1[2], 10u);
    seq1.insert(seq1.end(), 2, 30u);
    EXPECT_EQ(seq1[4], 10u);
    EXPECT_EQ(seq1[5], 30u);
    EXPECT_EQ(seq1[6], 30u);
    seq1.insert(seq1.begin() + 4, 2, 40u);
    EXPECT_EQ(seq1[0], 20u);
    EXPECT_EQ(seq1[1], 20u);
    EXPECT_EQ(seq1[2], 10u);
    EXPECT_EQ(seq1[3], 10u);
    EXPECT_EQ(seq1[4], 40u);
    EXPECT_EQ(seq1[5], 40u);
    EXPECT_EQ(seq1[6], 10u);
    EXPECT_EQ(seq1[7], 30u);
    EXPECT_EQ(seq1[8], 30u);

    // Iterator assignment
    sdv::sequence<uint32_t> seq2;
    sdv::sequence<uint32_t> seq({10u, 20u, 30u});
    EXPECT_TRUE(seq2.empty());
    seq2.insert(seq2.begin(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);
    seq2.insert(seq2.begin(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);
    EXPECT_EQ(seq2[3], 10u);
    seq2.insert(seq2.end(), seq.begin(), seq.end());
    EXPECT_EQ(seq2[5], 30u);
    EXPECT_EQ(seq2[6], 10u);
    EXPECT_EQ(seq2[7], 20u);
    EXPECT_EQ(seq2[8], 30u);
    seq2.insert(seq2.begin() + 4, seq.begin(), seq.end());
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);
    EXPECT_EQ(seq2[3], 10u);
    EXPECT_EQ(seq2[4], 10u);
    EXPECT_EQ(seq2[5], 20u);
    EXPECT_EQ(seq2[6], 30u);
    EXPECT_EQ(seq2[7], 20u);
    EXPECT_EQ(seq2[8], 30u);
    EXPECT_EQ(seq2[9], 10u);
    EXPECT_EQ(seq2[10], 20u);
    EXPECT_EQ(seq2[11], 30u);

    // Initializer list assignment
    sdv::sequence<uint32_t> seq3;
    EXPECT_TRUE(seq3.empty());
    seq3.insert(seq3.begin(), {10u, 20u, 30u});
    EXPECT_EQ(seq3[0], 10u);
    EXPECT_EQ(seq3[1], 20u);
    EXPECT_EQ(seq3[2], 30u);
    seq3.insert(seq3.begin(), {10u, 20u, 30u});
    EXPECT_EQ(seq3[0], 10u);
    EXPECT_EQ(seq3[1], 20u);
    EXPECT_EQ(seq3[2], 30u);
    EXPECT_EQ(seq3[3], 10u);
    seq3.insert(seq3.end(), {10u, 20u, 30u});
    EXPECT_EQ(seq3[5], 30u);
    EXPECT_EQ(seq3[6], 10u);
    EXPECT_EQ(seq3[7], 20u);
    EXPECT_EQ(seq3[8], 30u);
    seq3.insert(seq3.begin() + 4, {10u, 20u, 30u});
    EXPECT_EQ(seq3[0], 10u);
    EXPECT_EQ(seq3[1], 20u);
    EXPECT_EQ(seq3[2], 30u);
    EXPECT_EQ(seq3[3], 10u);
    EXPECT_EQ(seq3[4], 10u);
    EXPECT_EQ(seq3[5], 20u);
    EXPECT_EQ(seq3[6], 30u);
    EXPECT_EQ(seq3[7], 20u);
    EXPECT_EQ(seq3[8], 30u);
    EXPECT_EQ(seq3[9], 10u);
    EXPECT_EQ(seq3[10], 20u);
    EXPECT_EQ(seq3[11], 30u);
}

TEST_F(CSimpleSequenceTypeTest, EraseFunction)
{
    // Single element erasure
    sdv::sequence<uint32_t> seq({10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u, 90u, 100u});
    sdv::sequence<uint32_t>::iterator it = seq.begin();
    seq.erase(it);
    EXPECT_EQ(seq[0], 20u);
    it = seq.end() - 1;
    seq.erase(it);
    EXPECT_EQ(seq[seq.size() - 1], 90u);
    it = seq.begin() + 4;
    seq.erase(it);
    EXPECT_EQ(seq[3], 50u);
    EXPECT_EQ(seq[4], 70u);
    seq = {10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u, 90u, 100u};
    sdv::sequence<uint32_t>::const_iterator cit = seq.cbegin();
    seq.erase(cit);
    EXPECT_EQ(seq[0], 20u);
    cit = seq.cend() - 1;
    seq.erase(cit);
    EXPECT_EQ(seq[seq.size() - 1], 90u);
    cit = seq.begin() + 4;
    seq.erase(cit);
    EXPECT_EQ(seq[3], 50u);
    EXPECT_EQ(seq[4], 70u);

    // Element range erasure
    seq = {10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u, 90u, 100u};
    sdv::sequence<uint32_t>::iterator it1 = seq.begin();
    sdv::sequence<uint32_t>::iterator it2 = it1 + 1;
    seq.erase(it1, it2);
    EXPECT_EQ(seq[0], 20u);
    it1 = seq.end() - 1;
    it2 = seq.end();
    seq.erase(it1, it2);
    EXPECT_EQ(seq[seq.size() - 1], 90u);
    it1 = seq.begin() + 1;
    it2 = it1 + 3;
    seq.erase(it1, it2);
    EXPECT_EQ(seq[0], 20u);
    EXPECT_EQ(seq[1], 60u);
    seq = {10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u, 90u, 100u};
    sdv::sequence<uint32_t>::const_iterator cit1 = seq.cbegin();
    sdv::sequence<uint32_t>::const_iterator cit2 = cit1 + 1;
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[0], 20u);
    cit1 = seq.end() - 1;
    cit2 = seq.end();
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[seq.size() - 1], 90u);
    cit1 = seq.begin() + 1;
    cit2 = cit1 + 3;
    seq.erase(cit1, cit2);
    EXPECT_EQ(seq[0], 20u);
    EXPECT_EQ(seq[1], 60u);
}

TEST_F(CSimpleSequenceTypeTest, PushPopFunctions)
{
    sdv::sequence<uint32_t> seq;
    uint32_t uiVal1 = 10u;
    seq.push_back(10u);
    EXPECT_EQ(seq[0], uiVal1);
    EXPECT_EQ(seq.size(), 1);
    uint32_t uiVal2 = 20u;
    seq.push_back(std::move(uiVal2));
    EXPECT_EQ(seq[1], 20u);
    EXPECT_EQ(seq.size(), 2);
    seq.pop_back();
    EXPECT_EQ(seq.size(), 1);
    seq.pop_back();
    EXPECT_TRUE(seq.empty());
    EXPECT_NO_THROW(seq.pop_back());
}

TEST_F(CSimpleSequenceTypeTest, SwapFunctionStatic)
{
    sdv::sequence<uint32_t, 5> seq1 = {10u, 20u, 30u};
    sdv::sequence<uint32_t, 10> seq2 = {40u, 50u, 60u, 70u};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], 40u);
    EXPECT_EQ(seq1[1], 50u);
    EXPECT_EQ(seq1[2], 60u);
    EXPECT_EQ(seq1[3], 70u);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], 10u);
    EXPECT_EQ(seq1[1], 20u);
    EXPECT_EQ(seq1[2], 30u);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], 40u);
    EXPECT_EQ(seq2[1], 50u);
    EXPECT_EQ(seq2[2], 60u);
    EXPECT_EQ(seq2[3], 70u);
}

TEST_F(CSimpleSequenceTypeTest, SwapFunctionDynamic)
{
    sdv::sequence<uint32_t> seq1 = {10u, 20u, 30u};
    sdv::sequence<uint32_t> seq2 = {40u, 50u, 60u, 70u};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], 40u);
    EXPECT_EQ(seq1[1], 50u);
    EXPECT_EQ(seq1[2], 60u);
    EXPECT_EQ(seq1[3], 70u);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], 10u);
    EXPECT_EQ(seq1[1], 20u);
    EXPECT_EQ(seq1[2], 30u);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], 40u);
    EXPECT_EQ(seq2[1], 50u);
    EXPECT_EQ(seq2[2], 60u);
    EXPECT_EQ(seq2[3], 70u);
}

TEST_F(CSimpleSequenceTypeTest, SwapFunctionMixed)
{
    sdv::sequence<uint32_t, 5> seq1 = {10u, 20u, 30u};
    sdv::sequence<uint32_t> seq2 = {40u, 50u, 60u, 70u};
    seq1.swap(seq2);
    EXPECT_EQ(seq1.size(), 4);
    EXPECT_EQ(seq1[0], 40u);
    EXPECT_EQ(seq1[1], 50u);
    EXPECT_EQ(seq1[2], 60u);
    EXPECT_EQ(seq1[3], 70u);
    EXPECT_EQ(seq2.size(), 3);
    EXPECT_EQ(seq2[0], 10u);
    EXPECT_EQ(seq2[1], 20u);
    EXPECT_EQ(seq2[2], 30u);

    sdv::swap(seq1, seq2);
    EXPECT_EQ(seq1.size(), 3);
    EXPECT_EQ(seq1[0], 10u);
    EXPECT_EQ(seq1[1], 20u);
    EXPECT_EQ(seq1[2], 30u);
    EXPECT_EQ(seq2.size(), 4);
    EXPECT_EQ(seq2[0], 40u);
    EXPECT_EQ(seq2[1], 50u);
    EXPECT_EQ(seq2[2], 60u);
    EXPECT_EQ(seq2[3], 70u);
}

TEST_F(CSimpleSequenceTypeTest, CompareOperator)
{
    sdv::sequence<uint32_t> seq1 = {10u, 20u, 30u};
    sdv::sequence<uint32_t> seq2;
    EXPECT_FALSE(seq1 == seq2);
    EXPECT_TRUE(seq1 != seq2);
    EXPECT_FALSE(seq1 < seq2);
    EXPECT_FALSE(seq1 <= seq2);
    EXPECT_TRUE(seq1 > seq2);
    EXPECT_TRUE(seq1 >= seq2);
    seq2 = {10u, 20u, 30u};
    EXPECT_TRUE(seq1 == seq2);
    EXPECT_FALSE(seq1 != seq2);
    EXPECT_FALSE(seq1 < seq2);
    EXPECT_TRUE(seq1 <= seq2);
    EXPECT_FALSE(seq1 > seq2);
    EXPECT_TRUE(seq1 >= seq2);
    seq2.push_back(40u);
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

TEST_F(CSimpleSequenceTypeTest, IteratorDistance)
{
    sdv::sequence<uint32_t> seq1 = {10u, 20u, 30u};
    EXPECT_EQ(std::distance(seq1.begin(), seq1.end()), seq1.length());
}