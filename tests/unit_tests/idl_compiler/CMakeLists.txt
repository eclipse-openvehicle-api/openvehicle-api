# Define project
project (UnitTest_IDLCompiler VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/linked.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/linked.idl
    COMMENT "Build linked.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/linked.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/test.idl
    COMMENT "Build test.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/array.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/array.idl
    COMMENT "Build array.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/array.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/union.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/union.idl
    COMMENT "Build union.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/union.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/interfaces.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/interfaces.idl
    COMMENT "Build interfaces.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/interfaces.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_ifc_id1.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/test_ifc_id1.idl
    COMMENT "Build test_ifc_id1.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_ifc_id1.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated" --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_ifc_id2.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY  ${PROJECT_SOURCE_DIR}/test_ifc_id2.idl
    COMMENT "Build test_ifc_id2.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_ifc_id2.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated" --no_ps
    VERBATIM
    )
set_source_files_properties(generator_interface_id_test.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test.h)
set_source_files_properties(generator_ps_test.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test.h)
set_source_files_properties(generator_ps_interface_test.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/interfaces.h)

# Get xxHash from github
include(FetchContent)
FetchContent_Declare(
    xxhash
    GIT_REPOSITORY                https://github.com/Cyan4973/xxHash.git
    GIT_TAG                       v0.8.1
)
FetchContent_MakeAvailable(xxhash)

# Add include directories
include_directories(../../../export ${xxhash_SOURCE_DIR})

# Compile the source code
add_executable(UnitTest_IDLCompiler
    "generated/linked.h"
    "generated/test.h"
    "generated/array.h"
    "generated/test_ifc_id1.h"
    "generated/test_ifc_id2.h"
    "generated/union.h"
    "main.cpp"
    "lexer_test.cpp"
    "parser_test.cpp"
    "preproc_test.cpp"
    "source_test.cpp"
    "code_to_text_test.cpp"
    "parser_module_test.cpp"
    "constvariant_test.cpp"
    "parser_const_assignment_test.cpp"
    "parser_var_assignment_test.cpp"
    "parser_interface_test.cpp"
    "parser_struct_test.cpp"
    "parser_typedef_test.cpp"
    "parser_exception_test.cpp"
    "parser_union_test.cpp"
    "parser_enum_test.cpp"
    "commandline_test.cpp"
    "parser_meta_test.cpp"
    "parser_meta_test.cpp"
    "generator_interface_id_test.cpp"
    "generator_ps_test.cpp"
    "generator_test.cpp"
    "generator_test.h"
    "generator_ps_interface_test.cpp"
    "compiler_test.cpp"


    "idl_interface_id_compatibility.cpp"
    "parser_ext_test.cpp"
    )
target_link_libraries(UnitTest_IDLCompiler ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_IDLCompiler COMMAND UnitTest_IDLCompiler)

# Execute the test
add_custom_command(TARGET UnitTest_IDLCompiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_IDLCompiler>" "--gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_IDLCompiler.xml"
    VERBATIM
)

# The unit-test project depends on a proper compilation of the compiler before
add_dependencies(UnitTest_IDLCompiler dependency_sdv_components)
