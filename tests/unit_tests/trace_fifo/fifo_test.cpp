#include <gtest/gtest.h>
#include "../../../global/localmemmgr.h"
#include "../../../global/tracefifo/trace_fifo.h"
#include <vector>
#include <list>
#include <deque>

TEST(TraceFifoTest, Connect_Disconnect)
{
    CTraceFifoWriter fifo(9999);
    EXPECT_FALSE(fifo.IsOpened());
    EXPECT_TRUE(fifo.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifo.IsOpened());
    fifo.Close();
    EXPECT_FALSE(fifo.IsOpened());
}

TEST(TraceFifoTest, Connect_Channel)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader(9999);
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());

    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());

    fifoWriter.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());
    fifoReader.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());
}

TEST(TraceFifoTest, Connect_Channel_Reverse)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader(9999);
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());

    EXPECT_TRUE(fifoReader.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoReader.IsOpened());
    EXPECT_TRUE(fifoWriter.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());

    fifoReader.Close();
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());
    fifoWriter.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());
}

TEST(TraceFifoTest, DISABLED_Connect_MultiWriter)
{
    CTraceFifoWriter fifoWriter1(9999);
    CTraceFifoWriter fifoWriter2(9999);
    CTraceFifoReader fifoReader(9999);
    EXPECT_FALSE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());

    EXPECT_TRUE(fifoWriter1.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.Open());
    EXPECT_TRUE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());

    fifoWriter1.Close();
    EXPECT_FALSE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());
    fifoWriter2.Close();
    EXPECT_FALSE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());
    fifoReader.Close();
    EXPECT_FALSE(fifoWriter1.IsOpened());
    EXPECT_FALSE(fifoWriter2.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());
}

TEST(TraceFifoTest, Connect_MultiReader)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader1(9999);
    CTraceFifoReader fifoReader2(9999);
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader1.IsOpened());
    EXPECT_FALSE(fifoReader2.IsOpened());

    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader1.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader1.IsOpened());
    EXPECT_TRUE(fifoReader2.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader1.IsOpened());
    EXPECT_TRUE(fifoReader2.IsOpened());

    fifoWriter.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader1.IsOpened());
    EXPECT_TRUE(fifoReader2.IsOpened());
    fifoReader1.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader1.IsOpened());
    EXPECT_TRUE(fifoReader2.IsOpened());
    fifoReader2.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader1.IsOpened());
    EXPECT_FALSE(fifoReader2.IsOpened());
}

TEST(TraceFifoTest, Takeover_Size)
{
    CTraceFifoWriter fifoWriter(9999, 6000);
    CTraceFifoReader fifoReader(9999, 2048);
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());

    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_NE(fifoWriter.GetViewSize(), 0);
    EXPECT_EQ(fifoReader.GetViewSize(), 0);
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());
    EXPECT_EQ(fifoReader.GetViewSize(), fifoWriter.GetViewSize());

    fifoWriter.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());
    fifoReader.Close();
    EXPECT_FALSE(fifoWriter.IsOpened());
    EXPECT_FALSE(fifoReader.IsOpened());
}

TEST(TraceFifoTest, Open_100)
{
    CTraceFifoWriter rgfifoWriter[100];
    CTraceFifoReader rgfifoReader[100];
    size_t n = 100;
    size_t nCreated = 0;
    for (CTraceFifoWriter& rfifoWriter : rgfifoWriter)
    {
        rfifoWriter = CTraceFifoWriter(9999 + static_cast<uint32_t>(n++), 6000);
        nCreated += rfifoWriter.Open(0, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)) ? 1 : 0;
    }
    n = 100;
    size_t nOpened = 0;
    size_t nCorrectSize = 0;
    for (size_t nIndex = 0; nIndex < 100; nIndex++)
    {
        CTraceFifoReader& rfifoReader = rgfifoReader[nIndex];
        rfifoReader = CTraceFifoReader(9999 + static_cast<uint32_t>(n++), 2048);
        nOpened += rfifoReader.Open(0, true) ? 1 : 0;
        nCorrectSize += rfifoReader.GetViewSize() == rgfifoWriter[nIndex].GetViewSize() ? 1 : 0;
    }

    EXPECT_EQ(n, 200);
    EXPECT_EQ(nCreated, 100);
    EXPECT_EQ(nOpened, 100);
    EXPECT_EQ(nCorrectSize, 100);

    for (CTraceFifoWriter& rfifoWriter : rgfifoWriter)
        rfifoWriter.Close();
    for (CTraceFifoReader& rfifoReader : rgfifoReader)
        rfifoReader.Close();
}

TEST(TraceFifoTest, Simple_Publish_Monitor)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());

    std::vector<std::string> vecSent, vecReceived;
    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::thread thread([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader.WaitForMessage();
                if (!ss.empty())
                    vecReceived.push_back(ss);
            }
        });

    // Send 10 messages
    for (size_t n = 0; n < 10; n++)
    {
        std::stringstream sstream;
        sstream << "This is message #" << n;
        vecSent.push_back(sstream.str());
        fifoWriter.Publish(sstream.str());
    }

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bShutdown = true;
    thread.join();

    // Close both fifos
    fifoWriter.Close();
    fifoReader.Close();

    // Compare the messages
    EXPECT_EQ(vecSent, vecReceived);
}

TEST(TraceFifoTest, Simple_Publish_Monitor_Multi)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader1(9999);
    CTraceFifoReader fifoReader2(9999);
    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoReader1.Open());
    EXPECT_TRUE(fifoReader2.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader1.IsOpened());
    EXPECT_TRUE(fifoReader2.IsOpened());

    std::vector<std::string> vecSent, vecReceived1, vecReceived2;
    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::thread thread1([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader1.WaitForMessage();
                if (!ss.empty())
                    vecReceived1.push_back(ss);
            }
        });

    // Start receiving thread until shutdown flag is set.
    std::thread thread2([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader2.WaitForMessage();
                if (!ss.empty())
                    vecReceived2.push_back(ss);
            }
        });

    // Send 10 messages
    for (size_t n = 0; n < 10; n++)
    {
        std::stringstream sstream;
        sstream << "This is message #" << n;
        vecSent.push_back(sstream.str());
        fifoWriter.Publish(sstream.str());
    }

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bShutdown = true;
    thread1.join();
    thread2.join();

    // Close both fifos
    fifoWriter.Close();
    fifoReader1.Close();
    fifoReader2.Close();

    // Compare the messages
    EXPECT_EQ(vecSent, vecReceived1);
    EXPECT_EQ(vecSent, vecReceived2);
}

TEST(TraceFifoTest, Simple_Publish_Beyond_Buffer_With_Reading)
{
    CTraceFifoWriter fifoWriter(9999);
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoWriter.IsOpened());
    EXPECT_TRUE(fifoReader.IsOpened());

    std::vector<std::string> vecSent, vecReceived;
    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::thread thread([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader.WaitForMessage();
                if (!ss.empty())
                    vecReceived.push_back(ss);
            }
        });

    // Send until 110% has been reached
    size_t nTotal = 0;
    size_t n = 0;
    while (nTotal < (fifoWriter.GetDataBufferSize() * 11 / 10))
    {
        std::stringstream sstream;
        sstream << "This is message #" << n++;
        vecSent.push_back(sstream.str());
        nTotal += sstream.str().size() + 1;
        fifoWriter.Publish(sstream.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Allow the receiving thread to react
    }

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bShutdown = true;
    thread.join();

    // Close both fifos
    fifoWriter.Close();
    fifoReader.Close();

    // Compare the messages
    EXPECT_EQ(vecSent, vecReceived);
}

TEST(TraceFifoTest, Simple_Publish_Beyond_Buffer_With_Delayed_Reading)
{
    // Open writer
    CTraceFifoWriter fifoWriter(9999, 1024);
    EXPECT_TRUE(fifoWriter.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create)));
    EXPECT_TRUE(fifoWriter.IsOpened());

    std::deque<std::string> dequeSent, dequeReceived;

    // Send until 250% has been reached
    size_t nTotal = 0;
    size_t n = 0;
    while (nTotal < (fifoWriter.GetDataBufferSize() * 25 / 10))
    {
        std::stringstream sstream;
        sstream << "This is message #" << n++;
        nTotal += sstream.str().size() + 1;
        fifoWriter.Publish(sstream.str());
    }

    // Open reader
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoReader.IsOpened());

    // Read all messages (should be none)
    while (true)
    {
        std::string ss = fifoReader.WaitForMessage();
        if (ss.empty()) break;
        dequeReceived.push_back(ss);
    }

    // Send until 90% has been reached (this is the part where the reader should also receive)
    nTotal = 0;
    n = 0;
    while (nTotal < (fifoWriter.GetDataBufferSize() * 9 / 10))
    {
        std::stringstream sstream;
        sstream << "This is message #" << n++;
        dequeSent.push_back(sstream.str());
        nTotal += sstream.str().size() + 1;
        fifoWriter.Publish(sstream.str());
    }

    // Close writer
    fifoWriter.Close();

    // Read all messages (should be the 90%)
    while (true)
    {
        std::string ss = fifoReader.WaitForMessage();
        if (ss.empty()) break;
        dequeReceived.push_back(ss);
    }

    // Close reader
    fifoReader.Close();

    // Compare the messages
    EXPECT_EQ(dequeSent.size(), dequeReceived.size());
}

TEST(TraceFifoTest, Simple_Stream_Monitor)
{
    std::stringstream sstreamWriter;
    CTraceFifoStreamBuffer fifoWriterStreamBuf(9999);
    fifoWriterStreamBuf.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create));
    EXPECT_TRUE(fifoWriterStreamBuf.IsOpened());
    fifoWriterStreamBuf.InterceptStream(sstreamWriter);
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoReader.IsOpened());

    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::stringstream sstreamReader;
    std::thread thread([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader.WaitForMessage();
                if (!ss.empty())
                    sstreamReader << ss;
            }
        });

    // Send 10 messages
    for (size_t n = 0; n < 10; n++)
        sstreamWriter << "This is message #" << n;

    // Synchronize the writer
    fifoWriterStreamBuf.sync();

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bShutdown = true;
    thread.join();

    // Close the writer and the reader.
    fifoWriterStreamBuf.Close();
    fifoReader.Close();

    // Compare the messages - after close this is the original text
    EXPECT_EQ(sstreamWriter.str(), sstreamReader.str());
}

TEST(TraceFifoTest, Simple_Multi_Stream_Monitor)
{
    std::stringstream sstreamWriter1, sstreamWriter2;
    CTraceFifoStreamBuffer fifoWriterStreamBuf(9999);
    fifoWriterStreamBuf.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create));
    EXPECT_TRUE(fifoWriterStreamBuf.IsOpened());
    fifoWriterStreamBuf.InterceptStream(sstreamWriter1);
    fifoWriterStreamBuf.InterceptStream(sstreamWriter2);
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoReader.IsOpened());

    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::stringstream sstreamReader;
    std::thread thread([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader.WaitForMessage();
                if (!ss.empty())
                    sstreamReader << ss;
            }
        });

    // Send 10 messages
    for (size_t n = 0; n < 10; n++)
    {
        sstreamWriter1 << "This is message #" << n;
        sstreamWriter2 << "This is message #" << n;
    }

    // Synchronize the writer
    fifoWriterStreamBuf.sync();

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bShutdown = true;
    thread.join();

    // Close the writer and the reader.
    fifoWriterStreamBuf.Close();
    fifoReader.Close();

    // Compare the messages; one writer should be identical. The other writer didn't get any message... The choice of the writer to
    // forward the messages to depends on the order of the writer in the map.
    if (sstreamWriter1.str().empty())
    {
        EXPECT_EQ(sstreamWriter2.str(), sstreamReader.str());
        EXPECT_TRUE(sstreamWriter1.str().empty());
    }
    else
    {
        EXPECT_EQ(sstreamWriter1.str(), sstreamReader.str());
        EXPECT_TRUE(sstreamWriter2.str().empty());
    }
}

TEST(TraceFifoTest, Simple_Std_Stream_Monitor)
{
    std::stringstream sstreamWriter;
    CTraceFifoStdBuffer fifoWriterStreamBuf(9999);
    fifoWriterStreamBuf.Open(1000, static_cast<uint32_t>(ETraceFifoOpenFlags::force_create));
    EXPECT_TRUE(fifoWriterStreamBuf.IsOpened());
    CTraceFifoReader fifoReader(9999);
    EXPECT_TRUE(fifoReader.Open());
    EXPECT_TRUE(fifoReader.IsOpened());

    bool bShutdown = false;

    // Start receiving thread until shutdown flag is set.
    std::stringstream sstreamReader;
    std::thread thread([&]()
        {
            while (!bShutdown)
            {
                std::string ss = fifoReader.WaitForMessage();
                if (!ss.empty())
                    sstreamReader << ss;
            }
        });

    // Send 10 messages to COUT
    for (size_t n = 0; n < 10; n++)
    {
        std::cout << "This is COUT message #" << n;
        sstreamWriter << "This is COUT message #" << n;
    }
    std::cout.flush();

    // Send 10 messages to CLOG
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow sending
    for (size_t n = 0; n < 10; n++)
    {
        std::clog << "This is CLOG message #" << n;
        sstreamWriter << "This is CLOG message #" << n;
    }
    std::clog.flush();

    // Send 10 messages to CERR
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow sending
    for (size_t n = 0; n < 10; n++)
    {
        std::cerr << "This is CERR message #" << n;
        sstreamWriter << "This is CERR message #" << n;
    }
    std::cerr.flush();

    // Shutdown receiving thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cerr.flush();
    bShutdown = true;
    thread.join();

    // Close the writer and the reader.
    fifoWriterStreamBuf.Close();
    fifoReader.Close();

    // Compare the messages; writer1 should be identical (first writer). Writer2 didn't get any message...
    EXPECT_EQ(sstreamWriter.str(), sstreamReader.str());
}
