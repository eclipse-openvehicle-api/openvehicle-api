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
project(hardware_ident VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(hardware_ident SHARED
    "hardware_ident.h"
    "hardware_ident.cpp")
if (WIN32)
target_link_libraries(hardware_ident ${CMAKE_THREAD_LIBS_INIT} Rpcrt4.lib)
elseif(UNIX)
#target_link_libraries(hardware_ident rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(hardware_ident rt ${CMAKE_DL_LIBS})
endif()
target_link_options(hardware_ident PRIVATE)
target_include_directories(hardware_ident PRIVATE ./include/)
set_target_properties(hardware_ident PROPERTIES PREFIX "")
set_target_properties(hardware_ident PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(hardware_ident CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} hardware_ident PARENT_SCOPE)
