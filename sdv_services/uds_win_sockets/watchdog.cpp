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
*   Denisa Ros - initial API and implementation
********************************************************************************/

#ifdef _WIN32

#include "watchdog.h"

void CWinSocketsConnectionWatchDog::AddConnectionImpl(const std::shared_ptr<void>& connection)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Connections[connection.get()] = connection;
}

void CWinSocketsConnectionWatchDog::RemoveConnection(const void* connection)
{
    if (!connection)
    {
        return;
    }

    std::shared_ptr<void> removed;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Connections.find(connection);
        if (it == m_Connections.end())
        {
            return;
        }

        removed = std::move(it->second);
        m_Connections.erase(it);
    }

    removed.reset();
}

void CWinSocketsConnectionWatchDog::Clear()
{
    std::map<const void*, std::shared_ptr<void>> localConnections;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        localConnections.swap(m_Connections);
    }

    localConnections.clear();
}

#endif // _WIN32
