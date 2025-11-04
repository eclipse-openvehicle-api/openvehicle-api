# Define project
project (UnitTest_SDV_Control VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_component.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY test_component.idl
    COMMENT "Build test_component.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_component.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )

set_source_files_properties(preinstalled_test_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_app.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_basic_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_complex_service1.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_complex_service2.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_device.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_system_service.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_utility1.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_utility2.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)

# Add the prxy/stub library
add_library(UnitTest_SDV_Control_ps SHARED "test_component_ps.cpp")
set_target_properties(UnitTest_SDV_Control_ps PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_ps PROPERTIES SUFFIX ".sdv")

# Build module for test
add_library(UnitTest_SDV_Control_PreInstalled_Service SHARED preinstalled_test_service.cpp)
target_link_libraries(UnitTest_SDV_Control_PreInstalled_Service ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_PreInstalled_Service PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_PreInstalled_Service PROPERTIES SUFFIX ".sdv")

# Build module for app test
add_executable(UnitTest_SDV_Control_Test_App test_app.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_App ${CMAKE_DL_LIBS} GTest::GTest)

# Build module for basic service test
add_library(UnitTest_SDV_Control_Test_Basic_Service SHARED test_basic_service.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Basic_Service ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Basic_Service PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Basic_Service PROPERTIES SUFFIX ".sdv")

# Build module for complex service #1 test
add_library(UnitTest_SDV_Control_Test_Complex_Service1 SHARED test_complex_service1.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Complex_Service1 ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Complex_Service1 PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Complex_Service1 PROPERTIES SUFFIX ".sdv")

# Build module for complex service #2 test
add_library(UnitTest_SDV_Control_Test_Complex_Service2 SHARED test_complex_service2.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Complex_Service2 ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Complex_Service2 PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Complex_Service2 PROPERTIES SUFFIX ".sdv")

# Build module for device test
add_library(UnitTest_SDV_Control_Test_Device SHARED test_device.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Device ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Device PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Device PROPERTIES SUFFIX ".sdv")

# Build module for system service test
add_library(UnitTest_SDV_Control_Test_System_Service SHARED test_system_service.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_System_Service ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_System_Service PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_System_Service PROPERTIES SUFFIX ".sdv")

# Build module for utility #1 test
add_library(UnitTest_SDV_Control_Test_Utility1 SHARED test_utility1.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Utility1 ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Utility1 PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Utility1 PROPERTIES SUFFIX ".sdv")

# Build module for utility #2 test
add_library(UnitTest_SDV_Control_Test_Utility2 SHARED test_utility2.cpp)
target_link_libraries(UnitTest_SDV_Control_Test_Utility2 ${CMAKE_DL_LIBS} GTest::GTest)
set_target_properties(UnitTest_SDV_Control_Test_Utility2 PROPERTIES PREFIX "")
set_target_properties(UnitTest_SDV_Control_Test_Utility2 PROPERTIES SUFFIX ".sdv")

# Execute the installation helper utility to create an installation manifest for the core files.
add_custom_target(UnitTest_SDV_Control_install_manifest
# TODO EVE
#    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL ComponentTest_Repository --instance2007 "$<TARGET_FILE:UnitTest_SDV_Control_PreInstalled_Service>" "$<TARGET_FILE:UnitTest_SDV_Control_ps>" "-I$<TARGET_FILE_DIR:UnitTest_SDV_Control_PreInstalled_Service>" --settings --create_configtest.toml --exclude_config_class"TestObject_CreateChain" --exclude_config_class"TestObject_CreateChainLock" --exclude_config_class"TestObject_CreateChainLockThread" --exclude_config_class"TestObject_CreateDuringShutdown" --exclude_config_class"TestObject_IObjectControlFail"
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL ComponentTest_SDV_Control --instance2007 "$<TARGET_FILE:UnitTest_SDV_Control_PreInstalled_Service>" "$<TARGET_FILE:UnitTest_SDV_Control_ps>" "-I$<TARGET_FILE_DIR:UnitTest_SDV_Control_PreInstalled_Service>"  --overwrite --user_config
    DEPENDS UnitTest_SDV_Control_PreInstalled_Service UnitTest_SDV_Control_ps
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Compile the source code
add_executable(UnitTest_SDV_Control
    "main.cpp"
    "includes.h"
    "startup_shutdown.cpp"
    "list_info.cpp" "start_stop_service.cpp" "preinstalled_test_service.cpp" "test_component_ps.cpp" "install_update_uninstall.cpp")
target_link_libraries(UnitTest_SDV_Control ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_SDV_Control COMMAND UnitTest_SDV_Control)

# Execute the test
# Currently disabled due to issues causing the application to regularly crash. A bug ticket has been filed for MINGW:
# https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/608132
# and for ARM:
# https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/686503
#add_custom_command(TARGET UnitTest_SDV_Control POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SDV_Control>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SDV_Control.xml
#    VERBATIM
#)

# Build dependencies
add_dependencies(UnitTest_SDV_Control_PreInstalled_Service install_manifest)
add_dependencies(UnitTest_SDV_Control_PreInstalled_Service dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_ps dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_PreInstalled_Service)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_App)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Basic_Service)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Complex_Service1)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Complex_Service2)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Device)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_System_Service)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Utility1)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_Test_Utility2)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_ps)
add_dependencies(UnitTest_SDV_Control UnitTest_SDV_Control_install_manifest)
add_dependencies(UnitTest_SDV_Control_Test_App dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Basic_Service dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Complex_Service1 dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Complex_Service2 dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Device dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_System_Service dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Utility1 dependency_sdv_components)
add_dependencies(UnitTest_SDV_Control_Test_Utility2 dependency_sdv_components)
