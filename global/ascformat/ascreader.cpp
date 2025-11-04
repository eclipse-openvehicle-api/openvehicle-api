#include "ascreader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>
#include <processthreadsapi.h>
#elif defined __unix__
#include <pthread.h>
#endif

namespace asc
{
    CAscReader::CAscReader() : m_itCurrent(m_lstMessages.begin())
    {}

    CAscReader::~CAscReader()
    {
        StopPlayback();
    }

    bool CAscReader::Read(const std::filesystem::path& rpathFile)
    {
        // Clear current messages
        m_lstMessages.clear();
        JumpBegin();

        // Open and read the file
        std::ifstream fstream(rpathFile);
        if (!fstream.is_open()) return false;

        // Read the file line by line
        std::string ssLine;
        enum class EState {header, body, footer} eState = EState::header;
        while (std::getline(fstream, ssLine))
        {
            switch (eState)
            {
            case EState::header:
                if (ssLine.compare(0, 18, "Begin Triggerblock") == 0)
                    eState = EState::body;
                break;
            case EState::body:
                if (ssLine.compare(0, 16, "End TriggerBlock") == 0)
                    eState = EState::footer;
                else
                    ProcessSample(ssLine);
                break;
            default:
                break;
            }
        }
        fstream.close();

        // Set the current iterator
        JumpBegin();

        return true;
    }

    std::pair<SCanMessage, bool> CAscReader::Get() const
    {
        if (m_itCurrent == m_lstMessages.end())
            return std::make_pair(SCanMessage(), false);
        else
            return std::make_pair(*m_itCurrent, true);
    }

    uint32_t CAscReader::GetLoopCount() const
    {
        return m_uiLoopCount;
    }
    
    void CAscReader::JumpBegin()
    {
        m_itCurrent = m_lstMessages.begin();
    }

    void CAscReader::JumpEnd()
    {
        m_itCurrent = m_lstMessages.end();
    }

    CAscReader& CAscReader::operator++()
    {
        if (m_itCurrent != m_lstMessages.end())
            ++m_itCurrent;
        return *this;
    }

    CAscReader& CAscReader::operator++(int)
    {
        if (m_itCurrent != m_lstMessages.end())
            ++m_itCurrent;
        return *this;
    }

    CAscReader& CAscReader::operator--()
    {
        if (m_itCurrent != m_lstMessages.begin())
            --m_itCurrent;
        return *this;
    }

    CAscReader& CAscReader::operator--(int)
    {
        if (m_itCurrent != m_lstMessages.begin())
            --m_itCurrent;
        return *this;
    }

    bool CAscReader::IsBOF() const
    {
        return m_itCurrent == m_lstMessages.begin();
    }

    bool CAscReader::IsEOF() const
    {
        return m_itCurrent == m_lstMessages.end();
    }

    void CAscReader::StartPlayback(std::function<void(const SCanMessage&)> fnCallback, bool bRepeat /*= true*/)
    {
        StopPlayback();

        if (!fnCallback) return;
        if (m_lstMessages.empty()) return;

        m_bPlaybackThread = false;
        m_bPlayback = true;
        m_threadPlayback = std::thread(&CAscReader::PlaybackThreadFunc, this, fnCallback, std::ref(m_uiLoopCount), bRepeat);
        while (!m_bPlaybackThread) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    void CAscReader::StopPlayback()
    {
        m_bPlayback = false;
        if (m_threadPlayback.joinable()) m_threadPlayback.join();
    }

    void CAscReader::ResetPlayback()
    {
        StopPlayback();
        JumpBegin();
    }

    bool CAscReader::PlaybackRunning() const
    {
        return m_bPlayback;
    }

    void CAscReader::ProcessSample(const std::string& rssSample)
    {
        std::istringstream sstream(rssSample);

        SCanMessage sMsg{};

        // Expecting a time
        if (!(sstream >> sMsg.dTimestamp))
            return;  // All usable mesaurements must start with a timestamp

        // Skip whitespace
        while (std::isspace(sstream.peek())) sstream.get();

        // If the next timestamp is followed by the "CANFD" keyword, the sample is a CAN-FD sample
        if (std::isalpha(sstream.peek()))
        {
            std::string ssKeyword;
            if (!(sstream >> ssKeyword))
                return; // Unexpected
            if (ssKeyword != "CANFD")
                return; // Not a CAN-FD sample
            sMsg.bCanFd = true;

            // Expecting a channel
            if (!(sstream >> sMsg.uiChannel))
                return; // Expected a channel (otherwise the measurement might contain meta data)

            // Determine the direction
            std::string ssDirection;
            if (!(sstream >> ssDirection))
                return; // Expected a direction
            if (ssDirection == "Rx")
                sMsg.eDirection = SCanMessage::EDirection::rx;
            else if (ssDirection == "Tx")
                sMsg.eDirection = SCanMessage::EDirection::tx;
            else
                return; // Invalid direction

            // Expecting a message ID in hex format
            if (!(sstream >> std::hex >> sMsg.uiId))
                return; // Expected an ID (otherwise the measurement might contain meta data)

            // Skip whitespace
            while (std::isspace(sstream.peek())) sstream.get();

            // Optional 'x' for an extended ID
            if (std::tolower(sstream.peek()) == 'x')
            {
                sMsg.bExtended = true;
                sstream.get();  // Skip the character, since already processed
            }

            // Get bit rate switch (BRS)
            size_t nBRS = 0;
            if (!(sstream >> nBRS) || nBRS > 1)
                return; // Unexpected or invalid BRS

            // Get error state indicator (ESI)
            size_t nESI = 0;
            if (!(sstream >> nESI) || nESI > 1)
                return; // Unexpected or invalid ESI

            // Get data length code (DLC)
            size_t nDLC;
            if (!(sstream >> nDLC) || nDLC > 15)
                return; // Unexpected or invalid DLC

            // Get the data length
            if (!(sstream >> std::dec >> sMsg.uiLength) || sMsg.uiLength > 64)
                return; // Length expected or invalid length

            // Check for proper DLC vs. length
            const size_t rgnDLCLength[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };
            if (rgnDLCLength[nDLC] != sMsg.uiLength) return; // Invalid length
        }
        else // standard CAN
        {
            // Expecting a channel
            if (!(sstream >> sMsg.uiChannel))
                return; // Expected a channel (otherwise the measurement might contain meta data)

            // Expecting a message ID in hex format
            if (!(sstream >> std::hex >> sMsg.uiId))
                return; // Expected an ID (otherwise the measurement might contain meta data)

            // Skip whitespace
            while (std::isspace(sstream.peek())) sstream.get();

            // Optional 'x' for an extended ID
            if (std::tolower(sstream.peek()) == 'x')
            {
                sMsg.bExtended = true;
                sstream.get();  // Skip the character, since already processed
            }

            // Determine the direction
            std::string ssDirection;
            if (!(sstream >> ssDirection))
                return; // Expected a direction
            if (ssDirection == "Rx")
                sMsg.eDirection = SCanMessage::EDirection::rx;
            else if (ssDirection == "Tx")
                sMsg.eDirection = SCanMessage::EDirection::tx;
            else
                return; // Invalid direction

            // Determine whether the measurement contains data from a data frame (remote frames are not processed).
            char cLocation = '\0';
            if (!(sstream >> cLocation) || std::tolower(cLocation) != 'd')
                return; // Not supported location

            // Get the data length
            if (!(sstream >> std::dec >> sMsg.uiLength) || sMsg.uiLength > 8)
                return; // Length expected or invalid length
        }

        // Read the data
        for (uint32_t uiIndex = 0; uiIndex < sMsg.uiLength; uiIndex++)
        {
            uint32_t uiVal = 0;
            if (!(sstream >> std::hex >> uiVal))
                return; // Expected data byte
            sMsg.rguiData[uiIndex] = static_cast<uint8_t>(uiVal);
        }

        // Add the message to the vector
        m_lstMessages.push_back(std::move(sMsg));

        // Ignore the rest of the line (additional information could have been logged).
    }

    void CAscReader::PlaybackThreadFunc(std::function<void(const SCanMessage&)> fnCallback, std::atomic<uint32_t>& loopCount, bool bRepeat)
    {
#ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#elif defined __unix__
        pthread_attr_t thAttr;
        pthread_attr_init(&thAttr);
        int iPolicy = 0;
        pthread_attr_getschedpolicy(&thAttr, &iPolicy);
        int iMaxPrioForPolicy = sched_get_priority_max(iPolicy);
        pthread_setschedprio(pthread_self(), iMaxPrioForPolicy);
        pthread_attr_destroy(&thAttr);
#endif

        // Indicate that playback thread has started.
        m_bPlaybackThread = true;
        // when data set is repeated, we need the offset compared to the first run
        double dLoopOffset = 0.00000;

        // Define the offset
        double dOffset = 0.00000;
        if (!IsBOF())
        {
            operator--();
            auto prSample = Get();
            if (!prSample.second)
            {
                m_bPlayback = false;
                std::cout << "ASC PLAYBACK: Unexpected situation. Not BOF, but also no previous sample." << std::endl;
                return;    // Should not occur
            }
            dOffset = prSample.first.dTimestamp;
            operator++();
        }

        loopCount++;
        auto timepointStartOfFirstLoop = std::chrono::high_resolution_clock::now();
        auto timepointStart = std::chrono::high_resolution_clock::now();
        while (m_bPlayback)
        {
            if (IsEOF())
            {
                if (!bRepeat) break;    // Done

                // Data set will be repeated, callculate offset compared to the first run
                auto timepointStartOfNextLoop = std::chrono::high_resolution_clock::now();
                dLoopOffset = std::chrono::duration<double>(timepointStartOfNextLoop - timepointStartOfFirstLoop).count();

                // Restart
                loopCount++;                
                dOffset = 0.00000;
                JumpBegin();
            }

            // Get a sample
            auto prSample = Get();
            if (!prSample.second) 
                break;    // Should not occur

            // Need to sleep?
            while (m_bPlayback)
            {
                // Determine the current time relative to start time.
                auto timepoint = std::chrono::high_resolution_clock::now();
                double dTime = std::chrono::duration<double>(timepoint - timepointStart).count() + dOffset;

                // If the sample is overdue... do not sleep
                if ((prSample.first.dTimestamp + dLoopOffset) < dTime) 
                    break;

                // If the sample will be overdue in 10 ms, yield the current thread only (this will not put the thread in idle mode).
                //if (prSample.first.dTimestamp < dTime + 0.010)
                //    std::this_thread::yield();
                //else // Sleep shortly... this might cause a sleep of more than 1ms dependable on the load of the system
                std::this_thread::sleep_for(std::chrono::milliseconds(0));
            }

            // Send the sample
            fnCallback(prSample.first);

            // Next sample
            operator++();
        }

        m_bPlayback = false;
    }
}   // namespace asc
