# Cross Compilation Switch
if(VAPI_CROSS_COMPILE STREQUAL "arm")
    message("Setting up arm cross compiler toolchain...")
    set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc" CACHE FILEPATH "Compiler" FORCE)
    set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++" CACHE FILEPATH "Compiler" FORCE)
endif()