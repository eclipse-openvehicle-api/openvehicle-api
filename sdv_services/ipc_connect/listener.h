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

#ifndef LISTENER_H
#define LISTENER_H

#include <support/component_impl.h>
#include <interfaces/com.h>
#include <interfaces/ipc.h>

// Forward declarations
class CListener;

/**
 * @brief Channel broker to request new channels. This object is exposed to the client.
 */
class CChannelBroker : public sdv::IInterfaceAccess, public sdv::com::IRequestChannel
{
public:
    /**
     * @brief Constructor
     * @param[in] rListener Reference to the listener to forward the calls to.
     */
    CChannelBroker(CListener& rListener);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::com::IRequestChannel)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Request a channel. Overload of sdv::com::IRequestChannel::RequestChannel
     * @details This function creates a new endpoint and returns access to the repository through the new channel.
     * @param[in] ssConfig Configuration; currently not used.
     * @return The channel string needed to initialize the channel.
     */
    virtual sdv::u8string RequestChannel(/*in*/ const sdv::u8string& ssConfig) override;

private:
    CListener& m_rListener;         ///< Reference to the listener to forward the calls to.
};

/**
 * @brief Listener object
 * @details the lister is instantiated using the following parameter information
 * @code
 * # Provider to use for listening
 * [Provider]
 * Name = ""
 * 
 * # Additional channel information for the listener (needed for unique listener identification)
 * [IpcChannel]
 * xyz = ""
 * @endcode
 * 
 * For example for shared memory:
 * @code
 * [Provider]
 * Name = "DefaultSharedMemory"
 * [IpcChannel]
 * Name = "LISTENER_1234"
 * Size = 10240
 * @endcode
 */
class CListener : public sdv::CSdvObject
{
public:
    /**
     * @brief Default constructor
     */
    CListener();

    // Object declaration
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("ListenerConnectService")
    DECLARE_OBJECT_DEPENDENCIES("CommunicationControl")

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_GROUP("Provider")
        SDV_PARAM_ENTRY(m_ssProvider, "Name", "", "", "Provider name to create a listener for.")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

    /**
     * @brief Get the provider name used by this listener.
     * @return Reference to the provider name.
     */
    const sdv::u8string& GetProviderName();

private:
    sdv::u8string               m_ssProvider;               ///< Name of the provider to use for the listening.
    sdv::TObjectPtr             m_ptrConnection;            ///< The connection object.
    CChannelBroker              m_broker;                   ///< Channel broker, used to request new channels
    sdv::com::TConnectionID     m_tConnection = {};         ///< Channel connection ID.
};

DEFINE_SDV_OBJECT(CListener)

#endif // ! defined LISTENER_H