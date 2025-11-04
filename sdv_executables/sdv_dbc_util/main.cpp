#include "../../global/process_watchdog.h"
#include <interfaces/core.h>
#include <interfaces/mem.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/dbcparser/dbcparser.cpp"
#include "../../global/localmemmgr.h"
#include <iostream>
#include "can_dl.h"
#include "fmu.h"
#include "cmake_generator.h"
#include "../error_msg.h"

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int iArgc, const wchar_t* rgszArgv[])
#else
extern "C" int main(int iArgc, const char* rgszArgv[])
#endif
{
    // Workaround for GCC to make certain that POSIX thread library is loaded before the components are loaded.
    // REASON: If the first call to a thread is done in a dynamic library, the application is already classified as single
    // threaded and a termination is initiated.
    // See: https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-leads-to-a-sigsev
    // NOTE EVE 27.05.2025: This task has been taken over by the process watchdog.
    CProcessWatchdog watchdog;

    // Suppress the warning supplied by ccpcheck about an unsused variable. The memory manager registers itself into the system and
    // needs to stay in scope.
    // cppcheck-suppress unusedVariable
    CLocalMemMgr memmgr;

    CCommandLine cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    bool bHelp = false;
    bool bError = false;
    bool bSilent = false;
    bool bVerbose = false;
    size_t nIfcIndex = 0;
    std::string ssIfcName;
    std::string ssVersion;
    std::string ssModelIdentifier;
    std::string ssDataLinkLibName = "can_dl";
    std::vector<std::filesystem::path> vecFileNames;
    std::vector<std::string> vecNodes;
    std::filesystem::path pathOutputDir;
    try
    {
        auto& rArgHelpDef = cmdln.DefineOption("?", bHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("version", ssVersion, "Optional: version information of the dbc file.");
        cmdln.DefineSubOption("module", ssModelIdentifier, "Optional: module identifier for the fmu. Default 'BasicModelIdentifier'");
        cmdln.DefineSubOption("dl_lib_name", ssDataLinkLibName, "Data link library target name in the generated cmake file (default=\"can_dl\")..");
        cmdln.DefineOption("O", pathOutputDir, "Set output directory");
        cmdln.DefineSubOption("ifc_idx", nIfcIndex, "Interface index this DBC file is aiming for.");
        cmdln.DefineSubOption("ifc_name", ssIfcName, "Name of the interface this DBC file is aiming for.");
        cmdln.DefineSubOption("nodes", vecNodes, "List of nodes that will be implemented.");
        cmdln.DefineDefaultArgument(vecFileNames, "One or more DBC files");
        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        std::cout << "ERROR: " << rsExcept.what() << std::endl;
        bHelp = true;
        bError = true;
    }

    if (!bSilent)
    {
        std::cout << "DBC utility" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl;
    }
    if (bHelp)
    {
        if (!bSilent)
            cmdln.PrintHelp(std::cout);
        return bError ? CMDLN_ARG_ERR : NO_ERROR;
    }

    dbc::CDbcParser parser;
    for (const std::filesystem::path& rpath : vecFileNames)
    {
        try
        {
            if (bVerbose)
                std::cout << "Processing: " << rpath.generic_u8string() << std::endl;
            dbc::CDbcSource src(rpath);
            parser.Parse(src);
            CSoftcarFMUGen fmu(pathOutputDir, parser, ssModelIdentifier, ssVersion, vecNodes);
            CCanDataLinkGen dl(pathOutputDir, parser, ssVersion, ssIfcName, nIfcIndex, vecNodes);
            CDbcUtilCMakeGenerator cmakegen(pathOutputDir, ssDataLinkLibName);
        }
        catch (dbc::SDbcParserException& rsException)
        {
            if (!bSilent)
                std::cout << "ERROR " << LOAD_DBC_FILE_ERROR_MSG << " Reason: " << rsException.what() << std::endl;
            return LOAD_DBC_FILE_ERROR;
        }
    }

    return NO_ERROR;
}
