#include "object_lifetime_control.h"
#include "sdv_core.h"

CObjectLifetimeControl::CObjectLifetimeControl(sdv::IInterfaceAccess* pObject, IObjectDestroyHandler& rHandler,
    bool bAutoDestroy /*= true*/) : m_rHandler(rHandler), m_ptrObject(pObject), m_bAutoDestroy(bAutoDestroy)
{
    m_pObjectDestroy = m_ptrObject.GetInterface<sdv::IObjectDestroy>();
    m_pObjectLifetime = m_ptrObject.GetInterface<sdv::IObjectLifetime>();
}

void CObjectLifetimeControl::DestroyObject()
{
    if (m_pObjectDestroy)
        m_pObjectDestroy->DestroyObject();
    else if (m_pObjectLifetime)
        m_pObjectLifetime->Decrement();

    m_rHandler.OnDestroyObject(m_ptrObject);

    if (m_bAutoDestroy) delete this;
}

void CObjectLifetimeControl::Increment()
{
    if (m_pObjectLifetime) m_pObjectLifetime->Increment();
}

bool CObjectLifetimeControl::Decrement()
{
    bool bRet = false;
    if (m_pObjectLifetime) bRet = m_pObjectLifetime->Decrement();
    if (bRet)
    {
        m_rHandler.OnDestroyObject(m_ptrObject);
        if (m_bAutoDestroy) delete this;
    }
    return bRet;
}

uint32_t CObjectLifetimeControl::GetCount() const
{
    return m_pObjectLifetime ? m_pObjectLifetime->GetCount() : 0u;
}
