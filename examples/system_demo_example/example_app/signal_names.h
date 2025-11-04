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
namespace demo
{
	static std::string dsWheelAngle = "CAN_Input.SteeringWheel";     ///< float    RX  Vehicle.Chassis.SteeringWheel.Angle         
	static std::string dsVehicleSpeed = "CAN_Input.Speed";           ///< float    RX  Vehicle.Speed                                
	static std::string dsAxleAngle = "CAN_Output.RearAngle";         ///< float    TX  Vehicle.Chassis.RearAxle.Row.Wheel      
	static std::string dsLiveCounter = "CAN_Output.IsActiveCounter"; ///< uint8_t  TX  Vehicle.Software.Application.IsActiveCounter 
} // demo
#endif
#endif

#endif // ! defined  SIGNAL_NAMES_H

