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
file (COPY ${PROJECT_SOURCE_DIR}/config/door_comple_service.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_door.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

# Copy the ASC files used by can_com_sim.sdv which includes the CAN messages
# Required in both locations when running standalone or as instance
file (COPY ${PROJECT_SOURCE_DIR}/door_example_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)
file (COPY ${PROJECT_SOURCE_DIR}/door_example_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}../../bin)


######################################################################################################################################################################
#                                                                   preparation
######################################################################################################################################################################

message("Compiling Interface Door01Left RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01left_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01left_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door01Right RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01right_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01right_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Left RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02left_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02left_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Right RX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02right_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02right_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)


message("Compiling Interface Door01Left TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01left_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01left_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door01Right TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01right_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle01right_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Left TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02left_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02left_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

message("Compiling Interface Door02Right TX")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02right_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/interfaces/vss_vehiclechassisdooraxle02right_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/interfaces/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/  --ps_lib_namedoors_proxystub)

# Execute the IDL compiler for the complex service to digest interface code.
message("Compiling door_ifc.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/door_service/door_ifc.idl" "-O${PROJECT_SOURCE_DIR}/generated/door_service/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Iexample_service/ --ps_lib_namedoors_service_proxystub)

add_subdirectory(interfaces/ps)

add_subdirectory(vd/vd_front_door_left)  
add_subdirectory(vd/vd_front_door_right)
add_subdirectory(vd/vd_rear_door_left)
add_subdirectory(vd/vd_rear_door_right)

add_subdirectory(bs/bs_front_door_left)
add_subdirectory(bs/bs_front_door_right)
add_subdirectory(bs/bs_rear_door_left)
add_subdirectory(bs/bs_rear_door_right)

add_subdirectory(can_dl_door)


######################################################################################################################################################################
#                                                                     complex service
######################################################################################################################################################################

include_directories(interfaces)
include_directories(${CMAKE_CURRENT_LIST_DIR}/door_app/include)

message("Include: proxy/stub for complex doors service")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/door_service)
add_subdirectory(generated/door_service/ps)

add_library(door_complex_service SHARED
    door_service/complex_service.h
    door_service/complex_service.cpp
    door_service/lock_doors_thread.h)
	
set_target_properties(door_complex_service PROPERTIES OUTPUT_NAME "door_complex_service")
set_target_properties(door_complex_service PROPERTIES PREFIX "")
set_target_properties(door_complex_service PROPERTIES SUFFIX ".sdv")

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
        can_dl_door
        doors_vd_frontdoorleft
        doors_vd_frontdoorright
        doors_vd_reardoorleft
        doors_vd_reardoorright
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL VehicleDeviceDoors --instance3002 can_dl_door.sdv doors_vd_frontdoorleft.sdv doors_vd_frontdoorright.sdv doors_vd_reardoorleft.sdv doors_vd_reardoorright.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --interface_config --overwrite
    VERBATIM
)

add_custom_target(basic_service_doors_config
    ALL
    DEPENDS
        doors_proxystub 
        doors_bs_frontdoorleft
        doors_bs_frontdoorright
        doors_bs_reardoorleft
        doors_bs_reardoorright
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL BasicServiceDoors --instance3002 doors_proxystub.sdv doors_bs_frontdoorleft.sdv doors_bs_frontdoorright.sdv doors_bs_reardoorleft.sdv doors_bs_reardoorright.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --abstract_config --overwrite
    VERBATIM
)

add_custom_target(door_complex_service_config
    ALL
    DEPENDS
        door_complex_service
        doors_service_proxystub
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL DoorComplexService --instance3002 door_complex_service.sdv doors_service_proxystub.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --user_config --overwrite
    VERBATIM
)

######################################################################################################################################################################
#   TODO:           SDV_PACKAGER does not create the toml files, therefore we need to copy them
######################################################################################################################################################################
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/door_demo.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3002)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/platform.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3002)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/settings.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3002)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_abstract.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3002)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_ifc.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3002)

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
