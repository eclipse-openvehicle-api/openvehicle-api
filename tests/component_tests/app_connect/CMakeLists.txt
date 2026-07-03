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
project (ComponentTest_App_Connect VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_component.h ${PROJECT_SOURCE_DIR}/generated/ps/proxystub.cpp ${PROJECT_SOURCE_DIR}/generated/serdes/test_component_serdes.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY test_component.idl
    COMMENT "Build test_component.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_component.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )

set_source_files_properties(test_component_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/ps/proxystub.cpp)
set_source_files_properties(test_component_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/serdes/test_component_serdes.h)
set_source_files_properties(test_component_device.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_basic_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_complex_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_system_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_utility.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)

# Create the proxy stub dynamic library
add_library(AppConnect_Component_ps SHARED "test_component_ps.cpp")
set_target_properties(AppConnect_Component_ps PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_ps PROPERTIES SUFFIX ".sdv")

# Create the device dynamic library
add_library(AppConnect_Component_Device SHARED "test_component_device.cpp")
set_target_properties(AppConnect_Component_Device PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_Device PROPERTIES SUFFIX ".sdv")

# Create the basic service dynamic library
add_library(AppConnect_Component_BasicService SHARED "test_component_basic_service.cpp")
set_target_properties(AppConnect_Component_BasicService PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_BasicService PROPERTIES SUFFIX ".sdv")

# Create the complex service dynamic library
add_library(AppConnect_Component_ComplexService SHARED "test_component_complex_service.cpp")
set_target_properties(AppConnect_Component_ComplexService PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_ComplexService PROPERTIES SUFFIX ".sdv")

# Create the system service dynamic library
add_library(AppConnect_Component_SystemService SHARED "test_component_system_service.cpp")
set_target_properties(AppConnect_Component_SystemService PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_SystemService PROPERTIES SUFFIX ".sdv")

# Create the utility dynamic library
add_library(AppConnect_Component_Utility SHARED "test_component_utility.cpp")
set_target_properties(AppConnect_Component_Utility PROPERTIES PREFIX "")
set_target_properties(AppConnect_Component_Utility PROPERTIES SUFFIX ".sdv")

# Server instances: Install the device, system, basic and complex service, utility and prox/stub components in server instances
#   shared memory: 195 and 196
#   UDS: 199 and 200
#   Tunnel-UDS: 203 and 204
# Client instances: Install the proxy/stub components
#   shared memory: 197 and 198
#   UDS: 201 and 202
#   Tunnel-UDS: 205 and 206
add_custom_target(AppConnect_Component_Install
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance195 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance196 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance197 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance198 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance199 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance200 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance201 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance202 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance203 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component --instance204 "AppConnect_Component_*.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance205 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL AppConnect_Component_ps --instance206 "AppConnect_Component_ps.sdv" "-I$<TARGET_FILE_DIR:AppConnect_Component_ps>" --overwrite
    DEPENDS dependency_sdv_components AppConnect_Component_ps AppConnect_Component_Device AppConnect_Component_BasicService AppConnect_Component_ComplexService AppConnect_Component_SystemService AppConnect_Component_Utility
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Configure listener for server instance
#   Shared memory: 196 (195 is default)
#   UDS: 199, 200
#   Tunnel-UDS: 203, 204
# Connections for client instances
#   Shared memory: 197 and 198
#   UDS: 201 and 202
#   Tunnel-UDS: 205 and 206
add_custom_target(AppConnect_Communication_Install
    COMMAND "$<TARGET_FILE:sdv_control>" LISTENER ADD MySpecialListener DefaultSharedMemory Name=private_channel_196_198 --instance196
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD DefaultChannelTo195 DefaultSharedMemory Name=DEFAULT_CHANNEL_195 --instance197
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD MySpecialConnection DefaultSharedMemory Name=private_channel_196_198 --instance198
    COMMAND "$<TARGET_FILE:sdv_control>" LISTENER ADD Default unix_domain_sockets Name=DEFAULT_CHANNEL_199 --instance199
    COMMAND "$<TARGET_FILE:sdv_control>" LISTENER ADD MySpecialListener unix_domain_sockets Name=private_channel_200_202 --instance200
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD DefaultChannelTo199 unix_domain_sockets Name=DEFAULT_CHANNEL_199 --instance201
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD MySpecialConnection unix_domain_sockets Name=private_channel_200_202 --instance202
    COMMAND "$<TARGET_FILE:sdv_control>" LISTENER ADD Default unix_domain_sockets_tunnel Name=DEFAULT_CHANNEL_203 --instance203
    COMMAND "$<TARGET_FILE:sdv_control>" LISTENER ADD MySpecialListener unix_domain_sockets_tunnel Name=private_channel_204_206 --instance204
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD DefaultChannelTo203 unix_domain_sockets_tunnel Name=DEFAULT_CHANNEL_203 --instance205
    COMMAND "$<TARGET_FILE:sdv_control>" CONNECTION ADD MySpecialConnection unix_domain_sockets_tunnel Name=private_channel_204_206 --instance206
    DEPENDS dependency_sdv_components AppConnect_Component_ps AppConnect_Component_Device AppConnect_Component_BasicService AppConnect_Component_ComplexService AppConnect_Component_SystemService AppConnect_Component_Utility
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Compile the source code of the server
add_executable(AppConnect_Server
    "main_server.cpp")
target_link_libraries(AppConnect_Server ${CMAKE_DL_LIBS})

# Compile the source code
add_executable(ComponentTest_App_Connect
    "main.cpp"
    "app_connect_tests_shared_mem.cpp"
    "app_connect_tests_uds.cpp"
    "app_connect_tests_uds_tunnel.cpp"
    )
target_link_libraries(ComponentTest_App_Connect ${CMAKE_DL_LIBS} GTest::GTest)

# Add the connect test
add_test(NAME ComponentTest_App_Connect COMMAND ComponentTest_App_Connect)

# Execute the test
add_custom_command(TARGET ComponentTest_App_Connect POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_App_Connect>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_App_Connect.xml
    VERBATIM
)

# Build dependencies
add_dependencies(AppConnect_Server dependency_sdv_components)
add_dependencies(ComponentTest_App_Connect dependency_sdv_components)
add_dependencies(ComponentTest_App_Connect AppConnect_Server)
add_dependencies(ComponentTest_App_Connect AppConnect_Component_Install)
add_dependencies(ComponentTest_App_Connect AppConnect_Communication_Install)
add_dependencies(AppConnect_Component_ps dependency_sdv_components)
add_dependencies(AppConnect_Component_Device dependency_sdv_components)
add_dependencies(AppConnect_Component_BasicService dependency_sdv_components)
add_dependencies(AppConnect_Component_ComplexService dependency_sdv_components)
add_dependencies(AppConnect_Component_SystemService dependency_sdv_components)
add_dependencies(AppConnect_Component_Utility dependency_sdv_components)
