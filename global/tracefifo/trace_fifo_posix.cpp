#if defined __unix__

#ifndef INCLUDE_TRACE_FIFO_PLATFORM
#error Do not include this file directly. The file is included by trace_fifo.cpp.
#endif

#include "trace_fifo_posix.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <support/string.h>

CTraceFifoPosix::CTraceFifoPosix(uint32_t uiInstanceID /*= 1000u*/, size_t nSize /*= 16*1024*/) :
    CTraceFifoBase(uiInstanceID, nSize)
{}

CTraceFifoPosix::~CTraceFifoPosix()
{
    Close();
}

CTraceFifoPosix::CTraceFifoPosix(CTraceFifoPosix&& rfifo) :
    CTraceFifoBase(static_cast<CTraceFifoBase&&>(rfifo)), m_iFileDescr(rfifo.m_iFileDescr)
{
    rfifo.m_iFileDescr = 0;
}

CTraceFifoPosix& CTraceFifoPosix::operator=(CTraceFifoPosix&& rfifo)
{
    Close();
    CTraceFifoBase::operator=(static_cast<CTraceFifoBase&&>(rfifo));
    m_iFileDescr = rfifo.m_iFileDescr;
    rfifo.m_iFileDescr = 0;

    return *this;
}

bool CTraceFifoPosix::Open(size_t nTimeout /*= 1000*/, uint32_t uiFlags /*= 0u*/)
{
    std::string ssSharedMemName = "SDV_LOG_MONITOR_" + std::to_string(GetInstanceID());

    auto lock = CreateAccessLockObject();

    auto tpStart = std::chrono::high_resolution_clock::now();
    bool bOpenOnly = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::open_only);
    bool bForceCreate = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::force_create);
    bool bReadOnly = uiFlags & static_cast<uint32_t>(ETraceFifoOpenFlags::read_only);
    if (bOpenOnly && bForceCreate) return false;

    // In case of a force-create-flag, unlink a potential previous allocation.
    if (bForceCreate) shm_unlink(ssSharedMemName.c_str());

    do
    {
        if (IsOpened()) break;

        // Try creating the file mapping object
        if (!m_iFileDescr)
        {
            int iMapAccess = O_RDWR | O_CREAT;
            if (bOpenOnly)
                iMapAccess = bReadOnly ? O_RDONLY : O_RDWR;
            else if (bForceCreate)
                iMapAccess |= O_EXCL;
            m_iFileDescr = shm_open(ssSharedMemName.c_str(), iMapAccess, S_IRUSR | S_IWUSR);
        }
        if (m_iFileDescr == -1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Check for the size of the memory. If zero, the memory is not initialized yet.
        struct stat sStatistics{};
        int iResult = fstat(m_iFileDescr, &sStatistics);
        if (iResult == -1)
        {
            Close();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Extend shared memory object as by default it's initialized with size 0
        if (!sStatistics.st_size)
        {
            // Readonly files cannot be truncated
            if (bReadOnly)
            {
                Close();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            iResult = ftruncate(m_iFileDescr, static_cast<off_t>(GetDefaultSize()));
            if (iResult != -1)
                iResult = fstat(m_iFileDescr, &sStatistics);
            if (iResult == -1 || !sStatistics.st_size)
            {
                Close();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
        }

        // Map the file into memory
        int iViewAccess = PROT_READ;
        if (!bOpenOnly && !bReadOnly)
            iViewAccess |= PROT_WRITE;
        void* pView = mmap(NULL, sStatistics.st_size, iViewAccess, MAP_SHARED, m_iFileDescr, 0);
        if (pView == MAP_FAILED)
        {
            Close();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Needs initialization?
        InitializeBuffer(pView, sStatistics.st_size, bReadOnly);
    }
    while (std::chrono::duration_cast<std::chrono::duration<size_t, std::milli>>(
        std::chrono::high_resolution_clock::now() - tpStart).count() < nTimeout);

    return IsOpened();
}

void CTraceFifoPosix::Close()
{
    if (GetView()) munmap(GetView(), GetViewSize());
    if (m_iFileDescr)
        close(m_iFileDescr);
    m_iFileDescr = 0;
    Terminate();
}

bool CTraceFifoPosix::IsOpened() const
{
    return m_iFileDescr ? true : false;
}

#endif // defined __unix__