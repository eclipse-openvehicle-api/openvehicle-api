#include <string>
#include <support/app_control.h>
#include <support/signal_support.h>
#include "vss_vehiclespeed_bs_rx.h"


/**
* @brief Application Class of the open trunk example
*/
class CTrunkControl
{
public:

    /**
    * @brief Start and initialize the application control and load vehicle devices and basic services
    * @param[in] uiInstance Instance number the application will connect to. 0 will start a standalone application 
    * @return Return true on success otherwise false
    */
    bool Initialize(uint32_t uiInstance);

    /**
    * @brief After initialization/configuration the system mode needs to be set to running mode
    */
    void SetRunningMode();

    /**
    * @brief Shutdown the system.
    */
    void Shutdown();

private:

    /**
    * @brief check if SDV_FRAMEWORK_RUNTIME environment variable exists
    * @return Return true if environment variable is found otherwise false
    */
    bool IsSDVFrameworkEnvironmentSet();

    /**
     * @brief Loac config file and register vehicle device and basic service.
     * @param[in] inputMsg message string to be printed on console in case of success and failure
     * @param[in] configFileName config toml file name
     * @return Return true on success otherwise false
     */
    bool LoadConfigFile(const std::string& inputMsg, const std::string& configFileName);

    sdv::app::CAppControl m_appcontrol;           ///< App-control of SDV V-API.
    bool                  m_bInitialized = false; ///< Set when initialized.
};
