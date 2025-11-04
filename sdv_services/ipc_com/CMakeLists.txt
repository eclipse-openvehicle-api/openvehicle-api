# Include cross-compilation toolchain file
include(../../cross-compile-tools.cmake)

# Define project
project(service_component_isolation VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(ipc_com SHARED
     "com_ctrl.h"
     "com_ctrl.cpp"
     "com_channel.h"
     "com_channel.cpp"
     "marshall_object.h"
     "marshall_object.cpp"

     #"scheduler.cpp"
     )
if(UNIX)
target_link_libraries(ipc_com rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
else()
target_link_libraries(ipc_com ${CMAKE_THREAD_LIBS_INIT})
target_link_options(ipc_com PRIVATE)
endif()
target_include_directories(ipc_com PRIVATE ./include)
set_target_properties(ipc_com PROPERTIES PREFIX "")
set_target_properties(ipc_com PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(ipc_com CompileCoreIDL)
add_dependencies(ipc_com core_ps)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} ipc_com PARENT_SCOPE)
