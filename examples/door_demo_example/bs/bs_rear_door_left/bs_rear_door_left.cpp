/**
 * @file bs_rear_door_left.cpp
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "bs_rear_door_left.h"

/**
 * @brief Constructor
 */
CBasicServiceRearDoorLeft::CBasicServiceRearDoorLeft()
{
	auto leftDoorIsOpen02Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_IsOpen>();
	if (!leftDoorIsOpen02Device)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_IsOpen': [CBasicServiceRearDoorLeft]");
		throw std::runtime_error("Vehicle.Chassis.Door.Axle02.Left mode device not found");
	}
	leftDoorIsOpen02Device->RegisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteIsOpen_Event*> (this));

	m_ptrLock = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteLock>();
	if (!m_ptrLock)
	{
		SDV_LOG_ERROR("Could not get interface 'IVSS_WriteLock': [CBasicServiceRearDoorLeft]");
		throw std::runtime_error("Lock device not found");
	}

	SDV_LOG_TRACE("CBasicServiceRearDoorLeft created: [Vehicle.Chassis.Door.Axle02.Left_Device]");
	m_leftDoorIsOpen02 = 0;
}

/**
* @brief User-Defined Destructor
*/
CBasicServiceRearDoorLeft::~CBasicServiceRearDoorLeft()
{
	auto leftDoorIsOpen02Device = sdv::core::GetObject("Vehicle.Chassis.Door.Axle02.Left_Device").GetInterface<vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_IsOpen>();
	if (leftDoorIsOpen02Device)
	{
		leftDoorIsOpen02Device->UnregisterIsOpenEvent(dynamic_cast<vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteIsOpen_Event*> (this));
	}
    leftDoorIsOpen02Device = nullptr;
}

/**
 * @brief Set rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceRearDoorLeft::SetIsOpenL2(bool value)
{
	m_leftDoorIsOpen02 = value;
	std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
	for (auto callback : m_leftDoorIsOpen02Callbacks)
	{
		callback->SetIsOpenL2(value);
	}
}

/**
 * @brief Write rightDoorIsOpen
 * @param[in] value rightDoorIsOpen
 */
void CBasicServiceRearDoorLeft::WriteIsOpen(bool value)
{
    SetIsOpenL2(value);
}


/**
 * @brief Get rightDoorIsOpen
 * @return Returns the rightDoorIsOpen
 */
bool CBasicServiceRearDoorLeft::GetIsOpen() const
{
	return m_leftDoorIsOpen02;
}

/**
 * @brief Register Callback on signal change
 * @param[in] callback function
 */
void CBasicServiceRearDoorLeft::RegisterOnSignalChangeOfLeftDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event* leftDoorIsOpen02Callback)
{
	if ( leftDoorIsOpen02Callback)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
		m_leftDoorIsOpen02Callbacks.insert(leftDoorIsOpen02Callback);
	}
}

/**
 * @brief Unregister Callback
 * @param[in] callback function
 */
void CBasicServiceRearDoorLeft::UnregisterOnSignalChangeOfLeftDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event* leftDoorIsOpen02Callback)
{
	if (leftDoorIsOpen02Callback)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
		m_leftDoorIsOpen02Callbacks.erase(leftDoorIsOpen02Callback);
	}
}


/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false 
 */
bool CBasicServiceRearDoorLeft::SetLock(bool value)
{
	return m_ptrLock->WriteLock(value);
}
