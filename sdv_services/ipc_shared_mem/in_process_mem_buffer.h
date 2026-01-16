#ifndef IN_PROCESS_MEM_BUFFER_H
#define IN_PROCESS_MEM_BUFFER_H

#include <memory>
#include <cassert>
#include <string>
#include <condition_variable>
#include <support/toml.h>
#include "mem_buffer_accessor.h"

/**
 * @brief In-process memory buffer.
 */
template <class TAccessor>
class CInProcMemBuffer : public TAccessor
{

public:
    /**
     * @brief Default constructor
     * @param[in] uiSize Size of the buffer (default is 1 MByte). Must not be zero.
     */
    CInProcMemBuffer(uint32_t uiSize = 1048576);

    /**
     * @brief Connection constructor
     * @param[in] rssConnectionString Reference to string with connection information.
     */
    CInProcMemBuffer(const std::string& rssConnectionString);

    /**
     * @brief Default destructor
     */
    ~CInProcMemBuffer() = default;

    /**
     * @brief Return the connection string.
     * @return The connection string to connect to this buffer.
     */
    std::string GetConnectionString() const;

    /**
     * @brief Trigger listener that a write operation was completed.
     */
    void TriggerDataSend() override;

    /**
     * @brief Wait for a write operation to be completed.
     * @param[in] uiTimeoutMs The amount of time (in ms) to wait for a trigger.
     * @return Returns 'true' when data was stored, 'false' when a timeout occurred.
     */
    bool WaitForData(uint32_t uiTimeoutMs) const override;

    /**
    * @brief Trigger listener that a read operation was completed.
    */
    void TriggerDataReceive() override;

    /**
    * @brief Wait for a read operation to be completed.
    * @param[in] uiTimeoutMs The amount of time (in ms) to wait for a trigger.
    * @return Returns 'true' when data was stored, 'false' when a timeout occurred.
    */
    bool WaitForFreeSpace(uint32_t uiTimeoutMs) const override;

private:
    /**
     * @brief Synchronization object
    */
    struct SSync : public std::enable_shared_from_this<SSync>
    {
        std::mutex				mtx;            ///< Mutex to synchronize.
        std::condition_variable cv;             ///< Condition variable to signal.
    };
    std::shared_ptr<SSync>	    m_ptrSyncTx;    ///< Shared pointer to the TX synchronization object.
    std::shared_ptr<SSync>	    m_ptrSyncRx;    ///< Shared pointer to the RX synchronization object.
    std::unique_ptr<uint8_t[]>  m_ptrBuffer;    ///< Smart pointer to the buffer (only for the allocator).
    uint32_t                    m_uiSize = 0;   ///< The size of the buffer.
    std::string                 m_ssError;      ///< The last reported error.
};

/**
 * @brief Inproc memory buffer used for reading.
 */
using CInProcMemBufferRx = CInProcMemBuffer<CMemBufferAccessorRx>;

/**
 * @brief Inproc memory buffer used for writing.
 */
using CInProcMemBufferTx = CInProcMemBuffer<CMemBufferAccessorTx>;

template <class TAccessor>
inline CInProcMemBuffer<TAccessor>::CInProcMemBuffer(uint32_t uiSize) :
    m_ptrSyncTx(new SSync), m_ptrSyncRx(new SSync), m_ptrBuffer(std::unique_ptr<uint8_t[]>(new uint8_t[uiSize])), m_uiSize(uiSize)
{
    assert(uiSize);
    if (!uiSize) return;
    TAccessor::Attach(m_ptrBuffer.get(), uiSize);
}

template <class TAccessor>
inline CInProcMemBuffer<TAccessor>::CInProcMemBuffer(const std::string& rssConnectionString)
{
    if (rssConnectionString.empty())
    {
        m_ssError = "Missing connection string.";
        return;
    }

    // Interpret the connection string
    sdv::toml::CTOMLParser config(rssConnectionString);

    // The connection string can contain multiple parameters. Search for the first parameters fitting the accessor direction
    size_t nIndex = 0;
    sdv::toml::CNodeCollection nodeConnectParamCollection = config.GetDirect("ConnectParam");
    uint64_t uiLocation = 0ull, uiSyncTx = 0ull, uiSyncRx = 0ull;
    do
    {
        sdv::toml::CNodeCollection nodeConnectParam;
        switch (nodeConnectParamCollection.GetType())
        {
        case sdv::toml::ENodeType::node_array:
            if (nIndex >= nodeConnectParamCollection.GetCount()) break;
            nodeConnectParam = nodeConnectParamCollection[nIndex];
            break;
        case sdv::toml::ENodeType::node_table:
            if (nIndex > 0) break;
            nodeConnectParam = nodeConnectParamCollection;
            break;
        default:
            break;
        }
        if (nodeConnectParam.GetType() != sdv::toml::ENodeType::node_table) break;

        nIndex++;

        // Check for shared memory
        if (nodeConnectParam.GetDirect("Type").GetValue() != "inproc_mem") continue;

        // Check the direction
        if (nodeConnectParam.GetDirect("Direction").GetValue() !=
            (TAccessor::GetAccessType() == EAccessType::rx ? "response" : "request"))
            continue;

        // Get the information
        uiLocation = nodeConnectParam.GetDirect("Location").GetValue();
        m_uiSize = static_cast<uint32_t>(nodeConnectParam.GetDirect("Size").GetValue());
        uiSyncTx = nodeConnectParam.GetDirect("SyncTx").GetValue();
        uiSyncRx = nodeConnectParam.GetDirect("SyncRx").GetValue();
        break;
    } while (true);
    if (!uiLocation || !uiSyncTx || !uiSyncRx)
    {
        m_ssError = "Incomplete connection information.";
        return;
    }

    m_ptrSyncTx = reinterpret_cast<SSync*>(uiSyncTx)->shared_from_this();
    m_ptrSyncRx = reinterpret_cast<SSync*>(uiSyncRx)->shared_from_this();
    TAccessor::Attach(reinterpret_cast<uint8_t*>(uiLocation));
}

template <class TAccessor>
inline std::string CInProcMemBuffer<TAccessor>::GetConnectionString() const
{
    // The connection string contains the TOML file for connecting to this shared memory.
    std::stringstream sstream;
    sstream << "[[ConnectParam]]" << std::endl;
    sstream << "Type = \"inproc_mem\"" << std::endl;
    sstream << "Location = " << reinterpret_cast<uint64_t>(TAccessor::GetBufferPointer()) << std::endl;
    sstream << "Size = " << m_uiSize << std::endl;
    sstream << "SyncTx = " << reinterpret_cast<uint64_t>(m_ptrSyncTx.get()) << std::endl;
    sstream << "SyncRx = " << reinterpret_cast<uint64_t>(m_ptrSyncRx.get()) << std::endl;
    // The target direction is the opposite of the direction of the accessor. Therefore, if the accessor uses an RX access type,
    // the target uses an TX access type and should be configured as response, otherwise it is a request.
    sstream << "Direction = \"" << (TAccessor::GetAccessType() == EAccessType::rx ? "request" : "response") << "\"" << std::endl;
    return sstream.str();
}

template <class TAccessor>
inline void CInProcMemBuffer<TAccessor>::TriggerDataSend()
{
    if (!m_ptrSyncTx) return;
    std::unique_lock lock(m_ptrSyncTx->mtx);
    m_ptrSyncTx->cv.notify_all();
}

template <class TAccessor>
inline bool CInProcMemBuffer<TAccessor>::WaitForData(uint32_t uiTimeoutMs) const
{
    if (!m_ptrSyncTx) return false;

    // Check whether there is data; if so, return true.
    if (TAccessor::HasUnreadData()) return true;

    std::unique_lock lock(m_ptrSyncTx->mtx);
    return m_ptrSyncTx->cv.wait_for(lock, std::chrono::milliseconds(uiTimeoutMs)) == std::cv_status::no_timeout;
}

template <class TAccessor>
inline void CInProcMemBuffer<TAccessor>::TriggerDataReceive()
{
    if (!m_ptrSyncRx) return;
    std::unique_lock lock(m_ptrSyncRx->mtx);
    m_ptrSyncRx->cv.notify_all();
}

template <class TAccessor>
inline bool CInProcMemBuffer<TAccessor>::WaitForFreeSpace(uint32_t uiTimeoutMs) const
{
    if (!m_ptrSyncRx) return false;

    std::unique_lock lock(m_ptrSyncRx->mtx);
    if (TAccessor::Canceled())
        return false;
    return m_ptrSyncRx->cv.wait_for(lock, std::chrono::milliseconds(uiTimeoutMs)) == std::cv_status::no_timeout;
}

#endif // !defined(IN_PROCESS_MEM_BUFFER_H)