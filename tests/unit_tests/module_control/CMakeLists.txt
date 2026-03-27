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

# Define project (multiple EXEs)
project(ModuleControlTests VERSION 1.0 LANGUAGES CXX)

# ModuleControl test executable
add_executable(UnitTest_ModuleControl
    module_control_test.cpp
 "mock.h"  "main.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_ModuleControl GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_ModuleControl Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_ModuleControl ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_ModuleControl GTest::GTest Rpcrt4.lib)
endif()

# Add the ModuleControl unittest
add_test(NAME UnitTest_ModuleControl COMMAND UnitTest_ModuleControl)

# Execute the test
add_custom_command(TARGET UnitTest_ModuleControl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_ModuleControl>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_ModuleControl.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_ModuleControl dependency_sdv_components)
add_dependencies(UnitTest_ModuleControl install_manifest)
add_dependencies(UnitTest_ModuleControl data_dispatch_service)
add_dependencies(UnitTest_ModuleControl core_ps)
