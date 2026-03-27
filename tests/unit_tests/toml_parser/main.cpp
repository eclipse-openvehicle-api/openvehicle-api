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
 *   Martin Stimpfl - initial API and implementation
 *   Erik Verhoeven - writing TOML and whitespace preservation
 ********************************************************************************/

#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../sdv_services/core/toml_parser/character_reader_utf_8.cpp"
#include "../../../sdv_services/core/toml_parser/lexer_toml.cpp"
#include "../../../sdv_services/core/toml_parser/lexer_toml_token.cpp"
#include "../../../sdv_services/core/toml_parser/parser_toml.cpp"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.cpp"
#include "../../../sdv_services/core/toml_parser/miscellaneous.cpp"
#include "../../../sdv_services/core/toml_parser/code_snippet.cpp"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
