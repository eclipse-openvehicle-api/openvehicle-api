#include "../../include/gtest_custom.h"
#include "generated/smart_ifc.h"
#include <support/interface_ptr.h>

/**
 * @brief Member test helper class
 */
template <typename TInterface>
class CMemberHelper : public TInterface
{
public:
    /**
     * @brief Constructor
     * @param[in] rnTest Reference to the test call counter.
     */
    CMemberHelper(size_t& rnTest) : m_rnTest(rnTest)
    {}

    /**
     * @brief Test function. Overload of several interface functions.
     */
    virtual void Test() override
    {
        m_rnTest++;
    }

private:
    size_t&    m_rnTest;    ///< Referebce to test call counter.
};

/// Cast helper uses the same class.
using CCastHelper = CMemberHelper<IAmbiguousInterface>;

/**
 * @brief First class to test resolving the ambiguity cast.
 */
class CCastHelper1 : public CCastHelper
{
public:
    /**
     * @brief Constructor
     * @param[in] rnTest Reference to the test call counter.
     */
    CCastHelper1(size_t& rnTest) : CCastHelper(rnTest)
    {}
};

/**
 * @brief Second class to test resolving the ambiguity cast.
 */
class CCastHelper2 : public CCastHelper
{
public:
    /**
     * @brief Constructor
     * @param[in] rnTest Reference to the test call counter.
     */
    CCastHelper2(size_t& rnTest) : CCastHelper(rnTest)
    {}
};

/**
 * @brief Chain base helper class.
 */
template <typename TInterface1, typename TInterface2>
class CChainHelper : public sdv::IInterfaceAccess, public CMemberHelper<TInterface1>, public CMemberHelper<TInterface2>
{
public:
    /**
     * @brief Constructor
     * @param[in] rnTest Reference to the test call counter.
     */
    CChainHelper(size_t& rnTest) : CMemberHelper<TInterface1>(rnTest), CMemberHelper<TInterface2>(rnTest)
    {}

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(TInterface1)
        SDV_INTERFACE_ENTRY(TInterface2)
    END_SDV_INTERFACE_MAP()
};

/// Chain helper.
using TChainHelperBase = CChainHelper<IBaseInterface1, IBaseInterface2>;
using TChainHelperMember = CChainHelper<IChainMapMemberInterface1, IChainMapMemberInterface2>;
using TChainHelperMemberSmartPointer = CChainHelper<IChainMapMemberSmartPointerInterface1, IChainMapMemberSmartPointerInterface2>;
using TChainHelperMemberPointer = CChainHelper<IChainMapMemberPointerInterface1, IChainMapMemberPointerInterface2>;
using TChainHelperMemberSharedPointer = CChainHelper<IChainMapMemberSharedPointerInterface1, IChainMapMemberSharedPointerInterface2>;
using TChainHelperMemberWeakPointer = CChainHelper<IChainMapMemberWeakPointerInterface1, IChainMapMemberWeakPointerInterface2>;

/**
 * @brief Helper class to test the interface map.
 */
class CInterfaceMapHelper : public NamespaceTest1::INamespaceIfc1, public NamespaceTest2::INamespaceIfc2, public CCastHelper1,
    public CCastHelper2, public IDeniedInterface, public TChainHelperBase, public IConditionInterface,
    public IMainSectionInterface1, public IMainSectionInterface2, public ISection0Interface, public ISection1Interface,
    public ISection2Interface
{
public:
    CInterfaceMapHelper() : CCastHelper1(m_nTest), CCastHelper2(m_nTest), TChainHelperBase(m_nTest), m_member(m_nTest),
        m_memberPointer(m_nTest), m_pMember(&m_memberPointer), m_chainmember(m_nTest), m_chainmemberSmartPointer(m_nTest),
        m_ptrChainMemberSmartPointer(&m_chainmemberSmartPointer), m_chainmemberPointer(m_nTest),
        m_pChainMemberPointer(&m_chainmemberPointer)
    {
        m_ptrMember = std::make_shared<CMemberHelper<IMemberSharedPointer>>(m_nTest);
        m_ptrWeakMember = std::make_shared<CMemberHelper<IMemberWeakPointer>>(m_nTest);
        m_weakMember = m_ptrWeakMember;
        m_ptrChainMemberSharedPointer = std::make_shared<TChainHelperMemberSharedPointer>(m_nTest);
        m_ptrMemberWeakPointer = std::make_shared<TChainHelperMemberWeakPointer>(m_nTest);
        m_weakChainMemberWeakPointer = m_ptrMemberWeakPointer;
    }

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(NamespaceTest1::INamespaceIfc1)
        SDV_INTERFACE_USE_NAMESPACE(NamespaceTest2)
        SDV_INTERFACE_ENTRY(INamespaceIfc2)
        SDV_INTERFACE_ENTRY_MEMBER(IMember, m_member)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberPointerEmpty, m_pEmptyMember)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberPointer, m_pMember)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberSharedPointerEmpty, m_ptrEmptyMember)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberSharedPointer, m_ptrMember)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberWeakPointerEmpty, m_weakEmptyMember)
        SDV_INTERFACE_ENTRY_MEMBER(IMemberWeakPointer, m_weakMember)
        SDV_INTERFACE_ENTRY_INDIRECT(IAmbiguousInterface, CCastHelper2)
        SDV_INTERFACE_DENY_ENTRY(IDeniedInterface)
        SDV_INTERFACE_CHAIN_BASE(TChainHelperBase)
        SDV_INTERFACE_CHAIN_MEMBER(m_chainmember)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrChainMemberSmartPointerEmpty)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrChainMemberSmartPointer)
        SDV_INTERFACE_CHAIN_MEMBER(m_pChainMemberPointerEmpty)
        SDV_INTERFACE_CHAIN_MEMBER(m_pChainMemberPointer)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrChainMemberSharedPointerEmpty)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrChainMemberSharedPointer)
        SDV_INTERFACE_CHAIN_MEMBER(m_weakChainMemberWeakPointerEmpty)
        SDV_INTERFACE_CHAIN_MEMBER(m_weakChainMemberWeakPointer)
        SDV_INTERFACE_SET_SECTION_CONDITION(m_iSection == 0, 0)
        SDV_INTERFACE_SET_SECTION_CONDITION(m_iSection == 1, 1)
        SDV_INTERFACE_SET_SECTION_CONDITION(m_iSection == 2, 2)
        SDV_INTERFACE_SET_SECTION_CONDITION(m_iSection == 3, 3)
        SDV_INTERFACE_SECTION(0)
        SDV_INTERFACE_ENTRY(ISection0Interface)
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_ENTRY(IMainSectionInterface1)
        SDV_INTERFACE_SECTION(1)
        SDV_INTERFACE_ENTRY(ISection1Interface)
        SDV_INTERFACE_SET_SECTION(2)
        SDV_INTERFACE_SECTION(2)
        SDV_INTERFACE_ENTRY(ISection2Interface)
        SDV_INTERFACE_DEFAULT_SECTION()
        SDV_INTERFACE_ENTRY(IMainSectionInterface2)
        SDV_INTERFACE_CHECK_CONDITION(m_bConditionEnabled)
        SDV_INTERFACE_ENTRY(IConditionInterface)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Test function. Overload of several interface functions.
     */
    virtual void Test() override
    {
        m_nTest++;
    }

    /**
     * @brief Get the amount of times the test function has been called.
     * @return The test call counter.
     */
    size_t GetTestCallCount() const { return m_nTest; }

    /**
     * @brief When set, the interfaces following the conditional check are accessible.
     * @param[in] bEnable Flag to enable or disable the conditional check.
     */
    void SetCondition(bool bEnable) { m_bConditionEnabled = bEnable; }

    /**
     * @brief Set or reset the section to select.
     * @param[in] iSection The section to select or -1 for the main section.
     */
    void SetSection(int iSection = -1) { m_iSection = iSection; }

private:
    bool                                                        m_bConditionEnabled = false;            ///< Condition flag, enabling conditional interface.
    int                                                         m_iSection = -1;                        ///< Section selector
    size_t                                                      m_nTest = 0;                            ///< Test call counter.
    CMemberHelper<IMember>                                      m_member;                               ///< Member class.
    CMemberHelper<IMemberPointer>                               m_memberPointer;                        ///< Member for pointer class.
    CMemberHelper<IMemberPointerEmpty>*                         m_pEmptyMember = nullptr;               ///< Empty member class pointer.
    CMemberHelper<IMemberPointer>*                              m_pMember = nullptr;                    ///< Member class pointer.
    std::shared_ptr<CMemberHelper<IMemberSharedPointerEmpty>>   m_ptrEmptyMember;                       ///< Empty member class smart pointer.
    std::shared_ptr<CMemberHelper<IMemberSharedPointer>>        m_ptrMember;                            ///< Member class smart pointer.
    std::weak_ptr<CMemberHelper<IMemberWeakPointerEmpty>>       m_weakEmptyMember;                      ///< Empty member class weak pointer.
    std::shared_ptr<CMemberHelper<IMemberWeakPointer>>          m_ptrWeakMember;                        ///< Member class smart pointer.
    std::weak_ptr<CMemberHelper<IMemberWeakPointer>>            m_weakMember;                           ///< Member class weak pointer.
    TChainHelperMember                                          m_chainmember;                          ///< Chain the member.
    TChainHelperMemberSmartPointer                              m_chainmemberSmartPointer;              ///< Member for smart pointer chaining.
    sdv::TInterfaceAccessPtr                                    m_ptrChainMemberSmartPointerEmpty;      ///< Chain the empty smart pointer.
    sdv::TInterfaceAccessPtr                                    m_ptrChainMemberSmartPointer;           ///< Chain the smart pointer.
    TChainHelperMemberPointer                                   m_chainmemberPointer;                   ///< Member for pointer chaining.
    TChainHelperMemberPointer*                                  m_pChainMemberPointerEmpty = nullptr;   ///< Chain the empty pointer.
    TChainHelperMemberPointer*                                  m_pChainMemberPointer = nullptr;        ///< Chain the pointer.
    std::shared_ptr<TChainHelperMemberSharedPointer>            m_ptrChainMemberSharedPointerEmpty;     ///< Chain the empty shared pointer.
    std::shared_ptr<TChainHelperMemberSharedPointer>            m_ptrChainMemberSharedPointer;          ///< Chain the shared pointer.
    std::shared_ptr<TChainHelperMemberWeakPointer>              m_ptrMemberWeakPointer;                 ///< Member for weak pointer chaining.
    std::weak_ptr<TChainHelperMemberWeakPointer>                m_weakChainMemberWeakPointerEmpty;      ///< Chain the empty weak pointer.
    std::weak_ptr<TChainHelperMemberWeakPointer>                m_weakChainMemberWeakPointer;           ///< Chain the weak pointer.
};

TEST(Interface_Map_Test, Namespace)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    NamespaceTest1::INamespaceIfc1* pIfc1 = ptrHelper.GetInterface<NamespaceTest1::INamespaceIfc1>();
    ASSERT_NE(pIfc1, nullptr);
    pIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);

    NamespaceTest2::INamespaceIfc2* pIfc2 = ptrHelper.GetInterface<NamespaceTest2::INamespaceIfc2>();
    ASSERT_NE(pIfc2, nullptr);
    pIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, Member)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IMember* pMember = ptrHelper.GetInterface<IMember>();
    ASSERT_NE(pMember, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pMember->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, MemberPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IMemberPointerEmpty* pMemberPointerEmpty = ptrHelper.GetInterface<IMemberPointerEmpty>();
    IMemberPointer* pMemberPointer = ptrHelper.GetInterface<IMemberPointer>();
    EXPECT_EQ(pMemberPointerEmpty, nullptr);
    ASSERT_NE(pMemberPointer, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pMemberPointer->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, MemberSharedPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IMemberSharedPointerEmpty* pMemberPointerEmpty = ptrHelper.GetInterface<IMemberSharedPointerEmpty>();
    IMemberSharedPointer* pMemberPointer = ptrHelper.GetInterface<IMemberSharedPointer>();
    EXPECT_EQ(pMemberPointerEmpty, nullptr);
    ASSERT_NE(pMemberPointer, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pMemberPointer->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, MemberWeakPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IMemberWeakPointerEmpty* pMemberPointerEmpty = ptrHelper.GetInterface<IMemberWeakPointerEmpty>();
    IMemberWeakPointer* pMemberPointer = ptrHelper.GetInterface<IMemberWeakPointer>();
    EXPECT_EQ(pMemberPointerEmpty, nullptr);
    ASSERT_NE(pMemberPointer, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pMemberPointer->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, ResolveInterfaceAmbiguity)
{
    CInterfaceMapHelper helper;
    IAmbiguousInterface* pCastHelper1 = static_cast<CCastHelper1*>(&helper);
    IAmbiguousInterface* pCastHelper2 = static_cast<CCastHelper2*>(&helper);
    EXPECT_NE(pCastHelper1, nullptr);
    EXPECT_NE(pCastHelper2, nullptr);
    EXPECT_NE(pCastHelper1, pCastHelper2);

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IAmbiguousInterface* pCastedIfc = ptrHelper.GetInterface<IAmbiguousInterface>();
    ASSERT_NE(pCastedIfc, nullptr);
    EXPECT_EQ(pCastedIfc, pCastHelper2);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pCastedIfc->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, DenyInterface)
{
    CInterfaceMapHelper helper;
    IDeniedInterface* pDeniedIfc1 = static_cast<IDeniedInterface*>(&helper);
    EXPECT_NE(pDeniedIfc1, nullptr);

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IDeniedInterface* pDeniedIfc2 = ptrHelper.GetInterface<IDeniedInterface>();
    EXPECT_EQ(pDeniedIfc2, nullptr);
    EXPECT_NE(pDeniedIfc1, pDeniedIfc2);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseClass)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IBaseInterface1* pBaseIfc1 = ptrHelper.GetInterface<IBaseInterface1>();
    IBaseInterface2* pBaseIfc2 = ptrHelper.GetInterface<IBaseInterface2>();
    ASSERT_NE(pBaseIfc1, nullptr);
    ASSERT_NE(pBaseIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pBaseIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pBaseIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseMember)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IChainMapMemberInterface1* pChainMemberIfc1 = ptrHelper.GetInterface<IChainMapMemberInterface1>();
    IChainMapMemberInterface1* pChainMemberIfc2 = ptrHelper.GetInterface<IChainMapMemberInterface1>();
    ASSERT_NE(pChainMemberIfc1, nullptr);
    ASSERT_NE(pChainMemberIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pChainMemberIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pChainMemberIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseMemberSmartPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IChainMapMemberSmartPointerInterface1* pChainMemberIfc1 = ptrHelper.GetInterface<IChainMapMemberSmartPointerInterface1>();
    IChainMapMemberSmartPointerInterface2* pChainMemberIfc2 = ptrHelper.GetInterface<IChainMapMemberSmartPointerInterface2>();
    ASSERT_NE(pChainMemberIfc1, nullptr);
    ASSERT_NE(pChainMemberIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pChainMemberIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pChainMemberIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseMemberPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IChainMapMemberPointerInterface1* pChainMemberIfc1 = ptrHelper.GetInterface<IChainMapMemberPointerInterface1>();
    IChainMapMemberPointerInterface2* pChainMemberIfc2 = ptrHelper.GetInterface<IChainMapMemberPointerInterface2>();
    ASSERT_NE(pChainMemberIfc1, nullptr);
    ASSERT_NE(pChainMemberIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pChainMemberIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pChainMemberIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseMemberSharedPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IChainMapMemberSharedPointerInterface1* pChainMemberIfc1 = ptrHelper.GetInterface<IChainMapMemberSharedPointerInterface1>();
    IChainMapMemberSharedPointerInterface2* pChainMemberIfc2 = ptrHelper.GetInterface<IChainMapMemberSharedPointerInterface2>();
    ASSERT_NE(pChainMemberIfc1, nullptr);
    ASSERT_NE(pChainMemberIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pChainMemberIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pChainMemberIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ChainInterfaceMapBaseMemberWeakPointer)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IChainMapMemberWeakPointerInterface1* pChainMemberIfc1 = ptrHelper.GetInterface<IChainMapMemberWeakPointerInterface1>();
    IChainMapMemberWeakPointerInterface2* pChainMemberIfc2 = ptrHelper.GetInterface<IChainMapMemberWeakPointerInterface2>();
    ASSERT_NE(pChainMemberIfc1, nullptr);
    ASSERT_NE(pChainMemberIfc2, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pChainMemberIfc1->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
    pChainMemberIfc2->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 2ul);
}

TEST(Interface_Map_Test, ConditionalInterface)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    IConditionInterface* pIfc = ptrHelper.GetInterface<IConditionInterface>();
    EXPECT_EQ(pIfc, nullptr);

    helper.SetCondition(true);
    pIfc = ptrHelper.GetInterface<IConditionInterface>();
    ASSERT_NE(pIfc, nullptr);

    EXPECT_EQ(helper.GetTestCallCount(), 0ul);
    pIfc->Test();
    EXPECT_EQ(helper.GetTestCallCount(), 1ul);
}

TEST(Interface_Map_Test, SectionTest)
{
    CInterfaceMapHelper helper;

    sdv::TInterfaceAccessPtr ptrHelper = &helper;
    ASSERT_TRUE(ptrHelper);

    // Check for the interfaces
    auto fnCheck = [&](bool bExpectMain1, bool bExpectMain2, bool bExpectSection0, bool bExpectSection1, bool bExpectSection2, size_t nExpectCnt)
    {
        size_t nTemp = helper.GetTestCallCount();
        IMainSectionInterface1* pMain1 = ptrHelper.GetInterface<IMainSectionInterface1>();
        if (pMain1)
        {
            EXPECT_TRUE(bExpectMain1);
            pMain1->Test();
        } else
            EXPECT_FALSE(bExpectMain1);

        IMainSectionInterface2* pMain2 = ptrHelper.GetInterface<IMainSectionInterface2>();
        if (pMain2)
        {
            EXPECT_TRUE(bExpectMain2);
            pMain2->Test();
        } else
            EXPECT_FALSE(bExpectMain2);

        ISection0Interface* pSection0 = ptrHelper.GetInterface<ISection0Interface>();
        if (pSection0)
        {
            EXPECT_TRUE(bExpectSection0);
            pSection0->Test();
        } else
            EXPECT_FALSE(bExpectSection0);

        ISection1Interface* pSection1 = ptrHelper.GetInterface<ISection1Interface>();
        if (pSection1)
        {
            EXPECT_TRUE(bExpectSection1);
            pSection1->Test();
        } else
            EXPECT_FALSE(bExpectSection1);

        ISection2Interface* pSection2 = ptrHelper.GetInterface<ISection2Interface>();
        if (pSection2)
        {
            EXPECT_TRUE(bExpectSection2);
            pSection2->Test();
        } else
            EXPECT_FALSE(bExpectSection2);

        EXPECT_EQ(helper.GetTestCallCount(), nTemp + nExpectCnt);
    };

    // Mains section (-1) is enabled
    fnCheck(true, true, false, false, false, 2);

    // Enable section 0
    helper.SetSection(0);
    fnCheck(true, true, true, false, false, 3);

    // Enable section 1
    helper.SetSection(1);
    fnCheck(true, true, false, true, true, 4);

    // Enable section 2
    helper.SetSection(2);
    fnCheck(true, true, false, false, true, 3);

    // Enable section 3
    helper.SetSection(3);
    fnCheck(true, true, false, false, false, 2);

    // Enable main section
    helper.SetSection();
    fnCheck(true, true, false, false, false, 2);
}

