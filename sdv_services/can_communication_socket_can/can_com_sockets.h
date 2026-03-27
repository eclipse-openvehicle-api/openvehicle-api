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

#ifndef CAN_COM_SOCKET_H
#define CAN_COM_SOCKET_H

#include <iostream>
#include <cstring>
#include <deque>
#include <set>
#include <thread>
#include <mutex>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>

#include <support/toml.h>
#include <support/component_impl.h>
#include <interfaces/can.h>

#ifndef __linux__
// cppcheck-suppress preprocessorErrorDirective
#error This code builds only on LINUX
#endif

/**
* @brief Component to establish Socket CAN communication between VAPI and external application
*/
class CCANSockets : public sdv::CSdvObject, public sdv::can::IRegisterReceiver,
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

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_bus)
    DECLARE_OBJECT_CLASS_NAME("CAN_Com_Sockets")
    DECLARE_DEFAULT_OBJECT_NAME("CAN_Communication_Object")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::CSdvObject::OnInitialize.
     * The configuration contains either one interface name a list of interface names.
     * The Send() method must use the index of this list to determine the interface
     * In case of a single interface name the index is 0.
     * canSockets = "vcan0"
     * or
     * canSockets = ["vcan1", "vcan8", "vcan9", "vcan2"]
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
     * @param[in] sMsg Message to be sent.
     * @param[in] uiConfigIndex Interface index to use for sending.
     * Must match with the configuration list. In case configuration contains a single element the index is 0.
     * The message cannot be sent to all interfaces automatically
     */
    virtual void Send(/*in*/ const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiConfigIndex) override;

    /**
     * @brief Get a list of interface names. Overload of sdv::can::IInformation::GetInterfaces.
     * @return Sequence containing the names of the interfaces.
     */
    virtual sdv::sequence<sdv::u8string> GetInterfaces() const override;

private:
    /**
     * @brief Thread function to read data from all bound interfaces.
     */
    void ReceiveThreadFunc();

    /**
     * @brief Function to setup the sockets in the configuration
     * @param[in] vecConfigInterfaces List of interface names which should be connected to a socket
     */
    bool SetupCANSockets(const std::deque<std::string>& vecConfigInterfaces);

    /**
     * @brief Function to create and setup sockets, collected in m_vecSockets
     * @param[in] vecConfigInterfaces List of interface names which should be connected to a socket
     * @param[in] availableInterfaces List of available interface names
     */
    void CreateAndBindSockets(const std::deque<std::string>& vecConfigInterfaces,
        const std::set<std::string>& availableInterfaces);

    /**
     * @brief Write log information about the configured can sockets
     */
    void LogConfigurations();

    /**
     * @brief Write log information about the existing can hardware
     */
    void LogAllCanInterfaceNames();

    /**
    * @brief Socket definition structure
    */
    struct SSocketDefinition
    {
        int networkInterface; ///< network interface, must be > 0
        int32_t localSocket;  ///< local socket id; -1 represents an invalid socket element
        std::string name;     ///< interface name, can be empty in case of an invalid socket element
    };

    std::thread                     m_threadReceive;    ///< Receive thread.
    mutable std::mutex              m_mtxReceivers;     ///< Protect the receiver set.
    std::set<sdv::can::IReceive*>   m_setReceivers;     ///< Set with receiver interfaces.
    mutable std::mutex              m_mtxSockets;       ///< Protect the socket list.
    std::deque<SSocketDefinition>   m_vecSockets;       ///< Socket list
};

DEFINE_SDV_OBJECT(CCANSockets)

#endif // ! defined CAN_COM_SOCKET_H
