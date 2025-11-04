#include "includes.h"
#include "generator_test.h"
#include <support/local_service_access.h>

/**
 * @brief Align on 64 bit.
 * @tparam T The type to alignb the data for
 * @param[in] tVal The value to align
 * @return The aligned value
*/
template <typename T>
inline constexpr T Align(T tVal)
{
    return tVal % 8 ? tVal + 8 - tVal % 8 : tVal;
}

MockRepoService* MockRepoService::m_pRepo = nullptr;

void CGeneratorTest::SetUpTestCase()
{
    ASSERT_TRUE(true);
}

void CGeneratorTest::TearDownTestCase()
{}

void CGeneratorTest::SetUp()
{}

void CGeneratorTest::TearDown()
{}

void CGeneratorTest::RegisterStubObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub)
{
    // Check for valid interface
    if (!ifc) return;

    // Get the link interface of the stub object
    if (!pStub) return;
    sdv::ps::IStubLink* pStubLink = pStub->GetInterface<sdv::ps::IStubLink>();
    if (!pStubLink) return;

    // Register the stub object
    m_repo.m_ciService.RegisterStubObject(ifc, pStub);

    // Link the interface
    pStubLink->Link(ifc);
}

void CGeneratorTest::RegisterProxyObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub, sdv::IInterfaceAccess* pProxy)
{
    // Check for valid interface
    if (!ifc) return;
    if (!pStub) return;

    // Get the marshall interface of the stub object
    sdv::ps::IMarshall* pMarshall = pStub->GetInterface<sdv::ps::IMarshall>();
    if (!pMarshall) return;

    // Get the link interface of the proxy object
    if (!pProxy) return;
    sdv::ps::IMarshallLink* pMarshallLink = pProxy->GetInterface<sdv::ps::IMarshallLink>();
    if (!pMarshallLink) return;

    // Register the proxy
    m_repo.m_ciService.RegisterProxyObject(ifc, pProxy);

    // Link the proxy to the stub
    pMarshallLink->Link(pMarshall);
}

void MockCommunicationService::RegisterProxyObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pProxy)
{
    // Find the stub object
    auto itStub = m_mapStubObjects.find(ifc);
    if (itStub == m_mapStubObjects.end()) return;
    m_mapProxyObjects.try_emplace(itStub->second, std::make_pair(ifc.id(), pProxy));
}

void MockCommunicationService::RegisterStubObject(sdv::interface_t ifc, sdv::IInterfaceAccess* pStub)
{
    sdv::ps::TMarshallID tStubID{};
    tStubID.uiIdent = static_cast<uint32_t>(m_vecStubbjects.size());
    tStubID.uiControl = rand();
    m_vecStubbjects.push_back(pStub);
    m_mapStubObjects.try_emplace(ifc, tStubID);
}

sdv::pointer<uint8_t> CGeneratorTest::Allocate(uint32_t uiLength)
{
    return sdv::internal::make_ptr<uint8_t>(this, uiLength);
}

sdv::interface_t MockCommunicationService::GetProxy(/*in*/ const sdv::ps::TMarshallID& tStubID, /*in*/ sdv::interface_id id)
{
    auto itProxy = m_mapProxyObjects.find(tStubID);
    if (itProxy == m_mapProxyObjects.end()) return nullptr;
    if (itProxy->second.first != id) return nullptr;
    sdv::ps::IProxyControl* pControl = itProxy->second.second->GetInterface<sdv::ps::IProxyControl>();
    if (!pControl) throw sdv::XInvalidState();

    return pControl->GetTargetInterface();
}

sdv::ps::TMarshallID MockCommunicationService::GetStub(/*in*/ sdv::interface_t ifc)
{
    auto itStub = m_mapStubObjects.find(ifc);
    if (itStub == m_mapStubObjects.end()) return {};
    return itStub->second;
}


//sdv::interface_t MockCommunicationService::CreateInterfaceProxy(/*in*/ sdv::interface_id idInterfaceID, /*in*/ const sdv::com::SRemoteObjectID& sRemoteObject)
//{
//    if (sRemoteObject.uiClusterID != (uint64_t)this) return nullptr;
//    if (sRemoteObject.uiProcessID != (uint64_t)this) return nullptr;
//
//    // Get the proxy object
//    SStubInterface* pStubInterface = reinterpret_cast<SStubInterface*>(sRemoteObject.uiStubID);
//    if (!pStubInterface) return nullptr;
//    auto itProxy = m_mapProxyObjects.find(std::make_pair(idInterfaceID, pStubInterface->pStub));
//    if (itProxy == m_mapProxyObjects.end()) return nullptr;
//
//    // Assign the marshall interface
//    sdv::ps::IMarshallLink* pLink = itProxy->second->GetInterface<sdv::ps::IMarshallLink>();
//    if (!pLink) return nullptr;
//    pLink->Link(pMarshall);
//
//    // Return the proxy object interface
//    sdv::ps::IProxyControl* pGetIfc = itProxy->second->GetInterface<sdv::ps::IProxyControl>();
//    return pGetIfc->GetTargetInterface();
//}
//
//
//sdv::com::SRemoteObjectID MockCommunicationService::CreateInterfaceStub(/*in*/ sdv::interface_t pInterface)
//{
//    // Get the stub object
//    auto itStub = m_mapStubObjects.find(pInterface);
//    if (itStub == m_mapStubObjects.end()) return sdv::com::SRemoteObjectID{0,0,0};
//
//    // Assign the interface to the stub object
//    sdv::ps::IStubLink* pStubLink = itStub->second->GetInterface<sdv::ps::IStubLink>();
//    if (!pStubLink) return sdv::com::SRemoteObjectID{ 0,0,0 };
//    pStubLink->Link(pInterface);
//
//    // Store the information
//    std::shared_ptr<SStubInterface> ptrStubInterface = std::make_shared<SStubInterface>(pInterface.id(), pInterface, itStub->second);
//    m_mapStubInterfaces.insert(std::make_pair((uint64_t) ptrStubInterface.get(), ptrStubInterface));
//    uint64_t stubID=  (uint64_t)ptrStubInterface.get();
//    sdv::com::SParticipantID participant = GetParticipantInfo();
//    sdv::com::SRemoteObjectID ret;
//    ret.uiClusterID = participant.uiClusterID;
//    ret.uiProcessID = participant.uiProcessID;
//    ret.uiStubID = stubID;
//    return ret;
//}
//
//sdv::com::SParticipantID MockCommunicationService::GetParticipantInfo()
//{
//    sdv::com::SParticipantID sPartId{};
//    sPartId.uiProcessID = (uint64_t)this;
//    sPartId.uiClusterID = (uint64_t)this;
//    return sPartId;
//}

void* CGeneratorTest::Alloc(size_t nSize)
{
    return malloc(nSize);
}

void* CGeneratorTest::Realloc(void* pData, size_t nSize)
{
    return realloc(pData, nSize);
}

void CGeneratorTest::Free(void* pData)
{
    free(pData);
}

