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
project(BasicTypesTests VERSION 1.0 LANGUAGES CXX)

# BasicTypes unit test executable
add_executable(UnitTest_BasicTypes
    "main.cpp"
    "basic_types_test.h"
    "basic_types_test.cpp"
    "string.cpp"
    "u8string.cpp"
    "u16string.cpp"
    "u32string.cpp"
    "wstring.cpp"
    "ptr.cpp"
    "ptr_simple.cpp"
    "ptr_complex.cpp"
    "sequence_simple.cpp"
    "sequence_complex.cpp"
    "serdes.cpp"
    "crc.cpp"
    "interface.cpp"
    "any.cpp"   )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_BasicTypes GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
else()
    target_link_libraries(UnitTest_BasicTypes GTest::GTest)
endif()

# Add the BasicTypes unittest
add_test(NAME UnitTest_BasicTypes COMMAND UnitTest_BasicTypes)

# Execute the test
add_custom_command(TARGET UnitTest_BasicTypes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_BasicTypes>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_BasicTypes.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_BasicTypes dependency_sdv_components)
