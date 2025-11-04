# Define project
project (DbcUtilTests VERSION 1.0 LANGUAGES CXX)

# Find the DBC utility
# REMARKS The utility can only be found after it has build. Having both the utility and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(DBCUTIL NAMES "sdv_dbc_util" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (DBCUTIL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_dbc_util")
message("Use DBC utility: ${DBCUTIL}")

# Generate the datalink
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/dbc_struct_test/can_dl/datalink.h ${PROJECT_SOURCE_DIR}/generated/dbc_struct_test/can_dl/datalink.cpp
    DEPENDS sdv_dbc_util
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/dbc_struct_test.dbc
    COMMENT "Build dbc_struct_test.dbc"
    COMMAND ${DBCUTIL} ${PROJECT_SOURCE_DIR}/dbc_struct_test.dbc -O${PROJECT_SOURCE_DIR}/generated/dbc_struct_test/ --nodesTestECU_Tx,TestECU_Rx
    VERBATIM
    )

add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/example_can_test/can_dl/datalink.h ${PROJECT_SOURCE_DIR}/generated/example_can_test/can_dl/datalink.cpp
    DEPENDS sdv_dbc_util
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/example_can.dbc
    COMMENT "Build example_can.dbc"
    COMMAND ${DBCUTIL} ${PROJECT_SOURCE_DIR}/example_can.dbc -O${PROJECT_SOURCE_DIR}/generated/example_can_test/ --nodesDSC_ASC_ABS
    VERBATIM
    )

add_custom_target(PrecompileDBCUtilTest
    DEPENDS
        ${PROJECT_SOURCE_DIR}/generated/dbc_struct_test/can_dl/datalink.cpp
        ${PROJECT_SOURCE_DIR}/generated/example_can_test/can_dl/datalink.cpp
        ${PROJECT_SOURCE_DIR}/generated/dbc_struct_test/can_dl/datalink.h
        ${PROJECT_SOURCE_DIR}/generated/example_can_test/can_dl/datalink.h
    )

# DBC util component test executable
add_executable(ComponentTest_DbcUtil dbc_util_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(ComponentTest_DbcUtil ${CMAKE_THREAD_LIBS_INIT} GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentTest_DbcUtil Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_DbcUtil ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentTest_DbcUtil GTest::GTest Rpcrt4.lib)
endif()

# Add the DBC utility component test
add_test(NAME ComponentTest_DbcUtil COMMAND ComponentTest_DbcUtil WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# copy the folder 'test_dbc_util_config.toml' to output folder 'config'
file (COPY ${PROJECT_SOURCE_DIR}/test_dbc_util_config_asc.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/test_dbc_util_config_rx.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/test_dbc_util_config_tx.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)

# Execute the test
add_custom_command(TARGET ComponentTest_DbcUtil POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_DbcUtil>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_DbcUtil.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_DbcUtil sdv_dbc_util)
add_dependencies(ComponentTest_DbcUtil PrecompileDBCUtilTest)
add_dependencies(ComponentTest_DbcUtil dependency_sdv_components)
add_dependencies(ComponentTest_DbcUtil task_timer)
add_dependencies(ComponentTest_DbcUtil data_dispatch_service)
add_dependencies(ComponentTest_DbcUtil can_com_sim)

