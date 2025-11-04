#include "../door_app/include/door_extern_application.h"
#include "../door_app/include/signal_names.h"

#ifdef _WIN32
#include <conio.h>      // Needed for _kbhit
#else
#include <fcntl.h>
#endif

bool CDoorExternControl::Initialize()
{
    if (m_bInitialized) 
        return true;

    if (!IsSDVFrameworkEnvironmentSet())
    {
        // if SDV_FRAMEWORK_RUNTIME environment variable is not set we need to set the Framework Runtime directory
        m_appcontrol.SetFrameworkRuntimeDirectory("../../bin");
    }

    std::stringstream sstreamAppConfig;
    sstreamAppConfig << "[Application]" << std::endl;
    sstreamAppConfig << "Mode=\"External\"" << std::endl;
    sstreamAppConfig << "Instance=\"3002\"" << std::endl;
    sstreamAppConfig << "Retries=" << 6 << std::endl;
    sstreamAppConfig << "[Console]" << std::endl;
    sstreamAppConfig << "Report=\"Silent\"" << std::endl;

    if (!m_appcontrol.Startup(sstreamAppConfig.str()))
        return false;

    m_bInitialized = true;
    return true;
}

void CDoorExternControl::Shutdown()
{
    if (!m_bInitialized)
        m_appcontrol.Shutdown();
    m_bInitialized = false;
}

void CDoorExternControl::RunUntilBreak()
{
    bool bRunning = true;

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
        case 'x':
        case 'X':
            bRunning = false;
            break;
        default:
            break;
        }
    }
}

bool CDoorExternControl::IsSDVFrameworkEnvironmentSet()
{
    const char* envVariable = std::getenv("SDV_FRAMEWORK_RUNTIME");
    if (envVariable)
    {
        return true;
    }
    return false;
}

bool CDoorExternControl::KeyHit()
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

char CDoorExternControl::GetChar()
{
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    return getchar();
#endif
}
