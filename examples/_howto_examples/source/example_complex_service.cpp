#include "example_reception_interfaces.h"
#include "example_transfer_interfaces.h"

#include <support/signal_support.h>
#include <support/component_impl.h>
#include <interfaces/dispatch.h>

#include <iostream>
#include <set>

class CComplexService
    : public sdv::CSdvObject
    , public vss::Device::IReceptionSignalSpeed_Event
    //
    // complex service should offer interface(s) to be used by applications
    //
{
public:
    CComplexService()
    {
        m_SrvBrakeForce = sdv::core::GetObject("BasicService_Component").GetInterface<vss::Service::ITransferSignalBrakeForce>();
        if (!m_SrvBrakeForce)
        {
            SDV_LOG_ERROR("Could not get basic service: [CComplexService]");
            throw std::runtime_error("Brake force service not found");
        }

        auto serviceSpeed = sdv::core::GetObject("BasicService_Component").GetInterface<vss::Service::IReceptionSignalSpeed>();
        if (serviceSpeed)
        {
            serviceSpeed->RegisterCallBack(static_cast<vss::Device::IReceptionSignalSpeed_Event*> (this));
        }
        else
        {
            SDV_LOG_ERROR("Could not get basic service: [CComplexService]");
            throw std::runtime_error("Speed service not found");
        }
    }

    ~CComplexService()
    {
        auto serviceSpeed = sdv::core::GetObject("BasicService_Component").GetInterface<vss::Service::IReceptionSignalSpeed>();
        if (serviceSpeed)
        {
            serviceSpeed->UnregisterCallBack(static_cast<vss::Device::IReceptionSignalSpeed_Event*> (this));
        }
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(vss::Device::IReceptionSignalSpeed_Event)
        //
        // complex service should offer interface(s) to be used by applications
        //
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_NAME("ComplexService_Component")
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService);

    /**
    * @brief Set speed
    * @param[in] value speed
    */
    void SetSpeedValue(int32_t value) override
    {
        if (value > m_SpeedMaximum)
        {
            //Forward brake force to the basic service
            m_SrvBrakeForce->SetBrakeForce(value);
        }
    }

private:
    int32_t                                  m_SpeedMaximum = { 100 };  ///< maximum allowed speed in this service implementation
    int32_t                                  m_Speed = { 0 };           ///< speed value which will be received
    vss::Service::ITransferSignalBrakeForce* m_SrvBrakeForce = nullptr; ///< To write the brake force the basic service
};

DEFINE_SDV_OBJECT(CComplexService)
