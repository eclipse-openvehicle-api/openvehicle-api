/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Sudipta Durjoy - initial API and implementation
 *   Thomas Pfleiderer - refactored and finalized 
 ********************************************************************************/

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
class CCANSilKit : public sdv::CSdvObject, public sdv::can::IRegisterReceiver, public sdv::can::ISend, sdv::can::IInformation
{
public:

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::can::ISend)
        SDV_INTERFACE_ENTRY(sdv::can::IInformation)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_bus)
    DECLARE_OBJECT_CLASS_NAME("CAN_Com_SilKit")
    DECLARE_DEFAULT_OBJECT_NAME("CAN_Communication_Object")
    DECLARE_OBJECT_SINGLETON()

    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_ENTRY(m_SilKitJSONConfigContent, "SilKitConfig", "", "", "SilKit Config JSON.")
        SDV_PARAM_ENTRY(m_SilKitParticipantName, "SilKitParticipantName", "", "", "Name of the participant.")
        SDV_PARAM_ENTRY(m_SilKitNetwork, "CanSilKitNetwork", "", "", "Declaration of SilKit CAN network.")
        SDV_PARAM_ENTRY(m_SilKitRegistryUri, "RegistryURI", "", "", "SilKit Registry URI.")
        SDV_PARAM_ENTRY(m_SilKitIsSynchronousMode, "SyncMode", "", "", "Enable synchronization mode.")
        SDV_PARAM_ENTRY(m_SilKitDebugInfo, "DebugInfo", "", "", "Enable debug information.")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @details The CAN_Com_SilKit uses the following configuration:
     * @code
     *   DebugInfo = true
     *   SyncMode = true
     *   SilKitParticipantName = "can_writer"
     *   CanSilKitNetwork = "PrivateCAN"
     *   RegistryURI = "silkit://localhost:8500"
     *   SilKitConfig = """{
     *                       "Logging": {
     *                           "Sinks": [ { "Type": "Stdout", "Level": "Info" } ]
     *                   },
     *       }"""
     * @endcode
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

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
    * @return Return true if required settings are available
    */
    bool ValidateConfiguration();

    /**
     * @brief Function for SilKit Timesyncservice creation and to set simulation step handler.
     */
    void SetupTimeSyncService();

    /**
     * @brief Function to setup CAN interfaces.
     * @return Return true if CAN interfaces are setup successfully
     */
    bool CreateSilKitConnection();
    
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

    std::mutex                              m_ReceiversMtx;                     ///< Protect the receiver set.
    std::set<sdv::can::IReceive*>           m_SetReceivers;                     ///< Set with receiver interfaces.
     
    std::queue<sdv::can::SMessage>          m_MessageQueue;                     ///< Map of the messages to be sent on SilKit.
    std::mutex                              m_QueueMutex;                       ///< Protection for message map.
    
    SilKit::Services::Orchestration::ILifecycleService* m_SilKitLifeCycleService = nullptr; ///< SilKit lifecycle service.
    SilKit::Services::Can::ICanController*  m_SilKitCanController = nullptr;    ///< SilKit CAN1 Controller interface.
    sdv::core::ITimerSimulationStep*        m_TimerSimulationStep = nullptr;    ///< Timer simulation step.
    std::unique_ptr<SilKit::IParticipant>   m_SilKitParticipant = nullptr;      ///< SilKit participant.
    std::string                             m_SilKitParticipantName;            ///< Configured SilKit participants.
    bool                                    m_SilKitIsSynchronousMode = false;  ///< SilKit sync mode when true.
    bool                                    m_SilKitDebugInfo = false;          ///< SilKit debug information when true.

    uint32_t                                m_maxCanDataLength = 8;             ///< maximum size of the CAN message.
    std::string                             m_SilKitNetwork;                    ///< Declaration of SilKit CAN network.
    std::string                             m_SilKitJSONConfigContent;          ///< SilKit config JSON.
    std::string                             m_SilKitRegistryUri;
};

DEFINE_SDV_OBJECT(CCANSilKit)

#endif // ! defined CAN_COM_SILKIT_H
