# Define project
project(sdv_services_core VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(core_services SHARED
    "memory.cpp"
    "memory.h"
    "sdv_core.h"
    "sdv_core.cpp"
    "module_control.h"
    "module_control.cpp"
    "app_control.h"
    "app_control.cpp"
    "repository.h"
    "repository.cpp"
    "toml_parser/character_reader_utf_8.h"
    "toml_parser/lexer_toml.h"
    "toml_parser/parser_toml.h"
    "toml_parser/parser_toml.cpp"
    "toml_parser/lexer_toml.cpp"
    "toml_parser/character_reader_utf_8.cpp"
    "toml_parser/parser_node_toml.h"
    "toml_parser/parser_node_toml.cpp"
    "toml_parser/exception.h"
    "module.cpp"
    "app_config.h"
    "app_config.cpp"
    "logger_control.h"
    "logger_control.cpp"
    "logger.h"
    "logger.cpp"
    "log_csv_writer.h"
    "log_csv_writer.cpp"
    "object_lifetime_control.h"
    "object_lifetime_control.cpp"
    "toml_parser_util.h"
    "installation_manifest.h"
    "installation_manifest.cpp"
    "local_shutdown_request.h"
    "iso_monitor.h"
    "iso_monitor.cpp"
    "installation_composer.h"
    "installation_composer.cpp"

     "toml_parser/lexer_toml_token.h" "toml_parser/lexer_toml_token.cpp" "toml_parser/miscellaneous.h" "toml_parser/miscellaneous.cpp")

# Link target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(core_services PRIVATE)
    target_link_libraries(core_services ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(core_services Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(core_services ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(core_services Rpcrt4.lib)
endif()

set_target_properties(core_services PROPERTIES PREFIX "")
set_target_properties(core_services PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(core_services CompileCoreIDL)
add_dependencies(core_services hardware_ident)
add_dependencies(core_services ipc_shared_mem)
add_dependencies(core_services process_control)
add_dependencies(core_services core_ps)
add_dependencies(core_services ipc_com)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} core_services PARENT_SCOPE)
