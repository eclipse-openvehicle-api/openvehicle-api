#include <string>
#include <support/app_control.h>
#include <support/signal_support.h>

/**
* @brief Application Class of the door example
*/
class CDoorControl
{
public:

    /**
    * @brief Start and initialize the application control and load vehicle devices and 
    * basic services depending on the number of doors
    * @param[in] numberOfDoors number of doors (1-4), default 4
    * @return Return true on success otherwise false
    */
    bool Initialize(const uint32_t numberOfDoors = 4);

    /**
    * @brief After initialization/configuration the system mode needs to be set to running mode
    */
    void SetRunningMode();

    /**
    * @brief Ask user for input how many doors the vehicle should have (1-4)
    * @return Return number of doors (default 4)
    */
    uint32_t UserInputNumberOfDoors();

    /**
    * @brief Run loop as long as user input does not exit
    * Allow user to open/close each door.
    */
    void RunUntilBreak();

    /**
    * @brief Shutdown the system.
    */
    void Shutdown();

    /**
    * @brief Get number of doors
    * @return Return number of doors (default 4)
    */
    uint32_t GetNumberOfDoors() const;

private:

    /**
    * @brief check if SDV_FRAMEWORK_RUNTIME environment variable exists
    * @return Return true if environment variable is found otherwise false
    */
    bool IsSDVFrameworkEnvironmentSet();

    /**
     * @brief Loac config file and register vehicle device and basic service.
     * @remarks It is expected that each config file has the complete door:
     * vehicle device & basic service for input and output
     * @param[in] inputMsg message string to be printed on console in case of success and failure
     * @param[in] configFileName config toml file name
     * @return Return true on success otherwise false
     */
    bool LoadConfigFile(const std::string& inputMsg, const std::string& configFileName);


    /**
     * @brief Key hit check. Windows uses the _kbhit function; POSIX emulates this.
     * @return Returns whether a key has been pressed.
     */
    bool KeyHit();

    /**
     * @brief Get the character from the keyboard buffer if pressed.
     * @return Returns the character from the keyboard buffer.
     */
    char GetChar();

    /**
     * @brief Register/Create Signals in the dispatch service depending on the number of doors
     * @return Return true on success otherwise false
     */
    bool RegisterSignals();

    sdv::app::CAppControl   m_appcontrol;                      ///< App-control of SDV V-API.
    bool                    m_bInitialized = false;            ///< Set when initialized.
    uint32_t                m_iNumberOfDoors = 4;              ///< Number iof doors, maximuum 4

    sdv::core::CSignal      m_SignalFrontLeftDoorIsOpen;       ///< Front Left Door signal (RX input) - open / closed
    sdv::core::CSignal      m_SignalFrontRightDoorIsOpen;      ///< Front Right Door signal (RX input) - open / closed
    sdv::core::CSignal      m_SignalRearLeftDoorIsOpen;        ///< Rear Left Door signal (RX input) - open / closed
    sdv::core::CSignal      m_SignalRearRightDoorIsOpen;       ///< Rear Right Door signal (RX input) - open / closed

    sdv::core::CSignal      m_SignalFrontLeftDoorIsLocked;     ///< Front Left Latch signal (TX output) - locked / unlocked
    sdv::core::CSignal      m_SignalFrontRightDoorIsLocked;    ///< Front Right Latch signal (TX output) - locked / unlocked
    sdv::core::CSignal      m_SignalRearLeftDoorIsLocked;      ///< Rear Left Latch signal (TX output) - locked / unlocked
    sdv::core::CSignal      m_SignalRearRightDoorIsLocked;     ///< Rear Right Latch signal (TX output) - locked / unlocked

};
