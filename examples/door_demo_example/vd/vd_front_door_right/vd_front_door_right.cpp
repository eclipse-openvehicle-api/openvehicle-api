/**
 * @file vd_front_door_right.cpp
 * @date 2025-07-11 12:43:52
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#include <iostream>
#include "vd_front_door_right.h"

/**
 * @brief Constructor
 */
CVehicleDeviceFrontDoorRight::CVehicleDeviceFrontDoorRight()
{
}

/**
* @brief initialize device to get the object name or use the default.
* @param[in] objectConfig Configuration containing the object name
*/
void CVehicleDeviceFrontDoorRight::Initialize(const sdv::u8string&)
{
    if (m_status != sdv::EObjectStatus::initialization_pending)
	{
	    return;
    }
	sdv::core::CDispatchService dispatch;

	m_rightDoorIsOpen01Signal = dispatch.Subscribe(doors::dsRightDoorIsOpen01, [&](sdv::any_t value) { ExecuteAllCallBacksForRightDoorIsOpen01( value ); });
	if (!m_rightDoorIsOpen01Signal)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsRightDoorIsOpen01, " [CVehicleDeviceFrontDoorRight]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;	
	}

	m_rightLatch01 = dispatch.AddPublisher(doors::dsRightLatch01);
	if (!m_rightLatch01)
	{
		SDV_LOG_ERROR("Could not get signal: ", doors::dsRightLatch01, " [CVehicleDevice]");
        m_status = sdv::EObjectStatus::initialization_failure;
        return;		
	}	

    m_status = sdv::EObjectStatus::initialized;
}

/**
* @brief Gets the current status of the object
* @return EObjectStatus The current status of the object
*/
sdv::EObjectStatus CVehicleDeviceFrontDoorRight::GetStatus() const
{
	return m_status;
}

/**
 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
 * @param[in] eMode The operation mode, the component should run in.
 */
void CVehicleDeviceFrontDoorRight::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
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
void CVehicleDeviceFrontDoorRight::Shutdown()
{
    m_rightDoorIsOpen01Signal.Reset();

	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
    for (auto callback : m_rightDoorIsOpen01Callbacks)
    {
        UnregisterIsOpenEvent(callback);
    }
    m_rightDoorIsOpen01Callbacks.clear();

	m_status = sdv::EObjectStatus::destruction_pending;
}

/**
* @brief Register IsOpen event on signal change
* Collect all events and call them on signal change
* @param[in] event function
*/
void CVehicleDeviceFrontDoorRight::RegisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
		m_rightDoorIsOpen01Callbacks.insert(event);
	}
}

/**
* @brief Unregister IsOpen event
* @param[in] event function
*/
void CVehicleDeviceFrontDoorRight::UnregisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteIsOpen_Event* event)
{
	if (event)
	{
		std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
		m_rightDoorIsOpen01Callbacks.erase(event);
	}
}

/**
* @brief Execute all callbacks
*/
void CVehicleDeviceFrontDoorRight::ExecuteAllCallBacksForRightDoorIsOpen01(sdv::any_t value)
{
	bool rightDoorIsOpen = value.get<bool>();

	std::lock_guard<std::mutex> lock(m_rightDoorIsOpen01MutexCallbacks);
	for (auto callback : m_rightDoorIsOpen01Callbacks)
	{
		callback->WriteIsOpen(rightDoorIsOpen);
	}
}


/**
 * @brief Lock
 * @param[in] value
 * @return true on success otherwise false
 */
bool CVehicleDeviceFrontDoorRight::WriteLock(bool value)
{
	if (m_rightLatch01)
	{
		m_rightLatch01.Write(value);
		return true;
	}

	return false;
}
