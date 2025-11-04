
/**
 * @brief CoSimulation header template. File 'cosimulation.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szCosimulationHeaderTemplate[] = R"code(#pragma once

#include "model.h"

#define EPSILON (FIXED_SOLVER_STEP * 1e-6)

void doFixedStep(ModelInstance *comp, bool* stateEvent, bool* timeEvent);
)code";


/**
 * @brief Part one of FMI2 function header template. Due to the size split into 2 strings.
 * File 'fmi2Functions.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFMI2FunctionsHeaderTemplate1[] = R"code(#ifndef fmi2Functions_h
#define fmi2Functions_h

/* This header file must be utilized when compiling a FMU.
   It defines all functions of the
         FMI 2.0.1 Model Exchange and Co-Simulation Interface.

   In order to have unique function names even if several FMUs
   are compiled together (e.g. for embedded systems), every "real" function name
   is constructed by prepending the function name by "FMI2_FUNCTION_PREFIX".
   Therefore, the typical usage is:

      #define FMI2_FUNCTION_PREFIX MyModel_
      #include "fmi2Functions.h"

   As a result, a function that is defined as "fmi2GetDerivatives" in this header file,
   is actually getting the name "MyModel_fmi2GetDerivatives".

   This only holds if the FMU is shipped in C source code, or is compiled in a
   static link library. For FMUs compiled in a DLL/sharedObject, the "actual" function
   names are used and "FMI2_FUNCTION_PREFIX" must not be defined.

   Revisions:
   - Sep. 29, 2019: License changed to 2-clause BSD License (without extensions)
   - Apr.  9, 2014: All prefixes "fmi" renamed to "fmi2" (decision from April 8)
   - Mar. 26, 2014: FMI_Export set to empty value if FMI_Export and FMI_FUNCTION_PREFIX
                    are not defined (#173)
   - Oct. 11, 2013: Functions of ModelExchange and CoSimulation merged:
                      fmiInstantiateModel , fmiInstantiateSlave  -> fmiInstantiate
                      fmiFreeModelInstance, fmiFreeSlaveInstance -> fmiFreeInstance
                      fmiEnterModelInitializationMode, fmiEnterSlaveInitializationMode -> fmiEnterInitializationMode
                      fmiExitModelInitializationMode , fmiExitSlaveInitializationMode  -> fmiExitInitializationMode
                      fmiTerminateModel, fmiTerminateSlave  -> fmiTerminate
                      fmiResetSlave -> fmiReset (now also for ModelExchange and not only for CoSimulation)
                    Functions renamed:
                      fmiUpdateDiscreteStates -> fmiNewDiscreteStates
   - June 13, 2013: Functions removed:
                       fmiInitializeModel
                       fmiEventUpdate
                       fmiCompletedEventIteration
                       fmiInitializeSlave
                    Functions added:
                       fmiEnterModelInitializationMode
                       fmiExitModelInitializationMode
                       fmiEnterEventMode
                       fmiUpdateDiscreteStates
                       fmiEnterContinuousTimeMode
                       fmiEnterSlaveInitializationMode;
                       fmiExitSlaveInitializationMode;
   - Feb. 17, 2013: Portability improvements:
                       o DllExport changed to FMI_Export
                       o FUNCTION_PREFIX changed to FMI_FUNCTION_PREFIX
                       o Allow undefined FMI_FUNCTION_PREFIX (meaning no prefix is used)
                    Changed function name "fmiTerminate" to "fmiTerminateModel" (due to #113)
                    Changed function name "fmiGetNominalContinuousState" to
                                          "fmiGetNominalsOfContinuousStates"
                    Removed fmiGetStateValueReferences.
   - Nov. 14, 2011: Adapted to FMI 2.0:
                       o Split into two files (fmiFunctions.h, fmiTypes.h) in order
                         that code that dynamically loads an FMU can directly
                         utilize the header files).
                       o Added C++ encapsulation of C-part, in order that the header
                         file can be directly utilized in C++ code.
                       o fmiCallbackFunctions is passed as pointer to fmiInstantiateXXX
                       o stepFinished within fmiCallbackFunctions has as first
                         argument "fmiComponentEnvironment" and not "fmiComponent".
                       o New functions to get and set the complete FMU state
                         and to compute partial derivatives.
   - Nov.  4, 2010: Adapted to specification text:
                       o fmiGetModelTypesPlatform renamed to fmiGetTypesPlatform
                       o fmiInstantiateSlave: Argument GUID     replaced by fmuGUID
                                              Argument mimetype replaced by mimeType
                       o tabs replaced by spaces
   - Oct. 16, 2010: Functions for FMI for Co-simulation added
   - Jan. 20, 2010: stateValueReferencesChanged added to struct fmiEventInfo (ticket #27)
                    (by M. Otter, DLR)
                    Added WIN32 pragma to define the struct layout (ticket #34)
                    (by J. Mauss, QTronic)
   - Jan.  4, 2010: Removed argument intermediateResults from fmiInitialize
                    Renamed macro fmiGetModelFunctionsVersion to fmiGetVersion
                    Renamed macro fmiModelFunctionsVersion to fmiVersion
                    Replaced fmiModel by fmiComponent in decl of fmiInstantiateModel
                    (by J. Mauss, QTronic)
   - Dec. 17, 2009: Changed extension "me" to "fmi" (by Martin Otter, DLR).
   - Dez. 14, 2009: Added eventInfo to meInitialize and added
                    meGetNominalContinuousStates (by Martin Otter, DLR)
   - Sept. 9, 2009: Added DllExport (according to Peter Nilsson's suggestion)
                    (by A. Junghanns, QTronic)
   - Sept. 9, 2009: Changes according to FMI-meeting on July 21:
                    meInquireModelTypesVersion     -> meGetModelTypesPlatform
                    meInquireModelFunctionsVersion -> meGetModelFunctionsVersion
                    meSetStates                    -> meSetContinuousStates
                    meGetStates                    -> meGetContinuousStates
                    removal of meInitializeModelClass
                    removal of meGetTime
                    change of arguments of meInstantiateModel
                    change of arguments of meCompletedIntegratorStep
                    (by Martin Otter, DLR):
   - July 19, 2009: Added "me" as prefix to file names (by Martin Otter, DLR).
   - March 2, 2009: Changed function definitions according to the last design
                    meeting with additional improvements (by Martin Otter, DLR).
   - Dec. 3 , 2008: First version by Martin Otter (DLR) and Hans Olsson (Dynasim).


   Copyright (C) 2008-2011 MODELISAR consortium,
                 2012-2019 Modelica Association Project "FMI"
                 All rights reserved.

   This file is licensed by the copyright holders under the 2-Clause BSD License
   (https://opensource.org/licenses/BSD-2-Clause):

   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "fmi2TypesPlatform.h"
#include "fmi2FunctionTypes.h"
#include <stdlib.h>


/*
  Export FMI2 API functions on Windows and under GCC.
  If custom linking is desired then the FMI2_Export must be
  defined before including this file. For instance,
  it may be set to __declspec(dllimport).
*/
#if !defined(FMI2_Export)
  #if !defined(FMI2_FUNCTION_PREFIX)
    #if defined _WIN32 || defined __CYGWIN__
     /* Note: both gcc & MSVC on Windows support this syntax. */
        #define FMI2_Export __declspec(dllexport)
    #else
      #if __GNUC__ >= 4
        #define FMI2_Export __attribute__ ((visibility ("default")))
      #else
        #define FMI2_Export
      #endif
    #endif
  #else
    #define FMI2_Export
  #endif
#endif

/* Macros to construct the real function name
   (prepend function name by FMI2_FUNCTION_PREFIX) */
#if defined(FMI2_FUNCTION_PREFIX)
  #define fmi2Paste(a,b)     a ## b
  #define fmi2PasteB(a,b)    fmi2Paste(a,b)
  #define fmi2FullName(name) fmi2PasteB(FMI2_FUNCTION_PREFIX, name)
#else
  #define fmi2FullName(name) name
#endif
)code";


/**
 * @brief Part two of FMI2 function header template. Due to the size split into 2 strings.
 * File 'fmi2Functions.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFMI2FunctionsHeaderTemplate2[] = R"code(
/***************************************************
Common Functions
****************************************************/
#define fmi2GetTypesPlatform         fmi2FullName(fmi2GetTypesPlatform)
#define fmi2GetVersion               fmi2FullName(fmi2GetVersion)
#define fmi2SetDebugLogging          fmi2FullName(fmi2SetDebugLogging)
#define fmi2Instantiate              fmi2FullName(fmi2Instantiate)
#define fmi2FreeInstance             fmi2FullName(fmi2FreeInstance)
#define fmi2SetupExperiment          fmi2FullName(fmi2SetupExperiment)
#define fmi2EnterInitializationMode  fmi2FullName(fmi2EnterInitializationMode)
#define fmi2ExitInitializationMode   fmi2FullName(fmi2ExitInitializationMode)
#define fmi2Terminate                fmi2FullName(fmi2Terminate)
#define fmi2Reset                    fmi2FullName(fmi2Reset)
#define fmi2GetReal                  fmi2FullName(fmi2GetReal)
#define fmi2GetInteger               fmi2FullName(fmi2GetInteger)
#define fmi2GetBoolean               fmi2FullName(fmi2GetBoolean)
#define fmi2GetString                fmi2FullName(fmi2GetString)
#define fmi2SetReal                  fmi2FullName(fmi2SetReal)
#define fmi2SetInteger               fmi2FullName(fmi2SetInteger)
#define fmi2SetBoolean               fmi2FullName(fmi2SetBoolean)
#define fmi2SetString                fmi2FullName(fmi2SetString)
#define fmi2GetFMUstate              fmi2FullName(fmi2GetFMUstate)
#define fmi2SetFMUstate              fmi2FullName(fmi2SetFMUstate)
#define fmi2FreeFMUstate             fmi2FullName(fmi2FreeFMUstate)
#define fmi2SerializedFMUstateSize   fmi2FullName(fmi2SerializedFMUstateSize)
#define fmi2SerializeFMUstate        fmi2FullName(fmi2SerializeFMUstate)
#define fmi2DeSerializeFMUstate      fmi2FullName(fmi2DeSerializeFMUstate)
#define fmi2GetDirectionalDerivative fmi2FullName(fmi2GetDirectionalDerivative)


/***************************************************
Functions for FMI2 for Model Exchange
****************************************************/
#define fmi2EnterEventMode                fmi2FullName(fmi2EnterEventMode)
#define fmi2NewDiscreteStates             fmi2FullName(fmi2NewDiscreteStates)
#define fmi2EnterContinuousTimeMode       fmi2FullName(fmi2EnterContinuousTimeMode)
#define fmi2CompletedIntegratorStep       fmi2FullName(fmi2CompletedIntegratorStep)
#define fmi2SetTime                       fmi2FullName(fmi2SetTime)
#define fmi2SetContinuousStates           fmi2FullName(fmi2SetContinuousStates)
#define fmi2GetDerivatives                fmi2FullName(fmi2GetDerivatives)
#define fmi2GetEventIndicators            fmi2FullName(fmi2GetEventIndicators)
#define fmi2GetContinuousStates           fmi2FullName(fmi2GetContinuousStates)
#define fmi2GetNominalsOfContinuousStates fmi2FullName(fmi2GetNominalsOfContinuousStates)


/***************************************************
Functions for FMI2 for Co-Simulation
****************************************************/
#define fmi2SetRealInputDerivatives      fmi2FullName(fmi2SetRealInputDerivatives)
#define fmi2GetRealOutputDerivatives     fmi2FullName(fmi2GetRealOutputDerivatives)
#define fmi2DoStep                       fmi2FullName(fmi2DoStep)
#define fmi2CancelStep                   fmi2FullName(fmi2CancelStep)
#define fmi2GetStatus                    fmi2FullName(fmi2GetStatus)
#define fmi2GetRealStatus                fmi2FullName(fmi2GetRealStatus)
#define fmi2GetIntegerStatus             fmi2FullName(fmi2GetIntegerStatus)
#define fmi2GetBooleanStatus             fmi2FullName(fmi2GetBooleanStatus)
#define fmi2GetStringStatus              fmi2FullName(fmi2GetStringStatus)

/* Version number */
#define fmi2Version "2.0"


/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */
   FMI2_Export fmi2GetTypesPlatformTYPE fmi2GetTypesPlatform;
   FMI2_Export fmi2GetVersionTYPE       fmi2GetVersion;
   FMI2_Export fmi2SetDebugLoggingTYPE  fmi2SetDebugLogging;

/* Creation and destruction of FMU instances */
   FMI2_Export fmi2InstantiateTYPE  fmi2Instantiate;
   FMI2_Export fmi2FreeInstanceTYPE fmi2FreeInstance;

/* Enter and exit initialization mode, terminate and reset */
   FMI2_Export fmi2SetupExperimentTYPE         fmi2SetupExperiment;
   FMI2_Export fmi2EnterInitializationModeTYPE fmi2EnterInitializationMode;
   FMI2_Export fmi2ExitInitializationModeTYPE  fmi2ExitInitializationMode;
   FMI2_Export fmi2TerminateTYPE               fmi2Terminate;
   FMI2_Export fmi2ResetTYPE                   fmi2Reset;

/* Getting and setting variables values */
   FMI2_Export fmi2GetRealTYPE    fmi2GetReal;
   FMI2_Export fmi2GetIntegerTYPE fmi2GetInteger;
   FMI2_Export fmi2GetBooleanTYPE fmi2GetBoolean;
   FMI2_Export fmi2GetStringTYPE  fmi2GetString;

   FMI2_Export fmi2SetRealTYPE    fmi2SetReal;
   FMI2_Export fmi2SetIntegerTYPE fmi2SetInteger;
   FMI2_Export fmi2SetBooleanTYPE fmi2SetBoolean;
   FMI2_Export fmi2SetStringTYPE  fmi2SetString;

/* Getting and setting the internal FMU state */
   FMI2_Export fmi2GetFMUstateTYPE            fmi2GetFMUstate;
   FMI2_Export fmi2SetFMUstateTYPE            fmi2SetFMUstate;
   FMI2_Export fmi2FreeFMUstateTYPE           fmi2FreeFMUstate;
   FMI2_Export fmi2SerializedFMUstateSizeTYPE fmi2SerializedFMUstateSize;
   FMI2_Export fmi2SerializeFMUstateTYPE      fmi2SerializeFMUstate;
   FMI2_Export fmi2DeSerializeFMUstateTYPE    fmi2DeSerializeFMUstate;

/* Getting partial derivatives */
   FMI2_Export fmi2GetDirectionalDerivativeTYPE fmi2GetDirectionalDerivative;


/***************************************************
Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
   FMI2_Export fmi2EnterEventModeTYPE               fmi2EnterEventMode;
   FMI2_Export fmi2NewDiscreteStatesTYPE            fmi2NewDiscreteStates;
   FMI2_Export fmi2EnterContinuousTimeModeTYPE      fmi2EnterContinuousTimeMode;
   FMI2_Export fmi2CompletedIntegratorStepTYPE      fmi2CompletedIntegratorStep;

/* Providing independent variables and re-initialization of caching */
   FMI2_Export fmi2SetTimeTYPE             fmi2SetTime;
   FMI2_Export fmi2SetContinuousStatesTYPE fmi2SetContinuousStates;

/* Evaluation of the model equations */
   FMI2_Export fmi2GetDerivativesTYPE                fmi2GetDerivatives;
   FMI2_Export fmi2GetEventIndicatorsTYPE            fmi2GetEventIndicators;
   FMI2_Export fmi2GetContinuousStatesTYPE           fmi2GetContinuousStates;
   FMI2_Export fmi2GetNominalsOfContinuousStatesTYPE fmi2GetNominalsOfContinuousStates;


/***************************************************
Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
   FMI2_Export fmi2SetRealInputDerivativesTYPE  fmi2SetRealInputDerivatives;
   FMI2_Export fmi2GetRealOutputDerivativesTYPE fmi2GetRealOutputDerivatives;

   FMI2_Export fmi2DoStepTYPE     fmi2DoStep;
   FMI2_Export fmi2CancelStepTYPE fmi2CancelStep;

/* Inquire slave status */
   FMI2_Export fmi2GetStatusTYPE        fmi2GetStatus;
   FMI2_Export fmi2GetRealStatusTYPE    fmi2GetRealStatus;
   FMI2_Export fmi2GetIntegerStatusTYPE fmi2GetIntegerStatus;
   FMI2_Export fmi2GetBooleanStatusTYPE fmi2GetBooleanStatus;
   FMI2_Export fmi2GetStringStatusTYPE  fmi2GetStringStatus;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi2Functions_h */

)code";


/**
 * @brief FMI2 Function Type header template. File 'fmi2FunctionTypes.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFMI2FunctionTypesHeaderTemplate[] = R"code(#ifndef fmi2FunctionTypes_h
#define fmi2FunctionTypes_h

#include "fmi2TypesPlatform.h"

/* This header file must be utilized when compiling an FMU or an FMI master.
   It declares data and function types for FMI 2.0.1

   Revisions:
   - Sep. 30, 2019: License changed to 2-clause BSD License (without extensions)
   - Jul.  5, 2019: Remove const modifier from fields of fmi2CallbackFunctions  (#216)
   - Sep.  6, 2018: Parameter names added to function prototypes
   - Apr.  9, 2014: all prefixes "fmi" renamed to "fmi2" (decision from April 8)
   - Apr.  3, 2014: Added #include <stddef.h> for size_t definition
   - Mar. 27, 2014: Added #include "fmiTypesPlatform.h" (#179)
   - Mar. 26, 2014: Introduced function argument "void" for the functions (#171)
                      fmiGetTypesPlatformTYPE and fmiGetVersionTYPE
   - Oct. 11, 2013: Functions of ModelExchange and CoSimulation merged:
                      fmiInstantiateModelTYPE , fmiInstantiateSlaveTYPE  -> fmiInstantiateTYPE
                      fmiFreeModelInstanceTYPE, fmiFreeSlaveInstanceTYPE -> fmiFreeInstanceTYPE
                      fmiEnterModelInitializationModeTYPE, fmiEnterSlaveInitializationModeTYPE -> fmiEnterInitializationModeTYPE
                      fmiExitModelInitializationModeTYPE , fmiExitSlaveInitializationModeTYPE  -> fmiExitInitializationModeTYPE
                      fmiTerminateModelTYPE , fmiTerminateSlaveTYPE  -> fmiTerminate
                      fmiResetSlave -> fmiReset (now also for ModelExchange and not only for CoSimulation)
                    Functions renamed
                      fmiUpdateDiscreteStatesTYPE -> fmiNewDiscreteStatesTYPE
                    Renamed elements of the enumeration fmiEventInfo
                      upcomingTimeEvent             -> nextEventTimeDefined // due to generic naming scheme: varDefined + var
                      newUpdateDiscreteStatesNeeded -> newDiscreteStatesNeeded;
   - June 13, 2013: Changed type fmiEventInfo
                    Functions removed:
                       fmiInitializeModelTYPE
                       fmiEventUpdateTYPE
                       fmiCompletedEventIterationTYPE
                       fmiInitializeSlaveTYPE
                    Functions added:
                       fmiEnterModelInitializationModeTYPE
                       fmiExitModelInitializationModeTYPE
                       fmiEnterEventModeTYPE
                       fmiUpdateDiscreteStatesTYPE
                       fmiEnterContinuousTimeModeTYPE
                       fmiEnterSlaveInitializationModeTYPE;
                       fmiExitSlaveInitializationModeTYPE;
   - Feb. 17, 2013: Added third argument to fmiCompletedIntegratorStepTYPE
                    Changed function name "fmiTerminateType" to "fmiTerminateModelType" (due to #113)
                    Changed function name "fmiGetNominalContinuousStateTYPE" to
                                          "fmiGetNominalsOfContinuousStatesTYPE"
                    Removed fmiGetStateValueReferencesTYPE.
   - Nov. 14, 2011: First public Version


   Copyright (C) 2008-2011 MODELISAR consortium,
                 2012-2019 Modelica Association Project "FMI"
                 All rights reserved.

   This file is licensed by the copyright holders under the 2-Clause BSD License
   (https://opensource.org/licenses/BSD-2-Clause):

   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/* make sure all compiler use the same alignment policies for structures */
#if defined _MSC_VER || defined __GNUC__
#pragma pack(push,8)
#endif

/* Include stddef.h, in order that size_t etc. is defined */
#include <stddef.h>


/* Type definitions */
typedef enum {
    fmi2OK,
    fmi2Warning,
    fmi2Discard,
    fmi2Error,
    fmi2Fatal,
    fmi2Pending
} fmi2Status;

typedef enum {
    fmi2ModelExchange,
    fmi2CoSimulation
} fmi2Type;

typedef enum {
    fmi2DoStepStatus,
    fmi2PendingStatus,
    fmi2LastSuccessfulTime,
    fmi2Terminated
} fmi2StatusKind;

typedef void      (*fmi2CallbackLogger)        (fmi2ComponentEnvironment componentEnvironment,
                                                fmi2String instanceName,
                                                fmi2Status status,
                                                fmi2String category,
                                                fmi2String message,
                                                ...);
typedef void*     (*fmi2CallbackAllocateMemory)(size_t nobj, size_t size);
typedef void      (*fmi2CallbackFreeMemory)    (void* obj);
typedef void      (*fmi2StepFinished)          (fmi2ComponentEnvironment componentEnvironment,
                                                fmi2Status status);

typedef struct {
   fmi2CallbackLogger         logger;
   fmi2CallbackAllocateMemory allocateMemory;
   fmi2CallbackFreeMemory     freeMemory;
   fmi2StepFinished           stepFinished;
   fmi2ComponentEnvironment   componentEnvironment;
} fmi2CallbackFunctions;

typedef struct {
   fmi2Boolean newDiscreteStatesNeeded;
   fmi2Boolean terminateSimulation;
   fmi2Boolean nominalsOfContinuousStatesChanged;
   fmi2Boolean valuesOfContinuousStatesChanged;
   fmi2Boolean nextEventTimeDefined;
   fmi2Real    nextEventTime;
} fmi2EventInfo;


/* reset alignment policy to the one set before reading this file */
#if defined _MSC_VER || defined __GNUC__
#pragma pack(pop)
#endif


/* Define fmi2 function pointer types to simplify dynamic loading */

/***************************************************
Types for Common Functions
****************************************************/

/* Inquire version numbers of header files and setting logging status */
    typedef const char* fmi2GetTypesPlatformTYPE(void);
    typedef const char* fmi2GetVersionTYPE(void);
    typedef fmi2Status  fmi2SetDebugLoggingTYPE(fmi2Component c,
                                                fmi2Boolean loggingOn,
                                                size_t nCategories,
                                                const fmi2String categories[]);

/* Creation and destruction of FMU instances and setting debug status */
    typedef fmi2Component fmi2InstantiateTYPE(fmi2String instanceName,
                                              fmi2Type fmuType,
                                              fmi2String fmuGUID,
                                              fmi2String fmuResourceLocation,
                                              const fmi2CallbackFunctions* functions,
                                              fmi2Boolean visible,
                                              fmi2Boolean loggingOn);
   typedef void          fmi2FreeInstanceTYPE(fmi2Component c);

/* Enter and exit initialization mode, terminate and reset */
    typedef fmi2Status fmi2SetupExperimentTYPE       (fmi2Component c,
                                                      fmi2Boolean toleranceDefined,
                                                      fmi2Real tolerance,
                                                      fmi2Real startTime,
                                                      fmi2Boolean stopTimeDefined,
                                                      fmi2Real stopTime);
   typedef fmi2Status fmi2EnterInitializationModeTYPE(fmi2Component c);
   typedef fmi2Status fmi2ExitInitializationModeTYPE (fmi2Component c);
   typedef fmi2Status fmi2TerminateTYPE              (fmi2Component c);
   typedef fmi2Status fmi2ResetTYPE                  (fmi2Component c);

/* Getting and setting variable values */
   typedef fmi2Status fmi2GetRealTYPE   (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real    value[]);
   typedef fmi2Status fmi2GetIntegerTYPE(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]);
   typedef fmi2Status fmi2GetBooleanTYPE(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]);
   typedef fmi2Status fmi2GetStringTYPE (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]);

   typedef fmi2Status fmi2SetRealTYPE   (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real    value[]);
   typedef fmi2Status fmi2SetIntegerTYPE(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]);
   typedef fmi2Status fmi2SetBooleanTYPE(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]);
   typedef fmi2Status fmi2SetStringTYPE (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]);

/* Getting and setting the internal FMU state */
   typedef fmi2Status fmi2GetFMUstateTYPE           (fmi2Component c, fmi2FMUstate* FMUstate);
   typedef fmi2Status fmi2SetFMUstateTYPE           (fmi2Component c, fmi2FMUstate  FMUstate);
   typedef fmi2Status fmi2FreeFMUstateTYPE          (fmi2Component c, fmi2FMUstate* FMUstate);
   typedef fmi2Status fmi2SerializedFMUstateSizeTYPE(fmi2Component c, fmi2FMUstate  FMUstate, size_t* size);
   typedef fmi2Status fmi2SerializeFMUstateTYPE     (fmi2Component c, fmi2FMUstate  FMUstate, fmi2Byte[], size_t size);
   typedef fmi2Status fmi2DeSerializeFMUstateTYPE   (fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate);

/* Getting partial derivatives */
    typedef fmi2Status fmi2GetDirectionalDerivativeTYPE(fmi2Component c,
                                                        const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                                        const fmi2ValueReference vKnown_ref[],   size_t nKnown,
                                                        const fmi2Real dvKnown[],
                                                        fmi2Real dvUnknown[]);

/***************************************************
Types for Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
   typedef fmi2Status fmi2EnterEventModeTYPE         (fmi2Component c);
   typedef fmi2Status fmi2NewDiscreteStatesTYPE      (fmi2Component c, fmi2EventInfo* fmi2eventInfo);
   typedef fmi2Status fmi2EnterContinuousTimeModeTYPE(fmi2Component c);
   typedef fmi2Status fmi2CompletedIntegratorStepTYPE(fmi2Component c,
                                                      fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
                                                      fmi2Boolean*  enterEventMode,
                                                      fmi2Boolean*  terminateSimulation);

/* Providing independent variables and re-initialization of caching */
   typedef fmi2Status fmi2SetTimeTYPE            (fmi2Component c, fmi2Real time);
   typedef fmi2Status fmi2SetContinuousStatesTYPE(fmi2Component c, const fmi2Real x[], size_t nx);

/* Evaluation of the model equations */
   typedef fmi2Status fmi2GetDerivativesTYPE               (fmi2Component c, fmi2Real derivatives[],     size_t nx);
   typedef fmi2Status fmi2GetEventIndicatorsTYPE           (fmi2Component c, fmi2Real eventIndicators[], size_t ni);
   typedef fmi2Status fmi2GetContinuousStatesTYPE          (fmi2Component c, fmi2Real x[],               size_t nx);
   typedef fmi2Status fmi2GetNominalsOfContinuousStatesTYPE(fmi2Component c, fmi2Real x_nominal[],       size_t nx);


/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
    typedef fmi2Status fmi2SetRealInputDerivativesTYPE (fmi2Component c,
                                                        const fmi2ValueReference vr[], size_t nvr,
                                                        const fmi2Integer order[],
                                                        const fmi2Real value[]);
    typedef fmi2Status fmi2GetRealOutputDerivativesTYPE(fmi2Component c,
                                                        const fmi2ValueReference vr[], size_t nvr,
                                                        const fmi2Integer order[],
                                                        fmi2Real value[]);
    typedef fmi2Status fmi2DoStepTYPE   (fmi2Component c,
                                         fmi2Real      currentCommunicationPoint,
                                         fmi2Real      communicationStepSize,
                                         fmi2Boolean   noSetFMUStatePriorToCurrentPoint);
   typedef fmi2Status fmi2CancelStepTYPE(fmi2Component c);

/* Inquire slave status */
   typedef fmi2Status fmi2GetStatusTYPE       (fmi2Component c, const fmi2StatusKind s, fmi2Status*  value);
   typedef fmi2Status fmi2GetRealStatusTYPE   (fmi2Component c, const fmi2StatusKind s, fmi2Real*    value);
   typedef fmi2Status fmi2GetIntegerStatusTYPE(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value);
   typedef fmi2Status fmi2GetBooleanStatusTYPE(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value);
   typedef fmi2Status fmi2GetStringStatusTYPE (fmi2Component c, const fmi2StatusKind s, fmi2String*  value);


#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi2FunctionTypes_h */

)code";


/**
 * @brief FMI2 Types platform header template. File 'fmi2TypesPlatform.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFMI2TypesPlatformHeaderTemplate[] = R"code(#ifndef fmi2TypesPlatform_h
#define fmi2TypesPlatform_h

/* Standard header file to define the argument types of the
   functions of the Functional Mock-up Interface 2.0.1
   This header file must be utilized both by the model and
   by the simulation engine.

   Revisions:
   - Sep. 29, 2019:  License changed to 2-clause BSD License (without extensions)
   - Apr.  9, 2014:  All prefixes "fmi" renamed to "fmi2" (decision from April 8)
   - Mar   31, 2014: New datatype fmiChar introduced.
   - Feb.  17, 2013: Changed fmiTypesPlatform from "standard32" to "default".
                     Removed fmiUndefinedValueReference since no longer needed
                     (because every state is defined in ScalarVariables).
   - March 20, 2012: Renamed from fmiPlatformTypes.h to fmiTypesPlatform.h
   - Nov.  14, 2011: Use the header file "fmiPlatformTypes.h" for FMI 2.0
                     both for "FMI for model exchange" and for "FMI for co-simulation"
                     New types "fmiComponentEnvironment", "fmiState", and "fmiByte".
                     The implementation of "fmiBoolean" is change from "char" to "int".
                     The #define "fmiPlatform" changed to "fmiTypesPlatform"
                     (in order that #define and function call are consistent)
   - Oct.   4, 2010: Renamed header file from "fmiModelTypes.h" to fmiPlatformTypes.h"
                     for the co-simulation interface
   - Jan.   4, 2010: Renamed meModelTypes_h to fmiModelTypes_h (by Mauss, QTronic)
   - Dec.  21, 2009: Changed "me" to "fmi" and "meModel" to "fmiComponent"
                     according to meeting on Dec. 18 (by Martin Otter, DLR)
   - Dec.   6, 2009: Added meUndefinedValueReference (by Martin Otter, DLR)
   - Sept.  9, 2009: Changes according to FMI-meeting on July 21:
                     Changed "version" to "platform", "standard" to "standard32",
                     Added a precise definition of "standard32" as comment
                     (by Martin Otter, DLR)
   - July  19, 2009: Added "me" as prefix to file names, added meTrue/meFalse,
                     and changed meValueReferenced from int to unsigned int
                     (by Martin Otter, DLR).
   - March  2, 2009: Moved enums and function pointer definitions to
                     ModelFunctions.h (by Martin Otter, DLR).
   - Dec.  3, 2008 : First version by Martin Otter (DLR) and
                     Hans Olsson (Dynasim).


   Copyright (C) 2008-2011 MODELISAR consortium,
                 2012-2019 Modelica Association Project "FMI"
                 All rights reserved.

   This file is licensed by the copyright holders under the 2-Clause BSD License
   (https://opensource.org/licenses/BSD-2-Clause):

   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

/* Platform (unique identification of this header file) */
#define fmi2TypesPlatform "default"

/* Type definitions of variables passed as arguments
   Version "default" means:

   fmi2Component           : an opaque object pointer
   fmi2ComponentEnvironment: an opaque object pointer
   fmi2FMUstate            : an opaque object pointer
   fmi2ValueReference      : handle to the value of a variable
   fmi2Real                : double precision floating-point data type
   fmi2Integer             : basic signed integer data type
   fmi2Boolean             : basic signed integer data type
   fmi2Char                : character data type
   fmi2String              : a pointer to a vector of fmi2Char characters
                             ('\0' terminated, UTF8 encoded)
   fmi2Byte                : smallest addressable unit of the machine, typically one byte.
*/
   typedef void*           fmi2Component;               /* Pointer to FMU instance       */
   typedef void*           fmi2ComponentEnvironment;    /* Pointer to FMU environment    */
   typedef void*           fmi2FMUstate;                /* Pointer to internal FMU state */
   typedef unsigned int    fmi2ValueReference;
   typedef double          fmi2Real   ;
   typedef int             fmi2Integer;
   typedef int             fmi2Boolean;
   typedef char            fmi2Char;
   typedef const fmi2Char* fmi2String;
   typedef char            fmi2Byte;

/* Values for fmi2Boolean  */
#define fmi2True  1
#define fmi2False 0


#endif /* fmi2TypesPlatform_h */

)code";


/**
 * @brief Model header template. File 'model.h'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szModelHeaderTemplate[] = R"code(#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#if FMI_VERSION != 1 && FMI_VERSION != 2 && FMI_VERSION != 3
#error FMI_VERSION must be one of 1, 2 or 3
#endif

#define UNUSED(x) (void)(x)

#include <stddef.h>  // for size_t
#include <stdbool.h> // for bool
#include <stdint.h>

#include "config.h"

#if FMI_VERSION == 1

#define not_modelError (Instantiated| Initialized | Terminated)

    typedef enum {
        Instantiated = 1 << 0,
        Initialized = 1 << 1,
        Terminated = 1 << 2,
        modelError = 1 << 3
    } ModelState;

#elif FMI_VERSION == 2

    typedef enum {
        StartAndEnd = 1 << 0,
        Instantiated = 1 << 1,
        InitializationMode = 1 << 2,

        // ME states
        EventMode = 1 << 3,
        ContinuousTimeMode = 1 << 4,

        // CS states
        StepComplete = 1 << 5,
        StepInProgress = 1 << 6,
        StepFailed = 1 << 7,
        StepCanceled = 1 << 8,

        Terminated = 1 << 9,
        modelError = 1 << 10,
        modelFatal = 1 << 11,
    } ModelState;

#else

    typedef enum {
        StartAndEnd = 1 << 0,
        ConfigurationMode = 1 << 1,
        Instantiated = 1 << 2,
        InitializationMode = 1 << 3,
        EventMode = 1 << 4,
        ContinuousTimeMode = 1 << 5,
        StepMode = 1 << 6,
        ClockActivationMode = 1 << 7,
        StepDiscarded = 1 << 8,
        ReconfigurationMode = 1 << 9,
        IntermediateUpdateMode = 1 << 10,
        Terminated = 1 << 11,
        modelError = 1 << 12,
        modelFatal = 1 << 13,
    } ModelState;

#endif

    typedef enum {
        ModelExchange,
        CoSimulation,
        ScheduledExecution,
    } InterfaceType;

    typedef enum {
        OK,
        Warning,
        Discard,
        Error,
        Fatal,
        Pending
    } Status;

#if FMI_VERSION < 3
    typedef void (*loggerType) (void* componentEnvironment, const char* instanceName, int status, const char* category, const char* message, ...);
#else
    typedef void (*loggerType) (void* componentEnvironment, int status, const char* category, const char* message);
#endif

    typedef void (*lockPreemptionType)   (void);
    typedef void (*unlockPreemptionType) (void);


    typedef void (*intermediateUpdateType) (void* instanceEnvironment,
        double intermediateUpdateTime,
        bool intermediateVariableSetRequested,
        bool intermediateVariableGetAllowed,
        bool intermediateStepFinished,
        bool canReturnEarly,
        bool* earlyReturnRequested,
        double* earlyReturnTime);

    typedef void(*clockUpdateType) (void* instanceEnvironment);

    typedef struct {

        double startTime;
        double stopTime;
        double time;
        const char* instanceName;
        InterfaceType type;
        const char* resourceLocation;

        Status status;

        // callback functions
        loggerType logger;
        intermediateUpdateType intermediateUpdate;
        clockUpdateType clockUpdate;

        lockPreemptionType lockPreemtion;
        unlockPreemptionType unlockPreemtion;

        bool logEvents;
        bool logErrors;

        void* componentEnvironment;
        ModelState state;

        // event info
        bool newDiscreteStatesNeeded;
        bool terminateSimulation;
        bool nominalsOfContinuousStatesChanged;
        bool valuesOfContinuousStatesChanged;
        bool nextEventTimeDefined;
        double nextEventTime;
        bool clocksTicked;

        bool isDirtyValues;

        ModelData modelData;

#if NZ > 0
        // event indicators
        double z[NZ];
#endif

        // internal solver steps
        uint64_t nSteps;

        // Co-Simulation
        bool earlyReturnAllowed;
        bool eventModeUsed;

    } ModelInstance;

    ModelInstance* createModelInstance(
        loggerType logger,
        intermediateUpdateType intermediateUpdate,
        void* componentEnvironment,
        const char* instanceName,
        const char* instantiationToken,
        const char* resourceLocation,
        bool loggingOn,
        InterfaceType interfaceType);

    void freeModelInstance(ModelInstance* comp);

    void reset(ModelInstance* comp);


    bool setStartValues(ModelInstance* comp);

    Status calculateValues(ModelInstance* comp);

    Status getFloat32(ModelInstance* comp, ValueReference vr, float       values[], size_t nValues, size_t* index);
    Status getFloat64(ModelInstance* comp, ValueReference vr, double      values[], size_t nValues, size_t* index);
    Status getInt8(ModelInstance* comp, ValueReference vr, int8_t      values[], size_t nValues, size_t* index);
    Status getUInt8(ModelInstance* comp, ValueReference vr, uint8_t     values[], size_t nValues, size_t* index);
    Status getInt16(ModelInstance* comp, ValueReference vr, int16_t     values[], size_t nValues, size_t* index);
    Status getUInt16(ModelInstance* comp, ValueReference vr, uint16_t    values[], size_t nValues, size_t* index);
    Status getInt32(ModelInstance* comp, ValueReference vr, int32_t     values[], size_t nValues, size_t* index);
    Status getUInt32(ModelInstance* comp, ValueReference vr, uint32_t    values[], size_t nValues, size_t* index);
    Status getInt64(ModelInstance* comp, ValueReference vr, int64_t     values[], size_t nValues, size_t* index);
    Status getUInt64(ModelInstance* comp, ValueReference vr, uint64_t    values[], size_t nValues, size_t* index);
    Status getBoolean(ModelInstance* comp, ValueReference vr, bool        values[], size_t nValues, size_t* index);
    Status getString(ModelInstance* comp, ValueReference vr, const char* values[], size_t nValues, size_t* index);
    Status getBinary(ModelInstance* comp, ValueReference vr, size_t sizes[], const char* values[], size_t nValues, size_t* index);

    Status setFloat32(ModelInstance* comp, ValueReference vr, const float       values[], size_t nValues, size_t* index);
    Status setFloat64(ModelInstance* comp, ValueReference vr, const double      values[], size_t nValues, size_t* index);
    Status setInt8(ModelInstance* comp, ValueReference vr, const int8_t      values[], size_t nValues, size_t* index);
    Status setUInt8(ModelInstance* comp, ValueReference vr, const uint8_t     values[], size_t nValues, size_t* index);
    Status setInt16(ModelInstance* comp, ValueReference vr, const int16_t     values[], size_t nValues, size_t* index);
    Status setUInt16(ModelInstance* comp, ValueReference vr, const uint16_t    values[], size_t nValues, size_t* index);
    Status setInt32(ModelInstance* comp, ValueReference vr, const int32_t     values[], size_t nValues, size_t* index);
    Status setUInt32(ModelInstance* comp, ValueReference vr, const uint32_t    values[], size_t nValues, size_t* index);
    Status setInt64(ModelInstance* comp, ValueReference vr, const int64_t     values[], size_t nValues, size_t* index);
    Status setUInt64(ModelInstance* comp, ValueReference vr, const uint64_t    values[], size_t nValues, size_t* index);
    Status setBoolean(ModelInstance* comp, ValueReference vr, const bool        values[], size_t nValues, size_t* index);
    Status setString(ModelInstance* comp, ValueReference vr, const char* const values[], size_t nValues, size_t* index);
    Status setBinary(ModelInstance* comp, ValueReference vr, const size_t sizes[], const char* const values[], size_t nValues, size_t* index);

    Status activateClock(ModelInstance* comp, ValueReference vr);
    Status getClock(ModelInstance* comp, ValueReference vr, bool* value);
    Status setClock(ModelInstance* comp, ValueReference vr, const bool* value);

    Status getInterval(ModelInstance* comp, ValueReference vr, double* interval, int* qualifier);

    Status activateModelPartition(ModelInstance* comp, ValueReference vr, double activationTime);

    void getContinuousStates(ModelInstance* comp, double x[], size_t nx);
    void setContinuousStates(ModelInstance* comp, const double x[], size_t nx);
    void getDerivatives(ModelInstance* comp, double dx[], size_t nx);
    Status getOutputDerivative(ModelInstance* comp, ValueReference valueReference, int order, double* value);
    Status getPartialDerivative(ModelInstance* comp, ValueReference unknown, ValueReference known, double* partialDerivative);
    void getEventIndicators(ModelInstance* comp, double z[], size_t nz);
    void eventUpdate(ModelInstance* comp);
    //void updateEventTime(ModelInstance *comp);

    bool invalidNumber(ModelInstance* comp, const char* f, const char* arg, size_t actual, size_t expected);
    bool invalidState(ModelInstance* comp, const char* f, int statesExpected);
    bool nullPointer(ModelInstance* comp, const char* f, const char* arg, const void* p);
    void logError(ModelInstance* comp, const char* message, ...);
    Status setDebugLogging(ModelInstance* comp, bool loggingOn, size_t nCategories, const char* const categories[]);
    void logEvent(ModelInstance* comp, const char* message, ...);
    void logError(ModelInstance* comp, const char* message, ...);

    void* getFMUState(ModelInstance* comp);
    void setFMUState(ModelInstance* comp, void* FMUState);

    // shorthand to access the variables
#define M(v) (comp->modelData.v)

// "stringification" macros
#define xstr(s) str(s)
#define str(s) #s

// assert size of nValues for scalar variables
#define ASSERT_NVALUES(N)  do { \
    if (*index + (N) > nValues) { \
        logError(comp, "Expected nValues > %zu but was %zu.", *index, nValues); \
        return Error; \
        } \
    } while (0)


#ifdef __cplusplus
} // end of extern "C"
#endif

)code";


/**
 * @brief CoSimulation template. File 'cosimulation.c'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szCosimulationSrcFileTemplate[] = R"code(#include <stdlib.h>  // for calloc(), free()
#include <float.h>   // for DBL_EPSILON
#include <math.h>    // for fabs()
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "config.h"
#include "cosimulation.h"

#if FMI_VERSION == 3
#include "fmi3Functions.h"
#endif

#ifdef _MSC_VER
#define strdup _strdup
#endif


ModelInstance *createModelInstance(
    loggerType cbLogger,
    intermediateUpdateType intermediateUpdate,
    void *componentEnvironment,
    const char *instanceName,
    const char *instantiationToken,
    const char *resourceLocation,
    bool loggingOn,
    InterfaceType interfaceType) {

    ModelInstance *comp = NULL;

    if (!instanceName || strlen(instanceName) == 0) {
        if (cbLogger) {
#if FMI_VERSION < 3
            cbLogger(componentEnvironment, "?", Error, "error", "Missing instance name.");
#else
            cbLogger(componentEnvironment, Error, "error", "Missing instance name.");
#endif
        }
        return NULL;
    }

    if (!instantiationToken || strlen(instantiationToken) == 0) {
        if (cbLogger) {
#if FMI_VERSION < 3
            cbLogger(componentEnvironment, instanceName, Error, "error", "Missing GUID.");
#else
            cbLogger(componentEnvironment, Error, "error", "Missing instantiationToken.");
#endif
        }
        return NULL;
    }

    if (strcmp(instantiationToken, INSTANTIATION_TOKEN)) {
        if (cbLogger) {
#if FMI_VERSION < 3
            cbLogger(componentEnvironment, instanceName, Error, "error", "Wrong GUID.");
#else
            cbLogger(componentEnvironment, Error, "error", "Wrong instantiationToken.");
#endif
        }
        return NULL;
    }



    comp = (ModelInstance *)calloc(1, sizeof(ModelInstance));

    if (comp) {
        comp->componentEnvironment = componentEnvironment;
        comp->logger               = cbLogger;
        comp->intermediateUpdate   = intermediateUpdate;
        comp->lockPreemtion        = NULL;
        comp->unlockPreemtion      = NULL;
        comp->instanceName         = strdup(instanceName);
        comp->resourceLocation     = resourceLocation ? strdup(resourceLocation) : NULL;
        comp->status               = OK;
        comp->logEvents            = loggingOn;
        comp->logErrors            = true; // always log errors
        comp->nSteps               = 0;
        comp->earlyReturnAllowed   = false;
        comp->eventModeUsed        = false;
    }

    if (!comp || !comp->instanceName) {
        logError(comp, "Out of memory.");
        return NULL;
    }

    comp->time                              = 0.0;  // overwrite in fmi*SetupExperiment, fmi*SetTime
    comp->type                              = interfaceType;

    comp->state                             = Instantiated;

    comp->newDiscreteStatesNeeded           = false;
    comp->terminateSimulation               = false;
    comp->nominalsOfContinuousStatesChanged = false;
    comp->valuesOfContinuousStatesChanged   = false;
    comp->nextEventTimeDefined              = false;
    comp->nextEventTime                     = 0;

    if (!setStartValues(comp))
    {
        cbLogger(componentEnvironment, instanceName, Error, "error", "VAPI initialization failed.");
    }

    comp->isDirtyValues = true;


    return comp;
}

void freeModelInstance(ModelInstance *comp) {
    free((void *)comp->instanceName);
    free(comp);
}

void reset(ModelInstance* comp) {
    comp->state = Instantiated;
    comp->startTime = 0.0;
    comp->time = 0.0;
    comp->nSteps = 0;
    comp->status = OK;
    setStartValues(comp);
    comp->isDirtyValues = true;
}

bool invalidNumber(ModelInstance *comp, const char *f, const char *arg, size_t actual, size_t expected) {

    if (actual != expected) {
        comp->state = modelError;
        logError(comp, "%s: Invalid argument %s = %d. Expected %d.", f, arg, actual, expected);
        return true;
    }

    return false;
}

bool invalidState(ModelInstance *comp, const char *f, int statesExpected) {

    UNUSED(f);
    UNUSED(statesExpected);

    if (!comp) {
        return true;
    }

    // TODO: add missing states and check state
    return false;

//    if (!(comp->state & statesExpected)) {
//        comp->state = modelError;
//        logError(comp, "%s: Illegal call sequence.", f);
//        return true;
//    }
//
//    return false;
}

bool nullPointer(ModelInstance* comp, const char *f, const char *arg, const void *p) {

    if (!p) {
        comp->state = modelError;
        logError(comp, "%s: Invalid argument %s = NULL.", f, arg);
        return true;
    }

    return false;
}

Status setDebugLogging(ModelInstance *comp, bool loggingOn, size_t nCategories, const char * const categories[]) {

    if (nCategories > 0) {

        if (categories == NULL) {
            logError(comp, "Argument categories must not be NULL.");
            return Error;
        }

        for (size_t i = 0; i < nCategories; i++) {

            if (categories[i] == NULL) {
                logError(comp, "Argument categories[%zu] must not be NULL.", i);
                return Error;
            } else if (strcmp(categories[i], "logEvents") == 0) {
                comp->logEvents = loggingOn;
            } else if (strcmp(categories[i], "logStatusError") == 0) {
                comp->logErrors = loggingOn;
            } else {
                logError(comp, "Log categories[%zu] must be one of \"logEvents\" or \"logStatusError\" but was \"%s\".", i, categories[i]);
                return Error;
            }
        }

    } else {

        comp->logEvents = loggingOn;
        comp->logErrors = loggingOn;

    }

    return OK;
}

static void logMessage(ModelInstance *comp, int status, const char *category, const char *message, va_list args) {

    if (!comp->logger) {
        return;
    }

    va_list args1;
    int len = 0;
    char *buf = "";

    va_copy(args1, args);
    len = vsnprintf(buf, len, message, args1);
    va_end(args1);

    if (len < 0) {
        return;
    }

    va_copy(args1, args);
    buf = (char *)calloc(len + 1, sizeof(char));
    len = vsnprintf(buf, len + 1, message, args);
    va_end(args1);

    if (len >= 0) {
        // no need to distinguish between FMI versions since we're not using variadic arguments
#if FMI_VERSION < 3
        comp->logger(comp->componentEnvironment, comp->instanceName, status, category, buf);
#else
        comp->logger(comp->componentEnvironment, status, category, buf);
#endif
    }

    free(buf);
}

void logEvent(ModelInstance *comp, const char *message, ...) {

    if (!comp || !comp->logEvents) return;

    va_list args;
    va_start(args, message);
    logMessage(comp, OK, "logEvents", message, args);
    va_end(args);
}

void logError(ModelInstance *comp, const char *message, ...) {

    if (!comp || !comp->logErrors) return;

    va_list args;
    va_start(args, message);
    logMessage(comp, Error, "logStatusError", message, args);
    va_end(args);
}

// default implementations
#if NZ < 1
void getEventIndicators(ModelInstance *comp, double z[], size_t nz) {
    UNUSED(comp);
    UNUSED(z);
    UNUSED(nz);
    // do nothing
}
#endif

#define GET_NOT_ALLOWED(t) do { \
    UNUSED(vr); \
    UNUSED(values); \
    UNUSED(nValues); \
    UNUSED(index); \
    logError(comp, "Getting " t " is not allowed.");\
    return Error; \
} while (false)

#ifndef GET_FLOAT32
Status getFloat32(ModelInstance* comp, ValueReference vr, float values[], size_t nValues, size_t* index) {
    GET_NOT_ALLOWED("Float32");
}
#endif

#ifndef GET_INT8
Status getInt8(ModelInstance* comp, ValueReference vr, int8_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("Int8");
}
#endif

#ifndef GET_UINT8
Status getUInt8(ModelInstance* comp, ValueReference vr, uint8_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("UInt8");
}
#endif

#ifndef GET_INT16
Status getInt16(ModelInstance* comp, ValueReference vr, int16_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("Int16");

}
#endif

#ifndef GET_UINT16
Status getUInt16(ModelInstance* comp, ValueReference vr, uint16_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("UInt16");
}
#endif

#ifndef GET_INT32
Status getInt32(ModelInstance* comp, ValueReference vr, int32_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("Int32");
}
#endif

#ifndef GET_UINT32
Status getUInt32(ModelInstance* comp, ValueReference vr, uint32_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("UInt32");
}
#endif

#ifndef GET_INT64
Status getInt64(ModelInstance* comp, ValueReference vr, int64_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("Int64");
}
#endif

#ifndef GET_UINT64
Status getUInt64(ModelInstance* comp, ValueReference vr, uint64_t values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("UInt64");
}
#endif

#ifndef GET_BOOLEAN
Status getBoolean(ModelInstance* comp, ValueReference vr, bool values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("Boolean");
}
#endif

#ifndef GET_STRING
Status getString(ModelInstance* comp, ValueReference vr, const char* values[], size_t nValues, size_t *index) {
    GET_NOT_ALLOWED("String");
}
#endif

#ifndef GET_BINARY
Status getBinary(ModelInstance* comp, ValueReference vr, size_t sizes[], const char* values[], size_t nValues, size_t *index) {
    UNUSED(sizes);
    GET_NOT_ALLOWED("Binary");
}
#endif

#define SET_NOT_ALLOWED(t) do { \
    UNUSED(vr); \
    UNUSED(values); \
    UNUSED(nValues); \
    UNUSED(index); \
    logError(comp, "Setting " t " is not allowed.");\
    return Error; \
} while (false)

#ifndef SET_FLOAT32
Status setFloat32(ModelInstance* comp, ValueReference vr, const float values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Float32");
}
#endif

#ifndef SET_FLOAT64
Status setFloat64(ModelInstance* comp, ValueReference vr, const double values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Float64");
}
#endif

#ifndef SET_INT8
Status setInt8(ModelInstance* comp, ValueReference vr, const int8_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Int8");
}
#endif

#ifndef SET_UINT8
Status setUInt8(ModelInstance* comp, ValueReference vr, const uint8_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("UInt8");
}
#endif

#ifndef SET_INT16
Status setInt16(ModelInstance* comp, ValueReference vr, const int16_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Int16");
}
#endif

#ifndef SET_UINT16
Status setUInt16(ModelInstance* comp, ValueReference vr, const uint16_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("UInt16");
}
#endif

#ifndef SET_INT32
Status setInt32(ModelInstance* comp, ValueReference vr, const int32_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Int32");
}
#endif

#ifndef SET_UINT32
Status setUInt32(ModelInstance* comp, ValueReference vr, const uint32_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("UInt32");
}
#endif

#ifndef SET_INT64
Status setInt64(ModelInstance* comp, ValueReference vr, const int64_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Int64");
}
#endif

#ifndef SET_UINT64
Status setUInt64(ModelInstance* comp, ValueReference vr, const uint64_t values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("UInt64");
}
#endif

#ifndef SET_BOOLEAN
Status setBoolean(ModelInstance* comp, ValueReference vr, const bool values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("Boolean");
}
#endif

#ifndef SET_STRING
Status setString(ModelInstance* comp, ValueReference vr, const char *const values[], size_t nValues, size_t* index) {
    SET_NOT_ALLOWED("String");
}
#endif

#ifndef SET_BINARY
Status setBinary(ModelInstance* comp, ValueReference vr, const size_t size[], const char *const values[], size_t nValues, size_t* index) {
    UNUSED(size);
    SET_NOT_ALLOWED("Binary");
}
#endif

#ifndef ACTIVATE_CLOCK
Status activateClock(ModelInstance* comp, ValueReference vr) {
    UNUSED(comp);
    UNUSED(vr);
    return Error;
}
#endif

#ifndef GET_CLOCK
Status getClock(ModelInstance* comp, ValueReference vr, bool* value) {
    UNUSED(comp);
    UNUSED(vr);
    UNUSED(value);
    return Error;
}
#endif

#ifndef GET_INTERVAL
Status getInterval(ModelInstance* comp, ValueReference vr, double* interval, int* qualifier) {
    UNUSED(comp);
    UNUSED(vr);
    UNUSED(interval);
    UNUSED(qualifier);
    return Error;
}
#endif

#ifndef ACTIVATE_MODEL_PARTITION
Status activateModelPartition(ModelInstance* comp, ValueReference vr, double activationTime) {
    UNUSED(comp);
    UNUSED(vr);
    UNUSED(activationTime);
    return Error;
}
#endif

#if NX < 1
void getContinuousStates(ModelInstance *comp, double x[], size_t nx) {
    UNUSED(comp);
    UNUSED(x);
    UNUSED(nx);
}

void setContinuousStates(ModelInstance *comp, const double x[], size_t nx) {
    UNUSED(comp);
    UNUSED(x);
    UNUSED(nx);
}

void getDerivatives(ModelInstance *comp, double dx[], size_t nx) {
    UNUSED(comp);
    UNUSED(dx);
    UNUSED(nx);
}
#endif

#ifndef GET_PARTIAL_DERIVATIVE
Status getPartialDerivative(ModelInstance *comp, ValueReference unknown, ValueReference known, double *partialDerivative) {
    UNUSED(comp);
    UNUSED(unknown);
    UNUSED(known);
    UNUSED(partialDerivative);
    logError(comp, "Directional derivatives are not supported.");
    return Error;
}
#endif

void* getFMUState(ModelInstance* comp) {

    ModelInstance* fmuState = (ModelInstance*)calloc(1, sizeof(ModelInstance));

    if (fmuState) {
        memcpy(fmuState, comp, sizeof(ModelInstance));
    }

    return fmuState;
}

void setFMUState(ModelInstance* comp, void* FMUState) {

    ModelInstance* s = (ModelInstance*)FMUState;

    comp->startTime = s->startTime;
    comp->stopTime = s->stopTime;
    comp->time = s->time;
    comp->status = s->status;
    comp->state = s->state;
    comp->newDiscreteStatesNeeded = s->newDiscreteStatesNeeded;
    comp->terminateSimulation = s->terminateSimulation;
    comp->nominalsOfContinuousStatesChanged = s->nominalsOfContinuousStatesChanged;
    comp->valuesOfContinuousStatesChanged = s->valuesOfContinuousStatesChanged;
    comp->nextEventTimeDefined = s->nextEventTimeDefined;
    comp->nextEventTime = s->nextEventTime;
    comp->clocksTicked = s->clocksTicked;
    comp->isDirtyValues = s->isDirtyValues;
    comp->modelData = s->modelData;
#if NZ > 0
    memcpy(comp->z, s->z, NZ * sizeof(double));
#endif
    comp->nSteps = s->nSteps;
}

void doFixedStep(ModelInstance *comp, bool* stateEvent, bool* timeEvent) {

#if NX > 0
    double  x[NX] = { 0 };
    double dx[NX] = { 0 };

    getContinuousStates(comp, x, NX);
    getDerivatives(comp, dx, NX);

    // forward Euler step
    for (int i = 0; i < NX; i++) {
        x[i] += FIXED_SOLVER_STEP * dx[i];
    }

    setContinuousStates(comp, x, NX);
#endif

    comp->nSteps++;

    comp->time = comp->startTime + comp->nSteps * FIXED_SOLVER_STEP;

    // state event
    *stateEvent = false;

#if NZ > 0
    double z[NZ] = { 0.0 };

    getEventIndicators(comp, z, NZ);

    // check for zero-crossings
    for (int i = 0; i < NZ; i++) {
        *stateEvent |= (comp->z[i] <= 0 && z[i] > 0) || (comp->z[i] > 0 && z[i] <= 0);
    }

    // remember the current event indicators
    memcpy(comp->z, z, sizeof(double) * NZ);
#endif

    // time event
    *timeEvent = comp->nextEventTimeDefined && comp->time >= comp->nextEventTime;

    bool earlyReturnRequested;
    double earlyReturnTime;

    // intermediate update
    if (comp->intermediateUpdate) {
        comp->intermediateUpdate(
            comp->componentEnvironment, // instanceEnvironment
            comp->time,                 // intermediateUpdateTime
            false,                      // intermediateVariableSetRequested
            true,                       // intermediateVariableGetAllowed
            true,                       // intermediateStepFinished
            false,                      // canReturnEarly
            &earlyReturnRequested,      // earlyReturnRequested
            &earlyReturnTime);          // earlyReturnTime
    }
}
)code";


/**
 * @brief Part one of FMI2 function template. Due to the size split into 2 strings.
 * File 'fmi2Functions.c'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFmi2FunctionsSrcFileTemplate1[] = R"code(#if FMI_VERSION != 2
#error FMI_VERSION must be 2
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "config.h"
#include "model.h"
#include "cosimulation.h"


// C-code FMUs have functions names prefixed with MODEL_IDENTIFIER_.
// Define DISABLE_PREFIX to build a binary FMU.
#ifndef DISABLE_PREFIX
#define pasteA(a,b)     a ## b
#define pasteB(a,b)    pasteA(a,b)
#define FMI2_FUNCTION_PREFIX pasteB(MODEL_IDENTIFIER, _)
#endif
#include "fmi2Functions.h"

#define ASSERT_NOT_NULL(p) \
do { \
    if (!p) { \
        logError(S, "Argument %s must not be NULL.", xstr(p)); \
        S->state = modelError; \
        return (fmi2Status)Error; \
    } \
} while (0)

#define GET_VARIABLES(T) \
do { \
    Status status = OK; \
    if (nvr == 0) return (fmi2Status)status; \
    ASSERT_NOT_NULL(vr); \
    ASSERT_NOT_NULL(value); \
    size_t index = 0; \
    if (S->isDirtyValues) { \
        Status s = calculateValues(S); \
        status = max(status, s); \
        if (status > Warning) return (fmi2Status)status; \
        S->isDirtyValues = false; \
    } \
    for (size_t i = 0; i < nvr; i++) { \
        Status s = get ## T(S, vr[i], value, nvr, &index); \
        status = max(status, s); \
        if (status > Warning) return (fmi2Status)status; \
    } \
    return (fmi2Status)status; \
} while (0)

#define SET_VARIABLES(T) \
do { \
    Status status = OK; \
    if (nvr == 0) return (fmi2Status)status; \
    ASSERT_NOT_NULL(vr); \
    ASSERT_NOT_NULL(value); \
    size_t index = 0; \
    for (size_t i = 0; i < nvr; i++) { \
        Status s = set ## T(S, vr[i], value, nvr, &index); \
        status = max(status, s); \
        if (status > Warning) return (fmi2Status)status; \
    } \
    if (nvr > 0) S->isDirtyValues = true; \
    return (fmi2Status)status; \
} while (0)

#define GET_BOOLEAN_VARIABLES \
do { \
    Status status = OK; \
    for (size_t i = 0; i < nvr; i++) { \
        bool v = false; \
        size_t index = 0; \
        Status s = getBoolean(S, vr[i], &v, nvr, &index); \
        value[i] = v; \
        status = max(status, s); \
        if (status > Warning) return (fmi2Status)status; \
    } \
    return (fmi2Status)status; \
} while (0)

#define SET_BOOLEAN_VARIABLES \
do { \
    Status status = OK; \
    for (size_t i = 0; i < nvr; i++) { \
        bool v = value[i]; \
        size_t index = 0; \
        Status s = setBoolean(S, vr[i], &v, nvr, &index); \
        status = max(status, s); \
        if (status > Warning) return (fmi2Status)status; \
    } \
    return (fmi2Status)status; \
} while (0)

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

#ifndef DT_EVENT_DETECT
#define DT_EVENT_DETECT 1e-10
#endif

// ---------------------------------------------------------------------------
// Function calls allowed state masks for both Model-exchange and Co-simulation
// ---------------------------------------------------------------------------
#define MASK_fmi2GetTypesPlatform        (StartAndEnd | Instantiated | InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete | StepInProgress | StepFailed | StepCanceled \
| Terminated | modelError)
#define MASK_fmi2GetVersion              MASK_fmi2GetTypesPlatform
#define MASK_fmi2SetDebugLogging         (Instantiated | InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete | StepInProgress | StepFailed | StepCanceled \
| Terminated | modelError)
#define MASK_fmi2Instantiate             (StartAndEnd)
#define MASK_fmi2FreeInstance            (Instantiated | InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete | StepFailed | StepCanceled \
| Terminated | modelError)
#define MASK_fmi2SetupExperiment         Instantiated
#define MASK_fmi2EnterInitializationMode Instantiated
#define MASK_fmi2ExitInitializationMode  InitializationMode
#define MASK_fmi2Terminate               (EventMode | ContinuousTimeMode \
| StepComplete | StepFailed)
#define MASK_fmi2Reset                   MASK_fmi2FreeInstance
#define MASK_fmi2GetReal                 (InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete | StepFailed | StepCanceled \
| Terminated | modelError)
#define MASK_fmi2GetInteger              MASK_fmi2GetReal
#define MASK_fmi2GetBoolean              MASK_fmi2GetReal
#define MASK_fmi2GetString               MASK_fmi2GetReal
#define MASK_fmi2SetReal                 (Instantiated | InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete)
#define MASK_fmi2SetInteger              (Instantiated | InitializationMode \
| EventMode \
| StepComplete)
#define MASK_fmi2SetBoolean              MASK_fmi2SetInteger
#define MASK_fmi2SetString               MASK_fmi2SetInteger
#define MASK_fmi2GetFMUstate             MASK_fmi2FreeInstance
#define MASK_fmi2SetFMUstate             MASK_fmi2FreeInstance
#define MASK_fmi2FreeFMUstate            MASK_fmi2FreeInstance
#define MASK_fmi2SerializedFMUstateSize  MASK_fmi2FreeInstance
#define MASK_fmi2SerializeFMUstate       MASK_fmi2FreeInstance
#define MASK_fmi2DeSerializeFMUstate     MASK_fmi2FreeInstance
#define MASK_fmi2GetDirectionalDerivative (InitializationMode \
| EventMode | ContinuousTimeMode \
| StepComplete | StepFailed | StepCanceled \
| Terminated | modelError)

// ---------------------------------------------------------------------------
// Function calls allowed state masks for Model-exchange
// ---------------------------------------------------------------------------
#define MASK_fmi2EnterEventMode          (EventMode | ContinuousTimeMode)
#define MASK_fmi2NewDiscreteStates       EventMode
#define MASK_fmi2EnterContinuousTimeMode EventMode
#define MASK_fmi2CompletedIntegratorStep ContinuousTimeMode
#define MASK_fmi2SetTime                 (EventMode | ContinuousTimeMode)
#define MASK_fmi2SetContinuousStates     ContinuousTimeMode
#define MASK_fmi2GetEventIndicators      (InitializationMode \
| EventMode | ContinuousTimeMode \
| Terminated | modelError)
#define MASK_fmi2GetContinuousStates     MASK_fmi2GetEventIndicators
#define MASK_fmi2GetDerivatives          (EventMode | ContinuousTimeMode \
| Terminated | modelError)
#define MASK_fmi2GetNominalsOfContinuousStates ( Instantiated \
| EventMode | ContinuousTimeMode \
| Terminated | modelError)

// ---------------------------------------------------------------------------
// Function calls allowed state masks for Co-simulation
// ---------------------------------------------------------------------------
#define MASK_fmi2SetRealInputDerivatives (Instantiated | InitializationMode \
| StepComplete)
#define MASK_fmi2GetRealOutputDerivatives (StepComplete | StepFailed | StepCanceled \
| Terminated | Error)
#define MASK_fmi2DoStep                  StepComplete
#define MASK_fmi2CancelStep              StepInProgress
#define MASK_fmi2GetStatus               (StepComplete | StepInProgress | StepFailed \
| Terminated)
#define MASK_fmi2GetRealStatus           MASK_fmi2GetStatus
#define MASK_fmi2GetIntegerStatus        MASK_fmi2GetStatus
#define MASK_fmi2GetBooleanStatus        MASK_fmi2GetStatus
#define MASK_fmi2GetStringStatus         MASK_fmi2GetStatus

// shorthand to access the  instance
#define S ((ModelInstance *)c)

#define ASSERT_STATE(S) \
    if (!allowedState(c, MASK_fmi2##S, #S)) \
        return fmi2Error;

static bool allowedState(ModelInstance *instance, int statesExpected, char *name) {

    if (!instance) {
        return false;
    }

    if (!(instance->state & statesExpected)) {
        logError(instance, "fmi2%s: Illegal call sequence.", name);
        return false;
    }

    return true;

}

// ---------------------------------------------------------------------------
// FMI functions
// ---------------------------------------------------------------------------

fmi2Component fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID,
                            fmi2String fmuResourceLocation, const fmi2CallbackFunctions *functions,
                            fmi2Boolean visible, fmi2Boolean loggingOn) {

    UNUSED(visible);

    if (!functions || !functions->logger) {
        return NULL;
    }

    return createModelInstance(
        (loggerType)functions->logger,
        NULL,
        functions->componentEnvironment,
        instanceName,
        fmuGUID,
        fmuResourceLocation,
        loggingOn,
        (InterfaceType)fmuType);
}

fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined, fmi2Real tolerance,
                            fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime) {

    UNUSED(toleranceDefined);
    UNUSED(tolerance);

    ASSERT_STATE(SetupExperiment)

    S->startTime = startTime;
    S->stopTime = stopTimeDefined ? stopTime : INFINITY;
    S->time = startTime;

    return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {

    ASSERT_STATE(EnterInitializationMode)

    S->state = InitializationMode;

    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {

    ASSERT_STATE(ExitInitializationMode);

    fmi2Status status = fmi2OK;

    // if values were set and no fmi2GetXXX triggered update before,
    // ensure calculated values are updated now
    if (S->isDirtyValues) {
        status = (fmi2Status)calculateValues(S);
        S->isDirtyValues = false;
    }

    if (S->type == ModelExchange) {
        S->state = EventMode;
    } else {
        S->state = StepComplete;
    }

    return status;
}

fmi2Status fmi2Terminate(fmi2Component c) {

    ASSERT_STATE(Terminate)

    S->state = Terminated;

    return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c) {

    ASSERT_STATE(Reset);

    reset(S);

    return fmi2OK;
}

void fmi2FreeInstance(fmi2Component c) {

    if (S) {
        freeModelInstance(S);
    }
}

// ---------------------------------------------------------------------------
// FMI functions: class methods not depending of a specific model instance
// ---------------------------------------------------------------------------

const char* fmi2GetVersion(void) {
    return fmi2Version;
}

const char* fmi2GetTypesPlatform(void) {
    return fmi2TypesPlatform;
}

// ---------------------------------------------------------------------------
// FMI functions: logging control, setters and getters for Real, Integer,
// Boolean, String
// ---------------------------------------------------------------------------

fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]) {

    ASSERT_STATE(SetDebugLogging)

    return (fmi2Status)setDebugLogging(S, loggingOn, nCategories, categories);
}

fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {

    ASSERT_STATE(GetReal)

    if (nvr > 0 && nullPointer(S, "fmi2GetReal", "vr[]", vr))
        return fmi2Error;

    if (nvr > 0 && nullPointer(S, "fmi2GetReal", "value[]", value))
        return fmi2Error;

    if (nvr > 0 && S->isDirtyValues) {
        calculateValues(S);
        S->isDirtyValues = false;
    }

    GET_VARIABLES(Float64);
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {

    ASSERT_STATE(GetInteger)

    if (nvr > 0 && nullPointer(S, "fmi2GetInteger", "vr[]", vr))
            return fmi2Error;

    if (nvr > 0 && nullPointer(S, "fmi2GetInteger", "value[]", value))
            return fmi2Error;

    if (nvr > 0 && S->isDirtyValues) {
        calculateValues(S);
        S->isDirtyValues = false;
    }

    GET_VARIABLES(Int32);
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {

    ASSERT_STATE(GetBoolean)

    if (nvr > 0 && nullPointer(S, "fmi2GetBoolean", "vr[]", vr))
            return fmi2Error;

    if (nvr > 0 && nullPointer(S, "fmi2GetBoolean", "value[]", value))
            return fmi2Error;

    if (nvr > 0 && S->isDirtyValues) {
        calculateValues(S);
        S->isDirtyValues = false;
    }

    GET_BOOLEAN_VARIABLES;
}

fmi2Status fmi2GetString (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[]) {

    ASSERT_STATE(GetString)

    if (nvr>0 && nullPointer(S, "fmi2GetString", "vr[]", vr))
            return fmi2Error;

    if (nvr>0 && nullPointer(S, "fmi2GetString", "value[]", value))
            return fmi2Error;

    if (nvr > 0 && S->isDirtyValues) {
        calculateValues(S);
        S->isDirtyValues = false;
    }

    GET_VARIABLES(String);
}

fmi2Status fmi2SetReal (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) {

    ASSERT_STATE(SetReal)

    if (nvr > 0 && nullPointer(S, "fmi2SetReal", "vr[]", vr))
        return fmi2Error;

    if (nvr > 0 && nullPointer(S, "fmi2SetReal", "value[]", value))
        return fmi2Error;

    SET_VARIABLES(Float64);
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {

    ASSERT_STATE(SetInteger)

    if (nvr > 0 && nullPointer(S, "fmi2SetInteger", "vr[]", vr))
        return fmi2Error;

    if (nvr > 0 && nullPointer(S, "fmi2SetInteger", "value[]", value))
        return fmi2Error;

    SET_VARIABLES(Int32);
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {

    ASSERT_STATE(SetBoolean)

    if (nvr>0 && nullPointer(S, "fmi2SetBoolean", "vr[]", vr))
        return fmi2Error;

    if (nvr>0 && nullPointer(S, "fmi2SetBoolean", "value[]", value))
        return fmi2Error;

    SET_BOOLEAN_VARIABLES;
}

fmi2Status fmi2SetString (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]) {

    ASSERT_STATE(SetString);

    if (nvr>0 && nullPointer(S, "fmi2SetString", "vr[]", vr))
        return fmi2Error;

    if (nvr>0 && nullPointer(S, "fmi2SetString", "value[]", value))
        return fmi2Error;

    SET_VARIABLES(String);
}

fmi2Status fmi2GetFMUstate (fmi2Component c, fmi2FMUstate* FMUstate) {

    ASSERT_STATE(GetFMUstate);

    *FMUstate = getFMUState(S);

    return fmi2OK;
}
)code";


/**
 * @brief Part two of FMI2 function template. Due to the size split into 2 strings.
 * File 'fmi2Functions.c'. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szFmi2FunctionsSrcFileTemplate2[] = R"code(
fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate FMUstate) {

    ASSERT_STATE(SetFMUstate);

    if (nullPointer(S, "fmi2SetFMUstate", "FMUstate", FMUstate)) {
        return fmi2Error;
    }

    setFMUState(S, FMUstate);

    return fmi2OK;
}

fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) {

    ASSERT_STATE(FreeFMUstate);

    free(*FMUstate);

    *FMUstate = NULL;

    return fmi2OK;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate, size_t *size) {

    UNUSED(c);
    UNUSED(FMUstate);

    ASSERT_STATE(SerializedFMUstateSize);

    *size = sizeof(ModelInstance);

    return fmi2OK;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size) {

    ASSERT_STATE(SerializeFMUstate);

    if (nullPointer(S, "fmi2SerializeFMUstate", "FMUstate", FMUstate)) {
        return fmi2Error;
    }

    if (invalidNumber(S, "fmi2SerializeFMUstate", "size", size, sizeof(ModelInstance))) {
        return fmi2Error;
    }

    memcpy(serializedState, FMUstate, sizeof(ModelInstance));

    return fmi2OK;
}

fmi2Status fmi2DeSerializeFMUstate (fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate) {

    ASSERT_STATE(DeSerializeFMUstate);

    if (invalidNumber(S, "fmi2DeSerializeFMUstate", "size", size, sizeof(ModelInstance))) {
        return fmi2Error;
    }

    if (*FMUstate == NULL) {
        *FMUstate = calloc(1, sizeof(ModelInstance));
    }

    memcpy(*FMUstate, serializedState, sizeof(ModelInstance));

    return fmi2OK;
}

fmi2Status fmi2GetDirectionalDerivative(fmi2Component c, const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                        const fmi2ValueReference vKnown_ref[] , size_t nKnown,
                                        const fmi2Real dvKnown[], fmi2Real dvUnknown[]) {

    ASSERT_STATE(GetDirectionalDerivative);

    // TODO: check value references
    // TODO: assert nUnknowns == nDeltaOfUnknowns
    // TODO: assert nKnowns == nDeltaKnowns

    Status status = OK;

    for (size_t i = 0; i < nUnknown; i++) {
        dvUnknown[i] = 0;
        for (size_t j = 0; j < nKnown; j++) {
            double partialDerivative = 0;
            Status s = getPartialDerivative(S, vUnknown_ref[i], vKnown_ref[j], &partialDerivative);
            status = max(status, s);
            if (status > Warning) {
                return (fmi2Status)status;
            }
            dvUnknown[i] += partialDerivative * dvKnown[j];
        }
    }

    return fmi2OK;
}

// ---------------------------------------------------------------------------
// Functions for FMI for Co-Simulation
// ---------------------------------------------------------------------------
/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                                     const fmi2Integer order[], const fmi2Real value[]) {

    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);

    ASSERT_STATE(SetRealInputDerivatives);

    logError(S, "fmi2SetRealInputDerivatives: ignoring function call."
            " This model cannot interpolate inputs: canInterpolateInputs=\"fmi2False\"");

    return fmi2Error;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                                      const fmi2Integer order[], fmi2Real value[]) {

    ASSERT_STATE(GetRealOutputDerivatives);

#ifdef GET_OUTPUT_DERIVATIVE
    Status status = OK;

    for (size_t i = 0; i < nvr; i++) {
        const Status s = getOutputDerivative(S, vr[i], order[i], &value[i]);
        status = max(status, s);
        if (status > Warning) {
            return (fmi2Status)status;
        }
    }

    return (fmi2Status)status;
#else
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);

    logError(S, "fmi2GetRealOutputDerivatives: ignoring function call."
        " This model cannot compute derivatives of outputs: MaxOutputDerivativeOrder=\"0\"");

    return fmi2Error;
#endif
}

fmi2Status fmi2CancelStep(fmi2Component c) {

    ASSERT_STATE(CancelStep);

    logError(S, "fmi2CancelStep: Can be called when fmi2DoStep returned fmi2Pending."
        " This is not the case.");

    return fmi2Error;
}

fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint,
                    fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint) {

    UNUSED(noSetFMUStatePriorToCurrentPoint);

    ASSERT_STATE(DoStep);

    if (communicationStepSize <= 0) {
        logError(S, "Communication step size must be > 0 but was %g.", communicationStepSize);
        S->state = modelError;
        return fmi2Error;
    }

    if (currentCommunicationPoint + communicationStepSize > S->stopTime + EPSILON) {
        logError(S, "At communication point %.16g a step size of %.16g was requested but stop time is %.16g.",
            currentCommunicationPoint, communicationStepSize, S->stopTime);
        S->state = modelError;
        return fmi2Error;
    }

    const fmi2Real nextCommunicationPoint = currentCommunicationPoint + communicationStepSize + EPSILON;

    while (true) {

        if (S->time + FIXED_SOLVER_STEP > nextCommunicationPoint) {
            break;  // next communication point reached
        }

        bool stateEvent, timeEvent;

        doFixedStep(S, &stateEvent, &timeEvent);

#ifdef EVENT_UPDATE
        if (stateEvent || timeEvent) {
            eventUpdate(S);
        }
#endif
    }

    return S->terminateSimulation ? fmi2Discard : fmi2OK;
}

/* Inquire slave status */
static fmi2Status getStatus(char* fname, fmi2Component c, const fmi2StatusKind s) {

    switch(s) {
    case fmi2DoStepStatus: logError(S,
        "%s: Can be called with fmi2DoStepStatus when fmi2DoStep returned fmi2Pending."
        " This is not the case.", fname);
        break;
    case fmi2PendingStatus: logError(S,
        "%s: Can be called with fmi2PendingStatus when fmi2DoStep returned fmi2Pending."
        " This is not the case.", fname);
        break;
    case fmi2LastSuccessfulTime: logError(S,
        "%s: Can be called with fmi2LastSuccessfulTime when fmi2DoStep returned fmi2Discard."
        " This is not the case.", fname);
        break;
    case fmi2Terminated: logError(S,
        "%s: Can be called with fmi2Terminated when fmi2DoStep returned fmi2Discard."
        " This is not the case.", fname);
        break;
    }

    return fmi2Discard;
}

fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status *value) {

    UNUSED(value);

    ASSERT_STATE(GetStatus);

    return getStatus("fmi2GetStatus", c, s);
}

fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real *value) {

    ASSERT_STATE(GetRealStatus);

    if (s == fmi2LastSuccessfulTime) {
        *value = S->time;
        return fmi2OK;
    }

    return getStatus("fmi2GetRealStatus", c, s);
}

fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer *value) {

    UNUSED(value);

    ASSERT_STATE(GetIntegerStatus);

    return getStatus("fmi2GetIntegerStatus", c, s);
}

fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean *value) {

    ASSERT_STATE(GetBooleanStatus);

    if (s == fmi2Terminated) {
        *value = S->terminateSimulation;
        return fmi2OK;
    }

    return getStatus("fmi2GetBooleanStatus", c, s);
}

fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String *value) {
    UNUSED(value);
    ASSERT_STATE(GetStringStatus);
    return getStatus("fmi2GetStringStatus", c, s);
}

// ---------------------------------------------------------------------------
// Functions for FMI2 for Model Exchange
// ---------------------------------------------------------------------------
/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component c) {

    ASSERT_STATE(EnterEventMode);

    S->state = EventMode;

    return fmi2OK;
}

fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo *eventInfo) {

    ASSERT_STATE(NewDiscreteStates);

#ifdef EVENT_UPDATE
    eventUpdate(S);
#endif

    eventInfo->newDiscreteStatesNeeded           = S->newDiscreteStatesNeeded;
    eventInfo->terminateSimulation               = S->terminateSimulation;
    eventInfo->nominalsOfContinuousStatesChanged = S->nominalsOfContinuousStatesChanged;
    eventInfo->valuesOfContinuousStatesChanged   = S->valuesOfContinuousStatesChanged;
    eventInfo->nextEventTimeDefined              = S->nextEventTimeDefined;
    eventInfo->nextEventTime                     = S->nextEventTime;

    return fmi2OK;
}

fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c) {

    ASSERT_STATE(EnterContinuousTimeMode);

    S->state = ContinuousTimeMode;

    return fmi2OK;
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component c, fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                     fmi2Boolean *enterEventMode, fmi2Boolean *terminateSimulation) {



    UNUSED(noSetFMUStatePriorToCurrentPoint);

    ASSERT_STATE(CompletedIntegratorStep);

    if (nullPointer(S, "fmi2CompletedIntegratorStep", "enterEventMode", enterEventMode))
        return fmi2Error;

    if (nullPointer(S, "fmi2CompletedIntegratorStep", "terminateSimulation", terminateSimulation))
        return fmi2Error;

    *enterEventMode = fmi2False;
    *terminateSimulation = fmi2False;

    return fmi2OK;
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time) {

    ASSERT_STATE(SetTime);

    S->time = time;

    return fmi2OK;
}

fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx){

    ASSERT_STATE(SetContinuousStates);

    if (invalidNumber(S, "fmi2SetContinuousStates", "nx", nx, NX))
        return fmi2Error;

    if (nullPointer(S, "fmi2SetContinuousStates", "x[]", x))
        return fmi2Error;

    setContinuousStates(S, x, nx);

    return fmi2OK;
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx) {

    ASSERT_STATE(GetDerivatives);

    if (invalidNumber(S, "fmi2GetDerivatives", "nx", nx, NX))
        return fmi2Error;

    if (nullPointer(S, "fmi2GetDerivatives", "derivatives[]", derivatives))
        return fmi2Error;

    getDerivatives(S, derivatives, nx);

    return fmi2OK;
}

fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni) {

    ASSERT_STATE(GetEventIndicators);

#if NZ > 0

    if (invalidNumber(S, "fmi2GetEventIndicators", "ni", ni, NZ))
        return fmi2Error;

    getEventIndicators(S, eventIndicators, ni);
#else
    UNUSED(c);
    UNUSED(eventIndicators);
    if (ni > 0) return fmi2Error;
#endif
    return fmi2OK;
}

fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real states[], size_t nx) {

    ASSERT_STATE(GetContinuousStates);

    if (invalidNumber(S, "fmi2GetContinuousStates", "nx", nx, NX))
        return fmi2Error;

    if (nullPointer(S, "fmi2GetContinuousStates", "states[]", states))
        return fmi2Error;

    getContinuousStates(S, states, nx);

    return fmi2OK;
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx) {

    ASSERT_STATE(GetNominalsOfContinuousStates);

    if (invalidNumber(S, "fmi2GetNominalContinuousStates", "nx", nx, NX))
        return fmi2Error;

    if (nullPointer(S, "fmi2GetNominalContinuousStates", "x_nominal[]", x_nominal))
        return fmi2Error;

    for (size_t i = 0; i < nx; i++)
        x_nominal[i] = 1;

    return fmi2OK;
}
)code";
