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
class CCANSimulation : public sdv::CSdvObject, public sdv::can::IRegisterReceiver, public sdv::can::ISend, sdv::can::IInformation
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
        SDV_INTERFACE_ENTRY(sdv::can::IRegisterReceiver)
        SDV_INTERFACE_ENTRY(sdv::can::ISend)
        SDV_INTERFACE_ENTRY(sdv::can::IInformation)
    END_SDV_INTERFACE_MAP()

    // Declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_bus)
    DECLARE_OBJECT_CLASS_NAME("CAN_Com_Sim")
    DECLARE_DEFAULT_OBJECT_NAME("CAN_Communication_Object")
    DECLARE_OBJECT_SINGLETON()

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_PATH_ENTRY(m_pathSource, "Source", "", "Path to the source ASC file.")
        SDV_PARAM_PATH_ENTRY(m_pathTarget, "Target", "", "Path to the target ASC file.")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Change to configuration mode event. After this a call to this function locked parameters can be changed again.
     * Overload of sdv::CSdvObject::OnChangeToConfigMode.
     */
    virtual void OnChangeToConfigMode() override;

    /**
     * @brief Change to running mode event. Parameters were locked before the call to this event. Overload of
     * sdv::CSdvObject::OnChangeToRunningMode.
     * @return Returns 'true' when the configuration is valid and the running instances could be started. Otherwise returns
     * 'false'.
     */
    virtual bool OnChangeToRunningMode() override;

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
