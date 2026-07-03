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

#ifndef SDV_APP_CONTROL_H
#define SDV_APP_CONTROL_H

#include "sdv_core.h"
#include "../interfaces/app.h"
#include "../interfaces/config.h"
#include "interface_ptr.h"
#include "local_service_access.h"

namespace sdv
{
    namespace app
    {
        /**
         * @brief Application control class.
         */
        class CAppControl : public IInterfaceAccess, public IAppEvent
        {
        public:
            /**
             * @brief Default constructor; doesn't start the application control.
             */
            CAppControl() = default;

            /**
             * @brief Constructor; starts the application control with the supplied configuration.
             * @param[in] rssConfig Reference to the configuration string.
             */
            CAppControl(const std::string& rssConfig) : CAppControl()
            {
                Startup(rssConfig);
            }

            /**
             * @brief Shuts down during destruction if not explicitly done before.
             */
            ~CAppControl()
            {
                Shutdown();
            }

            // Interface map
            BEGIN_SDV_INTERFACE_MAP()
                SDV_INTERFACE_ENTRY(IAppEvent)
            END_SDV_INTERFACE_MAP()

            /**
             * @brief Starts the application control with the supplied configuration.
             * @param[in] rssConfig Reference to the configuration string.
             * @return Returns 'true' on success; otherwise returns 'false'.
             */
            bool Startup(const std::string& rssConfig)
            {
                IAppControl* pAppControl = core::GetCore() ? core::GetCore<IAppControl>() : nullptr;

                if (!pAppControl) return false;
                if (m_eState != EAppOperationState::not_started) return false;
                try
                {
                    // Start the application control
                    bool bRet = pAppControl->Startup(rssConfig, this);

                    // Get the application context
                    const IAppContext* pAppContext = core::GetCore<IAppContext>();
                    if (pAppContext)
                    {
                        m_eContext = pAppContext->GetContextType();
                        m_uiInstanceID = pAppContext->GetInstanceID();
                    }

                    return bRet;
                }
                catch (const XSysExcept& rxException)
                {
                    if (!ConsoleIsSilent())
                        std::cerr << "ERROR: " << rxException.what() << std::endl;
                    return false;
                }
            }

            /**
             * @brief Running loop until shutdown request is triggered.
             * @return Returns whether running the loop was successful.
             */
            bool RunLoop()
            {
                IAppControl* pAppControl = core::GetCore() ? core::GetCore<IAppControl>() : nullptr;
                if (!pAppControl) return false;

                try
                {
                    pAppControl->RunLoop();
                } catch (const XSysExcept&)
                {
                    return false;
                }
                return true;
            }

            /**
             * @brief Shutdown the application control
             * @attention The created objects are destroyed and the module unloaded in reverse order of their creation.
             */
            void Shutdown()
            {
                // Shutdown.
                IAppControl* pAppControl = core::GetCore() ? core::GetCore<IAppControl>() : nullptr;
                try
                {
                    if (pAppControl) pAppControl->Shutdown(false);
                } catch (const XSysExcept&)
                {
                    if (!ConsoleIsSilent())
                        std::cerr << "ERROR: Failed to shutdown app control." << std::endl;
                }
                m_eContext = EAppContext::no_context;
                m_uiInstanceID = 0u;
            }

            /**
             * @brief Is the system running?
             * @return Returns whether the system is running.
             */
            bool IsRunning() const
            {
                return m_eState == EAppOperationState::running;
            }

            /**
             * @brief Get the absolute path to the currently run executable
             * @return Absolute std::filesystem::path to the currently run executable
             */
            static std::filesystem::path GetAppDirectory()
            {
                static std::filesystem::path pathExeDir;
                if (!pathExeDir.empty())
                    return pathExeDir;
#ifdef _WIN32
                // Windows specific
                std::wstring ssPath(32768, '\0');
                GetModuleFileNameW(NULL, ssPath.data(), static_cast<DWORD>(ssPath.size() - 1));
#elif defined __linux__
                // Linux specific
                std::string ssPath(PATH_MAX + 1, '\0');
                const ssize_t nCount = readlink("/proc/self/exe", ssPath.data(), PATH_MAX);
                if (nCount < 0 || nCount >= PATH_MAX)
                    return pathExeDir; // some error
                ssPath.at(nCount) = '\0';
#else
    #error OS is not supported!
#endif
                pathExeDir = std::filesystem::path{ssPath.c_str()}.parent_path() / ""; // To finish the folder path with (back)slash
                return pathExeDir;
            }

            /**
             * @brief Get the filename of the currently run executable
             * @return The filename to the currently run executable
             */
            static std::filesystem::path GetAppFilename()
            {
                static std::filesystem::path pathExeFilename;
                if (!pathExeFilename.empty())
                    return pathExeFilename;
#ifdef _WIN32
                // Windows specific
                std::wstring ssPath(32768, '\0');
                GetModuleFileNameW(NULL, ssPath.data(), static_cast<DWORD>(ssPath.size() - 1));
#elif defined __linux__
                // Linux specific
                std::string ssPath(PATH_MAX + 1, '\0');
                const ssize_t nCount = readlink("/proc/self/exe", ssPath.data(), PATH_MAX);
                if (nCount < 0 || nCount >= PATH_MAX)
                    return pathExeFilename; // some error
                ssPath.at(nCount)          = '\0';
#else
    #error OS is not supported!
#endif
                pathExeFilename = std::filesystem::path{ssPath.c_str()}.filename();
                return pathExeFilename;
            }


            /**
             * @brief Get the SDV_FRAMEWORK_RUNTIME environment variable for this application.
             * @remarks If the environment variable is empty or if the variable contains a relative path, the path will be enhanced
             * with the application path.
             * @return Absolute path directing to the SDV Vehicle API Framework directory.
             */
            static std::filesystem::path GetFrameworkRuntimeDirectory()
            {
                std::filesystem::path path;
#ifdef _WIN32
                const wchar_t* szFrameworkDir = _wgetenv(L"SDV_FRAMEWORK_RUNTIME");
                if (szFrameworkDir) path = szFrameworkDir;
#elif defined __unix__
                const char* szFrameworkDir = getenv("SDV_FRAMEWORK_RUNTIME");
                if (szFrameworkDir) path = szFrameworkDir;
#else
    #error The OS is not supported!
#endif
                if (path.empty() || path.is_relative()) path = GetAppDirectory() / path;
                return path.lexically_normal();
            }

            /**
             * @brief Set or overwrite the SDV_FRAMEWORK_RUNTIME environment variable for this application.
             * @param[in] rpathDir Reference of the path directing to the SDV Vehicle API Framework directory.
             */
            static void SetFrameworkRuntimeDirectory(const std::filesystem::path& rpathDir)
            {
#ifdef _WIN32
                // NOTE: In windows there are two environment variable stacks which need to be updated.
                std::ignore = SetEnvironmentVariable(L"SDV_FRAMEWORK_RUNTIME", rpathDir.native().c_str());
                std::ignore = _wputenv((std::wstring(L"SDV_FRAMEWORK_RUNTIME=") + rpathDir.native()).c_str());
#elif defined __unix__
                std::ignore = setenv("SDV_FRAMEWORK_RUNTIME", rpathDir.generic_u8string().c_str(), 1);
#else
    #error The OS is not supported!
#endif
            }

            /**
             * @brief Get the SDV_COMPONENT_INSTALL environment variable for this application.
             * @remarks If the environment variable is empty, the runtime directory is taken. If the variable contains a relative
             * path, the path will be enhanced with the application path.
             * @return Absolute path directing to the SDV Vehicle API component installation directory.
             */
            static std::filesystem::path GetComponentInstallDirectory()
            {
                std::filesystem::path path;
#ifdef _WIN32
                const wchar_t* szComponentDir = _wgetenv(L"SDV_COMPONENT_INSTALL");
                if (szComponentDir) path = szComponentDir;
#elif defined __unix__
                const char* szComponentDir = getenv("SDV_COMPONENT_INSTALL");
                if (szComponentDir) path = szComponentDir;
#else
    #error The OS is not supported!
#endif
                if (path.empty()) return GetFrameworkRuntimeDirectory();
                if (path.is_relative()) path = GetAppDirectory() / path;
                return path.lexically_normal();
            }

            /**
             * @brief Set or overwrite the SDV_COMPONENT_INSTALL environment variable for this application.
             * @param[in] rpathDir Reference of the path directing to the SDV Vehicle API component installation directory.
             */
            static void SetComponentInstallDirectory(const std::filesystem::path& rpathDir)
            {
#ifdef _WIN32
                // NOTE: In windows there are two environment variable stacks which need to be updated.
                std::ignore = SetEnvironmentVariable(L"SDV_COMPONENT_INSTALL", rpathDir.native().c_str());
                std::ignore = _wputenv((std::wstring(L"SDV_COMPONENT_INSTALL=") + rpathDir.native()).c_str());
#elif defined __unix__
                std::ignore = setenv("SDV_COMPONENT_INSTALL", rpathDir.generic_u8string().c_str(), 1);
#else
#error The OS is not supported!
#endif
            }

            /**
             * @brief Get the application context.
             * @return The application context.
             */
            EAppContext GetAppContext() const
            {
                return m_eContext;
            }

            /**
             * @brief Get the core instance ID.
             * @return The core instance ID.
             */
            uint32_t GetInstanceID() const
            {
                return m_uiInstanceID;
            }

            /**
             * @brief Set the operation to running mode.
             * @pre The system is operating in configuration mode.
             */
            void SetRunningMode()
            {
                IAppOperation* pAppOperation = core::GetObject<IAppOperation>("AppControlService");
                if (pAppOperation)
                    pAppOperation->SetRunningMode();
            }

            /**
             * @brief Is the system in configuration state?
             * @return Returns whether the system is being configured.
             */
            bool IsConfiguring() const
            {
                return m_eState == EAppOperationState::configuring;
            }

            /**
             * @brief Set the operation to config mode.
             * @pre The system is operating in running mode.
             */
            void SetConfigMode()
            {
                IAppOperation* pAppOperation = core::GetObject<IAppOperation>("AppControlService");
                if (pAppOperation)
                    pAppOperation->SetConfigMode();
            }

            /**
             * @brief Process the provided configuration by loading modules and creating objects/stubs/proxies defined in the
             * configuration string.
             * @attention Configuration changes can only occur when the system is in configuration mode.
             * @param[in] ssContent The contents of the configuration file (TOML).
             * @return Returns 'true' on success; 'false' otherwise.
             */
            core::EConfigProcessResult ProcessConfig(/*in*/ const sdv::u8string& ssContent)
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return core::EConfigProcessResult::failed;

                bool bRunning = IsRunning();
                SetConfigMode();
                core::EConfigProcessResult eResult = pAppConfig->ProcessConfig(ssContent);
                if (bRunning) SetRunningMode();
                return eResult;
            }

            /**
             * @brief Read file pointed to by the provided file path and load modules and create objects/stubs/proxies as defined
             * in the configuration file.
             * @attention Configuration changes can only occur when the system is in configuration mode.
             * @param[in] ssFilename Path to the file containing the configuration (TOML). The path can be absolute as well as relative.
             * In case a relative path is provided, the current directory is searched as well as all directories supplied through
             * the AddConfigSearchDir function.
             * @return Returns a config process result enum value.
             */
            core::EConfigProcessResult LoadConfig(/*in*/ const sdv::u8string& ssFilename)
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return core::EConfigProcessResult::failed;

                bool bRunning = IsRunning();
                SetConfigMode();
                core::EConfigProcessResult eResult = pAppConfig->LoadConfig(ssFilename);
                if (bRunning) SetRunningMode();
                return eResult;
            }

            /**
             * @brief Save a configuration file pointed to by the provided file path. All components are saved that were added after
             * the last baseline with the configuration specific settings.
             * @remarks The function will only save when the configuration has changed.
             * @param[in] ssConfigPath Path to the file containing the configuration (TOML). The path can be an absolute as well as
             * a relative path. In case a relative path is provided, the configuration is stored relative to the executable
             * directory.
             * @return Returns 'true' on success (or no changes detected); 'false' otherwise.
             */
            bool SaveConfig(/*in*/ sdv::u8string ssConfigPath) const
            {
                const core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return false;
                return pAppConfig->SaveConfig(ssConfigPath);
            }

            /**
             * @brief Generate the configuration TOML string.
             * @return The generated configuration string.
             */
            sdv::u8string GenerateConfigString() const
            {
                const core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return {};
                return pAppConfig->GenerateConfigString();
            }

            /**
             * @brief Close the current configuration.
             * @details This will close und unload the components and modules from the current configuration as well as dependent
             * components that builds on top of the components being closed. Components that the current configuration depends on
             * are not closed.
             */
            void CloseConfig()
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return;
                pAppConfig->CloseConfig();
            }

            /**
             * @brief Add a search path to a folder where a config file can be found.
             * @param[in] rpathDir Reference to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddConfigSearchDir(/*in*/ const std::filesystem::path& rpathDir)
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return false;
                return pAppConfig->AddConfigSearchDir(rpathDir.generic_u8string());
            }

            /**
             * @brief Add a search path to a folder where a config file can be found.
             * @param[in] ssDir Relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddConfigSearchDir(/*in*/ const sdv::u8string& ssDir)
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return false;
                return pAppConfig->AddConfigSearchDir(ssDir);
            }

            /**
             * @brief Add a search path to a folder where a config file can be found.
             * @param[in] ssDir Relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddConfigSearchDir(/*in*/ const std::string& ssDir)
            {
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return false;
                return pAppConfig->AddConfigSearchDir(ssDir);
            }

            /**
             * @brief Add a search path to a folder where a config file can be found.
             * @param[in] szDir Zero terminated string to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddConfigSearchDir(/*in*/ const char* szDir)
            {
                if (!szDir) return false;
                core::IConfig* pAppConfig = nullptr;
                sdv::TInterfaceAccessPtr ptrConfigObj = core::GetObject("ConfigService");
                if (ptrConfigObj) pAppConfig = ptrConfigObj.GetInterface<core::IConfig>();
                if (!pAppConfig) return false;
                return pAppConfig->AddConfigSearchDir(szDir);
            }

            /**
             * @brief Add a search path to a folder where a module can be found.
             * @param[in] rpathDir Reference to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddModuleSearchDir(/*in*/ const std::filesystem::path& rpathDir)
            {
                sdv::core::IModuleControlConfig* pModuleConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
                if (!pModuleConfig) return false;
                return pModuleConfig->AddModuleSearchDir(rpathDir.generic_u8string());
            }

            /**
             * @brief Add a search path to a folder where a module can be found.
             * @param[in] rssDir Reference to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddModuleSearchDir(/*in*/ const sdv::u8string& rssDir)
            {
                sdv::core::IModuleControlConfig* pModuleConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
                if (!pModuleConfig) return false;
                return pModuleConfig->AddModuleSearchDir(rssDir);
            }

            /**
             * @brief Add a search path to a folder where a module can be found.
             * @param[in] rssDir Reference to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddModuleSearchDir(/*in*/ const std::string& rssDir)
            {
                sdv::core::IModuleControlConfig* pModuleConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
                if (!pModuleConfig) return false;
                return pModuleConfig->AddModuleSearchDir(rssDir);
            }

            /**
             * @brief Add a search path to a folder where a module can be found.
             * @param[in] szDir Zero terminated string to the relative or absolute path to an existing folder.
             * @return Returns 'true' on success; 'false' otherwise.
             */
            bool AddModuleSearchDir(/*in*/ const char* szDir)
            {
                if (!szDir) return false;
                sdv::core::IModuleControlConfig* pModuleConfig = sdv::core::GetCore<sdv::core::IModuleControlConfig>();
                if (!pModuleConfig) return false;
                return pModuleConfig->AddModuleSearchDir(szDir);
            }

        private:
            /**
             * @brief Process the event. Overload of IAppEvent::ProcessEvent.
             * @param[inout] sEvent Event structure containing the information to process.
             */
            virtual void ProcessEvent(/*inout*/ SAppEvent& sEvent) override
            {
                // Only process state change events
                if (sEvent.uiEventID != EVENT_OPERATION_STATE_CHANGED) return;

                m_eState = static_cast<EAppOperationState>(sEvent.uiInfo);
            }

            EAppOperationState      m_eState = EAppOperationState::not_started;     ///< Application state.
            EAppContext             m_eContext = EAppContext::no_context;           ///< Application context.
            uint32_t                m_uiInstanceID = 0u;                            ///< Core instance.
            uint32_t                m_uiConnectRetries = 0u;                        ///< Number of retries to establish a connection.
        };
    } // namespace app
} // namespace sdv
#endif // !defined SDV_APP_CONTROL_H
