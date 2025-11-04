#include "start_stop_service.h"
#include "../../global/cmdlnparser/cmdlnparser.h"
#include <interfaces/process.h>
#include <support/interface_ptr.h>
#include <support/local_service_access.h>
#include <interfaces/com.h>
#include <interfaces/app.h>
#include "../error_msg.h"

void StartStopServiceHelp(const SContext& rsContext)
{
    // First argument should be "START" or "STOP".
    if (rsContext.seqCmdLine.size() < 1)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: missing start/stop command..." << std::endl;
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "START"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control START <name> [options...]\n"
            "       sdv_control START <class> <name> [options...]\n\n"
            "Start a complex service with the supplied object name as is defined in the configuration (first usage) or the "
            "supplied class name and with object name assignment and add to the configuration (second usage).\n"
            "Only complex services can be started. If the service depends on not running other services, these are started as "
            "well.\n\n");
        return;
    }
    if (iequals(rsContext.seqCmdLine[0], "STOP"))
    {
        CCommandLine::PrintHelpText(std::cout, "Usage: sdv_control STOP <name> [options...]\n"
            "       sdv_control STOP <ID> [options...]\n"
            "       sdv_control STOP <class> [options...]\n"
            "       sdv_control STOP <module> [options...]\n\n"
            "Stop the complex service(s) with the supplied object name (first usage), with supplied object ID (second usage), with "
            "supplied class name (third usage) or contained in the module with the supplied module name (fourth usage).\n"
            "Only complex services can be stopped. Dependent services are stopped as well. If one of object to be stopped is not a "
            "complex service, the command fails.\n\n");
        return;
    }
    if (!rsContext.bSilent)
        std::cerr << "ERROR: invalid start/stop command..." << std::endl;
}


int StartService(const SContext& rsContext)
{
    // First argument should be "START"
    if (rsContext.seqCmdLine.size() < 2)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Not enough parameters for START command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (rsContext.seqCmdLine.size() > 3)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Too many parameters for START command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "START"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command: " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

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
            std::cerr << "ERROR: " << REPOSITORY_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return REPOSITORY_SERVICE_ACCESS_ERROR;
    }

    // Create the object
    sdv::core::TObjectID tObjectID = pRepoControl->CreateObject(ssClass, ssName, {});
    if (!tObjectID)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << START_OBJECT_ERROR_MSG << " Object name: " << ssName << std::endl;
        return START_OBJECT_ERROR;
    }

    // All good...
    return NO_ERROR;
}

int StopService(const SContext& rsContext)
{
    // First argument should be "STOP"
    if (rsContext.seqCmdLine.size() < 2)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Not enough parameters for STOP command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (rsContext.seqCmdLine.size() > 3)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Too many parameters for STOP command." << std::endl;
        return CMDLN_ARG_ERR;
    }
    if (!iequals(rsContext.seqCmdLine[0], "STOP"))
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << CMDLN_ARG_ERR_MSG << " Invalid command : " << rsContext.seqCmdLine[0] << std::endl;
        return CMDLN_ARG_ERR;
    }

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
            std::cerr << "ERROR: " << REPOSITORY_SERVICE_ACCESS_ERROR_MSG << std::endl;
        return REPOSITORY_SERVICE_ACCESS_ERROR;
    }

    // Create the object
    sdv::core::TObjectID tObjectID = pRepoControl->DestroyObject(ssName);
    if (!tObjectID)
    {
        if (!rsContext.bSilent)
            std::cerr << "ERROR: " << STOP_OBJECT_ERROR_MSG << " Object name: " << ssName << std::endl;
        return STOP_OBJECT_ERROR;
    }

    // All good...
    return NO_ERROR;
}
