#include "ascwriter.h"
#include <fstream>

namespace asc
{
    CAscWriter::CAscWriter()
    {
        StartTimer();
    }

    void CAscWriter::AddSample(const SCanMessage& rsSample)
    {
        SCanMessage sSampleCopy(rsSample);
        if (sSampleCopy.dTimestamp == 0.0000000)
            sSampleCopy.dTimestamp =
            std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_timepointStart).count();
        else
        {
            if (!m_lstMessages.empty() && m_lstMessages.back().dTimestamp > sSampleCopy.dTimestamp) return;
        }
        m_lstMessages.push_back(std::move(sSampleCopy));
    }

    bool CAscWriter::HasSamples() const
    {
        return !m_lstMessages.empty();
    }

    void CAscWriter::StartTimer()
    {
        if (m_lstMessages.empty())
        {
            m_timepointStart = std::chrono::high_resolution_clock::now();
            m_timepointSystem = std::chrono::system_clock::now();
        }
    }

    void CAscWriter::Clear()
    {
        m_lstMessages.clear();
        m_timepointStart = std::chrono::high_resolution_clock::now();
    }

    bool CAscWriter::Write(const std::filesystem::path& rpathFile)
    {
        // Open and write the file
        std::ofstream fstream(rpathFile, std::ios::out | std::ios::trunc);
        if (!fstream.is_open()) return false;

        // Fill in header
        time_t tmSystemTime = std::chrono::system_clock::to_time_t(m_timepointSystem);
        fstream << "date " << std::put_time(std::localtime(&tmSystemTime), "%c") << std::endl;
        fstream << "base hex  timestamps absolute" << std::endl;

        // Stream trigger block
        fstream << "Begin TriggerBlock " << std::put_time(std::localtime(&tmSystemTime), "%c") << std::endl;
        fstream << "   0.000000 Start of measurement" << std::endl;
        for (const SCanMessage& rsSample : m_lstMessages)
        {
            std::stringstream sstreamTimestamp;
            sstreamTimestamp << std::fixed << std::setprecision(6) << rsSample.dTimestamp;
            fstream << std::string(11ull - sstreamTimestamp.str().length(), ' ') << sstreamTimestamp.str();
            if (rsSample.bCanFd)
            {
                fstream << " " << "CANFD" << " " << rsSample.uiChannel << " " <<
                    (rsSample.eDirection == SCanMessage::EDirection::rx ? "Rx" : "Tx") <<
                    std::hex << std::uppercase << rsSample.uiId << (rsSample.bExtended ? "x" : "") << " 1 0 ";
                const size_t rgnDLCLength[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };
                size_t nDLC = 0;
                for (size_t n = 0; n < 16; n++)
                    if (rgnDLCLength[n] == rsSample.uiLength) nDLC = n;
                fstream << nDLC << " " << rsSample.uiLength;
            }
            else // Normal CAN
            {
                fstream << " " << rsSample.uiChannel;
                std::stringstream sstreamID;
                sstreamID << std::hex << std::uppercase << rsSample.uiId;
                fstream << "  " << sstreamID.str() << (rsSample.bExtended ? "x" : "");
                fstream << std::string(16ull - sstreamID.str().length(), ' ') <<
                    (rsSample.eDirection == SCanMessage::EDirection::rx ? "Rx" : "Tx");
                fstream << "   " << "d";
                fstream << " " << rsSample.uiLength;
            }
            for (uint32_t uiIndex = 0; uiIndex < rsSample.uiLength; uiIndex++)
                fstream << " " << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << static_cast<size_t>(rsSample.rguiData[uiIndex]);
            fstream << std::endl;
        }
        fstream << "End TriggerBlock" << std::endl;

        // Done
        fstream.close();

        return true;
    }
} // namespace asc