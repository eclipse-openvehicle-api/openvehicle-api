project(AutoHeadlight)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Library symbols are hidden by default
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
message("Create interface code for devices and basic services of auto headlight example.")
execute_process(COMMAND "${SDV_VSS_UTIL}" "${PROJECT_SOURCE_DIR}/vss_autoheadlight_example.csv" "-O${PROJECT_SOURCE_DIR}/generated/" --prefixheadlight --version1.0.0.1 --enable_components)

# Execute the IDL compiler for the VSS interfaces to digest interface code.
message("Compiling vss_vehiclepositioncurrentlatitude_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclepositioncurrentlatitude_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclepositioncurrentlatitude_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclepositioncurrentlatitude_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nameautoheadlight_proxystub)
message("Compiling vss_vehiclepositioncurrentlongitude_vd_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclepositioncurrentlongitude_vd_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclepositioncurrentlongitude_bs_rx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclepositioncurrentlongitude_bs_rx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nameautoheadlight_proxystub)
message("Compiling vss_vehiclebodyheadlight_vd_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodylightfrontlowbeam_vd_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --no_ps)
message("Compiling vss_vehiclebodyheadlight_bs_tx.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/generated/vss_files/vss_vehiclebodylightfrontlowbeam_bs_tx.idl" "-O${PROJECT_SOURCE_DIR}/generated/vss_files/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Igenerated/vss_files/ --ps_lib_nameautoheadlight_proxystub)

# Execute sdv_dbc_util to create FMU code.
message("Create functional mockup unit (FMU) of auto headlight example.")
execute_process(COMMAND ${SDV_DBC_UTIL} "${PROJECT_SOURCE_DIR}/datalink_autoheadlight_example.dbc" "-O${PROJECT_SOURCE_DIR}/generated/" --nodesheadlight --version1.0.0.1 --moduleAutoHeadlightFMU --dl_lib_nameautoheadlight_can_dl_example)

# Execute the IDL compiler for the complex service to digest interface code.
message("Compiling autoheadlight_cs.idl")
execute_process(COMMAND "${SDV_IDL_COMPILER}" "${PROJECT_SOURCE_DIR}/autoheadlight_service/autoheadlight_cs_ifc.idl" "-O${PROJECT_SOURCE_DIR}/generated/autoheadlight_service/" "-I${SDV_FRAMEWORK_DEV_INCLUDE}" -Iexample_service/ --ps_lib_nameautoheadlight_service_proxystub)

#######################################################################################################################################################################
##                                                                   vehicle devices and basic services
#######################################################################################################################################################################

# REMARK: Proxy/stub and vehicle device and basic service code was generated during the configuration phase of CMake. Following
# below are the build steps to build the components that were generated.

message("Include: auto headlight proxy/stub for vehicle devices and basic services")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/vss_files)
add_subdirectory(generated/vss_files/ps)


add_subdirectory(generated/vss_files/vd_currentlatitude)
add_subdirectory(generated/vss_files/vd_currentlongitude)
add_subdirectory(generated/vss_files/vd_headlightlowbeam)

add_subdirectory(generated/vss_files/bs_currentlatitude)
add_subdirectory(generated/vss_files/bs_currentlongitude)
add_subdirectory(generated/vss_files/bs_headlightlowbeam)


######################################################################################################################################################################
#                                                                     complex service
######################################################################################################################################################################

message("Include: proxy/stub for complex autoheadlight service")
include_directories(${CMAKE_CURRENT_LIST_DIR}/generated/autoheadlight_service)
add_subdirectory(generated/autoheadlight_service/ps)

message("Include: Auto Headlight complex service")
add_library(autoheadlight_complex_service SHARED
    "autoheadlight_service/autoheadlight_cs.h"
    "autoheadlight_service/autoheadlight_cs.cpp"
    )
set_target_properties(autoheadlight_complex_service PROPERTIES OUTPUT_NAME "autoheadlight_service")
set_target_properties(autoheadlight_complex_service PROPERTIES PREFIX "")
set_target_properties(autoheadlight_complex_service PROPERTIES SUFFIX ".sdv")

######################################################################################################################################################################
#                                                                   autoheadlight application
######################################################################################################################################################################

# Define the executable
add_executable(auto_headlight_app 
    autoheadlight_app/autoheadlight_app.cpp
    autoheadlight_app/autoheadlight_simulate.h
    autoheadlight_app/autoheadlight_simulate.cpp
    autoheadlight_app/autoheadlight_console.h
    autoheadlight_app/autoheadlight_console.cpp
    autoheadlight_app/signal_names.h
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(auto_headlight_app Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(auto_headlight_app ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(auto_headlight_app Rpcrt4.lib)
endif()

# # Copy the config files
# file (COPY ${PROJECT_SOURCE_DIR}/config/CanSocketExample.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_link_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/task_timer_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/autoheadlight_vd_bs.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/autoheadlight_cs.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)



######################################################################################################################################################################
#                                                                   auto headlight fmu for OpenXilEnv
######################################################################################################################################################################

# REMARK: The CMAKE created by sdv_dbc_util creates all files including the buildDescription.xml
#
# What cannot be created automatically is the method OpenAPILoad(const std::string& resources) in file model.cpp
# The method must load all required components
# Therfore here the file is copied and overwritten the auto generated file
#
# The required toml files need to be copied to the folder:
# generated/fmu_DemoExampleFMU/DemoExampleFMU/resources

message("Include: FMU AutoHeadlightFMU")
# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/fmu_files/resources/fmu_autoheadlight_vd_bs.toml DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_AutoHeadlightFMU/AutoHeadlightFMU/resources)
file (COPY ${PROJECT_SOURCE_DIR}/fmu_files/resources/fmu_autoheadlight_cs.toml DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_AutoHeadlightFMU/AutoHeadlightFMU/resources)

# Overwrite model.cpp with an identical file but loads all components in OpenAPILoad(const std::string& resources)
file (COPY ${PROJECT_SOURCE_DIR}/fmu_files/model.cpp DESTINATION ${PROJECT_SOURCE_DIR}/generated/fmu_AutoHeadlightFMU/AutoHeadlightFMU)

# Now the project can be build
add_subdirectory(generated/fmu_AutoHeadlightFMU)
