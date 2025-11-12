/**
 * @file bs_rear_door_right.h
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__BS_REARDOORRIGHT_H_20250711_125718_616__
#define __VSS_GENERATED__BS_REARDOORRIGHT_H_20250711_125718_616__

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
#include "vss_vehiclechassisdooraxle02right_bs_rx.h"
#include "vss_vehiclechassisdooraxle02right_vd_tx.h"
#include "vss_vehiclechassisdooraxle02right_bs_tx.h"

/**
 * @brief Basic Service Vehicle.Chassis.Door.Axle02.Right
 */
class CBasicServiceRearDoorRight
	: public sdv::CSdvObject
    , public vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_GetIsOpen
	, public vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetLock	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_GetIsOpen)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetLock)		
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Chassis.Door.Axle02.Right_Service")

	/**
	 * @brief Constructor
	 */
	CBasicServiceRearDoorRight();

	/**
	* @brief User-Defined Destructor
	*/
	~CBasicServiceRearDoorRight();

	/**
	 * @brief Set rightDoorIsOpen signal
	 * @param[in] value rightDoorIsOpen
	 */
	void SetIsOpenR2(bool value) override;
	
	/**
	 * @brief Set rightLatch02 signal
	 * @param[in] value 
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
	void RegisterOnSignalChangeOfRightDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event* callback) override;

	/**
	* @brief Unregister CallBack function on signal change
	* @param[in] callback function
	*/
	void UnregisterOnSignalChangeOfRightDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event* callback) override;

private:

	bool m_rightDoorIsOpen02 { 0 };                                                        ///< open&close state of the door
	mutable std::mutex m_rightDoorIsOpen02MutexCallbacks;                                  ///< Mutex protecting m_rightDoorIsOpen02Callbacks
	std::set<vss::Vehicle::Chassis::Door::Axle02::RightService::IVSS_SetIsOpen_Event*> m_rightDoorIsOpen02Callbacks; ///< collection of events to be called
	vss::Vehicle::Chassis::Door::Axle02::RightDevice::IVSS_WriteLock* m_ptrLock = nullptr; ///< Interface pointer to lock/unlock the door
};

DEFINE_SDV_OBJECT(CBasicServiceRearDoorRight)

#endif // !define __VSS_GENERATED__BS_REARDOORRIGHT_H_20250711_125718_616__
