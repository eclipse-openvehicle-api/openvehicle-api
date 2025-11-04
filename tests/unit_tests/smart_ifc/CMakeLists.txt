# Define project
project (UnitTest_Smart_Interface VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/smart_ifc.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/smart_ifc.idl
    COMMENT "Build smart_ifc.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/smart_ifc.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated" --no_ps
    VERBATIM
    )
set_source_files_properties(smart_ifc.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/smart_ifc.h)

# Compile the source code
add_executable(UnitTest_Smart_Interface
    "main.cpp"
    "smart_ifc.cpp" "ifc_map.cpp" )
target_link_libraries(UnitTest_Smart_Interface ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_Smart_Interface COMMAND UnitTest_Smart_Interface)

# Execute the test
add_custom_command(TARGET UnitTest_Smart_Interface POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_Smart_Interface>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_Smart_Interface.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_Smart_Interface CompileCoreIDL)
