#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

project(SystemDemoExample)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include directory to the core framework
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

######################################################################################################################################################################
#                                                                   preparation
######################################################################################################################################################################

# REMARK: The code generation for the proxy/stub, interface definitions and serialization, the vehicle devices and the basic
# services are generated during the configuration phase of CMake. This is necessary, since CMakeFiles.txt files are generated and
# they have to be available during the configuration phase of CMake to be taken into the build process. Requisite for the code
# generation during the configuration time of CMake is the availability of the tools to do the generation. Hence the tools cannot be
# created during the build process, which is executed after the configuration process.

# Execute sdv_vss_util to create IDL files for devices and basic services.
# REMARK: We need 2 cvs definition files because we want to create 2 identical components. 
# Both need to have the same input signal as well as the same output interface. Output must be vehicle speed in km/h.
# One of the component has to convert the value from m/s to km/h, the other one can pass the value directly.
message("Create interface code for devices and basic services of vehicle abstraction example.")
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vehicle_abstraction_example.csv" "-O${PROJECT_SOURCE_DIR}/generated/" --prefixabstraction --version1.0.0.1 --enable_components)
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vehicle_abstraction_example_ms.csv" "-O${PROJECT_SOURCE_DIR}/generated2/" --prefixabstraction --version1.0.0.1 --enable_components)

# Execute the IDL compiler for the VSS interfaces to digest interface code. Compile with --no_ps as we do not need proxies and stubs as we do not like to expose these interfaces for complex services or applications
message("Compiling vss_vehiclespeed_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclespeed_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated2/vss_files/vss_vehiclespeed_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated2/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated2/vss_files/ --no_ps)


# We need proxies and stubs for basic services to give access to the interfaces for complex services and applications: --ps_lib_namedemo_proxystub
message("Compiling vss_vehiclespeed_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nameabstraction_proxystub)


# Execute sdv_dbc_util to create data link code & FMU code.
message("Create data link for vehicle abstraction example")
execute_process(COMMAND ${SDV_DBC_UTIL} "${PROJECT_SOURCE_DIR}/vehicle_abstraction_example.dbc" "-O${PROJECT_SOURCE_DIR}/generated/" --nodesabstraction --version1.0.0.1  --dl_lib_namecan_dl_abstraction)

######################################################################################################################################################################
#                                                                   data link component
######################################################################################################################################################################

# REMARK: CAN data link code was generated during the configuration phase of CMake. Following below is the build step to build the
# component that was generated.

message("Include: example component can_dl_abstraction")
add_subdirectory(generated/can_dl)

#######################################################################################################################################################################
##                                                                   vehicle devices and basic services
#######################################################################################################################################################################

# REMARK: Proxy/stub and vehicle device and basic service code was generated during the configuration phase of CMake. Following
# below are the build steps to build the components that were generated.
message("Include: vehicle abstraction proxy/stub for basic services")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/vss_files)
add_subdirectory(generated/vss_files/ps)

# REMARK: We create 2 'vd_*' components with identical input and output interface. Therefore we needed 2 csv definition files for their creation.
# The difference is: one passes the input value to output, the other one needs to convert the value from m/s to km/h to match the unit.
add_subdirectory(generated/vss_files/vd_vehiclespeedkmh)
add_subdirectory(generated2/vss_files/vd_vehiclespeedms)

# REMARK: Because the output interface of the 'vd_*' components is identical (km/h), we need only one bs* component which expects to get a value in km/h.
add_subdirectory(generated/vss_files/bs_vehiclespeed)

######################################################################################################################################################################
#                                                                   vehicle abstraction application
######################################################################################################################################################################

# Define the executable
add_executable(vehicle_abstraction_example
    vehicle_abstraction_app/vehicle_abstraction_example.cpp
    vehicle_abstraction_app/vehicle_abstraction_application.cpp
    vehicle_abstraction_app/vehicle_abstraction_application.h
    vehicle_abstraction_app/console.cpp
    vehicle_abstraction_app/console.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(vehicle_abstraction_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(vehicle_abstraction_example  ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(vehicle_abstraction_example Rpcrt4.lib)
endif()

# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/can_com_simulation_vehicle_abstraction_ms.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/can_com_simulation_vehicle_abstraction_kmh.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_vehicle_abstraction.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_vehicle_abstraction.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/task_timer_vehicle.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/vehicle_abstraction_device_kmh.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/vehicle_abstraction_device_ms.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/vehicle_abstraction_basic_service.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

# Copy the ASC files used by can_com_sim.sdv which includes the CAN messages. One file includes vehicle speed in m/s, the other file in km/h.
file (COPY ${PROJECT_SOURCE_DIR}/vehicle_abstraction_example_receiver_ms.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)
file (COPY ${PROJECT_SOURCE_DIR}/vehicle_abstraction_example_receiver_kmh.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)

