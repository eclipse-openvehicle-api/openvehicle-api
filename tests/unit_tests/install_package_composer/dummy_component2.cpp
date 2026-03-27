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

#include <support/component_impl.h>

class CDummyService2 : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::complex_service)
    DECLARE_OBJECT_CLASS_NAME("DummyService #2")
};

DEFINE_SDV_OBJECT(CDummyService2)
