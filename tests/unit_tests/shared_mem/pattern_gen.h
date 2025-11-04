#ifndef PATTERN_GEN_H
#define PATTERN_GEN_H

#include <thread>
#include "../../../sdv_services/ipc_shared_mem/mem_buffer_accessor.h"

/**
 * The pattern test time in seconds.
 */
#define PATTERN_TEST_TIME_S 2

class CPatternSender
{
public:
    /**
     * @brief Constructor
     * @param[in] raccessorOut Reference to the output accessor.
     * @param[in] uiDelayMs The amount of delay (in ms) between calls. If 0, there is no delay.
     */
    CPatternSender(CMemBufferAccessorTx& raccessorOut, uint32_t uiDelayMs = 0);

    /**
     * @brief Shutdown
     */
    void Shutdown() { m_bShutdown = true; m_thread.join(); }

    /**
     * @brief Processing thread function sending packets with variable length and incrementing counter for every byte.
     */
    void Process();

    /**
     * @{
     * @brief Statistics
     */
    uint32_t GetCycleCnt() const { return m_uiCycleCnt; }
    uint32_t GetPacketCnt() const { return m_uiPacketCnt; }
    uint64_t GetByteCnt() const { return m_uiByteCnt; }
    /**
     * @}
     */

private:
	CMemBufferAccessorTx& m_raccessorOut;		 //!< Reference to the output accessor
	std::thread			  m_thread;				 //!< Processing thread
    bool				  m_bStarted	= false; //!< Set by the thread when started.
    bool				  m_bShutdown	= false; //!< When set, shutdown the thread.
	uint32_t			  m_uiDelayMs	= 0u;	 //!< Delay (in ms) to insert while processing.
	uint32_t			  m_uiCycleCnt	= 0u;	 //!< Amount of packets
	uint32_t			  m_uiPacketCnt = 0u;	 //!< Amount of packets
	uint64_t			  m_uiByteCnt	= 0ull;	 //!< Amount of bytes
};

class CPatternReceiver
{
public:
    /**
     * @brief Constructor
     * @param[in] raccessorIn Reference to the input accessor.
     * @param[in] uiDelayMs The amount of delay (in ms) between calls. If 0, there is no delay.
     */
    CPatternReceiver(CMemBufferAccessorRx& raccessorIn, uint32_t uiDelayMs = 0);

    /**
     * @brief Shutdown
     */
    void Shutdown() { m_bShutdown = true; m_thread.join(); }

    /**
     * @brief Processing thread function receiving packets of variable length and incrementing counter for every byte.
     */
    void Process();

    /**
     * @{
     * @brief Statistics
     */
    uint32_t GetCycleCnt() const { return m_uiCycleCnt; }
    uint32_t GetErrorCnt() const { return m_uiErrorCnt; }
    uint32_t GetPacketCnt() const { return m_uiPacketCnt; }
    uint64_t GetByteCnt() const { return m_uiByteCnt; }
    /**
     * @}
     */

private:
	CMemBufferAccessorRx& m_raccessorIn;		 //!< Reference to the input accessor
	std::thread			  m_thread;				 //!< Processing thread
    bool				  m_bStarted	= false; //!< Set by the thread when started.
    bool				  m_bShutdown	= false; //!< When set, shutdown the thread.
	uint32_t			  m_uiDelayMs	= 0u;	 //!< Delay (in ms) to insert while processing.
	uint32_t			  m_uiCycleCnt	= 0u;	 //!< Amount of packets
	uint32_t			  m_uiErrorCnt	= 0u;	 //!< Amount of counter errors
	uint32_t			  m_uiPacketCnt = 0u;	 //!< Amount of packets
	uint64_t			  m_uiByteCnt	= 0ull;	 //!< Amount of bytes
};

class CPatternRepeater
{
public:
    /**
     * @brief Constructor
     * @param[in] raccessorIn Reference to the input accessor.
     * @param[in] raccessorOut Reference to the output accessor.
     * @param[in] uiDelayMs The amount of delay (in ms) between calls. If 0, there is no delay.
     */
    CPatternRepeater(CMemBufferAccessorRx& raccessorIn, CMemBufferAccessorTx& raccessorOut, uint32_t uiDelayMs = 0);

    /**
     * @brief Shutdown
     */
    void Shutdown() { m_bShutdown = true; m_thread.join(); }

    /**
     * @brief Processing thread function receiving and dispatching packets of variable length.
     */
    void Process();

    /**
     * @{
     * @brief Statistics
     */
    uint32_t GetCycleCnt() const { return m_uiCycleCnt; }
    uint32_t GetErrorCnt() const { return m_uiErrorCnt; }
    uint32_t GetPacketCnt() const { return m_uiPacketCnt; }
    uint64_t GetByteCnt() const { return m_uiByteCnt; }
    /**
     * @}
     */

private:
	CMemBufferAccessorRx& m_raccessorIn;		 //!< Reference to the input accessor
	CMemBufferAccessorTx& m_raccessorOut;		 //!< Reference to the output accessor
	std::thread			  m_thread;				 //!< Processing thread
    bool				  m_bStarted	= false; //!< Set by the thread when started.
    bool				  m_bShutdown	= false; //!< When set, shutdown the thread.
	uint32_t			  m_uiDelayMs	= 0u;	 //!< Delay (in ms) to insert while processing.
	uint32_t			  m_uiCycleCnt	= 0u;	 //!< Amount of packets
	uint32_t			  m_uiErrorCnt	= 0u;	 //!< Amount of counter errors
	uint32_t			  m_uiPacketCnt = 0u;	 //!< Amount of packets
	uint64_t			  m_uiByteCnt	= 0ull;	 //!< Amount of bytes
};

#endif // !defined(PATTERN_GEN_H)