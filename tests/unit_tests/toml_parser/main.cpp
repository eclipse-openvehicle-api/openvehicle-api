#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include "../../../sdv_services/core/toml_parser/character_reader_utf_8.cpp"
#include "../../../sdv_services/core/toml_parser/lexer_toml.cpp"
#include "../../../sdv_services/core/toml_parser/parser_toml.cpp"
#include "../../../sdv_services/core/toml_parser/parser_node_toml.cpp"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
    CProcessWatchdog watchdog;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
