/**
 * @file vd_front_door_left.h
 * @date 2025-07-11 12:43:52
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__VD_FRONTDOORLEFT_H_20250711_124352_681__
#define __VSS_GENERATED__VD_FRONTDOORLEFT_H_20250711_124352_681__

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
#include "vss_vehiclechassisdooraxle01left_vd_rx.h"
#include "vss_vehiclechassisdooraxle01left_vd_tx.h"
#include "signal_identifier.h"

/**
 * @brief Vehicle device Vehicle.Chassis.Door.Axle01.Left
 */
class CVehicleDeviceFrontDoorLeft
	: public sdv::CSdvObject
	, public sdv::IObjectControl
	, public vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen
	, public vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteLock	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::IObjectControl)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_IsOpen)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteLock)		
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Chassis.Door.Axle01.Left_Device")

	/**
	 * @brief Constructor
	 */
	CVehicleDeviceFrontDoorLeft();

	/**
	* @brief initialize device to get the object name or use the default.
	* @param[in] objectConfig Configuration containing the object name
	*/
	void Initialize(const sdv::u8string& objectConfig) override;

	/**
	* @brief Gets the current status of the object
	* @return EObjectStatus The current status of the object
	*/
	sdv::EObjectStatus GetStatus() const override;

	/**
	 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
	 * @param[in] eMode The operation mode, the component should run in.
	 */
	void SetOperationMode(/*in*/ sdv::EOperationMode eMode) override;

	/**
	* @brief Shutdown function is to shutdown the execution of request thread
	*/
	void Shutdown() override;
	
	/**
	 * @brief Set leftLatch signal
	 * @param[in] value
     * @return true on success otherwise false
	 */
	bool WriteLock(bool value) override;	

	/**
	* @brief Register IVSS_WriteIsOpen_Event on signal change
	* Collect all events and call them on signal change
	* @param[in] event function
	*/
	void RegisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event* event) override;

	/**
	* @brief Unregister IVSS_IsOpen_Event
	* @param[in] event function
	*/
	void UnregisterIsOpenEvent(vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event* event) override;

private:

	/**
	* @brief Execute all callbacks
	*/
	void ExecuteAllCallBacksForLeftDoorIsOpen01(sdv::any_t value);

	sdv::core::CSignal m_leftLatch01;
	sdv::core::CSignal m_leftDoorIsOpen01Signal;                                                        ///< Signal of the vehicle device
	mutable std::mutex m_leftDoorIsOpen01MutexCallbacks;                                                ///< Mutex protecting m_leftDoorIsOpen01Callbacks
	std::set<vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event*> m_leftDoorIsOpen01Callbacks; ///< collection of events to be called

	std::atomic<sdv::EObjectStatus> m_status = { sdv::EObjectStatus::initialization_pending };  ///< To update the object status when it changes.
};

DEFINE_SDV_OBJECT(CVehicleDeviceFrontDoorLeft)

#endif // !define __VSS_GENERATED__VD_FRONTDOORLEFT_H_20250711_124352_681__
