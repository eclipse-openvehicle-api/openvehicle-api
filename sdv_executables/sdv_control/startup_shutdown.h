#ifndef STARTUP_SHUTDOWN_H
#define STARTUP_SHUTDOWN_H

#include "context.h"
#include <vector>
#include <string>
#include <support/sequence.h>
#include <support/string.h>
#include <interfaces/process.h>

/**
 * @brief Help for startup or shutdown the server.
 * @param[in] rsContext Reference to the context.
 */
void StartupShutdownHelp(const SContext& rsContext);

/**
 * @brief Startup the SDV server.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int StartupSDVServer(const SContext& rsContext);

/**
 * @brief Shutdown the SDV Server.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int ShutdownSDVServer(const SContext& rsContext);

/**
 * @brief Get the server process ID after a startup of the server.
 * @remarks The process ID is set when server execution is initiated. The presence of the process ID doesn't guarantee a successful
 * running of the server.
 * @return The process ID if the process execution was initiated. Or zero when the execution failed.
 */
sdv::process::TProcessID GetServerProcessID();

#endif // !defined STARTUP_SHUTDOWN_H