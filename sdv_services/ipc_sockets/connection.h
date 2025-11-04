/**
 * @file connection.h
 * @author Sudipta Babu Durjoy FRD DISS21 (mailto:sudipta.durjoy@zf.com)
 * @brief
 * @version 1.0
 * @date 2023-04-18
 *
 * @copyright Copyright ZF Friedrichshafen AG (c) 2023
 *
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include <thread>
#include <algorithm>

#include <interfaces/ipc.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <support/component_impl.h>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

constexpr uint32_t m_MsgStart = 0x01020304; ///< value to mark the start of the message header
constexpr uint32_t m_MsgEnd	 = 0x05060708;  ///< value to mark the end of the message header

/**
 * @brief Message header which will be put before a message.
 * Can be used for validation and includes complete size of the message. Other values are not used yet
 */
struct SMsgHeader
{
	uint32_t msgStart         = 0; ///< marker for the start of the header
	uint32_t msgId            = 0; ///< message Id, must match for all message packages
	uint32_t msgSize          = 0; ///< size of the message without the header
	uint32_t packetNumber     = 0; ///< number of the package starting with 1
	uint32_t totalPacketCount = 0; ///< total number of paackes required for the message
	uint32_t msgEnd	          = 0; ///< marker for the end of the header
};

/**
* Class for remote IPC connection
* Created and managed by IPCAccess::AccessRemoveIPCConnection(best use unique_ptr to store, so memory address stays
 * valid)
 */
class CConnection : public sdv::IInterfaceAccess, public sdv::ipc::IDataSend, public sdv::ipc::IConnect, public sdv::IObjectDestroy
{
public:
    /**
     * @brief default constructor used by create endpoint - allocates new buffers m_Sender and m_Receiver
     */
    CConnection();

    /**
     * @brief access existing connection
     * @param[in] preconfiguredSocket Prepared socket for the connection.
     * @param[in] acceptConnectionRequired If true connection has to be accepted before receive thread can be started.
     */
    CConnection(SOCKET preconfiguredSocket, bool acceptConnectionRequired);

    /**
    * @brief Virtual destructor needed for "delete this;".
    */
    virtual ~CConnection() = default;

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataSend)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnect)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

	/**
	* @brief Sends data consisting of multiple data chunks via the IPC connection.
	* Overload of sdv::ipc::IDataSend::SendData.
	* @param[inout] seqData Sequence of data buffers to be sent. The sequence might be changed to optimize the communication
	* without having to copy the data.
	* @return Return 'true' if all data could be sent; 'false' otherwise.
	*/
	virtual bool SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override;

    /**
     * @brief Establish a connection and start sending/receiving messages. Overload of
     * sdv::ipc::IConnect::AsyncConnect.
     * @param[in] pReceiver The message has to be forwarded.
     * @return Returns 'true' when a connection could be established. Use IConnectStatus or IConnectEventCallback to check the
     * connection state.
     */
    virtual bool AsyncConnect(/*in*/ sdv::IInterfaceAccess* pReceiver) override;

    /**
     * @brief Wait for a connection to take place. Overload of sdv::ipc::IConnect::WaitForConnection.
     * @param[in] uiWaitMs Wait for a connection to take place. A value of 0 doesn't wait at all, a value of 0xffffffff
     * waits for infinite time.
     * @return Returns 'true' when a connection took place.
     */
    virtual bool WaitForConnection(/*in*/ uint32_t uiWaitMs) override;

    /**
     * @brief Cancel a wait for connection. Overload of sdv::ipc::IConnect::CancelWait.
     */
    virtual void CancelWait() override;

	/**
	 * @brief Disconnect from a connection. This will set the connect status to disconnected and release the interface
	 * used for the status events.
	 */
	virtual void Disconnect() override;

    /**
     * @brief Register event callback interface. Overload of sdv::ipc::IConnect::RegisterStatusEventCallback.
     * @details Register a connection status event callback interface. The exposed interface must be of type
     * IConnectEventCallback. The registration will exist until a call to the unregister function with the returned cookie
     * or until the connection is terminated.
     * @param[in] pEventCallback Pointer to the object exposing the IConnectEventCallback interface.
     * @return The cookie assigned to the registration.
     */
    virtual uint64_t RegisterStatusEventCallback(/*in*/ sdv::IInterfaceAccess* pEventCallback) override;

    /**
     * @brief Unregister the status event callback with the returned cookie from the registration. Overload of
     * sdv::ipc::IConnect::UnregisterStatusEventCallback.
     * @param[in] uiCookie The cookie returned by a previous call to the registration function.
     */
    virtual void UnregisterStatusEventCallback(/*in*/ uint64_t uiCookie) override;

	/**
     * @brief Get status of the connection
     * @return Returns the ipc::EConnectStatus struct
     */
    virtual sdv::ipc::EConnectStatus GetStatus() const override;

	/**
	 * @brief Destroy the object. Overload of IObjectDestroy::DestroyObject.
	 * @attention After a call of this function, all exposed interfaces render invalid and should not be used any more.
	 */
	virtual void DestroyObject() override;

private:
	std::thread					 			m_ReceiveThread;				///< Thread which receives data from the socket
	std::atomic<bool>			 			m_StopReceiveThread = false;	///< bool variable to stop thread
	std::atomic<sdv::ipc::EConnectStatus>	m_ConnectionStatus;				///< the status of the connection
	SOCKET						 			m_ConnectionSocket;				///< The socket to send and receive data
	sdv::ipc::IDataReceiveCallback*         m_pReceiver = nullptr;			///< Receiver to pass the messages if available
	sdv::ipc::IConnectEventCallback*		m_pEvent = nullptr;				///< Event receiver.
	bool						 			m_AcceptConnectionRequired;		///< if true connection has to be accepted before receive thread can be started
	mutable std::recursive_mutex 			m_SendMutex;					///< Synchronize all packages to be send.

	static constexpr uint32_t	 			m_SendMessageSize{ 1024 };									///< size for the message to be send.
	static constexpr uint32_t    			m_SendBufferSize = sizeof(SMsgHeader) + m_SendMessageSize;	///< Initial size of the send buffer.
	char					     			m_SendBuffer[m_SendBufferSize];								///< send buffer length
	char                         			m_ReceiveBuffer[sizeof(SMsgHeader)];						///< receive buffer, just for reading the message header
	uint32_t	                 			m_ReceiveBufferLength = sizeof(SMsgHeader);					///< receive buffer length

    /**
     * @brief Function to accept the connection to the client.
     * @return Returns the socket to receive data
     */
    SOCKET AcceptConnection();

	/**
	* @brief Send data function via socket.
	* @param[in] data to be send
	* @param[in] dataLength size of the data to be sent
	* @return Returns number of bytes which has been sent
	*/
	int32_t Send(const char* data, int32_t dataLength);

	/**
	* @brief Function to receive data, runs in a thread
	*/
	void ReceiveMessages();

	/**
	* @brief Validates the header of the message to determine if message is valid
	* @param[in]  msgHeader filled message header structure
	* @return true if valid header was found, otherwise false
	*/
	bool ValidateHeader(const SMsgHeader& msgHeader);

	/**
	* @brief read header and get the values from the header. In case verify == true validate the input values
	* @param[in] msgSize size of the message without headers
	* @param[in] msgId message id of the message
	* @param[in] packageNumber package number of the message
	* @param[in] totalPackageCount number of packages the message requires
	* @param[in] verifyHeader If true verify that the input of msgId and packageNumber match the header values
	* @return if verify == false, return true if a valid header can be read.
	* if verify == true input values of msdId and package number must match with the header values.
	*/
	bool ReadMessageHeader(uint32_t &msgSize, uint32_t &msgId, uint32_t &packageNumber, uint32_t &totalPackageCount, bool verifyHeader);

	/**
	* @brief read number of bytes and write them to the given buffer
	* @param[in] buffer Buffer the data is stored
	* @param[in] length of the buffer to be filled
	* @return return true if the number of bytes can be read, otherwise false
	*/
	bool ReadNumberOfBytes(char* buffer, uint32_t length);
};

#endif // !define CHANNEL_H
