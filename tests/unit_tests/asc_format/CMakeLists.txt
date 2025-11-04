# Define project
project (UnitTest_ASC_Format VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_ASC_Format
    "asc_reader_test.cpp"
    "main.cpp"
    "asc_writer_test.cpp")
target_link_libraries(UnitTest_ASC_Format ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_ASC_Format COMMAND UnitTest_ASC_Format)

# Execute the test
add_custom_command(TARGET UnitTest_ASC_Format POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_ASC_Format>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_ASC_Format.xml
    VERBATIM
)

# copy the ASC test file to binary folder
file (COPY ${PROJECT_SOURCE_DIR}/asc_reader_timing_test.asc DESTINATION ${CMAKE_BINARY_DIR}/tests/bin)
file (COPY ${PROJECT_SOURCE_DIR}/asc_reader_test.asc DESTINATION ${CMAKE_BINARY_DIR}/tests/bin)
file (COPY ${PROJECT_SOURCE_DIR}/asc_reader_ext_id_test.asc DESTINATION ${CMAKE_BINARY_DIR}/tests/bin)
file (COPY ${PROJECT_SOURCE_DIR}/asc_reader_canfd_test.asc DESTINATION ${CMAKE_BINARY_DIR}/tests/bin)

# Build dependencies
add_dependencies(UnitTest_ASC_Format dependency_sdv_components)
