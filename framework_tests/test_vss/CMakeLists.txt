# Define project
project (VSSComponentsTests VERSION 1.0 LANGUAGES CXX)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include directory to the core framework
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

file (COPY ${PROJECT_SOURCE_DIR}/config/data_dispatch_service.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeboolean.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypefloat.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypedouble.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypelongdouble.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeint8.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeint16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeint32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeint64.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypelong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypelonglong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeshort.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypestring.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeu8string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeu16string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeu32string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypenative.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypewstring.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypechar.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypechar16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypechar32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypewchar.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeuint8.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeuint16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeuint32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeuint64.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeunsignedlong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeunsignedlonglong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/rxtypeunsignedshort.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)

file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeboolean.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypefloat.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypedouble.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypelongdouble.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeint8.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeint16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeint32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeint64.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypelong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypelonglong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeshort.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypestring.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeu8string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeu16string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeu32string.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypenative.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypewstring.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypechar.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypechar16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypechar32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypewchar.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeuint8.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeuint16.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeuint32.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeuint64.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeunsignedlong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeunsignedlonglong.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/config/txtypeunsignedshort.toml  DESTINATION ${CMAKE_BINARY_DIR}/bin/config/)

# VSS util component test executable
add_executable(ComponentTest_VSSComponents load_components_test.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../vss_util/generated/all_types/vss_files/)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(ComponentTest_VSSComponents ${CMAKE_THREAD_LIBS_INIT} GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentTest_VSSComponents Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_VSSComponents ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentTest_VSSComponents GTest::GTest Rpcrt4.lib)
endif()

# Add the VSS utility component test
add_test(NAME ComponentTest_VSSComponents COMMAND ComponentTest_VSSComponents WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_VSSComponents POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_VSSComponents>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_VSSComponents.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_VSSComponents  testcase4_vd_rxclassforboolean_rx testcase4_bs_rxclassforboolean_rx
        testcase4_vd_rxclassforboolean_rx testcase4_bs_rxclassforboolean_rx
        testcase4_vd_rxclassforfloat_rx testcase4_bs_rxclassforfloat_rx
        testcase4_vd_rxclassforfloat_rx testcase4_bs_rxclassforfloat_rx
        testcase4_vd_rxclassfordouble_rx testcase4_bs_rxclassfordouble_rx
        testcase4_vd_rxclassfordouble_rx testcase4_bs_rxclassfordouble_rx
        testcase4_vd_rxclassforlongdouble_rx testcase4_bs_rxclassforlongdouble_rx
        testcase4_vd_rxclassforlongdouble_rx testcase4_bs_rxclassforlongdouble_rx
        testcase4_vd_rxclassforshort_rx testcase4_bs_rxclassforshort_rx
        testcase4_vd_rxclassforlong_rx testcase4_bs_rxclassforlong_rx
        testcase4_vd_rxclassforunsignedlong_rx testcase4_bs_rxclassforunsignedlong_rx
        testcase4_vd_rxclassforlonglong_rx testcase4_bs_rxclassforlonglong_rx
        testcase4_vd_rxclassforunsignedlonglong_rx testcase4_bs_rxclassforunsignedlonglong_rx
        testcase4_vd_rxclassforchar_rx testcase4_bs_rxclassforchar_rx
        testcase4_vd_rxclassforchar16_rx testcase4_bs_rxclassforchar16_rx
        testcase4_vd_rxclassforchar32_rx testcase4_bs_rxclassforchar32_rx
        testcase4_vd_rxclassforwchar_rx testcase4_bs_rxclassforwchar_rx
        testcase4_vd_rxclassforint8_rx testcase4_bs_rxclassforint8_rx
        testcase4_vd_rxclassforuint8_rx testcase4_bs_rxclassforuint8_rx
        testcase4_vd_rxclassforuint16_rx testcase4_bs_rxclassforint16_rx
        testcase4_vd_rxclassforint16_rx testcase4_bs_rxclassforuint16_rx
        testcase4_vd_rxclassforint32_rx testcase4_bs_rxclassforint32_rx
        testcase4_vd_rxclassforuint32_rx testcase4_bs_rxclassforuint32_rx
        testcase4_vd_rxclassforint64_rx testcase4_bs_rxclassforint64_rx
        testcase4_vd_rxclassforuint64_rx testcase4_bs_rxclassforuint64_rx
        testcase4_vd_rxclassfornative_rx testcase4_bs_rxclassfornative_rx
        testcase4_vd_rxclassforstring_rx testcase4_bs_rxclassforstring_rx
        testcase4_vd_rxclassforu8string_rx testcase4_bs_rxclassforu8string_rx
        testcase4_vd_rxclassforu16string_rx testcase4_bs_rxclassforu16string_rx
        testcase4_vd_rxclassforu32string_rx testcase4_bs_rxclassforu32string_rx
        testcase4_vd_rxclassforwstring_rx testcase4_bs_rxclassforwstring_rx

        testcase4_vd_txclassforboolean_tx testcase4_bs_txclassforboolean_tx
        testcase4_vd_txclassforboolean_tx testcase4_bs_txclassforboolean_tx
        testcase4_vd_txclassforfloat_tx testcase4_bs_txclassforfloat_tx
        testcase4_vd_txclassforfloat_tx testcase4_bs_txclassforfloat_tx
        testcase4_vd_txclassfordouble_tx testcase4_bs_txclassfordouble_tx
        testcase4_vd_txclassfordouble_tx testcase4_bs_txclassfordouble_tx
        testcase4_vd_txclassforlongdouble_tx testcase4_bs_txclassforlongdouble_tx
        testcase4_vd_txclassforlongdouble_tx testcase4_bs_txclassforlongdouble_tx
        testcase4_vd_txclassforshort_tx testcase4_bs_txclassforshort_tx
        testcase4_vd_txclassforlong_tx testcase4_bs_txclassforlong_tx
        testcase4_vd_txclassforunsignedlong_tx testcase4_bs_txclassforunsignedlong_tx
        testcase4_vd_txclassforlonglong_tx testcase4_bs_txclassforlonglong_tx
        testcase4_vd_txclassforunsignedlonglong_tx testcase4_bs_txclassforunsignedlonglong_tx
        testcase4_vd_txclassforchar_tx testcase4_bs_txclassforchar_tx
        testcase4_vd_txclassforchar16_tx testcase4_bs_txclassforchar16_tx
        testcase4_vd_txclassforchar32_tx testcase4_bs_txclassforchar32_tx
        testcase4_vd_txclassforwchar_tx testcase4_bs_txclassforwchar_tx
        testcase4_vd_txclassforint8_tx testcase4_bs_txclassforint8_tx
        testcase4_vd_txclassforuint8_tx testcase4_bs_txclassforuint8_tx
        testcase4_vd_txclassforuint16_tx testcase4_bs_txclassforint16_tx
        testcase4_vd_txclassforint16_tx testcase4_bs_txclassforuint16_tx
        testcase4_vd_txclassforint32_tx testcase4_bs_txclassforint32_tx
        testcase4_vd_txclassforuint32_tx testcase4_bs_txclassforuint32_tx
        testcase4_vd_txclassforint64_tx testcase4_bs_txclassforint64_tx
        testcase4_vd_txclassforuint64_tx testcase4_bs_txclassforuint64_tx
        testcase4_vd_txclassfornative_tx testcase4_bs_txclassfornative_tx 
        testcase4_vd_txclassforstring_tx testcase4_bs_txclassforstring_tx 
        testcase4_vd_txclassforu8string_tx testcase4_bs_txclassforu8string_tx 
        testcase4_vd_txclassforu16string_tx testcase4_bs_txclassforu16string_tx 
        testcase4_vd_txclassforu32string_tx testcase4_bs_txclassforu32string_tx 
        testcase4_vd_txclassforwstring_tx testcase4_bs_txclassforwstring_tx)  
     











