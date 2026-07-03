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
project (ComponentTest_ParamUtils VERSION 1.0 LANGUAGES CXX)

# Data maneger executable
add_executable(ComponentTest_ParamUtils
    "main.cpp"
    "get_parameter.cpp"
    "resolve_text.cpp"
    )

target_link_libraries(ComponentTest_ParamUtils ${CMAKE_DL_LIBS} GTest::GTest)

# Add the ParamUtils component test
add_test(NAME ComponentTest_ParamUtils COMMAND ComponentTest_ParamUtils WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_ParamUtils POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_ParamUtils>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_ParamUtils.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_ParamUtils dependency_sdv_components)
