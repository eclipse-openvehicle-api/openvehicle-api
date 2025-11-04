#include "ps_class_generator_base.h"
#include "../exception.h"
#include <fstream>
#include <thread>
#include <chrono>

CPSClassGeneratorBase::CPSClassGeneratorBase(sdv::IInterfaceAccess* pParser) : CGenContext(pParser)
{}

bool CPSClassGeneratorBase::Generate()
{
    // Create target directory if it doesn't exist. Since rqce conditions could exist due to parallel processing, do this
    // five times.
    std::filesystem::path pathPSTarget = GetOutputDir() / "ps";
    for (size_t nCnt = 0; nCnt < 5; nCnt++)
    {
        if (!std::filesystem::exists(pathPSTarget))
            std::filesystem::create_directories(pathPSTarget);
        else
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    if (!std::filesystem::exists(pathPSTarget))
        throw CCompileException("Cannot create proxy/stub directory: ", pathPSTarget.generic_u8string());
    std::filesystem::path pathSerDesTarget = GetOutputDir() / "serdes";
    for (size_t nCnt = 0; nCnt < 5; nCnt++)
    {
        if (!std::filesystem::exists(pathSerDesTarget))
            std::filesystem::create_directories(pathSerDesTarget);
        else
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    if (!std::filesystem::exists(pathSerDesTarget))
        throw CCompileException("Cannot create serdes directory: ", pathSerDesTarget.generic_u8string());

    // Add "_proxy" to the path and replace the extension by ".cpp" and ".h";
    std::filesystem::path pathPSFileBase = pathPSTarget / GetSource().filename();
    pathPSFileBase.replace_extension("");
    std::filesystem::path pathFileNameDef = ".." / pathPSFileBase.filename();
    pathFileNameDef += ".h";
    pathPSFileBase += std::string("_") + GetNameAppendix();
    std::filesystem::path pathFileCpp = pathPSFileBase;
    pathFileCpp += ".cpp";
    std::filesystem::path pathFileHdr = pathPSFileBase;
    pathFileHdr += ".h";
    std::filesystem::path pathSerDesFileBase = pathSerDesTarget / GetSource().filename();
    pathSerDesFileBase.replace_extension("");
    std::filesystem::path pathSerDesFile = pathSerDesFileBase;
    pathSerDesFile += "_serdes.h";

    if (g_log_control.GetVerbosityMode() == EVerbosityMode::report_all)
    {
        std::cout << "Target header file: " << pathFileHdr.generic_u8string() << std::endl;
        std::cout << "Target source file: " << pathFileCpp.generic_u8string() << std::endl;
    }

    // Open the file for writing
    std::ofstream streamCpp(pathFileCpp);
    std::ofstream streamHdr(pathFileHdr);

    // Add file headers
    streamHdr << Header(pathFileHdr, GetClassDefFileComments());
    streamCpp << Header(pathFileCpp, GetClassImplFileComments());

    // Add safeguard
    streamHdr << Safeguard(pathFileHdr, true);

    // Include proxy stub base header
    streamHdr << "// Proxy/stub interfaces." << std::endl;
    streamHdr << "#include <interfaces/core_ps.h>" << std::endl;
    streamHdr << "#include <support/pssup.h>" << std::endl;

    // Include definition header in header file
    streamHdr << "#include \"" << pathFileNameDef.generic_u8string() << "\"" << std::endl;
    streamHdr << std::endl;

    // Include definition header in cpp file
    streamCpp << "#include \"" << pathFileHdr.filename().generic_u8string() << "\"" << std::endl;

    // Include serdes header in cpp file
    streamCpp << "#include \"../serdes/" << pathSerDesFile.filename().generic_u8string() << "\"" << std::endl;
    streamCpp << "#include <support/serdes.h>" << std::endl;
    streamCpp << "#include <support/pssup.h>" << std::endl;
    streamCpp << std::endl;

    // Run through the entities and create the proxy code of every interface...
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(GetParser());
    if (!pDefinition) throw CCompileException("Internal error: the parser doesn't have a root definition.");
    sdv::idl::IEntityIterator* pIterator = pDefinition->GetChildren();
    if (!pIterator) throw CCompileException("Internal error: the parser doesn't support entity iteration.");
    ProcessEntities(streamHdr, streamCpp, pIterator);

    // End of safeguard
    streamHdr << Safeguard(pathFileHdr, false);

    return true;
}

void CPSClassGeneratorBase::ProcessEntities(std::ostream& rstreamHdr, std::ostream& rstreamCpp,
    sdv::idl::IEntityIterator* pIterator)
{
    if (!pIterator) throw CCompileException("Internal error: processing entities without iterator.");
    for (uint32_t uiIndex = 0; uiIndex < pIterator->GetCount(); uiIndex++)
    {
        // Get the entity
        sdv::IInterfaceAccess* pEntity = pIterator->GetEntityByIndex(uiIndex);
        if (!pEntity) throw CCompileException("Internal error: processing non-existent entity.");

        // Get the entity interfaces.
        const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
        if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
        const sdv::idl::IEntityContext* pContext = GetInterface<sdv::idl::IEntityContext>(pEntity);
        if (!pContext) throw CCompileException("Internal error: the entity doesn't expose context information.");
        sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pEntity);
        const sdv::idl::IInterfaceEntity* pInterface = GetInterface<sdv::idl::IInterfaceEntity>(pEntity);

        // Only process entities in the source code
        if (pContext->GetLocation() != sdv::idl::IEntityContext::ELocation::source)
            continue;

        // Forward declaration
        if (pEntityInfo->ForwardDeclaration())
            continue;

        // Process interfaces only... but only when not local
        if (pInterface && pInterface->IsLocal()) continue;
        switch (pEntityInfo->GetType())
        {
        case sdv::idl::EEntityType::type_interface:
            StreamInterface(rstreamHdr, rstreamCpp, pEntity);
            break;
        default:
            break;
        }

        // Does the entity have children?
        sdv::idl::IEntityIterator* pChildIterator = pDefinition ? pDefinition->GetChildren() : nullptr;
        if (pChildIterator) ProcessEntities(rstreamHdr, rstreamCpp, pChildIterator);
    }
}

void CPSClassGeneratorBase::StreamInterface(std::ostream& rstreamHdr, std::ostream& rstreamCpp,
    sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");

    std::string ssClassName = QualifyName(pEntityInfo->GetScopedName()) + "__" + GetNameAppendix();

    // TODO: Deal with multiple inheritance

    std::string ssAliasName = pEntityInfo->GetScopedName();
    for (size_t nPos = ssAliasName.find("::"); nPos != std::string::npos; nPos = ssAliasName.find("::"))
        ssAliasName.replace(nPos, 2, "_");
    CKeywordMap mapKeywords = {
        {"class_name", ssClassName},
        {"alias_name", ssAliasName},
        {"interface_name", pEntityInfo->GetScopedName()},
        {"interface_id", std::to_string(pEntityInfo->GetId())}
    };

    // Generate class definition and constructor/destructor
    rstreamHdr << ReplaceKeywords(GetClassDefBegin(mapKeywords), mapKeywords);
    rstreamCpp << ReplaceKeywords(GetConstructImplBegin(mapKeywords), mapKeywords);

    // Stream the interface content
    std::stringstream sstreamImpl;
    uint32_t uiFuncIndex = 0;
    StreamInterfaceContent(rstreamHdr, rstreamCpp, sstreamImpl, mapKeywords, pEntity, uiFuncIndex);

    // Finalize the interface
    rstreamHdr << ReplaceKeywords(GetClassDefEnd(mapKeywords), mapKeywords);
    rstreamCpp << ReplaceKeywords(GetConstructImplEnd(mapKeywords), mapKeywords);
    rstreamCpp << sstreamImpl.str();
}

void CPSClassGeneratorBase::StreamInterfaceContent(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody,
    std::ostream& rstreamClassImpl, const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt)
{
    // Stream base interfaces first
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pEntity);
    sdv::idl::IEntityIterator* pInheritanceIterator = pDefinition ? pDefinition->GetInheritance() : nullptr;
    for (uint32_t uiIndex = 0; pInheritanceIterator && uiIndex < pInheritanceIterator->GetCount(); uiIndex++)
    {
        sdv::IInterfaceAccess* pBaseEntity = pInheritanceIterator->GetEntityByIndex(uiIndex);
        if (!pBaseEntity->GetInterface<sdv::idl::IEntityInfo>())
            throw CCompileException("Internal error: the entity inherits from an unknown entity.");
        StreamInterfaceContent(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, rmapKeywords, pBaseEntity, ruiFuncCnt);
    }

    // Run through the children and process attributes and operations
    sdv::idl::IEntityIterator* pChildIterator = pDefinition ? pDefinition->GetChildren() : nullptr;
    for (uint32_t uiIndex = 0; pChildIterator && uiIndex < pChildIterator->GetCount(); uiIndex++)
    {
        // Get the entity
        sdv::IInterfaceAccess* pChildEntity = pChildIterator->GetEntityByIndex(uiIndex);
        if (!pEntity) throw CCompileException("Internal error: processing non-existent entity.");

        // Get the entity interfaces.
        const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pChildEntity);
        if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
        const sdv::idl::IEntityContext* pContext = GetInterface<sdv::idl::IEntityContext>(pChildEntity);
        if (!pContext) throw CCompileException("Internal error: the entity doesn't expose context information.");

        // Forward declaration
        if (pEntityInfo->ForwardDeclaration())
            continue;

        switch (pEntityInfo->GetType())
        {
        case sdv::idl::EEntityType::type_attribute:
            StreamAttribute(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, rmapKeywords, pChildEntity, ruiFuncCnt);
            break;
        case sdv::idl::EEntityType::type_operation:
            StreamOperation(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, rmapKeywords, pChildEntity, ruiFuncCnt);
            break;
        default:
            break;
        }
    }
}

void CPSClassGeneratorBase::StreamAttribute(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody,
    std::ostream& rstreamClassImpl, const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_attribute)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CExceptionVector vecReadExceptions;
    sdv::idl::IAttributeEntity* pAttribute = GetInterface<sdv::idl::IAttributeEntity>(pEntity);
    if (!pAttribute) throw CCompileException("Internal error: operation is not exposing operation information.");
    sdv::idl::IEntityIterator* pExceptionIterator = pAttribute->GetReadExceptions();
    for (uint32_t uiIndex = 0; pExceptionIterator && uiIndex < pExceptionIterator->GetCount(); uiIndex++)
    {
        // Get the parameter entity
        sdv::IInterfaceAccess* pExceptionEntity = pExceptionIterator->GetEntityByIndex(uiIndex);
        if (!pExceptionEntity) throw CCompileException("Internal error: processing non-existent exception entity.");
        const sdv::idl::IEntityInfo* pExceptionEntityInfo = pExceptionEntity->GetInterface<sdv::idl::IEntityInfo>();
        if (!pExceptionEntityInfo) throw CCompileException("Internal error: processing non-existent exception entity.");
        vecReadExceptions.push_back(pExceptionEntityInfo->GetScopedName());
    }

    // Stream the getter function
    CKeywordMap mapKeywordsGetter = rmapKeywords;
    mapKeywordsGetter.insert(std::make_pair("func_name", std::string("get_") + pEntityInfo->GetName()));
    StreamFunction(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, mapKeywordsGetter, pEntity, ruiFuncCnt, true,
        std::vector<sdv::IInterfaceAccess*>(), vecReadExceptions);

    // Stream the setter function if not readonly.
    if (!pDeclaration->IsReadOnly())
    {
        CExceptionVector vecWriteExceptions;
        pExceptionIterator = pAttribute->GetWriteExceptions();
        for (uint32_t uiIndex = 0; pExceptionIterator && uiIndex < pExceptionIterator->GetCount(); uiIndex++)
        {
            // Get the parameter entity
            sdv::IInterfaceAccess* pExceptionEntity = pExceptionIterator->GetEntityByIndex(uiIndex);
            if (!pExceptionEntity) throw CCompileException("Internal error: processing non-existent exception entity.");
            const sdv::idl::IEntityInfo* pExceptionEntityInfo = pExceptionEntity->GetInterface<sdv::idl::IEntityInfo>();
            if (!pExceptionEntityInfo) throw CCompileException("Internal error: processing non-existent exception entity.");
            vecWriteExceptions.push_back(pExceptionEntityInfo->GetScopedName());
        }

        CKeywordMap mapKeywordsSetter = rmapKeywords;
        mapKeywordsSetter.insert(std::make_pair("func_name", std::string("set_") + pEntityInfo->GetName()));
        StreamFunction(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, mapKeywordsSetter, nullptr, ruiFuncCnt, false,
            std::vector<sdv::IInterfaceAccess*>({ pEntity }), vecWriteExceptions);
    }
}

void CPSClassGeneratorBase::StreamOperation(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody,
    std::ostream& rstreamClassImpl, const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pEntity, uint32_t& ruiFuncCnt)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_operation)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    // Build a parameter vector (if there are any).
    std::vector<sdv::IInterfaceAccess*> vecParams;
    sdv::idl::IOperationEntity* pOperation = GetInterface<sdv::idl::IOperationEntity>(pEntity);
    if (!pOperation) throw CCompileException("Internal error: operation is not exposing operation information.");
    sdv::idl::IEntityIterator* pParamIterator = pOperation->GetParameters();
    for (uint32_t uiIndex = 0; pParamIterator && uiIndex < pParamIterator->GetCount(); uiIndex++)
    {
        // Get the parameter entity
        sdv::IInterfaceAccess* pParamEntity = pParamIterator->GetEntityByIndex(uiIndex);
        if (!pParamEntity) throw CCompileException("Internal error: processing non-existent parameter entity.");
        const sdv::idl::IParameterEntity* pParameter = GetInterface<sdv::idl::IParameterEntity>(pParamEntity);
        if (!pParameter)
            throw CCompileException("Internal error: the entity is a parameter, but doesn't expose parameter information.");
        vecParams.push_back(pParamEntity);
    }

    CExceptionVector vecExceptions;
    sdv::idl::IEntityIterator* pExceptionIterator = pOperation ? pOperation->GetExceptions() : nullptr;
    for (uint32_t uiIndex = 0; pExceptionIterator && uiIndex < pExceptionIterator->GetCount(); uiIndex++)
    {
        // Get the parameter entity
        sdv::IInterfaceAccess* pExceptionEntity = pExceptionIterator->GetEntityByIndex(uiIndex);
        if (!pExceptionEntity) throw CCompileException("Internal error: processing non-existent exception entity.");
        const sdv::idl::IEntityInfo* pExceptionEntityInfo = pExceptionEntity->GetInterface<sdv::idl::IEntityInfo>();
        if (!pExceptionEntityInfo) throw CCompileException("Internal error: processing non-existent exception entity.");
        vecExceptions.push_back(pExceptionEntityInfo->GetScopedName());
    }

    // Stream the operation
    CKeywordMap mapKeywordsOperation = rmapKeywords;
    mapKeywordsOperation.insert(std::make_pair("func_name", pEntityInfo->GetName()));
    StreamFunction(rstreamClassDef, rstreamConstrBody, rstreamClassImpl, mapKeywordsOperation, pEntity, ruiFuncCnt,
        pDeclaration->IsReadOnly(), vecParams, vecExceptions);
}

void CPSClassGeneratorBase::StreamFunction(std::ostream& rstreamClassDef, std::ostream& rstreamConstrBody,
    std::ostream& rstreamClassImpl, const CKeywordMap& rmapKeywords, sdv::IInterfaceAccess* pRetParam,
    uint32_t& ruiFuncCnt, bool bConst, const std::vector<sdv::IInterfaceAccess*>& rvecParams, const CExceptionVector& rvecExceptions)
{
    // Get the parameter information and build the parameter pack definitions
    std::vector<SParamInfo> vecParamInfos;
    std::stringstream sstreamParamPackDef, sstreamParamPackUse;
    SParamInfo sReturnInfo = GetParamInfo(pRetParam, true);
    if (sReturnInfo.bValidType)
        vecParamInfos.push_back(sReturnInfo);
    size_t nInputCnt = 0;
    size_t nOutputCnt = vecParamInfos.size();
    for (sdv::IInterfaceAccess* pParam : rvecParams)
    {
        // Get parameter info
        SParamInfo sParamInfo = GetParamInfo(pParam);

        // Add to parameter pack definition
        switch (sParamInfo.eDirection)
        {
        case SParamInfo::EDirection::in:
            if (!sstreamParamPackDef.str().empty()) sstreamParamPackDef << ", ";
            if (!sstreamParamPackUse.str().empty()) sstreamParamPackUse << ", ";
            if ((sParamInfo.bIsPointer || sParamInfo.bIsComplex) && !sParamInfo.bIsInterface)
                sstreamParamPackDef << "const ";
            sstreamParamPackDef << sParamInfo.ssDeclType;
            if (sParamInfo.bIsComplex && !sParamInfo.bIsInterface)
                sstreamParamPackDef << "&";
            sstreamParamPackDef << " " << sParamInfo.ssName;
            sstreamParamPackUse << sParamInfo.ssName;
            nInputCnt++;
            break;
        case SParamInfo::EDirection::inout:
            nInputCnt++;
            if (!sstreamParamPackDef.str().empty()) sstreamParamPackDef << ", ";
            if (!sstreamParamPackUse.str().empty()) sstreamParamPackUse << ", ";
            sstreamParamPackDef << sParamInfo.ssDeclType << "& " << sParamInfo.ssName;
            sstreamParamPackUse << sParamInfo.ssName;
            nOutputCnt++;
            nInputCnt++;
            break;
        case SParamInfo::EDirection::out:
            if (!sstreamParamPackDef.str().empty()) sstreamParamPackDef << ", ";
            if (!sstreamParamPackUse.str().empty()) sstreamParamPackUse << ", ";
            sstreamParamPackDef << sParamInfo.ssDeclType << "& " << sParamInfo.ssName;
            sstreamParamPackUse << sParamInfo.ssName;
            nOutputCnt++;
            break;
        default:
            // Do not add anything
            break;
        }

        // Store in vector
        vecParamInfos.push_back(std::move(sParamInfo));
    }

    // In case there are no parameters and no return values, add at least one parameter to be able to receive exceptions.
	if (vecParamInfos.empty())
		vecParamInfos.push_back(SParamInfo());

    // Get function information
    SFuncInfo sFuncInfo{};
    sFuncInfo.ssName = ReplaceKeywords("%func_name%", rmapKeywords);
    sFuncInfo.ssDecl = sReturnInfo.ssDeclType;
	sFuncInfo.ssDeclType = sReturnInfo.ssDeclType;
	sFuncInfo.ssDefRetValue = sReturnInfo.ssDefaultValue;
    sFuncInfo.bIsConst = bConst;
	sFuncInfo.nInputParamCnt = nInputCnt;
	sFuncInfo.nOutputParamCnt = nOutputCnt;

    // Stream the getter function
    CKeywordMap mapKeywordsFunction = rmapKeywords;
	mapKeywordsFunction.insert(std::make_pair("func_decl_type", sReturnInfo.ssDeclType));
	mapKeywordsFunction.insert(std::make_pair("func_default_ret_value", sReturnInfo.ssDefaultValue));
	mapKeywordsFunction.insert(std::make_pair("func_index", std::to_string(ruiFuncCnt)));
    mapKeywordsFunction.insert(std::make_pair("param_pack_def", sstreamParamPackDef.str()));
    mapKeywordsFunction.insert(std::make_pair("param_pack_use", sstreamParamPackUse.str()));
	mapKeywordsFunction.insert(std::make_pair("total_param_cnt", std::to_string(vecParamInfos.size())));

    // Stream constructor implementation
    rstreamConstrBody << ReplaceKeywords(GetConstructFuncImpl(sFuncInfo, mapKeywordsFunction), mapKeywordsFunction);

    // Stream func prototype
    rstreamClassDef << ReplaceKeywords(GetFuncDef(sFuncInfo, mapKeywordsFunction), mapKeywordsFunction);

    // Stream the parameter init code.
    std::stringstream sstreamParamInit;
    size_t nIndex = 0;
    for (const SParamInfo& rsParam : vecParamInfos)
    {
        CKeywordMap mapKeywordsParam = mapKeywordsFunction;
        mapKeywordsParam.insert(std::make_pair("param_name", rsParam.ssName));
		mapKeywordsParam.insert(std::make_pair("param_decl_type", rsParam.ssDeclType));
		mapKeywordsParam.insert(std::make_pair("param_index", std::to_string(nIndex)));
        mapKeywordsParam.insert(std::make_pair("param_default_val", rsParam.ssDefaultValue));
        mapKeywordsParam.insert(std::make_pair("param_size", rsParam.ssSize));
        sstreamParamInit << ReplaceKeywords(GetFuncImplParamInit(sFuncInfo, rsParam, mapKeywordsParam), mapKeywordsParam);
        nIndex++;
    }

    // Stream the parameter input code.
    std::stringstream sstreamParamInput;
    nIndex = 0;
    for (const SParamInfo& rsParam : vecParamInfos)
    {
        CKeywordMap mapKeywordsParam = mapKeywordsFunction;
        mapKeywordsParam.insert(std::make_pair("param_name", rsParam.ssName));
        mapKeywordsParam.insert(std::make_pair("param_decl_type", rsParam.ssDeclType));
        mapKeywordsParam.insert(std::make_pair("param_index", std::to_string(nIndex)));
        mapKeywordsParam.insert(std::make_pair("param_default_val", rsParam.ssDefaultValue));
        mapKeywordsParam.insert(std::make_pair("param_size", rsParam.ssSize));
        sstreamParamInput << ReplaceKeywords(GetFuncImplStreamParamInput(sFuncInfo, rsParam, mapKeywordsParam), mapKeywordsParam);
        nIndex++;
    }

    // Stream the parameter input code.
    std::stringstream sstreamParamOutput;
    nIndex = 0;
    for (const SParamInfo& rsParam : vecParamInfos)
    {
        CKeywordMap mapKeywordsParam = mapKeywordsFunction;
        mapKeywordsParam.insert(std::make_pair("param_name", rsParam.ssName));
        mapKeywordsParam.insert(std::make_pair("param_decl_type", rsParam.ssDeclType));
        mapKeywordsParam.insert(std::make_pair("param_index", std::to_string(nIndex)));
        mapKeywordsParam.insert(std::make_pair("param_default_val", rsParam.ssDefaultValue));
        mapKeywordsParam.insert(std::make_pair("param_size", rsParam.ssSize));
        sstreamParamOutput << ReplaceKeywords(GetFuncImplStreamParamOutput(sFuncInfo, rsParam, mapKeywordsParam), mapKeywordsParam);
        nIndex++;
    }

    // Stream the parameter term code.
    std::stringstream sstreamParamTerm;
    nIndex = 0;
    for (const SParamInfo& rsParam : vecParamInfos)
    {
        CKeywordMap mapKeywordsParam = mapKeywordsFunction;
        mapKeywordsParam.insert(std::make_pair("param_name", rsParam.ssName));
        mapKeywordsParam.insert(std::make_pair("param_decl_type", rsParam.ssDeclType));
        mapKeywordsParam.insert(std::make_pair("param_index", std::to_string(nIndex)));
        mapKeywordsParam.insert(std::make_pair("param_default_val", rsParam.ssDefaultValue));
        mapKeywordsParam.insert(std::make_pair("param_size", rsParam.ssSize));
        sstreamParamInit << ReplaceKeywords(GetFuncImplParamTerm(sFuncInfo, rsParam, mapKeywordsParam), mapKeywordsParam);
        nIndex++;
    }

    // Stream func implementation
    CKeywordMap mapKeywordsFunctionImpl = mapKeywordsFunction;
    mapKeywordsFunctionImpl.insert(std::make_pair("param_init", sstreamParamInit.str()));
    mapKeywordsFunctionImpl.insert(std::make_pair("stream_param_input", sstreamParamInput.str()));
    mapKeywordsFunctionImpl.insert(std::make_pair("stream_param_output", sstreamParamOutput.str()));
    mapKeywordsFunctionImpl.insert(std::make_pair("param_term", sstreamParamTerm.str()));
    rstreamClassImpl << ReplaceKeywords(GetFuncImpl(sFuncInfo, mapKeywordsFunctionImpl, rvecExceptions), mapKeywordsFunctionImpl);

    // Increase the function index
    ruiFuncCnt++;
}

CPSClassGeneratorBase::SParamInfo CPSClassGeneratorBase::GetParamInfo(sdv::IInterfaceAccess* pParam, bool bIsRetValue /*= false*/) const
{
    SParamInfo sInfo;
    if (bIsRetValue)
        sInfo.eDirection = SParamInfo::EDirection::ret;
    if (!pParam)
    {
        // Special case... void return parameter
        if (!bIsRetValue) throw CCompileException("Internal error: function parameter cannot be void.");
        sInfo.ssDeclType = "void";
        sInfo.eDirection = SParamInfo::EDirection::ret;
        return sInfo;
    }

    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pParam);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pParam);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");
    const sdv::idl::IParameterEntity* pParameter = GetInterface<sdv::idl::IParameterEntity>(pParam);

    sInfo.ssName = bIsRetValue ? "return_value" : pEntityInfo->GetName();
    static_cast<SCDeclInfo&>(sInfo) = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), std::string(), true);
    if (sInfo.bIsDynamic)
    {
        sInfo.ssSize = std::string("(") + sInfo.ssName + " ? static_cast<uint32_t>(std::char_traits<" + sInfo.ssDeclType +
            ">::length(" + sInfo.ssName + ") + 1) * sizeof(" + sInfo.ssDeclType + ") : 0)";
        sInfo.eAllocType = SParamInfo::EAllocType::indirect;
    } else
        sInfo.ssSize = std::string("static_cast<uint32_t>(sizeof(") + sInfo.ssDeclType + "))";
    if (!bIsRetValue || sInfo.ssDeclType != "void")
    {
        // GCC-issue: Types defined as "long long", "long int", "long long int", "long double" cannot be initialized using brackets.
        if (sInfo.ssDeclType.substr(0, 4) == "long")
            sInfo.ssDefaultValue = sInfo.bIsPointer ? "nullptr" : std::string("static_cast<") + sInfo.ssDeclType + ">(0)";
        else
            sInfo.ssDefaultValue = sInfo.bIsPointer ? "nullptr" : sInfo.ssDeclType + "{}";
    }
    if (!bIsRetValue)
    {
        // Stream the parameter direction. All but the input parameter need to support the ouput of values.
        switch (pParameter ? pParameter->GetDirection() : sdv::idl::IParameterEntity::EParameterDirection::input)
        {
        case sdv::idl::IParameterEntity::EParameterDirection::output:
            sInfo.eDirection = SParamInfo::EDirection::out;
            break;
        case sdv::idl::IParameterEntity::EParameterDirection::in_out:
            sInfo.eDirection = SParamInfo::EDirection::inout;
            break;
        case sdv::idl::IParameterEntity::EParameterDirection::input:
        default:
            sInfo.eDirection = SParamInfo::EDirection::in;
            break;
        }
    }
    return sInfo;
}

void CPSClassGeneratorBase::StreamMarshallDecl(std::ofstream& rstream, const CKeywordMap& rmapKeywords, uint32_t uiFuncIndex,
    uint32_t uiParamCnt)
{
    CKeywordMap mapKeywordsMarshall = rmapKeywords;
    mapKeywordsMarshall.insert(std::make_pair("index", std::to_string(uiFuncIndex)));
    mapKeywordsMarshall.insert(std::make_pair("paramcnt", std::to_string(uiParamCnt)));

    // Code generation
    std::string ssDeclCode = R"code(    // Declare the marshall structure
    sdv::core::SMarshall sPacket{};
    union { uint16_t uiWord; uint8_t rguiBytes[2]; } uEndian = {1};
    sPacket.uiEndian = uEndian.rguiBytes[1];
    sPacket.uiPadding = 0;
    sPacket.uiVersion = 100;
    sPacket.tIfcId = %interface_name%::_id;
    sPacket.uiFuncIndex = %index%;
)code";

    rstream << ReplaceKeywords(ssDeclCode, mapKeywordsMarshall);
    if (uiParamCnt)
    {
        std::string ssParamDeclCode = R"code(    sPacket.uiCount = %paramcnt%;
    sdv::core::SMarshall::SParam rgsParams[%paramcnt%] = {};
    sPacket.rgsParams = rgsParams;
)code";
        rstream << ReplaceKeywords(ssParamDeclCode, mapKeywordsMarshall);
    }
}

bool CPSClassGeneratorBase::RuntimeProcessingRequired(sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    sdv::idl::IDeclarationEntity* pDeclaration = pEntity->GetInterface<sdv::idl::IDeclarationEntity>();
    if (!pDeclaration) throw CCompileException("Internal error: expecting a declaration.");

    // Request the type
	sdv::IInterfaceAccess* pTypeObj = pDeclaration->GetDeclarationType();
	if (!pTypeObj) throw CCompileException("Internal error: expecting a declaration type.");
	const sdv::idl::IDeclarationType* pDeclType = pTypeObj->GetInterface<sdv::idl::IDeclarationType>();
    if (!pDeclType) throw CCompileException("Internal error: expecting a declaration type.");
	sdv::idl::EDeclType eType = pDeclType->GetBaseType();
	sdv::IInterfaceAccess* pType = pDeclType->GetTypeDefinition();

    // Check whether the entity requires runtime processing.
    switch (eType)
    {
    case sdv::idl::EDeclType::decltype_interface:
	case sdv::idl::EDeclType::decltype_string:
	case sdv::idl::EDeclType::decltype_u8string:
	case sdv::idl::EDeclType::decltype_u16string:
	case sdv::idl::EDeclType::decltype_u32string:
	case sdv::idl::EDeclType::decltype_wstring:
        return true;
    default:
        break;
    }

    // If there is a pType, this can either be a typedef or a definition.
    const sdv::idl::IDeclarationEntity* pTypedefDeclaration = pType->GetInterface<sdv::idl::IDeclarationEntity>();

    // Forward the request in case the type is a declaration
    if (pTypedefDeclaration) return RuntimeProcessingRequired(pType);

    // Get the definition and check for children
    sdv::idl::IDefinitionEntity* pDefinition = pType->GetInterface<sdv::idl::IDefinitionEntity>();
    sdv::idl::IEntityIterator* pChildIterator =
        pDefinition ? pType->GetInterface<sdv::idl::IEntityIterator>() : nullptr;
    for (uint32_t uiIndex = 0; pChildIterator && uiIndex < pChildIterator->GetCount(); uiIndex++)
    {
        sdv::IInterfaceAccess* pChildEntity = pChildIterator->GetEntityByIndex(uiIndex);
        if (!pChildEntity) throw CCompileException("Internal error: definition doesn't have a valid child entity.");
        const sdv::idl::IEntityInfo* pChildEntityInfo = pChildEntity->GetInterface<sdv::idl::IEntityInfo>();
        if (!pChildEntityInfo) throw CCompileException("Internal error: definition doesn't have valid child entity info.");
        if (pChildEntityInfo->GetType() != sdv::idl::EEntityType::type_variable) continue;  // Only variables are of interest.
        const sdv::idl::IDeclarationEntity* pChildDeclaration = pChildEntity->GetInterface<sdv::idl::IDeclarationEntity>();
        if (!pChildDeclaration) throw CCompileException("Internal error: variable doesn't expose a declaration interface.");
        if (pChildDeclaration->IsReadOnly()) continue; // Static const variables are counting.

        // Check the child
        if (RuntimeProcessingRequired(pChildEntity))
            return true;   // At least one member of the variable requires runtime processing.
    }

    // No variables requiring runtime processing detected
    return false;
}
