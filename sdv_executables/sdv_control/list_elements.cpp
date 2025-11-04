#include "list_elements.h"
#include "print_table.h"
#include <interfaces/config.h>
#include <support/sdv_core.h>
#include <support/local_service_access.h>
#include "../../global/cmdlnparser/cmdlnparser.h"
#include "../../global/exec_dir_helper.h"
#include "../../global/flags.h"
#include "../error_msg.h"

void ListHelp(const SContext& rsContext)
{
    // First argument should be "LIST"
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing listing command..." << std::endl;
        return;
    }
    if (!iequals(rsContext.seqCmdLine[0], "LIST"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: invalid command: " << rsContext.seqCmdLine[0] << std::endl;
        return;
    }

    // Which list command help is requested
    enum class EListCommand { unknown, modules, classes, components, installations, connections } eListCommand = EListCommand::unknown;
    if (rsContext.seqCmdLine.size() >= 2)
    {
        if (iequals(rsContext.seqCmdLine[1], "MODULES")) eListCommand = EListCommand::modules;
        else if (iequals(rsContext.seqCmdLine[1], "CLASSES")) eListCommand = EListCommand::classes;
        else if (iequals(rsContext.seqCmdLine[1], "COMPONENTS")) eListCommand = EListCommand::components;
        else if (iequals(rsContext.seqCmdLine[1], "INSTALLATIONS")) eListCommand = EListCommand::installations;
        else if (iequals(rsContext.seqCmdLine[1], "CONNECTIONS")) eListCommand = EListCommand::connections;
    }

    // Print help if requested
    if (!rsContext.bSilent)
    {
        switch (eListCommand)
        {
        case EListCommand::modules:
            CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control LIST MODULES [options...]\n\nShow a list of all loaded modules.\n");
            break;
        case EListCommand::classes:
            CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control LIST CLASSES [options...]\n\nShow a list of available component classes.\n");
            break;
        case EListCommand::components:
            CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control LIST COMPONENTS [options...]\n\nShow a list of all loaded components.\n");
            break;
        case EListCommand::installations:
            CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control LIST INSTALLATIONS [options...]\n\nShow a list of all installations.\n");
            break;
        case EListCommand::connections:
            CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control LIST CONNECTIONS [options...]\n\nShow a list of all connected applications.\n");
            break;
        default:
            CCommandLine::PrintHelpText(std::cout, R"code(Usage: sdv_control LIST <list_command> [options...]

Supported listing commands:
    LIST MODULES        Show a list of loaded server modules.
    LIST CLASSES        Show a list of available component classes.
    LIST COMPONENTS     Show a list of instantiated server components
    LIST INSTALLATIONS  Show a list of installations.
    LIST CONNECTIONS    Show a list of current connections.

)code");
            break;
        }
        std::cout << "Options:\n";
        std::cout << " --no_header  Do not print a header for the listing table.\n";
        std::cout << " --short      Print only the most essential information as one column.\n\n";
    }
}

int ListElements(const SContext& rsContext, std::ostream& rstream /*= std::cout*/)
{
    // First argument should be "LIST"
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing listing command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "LIST"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command: " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Which list command help is requested
    enum class EListCommand { unknown, modules, classes, components, installations, connections } eListCommand = EListCommand::unknown;
    if (rsContext.seqCmdLine.size() >= 2)
    {
        if (iequals(rsContext.seqCmdLine[1], "MODULES")) eListCommand = EListCommand::modules;
        else if (iequals(rsContext.seqCmdLine[1], "CLASSES")) eListCommand = EListCommand::classes;
        else if (iequals(rsContext.seqCmdLine[1], "COMPONENTS")) eListCommand = EListCommand::components;
        else if (iequals(rsContext.seqCmdLine[1], "INSTALLATIONS")) eListCommand = EListCommand::installations;
        else if (iequals(rsContext.seqCmdLine[1], "CONNECTIONS")) eListCommand = EListCommand::connections;
    }
    else
    {
        if (!rsContext.bSilent)
        {
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Missing listing command.." << std::endl << std::endl;
            ListHelp(rsContext);
        }
        return CMDLN_ARG_ERR;
    }

    // Try to connect
    sdv::TObjectPtr ptrRepository = sdv::com::ConnectToLocalServerRepository(rsContext.uiInstanceID);
    if (!ptrRepository)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << " Instance #" << rsContext.uiInstanceID << "." << std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }

    int iRet = -100;
    switch (eListCommand)
    {
    case EListCommand::modules:
        iRet = ListModules(rsContext, ptrRepository, rstream);
        break;
    case EListCommand::classes:
        iRet = ListClasses(rsContext, ptrRepository, rstream);
        break;
    case EListCommand::components:
        iRet = ListComponents(rsContext, ptrRepository, rstream);
        break;
    case EListCommand::installations:
        iRet = ListInstallations(rsContext, ptrRepository, rstream);
        break;
    case EListCommand::connections:
        iRet = ListConnections(rsContext, ptrRepository, rstream);
        break;
    default:
        break;
    }
    return iRet;
}

int ListModules(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream)
{
    // Get access to the module control service
    sdv::core::IObjectAccess* pObjectAccess = rptrRepository.GetInterface<sdv::core::IObjectAccess>();
    const sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
        pModuleInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService")).
            GetInterface<sdv::core::IModuleInfo>();
    if (!pModuleInfo)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << MODULE_CONTROL_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return MODULE_CONTROL_SERVICE_ACCESS_ERROR;
    }

    // Relative path from exe
    auto fnPrintRelPath = [](const sdv::u8string& rssPath)
    {
        std::filesystem::path path = rssPath.c_str();
        if (path.is_relative()) return path.generic_u8string();
        return path.lexically_relative(GetExecDirectory()).generic_u8string();
    };

    // Create the module list
    sdv::sequence<sdv::core::SModuleInfo> seqModules = pModuleInfo->GetModuleList();
    if (rsContext.bListShort)
    {
        std::vector<std::array<std::string, 1>> vecShortModuleList;
        vecShortModuleList.push_back({ "Path" });
        for (const sdv::core::SModuleInfo& rsModuleInfo : seqModules)
            vecShortModuleList.push_back({ fnPrintRelPath(rsModuleInfo.ssPath) });

        // Print the module list
        PrintTable(vecShortModuleList, rstream, rsContext.bListNoHdr);
    }
    else
    {
        std::vector<std::array<std::string, 5>> vecModuleList;
        vecModuleList.push_back({ "Module ID", "Filename", "Version", "Active", "Path" });
        for (const sdv::core::SModuleInfo& rsModuleInfo : seqModules)
            vecModuleList.push_back({
                std::to_string(static_cast<int64_t>(rsModuleInfo.tModuleID)),
                rsModuleInfo.ssFilename,
                std::to_string(rsModuleInfo.uiVersion / 100) + "." + std::to_string(rsModuleInfo.uiVersion % 100),
                (rsModuleInfo.bActive ? " +" : " -"),
                fnPrintRelPath(rsModuleInfo.ssPath) });

        // Print the module list
        PrintTable(vecModuleList, rstream, rsContext.bListNoHdr);
    }
    return NO_ERROR;
}

int ListClasses(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream)
{
    // Get access to the module control service
    sdv::core::IObjectAccess* pObjectAccess = rptrRepository.GetInterface<sdv::core::IObjectAccess>();
    const sdv::core::IModuleInfo* pModuleInfo = nullptr;
    if (pObjectAccess)
        pModuleInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ModuleControlService")).
        GetInterface<sdv::core::IModuleInfo>();
    if (!pModuleInfo)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << MODULE_CONTROL_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return MODULE_CONTROL_SERVICE_ACCESS_ERROR;
    }

    // Class type
    auto fnPrintClassType = [](sdv::EObjectType eType)
    {
        switch (eType)
        {
        case sdv::EObjectType::SystemObject:    return "SystemObject";
        case sdv::EObjectType::Device:          return "Device";
        case sdv::EObjectType::BasicService:    return "BasicService";
        case sdv::EObjectType::ComplexService:  return "ComplexService";
        case sdv::EObjectType::Application:     return "Application";
        case sdv::EObjectType::Proxy:           return "Proxy";
        case sdv::EObjectType::Stub:            return "Stub";
        case sdv::EObjectType::Utility:         return "Utility";
        default:                                return "Unknown";
        }
    };

    // Class flags
    auto fnPrintClassFlags = [](uint32_t uiFlags)
    {
        hlpr::flags<sdv::EObjectFlags> flags(uiFlags);
        std::stringstream sstream;
        if (flags.check(sdv::EObjectFlags::singleton))
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << "Singleton";
        }
        return std::string("{") + sstream.str() + std::string("}");
    };

    // Dependencies
    auto fnPrintList = [](const sdv::sequence<sdv::u8string>& rseqList)
    {
        std::stringstream sstream;
        for (const sdv::u8string& rss : rseqList)
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << rss;
        }
        return std::string("{") + sstream.str() + std::string("}");
    };

    // Create the module list
    if (rsContext.bListShort)
    {
        std::vector<std::array<std::string, 2>> vecShortClassList;
        vecShortClassList.push_back({ "Class name", "Aliases"});
        sdv::sequence<sdv::core::SModuleInfo> seqModules = pModuleInfo->GetModuleList();
        for (const sdv::core::SModuleInfo& rsModuleInfo : seqModules)
        {
            sdv::sequence<sdv::SClassInfo> seqClasses = pModuleInfo->GetClassList(rsModuleInfo.tModuleID);
            for (const sdv::SClassInfo& rsClassInfo : seqClasses)
                vecShortClassList.push_back({ rsClassInfo.ssClassName, fnPrintList(rsClassInfo.seqClassAliases) });
        }

        // Print the module list
        PrintTable(vecShortClassList, rstream, rsContext.bListNoHdr);
    }
    else
    {
        std::vector<std::array<std::string, 7>> vecClassList;
        vecClassList.push_back({ "Class name", "Aliases", "Default name", "Type", "Flags", "Module ID", "Dependencies" });
        sdv::sequence<sdv::core::SModuleInfo> seqModules = pModuleInfo->GetModuleList();
        for (const sdv::core::SModuleInfo& rsModuleInfo : seqModules)
        {
            sdv::sequence<sdv::SClassInfo> seqClasses = pModuleInfo->GetClassList(rsModuleInfo.tModuleID);
            for (const sdv::SClassInfo& rsClassInfo : seqClasses)
                vecClassList.push_back({
                    rsClassInfo.ssClassName,
                    fnPrintList(rsClassInfo.seqClassAliases),
                    rsClassInfo.ssDefaultObjectName,
                    fnPrintClassType(rsClassInfo.eType),
                    fnPrintClassFlags(rsClassInfo.uiFlags),
                    std::to_string(static_cast<int64_t>(rsModuleInfo.tModuleID)),
                    fnPrintList(rsClassInfo.seqDependencies) });
        }

        // Print the module list
        PrintTable(vecClassList, rstream, rsContext.bListNoHdr);
    }
    return NO_ERROR;
}

int ListComponents(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream)
{
    // Get access to the module control service
    sdv::core::IObjectAccess* pObjectAccess = rptrRepository.GetInterface<sdv::core::IObjectAccess>();
    const sdv::core::IRepositoryInfo* pRepoInfo = nullptr;
    if (pObjectAccess)
        pRepoInfo = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("RepositoryService")).
        GetInterface<sdv::core::IRepositoryInfo>();
    if (!pRepoInfo)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << REPOSITORY_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return REPOSITORY_SERVICE_ACCESS_ERROR;
    }

    // Class type
    auto fnPrintClassType = [](sdv::EObjectType eType)
    {
        switch (eType)
        {
        case sdv::EObjectType::SystemObject:    return "SystemObject";
        case sdv::EObjectType::Device:          return "Device";
        case sdv::EObjectType::BasicService:    return "BasicService";
        case sdv::EObjectType::ComplexService:  return "ComplexService";
        case sdv::EObjectType::Application:     return "Application";
        case sdv::EObjectType::Proxy:           return "Proxy";
        case sdv::EObjectType::Stub:            return "Stub";
        case sdv::EObjectType::Utility:         return "Utility";
        default:                                return "Unknown";
        }
    };

    // Object flags
    auto fnPrintObjectFlags = [](uint32_t uiFlags)
    {
        hlpr::flags<sdv::core::EObjectInfoFlags> flags(uiFlags);
        std::stringstream sstream;
        if (flags.check(sdv::core::EObjectInfoFlags::object_controlled))
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << "Controlled";
        }
        if (flags.check(sdv::core::EObjectInfoFlags::object_foreign))
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << "Foreign";
        }
        if (flags.check(sdv::core::EObjectInfoFlags::object_isolated))
        {
            if (!sstream.str().empty()) sstream << ", ";
            sstream << "Isolated";
        }
        return std::string("{") + sstream.str() + std::string("}");
    };

    // Create the object list
    if (rsContext.bListShort)
    {
        std::vector<std::array<std::string, 6>> vecShortObjectList;
        vecShortObjectList.push_back({ "Object name" });
        sdv::sequence<sdv::core::SObjectInfo> seqObjects = pRepoInfo->GetObjectList();
        for (const sdv::core::SObjectInfo& rsObjectInfo : seqObjects)
            vecShortObjectList.push_back({ rsObjectInfo.ssObjectName });

        // Print the module list
        PrintTable(vecShortObjectList, rstream, rsContext.bListNoHdr);
    }
    else
    {
        std::vector<std::array<std::string, 6>> vecObjectList;
        vecObjectList.push_back({ "Object ID", "Class name", "Object name", "Type", "Flags", "Module ID" });
        sdv::sequence<sdv::core::SObjectInfo> seqObjects = pRepoInfo->GetObjectList();
        for (const sdv::core::SObjectInfo& rsObjectInfo : seqObjects)
            vecObjectList.push_back({
                std::to_string(static_cast<int64_t>(rsObjectInfo.tModuleID)),
                rsObjectInfo.sClassInfo.ssClassName,
                rsObjectInfo.ssObjectName,
                fnPrintClassType(rsObjectInfo.sClassInfo.eType),
                fnPrintObjectFlags(rsObjectInfo.uiFlags),
                std::to_string(static_cast<int64_t>(rsObjectInfo.tModuleID)) });

        // Print the module list
        PrintTable(vecObjectList, rstream, rsContext.bListNoHdr);
    }
    return NO_ERROR;
}

int ListInstallations(const SContext& rsContext, const sdv::TObjectPtr& rptrRepository, std::ostream& rstream)
{
    // Get access to the module control service
    sdv::core::IObjectAccess* pObjectAccess = rptrRepository.GetInterface<sdv::core::IObjectAccess>();
    const sdv::installation::IAppInstall* pAppInstall = nullptr;
    if (pObjectAccess)
        pAppInstall = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("ConfigService")).
        GetInterface<sdv::installation::IAppInstall>();
    if (!pAppInstall)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CONFIG_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return CONFIG_SERVICE_ACCESS_ERROR;
    }

    // Create the object list
    std::vector<std::array<std::string, 1>> vecInstallList;
    vecInstallList.push_back({"Installation"});
    sdv::sequence<sdv::u8string> seqInstallations = pAppInstall->GetInstallations();
    for (const sdv::u8string& rssInstallation : seqInstallations)
        vecInstallList.push_back({
        rssInstallation});

    // Print the module list
    PrintTable(vecInstallList, rstream, rsContext.bListNoHdr);
    return NO_ERROR;
}

int ListConnections(const SContext& /*rsContext*/, const sdv::TObjectPtr& /*rptrRepository*/, std::ostream& /*rstream = std::cout */ )
{
    std::cout << "ERROR: " << NOT_IMPLEMENTED_MSG << " :-(" << std::endl;
    return NOT_IMPLEMENTED;
}
