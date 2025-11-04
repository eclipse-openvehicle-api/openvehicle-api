# Define project
project (UnitTest_IPC_Connect VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_IPC_Connect
    "main.cpp"

    "ipc_connect.cpp"
    )
target_link_libraries(UnitTest_IPC_Connect ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_IPC_Connect COMMAND UnitTest_IPC_Connect)

# Execute the test
add_custom_command(TARGET UnitTest_IPC_Connect POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_IPC_Connect>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_IPC_Connect.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_IPC_Connect dependency_sdv_components)
