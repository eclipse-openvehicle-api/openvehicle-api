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
    DECLARE_OBJECT_CLASS_NAME("ConnectionListenerService")

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENABLE_LOCKING()
        SDV_PARAM_GROUP("Listener")
        SDV_PARAM_ENTRY(m_ssType, "Type", "Local", "", "The type of listener \"Local\" or \"Remote\".")
        SDV_PARAM_ENTRY(m_uiInstanceID, "Instance", 0, "", "The instance ID to listen for.")
        SDV_PARAM_ENTRY(m_ssInterface, "Interface", "", "", "Interface identification.")
        SDV_PARAM_ENTRY(m_uiPort, "Port", 0, "", "Port number for connection.")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @details The object configuration contains the information needed to start the listener. The following configuration is
     * available for the local listener:
     * @code
     * [Listener]
     * Type = "Local"
     * Instance = 1000    # Normally not used; system instance ID is used automatically.
     * @endcode
     * And the following is available for a remote listener:
     * @code
     * [Listener]
     * Type = "Remote"
     * Interface = "127.0.0.1"
     * Port = 2000
     * @endcode
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

    /**
     * @brief When set, the listener is configured to be a local listener. Otherwise the listerner is configured as remote listener.
     * @return Boolean set when local lostener.
     */
    bool IsLocalListener() const;

private:
    sdv::u8string               m_ssType;                   ///< Listener type: "Local" or "Remote"
    uint32_t                    m_uiInstanceID = 0;         ///< Instance ID to listen for.
    std::string                 m_ssInterface;              ///< Interface string for remote listener.
    uint32_t                    m_uiPort = 0;               ///< Port for remote listener.
    sdv::TObjectPtr             m_ptrConnection;            ///< The connection object.
    CChannelBroker              m_broker;                   ///< Channel broker, used to request new channels
    bool                        m_bLocalListener = true;    ///< When set, the listener is a local listener; otherwise a remote listener.
    sdv::com::TConnectionID     m_tConnection = {};         ///< Channel connection ID.
};

DEFINE_SDV_OBJECT(CListener)

#endif // ! defined LISTENER_H