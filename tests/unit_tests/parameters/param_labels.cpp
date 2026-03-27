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
#include <support/param_impl.h>

enum class EEnumWithoutLabelMap
{
    one,
    two,
    three
};

enum class EEnumWithEmptyLabelMap
{
    four,
    five,
    six
};

BEGIN_SDV_LABEL_MAP(EEnumWithEmptyLabelMap)
END_SDV_LABEL_MAP()

enum class EEnumWithLabelMap
{
    seven,
    eight,
    nine
};

BEGIN_SDV_LABEL_MAP(EEnumWithLabelMap)
    SDV_LABEL_ENTRY(EEnumWithLabelMap::seven, "seven")
    SDV_LABEL_ENTRY(EEnumWithLabelMap::eight, "eight")
    SDV_LABEL_ENTRY(EEnumWithLabelMap::nine, "nine")
END_SDV_LABEL_MAP()

enum class EEnumWithLabelMapBehindFunction
{
    ten,
    eleven,
    twelve
};

sdv::sequence<sdv::SLabelInfo::SLabel> GetLabelMapForEnumBeforMapDefinition()
{
    return sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumWithLabelMapBehindFunction>();
}

BEGIN_SDV_LABEL_MAP(EEnumWithLabelMapBehindFunction)
    SDV_LABEL_ENTRY(EEnumWithLabelMapBehindFunction::ten, "ten")
    SDV_LABEL_ENTRY(EEnumWithLabelMapBehindFunction::eleven, "eleven")
    SDV_LABEL_ENTRY(EEnumWithLabelMapBehindFunction::twelve, "twelve")
END_SDV_LABEL_MAP()

namespace LabelMapTest
{
    enum class EEnumInNamespaceWithLabelMap
    {
        thirteen,
        fourteen,
        fifteen
    };

    BEGIN_SDV_LABEL_MAP(EEnumInNamespaceWithLabelMap)
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMap::thirteen, "thirteen")
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMap::fourteen, "fourteen")
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMap::fifteen, "fifteen")
    END_SDV_LABEL_MAP()

    enum class EEnumInNamespaceWithLabelMapBehindFunction
    {
        sixteen,
        seventeen,
        eighteen
    };

    sdv::sequence<sdv::SLabelInfo::SLabel> GetLabelMapForEnumBeforMapDefinition()
    {
        return sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumInNamespaceWithLabelMapBehindFunction>();
    }

    BEGIN_SDV_LABEL_MAP(EEnumInNamespaceWithLabelMapBehindFunction)
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMapBehindFunction::sixteen, "sixteen")
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMapBehindFunction::seventeen, "seventeen")
        SDV_LABEL_ENTRY(EEnumInNamespaceWithLabelMapBehindFunction::eighteen, "eighteen")
    END_SDV_LABEL_MAP()
} // namespace LabelMapTest

class CLabelMapTest
{
public:
    enum class EEnumInClassWithLabelMap
    {
        nineteen,
        twenty,
        twenty_one
    };

    BEGIN_SDV_LABEL_MAP(EEnumInClassWithLabelMap)
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMap::nineteen, "nineteen")
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMap::twenty, "twenty")
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMap::twenty_one, "twenty_one")
    END_SDV_LABEL_MAP()

    enum class EEnumInClassWithLabelMapBehindFunction
    {
        twenty_two,
        twenty_three,
        twenty_four
    };

    static sdv::sequence<sdv::SLabelInfo::SLabel> GetLabelMapForEnumBeforMapDefinition()
    {
        return sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumInClassWithLabelMapBehindFunction>();
    }

    BEGIN_SDV_LABEL_MAP(EEnumInClassWithLabelMapBehindFunction)
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMapBehindFunction::twenty_two, "twenty_two")
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMapBehindFunction::twenty_three, "twenty_three")
        SDV_LABEL_ENTRY(EEnumInClassWithLabelMapBehindFunction::twenty_four, "twenty_four")
    END_SDV_LABEL_MAP()

    enum class EEnumInClassWithLabelMapOutsideClass
    {
        twenty_five,
        twenty_six,
        twenty_seven
    };

    enum class EEnumInClassWithLabelMapOutsideClassBehindFunction
    {
        twenty_eight,
        twenty_nine,
        thirty
    };
};

BEGIN_SDV_LABEL_MAP(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass)
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_five, "twenty_five")
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_six, "twenty_six")
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_seven, "twenty_seven")
END_SDV_LABEL_MAP()

sdv::sequence<sdv::SLabelInfo::SLabel> GetLabelMapForEnumBeforMapDefinitionOutsideClass()
{
    return sdv::internal::GetLabelMapHelper().GetLabelMap<CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction>();
}

BEGIN_SDV_LABEL_MAP(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction)
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::twenty_eight, "twenty_eight")
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::twenty_nine, "twenty_nine")
    SDV_LABEL_ENTRY(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::thirty, "thirty")
END_SDV_LABEL_MAP()

TEST(ParameterTest, LabelMap)
{
    // Enum without label map
    auto seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumWithoutLabelMap>();
    EXPECT_TRUE(seqLabelMap.empty());

    // Enum with empty label map
    seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumWithEmptyLabelMap>();
    EXPECT_TRUE(seqLabelMap.empty());

    // Enum with label map
    seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<EEnumWithLabelMap>();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(EEnumWithLabelMap::seven));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(EEnumWithLabelMap::eight));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(EEnumWithLabelMap::nine));

    // Enum with label map with access function defined before label map
    seqLabelMap = GetLabelMapForEnumBeforMapDefinition();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(EEnumWithLabelMapBehindFunction::ten));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(EEnumWithLabelMapBehindFunction::eleven));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(EEnumWithLabelMapBehindFunction::twelve));

    // Enum in namespace with label map
    seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<LabelMapTest::EEnumInNamespaceWithLabelMap>();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMap::thirteen));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMap::fourteen));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMap::fifteen));

    // Enum in namespace with label map with access function defined before label map
    seqLabelMap = LabelMapTest::GetLabelMapForEnumBeforMapDefinition();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMapBehindFunction::sixteen));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMapBehindFunction::seventeen));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(LabelMapTest::EEnumInNamespaceWithLabelMapBehindFunction::eighteen));

    // Enum in class with label map
    seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<CLabelMapTest::EEnumInClassWithLabelMap>();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMap::nineteen));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMap::twenty));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMap::twenty_one));

    // Enum in class with label map with access function defined before label map
    seqLabelMap = CLabelMapTest::GetLabelMapForEnumBeforMapDefinition();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapBehindFunction::twenty_two));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapBehindFunction::twenty_three));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapBehindFunction::twenty_four));

    // Enum in class with label map outside class
    seqLabelMap = sdv::internal::GetLabelMapHelper().GetLabelMap<CLabelMapTest::EEnumInClassWithLabelMapOutsideClass>();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(seqLabelMap[0].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_five));
    EXPECT_EQ(seqLabelMap[1].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_six));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClass::twenty_seven));

    // Enum in class with label map outside class behind access function
    seqLabelMap = GetLabelMapForEnumBeforMapDefinitionOutsideClass();
    ASSERT_EQ(seqLabelMap.size(), 3u);
    EXPECT_EQ(
        seqLabelMap[0].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::twenty_eight));
    EXPECT_EQ(
        seqLabelMap[1].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::twenty_nine));
    EXPECT_EQ(seqLabelMap[2].anyValue, static_cast<int>(CLabelMapTest::EEnumInClassWithLabelMapOutsideClassBehindFunction::thirty));
};
