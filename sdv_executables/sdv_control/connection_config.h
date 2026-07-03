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

#ifndef CONNECTION_CONFIG_H
#define CONNECTION_CONFIG_H

#include "context.h"
#include <support/sequence.h>
#include <support/string.h>

/**
 * @brief Help for configuring listeners.
 * @param[in] rsContext Reference to the context.
 */
void ListenerHelp(const SContext& rsContext);

/**
 * @brief Help for configuring connections.
 * @param[in] rsContext Reference to the context.
 */
void ConnectionHelp(const SContext& rsContext);

/**
 * @brief Configure the listener.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int ConfigureListener(const SContext& rsContext);

/**
 * @brief Add a listener to the system settings.
 * @param[in] rssListener Reference to the string containing the listener configuration name.
 * @param[in] rssConfig Reference to the string containing the listener configuration.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int AddListener(const std::string& rssListener, const std::string& rssConfig, const SContext& rsContext);

/**
 * @brief Remove a listener from the system settings.
 * @param[in] rssListener Reference to the string containing the listener configuration name.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int RemoveListener(const std::string& rssListener, const SContext& rsContext);

/**
 * @brief Show a listener configuration.
 * @param[in] rssListener Reference to the string containing the listener configuration name.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int ShowListenerConfig(const std::string& rssListener, const SContext& rsContext);

/**
 * @brief Configure the connection.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int ConfigureConnection(const SContext& rsContext);

/**
 * @brief Add a connection to the system settings.
 * @param[in] rssConnection Reference to the string containing the connection configuration name.
 * @param[in] rssConfig Reference to the string containing the connection configuration.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int AddConnection(const std::string& rssConnection, const std::string& rssConfig, const SContext& rsContext);

/**
 * @brief Remove a connection from the system settings.
 * @param[in] rssConnection Reference to the string containing the connection configuration name.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int RemoveConnection(const std::string& rssConnection, const SContext& rsContext);

/**
 * @brief Show a connection configuration.
 * @param[in] rssConnection Reference to the string containing the connection configuration name.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int ShowConnectionConfig(const std::string& rssConnection, const SContext& rsContext);


#endif // !defined CONNECTION_CONFIG_H
