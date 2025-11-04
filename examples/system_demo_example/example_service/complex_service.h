#ifndef COMPLEX_SERVICE_EXAMPLE_H
#define COMPLEX_SERVICE_EXAMPLE_H

// C++ library
#include <iostream>

// SDV framework support
#include <support/component_impl.h>
#include <support/signal_support.h>
#include <support/timer.h>

// VSS interfaces - located in ../generated/vss_files/include
#include "vss_vehiclechassisrearaxlerowwheel_bs_tx.h"
#include "vss_vehiclechassissteeringwheelangle_bs_rx.h"
#include "vss_vehiclesoftwareapplicationisactivecounter_bs_tx.h"
#include "vss_vehiclespeed_bs_rx.h"

// Complex service counter steering interface - located in ../generated/example_service
#include "countersteering.h"

/**
 * @brief Counter steering example service
 * @details This complex service provides a simple counter steering example (rear axle steering to reduce the turning radius of the
 * vehicle) that will be activated dependable on the speed. For a speed of 30 km/s or less, the steering wheel angle is
 * counter-steering the rear axle. The amount of counter-steering is proportional to the speed (the lower the speed the more
 * counter-steering takes place).
 * Input events from basic service: steering wheel angle (-16...16 rad)
 *									vehicle speed (0...128 m/s)
 * Output calls for basic service:	rear axle angle (-5.12...5.11 deg)
 *									alive counter (increase every 10 ms)
 * Input calls for applications:    counter steering enabled (true/false)
 * Output info for applications:	rear axle angle (-5.12...5.11 deg)
 *                                  counter steering active (true/false)
 *									counter steering enabled (true/false)
 */
class CCounterSteeringExampleService :
    public sdv::CSdvObject,
    public sdv::IObjectControl,
    public vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event,
    public vss::Vehicle::SpeedService::IVSS_SetSpeed_Event,
    public ICounterSteeringService
{
public:
    /**
     * @brief Constructor
     */
    CCounterSteeringExampleService();

    /**
     * @brief Destructor
     */
    ~CCounterSteeringExampleService();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event)
        SDV_INTERFACE_ENTRY(vss::Vehicle::SpeedService::IVSS_SetSpeed_Event)
        SDV_INTERFACE_ENTRY(ICounterSteeringService)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("Counter Steering Example Service")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Set steering angle event. Overload of
     * vss::Vehicle::Chassis::SteeringWheel::AngleService::IVSS_SetSteeringWheel_Event::SetSteeringWheel.
     * @param[in] value Steering wheel angle in radials (-16...16 rad)
     */
    virtual void SetSteeringWheel(float value) override;

    /**
     * @brief Speed event. Overload of vss::Vehicle::SpeedService::IVSS_SetSpeed_Event::SetSpeed.
     * @param[in] value Vehicle speed in m/s (0... 128 m/s)
     */
    virtual void SetSpeed(float value) override;

    /**
     * @brief Activate or deactivate the counter steering service. Overload of ICounterSteeringService::ActivateService.
     * @param[in] bActivate When set, the service will be activated; otherwise deactivated.
     */
    virtual void ActivateService(/*in*/ bool bActivate) override;

    /**
     * @brief Is the counter steering service currently activated? Overload of ICounterSteeringService::IsActivated.
     * @return Returns whether the counter steering service is currently activated.
     */
    virtual bool IsActivated() const override;

    /**
     * @brief Does counter steering currently take place (speed > 30 km/s)? Overload of
     * ICounterSteeringService::CounterSteeringActive.
     * @return Returns whether counter steering currently takes place.
     */
    virtual bool CounterSteeringActive() const override;

    /**
     * @brief The calculated rear steering axle position. Overload of ICounterSteeringService::RearAxleAngle.
     * @return Returns the rear steering axle position in degrees (or 0 when not active).
     */
    virtual double RearAxleAngle() const override;

private:
    /**
     * @brief Update the rear axle angle baed on the stored input.
     */
    void UpdateRearAxleAngle();

    /**
     * @brief Timer function, function will be called by the timer, default every 10 ms
     */
    void TimerFunction();

    sdv::EObjectStatus      m_eStatus = sdv::EObjectStatus::initialization_pending; ///< Current object status
    volatile float          m_fSteeringWheel = 0.0;         ///< Steering wheel angle
    volatile float          m_fVehSpeed = 0.0;              ///< Vehicle speed
    volatile float          m_fRearAxleAngle = 0.0;         ///< Output rear angle
    uint8_t                 m_uiAliveCounter = 0u;          ///< Output alive counter
    bool	                m_bActivated = false;           ///< Is the service activated?
    sdv::core::CTaskTimer   m_Timer;                        ///< Timer for alive counter (10 ms)

    ///< Alive counter interface.
    vss::Vehicle::Software::Application::IsActiveCounterService::IVSS_SetCounter* m_pAliveCounterSvc = nullptr;

    ///< Rear axle interface.
    vss::Vehicle::Chassis::RearAxle::Row::WheelService::IVSS_SetRearAxle* m_pRearAxleSvc = nullptr;
};

DEFINE_SDV_OBJECT(CCounterSteeringExampleService)

#endif // !define COMPLEX_SERVICE_EXAMPLE_H
