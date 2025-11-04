# Define project
project(ConfigTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(ComponentTest_Config config_tests.cpp)

target_link_libraries(ComponentTest_Config ${CMAKE_DL_LIBS} GTest::GTest)

# Add the test
add_test(NAME ComponentTest_Config COMMAND ComponentTest_Config WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_Config POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_Config>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_Config.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_Config dependency_sdv_components)
