#include "../../global/process_watchdog.h"
#include "parser.h"
#include "core_idl_backup.h"
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include "exception.h"
#include "environment.h"
#include "generator/definition_generator.h"
#include "generator/proxy_generator.h"
#include "generator/stub_generator.h"
#include "generator/serdes_generator.h"
#include "generator/ps_cpp_generator.h"
#include "generator/cmake_generator.h"
#include <iostream>
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

    std::cout << "SDV IDL Compiler" << std::endl;
    std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
    std::cout << "Author: Erik Verhoeven" << std::endl;

    // Create a list of UTF-8 arguments
    std::vector<std::string> vecArgs;
    for (int iIndex = 0; iIndex < iArgc; iIndex++)
        vecArgs.push_back(sdv::MakeUtf8String(rgszArgv[iIndex]));

    // Parse the command line
    bool bCompilerError = false;
    CIdlCompilerEnvironment environment;
    try
    {
        environment = CIdlCompilerEnvironment(iArgc, rgszArgv);
    } catch (const sdv::idl::XCompileError& rexcept)
    {
        bCompilerError = true;
        std::stringstream sstreamError;
        sstreamError << "error: " << rexcept.ssReason << std::endl;
        g_log_control.Log(sstreamError.str(), true);
    }

    // Show help?
    if (environment.Help())
    {
        environment.ShowHelp();
        return bCompilerError ? CMDLN_ARG_ERR : NO_ERROR;
    }

    // Process files.
    size_t nCnt = 0;
    do
    {
        std::filesystem::path pathFile = environment.GetNextFile();
        if (pathFile.empty())
        {
            if (!nCnt)
                std::cout << "No files provided; nothing to do. Use \"idl_compiler --help\" to request command line information." <<
                std::endl;
            break;
        }
        nCnt++;

        try
        {
            if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                std::cout << "Processing: " << pathFile.generic_u8string() << std::endl;

            // Parse file
            CParser parser(pathFile, environment);
            parser.Parse();

            // Generate definition
            if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                std::cout << "Generating interface definition..."<< std::endl;
            CDefinitionGenerator defgen(&parser);
            defgen.Generate();

            if (!environment.NoProxyStub())
            {
                // Proxy code
                if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                    std::cout << "Generating proxy..." << std::endl;
                CProxyGenerator proxygen(&parser);
                proxygen.Generate();

                // Stub code
                if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                    std::cout << "Generating stub..." << std::endl;
                CStubGenerator stubgen(&parser);
                stubgen.Generate();

                // Proxy and stub CPP generation
                if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                    std::cout << "Generating proxy/stub CPP file..." << std::endl;
                CPsCppGenerator pscppgen(&parser);
                pscppgen.Generate();

                // CMake code generation
                if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                    std::cout << "Generating CMake file..." << std::endl;
                CIdlCompilerCMakeGenerator cmakegen(&parser);
                cmakegen.Generate(environment.GetProxStubCMakeTarget());
            }

            // Serdes code
            if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
                std::cout << "Generating serdes functions..." << std::endl;
            CSerdesGenerator serdesgen(&parser);
            serdesgen.Generate();
        }
        catch (const sdv::idl::XCompileError& rexcept)
        {
            bCompilerError = true;
            std::stringstream sstreamError;
            if (!rexcept.ssFile.empty())
                sstreamError << rexcept.ssFile << "(line=" << rexcept.uiLine << ", col=" << rexcept.uiCol <<
                ") error: " << rexcept.ssReason << std::endl;
            else
                sstreamError << "error: " << rexcept.ssReason << std::endl;
            g_log_control.Log(sstreamError.str(), true);
        }
    } while (true);

    if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_none)
        std::cout << "Done..." << std::endl;

    return bCompilerError ? COMPILE_ERROR : NO_ERROR;
}