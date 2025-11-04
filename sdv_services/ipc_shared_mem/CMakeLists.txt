# Define project
project(ipc_shared_mem VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(ipc_shared_mem SHARED
    "channel_mgnt.h"
    "channel_mgnt.cpp"
    "connection.h"
    "connection.cpp"
    "shared_mem_buffer_posix.h"
    "shared_mem_buffer_windows.h"
    "in_process_mem_buffer.h"
    "mem_buffer_accessor.h"
    "mem_buffer_accessor.cpp"
    "watchdog.h"
    "watchdog.cpp"
    )
if(UNIX)
target_link_libraries(ipc_shared_mem rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
else()
target_link_libraries(ipc_shared_mem ${CMAKE_THREAD_LIBS_INIT})
endif()
target_link_options(ipc_shared_mem PRIVATE)
target_include_directories(ipc_shared_mem PRIVATE ./include/)
set_target_properties(ipc_shared_mem PROPERTIES PREFIX "")
set_target_properties(ipc_shared_mem PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(ipc_shared_mem CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} ipc_shared_mem PARENT_SCOPE)
