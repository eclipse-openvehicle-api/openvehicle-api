# Define project
project (sdv_idl_compiler VERSION 1.0 LANGUAGES CXX)

# Get xxHash from github
include(FetchContent)
FetchContent_Declare(
  xxhash
  GIT_REPOSITORY                https://github.com/Cyan4973/xxHash.git
  GIT_TAG                       v0.8.3
)
FetchContent_MakeAvailable(xxhash)

# Add include directories
include_directories(../export ${xxhash_SOURCE_DIR})

# Define the executable
add_executable(sdv_idl_compiler
    main.cpp
    exception.cpp
    environment.cpp
    logger.cpp
    token.cpp
    tokenlist.cpp
    codepos.cpp
    lexer.cpp
    parser.cpp
    parsecontext.cpp
    preproc.cpp
    environment.cpp
    source.cpp
    macro.cpp
    constvariant.cpp
    entities/entity_base.cpp
    entities/root_entity.cpp
    entities/module_entity.cpp
    entities/declaration_entity.cpp
    entities/typedef_entity.cpp
    entities/variable_entity.cpp
    entities/entity_value.cpp
    entities/definition_entity.cpp
    entities/struct_entity.cpp
    entities/interface_entity.cpp
    entities/attribute_entity.cpp
    entities/operation_entity.cpp
    entities/parameter_entity.cpp
    entities/exception_entity.cpp
    entities/union_entity.cpp
    entities/enum_entity.cpp
    entities/hash_calc.cpp
    entities/meta_entity.cpp
    generator/definition_generator.cpp
    generator/proxy_generator.cpp
    generator/stub_generator.cpp
    generator/context.cpp
    "generator/ps_class_generator_base.cpp"
    "generator/serdes_generator.h"
    "generator/serdes_generator.cpp"
    "generator/cmake_generator.cpp"
    "generator/ps_cpp_generator.h"
    "generator/ps_cpp_generator.cpp"   "generator/definition_generator_base.h")

if (UNIX)
    target_link_libraries(sdv_idl_compiler ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
else()
    target_link_libraries(sdv_idl_compiler ${CMAKE_DL_LIBS})
endif (UNIX)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_idl_compiler PARENT_SCOPE)
