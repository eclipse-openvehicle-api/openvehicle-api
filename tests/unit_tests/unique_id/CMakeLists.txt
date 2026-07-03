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
project(UniqueIDTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(UnitTest_UniqueID unique_id_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_UniqueID GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_UniqueID Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_UniqueID ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_UniqueID GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_UniqueID COMMAND UnitTest_UniqueID WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_UniqueID POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_UniqueID>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_UniqueID.xml
    VERBATIM
)

# The unit-test project depends on a proper compilation of sdv components before
add_dependencies(UnitTest_UniqueID dependency_sdv_components)
