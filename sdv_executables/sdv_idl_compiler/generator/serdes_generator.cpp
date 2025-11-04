#include "serdes_generator.h"
#include "../logger.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <thread>
#include <chrono>

#include "../exception.h"

CSerdesContext::CSerdesContext(const CGenContext& rGenContext, sdv::IInterfaceAccess* pEntity) :
    CDefEntityContext<CSerdesContext>(rGenContext, pEntity)
{}

CSerdesContext::CSerdesContext(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity,
    const std::string& rssDeclName /*= std::string()*/) :
    CDefEntityContext<CSerdesContext>(rcontext, pEntity), m_ssMemberScope(rcontext.m_ssMemberScope),
    m_bSerDesNamespace(rcontext.m_bSerDesNamespace)
{
    if (!rssDeclName.empty())
    {
        if (!m_ssMemberScope.empty())
            m_ssMemberScope += ".";
        m_ssMemberScope += rssDeclName;
    }
}

CSerdesContext& CSerdesContext::operator<<(const CSerdesContext& rcontext)
{
    std::string ssSerDesCode = rcontext.GetSerDesCode();
    if (!ssSerDesCode.empty())
    {
        if (!m_sstreamSerDesCode.str().empty()) m_sstreamSerDesCode << std::endl;
        m_sstreamSerDesCode << ssSerDesCode;
    }

    return *this;
}

std::string CSerdesContext::ComposeMemberScope(const std::string& rssDeclName, bool bFullScope /*= false*/) const
{
    std::string ssScopedMemberName;
    if (bFullScope)
    {
        ssScopedMemberName = GetScope();
        if (!ssScopedMemberName.empty()) ssScopedMemberName += "::";
    }
    ssScopedMemberName += m_ssMemberScope;
    if (!m_ssMemberScope.empty()) ssScopedMemberName += ".";
    ssScopedMemberName += rssDeclName;
    return ssScopedMemberName;
}

void CSerdesContext::EnableSerDesNamespace()
{
    if (!m_bSerDesNamespace)
    {
        if (!m_sstreamSerDesCode.str().empty()) m_sstreamSerDesCode << std::endl;
        m_sstreamSerDesCode << R"code(// Serializer / deserializer namespace
namespace serdes
{)code";
        IncrIndent();
        m_bSerDesNamespace = true;
    }
}

void CSerdesContext::DisableSerDesNamespace()
{
    if (m_bSerDesNamespace)
    {
        m_sstreamSerDesCode << R"code(} // serdes namespace
)code";
        DecrIndent();
        m_bSerDesNamespace = false;
    }
}

std::stringstream& CSerdesContext::GetSerDesCodeStream()
{
    return m_sstreamSerDesCode;
}

std::string CSerdesContext::GetSerDesCode() const
{
    return m_sstreamSerDesCode.str();
}

std::stringstream& CSerdesContext::GetSizeFuncStream(EFuncStreamPart ePart /*= EFuncStreamPart::body*/)
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamSizeFuncHdr;
    case EFuncStreamPart::footer:
        return m_sstreamSizeFuncFtr;
    default:
        return m_sstreamSizeFunc;
    }
}

std::stringstream& CSerdesContext::GetSerFuncStream(EFuncStreamPart ePart /*= EFuncStreamPart::body*/)
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamSerFuncHdr;
    case EFuncStreamPart::footer:
        return m_sstreamSerFuncFtr;
    default:
        return m_sstreamSerFunc;
    }
}

std::stringstream& CSerdesContext::GetDesFuncStream(EFuncStreamPart ePart /*= EFuncStreamPart::body*/)
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamDesFuncHdr;
    case EFuncStreamPart::footer:
        return m_sstreamDesFuncFtr;
    default:
        return m_sstreamDesFunc;
    }
}

std::string CSerdesContext::GetSizeFuncCode(EFuncStreamPart ePart /*= EFuncStreamPart::body*/) const
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamSizeFuncHdr.str();
    case EFuncStreamPart::footer:
        return m_sstreamSizeFuncFtr.str();
    default:
        return m_sstreamSizeFunc.str();
    }
}

std::string CSerdesContext::GetSerFuncCode(EFuncStreamPart ePart /*= EFuncStreamPart::body*/) const
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamSerFuncHdr.str();
    case EFuncStreamPart::footer:
        return m_sstreamSerFuncFtr.str();
    default:
        return m_sstreamSerFunc.str();
    }
}

std::string CSerdesContext::GetDesFuncCode(EFuncStreamPart ePart /*= EFuncStreamPart::body*/) const
{
    switch (ePart)
    {
    case EFuncStreamPart::header:
        return m_sstreamDesFuncHdr.str();
    case EFuncStreamPart::footer:
        return m_sstreamDesFuncFtr.str();
    default:
        return m_sstreamDesFunc.str();
    }
}

void CSerdesContext::JoinFuncBodyStreams(const CSerdesContext& rcontext, bool bDoNotIncludeNewline /*= false*/)
{
    // Take over any serdes code
    std::string ssSerdesCode = rcontext.GetSerDesCode();
    if (!ssSerdesCode.empty() && !m_sstreamSerDesCode.str().empty()) m_sstreamSerDesCode << std::endl;
    m_sstreamSerDesCode << ssSerdesCode;

    // Check for body content...
    if (!rcontext.NeedsVariableStreaming()) return;

    // Insert a newline if there are already variables within
    if (!bDoNotIncludeNewline)
    {
        if (!m_sstreamSizeFunc.str().empty()) m_sstreamSizeFunc << std::endl;
        if (!m_sstreamSerFunc.str().empty()) m_sstreamSerFunc << std::endl;
        if (!m_sstreamDesFunc.str().empty()) m_sstreamDesFunc << std::endl;
    }

    // Stream the content
    m_sstreamSizeFunc << rcontext.GetSizeFuncCode();
    m_sstreamSerFunc << rcontext.GetSerFuncCode();
    m_sstreamDesFunc << rcontext.GetDesFuncCode();
}

void CSerdesContext::StreamAndClearSerFuncStreams(const std::string& rssClassBegin, const std::string& rssClassEnd,
    const CGenContext::CKeywordMap& rmapKeywords)
{
    if (!m_sstreamSerDesCode.str().empty()) m_sstreamSerDesCode << std::endl;
    m_sstreamSerDesCode << CGenContext::ReplaceKeywords(rssClassBegin, rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSizeFuncHdr.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSizeFunc.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSizeFuncFtr.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSerFuncHdr.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSerFunc.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamSerFuncFtr.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamDesFuncHdr.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamDesFunc.str(), rmapKeywords) <<
        CGenContext::ReplaceKeywords(m_sstreamDesFuncFtr.str(), rmapKeywords) <<
        rssClassEnd;
    m_sstreamSizeFuncHdr = std::stringstream{};
    m_sstreamSizeFuncFtr = std::stringstream{};
    m_sstreamSizeFunc = std::stringstream{};
    m_sstreamSerFuncHdr = std::stringstream{};
    m_sstreamSerFuncFtr = std::stringstream{};
    m_sstreamSerFunc = std::stringstream{};
    m_sstreamDesFuncHdr = std::stringstream{};
    m_sstreamDesFuncFtr = std::stringstream{};
    m_sstreamDesFunc = std::stringstream{};
}

bool CSerdesContext::NeedsVariableStreaming() const
{
    return m_sstreamSerFunc.str().empty() ? false : true;
}

CSerdesGenerator::CSerdesGenerator(sdv::IInterfaceAccess* pParser) : CDefinitionGeneratorBase(pParser)
{}

CSerdesGenerator::~CSerdesGenerator() {}

void CSerdesGenerator::GetTargetFileInfo(std::string& rssTargetSubDir, std::string& rssTargetFileEnding)
{
    rssTargetSubDir = "serdes";         // Target directory is "serdes".
    rssTargetFileEnding = "_serdes.h";  // Target is a header file with the same name as the source file.
}

std::string CSerdesGenerator::GetFileHeaderText() const
{
    return "This file contains the serialization/deserialization specialization functions.";
}

void CSerdesGenerator::StreamIntoFile(CSerdesContext& rcontext, std::ofstream& rfstream)
{
    // Finalize the serdes namespace
    rcontext.DisableSerDesNamespace();

    // Stream the context into the file.
    rfstream << rcontext.GetSerDesCode();
}

void CSerdesGenerator::StreamIncludeSection(CSerdesContext& rcontext)
{
    // Include headers
    std::filesystem::path pathFileDef = GetOutputDir() / GetSource().filename();
    pathFileDef.replace_extension("");
    pathFileDef += ".h";
    rcontext.GetSerDesCodeStream() << "// Includes..." << std::endl;
    rcontext.GetSerDesCodeStream() << "#include <cstdint>" << std::endl;
    rcontext.GetSerDesCodeStream() << "#include <support/serdes.h>" << std::endl;
    rcontext.GetSerDesCodeStream() << "#include <support/local_service_access.h>" << std::endl;
    rcontext.GetSerDesCodeStream() << "#include <interfaces/com.h>" << std::endl;
    rcontext.GetSerDesCodeStream() << "#include <interfaces/serdes/core_ps_serdes.h>" << std::endl;
    rcontext.GetSerDesCodeStream() << "#include \"../" << pathFileDef.filename().generic_u8string() << "\"" << std::endl;
    rcontext.GetSerDesCodeStream() << std::endl;
}

void CSerdesGenerator::StreamMetaEntity(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    const sdv::idl::IMetaEntity* pMeta = GetInterface<sdv::idl::IMetaEntity>(pEntity);
    if (!pMeta) return;

    std::string ssContent, ssContentLC;
    char cSeparator = '/';
    size_t nPos = std::string::npos;
    switch (pMeta->GetMetaType())
    {
    case sdv::idl::IMetaEntity::EType::include_global:
    case sdv::idl::IMetaEntity::EType::include_local:
        rcontext.DisableSerDesNamespace();
        ssContent = pMeta->GetContent();
        ssContentLC = ssContent;
        for (char& rc : ssContentLC) rc = static_cast<char>(std::tolower(rc));
        nPos = ssContentLC.find(".idl");
        if (nPos != std::string::npos)
            ssContent.replace(nPos, 4, "_serdes.h");
#ifdef _MSC_VER
        nPos = ssContent.find_last_of("/\\");
#else
        nPos = ssContent.find_last_of("/");
#endif
        // If not found, the file must lay next to the current file.
        // Otherwise, take the position after the slash and insert the serdes directory.

        if (nPos != std::string::npos)
        {
            cSeparator = ssContent[nPos];
            nPos++;
            ssContent.insert(nPos, std::string("serdes") + cSeparator);
        }
        rcontext.DisableIndent();
        rcontext.GetSerDesCodeStream() << "#include " << ssContent << std::endl;
        rcontext.EnableIndent();
        break;
    default:
        break;
    }
}

bool CSerdesGenerator::StreamDeclaration(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Ignore global declarations
    if (!rcontext.IsCompound()) return true;

    // All code is within serdes namespace.
    rcontext.EnableSerDesNamespace();

    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration) throw CCompileException("Internal error: the declaration entity doesn't expose declaration interface.");
    std::string ssFullScopedMemberName = rcontext.ComposeMemberScope(pEntityInfo->GetName(), true);
    std::string ssScopedMemberName = rcontext.ComposeMemberScope(pEntityInfo->GetName(), false);
    if (m_setHistory.find(ssFullScopedMemberName) != m_setHistory.end()) return true;   // Already streamed
    m_setHistory.insert(ssFullScopedMemberName);
    auto seqArrayDimensions = pDeclaration->GetArrayDimensions();

    CLog log("Stream declaration ", pEntityInfo->GetScopedName());

    // In case the declaration is a typedef declaration, call the specific streaming function (the typedef declaration is composed
    // in a different order).
    bool bInsideCase = false;
    switch (pEntityInfo->GetType())
    {
        case sdv::idl::EEntityType::type_typedef:
            //StreamTypedef(pEntity);
            // Nothing to do...
            return true;
        case sdv::idl::EEntityType::type_attribute:
            //StreamAttribute(pEntity);
            // TODO EVE
            return false;
        case sdv::idl::EEntityType::type_parameter:
            // Nothing to do...
            return true;
        case sdv::idl::EEntityType::type_operation:
        {
            // Iterate through the parameters (their types might be streamable)
            sdv::idl::IOperationEntity* pOperation = pEntity->GetInterface<sdv::idl::IOperationEntity>();
            if (!pOperation) throw CCompileException("Internal error: operation entity doesn't have an operation interface.");
            sdv::idl::IEntityIterator* pParameterIterator = pOperation->GetParameters();
            if (pParameterIterator)
                ProcessEntities(rcontext, pParameterIterator);

            // Nothing else to do...
            return true;
        }
        case sdv::idl::EEntityType::type_enum_entry:
            // Entries are not streamed
            return true;
        case sdv::idl::EEntityType::type_case_entry:
        {
            bInsideCase = true;
            const sdv::idl::ICaseEntity* pCase = pEntity->GetInterface<sdv::idl::ICaseEntity>();
            if (!pCase) throw CCompileException("Internal error: the case entity doesn't expose the case entity interface.");
            rcontext.DecrIndent();
            if (pCase->IsDefault())
            {
                rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "default:" << std::endl;
                rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "default:" << std::endl;
                rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "default:" << std::endl;
            }
            else
            {
                rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "case " << pCase->GetLabel() << ":" << std::endl;
                rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "case " << pCase->GetLabel() << ":" << std::endl;
                rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "case " << pCase->GetLabel() << ":" << std::endl;
            }
            rcontext.IncrIndent();
            break;  // Note... a variable declaration is following
        }
        case sdv::idl::EEntityType::type_switch_variable:
            // This entity is only streamable by the joint container of a union and this switch variable.
            {
                std::string ssSwitchVarParentName;
                const sdv::idl::IEntityInfo* pSwitchVarParent = GetInterface<sdv::idl::IEntityInfo>(pEntityInfo->GetParent());
                if (!pSwitchVarParent)
                    throw CCompileException("Internal error: switch variable declaration without parent container.");

                auto itInlineDef = m_mapInlineDef.find(pSwitchVarParent->GetScopedName());
                if (itInlineDef == m_mapInlineDef.end())
                    return false;  // Unknown
                if (std::find(itInlineDef->second.begin(), itInlineDef->second.end(), rcontext.GetScope()) ==
                    itInlineDef->second.end())
                    return false;   // Unknown

                auto vecSwitchCodeContexts = rcontext.GetSwitchCodeContexts(pEntityInfo->GetScopedName());
                std::string ssTempName = std::string("temp_") + QualifyName(ssScopedMemberName);
                bool bDoOnce = false;
                for (auto ptrSwitchCode : vecSwitchCodeContexts)
                {
                    if (!bDoOnce)
                        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << ptrSwitchCode->ptrSwitchVar->ssType <<
                            " " << ssTempName << "{};" << std::endl;
                    bDoOnce = true;
                    rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rDeserializer >> " << ssTempName << ";" <<
                        std::endl;
                    if (rcontext.HasMultipleSwitchCodeContexts())
                    {
                        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "sdv::ser_size(rVar.get_switch_" <<
                            QualifyName(ptrSwitchCode->ssSwitchVarName) << "(), rnSize);" << std::endl;
                        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << rVar.get_switch_" <<
                            QualifyName(ptrSwitchCode->ssSwitchVarName) << "();" << std::endl;
                        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rVar.switch_to_" <<
                            QualifyName(ptrSwitchCode->ssSwitchVarName) << "(" << ssTempName << ");" << std::endl;
                    }
                    else
                    {
                        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) <<
                            "sdv::ser_size(rVar.get_switch(), rnSize);" << std::endl;
                        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << rVar.get_switch();" <<
                            std::endl;
                        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rVar.switch_to(" << ssTempName <<
                            ");" << std::endl;
                    }
                }
            }
            return true;
            break;
        default:
            break;
    }

    sdv::IInterfaceAccess* pTypeObj = pDeclaration->GetDeclarationType();
    if (!pTypeObj) throw CCompileException("Internal error: the declaration misses a declaration type object.");
    sdv::idl::IDeclarationType* pType = pTypeObj->GetInterface<sdv::idl::IDeclarationType>();
    if (!pType) throw CCompileException("Internal error: the declaration entity doesn't expose declaration interface.");

    // Get any underlying type (could be a typedef).
    while (pType->GetValueType())
    {
        sdv::idl::IDeclarationType* pValueType = pType->GetValueType()->GetInterface<sdv::idl::IDeclarationType>();
        if (pValueType && pValueType->GetTypeDefinition())
            pType = pValueType;
        else
            break;
    }

    // Check whether the type definition was streamed as a separate entity. Then stream the definition as inline.
    // Or whether the type definition is unnamed, but the declaration is not anonymous; then explicitly stream the definition.
    sdv::IInterfaceAccess* pTypeDef = pType->GetTypeDefinition();
    const sdv::idl::IEntityInfo* pTypeDefEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pTypeDef);
    bool bStreamDefContent = false;
    if (pTypeDefEntityInfo)
    {
        std::string ssTypeScopedName = pTypeDefEntityInfo->GetScopedName();

        // When the definition is unnamed, it is not streamed yet. Explicitly stream the declaration.
        const sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pTypeDef);
        if (pDefinition && pDefinition->IsUnnamed())
            StreamDefinition(rcontext, pTypeDef, true, pDeclaration->IsAnonymous());

        // Get union information
        sdv::idl::IUnionEntity::ESwitchInterpret eSwitchInterpret = sdv::idl::IUnionEntity::ESwitchInterpret::switch_type;
        const sdv::idl::IUnionEntity* pUnion = GetInterface<sdv::idl::IUnionEntity>(pTypeDef);
        if (pUnion) eSwitchInterpret = pUnion->GetSwitchInterpretation();

        // Unions with variable based switch should be streamed within this body.
        if (eSwitchInterpret == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
            bStreamDefContent = true;
        else
        {
            // Check the list of non-streamable definitions. If the definition of the declaration is marked as non-streamable, try
            // to stream the content directly using the declaration as a scope.
            if (m_setNonStreamableDef.find(ssTypeScopedName) != m_setNonStreamableDef.end())
                bStreamDefContent = true;
        }
    }

    // Stream the non-const declarations into the function bodies
    if (pDeclaration->IsReadOnly()) return true;

    // In case of streaming a definition content with arrays, add the comments here (so it includes the array itteration).
    if (bStreamDefContent && !seqArrayDimensions.empty())
    {
        // Include a newline if some variables were streamed already
        if (rcontext.NeedsVariableStreaming())
        {
            rcontext.GetSizeFuncStream() << std::endl;
            rcontext.GetSerFuncStream() << std::endl;
            rcontext.GetDesFuncStream() << std::endl;
        }

        std::string ssRelativeScope = GetRelativeScopedName(pTypeDefEntityInfo->GetScopedName(), rcontext.GetScope());
        if (ssRelativeScope.empty()) ssRelativeScope = pEntityInfo->GetName();
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "// Calculating the size of " << ssRelativeScope <<
            std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "// Serializing members of " << ssRelativeScope <<
            std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "// Deserializing members of " << ssRelativeScope <<
            std::endl;
    }

    // Start array iteration if the union was allocated using a single or multi dimensional array
    size_t nIndex = 0;
    std::string ssScopedMemberNameWithArray = ssScopedMemberName;
    std::string ssMemberNameWithArray = pEntityInfo->GetName();
    for (const auto& rsArrayDimension : seqArrayDimensions)
    {
        std::string ssIndexName = std::string("uiIndex_") + pEntityInfo->GetName();
        if (seqArrayDimensions.size() > 1) ssIndexName += std::to_string(nIndex++);
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "for (uint32_t " << ssIndexName << " = 0; " <<
            ssIndexName << " < " << GetRelativeScopedName(rsArrayDimension.ssExpression, rcontext.GetScope()) << "; " <<
            ssIndexName << "++)" << std::endl << rcontext.GetIndent(false, true) << "{" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "for (uint32_t " << ssIndexName << " = 0; " <<
            ssIndexName << " < " << GetRelativeScopedName(rsArrayDimension.ssExpression, rcontext.GetScope()) << "; " <<
            ssIndexName << "++)" << std::endl << rcontext.GetIndent(false, true) << "{" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "for (uint32_t " << ssIndexName << " = 0; " <<
            ssIndexName << " < " << GetRelativeScopedName(rsArrayDimension.ssExpression, rcontext.GetScope()) << "; " <<
            ssIndexName << "++)" << std::endl << rcontext.GetIndent(false, true) << "{" << std::endl;
        ssScopedMemberNameWithArray += "[" + ssIndexName + "]";
        ssMemberNameWithArray += "[" + ssIndexName + "]";
        rcontext.IncrIndent();
    }

    // Stream type content or just the variables directly.
    if (bStreamDefContent)
    {
        CSerdesContext sMemberContext(rcontext, rcontext.GetDefEntity(),
            (pDeclaration->IsAnonymous() ? std::string() : ssMemberNameWithArray));
        if (!StreamDefinitionContent(sMemberContext, pTypeDef, !seqArrayDimensions.empty()))
            return false;
        rcontext.JoinFuncBodyStreams(sMemberContext, !seqArrayDimensions.empty());
    } else
    {
        // Add the streaming of the variables to the function body - if the type is unknown, don't stream (this is valid when the
        // variable represents a case statement, but doesn't contain an assignment).
        if (pType->GetBaseType() != sdv::idl::EDeclType::decltype_unknown)
        {
            rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "sdv::ser_size(rVar." <<
                ssScopedMemberNameWithArray << ", rnSize);" << std::endl;
            rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << rVar." <<
                ssScopedMemberNameWithArray << ";" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rDeserializer >> rVar." <<
                ssScopedMemberNameWithArray << ";" << std::endl;
        }
    }

    // Finalize the case...
    if (bInsideCase)
    {
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "break;" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "break;" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "break;" << std::endl;
    }

    // Stop array iteration if the allocation is using a single or multi dimensional array
    for ([[maybe_unused]] const auto& rsArrayDimension : seqArrayDimensions)
    {
        rcontext.DecrIndent();
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
    }

    return true;
}

void CSerdesGenerator::StreamDefinition(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInline /*= false*/,
    bool bAnonymousDecl /*= false*/)
{
    // All code is within serdes namespace.
    rcontext.EnableSerDesNamespace();

    CSerdesContext sDefStreamContext(rcontext, pEntity);

    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    sdv::idl::IDefinitionEntity* pDefinition = pEntity->GetInterface<sdv::idl::IDefinitionEntity>();
    if (!pDefinition)
        throw CCompileException("Internal error: the definition entity doesn't expose definition interface.");

    const sdv::idl::IEntityContext* pContext = GetInterface<sdv::idl::IEntityContext>(pEntity);
    if (!pContext) throw CCompileException("Internal error: the entity doesn't expose context information.");

    // Only process entities in the source code
    if (pContext->GetLocation() != sdv::idl::IEntityContext::ELocation::source)
        return;

    // When unnamed, only stream when the definition was inline
    if (pDefinition->IsUnnamed() && !bInline) return;

    // Forward declaration
    if (pEntityInfo->ForwardDeclaration()) return;

    // Do not stream local interfaces
    const sdv::idl::IInterfaceEntity* pInterface = GetInterface<sdv::idl::IInterfaceEntity>(pEntity);
    if (pInterface && pInterface->IsLocal()) return;

    // If the definitions were specialized already, do not specialize them once more.
    // Exception to the rule: the module is allowed to be defined multiple times... extending the module every time.
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_module &&
        m_setHistory.find(pEntityInfo->GetScopedName()) != m_setHistory.end()) return;

    CLog log("Stream definition ", pEntityInfo->GetScopedName());
    if (pDefinition->IsUnnamed())
        log << "Definition is unnamed" << std::endl;
    if (bInline)
        log << "Definition is followed by an inline declaration" << std::endl;
    if (bAnonymousDecl)
        log << "Definition is part of an anonymous declaration" << std::endl;

    // Add the definition to the history
    m_setHistory.insert(pEntityInfo->GetScopedName());

    // Unions with variable switched union entities should not get their own definition
    // For type based switch, process the union in this container context.
    const sdv::idl::IUnionEntity* pUnion = pEntity->GetInterface<sdv::idl::IUnionEntity>();
    if (pUnion && pUnion->GetSwitchInterpretation() == sdv::idl::IUnionEntity::ESwitchInterpret::switch_type)
            ProcessUnionInContainerContext(sDefStreamContext, "", "", pEntity);

    // Stream function prototypes (for all definitions except interfaces and modules)
    // Interfaces will be streamed separately below.
    std::stringstream sstreamClassBegin, sstreamClassEnd;
    CKeywordMap mapKeywords = {
        {"name", pEntityInfo->GetName()},
        {"scoped_name", pEntityInfo->GetScopedName()}
    };
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_module &&
        pEntityInfo->GetType() != sdv::idl::EEntityType::type_interface)
    {
        sDefStreamContext.IncrIndent();
        sstreamClassBegin << rcontext.GetIndent(false) << R"code(/**
     * @brief Specialization of serializer/deserializer class for %scoped_name%.
     */
    template <>
    class CSerdes<%scoped_name%>
    {
    public:)code";
        sDefStreamContext.GetSizeFuncStream(CSerdesContext::EFuncStreamPart::header) << R"code(
        /**
         * @brief Calculate the size of the value in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] rVar Reference to the variable.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize(%maybe_unused_attr%const %scoped_name%& rVar, %maybe_unused_attr%size_t& rnSize)
        {

)code";
        sDefStreamContext.GetSerFuncStream(CSerdesContext::EFuncStreamPart::header) << R"code(
        /**
         * @brief Stream the variable into the serializer.
         * @tparam eTargetEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] rVar Reference to the variable.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, %maybe_unused_attr%const %scoped_name%& rVar)
        {
)code";
        sDefStreamContext.GetDesFuncStream(CSerdesContext::EFuncStreamPart::header) << R"code(
        /**
         * @brief Stream the variable from the deserializer.
         * @tparam eSourceEndianess The source endianness determines whether to swap the bytes after retrieving them from the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rVar Reference to the variable to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, %maybe_unused_attr%%scoped_name%& rVar)
        {
)code";
        sstreamClassEnd << R"code(    };
)code";
    }

    // Check for exceptions; they need an extra ID
    if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_exception)
    {
        // Serialize the exception ID.
        sDefStreamContext.GetSerFuncStream(CSerdesContext::EFuncStreamPart::header) << sDefStreamContext.GetIndent(false, true) <<
            "rSerializer << sdv::GetExceptionId<" << pEntityInfo->GetScopedName() << ">();" << std::endl;

        // Deserialize the exception ID and check if the ID corresponds to the exception.
        sDefStreamContext.GetDesFuncStream(CSerdesContext::EFuncStreamPart::header) << sDefStreamContext.GetIndent(false, true) <<
            "sdv::exception_id idLocal = 0ull;" << std::endl;
        sDefStreamContext.GetDesFuncStream(CSerdesContext::EFuncStreamPart::header) << sDefStreamContext.GetIndent(false, true) <<
            "rDeserializer >> idLocal;" << std::endl;
        sDefStreamContext.GetDesFuncStream(CSerdesContext::EFuncStreamPart::header) << sDefStreamContext.GetIndent(false, true) <<
            "if (idLocal != sdv::GetExceptionId<" << pEntityInfo->GetScopedName() << ">()) throw sdv::ps::XMarshallIntegrity{};" <<
            std::endl;
    }

    // Check for inheritance (definitions only)
    sdv::idl::IEntityIterator* pInheritanceIterator = pDefinition->GetInheritance();
    if (pInheritanceIterator && pInheritanceIterator->GetCount())
    {
        // Stream the functions for this base type first (so they are known).
        for (uint32_t uiIndex = 0; uiIndex < pInheritanceIterator->GetCount(); uiIndex++)
        {
            // Stream the definition first
            sdv::IInterfaceAccess* pInheritedDef = pInheritanceIterator->GetEntityByIndex(uiIndex);
            const sdv::idl::IEntityInfo* pInheritedEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pInheritedDef);
            if (!pInheritedEntityInfo) continue;
            StreamDefinition(rcontext, pInheritedDef);

            // Stream the declarations here...
            // Check the list of non-streamable definitions. If the definition of the variable is marked as non-streamable, try
            // to stream the content directly using the declaration as a scope.
            if (m_setNonStreamableDef.find(pInheritedEntityInfo->GetScopedName()) != m_setNonStreamableDef.end())
            {
                if (!StreamDefinitionContent(rcontext, pInheritedDef))
                {
                    m_setNonStreamableDef.insert(pEntityInfo->GetScopedName());
                    log << "Cannot stream " << pEntityInfo->GetScopedName() << " in this context... " << std::endl;
                    return;
                }
            }

        }
    }

    // Stream the content
    if (!StreamDefinitionContent(sDefStreamContext, pEntity))
    {
        m_setNonStreamableDef.insert(sDefStreamContext.GetScope());
        log << "Cannot stream " << sDefStreamContext.GetScope() << " in this context... " << std::endl;
        return;
    }

    // Finalize
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_module &&
        pEntityInfo->GetType() != sdv::idl::EEntityType::type_interface)
    {
        sDefStreamContext.GetSizeFuncStream(CSerdesContext::EFuncStreamPart::footer) << R"code(        }
)code";
        sDefStreamContext.GetSerFuncStream(CSerdesContext::EFuncStreamPart::footer) << R"code(            return rSerializer;
        }
)code";
        sDefStreamContext.GetDesFuncStream(CSerdesContext::EFuncStreamPart::footer) << R"code(            return rDeserializer;
        }
)code";

        // In case there are declarations, insert the variable 'rVar' in the function declaration
        mapKeywords.insert(std::make_pair("maybe_unused_attr", sDefStreamContext.NeedsVariableStreaming() ? "" : "[[maybe_unused]] "));

        // Stream the function content
        sDefStreamContext.StreamAndClearSerFuncStreams(sstreamClassBegin.str(), sstreamClassEnd.str(), mapKeywords);

        sDefStreamContext.DecrIndent();
    }

    // Deal with interfaces
    if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_interface)
        StreamInterface(rcontext, pEntity);

    // Store the definition entity in the history to prevent double streaming.
    m_setHistory.insert(sDefStreamContext.GetScope());

    // Join the content of definition context into this context.
    rcontext << sDefStreamContext;
}

bool CSerdesGenerator::StreamDefinitionContent(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bSuppressComments /*= false*/)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pEntity);
    if (!pDefinition) throw CCompileException("Internal error: the definition entity doesn't expose definition interface.");

    CLog log;
    log << "Streaming definition content from " << pEntityInfo->GetScopedName() << std::endl;

    // Flag to indicate that an extra level of processing is inserted.
    bool bNested = false;

    // Add comments for serialization
    bool bCommented = bSuppressComments;
    auto fnCommentSerDes = [&]()
    {
        if (bCommented) return;

        // Include a newline if some variables were streamed already
        if (rcontext.NeedsVariableStreaming())
        {
            rcontext.GetSizeFuncStream() << std::endl;
            rcontext.GetSerFuncStream() << std::endl;
            rcontext.GetDesFuncStream() << std::endl;
        }

        std::string ssRelativeScope = GetRelativeScopedName(pEntityInfo->GetScopedName(), rcontext.GetScope());
        if (ssRelativeScope.empty()) ssRelativeScope = pEntityInfo->GetName();
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "// Calculating the size of " << ssRelativeScope <<
            std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "// Serializing members of " << ssRelativeScope <<
            std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "// Deserializing members of " << ssRelativeScope <<
            std::endl;
        bCommented = true;
    };

    const sdv::idl::IUnionEntity* pUnion = GetInterface<sdv::idl::IUnionEntity>(pEntity);
    if (pUnion)
    {
        std::string ssSwitchVar, ssSwitchType, ssSwitchVarScopedName;
        if (pUnion->GetSwitchInterpretation() == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
        {
            sdv::u8string ssSwitchStr;
            sdv::IInterfaceAccess* pSwitchVar = nullptr;
            sdv::IInterfaceAccess* pContainer = nullptr;
            pUnion->GetSwitchVar(ssSwitchStr, pSwitchVar, pContainer);

            // if the container needed for streaming the union is not identical to the definition of the stream context, then
            // cancel the this stream context.
            if (pContainer != rcontext.GetDefEntity())
                return false;

            auto ptrSwitchVarContext = GetOrCreateVarBasedSwitch(pSwitchVar);
            if (ptrSwitchVarContext)
            {
                ssSwitchVar = ptrSwitchVarContext->ssName;
                ssSwitchVarScopedName = ptrSwitchVarContext->ssScopedName;
            }
        }
        else
        {
            ssSwitchVar = "switch_value";
            ssSwitchVarScopedName = rcontext.GetScope();
            if (!ssSwitchVarScopedName.empty()) ssSwitchVarScopedName += ".";
            ssSwitchVarScopedName += ssSwitchVar;
        }

        // Get the switch type information
        sdv::idl::EDeclType eSwitchDeclType;
        sdv::IInterfaceAccess* pSwitchType = nullptr;
        pUnion->GetSwitchType(eSwitchDeclType, pSwitchType);
        if (pSwitchType)
        {
            const sdv::idl::IEntityInfo* pSwitchEntityInfo = pSwitchType->GetInterface<sdv::idl::IEntityInfo>();
            if (pSwitchEntityInfo) ssSwitchType = pSwitchEntityInfo->GetName();
        }
        else
            ssSwitchType = MapDeclType2CType(eSwitchDeclType);

        // Stream the switch case.
        fnCommentSerDes();
        std::string ssGetSwitchFunc = "rVar.get_switch" +
            (rcontext.HasMultipleSwitchCodeContexts() ? std::string("_") + QualifyName(ssSwitchVarScopedName) : "") +
            "()";
        std::string ssTempSwitchVar = "temp_" + QualifyName(ssSwitchVarScopedName);
        std::string ssSwitchToFunc = "rVar.switch_to" +
            (rcontext.HasMultipleSwitchCodeContexts() ? std::string("_") + QualifyName(ssSwitchVarScopedName) : "") +
            "(" + ssTempSwitchVar + ")";

        // In case the switch variable hasn't been streamed yet, stream the switch.
        if (m_setHistory.find(ssSwitchVarScopedName) == m_setHistory.end())
        {
            rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "sdv::ser_size(" << ssGetSwitchFunc << ", rnSize);" <<
                std::endl;
            rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << " << ssGetSwitchFunc << ";" <<
                std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << ssSwitchType << " " << ssTempSwitchVar << "{};" <<
                std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rDeserializer >> " << ssTempSwitchVar << ";" <<
                std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << ssSwitchToFunc << ";" << std::endl;
            m_setHistory.insert(ssSwitchVarScopedName);
        }

        // Stream the union
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "switch (" << ssGetSwitchFunc << ")" << std::endl;
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "{" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "switch (" << ssGetSwitchFunc << ")" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "{" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "switch (" << ssGetSwitchFunc << ")" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "{" << std::endl;
        rcontext.IncrIndent();
        bNested = true;
    }

    const sdv::idl::IEnumEntity* pEnum = GetInterface<sdv::idl::IEnumEntity>(pEntity);
    if (pEnum)
    {
        // Get the enum base type
        sdv::idl::EDeclType eBaseType = sdv::idl::EDeclType::decltype_unknown;
        sdv::IInterfaceAccess* pBaseType = nullptr;
        pEnum->GetBaseType(eBaseType, pBaseType);

        // Cast and stream the base type value.
        if (pBaseType)
        {
            // Stream the functions for this base type first (so they are known).
            StreamDefinition(rcontext, pBaseType);

            const sdv::idl::IEntityInfo* pBaseEntity = GetInterface<sdv::idl::IEntityInfo>(pBaseType);
            if (!pBaseEntity) throw CCompileException("Internal error: enum base type is not an entity.");
            fnCommentSerDes();
            rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "sdv::ser_size(static_cast<" <<
                pBaseEntity->GetScopedName() << ">(rVar), rnSize);" << std::endl;
            rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << static_cast<" <<
                pBaseEntity->GetScopedName() << ">(rVar);" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << pBaseEntity->GetScopedName() << " base;" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rDeserializer >> base;" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rVar = static_cast<" <<
                pEntityInfo->GetScopedName() << ">(base);" << std::endl;
        }
        else
        {
            fnCommentSerDes();
            rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "sdv::ser_size(static_cast<" <<
                MapDeclType2CType(eBaseType) << ">(rVar), rnSize);" << std::endl;
            rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "rSerializer << static_cast<" <<
                MapDeclType2CType(eBaseType) << ">(rVar);" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << MapDeclType2CType(eBaseType) << " base = 0;" <<
                std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rDeserializer >> base;" << std::endl;
            rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "rVar = static_cast<" <<
                pEntityInfo->GetScopedName() << ">(base);" << std::endl;
        }
        return true;
    }

    // Does the entity have children?
    sdv::idl::IEntityIterator* pChildIterator = pDefinition->GetChildren();
    if (pChildIterator)
    {
        CSerdesContext contextChildren(rcontext, rcontext.GetDefEntity());
        if (!ProcessEntities(contextChildren, pChildIterator))
            return false;
        if (contextChildren.NeedsVariableStreaming())
        {
            // The StreamDefinitionContext is iterative. Check for a strarting comment (following whitespace). If there is none,
            // then add a comment. Otherwise not.
            std::string ssSerFuncStream = contextChildren.GetSerFuncCode();
            size_t nStartOfSerStream = ssSerFuncStream.find_first_not_of("\t \r\n\v");
            if (ssSerFuncStream.size() > nStartOfSerStream && ssSerFuncStream[nStartOfSerStream] != '/')
                fnCommentSerDes();

            // Join the streams together
            rcontext.JoinFuncBodyStreams(contextChildren, true);
        } else
            rcontext.GetSerDesCodeStream() << contextChildren.GetSerDesCode();
    }

    // If compound flag is set to true, then close the compound block.
    if (bNested)
    {
        rcontext.DecrIndent();
        rcontext.GetSizeFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
        rcontext.GetSerFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
        rcontext.GetDesFuncStream() << rcontext.GetIndent(false, true) << "}" << std::endl;
    }

    return true;
}

void CSerdesGenerator::StreamInterface(CSerdesContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    sdv::idl::IDefinitionEntity* pDefinition = pEntity->GetInterface<sdv::idl::IDefinitionEntity>();
    if (!pDefinition) throw CCompileException("Internal error: the definition entity doesn't expose definition interface.");
    const sdv::idl::IInterfaceEntity* pInterface = pEntity->GetInterface<sdv::idl::IInterfaceEntity>();
    if (!pInterface) throw CCompileException("Internal error: the interface entity doesn't expose entity interface.");

    // Does the entity have children?
    sdv::idl::IEntityIterator* pChildIterator = pDefinition->GetChildren();
    if (pChildIterator)
    {
        for (uint32_t uiIndex = 0; uiIndex < pChildIterator->GetCount(); uiIndex++)
        {
            auto pChild = pChildIterator->GetEntityByIndex(uiIndex);
            const sdv::idl::IDefinitionEntity* pChildDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pChild);
            if (pChildDefinition)
                StreamDefinition(rcontext, pChild);
        }

        ProcessEntities(rcontext, pChildIterator);
    }

    // Stream interface serialization functions
    CKeywordMap mapKeywords = {
        {"name", pEntityInfo->GetName()},
        {"scoped_name", pEntityInfo->GetScopedName()}
    };
    std::stringstream sstreamClassBegin, sstreamClassEnd;
    sstreamClassBegin << rcontext.GetIndent(false) << R"code(/**
     * @brief Specialization of serializer/deserializer class for %scoped_name%.
     */
    template <>
    class CSerdes<%scoped_name%*>
    {
    public:)code";
    rcontext.GetSerFuncStream() << R"code(
        /**
         * @brief Calculate the size of the interface identification in serialized form.
         * @remarks Dependable on the size value, padding is added to align data.
         * @param[in] pInterface Pointer to the interface.
         * @param[in, out] rnSize Reference to the variable containing the current size and increased by the size of the value.
         */
        static void CalcSize([[maybe_unused]] %scoped_name%* pInterface, size_t& rnSize)
        {
            sdv::ser_size(sdv::interface_id(), rnSize);
            sdv::ser_size(sdv::ps::TMarshallID(), rnSize);
        }
)code";
    rcontext.GetSerFuncStream() << R"code(
        /**
         * @brief Stream the interface into the serializer.
         * @tparam eTargetEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rSerializer Reference to the serializer.
         * @param[in] pInterface Pointer to the interface.
         * @return Reference to the serializer.
         */
        template <sdv::EEndian eTargetEndianess, typename TCRC>
        static sdv::serializer<eTargetEndianess, TCRC>& Serialize(sdv::serializer<eTargetEndianess, TCRC>& rSerializer, %scoped_name%* pInterface)
        {
            // Get interface to the component isolation service.
            auto ptrComControl = sdv::core::GetObject("CommunicationControl");
            if (!ptrComControl) throw sdv::ps::XMarshallNotInitialized{};
            sdv::ps::IMarshallAccess* pMarshallAccess = ptrComControl.GetInterface<sdv::ps::IMarshallAccess>();
            if (!pMarshallAccess) throw sdv::ps::XMarshallNotInitialized{};

            // Serialize the interface ID first
            rSerializer << sdv::GetInterfaceId<%scoped_name%>();

            // Create and serialize a stub object for the interface.
            sdv::ps::TMarshallID tStubID = pMarshallAccess->GetStub(pInterface);
            rSerializer << tStubID;

            return rSerializer;
        }
)code";
rcontext.GetDesFuncStream() << R"code(
        /**
         * @brief Stream the variable from the deserializer.
         * @tparam eTargetEndianess The target endianness determines whether to swap the bytes before storing them into the buffer.
         * @tparam TCRC The CRC type to use for the checksum calculation.
         * @param[in] rDeserializer Reference to the deserializer.
         * @param[out] rpInterface Reference to the interface pointer to be filled.
         * @return Reference to the deserializer.
         */
        template <sdv::EEndian eSourceEndianess, typename TCRC>
        static sdv::deserializer<eSourceEndianess, TCRC>& Deserialize(sdv::deserializer<eSourceEndianess, TCRC>& rDeserializer, %scoped_name%*& rpInterface)
        {
            // Get interface to the component isolation service.
            auto ptrComControl = sdv::core::GetObject("CommunicationControl");
            if (!ptrComControl) throw sdv::ps::XMarshallNotInitialized{};
            sdv::ps::IMarshallAccess* pMarshallAccess = ptrComControl.GetInterface<sdv::ps::IMarshallAccess>();
            if (!pMarshallAccess) throw sdv::ps::XMarshallNotInitialized{};

            // Get the interface ID
            sdv::interface_id id = 0;
            rDeserializer >> id;

            // Get the stub ID
            sdv::ps::TMarshallID tStubID{};
            rDeserializer >> tStubID;

            // Create the proxy
            rpInterface = pMarshallAccess->GetProxy(tStubID, id).get<%scoped_name%>();

            return rDeserializer;
        }
)code";
    sstreamClassEnd << R"code(    };
)code";

    // Stream the function content
    rcontext.StreamAndClearSerFuncStreams(sstreamClassBegin.str(), sstreamClassEnd.str(), mapKeywords);
}

void CSerdesGenerator::ProcessUnionInContainerContext(CSerdesContext& rcontext, std::string rssMemberScopeUnionDecl,
    std::string rssMemberScopeSwitchVar, sdv::IInterfaceAccess* pUnionDef,
    const std::vector<SArrayIterationInfo>& rvecArrayIndices /*= std::vector<SArrayIterationInfo>()*/)
{
    // Fill in constructor and destructor code
    if (!pUnionDef) return;
    const sdv::idl::IEntityInfo* pEntityInfo = pUnionDef->GetInterface<sdv::idl::IEntityInfo>();
    if (!pEntityInfo) return;
    const sdv::idl::IUnionEntity* pUnionInfo = pUnionDef->GetInterface<sdv::idl::IUnionEntity>();
    if (!pUnionInfo) return;
    sdv::idl::IDefinitionEntity* pDefinition = pUnionDef->GetInterface<sdv::idl::IDefinitionEntity>();
    if (!pDefinition) return;
    const sdv::idl::IEntityIterator* pChildrenIterator = pDefinition->GetChildren();
    if (!pChildrenIterator || !pChildrenIterator->GetCount()) return;

    // If a member variable was provided along, create a prefix from the member variable.
    //std::string ssVarPrefix;
    std::string ssSwitchVarName;
    //if (!rssMemberScopeUnionDecl.empty())
    //    ssVarPrefix += rssMemberScopeUnionDecl + ".";
    if (!rssMemberScopeSwitchVar.empty())
        ssSwitchVarName = rssMemberScopeSwitchVar;
    else
        ssSwitchVarName = pEntityInfo->GetName();

    CLog log;
    log << "Detecting union construct for union " << pEntityInfo->GetScopedName() << std::endl;
    log << "Context structure " << rssMemberScopeUnionDecl << std::endl;
    log << "Switch variable " << rssMemberScopeSwitchVar << std::endl;

    // Get the switch type information
    std::shared_ptr<SSwitchVarContext> ptrSwitchVarContext;
    if (pUnionInfo->GetSwitchInterpretation() == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
    {
        sdv::u8string ssSwitchVarStr;
        sdv::IInterfaceAccess* pSwitchVar = nullptr;
        sdv::IInterfaceAccess* pContainer = nullptr;
        pUnionInfo->GetSwitchVar(ssSwitchVarStr, pSwitchVar, pContainer);
        if (!pSwitchVar) return;
        ptrSwitchVarContext = GetOrCreateVarBasedSwitch(pSwitchVar);
        if (!ptrSwitchVarContext) return;
    }
    else
    {
        // Create an additional local switch var for the switch_value
        sdv::idl::EDeclType eSwitchDeclType = sdv::idl::EDeclType::decltype_unknown;
        sdv::IInterfaceAccess* pSwitchType = nullptr;
        pUnionInfo->GetSwitchType(eSwitchDeclType, pSwitchType);
        ptrSwitchVarContext = std::make_shared<SSwitchVarContext>();
        ptrSwitchVarContext->ssName = "switch_value";
        ptrSwitchVarContext->ssScopedName = pEntityInfo->GetScopedName() + "::switch_value";
        ssSwitchVarName = "switch_value";
        if (pSwitchType)
        {
            const sdv::idl::IEntityInfo* pSwitchEntityInfo = pSwitchType->GetInterface<sdv::idl::IEntityInfo>();
            if (!pSwitchEntityInfo) return;
            ptrSwitchVarContext->ssType = pSwitchEntityInfo->GetName();
        }
        else
            ptrSwitchVarContext->ssType = MapDeclType2CType(eSwitchDeclType);
    }

    // Create a code context for this entity and during creation, it will be added it to the current definition context
    /*auto ptrSwitchCodeContext =*/ rcontext.GetOrCreateSwitchCodeContext<>(ssSwitchVarName, ptrSwitchVarContext, rvecArrayIndices);

    // Store this union in the switch var context
    ptrSwitchVarContext->vecUnionDecl.push_back(pEntityInfo->GetName());
}

void CSerdesGenerator::ProcessUnionJointContainerForSwitchVar(CSerdesContext& rcontext, sdv::IInterfaceAccess* pSwitchVarEntity,
    sdv::IInterfaceAccess* pContainerEntity)
{
    // Check whether the container entity is identical to the definition entity of the context. If so, the entity can be streamed
    // directly. Otherwise, add a redirection entry into the redirection map and mark the context as non-streamable...
    const sdv::idl::IEntityInfo* pSwitchVarEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pSwitchVarEntity);
    if (!pSwitchVarEntityInfo) return;
    const sdv::idl::IEntityInfo* pContainerEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pContainerEntity);
    if (!pContainerEntityInfo) return;
    std::string ssSwitchVarContainer = rcontext.GetScope();
    std::string ssStreamContainer = pContainerEntityInfo->GetScopedName();

    CLog log;
    log << "Detect switch var streaming for " << pSwitchVarEntityInfo->GetName() << std::endl;
    log << "Target container " << ssStreamContainer << std::endl;

    // Check for an entry in the inline definition map
    auto itInlineDef = m_mapInlineDef.find(ssSwitchVarContainer);
    if (itInlineDef == m_mapInlineDef.end())
        itInlineDef = m_mapInlineDef.emplace(ssSwitchVarContainer, std::list<std::string>{}).first;
    if (std::find(itInlineDef->second.begin(), itInlineDef->second.end(), ssStreamContainer) == itInlineDef->second.end())
        itInlineDef->second.push_back(ssStreamContainer);
}
