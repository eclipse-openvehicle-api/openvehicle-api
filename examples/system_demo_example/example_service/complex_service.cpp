#include <iostream>
#include "complex_service.h"

const float g_fSpeedThreshold = 30.0f;

CCounterSteeringExampleService::CCounterSteeringExampleService()
{
}

CCounterSteeringExampleService::~CCounterSteeringExampleService()
{
    // Just in case...
    Shutdown();
}

void CCounterSteeringExampleService::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    m_eStatus = sdv::EObjectStatus::initializing;

    // Request the basic service for monitoring the alive counter.
    m_pAliveCounterSvc = sdv::core::GetObject("Vehicle.Software.Application.IsActiveCounter_Service").GetInterface<vss::Vehicle::Software::Application::IsActiveCounterService::IVSS_SetCounter>();
    if (!m_pAliveCounterSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetCounter': [CCounterSteeringExampleService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Request the basic service for the rear axle.
    m_pRearAxleSvc = sdv::core::GetObject("Vehicle.Chassis.RearAxle.Row.Wheel_Service").GetInterface<vss::Vehicle::Chassis::RearAxle::Row::WheelService::IVSS_SetRearAxle>();
    if (!m_pRearAxleSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetAngle': [CCounterSteeringExampleService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Request the basic service for the steering wheel.
    auto pSteeringWheelSvc = sdv::core::GetObject("Vehicle.Chassis.SteeringWheel.Angle_Service").GetInterface<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_GetSteeringWheel>();
    if (!pSteeringWheelSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetSteeringAngle': [CCounterSteeringExampleService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Request the basic service for the vehicle speed.
    auto pVehSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (!pVehSpeedSvc)
    {
        SDV_LOG_ERROR("Could not get interface 'IVSS_SetSpeed': [CCounterSteeringExampleService]");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Register steering wheel change event handler.
    pSteeringWheelSvc->RegisterOnSignalChangeOfWheelAngle(static_cast<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event*> (this));

    // Register vehicle speed change event handler.
    pVehSpeedSvc->RegisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));

    // Start the alive timer with 10 ms period.
    m_Timer = sdv::core::CTaskTimer(10, [&]() {TimerFunction(); });
    if (!m_Timer)
    {
        SDV_LOG_ERROR("CCounterSteeringExampleService: tasktimer with 10 milliseconds could not be created.");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    SDV_LOG_INFO("CCounterSteeringExampleService: tasktimer created with 10 milliseconds");

    m_eStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CCounterSteeringExampleService::GetStatus() const
{
    return m_eStatus;
}

void CCounterSteeringExampleService::SetOperationMode(sdv::EOperationMode /*eMode*/)
{
    // Not applicable
}

void CCounterSteeringExampleService::Shutdown()
{
    // Terminate the alive counter
    m_Timer.Reset();

    // Unregister the steering wheel event handler.
    auto pSteeringWheelSvc = sdv::core::GetObject("Vehicle.Chassis.SteeringWheel.Angle_Service").GetInterface<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_GetSteeringWheel>();
    if (pSteeringWheelSvc)
        pSteeringWheelSvc->UnregisterOnSignalChangeOfWheelAngle(static_cast<vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event*> (this));

    // Unregister the vehicle speed event handler.
    auto pVehSpeedSvc = sdv::core::GetObject("Vehicle.Speed_Service").GetInterface<vss::Vehicle::SpeedService::IVSS_GetSpeed>();
    if (pVehSpeedSvc)
        pVehSpeedSvc->UnregisterOnSignalChangeOfVehicleSpeed(static_cast<vss::Vehicle::SpeedService::IVSS_SetSpeed_Event*> (this));
}

void CCounterSteeringExampleService::SetSteeringWheel(float value)
{
    if (m_fSteeringWheel == value) return;
    m_fSteeringWheel = value;
    UpdateRearAxleAngle();
}

void CCounterSteeringExampleService::SetSpeed(float value)
{
    if (m_fVehSpeed == value) return;
    m_fVehSpeed = value;
    UpdateRearAxleAngle();
}

void CCounterSteeringExampleService::ActivateService(/*in*/ bool bActivate)
{
    if (m_bActivated == bActivate) return;
    m_bActivated = bActivate;
    UpdateRearAxleAngle();
}

bool CCounterSteeringExampleService::IsActivated() const
{
    return m_bActivated;
}

bool CCounterSteeringExampleService::CounterSteeringActive() const
{
    return m_bActivated && m_fVehSpeed < g_fSpeedThreshold;
}

double CCounterSteeringExampleService::RearAxleAngle() const
{
    return m_fRearAxleAngle;
}

void CCounterSteeringExampleService::UpdateRearAxleAngle()
{
    float fRearAxleAngle = 0.0;
    if (CounterSteeringActive())
    {
        // Get steering wheel angle percentage
        // The steering wheel can have values from -916.736 to 916.736 degrees.
        float fSteeringWheelPercent = m_fSteeringWheel / 916.736f;
        if (fSteeringWheelPercent > 1.0) fSteeringWheelPercent = 1.0;
        if (fSteeringWheelPercent < -1.0) fSteeringWheelPercent = -1.0;

        // Calculate the vehicle speed percentage up to 30 km/h
        // The vehicle speed is calculated in m/s.
        float fVehSpeedPercent = m_fVehSpeed / g_fSpeedThreshold;
        if (fVehSpeedPercent > 1.0) fVehSpeedPercent = 1.0;

        // Calculate the counter steering percentage (counter proportional to the vehicle speed - the quicker, the less counter
        // steering).
        // The rear axle angle can change between -5.12...5.11 deg.
        fRearAxleAngle = 5.11f * -fSteeringWheelPercent * (1.0f - fVehSpeedPercent);
    }
    if (fRearAxleAngle == m_fRearAxleAngle) return;

    // Inform the rear exle about the new steering position.
    m_fRearAxleAngle = fRearAxleAngle;
    if (m_pRearAxleSvc) m_pRearAxleSvc->SetRearAxle(fRearAxleAngle);
}

void CCounterSteeringExampleService::TimerFunction()
{
    // Increase the counter (max 5 bits).
    m_uiAliveCounter++;
    if (m_uiAliveCounter > 31)
        m_uiAliveCounter = 0;

    // Inform the alive counter monitor service
    if (m_pAliveCounterSvc)
        m_pAliveCounterSvc->SetCounter(m_uiAliveCounter);
}
