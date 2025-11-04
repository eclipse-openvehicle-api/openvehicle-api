#include "variable_entity.h"
#include "typedef_entity.h"
#include "../exception.h"

CVariableEntity::CVariableEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bConst, bool bAnonymous) :
    CDeclarationEntity(rptrContext, ptrParent), m_bConst(bConst), m_bAnonymous(bAnonymous)
{}

CVariableEntity::CVariableEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, const CTokenList& rlstTokenList,
    bool bConst, bool bAnonymous) :
    CDeclarationEntity(rptrContext, ptrParent, rlstTokenList), m_bConst(bConst), m_bAnonymous(bAnonymous)
{}

std::string CVariableEntity::GetDeclTypeStr(bool bResolveTypedef) const
{
    if (GetTypeEntity())
        return GetTypeEntity()->GetDeclTypeStr(bResolveTypedef);
    else
        return DeclTypeToString(GetBaseType());
}

void CVariableEntity::Process()
{
    CDeclarationEntity::Process();

    // TODO: Const variables cannot contain:
    // - dynamic arrays when no assignment is there
    // - interfaces
    // - structure or unions with unassigned dynamic arrays or interfaces
}

