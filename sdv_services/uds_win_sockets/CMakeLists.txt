#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

if(WIN32)
# Define project
project(uds_win_sockets VERSION 1.0 LANGUAGES CXX)

# Build sources once and expose them in two forms:
# - uds_win_sockets: static link target for tests / other local libraries on MinGW
# - uds_win_sockets_module: runtime-loadable .sdv module for ModuleControl
set(UDS_WIN_SOCKETS_SOURCES
    channel_mgnt.cpp
    connection.cpp
    watchdog.cpp
)

add_library(uds_win_sockets STATIC
    ${UDS_WIN_SOCKETS_SOURCES}
)

add_library(uds_win_sockets_module SHARED
    ${UDS_WIN_SOCKETS_SOURCES}
)

target_include_directories(uds_win_sockets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./include/
)

target_include_directories(uds_win_sockets_module
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./include/
)

target_link_libraries(uds_win_sockets
    PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        Ws2_32.lib
)

target_link_libraries(uds_win_sockets_module
    PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        Ws2_32.lib
)

set_target_properties(uds_win_sockets_module PROPERTIES
    PREFIX ""
    SUFFIX ".sdv"
    OUTPUT_NAME "uds_win_sockets"
    ARCHIVE_OUTPUT_NAME "uds_win_sockets_module"
)

# Build dependencies
add_dependencies(uds_win_sockets CompileCoreIDL)
add_dependencies(uds_win_sockets_module CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_win_sockets_module PARENT_SCOPE)

endif()
