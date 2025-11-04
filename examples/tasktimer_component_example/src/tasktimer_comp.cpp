#include <iostream>
#include <chrono>
#include <support/component_impl.h>
#include <support/timer.h>
#include <support/toml.h>

class DemoTimerComponent : public sdv::CSdvObject, public sdv::IObjectControl
{
  public:
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("Timer_Example")

    /**
    * @brief initialize function to register, access the task timer interface from platform abstraction.
    * After initialization 'CreateTimer' function is called to execute the task periodically.
    * @param[in] ssObjectConfig An object configuration is currently not used by this demo component.
    */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig) override
    {
        try
        {
            sdv::toml::CTOMLParser config(ssObjectConfig.c_str());
            sdv::toml::CNode timerNode = config.GetDirect("Timer");
            if (timerNode.GetType() == sdv::toml::ENodeType::node_integer)
            {
                m_PeriodicValue = static_cast<uint32_t>(timerNode.GetValue());
            }
        }
        catch (const sdv::toml::XTOMLParseException& e)
        {
            SDV_LOG_ERROR("Parser error: ", e.what());

            m_status = sdv::EObjectStatus::initialization_failure;
            return;
        }

        m_Timer = sdv::core::CTaskTimer(m_PeriodicValue, [&]() {CustomerExecute(); });
        if (!m_Timer)
        {
            SDV_LOG_ERROR("Tasktimer with ", std::to_string(m_PeriodicValue), " milliseconds could not be created.");
            m_status = sdv::EObjectStatus::initialization_failure;
            return;
        }
        else
        {
            SDV_LOG_INFO("Tasktimer created with ", std::to_string(m_PeriodicValue), " milliseconds");
        }
        m_status = sdv::EObjectStatus::initialized;
    };

    /**
    * @brief Gets the current status of the object
    * @return EObjectStatus The current status of the object
    */
    virtual sdv::EObjectStatus GetStatus() const override
    {
        return m_status;
    };

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(/*in*/ sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_status == sdv::EObjectStatus::running || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_status == sdv::EObjectStatus::configuring || m_status == sdv::EObjectStatus::initialized)
                m_status = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
    * @brief Shutdown function is to shutdown the execution of periodic task.
    * Timer ID of the task is used to shutdown the specific task.
    */
    virtual void Shutdown() override
    {
        if (m_Timer)
        {
            m_Timer.Reset();
        }
        m_status = sdv::EObjectStatus::destruction_pending;
    }

    /**
    * @brief Execute function contains the task to be executed in provided period.
    * This function will be called periodically. Currently it's kept simple which just prints the current timestamp to the console.
    */
	void CustomerExecute() const
	{
		std::cout << "Executing the task with timestamp:" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
	};

  private:
    std::atomic<sdv::EObjectStatus> m_status = {sdv::EObjectStatus::initialization_pending}; //!< To update the object status when it changes.
    sdv::core::CTaskTimer m_Timer;             ///< timer 
    uint32_t m_PeriodicValue = 10;             ///< periodix in milliseconds
};

DEFINE_SDV_OBJECT(DemoTimerComponent)
