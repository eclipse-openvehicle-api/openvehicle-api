#if !defined POSIX_SHARED_MEM_BUFFER_H && defined __unix__
#define POSIX_SHARED_MEM_BUFFER_H

#include <cassert>

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <support/toml.h>
#include "../../global/trace.h"
#include "mem_buffer_accessor.h"

/**
 * @brief In-process memory buffer.
 */
template <class TAccessor>
class CSharedMemBuffer : public TAccessor
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiSize Optional size of the buffer. If zero, a default buffer size of 10k is configured.
     * @param[in] rssName Optional name to be used for the connection. If empty, a random name is generated.
     * @param[in] bServer Optional boolean indicating whether the connection is a server (true), which initiates the connection, or
     * a client (false), which opens an existing connection.
     */
    CSharedMemBuffer(uint32_t uiSize = 0, const std::string& rssName = std::string(), bool bServer = true);

    /**
     * @brief Connection constructor
     * @param[in] rssConnectionString Reference to string with connection information.
     */
    CSharedMemBuffer(const std::string& rssConnectionString);

    /** No copy constructor */
    CSharedMemBuffer(const CSharedMemBuffer&) = delete;

    /** No move constructor */
    CSharedMemBuffer(CSharedMemBuffer&&) = delete;

    /**
     * @brief Default destructor
     */
    ~CSharedMemBuffer();

    /** No copy-assignment operator */
    CSharedMemBuffer& operator=(const CSharedMemBuffer&) = delete;

    /** No move-assignment operator */
    CSharedMemBuffer& operator=(CSharedMemBuffer&&) = delete;

    /**
     * @brief Detach the buffer. Overload of CMemBufferAccessorBase::Detach.
     * @details The detach function detaches the shared memory without deleting the memory. This keeps the memory alive for reuse.
     */
    virtual void Detach() override;

    /**
     * @brief Return the connection string to connect to this shared memory.
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

    /**
     * @brief Return the last reported error.
     * @return Error string.
     */
    std::string GetError() const { return m_ssError; }

    /**
    * @brief Get the size of the buffer.
    * @return Returns the size of the buffer.
    */
    uint32_t GetSize() const { return m_uiSize; }

    /**
    * @brief Get the name of the buffer.
    * @return Returns the name of the buffer.
    */
    std::string GetName() const { return m_ssName; }

private:
    uint32_t        m_uiSize = 0u;                  ///< Size of the shared memory buffer.
    int             m_iFileDescr = 0;               ///< File descriptor of the shared memory.
    std::string     m_ssName;                       ///< Name of the shared memory.
    uint8_t*        m_pBuffer = nullptr;            ///< Pointer to the mapped buffer.
    std::string     m_ssSyncTx;                     ///< Name of the signalling event.
    sem_t*          m_pSemaphoreTx = nullptr;       ///< Semaphore to trigger written.
    std::string     m_ssSyncRx;                     ///< Name of the signalling event.
    sem_t*          m_pSemaphoreRx = nullptr;       ///< Semaphore to trigger written.
    std::string     m_ssError;                      ///< The last reported error.
    bool            m_bServer = false;              ///< Set when the shared memory is configured as server. Otherwise as client.
};

/**
 * @brief Shared memory buffer used for reading.
 */
using CSharedMemBufferRx = CSharedMemBuffer<CMemBufferAccessorRx>;

/**
 * @brief Shared memory buffer used for writing.
 */
using CSharedMemBufferTx = CSharedMemBuffer<CMemBufferAccessorTx>;

template <class TAccessor>
inline CSharedMemBuffer<TAccessor>::CSharedMemBuffer(uint32_t uiSize /*= 0*/, const std::string& rssName /*= std::string()*/,
    bool bServer /*= true*/) : m_uiSize(bServer ? (uiSize ? uiSize : 128 * 1024) : 0), m_bServer(bServer)
{
    // Create a name to be used in the connection string
    std::string ssDirectionString;
    if (bServer)
        ssDirectionString = TAccessor::GetAccessType() == EAccessType::rx ? "RESPONSE_" : "REQUEST_";
    else
        ssDirectionString = TAccessor::GetAccessType() == EAccessType::rx ? "REQUEST_" : "RESPONSE_";
    if (!rssName.empty())
    {
        m_ssName = std::string("SDV_SHARED_") + ssDirectionString + rssName;
        m_ssSyncTx = std::string("SDV_TX_SYNC_") + ssDirectionString + rssName;
        m_ssSyncRx = std::string("SDV_RX_SYNC_") + ssDirectionString + rssName;
    }
    else
    {
        uint64_t uiCnt = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        m_ssName = std::string("SDV_SHARED_") + ssDirectionString + std::to_string(uiCnt);
        m_ssSyncTx = std::string("SDV_TX_SYNC_") + ssDirectionString + std::to_string(uiCnt);
        m_ssSyncRx = std::string("SDV_RX_SYNC_") + ssDirectionString + std::to_string(uiCnt);
    }

    // Create a path
    std::string ssNamePath = "/" + m_ssName;
    // std::string ssSyncTxPath = "/" + m_ssSyncTx;
    // std::string ssSyncRxPath = "/" + m_ssSyncRx;

    // Unlink just in case the last server had crashed and the mapping still exists.
    if (m_bServer)
    {
        shm_unlink((std::string("/") + m_ssName).c_str());
        sem_unlink(m_ssSyncTx.c_str());
        sem_unlink(m_ssSyncRx.c_str());
    }

    // Initialize the semaphores
    if (bServer)
    {
        m_pSemaphoreTx = sem_open(m_ssSyncTx.c_str(), O_CREAT | O_EXCL, 0777 /*O_RDWR*/, 0);
        if (!m_pSemaphoreTx || m_pSemaphoreTx == SEM_FAILED)
        {
            m_ssError = "Failed to create new semaphore " + m_ssSyncTx + ".";
            return;
        }
        m_pSemaphoreRx = sem_open(m_ssSyncRx.c_str(), O_CREAT | O_EXCL, 0777 /*O_RDWR*/, 0);
        if (!m_pSemaphoreRx || m_pSemaphoreRx == SEM_FAILED)
        {
            m_ssError = "Failed to create new semaphore " + m_ssSyncRx + ".";
            return;
        }
    }
    else
    {
        m_pSemaphoreTx = sem_open(m_ssSyncTx.c_str(), 0);
        if (!m_pSemaphoreTx || m_pSemaphoreTx == SEM_FAILED)
        {
            m_ssError = "Failed to open existing semaphore " + m_ssSyncTx + ".";
            return;
        }
        m_pSemaphoreRx = sem_open(m_ssSyncRx.c_str(), 0);
        if (!m_pSemaphoreRx || m_pSemaphoreRx == SEM_FAILED)
        {
            m_ssError = "Failed to open existing semaphore " + m_ssSyncRx + ".";
            return;
        }
    }

    // Get shared memory file descriptor (NOT a file)
    if (bServer)
    {
        m_iFileDescr = shm_open(ssNamePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (m_iFileDescr == -1)
        {
            m_ssError = "Failed to create the shared memory file descriptor " + ssNamePath + ".";
            return;
        }

        // Extend shared memory object as by default it's initialized with size 0
        int iResult = ftruncate(m_iFileDescr, m_uiSize);
        if (iResult == -1)
        {
            m_ssError = "Failed to extend the shared memory.";
            return;
        }
    }
    else
    {
        m_iFileDescr = shm_open(ssNamePath.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
        if (m_iFileDescr == -1)
        {
            m_ssError = "Failed to open the shared memory file descriptor " + ssNamePath + ".";
            return;
        }

        // Get the size of the shared memory
        struct stat sMemInfo{};
        if (fstat(m_iFileDescr, &sMemInfo) == -1 || !sMemInfo.st_size)
        {
            m_ssError = "Failed to request the size of the shared memory file descriptor " + ssNamePath + ".";
            return;
        }
        m_uiSize = static_cast<uint32_t>(sMemInfo.st_size);
    }

    // map shared memory to process address space
    m_pBuffer = reinterpret_cast<uint8_t*>(mmap(NULL, m_uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_iFileDescr, 0));
    if (!m_pBuffer || m_pBuffer == MAP_FAILED)
    {
        m_ssError = "Failed to map the shared memory in process address space.";
        return;
    }

    // If this is a server, the size causes the initialization. For a client, no initialization should take place (the server has
    // done so already).
    TAccessor::Attach(m_pBuffer, bServer ? m_uiSize : 0);

    TRACE("Accessed shared memory for ", m_ssSyncTx, " and ", m_ssSyncRx, ".");
}

template <class TAccessor>
inline CSharedMemBuffer<TAccessor>::CSharedMemBuffer(const std::string& rssConnectionString)
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
        if (nodeConnectParam.GetDirect("Type").GetValue() != "shared_mem") continue;

        // Check the direction
        if (nodeConnectParam.GetDirect("Direction").GetValue() !=
            (TAccessor::GetAccessType() == EAccessType::rx ? "response" : "request"))
            continue;

        // Get the information
        m_ssName = static_cast<std::string>(nodeConnectParam.GetDirect("Location").GetValue());
        m_ssSyncTx = static_cast<std::string>(nodeConnectParam.GetDirect("SyncTx").GetValue());
        m_ssSyncRx = static_cast<std::string>(nodeConnectParam.GetDirect("SyncRx").GetValue());
        break;
    } while (true);
    if (m_ssName.empty() || m_ssSyncTx.empty() || m_ssSyncRx.empty())
    {
        m_ssError = "Incomplete connection information.";
        return;
    }

    // Create a path
    std::string ssPath	   = "/" + m_ssName;
    // std::string ssSyncTxPath = "/" + m_ssSyncTx;
    // std::string ssSyncRxPath = "/" + m_ssSyncRx;

    // Get shared memory file descriptor (NOT a file)
    m_iFileDescr = shm_open(ssPath.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (m_iFileDescr == -1)
    {
        m_ssError = "Failed to open the shared memory file descriptor " + ssPath + ".";
        return;
    }

    // Get the size of the shared memory
    struct stat sMemInfo{};
    if (fstat(m_iFileDescr, &sMemInfo) == -1 || !sMemInfo.st_size)
    {
        m_ssError = "Failed to request the size of the shared memory file descriptor " + ssPath + ".";
        return;
    }
    m_uiSize = static_cast<uint32_t>(sMemInfo.st_size);

    // Map shared memory to process address space
    m_pBuffer = reinterpret_cast<uint8_t*>(mmap(NULL, m_uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_iFileDescr, 0));
    if (!m_pBuffer || m_pBuffer == MAP_FAILED)
    {
        m_ssError = "Failed to map the shared memory in process address space.";
        return;
    }

    // Initialize the semaphore
    m_pSemaphoreTx = sem_open(m_ssSyncTx.c_str(), 0);
    if (!m_pSemaphoreTx || m_pSemaphoreTx == SEM_FAILED)
    {
        m_ssError = "Failed to open existing semaphore " + m_ssSyncTx + ".";
        return;
    }
    m_pSemaphoreRx = sem_open(m_ssSyncRx.c_str(), 0);
    if (!m_pSemaphoreRx || m_pSemaphoreRx == SEM_FAILED)
    {
        m_ssError = "Failed to open existing semaphore " + m_ssSyncRx + ".";
        return;
    }

    TAccessor::Attach(m_pBuffer);

    TRACE("Opened shared memory for ", m_ssSyncTx, " and ", m_ssSyncRx, ".");
}

template <class TAccessor>
CSharedMemBuffer<TAccessor>::~CSharedMemBuffer()
{
    // ATTENTION unmapping and unlinking will remove any connection to the shared memory within this process. When multiple
    // accessors are used, this will invalidate them immediately.
    if (m_pBuffer && m_pBuffer != MAP_FAILED)
        munmap(m_pBuffer, m_uiSize);
    if (m_bServer && !m_ssName.empty())
        shm_unlink((std::string("/") + m_ssName).c_str());
    if (m_iFileDescr >= 0) close(m_iFileDescr);
    if (m_bServer && m_pSemaphoreTx)
        sem_unlink(m_ssSyncTx.c_str());
    if (m_pSemaphoreTx)
        sem_close(m_pSemaphoreTx);
    if (m_bServer && m_pSemaphoreRx)
        sem_unlink(m_ssSyncRx.c_str());
    if (m_pSemaphoreRx)
        sem_close(m_pSemaphoreRx);
}

template <class TAccessor>
void CSharedMemBuffer<TAccessor>::Detach()
{
    m_uiSize = 0u;
    m_iFileDescr = 0;
    m_ssName.clear();
    m_pBuffer = nullptr;
    m_ssSyncTx.clear();
    m_pSemaphoreTx = nullptr;
    m_ssSyncRx.clear();
    m_pSemaphoreRx = nullptr;
    m_ssError.clear();
}

template <class TAccessor>
inline std::string CSharedMemBuffer<TAccessor>::GetConnectionString() const
{
    // The connection string contains the TOML file for connecting to this shared memory.
    std::stringstream sstream;
    sstream << "[[ConnectParam]]" << std::endl;
    sstream << "Type = \"shared_mem\"" << std::endl;
    sstream << "Location = \"" << m_ssName << "\"" << std::endl;
    sstream << "SyncTx = \"" << m_ssSyncTx << "\"" << std::endl;
    sstream << "SyncRx = \"" << m_ssSyncRx << "\"" << std::endl;
    // The target direction is the opposite of the direction of the accessor. Therefore, if the accessor uses an RX access type,
    // the target uses an TX access type and should be configured as response, otherwise it is a request.
    sstream << "Direction = \"" << (TAccessor::GetAccessType() == EAccessType::rx ? "request" : "response") << "\"" << std::endl;
    return sstream.str();
}

template <class TAccessor>
inline void CSharedMemBuffer<TAccessor>::TriggerDataSend()
{
    if (!m_pSemaphoreTx || m_pSemaphoreTx == SEM_FAILED)
        return;
    sem_post(m_pSemaphoreTx);
}

template <class TAccessor>
inline bool CSharedMemBuffer<TAccessor>::WaitForData(uint32_t uiTimeoutMs) const
{
    if (!m_pSemaphoreTx || m_pSemaphoreTx == SEM_FAILED)
        return false;

    // Check whether there is data; if so, return true.
    if (TAccessor::HasUnreadData())
        return true;

    // Get the time from the realtime clock
    timespec sTimespec{};
    if (clock_gettime(CLOCK_REALTIME, &sTimespec) == -1)
        return false;
    uint64_t uiTimeNs = sTimespec.tv_nsec + uiTimeoutMs * 1000000ull;
    sTimespec.tv_nsec = uiTimeNs % 1000000000ull;
    sTimespec.tv_sec += uiTimeNs / 1000000000ull;

    // Wait for the semaphore
    int iResult = sem_timedwait(m_pSemaphoreTx, &sTimespec);
    if (iResult < 0) return false;
    return true;
}

template <class TAccessor>
inline void CSharedMemBuffer<TAccessor>::TriggerDataReceive()
{
    if (!m_pSemaphoreRx || m_pSemaphoreRx == SEM_FAILED)
        return;
    sem_post(m_pSemaphoreRx);
}

template <class TAccessor>
inline bool CSharedMemBuffer<TAccessor>::WaitForFreeSpace(uint32_t uiTimeoutMs) const
{
    if (!m_pSemaphoreRx || m_pSemaphoreRx == SEM_FAILED)
        return false;

    // Get the time from the realtime clock
    timespec sTimespec{};
    if (clock_gettime(CLOCK_REALTIME, &sTimespec) == -1)
        return false;
    uint64_t uiTimeNs = sTimespec.tv_nsec + uiTimeoutMs * 1000000ull;
    sTimespec.tv_nsec = uiTimeNs % 1000000000ull;
    sTimespec.tv_sec += uiTimeNs / 1000000000ull;

    // Wait for the semaphore
    int iResult = sem_timedwait(m_pSemaphoreRx, &sTimespec);
    if (iResult < 0)
        return false;
    if (TAccessor::Canceled())
        return false;
    return true;
}

#endif // !defined POSIX_SHARED_MEM_BUFFER_H
