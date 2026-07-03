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

#include <mutex>
#include <iostream>
#include <functional>
#include <thread>
#include <support/component_impl.h>
#include <support/local_service_access.h>
#include "generated/test_component.h"

inline sdv::process::TProcessID GetProcessID()
{
    static sdv::process::TProcessID tProcessID = 0;
    if (!tProcessID)
    {
        sdv::process::IProcessInfo* pProcessInfo = sdv::core::GetObject<sdv::process::IProcessInfo>("ProcessControlService");
        if (!pProcessInfo) return 0;
        tProcessID = pProcessInfo->GetProcessID();
    }
    return tProcessID;
}

/**
 * @brief Hello device
 */
class CHelloDevice : public sdv::CSdvObject, public IHello
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_HelloDevice")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @param[in, out] sStages The struct stages will be filled by the components. The complex service calls the basic service. The
     * basic service calls the device. The device calls the system service and the utility.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello(/*inout*/ SStages& sStages) const override
    {
        sStages.bDevice = true;

        // Request for the system service
        auto pSystemHelloObject = sdv::core::GetObject<IHello>("TestObject_SystemHelloService");
        auto ptrUtilityHelloObject = sdv::core::CreateUtility("TestObject_HelloUtility");
        auto pUtilityHelloObject = ptrUtilityHelloObject.GetInterface<IHello>();
        return "PID#" + std::to_string(GetPID()) + " Hello from device\n" +
            std::string(pSystemHelloObject ? pSystemHelloObject->SayHello(sStages) : "") +
            std::string(pUtilityHelloObject ? pUtilityHelloObject->SayHello(sStages) : "");
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CHelloDevice)

