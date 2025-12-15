/**
 * @file signal_identifier.h
 * @date 2025-07-11 11:03:55
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__SIGNALIDENTIFIER_H_20250711_110355_840__
#define __VSS_GENERATED__SIGNALIDENTIFIER_H_20250711_110355_840__

namespace doors
{
    // Data Dispatch Service  signal names       to  dbc variable names                   C-type  RX/TX  vss name space
    static std::string      dsLeftDoorIsOpen01   =  "CAN_Input_L1.Door01LeftIsOpen";   ///< bool    RX     Vehicle.Chassis.Door.Axle01.Left     
    static std::string      dsRightDoorIsOpen01  =  "CAN_Input_R1.Door01RightIsOpen";  ///< bool    RX     Vehicle.Chassis.Door.Axle01.Right    
    static std::string      dsLeftDoorIsOpen02   =  "CAN_Input_L2.Door02LeftIsOpen";   ///< bool    RX     Vehicle.Chassis.Door.Axle02.Left     
    static std::string      dsRightDoorIsOpen02  =  "CAN_Input_R2.Door02RightIsOpen";  ///< bool    RX     Vehicle.Chassis.Door.Axle02.Right    
    static std::string      dsLeftLatch01        =  "CAN_Output.LockDoor01Left" ;      ///< bool    TX     Vehicle.Chassis.TX.Door.Axle01.Left  
    static std::string      dsRightLatch01       =  "CAN_Output.LockDoor01Right";      ///< bool    TX     Vehicle.Chassis.TX.Door.Axle01.Right 
    static std::string      dsLeftLatch02        =  "CAN_Output.LockDoor02Left" ;      ///< bool    TX     Vehicle.Chassis.TX.Door.Axle02.Left  
    static std::string      dsRightLatch02       =  "CAN_Output.LockDoor02Right";      ///< bool    TX     Vehicle.Chassis.TX.Door.Axle02.Right 
} // doors

#endif // __VSS_GENERATED__SIGNALIDENTIFIER_H_20250711_110355_840__
