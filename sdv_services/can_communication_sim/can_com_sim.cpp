#include "can_com_sim.h"
#include <support/toml.h>
#include "../../global/ascformat/ascreader.cpp"
#include "../../global/ascformat/ascwriter.cpp"

CCANSimulation::CCANSimulation()
{}

CCANSimulation::~CCANSimulation()
{}

void CCANSimulation::Initialize(const sdv::u8string& rssObjectConfig)
{
    try
    {
        sdv::toml::CTOMLParser config(rssObjectConfig.c_str());
        auto nodeSource = config.GetDirect("Source");
        if (nodeSource.GetType() == sdv::toml::ENodeType::node_string)
            m_pathSource = nodeSource.GetValue();
        auto nodeTarget = config.GetDirect("Target");
        if (nodeTarget.GetType() == sdv::toml::ENodeType::node_string)
            m_pathTarget = nodeTarget.GetValue();
        if (m_pathSource.empty() && m_pathTarget.empty())
        {
            SDV_LOG(sdv::core::ELogSeverity::error,
                "At least the source or the target ASC files must be specified.");
            m_eStatus = sdv::EObjectStatus::initialization_failure;
        }
        else if (m_pathSource == m_pathTarget)
        {
            SDV_LOG(sdv::core::ELogSeverity::error,
                "Source and target ASC files '" + m_pathSource.generic_u8string() + "' cannot be the same.");
            m_eStatus = sdv::EObjectStatus::initialization_failure;
        }
        else if (std::filesystem::exists(m_pathTarget))
        {
            SDV_LOG(sdv::core::ELogSeverity::warning,
                "Target ASC file '" + m_pathSource.generic_u8string() + "' will be overwritten.");
        }
        else if (m_pathSource.empty() && m_pathTarget.empty())
        {
            SDV_LOG(sdv::core::ELogSeverity::error, "No ASC file configured for reading or writing.");
            m_eStatus = sdv::EObjectStatus::initialization_failure;
        }
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        SDV_LOG(sdv::core::ELogSeverity::error, "Configuration could not be read: ", e.what());
        m_eStatus = sdv::EObjectStatus::initialization_failure;
    }
    catch (const std::runtime_error& e)
    {
        SDV_LOG(sdv::core::ELogSeverity::error, "Configuration could not be read: ", e.what());
        m_eStatus = sdv::EObjectStatus::initialization_failure;
    }
    if (m_eStatus == sdv::EObjectStatus::initialization_failure) return;

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
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Update the status
    m_eStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CCANSimulation::GetStatus() const
{
    return m_eStatus;
}

void CCANSimulation::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eStatus == sdv::EObjectStatus::running || m_eStatus == sdv::EObjectStatus::initialized)
        {
            m_eStatus = sdv::EObjectStatus::configuring;

            // Stop playback
            m_reader.StopPlayback();
        }
        break;
    case sdv::EOperationMode::running:
        if (m_eStatus == sdv::EObjectStatus::configuring || m_eStatus == sdv::EObjectStatus::initialized)
        {
            m_eStatus = sdv::EObjectStatus::running;

            // Start playback
            m_reader.StartPlayback([&](const asc::SCanMessage& rsMsg) { PlaybackFunc(rsMsg); });
        }
        break;
    default:
        break;
    }
}

void CCANSimulation::Shutdown()
{
    m_eStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Stop playback
    m_reader.StopPlayback();

    // Write the recording
    if (m_writer.HasSamples() && !m_pathTarget.empty())
    {
        if (!m_writer.Write(m_pathTarget))
            SDV_LOG(sdv::core::ELogSeverity::error,
                "Failed to write ASC file '" + m_pathTarget.generic_u8string() + "' with CAN recording.");
    }

    // Update the status
    m_eStatus = sdv::EObjectStatus::destruction_pending;
}

void CCANSimulation::RegisterReceiver(/*in*/ sdv::can::IReceive* pReceiver)
{
    if (m_eStatus != sdv::EObjectStatus::configuring) return;
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
    if (m_eStatus != sdv::EObjectStatus::running) return;

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
    if (m_eStatus != sdv::EObjectStatus::running) return seqIfcNames;
    std::unique_lock<std::mutex> lock(m_mtxInterfaces);
    for (const auto& rprInterface : m_vecInterfaces)
        seqIfcNames.push_back(rprInterface.second);
    return seqIfcNames;
}

void CCANSimulation::PlaybackFunc(const asc::SCanMessage& rsMsg)
{
    if (m_eStatus != sdv::EObjectStatus::running) return;

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
