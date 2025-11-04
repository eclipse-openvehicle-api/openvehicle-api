#if !defined TRACE_FIFO_WINDOWS_H && defined _WIN32
#define TRACE_FIFO_WINDOWS_H

// Resolve conflict
#pragma push_macro("interface")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>

// Resolve conflict
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

/**
 * @brief Trace fifo shared memory class for Windows.
 */
class CTraceFifoWindows : public CTraceFifoBase
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Size of the fifo.
     */
    CTraceFifoWindows(uint32_t uiInstanceID = 1000u, size_t nSize = 16*1024);

    /**
     * @brief Default destructor
     * @remarks Automatically closes the fifo if opened before.
     */
    virtual ~CTraceFifoWindows() override;

    /**
     * @brief Copy constructor is not available.
     * @param[in] rfifo Reference to the fifo to copy.
     */
    CTraceFifoWindows(const CTraceFifoWindows& rfifo) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rfifo Reference to the fifo.
     */
    CTraceFifoWindows(CTraceFifoWindows&& rfifo) noexcept;

    /**
     * @brief Copy assignment is not available.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoWindows& operator=(const CTraceFifoWindows& rfifo) = delete;

    /**
     * @brief Move assignment.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoWindows& operator=(CTraceFifoWindows&& rfifo) noexcept;

    /**
     * @brief Open the fifo. Override of CTraceFifoBase::Open.
     * @param[in] nTimeout Timeout to return from this function. A timeout of 0xffffffff does not return until a connection has been
     * established.
     * @param[in] uiFlags Zero or more flags of ETraceFifoOpenFlags enum.
     * @return Returns true when connected; false otherwise.
     */
    virtual bool Open(size_t nTimeout = 1000, uint32_t uiFlags = 0u) override;

    // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Cancel any running task and close an open fifo. Override of CTraceFifoBase::Close.
     */
    virtual void Close() override;

    /**
     * @brief Is the fifo open for reading and writing? Override of CTraceFifoBase::IsOpened.
     * @return Returns true when opened; false otherwise.
     */
    virtual bool IsOpened() const override;

private:
    HANDLE      m_hMapFile = INVALID_HANDLE_VALUE;  ///< Handle to the shared memory buffer.
};

/// The Windows implementation of the trace fifo.
using CTraceFifoImpl = CTraceFifoWindows;

#endif // !defined TRACE_FIFO_WINDOWS_H