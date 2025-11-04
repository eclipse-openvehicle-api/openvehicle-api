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
message("Create interface code for devices and basic services of demo example.")
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vss_demo_example.csv" "-O${PROJECT_SOURCE_DIR}/generated/" --prefixdemo --version1.0.0.1 --enable_components)

# Execute the IDL compiler for the VSS interfaces to digest interface code. Compile with --no_ps as we do not need proxies and stubs as we do not like to expose these interfaces for complex services or applications
message("Compiling vss_vehiclechassisrearaxlerowwheel_vd_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclechassisrearaxlerowwheel_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclechassissteeringwheelangle_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclechassissteeringwheelangle_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclesoftwareapplicationisactivecounter_vd_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclesoftwareapplicationisactivecounter_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclespeed_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)

# We need proxies and stubs for basic services to give access to the interfaces for complex services ans applications: --ps_lib_namedemo_proxystub
message("Compiling vss_vehiclechassisrearaxlerowwheel_bs_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclechassisrearaxlerowwheel_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_namedemo_proxystub)
message("Compiling vss_vehiclechassissteeringwheelangle_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclechassissteeringwheelangle_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_namedemo_proxystub)
message("Compiling vss_vehiclesoftwareapplicationisactivecounter_bs_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclesoftwareapplicationisactivecounter_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_namedemo_proxystub)
message("Compiling vss_vehiclespeed_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclespeed_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_namedemo_proxystub)

# Execute sdv_dbc_util to create data link code & FMU code.
message("Create functional mockup unit (FMU) of demo example.")
execute_process(COMMAND ${SDV_DBC_UTIL} "${PROJECT_SOURCE_DIR}/datalink_demo_example.dbc" "-O${PROJECT_SOURCE_DIR}/generated/" --nodesdemo --version1.0.0.1 --moduleDemoExampleFMU --dl_lib_namecan_dl_example)

# Execute the IDL compiler for the complex service to digest interface code.
message("Compiling countersteering.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/example_service/countersteering.idl" "-O${PROJECT_SOURCE_DIR}/generated/example_service/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Iexample_service/ --ps_lib_nameexample_service_proxystub)

######################################################################################################################################################################
#                                                                   data link component
######################################################################################################################################################################

# REMARK: CAN data link code was generated during the configuration phase of CMake. Following below is the build step to build the
# component that was generated.

message("Include: example component can_dl_example")
add_subdirectory(generated/can_dl)

#######################################################################################################################################################################
##                                                                   vehicle devices and basic services
#######################################################################################################################################################################

# REMARK: Proxy/stub and vehicle device and basic service code was generated during the configuration phase of CMake. Following
# below are the build steps to build the components that were generated.

message("Include: demo proxy/stub for vehicle devices and basic services")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/vss_files)
add_subdirectory(generated/vss_files/ps)


add_subdirectory(generated/vss_files/vd_isactivecounter)
add_subdirectory(generated/vss_files/vd_vehiclechassisaxle)
add_subdirectory(generated/vss_files/vd_steeringwheel)
add_subdirectory(generated/vss_files/vd_vehiclespeed)

add_subdirectory(generated/vss_files/bs_isactivecounter)
add_subdirectory(generated/vss_files/bs_vehiclechassisaxle)
add_subdirectory(generated/vss_files/bs_steeringwheel)
add_subdirectory(generated/vss_files/bs_vehiclespeed)

######################################################################################################################################################################
#                                                                     complex service
######################################################################################################################################################################

message("Include: proxy/stub for complex example service")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/example_service)
add_subdirectory(generated/example_service/ps)

message("Include: example component demo_complex_service")
add_library(demo_complex_service SHARED
    example_service/complex_service.h
    example_service/complex_service.cpp
    )
set_target_properties(demo_complex_service PROPERTIES OUTPUT_NAME "demo_complex_service")
set_target_properties(demo_complex_service PROPERTIES PREFIX "")
set_target_properties(demo_complex_service PROPERTIES SUFFIX ".sdv")

######################################################################################################################################################################
#                                                                   basic_system demo application
######################################################################################################################################################################

# Define the executable
add_executable(system_extern_example
    example_app/system_extern_example.cpp
    example_app/signal_names.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(system_extern_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(system_extern_example  ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(system_extern_example Rpcrt4.lib)
endif()


######################################################################################################################################################################
#                                                                   system demo application
######################################################################################################################################################################

# Define the executable
add_executable(system_demo_example
    example_app/system_demo_example.cpp
    example_app/control.h
    example_app/control.cpp
    example_app/console.h
    example_app/console.cpp
    example_app/signal_names.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(system_demo_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(system_demo_example ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(system_demo_example Rpcrt4.lib)
endif()

# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/can_com_simulation.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/task_timer_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/vehicle_devices_basic_services_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/complex_service_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

# Copy the ASC files used by can_com_sim.sdv which includes the CAN messages
# Required in both locations when running standalone or as instance
file (COPY ${PROJECT_SOURCE_DIR}/system_demo_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}/bin)
file (COPY ${PROJECT_SOURCE_DIR}/system_demo_receiver.asc DESTINATION ${CMAKE_BINARY_DIR}../../bin)

######################################################################################################################################################################
#                                                       create instance 3001 of system demo application
######################################################################################################################################################################

# REMARK: Currently the "sdv_packager" tool takes the created component(s), creates a configuration and installation manifest and
# copies the package including manifest and configuration to the runtime installation directory (SDV_FRAMEWORK_RUNTIME
# environment variable with instance #3001). The copying into the target directory is not the wanted installation procedure and only
# a bypass feature of the packager. The proper installation would be creating a installation package with the packager, uploading
# the package to the target system and installing the package using the "sdv_control" tool. Since the creation and installation of
# packages is not available yet, the bypass solution is the only solution available at the moment.

add_custom_target(example_interface_config
    ALL
    DEPENDS
        can_dl_example
        demo_vd_isactivecounter_tx
        demo_vd_steeringwheel_rx
        demo_vd_vehiclechassisaxle_tx
        demo_vd_vehiclespeed_rx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL ExampleInterfaceComponents --instance3001 can_dl_example.sdv demo_vd_isactivecounter_tx.sdv demo_vd_steeringwheel_rx.sdv demo_vd_vehiclechassisaxle_tx.sdv demo_vd_vehiclespeed_rx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --interface_config   --overwrite
    VERBATIM
)

add_custom_target(example_abstract_config
    ALL
    DEPENDS
        demo_proxystub
        demo_bs_isactivecounter_tx
        demo_bs_steeringwheel_rx
        demo_bs_vehiclechassisaxle_tx
        demo_bs_vehiclespeed_rx
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL ExampleAbstractComponents --instance3001 demo_proxystub.sdv demo_bs_isactivecounter_tx.sdv demo_bs_steeringwheel_rx.sdv demo_bs_vehiclechassisaxle_tx.sdv demo_bs_vehiclespeed_rx.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --abstract_config  --overwrite 
    VERBATIM
)

add_custom_target(example_user_config
    ALL
    DEPENDS
        demo_complex_service
        example_service_proxystub
    COMMAND "${SDV_PACKAGER}" DIRECT_INSTALL DemoApplication --instance3001 demo_complex_service.sdv example_service_proxystub.sdv "-I${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" --user_config   --overwrite
    VERBATIM
)

######################################################################################################################################################################
#   TODO:           SDV_PACKAGER does not create the toml files, therefore we need to copy them
######################################################################################################################################################################
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/demo.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3001)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/platform.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3001)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/settings.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3001)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_abstract.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3001)
file (COPY ${PROJECT_SOURCE_DIR}/coreconfig/vehicle_ifc.toml DESTINATION ${SDV_FRAMEWORK_RUNTIME}/3001)



######################################################################################################################################################################
#                                                                   system demo fmu for OpenXilEnv
######################################################################################################################################################################

# REMARK: The CMAKE created by sdv_dbc_util creates all files including the buildDescription.xml
#
# What cannot be created automatically is the method OpenAPILoad(const std::string& resources) in file model.cpp
# The method must load all required components
# Therfore here the file is copied and overwritten the auto generated file
#
# The required toml files need to be copied to the folder:
# generated/fmu_DemoExampleFMU/DemoExampleFMU/resources

message("Include: FMU DemoExampleFMU")
# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/fmu/resources/fmu_vehicle_devices_basic_services.toml DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_DemoExampleFMU/DemoExampleFMU/resources)
file (COPY ${PROJECT_SOURCE_DIR}/fmu/resources/fmu_complex_service.toml DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_DemoExampleFMU/DemoExampleFMU/resources)

# Overwrite model.cpp with an identical file but loads all components in OpenAPILoad(const std::string& resources)
file (COPY ${PROJECT_SOURCE_DIR}/fmu/model.cpp DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_DemoExampleFMU/DemoExampleFMU)

# Now the project can be build
add_subdirectory(generated/fmu_DemoExampleFMU)
