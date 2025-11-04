#include <iostream>
#include <fstream>
#include <chrono>
#include <support/timer.h>
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
	bResult &= appcontrol.LoadConfig("test_config_task_timer.toml") == sdv::core::EConfigProcessResult::successful;

	if (!bResult)
	{
		std::cout << "Exit, Could not load task timer component." << std::endl;
		appcontrol.Shutdown();
	}

	appcontrol.SetRunningMode();

	//run for 10 seconds - in that time task timer callbacks in the module get triggered
	uint32_t counter = 10;
	std::chrono::seconds sleepDurtaion (1);
	while (counter-- > 0)
	{
		std::cout << "counter: " << std::to_string(counter) << std::endl;
		std::this_thread::sleep_for(sleepDurtaion);
	}

	appcontrol.Shutdown();

	return 0;
}
