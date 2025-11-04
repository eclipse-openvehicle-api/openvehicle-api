#ifndef EXECUTABLES_ERROR_MSG
#define EXECUTABLES_ERROR_MSG

#include <string>

#ifdef WIN32
// The define NO_ERROR could have been defined already by Windows. Suppress the definition during the error message creation.
// Then re-enable the define. This only works, because the newly assigned value and the value used by Windows is identical.
#ifdef NO_ERROR
#pragma push_macro("NO_ERROR")
#undef NO_ERROR
#define SUPPRESS_NO_ERROR
#endif
#endif

/**
 * @brief Macro for making an error message definition.
 * @details This macro creates three variables for the use of error reporting:<br>
 * &lt;identifier&gt; The assigned error code (application return code).<br>
 * &lt;identifier&gt;_MSG The assigned error message.<br>
 * &lt;identifier&gt;_DESC The assigned error explanation.<br>
 * @param id The ID of the message (integer)
 * @param identifier The identifier used for the message assignment.
 * @param topic The string to explain the error in one sentence.
 * @param description The string to provide detailed information about the error.
 */
#define MAKE_ERROR_MSG(id, identifier, topic, description) \
    [[maybe_unused]] static const int identifier = id;  \
    [[maybe_unused]] static const char* identifier##_MSG = topic; \
    [[maybe_unused]] static const char* identifier##_DESC = description;

////////// GENERIC ERROR CODES ////////////

MAKE_ERROR_MSG(0, NO_ERROR, "No error.", "Execution succeeded expectinly.")
MAKE_ERROR_MSG(-1, NOT_IMPLEMENTED, "Not implemented.", "The function is not implemented.")
MAKE_ERROR_MSG(-2, CMDLN_ARG_ERR, "Command line argument error.", "An incorrect argument was supplied on the command line.")
MAKE_ERROR_MSG(-3, CMDLN_SILENT_VERBOSE, "Cannot supply both silent and verbose options.", "Both silent and verbose were supplied, which cannot be used at the same time.")
MAKE_ERROR_MSG(-100, PROCESS_TIMEOUT, "Potential deadlock detected; termination enforced.", "The process watchdog has detected a potential deadlock and initiated the process termination.")

////////// CONNECTION AND CORE SERVICE ACCESS ERROR CODES ////////////

MAKE_ERROR_MSG(-124, CONNECT_SDV_SERVER_ERROR, "Could not connect to SDV Server.", "Failed to connect to a running SDV service instance.")
MAKE_ERROR_MSG(-131, PROCESS_CONTROL_SERVICE_ACCESS_ERROR, "Process control service is not accessible.", "The process control service could not be accessed.")
MAKE_ERROR_MSG(-132, MODULE_CONTROL_SERVICE_ACCESS_ERROR, "Module control service is not accessible.", "The module control service could not be accessed.")
MAKE_ERROR_MSG(-133, REPOSITORY_SERVICE_ACCESS_ERROR, "Repository service is not accessible.", "The repository service could not be accessed.")
MAKE_ERROR_MSG(-134, CONFIG_SERVICE_ACCESS_ERROR, "Configuration service is not accessible.", "The configuration service could not be accessed.")
MAKE_ERROR_MSG(-135, COMMUNICATION_CONTROL_SERVICE_ACCESS_ERROR, "Communication control service is not accessible.", "The communication control service service could not be accessed.")
MAKE_ERROR_MSG(-136, APP_CONTROL_SERVICE_ACCESS_ERROR, "Application control service is not accessible.", "The application control service could not be accessed.")
MAKE_ERROR_MSG(-170, CANNOT_FIND_OBJECT, "The object cannot be found.", "A search for an object with supplied name was not successful.")


////////// LOCAL APP CONTROL ERROR CODES ////////////

MAKE_ERROR_MSG(-204, APP_CONTROL_STARTUP_ERROR, "Cannot start application control.", "Application control start was initiated, but returned an error.")
MAKE_ERROR_MSG(-205, APP_CONTROL_START_LOOP_FAIELD, "Starting the running loop failed.", "Failed to enter the processing loop for a server application.")
MAKE_ERROR_MSG(-206, APP_CONTROL_DUPLICATE_INSTANCE, "Another process with specified instance is already running.", "Another core process initiated the application control with the same instance. Only one core process per instance is allowed.")
MAKE_ERROR_MSG(-210, APP_CONTROL_INVALID_ISOLATION_CONFIG, "An invalid isolation configuration was supplied.", "The application control configuration section contains an invalid isolation configuration.")
MAKE_ERROR_MSG(-218, SETTING_FILE_VERSION_INVALID, "Settings file has an invalid version.", "Settings file contains an invalid/incompatible version number.")
MAKE_ERROR_MSG(-222, REGISTER_PROCESS_MONITOR_ERROR, "Failed to register the process monitor.", "During the registration of the process monitor, an error occurred.")
MAKE_ERROR_MSG(-214, LINK_REPO_SERVICE_ERROR, "Cannot link the core repository to the isolated repository.", "Failed to link the core repository to the repository of the isolated application.")
MAKE_ERROR_MSG(-230, ISOLATION_INVALID_OBJECT_TYPE, "An object of this type cannot be run isolated.", "The isolation of an objeczt is supported for complex services and utilities.")
MAKE_ERROR_MSG(-231, ISOLATION_FAILED, "The isolation of the object failed.", "The request to run an object in an isolated environment failed.")
MAKE_ERROR_MSG(-240, REGISTER_FOREIGN_OBJECT_FAILED, "The registration of a foreign object failed.", "An running object registration as a foreign object was not successful.")


////////// SDV CONTROL ERROR CODES ////////////

MAKE_ERROR_MSG(-821, STARTUP_CORE_ERROR, "Could not start the SDV core process.", "Failed to start the SDV core process.")
MAKE_ERROR_MSG(-822, CORE_NOT_STARTED, "The SDV core process was not running.", "A request for the status of a SDV core process returned that the process was not running.")
MAKE_ERROR_MSG(-823, SHUTDOWN_CORE_ERROR, "Could not start the SDV core process.", "Failed to start the SDV core process.")
MAKE_ERROR_MSG(-840, START_OBJECT_ERROR, "Could not start the object.", "Failed to start an object.")
MAKE_ERROR_MSG(-841, STOP_OBJECT_ERROR, "Could not stop the object.", "Failed to stop/destroy an object.")


////////// SDV PACKAGER ERROR CODES ////////////

MAKE_ERROR_MSG(-1005, CMDLN_INSTALL_NAME_MISSING, "No installation name provided.", "The installation name was not supplied at the command line. This is needed for packing, direct installation and uninstallation.")
MAKE_ERROR_MSG(-1006, CMDLN_SOURCE_LOCATION_ERROR, "Source location cannot be found or is not a directory.", "The supplied source location is not a valid directory.")
MAKE_ERROR_MSG(-1007, CMDLN_OUTPUT_LOCATION_ERROR, "Output location cannot be found or is not a directory.", "The supplied output location is not a valid directory.")
MAKE_ERROR_MSG(-1004, CMDLN_TARGET_LOCATION_ERROR, "Target location cannot be found or is not a directory.", "The supplied target location is not a valid directory.")
MAKE_ERROR_MSG(-1008, CMDLN_MULTIPLE_CFG_FILES, "Only one configuration file can be generated.", "Multiple configuration files were supplied at the command line. Only one configuration file can be generated at the time.")
MAKE_ERROR_MSG(-1009, CMDLN_CREATE_ONLY_MANIFEST_DIRECT_INSTALL_ERROR, "create_manifest_only-flag is not compatible with direct_install-flag.", "Command line flag for only creation of a manifest cannot be combined with the flag for direct installation.")
MAKE_ERROR_MSG(-1010, CMDLN_NOTHING_TO_DO, "No action supplied; nothing to do.", "No command was supplied on the command line. There is nothing to do.")
MAKE_ERROR_MSG(-1011, CMDLN_UPDATE_OVERWRITE_ERROR, "Cannot specify update and overwrite at the same time.", "Update and overwrite were specified at the same time. This is not valid.")
MAKE_ERROR_MSG(-1012, CMDLN_SOURCE_FILE_ERROR, "Source file cannot be found or is invalid.", "The supplied source file name is not a valid file.")
MAKE_ERROR_MSG(-1013, CMDLN_SOURCE_FILE_MISSING, "No source file specified.", "There was no source file specified at the command line.")
MAKE_ERROR_MSG(-1014, CMDLN_TOO_MANY_SOURCE_FILES, "Too many source files were specified.", "Too many source files were specified at the command line. Only one file is supported at the time.")
MAKE_ERROR_MSG(-1015, CMDLN_INCOMPATIBLE_ARGUMENTS, "Incompatible arguments were supplied, not fitting the command.", "Some arguments cannot be combined with the command on the command line.")
MAKE_ERROR_MSG(-1016, CMDLN_MISSING_SHOW_COMMAND, "Missing show command.", "The command SHOW was supplied on the command line, but not what to show.")
MAKE_ERROR_MSG(-1017, CMDLN_INVALID_CONFIG_STRING, "The configuration string is invalid.", "The configuration string should consist of path (only for local) followed by +component,component,....")
MAKE_ERROR_MSG(-1018, SAVE_INSTALL_MANIFEST_ERROR, "Failed to save the installation manifest.", "Saving the installation manifest returned with an error.")
MAKE_ERROR_MSG(-1019, SAVE_SETTINGS_FILE_ERROR, "Failed to save application settings file.", "Saving the application settings file returned with an error.")
MAKE_ERROR_MSG(-1020, SAVE_CONFIG_FILE_ERROR, "Failed to save the config file.", "Saving the configuration file returned with an error.")
MAKE_ERROR_MSG(-1021, CANNOT_REMOVE_INSTALL_DIR, "Cannot remove existing installation directory.", "Failed to remove an existing installation directory.")
MAKE_ERROR_MSG(-1022, CREATE_INSTALL_DIR_ERROR, "Cannot create installation directory.", "Failed to create the installation directory.")
MAKE_ERROR_MSG(-1023, CREATE_TARGET_DIR_ERROR, "Cannot create target root directory.", "Failed to create the target root directory.")
MAKE_ERROR_MSG(-1024, CREATE_CONFIG_DIR_ERROR, "Cannot create config target directory.", "Failed to create the config target directory.")
MAKE_ERROR_MSG(-1026, NO_SOURCE_FILES, "No source files were found.", "No source files were found to add to the package.")
MAKE_ERROR_MSG(-1027, PACKAGE_CREATION_ERROR, "A compose package error has occurred.", "An error has occurred during the package creation.")
MAKE_ERROR_MSG(-1028, PACKAGE_READ_ERROR, "A read package error has occurred.", "An error has occurred while reading the package.")

////////// SDV TRACE MONITOR ERROR CODES ////////////
MAKE_ERROR_MSG(-5500, TRACE_MON_REG_HNDLR_ERROR, "Failed to register application control handler.", "The OS returned an error during the registration of the application control handler.")
MAKE_ERROR_MSG(-5501, TRACE_MON_FIFO_OPEN_ERROR, "Failed to open the trace fifo.", "Failure trying to open a connection to the trace fifo.")



MAKE_ERROR_MSG(-2051, LOAD_DBC_FILE_ERROR, "Cannot load the DBC file.", "Trying to read the DBC file failed.")
MAKE_ERROR_MSG(-2052, COMPILE_ERROR, "Failed to compile.", "An compilation attempt failed.")
MAKE_ERROR_MSG(-2052, BASIC_SERVICE_DATA_ERROR, "Cannot find vehicle device.", "Creating basic service component failed.")

#ifdef WIN32
#ifdef SUPPRESS_NO_ERROR
#pragma pop_macro("NO_ERROR")
#endif
#endif


#endif // !defined EXECUTABLES_ERROR_MSG
