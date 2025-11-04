#include <iostream>
#include <fstream>
#include <support/app_control.h>
#include <support/signal_support.h>
#include "signal_names.h"

#include "autoheadlight_simulate.h"
#include "autoheadlight_console.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain()
#else
extern "C" int main()
#endif
{
	CAutoHeadlightAppSimulate AppSimulate; 
	
	if(AppSimulate.Initialize()) //Initialize and if failed do not run the test run.
	{
		CConsole visual_obj;
		visual_obj.PrintHeader(); 
		visual_obj.PrepareDataConsumers(); // Get access to required services 
		visual_obj.StartUpdateDataThread(); // start a thread to get Headlight and tunnel information and print on console 

		AppSimulate.ExecuteTestRun(); // Execute the test run feeding driveway data 

		visual_obj.StopUpdateDataThread(); 
		visual_obj.ResetSignals();

		AppSimulate.Shutdown(); //Shutdown the app.
	}

	return 0;
}

