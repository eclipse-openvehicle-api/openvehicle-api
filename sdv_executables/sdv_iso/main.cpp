#include <atomic>
#include <interfaces/ipc.h>
#include <support/sdv_core.h>
#include "../../global/cmdlnparser/cmdlnparser.cpp"
#include <support/app_control.h>
#include "../../global/base64.h"
#include "../../global/exec_dir_helper.h"
#include <support/toml.h>
#include <support/local_service_access.h>
#include "../error_msg.h"

/**
 * @brief Connect event callback wrapper. Calls shutdown request on disconnect or error.
 */
struct SConnectEventCallbackWrapper : sdv::IInterfaceAccess, sdv::ipc::IConnectEventCallback
{
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::ipc::IConnectEventCallback)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Set the current status. Overload of sdv::ipc::IConnectEventCallback::SetStatus.
     * @param[in] eConnectStatus The connection status.
     */
    virtual void SetStatus(/*in*/ sdv::ipc::EConnectStatus eConnectStatus) override
    {
        switch (eConnectStatus)
        {
        case sdv::ipc::EConnectStatus::disconnected:
        case sdv::ipc::EConnectStatus::disconnected_forced:
        case sdv::ipc::EConnectStatus::connection_error:
        case sdv::ipc::EConnectStatus::communication_error:
        {
            auto pRequestShutdown = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            if (!pRequestShutdown) break;
            pRequestShutdown->RequestShutdown();
            break;
        }
        default:
            break;
        }
    }
};


/**
 * @brief Main function of the isolation app executable.
 * @details The isolation app is initialized using arguments containing the several options and one compulsory configuration. The
 * configuration is a TOML string encoded as base64. It has the following structure:
 * @code
 * [Isolation]
 * Class = "object class to instantiate"        # services are instantiated, utilities are instantiated when needed.
 * Object = "object name of the instantiation"  # optional for services
 * [Isolation.Config]                           # Optional object configuration
 * [Isolation.Connection]                       # Section with connection information to connect to the core
 * @endcode
 * @param[in] iArgc Amount of arguments provided.
 * @param[in] rgszArgv Array of C-string pointers. The first entry is the name or path to this executable.
 * @return Returns the executable's exit code.
 */
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
    // NOTE EVE 27.05.2025: in release builds, starting and ending the thread right after each other causes incorrect behavior and
    // leads in some cases to create a deadlock in the join-function. The solution is to add delays in the thread processing.
    std::atomic_bool bThreadStarted = false;
    std::thread thread = std::thread([&]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            bThreadStarted = true;
        });
    while (!bThreadStarted) {std::this_thread::sleep_for(std::chrono::milliseconds(100));}
    if (thread.joinable())
        thread.join();

    // If not set, set the runtime location to the EXE directory.
    if (sdv::app::CAppControl::GetFrameworkRuntimeDirectory().empty())
        sdv::app::CAppControl::SetFrameworkRuntimeDirectory(GetExecDirectory());
    if (sdv::app::CAppControl::GetComponentInstallDirectory().empty())
        sdv::app::CAppControl::SetComponentInstallDirectory(GetExecDirectory());

    CCommandLine cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character));
    bool bHelp = false;
    bool bError = false;
    bool bSilent = false;
    bool bVerbose = false;
    bool bVersion = false;
    std::string ssConfigBase64;
    uint32_t uiInstanceID = 1000;   // Default instance is 1000
    std::filesystem::path pathInstallDir;
    std::string ssArgError;
    try
    {
        auto& rArgHelpDef = cmdln.DefineOption("?", bHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        cmdln.DefineSubOption("version", bVersion, "Show version information.");
        cmdln.DefineSubOption("instance", uiInstanceID, "The instance ID of the SDV instance (default ID is 1000).");
        cmdln.DefineSubOption("install_dir", pathInstallDir, "Installation directory (absolute or relative to this executable).");
        cmdln.DefineDefaultArgument(ssConfigBase64, "Isolation app configuration as base64.");
        cmdln.Parse(static_cast<size_t>(iArgc), rgszArgv);
    } catch (const SArgumentParseException& rsExcept)
    {
        ssArgError = rsExcept.what();
        bHelp = true;
        bError = true;
    }

    if (!bSilent)
    {
        std::cout << "SDV isolation application" << std::endl;
        std::cout << "Copyright (C): 2022-2025 ZF Friedrichshafen AG" << std::endl;
        std::cout << "Author: Erik Verhoeven" << std::endl << std::endl;
    }

    if (!ssArgError.empty() && !bSilent)
        std::cerr << "ERROR: " << ssArgError << std::endl;

    if (ssConfigBase64.empty())
    {
        ssArgError = "Missing app configuration.";
        bHelp = true;
    }

    if (bHelp)
    {
        if (!bSilent)
        {
            if (bError)
                std::cout << std::endl;
            cmdln.PrintHelp(std::cout, "The isolation application is used to isolate components from the core process. The "
                "isolation aims to improve the overall stability of the system.\n");
            return bError ? CMDLN_ARG_ERR : NO_ERROR;
        }
    }
    if (bError) return CMDLN_ARG_ERR;

    // Check for encoded configuration
    std::string ssIsoAppConfig = Base64DecodePlainText(ssConfigBase64);

    if (bVersion || bVerbose)
        std::cout << "Version: " << (SDVFrameworkBuildVersion / 100) << "." << (SDVFrameworkBuildVersion % 100) << " build " <<
        SDVFrameworkSubbuildVersion << " interface " << SDVFrameworkInterfaceVersion << std::endl;
    if (!bSilent)
        std::cout << "Instance ID: " << uiInstanceID << std::endl;

    // Create the startup config
    std::stringstream sstreamConfig;
    sstreamConfig << "[Application]" << std::endl;
    sstreamConfig << "Mode = \"Isolated\"" << std::endl;
    sstreamConfig << "Instance = " << uiInstanceID << std::endl;
    if (!pathInstallDir.empty())
    {
        if (pathInstallDir.is_relative())
            sstreamConfig << "InstallDir = \"" << pathInstallDir.generic_u8string() << "\"" << std::endl;
        else
            sstreamConfig << "InstallDir = \"" << pathInstallDir.lexically_relative(GetExecDirectory()).generic_u8string() <<
                "\"" << std::endl;
    }

    // Start the application control
    sdv::app::CAppControl appcontrol;
    if (!appcontrol.Startup(sstreamConfig.str()))
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_STARTUP_ERROR_MSG << std::endl;
        return APP_CONTROL_STARTUP_ERROR;
    }

    // Activate the configuration mode.
    appcontrol.SetConfigMode();

    // Check the configuration
    sdv::toml::CTOMLParser parser(ssIsoAppConfig);
    if (!parser.IsValid())
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_INVALID_ISOLATION_CONFIG_MSG << std::endl;
        return APP_CONTROL_INVALID_ISOLATION_CONFIG;
    }

    // Get the connection information
    sdv::toml::CNode nodeConnection = parser.GetDirect("Isolation.Connection");
    if (!nodeConnection)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_INVALID_ISOLATION_CONFIG_MSG << " Connection information is missing." <<
                std::endl;
        return APP_CONTROL_INVALID_ISOLATION_CONFIG;
    }

    // Connect to channel directly to be able to register event callback (needed to trigger app shutdown).
    std::string ssProvider = parser.GetDirect("Isolation.Connection.Provider.Name").GetValue();
    sdv::ipc::IChannelAccess* pChannelAccess = sdv::core::GetObject<sdv::ipc::IChannelAccess>(ssProvider);
    if (!pChannelAccess)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_INVALID_ISOLATION_CONFIG_MSG << " No channel control." << std::endl;
        return APP_CONTROL_INVALID_ISOLATION_CONFIG;
    }
    sdv::TObjectPtr ptrChannelEndpoint(pChannelAccess->Access(nodeConnection.GetTOML()));
    if (!ptrChannelEndpoint)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_INVALID_ISOLATION_CONFIG_MSG << " No channel endpoint." << std::endl;
        return APP_CONTROL_INVALID_ISOLATION_CONFIG;
    }
    sdv::ipc::IConnect* pConnect = ptrChannelEndpoint.GetInterface<sdv::ipc::IConnect>();
    if (!pConnect)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_INVALID_ISOLATION_CONFIG_MSG << " Missing connect interface on channel." <<
                std::endl;
        return APP_CONTROL_INVALID_ISOLATION_CONFIG;
    }
    SConnectEventCallbackWrapper sConnectEventWrapper;
    uint64_t uiCookie = pConnect->RegisterStatusEventCallback(&sConnectEventWrapper);

    // Connect to the core repository
    sdv::com::IConnectionControl* pConnectionControl = sdv::core::GetObject<sdv::com::IConnectionControl>("CommunicationControl");
    if (!pConnectionControl)
    {
        pConnect->UnregisterStatusEventCallback(uiCookie);
        if (!bSilent)
            std::cerr << "ERROR: " << COMMUNICATION_CONTROL_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return COMMUNICATION_CONTROL_SERVICE_ACCESS_ERROR;
    }
    sdv::IInterfaceAccess* pCoreRepo = nullptr;
    sdv::com::TConnectionID tConnection = pConnectionControl->AssignClientEndpoint(ptrChannelEndpoint, 3000, pCoreRepo);
    if (!tConnection.uiControl || !pCoreRepo)
    {
        pConnect->UnregisterStatusEventCallback(uiCookie);
        if (!bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }

    // Register the core repo as repo access
    sdv::core::ILinkCoreRepository* pLinkCoreRepo = sdv::core::GetObject<sdv::core::ILinkCoreRepository>("RepositoryService");
    if (!pLinkCoreRepo)
    {
        pConnect->UnregisterStatusEventCallback(uiCookie);
        if (!bSilent)
            std::cerr << "ERROR: " << LINK_REPO_SERVICE_ERROR_MSG << std::endl;
        return LINK_REPO_SERVICE_ERROR;
    }
    pLinkCoreRepo->LinkCoreRepository(pCoreRepo);

    // Create the one object... is this a service or a utility?
    std::string ssClassName = parser.GetDirect("Isolation.Class").GetValue();
    std::string ssObjectName = parser.GetDirect("Isolation.Object").GetValue();
    sdv::toml::CNode nodeConfig = parser.GetDirect("Isolation.Config");
    std::string ssConfig;
    if (nodeConfig) ssConfig = nodeConfig.GetTOML();
    const sdv::core::IRepositoryInfo* pRepositoryInfo = sdv::core::GetObject<sdv::core::IRepositoryInfo>("RepositoryService");
    sdv::core::IRepositoryControl* pRepositoryControl = sdv::core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
    sdv::core::IObjectAccess* pObjectAccess = sdv::core::GetObject<sdv::core::IObjectAccess>("RepositoryService");
    sdv::core::IRepositoryUtilityCreate* pCreateUtility = sdv::core::GetObject<sdv::core::IRepositoryUtilityCreate>("RepositoryService");
    if (!pRepositoryInfo || !pRepositoryControl || !pObjectAccess || !pCreateUtility)
    {
        pConnect->UnregisterStatusEventCallback(uiCookie);
        if (!bSilent)
            std::cerr << "ERROR: " << REPOSITORY_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return REPOSITORY_SERVICE_ACCESS_ERROR;
    }
    sdv::SClassInfo sClassInfo = pRepositoryInfo->FindClass(parser.GetDirect("Isolation.Class").GetValue());
    if (sClassInfo.ssClassName.empty())
    {
        if (!bSilent)
            std::cerr << "ERROR: " << CANNOT_FIND_OBJECT_MSG << " Class name: " << ssClassName << std::endl;
        return CANNOT_FIND_OBJECT;
    }

    sdv::IInterfaceAccess* pObject = nullptr;
    switch (sClassInfo.eType)
    {
    case sdv::EObjectType::ComplexService:
    {
        sdv::core::TObjectID tObjectID = pRepositoryControl->CreateObject(ssClassName, ssObjectName, ssConfig);
        pObject = pObjectAccess->GetObjectByID(tObjectID);
        break;
    }
    case sdv::EObjectType::Utility:
        pObject = pCreateUtility->CreateUtility(ssClassName, ssConfig);
        break;
    default:
        if (!bSilent)
            std::cerr << "ERROR: " << ISOLATION_INVALID_OBJECT_TYPE_MSG << " Class name: " << ssClassName << std::endl;
        return ISOLATION_INVALID_OBJECT_TYPE;
    }
    if (!pObject)
    {
        if (!bSilent)
            std::cerr << "ERROR: " << ISOLATION_FAILED_MSG << "Class name: " << ssClassName << std::endl;
        return ISOLATION_FAILED;
    }
    sdv::TObjectPtr ptrObject(pObject); // Manage lifetime

    // Register the object at the core
    sdv::core::IRegisterForeignObject* pRegisterForeignObject = pCoreRepo->GetInterface<sdv::core::IRegisterForeignObject>();
    if (!pRegisterForeignObject || !pRegisterForeignObject->RegisterObject(pObject, ssObjectName))
    {
        if (!bSilent)
            std::cerr << "ERROR: " << REGISTER_FOREIGN_OBJECT_FAILED_MSG << std::endl;
        return REGISTER_FOREIGN_OBJECT_FAILED;
    }

    // The lifetime of the object is now taken over by the core or if failed, the object will be destroyed.
    ptrObject.Clear();

    // Deactivate the configuration mode.
    appcontrol.SetRunningMode();

    // Start the running loop
    if (!appcontrol.RunLoop())
    {
        if (!bSilent)
            std::cerr << "ERROR: " << APP_CONTROL_START_LOOP_FAIELD_MSG << std::endl;
        return APP_CONTROL_START_LOOP_FAIELD;
    }

    // TODO EVE
    if (appcontrol.IsConfiguring())
        std::cout << "Iso app configuring... " << __LINE__ << std::endl;
    else
        std::cout << "Iso app running... " << __LINE__ << std::endl;

    if (bVerbose)
    {
        std::cout << "Shutdown request received..." << std::endl;
        std::cout << "Shutting down..." << std::endl;
    }

    // Shutdwown
    pConnect->UnregisterStatusEventCallback(uiCookie);
    parser.Clear();
    pLinkCoreRepo->UnlinkCoreRepository();
    appcontrol.Shutdown();

    if (!bSilent)
        std::cout << std::endl << "Done..." << std::endl;

    return NO_ERROR;
}