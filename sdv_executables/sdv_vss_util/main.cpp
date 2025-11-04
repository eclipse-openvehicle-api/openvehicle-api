#include <iostream>
#include <unordered_map>

#include "../../global/process_watchdog.h"
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "../../global/dbcparser/dbcparser.cpp"
#include "../../global/localmemmgr.h"
#include "../error_msg.h"
#include <interfaces/core.h>
#include <interfaces/mem.h>

#include "csv_file_reader.h"
#include "vss_vd_generator.h"
#include "vss_bs_generator.h"
#include "vss_helper.h"

void makeSignalNamesUnique(std::vector< CVSSHelper::SSignalVDDefinition>& vdSignals)
{
    std::unordered_map<std::string, int> nameCount;
    std::vector<std::string> allUniqueNames;

    auto process = [&](std::vector< CVSSHelper::SSignalVDDefinition>& signals)
    {
        for (auto& signal : signals)
        {
            for (auto& func : signal.vecFunctions)
            {
                std::string& name = func.signalName;
                std::string originalName = name;
                while (nameCount[name] > 0)
                {
                    name = originalName + "_" + std::to_string(nameCount[originalName]);
                    nameCount[originalName]++;
                }
                nameCount[name]++;
                allUniqueNames.push_back(name);
            }
        }        
    };

    process(vdSignals);
}

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int iArgc, const wchar_t* rgszArgv[])
#else
extern "C" int main(int iArgc, const char* rgszArgv[])
#endif
{
    // Workaround for GCC to make certain that POSIX thread library is loaded before the components are loaded.
    // REASON: If the first call to a thread is done in a dynamic library, the application is already classified as single
    // threaded and a termination is initiated.
    // See: https://stackoverflow.com/questions/51209268/using-stdthread-in-a-library-loaded-with-dlopen-levds-to-a-sigsev
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
    bool bEnableComponents = false;
    std::string ssVersion;
    std::string ssPrefix;
    std::string ssVSSFileName;
    std::filesystem::path pathOutputDir;
    try
    {
        auto& rArgHelpDef = cmdln.DefineOption("?", bHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("prefix", ssPrefix, "prefix, used by cmake library and signal definition in signal_identifier.h file.");
        cmdln.DefineSubOption("enable_components", bEnableComponents, "Creates additionally to the idl files the code for the components.");
        cmdln.DefineSubOption("version", ssVersion, "Optional: version information of the dbc file.");
        cmdln.DefineOption("O", pathOutputDir, "Set output directory (required).");
        cmdln.DefineDefaultArgument(ssVSSFileName, "Excel file.");
        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        std::cout << "ERROR: " << rsExcept.what() << std::endl;
        bHelp = true;
        bError = true;
    }
    if (bSilent)
    {
        bVerbose = false;
    }
    if (!bSilent)
    {
        std::cout << "abtract device / basic services utility" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Thomas Pfleiderer" << std::endl;
    }

    if (bHelp || (ssVSSFileName.empty() || pathOutputDir.empty() || ssPrefix.empty()))
    {
        if (!bSilent)
        {
            cmdln.PrintHelp(std::cout);
            //CCSVFileReader csv("", bSilent, bVerbose);
        }
        return bError ? CMDLN_ARG_ERR : NO_ERROR;
    }

    CCSVFileReader csv(ssVSSFileName, bSilent, bVerbose);
    auto vdSignals = csv.GetVDSignals();
    auto bsSignals = csv.GetBSSignals();
    if (!CVSSHelper::VehicleDeviceVSSDefinitionExists(bSilent, vdSignals, bsSignals))
    {
        return BASIC_SERVICE_DATA_ERROR;
    }

    if (vdSignals.size() > 0)
    {
        makeSignalNamesUnique(vdSignals);
        CVSSVDGenerator vssVD(vdSignals, pathOutputDir, ssPrefix, ssVersion, bEnableComponents);
        vssVD.GeneratedCode();

        if (bsSignals.size() > 0)
        {
            CVSSBSGenerator vssBS(bsSignals, vdSignals, pathOutputDir, ssPrefix, ssVersion, bEnableComponents);
            vssBS.GeneratedCode();
        }
    }

    return NO_ERROR;
}
