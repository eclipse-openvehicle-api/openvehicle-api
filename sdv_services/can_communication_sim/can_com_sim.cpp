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
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#include "can_com_sim.h"
#include <support/toml.h>
#include "../../global/ascformat/ascreader.cpp"
#include "../../global/ascformat/ascwriter.cpp"

CCANSimulation::CCANSimulation()
{}

CCANSimulation::~CCANSimulation()
{}

bool CCANSimulation::OnInitialize()
{
    if (m_pathSource.empty() && m_pathTarget.empty())
    {
        SDV_LOG(sdv::core::ELogSeverity::error,
            "At least the source or the target ASC files must be specified.");
        return false;
    }
    else if (m_pathSource == m_pathTarget)
    {
        SDV_LOG(sdv::core::ELogSeverity::error,
            "Source and target ASC files '" + m_pathSource.generic_u8string() + "' cannot be the same.");
        return false;
    }
    else if (std::filesystem::exists(m_pathTarget))
    {
        SDV_LOG(sdv::core::ELogSeverity::warning,
            "Target ASC file '" + m_pathSource.generic_u8string() + "' will be overwritten.");
    }
    else if (m_pathSource.empty() && m_pathTarget.empty())
    {
        SDV_LOG(sdv::core::ELogSeverity::error, "No ASC file configured for reading or writing.");
        return false;
    }

    // Initialize the ASC writer
    if (!m_pathTarget.empty())
        SDV_LOG(sdv::core::ELogSeverity::info,
            "CAN simulator uses ASC file '" + m_pathTarget.generic_u8string() + "' to record CAN data.");
    m_writer.StartTimer();

    // Initialize the ASC reader
    if (!m_pathSource.empty())
        SDV_LOG(sdv::core::ELogSeverity::info,
            "CAN simulator uses ASC file '" + m_pathSource.generic_u8string() + "' to playback CAN data.");
    if (!m_pathSource.empty() && !m_reader.Read(m_pathSource))
    {
        SDV_LOG(sdv::core::ELogSeverity::error,
            "Failed to read ASC file '" + m_pathSource.generic_u8string() + "' for CAN playback.");
        return false;
    }
    if (!m_pathSource.empty() && !m_reader.GetMessageCount())
    {
        SDV_LOG(sdv::core::ELogSeverity::error,
            "No messages in ASC file '" + m_pathSource.generic_u8string() + "' found. File must contain 'Begin TriggerBlock' and 'End TriggerBlock' line.");
        return false;
    }
    if (!m_pathSource.empty())
        SDV_LOG(sdv::core::ELogSeverity::info,
            "CAN simulator ASC file '" + m_pathSource.generic_u8string() + "' contains ", m_reader.GetMessageCount(), " messages.");

    return true;
}

void CCANSimulation::OnChangeToConfigMode()
{
    // Stop playback
    m_reader.StopPlayback();
}

bool CCANSimulation::OnChangeToRunningMode()
{
    // Start playback
    m_reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { PlaybackFunc(rsMsg); });
    return true;
}

void CCANSimulation::OnShutdown()
{
    // Stop playback
    m_reader.StopPlayback();

    // Write the recording
    if (m_writer.HasSamples() && !m_pathTarget.empty())
    {
        if (!m_writer.Write(m_pathTarget))
            SDV_LOG(sdv::core::ELogSeverity::error,
                "Failed to write ASC file '" + m_pathTarget.generic_u8string() + "' with CAN recording.");
    }
}

void CCANSimulation::RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    if (GetObjectState() != sdv::EObjectState::configuring) return;
    if (!pReceiver) return;

    std::unique_lock<std::mutex> lock(m_mtxReceivers);
    m_setReceivers.insert(pReceiver);
}

void CCANSimulation::UnregisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    // NOTE: Normally the remove function should be called in the configuration mode. Since it doesn't give
    // feedback and the associated caller might delete any receiving function, allow the removal to take place even
    // when running.

    if (!pReceiver) return;

    std::unique_lock<std::mutex> lock(m_mtxReceivers);
    m_setReceivers.erase(pReceiver);
}

void CCANSimulation::Send(/*in*/ const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiIfcIndex)
{
    if (GetObjectState() != sdv::EObjectState::running) return;

    asc::SCanMessage sAscCan{};
    sAscCan.uiChannel = uiIfcIndex + 1;
    sAscCan.uiId = sMsg.uiID;
    sAscCan.bExtended = sMsg.bExtended;
    sAscCan.bCanFd = sMsg.bCanFd;
    sAscCan.eDirection = asc::SCanMessage::EDirection::tx;
    sAscCan.uiLength = static_cast<uint32_t>(sMsg.seqData.length());
    std::copy_n(sMsg.seqData.begin(), sMsg.seqData.length(), std::begin(sAscCan.rguiData));
    m_writer.AddSample(sAscCan);
}

sdv::sequence<sdv::u8string> CCANSimulation::GetInterfaces() const
{
    sdv::sequence<sdv::u8string> seqIfcNames;
    if (GetObjectState() != sdv::EObjectState::running) return seqIfcNames;
    std::unique_lock<std::mutex> lock(m_mtxInterfaces);
    for (const auto& rprInterface : m_vecInterfaces)
        seqIfcNames.push_back(rprInterface.second);
    return seqIfcNames;
}

void CCANSimulation::PlaybackFunc(const asc::SCanMessage& rsMsg)
{
    if (GetObjectState() != sdv::EObjectState::running) return;

    // Create sdv CAN message
    sdv::can::SMessage sSdvCan{};
    sSdvCan.uiID = rsMsg.uiId;
    sSdvCan.bExtended = rsMsg.bExtended;
    sSdvCan.bCanFd = rsMsg.bCanFd;
    sSdvCan.seqData = sdv::sequence<uint8_t>(std::begin(rsMsg.rguiData), std::begin(rsMsg.rguiData) + rsMsg.uiLength);

    // Distribute the CAN message to all receivers
    std::unique_lock<std::mutex> lock(m_mtxReceivers);
    for (sdv::can::IReceive* pReceiver : m_setReceivers)
        pReceiver->Receive(sSdvCan, rsMsg.uiChannel - 1);
}
