#include <iostream>
#include <fstream>
#include <support/app_control.h>
#include <thread>

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
	auto bResult = appcontrol.Startup("");
	appcontrol.SetConfigMode();

	bResult &= appcontrol.AddConfigSearchDir("config");
	bResult &= appcontrol.LoadConfig("test_configuration_example.toml") == sdv::core::EConfigProcessResult::successful;

	if (!bResult)
	{
		std::cout << "Exit, Could not load configuration component." << std::endl;
		appcontrol.Shutdown();
	}

	appcontrol.SetRunningMode();

	appcontrol.Shutdown();

	return 0;
}
