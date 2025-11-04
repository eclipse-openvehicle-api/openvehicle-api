#ifndef INSTALLATION_H
#define INSTALLATION_H

#include "context.h"
#include <support/sequence.h>
#include <support/string.h>
#include <support/interface_ptr.h>

/**
 * @brief Help for installing, updating and uninstalling components.
 * @param[in] rsContext Reference to the context.
 */
void InstallationHelp(const SContext& rsContext);

/**
 * @brief Install components.
 * @details The command line context.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int Install(const SContext& rsContext);

/**
 * @brief Update installation.
 * @details The command line context.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int Update(const SContext& rsContext);

/**
 * @brief Uninstall components.
 * @details The command line context.
 * @param[in] rsContext Reference to the context.
 * @return The application exit code. 0 is no error.
 */
int Uninstall(const SContext& rsContext);

#endif // !defined INSTALLATION_H