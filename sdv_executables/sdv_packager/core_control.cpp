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

#include "core_control.h"
#include "../../sdv_services/core/app_settings.cpp"
#include "../../sdv_services/core/app_config.cpp"
#include "../../sdv_services/core/app_config_file.cpp"
#include "../../sdv_services/core/installation_manifest.cpp"
#include "../../sdv_services/core/installation_composer.cpp"
#include "../../sdv_services/core/toml_parser/parser_toml.cpp"
#include "../../sdv_services/core/toml_parser/lexer_toml.cpp"
#include "../../sdv_services/core/toml_parser/lexer_toml_token.cpp"
#include "../../sdv_services/core/toml_parser/parser_node_toml.cpp"
#include "../../sdv_services/core/toml_parser/character_reader_utf_8.cpp"
#include "../../sdv_services/core/toml_parser/miscellaneous.cpp"
#include "../../sdv_services/core/toml_parser/code_snippet.cpp"

