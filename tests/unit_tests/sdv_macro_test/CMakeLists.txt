# Define project
project (UnitTest_SDV_Macro_Test VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_SDV_Macro_Test
    "sdv_macro_test.cpp"
    )

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

target_link_libraries(UnitTest_SDV_Macro_Test
    ${CMAKE_DL_LIBS}
    GTest::GTest
)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_SDV_Macro_Test COMMAND UnitTest_SDV_Macro_Test)

# Execute the test
add_custom_command(TARGET UnitTest_SDV_Macro_Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SDV_Macro_Test>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SDV_Macro_Test.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_SDV_Macro_Test dependency_sdv_components)
