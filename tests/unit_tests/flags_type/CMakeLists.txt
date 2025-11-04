# Define project
project(FlagsTypeTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(UnitTest_FlagsType flags_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_FlagsType GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_FlagsType Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_FlagsType ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_FlagsType GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_FlagsType COMMAND UnitTest_FlagsType WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_FlagsType POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_FlagsType>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_FlagsType.xml
    VERBATIM
)

# The unit-test project depends on a proper compilation of sdv components before
add_dependencies(UnitTest_FlagsType dependency_sdv_components)
