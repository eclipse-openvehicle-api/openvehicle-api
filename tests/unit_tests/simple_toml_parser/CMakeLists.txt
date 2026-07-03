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

# Define project
project (UnitTest_SimpleTOMLParser VERSION 1.0 LANGUAGES CXX)

# Character Reader executable
add_executable(UnitTest_SimpleTOMLParser
    "parser_tests.cpp" "main.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SimpleTOMLParser GTest::GTest ${CMAKE_THREAD_LIBS_INIT})
    if (WIN32)
        target_link_libraries(UnitTest_SimpleTOMLParser Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SimpleTOMLParser ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SimpleTOMLParser GTest::GTest Rpcrt4.lib)
endif()

# Add the Reader unittest
add_test(NAME UnitTest_SimpleTOMLParser COMMAND UnitTest_SimpleTOMLParser)

# Execute the test
add_custom_command(TARGET UnitTest_SimpleTOMLParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SimpleTOMLParser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SimpleTOMLParser.xml
    VERBATIM
)

# Build dependencies
#add_dependencies(UnitTest_SimpleTOMLParser dependency_sdv_components)



