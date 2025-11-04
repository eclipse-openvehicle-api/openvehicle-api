#include "trace_fifo.h"

// Include the platform support
#define INCLUDE_TRACE_FIFO_PLATFORM
#include "trace_fifo_windows.cpp"
#include "trace_fifo_posix.cpp"
#undef INCLUDE_TRACE_FIFO_PLATFORM

#ifdef _WIN32
#include <io.h>
#pragma push_macro("O_TEXT")
#define O_TEXT _O_TEXT
#pragma push_macro("dup")
#define dup _dup
#pragma push_macro("dup2")
#define dup2 _dup2
#pragma push_macro("fileno")
#define fileno _fileno
#pragma push_macro("close")
#define close _close
#pragma push_macro("pipe")
#define pipe _pipe
#pragma push_macro("read")
#define read _read
#pragma push_macro("eof")
#define eof _eof
#else
#include <unistd.h>
#include <poll.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <chrono>

CTraceFifoBase::CTraceFifoBase(uint32_t uiInstanceID, size_t nSize) :
    m_uiInstanceID(uiInstanceID), m_nDefaultSize(nSize)
{}

CTraceFifoBase::~CTraceFifoBase()
{}

CTraceFifoBase::CTraceFifoBase(CTraceFifoBase&& rfifo) noexcept:
    m_uiInstanceID(rfifo.m_uiInstanceID), m_nSize(rfifo.m_nSize), m_pBuffer(rfifo.m_pBuffer), m_psHdr(rfifo.m_psHdr)
{
    rfifo.m_pBuffer = nullptr;
    rfifo.m_psHdr = nullptr;
}

CTraceFifoBase& CTraceFifoBase::operator=(CTraceFifoBase&& rfifo) noexcept
{
    Close();
    m_uiInstanceID = rfifo.m_uiInstanceID;
    m_nSize = rfifo.m_nSize;
    m_pBuffer = rfifo.m_pBuffer;
    m_psHdr = rfifo.m_psHdr;
    rfifo.m_pBuffer = nullptr;
    rfifo.m_psHdr = nullptr;

    return *this;
}

bool CTraceFifoBase::SetInstanceID(uint32_t uiInstanceID)
{
    if (IsOpened()) return false;
    m_uiInstanceID = uiInstanceID;
    return true;
}

uint32_t CTraceFifoBase::GetInstanceID() const
{
    return m_uiInstanceID;
}

void CTraceFifoBase::SetDefaultSize(size_t nSize)
{
    m_nDefaultSize = nSize;
}

size_t CTraceFifoBase::GetDefaultSize() const
{
    return m_nDefaultSize;
}

size_t CTraceFifoBase::GetViewSize() const
{
    return m_nSize;
}

size_t CTraceFifoBase::GetDataBufferSize() const
{
    return IsInitialized() ? m_nSize - sizeof(SSharedMemBufHeader) : 0;
}

void CTraceFifoBase::InitializeBuffer(void* pView, size_t nBufferSize, bool bReadOnly)
{
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    m_pBuffer = reinterpret_cast<uint8_t*>(pView);
    m_psHdr = reinterpret_cast<SSharedMemBufHeader*>(pView);
    m_nSize = nBufferSize;
    if (!IsInitialized())
    {
        if (!bReadOnly)
        {
            std::copy_n("SDV_MON\0", 8, m_psHdr->rgszSignature);
            m_psHdr->uiInstanceID = m_uiInstanceID;
            m_psHdr->uiTxOffs = 0u;
            m_bInitConfirmed = true;
        }
    }
}

bool CTraceFifoBase::IsInitialized() const
{
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());

    // Bypass of initialization?
    if (m_bInitConfirmed) return true;

    bool bRet = m_pBuffer && m_psHdr && m_psHdr->uiInstanceID == m_uiInstanceID &&
        std::equal(m_psHdr->rgszSignature, m_psHdr->rgszSignature + 8, "SDV_MON\0");
    m_bInitConfirmed = bRet;
    return bRet;
}

void CTraceFifoBase::Terminate()
{
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    m_pBuffer = nullptr;
    m_psHdr = nullptr;
    m_nSize = 0;
    m_bInitConfirmed = false;
}

#ifdef _MSC_VER
    // Static code analysis: not releasing lock in this function is not a failure.
    #pragma warning(push)
    #pragma warning(disable : 26115)
#endif

std::unique_lock<std::recursive_mutex> CTraceFifoBase::CreateAccessLockObject() const
{
    return std::unique_lock<std::recursive_mutex>(m_mtxAccess);
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

void* CTraceFifoBase::GetView()
{
    return IsInitialized() ? m_pBuffer : 0;
}

uint8_t* CTraceFifoBase::GetDataPtr()
{
    return IsInitialized() ? m_pBuffer + sizeof(SSharedMemBufHeader) : 0;
}

const uint8_t* CTraceFifoBase::GetDataPtr() const
{
    return IsInitialized() ? m_pBuffer + sizeof(SSharedMemBufHeader) : 0;
}

size_t CTraceFifoBase::GetWritePos() const
{
    return IsInitialized() ? m_psHdr->uiTxOffs : 0;
}

void CTraceFifoBase::SetWritePos(size_t nTxPos)
{
    if (IsInitialized() && nTxPos < GetDataBufferSize())
        m_psHdr->uiTxOffs = static_cast<uint32_t>(nTxPos);
}

CTraceFifoReader::CTraceFifoReader(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16384*/) :
    CTraceFifoImpl(uiInstanceID, nSize)
{}

CTraceFifoReader::~CTraceFifoReader()
{
    Close();
}

CTraceFifoReader::CTraceFifoReader(CTraceFifoReader&& rfifo) noexcept : CTraceFifoImpl(static_cast<CTraceFifoImpl&&>(rfifo)), m_nRxOffs(rfifo.m_nRxOffs)
{
    rfifo.m_nRxOffs = 0;
}

CTraceFifoReader& CTraceFifoReader::operator=(CTraceFifoReader&& rfifo) noexcept
{
    CTraceFifoImpl::operator=(static_cast<CTraceFifoImpl&&>(rfifo));
    rfifo.m_nRxOffs = 0;
    return *this;
}

bool CTraceFifoReader::Open(size_t nTimeout /*= 1000*/, uint32_t uiFlags /*= 0u*/)
{
    if (IsOpened()) return true;
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    bool bRet = CTraceFifoImpl::Open(nTimeout, uiFlags | static_cast<uint32_t>(ETraceFifoOpenFlags::read_only));
    if (bRet) m_nRxOffs = GetWritePos();
    return bRet && IsOpened();
}

inline void CTraceFifoReader::Close()
{
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    CTraceFifoImpl::Close();
    Terminate();
}

std::string CTraceFifoReader::WaitForMessage(size_t nTimeout /*= 1000*/)
{
    if (!IsOpened())
    {
        Open(nTimeout);
        if (!IsOpened()) return {};     // Must be connected
    }

    auto tpStart = std::chrono::high_resolution_clock::now();
    do
    {
        std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
        if (!IsInitialized()) break;

        // Count the characters in the string, not including the null-character. Returns nMaxLen when no null-character has been
        // found.
        auto fnCountStr = [](const uint8_t* szStr, size_t nMaxLen)
        {
            for (size_t nCount = 0; nCount < nMaxLen; nCount++)
                if (!szStr[nCount]) return nCount;
            return nMaxLen;
        };

        // If Rx is higher than Tx, read at the most until the end of the buffer and the rest from the start. If the Tx is smaller
        // than Tx, read at the most until Tx.
        size_t nBuffSize = GetDataBufferSize();
        uint8_t* pBuffer = GetDataPtr();
        size_t nLocalRx = m_nRxOffs;
        std::string ssMsg;
        if (nLocalRx > GetWritePos())
        {
            // Detect the length
            size_t uiMaxLen1 = nBuffSize - m_nRxOffs;
            size_t nLen1 = fnCountStr(pBuffer + m_nRxOffs, uiMaxLen1);
            size_t nMaxLen2 = GetWritePos();
            size_t nLen2 = nLen1 == uiMaxLen1 ? fnCountStr(pBuffer, nMaxLen2) : 0;
            if (nLen1 || nLen2 != nMaxLen2)
            {
                ssMsg.resize(nLen1 + nLen2);
                if (nLen1)
                    std::copy(pBuffer + m_nRxOffs, pBuffer + m_nRxOffs + nLen1, ssMsg.begin());
                if (nLen2)
                    std::copy(pBuffer, pBuffer + nLen2, ssMsg.begin() + nLen1);

                // Update Rx
                m_nRxOffs = nLen2 ? nLen2 + 1 : (nLen1 == uiMaxLen1 ? 1 : m_nRxOffs + nLen1 + 1);

                // Return result
                return ssMsg;
            }
        }
        else if (m_nRxOffs < GetWritePos())
        {
            // Copy the string
            size_t nMaxLen = GetWritePos() - m_nRxOffs;
            size_t nLen = fnCountStr(pBuffer + m_nRxOffs, nMaxLen);
            if (nLen != nMaxLen)
            {
                // Copy characters and include null-character.
                ssMsg.resize(nLen);
                std::copy(pBuffer + m_nRxOffs, pBuffer + m_nRxOffs + nLen, ssMsg.begin());

                // Update Rx
                m_nRxOffs += nLen + 1;

                // Return result
                return ssMsg;
            }
        }
        else if (!nTimeout)
            return {};
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (std::chrono::duration_cast<std::chrono::duration<size_t, std::milli>>(
        std::chrono::high_resolution_clock::now() - tpStart).count() < nTimeout);

    return {};
}

CTraceFifoWriter::CTraceFifoWriter(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16384*/) :
    CTraceFifoImpl(uiInstanceID, nSize)
{}

CTraceFifoWriter::~CTraceFifoWriter()
{
    Close();
}

CTraceFifoWriter::CTraceFifoWriter(CTraceFifoWriter&& rfifo) noexcept : CTraceFifoImpl(static_cast<CTraceFifoImpl&&>(rfifo))
{}

CTraceFifoWriter& CTraceFifoWriter::operator=(CTraceFifoWriter&& rfifo) noexcept
{
    CTraceFifoImpl::operator=(static_cast<CTraceFifoImpl&&>(rfifo));
    return *this;
}

bool CTraceFifoWriter::Open(size_t nTimeout /*= 1000*/, uint32_t uiFlags /*= 0u*/)
{
    if (IsOpened()) return true;
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    return CTraceFifoImpl::Open(nTimeout, uiFlags & ~static_cast<uint32_t>(ETraceFifoOpenFlags::read_only));
}

void CTraceFifoWriter::Close()
{
    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());
    CTraceFifoImpl::Close();
    Terminate();
}

void CTraceFifoWriter::Publish(const std::string& rssMessage)
{
    if (!IsOpened())
    {
        Open();
        if (!IsOpened()) return;     // Must be connected
    }

    std::unique_lock<std::recursive_mutex> lock(CreateAccessLockObject());

    // Message size is limited
    size_t nBuffSize = GetDataBufferSize();
    uint8_t* pBuffer = GetDataPtr();
    if (rssMessage.size() + 2u > nBuffSize) return;

    // Two chunks to copy:
    //  1. From start of message to end of message or end of buffer (whatever is lower).
    //  2. When leftover, from start of buffer until end of message.
    size_t nStart = GetWritePos();
    size_t nLen = rssMessage.size() + 1u;
    size_t nStop = nStart + nLen;
    size_t nLen1 = std::min(nBuffSize - nStart, nLen);
    size_t nLen2 = nLen - nLen1;

    // Copy the message
    if (nLen1) std::copy_n(rssMessage.c_str(), nLen1, pBuffer + GetWritePos());
    if (nLen2) std::copy_n(rssMessage.c_str() + nLen1, nLen2, pBuffer);
    SetWritePos(nStop > nBuffSize ? nLen2 : nStop);
}

CTraceFifoStreamBuffer::CTraceFifoStreamBuffer(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16384*/) :
    CTraceFifoWriter(uiInstanceID, nSize)
{}

CTraceFifoStreamBuffer::~CTraceFifoStreamBuffer()
{
    Close();
}

void CTraceFifoStreamBuffer::InterceptStream(std::ostream& rstream)
{
    m_mapBindings.emplace(&rstream, std::make_unique<SInterceptBinding>(rstream, *this));
}

void CTraceFifoStreamBuffer::RevertInterception(std::ostream& rstream)
{
    auto itBinding = m_mapBindings.find(&rstream);
    if (itBinding != m_mapBindings.end())
        m_mapBindings.erase(itBinding);
}

void CTraceFifoStreamBuffer::Close()
{
    sync();
    m_mapBindings.clear();
    CTraceFifoWriter::Close();
}

int CTraceFifoStreamBuffer::sync()
{
    Publish(str());
    if (!m_mapBindings.empty())
        m_mapBindings.begin()->second->streamOrginal << str();
    str(std::string()); // Clear the string buffer
    return 0;
}

CTraceFifoStreamBuffer::SInterceptBinding::SInterceptBinding(std::ostream& rstream, CTraceFifoStreamBuffer& rstreamBuffer) :
    rstreamIntercepted(rstream), streamOrginal(rstream.rdbuf(&rstreamBuffer))
{}

CTraceFifoStreamBuffer::SInterceptBinding::~SInterceptBinding()
{
    rstreamIntercepted.rdbuf(streamOrginal.rdbuf(nullptr));
}

CTraceFifoStdBuffer::CTraceFifoStdBuffer(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16384*/) :
    CTraceFifoWriter(uiInstanceID, nSize)
{}

CTraceFifoStdBuffer::~CTraceFifoStdBuffer()
{
    Close();
}

bool CTraceFifoStdBuffer::Open(size_t nTimeout /*= 1000*/, uint32_t uiFlags /*= 0u*/)
{
    // Close before...
    Close();

    // Open the fifo
    bool bRet = CTraceFifoWriter::Open(nTimeout, uiFlags);
    if (!bRet)
    {
        std::cout << "Help -1" << std::endl;
        return false;
    }

    // Create two pipes for the standard streams stdout and stderr.
#ifdef _WIN32
    bRet = pipe(m_rgPipeStdOut, 16384, O_TEXT) == 0;
    if (bRet) bRet = pipe(m_rgPipeStdErr, 16384, O_TEXT) == 0;
#else
    bRet = pipe(m_rgPipeStdOut) == 0;
    if (bRet) bRet = pipe(m_rgPipeStdErr) == 0;
#endif

    // Duplicate the StdOut and StdErr descriptors
    if (bRet)
    {
        m_iOldStdOut = dup(fileno(stdout));
        m_iOldStdErr = dup(fileno(stderr));
        if (m_iOldStdOut == -1 || m_iOldStdErr == -1)
            bRet = false;
    }

    // Assign the pipes to the StdOut and StdErr
    if (bRet) bRet = dup2(m_rgPipeStdOut[nWriteIndex], fileno(stdout)) >= 0;
    if (bRet) bRet = dup2(m_rgPipeStdErr[nWriteIndex], fileno(stderr)) >= 0;

    // Start the dispatch thread
    m_bShutdown = false;
    if (bRet) m_threadDispatch = std::thread(&CTraceFifoStdBuffer::DispatchThreadFunc, this);

    if (!bRet)
    {
        Close();
        return false;
    }
    return true;
}

#ifdef _MSC_VER
    // Prevent static code analysis warning for unused return value of _dup2.
    #pragma warning(push)
    #pragma warning(disable : 6031)
#endif

void CTraceFifoStdBuffer::Close()
{
    // Prevent multiple re-entries
    if (!m_bShutdown && (m_iOldStdOut != -1 || m_iOldStdErr != -1))
    {
        // Flush all streams
        std::cout.flush();
        std::clog.flush();
        std::cerr.flush();
        fflush(stdout);
        fflush(stderr);

        // Wait for processing finishing the dispatching
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Reassign the existing descriptors of StdOut and StdErr.
        if (m_iOldStdOut != -1)
            dup2(m_iOldStdOut, fileno(stdout));
        if (m_iOldStdErr != -1)
            dup2(m_iOldStdErr, fileno(stderr));

        // Shutdown the thread
        m_bShutdown = true;
        if (m_threadDispatch.joinable())
            m_threadDispatch.join();
    }

    // Close the duplictaed desciptors
    if (m_iOldStdOut != -1)
        close(m_iOldStdOut);
    if (m_iOldStdErr != -1)
        close(m_iOldStdErr);
    if (m_rgPipeStdOut[nReadIndex] != -1)
        close(m_rgPipeStdOut[nReadIndex]);
    if (m_rgPipeStdOut[nWriteIndex] != -1)
        close(m_rgPipeStdOut[nWriteIndex]);
    if (m_rgPipeStdErr[nReadIndex] != -1)
        close(m_rgPipeStdErr[nReadIndex]);
    if (m_rgPipeStdErr[nWriteIndex] != -1)
        close(m_rgPipeStdErr[nWriteIndex]);
    m_iOldStdOut = -1;
    m_iOldStdErr = -1;
    m_rgPipeStdOut[nReadIndex] = -1;
    m_rgPipeStdOut[nWriteIndex] = -1;
    m_rgPipeStdErr[nReadIndex] = -1;
    m_rgPipeStdErr[nWriteIndex] = -1;

    // Close the fifo
    CTraceFifoWriter::Close();
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

void CTraceFifoStdBuffer::DispatchThreadFunc()
{
    sdv::pointer<char> ptrBuffer;
    ptrBuffer.resize(8192);
    while (!m_bShutdown)
    {
        // Read the StdOut pipe
        while (!m_bShutdown && m_rgPipeStdOut[nReadIndex] != -1)
        {
#ifdef _WIN32
            if (eof(m_rgPipeStdOut[nReadIndex])) break;
#else
            struct pollfd sPoll{};
            sPoll.fd = m_rgPipeStdOut[nReadIndex];
            sPoll.events = POLLIN;
            if (poll(&sPoll, 1, 1) <= 0) break;
#endif
            int iBytesRead = read(m_rgPipeStdOut[nReadIndex], ptrBuffer.get(), static_cast<unsigned int>(ptrBuffer.size()));
            if (iBytesRead <= 0) break;
            std::string ssMsg(ptrBuffer.get(), static_cast<size_t>(iBytesRead));
            Publish(ssMsg);
        }

        // Read the StdErr pipe
        while (!m_bShutdown && m_rgPipeStdErr[nReadIndex] != -1)
        {
#ifdef _WIN32
            if (eof(m_rgPipeStdErr[nReadIndex])) break;
#else
            struct pollfd sPoll{};
            sPoll.fd = m_rgPipeStdErr[nReadIndex];
            sPoll.events = POLLIN;
            if (poll(&sPoll, 1, 1) <= 0) break;
#endif
            int iBytesRead = read(m_rgPipeStdErr[nReadIndex], ptrBuffer.get(), static_cast<unsigned int>(ptrBuffer.size()));
            if (iBytesRead <= 0) break;
            std::string ssMsg(ptrBuffer.get(), static_cast<size_t>(iBytesRead));
            Publish(ssMsg);
        }

        // Wait 10 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

#ifdef _WIN32
#pragma pop_macro("O_TEXT")
#pragma pop_macro("dup")
#pragma pop_macro("dup2")
#pragma pop_macro("fileno")
#pragma pop_macro("close")
#pragma pop_macro("pipe")
#pragma pop_macro("read")
#pragma pop_macro("eof")
#endif
