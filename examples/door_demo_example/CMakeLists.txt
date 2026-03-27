#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

project(DoorDemoExample)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include directory to the core framework
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/task_timer_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/front_left_door_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/front_right_door_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/rear_left_door_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/rear_right_door_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/door_complex_service.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_door.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

# Copy the ASC files used by can_com_sim.sdv which includes the CAN messages
# Required in both locations when running standalone or as instance
file (COPY ${PROJECT_SOURCE_DIR}/door_example_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)
file (COPY ${PROJECT_SOURCE_DIR}/door_example_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}../../bin)


######################################################################################################################################################################
#                                                                   preparation
######################################################################################################################################################################

# Execute sdv_vss_util to create IDL files for devices and basic services.
message("Create interface code for devices and basic services of doors example.")
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vss_doors_example.csv" "-O${PROJECT_SOURCE_DIR}/generated/" --prefixdoors --version1.0.0.1 --enable_components)


message("Compiling Interface Door01Left RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01left_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01left_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door01Right RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01right_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01right_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Left RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02left_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02left_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Right RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02right_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02right_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)


message("Compiling Interface Door01Left TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01left_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01left_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door01Right TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01right_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_01right_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Left TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02left_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02left_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Right TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02right_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodydooraxle_02right_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

# Execute the IDL compiler for the complex service to digest interface code.
message("Compiling door_ifc.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/door_service/door_ifc.idl" "-O${PROJECT_SOURCE_DIR}/generated/door_service/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Iexample_service/ --ps_lib_namedoors_service_proxystub)

add_subdirectory(generated/door_service/ps)
add_subdirectory(generated/vss_files/ps)

add_subdirectory(generated/vss_files/vd_frontdoorleft)
add_subdirectory(generated/vss_files/vd_frontdoorright)
add_subdirectory(generated/vss_files/vd_reardoorleft)
add_subdirectory(generated/vss_files/vd_reardoorright)

add_subdirectory(generated/vss_files/bs_frontdoorleft)
add_subdirectory(generated/vss_files/bs_frontdoorright)
add_subdirectory(generated/vss_files/bs_reardoorleft)
add_subdirectory(generated/vss_files/bs_reardoorright)

# Execute sdv_dbc_util to create data link code & FMU code.
message("Create data link for doors example")
execute_process(COMMAND ${SDV_DBC_UTIL} "${PROJECT_SOURCE_DIR}/datalink_4doors_example.dbc" "-O${PROJECT_SOURCE_DIR}/generated/" --nodesdoors --version1.0.0.1 --moduleDoorsExampleFMU --dl_lib_namecan_dl_doors)

add_subdirectory(generated/can_dl)


######################################################################################################################################################################
#                                                                     complex service
######################################################################################################################################################################

include_directories(${CMAKE_CURRENT_LIST_DIR}/door_app/include)

message("Include: proxy/stub for complex doors service")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/door_service)

add_library(doors_complex_service SHARED
    door_service/complex_service.h
    door_service/complex_service.cpp
    door_service/lock_doors_thread.h)
	
set_target_properties(doors_complex_service PROPERTIES OUTPUT_NAME "doors_complex_service")
set_target_properties(doors_complex_service PROPERTIES PREFIX "")
set_target_properties(doors_complex_service PROPERTIES SUFFIX ".sdv")

######################################################################################################################################################################
#                                                                     executable
######################################################################################################################################################################

add_executable(door_demo_example 
        "door_app/door_demo_example.cpp"
        "door_app/include/door_application.h"
        "door_app/door_application.cpp"
        "door_app/include/console.h"
        "door_app/console.cpp" 
        "door_app/include/signal_names.h"
        "door_service/lock_doors_thread.h")
        
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(door_demo_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(door_demo_example  ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(door_demo_example Rpcrt4.lib)
endif()

######################################################################################################################################################################
#                                                                     external application
######################################################################################################################################################################


add_executable(door_external_app 
        "door_app/door_extern_example.cpp"
        "door_app/include/door_extern_application.h"
        "door_app/door_extern_application.cpp"
        "door_app/include/console.h"
        "door_app/console.cpp" 
        "door_app/include/signal_names.h"
        "door_service/lock_doors_thread.h")
        
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(door_external_app Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(door_external_app  ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(door_external_app Rpcrt4.lib)
endif()


######################################################################################################################################################################
#                                                       create instance 3002 of door demo example
######################################################################################################################################################################
add_custom_target(vehicle_device_doors_config
    ALL
    DEPENDS
        can_dl_doors
        doors_vd_frontdoorleft_rx
        doors_vd_frontdoorright_rx
        doors_vd_reardoorleft_rx
        doors_vd_reardoorright_rx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL VehicleDeviceDoors --instance3002 can_dl_doors.sdv doors_vd_frontdoorleft_rx.sdv doors_vd_frontdoorright_rx.sdv doors_vd_reardoorleft_rx.sdv doors_vd_reardoorright_rx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --interface_config --overwrite
    VERBATIM
)

add_custom_target(basic_service_doors_config
    ALL
    DEPENDS
        doors_proxystub 
        doors_bs_frontdoorleft_rx
        doors_bs_frontdoorright_rx
        doors_bs_reardoorleft_rx
        doors_bs_reardoorright_rx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL BasicServiceDoors --instance3002 doors_proxystub.sdv doors_bs_frontdoorleft_rx.sdv doors_bs_frontdoorright_rx.sdv doors_bs_reardoorleft_rx.sdv doors_bs_reardoorright_rx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --abstract_config --overwrite
    VERBATIM
)

add_custom_target(door_complex_service_config
    ALL
    DEPENDS
        doors_complex_service
        doors_service_proxystub
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL DoorsComplexService --instance3002 doors_complex_service.sdv doors_service_proxystub.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --user_config --overwrite
    VERBATIM
)

add_custom_target(platform_doors_config
    ALL
    COMMAND "${SDV_PACKAGER}" CONFIGURE ${PROJECT_SOURCE_DIR}/coreconfig/platform.toml --instance3002 --platform_config
    VERBATIM
)


######################################################################################################################################################################
# the FMU files have been created via the dbc file
#    ..\\..\\build\\<configurePreset>\\bin\\sdv_dbc_util datalink_2doors_example.dbc -Ogenerated\\ --nodesdoors --version1.0.0.1 --moduleDoors2ExampleFMU --dl_lib_namecan_dl_door
#    ..\\..\\build\\<configurePreset>\\bin\\sdv_dbc_util datalink_4doors_example.dbc -Ogenerated\\ --nodesdoors --version1.0.0.1 --moduleDoors4ExampleFMU --dl_lib_namecan_dl_door
#
# following steps needs to be done:
# required configuration files have to be put into the subfolder
#     ...\fmu_Doors2ExampleFMU\Doors2ExampleFMU\resources
#     ...\fmu_Doors2ExampleFMU\Doors4ExampleFMU\resources
# function OpenAPILoad() needs to be updated in the model.cpp file
#     ...\fmu_Doors2ExampleFMU\Doors2ExampleFMU\model.cpp
#     ...\fmu_Doors2ExampleFMU\Doors4ExampleFMU\model.cpp
######################################################################################################################################################################
add_subdirectory(fmu_Doors2ExampleFMU)
add_subdirectory(fmu_Doors4ExampleFMU)
