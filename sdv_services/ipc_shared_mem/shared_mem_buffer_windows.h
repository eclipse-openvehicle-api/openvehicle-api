#if !defined WINDOWS_SHARED_MEM_BUFFER_H && defined _WIN32
#define WINDOWS_SHARED_MEM_BUFFER_H

#include <cassert>

// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

#include <support/string.h>
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
     * @return Reference to the error string.
     */
    const std::string& GetError() const { return m_ssError; }

    /**
     * @brief Get the size of the buffer.
     * @return Returns the size of the buffer.
     */
    uint32_t GetSize() const { return m_uiSize; }

    /**
     * @brief Get the name of the buffer.
     * @return Reference to the string holding the name of the buffer.
     */
    const std::string& GetName() const { return m_ssName; }

private:
    uint32_t        m_uiSize = 0u;                      ///< Size of the shared memory buffer.
    HANDLE          m_hMapFile = INVALID_HANDLE_VALUE;  ///< Handle to the shared memory buffer.
    std::string     m_ssName;                           ///< Name of the shared memory.
    uint8_t*        m_pBuffer = nullptr;                ///< Pointer to the mapped buffer.
    std::string     m_ssSyncTx;                         ///< Name of the signalling event.
    HANDLE          m_hSignalTx = INVALID_HANDLE_VALUE; ///< Handle to the signalling event.
    std::string     m_ssSyncRx;                         ///< Name of the signalling event.
    HANDLE          m_hSignalRx = INVALID_HANDLE_VALUE; ///< Handle to the signalling event.
    std::string     m_ssError;                          ///< The last reported error.
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
    bool bServer /*= true*/) : m_uiSize(bServer ? (uiSize ? uiSize : 128 * 1024) : 0)
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
    std::string ssNamePath = /*"Global\\" +*/ m_ssName;
    std::string ssSyncTxPath = /*"Global\\" +*/ m_ssSyncTx;
    std::string ssSyncRxPath = /*"Global\\" +*/ m_ssSyncRx;

    auto fnReportWin32Error = [this]()
    {
        TCHAR* szMsg = nullptr;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPTSTR) &szMsg,
            0,
            NULL);
        m_ssError += sdv::MakeUtf8String(szMsg);
        LocalFree(szMsg);
    };

    auto fnCloseAll = [this]()
    {
        if (m_pBuffer) UnmapViewOfFile(m_pBuffer);
        if (m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE) CloseHandle(m_hMapFile);
        if (m_hSignalTx && m_hSignalTx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalTx);
        if (m_hSignalRx && m_hSignalRx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalRx);
        m_pBuffer = 0;
        m_hMapFile = INVALID_HANDLE_VALUE;
        m_hSignalTx = INVALID_HANDLE_VALUE;
        m_hSignalRx = INVALID_HANDLE_VALUE;
    };

    // Create sync event object
    m_hSignalTx = CreateEventA(nullptr, FALSE, FALSE, ssSyncTxPath.c_str());
    if (!m_hSignalTx || m_hSignalTx == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to create event " + ssSyncTxPath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Create sync event object
    m_hSignalRx = CreateEventA(nullptr, FALSE, FALSE, ssSyncRxPath.c_str());
    if (!m_hSignalRx || m_hSignalRx == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to create event " + ssSyncRxPath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    if (bServer)
    {
        // Create the file mapping object
        m_hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, // use paging file
            NULL,				 // default security
            PAGE_READWRITE,		 // read/write access
            0,					 // maximum object size (high-order DWORD)
            m_uiSize,		     // maximum object size (low-order DWORD)
            ssNamePath.c_str()); // name of mapping object
    } else
    {
        // Open the file mapping object
        m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,	// read/write access
            FALSE,					// do not inherit the name
            ssNamePath.c_str());		// name of mapping object

    }
    if (!m_hMapFile || m_hMapFile == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to access file mapping " + ssNamePath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Map the file into memory
    m_pBuffer = reinterpret_cast<uint8_t*>(MapViewOfFile(m_hMapFile,		  // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,					  // Offset high
        0,					  // Offset low
        m_uiSize));			  // Amount of bytes
    if (!m_pBuffer)
    {
        m_ssError = "Failed to create file mapping view: ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    if (!m_uiSize)
    {
        // Request the size of the mapping
        MEMORY_BASIC_INFORMATION sMemInfo{};
        auto nRet = VirtualQuery(m_pBuffer, &sMemInfo, sizeof(sMemInfo));
        if (nRet != sizeof(sMemInfo) || !sMemInfo.RegionSize)
        {
            m_ssError = "Failed to request mapping view size: ";
            fnReportWin32Error();
            fnCloseAll();
            return;
        }
        m_uiSize = static_cast<uint32_t>(sMemInfo.RegionSize);
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
    std::string ssPath = /*"Global\\" +*/ m_ssName;
    std::string ssSyncTxPath = /*"Global\\" +*/ m_ssSyncTx;
    std::string ssSyncRxPath = /*"Global\\" +*/ m_ssSyncRx;

    auto fnReportWin32Error = [this]()
    {
        TCHAR* szMsg = nullptr;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      GetLastError(),
                      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                      (LPTSTR)&szMsg,
                      0,
                      NULL);
        m_ssError += sdv::MakeUtf8String(szMsg);
        LocalFree(szMsg);
    };

    auto fnCloseAll = [this]()
    {
        if (m_pBuffer) UnmapViewOfFile(m_pBuffer);
        if (m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE) CloseHandle(m_hMapFile);
        if (m_hSignalTx && m_hSignalTx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalTx);
        if (m_hSignalRx && m_hSignalRx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalRx);
        m_pBuffer = 0;
        m_hMapFile = INVALID_HANDLE_VALUE;
        m_hSignalTx = INVALID_HANDLE_VALUE;
        m_hSignalRx = INVALID_HANDLE_VALUE;
    };

    // Create TX sync event object
    m_hSignalTx = CreateEventA(nullptr, FALSE, FALSE, ssSyncTxPath.c_str());
    if (!m_hSignalTx || m_hSignalTx == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to create event " + ssSyncTxPath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Create RX sync event object
    m_hSignalRx = CreateEventA(nullptr, FALSE, FALSE, ssSyncRxPath.c_str());
    if (!m_hSignalRx || m_hSignalRx == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to create event " + ssSyncRxPath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Open the file mapping object
    m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,	// read/write access
                                 FALSE,					// do not inherit the name
                                 ssPath.c_str());		// name of mapping object
    if (!m_hMapFile || m_hMapFile == INVALID_HANDLE_VALUE)
    {
        m_ssError = "Failed to open file mapping " + ssPath + ": ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Map the file into memory
    m_pBuffer = reinterpret_cast<uint8_t*>(MapViewOfFile(m_hMapFile,		  // handle to map object
                                                         FILE_MAP_ALL_ACCESS, // read/write permission
                                                         0,					  // Offset high
                                                         0,					  // Offset low
                                                         0));		          // Amount of bytes (all in this case)
    if (!m_pBuffer)
    {
        m_ssError = "Failed to create file mapping view: ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }

    // Request the size of the mapping
    MEMORY_BASIC_INFORMATION sMemInfo{};
    auto nRet = VirtualQuery(m_pBuffer, &sMemInfo, sizeof(sMemInfo));
    if (nRet != sizeof(sMemInfo) || !sMemInfo.RegionSize)
    {
        m_ssError = "Failed to request mapping view size: ";
        fnReportWin32Error();
        fnCloseAll();
        return;
    }
    m_uiSize = static_cast<uint32_t>(sMemInfo.RegionSize);

    TAccessor::Attach(m_pBuffer);

    TRACE("Opened shared memory for ", m_ssSyncTx, " and ", m_ssSyncRx, ".");
}

template <class TAccessor>
CSharedMemBuffer<TAccessor>::~CSharedMemBuffer()
{
    if (m_pBuffer) UnmapViewOfFile(m_pBuffer);
    if (m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE) CloseHandle(m_hMapFile);
    if (m_hSignalTx && m_hSignalTx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalTx);
    if (m_hSignalRx && m_hSignalRx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalRx);
}

template <class TAccessor>
void CSharedMemBuffer<TAccessor>::Detach()
{
    if (m_pBuffer) UnmapViewOfFile(m_pBuffer);
    if (m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE) CloseHandle(m_hMapFile);
    if (m_hSignalTx && m_hSignalTx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalTx);
    if (m_hSignalRx && m_hSignalRx != INVALID_HANDLE_VALUE) CloseHandle(m_hSignalRx);
    m_uiSize = 0u;
    m_hMapFile = INVALID_HANDLE_VALUE;
    m_ssName.clear();
    m_pBuffer = nullptr;
    m_ssSyncTx.clear();
    m_hSignalTx = INVALID_HANDLE_VALUE;
    m_ssSyncRx.clear();
    m_hSignalRx = INVALID_HANDLE_VALUE;
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
    if (!m_hSignalTx || m_hSignalTx == INVALID_HANDLE_VALUE) return;
    SetEvent(m_hSignalTx);
}

template <class TAccessor>
inline bool CSharedMemBuffer<TAccessor>::WaitForData(uint32_t uiTimeoutMs) const
{
    if (!m_hSignalTx || m_hSignalTx == INVALID_HANDLE_VALUE) return false;

    // Check whether there is data; if so, return true.
    if (TAccessor::HasUnreadData())
        return true;

    return WaitForSingleObject(m_hSignalTx, uiTimeoutMs) == WAIT_OBJECT_0;
}

template <class TAccessor>
inline void CSharedMemBuffer<TAccessor>::TriggerDataReceive()
{
    if (!m_hSignalRx || m_hSignalRx == INVALID_HANDLE_VALUE) return;
    SetEvent(m_hSignalRx);
}

template <class TAccessor>
inline bool CSharedMemBuffer<TAccessor>::WaitForFreeSpace(uint32_t uiTimeoutMs) const
{
    if (!m_hSignalRx || m_hSignalRx == INVALID_HANDLE_VALUE) return false;

    DWORD dwResult = WaitForSingleObject(m_hSignalRx, uiTimeoutMs);
    if (TAccessor::Canceled())
        return false;
    return dwResult == WAIT_OBJECT_0;
}

#endif // !defined WINDOWS_SHARED_MEM_BUFFER_H
