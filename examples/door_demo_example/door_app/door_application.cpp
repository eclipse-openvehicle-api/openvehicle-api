#include "../door_app/include/door_application.h"
#include "../door_app/include/signal_names.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

bool CDoorControl::LoadConfigFile(const std::string& inputMsg, const std::string& configFileName)
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


bool CDoorControl::KeyHit()
{
#ifdef _WIN32
    return _kbhit();
#elif __unix__
    int ch = getchar();
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
#endif
}

char CDoorControl::GetChar()
{
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    return getchar();
#endif
}

bool CDoorControl::RegisterSignals()
{
    sdv::core::CDispatchService dispatch;

    m_SignalFrontLeftDoorIsOpen = dispatch.RegisterRxSignal(doors::dsLeftDoorIsOpen01);
    m_SignalFrontLeftDoorIsLocked = dispatch.RegisterTxSignal(doors::dsLeftLatch01, 0);

    if (m_iNumberOfDoors > 1)
    {
        m_SignalFrontRightDoorIsOpen = dispatch.RegisterRxSignal(doors::dsRightDoorIsOpen01);
        m_SignalFrontRightDoorIsLocked = dispatch.RegisterTxSignal(doors::dsRightLatch01, 0);

        if (m_iNumberOfDoors > 2)
        {
            m_SignalRearLeftDoorIsOpen = dispatch.RegisterRxSignal(doors::dsLeftDoorIsOpen02);
            m_SignalRearLeftDoorIsLocked = dispatch.RegisterTxSignal(doors::dsLeftLatch02, 0);

            if (m_iNumberOfDoors > 3)
            {
                m_SignalRearRightDoorIsOpen = dispatch.RegisterRxSignal(doors::dsRightDoorIsOpen02);
                m_SignalRearRightDoorIsLocked = dispatch.RegisterTxSignal(doors::dsRightLatch02, 0);
            }
        }
    }
    
    return true;
}


bool CDoorControl::IsSDVFrameworkEnvironmentSet()
{
    const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
    if (envVariable)
    {
        return true;
    }
    return false;
}

bool CDoorControl::Initialize(const uint32_t numberOfDoors)
{
    if (m_bInitialized) 
        return true;

    if ((numberOfDoors >= 1) && (numberOfDoors <= 4))
    {
        m_iNumberOfDoors = numberOfDoors;
    }

    if (!IsSDVFrameworkEnvironmentSet())
    {
        // if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
        m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
    }

    if(!m_appcontrol.Startup(""))
        return false; 

    // Switch to config mode.
    m_appcontrol.SetConfigMode();
    bool bResult = LoadConfigFile("Load dispatch example: ", "data_dispatch_example.toml");

    bResult &= LoadConfigFile("Load task timer: ", "task_timer_example.toml");
    bResult &= RegisterSignals();

    bResult &= LoadConfigFile("Load vehicle devices and basic services for front left door: ", "front_left_door_example.toml");
    if (m_iNumberOfDoors > 1)
    {
        bResult &= LoadConfigFile("Load vehicle devices and basic services for front right door: ", "front_right_door_example.toml");
        if (m_iNumberOfDoors > 2)
        {
            bResult &= LoadConfigFile("Load vehicle devices and basic services for rear left door: ", "rear_left_door_example.toml");
            if (m_iNumberOfDoors > 3)
            {
                bResult &= LoadConfigFile("Load vehicle devices and basic services for rear right door: ", "rear_right_door_example.toml");
            }
        }
    }
    
    bResult &= LoadConfigFile("Load door service (complex service): ", "door_comple_service.toml");

    if (!bResult) 
    {
		SDV_LOG_ERROR("One or more configurations could not be loaded. Cannot continue.");
        return false;
    }

    m_bInitialized = true;
    return true;
}

uint32_t CDoorControl::GetNumberOfDoors() const
{
    return m_iNumberOfDoors;
}

void CDoorControl::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

void CDoorControl::SetRunningMode()
{
    m_appcontrol.SetRunningMode();
}


void CDoorControl::RunUntilBreak()
{
    bool bRunning = true;
    bool openFrontLeftDoor = true;
    bool openFrontRightDoor = true;
    bool openRearLeftDoor = true;
    bool openRearRightDoor = true;

    // Update console by writing the first value if available
    if (m_SignalFrontLeftDoorIsOpen)
        m_SignalFrontLeftDoorIsOpen.Write<bool>(openFrontLeftDoor);
    if (m_SignalFrontRightDoorIsOpen)
        m_SignalFrontRightDoorIsOpen.Write<bool>(openFrontRightDoor);
    if (m_SignalRearLeftDoorIsOpen)
        m_SignalRearLeftDoorIsOpen.Write<bool>(openRearLeftDoor);
    if (m_SignalRearRightDoorIsOpen)
        m_SignalRearRightDoorIsOpen.Write<bool>(openRearRightDoor);

    while (bRunning)
    {
        // Check for a key
        if (!KeyHit())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Get a keyboard value (if there is any).
        char c = GetChar();
        switch (c)
        {
        case '1':
            openFrontLeftDoor = !openFrontLeftDoor;
            if (m_SignalFrontLeftDoorIsOpen)
                m_SignalFrontLeftDoorIsOpen.Write<bool>(openFrontLeftDoor);
            break;
        case '2':
            openFrontRightDoor = !openFrontRightDoor;
            if (m_SignalFrontRightDoorIsOpen)
                m_SignalFrontRightDoorIsOpen.Write<bool>(openFrontRightDoor);
            break;
        case '3':
            openRearLeftDoor = !openRearLeftDoor;
            if (m_SignalRearLeftDoorIsOpen)
                m_SignalRearLeftDoorIsOpen.Write<bool>(openRearLeftDoor);
            break;
        case '4':
            openRearRightDoor = !openRearRightDoor;
            if (m_SignalRearRightDoorIsOpen)
                m_SignalRearRightDoorIsOpen.Write<bool>(openRearRightDoor);
            break;
        case 'x':
        case 'X':
            bRunning = false;
            break;
        default:
            break;
        }
    }
}

uint32_t CDoorControl::UserInputNumberOfDoors()
{
    uint32_t numberOfDoors = 4;
    // Clear the screen and goto top...
    std::cout << "\x1b[2J\033[0;0H";
    std::cout << "How many doors does the vehicle have? Press a number between 1 and 4: ";

    // Get a keyboard value (if there is any).
    char c = GetChar();
    switch (c)
    {
    case '1':
        numberOfDoors = 1;
        break;
    case '2':
        numberOfDoors = 2;
        break;
    case '3':
        numberOfDoors = 3;
        break;
    case '4':
        break;
    default:
        break;
    }
    return  numberOfDoors;
}
