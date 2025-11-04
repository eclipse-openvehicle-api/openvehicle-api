#ifndef GENERATOR_TEST_H
#define GENERATOR_TEST_H

#include "includes.h"
#include <queue>
#include <thread>
#include <interfaces/core.h>
#include <interfaces/core_ps.h>
#include <support/local_service_access.h>
#include <support/mem_access.h>
#include <support/interface_ptr.h>
#include <interfaces/com.h>
#include <interfaces/repository.h>
#include <support/pssup.h>

class MockCommunicationService :
    public sdv::IInterfaceAccess,
    public sdv::ps::IMarshallAccess
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::ps::IMarshallAccess)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Register proxy object.
     */
    void RegisterProxyObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pProxy);

    /**
     * @brief Register stub object.
     */
    void RegisterStubObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub);

    /**
     * @brief Get a proxy for the interface connection to the stub. Overload of sdv::ps::IMarshallAcess::GetProxy.
     * @param[in] tStubID Reference to the ID of the stub to connect to.
     * @param[in] id The interface ID to get the proxy for.
     * @return Returns the interface to the proxy object.
     */
    virtual sdv::interface_t GetProxy(/*in*/ const sdv::ps::TMarshallID& tStubID, /*in*/ sdv::interface_id id) override;

    /**
     * @brief Get a stub for the interface with the supplied ID. Overload of sdv::ps::IMarshallAcess::GetStub.
     * @param[in] ifc The interface to get the stub for..
     * @return Returns the Stub ID that is assigned to the interface. Or an empty ID when no stub could be found.
     */
    virtual sdv::ps::TMarshallID GetStub(/*in*/ sdv::interface_t ifc) override;

private:
    /// Vector with stub objects. The position in the vector determines the stub ID.
    std::vector<sdv::IInterfaceAccess*> m_vecStubbjects;

    /// Map with stub objects for a specific interface
    std::map<sdv::interface_t, sdv::ps::TMarshallID> m_mapStubObjects;

    /// Map with proxy objects for a specific stub ID.
    std::map<sdv::ps::TMarshallID, std::pair<sdv::interface_id, sdv::IInterfaceAccess*>> m_mapProxyObjects;
};

class MockRepoService: public sdv::IInterfaceAccess, public sdv::core::IObjectAccess
{
public:
    MockRepoService()
    {
        m_pRepo = this;
    }

    ~MockRepoService()
    {
        m_pRepo = nullptr;
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::core::IObjectAccess)
    END_SDV_INTERFACE_MAP()

    virtual IInterfaceAccess* GetObject(/*in*/ const sdv::u8string& ssObjectName) override
    {
        if(ssObjectName == "CommunicationControl")
        {
            return &m_ciService;
        }
        return nullptr;
    }

    virtual IInterfaceAccess* GetObjectByID(/*in*/ sdv::core::TObjectID /*rObjectID*/) override
    {
        return nullptr;
    }

    MockCommunicationService m_ciService;
    static MockRepoService* m_pRepo;      ///< Global repository access
};

inline sdv::TInterfaceAccessPtr sdv::core::GetObject(const std::string& ssObjectName)
{
    return MockRepoService::m_pRepo ? MockRepoService::m_pRepo->GetObject(ssObjectName) : nullptr;
}

/**
* @brief Test class for code genertion tests.
*/
class CGeneratorTest :
	public testing::Test, public sdv::core::IMemoryAlloc, public sdv::IInterfaceAccess, public sdv::internal::IInternalMemAlloc
{
public:
    /**
     * @brief Constructor
     */
    CGeneratorTest() = default;

    /**
     * @brief Set up the test suite.
     */
    static void SetUpTestCase();

    /**
     * @brief Tear down the test suite.
     */
    static void TearDownTestCase();

    BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::core::IMemoryAlloc)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Test setup.
     */
    void SetUp() override;

    /**
     * @brief Test teardown.
     */
    void TearDown() override;

    /**
     * @brief Register stub object and link the object to the stub.
     * @param[in] ifc Target interface to marshall.
     * @param[in] pStub Pointer to the stub object.
     */
    void RegisterStubObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub);

    /**
    * @brief Register proxy object and link the previously registered stub to the proxy.
    * @attention Will only work if the stub was registered before the proxy.
    * @param[in] ifc Target interface to marshall.
    * @param[in] pStub Pointer to the stub object.
    * @param[in] pProxy Pointer to the proxy object.
    */
    void RegisterProxyObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub, sdv::IInterfaceAccess* pProxy);

    /**
	 * @brief Allocate a memory block of the provided length. Overload of sdv::core::IMemoryAlloc::Allocate.
	 * @param[in] uiLength The length of the memory block to allocate.
	 * @return The allocated memory or NULL when memory allocation was not possible.
	 */
	virtual sdv::pointer<uint8_t> Allocate(uint32_t uiLength) override;

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

    MockRepoService m_repo;
};

#endif // !defined(GENERATOR_TEST_H)
