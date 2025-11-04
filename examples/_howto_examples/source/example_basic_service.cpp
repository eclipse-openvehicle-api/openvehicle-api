#include "example_interfaces.h"
#include "example_reception_interfaces.h"
#include "example_transfer_interfaces.h"

#include <support/signal_support.h>
#include <support/component_impl.h>
#include <interfaces/dispatch.h>

#include <iostream>
#include <set>

class CBasicService
    : public sdv::CSdvObject
    , public vss::Device::IReceptionSignalSpeed_Event
    , public vss::Service::IReceptionSignalSpeed
    , public vss::Service::ITransferSignalBrakeForce
{
public:
    CBasicService()
    {
        auto device = sdv::core::GetObject("VehicleDevice_Component").GetInterface<vss::Device::IReceptionSignalSpeed>();
        if (!device)
        {
            SDV_LOG_ERROR("Could not get abstract device: [CBasicService]");
            throw std::runtime_error("VehicleDevice_Component device not found");
        }
        device->RegisterSpeedEvent(static_cast<vss::Device::IReceptionSignalSpeed_Event*> (this));

        m_BrakeForce = sdv::core::GetObject("VehicleDevice_Component").GetInterface<vss::Device::ITransferSignalBrakeForce>();
        if (!m_BrakeForce)
        {
            SDV_LOG_ERROR("Could not get abstract device: [CBasicService]");
            throw std::runtime_error("VehicleDevice_Component device not found");
        }

        SDV_LOG_TRACE("CBasicService created: [BasicService_Component]");
    }

    ~CBasicService()
    {
        auto device = sdv::core::GetObject("VehicleDevice_Component").GetInterface<vss::Device::IReceptionSignalSpeed>();
        if (device)
        {
            device->UnRegisterSpeedEvent(static_cast<vss::Device::IReceptionSignalSpeed_Event*> (this));
        }
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(vss::Service::IReceptionSignalSpeed)
        SDV_INTERFACE_ENTRY(vss::Device::IReceptionSignalSpeed_Event)
        SDV_INTERFACE_ENTRY(vss::Service::ITransferSignalBrakeForce)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_NAME("BasicService_Component")
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService);

    /**
     * @brief Set brake force
     * @param[in] value of the brake force
     * @return true on success otherwise false
     */
    bool SetBrakeForce(uint32_t value)  override
    {
        return m_BrakeForce->SetBrakeForce(value);
    }

    /**
     * @brief Set speed
     * @param[in] value speed
     */
    void SetSpeedValue(int32_t value) override
    {
        m_Speed = value;
        std::lock_guard<std::mutex> lock(m_mutexCallbacks);
        for (auto callback : m_Callbacks)
        {
            callback->SetSpeedValue(value);
        }
    }

    /**
     * @brief Get Speedspeed
     */
    int32_t GetSpeedValue()  override
    {
        return  m_Speed;
    }

    /**
     * @brief Register Callback on signal change
     * @param[in] callback function
     */
    void RegisterCallBack(/*in*/ vss::Device::IReceptionSignalSpeed_Event* callback) override
    {
        if (callback)
        {
            std::lock_guard<std::mutex> lock(m_mutexCallbacks);
            m_Callbacks.insert(callback);
        }
    }

    /**
     * @brief Unregister Callback
     * @param[in] callback function
     */
    void UnregisterCallBack(vss::Device::IReceptionSignalSpeed_Event* callback) override
    {
        if (callback)
        {
            std::lock_guard<std::mutex> lock(m_mutexCallbacks);
            m_Callbacks.erase(callback);
        }
    }

private:
    uint32_t                                            m_Speed = { 0 };        ///< speed value which will be received
    mutable std::mutex                                  m_mutexCallbacks;       ///< Mutex protecting m_Callbacks
    std::set<vss::Device::IReceptionSignalSpeed_Event*> m_Callbacks;            ///< collection of events to be called (received value)
    vss::Device::ITransferSignalBrakeForce*             m_BrakeForce = nullptr; ///< To write the brake force to the abstract device
};

DEFINE_SDV_OBJECT(CBasicService)
