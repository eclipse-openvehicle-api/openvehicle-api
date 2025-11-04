/**
 * @file vd_rear_door_right.cpp
 * @date 2025-07-11 12:43:52
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "vd_rear_door_right.h"

/**
 * @brief Constructor
 */
CVehicleDeviceRearDoorRight::CVehicleDeviceRearDoorRight()
{
}

/**
* @brief initialize device to get the object name or use the default.
* @param[in] objectConfig Configuration containing the object name
*/
void CVehicleDeviceRearDoorRight::Initialize(const sdv::u8string&)
{
    if (m_status != sdv::EObjectStatus::initialization_pending)
	{
	    return;
    }
	sdv::core::CDispatchService dispatch;

	m_rightDoorIsOpen02Signal = dispatch.Subscribe(doors::dsRightDoorIsOpen02, [&](sdv::any_t value) { ExecuteAllCallBacksForRightDoorIsOpen02( value ); });
	if (!m_rightDoorIsOpen02Signal)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsRightDoorIsOpen02, " [CVehicleDeviceRearDoorRight]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;
	}

	m_rightLatch02 = dispatch.AddPublisher(doors::dsRightLatch02);
	if (!m_rightLatch02)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsRightLatch02, " [CVehicleDevice]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;		
	}

    m_status = sdv::EObjectStatus::initialized;
}

/**
* @brief Gets the current status of the object
* @return EObjectStatus The current status of the object
*/
sdv::EObjectStatus CVehicleDeviceRearDoorRight::GetStatus() const
{
	return m_status;
}

/**
 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
 * @param[in] eMode The operation mode, the component should run in.
 */
void CVehicleDeviceRearDoorRight::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
{
	switch (eMode)
	{
	case sdv::EOperationMode::configuring:
		if (m_status == sdv::EObjectStatus::running || m_status == sdv::EObjectStatus::initialized)
			m_status = sdv::EObjectStatus::configuring;
		break;
	case sdv::EOperationMode::running:
		if (m_status == sdv::EObjectStatus::configuring || m_status == sdv::EObjectStatus::initialized)
			m_status = sdv::EObjectStatus::running;
		break;
	default:
		break;
	}
}

/**
* @brief Shutdown function is to shutdown the execution of request thread
*/
void CVehicleDeviceRearDoorRight::Shutdown()
{
    m_rightDoorIsOpen02Signal.Reset();

	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
    for (auto callback : m_rightDoorIsOpen02Callbacks)
    {
        UnregisterIsOpenEvent(callback);
    }
    m_rightDoorIsOpen02Callbacks.clear();

	m_status = sdv::EObjectStatus::destruction_pending;
}

/**
* @brief Register IsOpen event on signal change
* Collect all events and call them on signal change
* @param[in] event function
*/
void CVehicleDeviceRearDoorRight::RegisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
		m_rightDoorIsOpen02Callbacks.insert(event);
	}
}

/**
* @brief Unregister IsOpen event
* @param[in] event function
*/
void CVehicleDeviceRearDoorRight::UnregisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
		m_rightDoorIsOpen02Callbacks.erase(event);
	}
}

/**
* @brief Execute all callbacks
*/
void CVehicleDeviceRearDoorRight::ExecuteAllCallBacksForRightDoorIsOpen02(sdv::any_t value)
{
	bool rightDoorIsOpen = value.get<bool>();

	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen02MutexCallbacks);
	for (auto callback : m_rightDoorIsOpen02Callbacks)
	{
		callback->WriteIsOpen(rightDoorIsOpen);
	}
}



/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false
 */
bool CVehicleDeviceRearDoorRight::WriteLock(bool value)
{
	if (m_rightLatch02)
	{
		m_rightLatch02.Write(value);
		return true;
	}

	return false;
}

