#if !defined TRACE_FIFO_POSIX_H && defined __unix__
#define TRACE_FIFO_POSIX_H

#ifndef TRACE_FIFO_H
#error Do not include this file directly. Include trace_fifo.h instead.
#endif

/**
* @brief Trace fifo shared memory class for Posix.
*/
class CTraceFifoPosix : public CTraceFifoBase
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Size of the fifo.
     */
    CTraceFifoPosix(uint32_t uiInstanceID = 1000u, size_t nSize = 16*1024);

    /**
     * @brief Default destructor
     * @remarks Automatically closes the fifo if opened before.
     */
    virtual ~CTraceFifoPosix();

    /**
     * @brief Copy constructor is not available.
     * @param[in] rfifo Reference to the fifo to copy.
     */
    CTraceFifoPosix(const CTraceFifoPosix& rfifo) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rfifo Reference to the fifo.
     */
    CTraceFifoPosix(CTraceFifoPosix&& rfifo);

    /**
     * @brief Copy assignment is not available.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoPosix& operator=(const CTraceFifoPosix& rfifo) = delete;

    /**
     * @brief Move assignment.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoPosix& operator=(CTraceFifoPosix&& rfifo);

    /**
     * @brief Open the fifo. Overload of CTraceFifoBase::Open.
     * @param[in] nTimeout Timeout to return from this function. A timeout of 0xffffffff does not return until a connection has been
     * established.
     * @param[in] uiFlags Zero or more flags of ETraceFifoOpenFlags enum.
     * @return Returns true when connected; false otherwise.
     */
    virtual bool Open(size_t nTimeout = 1000, uint32_t uiFlags = 0u) override;

    // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Cancel any running task and close an open fifo. Overload of CTraceFifoBase::Close.
     */
    virtual void Close() override;

    /**
     * @brief Is the fifo open for reading and writing? Overload of CTraceFifoBase::IsOpened.
     * @return Returns true when opened; false otherwise.
     */
    virtual bool IsOpened() const override;

private:
    int     m_iFileDescr = 0;   ///< File descriptor of the shared memory.
};

/// The Posix implementation of the trace fifo.
using CTraceFifoImpl = CTraceFifoPosix;

#endif // !defined TRACE_FIFO_POSIX_H