#include "typedef_entity.h"
#include "../exception.h"

CTypedefEntity::CTypedefEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDeclarationEntity(rptrContext, ptrParent)
{

}

CTypedefEntity::CTypedefEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent,
    const std::string& rssName) :
    CDeclarationEntity(rptrContext, ptrParent)
{
    if (rssName.empty()) throw CCompileException("Internal error: trying to create a named type entity without valid name.");
    SetName(rssName, true, true);
}

std::string CTypedefEntity::GetDeclTypeStr(bool bResolveTypedef) const
{
    // When not resolving the typedef, return the default implementation ("typedef <name>").
    if (!bResolveTypedef) return std::string("typedef ") + GetScopedName();

    // Otherwise resolve the typedef by taking the base type.
    if (GetTypeEntity())
        return GetTypeEntity()->GetDeclTypeStr(bResolveTypedef);
    else
        return DeclTypeToString(GetBaseType());
}

void CTypedefEntity::Process()
{
    CDeclarationEntity::Process();
}

std::pair<CEntityPtr, bool> CTypedefEntity::FindLocal(const std::string& rssName, bool bDeclaration) const
{
    // Call the find local function on the original type.
    if (!GetTypeEntity()) return std::pair<CEntityPtr, bool>();

    return GetTypeEntity()->FindLocal(rssName, bDeclaration);
}



