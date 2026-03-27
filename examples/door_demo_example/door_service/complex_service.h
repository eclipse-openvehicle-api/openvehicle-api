 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#ifndef DOORS_COMPLEX_SERVICE_EXAMPLE_H
#define DOORS_COMPLEX_SERVICE_EXAMPLE_H

#include <iostream>

// SDV framework support
#include <support/component_impl.h>
#include <support/signal_support.h>
#include <support/timer.h>

// VSS interfaces - located in ../generated/vss_files/
#include "../generated/vss_files/vss_vehiclebodydooraxle_01left_bs_rx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_01left_bs_tx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_01right_bs_rx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_01right_bs_tx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_02left_bs_rx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_02left_bs_tx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_02right_bs_rx.h"
#include "../generated/vss_files/vss_vehiclebodydooraxle_02right_bs_tx.h"

#include "lock_doors_thread.h"

#include "../generated/door_service/door_ifc.h"

/**
 * @brief Doors example service: locks/unlocks doors after closing/opening doors
 */
class CDoorsExampleService : public sdv::CSdvObject
    , public vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetIsOpenL1_Event
    , public vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetIsOpenR1_Event
    , public vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetIsOpenL2_Event
    , public vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetIsOpenR2_Event
    , public IDoorService
{
public:
    /**
     * @brief Constructor
     */
    CDoorsExampleService() : m_doorsThread([&] { this->LockDoorsIfAllDoorsAreClosed(); }) {}

    /**
     * @brief Destructor
     */
    ~CDoorsExampleService()
    {}

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetIsOpenL1_Event)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetIsOpenR1_Event)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetIsOpenL2_Event)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetIsOpenR2_Event)
        SDV_INTERFACE_ENTRY(IDoorService)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_function)
    DECLARE_OBJECT_CLASS_NAME("Doors Example Service")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;

    /**
     * @brief Set leftDoorIsOpen signal (front door)
     * @param[in] value leftDoorIsOpen
     */
    void SetIsOpenL1(bool value) override
    {
        auto haschanged = (m_bFrontLeftDoorIsOpen != value);
        m_bFrontLeftDoorIsOpen = value;

        if (haschanged)
            AreAllDoorsClosed();
    }

    /**
     * @brief Set rightDoorIsOpen signal (front door)
     * @param[in] value rightDoorIsOpen
     */
    void SetIsOpenR1(bool value) override
    {
        auto haschanged = (m_bFrontRightDoorIsOpen != value);
        m_bFrontRightDoorIsOpen = value;

        if (haschanged)
            AreAllDoorsClosed();
    }

    /**
     * @brief Set leftDoorIsOpen signal (rear door)
     * @param[in] value leftDoorIsOpen
     */
    void SetIsOpenL2(bool value) override
    {
        auto haschanged = (m_bRearLeftDoorIsOpen != value);
        m_bRearLeftDoorIsOpen = value;

        if (haschanged)
            AreAllDoorsClosed();
    }

    /**
     * @brief Set rightDoorIsOpen signal (rear door)
     * @param[in] value rightDoorIsOpen
     */
    void SetIsOpenR2(bool value) override
    {
        auto haschanged = (m_bRearRightDoorIsOpen != value);
        m_bRearRightDoorIsOpen = value;

        if (haschanged)
            AreAllDoorsClosed();
    }

    /**
     * @brief Get doors state. If the doors are locked/unlocked
     */
    virtual bool GetDoorsStatus() override
    {
        return m_bAllDoorsAreLocked;
    }

private:

    /**
     * @brief Check if all doors are close
     * @details If all doors are not closed, unlock doors 
     */
    void AreAllDoorsClosed();

    /**
     * @brief Check if all doors are close
     * @details If all doors are closed, lock doors, otherwise do nothing. This is a callback function for timer. 
     */    
    void LockDoorsIfAllDoorsAreClosed();

    /**
     * @brief Lock or unlock doors
     * @param[in] lock if true lock doors, otherwise unlock doors
     */
    void LockDoors(const bool lock) const;

    bool	            m_bFrontLeftDoorIsOpen = false;  ///< Front Left Door Status 
    bool	            m_bFrontRightDoorIsOpen = false; ///< Front Right Door Status 
    bool	            m_bRearLeftDoorIsOpen = false;   ///< Rear Left Door Status 
    bool	            m_bRearRightDoorIsOpen = false;  ///< Rear Right Door Status 

    bool                m_bAllDoorsAreLocked = false;    ///< state for locked/unlocked of all doors

    ///< Door lock interfaces.
    vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetLock* m_pFrontLeftDoorSvc = nullptr;    ///< Front Left Door
    vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetLock* m_pFrontRightDoorSvc = nullptr;  ///< Front Right Door
    vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetLock* m_pRearLeftDoorSvc   = nullptr;   ///< Rear Left Door
    vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetLock* m_pRearRightDoorSvc  = nullptr;  ///< Rear Right Door

    LockDoorsThread m_doorsThread;  ///< timer  thread
    uint32_t m_Interval = 18;       ///< interval value * 100  = x milliseconds
};

DEFINE_SDV_OBJECT(CDoorsExampleService)

#endif // !define DOORS_COMPLEX_SERVICE_EXAMPLE_H
