#ifndef BASIC_TYPES_TEST_H
#define BASIC_TYPES_TEST_H

#include <gtest/gtest.h>
#define NO_SDV_LOCAL_CORE_FUNC
#include "../../../global/localmemmgr.h"

class CBasicTypesTestAllocator : public CLocalMemMgr
{
public:
    /**
    * @brief Get the amount of pointers allocated.
    * @return
    */
    size_t GetPtrCount() const;

    /**
    * @brief Reset the pointer set to start a new measurement.
    */
    void ResetPtrSet();

private:
    /**
    * @brief Allocate memory. Overload of sdv::internal::IInternalMemAlloc::Alloc.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Alloc(size_t nSize) override;

    /**
    * @brief Reallocate memory. Overload of sdv::internal::IInternalMemAlloc::Realloc.
    * @param[in] pData Pointer to a previous allocation or NULL when no previous allocation was available.
    * @param[in] nSize The size of the memory to allocate (in bytes).
    * @return Pointer to the memory allocation or NULL when memory allocation failed.
    */
    virtual void* Realloc(void* pData, size_t nSize) override;

    /**
    * @brief Free a memory allocation. Overload of sdv::internal::IInternalMemAlloc::Free.
    * @param[in] pData Pointer to a previous allocation.
    */
    virtual void Free(void* pData) override;

    std::set<void*> m_setPointers;      ///< Pointer set for pointer tracking.
};

/**
 * @brief Get the instance of the local memory manager.
 * @return Reference to the memory manager.
*/
inline CBasicTypesTestAllocator& GetMemMgr()
{
    static CBasicTypesTestAllocator memmgr;
    return memmgr;
}

namespace sdv
{
    namespace core
    {
        /**
         * @brief Access to the core.
         * @return Smart pointer to the core services interface.
         */
        inline TInterfaceAccessPtr GetCore()
        {
            static CBasicTypesTestAllocator& rmemmgr = ::GetMemMgr();
            return &rmemmgr;
        }

        /**
         * @brief Access to specific interface of the core.
         * @tparam TInterface Type of interface to return.
         * @return Pointer to the interface or NULL when the interface was not exposed.
         */
        template <typename TInterface>
        inline TInterface* GetCore()
        {
            return GetCore().GetInterface<TInterface>();
        }
    }
}

/**
 * @brief Test class for code genertion tests.
 */
class CBasicTypesTest : public testing::Test
{
public:
    /**
     * @brief Constructor
     */
    CBasicTypesTest() = default;

    /**
     * @brief Set up the test suite.
     */
    static void SetUpTestCase();

    /**
     * @brief Tear down the test suite.
     */
    static void TearDownTestCase();

    /**
     * @brief Test setup.
     */
    void SetUp() override;

    /**
     * @brief Test teardown.
     */
    void TearDown() override;
};

namespace testing::internal
{
    // Helper function for *_STREQ on wide strings.
    inline AssertionResult CmpHelperSTREQ(const char* lhs_expression, const char* rhs_expression, const char16_t* lhs, const char16_t* rhs)
    {
        if (std::equal(lhs, lhs + std::char_traits<char16_t>::length(lhs), rhs))
        {
            return AssertionSuccess();
        }

        return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs), PrintToString(rhs), false);
    }

    inline AssertionResult CmpHelperSTRNE(const char* s1_expression, const char* s2_expression, const char16_t* lhs, const char16_t* rhs)
    {
        if (!std::equal(lhs, lhs + std::char_traits<char16_t>::length(lhs), rhs))
        {
            return AssertionSuccess();
        }

        return AssertionFailure() << "Expected: (" << s1_expression << ") != (" << s2_expression
                                  << "), actual: " << PrintToString(lhs) << " vs " << PrintToString(rhs);
    }

    inline AssertionResult CmpHelperSTREQ(const char* lhs_expression, const char* rhs_expression, const char32_t* lhs, const char32_t* rhs)
    {
        if (std::equal(lhs, lhs + std::char_traits<char32_t>::length(lhs), rhs))
        {
            return AssertionSuccess();
        }

        return EqFailure(lhs_expression, rhs_expression, PrintToString(lhs), PrintToString(rhs), false);
    }

    inline AssertionResult CmpHelperSTRNE(const char* s1_expression, const char* s2_expression, const char32_t* lhs, const char32_t* rhs)
    {
        if (!std::equal(lhs, lhs + std::char_traits<char32_t>::length(lhs), rhs))
        {
            return AssertionSuccess();
        }

        return AssertionFailure() << "Expected: (" << s1_expression << ") != (" << s2_expression
                                  << "), actual: " << PrintToString(lhs) << " vs " << PrintToString(rhs);
    }
}

#endif // !defined BASIC_TYPES_TEST_H