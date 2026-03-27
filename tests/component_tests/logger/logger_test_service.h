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

#ifndef VAPI_DUMMY_TEST_SERVICE_H
#define VAPI_DUMMY_TEST_SERVICE_H

#include <support/component_impl.h>

/**
 * @brief  Class to create a dummy test service.
 */
class CLoggerTestService
	: public sdv::CSdvObject
{
public:
	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::basic_service)
	DECLARE_OBJECT_CLASS_NAME("LoggerTestService")

	CLoggerTestService();

};

DEFINE_SDV_OBJECT(CLoggerTestService)


#endif // !define VAPI_DUMMY_TEST_SERVICE_H
