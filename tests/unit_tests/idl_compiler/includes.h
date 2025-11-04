#ifndef INCLUDES_H
#define INCLUDES_H

#include "../../include/gtest_custom.h"

/**
 * @brief This project implements its own GetObject function.
 */
#define SDV_CUSTOM_GETOBJECT

#include "../../../sdv_executables/sdv_idl_compiler/logger.h"

/**
 * @brief For testing purposes, enable the verbosity mode.
 */
inline void EnableVerbosityMode()
{
    g_log_control.SetVerbosityMode(EVerbosityMode::report_all);
}

/**
 * @brief For testing purposes, disable the verbosity mode.
 */
inline void DisableVerbosityMode()
{
    g_log_control.SetVerbosityMode(EVerbosityMode::report_errors);
}

/**
 * @brief For testing purposes, enable the verbosity mode within local scope.
 */
class CVerbosityMode
{
public:
    CVerbosityMode() {EnableVerbosityMode();}
    ~CVerbosityMode() {DisableVerbosityMode();}
};

#endif // !defined INCLUDES_H