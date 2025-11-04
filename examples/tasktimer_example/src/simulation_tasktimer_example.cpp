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
	bResult &= appcontrol.LoadConfig("test_config_simulation_task_timer.toml") == sdv::core::EConfigProcessResult::successful;

	if (!bResult)
	{
		std::cout << "Exit, Could not load task timer component." << std::endl;
		appcontrol.Shutdown();
	}

	appcontrol.SetRunningMode();

	// Get the task timer service.
	sdv::core::ITimerSimulationStep* pTimerSimulationStep = sdv::core::GetObject<sdv::core::ITimerSimulationStep>("SimulationTaskTimerService");
	if (!pTimerSimulationStep)
	{
		std::cout << "---------------------- Error, Timer step not available " << std::endl;
	}

	//run for 3 seconds - in that time task timer callbacks in the module get triggered
	uint32_t counter = 2000;
	std::chrono::milliseconds sleepDuration (5);
	while (counter-- > 0)
	{
		if (counter % 20 == 1)
			std::cout << "counter: " << std::to_string(counter) << std::endl;
		if (pTimerSimulationStep)
		{
			pTimerSimulationStep->SimulationStep(1000);
		}

		std::this_thread::sleep_for(sleepDuration);
	}

	appcontrol.Shutdown();

	return 0;
}
