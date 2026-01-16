#include "marshall_object.h"
#include "com_ctrl.h"
#include <support/serdes.h>
#include "com_channel.h"

CMarshallObject::CMarshallObject(CCommunicationControl& rcontrol) : m_rcontrol(rcontrol)
{}

CMarshallObject::~CMarshallObject()
{

}

bool CMarshallObject::IsValid() const
{
    return m_tMarshallID.uiControl ? true : false;
}

void CMarshallObject::Reset()
{
    m_eType = EType::unknown;
    m_tMarshallID = { 0, 0, 0, 0 };
    m_pMarshall = nullptr;
    m_tStubID = {};
    m_ifcProxy = {};
    m_pConnector = nullptr;
}

sdv::interface_t CMarshallObject::InitializeAsProxy(uint32_t uiProxyIndex, sdv::interface_id id,
    sdv::ps::TMarshallID tStubID, CChannelConnector& rConnector)
{
    m_eType = EType::proxy;

    // Create marshall ID from index and a random number.
    sdv::ps::TMarshallID tMarshallID = { 0, GetProcessID(), uiProxyIndex, 0 };
    while (!tMarshallID.uiControl)
        tMarshallID.uiControl = static_cast<uint32_t>(rand());

    // Get the stub creation interface from the repository
    sdv::core::IRepositoryMarshallCreate* pMarshallCreate =
        sdv::core::GetObject<sdv::core::IRepositoryMarshallCreate>("RepositoryService");
    if (!pMarshallCreate)
    {
        Reset();
        return {};
    }
    m_ptrMarshallObject = pMarshallCreate->CreateProxyObject(id);
    if (!m_ptrMarshallObject)
    {
        Reset();
        return {};
    }

    // Set the proxy ID and control value
    sdv::ps::IMarshallObjectIdent* pObjectIdent = m_ptrMarshallObject.GetInterface<sdv::ps::IMarshallObjectIdent>();
    if (!pObjectIdent)
    {
        Reset();
        return {};
    }
    pObjectIdent->SetIdentification(tMarshallID);

    // Set the connecting stub ID for this proxy and get the target interface that the proxy provides.
    sdv::ps::IProxyControl* pProxyControl = m_ptrMarshallObject.GetInterface<sdv::ps::IProxyControl>();
    if (!pProxyControl)
    {
        Reset();
        return {};
    }
    m_tStubID = tStubID;
    m_ifcProxy = pProxyControl->GetTargetInterface();
    if (!m_ifcProxy)
    {
        Reset();
        return {};
    }

    // Get the IMarshallLink on the proxy object interface to link to the proxy and connect to IDataSend...
    sdv::ps::IMarshallLink* pMarshallLink = m_ptrMarshallObject.GetInterface<sdv::ps::IMarshallLink>();
    if (!pMarshallLink)
    {
        Reset();
        return {};
    }
    pMarshallLink->Link(this);

    // Store the channel.
    m_pConnector = &rConnector;

    // Store the ID...
    m_tMarshallID = tMarshallID;

    return m_ifcProxy;
}

bool CMarshallObject::InitializeAsStub(uint32_t uiStubIndex, sdv::interface_t ifc)
{
    if (!ifc) return false;

    m_eType = EType::stub;

    // Create marshall ID from index and a random number.
    sdv::ps::TMarshallID tMarshallID = { 0, GetProcessID(), uiStubIndex, static_cast<uint32_t>(rand()) };
    while (!tMarshallID.uiControl)
        tMarshallID.uiControl = static_cast<uint32_t>(rand());

    // Get the stub creation interface from the repository
    sdv::core::IRepositoryMarshallCreate* pMarshallCreate =
        sdv::core::GetObject<sdv::core::IRepositoryMarshallCreate>("RepositoryService");
    if (!pMarshallCreate)
    {
        Reset();
        return false;
    }
    m_ptrMarshallObject = pMarshallCreate->CreateStubObject(ifc.id());
    if (!m_ptrMarshallObject)
    {
        Reset();
        return false;
    }

    // Set the stub ID and control value
    sdv::ps::IMarshallObjectIdent* pObjectIdent = m_ptrMarshallObject.GetInterface<sdv::ps::IMarshallObjectIdent>();
    if (!pObjectIdent)
    {
        Reset();
        return {};
    }
    pObjectIdent->SetIdentification(tMarshallID);

    // Link the object to the stub.
    sdv::ps::IStubLink* psStubLink = m_ptrMarshallObject.GetInterface<sdv::ps::IStubLink>();
    if (!psStubLink)
    {
        Reset();
        return false;
    }
    psStubLink->Link(ifc);

    // Get the marshall interface
    m_pMarshall = m_ptrMarshallObject.GetInterface<sdv::ps::IMarshall>();
    if (!m_pMarshall)
    {
        Reset();
        return false;
    }

    // Everything is successful. Store the ID...
    m_tMarshallID = tMarshallID;

    return true;
}

sdv::ps::TMarshallID CMarshallObject::GetMarshallID() const
{
    return m_tMarshallID;
}

sdv::interface_t CMarshallObject::GetProxy()
{
    return m_ifcProxy;
}

sdv::sequence<sdv::pointer<uint8_t>> CMarshallObject::Call(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqInputData)
{
    // Differentiate between proxy and stub processing
    if (m_eType == EType::proxy)
    {
        // Make the call through the connector.
        if (!m_pConnector) throw sdv::ps::XMarshallNotInitialized();
        return m_pConnector->MakeCall(m_tMarshallID, m_tStubID, seqInputData);
    }
    else
    {
        // Make the call through the stored marshall object.
        if (!m_pMarshall) throw sdv::ps::XMarshallNotInitialized();
        return m_pMarshall->Call(seqInputData);
    }
}

