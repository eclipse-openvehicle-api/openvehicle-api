#include "../../include/gtest_custom.h"
#include "../../../global/ascformat/ascreader.cpp"
#include <iostream>
#include <fstream>
#include "../../../global/exec_dir_helper.h"

TEST(CAscReaderTest, ReadASCFile)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_TRUE(reader.IsEOF());
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
}

TEST(CAscReaderTest, SingleSampleNavigation)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // First value
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007096);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0xb6ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 5ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x06);
    EXPECT_EQ(prSample.first.rguiData[1], 0x06);
    EXPECT_EQ(prSample.first.rguiData[2], 0x80);
    EXPECT_EQ(prSample.first.rguiData[3], 0x00);
    EXPECT_EQ(prSample.first.rguiData[4], 0xc8);
    EXPECT_EQ(prSample.first.rguiData[5], 0x00);

    // Next value
    ++reader;
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007123);
    EXPECT_EQ(prSample.first.uiChannel, 2ul);
    EXPECT_EQ(prSample.first.uiId, 0x85ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 8ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x00);
    EXPECT_EQ(prSample.first.rguiData[1], 0x00);
    EXPECT_EQ(prSample.first.rguiData[2], 0x00);
    EXPECT_EQ(prSample.first.rguiData[3], 0x00);
    EXPECT_EQ(prSample.first.rguiData[4], 0xff);
    EXPECT_EQ(prSample.first.rguiData[5], 0xff);
    EXPECT_EQ(prSample.first.rguiData[6], 0xf3);
    EXPECT_EQ(prSample.first.rguiData[7], 0x1d);
    EXPECT_EQ(prSample.first.rguiData[8], 0x00);

    // Next value
    ++reader;
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007333);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0x19eul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 8ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x00);
    EXPECT_EQ(prSample.first.rguiData[1], 0xec);
    EXPECT_EQ(prSample.first.rguiData[2], 0xef);
    EXPECT_EQ(prSample.first.rguiData[3], 0xfc);
    EXPECT_EQ(prSample.first.rguiData[4], 0xff);
    EXPECT_EQ(prSample.first.rguiData[5], 0x0c);
    EXPECT_EQ(prSample.first.rguiData[6], 0x00);
    EXPECT_EQ(prSample.first.rguiData[7], 0x85);
    EXPECT_EQ(prSample.first.rguiData[8], 0x00);

    // Previous value
    --reader;
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007123);
    EXPECT_EQ(prSample.first.uiChannel, 2ul);
    EXPECT_EQ(prSample.first.uiId, 0x85ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 8ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x00);
    EXPECT_EQ(prSample.first.rguiData[1], 0x00);
    EXPECT_EQ(prSample.first.rguiData[2], 0x00);
    EXPECT_EQ(prSample.first.rguiData[3], 0x00);
    EXPECT_EQ(prSample.first.rguiData[4], 0xff);
    EXPECT_EQ(prSample.first.rguiData[5], 0xff);
    EXPECT_EQ(prSample.first.rguiData[6], 0xf3);
    EXPECT_EQ(prSample.first.rguiData[7], 0x1d);
    EXPECT_EQ(prSample.first.rguiData[8], 0x00);

    // Previous value
    --reader;
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007096);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0xb6ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 5ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x06);
    EXPECT_EQ(prSample.first.rguiData[1], 0x06);
    EXPECT_EQ(prSample.first.rguiData[2], 0x80);
    EXPECT_EQ(prSample.first.rguiData[3], 0x00);
    EXPECT_EQ(prSample.first.rguiData[4], 0xc8);
    EXPECT_EQ(prSample.first.rguiData[5], 0x00);

    // Jump to end
    reader.JumpEnd();
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_TRUE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_FALSE(prSample.second);

    // Jump to begin
    reader.JumpBegin();
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.007096);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0xb6ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 5ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x06);
    EXPECT_EQ(prSample.first.rguiData[1], 0x06);
    EXPECT_EQ(prSample.first.rguiData[2], 0x80);
    EXPECT_EQ(prSample.first.rguiData[3], 0x00);
    EXPECT_EQ(prSample.first.rguiData[4], 0xc8);
    EXPECT_EQ(prSample.first.rguiData[5], 0x00);
}

TEST(CAscReaderTest, NormalPlayback)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Do measurement
    asc::SCanMessage sMsg{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg = rsMsg; }, false);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    reader.StopPlayback();

    // Last sent message is the one before the current message.
    --reader;
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg.rguiData[8]);

    // Time deviation
    std::cout << "The current timestamp is " << sMsg.dTimestamp << " (should be somewhat more than 250ms)" << std::endl;
}

TEST(CAscReaderTest, PlaybackRestart)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Do first measurement
    asc::SCanMessage sMsg1{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg1 = rsMsg; }, false);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    reader.StopPlayback();
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());

    // Last sent message is the one before the current message.
    --reader;
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg1.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg1.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg1.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg1.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg1.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg1.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg1.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg1.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg1.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg1.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg1.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg1.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg1.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg1.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg1.rguiData[8]);

    // Do second measurement
    asc::SCanMessage sMsg2{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg2 = rsMsg; }, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    reader.StopPlayback();

    // Last sent message is the one before the current message.
    --reader;
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg2.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg2.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg2.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg2.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg2.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg2.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg2.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg2.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg2.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg2.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg2.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg2.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg2.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg2.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg2.rguiData[8]);

    // The time of message 1 should be less than the time of message 2.
    EXPECT_LT(sMsg1.dTimestamp, sMsg2.dTimestamp);
}

TEST(CAscReaderTest, PlaybackReset)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Do first measurement
    asc::SCanMessage sMsg1{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg1 = rsMsg; }, false);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    reader.StopPlayback();
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());

    // Last sent message is the one before the current message.
    --reader;
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg1.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg1.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg1.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg1.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg1.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg1.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg1.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg1.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg1.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg1.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg1.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg1.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg1.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg1.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg1.rguiData[8]);

    // Do second measurement
    asc::SCanMessage sMsg2{};
    reader.ResetPlayback();
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg2 = rsMsg; }, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(125));
    reader.StopPlayback();

    // Last sent message is the one before the current message.
    --reader;
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg2.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg2.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg2.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg2.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg2.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg2.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg2.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg2.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg2.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg2.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg2.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg2.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg2.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg2.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg2.rguiData[8]);

    // The time of message 1 should be more than the time of message 2.
    EXPECT_GT(sMsg1.dTimestamp, sMsg2.dTimestamp);
}

TEST(CAscReaderTest, FinishPlayback)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Do measurement
    reader.JumpEnd();
    for (size_t n = 0; n < 150; n++)
        --reader;
    asc::SCanMessage sMsg{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg = rsMsg; }, false);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    EXPECT_FALSE(reader.PlaybackRunning());
    reader.StopPlayback(); // Just in case...
    EXPECT_TRUE(reader.IsEOF());

    // Last sent message is the one before the current message.
    --reader;
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsg.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsg.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsg.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsg.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsg.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsg.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsg.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsg.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsg.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsg.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsg.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsg.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsg.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsg.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsg.rguiData[8]);
}

TEST(CAscReaderTest, RepeatPlayback)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_test.asc"));

    // Do a measurement startin at the end
    reader.JumpEnd();
    for (size_t n = 0; n < 20; n++)
        --reader;
    asc::SCanMessage sMsgStart = reader.Get().first;
    asc::SCanMessage sMsgCurrent{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsgCurrent = rsMsg; }, true);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    reader.StopPlayback();

    // Last sent message is the one before the current message.
    --reader;
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, sMsgCurrent.dTimestamp);
    EXPECT_EQ(prSample.first.uiChannel, sMsgCurrent.uiChannel);
    EXPECT_EQ(prSample.first.uiId, sMsgCurrent.uiId);
    EXPECT_EQ(prSample.first.bExtended, sMsgCurrent.bExtended);
    EXPECT_EQ(prSample.first.eDirection, sMsgCurrent.eDirection);
    EXPECT_EQ(prSample.first.uiLength, sMsgCurrent.uiLength);
    EXPECT_EQ(prSample.first.rguiData[0], sMsgCurrent.rguiData[0]);
    EXPECT_EQ(prSample.first.rguiData[1], sMsgCurrent.rguiData[1]);
    EXPECT_EQ(prSample.first.rguiData[2], sMsgCurrent.rguiData[2]);
    EXPECT_EQ(prSample.first.rguiData[3], sMsgCurrent.rguiData[3]);
    EXPECT_EQ(prSample.first.rguiData[4], sMsgCurrent.rguiData[4]);
    EXPECT_EQ(prSample.first.rguiData[5], sMsgCurrent.rguiData[5]);
    EXPECT_EQ(prSample.first.rguiData[6], sMsgCurrent.rguiData[6]);
    EXPECT_EQ(prSample.first.rguiData[7], sMsgCurrent.rguiData[7]);
    EXPECT_EQ(prSample.first.rguiData[8], sMsgCurrent.rguiData[8]);

    // The time of start message should be more than the time of the current message.
    EXPECT_GT(sMsgStart.dTimestamp, sMsgCurrent.dTimestamp);
}

TEST(CAscReaderTest, RepeatPlaybackCheckLoopExecution)
{
    asc::CAscReader readerOneLoop;
    EXPECT_TRUE(readerOneLoop.Read(GetExecDirectory() / "asc_reader_timing_test.asc"));

    asc::SCanMessage sMsgOneLoop{};
    readerOneLoop.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsgOneLoop = rsMsg; }, false);
    EXPECT_TRUE(readerOneLoop.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    readerOneLoop.StopPlayback();
    EXPECT_TRUE(readerOneLoop.GetLoopCount() == 1);

    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_timing_test.asc"));

    asc::SCanMessage sMsg{};
    reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { sMsg = rsMsg; }, true);
    EXPECT_TRUE(reader.PlaybackRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    reader.StopPlayback();

    // Second loop must run in the same speed, therfore LoopCount = 2, not mmore
    EXPECT_TRUE((readerOneLoop.GetLoopCount() + 1) == reader.GetLoopCount());
}

TEST(CAscReaderTest, ExtendedId)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_ext_id_test.asc"));

    // First value
    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.590000);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0x539ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 1ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x03);
    EXPECT_EQ(prSample.first.rguiData[1], 0x00);

    // Next value
    ++reader;
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 0.619000);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0x71ul);
    EXPECT_TRUE(prSample.first.bExtended);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 1ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x42);
    EXPECT_EQ(prSample.first.rguiData[1], 0x00);
}

TEST(CAscReaderTest, CAN_FD)
{
    asc::CAscReader reader;
    EXPECT_TRUE(reader.Read(GetExecDirectory() / "asc_reader_canfd_test.asc"));

    EXPECT_TRUE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());

    // Skip two values
    ++reader;
    ++reader;

    // third value
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    auto prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 3.36716);
    EXPECT_EQ(prSample.first.uiChannel, 1ul);
    EXPECT_EQ(prSample.first.uiId, 0xB4323ul);
    EXPECT_TRUE(prSample.first.bExtended);
    EXPECT_FALSE(prSample.first.bCanFd);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 8ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x73);
    EXPECT_EQ(prSample.first.rguiData[1], 0xe5);
    EXPECT_EQ(prSample.first.rguiData[2], 0xe9);
    EXPECT_EQ(prSample.first.rguiData[3], 0x24);
    EXPECT_EQ(prSample.first.rguiData[4], 0xe2);
    EXPECT_EQ(prSample.first.rguiData[5], 0x00);
    EXPECT_EQ(prSample.first.rguiData[6], 0x00);
    EXPECT_EQ(prSample.first.rguiData[7], 0x00);
    EXPECT_EQ(prSample.first.rguiData[8], 0x00);

    // Next value
    ++reader;
    EXPECT_FALSE(reader.IsBOF());
    EXPECT_FALSE(reader.IsEOF());
    prSample = reader.Get();
    EXPECT_TRUE(prSample.second);
    EXPECT_EQ(prSample.first.dTimestamp, 3.36722);
    EXPECT_EQ(prSample.first.uiChannel, 15ul);
    EXPECT_EQ(prSample.first.uiId, 0x86ul);
    EXPECT_FALSE(prSample.first.bExtended);
    EXPECT_TRUE(prSample.first.bCanFd);
    EXPECT_EQ(prSample.first.eDirection, asc::SCanMessage::EDirection::rx);
    EXPECT_EQ(prSample.first.uiLength, 8ul);
    EXPECT_EQ(prSample.first.rguiData[0], 0x48);
    EXPECT_EQ(prSample.first.rguiData[1], 0x75);
    EXPECT_EQ(prSample.first.rguiData[2], 0x6d);
    EXPECT_EQ(prSample.first.rguiData[3], 0x61);
    EXPECT_EQ(prSample.first.rguiData[4], 0x6e);
    EXPECT_EQ(prSample.first.rguiData[5], 0x69);
    EXPECT_EQ(prSample.first.rguiData[6], 0x73);
    EXPECT_EQ(prSample.first.rguiData[7], 0x6d);
    EXPECT_EQ(prSample.first.rguiData[8], 0x00);
}

