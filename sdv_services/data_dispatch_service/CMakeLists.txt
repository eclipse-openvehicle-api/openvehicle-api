# Define project
project(sdv_service_datadispatchservice VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(data_dispatch_service SHARED
    "dispatchservice.cpp"
    "dispatchservice.h"
    "transaction.cpp"
    "transaction.h"
    "signal.cpp"
    "signal.h" "trigger.h" "trigger.cpp")
target_link_libraries(data_dispatch_service ${CMAKE_THREAD_LIBS_INIT})
target_link_options(data_dispatch_service PRIVATE)
target_include_directories(data_dispatch_service PRIVATE ./include/)
set_target_properties(data_dispatch_service PROPERTIES PREFIX "")
set_target_properties(data_dispatch_service PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(data_dispatch_service CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} data_dispatch_service PARENT_SCOPE)
