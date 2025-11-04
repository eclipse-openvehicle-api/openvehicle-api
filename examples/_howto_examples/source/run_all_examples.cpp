#include <iostream>
#include <support/signal_support.h>
#include <support/app_control.h>
#include "example_interfaces.h"
#include "example_reception_interfaces.h"
#include "example_transfer_interfaces.h"
#include "example_dispatch_service.cpp"

sdv::core::CSignal m_signalSpeedRx;
sdv::core::CSignal m_signalBrakeForceTx;

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

void UsageOfGeneralComponent()
{
    std::cout << "Running example of General Component use:" << std::endl;
    auto hello = sdv::core::GetObject("Hello_Component").GetInterface<ISayHello>();
    auto bye = sdv::core::GetObject("Hello_Component").GetInterface<ISayGoodbye>();
    if (!hello)
        std::cout << "Could not get hello interface" << std::endl;
    else
        hello->SayHello();
    if (!bye)
        std::cout << "Could not get bye interface" << std::endl;
    else
        bye->SayGoodbye();
    std::cout << "\n";
}

void UsageOfGeneralComponentWithInitialization()
{
    std::cout << "Running example of General Component  With Initialization, the number should be 42:" << std::endl;
    auto hello = sdv::core::GetObject("Hello_Component_With_Initialization").GetInterface<ISayHello>();
    auto bye = sdv::core::GetObject("Hello_Component_With_Initialization").GetInterface<ISayGoodbye>();
    if (!hello)
        std::cout << "Could not get hello interface" << std::endl;
    else
        hello->SayHello();
    if (!bye)
        std::cout << "Could not get bye interface" << std::endl;
    else
        bye->SayGoodbye();
    std::cout << "\n";
}

void UsageOfAbstractDeviceAndBasicService()
{
    std::cout << "Running example of Abstract Device Component use:" << std::endl;

    auto brakeForce = sdv::core::GetObject("BasicService_Component").GetInterface<vss::Service::ITransferSignalBrakeForce>();
    if (brakeForce)
    {
        brakeForce->SetBrakeForce(123);
        sdv::core::CDispatchService dispatch;
        auto transaction = dispatch.CreateTransaction();
        auto value = m_signalBrakeForceTx.Read(transaction).get<uint32_t>();
        transaction.Finish();
        std::cout << "Set Brake force 123 and read value: " << std::to_string(value) << std::endl;
    }
    else
    {
        std::cout << "No interface 'vss::Service::ITransferSignalBrakeForce' of the basic service " << std::endl;
    }

    std::cout << "Now a callback function should be called with 12345:" << std::endl;
    m_signalSpeedRx.Write(12345);

    auto speed = sdv::core::GetObject("BasicService_Component").GetInterface<vss::Service::IReceptionSignalSpeed>();
    if (speed)
    {
        auto value = speed->GetSpeedValue();
        std::cout << "Speed value should be 12345: " << std::to_string(value) << std::endl;
    }
    else
    {
        std::cout << "No interface 'vss::Service::IReceptionSignalSpeed' of the basic service " << std::endl;
    }

    std::cout << "Complex service exists?  " << std::to_string(sdv::core::GetObject("ComplexService_Component").IsValid()) << std::endl;
    std::cout << "\n";
}

void UsageOfDataDispatchServiceAccess(CExampleDispatchService* exampleDispatchService)
{
    uint32_t value01{ 0 };
    uint32_t value02{ 0 };
    uint32_t value03{ 0 };
    std::cout << "Running example of dispatchservice\n";
    std::cout << "Start: " << std::to_string(value01) << " " << std::to_string(value02) << " " << std::to_string(value03) << " " << std::endl;

    exampleDispatchService->GetTxSignalValue("after creation", value01, value02, value03);

    exampleDispatchService->WriteToSignalAsPublisher(17, 18, 19);
    exampleDispatchService->GetTxSignalValue("after 17, 18, 19 written", value01, value02, value03);
    exampleDispatchService->GetTxSignalValue("after nothing changed", value01, value02, value03);

    exampleDispatchService->WriteToSignalAsPublisher(456, 455, 454);
    exampleDispatchService->GetTxSignalValue("after 456, 455, 454 written", value01, value02, value03);
    
    std::cout << "Write 77 to RX signal: " << std::endl;
    exampleDispatchService->DataLinkWriter(77);

    std::cout << "\n";
}

bool InitializeAppControl(sdv::app::CAppControl* appcontrol)
{
    auto bResult = appcontrol->AddModuleSearchDir("../../bin");
    bResult &= appcontrol->Startup("");
    appcontrol->SetConfigMode();
    if (appcontrol->LoadConfig("docu_dispatch.toml") != sdv::core::EConfigProcessResult::successful)
    {
        std::cout << "dispatch service could be loaded" << std::endl;
    }

    sdv::core::CDispatchService dispatch;
    m_signalSpeedRx = dispatch.RegisterRxSignal("SPEED_SIGNAL_NAME");  ///< signals for abstract device
    m_signalBrakeForceTx = dispatch.RegisterTxSignal("BRAKE_FORCE_SIGNAL_NAME", 0);
    if (!m_signalSpeedRx || !m_signalBrakeForceTx)
    {
        std::cout << "Signal for Speed and Brake Force  could not be registered" << std::endl;
        bResult = false;
    }

    if (appcontrol->LoadConfig("docu_examples.toml") != sdv::core::EConfigProcessResult::successful)
    {
        std::cout << "Not all examples could be loaded" << std::endl;
        bResult = false;
    }

    return  bResult;
}

void AddPuplisherAndSubscriper(CExampleDispatchService* exampleDispatchService)
{
    exampleDispatchService->AddPublisherSignals();
    exampleDispatchService->SubscripSignals();
    std::cout << "\n";
}

#if defined(_WIN32) && defined(_UNICODE)
extern "C"  int wmain()
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

    std::cout << "Run documentation code examples:\n- Initialization/Configuration mode.\n" << std::endl;
    InitializeAppControl(&appcontrol);

    CExampleDispatchService exampleDispatchService;
    AddPuplisherAndSubscriper(&exampleDispatchService); // for dispatch example, must be done before appcontrol.SetRunningMode();
    appcontrol.SetRunningMode();
    std::cout << "- Set running mode\n" << std::endl;

    std::cout << "------------------------------------------------\n---Running UsageOfDataDispatchServiceAccess():" << std::endl;
    UsageOfDataDispatchServiceAccess(&exampleDispatchService);

    std::cout << "------------------------------------------------\n---Running UsageOfGeneralComponent():" << std::endl;
    UsageOfGeneralComponent();

    std::cout << "------------------------------------------------\n---Running UsageOfGeneralComponentWithInitialization():" << std::endl;
    UsageOfGeneralComponentWithInitialization();

    std::cout << "------------------------------------------------\n---Running UsageOfAbstractDeviceAndBasicService():" << std::endl;
    UsageOfAbstractDeviceAndBasicService();
    
    appcontrol.Shutdown();
    std::cout << "\n\nTest finished." << std::endl;
    return 0;
}
