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

#include "permission_control.h"

#if defined __GNUC__ && defined _WIN32
thread_local std::list<std::pair<sdv::core::EAccessPermission, size_t>>&
    CPermissionControl::m_lstPermissionTracker = CreatePermissionList();
thread_local std::map<sdv::core::TPermissionID, CAccessPermission>&
    CPermissionControl::m_mapPermissions = CreatePermissionMap();
#else
thread_local std::list<std::pair<sdv::core::EAccessPermission, size_t>> CPermissionControl::m_lstPermissionTracker;
thread_local std::map<sdv::core::TPermissionID, CAccessPermission> CPermissionControl::m_mapPermissions;
#endif

CPermissionControl& GetPermissionControl()
{
    static CPermissionControl control;
    return control;
}

CAccessPermission::CAccessPermission(CPermissionControl& rPermissionControl, sdv::core::EAccessPermission ePermission) :
    m_rPermissionControl(rPermissionControl), m_ePermission(ePermission)
{
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        rPermissionControl.IncrementPermissionCount(ePermission);
}

CAccessPermission::CAccessPermission(const CAccessPermission& rPermission) :
    m_rPermissionControl(rPermission.m_rPermissionControl), m_ePermission(rPermission.m_ePermission)
{
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        m_rPermissionControl.IncrementPermissionCount(m_ePermission);
}

CAccessPermission::CAccessPermission(CAccessPermission&& rPermission) :
    m_rPermissionControl(rPermission.m_rPermissionControl), m_ePermission(rPermission.m_ePermission)
{
    rPermission.m_ePermission = sdv::core::EAccessPermission::not_set;
}

CAccessPermission& CAccessPermission::operator=(const CAccessPermission& rPermission)
{
    // Remove current permission and copy the new permission
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        m_rPermissionControl.DecrementPermissionCount(m_ePermission);
    m_ePermission = rPermission.m_ePermission;
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        m_rPermissionControl.IncrementPermissionCount(m_ePermission);
    return *this;
}

CAccessPermission& CAccessPermission::operator=(CAccessPermission && rPermission)
{
    // Remove current permission and move the new permission
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        m_rPermissionControl.DecrementPermissionCount(m_ePermission);
    m_ePermission = rPermission.m_ePermission;
    rPermission.m_ePermission = sdv::core::EAccessPermission::not_set;
    return *this;
}

CAccessPermission ::~CAccessPermission()
{
    if (m_ePermission != sdv::core::EAccessPermission::not_set)
        m_rPermissionControl.DecrementPermissionCount(m_ePermission);
}

sdv::core::EAccessPermission CAccessPermission::Permission() const
{
    return m_ePermission;
}

sdv::core::TPermissionID CPermissionControl::RestrictAccessPermission(/*in*/ sdv::core::EAccessPermission ePermission)
{
    // Unlike the CreateAccessPermission this function can only be used to restrict an already available restriction. It cannot be
    // used to set the initial restriction.
    if (m_lstPermissionTracker.empty()) return false;

    sdv::core::TPermissionID tPermissionID = m_idgen.Generate();
    if (!tPermissionID) return 0u;
    m_mapPermissions.emplace(tPermissionID, CreatePermissionObject(ePermission));
    return tPermissionID;
}

bool CPermissionControl::ReleaseAccessPermission(/*in*/ sdv::core::TPermissionID tPermissionID)
{
    auto itPermission = m_mapPermissions.find(tPermissionID);
    if (itPermission == m_mapPermissions.end()) return false;
    m_mapPermissions.erase(itPermission);
    return true;
}

sdv::core::TPermissionTransferID CPermissionControl::TransferCurrentPermission()
{
    std::unique_lock<std::mutex> lock(m_mtxPermissionTransfer);
    sdv::core::TPermissionTransferID tTransferID = m_idgen.Generate();
    m_mapPermissionTransfer[tTransferID] = GetCurrentPermission();
    return tTransferID;
}

sdv::core::TPermissionID CPermissionControl::SetAccessPermission(/*in*/ sdv::core::TPermissionTransferID tTransferID)
{
    // Get the access restriction level from the transfer permission map.
    std::unique_lock<std::mutex> lock(m_mtxPermissionTransfer);
    auto itTransferPermission = m_mapPermissionTransfer.find(tTransferID);
    if (itTransferPermission == m_mapPermissionTransfer.end()) return 0u;
    sdv::core::EAccessPermission ePermission = itTransferPermission->second;
    m_mapPermissionTransfer.erase(itTransferPermission);

    // Unlike the RestrictAccessPermission function, the SetAccessPermission function can be used to set an initial permission.
    sdv::core::TPermissionID tPermissionID = m_idgen.Generate();
    if (!tPermissionID) return 0u;
    m_mapPermissions.emplace(tPermissionID, CreatePermissionObject(ePermission));
    return tPermissionID;
}

sdv::core::EAccessPermission CPermissionControl::GetCurrentPermission() const
{
    return m_lstPermissionTracker.empty() ? sdv::core::EAccessPermission::restricted_access : m_lstPermissionTracker.front().first;
}

CAccessPermission CPermissionControl::CreatePermissionObject(sdv::core::EAccessPermission ePermission)
{
    sdv::core::EAccessPermission ePermissionLocal = std::max(ePermission, sdv::core::EAccessPermission::restricted_access);
    return CAccessPermission(*this, ePermissionLocal);
}

void CPermissionControl::IncrementPermissionCount(sdv::core::EAccessPermission ePermission)
{
    // In the permission tracker list, search for the counter for this permission or, if not existing, the next higher permission.
    auto itPermissionCounter = m_lstPermissionTracker.begin();
    for (; itPermissionCounter != m_lstPermissionTracker.end(); ++itPermissionCounter)
    {
        if (itPermissionCounter->first >= ePermission)
            break;
    }

    // If counter doesn't exist, insert a new counter.
    if (itPermissionCounter == m_lstPermissionTracker.end() || itPermissionCounter->first > ePermission)
        itPermissionCounter = m_lstPermissionTracker.insert(itPermissionCounter, std::make_pair(ePermission, 1));
    else // exists, increase the counter
        itPermissionCounter->second++;
}

void CPermissionControl::DecrementPermissionCount(sdv::core::EAccessPermission ePermission)
{
    // Find the permission counter
    auto itPermissionCounter = std::find_if(m_lstPermissionTracker.begin(), m_lstPermissionTracker.end(), [&](const auto& rprPermissionCount)
        { return rprPermissionCount.first == ePermission; });
    if (itPermissionCounter == m_lstPermissionTracker.end()) return;

    // Reduce the counter and if reaching zero, erase the counter
    itPermissionCounter->second--;
    if (!itPermissionCounter->second)
        m_lstPermissionTracker.erase(itPermissionCounter);
}
