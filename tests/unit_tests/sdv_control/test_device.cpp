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

#include <gtest/gtest.h>
#include <support/component_impl.h>
#include "generated/test_component.h"

/**
 * @brief Device test component
 */
class CTestDevice : public sdv::CSdvObject, public IHello

{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("SDVControl_Test_Device")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from device";
    }
};

DEFINE_SDV_OBJECT(CTestDevice)

