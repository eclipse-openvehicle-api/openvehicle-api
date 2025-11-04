if(WIN32)
# Define project
project(ipc_sockets VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(ipc_sockets SHARED
    "channel_mgnt.h"
    "channel_mgnt.cpp"
    "connection.h"
    "connection.cpp")
target_link_libraries(ipc_sockets ${CMAKE_THREAD_LIBS_INIT} Ws2_32.lib)
target_link_options(ipc_sockets PRIVATE)
target_include_directories(ipc_sockets PRIVATE ./include/)

set_target_properties(ipc_sockets PROPERTIES PREFIX "")
set_target_properties(ipc_sockets PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(ipc_sockets CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} ipc_sockets PARENT_SCOPE)

endif()
