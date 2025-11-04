/**
 * @file bs_front_door_left.h
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__BS_FRONTDOORLEFT_H_20250711_125718_601__
#define __VSS_GENERATED__BS_FRONTDOORLEFT_H_20250711_125718_601__

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
#include "vss_vehiclechassisdooraxle01left_bs_rx.h"
#include "vss_vehiclechassisdooraxle01left_vd_tx.h"
#include "vss_vehiclechassisdooraxle01left_bs_tx.h"

/**
 * @brief Basic Service Vehicle.Chassis.Door.Axle01.Left
 */
class CBasicServiceFrontDoorLeft
	: public sdv::CSdvObject
    , public vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_GetIsOpen
	, public vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetLock	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_GetIsOpen)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetLock)		
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Chassis.Door.Axle01.Left_Service")

	/**
	 * @brief Constructor
	 */
	CBasicServiceFrontDoorLeft();

	/**
	* @brief User-Defined Destructor
	*/
	~CBasicServiceFrontDoorLeft();

	/**
	 * @brief Set leftDoorIsOpen signal
	 * @param[in] value leftDoorIsOpen
	 */
	void SetIsOpenL1(bool value) override;

	/**
     * @brief Get leftDoorIsOpen
     * @return Returns the leftDoorIsOpen
     */
	bool GetIsOpen() const override;

	/**
	 * @brief Write leftDoorIsOpen signal
	 * @param[in] value leftDoorIsOpen
	 */
	void WriteIsOpen(bool value) override;

	/**
	 * @brief Set leftLatch signal
	 * @param[in] value
	 * @return true on success otherwise false
	 */
	bool SetLock(bool value) override;

	/**
	* @brief Register CallBack function on signal change
	* @param[in] callback function
	*/
	void RegisterOnSignalChangeOfLeftDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event* callback) override;

	/**
	* @brief Unregister CallBack function on signal change
	* @param[in] callback function
	*/
	void UnregisterOnSignalChangeOfLeftDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event* callback) override;

private:

	bool m_leftDoorIsOpen01 { 0 };
	mutable std::mutex m_leftDoorIsOpen01MutexCallbacks; ///< Mutex protecting m_leftDoorIsOpen01Callbacks
	std::set<vss::Vehicle::Chassis::Door::Axle01::LeftService::IVSS_SetIsOpen_Event*> m_leftDoorIsOpen01Callbacks; ///< collection of events to be called
	vss::Vehicle::Chassis::Door::Axle01::LeftDevice::IVSS_WriteLock* m_ptrLock = nullptr;
};

DEFINE_SDV_OBJECT(CBasicServiceFrontDoorLeft)

#endif // !define __VSS_GENERATED__BS_FRONTDOORLEFT_H_20250711_125718_601__
