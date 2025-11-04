#include <string>
#include <support/app_control.h>
#include <support/signal_support.h>

/**
* @brief Application Class of the door example
*/
class CDoorExternControl
{
public:

    /**
    * @brief Start and initialize the application control and load vehicle devices and 
    * basic services depending on the numerb of doors
    * @return Return true on success otherwise false
    */
    bool Initialize();


    /**
    * @brief Run loop as long as user input does not exit
    * Allow user to open/close each door.
    */
    void RunUntilBreak();

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
     * @brief Key hit check. Windows uses the _kbhit function; POSIX emulates this.
     * @return Returns whether a key has been pressed.
     */
    bool KeyHit();

    /**
     * @brief Get the character from the keyboard buffer if pressed.
     * @return Returns the character from the keyboard buffer.
     */
    char GetChar();

    sdv::app::CAppControl   m_appcontrol;                      ///< App-control of SDV V-API.
    bool                    m_bInitialized = false;            ///< Set when initialized.
    uint32_t                m_iNumberOfDoors = 4;              ///< Number iof doors, maximuum 4

};
