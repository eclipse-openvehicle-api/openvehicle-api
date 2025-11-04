/**
 * @file vd_front_door_left.cpp
 * @date 2025-07-11 12:43:52
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "vd_front_door_left.h"

/**
 * @brief Constructor
 */
CVehicleDeviceFrontDoorLeft::CVehicleDeviceFrontDoorLeft()
{
}

/**
* @brief initialize device to get the object name or use the default.
* @param[in] objectConfig Configuration containing the object name
*/
void CVehicleDeviceFrontDoorLeft::Initialize(const sdv::u8string&)
{
    if (m_status != sdv::EObjectStatus::initialization_pending)
	{
	    return;
    }

	sdv::core::CDispatchService dispatch;
	m_leftDoorIsOpen01Signal = dispatch.Subscribe(doors::dsLeftDoorIsOpen01, [&](sdv::any_t value) { ExecuteAllCallBacksForLeftDoorIsOpen01( value ); });
	if (!m_leftDoorIsOpen01Signal)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsLeftDoorIsOpen01, " [CVehicleDeviceFrontDoorLeft]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;		
	}

	m_leftLatch01 = dispatch.AddPublisher(doors::dsLeftLatch01);
	if (!m_leftLatch01)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsLeftLatch01, " [CVehicleDevice]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;		
	}

    m_status = sdv::EObjectStatus::initialized;
}

/**
* @brief Gets the current status of the object
* @return EObjectStatus The current status of the object
*/
sdv::EObjectStatus CVehicleDeviceFrontDoorLeft::GetStatus() const
{
	return m_status;
}

/**
 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
 * @param[in] eMode The operation mode, the component should run in.
 */
void CVehicleDeviceFrontDoorLeft::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
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
void CVehicleDeviceFrontDoorLeft::Shutdown()
{
    m_leftDoorIsOpen01Signal.Reset();

    std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
    for (auto callback : m_leftDoorIsOpen01Callbacks)
    {
        UnregisterIsOpenEvent(callback);
    }
    m_leftDoorIsOpen01Callbacks.clear();

	m_status = sdv::EObjectStatus::destruction_pending;
}

/**
* @brief Register IsOpen event on signal change
* Collect all events and call them on signal change
* @param[in] event function
*/
void CVehicleDeviceFrontDoorLeft::RegisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
		m_leftDoorIsOpen01Callbacks.insert(event);
	}
}

/**
* @brief Unregister IsOpen event
* @param[in] event function
*/
void CVehicleDeviceFrontDoorLeft::UnregisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
		m_leftDoorIsOpen01Callbacks.erase(event);
	}
}

/**
* @brief Execute all callbacks
*/
void CVehicleDeviceFrontDoorLeft::ExecuteAllCallBacksForLeftDoorIsOpen01(sdv::any_t value)
{
	bool leftDoorIsOpen = value.get<bool>();

	std::lock_guard<std::mutex> lock(m_leftDoorIsOpen01MutexCallbacks);
	for (auto callback : m_leftDoorIsOpen01Callbacks)
	{
		callback->WriteIsOpen(leftDoorIsOpen);
	}
}


/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false
 */
bool CVehicleDeviceFrontDoorLeft::WriteLock(bool value)
{
	if (m_leftLatch01)
	{
		m_leftLatch01.Write(value);
		return true;
	}

	return false;
}
