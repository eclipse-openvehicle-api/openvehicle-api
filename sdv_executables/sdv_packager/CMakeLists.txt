# Define project
project(sdv_packager VERSION 1.0 LANGUAGES CXX)

# build test application
add_executable(sdv_packager
    "main.cpp"
    "../../sdv_services/core/installation_manifest.h"
    "../../sdv_services/core/installation_manifest.cpp"
    "../../sdv_services/core/installation_composer.h"
    "../../sdv_services/core/installation_composer.cpp"
    "../../sdv_services/core/toml_parser/parser_toml.h"
    "../../sdv_services/core/toml_parser/parser_toml.cpp"
    "../../sdv_services/core/toml_parser/lexer_toml.h"
    "../../sdv_services/core/toml_parser/lexer_toml.cpp"
    "../../sdv_services/core/toml_parser/parser_node_toml.h"
    "../../sdv_services/core/toml_parser/parser_node_toml.cpp"
    "../../sdv_services/core/toml_parser/character_reader_utf_8.h"
    "../../sdv_services/core/toml_parser/character_reader_utf_8.cpp"
    "../../sdv_services/core/toml_parser/exception.h"
    "environment.cpp"
    "environment.h"
    "packager.cpp"
    "packager.h"
    )
if (WIN32)
    target_link_libraries(sdv_packager ${CMAKE_THREAD_LIBS_INIT} Ws2_32 Winmm Rpcrt4.lib)
else()
    target_link_libraries(sdv_packager ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Build dependencies
add_dependencies(sdv_packager core_services)

# Execute the installation packager utility to create an installation manifest for the core files.
# TODO EVE
add_custom_command(
    OUTPUT install_manifest.toml
    COMMAND "$<TARGET_FILE:sdv_packager>" PACK core -O. ./*.sdv --create_manifest_only --verbose
    DEPENDS sdv_packager dependency_sdv_services
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    VERBATIM
)
add_custom_target(
  install_manifest
  DEPENDS install_manifest.toml
)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_packager PARENT_SCOPE)
set(SDV_Executable_List ${SDV_Executable_List} install_manifest PARENT_SCOPE)
