# Define project
project (UnitTest_App_Connect VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_App_Connect
    "main.cpp"
     "app_connect_local.cpp")
target_link_libraries(UnitTest_App_Connect ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_App_Connect COMMAND UnitTest_App_Connect)

# Execute the test
add_custom_command(TARGET UnitTest_App_Connect POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_App_Connect>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_App_Connect.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_App_Connect dependency_sdv_components)
