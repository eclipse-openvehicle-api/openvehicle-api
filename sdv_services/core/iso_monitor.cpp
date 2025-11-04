#include "iso_monitor.h"
#include "sdv_core.h"
#include "app_control.h"

CIsoMonitor::CIsoMonitor(sdv::IInterfaceAccess* pObject) :
    m_ptrObject(pObject), m_pObjectControl(m_ptrObject.GetInterface<sdv::IObjectControl>())
{}

CIsoMonitor::~CIsoMonitor()
{
    GetAppControl().RequestShutdown();
}

void CIsoMonitor::Initialize(/*in*/ const sdv::u8string& ssObjectConfig)
{
    if (m_pObjectControl)
    {
        m_pObjectControl->Initialize(ssObjectConfig);
        m_eObjectStatus = m_pObjectControl->GetStatus();
    }
    else
        m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CIsoMonitor::GetStatus() const
{
    if (m_pObjectControl) return m_pObjectControl->GetStatus();
    return m_eObjectStatus;
}

void CIsoMonitor::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
{
    if (m_pObjectControl) m_pObjectControl->SetOperationMode(eMode);
}

void CIsoMonitor::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;
    if (m_pObjectControl)
    {
        m_pObjectControl->Shutdown();
        m_eObjectStatus = m_pObjectControl->GetStatus();
    }
        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    GetAppControl().RequestShutdown();
    m_pObjectControl = nullptr;
}

sdv::IInterfaceAccess* CIsoMonitor::GetContainedObject()
{
    return m_ptrObject;
}
