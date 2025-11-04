#if defined _WIN32

#ifndef INCLUDE_TRACE_FIFO_PLATFORM
#error Do not include this file directly. The file is included by trace_fifo.cpp.
#endif

#include "trace_fifo_windows.h"

#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <support/string.h>

CTraceFifoWindows::CTraceFifoWindows(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16*1024*/) :
    CTraceFifoBase(uiInstanceID, nSize)
{}

CTraceFifoWindows::~CTraceFifoWindows()
{
    Close();
}

CTraceFifoWindows::CTraceFifoWindows(CTraceFifoWindows&& rfifo) noexcept:
    CTraceFifoBase(static_cast<CTraceFifoBase&&>(rfifo)), m_hMapFile(rfifo.m_hMapFile)
{
    rfifo.m_hMapFile = INVALID_HANDLE_VALUE;
}

CTraceFifoWindows& CTraceFifoWindows::operator=(CTraceFifoWindows&& rfifo) noexcept
{
    Close();
    CTraceFifoBase::operator=(static_cast<CTraceFifoBase&&>(rfifo));
    m_hMapFile = rfifo.m_hMapFile;
    rfifo.m_hMapFile = INVALID_HANDLE_VALUE;

    return *this;
}

#ifdef _MSC_VER
    // Prevent bogus warning about uninitialized memory for the variable *hFile.
    #pragma warning(push)
    #pragma warning(disable : 6001)
#endif

bool CTraceFifoWindows::Open(size_t nTimeout /*= 1000*/, uint32_t uiFlags /*= 0u*/)
{
    std::string ssSharedMemName = "SDV_TRACE_MONITOR_" + std::to_string(GetInstanceID());

    auto tpStart = std::chrono::high_resolution_clock::now();
    bool bOpenOnly = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::open_only);
    bool bForceCreate = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::force_create);
    bool bReadOnly = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::read_only);
    if (bOpenOnly && bForceCreate) return false;

    do
    {
        if (IsOpened()) break;

        // Try creating the file mapping object
        if (m_hMapFile == nullptr || m_hMapFile == INVALID_HANDLE_VALUE)
        {
            if (bOpenOnly)
            {
                DWORD dwMapAccess = bReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
                m_hMapFile = OpenFileMappingA(dwMapAccess, false, ssSharedMemName.c_str());
            }
            else
            {
                DWORD dwMapAccess = PAGE_READWRITE;
                m_hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, dwMapAccess, 0, static_cast<DWORD>(GetDefaultSize()), ssSharedMemName.c_str());
                if (m_hMapFile != nullptr && m_hMapFile != INVALID_HANDLE_VALUE && bForceCreate && GetLastError() == ERROR_ALREADY_EXISTS)
                {
                    CloseHandle(m_hMapFile);
                    m_hMapFile = INVALID_HANDLE_VALUE;
                }
            }
        }
        if (m_hMapFile == nullptr || m_hMapFile == INVALID_HANDLE_VALUE)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Map the file into memory
        DWORD dwViewAccess = bReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
        void* pView = MapViewOfFile(m_hMapFile, dwViewAccess, 0, 0, /*GetDefaultSize()*/0);
        if (!pView)
        {
            CloseHandle(m_hMapFile);
            m_hMapFile = INVALID_HANDLE_VALUE;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Request the size of the mapping - this will update the size...
        MEMORY_BASIC_INFORMATION sMemInfo{};
        if (VirtualQuery(pView, &sMemInfo, sizeof(sMemInfo)) != sizeof(sMemInfo) || !sMemInfo.RegionSize)
        {
            UnmapViewOfFile(pView);
            CloseHandle(m_hMapFile);
            m_hMapFile = INVALID_HANDLE_VALUE;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Needs Initialize
        InitializeBuffer(pView, sMemInfo.RegionSize, bReadOnly);
    }
    while (std::chrono::duration_cast<std::chrono::duration<size_t, std::milli>>(
        std::chrono::high_resolution_clock::now() - tpStart).count() < nTimeout);

    return IsOpened();
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

void CTraceFifoWindows::Close()
{
    if (GetView()) UnmapViewOfFile(GetView());
    if (m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hMapFile);
    m_hMapFile = INVALID_HANDLE_VALUE;
    Terminate();
}

bool CTraceFifoWindows::IsOpened() const
{
    return m_hMapFile && m_hMapFile != INVALID_HANDLE_VALUE;
}

#endif // defined _WIN32