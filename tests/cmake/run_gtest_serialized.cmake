cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED TEST_EXE)
    message(FATAL_ERROR "TEST_EXE is required")
endif()

if(NOT DEFINED TEST_XML)
    message(FATAL_ERROR "TEST_XML is required")
endif()

if(NOT DEFINED LOCK_FILE)
    message(FATAL_ERROR "LOCK_FILE is required")
endif()

if(NOT DEFINED TEST_EXECUTION_MODE)
    set(TEST_EXECUTION_MODE "CMake")
endif()

if(NOT DEFINED LOCK_TIMEOUT_SEC)
    set(LOCK_TIMEOUT_SEC 1200)
endif()

file(LOCK "${LOCK_FILE}" TIMEOUT ${LOCK_TIMEOUT_SEC} RESULT_VARIABLE LOCK_RESULT)
if(NOT LOCK_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to acquire test lock '${LOCK_FILE}': ${LOCK_RESULT}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "TEST_EXECUTION_MODE=${TEST_EXECUTION_MODE}"
            "${TEST_EXE}"
            "--gtest_output=xml:${TEST_XML}"
    RESULT_VARIABLE TEST_RESULT
)

file(LOCK "${LOCK_FILE}" RELEASE)

if(NOT TEST_RESULT EQUAL 0)
    message(FATAL_ERROR "Test execution failed with code ${TEST_RESULT}: ${TEST_EXE}")
endif()
