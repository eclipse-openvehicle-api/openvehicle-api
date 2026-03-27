#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#
# Contributors:
#   Martin Stimpfl - initial API and implementation
#   Erik Verhoeven - writing TOML and whitespace preservation
#*******************************************************************************

# Character Reader executable
add_executable(UnitTest_TOMLParser
    "character_reader_tests.cpp"
    "lexer_tests.cpp"
    "parser_tests.cpp"
    "main.cpp"
    "generate_toml_tests.cpp"
    "generate_toml_delete_node.cpp"
    "statement_boundary_detection.cpp"
    "miscellaneous_tests.cpp"
    "generate_toml_with_transfer.cpp"
    "generate_toml_switch_inline.cpp"
    "generate_toml_getset_comment.cpp"
    "generate_toml_insert_node.cpp" "generate_toml_miscellaneous.cpp" "generate_toml_combine_reduce.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_TOMLParser GTest::GTest ${CMAKE_THREAD_LIBS_INIT})
    if (WIN32)
        target_link_libraries(UnitTest_TOMLParser Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_TOMLParser ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_TOMLParser GTest::GTest Rpcrt4.lib)
endif()

# Add the Reader unittest
add_test(NAME UnitTest_TOMLParser COMMAND UnitTest_TOMLParser)

# Execute the test
add_custom_command(TARGET UnitTest_TOMLParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_TOMLParser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_TOMLParser.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_TOMLParser dependency_sdv_components)



