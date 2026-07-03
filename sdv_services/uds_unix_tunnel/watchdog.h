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

#if defined(__unix__)
#ifndef UDS_UNIX_TUNNEL_WATCHDOG_H
#define UDS_UNIX_TUNNEL_WATCHDOG_H

#include <map>
#include <memory>
#include <mutex>

class CUnixTunnelConnectionWatchDog
{
public:
    template<typename T>
    void AddConnection(const std::shared_ptr<T>& connection)
    {
        if (!connection)
        {
            return;
        }

        AddConnectionImpl(std::static_pointer_cast<void>(connection));
    }

    void RemoveConnection(const void* connection);
    void Clear();

private:
    void AddConnectionImpl(const std::shared_ptr<void>& connection);

    std::mutex m_Mutex;
    std::map<const void*, std::shared_ptr<void>> m_Connections;
};

#endif // UDS_UNIX_TUNNEL_WATCHDOG_H
#endif // defined(__unix__)
