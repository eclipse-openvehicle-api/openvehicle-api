# Define project
project (VSSComponentsFormulaTests VERSION 1.0 LANGUAGES CXX)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include directory to the core framework
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})
file (COPY ${PROJECT_SOURCE_DIR}/config/rxformulatypeint32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxformulatypestring.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)

# VSS util component formula test executable
add_executable(ComponentFormulaTest_VSSComponents load_components_test.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../vss_util/generated/with_formula/vss_files/)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(ComponentFormulaTest_VSSComponents ${CMAKE_THREAD_LIBS_INIT} GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentFormulaTest_VSSComponents Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentFormulaTest_VSSComponents ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentFormulaTest_VSSComponents GTest::GTest Rpcrt4.lib)
endif()

# Add the VSS utility component formula test
add_test(NAME ComponentFormulaTest_VSSComponents COMMAND ComponentFormulaTest_VSSComponents WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentFormulaTest_VSSComponents POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentFormulaTest_VSSComponents>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentFormulaTest_VSSComponents.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentFormulaTest_VSSComponents 
        testcase5_vd_rxclassforint32_rx testcase5_vd_rxclassforint32_rx
        testcase5_vd_rxclassforstring_rx testcase5_vd_rxclassforstring_rx)  
     











