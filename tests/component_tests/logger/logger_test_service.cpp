#include "logger_test_service.h"

CLoggerTestService::CLoggerTestService()
{
	// auto test = sdv::core::GetObject("DataDispatchService");
	// auto foo = test.GetInterface<sdv::core::IDataDispatchService>();
	// foo->HasSignal(0);

	SDV_LOG(sdv::core::ELogSeverity::info, "Info: Logging from Dummy test service via macro!");

}
