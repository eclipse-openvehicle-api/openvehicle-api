#include <iostream>
#include <stdexcept>

#include "example_interfaces.h"

#include <support/component_impl.h>

class CAccessComponent
	: public sdv::CSdvObject
	, public IShowExample
{
public:
	CAccessComponent()
	{
		m_HelloInterface = sdv::core::GetObject("Hello_Component").GetInterface<ISayHello>();

		sdv::TInterfaceAccessPtr helloComp = sdv::core::GetObject("Hello_Component");
		m_GoodbyeInterface = helloComp.GetInterface<ISayGoodbye>();

		if (!m_HelloInterface || !m_GoodbyeInterface)
		{
			throw std::runtime_error("Could not get all needed interfaces!");
		}
	}

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(IShowExample)
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_NAME("Access_Component")
	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService);
	
	/**
	* @brief Show messages, implements the function of IShowExample
	*/
	void Show() override
	{
		m_HelloInterface->SayHello();
		m_GoodbyeInterface->SayGoodbye();
	}

private:
	ISayHello *   m_HelloInterface   = nullptr;
	ISayGoodbye * m_GoodbyeInterface = nullptr;
};

DEFINE_SDV_OBJECT(CAccessComponent)
