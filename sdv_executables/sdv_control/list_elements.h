#ifndef LIST_ELEMENTS_H
#define LIST_ELEMENTS_H

#include "context.h"
#include <support/sequence.h>
#include <support/string.h>
#include <support/interface_ptr.h>

/**
 * @brief Help for listing elements.
 * @param[in] rsContext Reference to the context.
 */
void ListHelp(const SContext& rsContext);

/**
 * @brief List elements in a table. Base function. The elements to list are to be parsed from the arguments. The context could
 * indicate additional flags.
 * @param[in] rsContext Reference to the context.
 * @param[in] rstream The output stream to use for printing (table only).
 * @return The application exit code. 0 is no error.
 */
int ListElements(const SContext& rsContext, std::ostream& rstream = std::cout);

/**
 * @brief List the loaded server modules.
 * @param[in] rsContext Reference to the context.
 * @param[in] rptrRepository Reference to the saerver repository.
 * @param[in] rstream The output stream to use for printing (table only).
 * @return The application exit code. 0 is no error.
 */
int ListModules(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream);

/**
 * @brief List the available component classes.
 * @param[in] rsContext Reference to the context.
 * @param[in] rptrRepository Reference to the saerver repository.
 * @param[in] rstream The output stream to use for printing (table only).
 * @return The application exit code. 0 is no error.
 */
int ListClasses(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream);

/**
 * @brief List the instantiated server components.
 * @param[in] rsContext Reference to the context.
 * @param[in] rptrRepository Reference to the saerver repository.
 * @param[in] rstream The output stream to use for printing (table only).
 * @return The application exit code. 0 is no error.
 */
int ListComponents(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream);

/**
* @brief List the installations.
* @param[in] rsContext Reference to the context.
* @param[in] rptrRepository Reference to the saerver repository.
* @param[in] rstream The output stream to use for printing (table only).
* @return The application exit code. 0 is no error.
*/
int ListInstallations(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream);

/**
 * @brief List the current connections.
 * @param[in] rsContext Reference to the context.
 * @param[in] rptrRepository Reference to the saerver repository.
 * @param[in] rstream The output stream to use for printing (table only).
 * @return The application exit code. 0 is no error.
 */
int ListConnections(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream);

#endif // !defined LIST_ELEMENTS_H