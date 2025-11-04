/**
 * @file bs_front_door_right.cpp
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "bs_front_door_right.h"

/**
 * @brief ConstructorF
 */
CBasicServiceFrontDoorRight::CBasicServiceFrontDoorRight()
{
	auto rightDoorIsOpen01Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_IsOpen>();
	if (!rightDoorIsOpen01Device)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_IsOpen': [CBasicServiceFrontDoorRight]");
		throw std::runtime_error("Vehicle.Chassis.Door.Axle01.Right mode device not found");
	}
	rightDoorIsOpen01Device->RegisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteIsOpen_Event*> (this));

	m_ptrLock = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteLock>();
	if (!m_ptrLock)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_WriteLock': [CBasicServiceFrontDoorRight]");
		throw std::runtime_error("Lock device not found");
	}

	SDV_LOG_TRACE("CBasicServiceFrontDoorRight created: [Vehicle.Chassis.Door.Axle01.Right_Device]");
	m_rightDoorIsOpen01 = 0;
}

/**
* @brief User-Defined Destructor
*/
CBasicServiceFrontDoorRight::~CBasicServiceFrontDoorRight()
{
	auto rightDoorIsOpen01Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle01.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_IsOpen>();
	if (rightDoorIsOpen01Device)
	{
		rightDoorIsOpen01Device->UnregisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteIsOpen_Event*> (this));
	}
    rightDoorIsOpen01Device = nullptr;
}

/**
 * @brief Set rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceFrontDoorRight::SetIsOpenR1(bool value)
{
	m_rightDoorIsOpen01 = value;
	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
	for (auto callback : m_rightDoorIsOpen01Callbacks)
	{
		callback->SetIsOpenR1(value);
	}
}

/**
 * @brief Write rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceFrontDoorRight::WriteIsOpen(bool value)
{
    SetIsOpenR1(value);
}


/**
 * @brief Get rightDoorIsOpen
 * @return Returns the rightDoorIsOpen
 */
bool CBasicServiceFrontDoorRight::GetIsOpen() const
{
	return m_rightDoorIsOpen01;
}

/**
 * @brief Register Callback on signal change
 * @param[in] callback function
 */
void CBasicServiceFrontDoorRight::RegisterOnSignalChangeOfRightDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event* rightDoorIsOpen01Callback)
{
	if ( rightDoorIsOpen01Callback)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
		m_rightDoorIsOpen01Callbacks.insert(rightDoorIsOpen01Callback);
	}
}

/**
 * @brief Unregister Callback
 * @param[in] callback function
 */
void CBasicServiceFrontDoorRight::UnregisterOnSignalChangeOfRightDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event* rightDoorIsOpen01Callback)
{
	if (rightDoorIsOpen01Callback)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
		m_rightDoorIsOpen01Callbacks.erase(rightDoorIsOpen01Callback);
	}
}

/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicServiceFrontDoorRight::SetLock(bool value)
{
	return m_ptrLock->WriteLock(value);
}

