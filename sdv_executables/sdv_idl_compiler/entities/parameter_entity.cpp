#include "parameter_entity.h"
#include "interface_entity.h"
#include "operation_entity.h"
#include "../exception.h"

CParameterEntity::CParameterEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent,
    const CTokenList& rlstTokenList, bool bEnforceDirection /*= true*/) :
    CDeclarationEntity(rptrContext, ptrParent, rlstTokenList), m_bEnforceDirection(bEnforceDirection)
{}

sdv::interface_t CParameterEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IParameterEntity>())
        return static_cast<sdv::idl::IParameterEntity*>(this);
    return CDeclarationEntity::GetInterface(idInterface);
}

void CParameterEntity::Process()
{
    // Check for direction indicators.
    CToken token = GetToken();
    if (token == "in") m_eDirection = sdv::idl::IParameterEntity::EParameterDirection::input;
    else if (token == "out") m_eDirection = sdv::idl::IParameterEntity::EParameterDirection::output;
    else if (token == "inout") m_eDirection = sdv::idl::IParameterEntity::EParameterDirection::in_out;
    else
    {
        PrependToken(token);

        // Enforce the direction?
        if (m_bEnforceDirection) throw CCompileException(token, "Missing direction indicator.");
    }

    // Let the basic type entity process further
    CDeclarationEntity::Process();
}

bool CParameterEntity::RequiresAssignment() const
{
    COperationEntity* pOperation = GetParentEntity() ? GetParentEntity()->Get<COperationEntity>() : nullptr;
    const CInterfaceEntity* pInterface = pOperation && pOperation->GetParentEntity() ? pOperation->GetParentEntity()->Get<CInterfaceEntity>() : nullptr;
    if (pInterface && pInterface->IsLocal()) return false;
    return CDeclarationEntity::RequiresAssignment();
}

void CParameterEntity::CalcHash(CHashObject& rHash) const
{
    // Call base class
    CDeclarationEntity::CalcHash(rHash);

    // Add the direction
    switch (m_eDirection)
    {
    case sdv::idl::IParameterEntity::EParameterDirection::input:
        rHash << "in";
        break;
    case sdv::idl::IParameterEntity::EParameterDirection::output:
        rHash << "out";
        break;
    case sdv::idl::IParameterEntity::EParameterDirection::in_out:
        rHash << "inout";
        break;
    default:
        break;
    }

}
