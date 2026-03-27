#include <iostream>
#include <support/toml.h>
#include <support/component_impl.h>
#include "example_interfaces.h"

class CTestComponentWithInitialization
	: public sdv::CSdvObject
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
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
	DECLARE_OBJECT_CLASS_NAME("Hello_Component_With_Initialization")

    // Parameter map
    BEGIN_SDV_PARAM_MAP()
        SDV_PARAM_ENTRY(m_Number, "number", -1, "", "A number")
    END_SDV_PARAM_MAP()

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}

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
    int32_t m_Number = -1;
};

DEFINE_SDV_OBJECT(CTestComponentWithInitialization)
