#include "attribute_entity.h"

CAttributeEntity::CAttributeEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bReadOnly) :
    CDeclarationEntity(rptrContext, ptrParent), m_bReadOnly(bReadOnly),
    m_iteratorReadExceptions(GetReadExceptionVector()), m_iteratorWriteExceptions(GetWriteExceptionVector())
{}

sdv::interface_t CAttributeEntity::GetInterface(sdv::interface_id idInterface)
{
    // Expose interfaces
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IAttributeEntity>())
        return static_cast<sdv::idl::IAttributeEntity*>(this);
    return CDeclarationEntity::GetInterface(idInterface);
}

sdv::idl::IEntityIterator* CAttributeEntity::GetReadExceptions()
{
    if (!GetReadExceptionVector().empty()) return &m_iteratorReadExceptions;
    return nullptr;
}

sdv::idl::IEntityIterator* CAttributeEntity::GetWriteExceptions()
{
    if (!GetWriteExceptionVector().empty()) return &m_iteratorWriteExceptions;
    return nullptr;
}

void CAttributeEntity::Process()
{
    CDeclarationEntity::Process();
}

bool CAttributeEntity::SupportArrays() const
{
    return false;
}

bool CAttributeEntity::IsReadOnly() const
{
    return m_bReadOnly;
}

bool CAttributeEntity::SupportMultipleDeclarations() const
{
    return true;
}

bool CAttributeEntity::SupportRaiseExceptions() const
{
    return true;
}

bool CAttributeEntity::SupportSeparateSetGetRaiseExceptions() const
{
    return true;
}

