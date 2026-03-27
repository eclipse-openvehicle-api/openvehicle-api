/********************************************************************************
 * Copyright (c) 2025-2026 Contributors to the Eclipse Foundation
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
#include "../../../global/localmemmgr.h"
#include "../../../global/process_watchdog.h"

#include <support/param_impl.h>

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    // The memory manager registers itself into the system and needs to stay in scope.
    CLocalMemMgr memmgr;

    testing::InitGoogleTest(&argc, argv);
    auto iRet = RUN_ALL_TESTS();

    // Clear the label map, deallocating the labels before the memory manager gets out of scope.
    sdv::internal::GetLabelMapHelper().Clear();

    return iRet;
}
