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
project (ComponentTest_AppControl VERSION 1.0 LANGUAGES CXX)

# Data maneger executable
add_executable(ComponentTest_AppControl
    "main.cpp"
    "app_control_test.cpp"
    "app_control_test_no_event_handler.cpp"
    "app_control_test_event_handler.cpp"
    "app_control_test_mgnt_class.cpp"
    )

target_link_libraries(ComponentTest_AppControl ${CMAKE_DL_LIBS} GTest::GTest)

# Add the AppControl component test
add_test(NAME ComponentTest_AppControl COMMAND ComponentTest_AppControl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the installation helper utility to create an installation manifest for the core files.
add_custom_target(ComponentTest_AppControl_install_manifest
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL ComponentTest_Repository --instance2009 "$<TARGET_FILE:ComponentTest_Repository_test_module>" "$<TARGET_FILE:ComponentTest_Repository_ps>" "-I$<TARGET_FILE_DIR:ComponentTest_Repository_test_module>" --overwrite --user_config+TestObject_ComplexHelloService+TestObject_HelloUtility --interface_config+Example_Object+Example_Object_2+TestObject_HelloDevice+TestObject_BasicHelloService+TestObject_SystemHelloService
    DEPENDS ComponentTest_Repository_test_module ComponentTest_Repository_ps
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Execute the test
# Currently disabled due to issues with MINGW causing the application to sporadically crash. A bug ticket has been filed:
# https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/608132
add_custom_command(TARGET ComponentTest_AppControl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_AppControl>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_AppControl.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_AppControl dependency_sdv_components)
add_dependencies(ComponentTest_AppControl ComponentTest_Repository)    # Use the repository components for this test
add_dependencies(ComponentTest_AppControl ComponentTest_AppControl_install_manifest)    # Use the repository components for this test
