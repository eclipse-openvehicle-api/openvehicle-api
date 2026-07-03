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
project (UnitTest_App_Settings VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_App_Settings
    "main.cpp"
     "startup_config.cpp" "settings_config.cpp")
target_link_libraries(UnitTest_App_Settings ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_App_Settings COMMAND UnitTest_App_Settings)

# Execute the test
add_custom_command(TARGET UnitTest_App_Settings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_App_Settings>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_App_Settings.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_App_Settings dependency_sdv_components)
