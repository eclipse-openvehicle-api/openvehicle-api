#ifndef MEM_BUFFER_ACCESSOR_H
#define MEM_BUFFER_ACCESSOR_H

#include <deque>
#include <mutex>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <optional>
#include <interfaces/core_types.h>

/**
 * @brief Accessor direction
*/
enum class EAccessType
{
    rx,     ///< Reading accessor
    tx,     ///< Writing accessor
};

/**
 * @brief Memory buffer accessor.
 * @details Lock-free memory buffer accessor. This buffer allows the writing and reading of memory packets in a circular buffer.
 * @attention This class does not synchronize calls! It works if only one thread at the time writes data and only one thread at
 * the time reads data. Reading and writing can occur simulateously.
 * @attention No protection is implemented to monitor the lifetime of the buffer. It is assumed that during any call this buffer
 * is valid.
 * @remarks This class doesn't provide any handshaking. This is the task of the objects using this class.
 */
class CMemBufferAccessorBase
{
protected:
    /**
     * @brief Memory buffer header
     */
    struct SBufferHdr
    {
        uint32_t uiVersion = SDVFrameworkInterfaceVersion;  ///< Check for compatible communication channel
        uint32_t uiSize	   = 0u;                            ///< The size of the buffer
        uint32_t uiTxPos   = 0u;                            ///< Current write position
        uint32_t uiRxPos   = 0u;                            ///< Current read position
    };

public:
    /**
     * @brief Packet header
     * @details The packet header is placed in front of every packet and describes the packet type as well as the size of the
     * packet.
     * @remarks Packets are always starting at a 64-bit boundary.
     */
    struct SPacketHdr
    {
        /**
         * @brief Packet types
         */
        enum class EType : uint16_t
        {
            data	 = 0, ///< The packet contains data
            stuffing = 1, ///< The packet doesn't contain data and is used to fill up space
        } eType;		  ///< Packet type

        /**
         * @brief Packet state
         */
        enum class EState : uint16_t
        {
            free	 = 0, ///< Packet is not reserved; overwriting is allowed
            reserved = 1, ///< Space is reserved but not committed.
            commit	 = 2, ///< Packet is committed and available for reading.
            read     = 3, ///< Packet is currently being read.
        } eState;	///< The packet state

        uint32_t uiSize; ///< Packet size
    };

    /**
     * @brief Constructor
     */
    CMemBufferAccessorBase() = default;

    /**
     * @brief Attach the buffer to the accessor.
     * @param[in] pBuffer Pointer to the buffer that should be accessed.
     * @param[in] uiSize Size of the buffer when still uninitialized (during buffer creation); otherwise 0.
     */
    virtual void Attach(uint8_t* pBuffer, uint32_t uiSize = 0);

    /**
     * @brief Detach the buffer from the accessor.
     */
    virtual void Detach();

    /**
     * @brief Is valid.
     * @return Returns 'true' when valid; otherwise 'false'.
     */
    bool IsValid() const;

    /**
     * @brief Get the buffer pointer.
     * @return Returns a pointer to the buffer or NULL when there is no buffer.
     */
    const uint8_t* GetBufferPointer() const;

    /**
     * @brief Reset the Rx position to skip any sent data. Typically needed when a reconnect should take place.
     */
    void ResetRx() { if (m_pHdr) m_pHdr->uiRxPos = m_pHdr->uiTxPos; m_bCancel = false; }

    /**
     * @brief Cancel send operation.
     */
    void CancelSend() { m_bCancel = true; TriggerDataReceive(); std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

    /**
     * @brief Canceled?
     * @return Returns 'true' when the currend send job has been canceled; 'false' when not.
     */
    bool Canceled() const { return m_bCancel; }

protected:
    /**
     * @brief Providing the position, return the packet header preceding the data.
     * @param[in] uiPos The position in the buffer
     * @return Pointer to the packet header or NULL when the position isn't within the buffer.
     */
    SPacketHdr* GetPacketHdr(uint32_t uiPos) const;

    /**
     * @brief Providing the position, return the packet data following the header.
     * @param[in] uiPos The position in the buffer
     * @return Pointer to the packet data or NULL when the position isn't within the buffer.
     */
    uint8_t* GetPacketData(uint32_t uiPos) const;

    /**
     * @brief Has unread data.
     * @return Returns 'true' when unread data is still available; otherwise returns 'false'.
     */
    virtual bool HasUnreadData() const { return m_pHdr && m_pHdr->uiRxPos != m_pHdr->uiTxPos; }

    /**
     * @brief Trigger listener that a write operation was completed.
     */
    virtual void TriggerDataSend() = 0;

    /**
     * @brief Wait for a write operation to be completed.
     * @param[in] uiTimeoutMs The amount of time (in ms) to wait for a trigger.
     * @return Returns 'true' when data was stored, 'false' when a timeout occurred.
     */
    virtual bool WaitForData(uint32_t uiTimeoutMs) const = 0;

    /**
     * @brief Trigger listener that a read operation was completed.
     */
    virtual void TriggerDataReceive() = 0;

    /**
     * @brief Wait for a read operation to be completed.
     * @param[in] uiTimeoutMs The amount of time (in ms) to wait for a trigger.
     * @return Returns 'true' when data was stored, 'false' when a timeout occurred.
     */
    virtual bool WaitForFreeSpace(uint32_t uiTimeoutMs) const = 0;

    /**
     * @brief Align to 64 bits
     * @tparam T The type of the size argument.
     * @param[in] tSize The current size.
     * @return The 64-bit aligned size.
     */
    template <typename T>
    inline uint32_t Align(T tSize)
    {
        uint32_t uiSize = static_cast<uint32_t>(tSize);
        return (uiSize % 8) ? uiSize + 8 - uiSize % 8 : uiSize;
    }

    uint8_t*    m_pBuffer = nullptr;    ///< Buffer pointer
    SBufferHdr* m_pHdr  = nullptr;      ///< Buffer header
    bool        m_bCancel = false;      ///< Cancel the send operation
};

// Forward declaration
class CMemBufferAccessorTx;

/**
 * @brief Tx access can be done randomly. To prevent shutting down while packets are reserved but not committed, the accessor
 * packet take over the management of this.
 */
class CAccessorTxPacket
{
public:
    /**
     * @brief Default constructor
     */
    CAccessorTxPacket() = default;

    /**
     * @brief Constructor
     * @param[in] rAccessor Reference to the accessor managing the buffer.
     * @param[in] pPacketHdr Pointer to the packet header.
     */
    CAccessorTxPacket(CMemBufferAccessorTx& rAccessor, CMemBufferAccessorBase::SPacketHdr* pPacketHdr);

    /**
     * @brief Copy constructor is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     */
    CAccessorTxPacket(const CAccessorTxPacket& rpacket) = delete;

    /**
     * @brief Move constructor
     * @param[in] rpacket Reference to the packet to move from.
     */
    CAccessorTxPacket(CAccessorTxPacket&& rpacket) noexcept;

    /**
     * @brief Destructor - will automatically commit the packet if not done so already.
     */
    ~CAccessorTxPacket();

    /**
     * @brief Copy operator is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     * @return Reference to this packet class.
     */
    CAccessorTxPacket& operator=(const CAccessorTxPacket& rpacket) = delete;

    /**
     * @brief Copy operator is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     * @return Reference to this packet class.
     */
    CAccessorTxPacket& operator=(CAccessorTxPacket&& rpacket) noexcept;

    /**
     * @brief Cast operator used to check validity.
     */
    operator bool() const;

    /**
     * @brief Does the packet contain valid data (data is not committed yet).
     * @return The validity of the packet.
     */
    bool IsValid() const;

    /**
     * @brief Commit the data. This will invalidate the content.
    */
    void Commit();

    /**
     * @brief Get the size of the packet data.
     * @return The packet size; could be 0 when no data was allocated.
     */
    uint32_t GetSize() const;

    /**
     * @brief Get the packet pointer.
     * @return Pointer to the buffer holding the packet data or nullptr when there is no data allocated or the packet has been
     * committed.
     */
    uint8_t* GetDataPtr();

    /**
     * @brief Templated version of GetData.
     * @tparam TData The type to cast the data pointer to.
     * @param[in] uiOffset The offset to use in bytes (default no offset).
     * @return Pointer to the data or NULL when the data doesn't fit in the buffer.
     */
    template <typename TData>
    TData* GetDataPtr(uint32_t uiOffset = 0)
    {
        return GetSize() >= (uiOffset + sizeof(TData)) ? reinterpret_cast<TData*>(GetDataPtr()) : nullptr;
    }

private:
    CMemBufferAccessorTx*               m_pAccessor = nullptr;      ///< Pointer to the accessor.
    CMemBufferAccessorBase::SPacketHdr* m_pPacketHdr = nullptr;     ///< Packet header.
};

/**
 * @brief Accessor TX implementation
 */
class CMemBufferAccessorTx : public CMemBufferAccessorBase
{
    friend CAccessorTxPacket;

public:
    /**
     * @brief Destructor
     */
    ~CMemBufferAccessorTx();

    /**
     * @brief Reserve memory for writing to the buffer without an extra copy.
     * @param[in] uiSize Size of the memory block to reserve. Must be smaller than the buffer size.
     * @param[in] uiTimeoutMs The amount of time (in ms) to wait for a free buffer. Default 1000 ms (needed when allocating large
     * chunks of data that need to be allocated on the receiving side).
     * @return Returns a packet object if successful or an empty option when a timeout occurred.
     * @attention The reader will only be able to continue reading after a commit. If, for some reason, there is no commit, the
     * reader won't be able to retrieve any more data.
     * @attention Performance is poor if the size is close to the buffer size. As a rule, uiSize should be at the most 1/4th of
     * the buffer size.
     */
    std::optional<CAccessorTxPacket> Reserve(uint32_t uiSize, uint32_t uiTimeoutMs = 1000);

    /**
     * @brief Write data to the buffer.
     * @param[in] pData Pointer to the data to write.
     * @param[in] uiSize Length of the data in bytes.
     * @return Returns 'true' if writing was successful or 'false' if the packet was invalid or (currently) doesn't fit into the
     * buffer.
     */
    bool TryWrite(const void* pData, uint32_t uiSize);

    /**
     * @brief Get the accessor access type.
     * @return The access type of this accessor.
     */
    static constexpr EAccessType GetAccessType() { return EAccessType::tx; };

private:
    /**
     * @brief Commit the writing to the buffer.
     * @param[in] pPacketHdr Pointer to the previously reserved packet header.
     */
    void Commit(SPacketHdr* pPacketHdr);

    bool                    m_bBlockReserve = false;    ///< When set, do not reserve any more packets.
    std::mutex              m_mtxReservedPackes;        ///< Access protection for the reserved access queue.
    std::deque<SPacketHdr*> m_queueReservedPackets;     ///< Queue containing the currently reserved packets.
};

// Forward declaration
class CMemBufferAccessorRx;

/**
 * @brief Rx access can be done randomly. To ensure the the data is released properly, the accessor packet is returned to manage
 * this.
 */
class CAccessorRxPacket
{
public:
    /**
     * @brief Default constructor
     */
    CAccessorRxPacket() = default;

    /**
     * @brief Constructor
     * @param[in] rAccessor Reference to the accessor managing the buffer.
     * @param[in] pPacketHdr Pointer to the packet header.
     */
    CAccessorRxPacket(CMemBufferAccessorRx& rAccessor, CMemBufferAccessorBase::SPacketHdr* pPacketHdr);

    /**
     * @brief Copy constructor is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     */
    CAccessorRxPacket(const CAccessorRxPacket& rpacket) = delete;

    /**
     * @brief Move constructor
     * @param[in] rpacket Reference to the packet to move from.
     */
    CAccessorRxPacket(CAccessorRxPacket&& rpacket) noexcept;

    /**
     * @brief Copy operator is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     * @return Reference to this packet class.
     */
    CAccessorRxPacket& operator=(const CAccessorRxPacket& rpacket) = delete;

    /**
     * @brief Copy operator is deleted.
     * @param[in] rpacket Reference to the packet to copy from.
     * @return Reference to this packet class.
     */
    CAccessorRxPacket& operator=(CAccessorRxPacket&& rpacket) noexcept;

    /**
     * @brief Cast operator used to check validity.
     */
    operator bool() const;

    /**
     * @brief Does the packet contain valid data (data pointer is available and the size is not zero).
     * @return The validity of the packet.
     */
    bool IsValid() const;

    /**
     * @brief Reset the packet (not accepting the content). Packet will be available again for next request.
     */
    void Reset();

    /**
     * @brief Get the size of the packet data.
     * @return The packet size.
     */
    uint32_t GetSize() const;

    /**
     * @brief Get the packet pointer.
     * @return Pointer to the buffer holding the packet data or nullptr when the packet is cleared.
     */
    const uint8_t* GetData() const;

    /**
     * @brief Templated version of GetData.
     * @tparam TData The type to cast the data pointer to.
     * @param[in] uiOffset The offset to use in bytes (default no offset).
     * @return Pointer to the data or NULL when the data doesn't fit in the buffer.
     */
    template <typename TData>
    const TData* GetData(uint32_t uiOffset = 0) const
    {
        return GetSize() >= (uiOffset + sizeof(TData)) ? reinterpret_cast<const TData*>(GetData()) : nullptr;
    }

    /**
     * @brief Accept the packet; packet will be released for new packet data.
     */
    void Accept();

private:
    CMemBufferAccessorRx*               m_pAccessor = nullptr;      ///< Pointer to the accessor.
    CMemBufferAccessorBase::SPacketHdr* m_pPacketHdr = nullptr;     ///< Packet header.
};

/**
 * @brief Accessor RX implementation
 */
class CMemBufferAccessorRx : public CMemBufferAccessorBase
{
    friend CAccessorRxPacket;
public:
    // Suppress cppcheck warning about a useless override. The function is here for clearer code documentation.
    // cppcheck-suppress uselessOverride
    /**
     * @brief Attach the buffer to the accessor. Overload of CMemBufferAccessorBase::Attach.
     * @param[in] pBuffer Pointer to the buffer that should be accessed.
     * @param[in] uiSize Size of the buffer when still uninitialized (during buffer creation); otherwise 0.
     */
    virtual void Attach(uint8_t* pBuffer, uint32_t uiSize = 0) override;

    /**
     * @brief Get a packet. If the packet is accepted, call release before the packet
     * @return Returns 'true' when the access request was successful or 'false' when currently there is no data.
     * @attention The writer will only be able to overwrite previously read data after it is released. If, for some reason there is
     * no release, the writer won't be able to write past the accessed packet.
     */
    std::optional<CAccessorRxPacket> TryRead();

    /**
     * @brief Get the accessor access type.
     * @return The access type of this accessor.
     */
    static constexpr EAccessType GetAccessType() { return EAccessType::rx; };

protected:
    /**
     * @brief Release any read packets and update the read pointer.
     */
    void ReleasePackets();

private:
    std::mutex          m_mtxReadAccess;            ///< Synchronize read access.
};

#endif // !defined MEM_BUFFER_ACCESSOR_H