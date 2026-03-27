 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

/**
 * namespace for the signal names
 * in case /interfaces/signal_identifier.h
 * exists, use the file, otherwise define the namespace
 */
#ifndef SIGNAL_NAMES_H
#define SIGNAL_NAMES_H

#ifdef __has_include
#if __has_include("../interfaces/signal_identifier.h")
#include "../interfaces/signal_identifier.h"
#else

namespace trunk
{
    // Data Dispatch Service  signal names       to  dbc variable names                   C-type  RX/TX    vss name space
    static std::string      dsVehicleSpeed       =  "CAN_Input.Speed";                 ///< bool    RX     Vehicle.Speed   
    static std::string      dsTrunk              =  "CAN_Output.OpenTrunk";            ///< bool    TX     Vehicle.Body.Trunk 
} // trunk

#endif
#endif

#endif // SIGNAL_NAMES_H

