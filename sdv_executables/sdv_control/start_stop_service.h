#ifndef START_STOP_SERVICE_H
#define START_STOP_SERVICE_H

#include "context.h"
#include <support/sequence.h>
#include <support/string.h>
#include <support/interface_ptr.h>

/**
 * @brief Help for start or stop a complex service.
 * @param[in] rsContext Reference to the context.
 */
void StartStopServiceHelp(const SContext& rsContext);

/**
* @brief Start the service.
* @details The command line context includes the service to start or stop.
* @param[in] rsContext Reference to the context.
* @return The application exit code. 0 is no error.
*/
int StartService(const SContext& rsContext);

/**
* @brief Stop the complex service.
* @param[in] rsContext Reference to the context.
* @return The application exit code. 0 is no error.
*/
int StopService(const SContext& rsContext);

#endif // !defined START_STOP_SERVICE_H