#ifndef TRACE_FIFO_H
#define TRACE_FIFO_H

#include <cstdint>
#include <mutex>
#include <string>
#include <sstream>
#include <map>

/**
 * @brief trace fifo open flags
 */
enum class ETraceFifoOpenFlags : uint32_t
{
    open_only = 1,      ///< Open only. Do not create a new shared memory area.
    force_create = 2,   ///< Create a new shared memory area, removing any previous connection.
    read_only = 4,      ///< Open the shared memory for read-only access.
};

/**
 * @brief Trace fifo class allowing the publishing and monitoring of trace messages.
 */
class CTraceFifoBase
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Default size of the fifo.
     */
    CTraceFifoBase(uint32_t uiInstanceID, size_t nSize);

    /**
     * @brief Default destructor
     * @remarks Automatically closes the fifo if opened before.
     */
    virtual ~CTraceFifoBase();

    /**
     * @brief Copy constructor is not available.
     * @param[in] rfifo Reference to the fifo to copy.
     */
    CTraceFifoBase(const CTraceFifoBase& rfifo) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rfifo Reference to the fifo.
     */
    CTraceFifoBase(CTraceFifoBase&& rfifo) noexcept;

    /**
     * @brief Copy assignment is not available.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoBase& operator=(const CTraceFifoBase& rfifo) = delete;

    /**
     * @brief Move assignment.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoBase& operator=(CTraceFifoBase&& rfifo) noexcept;

    /**
     * @brief Open the fifo. Implemented by derived class.
     * @param[in] nTimeout Timeout to return from this function. A timeout of 0xffffffff does not return until a connection has been
     * established.
     * @param[in] uiFlags Zero or more flags of ETraceFifoOpenFlags enum.
     * @return Returns true when connected; false otherwise.
     */
    virtual bool Open(size_t nTimeout = 1000, uint32_t uiFlags = 0u) = 0;

    /**
     * @brief Cancel any running task and close an open fifo. Implemented by derived class.
     */
    virtual void Close() = 0;

    /**
     * @brief Is the fifo open for reading and writing? Implemented by derived class.
     * @return Returns true when opened; false otherwise.
     */
    virtual bool IsOpened() const = 0;

    /**
     * @brief Set a new instance ID for the communication.
     * @remarks The instance ID can only be set when the fifo is not opened yet.
     * @param[in] uiInstanceID The new instance ID.
     * @return Returns whether setting the instance ID was successful.
     */
    bool SetInstanceID(uint32_t uiInstanceID);

    /**
     * @brief Return the instance ID to use for the communication.
     * @return The instance ID.
     */
    uint32_t GetInstanceID() const;

    /**
     * @brief Set a new default size to be used during creation of the fifo.
     * @param[in] nSize The new default size of the buffer.
     */
    void SetDefaultSize(size_t nSize);

    /**
     * @brief Get the default size used during creation of the fifo.
     * @return The default size of the buffer.
     */
    size_t GetDefaultSize() const;

    /**
     * @brief Get the size of the view.
     * @return The view size.
     */
    size_t GetViewSize() const;

     /**
     * @brief Get the size of the buffer without any headers.
     * @return Returns the size of the buffer or 0 when the buffer is not initialized.
     */
    size_t GetDataBufferSize() const;

protected:
    /**
     * @brief Initialize the buffer after successful opening.
     * @param[in] pView Pointer to the shared memory view.
     * @param[in] nBufferSize The size of the buffer allocated.
     * @param[in] bReadOnly When set, the buffer is initialized as read-only. Multiple read-only and only one writable buffer access
     * are allowed.
     */
    void InitializeBuffer(void* pView, size_t nBufferSize, bool bReadOnly);

    /**
     * @brief Check whether the buffer is initialized (signature and instance ID are set).
     * @return Returns true when the buffer has been initialized; false when not.
    */
    bool IsInitialized() const;

    /**
     * @brief Clear the buffer pointers
     */
    void Terminate();

    /**
     * @brief Create a lock object for exclusive access of the buffer.
     * @return The lock object that allows access.
     */
    std::unique_lock<std::recursive_mutex> CreateAccessLockObject() const;

    /**
     * @brief Get access to the view.
     * @return Get a pointer to the buffer. Returns NULL when the buffer is not initialized.
     */
    void* GetView();

    /**
     * @{
     * @brief Get access to the data portion of the buffer.
     * @return Get a pointer to the data. Returns NULL when the buffer is not initialized.
     */
    uint8_t* GetDataPtr();
    const uint8_t* GetDataPtr() const;
    /**
     * @}
     */

    /**
     * @brief Get the write position.
     * @return The current write position within the buffer.
    */
    size_t GetWritePos() const;

    /**
     * @brief Set the write position. Can only be used by writable buffer (causing an access violation otherwise).
     * @param[in] nTxPos The new write position.
     */
    void SetWritePos(size_t nTxPos);

private:
    /**
     * @brief Shared memory header structure; at the front of the shared memory buffer.
     */
    struct SSharedMemBufHeader
    {
        char                rgszSignature[8];   ///< Signature "SDV_MON\0"
        uint32_t            uiInstanceID;       ///< Instance ID of the server instance
        volatile uint32_t   uiTxOffs;           ///< Tx position
    };

    mutable volatile bool           m_bInitConfirmed = false;           ///< When set, bypasses the header checking.
    uint32_t                        m_uiInstanceID = 1000;              ///< Instance ID to use while connecting.
    size_t                          m_nSize = 0;                        ///< Size of the fifo.
    size_t                          m_nDefaultSize = 0;                 ///< Requested size.
    uint8_t*                        m_pBuffer = nullptr;                ///< Pointer to the buffer.
    SSharedMemBufHeader*            m_psHdr = nullptr;                  ///< Shared memory header at from of the buffer.
    mutable std::recursive_mutex    m_mtxAccess;                        ///< Protect against sudden closure.
};

// Include the implementation classes classes for the trace fifo
#include "trace_fifo_windows.h"
#include "trace_fifo_posix.h"

/**
 * @brief Reader class for the trace fifo. Multiple readers can coexist.
 */
class CTraceFifoReader : public CTraceFifoImpl
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Default size of the fifo.
     */
    CTraceFifoReader(uint32_t uiInstanceID = 1000u, size_t nSize = 16384);

    /**
     * @brief Default destructor
     * @remarks Automatically closes the fifo if opened before.
     */
    virtual ~CTraceFifoReader() override;

    /**
     * @brief Copy constructor is not available.
     * @param[in] rfifo Reference to the fifo to copy.
     */
    CTraceFifoReader(const CTraceFifoReader& rfifo) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rfifo Reference to the fifo.
     */
    CTraceFifoReader(CTraceFifoReader&& rfifo) noexcept;

    /**
     * @brief Copy assignment is not available.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoReader& operator=(const CTraceFifoReader& rfifo) = delete;

    /**
     * @brief Move assignment.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoReader& operator=(CTraceFifoReader&& rfifo) noexcept;

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
     * @brief Wait for a message.
     * @remarks Automatically opens the fifo if not opened before.
     * @param[in] nTimeout Timeout to return from this function. A timeout of 0xffffffff does not return until a message has been
     * received.
     * @return The received message or an empty message if a timeout occurred or the publisher had sent an empty message.
     */
    std::string WaitForMessage(size_t nTimeout = 1000);

private:
    size_t      m_nRxOffs = 0;      ///< Reader position
};

/**
 * @brief Writer class for a trace fifo. Only one writer should be present.
 */
class CTraceFifoWriter : public CTraceFifoImpl
{
public:
    /**
     * @brief Default constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Default size of the fifo.
     */
    CTraceFifoWriter(uint32_t uiInstanceID = 1000u, size_t nSize = 16384);

    /**
     * @brief Default destructor
     * @remarks Automatically closes the fifo if opened before.
     */
    virtual ~CTraceFifoWriter() override;

    /**
     * @brief Copy constructor is not available.
     * @param[in] rfifo Reference to the fifo to copy.
     */
    CTraceFifoWriter(const CTraceFifoWriter& rfifo) = delete;

    /**
     * @brief Move constructor.
     * @param[in] rfifo Reference to the fifo.
     */
    CTraceFifoWriter(CTraceFifoWriter&& rfifo) noexcept;

    /**
     * @brief Copy assignment is not available.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoWriter& operator=(const CTraceFifoWriter& rfifo) = delete;

    /**
     * @brief Move assignment.
     * @param[in] rfifo Reference to the fifo.
     * @return Returns a reference to this fifo.
     */
    CTraceFifoWriter& operator=(CTraceFifoWriter&& rfifo) noexcept;

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
     * @brief Publish a message. If the buffer is full, the oldest message is removed.
     * @remarks Automatically opens the fifo if not opened before.
     * @param[in] rssMessage Reference to the message to publish.
     */
    void Publish(const std::string& rssMessage);

};

/**
 * @brief Trace fifo stream buffer object.
 * @remarks This implementation works for applications within the scope of the C++ library source compilation into one binary unit.
 * This is due to the templated nature of the C++ library allowing very little reuse of compiled code across binary units. This
 * means, that although interception will work in one unit (e.g. executable or static/shared library), it might not intercept
 * messages from another unit.
 */
class CTraceFifoStreamBuffer : public CTraceFifoWriter, public std::stringbuf
{
public:
    /**
     * @brief Constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Default size of the fifo.
     */
    CTraceFifoStreamBuffer(uint32_t uiInstanceID = 1000u, size_t nSize = 16384);

    /**
     * @brief Destructor
     */
    virtual ~CTraceFifoStreamBuffer() override;

    /**
     * @brief Assign this buffer to a stream object. Any message will be streamed to both the original stream as well as the trace
     * fifo.
     * @attention The stream object needs to stay in scope until the assignment is removed or this class is destroyed.
     * @attention Works for text only.
     * @remarks Removal of this assignment is done automatically during destruction of this class.
     * @param[in] rstream Reference to the stream to intercept the communication for.
     */
    void InterceptStream(std::ostream& rstream);

    /**
     * @brief Remove the interception of a stream.
     * @param[in] rstream Reference to the stream to revert the interception for.
     */
    void RevertInterception(std::ostream& rstream);

    // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Cancel any running task and close an open fifo. Override of CTraceFifoBase::Close.
     */
    virtual void Close() override;

    /**
     * @brief Synchronizes the buffers with the associated character sequence.
     * @return Returns 0 if successful; -1 if not.
     */
    int sync();

private:
    /**
     * @brief Stream interception binding.
     */
    struct SInterceptBinding
    {
        /**
         * @brief Constructor doing the binding.
         */
        SInterceptBinding(std::ostream& rstream, CTraceFifoStreamBuffer& rstreamBuffer);

        //SInterceptBinding(SInterceptBinding& rsBinding) : rstreamIntercepted(rsBinding.rstreamIntercepted), streamOrginal(rsBinding.streamOrginal.rdbuf()) {}

        /**
         * @brief Destructor undoing the binding.
         */
        ~SInterceptBinding();

        std::ostream&   rstreamIntercepted;     ///< The intercepted stream
        std::ostream    streamOrginal;          ///< Stream object redirecting the buffer.
    };

    std::map<std::ostream*, std::unique_ptr<SInterceptBinding>> m_mapBindings;    ///< Map with intercepted stream bindings.
};

/**
 * @brief Interception and dispatching of all messages from STDOUT and STDERR to the trace fifo.
 */
class CTraceFifoStdBuffer : public CTraceFifoWriter
{
public:
    /**
     * @brief Constructor
     * @param[in] uiInstanceID The instance ID to use for sending/monitoring the messages.
     * @param[in] nSize Default size of the fifo.
     */
    CTraceFifoStdBuffer(uint32_t uiInstanceID = 1000u, size_t nSize = 16384);

    /**
     * @brief Destructor
     */
    virtual ~CTraceFifoStdBuffer() override;

    /**
     * @brief Open the fifo and direct all messages from StdOut and StdErr to the trace fifo. Override of CTraceFifoBase::Open.
     * @param[in] nTimeout Timeout to return from this function. A timeout of 0xffffffff does not return until a connection has been
     * established.
     * @param[in] uiFlags Zero or more flags of ETraceFifoOpenFlags enum.
     * @return Returns true when connected; false otherwise.
     */
    virtual bool Open(size_t nTimeout = 1000, uint32_t uiFlags = 0u) override;

    // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
     * @brief Revert the redirection and close the open fifo. Override of CTraceFifoBase::Close.
     */
    virtual void Close() override;

private:
    /**
     * @brief Read from the pipe and forward the text to the trace fifo. Read until shutdown flag is switch on.
     */
    void DispatchThreadFunc();

    const size_t nReadIndex = 0;                    ///< The read index of the pipe descriptor.
    const size_t nWriteIndex = 1;                   ///< The write index of the pipe descriptor.
    bool        m_bShutdown = false;                ///< Shutdown flag for the pipe reader thread.
    int         m_rgPipeStdOut[2] = { -1, -1 };     ///< StdOut pipe with read and write descriptors.
    int         m_rgPipeStdErr[2] = { -1, -1 };     ///< StdErr pipe with read and write descriptors.
    int         m_iOldStdOut = -1;                  ///< Old descriptor for the StdOut
    int         m_iOldStdErr = -1;                  ///< Old descriptor for the StdErr
    std::thread m_threadDispatch;                   ///< Dispatch thread
};

#endif // !defined TRACE_FIFO_H