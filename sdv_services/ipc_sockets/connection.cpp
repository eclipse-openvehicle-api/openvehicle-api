#include "connection.h"

CConnection::CConnection(SOCKET preconfiguredSocket, bool acceptConnectionRequired)
{
    std::fill(std::begin(m_SendBuffer), std::end(m_SendBuffer), '\0');
    std::fill(std::begin(m_ReceiveBuffer), std::end(m_ReceiveBuffer), '\0');
	m_ConnectionStatus = sdv::ipc::EConnectStatus::uninitialized;
	m_ConnectionSocket = preconfiguredSocket;
	m_AcceptConnectionRequired = acceptConnectionRequired;
}

int32_t CConnection::Send(const char* data, int32_t dataLength)
{
	int32_t bytesSent = send(m_ConnectionSocket, data, dataLength, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		SDV_LOG_ERROR("send failed with error: ", std::to_string(WSAGetLastError()));
		m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
		m_ConnectionSocket = INVALID_SOCKET;
	}

	return bytesSent;
}

bool CConnection::SendData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData)
{
	static uint32_t msgId = 0;
	msgId++;
	size_t requiredSize = 0;
	std::for_each(seqData.cbegin(),
		seqData.cend(),
		[&](const sdv::pointer<uint8_t>& rBuffer) { requiredSize += rBuffer.size(); });

	std::unique_lock<std::recursive_mutex> lock(m_SendMutex);

	uint32_t packageNumber    = 1;
	SMsgHeader msgHeader;
	msgHeader.msgStart	       = m_MsgStart;
	msgHeader.msgEnd	       = m_MsgEnd;
	msgHeader.msgId		       = msgId;
	msgHeader.msgSize		   = static_cast<uint32_t>(requiredSize);
	msgHeader.packetNumber     = packageNumber;
	msgHeader.totalPacketCount = static_cast<uint32_t>((requiredSize + m_SendMessageSize - 1) / m_SendMessageSize);
	memcpy_s(&m_SendBuffer[0], m_SendBufferSize, &msgHeader, sizeof(msgHeader));
	uint32_t offsetBuffer = sizeof(SMsgHeader);

	std::for_each(seqData.cbegin(),
		seqData.cend(),
		[&](const sdv::pointer<uint8_t>& rBuffer)
		{
			uint32_t offsetData	 = 0;
			while (rBuffer.size() > offsetData)
			{
				if (offsetBuffer == 0)
				{
					msgHeader.packetNumber = packageNumber;
					memcpy_s(&m_SendBuffer[0], m_SendBufferSize, &msgHeader, sizeof(msgHeader));
					offsetBuffer = sizeof(SMsgHeader);
				}

				auto availableBufferSize = m_SendBufferSize - offsetBuffer;

				if (availableBufferSize > (rBuffer.size() - offsetData))
				{
					// fragments fits in buffer, go to next fragment without sending
					memcpy_s(&m_SendBuffer[offsetBuffer],
						availableBufferSize,
						rBuffer.get() + offsetData,
						rBuffer.size() - offsetData);

					offsetBuffer += (static_cast<uint32_t>(rBuffer.size()) - offsetData);
					break;
				}
				else
				{
					// fragments exceeds buffer, fill buffer, send buffer, keep fragment
					memcpy_s(&m_SendBuffer[offsetBuffer],
						availableBufferSize,
						rBuffer.get() + offsetData,
						availableBufferSize);

					Send(m_SendBuffer, static_cast<int>(m_SendBufferSize));
					offsetData += (static_cast<uint32_t>(availableBufferSize));
					offsetBuffer = 0;
					packageNumber++;
				}
			}
		});

	if (0 != offsetBuffer)
	{
		Send(m_SendBuffer, static_cast<int>(offsetBuffer));
	}

	return true;
}

SOCKET CConnection::AcceptConnection()
{
	SOCKET clientSocket{INVALID_SOCKET};

	int32_t attempt = 0;
	while (clientSocket == INVALID_SOCKET)
	{
		clientSocket = accept(m_ConnectionSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			Sleep(2);
		}
		attempt++;
		if (attempt > 2000)
		{
			SDV_LOG_ERROR("Accept socket failed, loop exeeded.");
			m_ConnectionStatus = sdv::ipc::EConnectStatus::connection_error;
			m_ConnectionSocket = INVALID_SOCKET;
			break;
		}
	}

	return clientSocket;
}

bool CConnection::AsyncConnect(sdv::IInterfaceAccess* pReceiver)
{
	m_ConnectionStatus = sdv::ipc::EConnectStatus::initializing;
	if (m_AcceptConnectionRequired)
	{
		m_ConnectionSocket = AcceptConnection();
	}

	if (m_ConnectionSocket == INVALID_SOCKET)
	{
		m_ConnectionStatus = sdv::ipc::EConnectStatus::connection_error;
		return false;
	}
	m_pReceiver = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IDataReceiveCallback>();
	m_pEvent = sdv::TInterfaceAccessPtr(pReceiver).GetInterface<sdv::ipc::IConnectEventCallback>();
	m_ReceiveThread = std::thread(std::bind(&CConnection::ReceiveMessages, this));
	m_ConnectionStatus = sdv::ipc::EConnectStatus::connected;

	// TODO: Connection negotiation didn't take place... implement this!

	return true;
}

bool CConnection::WaitForConnection(/*in*/ uint32_t /*uiWaitMs*/)
{
	if (m_ConnectionStatus == sdv::ipc::EConnectStatus::connected) return true;

	// TODO: Implementation here!

	// TODO: Suppress static code analysis while there is no implementation yet.
	// cppcheck-suppress identicalConditionAfterEarlyExit
	return m_ConnectionStatus == sdv::ipc::EConnectStatus::connected;
}

void CConnection::CancelWait()
{
	// TODO: Implementation here!
}

void CConnection::Disconnect()
{
	// TODO: Implementation here!
}

uint64_t CConnection::RegisterStatusEventCallback(/*in*/ sdv::IInterfaceAccess* /*pEventCallback*/)
{
	// TODO: Implementation here!
	return 0;
}

void CConnection::UnregisterStatusEventCallback(/*in*/ uint64_t /*uiCookie*/)
{
	// TODO: Implementation here!
}

sdv::ipc::EConnectStatus CConnection::GetStatus() const
{
	return m_ConnectionStatus;
}

void CConnection::DestroyObject()
{
	m_StopReceiveThread = true;
	if (m_ReceiveThread.joinable())
	{
		m_ReceiveThread.join();
	}
	closesocket(m_ConnectionSocket);
	m_ConnectionSocket = INVALID_SOCKET;
	m_ConnectionStatus = sdv::ipc::EConnectStatus::disconnected;

	delete this;
}

bool CConnection::ValidateHeader(const SMsgHeader& msgHeader)
{
	if ((msgHeader.msgStart == m_MsgStart) && (msgHeader.msgEnd == m_MsgEnd))
	{
		if (msgHeader.msgSize != 0)
		{
			return true;
		}
	}

	return false;
}


bool CConnection::ReadNumberOfBytes(char* buffer, uint32_t bufferLength)
{
	uint32_t bytesReceived = 0;
	while (!m_StopReceiveThread && (bufferLength > bytesReceived))
	{
		bytesReceived += recv(m_ConnectionSocket, buffer + bytesReceived, bufferLength - bytesReceived, 0);
		if (bytesReceived == static_cast<uint32_t>(SOCKET_ERROR))
		{
			auto error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				if(error == WSAECONNRESET)
				{
					SDV_LOG_INFO("Reset SOCKET, recv() failed with error: ", error, "   ", m_StopReceiveThread);
					m_ConnectionStatus = sdv::ipc::EConnectStatus::disconnected;
					m_ConnectionSocket = INVALID_SOCKET;
					break;
				}
				else if (!m_StopReceiveThread)
				{
					SDV_LOG_ERROR("SOCKET_ERROR, recv() failed with error: ", error);
					m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
					m_ConnectionSocket = INVALID_SOCKET;
					break;
				}
			}
			bytesReceived = 0;
		}
	}

	if (bufferLength != bytesReceived)
	{
		if (!m_StopReceiveThread || (m_ConnectionStatus == sdv::ipc::EConnectStatus::disconnected))
		{
			SDV_LOG_INFO("The expected bytes could not be received.");
		}

		return false;
	}

	return true;
}


bool CConnection::ReadMessageHeader(uint32_t& msgSize, uint32_t& msgId, uint32_t& packageNumber, uint32_t& totalPackageCount, bool verifyHeader)
{
	SMsgHeader msgHeader;

	if (ReadNumberOfBytes(reinterpret_cast<char*>(&msgHeader), sizeof(SMsgHeader)))
	{
		if (ValidateHeader(msgHeader))
		{
			if (!verifyHeader)
			{
				if (msgHeader.msgSize == 0)
				{
					return false;
				}
				msgSize           = msgHeader.msgSize;
				msgId             = msgHeader.msgId;
				packageNumber     = msgHeader.packetNumber;
				totalPackageCount = msgHeader.totalPacketCount;
				return true;
			}
			else
			{
				if ((msgId == msgHeader.msgId) && (packageNumber == msgHeader.packetNumber))
				{
					return true;
				}
				else
				{
					SDV_LOG_WARNING("Received wrong message, Id = ", std::to_string(msgHeader.msgId), " package = ",
						std::to_string(packageNumber), " (expected id = ", std::to_string(msgId), " package = ", std::to_string(packageNumber), ")");
				}
			}
		}
		else
		{
			if (!m_StopReceiveThread)
			{
				SDV_LOG_WARNING("Could not read message header");
			}
		}
	}

	return false;
}


void CConnection::ReceiveMessages()
{
	while (!m_StopReceiveThread && (m_ConnectionSocket != INVALID_SOCKET))
	{
		uint32_t messageSize{ 0 };
		uint32_t msgId{ 0 };
		uint32_t packageNumber{ 0 };
		uint32_t totalPackageCount{ 0 };

		if (ReadMessageHeader(messageSize, msgId, packageNumber, totalPackageCount, false))
		{
			uint32_t dataOffset{ 0 };
			sdv::pointer<uint8_t> message;
			message.resize(messageSize);
			for (uint32_t package = 1; package <= totalPackageCount; package++)
			{
                uint32_t bytesToBeRead = m_SendMessageSize;
				if (package == totalPackageCount)
				{
					bytesToBeRead = messageSize - dataOffset;  // last package
				}

				if (package != 1)
				{
					if (!ReadMessageHeader(messageSize, msgId, package, totalPackageCount, true))
					{
						m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
						m_ConnectionSocket = INVALID_SOCKET;
						break;
					}
				}

				if (!ReadNumberOfBytes(reinterpret_cast<char*>(message.get()) + dataOffset, bytesToBeRead))
				{
					m_ConnectionStatus = sdv::ipc::EConnectStatus::communication_error;
					m_ConnectionSocket = INVALID_SOCKET;
					break;
				}

				dataOffset += bytesToBeRead;
			}

			if (!m_StopReceiveThread && (m_pReceiver != nullptr) && (m_ConnectionSocket != INVALID_SOCKET)) // In case of shutdown the message maybe invalid/incomplete
			{
				//m_pReceiver->ReceiveData(message);
			}
		}
		else
		{
			Sleep(1);
		}
	}
}
