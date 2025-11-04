#ifndef CAN_COM_SIMULATION_H
#define CAN_COM_SIMULATION_H

#include <iostream>
#include <queue>
#include <set>
#include <thread>
#include <mutex>

#include <interfaces/can.h>
#include <support/component_impl.h>
#include "../../global/ascformat/ascreader.h"
#include "../../global/ascformat/ascwriter.h"

/**
* @brief Component to establish Socket CAN communication between VAPI and external application
*/
class CCANSimulation : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::can::IRegisterReceiver,
    public sdv::can::ISend, sdv::can::IInformation
{
public:
    /**
     * @brief Constructor
     */
    CCANSimulation();

    /**
     * @brief Destructor
     */
    virtual ~CCANSimulation() override;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::can::IRegisterReceiver)
        SDV_INTERFACE_ENTRY(sdv::can::ISend)
        SDV_INTERFACE_ENTRY(sdv::can::IInformation)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("CAN_Com_Sim")
    DECLARE_DEFAULT_OBJECT_NAME("CAN_Communication_Object")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    virtual sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    virtual void Shutdown() override;

    /**
     * @brief Register a CAN message receiver. Overload of sdv::can::IRegisterReceiver::RegisterReceiver.
     * @param[in] pReceiver Pointer to the receiver interface.
     */
    virtual void RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver) override;

    /**
     * @brief Unregister a previously registered CAN message receiver. Overload of
     * sdv::can::IRegisterReceiver::UnregisterReceiver.
     * @param[in] pReceiver Pointer to the receiver interface.
     */
    virtual void UnregisterReceiver(/*in*/ sdv::can::IReceive* pReceiver) override;

    /**
     * @brief Send a CAN message. Overload of sdv::can::ISend::Send.
     * @param[in] sMsg Message that is to be sent. The source node information is ignored. The target node determines over
     * what interface the message will be sent.
     * @param[in] uiIfcIndex Interface index to use for sending.
     */
    virtual void Send(/*in*/ const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiIfcIndex) override;

    /**
     * @brief Get a list of interface names. Overload of sdv::can::IInformation::GetInterfaces.
     * @return Sequence containing the names of the interfaces.
     */
    virtual sdv::sequence<sdv::u8string> GetInterfaces() const override;

private:
    /**
     * @brief Playback function for ASC data playback.
     */
    void PlaybackFunc(const asc::SCanMessage& rsMsg);

    std::atomic<sdv::EObjectStatus>             m_eStatus = sdv::EObjectStatus::initialization_pending;  ///< Object status
    std::thread                                 m_threadReceive;            ///< Receive thread.
    mutable std::mutex                          m_mtxReceivers;             ///< Protect the receiver set.
    std::set<sdv::can::IReceive*>               m_setReceivers;             ///< Set with receiver interfaces.
    mutable std::mutex                          m_mtxInterfaces;            ///< Protect the nodes set.
    std::map<int, size_t>                       m_mapIfc2Idx;               ///< Map with interface to index.
    std::vector<std::pair<int, std::string>>    m_vecInterfaces;            ///< Vector with interfaces.
    std::filesystem::path                       m_pathSource;               ///< Path to the source ASC file.
    std::filesystem::path                       m_pathTarget;               ///< Path to the target ASC file.
    asc::CAscReader                             m_reader;                   ///< Reader for ASC file playback.
    asc::CAscWriter                             m_writer;                   ///< Writer for ASC file recording.
};

DEFINE_SDV_OBJECT(CCANSimulation)

#endif // ! defined CAN_COM_SIMULATION_H
