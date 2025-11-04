set(DBC_CONFIGURATION_FAILED FALSE)

# Execute sdv_dbc_util to create datalink component
message("Create interface code for 'dbc_struct_test.dbc'.")
execute_process(COMMAND "${SDV_DBC_UTIL}" "${CMAKE_CURRENT_LIST_DIR}/dbc_struct_test.dbc" "-O${CMAKE_CURRENT_LIST_DIR}/generated/dbc_struct_test/" --nodesTestECU_Tx,TestECU_Rx --dl_lib_namestruct_test)
set(DBC_STRUCT_TEST_FOLDER "${CMAKE_CURRENT_LIST_DIR}/generated/dbc_struct_test")
if(EXISTS "${DBC_STRUCT_TEST_FOLDER}")
    message(STATUS "(OK) Folder exists: ${DBC_STRUCT_TEST_FOLDER}")
else()
    message(WARNING "(Fail) Folder does NOT exist: ${DBC_STRUCT_TEST_FOLDER}")
    set(DBC_CONFIGURATION_FAILED TRUE)
endif()

# Execute sdv_dbc_util to create datalink component
message("Create interface code for 'example_can.dbc'.")
execute_process(COMMAND "${SDV_DBC_UTIL}" "${CMAKE_CURRENT_LIST_DIR}/example_can.dbc" "-O${CMAKE_CURRENT_LIST_DIR}/generated/example_can/" --nodesTestECU_Tx,TestECU_Rx --dl_lib_nameexample_can_test)
set(EXAMPLE_CAN_FOLDER "${CMAKE_CURRENT_LIST_DIR}/generated/example_can")
if(EXISTS "${EXAMPLE_CAN_FOLDER}")
    message(STATUS "(OK) Folder exists: ${EXAMPLE_CAN_FOLDER}")
else()
    message(WARNING "(Fail) Folder does NOT exist: ${EXAMPLE_CAN_FOLDER}")
    set(DBC_CONFIGURATION_FAILED TRUE)
endif()


# --- Final check ---
if(DBC_CONFIGURATION_FAILED)
    message(FATAL_ERROR "Configuration failed due to missing folders.")
endif()

# compile the different data link components:
add_subdirectory(generated/dbc_struct_test/can_dl)
add_subdirectory(generated/example_can/can_dl)
