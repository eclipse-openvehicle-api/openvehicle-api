# Include cross-compilation toolchain file
include(../../cross-compile-tools.cmake)

# Define project
project(ipc_connect VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(ipc_listener SHARED  "listener.h" "listener.cpp")
if (WIN32)
target_link_libraries(ipc_listener ${CMAKE_THREAD_LIBS_INIT} Rpcrt4.lib)
elseif(UNIX)
target_link_libraries(ipc_listener rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()
target_link_options(ipc_listener PRIVATE)
set_target_properties(ipc_listener PROPERTIES PREFIX "")
set_target_properties(ipc_listener PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(ipc_listener CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} ipc_listener PARENT_SCOPE)

# Define target
add_library(ipc_connect SHARED  "client.h" "client.cpp")
if (WIN32)
target_link_libraries(ipc_connect ${CMAKE_THREAD_LIBS_INIT} Rpcrt4.lib)
elseif(UNIX)
target_link_libraries(ipc_connect rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()
target_link_options(ipc_connect PRIVATE)
set_target_properties(ipc_connect PROPERTIES PREFIX "")
set_target_properties(ipc_connect PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(ipc_connect CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} ipc_connect ipc_listener PARENT_SCOPE)
