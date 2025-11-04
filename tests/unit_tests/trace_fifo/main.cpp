#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../global/localmemmgr.h"
#include "../../../global/tracefifo/trace_fifo.cpp"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    CLocalMemMgr memmgr;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
