/**
 * @file bs_front_door_left.cpp
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "bs_front_door_left.h"

/**
 * @brief ConstructorF
 */
CBasicServiceFrontDoorLeft::CBasicServiceFrontDoorLeft()
{
	auto leftDoorIsOpen01Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen>();
	if (!leftDoorIsOpen01Device)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_IsOpen': [CBasicServiceFrontDoorLeft]");
		throw std::runtime_error("Vehicle.Chassis.Door.Axle01.Left mode device not found");
	}
	leftDoorIsOpen01Device->RegisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event*> (this));

	m_ptrLock = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteLock>();
	if (!m_ptrLock)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_WriteLock': [CBasicServiceFrontDoorLeft]");
		throw std::runtime_error("Lock device not found");
	}

	SDV_LOG_TRACE("CBasicServiceFrontDoorLeft created: [Vehicle.Chassis.Door.Axle01.Left_Device]");
	m_leftDoorIsOpen01 = 0;
}

/**
* @brief User-Defined Destructor
*/
CBasicServiceFrontDoorLeft::~CBasicServiceFrontDoorLeft()
{
	auto leftDoorIsOpen01Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen>();
	if (leftDoorIsOpen01Device)
	{
		leftDoorIsOpen01Device->UnregisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event*> (this));
	}
    leftDoorIsOpen01Device = nullptr;
}

/**
 * @brief Set leftDoorIsOpen
 * @param[in] value leftDoorIsOpen
 */
void CBasicServiceFrontDoorLeft::SetIsOpenL1(bool value)
{
	m_leftDoorIsOpen01 = value;
	std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
	for (auto callback : m_leftDoorIsOpen01Callbacks)
	{
		callback->SetIsOpenL1(value);
	}
}

/**
 * @brief Write leftDoorIsOpen
 * @param[in] value leftDoorIsOpen
 */
void CBasicServiceFrontDoorLeft::WriteIsOpen(bool value)
{
	SetIsOpenL1(value);
}


/**
 * @brief Get leftDoorIsOpen
 * @return Returns the leftDoorIsOpen
 */
bool CBasicServiceFrontDoorLeft::GetIsOpen() const
{
	return m_leftDoorIsOpen01;
}

/**
 * @brief Register Callback on signal change
 * @param[in] callback function
 */
void CBasicServiceFrontDoorLeft::RegisterOnSignalChangeOfLeftDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event* leftDoorIsOpen01Callback)
{
	if ( leftDoorIsOpen01Callback)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
		m_leftDoorIsOpen01Callbacks.insert(leftDoorIsOpen01Callback);
	}
}

/**
 * @brief Unregister Callback
 * @param[in] callback function
 */
void CBasicServiceFrontDoorLeft::UnregisterOnSignalChangeOfLeftDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event* leftDoorIsOpen01Callback)
{
	if (leftDoorIsOpen01Callback)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
		m_leftDoorIsOpen01Callbacks.erase(leftDoorIsOpen01Callback);
	}
}

/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicServiceFrontDoorLeft::SetLock(bool value)
{
	return m_ptrLock->WriteLock(value);
}

