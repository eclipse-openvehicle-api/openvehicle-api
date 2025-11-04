#ifndef VAPI_DUMMY_TEST_SERVICE_H
#define VAPI_DUMMY_TEST_SERVICE_H

#include <support/component_impl.h>

/**
 * @brief  Class to create a dummy test service.
 */
class CLoggerTestService
	: public sdv::CSdvObject
{
public:
	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("LoggerTestService")

	CLoggerTestService();

};

DEFINE_SDV_OBJECT(CLoggerTestService)


#endif // !define VAPI_DUMMY_TEST_SERVICE_H
