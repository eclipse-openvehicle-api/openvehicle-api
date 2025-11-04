# Build for linux only
if(UNIX)

    # Define project
    project(can_com_sockets VERSION 1.0 LANGUAGES CXX)

    # Copy the config files
    file (COPY ${PROJECT_SOURCE_DIR}/config/test_can_communication_sockets.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)

    # Define target
    add_library(can_com_sockets SHARED "can_com_sockets.h" "can_com_sockets.cpp")
    target_link_libraries(can_com_sockets ${CMAKE_THREAD_LIBS_INIT})
    target_link_options(can_com_sockets PRIVATE)

    set_target_properties(can_com_sockets PROPERTIES PREFIX "")
    set_target_properties(can_com_sockets PROPERTIES SUFFIX ".sdv")

    # Build dependencies
    add_dependencies(can_com_sockets CompileCoreIDL)

    # Appending the service in the service list
    set(SDV_Service_List ${SDV_Service_List} can_com_sockets PARENT_SCOPE)
endif()
