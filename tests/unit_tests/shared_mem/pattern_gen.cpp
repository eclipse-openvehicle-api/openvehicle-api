#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
#include "pattern_gen.h"
#include "../../../sdv_services/ipc_shared_mem/mem_buffer_accessor.h"

CPatternSender::CPatternSender(CMemBufferAccessorTx& raccessorOut, uint32_t uiDelayMs /*= 0*/) :
    m_raccessorOut(raccessorOut), m_uiDelayMs(uiDelayMs)
{
    // Start the thread
    m_thread = std::thread(&CPatternSender::Process, this);

    // Wait for the thread to run
    while (!m_bStarted) std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void CPatternSender::Process()
{
    m_bStarted = true;
    std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
    uint8_t uiCounter = 0;
    while (!m_bShutdown)
    {
        m_uiCycleCnt++;

        // Allocate space for a random sized packet
        uint32_t uiLen = static_cast<uint32_t>(std::rand() % 8191);
        auto optPacket = m_raccessorOut.Reserve(uiLen);
        if (!optPacket)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        m_uiPacketCnt++;
        m_uiByteCnt += uiLen;

        // Fill the packet
        for (uint32_t uiIndex = 0; uiIndex < uiLen; uiIndex++)
            optPacket->GetDataPtr()[uiIndex] = uiCounter++;

        // Commit...
        optPacket->Commit();

        // Sleep if necessary
        if (m_uiDelayMs)
            std::this_thread::sleep_for(std::chrono::milliseconds(m_uiDelayMs));
    }
}

CPatternReceiver::CPatternReceiver(CMemBufferAccessorRx& raccessorIn, uint32_t uiDelayMs /*= 0*/) :
    m_raccessorIn(raccessorIn),m_uiDelayMs(uiDelayMs)
{
    // Start the thread
    m_thread = std::thread(& CPatternReceiver::Process, this);

    // Wait for the thread to run
    while (!m_bStarted) std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void CPatternReceiver::Process()
{
    m_bStarted = true;
    uint8_t uiCounter = 0;
    bool	bInitial  = true;
    while (!m_bShutdown)
    {
        m_uiCycleCnt++;

        // Check for a packet
        auto optPacket = m_raccessorIn.TryRead();
        if (!optPacket)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        const uint8_t* pData = optPacket->GetData();
        uint32_t uiLen = optPacket->GetSize();
        m_uiPacketCnt++;
        m_uiByteCnt += uiLen;

        // The first byte of the first packet sets the counter
        if (bInitial)
            uiCounter = pData[0];
        bInitial = false;

        // Check the counter values
        for (uint32_t uiIndex = 0; uiIndex < uiLen; uiIndex++)
        {
            if (uiCounter != pData[uiIndex])
            {
                m_uiErrorCnt++;
                uiCounter = pData[uiIndex];
            }
            uiCounter++; // Expecting new counter value
        }

        // Release
        optPacket->Accept();

        // Sleep if necessary
        if (m_uiDelayMs)
            std::this_thread::sleep_for(std::chrono::milliseconds(m_uiDelayMs));
    }
}

CPatternRepeater::CPatternRepeater(CMemBufferAccessorRx& raccessorIn, CMemBufferAccessorTx& raccessorOut, uint32_t uiDelayMs /*= 0*/) :
    m_raccessorIn(raccessorIn), m_raccessorOut(raccessorOut), m_uiDelayMs(uiDelayMs)
{
    // Start the thread
    m_thread = std::thread(&CPatternRepeater::Process, this);

    // Wait for the thread to run
    while (!m_bStarted) std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void CPatternRepeater::Process()
{
    m_bStarted = true;
    uint8_t uiCounter = 0;
    bool	bInitial  = true;
    while (!m_bShutdown)
    {
        m_uiCycleCnt++;

        // Check for a packet
        auto optPacket = m_raccessorIn.TryRead();
        if (!optPacket)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        const uint8_t* pData = optPacket->GetData();
        uint32_t uiLen = optPacket->GetSize();
        m_uiPacketCnt++;
        m_uiByteCnt += uiLen;

        // The first byte of the first packet sets the counter
        if (bInitial)
            uiCounter = pData[0];
        bInitial = false;

        // Check the counter values
        for (uint32_t uiIndex = 0; uiIndex < uiLen; uiIndex++)
        {
            if (uiCounter != pData[uiIndex])
            {
                m_uiErrorCnt++;
                uiCounter = pData[uiIndex];
            }
            uiCounter++; // Expecting new counter value
        }

        // Send away again
        while (!m_raccessorOut.TryWrite(pData, uiLen))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (m_bShutdown)
                break;
        }

        // Release
        optPacket->Accept();

        // Sleep if necessary
        if (m_uiDelayMs)
            std::this_thread::sleep_for(std::chrono::milliseconds(m_uiDelayMs));
    }
}

