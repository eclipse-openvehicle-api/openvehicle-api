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
project(sdv_packager VERSION 1.0 LANGUAGES CXX)

# build test application
add_executable(sdv_packager
    "main.cpp"
    "environment.cpp"
    "environment.h"
    "packager.cpp"
    "packager.h"
      "core_control.cpp" "core_control.h")

# Compiler settings
add_compile_definitions(SDV_NO_CLASS_DEFINITION)

# Linker settings
if (WIN32)
    target_link_libraries(sdv_packager ${CMAKE_THREAD_LIBS_INIT} Ws2_32 Winmm Rpcrt4.lib)
else()
    target_link_libraries(sdv_packager ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Build dependencies
add_dependencies(sdv_packager core_services)

# Execute the installation packager utility to create an installation manifest for the core files.
# TODO EVE
add_custom_command(
    OUTPUT install_manifest.toml
    COMMAND "$<TARGET_FILE:sdv_packager>" PACK core -O. ./*.sdv --create_manifest_only --verbose
    DEPENDS sdv_packager dependency_sdv_services
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    VERBATIM
)
add_custom_target(
  install_manifest
  DEPENDS install_manifest.toml
)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_packager PARENT_SCOPE)
set(SDV_Executable_List ${SDV_Executable_List} install_manifest PARENT_SCOPE)
