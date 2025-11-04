#ifndef INCLUDES_IDL_COMPILER_H
#define INCLUDES_IDL_COMPILER_H

// Comment the following file to use the backup file. Uncomment to use the IDL file. Both files should contain identical
// definitions and declarations.
//#include <interfaces/core_idl.h>

// REMARKS: Using the "core_idl.h" works once; after that CMake detects circular dependencies and doesn't want to build any
// more. Therefore, it is advisable to use it only for checking the interfaces.
// To re-allow building again after a circular reference, clear the CMake cache (in the project-menu item).

#ifndef __IDL_GENERATED__CORE_IDL_DEF_H__
// BACKUP MODE - IN CASE "core_idl.h" is destroyed or doesn't exist
#include "core_idl_backup.h"
#endif

#endif // !defined(INCLUDES_IDL_COMPILER_H)