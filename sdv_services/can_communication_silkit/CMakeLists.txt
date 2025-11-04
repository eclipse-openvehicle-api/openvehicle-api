# Only build on MSVC/Windows or 64-bit Linux (not ARM)
if((WIN32 AND NOT MSVC) OR (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64"))

    # Define project
    project(can_com_silkit VERSION 1.0 LANGUAGES CXX)

    # Platform-specific SilKit flavor and library settings
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(SILKIT_FLAVOR "ubuntu-18.04-x86_64-gcc" CACHE STRING "SIL Kit package flavor for Linux")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SILKIT_FLAVOR "Win-x86_64-VS2017" CACHE STRING "SIL Kit package flavor for Windows")
    else()
        message(FATAL_ERROR "Unsupported platform for SilKit build.")
    endif()

    set(SILKIT_VERSION "4.0.37" CACHE STRING "SIL Kit package is specified, this version will be downloaded")
    message ("Build SilKit component on system: ${CMAKE_SYSTEM_PROCESSOR}")

    # Fetch SIL Kit  from github.com
    message(STATUS "Attempting to fetch [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}] from github.com")
    include(FetchContent)
    FetchContent_Declare(
        silkit
        URL https://github.com/vectorgrp/sil-kit/releases/download/sil-kit%2Fv${SILKIT_VERSION}/SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}.zip
        DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/sil-kit
        )

    message(STATUS "SIL Kit: fetching [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}]")
    FetchContent_MakeAvailable(silkit)

    message(STATUS "SIL Kit: using source code from: \"${silkit_SOURCE_DIR}\"")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(SILKIT_LIB "${silkit_SOURCE_DIR}/SilKit/lib/libSilKit.so")
        message(STATUS "Expecting libSilKit.so at: ${SILKIT_LIB}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SILKIT_LIB "${silkit_SOURCE_DIR}/SilKit/lib/SilKit.lib")
        message(STATUS "Expecting SilKit.lib at: ${SILKIT_LIB}")
    else()
        message(FATAL_ERROR "SilKit libraries not found at: ${SILKIT_LIB}. Check the SilKit flavor, version, and extraction path.")
    endif()

    set(SilKit_DIR "${silkit_SOURCE_DIR}/SilKit/lib/cmake/SilKit")
    message(STATUS "SilKit_DIR: ${SilKit_DIR}")

    find_package(SilKit REQUIRED CONFIG)

    add_library(can_com_silkit SHARED "can_com_silkit.h" "can_com_silkit.cpp")

    # Only add -Wno-shadow for GCC or Clang
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(can_com_silkit PRIVATE -Wno-shadow)
    endif()

    target_link_libraries(can_com_silkit PRIVATE "${SILKIT_LIB}" ${CMAKE_THREAD_LIBS_INIT})
    target_include_directories(can_com_silkit PRIVATE "${silkit_SOURCE_DIR}/SilKit/include")

    set_target_properties(can_com_silkit PROPERTIES PREFIX "")

    set_target_properties(can_com_silkit PROPERTIES SUFFIX ".sdv")

    add_dependencies(can_com_silkit CompileCoreIDL)

    # Appending the service in the service list
    set(SDV_Service_List ${SDV_Service_List} can_com_silkit PARENT_SCOPE)
endif() # SilKit library is not compatible for ARM and MINGW
