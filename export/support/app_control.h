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
                        m_uiRetries = pAppContext->GetRetries();
                    }

                    // Automatically connect to the server
                    if (m_eContext == EAppContext::external)
                    {
                        // Try to connect
                        m_ptrServerRepository = sdv::com::ConnectToLocalServerRepository(m_uiInstanceID, m_uiRetries);
                        if (!m_ptrServerRepository)
                        {
                            if (!ConsoleIsSilent())
                                std::cerr << "ERROR: Failed to connect to the server repository." << std::endl;
                            Shutdown();
                            return false;
                        }

                        // Get access to the module control service
                        sdv::core::IObjectAccess* pObjectAccess = m_ptrServerRepository.GetInterface<sdv::core::IObjectAccess>();
                        const sdv::core::IRepositoryControl* pRepoControl = nullptr;
                        if (pObjectAccess)
                            pRepoControl = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("RepositoryService")).
                            GetInterface<sdv::core::IRepositoryControl>();
                        if (!pRepoControl)
                        {
                            if (!ConsoleIsSilent())
                                std::cerr << "ERROR: Failed to access the server repository." << std::endl;
                            return false;
                        }

                        // Link the local repository to the server repository.
                        sdv::core::ILinkCoreRepository* pLinkCoreRepo =
                            sdv::core::GetObject<sdv::core::ILinkCoreRepository>("RepositoryService");
                        if (!pLinkCoreRepo)
                        {
                            if (!ConsoleIsSilent())
                                std::cerr << "ERROR: Cannot link local and server repositories." << std::endl;
                            return false;
                        }
                        pLinkCoreRepo->LinkCoreRepository(m_ptrServerRepository);
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
                // Disconnect local and remote repositories.
                if (m_ptrServerRepository)
                {
                    // Link the local repository to the server repository.
                    sdv::core::ILinkCoreRepository* pLinkCoreRepo =
                        sdv::core::GetObject<sdv::core::ILinkCoreRepository>("RepositoryService");
                    if (!pLinkCoreRepo)
                    {
                        if (!ConsoleIsSilent())
                            std::cerr << "ERROR: Cannot unlink local and server repositories." << std::endl;
                    } else
                        pLinkCoreRepo->UnlinkCoreRepository();
                }

                // Disconnect from the server (if connected at all).
                m_ptrServerRepository.Clear();

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
             * @brief Get the SDV_FRAMEWORK_RUNTIME environment variable for this application.
             * @return Path directing to the SDV V-API Framework directory if available or an empty path if not.
             */
            static std::filesystem::path GetFrameworkRuntimeDirectory()
            {
#ifdef _WIN32
                const wchar_t* szFrameworkDir = _wgetenv(L"SDV_FRAMEWORK_RUNTIME");
                if (!szFrameworkDir) return {};
                return szFrameworkDir;
#elif defined __unix__
                const char* szFrameworkDir = getenv("SDV_FRAMEWORK_RUNTIME");
                if (!szFrameworkDir) return {};
                return szFrameworkDir;
#else
    #error The OS is not supported!
#endif
            }

            /**
             * @brief Set or overwrite the SDV_FRAMEWORK_RUNTIME environment variable for this application.
             * @param[in] rpathDir Reference of the path directing to the SDV V-API Framework directory.
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
             * @return Path directing to the SDV V-API component installation directory if available or an empty path if not.
             */
            static std::filesystem::path GetComponentInstallDirectory()
            {
#ifdef _WIN32
                const wchar_t* szComponentDir = _wgetenv(L"SDV_COMPONENT_INSTALL");
                if (!szComponentDir) return {};
                return szComponentDir;
#elif defined __unix__
                const char* szComponentDir = getenv("SDV_COMPONENT_INSTALL");
                if (!szComponentDir) return {};
                return szComponentDir;
#else
    #error The OS is not supported!
#endif
            }

            /**
             * @brief Set or overwrite the SDV_COMPONENT_INSTALL environment variable for this application.
             * @param[in] rpathDir Reference of the path directing to the SDV V-API component installation directory.
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
             * @return Returns 'true' on success; 'false' otherwise.
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
            uint32_t                m_uiRetries = 0u;                               ///< Number of retries to establish a connection.
            sdv::TObjectPtr         m_ptrServerRepository;                          ///< Server repository interface.
        };
    } // namespace app
} // namespace sdv
#endif // !defined SDV_APP_CONTROL_H
