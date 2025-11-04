#include "installation.h"
#include "../../global/cmdlnparser/cmdlnparser.h"
#include <interfaces/process.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <interfaces/com.h>
#include <interfaces/app.h>
#include "../error_msg.h"

void InstallationHelp(const SContext& rsContext)
{
    // First argument should be "INSTALL", "UPDATE" or "UNINSTALL".
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing install/update/uninstall command..." << std::endl;
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "INSTALL"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control INSTALL <name> [files...] [options...]\n\n"
            "Make a new installation with a set of files. This command succeeds when the files contain at least one component "
            "module and another installation with the same name is not existing. Complex services are automatically added to the "
            "configuration.\n\n");
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "UPDATE"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control UPDATE <name> [files...] [options...]\n\n"
            "Update an existing installation. This will not remove any configuration (unless a component is not available any "
            "more after, or a new component was added by the update.\n\n");
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "UNINSTALL"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control UNINSTALL <name> [files...] [options...]\n\n"
            "Remove an existing installation. This will remove the configuration for the components as well. Dependent components "
            "become inactive and will be activated again when the component is installed again.\n\n");
        return;
    }
    if (!rsContext.bSilent)
        std::cerr << "ERROR: invalid install/update/uninstall command..." << std::endl;
}

int Install(const SContext& rsContext)
{
    // First argument should be "INSTALL"
    if (rsContext.seqCmdLine.size() < 3)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Not enough parameters for installation command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "INSTALL"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command : " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

    // Followed by the installation name
    //std::string ssName = rsContext.seqCmdLine[1];



#if 0

    // Determine the service to start
    sdv::u8string ssClass, ssName;
    if (rsContext.seqCmdLine.size() == 3)
    {
        ssClass = rsContext.seqCmdLine[1];
        ssName = rsContext.seqCmdLine[2];
    } else
        ssName = rsContext.seqCmdLine[1];

    // Try to connect
    sdv::TObjectPtr ptrRepository = sdv::com::ConnectToLocalServerRepository(rsContext.uiInstanceID);
    if (!ptrRepository)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CONNECT_SDV_SERVER_ERROR_MSG << " Instance #" << rsContext.uiInstanceID << std::endl;
        return CONNECT_SDV_SERVER_ERROR;
    }

    // Get access to the module control service
    sdv::core::IObjectAccess* pObjectAccess = ptrRepository.GetInterface<sdv::core::IObjectAccess>();
    sdv::core::IRepositoryControl* pRepoControl = nullptr;
    if (pObjectAccess)
        pRepoControl = sdv::TInterfaceAccessPtr(pObjectAccess->GetObject("RepositoryService")).
        GetInterface<sdv::core::IRepositoryControl>();
    if (!pRepoControl)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << REPO_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return REPOSITORY_SERVICE_ACCESS_ERROR;
    }

    // Create the object
    sdv::core::TObjectID tObjectID = pRepoControl->CreateObject(ssClass, ssName, {});
    if (!tObjectID)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << START_OBJECT_ERROR_ERROR_MSG << " Object name: " << ssName << std::endl;
        return START_OBJECT_ERROR;
    }
#endif

    // All good...
    return NO_ERROR;
}

int Update(const SContext& /*rsContext*/)
{
    std::cout << "ERROR: " << NOT_IMPLEMENTED_MSG << " :-(" << std::endl;
    return NOT_IMPLEMENTED;
}

int Uninstall(const SContext& /*rsContext*/)
{
    std::cout << "ERROR: " << NOT_IMPLEMENTED_MSG << " :-(" << std::endl;
    return NOT_IMPLEMENTED;
}

