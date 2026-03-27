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

namespace doors
{
    // Data Dispatch Service  signal names       to  dbc variable names                   C-type  RX/TX    vss name space
    static std::string      dsLeftDoorIsOpen01   = "CAN_Input_L1.Door01LeftIsOpen";  ///< bool    RX     Vehicle.Body.Door.Axle._01.Left
    static std::string      dsRightDoorIsOpen01  = "CAN_Input_R1.Door01RightIsOpen"; ///< bool    RX     Vehicle.Body.Door.Axle._01.Right
    static std::string      dsLeftDoorIsOpen02   = "CAN_Input_L2.Door02LeftIsOpen";  ///< bool    RX     Vehicle.Body.Door.Axle._02.Left
    static std::string      dsRightDoorIsOpen02  = "CAN_Input_R2.Door02RightIsOpen"; ///< bool    RX     Vehicle.Body.Door.Axle._02.Right
    static std::string      dsLeftLatch01        = "CAN_Output.LockDoor01Left";      ///< bool    TX     Vehicle.Body.Door.Axle._01.Left
    static std::string      dsRightLatch01       = "CAN_Output.LockDoor01Right";     ///< bool    TX     Vehicle.Body.Door.Axle._01.Right
    static std::string      dsLeftLatch02        = "CAN_Output.LockDoor02Left";      ///< bool    TX     Vehicle.Body.Door.Axle._02.Left
    static std::string      dsRightLatch02       = "CAN_Output.LockDoor02Right";     ///< bool    TX     Vehicle.Body.Door.Axle._02.Right 
	
} // doors

#endif // SIGNAL_NAMES_H
