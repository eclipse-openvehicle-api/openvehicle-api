# Define project
project (UnitTest_IPC_Communication VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_ifc.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/test_ifc.idl
    COMMENT "Build /test_ifc.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_ifc.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
set_source_files_properties(ipc_com_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_ifc.h)
set_source_files_properties(ipc_com.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_ifc.h)

# Add the dynamic library
add_library(UnitTest_IPC_Communication_ps SHARED
    "ipc_com_ps.cpp"
    )

# Set extension to .sdv
set_target_properties(UnitTest_IPC_Communication_ps PROPERTIES PREFIX "")
set_target_properties(UnitTest_IPC_Communication_ps PROPERTIES SUFFIX ".sdv")

# Compile the source code
add_executable(UnitTest_IPC_Communication
    "main.cpp"

    "ipc_com.cpp"
    )
target_link_libraries(UnitTest_IPC_Communication ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_IPC_Communication COMMAND UnitTest_IPC_Communication)

# Execute the test
add_custom_command(TARGET UnitTest_IPC_Communication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_IPC_Communication>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_IPC_Communication.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_IPC_Communication_ps dependency_sdv_components)
add_dependencies(UnitTest_IPC_Communication UnitTest_IPC_Communication_ps)
