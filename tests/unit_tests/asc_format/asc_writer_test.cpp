#include "../../include/gtest_custom.h"
#include "../../../global/ascformat/ascwriter.cpp"
#include <iostream>
#include <fstream>
#include "../../../global/exec_dir_helper.h"
#include "../../include/sdv_test_macro.h"

TEST(CAscWriterTest, AddSamplesDirect)
{
    asc::CAscReader readerGroundThruth;
    EXPECT_TRUE(readerGroundThruth.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Add 20 samples
    asc::CAscWriter writer;
    for (size_t n = 0; n < 20; n++)
    {
        auto prSample = readerGroundThruth.Get();
        EXPECT_TRUE(prSample.second);
        writer.AddSample(prSample.first);
        ++readerGroundThruth;
    }

    // Write the samples
    writer.Write(GetExecDirectory() / "asc_writer_test.asc");

    // Read the samples
    asc::CAscReader readerGenerate;
    EXPECT_TRUE(readerGenerate.Read(GetExecDirectory() / "asc_writer_test.asc"));

    // Check whether the 20 samples correspond to the original samples
    readerGroundThruth.JumpBegin();
    while (!readerGenerate.IsEOF())
    {
        auto prSampleGroundTruth = readerGroundThruth.Get();
        EXPECT_TRUE(prSampleGroundTruth.second);
        auto prSampleGenerated = readerGenerate.Get();

        EXPECT_EQ(prSampleGroundTruth.first.dTimestamp, prSampleGenerated.first.dTimestamp);
        EXPECT_EQ(prSampleGroundTruth.first.uiChannel, prSampleGenerated.first.uiChannel);
        EXPECT_EQ(prSampleGroundTruth.first.uiId, prSampleGenerated.first.uiId);
        EXPECT_EQ(prSampleGroundTruth.first.bExtended, prSampleGenerated.first.bExtended);
        EXPECT_EQ(prSampleGroundTruth.first.eDirection, prSampleGenerated.first.eDirection);
        EXPECT_EQ(prSampleGroundTruth.first.uiLength, prSampleGenerated.first.uiLength);
        for (size_t nIndex = 0; nIndex < 64; nIndex++)
            EXPECT_EQ(prSampleGroundTruth.first.rguiData[nIndex], prSampleGenerated.first.rguiData[nIndex]);

        ++readerGroundThruth;
        ++readerGenerate;
    }
}

TEST(CAscWriterTest, AddTimedSamples)
{
    asc::CAscReader readerGroundThruth;
    EXPECT_TRUE(readerGroundThruth.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Skip 100 samples
    for (size_t n = 0; n < 100; n++)
        ++readerGroundThruth;
    EXPECT_FALSE(readerGroundThruth.IsBOF());
    EXPECT_FALSE(readerGroundThruth.IsEOF());

    // Start the timer
    asc::CAscWriter writer;
    writer.StartTimer();

    // Add 250ms of samples
    readerGroundThruth.StartPlayback([&](const asc::SCanMessage& rsSample)
        {
            asc::SCanMessage sSampleCopy(rsSample);
            sSampleCopy.dTimestamp = 0.000;
            writer.AddSample(sSampleCopy);
        }, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    readerGroundThruth.StopPlayback();

    // Write the samples
    writer.Write(GetExecDirectory() / "asc_writer_test.asc");

    // Read the samples
    asc::CAscReader readerGenerate;
    EXPECT_TRUE(readerGenerate.Read(GetExecDirectory() / "asc_writer_test.asc"));

    // Start at the 101st sample
    readerGroundThruth.JumpBegin();
    for (size_t n = 0; n < 100; n++)
        ++readerGroundThruth;

    auto fnRound = [](double dTimestamp) { return std::round(dTimestamp * 1000.0) / 1000.0; };

    // Check whether the samples correspond to the original samples, except for the generated timestamp
    double dLastTSGroundThruth = fnRound(readerGroundThruth.Get().first.dTimestamp);
    double dLastTSGenerated = fnRound(readerGenerate.Get().first.dTimestamp);
    ++readerGroundThruth;
    ++readerGenerate;

    while (!readerGenerate.IsEOF())
    {
        auto prSampleGroundTruth = readerGroundThruth.Get();
        EXPECT_TRUE(prSampleGroundTruth.second);
        auto prSampleGenerated = readerGenerate.Get();

        // Delta timestamp should not differ more than +/- 2 ms.
        double dDeltaTSGroundThruth = fnRound(readerGroundThruth.Get().first.dTimestamp) - dLastTSGroundThruth;
        dLastTSGroundThruth = fnRound(readerGroundThruth.Get().first.dTimestamp);
        double dDeltaTSGenerated = fnRound(readerGenerate.Get().first.dTimestamp) - dLastTSGenerated;
        dLastTSGenerated = fnRound(readerGenerate.Get().first.dTimestamp);
        if (dDeltaTSGenerated < dDeltaTSGroundThruth - 0.002)
        {
            if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                SDV_EXPECT_GE_WARN(dDeltaTSGenerated, dDeltaTSGroundThruth - 0.002, sdv_test::WARNING_REDUCED);
            else
                SDV_EXPECT_GE_WARN(dDeltaTSGenerated, dDeltaTSGroundThruth - 0.002, sdv_test::WARNING_ENABLED);
        }
        if (dDeltaTSGenerated > dDeltaTSGroundThruth + 0.002)
        {
            if(SDV_IS_RUNNING_TESTS_WITH_CMAKE_BUILD)
                SDV_EXPECT_LE_WARN(dDeltaTSGenerated, dDeltaTSGroundThruth + 0.002, sdv_test::WARNING_REDUCED);
            else
                SDV_EXPECT_LE_WARN(dDeltaTSGenerated, dDeltaTSGroundThruth + 0.002, sdv_test::WARNING_ENABLED);
        }
            std::cout << "TIMING: Expected generated timestamp <= ground truth + 2ms (generated=" << dDeltaTSGenerated << "ms, ground truth=" << dDeltaTSGroundThruth << ")..." << std::endl;
        EXPECT_EQ(prSampleGroundTruth.first.uiChannel, prSampleGenerated.first.uiChannel);
        EXPECT_EQ(prSampleGroundTruth.first.uiId, prSampleGenerated.first.uiId);
        EXPECT_EQ(prSampleGroundTruth.first.bExtended, prSampleGenerated.first.bExtended);
        EXPECT_EQ(prSampleGroundTruth.first.eDirection, prSampleGenerated.first.eDirection);
        EXPECT_EQ(prSampleGroundTruth.first.uiLength, prSampleGenerated.first.uiLength);
        for (size_t nIndex = 0; nIndex < 64; nIndex++)
            EXPECT_EQ(prSampleGroundTruth.first.rguiData[nIndex], prSampleGenerated.first.rguiData[nIndex]);

        ++readerGroundThruth;
        ++readerGenerate;
    }
}

TEST(CAscWriterTest, ExtendedID)
{
    asc::CAscReader readerGroundThruth;
    SDV_EXPECT_TRUE(readerGroundThruth.Read(GetExecDirectory() / "asc_reader_ext_id_test.asc"), sdv_test::WARNING_REDUCED);

    // Add all samples
    asc::CAscWriter writer;
    while (!readerGroundThruth.IsEOF())
    {
        auto prSample = readerGroundThruth.Get();
        EXPECT_TRUE(prSample.second);
        writer.AddSample(prSample.first);
        ++readerGroundThruth;
    }

    // Write the samples
    writer.Write(GetExecDirectory() / "asc_writer_test.asc");

    // Read the samples
    asc::CAscReader readerGenerate;
    EXPECT_TRUE(readerGenerate.Read(GetExecDirectory() / "asc_writer_test.asc"));

    // Check whether the samples correspond to the original samples
    readerGroundThruth.JumpBegin();
    while (!readerGenerate.IsEOF())
    {
        auto prSampleGroundTruth = readerGroundThruth.Get();
        EXPECT_TRUE(prSampleGroundTruth.second);
        auto prSampleGenerated = readerGenerate.Get();

        EXPECT_EQ(prSampleGroundTruth.first.dTimestamp, prSampleGenerated.first.dTimestamp);
        EXPECT_EQ(prSampleGroundTruth.first.uiChannel, prSampleGenerated.first.uiChannel);
        EXPECT_EQ(prSampleGroundTruth.first.uiId, prSampleGenerated.first.uiId);
        EXPECT_EQ(prSampleGroundTruth.first.bExtended, prSampleGenerated.first.bExtended);
        EXPECT_EQ(prSampleGroundTruth.first.eDirection, prSampleGenerated.first.eDirection);
        EXPECT_EQ(prSampleGroundTruth.first.uiLength, prSampleGenerated.first.uiLength);
        for (size_t nIndex = 0; nIndex < 64; nIndex++)
            EXPECT_EQ(prSampleGroundTruth.first.rguiData[nIndex], prSampleGenerated.first.rguiData[nIndex]);

        ++readerGroundThruth;
        ++readerGenerate;
    }
}

TEST(CAscWriterTest, CAN_FD)
{
    asc::CAscReader readerGroundThruth;
    EXPECT_TRUE(readerGroundThruth.Read(GetExecDirectory() / "asc_reader_canfd_test.asc"));

    // Add all samples
    asc::CAscWriter writer;
    while (!readerGroundThruth.IsEOF())
    {
        auto prSample = readerGroundThruth.Get();
        EXPECT_TRUE(prSample.second);
        writer.AddSample(prSample.first);
        ++readerGroundThruth;
    }

    // Write the samples
    writer.Write(GetExecDirectory() / "asc_writer_test.asc");

    // Read the samples
    asc::CAscReader readerGenerate;
    EXPECT_TRUE(readerGenerate.Read(GetExecDirectory() / "asc_writer_test.asc"));

    // Check whether the samples correspond to the original samples
    readerGroundThruth.JumpBegin();
    while (!readerGenerate.IsEOF())
    {
        auto prSampleGroundTruth = readerGroundThruth.Get();
        EXPECT_TRUE(prSampleGroundTruth.second);
        auto prSampleGenerated = readerGenerate.Get();

        EXPECT_EQ(prSampleGroundTruth.first.dTimestamp, prSampleGenerated.first.dTimestamp);
        EXPECT_EQ(prSampleGroundTruth.first.uiChannel, prSampleGenerated.first.uiChannel);
        EXPECT_EQ(prSampleGroundTruth.first.uiId, prSampleGenerated.first.uiId);
        EXPECT_EQ(prSampleGroundTruth.first.bExtended, prSampleGenerated.first.bExtended);
        EXPECT_EQ(prSampleGroundTruth.first.bCanFd, prSampleGenerated.first.bCanFd);
        EXPECT_EQ(prSampleGroundTruth.first.eDirection, prSampleGenerated.first.eDirection);
        EXPECT_EQ(prSampleGroundTruth.first.uiLength, prSampleGenerated.first.uiLength);
        for (size_t nIndex = 0; nIndex < 64; nIndex++)
            EXPECT_EQ(prSampleGroundTruth.first.rguiData[nIndex], prSampleGenerated.first.rguiData[nIndex]);

        ++readerGroundThruth;
        ++readerGenerate;
    }
}

