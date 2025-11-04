#include "operation_entity.h"
#include "interface_entity.h"

COperationEntity::COperationEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDeclarationEntity(rptrContext, ptrParent), m_iteratorParameters(GetParamVector()), 
    m_iteratorExceptions(GetExceptionVector())
{}

sdv::interface_t COperationEntity::GetInterface(sdv::interface_id idInterface)
{
    // Expose interfaces
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IOperationEntity>())
        return static_cast<sdv::idl::IOperationEntity*>(this);
    return CDeclarationEntity::GetInterface(idInterface);
}

sdv::idl::IEntityIterator* COperationEntity::GetParameters()
{
    if (!GetParamVector().empty()) return &m_iteratorParameters;
    return nullptr;
}

sdv::idl::IEntityIterator* COperationEntity::GetExceptions()
{
    if (!GetExceptionVector().empty()) return &m_iteratorExceptions;
    return nullptr;
}

std::pair<CEntityPtr, bool> COperationEntity::FindLocal(const std::string& rssName, bool /*bDeclaration*/) const
{
    const CEntityVector& rvecParams = GetParamVector();
    for (const CEntityPtr& rptrEntity : rvecParams)
    {
        if (rptrEntity->GetName() == rssName)
            return std::make_pair(rptrEntity, false);
    }
    return std::make_pair(nullptr, false);
}

bool COperationEntity::RequiresAssignment() const
{
    const CInterfaceEntity* pInterface = GetParentEntity() ? GetParentEntity()->Get<CInterfaceEntity>() : nullptr;
    if (pInterface && pInterface->IsLocal()) return false;
    return CDeclarationEntity::RequiresAssignment();
}
