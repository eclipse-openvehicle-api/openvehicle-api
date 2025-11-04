#include <support/sdv_core.h>
#include <stdlib.h>
#include "../../../global/exec_dir_helper.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain([[maybe_unused]] int argc, [[maybe_unused]] wchar_t* argv[])
#else
extern "C" int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
#endif
{
    sdv::core::internal::CSDVCoreLoader loader;
    bool bResult = !static_cast<sdv::TInterfaceAccessPtr>(loader) ? true : false;
    std::cout << "Child process before loading: core interface availability: " << (bResult ? "not available" : "available") << std::endl;
    loader.Load();
    bResult &= static_cast<sdv::TInterfaceAccessPtr>(loader) ? true : false;
    std::ofstream fstream(GetExecDirectory() / "CoreLoading_result.txt");
    fstream << bResult;
    std::cout << "Child process after loading: core interface availability: " << (bResult ? "available" : "not available") << std::endl;
    return 0;
}
