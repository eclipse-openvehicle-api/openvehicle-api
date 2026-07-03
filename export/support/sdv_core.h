/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

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
#include <sstream>

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

#include "simple_toml.h"

namespace sdv
{
    /**
     * @brief Get the object type string from the type
     * @param[in] eType The object type to get the string from.
     * @return The object type string.
     */
    inline std::string ObjectType2String(sdv::EObjectType eType)
    {
        switch (eType)
        {
        case EObjectType::system_object:
            return "SystemObject";
        case EObjectType::device:
            return "Device";
        case EObjectType::platform_abstraction:
            return "PlatformAbstraction";
        case EObjectType::vehicle_bus:
            return "VehicleBus";
        case EObjectType::basic_service:
            return "BasicService";
        case EObjectType::sensor:
            return "Sensor";
        case EObjectType::actuator:
            return "Actuator";
        case EObjectType::complex_service:
            return "ComplexService";
        case EObjectType::vehicle_function:
            return "VehicleFunction";
        case EObjectType::application:
            return "Application";
        case EObjectType::proxy:
            return "Proxy";
        case EObjectType::stub:
            return "Stub";
        case EObjectType::utility:
            return "Utility";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Get the object type from the string
     * @param[in] rssType Reference to the string identifying the object type.
     * @return The object type or sdv::EObjectType::Undefined.
     */
    inline EObjectType String2ObjectType(const std::string& rssType)
    {
        if (rssType == "SystemObject")
            return EObjectType::system_object;
        if (rssType == "Device")
            return EObjectType::device;
        if (rssType == "PlatformAbstraction")
            return EObjectType::platform_abstraction;
        if (rssType == "VehicleBus")
            return EObjectType::vehicle_bus;
        if (rssType == "BasicService")
            return EObjectType::basic_service;
        if (rssType == "Sensor")
            return EObjectType::sensor;
        if (rssType == "Actuator")
            return EObjectType::actuator;
        if (rssType == "ComplexService")
            return EObjectType::complex_service;
        if (rssType == "VehicleFunction")
            return EObjectType::complex_service;
        if (rssType == "Application")
            return EObjectType::application;
        if (rssType == "Proxy")
            return EObjectType::proxy;
        if (rssType == "Stub")
            return EObjectType::stub;
        if (rssType == "Utility")
            return EObjectType::utility;
        return EObjectType::undefined;
    }
} // namespace sdv


#ifndef SDV_NO_LOADER
namespace sdv{

    namespace core
    {
        namespace internal
        {
            /**
             * @brief The SDV core loader
             * @details The SDV core loader class searches for the core library and does the first startup. The core library uses
             * the following environment variables to identify its location:
             * - SDV_FRAMEWORK_RUNTIME directs to the Vehicle API core location.
             * - SDV_COMPONENT_INSTALL directs to the location of the component installations.
             * - SDV_FRAMEWORK_DEV_TOOLS directs to the location of the development tools used during the build process of SDV
             *   components.
             * - SDV_FRAMEWORK_DEV_INCLUDE directs to the header file location to allow building components for use with the
             *   Vehicle API framework.
             * 
             * The SDV core loader supports different core stacks to coexist. For this there is a location procedure for the core
             * systems:
             * 1. Check for a "sdv_core_reloc.toml" file that contain one or more paths to the core system. The paths defined in the
             *    file override the global environment variables for this application and all child applications.
             * 2. Check for the environment variables. If the SDV_FRAMEWORK_RUNTIM variable is set, it is used to extract all other
             *    variables as well, if not set.
             * 3. If no variable is set, use the location of the EXE as core location. This might work in some situations if the
             *    path variable has been set properly.
             * 
             * If none of the above strategies work, the core cannot be loaded.
             * 
             * The "sdv_core_reloc.toml" file has the following format:
             * @code
             * [CoreLocation]
             * Version = 100                                    # Version; currently supported 100 for version 1.0
             * Runtime = "<core runtime directory>"
             * Install = "<component install directory>"        # Typically identical to runtime directory
             * DevTools = "<development tools directory>"      # Typically identical to runtime directory
             * Include = "<include directory>"                  # Typically in the include subdirectory
             * @endcode
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
                    std::string ssEnvironmentInfo = "";

                    // Step 1: check for the "sdv_core_reloc.toml"
                    ProcessRelocationFile();

                    // Step 2: check for environment variables (either set globally or overwritten by the sdv_core_reloc.toml).
#ifdef _WIN32
                    std::wstring ssPathCoreTemp(32768, '\0');
                    GetEnvironmentVariable(
                        L"SDV_FRAMEWORK_RUNTIME", ssPathCoreTemp.data(), static_cast<DWORD>(ssPathCoreTemp.size()));
                    ssPathCoreTemp.resize(wcsnlen(ssPathCoreTemp.c_str(), ssPathCoreTemp.size()));
                    if (!ssPathCoreTemp.empty())
                    {
                        m_pathCoreLib = std::filesystem::path(ssPathCoreTemp) / "core_services.sdv";
#else
                    std::string ssPathCoreTemp = std::getenv("SDV_FRAMEWORK_RUNTIME") ? std::getenv("SDV_FRAMEWORK_RUNTIME") : "";
                    if (!ssPathCoreTemp.empty())
                    {
                        m_pathCoreLib = std::filesystem::path(ssPathCoreTemp) / "core_services.sdv";
#endif
                        if (m_pathCoreLib.is_relative())
                            m_pathCoreLib = (GetExecDirectory() / m_pathCoreLib).lexically_normal();

                    }

                    // Step 3: check for the executable directory or otherwise check globally
                    if (m_pathCoreLib.empty())
                    {
                        if (std::filesystem::exists(GetExecDirectory() / "core_services.sdv"))
                            m_pathCoreLib = GetExecDirectory() / "core_services.sdv";
                        else
                            m_pathCoreLib = "core_services.sdv";
                    }
                    
                    ssEnvironmentInfo = "System environment path: " + m_pathCoreLib.generic_string();

#ifdef _WIN32
                    SetErrorMode(SEM_FAILCRITICALERRORS);
                    m_tModule = reinterpret_cast<core::TModuleID>(LoadLibraryW(m_pathCoreLib.native().c_str()));
#elif defined __unix__
                    m_tModule = reinterpret_cast<core::TModuleID>(dlopen(m_pathCoreLib.native().c_str(), RTLD_LAZY));
#else
#error OS is not supported!
#endif

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
                        if (!ssEnvironmentInfo.empty())
                            std::cerr << ssEnvironmentInfo << std::endl;
                        m_ssErrMsg = "Could not load \"core_services.sdv\" library";
                        if (!ssError.empty())
                            m_ssErrMsg += ": " + ssError;
                        std::cerr << m_ssErrMsg << std::endl;
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
                        m_ssErrMsg = "The library \"core_services.sdv\" doesn't expose the SDVCore function.";
                        std::cerr << m_ssErrMsg << std::endl;
                        return;
                    }

                    // TODO: Add version check!

                    // Get the core interface
                    m_pCore = fnSDVCore();
                    if (!m_pCore)
                    {
                        m_ssErrMsg = "The library \"core_services.sdv\" doesn't provide a valid interface.";
                        std::cerr << m_ssErrMsg << std::endl;
                        return;
                    }
                }

                /**
                 * @brief Return the core interface
                 */
                operator TInterfaceAccessPtr() const { return m_pCore; }

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
                 * @brief Get the path to the core library.
                 * @return Returns a reference to the path to the core library.
                */
                const std::filesystem::path& GetCoreLibPath() const
                {
                    return m_pathCoreLib;
                }

                /**
                 * @brief Has loaded successfully?
                 * @return Returns whether loading was successful.
                 */
                bool HasLoaded() const
                {
                    return m_bInit && m_tModule && m_pCore;
                }

                /**
                 * @brief Get the error message if loading was unsuccessful.
                 * @return Returns the a reference to the string containing the error message or an empty string if not available.
                 */
                const std::string& GetErrorMsg() const
                {
                    return m_ssErrMsg;
                }

            private:
                /**
                 * @brief Check for the 'sdv_core_reloc.toml' file containing the information about core relocation.
                 * @details Set the environment variables following the information from the core relocation file.
                 */
                static void ProcessRelocationFile()
                {
                    if (!std::filesystem::exists(GetExecDirectory() / "sdv_core_reloc.toml")) return;

                    std::ifstream fstream(GetExecDirectory() / "sdv_core_reloc.toml");
                    if (!fstream.is_open()) return;

                    std::stringstream sstreamContent;
                    sstreamContent << fstream.rdbuf();
                    fstream.close();

                    try
                    {
                        sdv::toml::simple_parser::CParser parser(sstreamContent.str());
                        if (parser.Root().GetDirect("CoreLocation.Version").GetValue<uint32_t>() != SDVFrameworkInterfaceVersion)
                        {
                            std::cerr << "Invalid version in sdv_core_reloc.toml" << std::endl;
                            return; // Version not supported
                        }

                        const auto& sRuntimeNode = parser.Root().GetDirect("CoreLocation.Runtime");
                        if (sRuntimeNode)
                        {
                            // Note: since the system might load multiple executables and the path is relative to this executable,
                            // but maybe not to another executable, create an absolute path before assigning the environment
                            // variable.
                            std::filesystem::path pathRuntime = sRuntimeNode.GetValue<std::string>();
                            if (pathRuntime.is_relative())
                                pathRuntime = (GetExecDirectory() / pathRuntime).lexically_normal();
#ifdef _WIN32
                            // NOTE: In windows there are two environment variables which need to be updated.
                            std::ignore = SetEnvironmentVariable(L"SDV_FRAMEWORK_RUNTIME", pathRuntime.native().c_str());
                            std::ignore = _wputenv((std::wstring(L"SDV_FRAMEWORK_RUNTIME=") + pathRuntime.native()).c_str());
#elif defined __unix__
                            std::ignore = setenv("SDV_FRAMEWORK_RUNTIME", pathRuntime.generic_u8string().c_str(), 1);
#else
#error The OS is not supported!
#endif
                        }

                        const auto& sInstallNode = parser.Root().GetDirect("CoreLocation.Install");
                        if (sInstallNode)
                        {
                            // Note: since the system might load multiple executables and the path is relative to this executable,
                            // but maybe not to another executable, create an absolute path before assigning the environment
                            // variable.
                            std::filesystem::path pathInstall = sInstallNode.GetValue<std::string>();
                            if (pathInstall.is_relative())
                                pathInstall = (GetExecDirectory() / pathInstall).lexically_normal();
#ifdef _WIN32
                            // NOTE: In windows there are two environment variables which need to be updated.
                            std::ignore = SetEnvironmentVariable(L"SDV_COMPONENT_INSTALL", pathInstall.native().c_str());
                            std::ignore = _wputenv((std::wstring(L"SDV_COMPONENT_INSTALL=") + pathInstall.native()).c_str());
#elif defined __unix__
                            std::ignore = setenv("SDV_COMPONENT_INSTALL", pathInstall.generic_u8string().c_str(), 1);
#else
#error The OS is not supported!
#endif
                        }

                        const auto& sDevToolsNode = parser.Root().GetDirect("CoreLocation.DevTools");
                        if (sDevToolsNode)
                        {
                            // Note: since the system might load multiple executables and the path is relative to this executable,
                            // but maybe not to another executable, create an absolute path before assigning the environment
                            // variable.
                            std::filesystem::path pathDevTools = sDevToolsNode.GetValue<std::string>();
                            if (pathDevTools.is_relative())
                                pathDevTools = (GetExecDirectory() / pathDevTools).lexically_normal();
#ifdef _WIN32
                            // NOTE: In windows there are two environment variables which need to be updated.
                            std::ignore = SetEnvironmentVariable(L"SDV_FRAMEWORK_DEV_TOOLS", pathDevTools.native().c_str());
                            std::ignore = _wputenv((std::wstring(L"SDV_FRAMEWORK_DEV_TOOLS=") + pathDevTools.native()).c_str());
#elif defined __unix__
                            std::ignore = setenv("SDV_FRAMEWORK_DEV_TOOLS", pathDevTools.generic_u8string().c_str(), 1);
#else
#error The OS is not supported!
#endif
                        }

                        const auto& sIncludeNode = parser.Root().GetDirect("CoreLocation.Include");
                        if (sIncludeNode)
                        {
                            // Note: since the system might load multiple executables and the path is relative to this executable,
                            // but maybe not to another executable, create an absolute path before assigning the environment
                            // variable.
                            std::filesystem::path pathInclude = sIncludeNode.GetValue<std::string>();
                            if (pathInclude.is_relative())
                                pathInclude = (GetExecDirectory() / pathInclude).lexically_normal();
#ifdef _WIN32
                            // NOTE: In windows there are two environment variables which need to be updated.
                            std::ignore = SetEnvironmentVariable(L"SDV_FRAMEWORK_DEV_INCLUDE", pathInclude.native().c_str());
                            std::ignore = _wputenv((std::wstring(L"SDV_FRAMEWORK_DEV_INCLUDE=") + pathInclude.native()).c_str());
#elif defined __unix__
                            std::ignore = setenv("SDV_FRAMEWORK_DEV_INCLUDE", pathInclude.generic_u8string().c_str(), 1);
#else
#error The OS is not supported!
#endif
                        }
                    }
                    catch (const std::exception&)
                    {}
                }

                bool                    m_bInit = false;        ///< Is the loader initialized?
                core::TModuleID         m_tModule = 0;          ///< Module ID
                IInterfaceAccess*       m_pCore = nullptr;      ///< Pointer to the core services.
                std::filesystem::path   m_pathCoreLib;          ///< Path to the core library.
                std::string             m_ssErrMsg;             ///< Error message for failed loading.
            };
        } // namespace internal

#ifndef NO_SDV_CORE_FUNC
        /**
         * @brief Access to the core loader.
         * @return Reference to the one core loader instance.
         */
        inline internal::CSDVCoreLoader& GetCoreLoader()
        {
            static internal::CSDVCoreLoader core;
            core.Load();
            return core;
        }

        /**
         * @brief Access to the core interfaces.
         * @return Smart pointer to the core services interfaces.
         */
        inline TInterfaceAccessPtr GetCore()
        {
            return GetCoreLoader();
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
#endif // !defined SDV_NO_LOADER

#endif // !define SDV_CORE_H
