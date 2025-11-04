#ifndef CHANNEL_MGNT_H
#define CHANNEL_MGNT_H

#include <support/component_impl.h>
#include <interfaces/ipc.h>
#include "connection.h"
#include "shared_mem_buffer_posix.h"
#include "shared_mem_buffer_windows.h"
#include "watchdog.h"
#include "connection.h"

#define TEST_DECLARE_OBJECT_CLASS_ALIAS(...)                                                                                            \
    static sdv::sequence<sdv::u8string> GetClassAliasesStaticMyTest()                                                                    \
    {                                                                                                                              \
        return sdv::sequence<sdv::u8string>({__VA_ARGS__});                                                                        \
    }


/**
 * @brief IPC channel management class for the shared memory communication.
 */
class CSharedMemChannelMgnt : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::ipc::ICreateEndpoint,
    public sdv::ipc::IChannelAccess
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::ipc::IChannelAccess)
        SDV_INTERFACE_ENTRY(sdv::ipc::ICreateEndpoint)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("DefaultSharedMemoryChannelControl")
    DECLARE_OBJECT_CLASS_ALIAS("LocalChannelControl")
    DECLARE_DEFAULT_OBJECT_NAME("LocalChannelControl")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Create IPC connection object and return the endpoint information. Overload of
     * sdv::ipc::ICreateEndpoint::CreateEndpoint.
     * @details The endpoints are generated using either a size and a name based on the provided channel configuration or if no
     * configuration is supplied a default size of 10k and a randomly generated name. The following configuration
     * can be supplied:
     * @code
     * [IpcChannel]
     * Name = "CHANNEL_1234"
     * Size = 10240
     * @endcode
     * @param[in] ssChannelConfig Optional channel type specific endpoint configuration.
     * @return IPC connection object
     */
    sdv::ipc::SChannelEndpoint CreateEndpoint(/*in*/ const sdv::u8string& ssChannelConfig) override;

    /**
     * @brief Create a connection object from the channel connection parameters string
     * @param[in] ssConnectString Reference to the string containing the channel connection parameters.
     * @return Pointer to IInterfaceAccess interface of the connection object or NULL when the object cannot be created.
     */
    sdv::IInterfaceAccess* Access(const sdv::u8string& ssConnectString) override;

private:

    /**
     * @brief Shared memory bridge
     * @attention The bridge is created here, but potentially used by two separated processed. To prevent channel destruction
     * keep the bridge alive.
     * @attention Under Posix, the unmapping in one process counts for all connections to this buffer within the process. Creating
     * additional buffer access interfaces might result in the buffer becoming invalid when one of them is removed again.
     */
    struct SChannel
    {
        /**
         * @brief Constructor
         */
        SChannel() : bufferTargetTx(bufferOriginRx.GetConnectionString()), bufferTargetRx(bufferOriginTx.GetConnectionString())
        {
            if (!bufferOriginRx.IsValid()) std::cout << "Channel Origin RX is invalid!" << std::endl;
            if (!bufferOriginTx.IsValid()) std::cout << "Channel Origin TX is invalid!" << std::endl;
            if (!bufferTargetRx.IsValid()) std::cout << "Channel Target RX is invalid!" << std::endl;
            if (!bufferTargetTx.IsValid()) std::cout << "Channel Target TX is invalid!" << std::endl;
        }

        CSharedMemBufferTx  bufferOriginTx;     ///< Origin Tx channel
        CSharedMemBufferRx  bufferOriginRx;     ///< Origin Rx channel
        CSharedMemBufferTx  bufferTargetTx;     ///< Target Tx channel
        CSharedMemBufferRx  bufferTargetRx;     ///< Target Rx channel
    };

    sdv::EObjectStatus  m_eObjectStatus = sdv::EObjectStatus::initialization_pending;    ///< Object status.
    std::map<std::string, std::unique_ptr<SChannel>>    m_mapChannels;      ///< Map with channels.
    CWatchDog                   m_watchdog;                                 ///< Process monitor for connections.
};
DEFINE_SDV_OBJECT(CSharedMemChannelMgnt)

#endif // ! defined CHANNEL_MGNT_H