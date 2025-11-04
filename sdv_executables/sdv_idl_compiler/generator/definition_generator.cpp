#include "definition_generator.h"
#include "../logger.h"
#include "../exception.h"
#include <cassert>
#include <cctype>
#include <thread>
#include <chrono>

CDefinitionContext::CDefinitionContext(const CGenContext& rGenContext, sdv::IInterfaceAccess* pEntity) :
    CDefEntityContext<CDefinitionContext>(rGenContext, pEntity), m_bPreface(true)
{}

CDefinitionContext::CDefinitionContext(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity) :
    CDefEntityContext<CDefinitionContext>(rcontext, pEntity)
{}

CDefinitionContext& CDefinitionContext::operator<<(const CDefinitionContext& rcontext)
{
    m_sstreamDefCode << rcontext.GetDefinitionCode();
    return *this;
}

void CDefinitionContext::SetDefAccessPublic()
{
    if (!m_bCurrentDefAccessPublic)
    {
        DecrIndent();
        m_sstreamDefCode << GetIndent() << "public:" << std::endl;
        IncrIndent();
        m_bCurrentDefAccessPublic = true;
    }
}

void CDefinitionContext::SetDefAccessPrivate()
{
    if (m_bCurrentDefAccessPublic)
    {
        DecrIndent();
        m_sstreamDefCode << GetIndent() << "private:" << std::endl;
        IncrIndent();
        m_bCurrentDefAccessPublic = false;
    }
}

std::stringstream& CDefinitionContext::GetPrefaceStream()
{
    return m_sstreamPreface;
}

std::stringstream& CDefinitionContext::GetDefCodeStream()
{
    return m_sstreamDefCode;
}

std::stringstream& CDefinitionContext::GetAutoStream()
{
    return m_bPreface ? m_sstreamPreface : m_sstreamDefCode;
}

std::string CDefinitionContext::GetDefinitionCode() const
{
    return m_sstreamPreface.str() + m_sstreamDefCode.str();
}

bool CDefinitionContext::UsePreface() const
{
    return m_bPreface;
}

void CDefinitionContext::DisablePreface()
{
    m_bPreface = false;
}

bool CDefinitionContext::NeedsConstruction() const
{
    return m_bConstructionCompulsory;
}

void CDefinitionContext::SetConstructionNeeded()
{
    m_bConstructionCompulsory = true;
}

bool CDefinitionContext::NeedsNewlineAfterContent() const
{
    return m_bNewlineAfterContent;
}

void CDefinitionContext::EnableNewlineAfterContent()
{
    m_bNewlineAfterContent = true;
}

void CDefinitionContext::DisableNewlineAfterContent()
{
    m_bNewlineAfterContent = true;
}

bool CDefinitionContext::HasFriends() const
{
    return !m_setFriends.empty();
}

const std::set<std::string>& CDefinitionContext::GetFriendSet() const
{
    return m_setFriends;
}

void CDefinitionContext::AddFriend(const std::string& rssScopedName)
{
    m_setFriends.insert(rssScopedName);
}

CDefinitionGenerator::CDefinitionGenerator(sdv::IInterfaceAccess* pParser) : CDefinitionGeneratorBase(pParser)
{}

CDefinitionGenerator::~CDefinitionGenerator()
{}

void CDefinitionGenerator::GetTargetFileInfo(std::string& rssTargetSubDir, std::string& rssTargetFileEnding)
{
    rssTargetSubDir.clear();    // No sub directory required.
    rssTargetFileEnding = ".h"; // Target is a header file with the same name as the source file.
}

std::string CDefinitionGenerator::GetFileHeaderText() const
{
    return "This file contains the interface definitions.";
}

void CDefinitionGenerator::StreamIntoFile(CDefinitionContext& rcontext, std::ofstream& rfstream)
{
    // Suppress analysis warnings
    rfstream << R"code(#ifdef _MSC_VER
// Prevent bogus warnings about uninitialized members during static code analysis.
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

)code";

    // If there are any, add forward declarations to the preface
    if (!m_setForwardDecl.empty())
    {
        rcontext.GetPrefaceStream() << "// Forward declarations" << std::endl;
        for (const std::string& rssDefEntity : m_setForwardDecl)
            rcontext.GetPrefaceStream() << "struct " << rssDefEntity << ";" << std::endl;
        rcontext.GetPrefaceStream() << std::endl;
    }

    rfstream << rcontext.GetDefinitionCode();

    // End analysis warning suppression
    rfstream << R"code(
#ifdef _MSC_VER
#pragma warning(pop)
#endif
)code";
}

void CDefinitionGenerator::StreamIncludeSection(CDefinitionContext& rcontext)
{
    // Include headers
    rcontext.GetAutoStream() << "// Provide fixed width integer types." << std::endl;
    rcontext.GetAutoStream() << "#include <cstdint>" << std::endl;
    rcontext.GetAutoStream() << std::endl;
}

void CDefinitionGenerator::StreamMetaEntity(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    const sdv::idl::IMetaEntity* pMeta = GetInterface<sdv::idl::IMetaEntity>(pEntity);
    if (!pMeta) return;

    // Add a newline if not the first statement in the body
    bool bAutoNewLine = !rcontext.GetAutoStream().str().empty() && rcontext.GetCurrentIteration();
    if (rcontext.NeedsNewlineAfterContent())
    {
        rcontext.GetAutoStream() << std::endl;
        bAutoNewLine = false;
    }
    rcontext.DisableNewlineAfterContent(); // Next statement should not automatically include a newline

    // Add a newline if not the first statement in the body and a newline was not already added
    if (bAutoNewLine) rcontext.GetAutoStream() << std::endl;
    rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline

    // Next statement should automatically include a newline
    rcontext.EnableNewlineAfterContent();

    std::string ssContent, ssContentLC;
    size_t nPos = std::string::npos;
    switch (pMeta->GetMetaType())
    {
    case sdv::idl::IMetaEntity::EType::define:
        rcontext.DisableIndent();
        StreamComments(rcontext, pEntity);
        rcontext.GetAutoStream() << "#define " << pMeta->GetContent() << std::endl;
        rcontext.EnableIndent();
        break;
    case sdv::idl::IMetaEntity::EType::undef:
        rcontext.DisableIndent();
        StreamComments(rcontext, pEntity);
        rcontext.GetAutoStream() << "#undef " << pMeta->GetContent() << std::endl;
        rcontext.EnableIndent();
        break;
    case sdv::idl::IMetaEntity::EType::include_global:
    case sdv::idl::IMetaEntity::EType::include_local:
        ssContent = pMeta->GetContent();
        ssContentLC = ssContent;
        for (char& rc : ssContentLC) rc = static_cast<char>(std::tolower(rc));
        nPos = ssContentLC.find(".idl");
        if (nPos != std::string::npos)
            ssContent.replace(nPos, 4, ".h");
        rcontext.GetAutoStream() << "#include " << ssContent << std::endl;
        break;
    case sdv::idl::IMetaEntity::EType::verbatim:
        // Insert the content with the current indentation
        ssContent = pMeta->GetContent();
        if (!ssContent.empty())
        {
            if (ssContent[0] != '#')
                rcontext.DisableIndent();
            StreamComments(rcontext, pEntity);
            rcontext.GetAutoStream() << SmartIndent(ssContent, rcontext.GetIndent());
            rcontext.EnableIndent();
        }
        else
            rcontext.GetAutoStream() << std::endl;
    }
}

void CDefinitionGenerator::StreamComments(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity,
    ECommentGroup eGroup /*= ECommentGroup::none*/)
{
    // Get the comments
    if (!pEntity) return;
    const sdv::idl::IEntityComments* pComments = GetInterface<sdv::idl::IEntityComments>(pEntity);
    if (!pComments) return;
    uint32_t uiFlags = 0;
    std::string ssComments = pComments->GetComments(uiFlags);
    if (ssComments.empty()) return;

    // Stream the lines of comments
    auto fnStreamLines = [&](const std::string& rssPrefix) -> void
    {
        // Group comment?
        switch (eGroup)
        {
        case ECommentGroup::begin:
            rcontext.GetAutoStream() << rcontext.GetIndent() << rssPrefix << "@{" << std::endl;
            break;
        case ECommentGroup::end:
            rcontext.GetAutoStream() << rcontext.GetIndent() << rssPrefix << "@}" << std::endl;
            return; // Do not stream comment
        default:
            break;
        }

        size_t nPos = 0;
        while (nPos < ssComments.size())
        {
            size_t nEnd = ssComments.find_first_of("\r\n", nPos);
            rcontext.GetAutoStream() << rcontext.GetIndent() << rssPrefix <<
                ssComments.substr(nPos, nEnd != std::string::npos ? nEnd - nPos : nEnd) << std::endl;

            nPos = nEnd;
            if (nPos < ssComments.size() && ssComments[nPos] == '\r')
                nPos++;
            if (nPos < ssComments.size() && ssComments[nPos] == '\n')
                nPos++;
        }
    };

    // C or C++
    switch (uiFlags & ~static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding))
    {
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_javadoc):
    //case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_javadoc_post):
        fnStreamLines("/// ");
        break;
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_qt):
    //case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_qt_post):
        fnStreamLines("//! ");
        break;
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style):
        fnStreamLines("// ");
        break;
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc):
    //case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc_post):
        rcontext.GetAutoStream() << rcontext.GetIndent() << "/**" << std::endl;
        fnStreamLines(" * ");
        rcontext.GetAutoStream() << rcontext.GetIndent() << " */" << std::endl;
        break;
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt):
    //case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt_post):
        rcontext.GetAutoStream() << rcontext.GetIndent() << "/*!" << std::endl;
        fnStreamLines(" * ");
        rcontext.GetAutoStream() << rcontext.GetIndent() << " */" << std::endl;
        break;
    case static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style):
    default:
        rcontext.GetAutoStream() << rcontext.GetIndent() << "/*" << std::endl;
        fnStreamLines(" * ");
        rcontext.GetAutoStream() << rcontext.GetIndent() << " */" << std::endl;
        break;
    }
}

bool CDefinitionGenerator::StreamDeclaration(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    rcontext.DisablePreface();

    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log("Stream declaration ", pEntityInfo->GetScopedName());

    // Add a newline if not the first statement in the body
    if (rcontext.NeedsNewlineAfterContent())
        rcontext.GetDefCodeStream() << std::endl;
    rcontext.DisableNewlineAfterContent(); // Next statement should not automatically include a newline

    // In case the declaration is a typedef declaration, call the specific streaming function (the typedef declaration is composed
    // in a different order).
    switch (pEntityInfo->GetType())
    {
        case sdv::idl::EEntityType::type_typedef:
            rcontext.SetDefAccessPublic();
            StreamTypedef(rcontext, pEntity);
            return true;
        case sdv::idl::EEntityType::type_attribute:
            rcontext.SetDefAccessPublic();
            StreamAttribute(rcontext, pEntity);
            return true;
        case sdv::idl::EEntityType::type_operation:
            rcontext.SetDefAccessPublic();
            StreamOperation(rcontext, pEntity);
            return true;
        case sdv::idl::EEntityType::type_enum_entry:
            rcontext.SetDefAccessPublic();
            StreamEnumEntry(rcontext, pEntity);
            return true;
        case sdv::idl::EEntityType::type_case_entry:
            rcontext.SetDefAccessPublic();
            StreamCaseEntry(rcontext, pEntity);
            break;  // Note... a variable declaration is following
        case sdv::idl::EEntityType::type_switch_variable:
            // The switch variable is private, since change is only allowed through functions.
            rcontext.SetDefAccessPrivate();
            break;  // Note... a variable declaration is following
        default:
            // Allow access.
            rcontext.SetDefAccessPublic();
            break;
    }

    // Exception for _description... if defined, add suppression of static code analysis
    if (pEntityInfo->GetName() == "_description")
    {
        rcontext.GetDefCodeStream() << rcontext.GetIndent()
                                    << "// _description might also be member of the base class. Suppress in static code analysis."
                                    << std::endl;
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "// cppcheck-suppress duplInheritedMember" << std::endl;
    }

    // Stream any comments
    StreamComments(rcontext, pEntity);

    // Stream the declaration
    bool bSkipInitialIndent = true;
    if (pDeclaration->IsReadOnly())
        rcontext.GetDefCodeStream() << rcontext.GetIndent() <<
            (rcontext.IsCompound() ? "inline static const " : "static const ");
    else
        bSkipInitialIndent = false;
    bool bDefinitionStreamed = false;
    if (!StreamDeclType(rcontext, pEntity, bDefinitionStreamed, pDeclaration->IsAnonymous(), bSkipInitialIndent))
        return true; // This declaration is non-streamable, since it is unknown (could be with case entity without declaration).

    // Only stream the name and any additional information when not anonymous.
    if (!pDeclaration->IsAnonymous())
    {
        // Stream the name
        rcontext.GetDefCodeStream() << " " << pEntityInfo->GetName();

        // Stream the array declaration if available.
        if (pDeclaration->HasArray())
        {
            sdv::sequence<sdv::idl::SArrayDimension> seqDimensions = pDeclaration->GetArrayDimensions();
            for (const sdv::idl::SArrayDimension& rsDimension : seqDimensions)
            {
                rcontext.GetDefCodeStream() << "[";
                rcontext.GetDefCodeStream() << GetRelativeScopedName(rsDimension.ssExpression, rcontext.GetScope());
                rcontext.GetDefCodeStream() << "]";
            }
        }

        // Stream the assignment if available.
        if (pDeclaration->HasAssignment())
            rcontext.GetDefCodeStream() << " = " << pDeclaration->GetAssignment();
    }

    // Finalize the statement.
    rcontext.GetDefCodeStream() << ";" << std::endl;

    // Definition? Add another newline.
    if (bDefinitionStreamed)
        rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline

    return true;
}

void CDefinitionGenerator::StreamDefinition(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInline /*= false*/,
    bool bAnonymousDecl /*= false*/)
{
    rcontext.DisablePreface();

    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pEntity);
    if (!pDefinition) throw CCompileException("Internal error: the definition entity doesn't expose definition interface.");

    // When unnamed, only stream when the definition was inline
    if (pDefinition->IsUnnamed() && !bInline) return;

    // Add a newline if not the first statement in the body
    bool bAutoNewLine = !rcontext.GetDefCodeStream().str().empty() && rcontext.GetCurrentIteration();
    if (rcontext.NeedsNewlineAfterContent())
    {
        rcontext.GetDefCodeStream() << std::endl;
        bAutoNewLine = false;
    }
    rcontext.DisableNewlineAfterContent(); // Next statement should not automatically include a newline

    // Add a newline if not the first statement in the body and a newline was not already added
    if (bAutoNewLine) rcontext.GetDefCodeStream() << std::endl;

    switch (pEntityInfo->GetType())
    {
    case sdv::idl::EEntityType::type_enum:
    case sdv::idl::EEntityType::type_exception:
    case sdv::idl::EEntityType::type_struct:
    case sdv::idl::EEntityType::type_union:
    case sdv::idl::EEntityType::type_module:
    case sdv::idl::EEntityType::type_interface:
        break;
    case sdv::idl::EEntityType::type_unknown:
    default:
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "// Unknown entity type: " <<
            pEntityInfo->GetName() << std::endl;
        return;
    }

    // Get the definition type
    std::string ssType = MapEntityType2CType(pEntityInfo->GetType());

    // There are two types of unions... the type based union (switch is done based on the value of a type) and a variable based
    // union (switch is done based on the value of an existing variable).
    // The type based union needs an additional variable defining the switch. This variable is named 'switch_value'. The union and
    // the variable as well as any construction and destruction functions are inserted into a struct. The union itself is an
    // anonymous (without name) and transparent (without explicit declaration) member of this struct.
    // The variable based union differs from the type based union by defining a member variable for the switch. This variable is a
    // member of the struct containing the union declaration. Therefore the variable based union can only be defined as a member.
    // An additional struct generation containing the switch type variable as well as the union definition and declaration is not
    // needed, since it exists already. There is one exception... the unnamed and anonymous union with variable based switch. This
    // union can only be part of a struct and the union-members are initialized as if they are members of the struct.

    // Function to check whether the entity is a union.
    auto fnIsUnion = [](sdv::IInterfaceAccess* pEntityObj) -> bool
    {
        if (!pEntityObj) return false;
        return pEntityObj->GetInterface<sdv::idl::IUnionEntity>() ? true : false;
    };

    // Function to check whether the entity is a type based union.
    auto fnIsTypeBaseUnion = [](sdv::IInterfaceAccess* pEntityObj) -> bool
    {
        if (!pEntityObj) return false;
        const sdv::idl::IUnionEntity* pUnion = pEntityObj->GetInterface<sdv::idl::IUnionEntity>();
        if (!pUnion) return false;
        return pUnion->GetSwitchInterpretation() == sdv::idl::IUnionEntity::ESwitchInterpret::switch_type;
    };

    // Forward declaration
    if (pEntityInfo->ForwardDeclaration())
    {
        CLog log;
        log << "Stream forward declaration " << pEntityInfo->GetScopedName() <<std::endl;

        // Get the full entity definition (if existing).
        sdv::idl::IForwardDeclarationEntity* pForwardDeclaration = pEntity->GetInterface<sdv::idl::IForwardDeclarationEntity>();
        sdv::IInterfaceAccess* pFullEntity = pForwardDeclaration ? pForwardDeclaration->GetEntity() : nullptr;

        // In case the entity is a union with type based switch, the generated type is a struct.
        if (fnIsTypeBaseUnion(pFullEntity)) ssType = "struct";

        // Stream the forward declarative statement.
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "/* Forward declaration */" << std::endl;
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << ssType << " " << pEntityInfo->GetName() << ";" << std::endl;

        rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline
        return;
    }

    CLog log("Stream definition ", pEntityInfo->GetScopedName());
    if (pDefinition->IsUnnamed())
        log << "Definition is unnamed" << std::endl;
    if (bInline)
        log << "Definition is followed by an inline declaration" << std::endl;
    if (bAnonymousDecl)
        log << "Definition is part of an anonymous declaration" << std::endl;

    // Add to history
    m_setHistory.insert(pEntityInfo->GetScopedName());

    // Create a dedicated context for this definition's content
    CDefinitionContext sContentStreamContext(rcontext, pEntity);
    sContentStreamContext.IncrIndent();

    // Stream any comments
    rcontext.SetDefAccessPublic();
    StreamComments(rcontext, pEntity);

    // Local interface?
    const sdv::idl::IInterfaceEntity* pInterface = GetInterface<sdv::idl::IInterfaceEntity>(pEntity);
    rcontext.GetDefCodeStream() << rcontext.GetIndent();
    if (pInterface && pInterface->IsLocal()) rcontext.GetDefCodeStream() << "/*local*/ ";

    // If the entity is a type based union, insert the additional struct and increase the indentation for definitions.
    bool bSuppressType = false;
    bool bForceName = false;
    std::string ssSwitchType = "__unknown__";
    std::string ssSwitchVar = "__unknown__";
    if (fnIsUnion(pEntity))
    {
        // Get the switch type
        const sdv::idl::IUnionEntity* pUnion = pEntity->GetInterface<sdv::idl::IUnionEntity>();
        if (!pUnion) throw CCompileException("Internal error: Could not acquire union-pointer.");

        // Diferentiate between type based and variable based switch.
        sdv::idl::IUnionEntity::ESwitchInterpret eInterpretation = pUnion->GetSwitchInterpretation();
        if (eInterpretation == sdv::idl::IUnionEntity::ESwitchInterpret::switch_type)
        {
            sdv::idl::EDeclType eDeclType = sdv::idl::EDeclType::decltype_unknown;
            sdv::IInterfaceAccess* pType = nullptr;
            pUnion->GetSwitchType(eDeclType, pType);

            // The variable type is needed
            if (pType)
            {
                const sdv::idl::IEntityInfo* pSwitchEntityInfo = pType->GetInterface<sdv::idl::IEntityInfo>();
                if (pSwitchEntityInfo)
                    ssSwitchType = pSwitchEntityInfo->GetName();
            }
            else
                ssSwitchType = MapDeclType2CType(eDeclType);

            // The variable name is "switch_value".
            ssSwitchVar = "switch_value";

            // Add an additional struct level to wrap the union
            rcontext.GetDefCodeStream() << "struct " << pEntityInfo->GetName();
            sContentStreamContext.EnableDeepIndent(); // Increase indentation for the definition code of the children

            // Create the construction and destruction functions
            ProcessUnionInContainerContext(sContentStreamContext, "", "", pEntity);

            // Prevent defining the name twice...
            bSuppressType = true;
        } else if (eInterpretation == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
        {
            sdv::u8string ssVarDeclName;
            sdv::IInterfaceAccess* pVar = nullptr;
            sdv::IInterfaceAccess* pContainer = nullptr;
            pUnion->GetSwitchVar(ssVarDeclName, pVar, pContainer);

            // The variable name is needed
            if (pVar)
            {
                const sdv::idl::IEntityInfo* pSwitchEntityInfo = pVar->GetInterface<sdv::idl::IEntityInfo>();
                if (pSwitchEntityInfo)
                    ssSwitchVar = pSwitchEntityInfo->GetName();
            }
        } else throw CCompileException("Internal error: the switch type of the union is invalid.");

        // When unnamed, the construction of the container is compulsory.
        if (pDefinition->IsUnnamed())
        {
            // Unnamed unions need to be constructed in the parent structure.
            rcontext.SetConstructionNeeded();

            // Name for Local definition needs to be added if not anonymous.
            bForceName = !bAnonymousDecl;
        }
    }

    // The type based union suppresses the type
    if (!bSuppressType)
    {
        // Stream the type
        rcontext.GetDefCodeStream() << ssType;

        // Stream the name
        if (!bInline || bForceName) rcontext.GetDefCodeStream() << " " << pEntityInfo->GetName();

        // Extra comments for a union
        if (fnIsUnion(pEntity))
            rcontext.GetDefCodeStream() << " /*switch(" << ssSwitchVar << ")*/";
    }

    // Check for inheritance (definitions only)
    sdv::idl::IEntityIterator* pInheritanceIterator = pDefinition ? pDefinition->GetInheritance() : nullptr;
    if (pInheritanceIterator && pInheritanceIterator->GetCount())
    {
        rcontext.GetDefCodeStream() << " : ";
        for (uint32_t uiIndex = 0; uiIndex < pInheritanceIterator->GetCount(); uiIndex++)
        {
            if (uiIndex != 0) rcontext.GetDefCodeStream() << ", ";
            const sdv::idl::IEntityInfo* pBaseEntity =
                GetInterface<sdv::idl::IEntityInfo>(pInheritanceIterator->GetEntityByIndex(uiIndex));
            if (!pBaseEntity) throw CCompileException("Internal error: the entity inherits from an unknown entity.");
            rcontext.GetDefCodeStream() << GetRelativeScopedName(pBaseEntity->GetScopedName(), rcontext.GetScope());
        }
    }

    // Check for enum classes
    const sdv::idl::IEnumEntity* pEnum = GetInterface<sdv::idl::IEnumEntity>(pEntity);
    if (pEnum)
    {
        rcontext.GetDefCodeStream() << " : ";
        sdv::idl::EDeclType eBaseType = sdv::idl::EDeclType::decltype_unknown;
        sdv::IInterfaceAccess* pBaseType = nullptr;
        pEnum->GetBaseType(eBaseType, pBaseType);
        if (pBaseType)
        {
            const sdv::idl::IEntityInfo* pBaseEntity = GetInterface<sdv::idl::IEntityInfo>(pBaseType);
            if (!pBaseEntity) throw CCompileException("Internal error: enum base type is not an entity.");
            rcontext.GetDefCodeStream() << pBaseEntity->GetScopedName();
        }
        else
            rcontext.GetDefCodeStream() << MapDeclType2CType(eBaseType);
    }

    // Start content
    rcontext.GetDefCodeStream() << std::endl << rcontext.GetIndent() << "{" << std::endl;
    rcontext.IncrIndent();

    // Run through the declarations and add each declaration. Add the definition before the declaration if there is one to be added.
    sdv::idl::IEntityIterator* pChildIterator = pDefinition ? pDefinition->GetChildren() : nullptr;
    bool bContainsConstDescription = false;
    for (uint32_t uiIndex = 0; !bInline && pChildIterator && uiIndex < pChildIterator->GetCount(); uiIndex++)
    {
        sdv::IInterfaceAccess* pChildEntity = pChildIterator->GetEntityByIndex(uiIndex);
        if (!pChildEntity) continue;
        const sdv::idl::IEntityInfo* pChildEntityInfo = pChildEntity->GetInterface<sdv::idl::IEntityInfo>();
        if (!pChildEntityInfo) continue;

        // Is the entity a declaration?
        sdv::idl::IDeclarationEntity* pChildDeclarationEntity = pChildEntity->GetInterface<sdv::idl::IDeclarationEntity>();
        if (!pChildDeclarationEntity) continue;
        sdv::IInterfaceAccess* pChildDeclTypeObj = pChildDeclarationEntity->GetDeclarationType();
        if (!pChildDeclTypeObj) continue;
        const sdv::idl::IDeclarationType* pChildDeclType = pChildDeclTypeObj->GetInterface<sdv::idl::IDeclarationType>();
        if (!pChildDeclType) continue;
        if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_exception && pChildEntityInfo->GetName() == "_description")
        {
            // Check for const variable
            if (!pChildDeclarationEntity->IsReadOnly())
                throw CCompileException("Exception definition declares '_description' member; this member must be a "
                    "const-variable.");
            if (pChildDeclType->GetBaseType() != sdv::idl::EDeclType::decltype_char || !pChildDeclarationEntity->HasArray())
                throw CCompileException("Exception definition declares '_description' member; "
                    "this member must be of char array (char[]) type.");
            bContainsConstDescription = true;
        }
    }

    // If this entity is an interface, stream an interface ID declaration.
    if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_interface)
    {
        if (pInheritanceIterator)
        {
            sContentStreamContext.GetDefCodeStream()
                << rcontext.GetIndent() << "// _id might also be member of the base class. Suppress in static code analysis."
                << std::endl;
            sContentStreamContext.GetDefCodeStream()
                << rcontext.GetIndent() << "// cppcheck-suppress duplInheritedMember" << std::endl;
        }
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "/** Interface ID. */" << std::endl;
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() <<
            "static constexpr ::sdv::interface_id _id = 0x" << std::hex << std::setfill('0') << std::setw(16) << std::uppercase <<
            pEntityInfo->GetId() << ";" << std::endl << std::endl;
    }

    // If this entity is an exception, stream an exception ID declaration.
    if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_exception)
    {
        if (pInheritanceIterator)
        {
            sContentStreamContext.GetDefCodeStream()
                << rcontext.GetIndent() << "// _id might also be member of the base class. Suppress in static code analysis."
                << std::endl;
            sContentStreamContext.GetDefCodeStream()
                << rcontext.GetIndent() << "// cppcheck-suppress duplInheritedMember" << std::endl;
        }
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "/** Exception ID. */" << std::endl;
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "inline static const ::sdv::exception_id _id = 0x" <<
            std::hex << std::setfill('0') << std::setw(16) << std::uppercase << pEntityInfo->GetId() << ";" << std::endl <<
            std::endl;

        // Check whether the exception has a const variable "_description". If not, generate one from the name of the exception.
        if (!bContainsConstDescription)
        {
            if (pInheritanceIterator)
            {
                sContentStreamContext.GetDefCodeStream()
                    << rcontext.GetIndent()
                    << "// _description might also be member of the base class. Suppress in static code analysis." << std::endl;
                sContentStreamContext.GetDefCodeStream()
                    << rcontext.GetIndent() << "// cppcheck-suppress duplInheritedMember" << std::endl;
            }
            sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "/** Explanatory string. */" << std::endl;
            sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() <<
               "inline static const char _description[] = \"" << pEntityInfo->GetName() << " exception\";" << std::endl;
        }
    }

    // If the entity itself is a named variable based union, it needs constructor and destructor functions, since they are/might be
    // deleted in case the members are complex. This works only for named unions.
    if (pEntityInfo && pDefinition && fnIsUnion(pEntity) && !fnIsTypeBaseUnion(pEntity) &&
        (!pDefinition->IsUnnamed() || bForceName))
    {
        // Stream construction/destruction function
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "/** Constructor */" << std::endl;
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << pEntityInfo->GetName() << "() {}" <<
            std::endl << std::endl;

        // Stream desstruction function
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "/** Destructor */" << std::endl;
        sContentStreamContext.GetDefCodeStream() << rcontext.GetIndent() << "~" << pEntityInfo->GetName() << "() {}" <<
            std::endl << std::endl;
    }

    // Does the entity have children?
    // NOTE: Compound definitions are all definitions except modules.
    if (pChildIterator && !ProcessEntities(sContentStreamContext, pChildIterator)) return;

    // Add the what-function to the exception
    if (pEntityInfo->GetType() == sdv::idl::EEntityType::type_exception)
    {
        // Create the "what" function analog to the std::exception:what function.
        sContentStreamContext.GetDefCodeStream() << std::endl << sContentStreamContext.GetIndent() <<
            "/** Return the explanatory string. @return The descriptive string. */" << std::endl;
        sContentStreamContext.GetDefCodeStream()
            << sContentStreamContext.GetIndent() << "virtual const char* what() const noexcept "
            << (pInheritanceIterator ? "override " : "")
            << "{ return _description; }" << std::endl;

        // Create the "id" function.
        sContentStreamContext.GetDefCodeStream() << std::endl << sContentStreamContext.GetIndent() <<
            "/** Return the id. @return The id of the exception.*/" << std::endl;
        sContentStreamContext.GetDefCodeStream()
            << sContentStreamContext.GetIndent() << "virtual ::sdv::exception_id id() const noexcept "
            << (pInheritanceIterator ? "override " : "") << "{ return _id; }" << std::endl;
    }

    // Stream any friend classes from switch variables.
    if (sContentStreamContext.HasFriends())
    {
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "// Friend classes access switch paramers." << std::endl;
        for (const std::string& rssFriend : sContentStreamContext.GetFriendSet())
            rcontext.GetDefCodeStream() << rcontext.GetIndent() << "friend " << rssFriend << ";" << std::endl;
        rcontext.GetDefCodeStream() << std::endl;
    }

    // Build the code streams from any switch variables
    std::stringstream sstreamConstructorImpl;
    std::stringstream sstreamCopyConstructImpl;
    std::stringstream sstreamMoveConstructImpl;
    std::stringstream sstreamDestructorImpl;
	std::stringstream sstreamPrivateFunc;
	std::stringstream sstreamPublicFunc;
    for (const auto& rptrSwitchCodeContext : sContentStreamContext.GetSwitchCodeContexts<SDefinitionSwitchCodeContext>())
    {
        std::string ssSwitchVarName = rptrSwitchCodeContext->ssSwitchVarName;
        std::string ssSwitchVarType = rptrSwitchCodeContext->ptrSwitchVar->ssType;
        //std::string ssSwitchVarValue;

        // Create a stream of constructor implementation code
        if (!rptrSwitchCodeContext->sstreamConstructorImpl.str().empty())
            sstreamConstructorImpl <<
                SmartIndent(rptrSwitchCodeContext->sstreamConstructorImpl.str(), sContentStreamContext.GetIndent(false, true));

        // Create a stream of destructor implementation code
        if (!rptrSwitchCodeContext->sstreamDestructorImpl.str().empty())
            sstreamDestructorImpl <<
                SmartIndent(rptrSwitchCodeContext->sstreamDestructorImpl.str(), sContentStreamContext.GetIndent(false, true));

        // Create constructor helper function
        if (!rptrSwitchCodeContext->sstreamConstructHelperImpl.str().empty())
        {
            if (!sstreamPrivateFunc.str().empty())
                sstreamPrivateFunc << std::endl;
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "/** Constructor helper function for ";
            sstreamPrivateFunc << ssSwitchVarName;
            sstreamPrivateFunc << " */" << std::endl;
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "void construct_" << QualifyName(ssSwitchVarName);
            sstreamPrivateFunc << "(";
            sstreamPrivateFunc << ssSwitchVarType << " val = ";
            //if (!ssSwitchVarValue.empty())
            //    sstreamPrivateFunc << ssSwitchVarValue;
            //else
                sstreamPrivateFunc << ssSwitchVarType << "{}";
            sstreamPrivateFunc << ")" << std::endl << sContentStreamContext.GetIndent(false) << "{" << std::endl;
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false, true) << ssSwitchVarName << " = val;" << std::endl;
            sstreamPrivateFunc <<
                SmartIndent(rptrSwitchCodeContext->sstreamConstructHelperImpl.str(), sContentStreamContext.GetIndent(false, true));
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "}" << std::endl;
        }

        // Create copy constructor impl for the switch type
        if (!rptrSwitchCodeContext->sstreamCopyConstructHelperImpl.str().empty())
        {
            if (!sstreamCopyConstructImpl.str().empty())
                sstreamCopyConstructImpl << std::endl;
            sstreamCopyConstructImpl << sContentStreamContext.GetIndent(false, true) << "// Construct content based on " <<
                ssSwitchVarName << std::endl;
            sstreamCopyConstructImpl <<
                SmartIndent(rptrSwitchCodeContext->sstreamCopyConstructHelperImpl.str(),
                    sContentStreamContext.GetIndent(false, true));
        }

        // Create move constructor impl for the switch type
        if (!rptrSwitchCodeContext->sstreamMoveConstructHelperImpl.str().empty())
        {
            if (!sstreamMoveConstructImpl.str().empty())
                sstreamMoveConstructImpl << std::endl;
            sstreamMoveConstructImpl << sContentStreamContext.GetIndent(false, true) << "// Construct content based on " <<
                ssSwitchVarName << std::endl;
            sstreamMoveConstructImpl <<
                SmartIndent(rptrSwitchCodeContext->sstreamMoveConstructHelperImpl.str(),
                    sContentStreamContext.GetIndent(false, true));
        }

        // Create destructor helper function
        if (!rptrSwitchCodeContext->sstreamDestructHelperImpl.str().empty())
        {
            if (!sstreamPrivateFunc.str().empty())
                sstreamPrivateFunc << std::endl;
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "/** Destructor helper function for ";
            sstreamPrivateFunc << ssSwitchVarName;
            sstreamPrivateFunc << " */" << std::endl;
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "void destruct_" << QualifyName(ssSwitchVarName);
            sstreamPrivateFunc << "()" << std::endl << sContentStreamContext.GetIndent(false) << "{" << std::endl;
            sstreamPrivateFunc <<
                SmartIndent(rptrSwitchCodeContext->sstreamDestructHelperImpl.str(), sContentStreamContext.GetIndent(false, true));
            sstreamPrivateFunc << sContentStreamContext.GetIndent(false) << "}" << std::endl;
        }

        // Create a stream of public function code
        if (!rptrSwitchCodeContext->sstreamCode.str().empty())
        {
            if (!sstreamPublicFunc.str().empty())
                sstreamPublicFunc << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "/** Set the switch type for the union";
            if (rptrSwitchCodeContext->ptrSwitchVar->vecUnionDecl.size() > 1)
                sstreamPublicFunc << "s";
            sstreamPublicFunc << " ";
            for (size_t nUnionDeclCnt = 0; nUnionDeclCnt < rptrSwitchCodeContext->ptrSwitchVar->vecUnionDecl.size(); nUnionDeclCnt++)
            {
                if (nUnionDeclCnt > 0 && nUnionDeclCnt == rptrSwitchCodeContext->ptrSwitchVar->vecUnionDecl.size() - 1)
                    sstreamPublicFunc << " and ";
                else if (nUnionDeclCnt)
                    sstreamPublicFunc << " ,";
                sstreamPublicFunc << rptrSwitchCodeContext->ptrSwitchVar->vecUnionDecl[nUnionDeclCnt];
            }
            sstreamPublicFunc << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) <<
                " * @param[in] val Value of the switch variable to select the switch type." << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            if (sContentStreamContext.HasMultipleSwitchCodeContexts())
                sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "void switch_to_" << QualifyName(ssSwitchVarName);
            else
                sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "void switch_to";
            sstreamPublicFunc << "(";
            sstreamPublicFunc << ssSwitchVarType << " val";
            sstreamPublicFunc << ")" << std::endl << sContentStreamContext.GetIndent(false) << "{" << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false, true) << "// Anything to do?" << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false, true) << "if (" <<  ssSwitchVarName <<
                " == val) return;" << std::endl;
            //sstreamPublicFunc << sContentStreamContext.GetIndent(false, true) << "// Assign the new value..." << std::endl;
            //sstreamPublicFunc << sContentStreamContext.GetIndent(false, true) <<
            //    rptrSwitchCodeContext->ssSwitchVarName << " = val;" << std::endl;
            sstreamPublicFunc << SmartIndent(rptrSwitchCodeContext->sstreamCode.str(), sContentStreamContext.GetIndent(false, true));
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "}" << std::endl << std::endl;

            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "/** Get the switch value" << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << " * @return Returns the current switch value." << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            if (sContentStreamContext.HasMultipleSwitchCodeContexts())
                sstreamPublicFunc << sContentStreamContext.GetIndent(false) << ssSwitchVarType << " get_switch_" <<
                QualifyName(ssSwitchVarName);
            else
                sstreamPublicFunc << sContentStreamContext.GetIndent(false) << ssSwitchVarType << " get_switch";
            sstreamPublicFunc << "() const" << std::endl << sContentStreamContext.GetIndent(false) << "{" << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false, true) << "return " << ssSwitchVarName << ";" << std::endl;
            sstreamPublicFunc << sContentStreamContext.GetIndent(false) << "}" << std::endl;
        }
    }

    // Copy the content of the streams to the definition body.
    if (sContentStreamContext.IsStructural())
    {
        // Stream constructor function
        std::string ssConstructorImpl = sstreamConstructorImpl.str();
        if (sContentStreamContext.NeedsConstruction() || !ssConstructorImpl.empty())
        {
            if (!ssConstructorImpl.empty())
            {
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false)
                    << "// No initialization of variables is done in the constructor; suppress cppcheck warning."
                                            << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false)
                                            << "// cppcheck-suppress uninitMemberVar" << std::endl;
            }
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/** Constructor */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << pEntityInfo->GetName() << "()";
            if (ssConstructorImpl.empty())
                rcontext.GetDefCodeStream() << " {}" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << ssConstructorImpl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Stream copy constructor function
        std::string ssCopyConstructImpl = sstreamCopyConstructImpl.str();
        if (sContentStreamContext.NeedsConstruction() || !ssCopyConstructImpl.empty())
        {
            if (!ssCopyConstructImpl.empty())
            {
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(
                    false) << "// No initialization of variables is done in the constructor; suppress cppcheck warning."
                                            << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "// cppcheck-suppress uninitMemberVar"
                                            << std::endl;
            }
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/**" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * Copy constructor " << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * @param[in] rvar Reference to the " <<
                pEntityInfo->GetName() << " variable to copy from." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << pEntityInfo->GetName() << "(";
            if (ssCopyConstructImpl.empty())
                rcontext.GetDefCodeStream() << "[[maybe_unused]] ";
            rcontext.GetDefCodeStream() << "const " << pEntityInfo->GetName() << "& rvar)";
            if (ssCopyConstructImpl.empty())
                rcontext.GetDefCodeStream() << " {}" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << ssCopyConstructImpl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Stream move constructor function
        std::string ssMoveConstructImpl = sstreamMoveConstructImpl.str();
        if (sContentStreamContext.NeedsConstruction() || !ssMoveConstructImpl.empty())
        {
            if (!ssMoveConstructImpl.empty())
            {
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(
                    false) << "// No initialization of variables is done in the constructor; suppress cppcheck warning."
                                            << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "// cppcheck-suppress uninitMemberVar"
                                            << std::endl;
            }
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/**" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * Move constructor " << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * @param[in] rvar Reference to the " <<
                pEntityInfo->GetName() << " variable to move from." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << pEntityInfo->GetName() << "(";
            if (ssMoveConstructImpl.empty())
                rcontext.GetDefCodeStream() << "[[maybe_unused]] ";
            rcontext.GetDefCodeStream() << pEntityInfo->GetName() << "&& rvar) noexcept";
            if (ssMoveConstructImpl.empty())
                rcontext.GetDefCodeStream() << " {}" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << ssMoveConstructImpl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Stream desstruction function
        std::string ssDestructorImpl = sstreamDestructorImpl.str();
        if (sContentStreamContext.NeedsConstruction() || !ssDestructorImpl.empty())
        {
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/** Destructor */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "~" << pEntityInfo->GetName() << "()";
            if (ssDestructorImpl.empty())
                rcontext.GetDefCodeStream() << " {}" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << ssDestructorImpl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Stream assignment operator function
        if (sContentStreamContext.NeedsConstruction() || !ssCopyConstructImpl.empty())
        {
            if (!ssCopyConstructImpl.empty())
            {
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(
                    false) << "// Assignment of variables is done in a member function; suppress cppcheck warning."
                                            << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "// cppcheck-suppress operatorEqVarError"
                                            << std::endl;
            }
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/**" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false)
                                        << " * Assignment operator" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * @param[in] rvar Reference to the " <<
                pEntityInfo->GetName() << " variable to copy from." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) <<
                " * @return Returns a reference to this entity." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << pEntityInfo->GetName() <<
                "& operator=(";
            if (ssCopyConstructImpl.empty())
                rcontext.GetDefCodeStream() << "[[maybe_unused]] ";
            rcontext.GetDefCodeStream() << "const " << pEntityInfo->GetName() << " & rvar)";
            if (ssCopyConstructImpl.empty())
                rcontext.GetDefCodeStream() << " { return *this; }" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false, true) << "// Destroy previous allocation." <<
                    std::endl;
                rcontext.GetDefCodeStream() << ssDestructorImpl;
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << ssCopyConstructImpl;
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false, true) << "return *this;" << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Stream move operator function
        if (sContentStreamContext.NeedsConstruction() || !ssMoveConstructImpl.empty())
        {
            if (!ssMoveConstructImpl.empty())
            {
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false)
                                            << "// Assignment of variables is done in a member function; suppress cppcheck warning."
                                            << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "// cppcheck-suppress operatorEqVarError"
                                            << std::endl;
            }
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "/**" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * Move operator" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " * @param[in] rvar Reference to the " <<
                pEntityInfo->GetName() << " variable to move from." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) <<
                " * @return Returns a reference to this entity." << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << " */" << std::endl;
            rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << pEntityInfo->GetName() <<
                "& operator=(";
            if (ssMoveConstructImpl.empty())
                rcontext.GetDefCodeStream() << "[[maybe_unused]] ";
            rcontext.GetDefCodeStream() << pEntityInfo->GetName() << "&& rvar) noexcept";
            if (ssMoveConstructImpl.empty())
                rcontext.GetDefCodeStream() << " { return *this; }" << std::endl;
            else
            {
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "{" << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false, true) << "// Destroy previous allocation." <<
                    std::endl;
                rcontext.GetDefCodeStream() << ssDestructorImpl;
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << ssMoveConstructImpl;
                rcontext.GetDefCodeStream() << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false, true) << "return *this;" << std::endl;
                rcontext.GetDefCodeStream() << sContentStreamContext.GetIndent(false) << "}" << std::endl;
            }
            rcontext.GetDefCodeStream() << std::endl;
        }

        // Private functions
        std::string ssPrivateFunc = sstreamPrivateFunc.str();
        if (!ssPrivateFunc.empty())
        {
            rcontext.SetDefAccessPrivate();
            rcontext.GetDefCodeStream() << ssPrivateFunc;
        }
    }

    // Public or global functions
    std::string ssPublicFunc = sstreamPublicFunc.str();
    if (!ssPublicFunc.empty())
    {
        if (!sstreamPrivateFunc.str().empty()) rcontext.GetDefCodeStream() << std::endl;
        rcontext.SetDefAccessPublic();
        rcontext.GetDefCodeStream() << ssPublicFunc << std::endl;
    }

    // For the type based union, add the switch variable
    if (fnIsTypeBaseUnion(pEntity))
    {
        // Stream switch var for type based unions
        rcontext.SetDefAccessPrivate();
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << ssSwitchType <<
            " switch_value{};   ///< Union switch variable." << std::endl << std::endl;

        // Start the unnamed union
        rcontext.SetDefAccessPublic();
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "union /*switch(switch_value)*/" <<
            std::endl << rcontext.GetIndent() << "{" << std::endl;
    }

    // Definition content
    std::string ssDefBody = sContentStreamContext.GetDefCodeStream().str();
    if (!ssDefBody.empty())
    {
        rcontext.SetDefAccessPublic();
        rcontext.GetDefCodeStream() << ssDefBody;
    }

    // Finalize (close the additional generated struct in case a type based union was inserted).
    if (fnIsTypeBaseUnion(pEntity))
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "};" << std::endl;
    rcontext.DecrIndent();
    rcontext.GetDefCodeStream() << rcontext.GetIndent() << "}";
    if (!bInline)
    {
        if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_module) // Modules don't need a semi-colon at the end.
            rcontext.GetDefCodeStream() << ";";
        rcontext.GetDefCodeStream() << std::endl;
        rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline
    }
}

void CDefinitionGenerator::StreamTypedef(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_typedef)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log;
    log << "Stream type definition " << pEntityInfo->GetName() << std::endl;

    // Stream any comments
    StreamComments(rcontext, pEntity);

    // Stream the typedef declaration
    bool bDefinitionStreamed = false;
    rcontext.GetDefCodeStream() << rcontext.GetIndent() << "typedef ";
    StreamDeclType(rcontext, pEntity, bDefinitionStreamed);
    rcontext.GetDefCodeStream() << " " << pEntityInfo->GetName();

    // Stream array dimensions
    if (pDeclaration->HasArray())
    {
        log << "Type definition contains an array" << std::endl;
        sdv::sequence<sdv::idl::SArrayDimension> seqDimensions = pDeclaration->GetArrayDimensions();
        for (const sdv::idl::SArrayDimension& rsDimension : seqDimensions)
        {
            rcontext.GetDefCodeStream() << "[";
            rcontext.GetDefCodeStream() << GetRelativeScopedName(rsDimension.ssExpression, rcontext.GetScope());
            rcontext.GetDefCodeStream() << "]";
        }
    }

    rcontext.GetDefCodeStream() << ";" << std::endl;
}

void CDefinitionGenerator::StreamAttribute(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_attribute)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log;
    log << "Stream " << (pDeclaration->IsReadOnly() ? "read-only " : "") << "attribute" << pEntityInfo->GetName() << std::endl;

    // Stream any comments, if necessary as compound comment
    StreamComments(rcontext, pEntity, pDeclaration->IsReadOnly() ? ECommentGroup::none : ECommentGroup::begin);

    // Stream the attribute getter function
    SCDeclInfo sCDeclInfo = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), rcontext.GetScope(), true);
    rcontext.GetDefCodeStream() << rcontext.GetIndent() << "virtual " << sCDeclInfo.ssDeclType << " get_" <<
        pEntityInfo->GetName() << "() const = 0;" << std::endl;

    // Stream the attribute setter function
    if (!pDeclaration->IsReadOnly())
    {
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "virtual void set_" << pEntityInfo->GetName() << "(";
        if ((sCDeclInfo.bIsPointer || sCDeclInfo.bIsComplex) && !sCDeclInfo.bIsInterface)
            rcontext.GetDefCodeStream() << "const ";
        rcontext.GetDefCodeStream() << sCDeclInfo.ssDeclType;
        if (sCDeclInfo.bIsComplex && !sCDeclInfo.bIsInterface)
            rcontext.GetDefCodeStream() << "&";
        rcontext.GetDefCodeStream() << " attr) = 0;" << std::endl;

        // Finalize the comments if necessary
        StreamComments(rcontext, pEntity, ECommentGroup::end);
    }

    rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline
}

void CDefinitionGenerator::StreamOperation(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_operation)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log;
    log << "Stream operation " << pEntityInfo->GetName() << std::endl;

    // Stream any comments
    StreamComments(rcontext, pEntity);

    // Stream the operation
    SCDeclInfo sCDeclInfo = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), rcontext.GetScope(), true);
    rcontext.GetDefCodeStream() << rcontext.GetIndent() << "virtual " << sCDeclInfo.ssDeclType << " " <<
        pEntityInfo->GetName() << "(";

    // Does the entity have parameter?
    sdv::idl::IOperationEntity* pOperation = GetInterface<sdv::idl::IOperationEntity>(pEntity);
    sdv::idl::IEntityIterator* pParamIterator = pOperation ? pOperation->GetParameters() : nullptr;
    for (uint32_t uiIndex = 0; pParamIterator && uiIndex < pParamIterator->GetCount(); uiIndex++)
    {
        // Get the entity
        sdv::IInterfaceAccess* pParamEntity = pParamIterator->GetEntityByIndex(uiIndex);
        if (!pParamEntity) throw CCompileException("Internal error: processing non-existent entity.");
        StreamParameter(rcontext, pParamEntity, uiIndex == 0);
    }

    // TODO: Deal with exceptions --> serialize

    // Finalize the operation
    rcontext.GetDefCodeStream() << ")";
    if (pDeclaration->IsReadOnly()) rcontext.GetDefCodeStream() << " const";
    rcontext.GetDefCodeStream() << " = 0;" << std::endl;

    rcontext.EnableNewlineAfterContent(); // Next statement should also include a newline
}

void CDefinitionGenerator::StreamParameter(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity, bool bInitial)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_parameter)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");
    const sdv::idl::IParameterEntity* pParameter = GetInterface<sdv::idl::IParameterEntity>(pEntity);
    if (!pParameter)
        throw CCompileException("Internal error: the entity is a parameter, but doesn't expose parameter information.");

    CLog log;
    log << "Stream parameter " << pEntityInfo->GetName() << std::endl;

    // Add a comma if this is not the initial parameter
    if (!bInitial)
        rcontext.GetDefCodeStream() << ", ";

    // Get the declaration information
    SCDeclInfo sDeclInfo = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), rcontext.GetScope(), true);

    // Stream the parameter direction. All but the input parameter need to support the ouput of values.
    switch (pParameter->GetDirection())
    {
    case sdv::idl::IParameterEntity::EParameterDirection::input:
        rcontext.GetDefCodeStream() << "/*in*/ ";
        if ((sDeclInfo.bIsComplex || sDeclInfo.bIsPointer) && !sDeclInfo.bIsInterface)
            rcontext.GetDefCodeStream() << "const ";
        break;
    case sdv::idl::IParameterEntity::EParameterDirection::output:
        rcontext.GetDefCodeStream() << "/*out*/ ";
        break;
    case sdv::idl::IParameterEntity::EParameterDirection::in_out:
        rcontext.GetDefCodeStream() << "/*inout*/ ";
        break;
    default:
        break;
    }

    // Stream the type
    rcontext.GetDefCodeStream() << sDeclInfo.ssDeclType;

    // Stream the reference if the type if not input
    if ((sDeclInfo.bIsComplex && !sDeclInfo.bIsInterface && !sDeclInfo.bIsPointer)
        ||
        pParameter->GetDirection() != sdv::idl::IParameterEntity::EParameterDirection::input)
        rcontext.GetDefCodeStream() << "&";

    // Stream the name
    rcontext.GetDefCodeStream() << " " << pEntityInfo->GetName();
}

void CDefinitionGenerator::StreamEnumEntry(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_enum_entry)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log;
    log << "Stream enum entry " << pEntityInfo->GetName() << std::endl;

    // Stream any comments
    StreamComments(rcontext, pEntity);

    rcontext.GetDefCodeStream() << rcontext.GetIndent() <<
        GetRelativeScopedName(pEntityInfo->GetScopedName(), rcontext.GetScope());
    if (pDeclaration->HasAssignment())
        rcontext.GetDefCodeStream() << " = " << pDeclaration->GetAssignment();
    rcontext.GetDefCodeStream() << "," << std::endl;
}

void CDefinitionGenerator::StreamCaseEntry(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity)
{
    // Get the entity interfaces.
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    if (!pEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
    if (pEntityInfo->GetType() != sdv::idl::EEntityType::type_case_entry)
        throw CCompileException("Internal error: the entity has incorrect type information.");
    const sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pDeclaration)
        throw CCompileException("Internal error: the entity is a declaration, but doesn't expose declaration information.");

    CLog log;

    // Stream the case entity
    const sdv::idl::ICaseEntity* pCaseEntry = GetInterface< sdv::idl::ICaseEntity>(pEntity);
    if (!pCaseEntry)
        throw CCompileException("Internal error: the entity is a case entry, but doesn't expose case entry information.");
    if (pCaseEntry->IsDefault())
    {
        log << "Stream default case switch" << std::endl;
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "// default" << std::endl;
    }
    else
    {
        log << "Stream case switch label " << pCaseEntry->GetLabel() << std::endl;
        rcontext.GetDefCodeStream() << rcontext.GetIndent() << "// case " << pCaseEntry->GetLabel() << ":" << std::endl;
    }
}

bool CDefinitionGenerator::StreamDeclType(CDefinitionContext& rcontext, sdv::IInterfaceAccess* pEntity,
    bool& rbDefinitionStreamed, bool bAnonymousDecl /*= false*/, bool bSkipInitialIndent /*= true*/)
{
    if (!pEntity) return false;
    const sdv::idl::IEntityInfo* pEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pEntity);
    sdv::idl::IDeclarationEntity* pDeclaration = GetInterface<sdv::idl::IDeclarationEntity>(pEntity);
    if (!pEntityInfo || !pDeclaration) return false;

    SCDeclInfo sDeclInfo = GetCDeclTypeStr(pDeclaration->GetDeclarationType(), rcontext.GetScope());
    if (sDeclInfo.eBaseType == sdv::idl::EDeclType::decltype_unknown) // Case label without declaration
        return false;

    // Request the declaration type
    sdv::IInterfaceAccess* pTypeObj = pDeclaration->GetDeclarationType();
    if (!pTypeObj) throw CCompileException("Internal error: expecting a declaration type.");
    sdv::idl::IDeclarationType* pDeclType = pTypeObj->GetInterface<sdv::idl::IDeclarationType>();
    if (!pDeclType) throw CCompileException("Internal error: expecting a declaration type.");
    sdv::idl::EDeclType eDeclType = pDeclType->GetBaseType();
    sdv::IInterfaceAccess* pTypeEntity = pDeclType->GetTypeDefinition();

    // Separate between system type and defined type.
    if (pTypeEntity)
    {
        // Deal with unnamed definitions
        const sdv::idl::IEntityInfo* pTypeEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pTypeEntity);
        if (!pTypeEntityInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
        const sdv::idl::IDefinitionEntity* pDefinition = GetInterface<sdv::idl::IDefinitionEntity>(pTypeEntity);
        if (pDefinition && pDefinition->IsUnnamed())
        {
            StreamDefinition(rcontext, pTypeEntity, true, bAnonymousDecl);
            rbDefinitionStreamed = true;
        }
        else
        {
            const sdv::idl::IEntityInfo* pTypeInfo = GetInterface<sdv::idl::IEntityInfo>(pTypeEntity);
            if (!pTypeInfo) throw CCompileException("Internal error: the entity doesn't expose information.");
            std::string ssTypeName = GetRelativeScopedName(pTypeInfo->GetScopedName(), rcontext.GetScope());
            if (ssTypeName.empty()) throw CCompileException("Internal error: the intity doesn't have a name.");
            if (!bSkipInitialIndent) rcontext.GetDefCodeStream() << rcontext.GetIndent();
            rcontext.GetDefCodeStream() << ssTypeName;
        }

        // If the type is an interface, add a pointer to the type
        if (eDeclType == sdv::idl::EDeclType::decltype_interface)
            rcontext.GetDefCodeStream() << "*";
    }
    else
    {
        if (!bSkipInitialIndent) rcontext.GetDefCodeStream() << rcontext.GetIndent();
        rcontext.GetDefCodeStream() << sDeclInfo.ssDeclType;
    }
    return true;
}

void CDefinitionGenerator::ProcessUnionJointContainerForSwitchVar(CDefinitionContext& rcontext,
    sdv::IInterfaceAccess* pSwitchVarEntity, sdv::IInterfaceAccess* pContainerEntity)
{
    // Add the container as friend to the switch variable structure
    const sdv::idl::IEntityInfo* pContainerEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pContainerEntity);
    if (!pContainerEntityInfo) return;
    const sdv::idl::IEntityInfo* pSwitchVarEntityInfo = GetInterface<sdv::idl::IEntityInfo>(pSwitchVarEntity);
    if (!pSwitchVarEntityInfo) return;
    if (pContainerEntity == rcontext.GetDefEntity()) return;    // Container and context are identical

    CLog log;
    log << "Stream switch var processing for " <<  pSwitchVarEntityInfo->GetName() << std::endl;
    log << "Adding friend " << pContainerEntityInfo->GetScopedName() << std::endl;

    rcontext.AddFriend(pContainerEntityInfo->GetScopedName());

    // Check for the friend class to exist in history (has it been streamed already?) If not, add a forward declaration entry.
    if (m_setHistory.find(pContainerEntityInfo->GetScopedName()) == m_setHistory.end())
        m_setForwardDecl.insert(pContainerEntityInfo->GetScopedName());
}

void CDefinitionGenerator::ProcessUnionInContainerContext(CDefinitionContext& rcontext, std::string rssMemberScopeUnionDecl,
    std::string rssMemberScopeSwitchVar, sdv::IInterfaceAccess* pUnionDef,
    const std::vector<SArrayIterationInfo>& rvecArrayIndices /*= std::vector<SArrayIterationInfo>()*/)
{
    // Fill in constructor and destructor code
    if (!pUnionDef) return;
    sdv::idl::IEntityInfo* pEntityInfo = pUnionDef->GetInterface<sdv::idl::IEntityInfo>();
    if (!pEntityInfo) return;
    sdv::idl::IUnionEntity* pUnionInfo = pUnionDef->GetInterface<sdv::idl::IUnionEntity>();
    if (!pUnionInfo) return;
    sdv::idl::IDefinitionEntity* pDefinition = pUnionDef->GetInterface<sdv::idl::IDefinitionEntity>();
    if (!pDefinition) return;
    sdv::idl::IEntityIterator* pChildrenIterator = pDefinition->GetChildren();
    if (!pChildrenIterator || !pChildrenIterator->GetCount()) return;

    // If a member variable was provided along, create a prefix from the member variable.
    std::string ssVarPrefix;
    std::string ssSwitchVarName;
    if (!rssMemberScopeUnionDecl.empty())
        ssVarPrefix += rssMemberScopeUnionDecl + ".";
    if (!rssMemberScopeSwitchVar.empty())
        ssSwitchVarName = rssMemberScopeSwitchVar;
    else
        ssSwitchVarName = pEntityInfo->GetName();

    CLog log;
    log << "Streaming union construct for union "<< pEntityInfo->GetScopedName() << std::endl;
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

    // Create a code context for this entity and at it to the current definition context
    auto ptrSwitchCodeContext =
        rcontext.GetOrCreateSwitchCodeContext<SDefinitionSwitchCodeContext>(ssSwitchVarName, ptrSwitchVarContext, rvecArrayIndices);
    std::stringstream& rsstreamConstructHelperImpl = ptrSwitchCodeContext->sstreamConstructHelperImpl;
    std::stringstream& rsstreamCopyConstructHelperImpl = ptrSwitchCodeContext->sstreamCopyConstructHelperImpl;
    std::stringstream& rsstreamMoveConstructHelperImpl = ptrSwitchCodeContext->sstreamMoveConstructHelperImpl;
    std::stringstream& rsstreamDestructHelperImpl = ptrSwitchCodeContext->sstreamDestructHelperImpl;

    // Get the first case statement and use it as default variable.
    //std::string ssInitialVarName;
    sdv::IInterfaceAccess* pFirstCaseEntity = pChildrenIterator->GetEntityByIndex(0);
    if (pFirstCaseEntity && ptrSwitchCodeContext->ssSwitchValue.empty())
    {
        const sdv::idl::IEntityInfo* pFirstCaseEntityInfo = pFirstCaseEntity->GetInterface<sdv::idl::IEntityInfo>();
        const sdv::idl::ICaseEntity* pFirstCaseSwitch = pFirstCaseEntity->GetInterface<sdv::idl::ICaseEntity>();
        if (pFirstCaseEntityInfo && pFirstCaseSwitch)
        {
            //ssInitialVarName = pFirstCaseEntityInfo->GetName();
            if (!pFirstCaseSwitch->IsDefault()) ptrSwitchCodeContext->ssSwitchValue = pFirstCaseSwitch->GetLabel();
        }
    }

    // Stream the construction/destruction functions
    ptrSwitchCodeContext->sstreamCode << std::endl << "// Destruct and construct " << ssSwitchVarName << "..." << std::endl;
    ptrSwitchCodeContext->sstreamCode << "destruct_" << QualifyName(ssSwitchVarName) << "();" << std::endl;
    ptrSwitchCodeContext->sstreamCode << "construct_" << QualifyName(ssSwitchVarName) << "(val);" << std::endl;
    ptrSwitchVarContext->vecUnionDecl.push_back(pEntityInfo->GetName());

    // Initialization and termination code for the constructor and destructor in case of multiple unions
    if (ptrSwitchCodeContext->sstreamConstructorImpl.str().empty())
        ptrSwitchCodeContext->sstreamConstructorImpl << "construct_" << QualifyName(ssSwitchVarName) << "();" << std::endl;
    if (ptrSwitchCodeContext->sstreamDestructorImpl.str().empty())
        ptrSwitchCodeContext->sstreamDestructorImpl << "destruct_" << QualifyName(ssSwitchVarName) << "();" << std::endl;

    // Automatic indentation
    size_t nLocalIndent = 1;
    auto fnLocalIndent = [&]()
    {
        std::string m_ssIndent;
        for (size_t n = 0; n < nLocalIndent; n++)
            m_ssIndent += GetIndentChars();
        return m_ssIndent;
    };

    // Start array iteration if the union was allocated using a single or multi dimensional array
    for (const auto& rsArrayIndex : rvecArrayIndices)
    {
        rsstreamConstructHelperImpl << fnLocalIndent() << "for (uint32_t " << rsArrayIndex.ssArrayIterator << " = 0; " <<
            rsArrayIndex.ssArrayIterator << " < " <<
            GetRelativeScopedName(rsArrayIndex.ssCountExpression, rcontext.GetScope()) << "; " <<
            rsArrayIndex.ssArrayIterator << "++)" << std::endl << fnLocalIndent() << "{" << std::endl;
        rsstreamCopyConstructHelperImpl << fnLocalIndent() << "for (uint32_t " << rsArrayIndex.ssArrayIterator << " = 0; " <<
            rsArrayIndex.ssArrayIterator << " < " <<
            GetRelativeScopedName(rsArrayIndex.ssCountExpression, rcontext.GetScope()) << "; " <<
            rsArrayIndex.ssArrayIterator << "++)" << std::endl << fnLocalIndent() << "{" << std::endl;
        rsstreamMoveConstructHelperImpl << fnLocalIndent() << "for (uint32_t " << rsArrayIndex.ssArrayIterator << " = 0; " <<
            rsArrayIndex.ssArrayIterator << " < " <<
            GetRelativeScopedName(rsArrayIndex.ssCountExpression, rcontext.GetScope()) << "; " <<
            rsArrayIndex.ssArrayIterator << "++)" << std::endl << fnLocalIndent() << "{" << std::endl;
        rsstreamDestructHelperImpl << fnLocalIndent() << "for (uint32_t " << rsArrayIndex.ssArrayIterator << " = 0; " <<
            rsArrayIndex.ssArrayIterator << " < " <<
            GetRelativeScopedName(rsArrayIndex.ssCountExpression, rcontext.GetScope()) << "; " <<
            rsArrayIndex.ssArrayIterator << "++)" << std::endl << fnLocalIndent() << "{" << std::endl;
        nLocalIndent++;
    }

    // Iterate through the children and create a constructor and destructor function for the union.
    rsstreamConstructHelperImpl << fnLocalIndent() << "switch (val)" << std::endl;
    rsstreamConstructHelperImpl << fnLocalIndent() << "{" << std::endl;
    rsstreamCopyConstructHelperImpl << fnLocalIndent() << ssSwitchVarName << " = rvar." << ssSwitchVarName << ";" << std::endl;
    rsstreamCopyConstructHelperImpl << fnLocalIndent() << "switch (rvar." << ssSwitchVarName << ")" << std::endl;
    rsstreamCopyConstructHelperImpl << fnLocalIndent() << "{" << std::endl;
    rsstreamMoveConstructHelperImpl << fnLocalIndent() << ssSwitchVarName << " = rvar." << ssSwitchVarName << ";" << std::endl;
    rsstreamMoveConstructHelperImpl << fnLocalIndent() << "switch (rvar." << ssSwitchVarName << ")" << std::endl;
    rsstreamMoveConstructHelperImpl << fnLocalIndent() << "{" << std::endl;
    rsstreamDestructHelperImpl << fnLocalIndent() << "switch (" << ssSwitchVarName << ")" << std::endl;
    rsstreamDestructHelperImpl << fnLocalIndent() << "{" << std::endl;
    bool bDefaultSet = false;
    for (uint32_t uiIndex = 0; uiIndex < pChildrenIterator->GetCount(); uiIndex++)
    {
        // Get case information
        sdv::IInterfaceAccess* pCaseEntity = pChildrenIterator->GetEntityByIndex(uiIndex);
		if (!pCaseEntity) continue;
        sdv::idl::IEntityInfo* pCaseEntityInfo = pCaseEntity->GetInterface<sdv::idl::IEntityInfo>();
        sdv::idl::ICaseEntity* pCaseLabel = pCaseEntity->GetInterface<sdv::idl::ICaseEntity>();
        sdv::idl::IDeclarationEntity* pCaseDeclaration = pCaseEntity->GetInterface<sdv::idl::IDeclarationEntity>();
        if (!pCaseEntityInfo || !pCaseLabel || !pCaseDeclaration) continue;
        sdv::IInterfaceAccess* pCaseEntityTypeObj = pCaseDeclaration->GetDeclarationType();
        if (!pCaseEntityTypeObj) continue;
        sdv::idl::IDeclarationType* pCaseDeclType = pCaseEntityTypeObj->GetInterface<sdv::idl::IDeclarationType>();
        if (!pCaseDeclType) continue;
        sdv::idl::IEntityInfo* pCaseDeclTypeEntityInfo =
            pCaseDeclType->GetTypeDefinition() ? pCaseDeclType->GetTypeDefinition()->GetInterface<sdv::idl::IEntityInfo>() : nullptr;

        if (pCaseLabel->IsDefault())
        {
            rsstreamConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "default:" << std::endl;
            bDefaultSet = true;
        }
        else
        {
            rsstreamConstructHelperImpl << fnLocalIndent() << "case " << pCaseLabel->GetLabel() << ":" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "case " << pCaseLabel->GetLabel() << ":" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "case " << pCaseLabel->GetLabel() << ":" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "case " << pCaseLabel->GetLabel() << ":" << std::endl;
        }
        nLocalIndent++;
        CGenContext::SCDeclInfo sDeclInfo = GetCDeclTypeStr(pCaseEntityTypeObj, pEntityInfo->GetScopedName(), true);

        switch (pCaseDeclType->GetBaseType())
		{
		case sdv::idl::EDeclType::decltype_short:
		case sdv::idl::EDeclType::decltype_long:
		case sdv::idl::EDeclType::decltype_long_long:
		case sdv::idl::EDeclType::decltype_octet:
		case sdv::idl::EDeclType::decltype_unsigned_short:
		case sdv::idl::EDeclType::decltype_unsigned_long:
		case sdv::idl::EDeclType::decltype_unsigned_long_long:
		case sdv::idl::EDeclType::decltype_float:
		case sdv::idl::EDeclType::decltype_double:
		case sdv::idl::EDeclType::decltype_long_double:
		case sdv::idl::EDeclType::decltype_char:
		case sdv::idl::EDeclType::decltype_char16:
		case sdv::idl::EDeclType::decltype_char32:
		case sdv::idl::EDeclType::decltype_wchar:
		case sdv::idl::EDeclType::decltype_boolean:
		case sdv::idl::EDeclType::decltype_native:
		case sdv::idl::EDeclType::decltype_enum:
        case sdv::idl::EDeclType::decltype_interface_id:
        case sdv::idl::EDeclType::decltype_interface:
        case sdv::idl::EDeclType::decltype_exception_id:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ")  " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ")  " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ")  " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        case sdv::idl::EDeclType::decltype_string:
        case sdv::idl::EDeclType::decltype_u8string:
        case sdv::idl::EDeclType::decltype_u16string:
        case sdv::idl::EDeclType::decltype_u32string:
        case sdv::idl::EDeclType::decltype_wstring:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~string_base();" <<
                std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        case sdv::idl::EDeclType::decltype_pointer:
        {
            //sdv::IInterfaceAccess* pValueTypeObj = pCaseDeclType->GetValueType();
            //std::string ssKeyType = "unknown";
            //if (pValueTypeObj)
            //{
            //    const sdv::idl::IEntityInfo* pValueTypeEntityInfo = pValueTypeObj->GetInterface<sdv::idl::IEntityInfo>();
            //    if (pValueTypeEntityInfo) ssKeyType = GetRelativeScopedName(pValueTypeEntityInfo->GetScopedName(),
            //        pEntityInfo->GetScopedName());
            //}
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~pointer();" <<
                std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        }
        case sdv::idl::EDeclType::decltype_sequence:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~sequence();" <<
                std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        case sdv::idl::EDeclType::decltype_interface_type:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~interface_t();" <<
                std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        case sdv::idl::EDeclType::decltype_any:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~any_t();" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        case sdv::idl::EDeclType::decltype_unknown: // no variable present
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            break;
        default:
            rsstreamConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << ";" << std::endl;
            rsstreamConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << "); " << std::endl;
            rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "new (&" << ssVarPrefix << pCaseEntityInfo->GetName() << ") " <<
                sDeclInfo.ssDeclType << "(std::move(rvar." << ssVarPrefix << pCaseEntityInfo->GetName() << ")); " << std::endl;
            rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break;" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << ssVarPrefix << pCaseEntityInfo->GetName() << ".~" <<
                pCaseDeclTypeEntityInfo->GetName() << "();" << std::endl;
            rsstreamDestructHelperImpl << fnLocalIndent() << "break;" << std::endl;
        }
        nLocalIndent--;
    }
    if (!bDefaultSet)
    {
        rsstreamConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
        rsstreamCopyConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
        rsstreamMoveConstructHelperImpl << fnLocalIndent() << "default:" << std::endl;
        rsstreamDestructHelperImpl << fnLocalIndent() << "default:" << std::endl;
        nLocalIndent++;
        rsstreamConstructHelperImpl << fnLocalIndent() << "break; " << std::endl;
        rsstreamCopyConstructHelperImpl << fnLocalIndent() << "break; " << std::endl;
        rsstreamMoveConstructHelperImpl << fnLocalIndent() << "break; " << std::endl;
        rsstreamDestructHelperImpl << fnLocalIndent() << "break; " << std::endl;
        nLocalIndent--;
    }

    // End switch/case
    rsstreamConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
    rsstreamCopyConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
    rsstreamMoveConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
    rsstreamDestructHelperImpl << fnLocalIndent() << "}" << std::endl;

    // Stop array iteration if the union was allocated using a single or multi dimensional array
    for ([[maybe_unused]] const auto& rsArrayIndex : rvecArrayIndices)
    {
        nLocalIndent--;
        rsstreamConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
        rsstreamCopyConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
        rsstreamMoveConstructHelperImpl << fnLocalIndent() << "}" << std::endl;
        rsstreamDestructHelperImpl << fnLocalIndent() << "}" << std::endl;
    }
}
