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

#include <fstream>
#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../global/unique_id.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(UniqueID, TestSmallID)
{
    CUniqueID<int, 4> generator;

    // Max 15 ids possible
    std::set<int> setUsed;
    int iNumber;
    for (size_t n = 0; n < 15; n++)
    {
        iNumber = generator.Generate();
        EXPECT_NE(iNumber, 0);
        EXPECT_EQ(setUsed.find(iNumber), setUsed.end());
        setUsed.insert(iNumber);
    }

    // One more ID should fail
    iNumber = generator.Generate();
    EXPECT_EQ(iNumber, 0);
}

TEST(UniqueID, TestSmallID2)
{
    CUniqueID<uint8_t> generator;

    // Max 255 ids possible
    std::set<uint32_t> setUsed;
    uint32_t uiNumber = 0;
    for (size_t n = 0; n < 255; n++)
    {
        uiNumber = generator.Generate();
        EXPECT_NE(uiNumber, 0u);
        EXPECT_EQ(setUsed.find(uiNumber), setUsed.end());
        setUsed.insert(uiNumber);
    }

    // One more ID should fail
    uiNumber = generator.Generate();
    EXPECT_EQ(uiNumber, 0u);
}

TEST(UniqueID, TestLargeID)
{
    CUniqueID<uint64_t> generator;

    // Run at the most 20000 times and check whether the number occurs multiple times
    std::set<uint64_t> setUsed;
    uint64_t uiNumber;
    for (size_t n = 0; n < 20000; n++)
    {
        uiNumber = generator.Generate();
        EXPECT_NE(uiNumber, 0u);
        EXPECT_EQ(setUsed.find(uiNumber), setUsed.end());
        setUsed.insert(uiNumber);
    }
}
