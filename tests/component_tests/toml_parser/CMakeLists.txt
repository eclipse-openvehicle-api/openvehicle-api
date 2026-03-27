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
#   Erik Verhoeven - initial API and implementation
#*******************************************************************************

# Define project
project(ConfigTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(ComponentTest_TOMLParser toml_parser_tests.cpp)

target_link_libraries(ComponentTest_TOMLParser ${CMAKE_DL_LIBS} GTest::GTest)

# Add the test
add_test(NAME ComponentTest_TOMLParser COMMAND ComponentTest_TOMLParser WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_TOMLParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_TOMLParser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_TOMLParser.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_TOMLParser dependency_sdv_components)
