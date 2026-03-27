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

#include "can_com_silkit.h"
#include <support/toml.h>
#include <bitset>

bool CCANSilKit::OnInitialize()
{
    m_TimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");

    if (!ValidateConfiguration())
    {
        return false;
    }

    if (!CreateSilKitConnection())
    {
        SDV_LOG_ERROR("Error createing SilKit connection, probably invalid JSON content");
        return false;
    }
    return true;
}

bool CCANSilKit::ValidateConfiguration()
{
    bool success = true;
    SDV_LOG_INFO("SilKit connecting to network: ", m_SilKitNetwork.c_str());
    SDV_LOG_INFO("SilKit registry URI: ", m_SilKitRegistryUri.c_str());
    m_SilKitIsSynchronousMode ? SDV_LOG_INFO("SilKit is running in synchronous mode.") : SDV_LOG_INFO("SilKit is running in asynchronous mode.");
    m_TimerSimulationStep ? SDV_LOG_WARNING("Run simulation with simulation timer service.") : SDV_LOG_WARNING("Run simulation with real time service.");

    if (m_SilKitJSONConfigContent.empty())
    {
        SDV_LOG_ERROR("Error reading config file content for SilKit, missing 'SilKitConfig'.");
        success = false;
    }
    if (m_SilKitParticipantName.empty())
    {
        SDV_LOG_ERROR("SilKit CAN participant is not found in configuration, missing 'SilKitParticipantName'.");
        success = false;
    }
    if (m_SilKitNetwork.empty())
    {
        SDV_LOG_ERROR("Error reading SilKit network name, missing 'CanSilKitNetwork'.");
        success = false;
    }
    else
    {
        SDV_LOG_INFO("SilKit connecting to network: ", m_SilKitNetwork.c_str());
    }
    if (m_SilKitRegistryUri.empty())
    {
        SDV_LOG_ERROR("Error reading SilKit registry URI, missing 'RegistryURI'.");
        success = false;
    }
    else
    {
        SDV_LOG_INFO("SilKit registry URI: ", m_SilKitRegistryUri.c_str());
    }

    if (!m_TimerSimulationStep)
    {
        SDV_LOG_WARNING("Run simulation with real timer.");
    }

    if (m_SilKitIsSynchronousMode)
    {
        SDV_LOG_INFO("SilKit is running in synchronous mode.");
    }
    else
    {
        SDV_LOG_INFO("SilKit is running in asynchronous mode.");
    }

    return success;
}

void CCANSilKit::OnShutdown()
{
    try
    {
        if (m_SilKitLifeCycleService)
        {
            SDV_LOG_INFO("Stopping SilKit Lifecycle Service...");
            m_SilKitLifeCycleService->Stop("Shutdown requested.");
        }

        if (m_SilKitCanController)
        {
            SDV_LOG_INFO("Stopping SilKit CAN Controller...");
            m_SilKitCanController->Stop();
        }

        if (m_SilKitParticipant)
        {
            SDV_LOG_INFO("Resetting SilKit Participant...");
            m_SilKitParticipant.reset();
        }
    }
    catch (const SilKit::SilKitError& e)
    {
        SDV_LOG_ERROR("SilKit exception occurred during shutdown: ", e.what());
    }
    catch (const std::exception& e)
    {
        SDV_LOG_ERROR("Unknown exception occurred during shutdown: ", e.what());
    }

    m_SilKitLifeCycleService = nullptr;
    m_SilKitCanController = nullptr;
    m_SilKitParticipant = nullptr;

    std::lock_guard<std::mutex> lock(m_QueueMutex);
    while (!m_MessageQueue.empty())
    {
        m_MessageQueue.pop();
    }
}

void CCANSilKit::RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    if (GetObjectState() != sdv::EObjectState::configuring) 
        return;

    if (!pReceiver)
    {
        SDV_LOG_ERROR("No CAN receiver available.");
        SetObjectIntoConfigErrorState();
        return;
    }

    SDV_LOG_INFO("Registering VAPI CAN communication receiver...");

    std::unique_lock<std::mutex> lock(m_ReceiversMtx);
    if (m_SetReceivers.find(pReceiver) == m_SetReceivers.end())
    {
        m_SetReceivers.insert(pReceiver);
        SDV_LOG_INFO("Receiver registered successfully.");
    }
    else
    {
        SDV_LOG_INFO("Receiver is already registered.");
    }
}

void CCANSilKit::UnregisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    // NOTE: Normally the remove function should be called in the configuration mode. Since it doesn't give
    // feedback and the associated caller might delete any receiving function, allow the removal to take place even
    // when running.

    if (!pReceiver)
    {
        return;
    }

    SDV_LOG_INFO("Unregistering VAPI CAN communication receiver...");

    std::unique_lock<std::mutex> lock(m_ReceiversMtx);
    m_SetReceivers.erase(pReceiver);
}

sdv::sequence<sdv::u8string> CCANSilKit::GetInterfaces() const
{
    sdv::sequence<sdv::u8string> seqIfcNames;
    if (GetObjectState() != sdv::EObjectState::running) return seqIfcNames;

    seqIfcNames.push_back(m_SilKitParticipantName);

    return seqIfcNames;
}

std::shared_ptr<SilKit::Config::IParticipantConfiguration> CCANSilKit::GetSilKitConfig(const std::string& ssSilKitJSONConfigContent)
{
    // Get the SilKit configuration from the config file. 
    //auto silKitParticipantCconfig = std::move(SilKit::Config::ParticipantConfigurationFromString(ssSilKitJSONConfigContent));
    auto silKitParticipantCconfig = SilKit::Config::ParticipantConfigurationFromString(ssSilKitJSONConfigContent);
    if (silKitParticipantCconfig == nullptr)
    {
        SDV_LOG_ERROR("Error parsing the SilKit config content: ", ssSilKitJSONConfigContent.c_str());
        SetObjectIntoConfigErrorState();
        return nullptr;
    }

    return silKitParticipantCconfig;
}

std::unique_ptr<SilKit::IParticipant> CCANSilKit::CreateParticipantFromJSONConfig(const std::string& ssSilKitJSONConfigContent, const std::string& ssSilKitRegistryUri)
{
    auto silKitParticipantConfig = GetSilKitConfig(ssSilKitJSONConfigContent);
    if (silKitParticipantConfig == nullptr)
    {
        SDV_LOG_ERROR("The SilKit configuaration file could not be opened.");
        SetObjectIntoConfigErrorState();
        return nullptr;
    }

    // TODO: get participant name from the configuration 
    // participant name must be unique in the SilKit network setup and must exit before running thee test
    //  1 single participant:  use complete name from the config
    //  more than one channel: do we need more that on participants or can one participant include multiple channels?
    //  in case every single channel requires another participant name: add channel to the name ['config_name' + 'channel']

    SDV_LOG_INFO("Creating SilKit participant: ", m_SilKitParticipantName.c_str());

    return SilKit::CreateParticipant(silKitParticipantConfig, m_SilKitParticipantName, ssSilKitRegistryUri);
}

SilKit::Services::Can::ICanController* CCANSilKit::CreateController(const std::string& ssSilKitNetwork)
{
    /* Create the SilKit CAN controller. */
    SilKit::Services::Can::ICanController* silKitCanController = m_SilKitParticipant->CreateCanController(m_SilKitParticipantName, ssSilKitNetwork);
    if (silKitCanController == nullptr)
    {
        SDV_LOG_ERROR("SilKit CAN controller is not available.");
        return nullptr;
    }

    /* Register the SilKit transmit status handler (use lambda function to map to member variable). */
    silKitCanController->AddFrameTransmitHandler(
        [this](SilKit::Services::Can::ICanController* /*ctrl*/, const SilKit::Services::Can::CanFrameTransmitEvent& rs_transmit_acknowledge)
        {
            SilKitTransmitAcknowledgeHandler(rs_transmit_acknowledge);
        },
        static_cast<SilKit::Services::Can::CanTransmitStatusMask>(SilKit::Services::Can::CanTransmitStatus::Transmitted) |
            static_cast<SilKit::Services::Can::CanTransmitStatusMask>(SilKit::Services::Can::CanTransmitStatus::Canceled) |
            static_cast<SilKit::Services::Can::CanTransmitStatusMask>(SilKit::Services::Can::CanTransmitStatus::TransmitQueueFull));

    /* Register the SilKit receive handler (use lambda function to map to member variable). */
    silKitCanController->AddFrameHandler(
        [this](SilKit::Services::Can::ICanController* /*ctrl*/, const SilKit::Services::Can::CanFrameEvent& rs_frame_event)
        {
            SilKitReceiveMessageHandler(rs_frame_event.frame);
        },
        static_cast<SilKit::Services::DirectionMask>(SilKit::Services::TransmitDirection::RX)
            /*| static_cast<SilKit::Services::DirectionMask>(SilKit::Services::TransmitDirection::TX)*/);

    return silKitCanController;
}

SilKit::Services::Orchestration::ILifecycleService* CCANSilKit::CreateSilKitLifecycleService()
{
    SilKit::Services::Orchestration::OperationMode silkit_operationMode = (
        m_SilKitIsSynchronousMode ? SilKit::Services::Orchestration::OperationMode::Coordinated
        : SilKit::Services::Orchestration::OperationMode::Autonomous
        );

    return m_SilKitParticipant->CreateLifecycleService({ silkit_operationMode });
}

bool CCANSilKit::SetHandlerFunctions(SilKit::Services::Orchestration::ILifecycleService* silKitLifeCycleService)
{
    try
    {
        // Set a SilKit Stop Handler
        silKitLifeCycleService->SetStopHandler(
            [this]()
            {
                SetObjectIntoRuntimeErrorState();
                SDV_LOG_INFO("SilKit StopHandlerhandler called");
            });

        // Set a Shutdown Handler
        silKitLifeCycleService->SetShutdownHandler([this]()
            {
                SetObjectIntoRuntimeErrorState();
                SDV_LOG_INFO("SilKit Shutdown handler called");
            });

        // Set a Shutdown Handler
        silKitLifeCycleService->SetAbortHandler([this](auto /*participantState*/)
            {
                SetObjectIntoRuntimeErrorState();
                SDV_LOG_INFO("SilKit Abort handler called");
            });

        silKitLifeCycleService->SetCommunicationReadyHandler([this]()
            {
                m_SilKitCanController->SetBaudRate(10'000, 1'000'000, 2'000'000);
                m_SilKitCanController->Start();
            });
    }

    catch (const SilKit::SilKitError& e)
    {
        SDV_LOG_ERROR("SilKit exception occurred when setting the silkit handlers.", e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        SDV_LOG_ERROR("Unknown std exception occurred when setting the silkit handlers.", e.what());
        return false;
    }

    return true;
}

bool CCANSilKit::CreateSilKitConnection()
{
    try
    {
        m_SilKitParticipant = CreateParticipantFromJSONConfig(m_SilKitJSONConfigContent, m_SilKitRegistryUri);
        if (m_SilKitParticipant == nullptr)
        {
            SDV_LOG_ERROR("SilKit COM adapter is not available.");
            return false;
        } 

        m_SilKitCanController = CreateController(m_SilKitNetwork);    
        if (m_SilKitCanController == nullptr)
        {
            SDV_LOG_ERROR("SilKit CAN controller is not available.");
            return false;
        }

        m_SilKitLifeCycleService = CreateSilKitLifecycleService();
        if (!SetHandlerFunctions(m_SilKitLifeCycleService))
        {
            SDV_LOG_ERROR("SilKit handler functions could not be set.");
            return false;
        }

        if (m_SilKitIsSynchronousMode)
        {
            SetupTimeSyncService();
        }

        auto finalStateFuture = m_SilKitLifeCycleService->StartLifecycle();
        if (!finalStateFuture.valid())
        {
            SDV_LOG_ERROR("Participant State = SilKit::Services::Orchestration::ParticipantState::Invalid");
            return false;
        }
    }
    catch (const SilKit::SilKitError& e)
    {
        SDV_LOG_ERROR("SilKit exception occurred in CreateSilKitConnection().", e.what());
        return false;
    }
    catch (const std::exception& e)
    {
        SDV_LOG_ERROR("Unknown std exception in CreateSilKitConnection():", e.what());
        return false;
    }
    /* Adapter is configured now, return true */
    return true;
}

void CCANSilKit::SilKitReceiveMessageHandler(const SilKit::Services::Can::CanFrame& rsSilKitCanFrame)
{
    if (GetObjectState() != sdv::EObjectState::running) 
        return;

    if (rsSilKitCanFrame.dlc > m_maxCanDataLength)
    {
        SDV_LOG_WARNING("Invalid data length.");        
        return;
    }

    sdv::can::SMessage sSDVCanMessage{};
    sSDVCanMessage.uiID = rsSilKitCanFrame.canId;
    for (size_t nDataIndex = 0; nDataIndex < static_cast<size_t>(rsSilKitCanFrame.dlc); nDataIndex++)
    {
        sSDVCanMessage.seqData.push_back(rsSilKitCanFrame.dataField[nDataIndex]);
    }

    // Broadcast the message to the receivers
    std::unique_lock<std::mutex> lockReceivers(m_ReceiversMtx);
    for (sdv::can::IReceive* pReceiver : m_SetReceivers)
    {
        pReceiver->Receive(sSDVCanMessage, 0);
    }
}

void CCANSilKit::SilKitTransmitAcknowledgeHandler(const SilKit::Services::Can::CanFrameTransmitEvent& rsSilKitTransmitAcknowledge)
{
  /* Get the Acknowledge Sync structure. */
  SAcknowledgeSync* acknowledgeSync = reinterpret_cast<SAcknowledgeSync*>(rsSilKitTransmitAcknowledge.userContext);
  if (acknowledgeSync == nullptr) 
      return;

  /* Synchronize threads. */
  std::unique_lock<std::mutex> lock(acknowledgeSync->mtx);

  /* Copy the content of the transmit status to the sync structure. */
  *static_cast<SilKit::Services::Can::CanFrameTransmitEvent *>(acknowledgeSync) = rsSilKitTransmitAcknowledge;

  /* Trigger the condition variable. */
  acknowledgeSync->bProcessed = true;
  lock.unlock();
  acknowledgeSync->cv.notify_all();
}

void CCANSilKit::Send(/*in*/ const sdv::can::SMessage& sSDVCanMessage, /*in*/ uint32_t)
{
    if (GetObjectState() != sdv::EObjectState::running)
        return;

    if(sSDVCanMessage.bCanFd)
    {
        SDV_LOG_ERROR("CAN-FD not supported.");        
        return;     // CAN-FD not supported
        // s_SilKit_message.flags |= static_cast< SilKit::Services::Can::CanFrameFlagMask>( SilKit::Services::Can::CanFrameFlag::Fdf) // FD Format Indicator
        // | static_cast< SilKit::Services::Can::CanFrameFlagMask>( SilKit::Services::Can::CanFrameFlag::Brs); // Bit Rate Switch (for FD Format only)
    }

    if (sSDVCanMessage.seqData.size() > m_maxCanDataLength)     // Invalid message length.
    {
        SDV_LOG_ERROR("Invalid message length:", sSDVCanMessage.seqData.size());
        return;
    }

    // Send the message in async mode
    if (!m_SilKitIsSynchronousMode)
    {
        SilKit::Services::Can::CanFrame silKitMessage{};
        silKitMessage.canId = sSDVCanMessage.uiID;

        std::vector<uint8_t> vecData;
        for (uint8_t index = 0; index < sSDVCanMessage.seqData.size(); index++)
        {
            vecData.emplace_back(sSDVCanMessage.seqData[index]);
        }
        silKitMessage.dataField = vecData;
        m_SilKitCanController->SendFrame(silKitMessage);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_MessageQueue.push(sSDVCanMessage);
    }
}

void CCANSilKit::SetupTimeSyncService()
{
    auto silKitTimeSyncService = m_SilKitLifeCycleService->CreateTimeSyncService();
        
    silKitTimeSyncService->SetSimulationStepHandler([this](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds duration)
    {
        if (m_TimerSimulationStep)
        {
            m_TimerSimulationStep->SimulationStep(duration.count() / 1000);
        }

        try
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            while (!m_MessageQueue.empty())
            {
                const sdv::can::SMessage& frontMsg = m_MessageQueue.front();        

                SilKit::Services::Can::CanFrame silKitMessage{};
                silKitMessage.canId = frontMsg.uiID;
                // Clamp or check to avoid narrowing conversion warning
                if (frontMsg.seqData.size() > std::numeric_limits<uint16_t>::max()) {
                    SDV_LOG_WARNING("CAN data length exceeds uint16_t max, truncating.");
                    silKitMessage.dlc = static_cast<uint16_t>(std::numeric_limits<uint16_t>::max());
                } else {
                    silKitMessage.dlc = static_cast<uint16_t>(frontMsg.seqData.size());
                }
                std::vector<uint8_t> vecData;
                for (uint8_t index = 0; index < frontMsg.seqData.size(); index++)
                {
                    vecData.emplace_back(frontMsg.seqData[index]);
                }
                silKitMessage.dataField = vecData;

                    m_SilKitCanController->SendFrame(silKitMessage);
                    m_MessageQueue.pop();
                }
            }
            catch (const SilKit::SilKitError& e)
            {
                SDV_LOG_ERROR("SilKit exception occurred when setting up TimeSyncService.", e.what());
            }
            catch (const std::exception& e)
            {
                SDV_LOG_ERROR("Unknown std exception when setting up TimeSyncService.", e.what());
            }
        }, std::chrono::milliseconds(1));           
}