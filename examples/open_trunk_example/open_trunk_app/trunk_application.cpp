#include "trunk_application.h"
#include "signal_names.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

bool CTrunkControl::LoadConfigFile(const std::string& inputMsg, const std::string& configFileName)
{
    std::string msg = inputMsg;
    if (m_appcontrol.LoadConfig(configFileName) == sdv::core::EConfigProcessResult::successful)
    {
         msg.append("ok\n");
        std::cout << msg.c_str();
        return true;
    }

    msg.append("FAILED.\n");
    std::cout << msg.c_str();
    return false;
}

bool CTrunkControl::IsSDVFrameworkEnvironmentSet()
{
    const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
    if (envVariable)
    {
        return true;
    }

    return false;
}

bool CTrunkControl::Initialize(uint32_t uiInstance)
{
    if (m_bInitialized) 
        return true;

    if (!IsSDVFrameworkEnvironmentSet())
    {
        // if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
        m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
    }

    if (uiInstance != 0)
    {
        std::stringstream sstreamAppConfig;
        sstreamAppConfig << "[Application]" << std::endl;
        sstreamAppConfig << "Mode=\"External\"" << std::endl;
        sstreamAppConfig << "Instance=" << uiInstance << std::endl;
        sstreamAppConfig << "Retries=" << 6 << std::endl;
        sstreamAppConfig << "[Console]" << std::endl;
        sstreamAppConfig << "Report=\"Silent\"" << std::endl;
        if (!m_appcontrol.Startup(sstreamAppConfig.str()))
            return false;
    }
    else
    {
        if (!m_appcontrol.Startup(""))
            return false; 

        // Switch to config mode.
        m_appcontrol.SetConfigMode();
        bool bResult = LoadConfigFile("Load dispatch service: ", "data_dispatch_trunk.toml");
        bResult &= LoadConfigFile("Load task_timer_trunk: ", "task_timer_trunk.toml");

        bResult &= LoadConfigFile("Load can_com_simulation_trunk: ", "can_com_simulation_trunk.toml");
        bResult &= LoadConfigFile("Load data_link_trunk: ", "data_link_trunk.toml");

        bResult &= LoadConfigFile("Load trunk_vehicle_device_and_basic_service: ", "trunk_vehicle_device_and_basic_service.toml");
        bResult &= LoadConfigFile("Load trunk service (complex service): ", "complex_service_trunk.toml");

        if (!bResult) 
        {
		    SDV_LOG_ERROR("One or more configurations could not be loaded. Cannot continue.");
            return false;
        }
    }

    m_bInitialized = true;
    return true;
}


void CTrunkControl::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

void CTrunkControl::SetRunningMode()
{
    m_appcontrol.SetRunningMode();
}

