/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include "iso_monitor.h"
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
        m_eObjectState = m_pObjectControl->GetObjectState();
    }
    else
        m_eObjectState = sdv::EObjectState::initialized;
}

sdv::EObjectState CIsoMonitor::GetObjectState() const
{
    if (m_pObjectControl) return m_pObjectControl->GetObjectState();
    return m_eObjectState;
}

void CIsoMonitor::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
{
    if (m_pObjectControl) m_pObjectControl->SetOperationMode(eMode);
}

sdv::u8string CIsoMonitor::GetObjectConfig() const
{
    if (m_pObjectControl)
        return m_pObjectControl->GetObjectConfig();
    return {};
}

void CIsoMonitor::Shutdown()
{
    m_eObjectState = sdv::EObjectState::shutdown_in_progress;
    if (m_pObjectControl)
    {
        m_pObjectControl->Shutdown();
        m_eObjectState = m_pObjectControl->GetObjectState();
    }
        m_eObjectState = sdv::EObjectState::destruction_pending;
    GetAppControl().RequestShutdown();
    m_pObjectControl = nullptr;
}

sdv::IInterfaceAccess* CIsoMonitor::GetContainedObject()
{
    return m_ptrObject;
}
