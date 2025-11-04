# CppCheck include file.
# Author: Erik Verhoeven

# Checkj whether
if (CMAKE_ENABLE_CPPCHECK)

    # Create a string with platform specific defines
    # NOTE: Windows MSVC and GCC as well as Linux GCC are supported. Windows only on x64. Linux on x64 and aarch64 (ARM)
    set (CPPCHECK_DEFINES)
    if (WIN32)
        set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} _WIN32)
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} _MSC_VER=1929 _WIN64)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} _M_ARM64)
        else()
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} _M_X64)
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} __GNUC__=15 __GNUC_MINOR___=1 __GNUC_PATCHLEVEL__=0 __linux__ __unix__)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} __aarch64__)
        else()
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} __x86_64__)
        endif()
        if (WIN32)
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} __MINGW64__)
        else()
            set (CPPCHECK_DEFINES ${CPPCHECK_DEFINES} _POSIX_C_SOURCE=200809L)
        endif()
    else()
        message(FATAL_ERROR "Only Windows and Linux are currently supported for x64 and aarch64 platforms using MSVC or GCC.")
    endif()

    
    # Convert CPPCHECK_DEFINES (space-separated) to a list
    string(REPLACE " " ";" CPPCHECK_DEFINES_LIST "${CPPCHECK_DEFINES}")

    # Prefix each define with -D
    set(CPPCHECK_ARGS "")
    foreach(DEF IN LISTS CPPCHECK_DEFINES_LIST)
        list(APPEND CPPCHECK_ARGS "-D${DEF}")
    endforeach()
    message ("CPPCHECK_ARGS = ${CPPCHECK_ARGS}")

    # Use generator expression for target definitions
    set(CPPCHECK_TARGET_DEFINES "$<JOIN:$<TARGET_PROPERTY:cppcheck,COMPILE_DEFINITIONS>,;-D>")

    # Execute CppCheck
    find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
    if (CPPCHECK_EXECUTABLE)
        message("CppCheck was found at ${CPPCHECK_EXECUTABLE}...")

        # Determine the version of CppCheck
        execute_process(
            COMMAND "${CPPCHECK_EXECUTABLE}" --version
            OUTPUT_VARIABLE CPPCHECK_VERSION_RAW
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        # Extract major and minor version
        string(REGEX MATCH "([0-9]+)\\.([0-9]+)" CPPCHECK_VERSION "${CPPCHECK_VERSION_RAW}")
        string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\1" CPPCHECK_VERSION_MAJOR "${CPPCHECK_VERSION}")
        string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\2" CPPCHECK_VERSION_MINOR "${CPPCHECK_VERSION}")

        # CppCheck before 2.10 reports a false positive warning of uninitialized member variables when declared as inline static
        # const. Furthermore, variables which are initialized, assigned to an object during binding, updated by the object and then
        # used, are not identified to have been changed. Suppress the warnings for files using these variables.
        if (CPPCHECK_VERSION_MAJOR GREATER 2 OR (CPPCHECK_VERSION_MAJOR EQUAL 2 AND CPPCHECK_VERSION_MINOR LESS 10))
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/support/pointer.inl")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/support/string.inl")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/support/sequence.inl")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/support/mem_access.h")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/support/serdes.inl")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/sdv_services/core/installation_composer.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/proxystub.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/module_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/app_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/com_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/core_idl_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/config_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/core_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/timer_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/toml_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=uninitvar:${CMAKE_SOURCE_DIR}/export/interfaces/ps/repository_proxy.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_dbc_util/main.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_vss_util/main.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_core/main.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_iso/main.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_local_shutdown/main.cpp")
            list (APPEND CPPCHECK_ARGS "--suppress=knownConditionTrueFalse:${CMAKE_SOURCE_DIR}/sdv_executables/sdv_trace_mon/main.cpp")
        endif()

        # Check for version 2.11 to add the exhaustive checking
        if (CPPCHECK_VERSION_MAJOR GREATER 2 OR (CPPCHECK_VERSION_MAJOR EQUAL 2 AND CPPCHECK_VERSION_MINOR GREATER_EQUAL 11))
            list (APPEND CPPCHECK_ARGS "--check-level=exhaustive")              # Do exhaustive checking
            list (APPEND CPPCHECK_ARGS "--suppress=missingIncludeSystem")       # The system include files might not be accessable at all times and should not be used.
        endif()

        # Check for version 2.12 to add the checker report
        if (CPPCHECK_VERSION_MAJOR GREATER 2 OR (CPPCHECK_VERSION_MAJOR EQUAL 2 AND CPPCHECK_VERSION_MINOR GREATER_EQUAL 12))
            list (APPEND CPPCHECK_ARGS "--checkers-report=${CMAKE_BINARY_DIR}/cppcheck-checkers-report.txt")
        endif()

        # Execute CppCheck
        add_custom_target(cppcheck ALL
            COMMAND "${CPPCHECK_EXECUTABLE}"
                -j8
                --enable=all
                --quiet
                --std=c++17
                --language=c++
                --project=${CMAKE_BINARY_DIR}/compile_commands.json
                --suppress=unmatchedSuppression         # Every CppCheck version checks differently and suppressions might not fit for evey version.
                --suppress=cppcheckError                # Internal errors of CppCheck should not be reported.
                --suppress=unusedFunction               # In a framework there might be many functions which are not used.
                --suppress=useStlAlgorithm              # STL doesn't always provide an easier iteration than the raw for loop (readability).
                --suppress=noExplicitConstructor        # Most constructors use implicit construction.
                --suppress=duplInheritedMember          # The generated code reuses common function and variable names. Also template specialization does this.
                --suppress=dangerousTypeCast:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h                      # Suppress warning in 3rd party library
                --suppress=cstyleCast:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h                             # Suppress warning in 3rd party library
                --suppress=nullPointerRedundantCheck:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h              # Suppress warning in 3rd party library
                --suppress=nullPointerArithmeticRedundantCheck:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h    # Suppress warning in 3rd party library
                --suppress=shiftNegative:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h                          # Suppress warning in 3rd party library
                --suppress=shiftTooManyBits:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h                       # Suppress warning in 3rd party library
                --suppress=signConversionCond:${CMAKE_BINARY_DIR}/_deps/xxhash-src/xxhash.h                     # Suppress warning in 3rd party library
                --suppress=uninitDerivedMemberVar:${CMAKE_BINARY_DIR}/_deps/silkit-src/SilKit/include/silkit/services/flexray/FlexrayDatatypes.hpp          # Suppress warning in 3rd party library
                --suppress=constVariableReference:${CMAKE_BINARY_DIR}/_deps/silkit-src/SilKit/include/silkit/detail/impl/SilKit.ipp                         # Suppress warning in 3rd party library
                --suppress=passedByValue:${CMAKE_BINARY_DIR}/_deps/silkit-src/SilKit/include/silkit/detail/impl/services/lin/LinController.hpp              # Suppress warning in 3rd party library
                --suppress=passedByValue:${CMAKE_BINARY_DIR}/_deps/silkit-src/SilKit/include/silkit/detail/impl/services/orchestration/LifecycleService.hpp # Suppress warning in 3rd party library
                --inline-suppr
                --xml
                --xml-version=2
                ${CPPCHECK_ARGS}
                2> ${CMAKE_BINARY_DIR}/cppcheck-results.xml
            COMMAND echo "cppcheck finished..."
            COMMAND echo "See ${CMAKE_BINARY_DIR}/cppcheck-results.xml for results."
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM
            DEPENDS dependency_sdv_components
        )
    else()
        message(FATAL_ERROR "CppCheck program was not found...")
    endif()
endif()
