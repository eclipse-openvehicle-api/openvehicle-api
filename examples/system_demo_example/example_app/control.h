#ifndef EXMAPLE_UTILITY_H
#define EXMAPLE_UTILITY_H

#include <string>
#include <support/app_control.h>
#include <support/signal_support.h>

#ifdef _WIN32

#include <codecvt>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#include "signal_names.h"

 /**
  * @brief Utility handler for example demos
  */
class CExampleControl
{
public:
    /**
     * @brief Constructor with the command line arguments.
     * @tparam TCharType Character type of the command line arguments.
     * @param[in] iArgs Amount of arguments.
     * @param[in] rgszArgs Array of argument strings. The first argument represents the application name.
     */
    template <typename TCharType>
    CExampleControl(int iArgs, const TCharType rgszArgs[]);

    /**
     * @brief Destructor.
     */
    ~CExampleControl();

    /**
     * @brief Initialize application control based on the command line arguments provided through the constructor.
     * @return Return true on success; false when not.
     */
    bool Initialize();

    /**
     * @brief Shutdown the system.
     */
    void Shutdown();

    /**
     * @brief Is the control initialized?
     * @return Returns whether control is initialized.
     */
    bool IsInitialized() const;

    /**
     * @brief Register signals, required by the vehicle devices.
     * @remarks Only valid for standalone applications.
     * @pre Initialization must be done before.
     * @param[in] inputMsg message string to be printed on console in case of success and failure
     * @param[in] configFileName config toml file name
     * @return Return true on success otherwise false
     */
    bool LoadConfigFile(const std::string& inputMsg, const std::string& configFileName);

    /**
     * @brief Start the test run if not running before.
     * @details Write the input signals. In case of data link the signals are written from a asc file by the can_com_sim.sdv component.
     * Otherwise the signals are create and can be written directly.
     */
    void StartTestRun();

    /**
     * @brief Stops a test run if currently running.
     */
    void StopTestRun();

    /**
     * @brief Did a command line error occur?
     * @return Returns whether a command line error occurred.
     */
    bool HasCommandLineError() const;

    /**
     * @brief Was command line help requested?
     * @return Returns whether a command line help was requested.
     */
    bool HasRequestedCommandLineHelp() const;

    /**
     * @brief Application operation.
     */
    enum class ERunAs
    {
        standalone,             ///< This application runs as standalone using data link and can_com_sim.sdv which reads the CAN messages from an asc file.
        standalone_simulated,   ///< This application runs as standalone simulating data read/write without datalink.
        server_connect,         ///< Connect to a server and run as a client application.
    };

    /**
     * @brief Get the current application operation (retrieved from command line).
     * @return The application operation.
     */
    ERunAs GetAppOperation() const;

    /**
     * @brief Is the simulation mode activated
     * @return Returns whether simulation or data link is active.
     */
    bool IsSimulationMode() const;

    /**
     * @brief Does the application run as a standalone application?
     * @return Returns whether the application runs as a standalone application.
     */
    bool RunAsStandaloneApp() const;

    /**
     * @brief Does the application connect to a server and run on the server?
     * @return Returns whether the application runs on the server.
     */
    bool RunAsServerApp() const;

    /**
     * @brief Register signals for datalink simulation; required by the vehicle devices.
     * @return Returns 'true' on success; 'false' on failure.
     */
    bool RegisterSignalsSimDatalink();

    /**
     * @brief Reset signals for datalink simulation; needed for a proper shutdown.
     */
    void ResetSignalsSimDatalink();

private:
    /**
     * @brief Provide simulated signals until m_bRunning is disabled.
     */
    void SimulateDatalinkThreadFunc();

    /**
     * @brief Reporting by the SDV V-API application control.
     */
    enum class EAppControlReporting
    {
        silent,         ///< No reporting by application control (default)
        normal,         ///< Normal reporting by application control
        verbose,        ///< Extensive reporting by application control
    };

    bool                    m_bInitialized = false;             ///< Set when initialized.
    bool                    m_bCmdLnError = false;              ///< Command line error occurred.
    bool                    m_bCmdLnHelp = false;               ///< Command line help provided.
    bool                    m_bRunning = false;                 ///< When set, the application is running.
    ERunAs                  m_eRunAs = ERunAs::standalone;      ///< Application operation.
    EAppControlReporting    m_eReporting = EAppControlReporting::silent; ///< Application control reporting.
    uint32_t                m_uiInstance = 1000u;               ///< Server instance to connect to.
    std::filesystem::path   m_pathFramework;                    ///< Path to the SDV V-API framework.
    sdv::app::CAppControl   m_appcontrol;                       ///< App-control of SDV V-API.
    std::thread             m_threadSimulateDatalink;           ///< Simulation datalink thread.
    sdv::core::CSignal      m_signalSteeringWheel;              ///< Steering wheel angle signal (input) - simulated datalink
    sdv::core::CSignal      m_signalSpeed;                      ///< Speed signal (input) - simulated datalink
    sdv::core::CSignal      m_signalRearAngle;                  ///< Rear angle signal (output) - simulated datalink
    sdv::core::CSignal      m_signalCounter;                    ///< Simple counter signal (output) - simulated datalink
};

template <typename TCharType>
CExampleControl::CExampleControl(int iArgs, const TCharType rgszArgs[])
{
    bool bStandalone = false;
    bool bStandaloneSimulated = false;
    bool bServerConnect = false;
    bool bSilent = false;
    bool bNormal = false;
    bool bVerbose = false;
    for (int i = 1; i < iArgs; i++)
    {
#ifdef _WIN32
        std::wstring wide_string = rgszArgs[i]; 
        size_t size_needed = std::wcstombs(nullptr, wide_string.c_str(), 0);
        std::string ssArg(size_needed, 0);
        std::wcstombs(&ssArg[0], wide_string.c_str(), size_needed);
#else
        std::string ssArg = rgszArgs[i];
#endif

#ifdef _WIN32
        if (ssArg == "/?" || ssArg == "--help" || ssArg == "-?")
#else
        if (ssArg == "--help" || ssArg == "-?")
#endif
        {
            m_bCmdLnHelp = true;
            continue;
        }
        if (ssArg.substr(0, 10) == "--instance")
        {
            m_uiInstance = std::stoul(ssArg.substr(10));
            continue;
        }
#ifdef _WIN32
        if (ssArg.substr(0, 2) == "/f" || ssArg.substr(0, 2) == "-f")
#else
        if (ssArg.substr(0, 2) == "-f")
#endif
        {
            m_pathFramework = std::filesystem::u8path(ssArg.substr(2));
            continue;
        }
        if (ssArg == "--standalone")
        {
            bStandalone = true;
            continue;
        }
        if (ssArg == "--connect")
        {
            bServerConnect = true;
            continue;
        }
        if (ssArg == "--simulate")
        {
            bStandaloneSimulated = true;
            continue;
        }

#ifdef _WIN32
        if (ssArg == "/s" || ssArg == "-s" || ssArg == "--silent")
#else
        if (ssArg == "-s" || ssArg == "--silent")
#endif
        {
            bSilent = true;
            continue;
        }
#ifdef _WIN32
        if (ssArg == "/n" || ssArg == "-n" || ssArg == "--normal")
#else
        if (ssArg == "-n" || ssArg == "--normal")
#endif
        {
            bSilent = true;
            continue;
        }
#ifdef _WIN32
        if (ssArg == "/v" || ssArg == "-v" || ssArg == "--verbose")
#else
        if (ssArg == "-v" || ssArg == "--verbose")
#endif
        {
            bVerbose = true;
            continue;
        }

        // Argument not known
        std::cerr << "ERROR: unknown argument '" << ssArg << "'" << std::endl;
        m_bCmdLnError = true;
    }

    // Check for an invalid flag.
    if (!m_bCmdLnError && (bStandalone || bStandaloneSimulated) && bServerConnect)
    {
        std::cerr << "ERROR: the application can run either as standalone application of connect to a server.";
        m_bCmdLnError = true;
    }

    // Set to server connect if requested.
    if (bServerConnect)
        m_eRunAs = ERunAs::server_connect;
    if (bStandaloneSimulated)
        m_eRunAs = ERunAs::standalone_simulated;

    // Check console output flags
    if ((bSilent && bNormal) ||
        (bSilent && bVerbose) ||
        (bNormal && bSilent))
    {
        std::cerr << "ERROR: multiple settings for the SDV V-API application control console output.";
        m_bCmdLnError = true;
    }

    // Set control app reporting mode if requested.
    if (bNormal)
        m_eReporting = EAppControlReporting::normal;
    if (bVerbose)
        m_eReporting = EAppControlReporting::verbose;

    // Check for the framework runtime location
    if (!m_bCmdLnError && m_pathFramework.empty())
    {
        const char* szSDVFramework = std::getenv("SDV_FRAMEWORK_RUNTIME");
        if (szSDVFramework)
            m_pathFramework = std::filesystem::u8path(szSDVFramework);
        else
        {
            std::cerr << "ERROR: Missing path to SDV V-API framework. Use commandline argument -f or set "
                "SDV_FRAMEWORK_RUNTIME environment variable." << std::endl;
            m_bCmdLnError = true;
        }
    }
    if (!m_bCmdLnError && !std::filesystem::is_directory(m_pathFramework) && !std::filesystem::exists(m_pathFramework / "core_services.sdv"))
    {
        std::cerr << "ERROR: Invalid path to SDV V-API framework." << std::endl;
        m_bCmdLnError = true;
    }

    // Print help or error information
    if (m_bCmdLnHelp || m_bCmdLnError)
    {
        std::cout << R"text(
  Usage: system_demo_example <options>

    --help, -?             Show help
    --instance<no>         Set instance number for server connection (default = 1000).
    -f<framework_location> Location of SDV V-API framework if SDV_FRAMEWORK_RUNTIME hasn't been set.
    --standalone           Run as standalone application (data link layer through can_com_sim.sdv which reads the CAN messages from an asc file).
    --simulate             Run as standalone application and simulate the data link layer.
    --connect              Connect to running server.
    --silent, -s           Start the SDV application control with console output in silent mode (default).
    --normal, -n           Start the SDV application control with console output in normal mode.
    --verbose, -v          Start the SDV application control with console output in verbose mode.

)text";
    }}


#endif // ! defined EXMAPLE_UTILITY_H
