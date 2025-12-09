#ifndef SDV_CORE_H
#define SDV_CORE_H

#ifndef DONT_LOAD_CORE_TYPES
#include "../interfaces/core.h"
#include "../interfaces/module.h"
#endif
#include "interface_ptr.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <functional>

#ifdef _WIN32
// Resolve conflict
#pragma push_macro("interface")
#pragma push_macro("GetObject")
#undef interface

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <objbase.h>

// Resolve conflict
#pragma pop_macro("interface")
#pragma pop_macro("GetObject")
#ifdef GetClassInfo
#undef GetClassInfo
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#elif defined __unix__
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#else
#error OS is not supported!
#endif

namespace sdv
{
    namespace core
    {
        namespace internal
        {
            /**
             * @brief The SDV core loader
            */
            class CSDVCoreLoader
            {
            public:
                /**
                 * @brief Default constructor.
                 * @remarks The constructor doesn't load the library, since this could cause a deadlock during the initialization
                 * phase. Loading is done through the Load function.
                 */
                CSDVCoreLoader() = default;

                /**
                 * @brief Destructor unloading the core
                 */
                ~CSDVCoreLoader()
                {
                    // Free the library
                    if (m_tModule)
#ifdef _WIN32
                        FreeLibrary(reinterpret_cast<HMODULE>(m_tModule));
#elif defined __unix__
                        dlclose(reinterpret_cast<void*>(m_tModule));
#else
#error OS not supported!
#endif
                }

                /**
                 * @brief Load the SDV core library.
                 * @details The function searches for the library core_services in different locations:
                 * First it searches in the directory of the executable for the library.
                 * Then it searches in the directory of the executable for a configuration file with the library location.
                 * The configuration file is called sdv_core_reloc.toml and should contain a section at least the line:
                 * directory=[path] whereas [path] represents the path to the core library. Other information in the file is
                 * ignored. Then it searches in the environment variable SDV_FRAMEWORK_RUNTIME for the location of the library.
                 * Last it searches in the path for the location of the library.
                 */
                void Load()
                {
                    if (m_bInit) return;    // Prevent trying to load again.
                    m_bInit = true;
                    bool bRelocFileError = false;
                    std::string ssRelocFileInfo = "";
                    std::string ssEnvironmentInfo = "";

                    // Check for the executable directory
                    std::filesystem::path pathCoreLib;
                    if (std::filesystem::exists(GetExecDirectory() / "core_services.sdv"))
                        pathCoreLib = GetExecDirectory() / "core_services.sdv";

                    if (pathCoreLib.empty())
                    {
                        if (std::filesystem::exists(GetExecDirectory() / "sdv_core_reloc.toml"))
                        {
                            // Check for the library in the relocation directory
                            auto relocFolder = GetRelocationPath();
                            if (!relocFolder.empty())
                            {
                                auto coreLibFolder = std::filesystem::path(relocFolder) / "core_services.sdv";
                                if (coreLibFolder.is_relative())
                                     coreLibFolder = (GetExecDirectory() / coreLibFolder).lexically_normal();

                                if (std::filesystem::exists(coreLibFolder))
                                    pathCoreLib = coreLibFolder;                                
                            }
                            if (pathCoreLib.empty())
                            {
                                bRelocFileError = true;  // we found the file but not the core library, we must run into an error
                                ssRelocFileInfo = "Error: Invalid \"sdv_core_reloc.toml\" file found (but no core library), it contains: " + relocFolder;
                            }
                        }
                    }

                    if (pathCoreLib.empty())
                    {
#ifdef _WIN32
                        std::wstring ssPathCoreTemp(32768, '\0');
                        GetEnvironmentVariable(L"SDV_FRAMEWORK_RUNTIME", ssPathCoreTemp.data(), static_cast<DWORD>(ssPathCoreTemp.size()));
                        ssPathCoreTemp.resize(wcsnlen(ssPathCoreTemp.c_str(), ssPathCoreTemp.size()));
                        if (!ssPathCoreTemp.empty())
                        {
                            pathCoreLib = std::filesystem::path(ssPathCoreTemp) / "core_services.sdv";
#else
                        std::string ssPathCoreTemp = std::getenv("SDV_FRAMEWORK_RUNTIME") ? std::getenv("SDV_FRAMEWORK_RUNTIME") : "";
                        if (!ssPathCoreTemp.empty())
                        {
                            pathCoreLib = std::filesystem::path(ssPathCoreTemp) / "core_services.sdv";
#endif
                            if (pathCoreLib.is_relative())
                                pathCoreLib = (GetExecDirectory() / pathCoreLib).lexically_normal();

                            if (!pathCoreLib.empty())
                                ssEnvironmentInfo = "System environment path: " + pathCoreLib.generic_string();
                        }
                    }

                    // Depend on system path to find the library
                    if (pathCoreLib.empty())
                        pathCoreLib = "core_services.sdv";

                    // Open the library only if there is no or a valid 'sdv_core_reloc.toml' file
                    if (!bRelocFileError)
                    {
#ifdef _WIN32
                        SetErrorMode(SEM_FAILCRITICALERRORS);
                        m_tModule = reinterpret_cast<core::TModuleID>(LoadLibraryW(pathCoreLib.native().c_str()));
#elif defined __unix__
                        m_tModule = reinterpret_cast<core::TModuleID>(dlopen(pathCoreLib.native().c_str(), RTLD_LAZY));
#else
#error OS is not supported!
#endif
                    }

                    if (!m_tModule)
                    {
                        std::string ssError;
#ifdef _WIN32
                        ssError.resize(1024);
                        ssError.resize(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
                            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &ssError.front(), 1024, NULL));
                        while (!ssError.empty() && std::isspace(ssError.back())) ssError.pop_back();
#elif defined __unix__
                        const char* szError = dlerror();
                        if (szError) ssError = szError;
#else
#error OS is not supported!
#endif
                        if (ssRelocFileInfo.empty() && ssEnvironmentInfo.empty())
                            std::cerr << "No environment variable set and no realocation file found." << std::endl;
                        if (!ssRelocFileInfo.empty())
                            std::cerr << ssRelocFileInfo << std::endl;
                        if (!ssEnvironmentInfo.empty())
                            std::cerr << ssEnvironmentInfo << std::endl;

                        std::cerr << "Could not load \"core_services.sdv\" library";
                        if (!ssError.empty()) std::cerr << ": " << ssError;
                        std::cerr << std::endl;
                        return;
                    }

                    // Get the SDVCore function pointer
                    using TFNSDVCore = IInterfaceAccess*();
#ifdef _WIN32
                    std::function<TFNSDVCore> fnSDVCore =
                        reinterpret_cast<TFNSDVCore*>(GetProcAddress(reinterpret_cast<HMODULE>(m_tModule), "SDVCore"));
#elif defined __unix__
                    std::function<TFNSDVCore> fnSDVCore =
                        reinterpret_cast<TFNSDVCore*>(dlsym(reinterpret_cast<void*>(m_tModule), "SDVCore"));
#else
#error OS is not supported!
#endif
                    if (!fnSDVCore)
                    {
                        std::cerr << "The library \"core_services.sdv\" doesn't expose the SDVCore function." << std::endl;
                        return;
                    }

                    // TODO: Add version check!

                    // Get the core interface
                    m_pCore = fnSDVCore();
                    if (!m_pCore)
                    {
                        std::cerr << "The library \"core_services.sdv\" doesn't provide a valid interface." << std::endl;
                        return;
                    }
                }

                /**
                 * @brief Return the core interface
                 */
                operator TInterfaceAccessPtr() const { return m_pCore; }

            private:
                /**
                 * @brief Get the directory of the executable.
                 * @return Path to the directory.
                 */
                static std::filesystem::path GetExecDirectory()
                {
#ifdef _WIN32
                    // Windows specific
                    std::wstring ssPath(32768, '\0');
                    GetModuleFileNameW(NULL, ssPath.data(), static_cast<DWORD>(ssPath.size() - 1));
#elif defined __linux__
                    // Linux specific
                    std::string ssPath(PATH_MAX + 1, '\0');
                    const ssize_t nCount = readlink("/proc/self/exe", ssPath.data(), PATH_MAX);
                    if (nCount < 0 || nCount >= PATH_MAX)
                        return {}; // some error
                    ssPath.at(nCount) = '\0';
#else
#error OS is not supported!
#endif
                    return std::filesystem::path{ssPath.c_str()}.parent_path() / ""; // To finish the folder path with (back)slash
                }

                /**
                 * @brief Get the folder path in the file 'sdv_core_reloc.toml'.
                 * @return Path content, empty string if not found.
                 */
                static std::string GetRelocationPath()
                {
                    if (std::filesystem::exists(GetExecDirectory() / "sdv_core_reloc.toml"))
                    {
                        std::ifstream fstream(GetExecDirectory() / "sdv_core_reloc.toml");
                        std::string ssLine;
                        while (std::getline(fstream, ssLine))
                        {
                            size_t nPos = 0;
                            auto fnSkipWhitespace = [&]() { while (std::isspace(ssLine[nPos])) nPos++; };
                            fnSkipWhitespace();
                            if (ssLine[nPos] == '#') continue;  // Rest of the line is comments
                            if (ssLine.substr(nPos, 9) != "directory") continue;    // not the keq of interest: skip line
                            nPos += 9;
                            fnSkipWhitespace();
                            if (ssLine[nPos] != '=')
                            {
                                std::cout << "Error in \"sdv_core_reloc.toml\": expecting assignment character '=' following"
                                    " keyword 'directory'." << std::endl;
                                break;
                            }
                            nPos++;
                            fnSkipWhitespace();
                            if (ssLine[nPos] != '\"')
                            {
                                std::cout << "Error in \"sdv_core_reloc.toml\": expecting double quote character '\"' indicating"
                                    " a string begin'." << std::endl;
                                break;
                            }
                            nPos++;
                            size_t nStart = nPos;
                            while (nPos < ssLine.length() && ssLine[nPos] != '\"')
                            {
                                // Check for escape character
                                if (ssLine[nPos] == '\\') nPos++;

                                // Skip character
                                nPos++;
                            }
                            if (nPos >= ssLine.length() || ssLine[nPos] != '\"')
                            {
                                std::cout << "Error in \"sdv_core_reloc.toml\": expecting double quote character '\"' indicating"
                                    " a string end'." << std::endl;
                                break;
                            }
                            std::string ssDirectory = ssLine.substr(nStart, nPos - nStart);
                            while (ssDirectory.empty())
                            {
                                std::cout << "Error in \"sdv_core_reloc.toml\": expecting a valid value following the assignment"
                                    " of the 'directory' key." << std::endl;
                                break;
                            }

                            return ssDirectory;
                        }
                    }
                    return "";
                }

                bool                m_bInit = false;        ///< Is the loader initialized?
                core::TModuleID     m_tModule = 0;          ///< Module ID
                IInterfaceAccess*   m_pCore = nullptr;      ///< Pointer to the core services.
            };
        } // namespace internal

#ifndef NO_SDV_CORE_FUNC
        /**
         * @brief Access to the core.
         * @return Smart pointer to the core services interface.
         */
        inline TInterfaceAccessPtr GetCore()
        {
            static internal::CSDVCoreLoader core;
            core.Load();
            return core;
        }

        /**
         * @brief Access to specific interface of the core.
         * @tparam TInterface Type of interface to return.
         * @return Pointer to the interface or NULL when the interface was not exposed.
         */
        template <typename TInterface>
        inline TInterface* GetCore()
        {
            return GetCore().GetInterface<TInterface>();
        }
#endif
    }
}


#endif // !define SDV_CORE_H
