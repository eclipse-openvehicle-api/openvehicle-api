# Define project
project(manifest_util VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(manifest_util SHARED
    "manifest_util.h"
    "manifest_util.cpp")
if (WIN32)
target_link_libraries(manifest_util ${CMAKE_THREAD_LIBS_INIT} Rpcrt4.lib)
elseif(UNIX)
target_link_libraries(manifest_util rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()
target_link_options(manifest_util PRIVATE)
target_include_directories(manifest_util PRIVATE ./include/)
set_target_properties(manifest_util PROPERTIES PREFIX "")
set_target_properties(manifest_util PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(manifest_util CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} manifest_util PARENT_SCOPE)
