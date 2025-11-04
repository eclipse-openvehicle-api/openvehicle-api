# Include cross-compilation toolchain file
include(../cross-compile-tools.cmake)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(SDVServices VERSION 1.0 LANGUAGES CXX)

# Include export into the include directory path
include_directories(../export)

# Compile the IDL
set(INTERFACE_DIR ${PROJECT_SOURCE_DIR}/../export/interfaces)
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/app.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/app.idl
    COMMENT "Compiling app.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/app.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/config.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/config.idl
    COMMENT "Compiling config.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/config.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/can.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/can.idl
    COMMENT "Compiling can.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/can.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/toml.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/toml.idl
    COMMENT "Compiling toml.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/toml.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/core.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/core.idl
    COMMENT "Compiling core.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/core.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/core_idl.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/core_idl.idl
    COMMENT "Compiling core_idl.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/core_idl.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/core_ps.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/core_ps.idl
    COMMENT "Compiling core_ps.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/core_ps.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/core_types.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/core_types.idl
    COMMENT "Compiling core_types.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/core_types.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/dispatch.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/dispatch.idl
    COMMENT "Compiling dispatch.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/dispatch.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/hw_ident.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/hw_ident.idl
    COMMENT "Compiling hw_ident.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/hw_ident.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/ipc.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/ipc.idl
    COMMENT "Compiling ipc.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/ipc.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/com.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/com.idl
    COMMENT "Compiling com.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/com.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/log.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/log.idl
    COMMENT "Compiling log.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/log.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/mem.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/mem.idl
    COMMENT "Compiling mem.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/mem.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/module.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/module.idl
    COMMENT "Compiling module.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/module.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/process.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/process.idl
    COMMENT "Compiling process.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/process.idl -O${INTERFACE_DIR} --no_ps
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/repository.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/repository.idl
    COMMENT "Compiling repository.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/repository.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_command(
    OUTPUT ${INTERFACE_DIR}/timer.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY ${INTERFACE_DIR}/timer.idl
    COMMENT "Compiling timer.idl"
    COMMAND sdv_idl_compiler ${INTERFACE_DIR}/timer.idl -O${INTERFACE_DIR}
    VERBATIM
    )
add_custom_target(CompileCoreIDL
    DEPENDS
        ${INTERFACE_DIR}/app.h
        ${INTERFACE_DIR}/config.h
        ${INTERFACE_DIR}/can.h
        ${INTERFACE_DIR}/toml.h
        ${INTERFACE_DIR}/core.h
        ${INTERFACE_DIR}/core_idl.h
        ${INTERFACE_DIR}/core_ps.h
        ${INTERFACE_DIR}/core_types.h
        ${INTERFACE_DIR}/dispatch.h
        ${INTERFACE_DIR}/hw_ident.h
        ${INTERFACE_DIR}/ipc.h
        ${INTERFACE_DIR}/com.h
        ${INTERFACE_DIR}/log.h
        ${INTERFACE_DIR}/mem.h
        ${INTERFACE_DIR}/module.h
        ${INTERFACE_DIR}/process.h
        ${INTERFACE_DIR}/repository.h
        ${INTERFACE_DIR}/timer.h
    )

# Add service projects
add_subdirectory(core)
add_subdirectory(data_dispatch_service)
add_subdirectory(proxy_stub)
add_subdirectory(can_communication_socket_can)
add_subdirectory(can_communication_silkit)
add_subdirectory(can_communication_sim)
add_subdirectory(task_timer)
add_subdirectory(ipc_com)
add_subdirectory(ipc_connect)
add_subdirectory(ipc_shared_mem)
add_subdirectory(ipc_sockets)
add_subdirectory(process_control)
add_subdirectory(hardware_ident)
add_subdirectory(manifest_util)

# Appending all services to the service list
set(SDV_Service_List ${SDV_Service_List} PARENT_SCOPE)
