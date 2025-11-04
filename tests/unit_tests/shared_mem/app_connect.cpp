#include <support/mem_access.h>
#include <support/app_control.h>
#include "../../../global/base64.h"

#define TIME_TRACKING
#include "../../../sdv_services/ipc_shared_mem/channel_mgnt.cpp"
#include "../../../sdv_services/ipc_shared_mem/connection.cpp"
#include "../../../sdv_services/ipc_shared_mem/watchdog.cpp"
#include "../../../sdv_services/ipc_shared_mem/mem_buffer_accessor.cpp"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#ifdef __GNUC__
#include <unistd.h>
#endif

/**
* @brief Receiver helper class.
*/
class CReceiver : public sdv::IInterfaceAccess, public sdv::ipc::IDataReceiveCallback, public sdv::ipc::IConnectEventCallback
{
public:
    /**
     * @brief Constructor
     */
    CReceiver()
    {
        m_threadSender = std::thread(&CReceiver::SendThreadFunc, this);
    }

    /**
     * @brief Destructor
     */
    ~CReceiver()
    {
        m_bShutdown = true;
        if (m_threadSender.joinable())
            m_threadSender.join();
    }

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IDataReceiveCallback)
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Assign a sender interface.
     * @param[in] pSend Pointer to the sending interface.
     */
    void AssignSender(sdv::ipc::IDataSend* pSend)
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_pSend = pSend;
    }

    /**
    * @brief Callback to be called by the IPC connection when receiving a data packet. Overload
    * sdv::ipc::IDataReceiveCallback::ReceiveData.
    * @param[inout] seqData Sequence of data buffers to received. The sequence might be changed to optimize the communication
    * without having to copy the data.
    */
    virtual void ReceiveData(/*inout*/ sdv::sequence<sdv::pointer<uint8_t>>& seqData) override
    {
        // Send the same data back again (if needed).
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_queueSendData.push(seqData);
        lock.unlock();

        m_cvReceived.notify_all();
    }

    /**
     * @brief Send thread for sending data
     */
    void SendThreadFunc()
    {
        while (!m_bShutdown && !m_bDisconnect)
        {
            // Wait for data to be received
            std::unique_lock<std::mutex> lock(m_mtxData);
            if (m_queueSendData.empty())
            {
                m_cvReceived.wait_for(lock, std::chrono::milliseconds(100));
                continue;
            }
            sdv::sequence<sdv::pointer<uint8_t>> seqData = std::move(m_queueSendData.front());
            m_queueSendData.pop();
            lock.unlock();

            // Send the data back to the sender
            if (m_pSend)
                m_pSend->SendData(seqData);
        }
    }

    /**
    * @brief Set the current status. Overload of sdv::ipc::IConnectEventCallback::SetStatus.
    * @param[in] eConnectStatus The connection status.
    */
    virtual void SetStatus(/*in*/ sdv::ipc::EConnectStatus eConnectStatus) override
    {
        switch (eConnectStatus)
        {
        case sdv::ipc::EConnectStatus::disconnected:
            // Disconnect only when connected before
            if (m_bConnected)
            {
                std::cout << "Forced disconnect communicated..." << std::endl;
                m_bDisconnect = true;
                m_cvDisconnect.notify_all();
            }
            break;
        case sdv::ipc::EConnectStatus::disconnected_forced:
            // Disconnect only when connected before
            if (m_bConnected)
            {
                std::cout << "Disconnect communicated..." << std::endl;
                m_bDisconnect = true;
                m_cvDisconnect.notify_all();
            }
            break;
        case sdv::ipc::EConnectStatus::connected:
            m_bConnected = true;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Wait until the disconnect is called.
     * @param[in] uiDurationMs The wait duration until timeout.
     * @return Returns whether a disconnect event was triggered or otherwise a timeout occurred.
     */
    bool WaitUntilDisconnect(uint32_t uiDurationMs = 5000)
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_cvDisconnect.wait_for(lock, std::chrono::milliseconds(uiDurationMs));
        lock.unlock();

        // Shutdown the thread already
        m_bShutdown = true;
        if (m_threadSender.joinable())
            m_threadSender.join();

        return m_bDisconnect;
    }

    /**
     * @brief Wait until the caller hasn't sent anything anymore for 1 second.
     */
    void WaitForNoActivity()
    {
        CConnection* pConnection = dynamic_cast<CConnection*>(m_pSend);

        // Wait until there is no activity any more.
        std::chrono::high_resolution_clock::time_point tpTickSent = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point tpTickReceive = std::chrono::high_resolution_clock::now();
        while (!m_bDisconnect)
        {
            std::unique_lock<std::mutex> lock(m_mtxData);
            m_cvReceived.wait_for(lock, std::chrono::milliseconds(25));

            // Does the connection have a better time?
            if (pConnection) tpTickSent = pConnection->GetLastSentTime();
            if (pConnection) tpTickReceive = pConnection->GetLastReceiveTime();
            std::chrono::high_resolution_clock::time_point tpNow = std::chrono::high_resolution_clock::now();

            // A duration of more than a second should not occur.
            if (std::chrono::duration<double>(tpNow - tpTickSent).count() > 1.0 &&
                std::chrono::duration<double>(tpNow - tpTickReceive).count() > 1.0)
                break;
        }

        TRACE("No new data for 1 second...");

        // Shutdown the thread already
        m_bShutdown = true;
        if (m_threadSender.joinable())
            m_threadSender.join();
    }

private:
    sdv::ipc::IDataSend*                                m_pSend = nullptr;              ///< Send interface to implement repeating function.
    mutable std::mutex                                  m_mtxData;                      ///< Protect data access.
    std::queue<sdv::sequence<sdv::pointer<uint8_t>>>    m_queueSendData;                 ///< Queue for sending data.
    std::condition_variable                             m_cvDisconnect;                 ///< Disconnect event.
    std::condition_variable                             m_cvReceived;                   ///< Receive event.
    std::thread                                         m_threadSender;                 ///< Thread to send data.
    bool                                                m_bConnected = false;           ///< Set when connected was triggered.
    bool                                                m_bDisconnect = false;          ///< Set when shutdown was triggered.
    bool                                                m_bShutdown = false;            ///< Set when shutdown is processed.
};


#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    if (argc < 2)
    {
        std::cout << "Missing connection string..." << std::endl;
        return -1;
    }

    sdv::app::CAppControl appcontrol;
    if (!appcontrol.Startup("")) return -1;
    appcontrol.SetRunningMode();

    std::string logName("appRepeater.log");

    // Create a connection string from the command line arguments separated by spaces. Skip the first argument, since it holds the
    // module file name.

    // The control connection information needs to be defined as the seconds argument.
    // It could hold the string "NONE"... which means it uses a data connection only.
    std::string ssControlConnectString;
    if (argc > 1)
    {
        auto ssArgv = sdv::MakeUtf8String(argv[1]);
        if (ssArgv != "NONE")
            ssControlConnectString = Base64DecodePlainText(ssArgv);
    }
    else
        return -1;

    // The third argument could be either the data connection string or additional parameters.
    std::string ssDataConnectString;
    bool bForceTerminate = false;
    bool bLongLife = false;
    bool bServer = true;
    for (int i = 2; i < argc; i++)
    {
        auto ssArgv = sdv::MakeUtf8String(argv[i]);

        // Check when forcefully termination is requested
        if (ssArgv == "FORCE_TERMINATE")
        {
            bForceTerminate = true;
            continue;
        }

        // Check for long life
        if (ssArgv == "LONG_LIFE")
        {
            bLongLife = true;
            continue;
        }

        // Try to convert to connect string (allowed only once)
        if (ssDataConnectString.empty())
        {
            ssDataConnectString = Base64DecodePlainText(ssArgv);
            bServer = ssDataConnectString.empty();
            continue;
        }

        TRACE("Unexpected command line parameter: ", ssArgv);
    }

    // At least one connection must be supplied - either control or data.
    if (ssControlConnectString.empty() && ssDataConnectString.empty()) return 10;

    TRACE("Start of connect process as ", bServer ? "server" : "client", " (", getpid(), ")...");
    TRACE("Forced termination of app ", bServer ? "server" : "client", " process is ", bForceTerminate ? "enabled" : "disabled");
    TRACE("Long life of app ",bServer ? "server" : "client", " process is ", bLongLife ? "enabled" : "disabled");

    // Create an control management channel (if required).
    CSharedMemChannelMgnt mgntControlMgntChannel;
    mgntControlMgntChannel.Initialize("");
    if (mgntControlMgntChannel.GetStatus() != sdv::EObjectStatus::initialized) return -11;
    mgntControlMgntChannel.SetOperationMode(sdv::EOperationMode::running);
    if (mgntControlMgntChannel.GetStatus() != sdv::EObjectStatus::running) return -11;

    // Open the control channel endpoint
    sdv::TObjectPtr ptrControlConnection;
    CReceiver receiverControl;
    if (!ssControlConnectString.empty())
    {
        TRACE(bServer ? "Server" : "Client", ": Start of control channel connection...");
        ptrControlConnection = mgntControlMgntChannel.Access(ssControlConnectString);
        if (!ptrControlConnection) return -12;
        sdv::ipc::IConnect* pControlConnect = ptrControlConnection.GetInterface<sdv::ipc::IConnect>();
        if (!pControlConnect) return -13;
        if (!pControlConnect->RegisterStatusEventCallback(&receiverControl)) return -20;
        if (!pControlConnect->AsyncConnect(&receiverControl)) return -14;
        if (!pControlConnect->WaitForConnection(250)) return -5;  // Note: Connection should be possible within 250ms.
        if (pControlConnect->GetStatus() != sdv::ipc::EConnectStatus::connected) return -15;
        TRACE(bServer ? "Server" : "Client", ": Connected to control channel...");
    }

    // Create the data management channel.
    CSharedMemChannelMgnt mgntDataMgntChannel;
    mgntDataMgntChannel.Initialize("");
    if (mgntDataMgntChannel.GetStatus() != sdv::EObjectStatus::initialized) return -1;
    mgntDataMgntChannel.SetOperationMode(sdv::EOperationMode::running);
    if (mgntDataMgntChannel.GetStatus() != sdv::EObjectStatus::running) return -1;

    // If this is a server, create a data endpoint and communicate this endpoint over the control channel.
    // If not, open the data endpoint.
    sdv::TObjectPtr ptrDataConnection;
    if (bServer)
    {
        TRACE("Server: Create data endpoint...");
        sdv::ipc::SChannelEndpoint sEndpoint = mgntDataMgntChannel.CreateEndpoint(R"code(
[IpcChannel]
Size = 1024000
)code");
        ptrDataConnection = sEndpoint.pConnection;
        sdv::pointer<uint8_t> ptrConnectInfoData;
        ptrConnectInfoData.resize(sEndpoint.ssConnectString.size());
        size_t n = 0;
        for (char c : sEndpoint.ssConnectString)
            ptrConnectInfoData[n++] = static_cast<uint8_t>(c);
        sdv::sequence<sdv::pointer<uint8_t>> seqData({ ptrConnectInfoData });
        sdv::ipc::IDataSend* pControlDataSend = ptrControlConnection.GetInterface<sdv::ipc::IDataSend>();
        if (!pControlDataSend) return -16;
        if (!pControlDataSend->SendData(seqData))
            return -17;
        TRACE("Server: Communicated data endpoint...");
    }
    else // Open the data endpoint
    {
        TRACE("Client: Accessed data endpoint...");
        ptrDataConnection = mgntDataMgntChannel.Access(ssDataConnectString);
    }
    if (!ptrDataConnection) return -2;

    CReceiver receiverData;
    sdv::ipc::IDataSend* pDataSend = ptrDataConnection.GetInterface<sdv::ipc::IDataSend>();
    if (!pDataSend) return -2;
    receiverData.AssignSender(pDataSend);

    // Establish the connection
    sdv::ipc::IConnect* pDataConnect = ptrDataConnection.GetInterface<sdv::ipc::IConnect>();
    if (!pDataConnect) return -3;
    if (!pDataConnect->RegisterStatusEventCallback(&receiverData)) return -21;
    if (!pDataConnect->AsyncConnect(&receiverData)) return -4;
    if (!pDataConnect->WaitForConnection(10000)) return -5;  // Note: Connection should be possible within 10000ms.
    if (pDataConnect->GetStatus() != sdv::ipc::EConnectStatus::connected) return -5;

    TRACE("Connected to data channel... waiting for data exchange");

    // Check for long life
    if (bLongLife)
    {
        // Wait until the sender doesn't send anything any more.
        receiverData.WaitForNoActivity();
    }
    else
    {
        // Repeat data until disconnect occurrs (differentiate between forced and not forced to allow two apps to start at the
        // same time).
        receiverData.WaitUntilDisconnect(bForceTerminate ? 800 : 1600);
        std::cout << "App " << (bServer ? "server" : "client") << " connect process disconnecting..." << std::endl;
    }

    if (bForceTerminate)
    {
        std::cout << "Forced termination of app " << (bServer ? "server" : "client") << " connect process..." << std::endl;
#ifdef _MSC_VER
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
        std::_Exit(0);   // Force exit without cleaning up...
    }

    // Initiate shutdown
    ptrDataConnection.Clear();
    mgntDataMgntChannel.Shutdown();
    if (mgntDataMgntChannel.GetStatus() != sdv::EObjectStatus::destruction_pending) return -6;
    ptrControlConnection.Clear();
    mgntControlMgntChannel.Shutdown();
    if (mgntControlMgntChannel.GetStatus() != sdv::EObjectStatus::destruction_pending) return -16;

    TRACE("Shutdown of app ", bServer ? "server" : "client", " connect process...");

    appcontrol.Shutdown();

    // Done....
    return 0;
}
