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

#include "connection_config.h"
#include "../../global/cmdlnparser/cmdlnparser.h"
#include <support/toml.h>
#include <interfaces/app.h>
#include "../error_msg.h"
#include <charconv>

void ListenerHelp(const SContext& rsContext)
{
    // First argument should be "LISTENER"
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing listener command..." << std::endl;
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "LISTENER"))
    {
        CCommandLine::PrintHelpText(std::cout,
            "Usage: sdv_control LISTENER ADD <name> <provider> <parameters...> [options...]\n"
            "       sdv_control LISTENER REMOVE <name> [options...]\n"
            "       sdv_control LISTENER SHOW <name> [options...]\n\n"
            "Configure the system to add (or update) or remove a listener. The command doesn't check for valid parameters. A "
            "server restart is required for the new listener settings to take effect.\n\n"
            "A list of actively configured listeners can be requested by executing \"sdv_control LIST LISTENERS\"\n\n"
            "Note: if not explicitly added, a default listener will added to the list for the communication between isolated "
            "applications and the core system.\n\n");
        return;
    }
    if (!rsContext.bSilent)
        std::cerr << "ERROR: invalid listener command..." << std::endl;
}

void ConnectionHelp(const SContext& rsContext)
{
    // First argument should be "CONNECTION"
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing connection command..." << std::endl;
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "CONNECTION"))
    {
        CCommandLine::PrintHelpText(std::cout,
            "Usage: sdv_control CONNECTION ADD <name> <provider> <parameters...> [options...]\n"
            "       sdv_control CONNECTION REMOVE <name> [options...]\n"
            "       sdv_control CONNECTION SHOW <name> [options...]\n\n"
            "Configure the system to add (or update) or remove a connection. The command doesn't check for valid parameters. A "
            "server restart is required for the new connection settings to take effect.\n\n"
            "A list of actively configured connections can be requested by executing \"sdv_control LIST CONNECTIONS\"\n\n"
            "Note: if not explicitly added, a default connetion will be added to the list for the communication between external "
            "applications and the core system.\n\n"
            "Options:\n"
            " --insert_before<string>   With ADD, Insert the connection before an existing connection with the supplied name.\n\n");
        return;
    }
    if (!rsContext.bSilent)
        std::cerr << "ERROR: invalid connection command..." << std::endl;
}

int ConfigureListener(const SContext& rsContext)
{
    // First argument should be "CONNECTION"
    if (rsContext.seqCmdLine.size() < 2)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing listener command... " << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "LISTENER"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command: " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Which list command is requested
    enum class ECommand { unknown, add, remove, show } eCommand = ECommand::unknown;
    if (iequals(rsContext.seqCmdLine[1], "ADD")) eCommand = ECommand::add;
    else if (iequals(rsContext.seqCmdLine[1], "REMOVE")) eCommand = ECommand::remove;
    else if (iequals(rsContext.seqCmdLine[1], "SHOW")) eCommand = ECommand::show;
    else
    {
        if (!rsContext.bSilent)
        {
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid listener command." << std::endl;
            ListenerHelp(rsContext);
        }
        return CMDLN_ARG_ERR;
    }

    if (rsContext.seqCmdLine.size() < 3)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing listener name... " << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Get the name
    std::string ssListener = rsContext.seqCmdLine[2];

    switch (eCommand)
    {
    case ECommand::remove:
        return RemoveListener(ssListener, rsContext);
    case ECommand::show:
        return ShowListenerConfig(ssListener, rsContext);
    case ECommand::add:
    default:
        break;
    }

    // For add, check for provider and parameters
    if (rsContext.seqCmdLine.size() < 4)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing listener provider... " << std::endl;
        return CMDLN_ARG_ERR;
    }
    std::string ssProvider = rsContext.seqCmdLine[3];

    sdv::toml::CTOMLParser parser;
    parser.AddValue("Provider.Name", ssProvider);

    // Check for parameters
    // Parameters have the format "param=value". It is possible that parameters have additional sub-tables. Then the format is:
    // "table.param=value".
    sdv::toml::CNodeCollection ipc_channel_table = parser.AddTable("IpcChannel");
    for (size_t nIndex = 4; nIndex < rsContext.seqCmdLine.size(); nIndex++)
    {
        std::string ssRawParam = rsContext.seqCmdLine[nIndex];

        // Search for the equal sign
        size_t nEqualPos = ssRawParam.find_first_of('=');
        std::string ssParamName = ssRawParam.substr(0, nEqualPos);
        while (!ssParamName.empty() && std::isspace(ssParamName.back()))
            ssParamName.pop_back();
        if (ssParamName.empty() || nEqualPos == std::string::npos)
        {
            if (!rsContext.bSilent)
                std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid listener parameter: " << ssRawParam << std::endl;
            return CMDLN_ARG_ERR;
        }
        sdv::any_t anyValue;
        size_t nValuePos = nEqualPos + 1;
        while (nValuePos < ssRawParam.size() && std::isspace(ssRawParam[nValuePos]))
            nValuePos++;
        if (nValuePos < ssRawParam.size())
        {
            std::string ssValue = ssRawParam.substr(nValuePos);
            if (!ssValue.empty() && ssValue.find_first_not_of("-0123456789") == std::string::npos)
            {
                int64_t iValue = 0;
                if (std::from_chars(ssValue.data(), ssValue.data() + ssValue.size(), iValue).ec == std::errc())
                    anyValue = iValue;
                else
                    anyValue = ssValue;
            }
            else
                anyValue = ssValue;

        }
        ipc_channel_table.AddValue(ssParamName, anyValue);
    }

    return AddListener(ssListener, parser.GetTOML(), rsContext);
}

int AddListener(const std::string& rssListener, const std::string& rssConfig, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Add the listener
    if (!pAppConnections->AddListenerConfig(rssListener, rssConfig))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << ADD_LISTENER_ERROR_MSG << std::endl;
        return ADD_LISTENER_ERROR;
    }

    // Save the settings.
    if (!pAppSettingsPersist->SaveSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_SAVE_SETTINGS_MSG << std::endl;
        return CANNOT_SAVE_SETTINGS;
    }

    return NO_ERROR;
}

int RemoveListener(const std::string& rssListener, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Remove the listener
    if (!pAppConnections->RemoveListenerConfig(rssListener))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << REMOVE_LISTENER_ERROR_MSG << std::endl;
        return REMOVE_LISTENER_ERROR;
    }

    // Save the settings.
    if (!pAppSettingsPersist->SaveSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_SAVE_SETTINGS_MSG << std::endl;
        return CANNOT_SAVE_SETTINGS;
    }

    return NO_ERROR;
}

int ShowListenerConfig(const std::string& rssListener, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Remove the listener
    std::string ssConfig = pAppConnections->GetListenerConfig(rssListener);
    if (ssConfig.empty())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << SHOW_LISTENER_ERROR_MSG << std::endl;
        return SHOW_LISTENER_ERROR;
    }

    std::cout << ssConfig << std::endl;

    return NO_ERROR;
}

int ConfigureConnection(const SContext& rsContext)
{
    // First argument should be "CONNECTION"
    if (rsContext.seqCmdLine.size() < 2)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing connection command... " << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "CONNECTION"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command: " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Which list command is requested
    enum class ECommand { unknown, add, remove, show } eCommand = ECommand::unknown;
    if (iequals(rsContext.seqCmdLine[1], "ADD")) eCommand = ECommand::add;
    else if (iequals(rsContext.seqCmdLine[1], "REMOVE")) eCommand = ECommand::remove;
    else if (iequals(rsContext.seqCmdLine[1], "SHOW")) eCommand = ECommand::show;
    else
    {
        if (!rsContext.bSilent)
        {
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid connection command." << std::endl;
            ConnectionHelp(rsContext);
        }
        return CMDLN_ARG_ERR;
    }

    if (rsContext.seqCmdLine.size() < 3)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing connection name... " << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Get the name
    std::string ssConnection = rsContext.seqCmdLine[2];

    switch (eCommand)
    {
    case ECommand::remove:
        return RemoveConnection(ssConnection, rsContext);
    case ECommand::show:
        return ShowConnectionConfig(ssConnection, rsContext);
    case ECommand::add:
    default:
        break;
    }

    // For add, check for provider and parameters
    if (rsContext.seqCmdLine.size() < 4)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing connection provider... " << std::endl;
        return CMDLN_ARG_ERR;
    }
    std::string ssProvider = rsContext.seqCmdLine[3];

    sdv::toml::CTOMLParser parser;
    parser.AddValue("Provider.Name", ssProvider);

    // Check for parameters
    // Parameters have the format "param=value". It is possible that parameters have additional sub-tables. Then the format is:
    // "table.param=value".
    sdv::toml::CNodeCollection ipc_channel_table = parser.AddTable("IpcChannel");
    for (size_t nIndex = 4; nIndex < rsContext.seqCmdLine.size(); nIndex++)
    {
        std::string ssRawParam = rsContext.seqCmdLine[nIndex];

        // Search for the equal sign
        size_t nEqualPos = ssRawParam.find_first_of('=');
        std::string ssParamName = ssRawParam.substr(0, nEqualPos);
        while (!ssParamName.empty() && std::isspace(ssParamName.back()))
            ssParamName.pop_back();
        if (ssParamName.empty() || nEqualPos == std::string::npos)
        {
            if (!rsContext.bSilent)
                std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid listener parameter: " << ssRawParam << std::endl;
            return CMDLN_ARG_ERR;
        }
        sdv::any_t anyValue;
        size_t nValuePos = nEqualPos + 1;
        while (nValuePos < ssRawParam.size() && std::isspace(ssRawParam[nValuePos]))
            nValuePos++;
        if (nValuePos < ssRawParam.size())
        {
            std::string ssValue = ssRawParam.substr(nValuePos);
            if (!ssValue.empty() && ssValue.find_first_not_of("-0123456789") == std::string::npos)
            {
                int64_t iValue = 0;
                if (std::from_chars(ssValue.data(), ssValue.data() + ssValue.size(), iValue).ec == std::errc())
                    anyValue = iValue;
                else
                    anyValue = ssValue;
            }
            else
                anyValue = ssValue;

        }
        ipc_channel_table.AddValue(ssParamName, anyValue);
    }

    return AddConnection(ssConnection, parser.GetTOML(), rsContext);
}

int AddConnection(const std::string& rssConnection, const std::string& rssConfig, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Add the connection
    if (!pAppConnections->AddConnectionConfig(rssConnection, rssConfig, rsContext.ssInsertBeforeConnection))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << ADD_CONNECTION_ERROR_MSG << std::endl;
        return ADD_CONNECTION_ERROR;
    }

    // Save the settings.
    if (!pAppSettingsPersist->SaveSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_SAVE_SETTINGS_MSG << std::endl;
        return CANNOT_SAVE_SETTINGS;
    }

    return NO_ERROR;
}

int RemoveConnection(const std::string& rssConnection, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Remove the connection
    if (!pAppConnections->RemoveConnectionConfig(rssConnection))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << REMOVE_CONNECTION_ERROR_MSG << std::endl;
        return REMOVE_CONNECTION_ERROR;
    }

    // Save the settings.
    if (!pAppSettingsPersist->SaveSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_SAVE_SETTINGS_MSG << std::endl;
        return CANNOT_SAVE_SETTINGS;
    }

    return NO_ERROR;
}

int ShowConnectionConfig(const std::string& rssConnection, const SContext& rsContext)
{
    // Load the settings.
    auto ptrAppService = sdv::core::GetObject("AppSettingsService");
    sdv::app::IAppSettingsPersist* pAppSettingsPersist = ptrAppService.GetInterface<sdv::app::IAppSettingsPersist>();
    sdv::app::IAppConnections* pAppConnections = ptrAppService.GetInterface<sdv::app::IAppConnections>();
    if (!pAppSettingsPersist || !pAppConnections || !pAppSettingsPersist->LoadSettings())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CANNOT_LOAD_SETTINGS_MSG << std::endl;
        return CANNOT_LOAD_SETTINGS;
    }

    // Remove the connection
    std::string ssConfig = pAppConnections->GetConnectionConfig(rssConnection);
    if (ssConfig.empty())
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << SHOW_CONNECTION_ERROR_MSG << std::endl;
        return SHOW_CONNECTION_ERROR;
    }

    std::cout << ssConfig << std::endl;

    return NO_ERROR;
}
