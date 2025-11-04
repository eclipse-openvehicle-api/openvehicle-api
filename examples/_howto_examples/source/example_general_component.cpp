#include <iostream>

#include "example_interfaces.h"
#include <support/component_impl.h>

class CTestComponent
	: public sdv::CSdvObject
	, public ISayHello
	, public ISayGoodbye
{
public:
	CTestComponent()
	{
		std::cout << "Entering CTestComponent constructor..." << std::endl;
	}
	~CTestComponent() override
	{
		std::cout << "Entering CTestComponent destructor..." << std::endl;
	}

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(ISayHello)
		SDV_INTERFACE_ENTRY(ISayGoodbye)
	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("Hello_Component")

	/**
	* @brief Show hello message, implements the function of ISayHello
	*/
	void SayHello() override
	{
		std::cout << "Hello from Hello_Component ..." << std::endl;
	}
	
	/**
	* @brief Show hello message, implements the function of ISayGoodbye
	*/
	void SayGoodbye() override
	{
		std::cout << "Goodbye from Hello_Component ..." << std::endl;
	}
};

DEFINE_SDV_OBJECT(CTestComponent)
