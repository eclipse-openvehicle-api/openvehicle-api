#ifndef LOCAL_SERVICE_ACCESS_H
#define LOCAL_SERVICE_ACCESS_H

#include "../interfaces/core.h"
#include "../interfaces/log.h"
#include "../interfaces/repository.h"
#include "../interfaces/com.h"
#include "interface_ptr.h"
#include <sstream>
#include <utility>

#ifdef __GNUC__
// Needed for getpid()
#include <unistd.h>
#endif

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    // Forward declaration
    interface IInterfaceAccess;

    /**
     * @brief Core features.
     */
    namespace core
    {
#ifndef SDV_CUSTOM_GETOBJECT
        /**
         * @brief Get an object from the repository service.
         * @attention This function only work when called from modules loaded via the repository service.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        inline TInterfaceAccessPtr GetObject(const std::string& rssObjectName)
        {
            if (!GetCore()) return nullptr;
            IObjectAccess* pRepository = GetCore<IObjectAccess>();
            if (!pRepository) return nullptr;
            return pRepository->GetObject(rssObjectName);
        }

        /**
         * @brief Get an object from the repository service.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        inline TInterfaceAccessPtr GetObject(TObjectID tObjectID)
        {
            if (!GetCore()) return nullptr;
            IObjectAccess* pRepository = GetCore<IObjectAccess>();
            if (!pRepository) return nullptr;
            return pRepository->GetObjectByID(tObjectID);
        }
#else
        /**
         * @brief Get an object from the repository service.
         * @attention This function only work when called from modules loaded via the repository service.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        TInterfaceAccessPtr GetObject(const std::string& rssObjectName);

        /**
         * @brief Get an object from the repository service.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        TInterfaceAccessPtr GetObject(TObjectID tObjectID);
#endif

        /**
         * @brief Get an object from the repository service.
         * @tparam TInterface The type of interface to return.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the interface pointer of the object instance if found or nullptr otherwise.
         */
        template <typename TInterface>
        inline TInterface* GetObject(const std::string& rssObjectName)
        {
            return GetObject(rssObjectName).GetInterface<TInterface>();
        }

        /**
         * @brief Get an object from the repository service.
         * @tparam TInterface The type of interface to return.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the interface pointer of the object instance if found or nullptr otherwise.
         */
        template <typename TInterface>
        inline TInterface* GetObject(TObjectID tObjectID)
        {
            return GetObject(tObjectID).GetInterface<TInterface>();
        }

        /**
        * @brief Log function enables logging for SDV.
        * @param[in] eSeverity Severity level of the log message which will be logged, e.g. Info, Warning, Error etc.
        * @param[in] rssSrcFile Name of the file from which the message is logged.
        * @param[in] uiSrcLine Line of the file from which the message is logged.
        * @param[in] rssMessage Reference to the log message to be logged.
        */
        inline void Log(ELogSeverity eSeverity, const u8string& rssSrcFile, uint32_t uiSrcLine, const u8string& rssMessage)
        {
            ILogger* pLogger = GetCore() ? GetCore<ILogger>() : nullptr;
#ifdef _WIN32
            if (pLogger) pLogger->Log(eSeverity, rssSrcFile, uiSrcLine, _getpid(), "", rssMessage);
#elif defined __unix__
            if (pLogger) pLogger->Log(eSeverity, rssSrcFile, uiSrcLine, getpid(), "", rssMessage);
#else
#error The OS is currently not supported!
#endif
        }

        /**
         * @brief Log a message with line and position.
         * @param severity The severity of the message. One of the entries from ELogServerity.
         */
#define SDV_LOG(severity, ...) sdv::core::internal::CSDVLogImpl(severity, __FILE__, __LINE__, __VA_ARGS__)

         /**
         * @brief Log a trace message with line and position.
         */
#define SDV_LOG_TRACE(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::trace, __FILE__, __LINE__, __VA_ARGS__)

         /**
          * @brief Log a debug message with line and position.
          */
#define SDV_LOG_DEBUG(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::debug, __FILE__, __LINE__, __VA_ARGS__)

          /**
           * @brief Log an information message with line and position.
           */
#define SDV_LOG_INFO(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::info, __FILE__, __LINE__, __VA_ARGS__)

           /**
            * @brief Log a warning message with line and position.
            */
#define SDV_LOG_WARNING(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::warning, __FILE__, __LINE__, __VA_ARGS__)

            /**
             * @brief Log an error message with line and position.
             */
#define SDV_LOG_ERROR(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::error, __FILE__, __LINE__, __VA_ARGS__)

             /**
              * @brief Log a fatal message with line and position.
              */
#define SDV_LOG_FATAL(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::fatal, __FILE__, __LINE__, __VA_ARGS__)

        namespace internal
        {
            /**
             * @brief Template to log messages with line and position.
             * @param[in] eSeverity Severity level of the message.
             * @param[in] szSrcFile Name of the file from which the message is logged. Specified by C++ standard.
             * @param[in] uiSrcLine Line of the file from which the message is logged. Specified by C++ standard.
             * @param[in] ...args identifier specified by C++ standard that uses the ellipsis notation in the parameters.
             */
            template <typename ...Args>
            inline void CSDVLogImpl(ELogSeverity eSeverity, const char* szSrcFile, uint32_t uiSrcLine, Args&& ...args)
            {
                std::ostringstream stream;
                (stream << ... << std::forward<Args>(args));

                Log(eSeverity, szSrcFile ? szSrcFile : "", uiSrcLine, stream.str().c_str());
            }
        }

        /**
         * @brief Create a utility
         * @param[in] rssUtilityName Reference to the utility name.
         * @param[in] rssUtilityConfig Optional reference to the utility configuration.
         * @return Smart pointer to the utility or NULL when the utility could not be found.
        */
        inline TObjectPtr CreateUtility(const std::string& rssUtilityName, const std::string& rssUtilityConfig = std::string())
        {
            TInterfaceAccessPtr ptrRepository = GetObject("RepositoryService");
            IRepositoryUtilityCreate* pUtilityCreate = ptrRepository.GetInterface<IRepositoryUtilityCreate>();
            if (!pUtilityCreate) return nullptr;
            return pUtilityCreate->CreateUtility(rssUtilityName, rssUtilityConfig);
        }
    } // namespace core

    namespace app
    {
        /**
         * @brief Get application attribute.
         * @param[in] rssAttribute Name of the attribute.
         * @return The attribute value or an empty any-value if the attribute wasn't found or didn't have a value.
         */
        inline any_t GetAppAttribute(const std::string& rssAttribute)
        {
            const IAttributes* ptrAppAttributes = core::GetObject<IAttributes>("AppControlService");
            if (!ptrAppAttributes) return {};
            return ptrAppAttributes->Get(rssAttribute);
        }

        /**
         * @brief Returns true if the current application is configured for silent console reporting.
         * @return Returns whether the silent console reporting has been enabled.
         */
        inline bool ConsoleIsSilent()
        {
            return GetAppAttribute("console.info_level") == "silent";
        }

        /**
         * @brief Returns true if the current application is configured for verbose console reporting.
         * @return Returns whether the verbose console reporting has been enabled.
         */
        inline bool ConsoleIsVerbose()
        {
            return GetAppAttribute("console.info_level") == "verbose";
        }

        /**
         * @brief Get the instance ID of the application.
         * @return The instance ID.
         */
        inline uint32_t GetAppInstanceID()
        {
            return GetAppAttribute("app.instance_id");
        }
    } // namespace app

    namespace com
    {
        /**
         * @brief Create a repository connection to a local server.
         * @param[in] uiInstanceID Optionally the instance ID of the target system to connect to or 0 (default) to connect to the
         * instance identified by app-control.
         * @param[in] nRetries Number of retries to connect (optional, default 30, minimum 3).
         * @return Returns a smart pointer to the repository proxy. Disconnection takes place when IObjectDestroy::DestroyObject is
         * called.
         */
        inline TObjectPtr ConnectToLocalServerRepository(uint32_t uiInstanceID = 0, size_t nRetries = 30)
        {
            com::IClientConnect* pClientConnect = core::GetObject<com::IClientConnect>("ConnectionService");
            if (!pClientConnect)
            {
                if (!app::ConsoleIsSilent())
                    std::cerr << "ERROR: Could not access the connection service." << std::endl;
                return {};
            }

            // Connect the client to the server and return the server repository interface.
            std::string ssConnectString = R"code([Client]
Type = "Local"
)code";
            if (uiInstanceID)
                ssConnectString += "Instance = " + std::to_string(uiInstanceID) + R"code(
)code";
            try
            {
                // Try to connect (30 times with 1 second in between).
                size_t nCnt = 0;
                sdv::TObjectPtr ptrRepository;
                while (!ptrRepository && nCnt < nRetries)
                {
                    nCnt++;
                    ptrRepository = pClientConnect->Connect(ssConnectString);
                    if (!ptrRepository)
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }

                // Return the result
                return ptrRepository;
            }
            catch (const XAccessDenied& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Access denied trying to connect to a local repository with server instance ID#" <<
                        (uiInstanceID?uiInstanceID : app::GetAppInstanceID()) << "." << std::endl;
                return {};
            }
            catch (const XNotFound& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Local repository with server instance ID#" <<
                        (uiInstanceID?uiInstanceID : app::GetAppInstanceID()) << " not found." << std::endl;
                return {};
            }
            catch (const XInvalidState& rExcept)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "The local repository with server instance ID#" <<
                        (uiInstanceID?uiInstanceID : app::GetAppInstanceID()) << " is in an invalid state: " << rExcept.what() <<
                        std::endl;
                return {};
            }
            catch (const XTimeout& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Timeout occurred trying to connect to a local repository with server instance ID#" <<
                        (uiInstanceID?uiInstanceID : app::GetAppInstanceID()) << "." << std::endl;
                return {};
            }
        }

    } // namespace core
} // namespace sdv

#endif // !defined LOCAL_SERVICE_ACCESS_H