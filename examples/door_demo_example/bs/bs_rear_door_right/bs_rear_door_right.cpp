/**
 * @file bs_rear_door_right.cpp
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "bs_rear_door_right.h"

/**
 * @brief Constructor
 */
CBasicServiceRearDoorRight::CBasicServiceRearDoorRight()
{
	auto rightDoorIsOpen02Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_IsOpen>();
	if (!rightDoorIsOpen02Device)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_IsOpen': [CBasicServiceRearDoorRight]");
		throw std::runtime_error("Vehicle.Chassis.Door.Axle02.Right mode device not found");
	}
	rightDoorIsOpen02Device->RegisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteIsOpen_Event*> (this));

	m_ptrLock = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteLock>();
	if (!m_ptrLock)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_WriteLock': [CBasicServiceRearDoorRight]");
		throw std::runtime_error("Lock device not found");
	}

	SDV_LOG_TRACE("CBasicServiceRearDoorRight created: [Vehicle.Chassis.Door.Axle02.Right_Device]");
	m_rightDoorIsOpen02 = 0;
}

/**
* @brief User-Defined Destructor
*/
CBasicServiceRearDoorRight::~CBasicServiceRearDoorRight()
{
	auto rightDoorIsOpen02Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Right_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_IsOpen>();
	if (rightDoorIsOpen02Device)
	{
		rightDoorIsOpen02Device->UnregisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteIsOpen_Event*> (this));
	}
    rightDoorIsOpen02Device = nullptr;
}

/**
 * @brief Set rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceRearDoorRight::SetIsOpenR2(bool value)
{
	m_rightDoorIsOpen02 = value;
	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
	for (auto callback : m_rightDoorIsOpen02Callbacks)
	{
		callback->SetIsOpenR2(value);
	}
}

/**
 * @brief Write rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceRearDoorRight::WriteIsOpen(bool value)
{
    SetIsOpenR2(value);
}


/**
 * @brief Get rightDoorIsOpen
 * @return Returns the rightDoorIsOpen
 */
bool CBasicServiceRearDoorRight::GetIsOpen() const
{
	return m_rightDoorIsOpen02;
}

/**
 * @brief Register Callback on signal change
 * @param[in] callback function
 */
void CBasicServiceRearDoorRight::RegisterOnSignalChangeOfRightDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event* rightDoorIsOpen02Callback)
{
	if ( rightDoorIsOpen02Callback)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
		m_rightDoorIsOpen02Callbacks.insert(rightDoorIsOpen02Callback);
	}
}

/**
 * @brief Unregister Callback
 * @param[in] callback function
 */
void CBasicServiceRearDoorRight::UnregisterOnSignalChangeOfRightDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event* rightDoorIsOpen02Callback)
{
	if (rightDoorIsOpen02Callback)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
		m_rightDoorIsOpen02Callbacks.erase(rightDoorIsOpen02Callback);
	}
}

/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicServiceRearDoorRight::SetLock(bool value)
{
	return m_ptrLock->WriteLock(value);
}

