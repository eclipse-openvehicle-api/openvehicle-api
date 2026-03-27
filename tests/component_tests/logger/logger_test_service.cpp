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

#include "logger_test_service.h"

CLoggerTestService::CLoggerTestService()
{
	// auto test = sdv::core::GetObject("DataDispatchService");
	// auto foo = test.GetInterface<sdv::core::IDataDispatchService>();
	// foo->HasSignal(0);

	SDV_LOG(sdv::core::ELogSeverity::info, "Info: Logging from Dummy test service via macro!");

}
