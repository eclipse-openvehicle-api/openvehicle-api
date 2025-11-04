/**
 * @file bs_rear_door_left.h
 * @date 2025-07-11 12:57:18
 * File is auto generated from VSS utility.
 * VSS Version:1.0.0.1
 */
#ifndef __VSS_GENERATED__BS_REARDOORLEFT_H_20250711_125718_611__
#define __VSS_GENERATED__BS_REARDOORLEFT_H_20250711_125718_611__

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
#include "vss_vehiclechassisdooraxle02left_bs_rx.h"
#include "vss_vehiclechassisdooraxle02left_vd_tx.h"
#include "vss_vehiclechassisdooraxle02left_bs_tx.h"

/**
 * @brief Basic Service Vehicle.Chassis.Door.Axle02.Left
 */
class CBasicServiceRearDoorLeft
	: public sdv::CSdvObject
    , public vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_GetIsOpen
	, public vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteIsOpen_Event
	, public vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetLock	
{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_GetIsOpen)
		SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetLock)		
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("Vehicle.Chassis.Door.Axle02.Left_Service")

	/**
	 * @brief Constructor
	 */
	CBasicServiceRearDoorLeft();

	/**
	* @brief User-Defined Destructor
	*/
	~CBasicServiceRearDoorLeft();

	/**
	 * @brief Set rightDoorIsOpen signal
	 * @param[in] value rightDoorIsOpen
	 */
	void SetIsOpenL2(bool value) override;
	
	/**
	 * @brief Set leftLatch02 signal
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
	void RegisterOnSignalChangeOfLeftDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event* callback) override;

	/**
	* @brief Unregister CallBack function on signal change
	* @param[in] callback function
	*/
	void UnregisterOnSignalChangeOfLeftDoorIsOpen02(vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event* callback) override;

private:

	bool m_leftDoorIsOpen02 { 0 };
	mutable std::mutex m_leftDoorIsOpen02MutexCallbacks; ///< Mutex protecting m_leftDoorIsOpen02Callbacks
	std::set<vss::Vehicle::Chassis::Door::Axle02::LeftService::IVSS_SetIsOpen_Event*> m_leftDoorIsOpen02Callbacks; ///< collection of events to be called
	vss::Vehicle::Chassis::Door::Axle02::LeftDevice::IVSS_WriteLock* m_ptrLock = nullptr;
};

DEFINE_SDV_OBJECT(CBasicServiceRearDoorLeft)

#endif // !define __VSS_GENERATED__BS_REARDOORLEFT_H_20250711_125718_611__
