/**
 * namespace for the signal names
 * in case /generated/vss_files/signal_identifier.h
 * exists, use the file, otherwise define the namespace
 */

#ifndef SIGNAL_NAMES_H
#define SIGNAL_NAMES_H

#ifdef __has_include
#if __has_include("../generated/vss_files/signal_identifier.h")
#include "../generated/vss_files/signal_identifier.h"
#else
namespace headlight
{
    // Data Dispatch Service  signal names       to  dbc variable names                  C-type  RX/TX  vss name space
    static std::string      dsFCurrentLatitude   =  "CAN_Input.Current_Latitude"  ; ///< float   RX     Vehicle.Position.CurrentLatitude  
    static std::string      dsFCurrentLongitude  =  "CAN_Input.Current_Longitude" ; ///< float   RX     Vehicle.Position.CurrentLongitude 
    static std::string      dsBHeadlightLowBeam  =  "CAN_Output.HeadLight_LowBeam"; ///< bool    TX     Vehicle.Body.Light.Front.Lowbeam  
} // headlight
#endif
#endif

#endif // ! defined  SIGNAL_NAMES_H

