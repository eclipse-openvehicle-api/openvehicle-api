#ifndef DEFINITION_GENERATOR_BASE_INL
#define DEFINITION_GENERATOR_BASE_INL

#ifndef DEFINITION_GENERATOR_BASE_H
#error Do not include this file directly. Include "definition_generator_base.h" instead.
#endif

#include "../exception.h"
#include <filesystem>
#include <thread>
#include <fstream>

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::CDefEntityContext(const CGenContext& rContext, sdv::IInterfaceAccess* pEntity) :
    m_rGenContext(rContext), m_pDefEntity(pEntity), m_ptrSwitchCodeMap(std::make_shared<TSwitchCodeMap>())
{
    static_assert(std::is_base_of_v<CDefEntityContext, TDerivedContext>);
}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::CDefEntityContext(CDefEntityContext& rcontext, sdv::IInterfaceAccess* pEntity) :
    m_rGenContext(rcontext.m_rGenContext), m_pDefEntity(pEntity), m_ssIndent(rcontext.m_ssIndent),
    m_ptrSwitchCodeMap(pEntity == rcontext.m_pDefEntity ? rcontext.m_ptrSwitchCodeMap : std::make_shared<TSwitchCodeMap>())
{}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::~CDefEntityContext()
{}

template <typename TDerivedContext>
TDerivedContext& CDefEntityContext<TDerivedContext>::operator<<(const TDerivedContext& /*rContext*/)
{
    return static_cast<TDerivedContext&>(*this);
}

template <typename TDerivedContext>
inline std::string CDefEntityContext<TDerivedContext>::GetScope() const
{
    if (!m_pDefEntity) return {};
    const sdv::idl::IEntityInfo* pEntityInfo = m_pDefEntity->GetInterface<sdv::idl::IEntityInfo>();
    if (!pEntityInfo) return {};
    return pEntityInfo->GetScopedName();
}

template <typename TDerivedContext>
inline bool CDefEntityContext<TDerivedContext>::IsCompound() const
{
    if (!m_pDefEntity) return false;
    const sdv::idl::IEntityInfo* pEntityInfo = m_pDefEntity->GetInterface<sdv::idl::IEntityInfo>();
    if (!pEntityInfo) return false;
    return pEntityInfo->GetType() != sdv::idl::EEntityType::type_module;
}

template <typename TDerivedContext>
inline bool CDefEntityContext<TDerivedContext>::IsStructural() const
{
    const sdv::idl::IEntityInfo* pEntityInfo = m_rGenContext.GetInterface<sdv::idl::IEntityInfo>(m_pDefEntity);
    if (!pEntityInfo) return false;
    switch (pEntityInfo->GetType())
    {
    case sdv::idl::EEntityType::type_struct:
    case sdv::idl::EEntityType::type_union:
    case sdv::idl::EEntityType::type_exception:
        return true;
    default:
        return false;
    }
}

template <typename TDerivedContext>
inline std::string CDefEntityContext<TDerivedContext>::GetIndent(bool bDefBody /*= true*/, bool bFuncImpl /*= false*/) const
{
    return m_ssIndent + (((bDefBody && m_bDeepDefIndent) || bFuncImpl) ? m_rGenContext.GetIndentChars() : "");
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::IncrIndent()
{
    EnableIndent();
    m_ssIndent += m_rGenContext.GetIndentChars();
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::DecrIndent()
{
    EnableIndent();
    m_ssIndent.resize(m_ssIndent.size() - m_rGenContext.GetIndentChars().size());
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::DisableIndent()
{
    if (!m_ssIndent.empty())
    {
        m_ssIndentBackup = m_ssIndent;
        m_ssIndent.clear();
    }
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::EnableIndent()
{
    if (!m_ssIndentBackup.empty())
    {
        m_ssIndent = m_ssIndentBackup;
        m_ssIndentBackup.clear();
    }
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::EnableDeepIndent()
{
    m_bDeepDefIndent = true;
}

template <typename TDerivedContext>
inline sdv::IInterfaceAccess* CDefEntityContext<TDerivedContext>::GetDefEntity() const
{
    return m_pDefEntity;
}

template <typename TDerivedContext>
template <typename TInterface>
inline TInterface* CDefEntityContext<TDerivedContext>::GetDefEntity() const
{
    return m_rGenContext.GetInterface<TInterface>(m_pDefEntity);
}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::SIterator::SIterator(CDefEntityContext<TDerivedContext>& rContextParam) :
    rContext(rContextParam), bValid(true), itPos(rContextParam.CreateIteratorObject())
{}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::SIterator::SIterator(SIterator&& rsIterator) :
    rContext(rsIterator.rContext), itPos(rsIterator.itPos), bValid(rsIterator.bValid)
{
    // The iterator that was moved from is not valid any more.
    rsIterator.bValid = false;
}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::SIterator::~SIterator()
{
    Release();
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::SIterator::Release()
{
    if (!bValid) return;
    rContext.RemoveIteratorObject(itPos);
    bValid = false;
}

template <typename TDerivedContext>
inline typename CDefEntityContext<TDerivedContext>::SIterator& CDefEntityContext<TDerivedContext>::SIterator::operator++()
{
    if (bValid) (*itPos)++;
    return *this;
}

template <typename TDerivedContext>
inline uint32_t CDefEntityContext<TDerivedContext>::SIterator::operator++(int /*iVal*/)
{
    if (!bValid) return 0;
    uint32_t uiTemp = *itPos;
    (*itPos)++;
    return uiTemp;
}

template <typename TDerivedContext>
inline CDefEntityContext<TDerivedContext>::SIterator::operator uint32_t() const
{
    if (!bValid) return 0;
    return *itPos;
}

template <typename TDerivedContext>
inline typename CDefEntityContext<TDerivedContext>::SIterator CDefEntityContext<TDerivedContext>::CreateIterator()
{
    return SIterator(*this);
}

template <typename TDerivedContext>
inline uint32_t CDefEntityContext<TDerivedContext>::GetCurrentIteration()
{
    return m_lstIterators.empty() ? 0u : m_lstIterators.back();
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::AssignSwitchVarContext(const std::shared_ptr<SSwitchVarContext>& rptrSwitchVarContext)
{
    m_vecSwitchVars.push_back(rptrSwitchVarContext);
}

template <typename TDerivedContext>
template <typename TSwitchCodeContext>
inline std::shared_ptr<TSwitchCodeContext> CDefEntityContext<TDerivedContext>::GetOrCreateSwitchCodeContext(const std::string& rssSwitchVarName,
    const std::shared_ptr<SSwitchVarContext>& rptrSwitchVar, const std::vector<SArrayIterationInfo>& rvecArrayIndices)
{
    static_assert(std::is_base_of_v<SSwitchCodeContext, TSwitchCodeContext>);

    // Get the switch code map...
    if (!m_ptrSwitchCodeMap) return {};     // Should not occur

    auto itSwitchCodeContext = m_ptrSwitchCodeMap->find(rssSwitchVarName);
    if (itSwitchCodeContext == m_ptrSwitchCodeMap->end())
    {
        itSwitchCodeContext = m_ptrSwitchCodeMap->emplace(rssSwitchVarName, std::make_shared<TSwitchCodeContext>()).first;
        itSwitchCodeContext->second->ptrSwitchVar = rptrSwitchVar;
        itSwitchCodeContext->second->ssSwitchVarName = rssSwitchVarName;
        itSwitchCodeContext->second->vecArrayIterationInfo = rvecArrayIndices;
    }
    return std::static_pointer_cast<TSwitchCodeContext>(itSwitchCodeContext->second);
}

template <typename TDerivedContext>
inline bool CDefEntityContext<TDerivedContext>::HasMultipleSwitchCodeContexts() const
{
    if (!m_ptrSwitchCodeMap) return false;
    return m_ptrSwitchCodeMap->size() > 1u;
}

template <typename TDerivedContext>
template <typename TSwitchCodeContext>
inline std::vector<std::shared_ptr<TSwitchCodeContext>> CDefEntityContext<TDerivedContext>::GetSwitchCodeContexts(
    const std::string& rssScopedSwitchVar /*= std::string()*/) const
{
    static_assert(std::is_base_of_v<SSwitchCodeContext, TSwitchCodeContext>);
    if (!m_ptrSwitchCodeMap) return {};
    std::vector<std::shared_ptr<TSwitchCodeContext>> vecSwitchCodeContexts;
    for (const auto& rptrSwitchCodeContext : *m_ptrSwitchCodeMap)
    {
        if (!rssScopedSwitchVar.empty() && rssScopedSwitchVar != rptrSwitchCodeContext.second->ptrSwitchVar->ssScopedName) continue;
        vecSwitchCodeContexts.push_back(std::static_pointer_cast<TSwitchCodeContext>(rptrSwitchCodeContext.second));
    }
    return vecSwitchCodeContexts;
}

template <typename TDerivedContext>
inline std::list<uint32_t>::iterator CDefEntityContext<TDerivedContext>::CreateIteratorObject()
{
    return m_lstIterators.insert(m_lstIterators.end(), 0);
}

template <typename TDerivedContext>
inline void CDefEntityContext<TDerivedContext>::RemoveIteratorObject(std::list<uint32_t>::iterator itPos)
{
    m_lstIterators.erase(itPos);
}

template <typename TDefEntityContext>
inline CDefinitionGeneratorBase<TDefEntityContext>::CDefinitionGeneratorBase(sdv::IInterfaceAccess* pParser) : CGenContext(pParser)
{}

template <typename TDefEntityContext>
inline bool CDefinitionGeneratorBase<TDefEntityContext>::Generate()
{
    // Get target file information for directory and file creation.
    std::string ssTargetSubDir;
    std::string ssTargetFileEnding;
    GetTargetFileInfo(ssTargetSubDir, ssTargetFileEnding);

    // Create target directory if it doesn't exist. Since race conditions could exist due to parallel processing, do this
    // up to five times before reporting an error.
    std::filesystem::path pathTargetDir = GetOutputDir();
    if (!ssTargetSubDir.empty()) pathTargetDir /= ssTargetSubDir;
    for (size_t nCnt = 0; nCnt < 5; nCnt++)
    {
        if (!std::filesystem::exists(pathTargetDir))
            std::filesystem::create_directories(pathTargetDir);
        else
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    if (!std::filesystem::exists(GetOutputDir()))
        throw CCompileException("Cannot create output directory: ", pathTargetDir.generic_u8string());

    // Replace the extension by the new file ending;
    std::filesystem::path pathFile = pathTargetDir / GetSource().filename();
    pathFile.replace_extension("");
    pathFile += ssTargetFileEnding;

    if (g_log_control.GetVerbosityMode() != EVerbosityMode::report_all)
        std::cout << "Target file: " << pathFile.generic_u8string() << std::endl;

    // Open the file for writing
    std::ofstream streamDefFile;
    streamDefFile.open(pathFile);
    if (!streamDefFile.is_open()) throw CCompileException("Failed to open the target file: ", pathFile.generic_u8string());

    // Add file header
    streamDefFile << Header(pathFile, GetFileHeaderText());

    // Add safeguard
    streamDefFile << Safeguard(pathFile, true);

    // Include headers
    TDefEntityContext sStreamContext(*this, GetParser());
    StreamIncludeSection(sStreamContext);

    // Run through the entities and process the entities...
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(GetParser());
    if (!pDefinition) throw CCompileException("Internal error: the parser doesn't have a root definition.");
    sdv::idl::IEntityIterator* pIterator = pDefinition->GetChildren();
    if (!pIterator) throw CCompileException("Internal error: the parser doesn't support entity iteration.");
    ProcessEntities(sStreamContext, pIterator);

    // Stream the result into the file
    StreamIntoFile(sStreamContext, streamDefFile);

    // End of safeguard
    streamDefFile << Safeguard(pathFile, false);

    // Finalize the stream
    streamDefFile.close();

    return true;
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::StreamIntoFile(TDefEntityContext& /*rcontext*/,
    std::ofstream& /*rfstream*/) {}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::StreamIncludeSection(TDefEntityContext& /*rcontext*/)
{}

template <typename TDefEntityContext>
inline bool CDefinitionGeneratorBase<TDefEntityContext>::ProcessEntities(TDefEntityContext& rcontext,
    sdv::idl::IEntityIterator* pIterator)
{
    if (!pIterator) throw CCompileException("Internal error: processing entities without iterator.");
    bool bStreamDeclSuccess = true;

    // Do detection first...
    for (auto sIterator = rcontext.CreateIterator(); sIterator < pIterator->GetCount(); ++sIterator)
    {
        // Get the entity
        sdv::IInterfaceAccess* pEntity = pIterator->GetEntityByIndex(sIterator);
        if (!pEntity) throw CCompileException("Internal error: processing non-existent entity.");

        // Get the entity interfaces.
        const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
        if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
        const sdv::idl::IEntityContext* pContext = GetInterface<sdv::idl::IEntityContext>(pEntity);
        if (!pContext) throw CCompileException("Internal error: the entity doesn't expose context information.");
        const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);

        // Only process entities in the source code
        if (pContext->GetLocation() != sdv::idl::IEntityContext::ELocation::source)
            continue;

        // Check whether the entity has a name.
        if (pEntityInfo->GetName().empty())
            throw CCompileException("Internal error: the entity doesn't have a name.");

        if (pDeclaration) // Handle declarations
        {
            // Create a switch variable if not already available
            if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_switch_variable)
            {
                rcontext.AssignSwitchVarContext(GetOrCreateVarBasedSwitch(pEntity));

                // Detect the container for the switch variable and the union using the variable.
                sdv::IInterfaceAccess* pOldestContainer = pEntity;
                while (true)
                {
                    sdv::idl::IEntityInfo* pHighestContainerEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pOldestContainer);
                    sdv::IInterfaceAccess* pContainerEntity = pHighestContainerEntityInfo->GetParent();
                    const sdv::idl::IEntityInfo* pContainerEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pContainerEntity);
                    if (!pContainerEntityInfo) break;
                    pOldestContainer = pContainerEntity;
                }
                DetectUnionJointContainerForSwitchVar(rcontext, pEntity, pOldestContainer);
            }

            // If a declaration is a compound type and has declarations of unions that use switch case variables that are within the
            // scope of the definition stream context provided to this function, add the switch functions to the definition.
            DetectUnionContainerForProcessing(rcontext, pDeclaration->IsAnonymous() ? "" : pEntityInfo->GetName(), pEntity);
        }
    }

    // Do the processing...
    for (auto sIterator = rcontext.CreateIterator(); sIterator < pIterator->GetCount(); ++sIterator)
    {
        // Get the entity
        sdv::IInterfaceAccess* pEntity = pIterator->GetEntityByIndex(sIterator);
        if (!pEntity) throw CCompileException("Internal error: processing non-existent entity.");

        // Get the entity interfaces.
        const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
        if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
        const sdv::idl::IEntityContext* pContext = GetInterface<sdv::idl::IEntityContext>(pEntity);
        if (!pContext) throw CCompileException("Internal error: the entity doesn't expose context information.");
        const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);

        // Only process entities in the source code
        if (pContext->GetLocation() != sdv::idl::IEntityContext::ELocation::source)
            continue;

        // Check whether the entity has a name.
        if (pEntityInfo->GetName().empty())
            throw CCompileException("Internal error: the entity doesn't have a name.");

        // Process the entity
        const sdv::idl::IMetaEntity* pMeta = GetInterface<sdv::idl::IMetaEntity>(pEntity);
        if (pMeta) // Handle meta data
        {
            // Stream the meta entity.
            StreamMetaEntity(rcontext, pEntity);
        }
        else if (pDeclaration) // Handle declarations
        {
            // Skip streaming of declarations when one declaration was already non-streamable
            if (!bStreamDeclSuccess) continue;

            // Stream the declaration
            bStreamDeclSuccess &= StreamDeclaration(rcontext, pEntity);
        }
        else // Handle definitions
            StreamDefinition(rcontext, pEntity);
    }
    return bStreamDeclSuccess;
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::StreamMetaEntity(TDefEntityContext& /*rcontext*/,
    sdv::IInterfaceAccess* /*pEntity*/)
{}

template <typename TDefEntityContext>
inline bool CDefinitionGeneratorBase<TDefEntityContext>::StreamDeclaration(TDefEntityContext& /*rcontext*/,
    sdv::IInterfaceAccess* /*pEntity*/)
{
    return false;
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::StreamDefinition(TDefEntityContext& /*rcontext*/,
    sdv::IInterfaceAccess* /*pEntity*/, bool /*bInline = false*/, bool /*bAnonymousDecl = false*/)
{
}

template <typename TDefEntityContext>
inline std::shared_ptr<SSwitchVarContext> CDefinitionGeneratorBase<TDefEntityContext>::GetOrCreateVarBasedSwitch(
    sdv::IInterfaceAccess* pSwitchVarEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pSwitchVarEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");

    // Check for the existence of the variable
    auto itSwitchVar = m_mapSwitchFunc.find(pEntityInfo->GetScopedName());
    if (itSwitchVar != m_mapSwitchFunc.end()) return itSwitchVar->second;

    // Get variable information
    auto ptrSwitchVar = std::make_shared<SSwitchVarContext>();
    ptrSwitchVar->pVarEntity = pSwitchVarEntity;
    ptrSwitchVar->ssScopedName = pEntityInfo->GetScopedName();
    ptrSwitchVar->ssName = pEntityInfo->GetName();
    size_t nPos = pEntityInfo->GetScopedName().find_last_of(".:");
    std::string ssScope = pEntityInfo->GetScopedName().substr(nPos == std::string::npos ? 0 : nPos + 1);

    // Get the declaration type string
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pSwitchVarEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_switch_variable)
        throw CCompileException("Internal error: the entity to be used as switch variable is not declared as switch variable.");
    SCDeclInfo sDeclInfo = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), ssScope, true);
    ptrSwitchVar->ssType = sDeclInfo.ssDeclType;

    // Add the switch var context to the map
    m_mapSwitchFunc[pEntityInfo->GetScopedName()] = ptrSwitchVar;

    // Return the result
    return ptrSwitchVar;
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::DetectUnionContainerForProcessing(TDefEntityContext& rcontext,
    const std::string& rssMemberScope, sdv::IInterfaceAccess* pDeclEntity,
    const std::vector<SArrayIterationInfo>& rvecArrayIndices /*= {}*/)
{
    // Get the declaration interface from the entity
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pDeclEntity);
    if (!pDeclaration) return;

    // Get the declaration type
    sdv::idl::IDeclarationType* pDeclType = GetInterface<sdv::idl::IDeclarationType>(pDeclaration->GetDeclarationType());
    if (!pDeclType) return;

    // Get the declaration variable name.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pDeclEntity);
    if (!pEntityInfo) return;

    // Get the definition that belongs to this entity declaration (only entities with definitions are currently of interest).
    sdv::IInterfaceAccess* pDefTypeEntity = pDeclType->GetTypeDefinition();
    if (!pDefTypeEntity) return;
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pDefTypeEntity);
    if (!pDefinition) return;

    // Create the new member string
    std::string ssDeclMemberScope = rssMemberScope;

    // If the declaration is declared as an array, get the dimensions
    std::vector<SArrayIterationInfo> vecNewInicesInfo = rvecArrayIndices;
    if (pDeclaration->HasArray())
    {
        // Check whether iteration through each element is necessary (for each dimension of a multi-vector array).
        sdv::sequence<sdv::idl::SArrayDimension> seqArrayDimensions = pDeclaration->GetArrayDimensions();
        size_t nIndex = 0;
        for (const sdv::idl::SArrayDimension& rsDimension : seqArrayDimensions)
        {
            // Create an index variable name
            std::string ssIndexVarName = std::string("uiIndex_") + pEntityInfo->GetName();
            if (seqArrayDimensions.size() > 1) ssIndexVarName += std::to_string(nIndex++);
            vecNewInicesInfo.push_back(SArrayIterationInfo{ ssIndexVarName, rsDimension.ssExpression });
            ssDeclMemberScope += std::string("[") + ssIndexVarName + "]";
        }
    }

    // Detection in child declaration entities.
    auto fnDetectInChildren = [&](sdv::idl::IDefinitionEntity* pLocalDefinition)
    {
        // Get the child iterator
        sdv::idl::IEntityIterator* pChildIterator = pLocalDefinition->GetChildren();
        if (!pChildIterator) return;

        // Iterate through each child
        for (uint32_t uiIndex = 0; uiIndex < pChildIterator->GetCount(); uiIndex++)
        {
            sdv::IInterfaceAccess* pChildEntity = pChildIterator->GetEntityByIndex(uiIndex);
            const sdv::idl::IEntityInfo* pChildEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pChildEntity);
            const sdv::idl::IDeclarationEntity* pChildDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pChildEntity);
            if (!pChildEntity || !pChildEntityInfo || !pChildDeclaration) continue;
            std::string ssChildMemberScope = ssDeclMemberScope;
            if (!pChildDeclaration->IsAnonymous())
                ssChildMemberScope += std::string(".") + static_cast<std::string>(pChildEntityInfo->GetName());
            DetectUnionContainerForProcessing(rcontext, ssChildMemberScope, pChildEntity, vecNewInicesInfo);
        }
    };

    // First deal with inheritance - iterate through each base type
    sdv::idl::IEntityIterator* pInheritanceIterator = pDefinition->GetInheritance();
    if (pInheritanceIterator)
    {
        for (uint32_t uiIndex = 0; uiIndex < pInheritanceIterator->GetCount(); uiIndex++)
        {
            sdv::idl::IDefinitionEntity* pDefinitionBase =
                GetInterface<sdv::idl::IDefinitionEntity>(pInheritanceIterator->GetEntityByIndex(uiIndex));
            if (!pDefinitionBase) continue;
            fnDetectInChildren(pDefinitionBase);
        }
    }

    // Further action depends on the type
    switch (pDeclType->GetBaseType())
    {
    case sdv::idl::EDeclType::decltype_exception:
    case sdv::idl::EDeclType::decltype_struct:
        // Detect within the children
        fnDetectInChildren(pDefinition);
        return;
    case sdv::idl::EDeclType::decltype_union:
        // Union processing below...
        break;
    default:
        return; // Other types don't contain unions.
    }

    // Union detected... check for variable based union
    const sdv::idl::IUnionEntity* pUnion = GetInterface<sdv::idl::IUnionEntity>(pDefTypeEntity);
    if (!pUnion || pUnion->GetSwitchInterpretation() != sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable) return;
    sdv::u8string ssVarDeclName;
    sdv::IInterfaceAccess* pSwitchVar = nullptr;
    sdv::IInterfaceAccess* pContainer = nullptr;
    pUnion->GetSwitchVar(ssVarDeclName, pSwitchVar, pContainer);

    // Check the scope names for the context entity and the container of the switch. If identical, this is the context to add
    // union initialization for.
    const sdv::idl::IEntityInfo* pSwitchParentEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pContainer);
    const sdv::idl::IEntityInfo* pDefEntityInfo = rcontext.template GetDefEntity<sdv::idl::IEntityInfo>();
    if (!pSwitchParentEntityInfo || !pDefEntityInfo ||
        pSwitchParentEntityInfo->GetScopedName() != pDefEntityInfo->GetScopedName())
        return;

    // Stream union initialization functions
    ProcessUnionInContainerContext(rcontext, ssDeclMemberScope, ssVarDeclName, pDefTypeEntity, vecNewInicesInfo);
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::ProcessUnionInContainerContext(TDefEntityContext& /*rcontext*/, std::string /*rssMemberScopeUnionDecl*/,
    std::string /*rssMemberScopeSwitchVar*/, sdv::IInterfaceAccess* /*pUnionDef*/,
    const std::vector<SArrayIterationInfo>& /*rvecArrayIndices = std::vector<SArrayIterationInfo>()*/)
{}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::DetectUnionJointContainerForSwitchVar(TDefEntityContext& rcontext,
    sdv::IInterfaceAccess* pSwitchVarEntity, sdv::IInterfaceAccess* pEntity)
{
    // Check for valid definitions
    sdv::idl::IEntityInfo* pDefEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    sdv::idl::IEntityInfo* pCurrentDefEntityInfo = rcontext.template GetDefEntity<sdv::idl::IEntityInfo>();
    sdv::idl::IDefinitionEntity* m_pDefEntity = GetInterface<sdv::idl::IDefinitionEntity>(pEntity);
    sdv::idl::IDefinitionEntity* pCurrentDefEntity = rcontext.template GetDefEntity<sdv::idl::IDefinitionEntity>();
    if (!pDefEntityInfo || !pCurrentDefEntityInfo || !m_pDefEntity || !pCurrentDefEntity) return;

    // Check for the current scope
    std::string ssCurrentScopedDefName = pCurrentDefEntityInfo->GetScopedName();

    // Run through the inherited entities and do detection.
    sdv::idl::IEntityIterator* pInheritanceIterator = m_pDefEntity->GetInheritance();
    if (pInheritanceIterator)
    {
        for (uint32_t uiIndex = 0; uiIndex < pInheritanceIterator->GetCount(); uiIndex++)
            DetectUnionJointContainerForSwitchVar(rcontext, pSwitchVarEntity,
                pInheritanceIterator->GetEntityByIndex(uiIndex));
    }

    // Detect for a union
    sdv::idl::IUnionEntity* pUnion = GetInterface<sdv::idl::IUnionEntity>(pEntity);
    if (pUnion && pUnion->GetSwitchInterpretation() == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
    {
        sdv::u8string ssVarName;
        sdv::IInterfaceAccess* pVarEntity = nullptr;
        sdv::IInterfaceAccess* pContainerEntity = nullptr;
        pUnion->GetSwitchVar(ssVarName, pVarEntity, pContainerEntity);
        sdv::idl::IEntityInfo* pContainerEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pContainerEntity);
        sdv::idl::IEntityInfo* pVarEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pVarEntity);
        sdv::idl::IEntityInfo* pVarEntityParentInfo =
            GetInterface<sdv::idl::IEntityInfo>(pVarEntityInfo ? pVarEntityInfo->GetParent() : nullptr);
        if (pContainerEntityInfo && pVarEntityParentInfo && pVarEntityParentInfo->GetScopedName() == ssCurrentScopedDefName)
            ProcessUnionJointContainerForSwitchVar(rcontext, pSwitchVarEntity, pContainerEntity);
    }

    // Run through the child entities and do detection.
    sdv::idl::IEntityIterator* pChildIterator = m_pDefEntity->GetChildren();
    if (pChildIterator)
    {
        for (uint32_t uiIndex = 0; uiIndex < pChildIterator->GetCount(); uiIndex++)
            DetectUnionJointContainerForSwitchVar(rcontext, pSwitchVarEntity, pChildIterator->GetEntityByIndex(uiIndex));
    }
}

template <typename TDefEntityContext>
inline void CDefinitionGeneratorBase<TDefEntityContext>::ProcessUnionJointContainerForSwitchVar(
    TDefEntityContext& /*rcontext*/, sdv::IInterfaceAccess* /*pSwitchVarEntity*/,
    sdv::IInterfaceAccess* /*pContainerEntity*/)
{}


#endif // !defined DEFINITION_GENERATOR_BASE_INL