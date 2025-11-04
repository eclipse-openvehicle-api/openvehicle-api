# Define project
project (ComponentTest_CoreLibrary VERSION 1.0 LANGUAGES CXX)

# Data maneger executable
add_executable(ComponentTest_CoreLibrary
    "main.cpp"
    "app_control_test.cpp"
    "app_control_test_no_event_handler.cpp"
    "app_control_test_event_handler.cpp"
    "app_control_test_mgnt_class.cpp"
     "app_config.cpp")

target_link_libraries(ComponentTest_CoreLibrary ${CMAKE_DL_LIBS} GTest::GTest)

# Add the Data Dispatch Service unittest
add_test(NAME ComponentTest_CoreLibrary COMMAND ComponentTest_CoreLibrary WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
# Currently disabled due to issues with MINGW causing the application to sporadically crash. A bug ticket has been filed:
# https://dev.azure.com/SW4ZF/AZP-074_DivDI_SofDCarResearch/_workitems/edit/608132
#add_custom_command(TARGET ComponentTest_CoreLibrary POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_CoreLibrary>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_CoreLibrary.xml
#    VERBATIM
#)


# Build dependencies
add_dependencies(ComponentTest_CoreLibrary install_manifest)
add_dependencies(ComponentTest_CoreLibrary dependency_sdv_components)
add_dependencies(ComponentTest_CoreLibrary data_dispatch_service)
add_dependencies(ComponentTest_CoreLibrary ComponentTest_Repository)    # Use the repository components for this test
