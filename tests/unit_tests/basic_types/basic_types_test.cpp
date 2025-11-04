#include "basic_types_test.h"

void CBasicTypesTest::SetUpTestCase()
{
    ASSERT_TRUE(true);
}

void CBasicTypesTest::TearDownTestCase() {}

void CBasicTypesTest::SetUp()
{}

void CBasicTypesTest::TearDown()
{}

size_t CBasicTypesTestAllocator::GetPtrCount() const
{
    return m_setPointers.size();
}

void CBasicTypesTestAllocator::ResetPtrSet()
{
    m_setPointers.clear();
}

void* CBasicTypesTestAllocator::Alloc(size_t nSize)
{
    void* p = CLocalMemMgr::Alloc(nSize);
    m_setPointers.insert(p);
    return p;
}

void* CBasicTypesTestAllocator::Realloc(void* pData, size_t nSize)
{
    m_setPointers.erase(pData);
    void* p = CLocalMemMgr::Realloc(pData, nSize);
    m_setPointers.insert(p);
    return p;
}

void CBasicTypesTestAllocator::Free(void* pData)
{
    m_setPointers.erase(pData);
    CLocalMemMgr::Free(pData);
}
