 /********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0 
 ********************************************************************************/

#include <iostream>
#include "complex_service.h"

bool CDoorsExampleService::OnInitialize()
{
    // Request the basic service for front left door. 
    auto pFrontLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_GetIsOpenL1>();
    if (pFrontLeftDoorSvc)
    {
        // Register front left door change event handler.
        pFrontLeftDoorSvc->RegisterOnSignalChangeOfLeftDoorIsOpen01(static_cast<vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetIsOpenL1_Event*> (this));
    }

    // Request the basic service for front right door.
    auto pFrontRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_GetIsOpenR1>();
    if (pFrontRightDoorSvc)
    {
        // Register front right door change event handler.
        pFrontRightDoorSvc->RegisterOnSignalChangeOfRightDoorIsOpen01(static_cast<vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetIsOpenR1_Event*> (this));
    }

    // Request the basic service for rear left door.
    auto pRearLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_GetIsOpenL2>();
    if (pRearLeftDoorSvc)
    {
        // Register rear  left door change event handler.
        pRearLeftDoorSvc->RegisterOnSignalChangeOfLeftDoorIsOpen02(static_cast<vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetIsOpenL2_Event*> (this));
    }

    // Request the basic service for front right door.
    auto pRearRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_GetIsOpenR2>();
    if (pRearRightDoorSvc)
    {
        // Register rear  right door change event handler.
        pRearRightDoorSvc->RegisterOnSignalChangeOfRightDoorIsOpen02(static_cast<vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetIsOpenR2_Event*> (this));
    }

    // Request the basic service for locking the front left door.
    m_pFrontLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetLock>();

    // Request the basic service for locking the front right door.
    m_pFrontRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetLock>();

    // Request the basic service for locking the rear left door.
    m_pRearLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetLock>();

    // Request the basic service for locking the rear right door.
    m_pRearRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetLock>();

    // Validate if we have the Open/Closed signal and the Lock/Unlock door signal, both must exist together or both must not exist
    // Front left door is an exception, it is always required
    if ((!pFrontLeftDoorSvc) || (!m_pFrontLeftDoorSvc))
    {
        SDV_LOG_ERROR("Could not get interfaces for 'Front left door': [CDoorsExampleService]");
        return false;
    }

    if ((pFrontRightDoorSvc == nullptr) != (m_pFrontRightDoorSvc == nullptr))
    {
        SDV_LOG_ERROR("Could not get both interfaces for 'Front right door': [CDoorsExampleService]");
        return false;
    }

    if ((pRearLeftDoorSvc == nullptr) != (m_pRearLeftDoorSvc == nullptr))
    {
        SDV_LOG_ERROR("Could not get both interfaces for 'Rear left door': [CDoorsExampleService]");
        return false;
    }

    if ((pRearRightDoorSvc == nullptr) != (m_pRearRightDoorSvc == nullptr))
    {
        SDV_LOG_ERROR("Could not get both interfaces for 'Rear right door': [CDoorsExampleService]");
        return false;
    }

    m_doorsThread.start(m_Interval);
    return true;
}

void CDoorsExampleService::OnShutdown()
{
    // Unregister front left door change event handler.
    auto  pFrontLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_GetIsOpenL1>();
    if (pFrontLeftDoorSvc)
        pFrontLeftDoorSvc->UnregisterOnSignalChangeOfLeftDoorIsOpen01(static_cast<vss::Vehicle::Body::Door::Axle::_01::LeftService::IVSS_SetIsOpenL1_Event*> (this));

    // Unregister front right door change event handler.
    auto  pFrontRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._01.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_GetIsOpenR1>();
    if (pFrontRightDoorSvc)
        pFrontRightDoorSvc->UnregisterOnSignalChangeOfRightDoorIsOpen01(static_cast<vss::Vehicle::Body::Door::Axle::_01::RightService::IVSS_SetIsOpenR1_Event*> (this));

    // Unregister rear left door change event handler.
    auto  pRearLeftDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Left_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_GetIsOpenL2>();
    if (pRearLeftDoorSvc)
        pRearLeftDoorSvc->UnregisterOnSignalChangeOfLeftDoorIsOpen02(static_cast<vss::Vehicle::Body::Door::Axle::_02::LeftService::IVSS_SetIsOpenL2_Event*> (this));

    // Unregister rear right door change event handler.
    auto  pRearRightDoorSvc = sdv::core::GetObject("Vehicle.Body.Door.Axle._02.Right_Service").GetInterface<vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_GetIsOpenR2>();
    if (pRearRightDoorSvc)
        pRearRightDoorSvc->UnregisterOnSignalChangeOfRightDoorIsOpen02(static_cast<vss::Vehicle::Body::Door::Axle::_02::RightService::IVSS_SetIsOpenR2_Event*> (this));

    m_doorsThread.stop();
}

void CDoorsExampleService::AreAllDoorsClosed()
{
    if (m_bFrontLeftDoorIsOpen || m_bFrontRightDoorIsOpen || m_bRearLeftDoorIsOpen || m_bRearRightDoorIsOpen)
    {
        m_doorsThread.stop();
        LockDoors(false);
        m_bAllDoorsAreLocked = false;
        return;
    }
    m_doorsThread.restart(m_Interval);
}

void CDoorsExampleService::LockDoorsIfAllDoorsAreClosed()
{
    if (m_bFrontLeftDoorIsOpen || m_bFrontRightDoorIsOpen || m_bRearLeftDoorIsOpen || m_bRearRightDoorIsOpen)
    {
        return;
    }
    
    m_bAllDoorsAreLocked = true;
    LockDoors(true);
}

void CDoorsExampleService::LockDoors(const bool lock) const
{
    if (m_pFrontLeftDoorSvc)
        m_pFrontLeftDoorSvc->SetLock(lock);
    if (m_pFrontRightDoorSvc)
        m_pFrontRightDoorSvc->SetLock(lock);
    if (m_pRearLeftDoorSvc)
        m_pRearLeftDoorSvc->SetLock(lock);
    if (m_pRearRightDoorSvc)
        m_pRearRightDoorSvc->SetLock(lock);
}
