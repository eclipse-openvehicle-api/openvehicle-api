#include "mem_buffer_accessor.h"
#include <cassert>

void CMemBufferAccessorBase::Attach(uint8_t* pBuffer, uint32_t uiSize /*= 0*/)
{
    // Attach is only allowed to be called once
    assert(!m_pHdr && !m_pBuffer);
    if (m_pHdr || m_pBuffer)
    {
        std::stringstream sstream;
        sstream << "Accessor: Attaching is only allowed once (";
        if (m_pHdr) sstream << "header";
        if (m_pHdr && m_pBuffer) sstream << ", ";
        if (m_pBuffer) sstream << "buffer";
        sstream << ")" << std::endl;
        std::cout << sstream.str();
        return;
    }

    assert(!uiSize || uiSize > sizeof(SBufferHdr));

    // Assign the buffer
    assert(pBuffer);
    m_pHdr = reinterpret_cast<SBufferHdr*>(pBuffer);
    if (!m_pHdr)
    {
        std::cout << "Accessor: header is NULL" << std::endl;
        return;
    }
    m_pBuffer = pBuffer + static_cast<uint32_t>(sizeof(SBufferHdr));

    // If a size has been provided, initialize the header
    if (uiSize)
    {
        *m_pHdr = SBufferHdr();
        m_pHdr->uiSize = uiSize - static_cast<uint32_t>(sizeof(SBufferHdr));
    }

    // Check for correct interface version (to prevent misaligned communication).
    assert(m_pHdr->uiVersion == SDVFrameworkInterfaceVersion);

    // Check for the size to be larger than the buffer header and 64-bit aligned
    assert(m_pHdr->uiSize > sizeof(SBufferHdr));
    assert(m_pHdr->uiSize % 8 == 0);
    if (m_pHdr->uiVersion != SDVFrameworkInterfaceVersion || m_pHdr->uiSize <= sizeof(SBufferHdr) || m_pHdr->uiSize % 8 != 0)
    {
        m_pHdr = nullptr;
        m_pBuffer = nullptr;
    }
}

void CMemBufferAccessorBase::Detach()
{
    m_pBuffer = nullptr;
    m_pHdr = nullptr;

}

bool CMemBufferAccessorBase::IsValid() const
{
    return m_pHdr && m_pBuffer;
}

const uint8_t* CMemBufferAccessorBase::GetBufferPointer() const
{
    return reinterpret_cast<uint8_t*>(m_pHdr);
}

CAccessorTxPacket::CAccessorTxPacket(CMemBufferAccessorTx& rAccessor, CMemBufferAccessorBase::SPacketHdr* pPacketHdr) :
    m_pAccessor(&rAccessor)
{
    // Checks
    if (!pPacketHdr) return;
    if (pPacketHdr->eType != CMemBufferAccessorBase::SPacketHdr::EType::data) return;
    if (pPacketHdr->eState != CMemBufferAccessorBase::SPacketHdr::EState::reserved) return;
    m_pPacketHdr = pPacketHdr;
}

CAccessorTxPacket::CAccessorTxPacket(CAccessorTxPacket&& rpacket) noexcept:
    m_pAccessor(rpacket.m_pAccessor), m_pPacketHdr(rpacket.m_pPacketHdr)
{
    rpacket.m_pAccessor = nullptr;
    rpacket.m_pPacketHdr = nullptr;
}

CAccessorTxPacket::~CAccessorTxPacket()
{
    Commit();
}

CAccessorTxPacket& CAccessorTxPacket::operator=(CAccessorTxPacket&& rpacket) noexcept
{
    m_pAccessor = rpacket.m_pAccessor;
    m_pPacketHdr = rpacket.m_pPacketHdr;
    rpacket.m_pAccessor = nullptr;
    rpacket.m_pPacketHdr = nullptr;
    return *this;
}

CAccessorTxPacket::operator bool() const
{
    return IsValid();
}

bool CAccessorTxPacket::IsValid() const
{
    return m_pPacketHdr;
}

void CAccessorTxPacket::Commit()
{
    if (m_pAccessor && m_pPacketHdr)
        m_pAccessor->Commit(m_pPacketHdr);
    m_pPacketHdr = nullptr;
}

uint32_t CAccessorTxPacket::GetSize() const
{
    return m_pPacketHdr ? m_pPacketHdr->uiSize : 0;
}

uint8_t* CAccessorTxPacket::GetDataPtr()
{
    return GetSize() ? reinterpret_cast<uint8_t*>(m_pPacketHdr + 1) : 0;
}

CMemBufferAccessorTx::~CMemBufferAccessorTx()
{
    m_bBlockReserve = true;

    // Wait until all reserved packets are committed (could cause a crash otherwise).
    std::unique_lock<std::mutex> lock(m_mtxReservedPackes);

    // Remove any committed packets
    while (!m_queueReservedPackets.empty())
    {
        uint32_t uiTxPos = m_pHdr->uiTxPos;
        SPacketHdr* pPacketHdr = m_queueReservedPackets.front();

        // Is the top most packet not in committed state, we'll wait.
        if (pPacketHdr->eState != SPacketHdr::EState::commit)
        {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock.lock();
            continue;
        }

        // Update the write position
        uiTxPos =
            Align(static_cast<uint32_t>(reinterpret_cast<uint8_t*>(pPacketHdr) - m_pBuffer + sizeof(SPacketHdr))
                + pPacketHdr->uiSize);

        // In case the write position is pointing to the end of the buffer, jump to the begin.
        if (uiTxPos >= m_pHdr->uiSize)
        {
            uiTxPos = 0;
        }

        // Remove from queue
        m_queueReservedPackets.pop_front();
        m_pHdr->uiTxPos = uiTxPos;
    }
}

std::optional<CAccessorTxPacket> CMemBufferAccessorTx::Reserve(uint32_t uiSize, uint32_t uiTimeoutMs)
{
    if (m_bBlockReserve) return {};
    if (!IsValid()) return {};
    if (uiSize > m_pHdr->uiSize) return {};

    uint32_t uiTxPos = 0;
    bool bStuffingNeeded = false;
    while (!m_bCancel)
    {
        // Create a snapshot of the read and write positions
        // If exists, use the last stored position in the queue. Otherwise use the position from the header.
        uint32_t uiRxPos = m_pHdr->uiRxPos;
        uiTxPos = m_pHdr->uiTxPos;
        std::unique_lock<std::mutex> lock(m_mtxReservedPackes);
        if (!m_queueReservedPackets.empty())
            uiTxPos = Align(reinterpret_cast<uint8_t*>(m_queueReservedPackets.back()) - m_pBuffer + sizeof(SPacketHdr)
                + m_queueReservedPackets.back()->uiSize);

        // Calculate the needed size (incl header) aligned to 64 bits.
        uint32_t uiNeededSize = Align(uiSize + static_cast<uint32_t>(sizeof(SPacketHdr)));

        // If the read position is beyond the write position, the available space is the diference
        // If the read position is behind the write position, the available space is eiher until the end of the buffer or if not
        // fitting from the beginning of the buffer until the read position.
        uint32_t uiMaxSize = 0;
        bStuffingNeeded = false;
        if (uiRxPos > uiTxPos)   // uiTxPos made a roundtrip already
            uiMaxSize = uiRxPos - uiTxPos - 1; // The last possible writing position is one before the reading position
        else // uiRxPos is running after uiTxPos
        {
            // uiMaxSize is the rest of the buffer
            uiMaxSize = m_pHdr->uiSize > uiTxPos ? m_pHdr->uiSize - uiTxPos : 0;

            // When uiRxPos is at the beginning, this is a special situation, max size is the rest minus 1
            if (!uiRxPos)
                uiMaxSize--;
            else if (uiMaxSize < uiNeededSize)
            {
                bStuffingNeeded = true;
                uiMaxSize = uiRxPos - 1;
            }
        }

        // Check for size
        if (uiNeededSize <= uiMaxSize)
            break;

        // Wait for a reserve
        if (!WaitForFreeSpace(uiTimeoutMs))
            return {};
    }

    if (m_bCancel) return {};

    // Stuffing needed?
    if (bStuffingNeeded)
    {
        // Create stuffing packet... but only if the header still fits
        if (m_pHdr->uiSize - uiTxPos >= static_cast<uint32_t>(sizeof(SPacketHdr)))
        {
            SPacketHdr* pStuffPacket = GetPacketHdr(uiTxPos);
            pStuffPacket->eType		 = SPacketHdr::EType::stuffing;
            pStuffPacket->uiSize	 = m_pHdr->uiSize - uiTxPos - static_cast<uint32_t>(sizeof(SPacketHdr));
            pStuffPacket->eState	 = SPacketHdr::EState::commit;
        }
        else if (uiTxPos < m_pHdr->uiSize)
            std::fill_n(m_pBuffer + uiTxPos, m_pHdr->uiSize - uiTxPos, static_cast<uint8_t>(0));

        // After stuffing, the new location is at the beginning of the buffer.
        uiTxPos = 0;
    }

    // Prepare a packet
    SPacketHdr* pPacket = GetPacketHdr(uiTxPos);
    pPacket->eType = SPacketHdr::EType::data;
    pPacket->uiSize = uiSize;
    pPacket->eState = SPacketHdr::EState::reserved;

    // Add the packet to the queue
    m_queueReservedPackets.push_back(pPacket);

    // Create the packet
    return CAccessorTxPacket(*this, pPacket);
}

void CMemBufferAccessorTx::Commit(SPacketHdr* pPacketHdr)
{
    if (!IsValid()) return;
    if (!pPacketHdr) return;

    // pData needs to point to an area in the buffer starting at the offset of a packet header.
    if (reinterpret_cast<uint8_t*>(pPacketHdr) > m_pBuffer + m_pHdr->uiSize)
        return; // Pointing beyond the buffer
    if (reinterpret_cast<uint8_t*>(pPacketHdr) < m_pBuffer)
        return; // Pointing before the first possible packet header

    // Check packet header
    if (pPacketHdr->eType != SPacketHdr::EType::data)
        return; // Must be of type data
    if (pPacketHdr->eState != SPacketHdr::EState::reserved)
        return; // Must have reserved state
    if (reinterpret_cast<uint8_t*>(pPacketHdr) + pPacketHdr->uiSize + sizeof(SPacketHdr) > m_pBuffer + m_pHdr->uiSize)
        return; // Size cannot be beyond buffer

    // Commit the packet
    pPacketHdr->eState = SPacketHdr::EState::commit;

    // Trigger processing
    TriggerDataSend();

    // Run through the queue and check whether the top most packet is actually committed
    std::unique_lock<std::mutex> lock(m_mtxReservedPackes);
    uint32_t uiTxPos = m_pHdr->uiTxPos;
    while (!m_queueReservedPackets.empty())
    {
        SPacketHdr* pPacketHdr2 = m_queueReservedPackets.front();

        // Is the top most packet not in committed state, we're done.
        if (pPacketHdr2->eState != SPacketHdr::EState::commit) break;

        // Update the write position
        uiTxPos = Align(static_cast<uint32_t>(reinterpret_cast<uint8_t*>(pPacketHdr2) - m_pBuffer + sizeof(SPacketHdr))
                + pPacketHdr2->uiSize);

        // In case the write position is pointing to the end of the buffer, jump to the begin.
        if (uiTxPos >= m_pHdr->uiSize)
        {
            uiTxPos = 0;
        }

        // Remove from queue
        m_queueReservedPackets.pop_front();
    }
    m_pHdr->uiTxPos = uiTxPos;
}

bool CMemBufferAccessorTx::TryWrite(const void* pData, uint32_t uiSize)
{
    if (!IsValid())
        return false;

    // pData is only allowed to be NULL when uiSize is zero
    if (uiSize && !pData)
        return false;

    // Reserve a packet
    auto optPacket = Reserve(uiSize);
    if (!optPacket) return false;

    // Copy the data
    if (optPacket->GetSize())
        std::copy(reinterpret_cast<const uint8_t*>(pData), reinterpret_cast<const uint8_t*>(pData) + uiSize, optPacket->GetDataPtr());

    return true;
}

CAccessorRxPacket::CAccessorRxPacket(CMemBufferAccessorRx& rAccessor, CMemBufferAccessorBase::SPacketHdr* pPacketHdr) :
    m_pAccessor(&rAccessor)
{
    // Checks
    if (!pPacketHdr) return;
    if (pPacketHdr->eType != CMemBufferAccessorBase::SPacketHdr::EType::data) return;
    if (pPacketHdr->eState != CMemBufferAccessorBase::SPacketHdr::EState::read) return;
    m_pPacketHdr = pPacketHdr;
}

CAccessorRxPacket::CAccessorRxPacket(CAccessorRxPacket&& rpacket) noexcept:
    m_pAccessor(rpacket.m_pAccessor), m_pPacketHdr(rpacket.m_pPacketHdr)
{
    rpacket.m_pAccessor = nullptr;
    rpacket.m_pPacketHdr = nullptr;
}

CAccessorRxPacket& CAccessorRxPacket::operator=(CAccessorRxPacket&& rpacket) noexcept
{
    m_pAccessor = rpacket.m_pAccessor;
    m_pPacketHdr = rpacket.m_pPacketHdr;
    rpacket.m_pAccessor = nullptr;
    rpacket.m_pPacketHdr = nullptr;
    return *this;
}

CAccessorRxPacket::operator bool() const
{
    return IsValid();
}

bool CAccessorRxPacket::IsValid() const
{
    return GetData() && GetSize();
}

void CAccessorRxPacket::Reset()
{
    m_pPacketHdr = nullptr;
}

uint32_t CAccessorRxPacket::GetSize() const
{
    return m_pPacketHdr ? m_pPacketHdr->uiSize : 0;
}

const uint8_t* CAccessorRxPacket::GetData() const
{
    return reinterpret_cast<const uint8_t*>(m_pPacketHdr ? m_pPacketHdr + 1 : nullptr);
}

void CAccessorRxPacket::Accept()
{
    if (!m_pAccessor) return;

    // Mark the packet as free
    std::unique_lock<std::mutex> lock(m_pAccessor->m_mtxReadAccess);
    if (m_pPacketHdr) m_pPacketHdr->eState = CMemBufferAccessorBase::SPacketHdr::EState::free;
    lock.unlock();

    // Release any read packets
    m_pAccessor->ReleasePackets();
}

void CMemBufferAccessorRx::Attach(uint8_t* pBuffer, uint32_t uiSize /*= 0*/)
{
    // Restore from a possible previous connection.
    CMemBufferAccessorBase::Attach(pBuffer, uiSize);
}

std::optional<CAccessorRxPacket> CMemBufferAccessorRx::TryRead()
{
    if (!IsValid()) return {};

    // Create a snapshot of the read and write positions
    std::unique_lock<std::mutex> lock(m_mtxReadAccess);
    uint32_t uiRxPos = m_pHdr->uiRxPos;
    uint32_t uiTxPos = m_pHdr->uiTxPos;

    // Find the next available data packet that is unread
    while (uiRxPos != uiTxPos)
    {
        // Check for an invalid position
        if (uiRxPos > m_pHdr->uiSize)
        {
            // Should not happen!
            uiRxPos -= m_pHdr->uiSize;
            continue;
        }

        // Is there still a header defined until the end of the buffer?
        if (m_pHdr->uiSize - uiRxPos < static_cast<uint32_t>(sizeof(SPacketHdr)))
        {
            // If the uiTxPos is equal or larger, no more header is available at the moment.
            if (uiTxPos >= uiRxPos) break;

            // Start at the beginning
            uiRxPos = 0;
            continue;
        }

        // Get the next packet
        SPacketHdr* pPacketHdr = GetPacketHdr(uiRxPos);

        // Check the packet state
        enum class ENextStep {process, cancel, skip} eNextStep = ENextStep::cancel;
        switch (pPacketHdr->eState)
        {
        case SPacketHdr::EState::commit:        // Packet is available and unread; process if packet is data packet.
            if (pPacketHdr->eType == SPacketHdr::EType::data)
                eNextStep = ENextStep::process;
            else
                eNextStep = ENextStep::skip;
            break;
        case SPacketHdr::EState::read:          // Packet is in use by other thread. Skip this packet.
        case SPacketHdr::EState::free:          // Packet is released, but TX has't been updated. Skip this packet.
            eNextStep = ENextStep::skip;
            break;
        case SPacketHdr::EState::reserved:      // Packet is not finall written. This should not happen.
        default:
            eNextStep = ENextStep::cancel;
            break;
        }

        // Do the next step
        if (eNextStep == ENextStep::cancel)
            break;
        if (eNextStep == ENextStep::skip)
        {
            // Update the read position
            uiRxPos = Align(static_cast<uint32_t>(reinterpret_cast<uint8_t*>(pPacketHdr) - m_pBuffer + sizeof(SPacketHdr))
                + pPacketHdr->uiSize);
            continue;
        }

        // Update packet header
        pPacketHdr->eState = SPacketHdr::EState::read;

        // Return a packet class
        return CAccessorRxPacket(*this, pPacketHdr);
    }

    // Packet not available.
    return {};
}

void CMemBufferAccessorRx::ReleasePackets()
{
    if (!IsValid()) return;

    // Create a snapshot of the read and write positions
    std::unique_lock<std::mutex> lock(m_mtxReadAccess);
    uint32_t uiRxPos = m_pHdr->uiRxPos;
    uint32_t uiTxPos = m_pHdr->uiTxPos;

    // Find the next available data packet that is unread
    while (uiRxPos != uiTxPos)
    {
        // Check for an invalid position
        if (uiRxPos > m_pHdr->uiSize)
        {
            // Should not happen!
            uiRxPos -= m_pHdr->uiSize;
            continue;
        }

        // Is there still a header defined until the end of the buffer?
        if (m_pHdr->uiSize - uiRxPos < static_cast<uint32_t>(sizeof(SPacketHdr)))
        {
            // If the uiTxPos is equal or larger, no more header is available at the moment.
            if (uiTxPos >= uiRxPos) break;

            // Start at the beginning
            uiRxPos = 0;
            continue;
        }

        // Get the next packet
        SPacketHdr* pPacketHdr = GetPacketHdr(uiRxPos);

        // Check whether the packet is an unread data packet
        if (pPacketHdr->eType == SPacketHdr::EType::data && pPacketHdr->eState != SPacketHdr::EState::free) break;

        // Update the read position
        uiRxPos = Align(static_cast<uint32_t>(reinterpret_cast<uint8_t*>(pPacketHdr) - m_pBuffer + sizeof(SPacketHdr))
            + pPacketHdr->uiSize);
    }

    // Store the new position
    m_pHdr->uiRxPos = uiRxPos;

    // Trigger
    TriggerDataReceive();
}

CMemBufferAccessorBase::SPacketHdr* CMemBufferAccessorBase::GetPacketHdr(uint32_t uiPos) const
{
    if (!m_pBuffer) return nullptr;
    return reinterpret_cast<SPacketHdr*>(m_pBuffer + uiPos);
}

uint8_t* CMemBufferAccessorBase::GetPacketData(uint32_t uiPos) const
{
    if (!m_pBuffer) return nullptr;
    return m_pBuffer + uiPos + static_cast<uint32_t>(sizeof(SPacketHdr));
}

