#include "entity_base.h"
#include "../parser.h"
#include "../exception.h"
#include "entity_value.h"
#include "declaration_entity.h"
#include "enum_entity.h"
#include "struct_entity.h"
#include "union_entity.h"
#include "typedef_entity.h"
#include "interface_entity.h"
#include "exception_entity.h"
#include <functional>
#include <type_traits>
#include <limits>
#include <functional>

sdv::idl::EDeclType CTypeDeclaration::GetBaseType() const
{
	return m_eBaseType;
}

void CTypeDeclaration::SetBaseType(sdv::idl::EDeclType eBaseType)
{
	m_eBaseType = eBaseType;
}

sdv::u8string CTypeDeclaration::GetTypeString() const
{
	return m_ssOriginalType;
}

void CTypeDeclaration::SetTypeString(const sdv::u8string& rssType)
{
	m_ssOriginalType = rssType;
}

void CTypeDeclaration::AddTypeString(const sdv::u8string& rss)
{
	m_ssOriginalType += rss;
}

sdv::IInterfaceAccess* CTypeDeclaration::GetTypeDefinition() const
{
	return m_ptrOriginalType.get();
}

CEntityPtr CTypeDeclaration::GetTypeDefinitionEntityPtr() const
{
	return m_ptrOriginalType;
}

void CTypeDeclaration::SetTypeDefinitionEntityPtr(const CEntityPtr& rptrDefinition)
{
	m_ptrOriginalType = rptrDefinition;
}

uint32_t CTypeDeclaration::GetFixedLength() const
{
	return m_uiFixedLen;
}

void CTypeDeclaration::SetFixedLength(uint32_t uiFixedLength)
{
	m_uiFixedLen = uiFixedLength;
}

uint32_t CTypeDeclaration::GetDecimals() const
{
	return m_uiDecimals;
}

void CTypeDeclaration::SetDecimals(uint32_t uiDecimals)
{
	m_uiDecimals = uiDecimals;
}

sdv::IInterfaceAccess* CTypeDeclaration::GetValueType() const
{
	return m_ptrValueType.get();
}

void CTypeDeclaration::SetValueTypePtr(const std::shared_ptr<CTypeDeclaration>& rptrValueType)
{
	m_ptrValueType = rptrValueType;
}

sdv::IInterfaceAccess* CTypeDeclaration::GetKeyType() const
{
	return m_ptrKeyType.get();
}

void CTypeDeclaration::SetKeyTypePtr(const std::shared_ptr<CTypeDeclaration>& rptrKeyType)
{
	m_ptrKeyType = rptrKeyType;
}

CEntity::CEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    m_ptrContext(rptrContext), m_ptrParent(ptrParent)
{
    if (!ptrParent)
        throw CCompileException("Internal error: expected a valid parent entity.");
    m_pParser = ptrParent->m_pParser;
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    m_vecDeclTypes = ptrParent->m_vecDeclTypes;
}

CEntity::CEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList) :
    m_ptrContext(rptrContext), m_ptrParent(ptrParent), m_lstLocalTokenList(rlstTokenList)
{
    if (!ptrParent)
        throw CCompileException("Internal error: expected a valid parent entity.");
    m_pParser = ptrParent->m_pParser;
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    m_vecDeclTypes = ptrParent->m_vecDeclTypes;
}

CEntity::CEntity(CParser& rParser, const CContextPtr& rptrContext) :
    m_ptrContext(rptrContext), m_pParser(&rParser), m_ssName("root"), m_vecDeclTypes(g_vecDeclTypes)
{
    // Add additional types based on enabled extensions
    if (rParser.GetEnvironment().InterfaceTypeExtension())
    {
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"interface_id", sdv::idl::EDeclType::decltype_interface_id});
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"interface_t", sdv::idl::EDeclType::decltype_interface_type});
    }
    if (rParser.GetEnvironment().ExceptionTypeExtension())
    {
        m_vecDeclTypes.push_back(TDeclTypeAssoc{ "exception_id", sdv::idl::EDeclType::decltype_exception_id });
    }
    if (rParser.GetEnvironment().PointerTypeExtension())
    {
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"pointer", sdv::idl::EDeclType::decltype_pointer});
    }
    if (rParser.GetEnvironment().UnicodeExtension())
    {
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"char16", sdv::idl::EDeclType::decltype_char16});
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"char32", sdv::idl::EDeclType::decltype_char32});
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"u8string", sdv::idl::EDeclType::decltype_u8string});
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"u16string", sdv::idl::EDeclType::decltype_u16string});
        m_vecDeclTypes.push_back(TDeclTypeAssoc{"u32string", sdv::idl::EDeclType::decltype_u32string});
    }
}

sdv::interface_t CEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IEntityInfo>())
        return static_cast<sdv::idl::IEntityInfo*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IEntityComments>())
        return static_cast<sdv::idl::IEntityComments*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IEntityContext>())
        return static_cast<sdv::idl::IEntityContext*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IForwardDeclarationEntity>() && m_bForwardDeclaration)
        return static_cast<sdv::idl::IForwardDeclarationEntity*>(this);
    return nullptr;
}

sdv::idl::EEntityType CEntity::GetType() const
{
    return sdv::idl::EEntityType::type_unknown;
}

sdv::IInterfaceAccess* CEntity::GetEntity()
{
    if (!m_bForwardDeclaration) return nullptr;
    if (!m_ptrParent) return nullptr;
    std::pair<CEntityPtr, bool> prEntity = m_ptrParent->FindLocal(m_ssName, Get<CDeclarationEntity>() && !Get<CTypedefEntity>() ? true : false);
    return static_cast<sdv::IInterfaceAccess*>(prEntity.first.get());
}

std::string CEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    throw CCompileException("Internal error: unexpected request for declaration type.");
}

sdv::u8string CEntity::GetName() const
{
    return m_ssName;
}

sdv::u8string CEntity::GetScopedName() const
{
    // Do not return the root entity.
    if (IsRootEntity()) return std::string();

    std::string ssScopeName;
    if (m_ptrParent) ssScopeName = m_ptrParent->GetScopedName();

    // Only add the name of the entity if not anonymous (only part of a declaration).
    const CDeclarationEntity* pDeclaration = Get<CDeclarationEntity>();
    if (!pDeclaration || !pDeclaration->IsAnonymous())
    {
        if (!ssScopeName.empty()) ssScopeName += "::";
        ssScopeName += m_ssName;
    }
    return ssScopeName;
}

std::string CEntity::MakeFullScoped(const std::string& rssExpression) const
{
    // Parse the expression
    CTokenList lstTokens = Tokenize(rssExpression, GetContext());

    // Create a new expression
    std::stringstream sstreamNewExpression;

    // Rescope any identifier if there is any...
    std::string ssIdentifier;
    auto fnWriteIdentifier = [&]()
    {
        if (ssIdentifier.empty())
            return;

        // Search for the entity
        CEntityPtr ptrEntity = Find(ssIdentifier);
        if (ptrEntity)
            sstreamNewExpression << ptrEntity->GetScopedName();
        else // Not found; add the identifier as is.
            sstreamNewExpression << ssIdentifier;
        ssIdentifier.clear();
    };

    // Run through all tokens.
    for (const CToken& rtoken : lstTokens)
    {
        switch (rtoken.GetType())
        {
        case ETokenType::token_identifier:
            ssIdentifier += static_cast<std::string>(rtoken);
            break;
        case ETokenType::token_separator:
            if (rtoken == "::")
            {
                ssIdentifier += static_cast<std::string>(rtoken);
                break;
            }
            fnWriteIdentifier();
            sstreamNewExpression << static_cast<std::string>(rtoken);
            break;
        default:
            fnWriteIdentifier();
            sstreamNewExpression << static_cast<std::string>(rtoken);
            break;
        }
    }

    // Any identifiers left?
    fnWriteIdentifier();

    // Done
    return sstreamNewExpression.str();
}

CContextPtr CEntity::GetContext() const
{
    return m_ptrContext;
}

CContextPtr CEntity::GetContext()
{
    return m_ptrContext;
}

CEntityPtr CEntity::GetParentEntity() const
{
    return m_ptrParent;
}

CEntityPtr CEntity::GetParentEntity()
{
    return m_ptrParent;
}

sdv::IInterfaceAccess* CEntity::GetParent() const
{
    auto ptrInterface = GetParentEntity();
    return ptrInterface.get();
}

CEntityPtr CEntity::GetRootEntity() const
{
    // The root doesn't have a parent.
    // Iterate through the parents until the root parent is returned.
    if (m_ptrParent) return m_ptrParent->GetRootEntity();

    // Get the root -> change the return type.
    // NOTE: shared_from_this returns a smart pointer to a const object. Since a smart pointer to a non-const object cannot be
    // casted to a smart pointer to a const object, we cannot use the shared_from_this function on the current this object.
    // Instead, get the non-const version of this first before calling shared_from_this.
    return const_cast<CEntity*>(this)->shared_from_this();
}

CEntityPtr CEntity::GetRootEntity()
{
    // The root doesn't have a parent.
    // Iterate through the parents until the root parent is returned.
    return !m_ptrParent ? shared_from_this() : m_ptrParent->GetRootEntity();
}

CEntityPtr CEntity::GetResolvedEntity() const
{
    // Is the entity a typedef, get the assigned type before finding the child
    CEntityPtr ptrResolvedEntity = const_cast<CEntity*>(this)->shared_from_this();
    do
    {
        // Check whether the type is a typedef declaration
        const CDeclarationEntity* pDeclaration = ptrResolvedEntity->Get<CDeclarationEntity>();
        if (!pDeclaration) break;

        // Type is a typedef, get the assigned type
        ptrResolvedEntity = pDeclaration->GetTypeEntity();
    } while (ptrResolvedEntity);

    // In case the typedef is not referring to an entity, but to a basic type, return the typedef itself.
    return ptrResolvedEntity ? ptrResolvedEntity : const_cast<CEntity*>(this)->shared_from_this();
}

CEntityPtr CEntity::GetResolvedEntity()
{
    // Is the entity a typedef, get the assigned type before finding the child
    CEntityPtr ptrResolvedEntity = shared_from_this();
    do
    {
        // Check whether the type is a typedef declaration
        if (ptrResolvedEntity->GetType() != sdv::idl::EEntityType::type_typedef) break;
        const CDeclarationEntity* pDeclaration = ptrResolvedEntity->Get<CDeclarationEntity>();
        if (!pDeclaration) break;

        // Type is a typedef, get the assigned type
        ptrResolvedEntity = pDeclaration->GetTypeEntity();
    } while (ptrResolvedEntity);

    // In case the typedef is not referring to an entity, but to a basic type, return the typedef itself.
    return ptrResolvedEntity ? ptrResolvedEntity : shared_from_this();
}

void CEntity::SetCommentTokens(const CTokenList& rlstComments, bool bPreceeding /*= true*/)
{
    std::stringstream sstreamComments;

    // Detect the type of comments, remove the indentation and create a (multi-line) comment string.
    uint32_t uiFlags = 0;
    for (const CToken& rComment : rlstComments)
    {
        uint32_t uiLocalFlags = 0;
        std::string ssComment = static_cast<std::string>(rComment);
        bool bEmpty = sstreamComments.str().empty();

        if (ssComment.substr(0, 2) == "/*") // C-stlye comment
        {
            uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style);
            size_t nPos = 2;
            if (ssComment.substr(0, 4) == "/**<") // C-style Javadoc comment following a statement
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc_post);
                nPos = 4;
            }
            else if (ssComment.substr(0, 4) == "/*!<") // C-style QT comment following statement
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt_post);
                nPos = 4;
            }
            else if (ssComment.substr(0, 3) == "/**") // C-style Javadoc comment
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc);
                nPos = 3;
            }
            else if (ssComment.substr(0, 3) == "/*!") // C-style QT comment
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt);
                nPos = 3;
            }

            // Check for a proper finalization of the comment
            if (ssComment.length() < 4 || ssComment.substr(ssComment.length() - 2) != "*/")
                continue;   // ERROR - not a valid comment

            bool bInitial = true;
            while (nPos < ssComment.length() - 3)
            {
                // Skip spaces and tabs
                if (ssComment[nPos] == ' ' || ssComment[nPos] == '\t')
                {
                    while (ssComment[nPos] == ' ' || ssComment[nPos] == '\t')
                        nPos++;
                    continue;
                }

                // Treat initial and following lines differently
                if (bInitial)
                {
                    // If the next character is a newline, skip the newline and continue with the next line
                    if (ssComment[nPos] == '\n')
                    {
                        nPos++;
                        continue;
                    }
                    if (ssComment[nPos] == '\r' && ssComment[nPos + 1] == '\n')
                    {
                        nPos += 2;
                        continue;
                    }
                }

                // If available, skip asterisk and one space
                if (ssComment[nPos] == '*')
                {
                    nPos++;
                    if (ssComment[nPos] == ' ') nPos++;
                }

                // If this is the last line, skip the line.
                if (nPos == ssComment.length() - 2) continue;

                // Read the line
                size_t nStart = nPos;
                size_t nStop = nStart;
                while (true)
                {
                    if (ssComment[nPos] == '\n')
                    {
                        if (!sstreamComments.str().empty()) sstreamComments << std::endl;
                        sstreamComments << ssComment.substr(nStart, nStop - nStart + 1);
                        nPos++;
                        break;
                    }
                    if (ssComment[nPos] == '\r' && ssComment[nPos + 1] == '\n')
                    {
                        if (!sstreamComments.str().empty()) sstreamComments << std::endl;
                        sstreamComments << ssComment.substr(nStart, nStop - nStart + 1);
                        nPos += 2;
                        break;
                    }
                    if (nPos >= ssComment.length() - 3)
                    {
                        if (nPos != nStart)
                        {
                            if (!sstreamComments.str().empty()) sstreamComments << std::endl;
                            sstreamComments << ssComment.substr(nStart, nStop - nStart + 1);
                        }
                        break;
                    }

                    // Increase position
                    nPos++;

                    // Do not include any whitespace
                    if (!std::isspace(ssComment[nPos]))
                        nStop = nPos;
                }
                bInitial = false;
            }
        }
        else if (ssComment.substr(0, 2) == "//") // C++-stlye comment
        {
            uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style);
            size_t nPos = 2;
            if (ssComment.substr(0, 4) == "///<") // C++-style Javadoc comment following statement
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_javadoc_post);
                nPos = 4;
            }
            else if (ssComment.substr(0, 4) == "//!<") // C++-style QT comment following statement
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_qt_post);
                nPos = 4;
            }
            else if (ssComment.substr(0, 3) == "///") // C++-style Javadoc comment
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_javadoc);
                nPos = 3;
            }
            else if (ssComment.substr(0, 3) == "//!") // C++-style QT comment
            {
                uiLocalFlags = static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style_qt);
                nPos = 3;
            }

            // If available, skip one space
            if (ssComment[nPos] == ' ') nPos++;

            // Do not include any whitespace at the end
            size_t nLen = ssComment.length() - nPos;
            while (nLen > 0 && std::isspace(ssComment[nPos + nLen - 1]))
                nLen--;

            // Copy the line
            if (nLen)
            {
                if (!sstreamComments.str().empty()) sstreamComments << std::endl;
                sstreamComments << ssComment.substr(nPos, nLen);
            }
        }
        else
            continue;   // Not a normal comment - ERROR?

        // Determine on how to combine the flags. For the first comment, the flags are all cleared.
        if (!bEmpty)
        {
            // Javadoc favors over QT comment.
            if (((uiLocalFlags | uiFlags) & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask)) ==
                (static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
                static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc)))
                uiFlags = (uiFlags & ~static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask)) |
                static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc);

            // Formatted comment favors over unformatted comment.
            if ((uiLocalFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask)) &&
                !(uiFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask)))
                uiFlags = uiFlags | (uiLocalFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask));

            // C-Style favors over C++-style comment.
            if ((uiLocalFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style)) !=
                (uiFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style)))
                uiFlags = (uiFlags & ~static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style)) |
                    static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style);

            // Preceding favors over succeeding comment.
            if ((uiLocalFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding)) !=
                (uiFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding)))
                uiFlags = uiFlags & ~static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding);
        }
        else // First comment or only C-style comment available.
            uiFlags = uiLocalFlags;
    }

    // Are there any comments to store?
    if (sstreamComments.str().empty()) return;

    // Replace the comment string if the existing comment string doesn't hold any formatted comments
    if ((m_uiCommentFlags & static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_mask)) == 0)
    {
        m_ssComments = sstreamComments.str();
        m_uiCommentFlags = uiFlags;
    }

    // Is this a succeeding comment? If so, set the flag and remove the comments from the parser
    if (!bPreceeding)
    {
        m_uiCommentFlags |= static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding);
        if (!m_pParser)
            throw CCompileException("Internal error: no parser available.");
        m_pParser->SkipAdjacentComments();
    }
}

CTokenList CEntity::GetPreCommentTokenList()
{
    CToken token = PeekToken();
    uint32_t uiLine = token.GetLine();
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    CTokenList lstComments = m_pParser->GetComments();
    CTokenList lstPreComments;
    for (CTokenList::reverse_iterator itComment = lstComments.rbegin(); itComment != lstComments.rend(); ++itComment)
    {
        // Check for a valid context
        if (!token.GetContext().get()) break;

        // Are the comments within the same context?
        if (*itComment->GetContext().get() != *token.GetContext().get()) continue;

        // Is this comment belonging to the entity?
        if (itComment->GetEndLine() < uiLine - 1) break;

        // Insert the comment before the other comments of this entity.
        lstPreComments.push_front(*itComment);
        uiLine = itComment->GetLine();
    }
    return lstPreComments;
}

void CEntity::ProcessPostCommentTokenList(uint32_t uiLine)
{
    uint32_t uiLineTemp = uiLine;
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    CTokenList lstComments = m_pParser->GetComments();
    CTokenList lstPostComments;
    for (CTokenList::iterator itComment = lstComments.begin(); itComment != lstComments.end(); ++itComment)
    {
        // Are the comments within the same context?
        if (*itComment->GetContext().get() != *GetContext().get()) continue;

        // Is this comment belonging to the entity?
        if (itComment->GetLine() > uiLineTemp) break;

        // Insert the comment after the other comments of this entity.
        lstPostComments.push_back(*itComment);
        uiLineTemp = itComment->GetEndLine() + 1;
    }
    if (!lstPostComments.empty())
        SetCommentTokens(lstPostComments, false);
}

sdv::u8string CEntity::GetComments(uint32_t& ruiFlags) const
{
    ruiFlags = m_uiCommentFlags;
    return m_ssComments;
}

sdv::idl::IEntityContext::ELocation CEntity::GetLocation() const
{
    return m_ptrContext ? m_ptrContext->GetLocation() : sdv::idl::IEntityContext::ELocation::source;
}

sdv::u8string CEntity::GetSourcePath() const
{
    if (!m_ptrContext) return sdv::u8string();
    return m_ptrContext->Source().GetPathRef().generic_u8string();
}

void CEntity::GetPosition(uint32_t& ruiLineBegin, uint32_t& ruiColBegin, uint32_t& ruiLineEnd, uint32_t& ruiColEnd)
{
    ruiLineBegin = m_uiLineBegin;
    ruiColBegin = m_uiColBegin;
    ruiLineEnd = m_uiLineEnd;
    ruiColEnd = m_uiColEnd;
}

void CEntity::CalcHash(CHashObject& rHash) const
{
    // Add the name to the hash
    rHash << GetScopedName();
}

const CParser& CEntity::GetParserRef() const
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    return *m_pParser;
}

CParser& CEntity::GetParserRef()
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    return *m_pParser;
}

CToken CEntity::GetToken()
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    CToken token;
    // When there is a tokenlist, only process the tokens from the tokenlist; do not ask the parser.
    if (!m_lstLocalTokenList.empty())
    {
        token = m_lstLocalTokenList.Current();
        ++m_lstLocalTokenList;
    } else
        token = m_pParser->GetToken();
    return token;
}

CToken CEntity::GetLastValidToken() const
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    return m_pParser->GetLastValidToken();
}

CToken CEntity::PeekToken(size_t nIndex /*= 0*/)
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    CToken token;
	// When there is a tokenlist, only process the tokens from the tokenlist; do not ask the parser.
	if (!m_lstLocalTokenList.empty())
        token = m_lstLocalTokenList.Current(nIndex);
    else
        token = m_pParser->PeekToken(nIndex);
    return token;
}

void CEntity::PrependToken(CToken& rToken)
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    if (!rToken.GetContext())
        rToken.SetContext(m_ptrContext);

    // When there is a tokenlist, only process the tokens from the tokenlist; do not ask the parser.
	if (!m_lstLocalTokenList.empty())
        m_lstLocalTokenList.insert(rToken);
    else
        m_pParser->PrependToken(rToken);
}

CEntityPtr CEntity::Find(const std::string& rssScopedName, bool bCheckParent /*= true*/) const
{
    if (rssScopedName.empty()) return nullptr;

    size_t nScopeSeparator = rssScopedName.find("::");
    size_t nMemberSeparator = rssScopedName.find(".");
    size_t nSeparator = std::min(nScopeSeparator, nMemberSeparator);

    // Special case if the separator at the beginning (root).
    if (nScopeSeparator == 0)
    {
        // Execute the request through the root.
        CModuleEntity* pRootModule = GetRootEntity()->Get<CModuleEntity>();
        if (!pRootModule) throw CCompileException("Internal error: no root module available.");
        return pRootModule->Find(rssScopedName.substr(nSeparator + 2));
    }

    // Check the current map of child entities for the fitting scoped name.
    CEntityPtr ptrChild = FindLocal(rssScopedName.substr(0, nSeparator), true).first;
    if (!ptrChild) ptrChild = FindLocal(rssScopedName.substr(0, nSeparator), false).first;

    // Check for any anonymous children, if they exist, forward the request to them.
    // NOTE: If returning, the complete scoped name has been processed, meaning, if a child is returned, it is the target entity.
    if (!ptrChild)
    {
        for (CEntityPtr ptrChildTemp : m_vecChildren)
        {
            const CDeclarationEntity* pDeclaration = ptrChildTemp->Get<CDeclarationEntity>();
            if (pDeclaration && pDeclaration->IsAnonymous())
            {
                // Get the type entity of the declaration (resolve any typedefs).
                CEntityPtr ptrTypeEntity = pDeclaration->GetTypeEntity()->GetResolvedEntity();
                if (ptrTypeEntity)
                    ptrChild = ptrTypeEntity->Find(rssScopedName, false);
                if (ptrChild) return ptrChild;
            }
        }
    }

    // If a child was found find the next part of the scoped string
    if (ptrChild && nSeparator != std::string::npos)
    {
        // Definitions are separated by the scope operator.
        if (nScopeSeparator != std::string::npos)
        {
            // Resolve the typedef before accessing children.
            ptrChild = ptrChild->GetResolvedEntity();
            const CDefinitionEntity* pDefinition = ptrChild->Get<CDefinitionEntity>();
            ptrChild = pDefinition ? ptrChild->Find(rssScopedName.substr(nSeparator + 2), false) : nullptr;
        } else // Declarations are separated by the member operator.
        {
            // Find the member
            ptrChild = ptrChild->FindMember(rssScopedName.substr(nSeparator + 1));
        }
    }
    if (ptrChild) return ptrChild;

    // If there is no child, is parent checking allowed?
    if (!bCheckParent) return nullptr;

    // Check the parent until there is a child fitting or there is no parent any more.
    if (!ptrChild && m_ptrParent)
        ptrChild = m_ptrParent->Find(rssScopedName, bCheckParent);

    // Return the result
    return ptrChild;
}

CValueNodePtr CEntity::FindValue(const CTokenList& rlstScopedNameInclMembers) const
{
    // Get the value of the scoped name.
    std::pair<std::string, CEntityPtr> prScopedName = ProcessScopedName(rlstScopedNameInclMembers);
    if (!prScopedName.second)
        throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Entity not found.");
    CValueNodePtr ptrValue = prScopedName.second->ValueRef();
    if (!ptrValue) throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Internal error: entity doesn't have a value.");

    // Process any arrays and or members....
    while (true)
    {
        if (!rlstScopedNameInclMembers.Current()) break;

        // Deal with arrays first.
        if (rlstScopedNameInclMembers.Current() == "[")     // Array node
        {
            // The value node pointer should be an array pointer
            CArrayValueNode* pArrayValueNode = ptrValue->Get<CArrayValueNode>();
            if (!pArrayValueNode)
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Unexpected token '['.");

            ++rlstScopedNameInclMembers;    // Skip bracket

            // Resolve the value
            std::pair<CConstVariant, bool> prIndex = ProcessNumericExpression(rlstScopedNameInclMembers);
            if (prIndex.second)
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Array indices must be a const value.");
            size_t nIndex = prIndex.first.Get<size_t>();

            // Expect closing bracket
            if (rlstScopedNameInclMembers.Current() != "]")
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Expecting '['.");
            ++rlstScopedNameInclMembers;    // Skip bracket

            // Get the value node...
            ptrValue = (*pArrayValueNode)[nIndex];
            if (!ptrValue)
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Invalid index for array.");
        } else if (rlstScopedNameInclMembers.Current() == ".")  // Compound node
        {
            // The value node pointer should be a compound value pointer
            const CCompoundTypeValueNode* pCompoundValueNode = ptrValue->Get<CCompoundTypeValueNode>();
            if (!pCompoundValueNode)
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Unexpected token '.'.");


            ++rlstScopedNameInclMembers;    // Skip dot

            // Expecting an identifier
            if (rlstScopedNameInclMembers.Current().GetType() != ETokenType::token_identifier) return nullptr;

            // Get the member
            ptrValue = pCompoundValueNode->Member(rlstScopedNameInclMembers.Current());
            if (!ptrValue)
                throw CCompileException(rlstScopedNameInclMembers.LastValid(), "Member '",
                    static_cast<std::string>(rlstScopedNameInclMembers.Current()), "' not found.");
            ++rlstScopedNameInclMembers; // Skip identifier
        } else
            break;  // Done....
    }

    return ptrValue;
}

CValueNodePtr CEntity::FindValue(const std::string& rssScopedNameInclMembers) const
{
    try
    {
        return FindValue(Tokenize(rssScopedNameInclMembers, GetContext()));
    } catch (const sdv::idl::XCompileError&)
    {}
    return nullptr;
}

CConstVariant CEntity::FindValueVariant(const std::string& rssScopedNameInclMembers) const
{
    const CSimpleTypeValueNode* pSimpleValueNode = FindValue<CSimpleTypeValueNode>(rssScopedNameInclMembers);
    return pSimpleValueNode ? pSimpleValueNode->Variant() : CConstVariant();
}

uint64_t CEntity::GetId() const
{
    CHashObject hash;
    CalcHash(hash);
    return hash.GetHash();
}

std::pair<std::string, CEntityPtr> CEntity::ProcessScopedName(bool bNoSearchError /*= false*/)
{
    // Get the scoped name
    // NOTE: a scoped name could start with the scope operator '::' identifying a root based type.
    CToken token = GetToken();
    std::string ssName = token;
    while (true)
    {
        if (ssName.back() != ':')
        {
            token = GetToken();
            if (token != "::")
            {
                PrependToken(token);
                break;
            }

            ssName += "::";
        }

        token = GetToken();
        if (token.GetType() != ETokenType::token_identifier)
            throw CCompileException(token, "Expecting an identifier.");
        ssName += static_cast<std::string>(token);
    }

    // Find the corresponding type (resolve typedefs).
    CEntityPtr ptrEntity = Find(ssName, true);
    if (!bNoSearchError && !ptrEntity)
        throw CCompileException(token, "Could not find the definition of '", ssName, "'.");
    return std::make_pair(ssName, ptrEntity);
}

std::pair<std::string, CEntityPtr> CEntity::ProcessScopedName(const CTokenList& rlstExpression,
    bool bNoSearchError /*= false*/) const
{
    if (rlstExpression.End())
        throw CCompileException("Internal error: trying to retrieve scoped name from empty tokenlist.");

    // Get the scoped name
    // NOTE: a scoped name could start with the scope operator '::' identifying a root based type.njjot
    std::string ssName = rlstExpression.Current();
    while (true)
    {
        ++rlstExpression;
        if (ssName.back() != ':')
        {
            if (rlstExpression.Current() != "::")
                break;

            ssName += "::";
            ++rlstExpression;
        }

        if (rlstExpression.Current().GetType() != ETokenType::token_identifier)
            throw CCompileException(rlstExpression.LastValid(), "Expecting an identifier.");
        ssName += static_cast<std::string>(rlstExpression.Current());
    }

    // Find the corresponding type.
    // Find the corresponding type.
    CEntityPtr ptrEntity = Find(ssName);
    if (!bNoSearchError && !ptrEntity)
        throw CCompileException(rlstExpression.LastValid(), "Could not find the definition of '", ssName, "'.");

    // In case the entity is a typedef, const or a variable declaration, trigger the processing of the assignment for this
    // entity, so that it contains a value which can be accessed by the function calling the ProcessScopedName. The call for
    // assignment processing will either do nothing (when the processing was done before already), cause an exception (if the
    // call to ProcessScopedName was triggered by the processing of the assignment - circular reference) or trigger processing
    // (if the processing hadn't been done yet, but is needed for the further processing of the assignment calling this
    // ProcessScopedName function).
    CDeclarationEntity* pBasicType = ptrEntity ? ptrEntity->Get<CDeclarationEntity>() : nullptr;
    if (pBasicType) pBasicType->PostProcess();

    // Return result (resolve any typedef)
    return std::make_pair(ssName, ptrEntity);
}

std::pair<CConstVariant, bool> CEntity::ProcessNumericExpression(const CTokenList& rlstExpression,
    uint32_t uiPrecedence /*= 100*/) const
{
    if (rlstExpression.End()) throw CCompileException("Expecting expression.");

    // Token can be either a left parenthesis, a unary operator (precedence 0), an identifier (defined, macro or unknown), or a
    // number.
    std::pair<CConstVariant, bool> prLValue{0, false};
    if (rlstExpression.Current() == "(")
    {
        ++rlstExpression;

        // Calculate the content between the parenthesis.
        prLValue = ProcessNumericExpression(rlstExpression);

        // Expecting a closing parenthesis
        if (rlstExpression.End() || rlstExpression.Current() != ")")
            throw CCompileException(rlstExpression.LastValid(), "Expecting a right parenthesis.");

        ++rlstExpression;
    } else if (rlstExpression.Current() == "-")  // unary minus
    {
        ++rlstExpression;

        prLValue = ProcessNumericExpression(rlstExpression, 0);
        prLValue.first = -prLValue.first;
    }
    else if (rlstExpression.Current() == "+") // unary plus
    {
        ++rlstExpression;
        prLValue = ProcessNumericExpression(rlstExpression, 0);
    }
    else if (rlstExpression.Current() == "!") // logical not
    {
        ++rlstExpression;
        prLValue = ProcessNumericExpression(rlstExpression, 0);
        prLValue.first = !prLValue.first;
    }
    else if (rlstExpression.Current() == "~") // bitwise not
    {
        ++rlstExpression;
        prLValue = ProcessNumericExpression(rlstExpression, 0);
        prLValue.first = ~prLValue.first;
    }
    else if (rlstExpression.Current() == "::" || rlstExpression.Current().GetType() == ETokenType::token_identifier)
    {
        // Process the identifier
        std::pair<std::string, CEntityPtr> prIdentifier = ProcessScopedName(rlstExpression);

        // Assignments of identifier must be defined first. Type must be of CDeclarationEntity.
        CDeclarationEntity* pEntity = prIdentifier.second->Get<CDeclarationEntity>();
        if (!pEntity) throw CCompileException(rlstExpression.LastValid(), "Identifier is not a definition.");

        // If the identifier is not const, the result is dynamic
        if (!pEntity->IsReadOnly())
            prLValue.second = true;

        // Get the value
        CValueNodePtr ptrValue = pEntity->ValueRef();
        if (!ptrValue) throw CCompileException(rlstExpression.LastValid(), "Identifier doesn't have any value.");

        // If the value is an array, get the indices.
        while (ptrValue->IsArray())
        {
            CArrayValueNode* pArrayValue = ptrValue->Get<CArrayValueNode>();
            if (!pArrayValue)
                throw CCompileException(rlstExpression.LastValid(), "Internal error: the value has array flag set, but is not an array value.");

            if (!rlstExpression.End() && rlstExpression.Current() != "[")
                throw CCompileException(rlstExpression.LastValid(), "Expecting a left square bracket '[', value is an array.");
            ++rlstExpression;

            // Get the value...
            std::pair<CConstVariant, bool> prIndex = ProcessNumericExpression(rlstExpression);

            // The value must be of integral type
            if (!prIndex.first.IsIntegral())
                throw CCompileException(rlstExpression.LastValid(), "Only integral data types are supported for the array size.");

            // If the value is dynamic, set the result to dynamic.
            if (prIndex.second) prLValue.second = true;

            // If the result is dynamic get the initial index
            ptrValue = (*pArrayValue)[prIndex.first.Get<int64_t>()];

            if (!rlstExpression.End() && rlstExpression.Current() != "]")
                throw CCompileException(rlstExpression.LastValid(), "Expecting a right square bracket ']'.");
            ++rlstExpression;
        }

        const CSimpleTypeValueNode* pValue = ptrValue->Get<CSimpleTypeValueNode>();
        if (!pValue)
            throw CCompileException(rlstExpression.LastValid(),
                "Internal error: the entity is defined as declaration type entity, but the value is not a declaration type"
                " value.");
        prLValue.first = pValue->Variant();
    } else if (rlstExpression.Current().GetType() == ETokenType::token_literal)
    {
        prLValue.first = rlstExpression.Current().ValueRef();
        ++rlstExpression;
    }
    else
        throw CCompileException(rlstExpression.LastValid(), "Unexpected token '", static_cast<std::string>(rlstExpression.Current()), "'.");

    // Run until all operations at this or any higher precedence have been executed
    bool bDone = false;
    while (!bDone)
    {
        // Done when the expression is terminated using one of the separators or no more tokens are available.
        if (rlstExpression.End() ||
           rlstExpression.Current() == "}" ||
           rlstExpression.Current() == "]" ||
           rlstExpression.Current() == "," ||
           rlstExpression.Current() == ";")
        {
            bDone = true;
            continue;
        }

        // If right parenthesis, leave the parenthesis and we're done.
        if (rlstExpression.Current() == ")")
        {
            bDone = true;
            continue;
        }

        // Conditionally calculate the result based on the supplied operator function and the precedence level.
        auto fnCalculate = [&](std::function<CConstVariant(CConstVariant&, CConstVariant&)> fnCalculation,
            uint32_t uiOperatorPrecedence)
        {
            // If operator has höhere oder gleiche precedence, replace operator and we're done.
            if (uiOperatorPrecedence >= uiPrecedence)
            {
                bDone = true;
                return;
            }

            // Skip the operator
            ++rlstExpression;

            // If operator has niedriger precedence, get next token and calculate result.
            std::pair<CConstVariant, bool> prRValue = ProcessNumericExpression(rlstExpression, uiOperatorPrecedence);

            // Calculate the result.
            prLValue.first = fnCalculation(prLValue.first, prRValue.first);
        };

        // Check for the operator and calculate the result
        const CToken& rtoken = rlstExpression.Current();
        if (rtoken == "*")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 * rvar2; }, 1);   continue; }
        if (rtoken == "/")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 / rvar2; }, 1);   continue; }
        if (rtoken == "+")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 + rvar2; }, 2);   continue; }
        if (rtoken == "-")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 - rvar2; }, 2);   continue; }
        if (rtoken == "<")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 < rvar2; }, 4);   continue; }
        if (rtoken == "<=") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 <= rvar2; }, 4);  continue; }
        if (rtoken == ">")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 > rvar2; }, 4);   continue; }
        if (rtoken == ">=") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 >= rvar2; }, 4);  continue; }
        if (rtoken == "==") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 == rvar2; }, 5);  continue; }
        if (rtoken == "!=") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 != rvar2; }, 5);  continue; }
        if (rtoken == "%")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 % rvar2; }, 1);   continue; }
        if (rtoken == "<<") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 << rvar2; }, 3);  continue; }
        if (rtoken == ">>") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 >> rvar2; }, 3);  continue; }
        if (rtoken == "&")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 & rvar2; }, 6);   continue; }
        if (rtoken == "^")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 ^ rvar2; }, 7);   continue; }
        if (rtoken == "|")  { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 | rvar2; }, 8);   continue; }
        if (rtoken == "&&") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 && rvar2; }, 9);  continue; }
        if (rtoken == "||") { fnCalculate([](const CConstVariant& rvar1, const CConstVariant& rvar2){ return rvar1 || rvar2; }, 10); continue; }

        // If not an operator, this is an error
        throw CCompileException(rlstExpression.LastValid(), "Expecting operator");
    };

    return prLValue;
}

std::pair<CConstVariant, bool> CEntity::ProcessStringExpression(const CTokenList& rlstExpression) const
{
    std::pair<CConstVariant, bool> prValue{0, false};

    if (rlstExpression.End()) throw CCompileException("Expecting expression.");

    // Run until all the strings are read (they are joined together).
    // If identifiers are used... this must be one single assignment.
    bool bDone = false;
    bool bInitial = true;
    bool bIdentifier = false;
    while (!bDone)
    {
        if (rlstExpression.End() ||
                   rlstExpression.Current() == "}" ||
                   rlstExpression.Current() == "]" ||
                   rlstExpression.Current() == "," ||
                   rlstExpression.Current() == ";")
        {
            if (bInitial) throw CCompileException(rlstExpression.LastValid(), "Missing assignment.");
            bDone = true;
            continue;
        }

        if (rlstExpression.Current() == "::" || rlstExpression.Current().GetType() == ETokenType::token_identifier)
        {
            if (!bInitial)
                throw CCompileException(rlstExpression.LastValid(), "Cannot mix strings and identifiers.");

            // Process the identifier
            std::pair<std::string, CEntityPtr> prIdentifier = ProcessScopedName(rlstExpression);

            // Assignments of identifier must be defined first. Type must be of CDeclarationEntity.
            CDeclarationEntity* pEntity = prIdentifier.second->Get<CDeclarationEntity>();
            if (!pEntity) throw CCompileException(rlstExpression.LastValid(), "Identifier is not a definition.");

            // If the identifier is not const, the result is dynamic
            if (!pEntity->IsReadOnly())
                prValue.second = true;

            // Get the value
            CValueNodePtr ptrValue = pEntity->ValueRef();
            if (!ptrValue) throw CCompileException(rlstExpression.LastValid(), "Identifier doesn't have any value.");

            // If the value is an array, get the indices.
            while (ptrValue->IsArray())
            {
                CArrayValueNode* pArrayValue = ptrValue->Get<CArrayValueNode>();
                if (!pArrayValue)
                    throw CCompileException(rlstExpression.LastValid(), "Internal error: the value has array flag set, but is not an array value.");

                if (!rlstExpression.End() && rlstExpression.Current() != "[")
                    throw CCompileException(rlstExpression.LastValid(), "Expecting a left square bracket '[', value is an array.");
                ++rlstExpression;

                // Get the value...
                std::pair<CConstVariant, bool> prIndex = ProcessNumericExpression(rlstExpression);

                // The value must be of integral type
                if (!prIndex.first.IsIntegral())
                    throw CCompileException(rlstExpression.LastValid(), "Only integral data types are supported for the array size.");

                // If the value is dynamic, set the result to dynamic.
                if (prIndex.second) prValue.second = true;

                ptrValue = (*pArrayValue)[prIndex.first.Get<int64_t>()];

                if (!rlstExpression.End() && rlstExpression.Current() != "]")
                    throw CCompileException(rlstExpression.LastValid(), "Expecting a right square bracket ']'.");
                ++rlstExpression;
            }

            const CSimpleTypeValueNode* pValue = ptrValue->Get<CSimpleTypeValueNode>();
            if (!pValue)
                throw CCompileException(rlstExpression.LastValid(),
                    "Internal error: the entity is defined as declaration type entity, but the value is not a declaration type"
                    " value.");
            prValue.first = pValue->Variant();
            bIdentifier = true;
        } else if (rlstExpression.Current().IsString())
        {
            if (bIdentifier) throw CCompileException(rlstExpression.LastValid(), "Cannot mix strings and identifiers.");

            // Check for UTF-8 and ASCII using the same data type
            const CDeclarationEntity* pDeclaration = Get<CDeclarationEntity>();
            if (pDeclaration &&
                ((pDeclaration->GetBaseType() == sdv::idl::EDeclType::decltype_string && !rlstExpression.Current().IsAscii()) ||
                 (pDeclaration->GetBaseType() == sdv::idl::EDeclType::decltype_u8string && !rlstExpression.Current().IsUtf8())))
                throw CCompileException(rlstExpression.LastValid(), "The string value doesn't fit the target value.");
            prValue.first = rlstExpression.Current().ValueRef();
            ++rlstExpression;
        } else
            throw CCompileException(rlstExpression.LastValid(), "Unexpected token '", static_cast<std::string>(rlstExpression.Current()), "'.");

        bInitial = false;
    };
    return prValue;
}

CTypeDeclaration CEntity::ProcessType(bool bNoSearchError /*= false*/)
{
    CTypeDeclaration typedecl;
    CLog log("Processing the type...");

    // Get the type identifier.
    CToken token = GetToken();
    if (token.GetType() != ETokenType::token_identifier && token.GetType() != ETokenType::token_keyword && token != "::")
        throw CCompileException(token, "Expecting a type identifier.");
    typedecl.SetTypeString(static_cast<std::string>(token));

    // Special cases: unsigned and long
    if (token == "long")
    {
        token = GetToken();
        if (token == "long") typedecl.AddTypeString(" long");
        else if (token == "double") typedecl.AddTypeString(" double");
        else PrependToken(token);
    } else if (token == "unsigned")
    {
        token = GetToken();
        if (token == "short") typedecl.AddTypeString(" short");
        else if (token == "long")
        {
            typedecl.AddTypeString(" long");
            token = GetToken();
            if (token == "long") typedecl.AddTypeString(" long");
            else PrependToken(token);
        }
    }

    // Check for a defined type?
    typedecl.SetBaseType(StringToDeclType(typedecl.GetTypeString()));

    // If the type is enum, struct or union or if the type could not be estimated, set the scoped name flag to potentially get the
    // rest of the name.
    std::pair<std::string, CEntityPtr> prEntity;
    switch (typedecl.GetBaseType())
    {
        case sdv::idl::EDeclType::decltype_enum:
            log << "Declaration type is 'enum'..." << std::endl;

            prEntity = ProcessScopedName(bNoSearchError);
            typedecl.SetTypeString(prEntity.first);
            log << "Declaration uses type " << typedecl.GetTypeString() << "..." << std::endl;
            if (!prEntity.second->Get<CEnumEntity>())
                throw CCompileException(token, "Provided identifier is not an enum.");
            typedecl.SetTypeDefinitionEntityPtr(prEntity.second);
            break;
        case sdv::idl::EDeclType::decltype_struct:
            log << "Declaration type is struct..." << std::endl;

            prEntity = ProcessScopedName(bNoSearchError);
            typedecl.SetTypeString(prEntity.first);
            log << "Declaration uses type " << typedecl.GetTypeString() << "..." << std::endl;
            if (!prEntity.second->Get<CStructEntity>())
                throw CCompileException(token, "Provided identifier is not a struct.");
            typedecl.SetTypeDefinitionEntityPtr(prEntity.second);
            break;
        case sdv::idl::EDeclType::decltype_interface:
            {
            log << "Declaration type is interface..." << std::endl;

            prEntity = ProcessScopedName(bNoSearchError);
            typedecl.SetTypeString(prEntity.first);
            log << "Declaration uses type " << typedecl.GetTypeString() << "..." << std::endl;
            if (!prEntity.second->Get<CInterfaceEntity>())
                throw CCompileException(token, "Provided identifier is not an interface.");
            typedecl.SetTypeDefinitionEntityPtr(prEntity.second);

            break;
        }
        case sdv::idl::EDeclType::decltype_union:
            log << "Declaration type is 'union'..." << std::endl;

            prEntity = ProcessScopedName(true); // Could be without name union followed by switch directly
            typedecl.SetTypeString(prEntity.first);
            log << "Declaration uses type " << typedecl.GetTypeString() << "..." << std::endl;
            if (!prEntity.second->Get<CUnionEntity>())
                throw CCompileException(token, "Provided identifier is not an interface.");
            typedecl.SetTypeDefinitionEntityPtr(prEntity.second);
            break;
		case sdv::idl::EDeclType::decltype_fixed:
            // Check for optional fixed length parameter
            token = PeekToken();
            if (token != "<") break;
            GetToken(); // Skip the bracket...
            // Get the size literal
            token = GetToken();
            if (token.GetType() != ETokenType::token_literal || !token.IsInteger() || token.ValueRef().Get<uint32_t>() < 1)
                throw CCompileException(token, "Expecting a positive number for the fixed size of the fixed point type.");
            typedecl.SetFixedLength(token.ValueRef().Get<uint32_t>());
            token = GetToken();
            if (token != ",")
                throw CCompileException(token, "Expecting a comma ','.");
            // Get the decimals literal
            token = GetToken();
            if (token.GetType() != ETokenType::token_literal || !token.IsInteger() ||
                token.ValueRef().Get<uint32_t>() > typedecl.GetFixedLength())
                throw CCompileException(token, "Expecting a positive number or zero for the decimals of the fixed point type.");
            typedecl.SetDecimals(token.ValueRef().Get<uint32_t>());
            token = GetToken();
            if (token == ">>") //!< Mistakingly detected as shift operator, but closing first nested template expression.
            {
                token = CToken(">", ETokenType::token_operator);
                PrependToken(token);
            }
            if (token != ">")
                throw CCompileException(token, "Expecting '>' closing the template section of the string.");
            break;
        case sdv::idl::EDeclType::decltype_string:
        case sdv::idl::EDeclType::decltype_wstring:
        case sdv::idl::EDeclType::decltype_u16string:
        case sdv::idl::EDeclType::decltype_u32string:
        case sdv::idl::EDeclType::decltype_u8string:
            // Check for optional fixed length parameter
            token = PeekToken();
            if (token != "<") break;
            GetToken(); // Skip the bracket...
            // Get the size literal
            token = GetToken();
            if (token.GetType() != ETokenType::token_literal || !token.IsInteger() || token.ValueRef().Get<uint32_t>() < 1)
                throw CCompileException(token, "Expecting a positive number for the fixed size of the string.");
            typedecl.SetFixedLength(token.ValueRef().Get<uint32_t>());
            token = GetToken();
            if (token == ">>") //!< Mistakingly detected as shift operator, but closing first nested template expression.
            {
                token = CToken(">", ETokenType::token_operator);
                PrependToken(token);
            }
            if (token != ">")
                throw CCompileException(token, "Expecting '>' closing the template section of the string.");
            break;
		case sdv::idl::EDeclType::decltype_pointer:
		case sdv::idl::EDeclType::decltype_sequence:
            // Check for data type and fixed length parameter (latter is optional).
            if (GetToken() != "<")
                throw CCompileException(token, "Expecting '<' defining the template section.");
            // Create a data type
            typedecl.SetValueTypePtr(std::make_shared<CTypeDeclaration>(ProcessType(bNoSearchError)));
            // Check for a comma or a closing bracket
            token = GetToken();
            if (token == ",")
            {
                // Get the size literal
                token = GetToken();
                if (token.GetType() != ETokenType::token_literal || !token.IsInteger() || token.ValueRef().Get<uint32_t>() < 1)
                    throw CCompileException(token, "Expecting a positive number for the fixed size.");
                typedecl.SetFixedLength(token.ValueRef().Get<uint32_t>());
                token = GetToken();
            }
			if (token == ">>") //!< Mistakingly detected as shift operator, but closing first nested template expression.
			{
				token = CToken(">", ETokenType::token_operator);
				PrependToken(token);
			}
            if (token != ">")
                throw CCompileException(token, "Expecting '>' closing the template section.");
            break;
		case sdv::idl::EDeclType::decltype_map:
            // Check for data types of key and value and fixed length parameter (latter is optional).
			if (GetToken() != "<")
				throw CCompileException(token, "Expecting '<' defining the template section.");
			// Create a data type for the key
			typedecl.SetKeyTypePtr(std::make_shared<CTypeDeclaration>(ProcessType(bNoSearchError)));
			// Check for a comma
			token = GetToken();
			if (token != ",") throw CCompileException(token, "Expecting a comma ','.");
			// Create a data type for the value
			typedecl.SetValueTypePtr(std::make_shared<CTypeDeclaration>(ProcessType(bNoSearchError)));
			// Check for a comma or a closing bracket
			token = GetToken();
			if (token == ",")
			{
				// Get the size literal
				token = GetToken();
				if (token.GetType() != ETokenType::token_literal || !token.IsInteger() || token.ValueRef().Get<uint32_t>() < 1)
					throw CCompileException(token, "Expecting a positive number for the fixed size.");
				typedecl.SetFixedLength(token.ValueRef().Get<uint32_t>());
				token = GetToken();
			}
			if (token == ">>") //!< Mistakingly detected as shift operator, but closing first nested template expression.
			{
				token = CToken(">", ETokenType::token_operator);
				PrependToken(token);
			}
			if (token != ">")
				throw CCompileException(token, "Expecting '>' closing the template section.");
			break;
		case sdv::idl::EDeclType::decltype_unknown:
            {
                // The type string still contains (part) of the scoped type name. Parse again
                CToken tokenType(typedecl.GetTypeString());
                PrependToken(tokenType);
                prEntity = ProcessScopedName(bNoSearchError);

                typedecl.SetTypeString(prEntity.first);
                if (!prEntity.second)
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (system type)..." << std::endl;
                else if (prEntity.second->Get<CUnionEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (union)..." << std::endl;
                    typedecl.SetBaseType(sdv::idl::EDeclType::decltype_union);
                }
                else if (prEntity.second->Get<CExceptionEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (union)..." << std::endl;
                    typedecl.SetBaseType(sdv::idl::EDeclType::decltype_union);
                }
                else if (prEntity.second->Get<CStructEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (struct)..." << std::endl;
                    typedecl.SetBaseType(sdv::idl::EDeclType::decltype_struct);
                }
                else if (prEntity.second->Get<CEnumEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (enum)..." << std::endl;
                    typedecl.SetBaseType(sdv::idl::EDeclType::decltype_enum);
                }
                else if (prEntity.second->Get<CTypedefEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (typedef)..." << std::endl;
                    typedecl.SetBaseType(prEntity.second->Get<CTypedefEntity>()->GetBaseType());
                }
                else if (prEntity.second->Get<CInterfaceEntity>())
                {
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (interface)..." << std::endl;
                    typedecl.SetBaseType(sdv::idl::EDeclType::decltype_interface);
                }
                else
                    log << "Declaration is of type " << typedecl.GetTypeString() << " (system type)..." << std::endl;

                typedecl.SetTypeDefinitionEntityPtr(prEntity.second);
            }
            break;
        default:
            log << "Declaration type is '" << typedecl.GetTypeString() << "'..." << std::endl;

            break;
    }


    // The following should not occur!
    if (!bNoSearchError && typedecl.GetBaseType() == sdv::idl::EDeclType::decltype_unknown)
        throw CCompileException(token, "Declaration type is invalid.");


    return typedecl;
}

void CEntity::SetName(const std::string& rssName, bool bForwardDeclaration /*= false*/, bool bNoInsert /*= false*/)
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    if (!m_ssName.empty())
        throw CCompileException("Internal error: entity name was set twice.");

    // Assign the name
    if (rssName.empty())
        throw CCompileException("Internal error: name was not provided while assigning it to the entity.");
    else
        m_ssName = rssName;

    // Check at the parent for an existing entity with the same name.
    bool bIsDeclaration = Get<CDeclarationEntity>() && !Get<CTypedefEntity>() ? true : false;
    if (!m_ptrParent) throw CCompileException("Internal error: parent was not set for this entity.");
    std::pair<CEntityPtr, bool> prEntity = m_ptrParent->FindLocal(m_ssName, bIsDeclaration);

    // Create lower case name to use as key in the entity map.
    std::string ssLCName = m_ssName;
    if (!m_pParser->GetEnvironment().CaseSensitiveTypeExtension())
    {
        std::transform(ssLCName.begin(), ssLCName.end(), ssLCName.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }

    // Is this entity a declaration only, then set the flag.
    m_bForwardDeclaration = bForwardDeclaration;

    if (prEntity.first)  // Entity with same name exists already
    {
        if (prEntity.second) throw CCompileException("An identifier with the same name exists already through inheritance.");

        // Is this entity a declararion only, then ignore this declaration.
        if (bForwardDeclaration) return;

        // Check whether extendability is not allowed for this entity.
        if (!prEntity.first->m_bForwardDeclaration && !prEntity.first->IsExtendable())
            throw CCompileException("An identifier with the same name exists already (case insensitive name checking).");

        // Check whether the entity is of the same type.
        if (GetType() != prEntity.first->GetType())
            throw CCompileException("An identifier with the same name exists already (case insensitive name checking).");

        // Is a shared children map existing? If not, create one.
        if (!prEntity.first->m_ptrChildDefMap) prEntity.first->m_ptrChildDefMap = std::make_shared<CEntityMap>();
        if (!prEntity.first->m_ptrChildDeclMap) prEntity.first->m_ptrChildDeclMap = std::make_shared<CEntityMap>();

        // Use the existing children map
        m_ptrChildDefMap = prEntity.first->m_ptrChildDefMap;
        m_ptrChildDeclMap = prEntity.first->m_ptrChildDeclMap;

        // Update the child map entry if the stored map entry is only a forward declaration
        if (prEntity.first->m_bForwardDeclaration)
        {
            if (bIsDeclaration)
                m_ptrParent->m_ptrChildDeclMap->at(ssLCName) = shared_from_this();
            else
                m_ptrParent->m_ptrChildDefMap->at(ssLCName) = shared_from_this();
        }
    } else
    {
        if (!bNoInsert)
        {
            // Does the parent have a children map? If not, create one.
            if (!m_ptrParent->m_ptrChildDefMap) m_ptrParent->m_ptrChildDefMap = std::make_shared<CEntityMap>();
            if (!m_ptrParent->m_ptrChildDeclMap) m_ptrParent->m_ptrChildDeclMap = std::make_shared<CEntityMap>();

            // Assign the entity to the children map of the parent.
            if (bIsDeclaration)
                m_ptrParent->m_ptrChildDeclMap->insert(CEntityMap::value_type(ssLCName, shared_from_this()));
            else
                m_ptrParent->m_ptrChildDefMap->insert(CEntityMap::value_type(ssLCName, shared_from_this()));
        }
    }
}

void CEntity::ProcessSystemTypeAssignments()
{
    if (m_lstUnassigned.empty()) CLog() << "No unprocessed entity assignments found..." << std::endl;

    // Go through the list of unassigned entities and process all entity assignments.
    while (!m_lstUnassigned.empty())
    {
        CDeclarationEntity* pEntity = m_lstUnassigned.front()->Get<CDeclarationEntity>();
        if (pEntity) pEntity->PostProcess();
        m_lstUnassigned.pop_front();
    }
}

void CEntity::AddChild(CEntityPtr ptrChild)
{
    // Add the entity to the list of children.
    m_vecChildren.push_back(ptrChild);

    // If this is a declaration entity, add it to the unassigned list to allow assignment processing later
    if (ptrChild->Get<CDeclarationEntity>())
        m_lstUnassigned.push_back(ptrChild);
}

std::pair<CEntityPtr, bool> CEntity::FindLocal(const std::string& rssName, bool bDeclaration) const
{
    if (!m_pParser)
        throw CCompileException("Internal error: no parser available.");
    if (rssName.empty())
        throw CCompileException("Internal error: name was not provided while requesting to search for it.");

    // The definition could have children of its own. Declarations could be of a type that has child declarations.
    bool bIsDeclaration = Get<CDeclarationEntity>() && !Get<CTypedefEntity>() ? true : false;

    // When reuse of names is allowed, check the names within a declaration entity only when the name belongs to a declarations.
    // Otherwise the entity is a definition; check the names of declatations and definitions accordingly to the provided type and
    // the reuse.
    if (m_pParser->GetEnvironment().ContextDependentNamesExtension())
    {
        if (bIsDeclaration && !bDeclaration) return std::make_pair(CEntityPtr(), false);
    }

    // Differentiate between definition and declaration.
    if (bIsDeclaration) // Declaration
    {
        // Get the declarations from the definition
        CEntityPtr ptrTypeEntity = Get<CDeclarationEntity>()->GetTypeEntity();
        const CDefinitionEntity* pDefinition = ptrTypeEntity ? ptrTypeEntity->Get<CDefinitionEntity>() : nullptr;
        if (!pDefinition) return std::make_pair(CEntityPtr(), false);

        // Iterate through the declarations and check for the name
        for (CEntityPtr ptrDecl : pDefinition->GetDeclMembers())
            if (ptrDecl->GetName() == rssName) return std::make_pair(ptrDecl, false);
    }
    else // Definitions
    {
        // Is there a shared children map? If not, we're done...
        if (bDeclaration && !m_ptrChildDeclMap) return std::make_pair(CEntityPtr(), false);
        if (!bDeclaration && !m_ptrChildDefMap) return std::make_pair(CEntityPtr(), false);

        // Create lower case name to be able to do case-insensitive comparison.
        std::string ssLCName = rssName;
        if (!m_pParser->GetEnvironment().CaseSensitiveTypeExtension())
        {
            std::transform(ssLCName.begin(), ssLCName.end(), ssLCName.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        }

        // Search for an entity with the same name.
        if (bDeclaration || !m_pParser->GetEnvironment().ContextDependentNamesExtension())
        {
            CEntityMap::const_iterator itEntity = m_ptrChildDeclMap->find(ssLCName);
            if (itEntity != m_ptrChildDeclMap->end()) return std::make_pair(itEntity->second, false);

            // For each anonymous child entity, do the check.
            for (CEntityMap::value_type vtEntity : *m_ptrChildDeclMap)
            {
                if (vtEntity.second->Get<CDeclarationEntity>() && vtEntity.second->Get<CDeclarationEntity>()->IsAnonymous())
                {
                    std::pair<CEntityPtr, bool> prChild = vtEntity.second->FindLocal(rssName, bDeclaration);
                    if (prChild.first) return prChild;
                }
            }
        }
        if (!bDeclaration || !m_pParser->GetEnvironment().ContextDependentNamesExtension())
        {
            CEntityMap::const_iterator itEntity = m_ptrChildDefMap->find(ssLCName);
            if (itEntity != m_ptrChildDefMap->end()) return std::make_pair(itEntity->second, false);
        }
    }

    // No entity found.
    return std::make_pair(CEntityPtr(), false);
}

CEntityPtr CEntity::FindMember(const std::string& rssScopedName) const
{
    if (rssScopedName.empty()) return nullptr;
    size_t nMemberSeparator = rssScopedName.find(".");

    // Is this a variable? Then get the type.
    CEntityPtr ptrEntity;
    if (Get<CVariableEntity>())
        ptrEntity = Get<CVariableEntity>()->GetTypeEntity();

    // Not a definition, nor a variable - no members defined
    if (!ptrEntity) return nullptr;

    // Resolve any typedefs.
    ptrEntity = ptrEntity->GetResolvedEntity();

    // Is this a compound entity (any entity derived from the struct entity)?
    const CStructEntity* pStructEntity = ptrEntity->Get<CStructEntity>();
    if (!pStructEntity) return nullptr;

    // Go through the declarations from the struct
    CEntityList lstEntities = pStructEntity->GetDeclMembers();
    for (CEntityPtr ptrDecl : lstEntities)
    {
        CEntityPtr ptrChild;
        const CDeclarationEntity* pDeclaration = ptrDecl->Get<CDeclarationEntity>();
        if (!pDeclaration) continue;

        // If the declaration is anonympous, forward the call directly to the declaration (without name checking)
        if (pDeclaration->IsAnonymous())
            ptrChild = ptrDecl->FindMember(rssScopedName);
        else
        {
            // Check for the fitting name
            if (ptrDecl->GetName() != rssScopedName.substr(0, nMemberSeparator)) continue;

            // Any more members?
            if (nMemberSeparator == std::string::npos)
                ptrChild = ptrDecl;
            else
                ptrChild = ptrDecl->FindMember(rssScopedName.substr(nMemberSeparator + 1));
        }

        // Child found?
        if (ptrChild) return ptrChild;
    }

    // When here, no child found
    return nullptr;
}

CEntity::CEntityIterator::CEntityIterator(CEntityVector& rvecEntities) : m_rvecEntities(rvecEntities)
{}

sdv::interface_t CEntity::CEntityIterator::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IEntityIterator>())
        return static_cast<sdv::idl::IEntityIterator*>(this);
    return nullptr;
}

uint32_t CEntity::CEntityIterator::GetCount() const
{
    return static_cast<uint32_t>(m_rvecEntities.size());
}

sdv::IInterfaceAccess* CEntity::CEntityIterator::GetEntityByIndex(uint32_t uiIndex)
{
    if (static_cast<size_t>(uiIndex) > m_rvecEntities.size()) return nullptr;
    return static_cast<sdv::IInterfaceAccess*>(m_rvecEntities[static_cast<size_t>(uiIndex)].get());
}

std::string CEntity::DeclTypeToString(sdv::idl::EDeclType eType) const
{
    for (const auto& rsEntry : m_vecDeclTypes)
        if (rsEntry.second == eType) return rsEntry.first;
    return std::string();
}

sdv::idl::EDeclType CEntity::StringToDeclType(const std::string& rssType) const
{
    for (const auto& rsEntry : m_vecDeclTypes)
        if (rsEntry.first == rssType) return rsEntry.second;
    return sdv::idl::EDeclType::decltype_unknown;
}
