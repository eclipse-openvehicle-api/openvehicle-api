#include <gtest/gtest.h>
#include "composer_test_suite.h"
#include "../../../sdv_services/core/installation_manifest.h"

using CPackageVersionTest = CInstallPackageComposerTest;

sdv::installation::SPackageVersion sVersion0    = {0, 0, 0};
sdv::installation::SPackageVersion sVersion0_9  = {0, 9, 0};
sdv::installation::SPackageVersion sVersion1    = {1, 0, 0};
sdv::installation::SPackageVersion sVersion1a   = {1, 0, 1};
sdv::installation::SPackageVersion sVersion1b   = {1, 0, 2};
sdv::installation::SPackageVersion sVersion1_1  = {1, 1, 0};
sdv::installation::SPackageVersion sVersion1_1a = {1, 1, 1};
sdv::installation::SPackageVersion sVersion1_2  = {1, 2, 0};
sdv::installation::SPackageVersion sVersion2    = {2, 0, 0};

TEST_F(CPackageVersionTest, PackageVersionEqual)
{
    EXPECT_TRUE(sVersion0     == sVersion0);
    EXPECT_FALSE(sVersion0_9  == sVersion0);
    EXPECT_FALSE(sVersion1    == sVersion0);
    EXPECT_FALSE(sVersion1a   == sVersion0);
    EXPECT_FALSE(sVersion1b   == sVersion0);
    EXPECT_FALSE(sVersion1_1  == sVersion0);
    EXPECT_FALSE(sVersion1_1a == sVersion0);
    EXPECT_FALSE(sVersion1_2  == sVersion0);
    EXPECT_FALSE(sVersion2    == sVersion0);

    EXPECT_FALSE(sVersion0    == sVersion0_9);
    EXPECT_TRUE(sVersion0_9   == sVersion0_9);
    EXPECT_FALSE(sVersion1    == sVersion0_9);
    EXPECT_FALSE(sVersion1a   == sVersion0_9);
    EXPECT_FALSE(sVersion1b   == sVersion0_9);
    EXPECT_FALSE(sVersion1_1  == sVersion0_9);
    EXPECT_FALSE(sVersion1_1a == sVersion0_9);
    EXPECT_FALSE(sVersion1_2  == sVersion0_9);
    EXPECT_FALSE(sVersion2    == sVersion0_9);

    EXPECT_FALSE(sVersion0    == sVersion1);
    EXPECT_FALSE(sVersion0_9  == sVersion1);
    EXPECT_TRUE(sVersion1     == sVersion1);
    EXPECT_FALSE(sVersion1a   == sVersion1);
    EXPECT_FALSE(sVersion1b   == sVersion1);
    EXPECT_FALSE(sVersion1_1  == sVersion1);
    EXPECT_FALSE(sVersion1_1a == sVersion1);
    EXPECT_FALSE(sVersion1_2  == sVersion1);
    EXPECT_FALSE(sVersion2    == sVersion1);

    EXPECT_FALSE(sVersion0    == sVersion1a);
    EXPECT_FALSE(sVersion0_9  == sVersion1a);
    EXPECT_FALSE(sVersion1    == sVersion1a);
    EXPECT_TRUE(sVersion1a    == sVersion1a);
    EXPECT_FALSE(sVersion1b   == sVersion1a);
    EXPECT_FALSE(sVersion1_1  == sVersion1a);
    EXPECT_FALSE(sVersion1_1a == sVersion1a);
    EXPECT_FALSE(sVersion1_2  == sVersion1a);
    EXPECT_FALSE(sVersion2    == sVersion1a);

    EXPECT_FALSE(sVersion0    == sVersion1b);
    EXPECT_FALSE(sVersion0_9  == sVersion1b);
    EXPECT_FALSE(sVersion1    == sVersion1b);
    EXPECT_FALSE(sVersion1a   == sVersion1b);
    EXPECT_TRUE(sVersion1b    == sVersion1b);
    EXPECT_FALSE(sVersion1_1  == sVersion1b);
    EXPECT_FALSE(sVersion1_1a == sVersion1b);
    EXPECT_FALSE(sVersion1_2  == sVersion1b);
    EXPECT_FALSE(sVersion2    == sVersion1b);

    EXPECT_FALSE(sVersion0    == sVersion1_1);
    EXPECT_FALSE(sVersion0_9  == sVersion1_1);
    EXPECT_FALSE(sVersion1    == sVersion1_1);
    EXPECT_FALSE(sVersion1a   == sVersion1_1);
    EXPECT_FALSE(sVersion1b   == sVersion1_1);
    EXPECT_TRUE(sVersion1_1   == sVersion1_1);
    EXPECT_FALSE(sVersion1_1a == sVersion1_1);
    EXPECT_FALSE(sVersion1_2  == sVersion1_1);
    EXPECT_FALSE(sVersion2    == sVersion1_1);

    EXPECT_FALSE(sVersion0    == sVersion1_1a);
    EXPECT_FALSE(sVersion0_9  == sVersion1_1a);
    EXPECT_FALSE(sVersion1    == sVersion1_1a);
    EXPECT_FALSE(sVersion1a   == sVersion1_1a);
    EXPECT_FALSE(sVersion1b   == sVersion1_1a);
    EXPECT_FALSE(sVersion1_1  == sVersion1_1a);
    EXPECT_TRUE(sVersion1_1a  == sVersion1_1a);
    EXPECT_FALSE(sVersion1_2  == sVersion1_1a);
    EXPECT_FALSE(sVersion2    == sVersion1_1a);

    EXPECT_FALSE(sVersion0    == sVersion1_2);
    EXPECT_FALSE(sVersion0_9  == sVersion1_2);
    EXPECT_FALSE(sVersion1    == sVersion1_2);
    EXPECT_FALSE(sVersion1a   == sVersion1_2);
    EXPECT_FALSE(sVersion1b   == sVersion1_2);
    EXPECT_FALSE(sVersion1_1  == sVersion1_2);
    EXPECT_FALSE(sVersion1_1a == sVersion1_2);
    EXPECT_TRUE(sVersion1_2   == sVersion1_2);
    EXPECT_FALSE(sVersion2    == sVersion1_2);

    EXPECT_FALSE(sVersion0    == sVersion2);
    EXPECT_FALSE(sVersion0_9  == sVersion2);
    EXPECT_FALSE(sVersion1    == sVersion2);
    EXPECT_FALSE(sVersion1a   == sVersion2);
    EXPECT_FALSE(sVersion1b   == sVersion2);
    EXPECT_FALSE(sVersion1_1  == sVersion2);
    EXPECT_FALSE(sVersion1_1a == sVersion2);
    EXPECT_FALSE(sVersion1_2  == sVersion2);
    EXPECT_TRUE(sVersion2     == sVersion2);
}

TEST_F(CPackageVersionTest, PackageVersionLargerThanOrEqual)
{
    EXPECT_TRUE(sVersion0    >= sVersion0);
    EXPECT_TRUE(sVersion0_9  >= sVersion0);
    EXPECT_TRUE(sVersion1    >= sVersion0);
    EXPECT_TRUE(sVersion1a   >= sVersion0);
    EXPECT_TRUE(sVersion1b   >= sVersion0);
    EXPECT_TRUE(sVersion1_1  >= sVersion0);
    EXPECT_TRUE(sVersion1_1a >= sVersion0);
    EXPECT_TRUE(sVersion1_2  >= sVersion0);
    EXPECT_TRUE(sVersion2    >= sVersion0);

    EXPECT_FALSE(sVersion0   >= sVersion0_9);
    EXPECT_TRUE(sVersion0_9  >= sVersion0_9);
    EXPECT_TRUE(sVersion1    >= sVersion0_9);
    EXPECT_TRUE(sVersion1a   >= sVersion0_9);
    EXPECT_TRUE(sVersion1b   >= sVersion0_9);
    EXPECT_TRUE(sVersion1_1  >= sVersion0_9);
    EXPECT_TRUE(sVersion1_1a >= sVersion0_9);
    EXPECT_TRUE(sVersion1_2  >= sVersion0_9);
    EXPECT_TRUE(sVersion2    >= sVersion0_9);

    EXPECT_FALSE(sVersion0   >= sVersion1);
    EXPECT_FALSE(sVersion0_9 >= sVersion1);
    EXPECT_TRUE(sVersion1    >= sVersion1);
    EXPECT_TRUE(sVersion1a   >= sVersion1);
    EXPECT_TRUE(sVersion1b   >= sVersion1);
    EXPECT_TRUE(sVersion1_1  >= sVersion1);
    EXPECT_TRUE(sVersion1_1a >= sVersion1);
    EXPECT_TRUE(sVersion1_2  >= sVersion1);
    EXPECT_TRUE(sVersion2    >= sVersion1);

    EXPECT_FALSE(sVersion0   >= sVersion1a);
    EXPECT_FALSE(sVersion0_9 >= sVersion1a);
    EXPECT_FALSE(sVersion1   >= sVersion1a);
    EXPECT_TRUE(sVersion1a   >= sVersion1a);
    EXPECT_TRUE(sVersion1b   >= sVersion1a);
    EXPECT_TRUE(sVersion1_1  >= sVersion1a);
    EXPECT_TRUE(sVersion1_1a >= sVersion1a);
    EXPECT_TRUE(sVersion1_2  >= sVersion1a);
    EXPECT_TRUE(sVersion2    >= sVersion1a);

    EXPECT_FALSE(sVersion0   >= sVersion1b);
    EXPECT_FALSE(sVersion0_9 >= sVersion1b);
    EXPECT_FALSE(sVersion1   >= sVersion1b);
    EXPECT_FALSE(sVersion1a  >= sVersion1b);
    EXPECT_TRUE(sVersion1b   >= sVersion1b);
    EXPECT_TRUE(sVersion1_1  >= sVersion1b);
    EXPECT_TRUE(sVersion1_1a >= sVersion1b);
    EXPECT_TRUE(sVersion1_2  >= sVersion1b);
    EXPECT_TRUE(sVersion2    >= sVersion1b);

    EXPECT_FALSE(sVersion0   >= sVersion1_1);
    EXPECT_FALSE(sVersion0_9 >= sVersion1_1);
    EXPECT_FALSE(sVersion1   >= sVersion1_1);
    EXPECT_FALSE(sVersion1a  >= sVersion1_1);
    EXPECT_FALSE(sVersion1b  >= sVersion1_1);
    EXPECT_TRUE(sVersion1_1  >= sVersion1_1);
    EXPECT_TRUE(sVersion1_1a >= sVersion1_1);
    EXPECT_TRUE(sVersion1_2  >= sVersion1_1);
    EXPECT_TRUE(sVersion2    >= sVersion1_1);

    EXPECT_FALSE(sVersion0   >= sVersion1_1a);
    EXPECT_FALSE(sVersion0_9 >= sVersion1_1a);
    EXPECT_FALSE(sVersion1   >= sVersion1_1a);
    EXPECT_FALSE(sVersion1a  >= sVersion1_1a);
    EXPECT_FALSE(sVersion1b  >= sVersion1_1a);
    EXPECT_FALSE(sVersion1_1 >= sVersion1_1a);
    EXPECT_TRUE(sVersion1_1a >= sVersion1_1a);
    EXPECT_TRUE(sVersion1_2  >= sVersion1_1a);
    EXPECT_TRUE(sVersion2    >= sVersion1_1a);

    EXPECT_FALSE(sVersion0    >= sVersion1_2);
    EXPECT_FALSE(sVersion0_9  >= sVersion1_2);
    EXPECT_FALSE(sVersion1    >= sVersion1_2);
    EXPECT_FALSE(sVersion1a   >= sVersion1_2);
    EXPECT_FALSE(sVersion1b   >= sVersion1_2);
    EXPECT_FALSE(sVersion1_1  >= sVersion1_2);
    EXPECT_FALSE(sVersion1_1a >= sVersion1_2);
    EXPECT_TRUE(sVersion1_2   >= sVersion1_2);
    EXPECT_TRUE(sVersion2     >= sVersion1_2);

    EXPECT_FALSE(sVersion0    >= sVersion2);
    EXPECT_FALSE(sVersion0_9  >= sVersion2);
    EXPECT_FALSE(sVersion1    >= sVersion2);
    EXPECT_FALSE(sVersion1a   >= sVersion2);
    EXPECT_FALSE(sVersion1b   >= sVersion2);
    EXPECT_FALSE(sVersion1_1  >= sVersion2);
    EXPECT_FALSE(sVersion1_1a >= sVersion2);
    EXPECT_FALSE(sVersion1_2  >= sVersion2);
    EXPECT_TRUE(sVersion2     >= sVersion2);
}

TEST_F(CPackageVersionTest, PackageVersionLargerThan)
{
    EXPECT_FALSE(sVersion0   > sVersion0);
    EXPECT_TRUE(sVersion0_9  > sVersion0);
    EXPECT_TRUE(sVersion1    > sVersion0);
    EXPECT_TRUE(sVersion1a   > sVersion0);
    EXPECT_TRUE(sVersion1b   > sVersion0);
    EXPECT_TRUE(sVersion1_1  > sVersion0);
    EXPECT_TRUE(sVersion1_1a > sVersion0);
    EXPECT_TRUE(sVersion1_2  > sVersion0);
    EXPECT_TRUE(sVersion2    > sVersion0);

    EXPECT_FALSE(sVersion0   > sVersion0_9);
    EXPECT_FALSE(sVersion0_9 > sVersion0_9);
    EXPECT_TRUE(sVersion1    > sVersion0_9);
    EXPECT_TRUE(sVersion1a   > sVersion0_9);
    EXPECT_TRUE(sVersion1b   > sVersion0_9);
    EXPECT_TRUE(sVersion1_1  > sVersion0_9);
    EXPECT_TRUE(sVersion1_1a > sVersion0_9);
    EXPECT_TRUE(sVersion1_2  > sVersion0_9);
    EXPECT_TRUE(sVersion2    > sVersion0_9);

    EXPECT_FALSE(sVersion0   > sVersion1);
    EXPECT_FALSE(sVersion0_9 > sVersion1);
    EXPECT_FALSE(sVersion1   > sVersion1);
    EXPECT_TRUE(sVersion1a   > sVersion1);
    EXPECT_TRUE(sVersion1b   > sVersion1);
    EXPECT_TRUE(sVersion1_1  > sVersion1);
    EXPECT_TRUE(sVersion1_1a > sVersion1);
    EXPECT_TRUE(sVersion1_2  > sVersion1);
    EXPECT_TRUE(sVersion2    > sVersion1);

    EXPECT_FALSE(sVersion0   > sVersion1a);
    EXPECT_FALSE(sVersion0_9 > sVersion1a);
    EXPECT_FALSE(sVersion1   > sVersion1a);
    EXPECT_FALSE(sVersion1a  > sVersion1a);
    EXPECT_TRUE(sVersion1b   > sVersion1a);
    EXPECT_TRUE(sVersion1_1  > sVersion1a);
    EXPECT_TRUE(sVersion1_1a > sVersion1a);
    EXPECT_TRUE(sVersion1_2  > sVersion1a);
    EXPECT_TRUE(sVersion2    > sVersion1a);

    EXPECT_FALSE(sVersion0   > sVersion1b);
    EXPECT_FALSE(sVersion0_9 > sVersion1b);
    EXPECT_FALSE(sVersion1   > sVersion1b);
    EXPECT_FALSE(sVersion1a  > sVersion1b);
    EXPECT_FALSE(sVersion1b  > sVersion1b);
    EXPECT_TRUE(sVersion1_1  > sVersion1b);
    EXPECT_TRUE(sVersion1_1a > sVersion1b);
    EXPECT_TRUE(sVersion1_2  > sVersion1b);
    EXPECT_TRUE(sVersion2    > sVersion1b);

    EXPECT_FALSE(sVersion0   > sVersion1_1);
    EXPECT_FALSE(sVersion0_9 > sVersion1_1);
    EXPECT_FALSE(sVersion1   > sVersion1_1);
    EXPECT_FALSE(sVersion1a  > sVersion1_1);
    EXPECT_FALSE(sVersion1b  > sVersion1_1);
    EXPECT_FALSE(sVersion1_1 > sVersion1_1);
    EXPECT_TRUE(sVersion1_1a > sVersion1_1);
    EXPECT_TRUE(sVersion1_2  > sVersion1_1);
    EXPECT_TRUE(sVersion2    > sVersion1_1);

    EXPECT_FALSE(sVersion0    > sVersion1_1a);
    EXPECT_FALSE(sVersion0_9  > sVersion1_1a);
    EXPECT_FALSE(sVersion1    > sVersion1_1a);
    EXPECT_FALSE(sVersion1a   > sVersion1_1a);
    EXPECT_FALSE(sVersion1b   > sVersion1_1a);
    EXPECT_FALSE(sVersion1_1  > sVersion1_1a);
    EXPECT_FALSE(sVersion1_1a > sVersion1_1a);
    EXPECT_TRUE(sVersion1_2   > sVersion1_1a);
    EXPECT_TRUE(sVersion2     > sVersion1_1a);

    EXPECT_FALSE(sVersion0    > sVersion1_2);
    EXPECT_FALSE(sVersion0_9  > sVersion1_2);
    EXPECT_FALSE(sVersion1    > sVersion1_2);
    EXPECT_FALSE(sVersion1a   > sVersion1_2);
    EXPECT_FALSE(sVersion1b   > sVersion1_2);
    EXPECT_FALSE(sVersion1_1  > sVersion1_2);
    EXPECT_FALSE(sVersion1_1a > sVersion1_2);
    EXPECT_FALSE(sVersion1_2  > sVersion1_2);
    EXPECT_TRUE(sVersion2     > sVersion1_2);

    EXPECT_FALSE(sVersion0    > sVersion2);
    EXPECT_FALSE(sVersion0_9  > sVersion2);
    EXPECT_FALSE(sVersion1    > sVersion2);
    EXPECT_FALSE(sVersion1a   > sVersion2);
    EXPECT_FALSE(sVersion1b   > sVersion2);
    EXPECT_FALSE(sVersion1_1  > sVersion2);
    EXPECT_FALSE(sVersion1_1a > sVersion2);
    EXPECT_FALSE(sVersion1_2  > sVersion2);
    EXPECT_FALSE(sVersion2    > sVersion2);
}

TEST_F(CPackageVersionTest, PackageVersionNotEqual)
{
    EXPECT_FALSE(sVersion0   != sVersion0);
    EXPECT_TRUE(sVersion0_9  != sVersion0);
    EXPECT_TRUE(sVersion1    != sVersion0);
    EXPECT_TRUE(sVersion1a   != sVersion0);
    EXPECT_TRUE(sVersion1b   != sVersion0);
    EXPECT_TRUE(sVersion1_1  != sVersion0);
    EXPECT_TRUE(sVersion1_1a != sVersion0);
    EXPECT_TRUE(sVersion1_2  != sVersion0);
    EXPECT_TRUE(sVersion2    != sVersion0);

    EXPECT_TRUE(sVersion0    != sVersion0_9);
    EXPECT_FALSE(sVersion0_9 != sVersion0_9);
    EXPECT_TRUE(sVersion1    != sVersion0_9);
    EXPECT_TRUE(sVersion1a   != sVersion0_9);
    EXPECT_TRUE(sVersion1b   != sVersion0_9);
    EXPECT_TRUE(sVersion1_1  != sVersion0_9);
    EXPECT_TRUE(sVersion1_1a != sVersion0_9);
    EXPECT_TRUE(sVersion1_2  != sVersion0_9);
    EXPECT_TRUE(sVersion2    != sVersion0_9);

    EXPECT_TRUE(sVersion0    != sVersion1);
    EXPECT_TRUE(sVersion0_9  != sVersion1);
    EXPECT_FALSE(sVersion1   != sVersion1);
    EXPECT_TRUE(sVersion1a   != sVersion1);
    EXPECT_TRUE(sVersion1b   != sVersion1);
    EXPECT_TRUE(sVersion1_1  != sVersion1);
    EXPECT_TRUE(sVersion1_1a != sVersion1);
    EXPECT_TRUE(sVersion1_2  != sVersion1);
    EXPECT_TRUE(sVersion2    != sVersion1);

    EXPECT_TRUE(sVersion0    != sVersion1a);
    EXPECT_TRUE(sVersion0_9  != sVersion1a);
    EXPECT_TRUE(sVersion1    != sVersion1a);
    EXPECT_FALSE(sVersion1a  != sVersion1a);
    EXPECT_TRUE(sVersion1b   != sVersion1a);
    EXPECT_TRUE(sVersion1_1  != sVersion1a);
    EXPECT_TRUE(sVersion1_1a != sVersion1a);
    EXPECT_TRUE(sVersion1_2  != sVersion1a);
    EXPECT_TRUE(sVersion2    != sVersion1a);

    EXPECT_TRUE(sVersion0    != sVersion1b);
    EXPECT_TRUE(sVersion0_9  != sVersion1b);
    EXPECT_TRUE(sVersion1    != sVersion1b);
    EXPECT_TRUE(sVersion1a   != sVersion1b);
    EXPECT_FALSE(sVersion1b  != sVersion1b);
    EXPECT_TRUE(sVersion1_1  != sVersion1b);
    EXPECT_TRUE(sVersion1_1a != sVersion1b);
    EXPECT_TRUE(sVersion1_2  != sVersion1b);
    EXPECT_TRUE(sVersion2    != sVersion1b);

    EXPECT_TRUE(sVersion0    != sVersion1_1);
    EXPECT_TRUE(sVersion0_9  != sVersion1_1);
    EXPECT_TRUE(sVersion1    != sVersion1_1);
    EXPECT_TRUE(sVersion1a   != sVersion1_1);
    EXPECT_TRUE(sVersion1b   != sVersion1_1);
    EXPECT_FALSE(sVersion1_1 != sVersion1_1);
    EXPECT_TRUE(sVersion1_1a != sVersion1_1);
    EXPECT_TRUE(sVersion1_2  != sVersion1_1);
    EXPECT_TRUE(sVersion2    != sVersion1_1);

    EXPECT_TRUE(sVersion0     != sVersion1_1a);
    EXPECT_TRUE(sVersion0_9   != sVersion1_1a);
    EXPECT_TRUE(sVersion1     != sVersion1_1a);
    EXPECT_TRUE(sVersion1a    != sVersion1_1a);
    EXPECT_TRUE(sVersion1b    != sVersion1_1a);
    EXPECT_TRUE(sVersion1_1   != sVersion1_1a);
    EXPECT_FALSE(sVersion1_1a != sVersion1_1a);
    EXPECT_TRUE(sVersion1_2   != sVersion1_1a);
    EXPECT_TRUE(sVersion2     != sVersion1_1a);

    EXPECT_TRUE(sVersion0    != sVersion1_2);
    EXPECT_TRUE(sVersion0_9  != sVersion1_2);
    EXPECT_TRUE(sVersion1    != sVersion1_2);
    EXPECT_TRUE(sVersion1a   != sVersion1_2);
    EXPECT_TRUE(sVersion1b   != sVersion1_2);
    EXPECT_TRUE(sVersion1_1  != sVersion1_2);
    EXPECT_TRUE(sVersion1_1a != sVersion1_2);
    EXPECT_FALSE(sVersion1_2 != sVersion1_2);
    EXPECT_TRUE(sVersion2    != sVersion1_2);

    EXPECT_TRUE(sVersion0    != sVersion2);
    EXPECT_TRUE(sVersion0_9  != sVersion2);
    EXPECT_TRUE(sVersion1    != sVersion2);
    EXPECT_TRUE(sVersion1a   != sVersion2);
    EXPECT_TRUE(sVersion1b   != sVersion2);
    EXPECT_TRUE(sVersion1_1  != sVersion2);
    EXPECT_TRUE(sVersion1_1a != sVersion2);
    EXPECT_TRUE(sVersion1_2  != sVersion2);
    EXPECT_FALSE(sVersion2   != sVersion2);
}

TEST_F(CPackageVersionTest, PackageVersionSmallerThan)
{
    EXPECT_FALSE(sVersion0    < sVersion0);
    EXPECT_FALSE(sVersion0_9  < sVersion0);
    EXPECT_FALSE(sVersion1    < sVersion0);
    EXPECT_FALSE(sVersion1a   < sVersion0);
    EXPECT_FALSE(sVersion1b   < sVersion0);
    EXPECT_FALSE(sVersion1_1  < sVersion0);
    EXPECT_FALSE(sVersion1_1a < sVersion0);
    EXPECT_FALSE(sVersion1_2  < sVersion0);
    EXPECT_FALSE(sVersion2    < sVersion0);

    EXPECT_TRUE(sVersion0     < sVersion0_9);
    EXPECT_FALSE(sVersion0_9  < sVersion0_9);
    EXPECT_FALSE(sVersion1    < sVersion0_9);
    EXPECT_FALSE(sVersion1a   < sVersion0_9);
    EXPECT_FALSE(sVersion1b   < sVersion0_9);
    EXPECT_FALSE(sVersion1_1  < sVersion0_9);
    EXPECT_FALSE(sVersion1_1a < sVersion0_9);
    EXPECT_FALSE(sVersion1_2  < sVersion0_9);
    EXPECT_FALSE(sVersion2    < sVersion0_9);

    EXPECT_TRUE(sVersion0     < sVersion1);
    EXPECT_TRUE(sVersion0_9   < sVersion1);
    EXPECT_FALSE(sVersion1    < sVersion1);
    EXPECT_FALSE(sVersion1a   < sVersion1);
    EXPECT_FALSE(sVersion1b   < sVersion1);
    EXPECT_FALSE(sVersion1_1  < sVersion1);
    EXPECT_FALSE(sVersion1_1a < sVersion1);
    EXPECT_FALSE(sVersion1_2  < sVersion1);
    EXPECT_FALSE(sVersion2    < sVersion1);

    EXPECT_TRUE(sVersion0     < sVersion1a);
    EXPECT_TRUE(sVersion0_9   < sVersion1a);
    EXPECT_TRUE(sVersion1     < sVersion1a);
    EXPECT_FALSE(sVersion1a   < sVersion1a);
    EXPECT_FALSE(sVersion1b   < sVersion1a);
    EXPECT_FALSE(sVersion1_1  < sVersion1a);
    EXPECT_FALSE(sVersion1_1a < sVersion1a);
    EXPECT_FALSE(sVersion1_2  < sVersion1a);
    EXPECT_FALSE(sVersion2    < sVersion1a);

    EXPECT_TRUE(sVersion0     < sVersion1b);
    EXPECT_TRUE(sVersion0_9   < sVersion1b);
    EXPECT_TRUE(sVersion1     < sVersion1b);
    EXPECT_TRUE(sVersion1a    < sVersion1b);
    EXPECT_FALSE(sVersion1b   < sVersion1b);
    EXPECT_FALSE(sVersion1_1  < sVersion1b);
    EXPECT_FALSE(sVersion1_1a < sVersion1b);
    EXPECT_FALSE(sVersion1_2  < sVersion1b);
    EXPECT_FALSE(sVersion2    < sVersion1b);

    EXPECT_TRUE(sVersion0     < sVersion1_1);
    EXPECT_TRUE(sVersion0_9   < sVersion1_1);
    EXPECT_TRUE(sVersion1     < sVersion1_1);
    EXPECT_TRUE(sVersion1a    < sVersion1_1);
    EXPECT_TRUE(sVersion1b    < sVersion1_1);
    EXPECT_FALSE(sVersion1_1  < sVersion1_1);
    EXPECT_FALSE(sVersion1_1a < sVersion1_1);
    EXPECT_FALSE(sVersion1_2  < sVersion1_1);
    EXPECT_FALSE(sVersion2    < sVersion1_1);

    EXPECT_TRUE(sVersion0     < sVersion1_1a);
    EXPECT_TRUE(sVersion0_9   < sVersion1_1a);
    EXPECT_TRUE(sVersion1     < sVersion1_1a);
    EXPECT_TRUE(sVersion1a    < sVersion1_1a);
    EXPECT_TRUE(sVersion1b    < sVersion1_1a);
    EXPECT_TRUE(sVersion1_1   < sVersion1_1a);
    EXPECT_FALSE(sVersion1_1a < sVersion1_1a);
    EXPECT_FALSE(sVersion1_2  < sVersion1_1a);
    EXPECT_FALSE(sVersion2    < sVersion1_1a);

    EXPECT_TRUE(sVersion0    < sVersion1_2);
    EXPECT_TRUE(sVersion0_9  < sVersion1_2);
    EXPECT_TRUE(sVersion1    < sVersion1_2);
    EXPECT_TRUE(sVersion1a   < sVersion1_2);
    EXPECT_TRUE(sVersion1b   < sVersion1_2);
    EXPECT_TRUE(sVersion1_1  < sVersion1_2);
    EXPECT_TRUE(sVersion1_1a < sVersion1_2);
    EXPECT_FALSE(sVersion1_2 < sVersion1_2);
    EXPECT_FALSE(sVersion2   < sVersion1_2);

    EXPECT_TRUE(sVersion0    < sVersion2);
    EXPECT_TRUE(sVersion0_9  < sVersion2);
    EXPECT_TRUE(sVersion1    < sVersion2);
    EXPECT_TRUE(sVersion1a   < sVersion2);
    EXPECT_TRUE(sVersion1b   < sVersion2);
    EXPECT_TRUE(sVersion1_1  < sVersion2);
    EXPECT_TRUE(sVersion1_1a < sVersion2);
    EXPECT_TRUE(sVersion1_2  < sVersion2);
    EXPECT_FALSE(sVersion2   < sVersion2);
}

TEST_F(CPackageVersionTest, PackageVersionSmallerThanOrEqual)
{
    EXPECT_TRUE(sVersion0     <= sVersion0);
    EXPECT_FALSE(sVersion0_9  <= sVersion0);
    EXPECT_FALSE(sVersion1    <= sVersion0);
    EXPECT_FALSE(sVersion1a   <= sVersion0);
    EXPECT_FALSE(sVersion1b   <= sVersion0);
    EXPECT_FALSE(sVersion1_1  <= sVersion0);
    EXPECT_FALSE(sVersion1_1a <= sVersion0);
    EXPECT_FALSE(sVersion1_2  <= sVersion0);
    EXPECT_FALSE(sVersion2    <= sVersion0);

    EXPECT_TRUE(sVersion0     <= sVersion0_9);
    EXPECT_TRUE(sVersion0_9   <= sVersion0_9);
    EXPECT_FALSE(sVersion1    <= sVersion0_9);
    EXPECT_FALSE(sVersion1a   <= sVersion0_9);
    EXPECT_FALSE(sVersion1b   <= sVersion0_9);
    EXPECT_FALSE(sVersion1_1  <= sVersion0_9);
    EXPECT_FALSE(sVersion1_1a <= sVersion0_9);
    EXPECT_FALSE(sVersion1_2  <= sVersion0_9);
    EXPECT_FALSE(sVersion2    <= sVersion0_9);

    EXPECT_TRUE(sVersion0     <= sVersion1);
    EXPECT_TRUE(sVersion0_9   <= sVersion1);
    EXPECT_TRUE(sVersion1     <= sVersion1);
    EXPECT_FALSE(sVersion1a   <= sVersion1);
    EXPECT_FALSE(sVersion1b   <= sVersion1);
    EXPECT_FALSE(sVersion1_1  <= sVersion1);
    EXPECT_FALSE(sVersion1_1a <= sVersion1);
    EXPECT_FALSE(sVersion1_2  <= sVersion1);
    EXPECT_FALSE(sVersion2    <= sVersion1);

    EXPECT_TRUE(sVersion0     <= sVersion1a);
    EXPECT_TRUE(sVersion0_9   <= sVersion1a);
    EXPECT_TRUE(sVersion1     <= sVersion1a);
    EXPECT_TRUE(sVersion1a    <= sVersion1a);
    EXPECT_FALSE(sVersion1b   <= sVersion1a);
    EXPECT_FALSE(sVersion1_1  <= sVersion1a);
    EXPECT_FALSE(sVersion1_1a <= sVersion1a);
    EXPECT_FALSE(sVersion1_2  <= sVersion1a);
    EXPECT_FALSE(sVersion2    <= sVersion1a);

    EXPECT_TRUE(sVersion0     <= sVersion1b);
    EXPECT_TRUE(sVersion0_9   <= sVersion1b);
    EXPECT_TRUE(sVersion1     <= sVersion1b);
    EXPECT_TRUE(sVersion1a    <= sVersion1b);
    EXPECT_TRUE(sVersion1b    <= sVersion1b);
    EXPECT_FALSE(sVersion1_1  <= sVersion1b);
    EXPECT_FALSE(sVersion1_1a <= sVersion1b);
    EXPECT_FALSE(sVersion1_2  <= sVersion1b);
    EXPECT_FALSE(sVersion2    <= sVersion1b);

    EXPECT_TRUE(sVersion0     <= sVersion1_1);
    EXPECT_TRUE(sVersion0_9   <= sVersion1_1);
    EXPECT_TRUE(sVersion1     <= sVersion1_1);
    EXPECT_TRUE(sVersion1a    <= sVersion1_1);
    EXPECT_TRUE(sVersion1b    <= sVersion1_1);
    EXPECT_TRUE(sVersion1_1   <= sVersion1_1);
    EXPECT_FALSE(sVersion1_1a <= sVersion1_1);
    EXPECT_FALSE(sVersion1_2  <= sVersion1_1);
    EXPECT_FALSE(sVersion2    <= sVersion1_1);

    EXPECT_TRUE(sVersion0    <= sVersion1_1a);
    EXPECT_TRUE(sVersion0_9  <= sVersion1_1a);
    EXPECT_TRUE(sVersion1    <= sVersion1_1a);
    EXPECT_TRUE(sVersion1a   <= sVersion1_1a);
    EXPECT_TRUE(sVersion1b   <= sVersion1_1a);
    EXPECT_TRUE(sVersion1_1  <= sVersion1_1a);
    EXPECT_TRUE(sVersion1_1a <= sVersion1_1a);
    EXPECT_FALSE(sVersion1_2 <= sVersion1_1a);
    EXPECT_FALSE(sVersion2   <= sVersion1_1a);

    EXPECT_TRUE(sVersion0    <= sVersion1_2);
    EXPECT_TRUE(sVersion0_9  <= sVersion1_2);
    EXPECT_TRUE(sVersion1    <= sVersion1_2);
    EXPECT_TRUE(sVersion1a   <= sVersion1_2);
    EXPECT_TRUE(sVersion1b   <= sVersion1_2);
    EXPECT_TRUE(sVersion1_1  <= sVersion1_2);
    EXPECT_TRUE(sVersion1_1a <= sVersion1_2);
    EXPECT_TRUE(sVersion1_2  <= sVersion1_2);
    EXPECT_FALSE(sVersion2   <= sVersion1_2);

    EXPECT_TRUE(sVersion0    <= sVersion2);
    EXPECT_TRUE(sVersion0_9  <= sVersion2);
    EXPECT_TRUE(sVersion1    <= sVersion2);
    EXPECT_TRUE(sVersion1a   <= sVersion2);
    EXPECT_TRUE(sVersion1b   <= sVersion2);
    EXPECT_TRUE(sVersion1_1  <= sVersion2);
    EXPECT_TRUE(sVersion1_1a <= sVersion2);
    EXPECT_TRUE(sVersion1_2  <= sVersion2);
    EXPECT_TRUE(sVersion2    <= sVersion2);
}

