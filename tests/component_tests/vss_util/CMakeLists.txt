# Define project
project (VSSUtilTests VERSION 1.0 LANGUAGES CXX)

file (COPY ${PROJECT_SOURCE_DIR}/vss_multiple_names.csv DESTINATION ${CMAKE_BINARY_DIR}/bin/)
file (COPY ${PROJECT_SOURCE_DIR}/vss_unique_names.csv DESTINATION ${CMAKE_BINARY_DIR}/bin/)

# VSS util component test executable
add_executable(ComponentTest_VSSUtil vss_util_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(ComponentTest_VSSUtil ${CMAKE_THREAD_LIBS_INIT} GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentTest_VSSUtil Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_VSSUtil ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentTest_VSSUtil GTest::GTest Rpcrt4.lib)
endif()

# Add the VSS utility component test
add_test(NAME ComponentTest_VSSUtil COMMAND ComponentTest_VSSUtil WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_VSSUtil POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_VSSUtil>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_VSSUtil.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_VSSUtil  sdv_vss_util) 


