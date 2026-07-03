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
 * @brief Complex hello service
 */
class CComplexHelloService : public sdv::CSdvObject, public IHello
{
public:
    CComplexHelloService()
    {
        std::cout << "CComplexHelloService constructor" << std::endl;
    }
    ~CComplexHelloService()
    {
        std::cout << "CComplexHelloService destructor" << std::endl;
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::complex_service)
    DECLARE_OBJECT_CLASS_NAME("TestObject_ComplexHelloService")

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @param[in, out] sStages The struct stages will be filled by the components. The complex service calls the basic service. The
     * basic service calls the device. The device calls the system service and the utility.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello(/*inout*/ SStages& sStages) const override
    {
        sStages.bComplexService = true;

        // Request for the basic server
        auto pHelloObject = sdv::core::GetObject<IHello>("TestObject_BasicHelloService");
        return "PID#" + std::to_string(GetPID()) + " Hello from complex service\n" +
            std::string(pHelloObject ? pHelloObject->SayHello(sStages) : "");
    }

    /**
     * @brief Get the PID of the process the component is running in... Overload of IHello::GetPID.
     */
    sdv::process::TProcessID GetPID() const
    {
        return GetProcessID();
    }
};
DEFINE_SDV_OBJECT(CComplexHelloService)

