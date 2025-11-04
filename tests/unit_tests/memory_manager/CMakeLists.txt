# Define project
project(MemoryManagerTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(UnitTest_MemoryManager memmgr_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_MemoryManager GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_MemoryManager Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_MemoryManager ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_MemoryManager GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_MemoryManager COMMAND UnitTest_MemoryManager WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_MemoryManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_MemoryManager>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_MemoryManager.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_MemoryManager dependency_sdv_components)
