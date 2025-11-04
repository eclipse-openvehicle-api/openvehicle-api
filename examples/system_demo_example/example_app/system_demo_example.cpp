#ifdef __unix__
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#endif

#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include "control.h"
#include "console.h"

#ifdef _WIN32

// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static HANDLE stdoutHandle, stdinHandle;
static DWORD outModeInit, inModeInit;

void setupConsole(void) {
    DWORD outMode = 0, inMode = 0;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);

    if(stdoutHandle == INVALID_HANDLE_VALUE || stdinHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
}

    if(!GetConsoleMode(stdoutHandle, &outMode) || !GetConsoleMode(stdinHandle, &inMode)) {
        exit(GetLastError());
    }

    outModeInit = outMode;
    inModeInit = inMode;

    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Set stdin as no echo and unbuffered
    inMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

    if(!SetConsoleMode(stdoutHandle, outMode) || !SetConsoleMode(stdinHandle, inMode)) {
        exit(GetLastError());
    }
}

void restoreConsole(void) {
    // Reset colors
    printf("\x1b[0m");

    // Reset console mode
    if(!SetConsoleMode(stdoutHandle, outModeInit) || !SetConsoleMode(stdinHandle, inModeInit)) {
        exit(GetLastError());
    }
}
#else

static struct termios orig_term;
static struct termios new_term;

void setupConsole(void) {
    tcgetattr(STDIN_FILENO, &orig_term);
    new_term = orig_term;

    new_term.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restoreConsole(void) {
    // Reset colors
    printf("\x1b[0m");

    // Reset console mode
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}
#endif

void getCursorPosition(int *row, int *col) {
    printf("\x1b[6n");
    char buff[128];
    int indx = 0;
    for(;;) {
        int cc = getchar();
        buff[indx] = (char)cc;
        indx++;
        if(cc == 'R') {
            buff[indx + 1] = '\0';
            break;
        }
    }
    sscanf(buff, "\x1b[%d;%dR", row, col);
    fseek(stdin, 0, SEEK_END);
}

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
{
#else
extern "C" int main(int argc, char* argv[])
{
#endif

    std::cout << "System demo example" << std::endl;
    std::cout << "----------------------------------------------------------------------------" << std::endl;

    // Start application control
    CExampleControl control(argc, argv);
    if (control.HasCommandLineError()) return -1;           // Command line error occurred; cannot continue.
    if (control.HasRequestedCommandLineHelp()) return 0;    // Command line help requested; job done.
    if (!control.Initialize())
    {
        std::cerr << "ERROR: Failed to initialize application control." << std::endl;
        return -2;
    }

    // The console provides output.
    CConsole console(control.RunAsStandaloneApp());

    // Load configuration files when running as standalone
    if (control.RunAsStandaloneApp())
    {
        bool bResult = control.LoadConfigFile("Load dispatch example: ", "data_dispatch_example.toml");
        bResult &= control.LoadConfigFile("Load task timer: ", "task_timer_example.toml");
        if (control.GetAppOperation() == CExampleControl::ERunAs::standalone)
        {
            std::cout << "Datalink enabled, load CAN Simulation device and datalink component." << std::endl;
            bResult &= control.LoadConfigFile("Load can_com_simulation: ", "can_com_simulation.toml");            
            bResult &= control.LoadConfigFile("Load data link: ", "data_link_example.toml");
        }
        else
        {
            std::cout << "Datalink disabled, register the required signals for a simulated datalink." << std::endl;
            control.RegisterSignalsSimDatalink();
        }

        bResult &= control.LoadConfigFile("Load vehicle_devices_basic_services_example: ", "vehicle_devices_basic_services_example.toml");
        bResult &= control.LoadConfigFile("Load complex_service_example: ", "complex_service_example.toml");
        if (!bResult)
        {
            std::cerr << std::endl << "ERROR: One or more configurations were not able to load. Cannot continue." << std::endl;
            control.Shutdown();
            return -3;
        }
    }

    // Print the application header
    console.PrintHeader(control.RunAsServerApp(), control.IsSimulationMode());

    // Prepare for data consumption.
    if (!console.PrepareDataConsumers())
    {
        std::cerr << std::endl << "ERROR: Cannot instantiate the data consumers. Cannot continue." << std::endl;
        control.Shutdown();
        return -4;
    }

    // Start the test run
    control.StartTestRun();

    // Run until break.
    console.RunUntilBreak();

    // Finish test run
    control.StopTestRun();

    // Reset the signals for a simulated datalink
    if (control.GetAppOperation() == CExampleControl::ERunAs::standalone_simulated)
        control.ResetSignalsSimDatalink();

    // Shutdown the example control
    control.Shutdown();

    return 0;
}
