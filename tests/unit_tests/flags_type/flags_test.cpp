#include <fstream>
#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../global/flags.h"

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

enum class ETest : uint16_t
{
    bit0 = 1,
    bit1 = 2,
    bit2 = 4,
    bit3 = 8,
    bit4 = 16,
    bit5 = 32,
    bit6 = 64,
    bit7 = 128,
    bit8 = 256,
    bit9 = 512,
    bit10 = 1024,
    bit11 = 2048,
    bit12 = 4096,
    bit13 = 8192,
    bit14 = 16384,
    bit15 = 32768,
};

TEST(FlagsTest, Construction)
{
    // Default constructor
    hlpr::flags<ETest> flagsEmpty;
    EXPECT_EQ(flagsEmpty._tValue, 0u);

    // Copy constructor
    hlpr::flags<ETest> flagsOrg;
    flagsOrg._tValue = 10;
    hlpr::flags<ETest> flagsCopy(flagsOrg);
    EXPECT_EQ(flagsOrg._tValue, 10u);
    EXPECT_EQ(flagsCopy._tValue, 10u);

    // Move constructor
    flagsOrg._tValue = 10;
    hlpr::flags<ETest> flagsMove(std::move(flagsOrg));
    EXPECT_EQ(flagsOrg._tValue, 0u);
    EXPECT_EQ(flagsCopy._tValue, 10u);

    // Flags value assignment
    hlpr::flags<ETest> flagsAssignFlagsValue(10);
    EXPECT_EQ(flagsAssignFlagsValue._tValue, 10u);

    // Value assignment
    hlpr::flags<ETest> flagsSingleValue(ETest::bit0);
    EXPECT_EQ(flagsSingleValue._tValue, 1u);

    // Init list constructor
    hlpr::flags<ETest> flagsInit{ ETest::bit0, ETest::bit10 };
    EXPECT_EQ(flagsInit._tValue, 1025u);
}

TEST(FlagsTest, Assignment)
{
    hlpr::flags<ETest> flagsTest;

    EXPECT_EQ(flagsTest._tValue, 0u);

    // Copy assignment
    hlpr::flags<ETest> flagsOrg;
    flagsOrg._tValue = 10;
    flagsTest = flagsOrg;
    EXPECT_EQ(flagsOrg._tValue, 10u);
    EXPECT_EQ(flagsTest._tValue, 10u);

    // Move constructor
    flagsOrg._tValue = 10;
    flagsTest = std::move(flagsOrg);
    EXPECT_EQ(flagsOrg._tValue, 0u);
    EXPECT_EQ(flagsTest._tValue, 10u);

    // Flags value assignment
    flagsTest = 20;
    EXPECT_EQ(flagsTest._tValue, 20u);

    // Value assignment
    flagsTest = ETest::bit5;
    EXPECT_EQ(flagsTest._tValue, 32u);

    // Init list constructor
    flagsTest = { ETest::bit0, ETest::bit10 };
    EXPECT_EQ(flagsTest._tValue, 1025u);
}

TEST(FlagsTest, Casting)
{
    hlpr::flags<ETest> flagsTest = { ETest::bit0, ETest::bit10 };
    EXPECT_EQ(static_cast<uint32_t>(flagsTest), 1025u);
    EXPECT_EQ(flagsTest.get(), 1025u);
}

TEST(FlagsTest, SetValue)
{
    hlpr::flags<ETest> flagsTest;
    EXPECT_EQ(flagsTest._tValue, 0u);

    // Set the value function
    flagsTest.add(ETest::bit0);
    EXPECT_EQ(flagsTest._tValue, 1u);

    // Set the value operator
    flagsTest += ETest::bit10;
    EXPECT_EQ(flagsTest._tValue, 1025u);

    // Set the value to a copy
    hlpr::flags<ETest> flagsCopy = flagsTest + ETest::bit11 + ETest::bit12;
    EXPECT_EQ(flagsTest._tValue, 1025u);
    EXPECT_EQ(flagsCopy._tValue, 7169u);
}

TEST(FlagsTest, RemoveValue)
{
    hlpr::flags<ETest> flagsTest = { ETest::bit0, ETest::bit10, ETest::bit11 };
    EXPECT_EQ(flagsTest._tValue, 3073u);

    // Remove the value function
    flagsTest.remove(ETest::bit11);
    EXPECT_EQ(flagsTest._tValue, 1025u);

    // Remove the value operator
    flagsTest -= ETest::bit10;
    EXPECT_EQ(flagsTest._tValue, 1u);

    // Set the value to a copy
    flagsTest = { ETest::bit0, ETest::bit10, ETest::bit11, ETest::bit12 };
    hlpr::flags<ETest> flagsCopy = flagsTest - ETest::bit11 - ETest::bit12;
    EXPECT_EQ(flagsTest._tValue, 7169u);
    EXPECT_EQ(flagsCopy._tValue, 1025u);
}

TEST(FlagsTest, CheckValue)
{
    // Compare value
    hlpr::flags<ETest> flagsTest = { ETest::bit0, ETest::bit10, ETest::bit11 };
    EXPECT_EQ(flagsTest, 3073u);
    EXPECT_EQ(flagsTest, hlpr::flags<ETest>(3073));
}

TEST(FlagsTest, CheckFlags)
{
    hlpr::flags<ETest> flagsTest = { ETest::bit0, ETest::bit10, ETest::bit11 };

    // Operator check
    EXPECT_FALSE(flagsTest & ETest::bit1);
    EXPECT_TRUE(flagsTest & ETest::bit10);

    // Check function
    EXPECT_FALSE(flagsTest.check(ETest::bit1));
    EXPECT_TRUE(flagsTest.check(ETest::bit10));

    // Check any
    EXPECT_FALSE(flagsTest.check_any({ ETest::bit1, ETest::bit2, ETest::bit3 }));
    EXPECT_TRUE(flagsTest.check_any({ ETest::bit1, ETest::bit2, ETest::bit10 }));

    // Check all
    EXPECT_FALSE(flagsTest.check_all({ ETest::bit0, ETest::bit1, ETest::bit10 }));
    EXPECT_TRUE(flagsTest.check_all({ ETest::bit0, ETest::bit10 }));
}

