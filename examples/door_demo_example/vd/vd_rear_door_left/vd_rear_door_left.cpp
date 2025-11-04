/**
 * @file vd_rear_door_left.cpp
 * @date 2025-07-11 12:43:52
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "vd_rear_door_left.h"

/**
 * @brief Constructor
 */
CVehicleDeviceRearDoorLeft::CVehicleDeviceRearDoorLeft()
{
}

/**
* @brief initialize device to get the object name or use the default.
* @param[in] objectConfig Configuration containing the object name
*/
void CVehicleDeviceRearDoorLeft::Initialize(const sdv::u8string&)
{
    if (m_status != sdv::EObjectStatus::initialization_pending)
	{
	    return;
    }

	sdv::core::CDispatchService dispatch;
	m_leftDoorIsOpen02Signal = dispatch.Subscribe(doors::dsLeftDoorIsOpen02, [&](sdv::any_t value) { ExecuteAllCallBacksForLeftDoorIsOpen02( value ); });
	if (!m_leftDoorIsOpen02Signal)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsLeftDoorIsOpen02, " [CVehicleDeviceRearDoorLeft]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;
	}

	m_leftLatch02 = dispatch.AddPublisher(doors::dsLeftLatch02);
	if (!m_leftLatch02)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsLeftLatch02, " [CVehicleDevice]");
		m_status = sdv::EObjectStatus::initialization_failure;
        return;
	}

    m_status = sdv::EObjectStatus::initialized;
}

/**
* @brief Gets the current status of the object
* @return EObjectStatus The current status of the object
*/
sdv::EObjectStatus CVehicleDeviceRearDoorLeft::GetStatus() const
{
	return m_status;
}

/**
 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
 * @param[in] eMode The operation mode, the component should run in.
 */
void CVehicleDeviceRearDoorLeft::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
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
void CVehicleDeviceRearDoorLeft::Shutdown()
{
    m_leftDoorIsOpen02Signal.Reset();

	std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
    for (auto callback : m_leftDoorIsOpen02Callbacks)
    {
        UnregisterIsOpenEvent(callback);
    }
    m_leftDoorIsOpen02Callbacks.clear();

	m_status = sdv::EObjectStatus::destruction_pending;
}

/**
* @brief Register IsOpen event on signal change
* Collect all events and call them on signal change
* @param[in] event function
*/
void CVehicleDeviceRearDoorLeft::RegisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
		m_leftDoorIsOpen02Callbacks.insert(event);
	}
}

/**
* @brief Unregister IsOpen event
* @param[in] event function
*/
void CVehicleDeviceRearDoorLeft::UnregisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
		m_leftDoorIsOpen02Callbacks.erase(event);
	}
}

/**
* @brief Execute all callbacks
*/
void CVehicleDeviceRearDoorLeft::ExecuteAllCallBacksForLeftDoorIsOpen02(sdv::any_t value)
{
	bool rightDoorIsOpen = value.get<bool>();

	std::lock_guard<std::mutex> lock(m_leftDoorIsOpen02MutexCallbacks);
	for (auto callback : m_leftDoorIsOpen02Callbacks)
	{
		callback->WriteIsOpen(rightDoorIsOpen);
	}
}


/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false
 */
bool CVehicleDeviceRearDoorLeft::WriteLock(bool value)
{
	if (m_leftLatch02)
	{
		m_leftLatch02.Write(value);
		return true;
	}

	return false;
}

