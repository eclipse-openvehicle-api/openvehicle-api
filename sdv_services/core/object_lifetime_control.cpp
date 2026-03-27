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
