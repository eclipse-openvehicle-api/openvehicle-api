#include <iostream>
#include <support/toml.h>
#include <support/component_impl.h>
#include "example_interfaces.h"

class CTestComponentWithInitialization
	: public sdv::CSdvObject
    , public sdv::IObjectControl
	, public ISayHello
	, public ISayGoodbye
{
public:
	CTestComponentWithInitialization()
	{
		std::cout << "Entering CTestComponentWithInitialization constructor..." << std::endl;
	}
	~CTestComponentWithInitialization() override
	{
		std::cout << "Entering CTestComponentWithInitialization destructor..." << std::endl;
	}

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(ISayHello)
		SDV_INTERFACE_ENTRY(ISayGoodbye)
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("Hello_Component_With_Initialization")

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    inline virtual void Initialize(const sdv::u8string& ssObjectConfig) override
    {
        if (!ParseConfigurationString(ssObjectConfig))
        {
            m_status = sdv::EObjectStatus::initialization_failure;
            return;
        }
        m_status = sdv::EObjectStatus::initialized;
    };

    /**
    * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
    * @param[in] eMode The operation mode, the component should run in.
    */
    void SetOperationMode(/*in*/ sdv::EOperationMode eMode) override
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
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    inline virtual sdv::EObjectStatus GetStatus() const override
    {
        return m_status;
    };

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    inline virtual void Shutdown() override
    {
        m_status = sdv::EObjectStatus::destruction_pending;
    }

    /**
    * @brief Show messages, implements the function of IShowExample
    */
    void SayHello() override
	{
		std::cout << "Hello from Hello_Component_With_Initialization ... " << std::to_string(m_Number) << std::endl;
	}

    /**
    * @brief Show messages, implements the function of ISayGoodbye
    */
    void SayGoodbye() override
	{
		std::cout << "Goodbye from Hello_Component_With_Initialization ... " << std::to_string(m_Number) << std::endl;
	}

private:
    bool ParseConfigurationString(const sdv::u8string& objectConfig)
    {
        try
        {
            sdv::toml::CTOMLParser config(objectConfig.c_str());

            // get any settings from the configuration
            auto nodeNumber = config.GetDirect("number");
            if (nodeNumber.GetType() == sdv::toml::ENodeType::node_integer)
            {
                m_Number = static_cast<int32_t>(nodeNumber.GetValue());
            }
        }
        catch (const sdv::toml::XTOMLParseException& e)
        {
            std::cout << "Configuration could not be read:" << e.what() << std::endl;
            return false;
        }

        std::cout << "Initialization number: " << std::to_string(m_Number) << std::endl;
        return true;
    }

    int32_t m_Number = -1;
    std::atomic<sdv::EObjectStatus> m_status = { sdv::EObjectStatus::initialization_pending }; //!< To update the object status when it changes.
};

DEFINE_SDV_OBJECT(CTestComponentWithInitialization)
