# Define project (multiple EXEs)
project(CoreLoaderTests VERSION 1.0 LANGUAGES CXX)

# BasicTypes unit test executable
add_executable(UnitTest_CoreLoader
    "core_loader_test.cpp"
    "main.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_CoreLoader GTest::GTest ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} stdc++fs)
else()
    target_link_libraries(UnitTest_CoreLoader GTest::GTest)
endif()

# BasicTypes unit test executable
add_executable(UnitTest_CoreLoader_TestApp
    "test_app.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_CoreLoader_TestApp ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} stdc++fs)
else()
endif()

# Add the BasicTypes unittest
add_test(NAME UnitTest_CoreLoader COMMAND UnitTest_CoreLoader)

# Execute the test
add_custom_command(TARGET UnitTest_CoreLoader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_CoreLoader>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_CoreLoader.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_CoreLoader_TestApp dependency_sdv_components)
add_dependencies(UnitTest_CoreLoader_TestApp install_manifest)
add_dependencies(UnitTest_CoreLoader UnitTest_CoreLoader_TestApp)
