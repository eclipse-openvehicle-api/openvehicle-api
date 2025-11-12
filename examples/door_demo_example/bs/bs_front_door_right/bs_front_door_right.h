/**
 * @file bs_front_door_right.h
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__BS_FRONTDOORRIGHT_H_20250711_125718_606__
#define __VSS_GENERATED__BS_FRONTDOORRIGHT_H_20250711_125718_606__

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
#include "vss_vehiclechassisdooraxle01right_bs_rx.h"
#include "vss_vehiclechassisdooraxle01right_vd_tx.h"
#include "vss_vehiclechassisdooraxle01right_bs_tx.h"

/**
 * @brief Basic Service Vehicle.Chassis.Door.Axle01.Right
 */
class CBasicServiceFrontDoorRight
	: public sdv::CSdvObject
    , public vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_GetIsOpen
	, public vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetLock	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_GetIsOpen)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetLock)		
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Chassis.Door.Axle01.Right_Service")

	/**
	 * @brief Constructor
	 */
	CBasicServiceFrontDoorRight();

	/**
	* @brief User-Defined Destructor
	*/
	~CBasicServiceFrontDoorRight();

	/**
	 * @brief Set rightDoorIsOpen signal
	 * @param[in] value rightDoorIsOpen
	 */
	void SetIsOpenR1(bool value) override;
	
	/**
	 * @brief Set  signal
	 * @param[in] value rightLatche 
     * @return true on success otherwise false
	 */
	bool SetLock(bool value) override;	

	/**
     * @brief Get rightDoorIsOpen
     * @return Returns the rightDoorIsOpen
     */
	bool GetIsOpen() const override;

	/**
	 * @brief Write rightDoorIsOpen signal
	 * @param[in] value rightDoorIsOpen
	 */
	void WriteIsOpen(bool value) override;

	/**
	* @brief Register CallBack function on signal change
	* @param[in] callback function
	*/
	void RegisterOnSignalChangeOfRightDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event* callback) override;

	/**
	* @brief Unregister CallBack function on signal change
	* @param[in] callback function
	*/
	void UnregisterOnSignalChangeOfRightDoorIsOpen01(vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event* callback) override;

private:

	bool m_rightDoorIsOpen01 { 0 };                                                        ///< open&close state of the door
	mutable std::mutex m_rightDoorIsOpen01MutexCallbacks;                                  ///< Mutex protecting m_rightDoorIsOpen01Callbacks
	std::set<vss::Vehicle::Chassis::Door::Axle01::RightService::IVSS_SetIsOpen_Event*> m_rightDoorIsOpen01Callbacks; ///< collection of events to be called
	vss::Vehicle::Chassis::Door::Axle01::RightDevice::IVSS_WriteLock* m_ptrLock = nullptr; ///< Interface pointer to lock/unlock the door
};

DEFINE_SDV_OBJECT(CBasicServiceFrontDoorRight)

#endif // !define __VSS_GENERATED__BS_FRONTDOORRIGHT_H_20250711_125718_606__
