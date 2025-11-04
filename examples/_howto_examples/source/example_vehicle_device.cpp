#include "example_interfaces.h"
#include "example_reception_interfaces.h"
#include "example_transfer_interfaces.h"

#include <support/signal_support.h>
#include <support/component_impl.h>
#include <interfaces/dispatch.h>

#include <iostream>
#include <set>

class CVehicleDevice
    : public sdv::CSdvObject
    , public vss::Device::IReceptionSignalSpeed
    , public vss::Device::ITransferSignalBrakeForce
{
public:
    CVehicleDevice()
    {
        sdv::core::CDispatchService dispatch;
        m_SpeedSignal = dispatch.Subscribe("SPEED_SIGNAL_NAME", [&](sdv::any_t value) {  CallBackFunctionSpeedSignal(value); });		
        if (!m_SpeedSignal)
        {
            std::cout << "Speed signal not found" << std::endl;
            throw std::runtime_error("SpeedSignal not found");
        }

        m_TransferSignalBrakeForce = dispatch.AddPublisher("BRAKE_FORCE_SIGNAL_NAME");
        if (!m_TransferSignalBrakeForce)
        {
            std::cout << "BrakeForce signal not found" << std::endl;
            throw std::runtime_error("BrakeForce not found");
        }        
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(vss::Device::IReceptionSignalSpeed)
        SDV_INTERFACE_ENTRY(vss::Device::ITransferSignalBrakeForce)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("VehicleDevice_Component")

    ~CVehicleDevice()
    {
        if (m_SpeedSignal)
        {
            m_SpeedSignal.Reset();
        }
        if (m_TransferSignalBrakeForce)
        {
            m_TransferSignalBrakeForce.Reset();
        }
    }

    /**
    * @brief Register ReceptionSignalSpeed event on signal change
    * Collect all events and call them on signal change
    * @param[in] event function
    */
    void RegisterSpeedEvent(vss::Device::IReceptionSignalSpeed_Event* event) override
    {
        if (event)
        {
            std::cout << "register a SignalSpeedEvent ..." << std::endl;
            std::lock_guard<std::mutex> lock(m_SpeedMutexCallbacks);
            m_SpeedCallbacks.insert(event);
        }
    }

    /**
    * @brief Register ReceptionSignalSpeed event on signal change
    * Collect all events and call them on signal change
    * @param[in] event function
    */
    void UnRegisterSpeedEvent(vss::Device::IReceptionSignalSpeed_Event* event) override
    {
        if (event)
        {
            std::cout << "unregister a SignalSpeedEvent ..." << std::endl;
            std::lock_guard<std::mutex> lock(m_SpeedMutexCallbacks);
            m_SpeedCallbacks.erase(event);
        }
    }

    /**
     * @brief Set brake force
     * @param[in] value of the brake force
     * @return true on success otherwise false
     */
    bool SetBrakeForce(uint32_t value) override
    {
        if (m_TransferSignalBrakeForce)
        {
            m_TransferSignalBrakeForce.Write(value);
            return true;
        }
        return false;
    }

private:

    /**
    * @brief Execute all callbacks for the Speed Signal
    */
    void CallBackFunctionSpeedSignal(sdv::any_t value)
    {
        uint32_t speed = value.get<uint32_t>();
        std::cout << "CallBackFunction SpeedSignal: " << std::to_string(speed) << std::endl;
        std::lock_guard<std::mutex> lock(m_SpeedMutexCallbacks);
        for (auto callback : m_SpeedCallbacks)
        {
            callback->SetSpeedValue(speed);
        }
    }

    sdv::core::CSignal m_TransferSignalBrakeForce;                        ///< SignalBrakeForce of the abstract device
    sdv::core::CSignal m_SpeedSignal;                                     ///< SpeedSignal of the abstract device
    mutable std::mutex m_SpeedMutexCallbacks;                             ///< Mutex protecting m_SpeedCallbacks
    std::set<vss::Device::IReceptionSignalSpeed_Event*> m_SpeedCallbacks; ///< collection of IReceptionSignalSpeed events to be called
};

DEFINE_SDV_OBJECT(CVehicleDevice)
