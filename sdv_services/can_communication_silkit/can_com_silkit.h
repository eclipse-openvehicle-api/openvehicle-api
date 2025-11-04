#ifndef CAN_COM_SILKIT_H
#define CAN_COM_SILKIT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <set>

//VAPI includes
#include <interfaces/can.h>
#include <support/component_impl.h>
#include <support/timer.h>

//SilKit includes
#include "silkit/SilKit.hpp"

/**
* @brief Component to establish Socket CAN communication between VAPI and external application
*/
class CCANSilKit : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::can::IRegisterReceiver,
    public sdv::can::ISend, sdv::can::IInformation
{
public:

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::can::IRegisterReceiver)
        SDV_INTERFACE_ENTRY(sdv::can::ISend)
        SDV_INTERFACE_ENTRY(sdv::can::IInformation)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("CAN_Com_SilKit")
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
     * @param[in] sSDVCanMessage Message that is to be sent. The source node information is ignored. The target node determines over
     * what interface the message will be sent.
     * @param[in] uiIfcIndex Interface index to use for sending.
     */
    virtual void Send(/*in*/ const sdv::can::SMessage& sSDVCanMessage, /*in*/ uint32_t uiIfcIndex) override;

    /**
     * @brief Get a list of interface names. Overload of sdv::can::IInformation::GetInterfaces.
     * @return Sequence containing the names of the interfaces.
     */
    virtual sdv::sequence<sdv::u8string> GetInterfaces() const override;

private:

    /**
    * @brief Acknowledge struct used to synchronize between Transmit and Acknowledge callback.
    */
    struct SAcknowledgeSync : public SilKit::Services::Can::CanFrameTransmitEvent
    {
        // False positive warning of CppCheck concerning the initialization of member variables. Suppress warning.
        // cppcheck-suppress uninitDerivedMemberVar
       /**
        * @brief Constructor
        */
       SAcknowledgeSync() : SilKit::Services::Can::CanFrameTransmitEvent{} {}

       /**
        * @brief Mutex used for synchronization.
        */
       std::mutex mtx;

       /**
        * @brief Condition variable to trigger transmission callback has been received.
        */
       std::condition_variable cv;

       /**
        * @brief FLag to indicate whether transmit acknowledge has been sent.
        */
       bool bProcessed = false;
    };

    /**
     * @brief Reading configuration for SilKIt from JSON.
     * @param[in] ssSilKitJSONConfigContent SilKit JSON config file.
     * @return Return true if SilKIt JSON could be parsed successfully
    */
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> GetSilKitConfig(const std::string& ssSilKitJSONConfigContent);

    /**
      * @brief Create Participant with unique name
      * @param[in] ssSilKitJSONConfigContent SilKit JSON config file.
      * @param[in] ssSilKitRegistryUri  SilKit Registry URI.
      * @return SilKit::IParticipant, nullptr on failure
     */
    std::unique_ptr<SilKit::IParticipant> CreateParticipantFromJSONConfig(const std::string& ssSilKitJSONConfigContent, const std::string& ssSilKitRegistryUri);

    /**
     * @brief Create SilKit can controller.
     * @param[in] ssSilKitNetwork SilKit network.
     * @return SilKit::Services::Can::ICanController, nullptr on failure.
    */
   SilKit::Services::Can::ICanController* CreateController(const std::string& ssSilKitNetwork);

    /**
    * @brief Validate if the configuration includes all required settings
    * @param[in] ssSilKitJSONConfigContent SilKit JSON config file.
    * @param[in] ssSilKitNetwork Declaration of SilKit CAN network.
    * @param[in] ssSilKitRegistryUri SilKit Registry URI.
    * @return Return true if required settings are available
    */
   bool ValidateConfiguration(const std::string& ssSilKitJSONConfigContent, const std::string& ssSilKitNetwork, const std::string& ssSilKitRegistryUri);

    /**
     * @brief Function for SilKit Timesyncservice creation and to set simulation step handler.
     */
    void SetupTimeSyncService();

    /**
     * @brief Function to setup CAN interfaces.
     * @param[in] ssSilKitJSONConfigContent SilKit JSON config file.
     * @param[in] ssSilKitNetwork Declaration of SilKit CAN network.
     * @param[in] ssSilKitRegistryUri SilKit Registry URI.
     * @return Return true if CAN interfaces are setup successfully
     */
    bool CreateSilKitConnection(const std::string& ssSilKitJSONConfigContent, const std::string& ssSilKitNetwork, const std::string& ssSilKitRegistryUri);
    
    /**
     * @brief Create lifecycle service.
     * @return Return SilKit lifecycle service.
     */
    SilKit::Services::Orchestration::ILifecycleService* CreateSilKitLifecycleService();

    /**
    * @brief Set all SillKit handler functiones
    * @return Return SilKit lifecycle service.
    */
    bool SetHandlerFunctions(SilKit::Services::Orchestration::ILifecycleService* silKitlifeCyleService);

    /**
     * @brief Method to receive CAN frame via SilKit
     * @param[in] rsSilKitCanFrame CAN frame in SilKit format.
     */
    void SilKitReceiveMessageHandler(const SilKit::Services::Can::CanFrame& rsSilKitCanFrame);

    /**
     * @brief Method to transmit acknowledgement callback.
     * @param[in] rsSilKitTransmitAcknowledge SilKit CAN message transmit acknowledgement.
     */
    void SilKitTransmitAcknowledgeHandler(const SilKit::Services::Can::CanFrameTransmitEvent& rsSilKitTransmitAcknowledge);

    std::mutex                                           m_ReceiversMtx;                     ///< Protect the receiver set.
    std::set<sdv::can::IReceive*>                        m_SetReceivers;                     ///< Set with receiver interfaces.
     
    std::queue<sdv::can::SMessage>                       m_MessageQueue;                     ///< Map of the messages to be sent on SilKit.
    std::mutex                                           m_QueueMutex;                       ///< Protection for message map.
    
    SilKit::Services::Orchestration::ILifecycleService*  m_SilKitLifeCycleService = nullptr; ///< SilKit lifecycle service.
    SilKit::Services::Can::ICanController*               m_SilKitCanController = nullptr;    ///< SilKit CAN1 Controller interface.
    sdv::core::ITimerSimulationStep*                     m_TimerSimulationStep = nullptr;    ///< Timer simulation step.
    std::unique_ptr<SilKit::IParticipant>                m_SilKitParticipant = nullptr;      ///< SilKit participant.
    std::string                                          m_SilKitParticipantName;            ///< Configured SilKit participants.
    bool                                                 m_SilKitIsSynchronousMode = false;  ///< SilKit sync mode when true.

    uint32_t                                             m_maxCanDataLength = 8;             ///< maximum size of the CAN message.
    std::atomic<sdv::EObjectStatus>                      m_eStatus = sdv::EObjectStatus::initialization_pending;  ///< Object status.
};

DEFINE_SDV_OBJECT(CCANSilKit)

#endif // ! defined CAN_COM_SILKIT_H
