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
# created during the build process, which is executed after the configuraiton process.

# Execute sdv_vss_util to create IDL files for devices and basic services.
message("Create interface code for devices and basic services of open trunk example.")
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vss_open_trunk_example.csv" "-O${PROJECT_SOURCE_DIR}/generated/" --prefixtrunk --version1.0.0.1 --enable_components)



# Execute the IDL compiler for the VSS interfaces to digest interface code. Compile with --no_ps as we do not need proxies and stubs as we do not like to expose these interfaces for complex services or applications
message("Compiling vss_vehiclebodytrunk_vd_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodytrunk_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclespeed_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)



# We need proxies and stubs for basic services to give access to the interfaces for complex services ans applications: --ps_lib_namedemo_proxystub
message("Compiling vss_vehiclebodytrunk_bs_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodytrunk_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nametrunk_proxystub)
message("Compiling vss_vehiclespeed_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nametrunk_proxystub)


# Execute sdv_dbc_util to create data link code & FMU code.
message("Create data link for open trunk example")
execute_process(COMMAND ${SDV_DBC_UTIL} "${PROJECT_SOURCE_DIR}/open_trunk_example.dbc" "-O${PROJECT_SOURCE_DIR}/generated/" --nodestrunk --version1.0.0.1 --moduleTrunkExampleFMU --dl_lib_namecan_dl_trunk)

# Execute the IDL compiler for the complex service to digest interface code.
message("Compiling trunkkit.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/trunk_service/trunkkit.idl" "-O${PROJECT_SOURCE_DIR}/generated/trunk_service/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Itrunk_service/ --ps_lib_nametrunk_service_proxystub)

######################################################################################################################################################################
#                                                                   data link component
######################################################################################################################################################################

# REMARK: CAN data link code was generated during the configuration phase of CMake. Following below is the build step to build the
# component that was generated.

message("Include: example component can_dl_trunk")
add_subdirectory(generated/can_dl)

#######################################################################################################################################################################
##                                                                   vehicle devices and basic services
#######################################################################################################################################################################

# REMARK: Proxy/stub and vehicle device and basic service code was generated during the configuration phase of CMake. Following
# below are the build steps to build the components that were generated.

message("Include: trunk proxy/stub for vehicle devices and basic services")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/vss_files)
add_subdirectory(generated/vss_files/ps)

add_subdirectory(generated/vss_files/vd_vehicletrunk)
add_subdirectory(generated/vss_files/vd_vehiclespeed)

add_subdirectory(generated/vss_files/bs_vehicletrunk)
add_subdirectory(generated/vss_files/bs_vehiclespeed)

######################################################################################################################################################################
#                                                                     complex service
######################################################################################################################################################################

message("Include: proxy/stub for complex trunk service")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/trunk_service)
add_subdirectory(generated/trunk_service/ps)

message("Include: example component trunk_complex_service")
add_library(trunk_complex_service SHARED
    trunk_service/complex_service.h
    trunk_service/complex_service.cpp
    )
set_target_properties(trunk_complex_service PROPERTIES OUTPUT_NAME "trunk_complex_service")
set_target_properties(trunk_complex_service PROPERTIES PREFIX "")
set_target_properties(trunk_complex_service PROPERTIES SUFFIX ".sdv")

######################################################################################################################################################################
#                                                                   basic_system trunk application
######################################################################################################################################################################

# Define the executable
add_executable(open_trunk_example
    open_trunk_app/trunk_example.cpp
    open_trunk_app/trunk_application.cpp
    open_trunk_app/trunk_application.h
    open_trunk_app/console.cpp
    open_trunk_app/console.h
    open_trunk_app/signal_names.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(open_trunk_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(open_trunk_example  ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(open_trunk_example Rpcrt4.lib)
endif()
#[[ 

######################################################################################################################################################################
#                                                                   open trunk application
######################################################################################################################################################################

# Define the executable
add_executable(system_trunk_example
    example_app/system_trunk_example.cpp
    example_app/control.h
    example_app/control.cpp
    example_app/console.h
    example_app/console.cpp
    example_app/signal_names.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(system_trunk_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(system_trunk_example ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(system_trunk_example Rpcrt4.lib)
endif()
#]]
# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/can_com_simulation_trunk.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/complex_service_trunk.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_trunk.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_trunk.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/task_timer_trunk.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/trunk_vehicle_device_and_basic_service.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

# Copy the ASC files used by can_com_sim.sdv which includes the CAN messages
# Required in both locations when running standalone or as instance
file (COPY ${PROJECT_SOURCE_DIR}/open_trunk_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)
file (COPY ${PROJECT_SOURCE_DIR}/open_trunk_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}../../bin)

######################################################################################################################################################################
#                                                       create instance 3005 of system trunk application
######################################################################################################################################################################

# REMARK: Currently the "sdv_packager" tool takes the created component(s), creates a configuration and installation manifest and
# copies the package including manifest and configuration to the runtime installation directory (SDV_FRAMEWORK_RUNTIME
# environment variable with instance #3001). The copying into the target directory is not the wanted installation procedure and only
# a bypass feature of the packager. The proper installation would be creating a installation package with the packager, uploading
# the package to the target system and installing the package using the "sdv_control" tool. Since the creation and installation of
# packages is not available yet, the bypass solution is the only solution available at the moment.

add_custom_target(trunk_interface_config
    ALL
    DEPENDS
        can_dl_trunk
        trunk_vd_vehiclespeed_rx
        trunk_vd_vehicletrunk_tx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL ExampleInterfaceComponents --instance3005 can_dl_trunk.sdv trunk_vd_vehiclespeed_rx.sdv trunk_vd_vehicletrunk_tx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --interface_config   --overwrite
    VERBATIM
)

add_custom_target(trunk_abstract_config
    ALL
    DEPENDS
        trunk_proxystub
        trunk_bs_vehiclespeed_rx
        trunk_bs_vehicletrunk_tx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL ExampleAbstractComponents --instance3005 trunk_proxystub.sdv trunk_bs_vehiclespeed_rx.sdv trunk_bs_vehicletrunk_tx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --abstract_config  --overwrite 
    VERBATIM
)

add_custom_target(trunk_user_config
    ALL
    DEPENDS
        trunk_complex_service
        trunk_service_proxystub
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL TrunkApplication --instance3005 trunk_complex_service.sdv trunk_service_proxystub.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --user_config   --overwrite
    VERBATIM
)

######################################################################################################################################################################
#   TODO:           SDV_PACKAGER does not create the toml files, therefore we need to copy them
######################################################################################################################################################################
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/trunk.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3005)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/platform.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3005)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/settings.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3005)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_abstract.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3005)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_ifc.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3005)




