#include <iostream>
#include <support/app_control.h>
#include "example_interfaces.h"
#include "example_reception_interfaces.h"
#include "example_transfer_interfaces.h"

/**
* @brief check if SDV_FRAMEWORK_RUNTIME environment variable exists
* @return Return true if environment variable is found otherwise false
*/
bool IsSDVFrameworkEnvironmentSet()
{
	const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
	if (envVariable)
	{
		return true;
	}
	return false;
}

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain()
#else
extern "C" int main()
#endif
{
	sdv::app::CAppControl appcontrol;
	if (!IsSDVFrameworkEnvironmentSet())
	{
		// if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
		appcontrol.SetFrameworkRuntimeDirectory("../../bin");
	}

	auto bResult = appcontrol.AddModuleSearchDir("../../bin");
	bResult &= appcontrol.Startup("");

	appcontrol.SetConfigMode();
	bResult &= appcontrol.AddConfigSearchDir("config");
	bResult &= appcontrol.LoadConfig("docu_app_examples.toml") == sdv::core::EConfigProcessResult::successful;
	if (!bResult)
	{
		std::cout << "Exit, Could not load docu_examples.toml." << std::endl;
		appcontrol.Shutdown();
		return 0;
	}

	appcontrol.SetRunningMode();
	auto hello1 = sdv::core::GetObject("Hello_Component").GetInterface<ISayHello>();
	auto bye1 = sdv::core::GetObject("Hello_Component").GetInterface<ISayGoodbye>();
	auto hello2 = sdv::core::GetObject("Hello_Component_With_Initialization").GetInterface<ISayHello>();
	auto bye2 = sdv::core::GetObject("Hello_Component_With_Initialization").GetInterface<ISayGoodbye>();
	if (!hello1 || !bye1 || !hello2 || !bye2)
	{
		hello1->SayHello();
		bye1->SayGoodbye();
		hello2->SayHello();
		bye2->SayGoodbye();
	}
	else
	{
		std::cout << "Could not get all interfaces interface" << std::endl;
	}

	appcontrol.Shutdown();
	return 0;
}
