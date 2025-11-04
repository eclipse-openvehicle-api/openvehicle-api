// App process providing repeating functionality

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include "pattern_gen.h"
#include <support/mem_access.h>
#include <support/app_control.h>
#include <../global/base64.h>
#include "../../../sdv_services/ipc_shared_mem/shared_mem_buffer_posix.h"
#include "../../../sdv_services/ipc_shared_mem/shared_mem_buffer_windows.h"
#include "../../../sdv_services/ipc_shared_mem/mem_buffer_accessor.cpp"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    if (argc < 2)
    {
        std::cout << "Missing connection string..." << std::endl;
        return -1;
    }

    sdv::app::CAppControl appcontrol;
    if (!appcontrol.Startup("")) return -1;
    appcontrol.SetRunningMode();

    std::string logName("appRepeater.log");

    // Create a connection string from the command line arguments separated by spaces. Skip the first argument, since it holds the
    // module file name.

    // The connection information needs to be defined as the seconds argument.
    std::string ssConnectString;
    if (argc > 1)
        ssConnectString = Base64DecodePlainText(sdv::MakeUtf8String(argv[1]));

    // The "SURROGATE_TRANSMITTER" argument, if provided, must be provided as third argument.
    bool bTransmitter = argc > 2 && sdv::MakeUtf8String(argv[2]) == "SURROGATE_TRANSMITTER";
    if (bTransmitter)
    {
        logName = "AppTransmitter.log";
        std::cout << "Start of transmitter process..." << std::endl;
    } else
        std::cout << "Start of repeater process..." << std::endl;

    std::ofstream log(logName.c_str(), std::ios::trunc);
    log << "Start of app process..."  << std::endl;

    CSharedMemBufferRx bufferRX(ssConnectString);
    CSharedMemBufferTx bufferTX(ssConnectString);
    if (!bufferRX.IsValid() || !bufferTX.IsValid())
    {
        log << "Invalid connection string..." << std::endl;
        log << "Connection string: " << std::endl << ssConnectString << std::endl;
        log << "RX Error: " << bufferRX.GetError() << std::endl;
        log << "TX Error: " << bufferTX.GetError() << std::endl;
        std::cout << "Error app process..." << std::endl;
        std::cout << "Process param: " << sdv::MakeUtf8String(argv[1]) << std::endl;
        std::cout << "  Connection string: " << std::endl << ssConnectString << std::endl;
        std::cout << "  RX Error: " << bufferRX.GetError() << std::endl;
        std::cout << "  TX Error: " << bufferTX.GetError() << std::endl;
        return -1;
    }

    if (bTransmitter)
    {
        log << "App process acts like transmitter of the pattern:\n" << std::endl;

        CPatternReceiver pattern_inspector(bufferRX);
        CPatternSender	 pattern_generator(bufferTX);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        pattern_generator.Shutdown();
        pattern_inspector.Shutdown();

        log << "Pattern generator " << pattern_generator.GetCycleCnt() << " cycles..." << std::endl;
        log << "Pattern generator " << pattern_generator.GetByteCnt() << " bytes..." << std::endl;
        log << "Pattern inspector " << pattern_inspector.GetCycleCnt() << " cycles..." << std::endl;
        log << "Pattern inspector " << pattern_inspector.GetByteCnt() << " bytes..." << std::endl;

        if (!pattern_generator.GetByteCnt() && !pattern_inspector.GetByteCnt() && pattern_inspector.GetErrorCnt())
        {
            log << "Error: unexpected " << pattern_generator.GetByteCnt() << " generator bytes, " << pattern_inspector.GetErrorCnt() << " inspector errors,"
                << pattern_inspector.GetByteCnt() << " inspector bytes." << std::endl;
        }

        std::cout << "Pattern generator: " << pattern_generator.GetCycleCnt() << " cyles, " << pattern_generator.GetPacketCnt()
            << " packets, " << pattern_generator.GetByteCnt() << " bytes" << std::endl;
        std::cout << "Pattern inspector: " << pattern_inspector.GetCycleCnt() << " cyles, " << pattern_inspector.GetPacketCnt()
            << " packets, " << pattern_inspector.GetByteCnt() << " bytes, " << pattern_inspector.GetErrorCnt()
            << " errors, " << std::endl;
    }
    else
    {
        CPatternRepeater pattern_repeater(bufferRX, bufferTX);

        std::this_thread::sleep_for(std::chrono::seconds(PATTERN_TEST_TIME_S * 1));

        pattern_repeater.Shutdown();

        log << "Processed " << pattern_repeater.GetCycleCnt() << " cycles..." << std::endl;
        log << "Processed " << pattern_repeater.GetPacketCnt() << " packets..." << std::endl;
        log << "Processed " << pattern_repeater.GetByteCnt() << " bytes..." << std::endl;
        log << "Detected " << pattern_repeater.GetErrorCnt() << " errors..." << std::endl;

        if (!pattern_repeater.GetPacketCnt() && !pattern_repeater.GetByteCnt() && pattern_repeater.GetErrorCnt())
        {
            log << "Error: unexpected " << pattern_repeater.GetPacketCnt() << " packets, "
                << pattern_repeater.GetByteCnt() << " bytes, " << pattern_repeater.GetErrorCnt() << " errors." << std::endl;
        }

        std::cout << "Pattern repeater: " << pattern_repeater.GetCycleCnt() << " cyles, " << pattern_repeater.GetPacketCnt()
            << " packets, " << pattern_repeater.GetByteCnt() << " bytes, "  << pattern_repeater.GetErrorCnt()
            << " errors..." << std::endl;
    }

    appcontrol.Shutdown();

    return 0;
}
